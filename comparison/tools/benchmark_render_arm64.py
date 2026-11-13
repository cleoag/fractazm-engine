"""Sequential full CPU/Metal renders with fresh-process and reused-context samples.

Cold means a new process/context, never a claim that OS/driver caches were purged.
Numerical qualification and effect-off controls execute outside measured runs.
"""
from __future__ import annotations

import argparse
import hashlib
import html
import json
import math
import os
from pathlib import Path, PurePosixPath
import re
import shutil
import statistics
import subprocess
import sys

from PIL import Image
from benchmark_gallery_arm64 import (ROOT, binary_identity, execute, inspect_png,
                                      native_host, relative_file, sha, utc_now, write_json)
from native_evidence import source_identity

SCHEMA = "mb3d.full-render-benchmark.v1"
SELECTION_SCHEMA = "mb3d.render-benchmark-selection.v1"
CPU_STAGES = {"preparation", "volume", "primary", "normals", "shadows", "ao",
              "color", "reflections", "dof"}
TIMINGS = ("render_ms", "readback_ms", "render_and_readback_ms", "png_encode_write_ms", "end_to_end_ms")
METAL_RENDER_SETTINGS = {"hard_shadows_override": False, "shadow_mode": "parallel",
                         "division_mode": "verified", "normal_mode": "parallel", "normal_threads": 32,
                         "primary_rows_per_command": 256, "primary_threads": 32, "primary_tile_width": 8,
                         "shadow_rows_per_command": 256, "shadow_prep_threads": 32, "shadow_trace_threads": 32}
METAL_STAGES = {"primary", "surface_packing", "shadow_preparation", "shadow_traversal",
                "hard_shadows", "ambient_occlusion", "final_color", "surface_copy"}
METAL_FAULTS = {"primary", "surface", "final_surface", "color", "shadow_rays", "shadow_traces",
                "normal_scratch", "latent_shadow_terminal_faults"}
BUILD_INPUT_SUFFIXES = {".c", ".cpp", ".h", ".hpp", ".mm", ".inc", ".metal"}
RECEIPT_BINARIES = {"cpu": "mb3d_cpu_render_benchmark", "renderer": "mb3d_render",
                    "metal": "mb3d_metal_exact_scene"}


def build_input_hashes(identity):
    """Compare build inputs, allowing later Python/report/runbook hardening."""
    changed = identity.get("changed_source_sha256")
    if not isinstance(changed, dict):
        raise ValueError("Build receipt/source changed-input inventory is missing")
    result = {}
    for name, digest in changed.items():
        if not isinstance(name, str):
            raise ValueError("Invalid build-input path")
        path = PurePosixPath(name)
        if path.name == "CMakeLists.txt" or path.suffix.lower() in BUILD_INPUT_SUFFIXES:
            if digest is not None and (not isinstance(digest, str) or not re.fullmatch(r"[0-9a-f]{64}", digest)):
                raise ValueError("Invalid build-input hash: " + name)
            result[name] = digest  # None retains a deleted build input.
    return result


def bootstrap_binding(path, source, identities):
    if path is None:
        return {"status": "NOT_SUPPLIED", "reason": "No build/test receipt supplied; source snapshot alone does not establish build identity"}
    path = path.resolve(strict=True)
    raw = path.read_bytes()
    receipt = json.loads(raw)
    if (not isinstance(receipt, dict) or receipt.get("status") != "PASS_SCOPED"
            or receipt.get("scope") != "ordinary" or receipt.get("machine") != "arm64"
            or not str(receipt.get("platform", "")).startswith(("macOS-", "Darwin-"))):
        raise ValueError("Build receipt must pass the ordinary native ARM64 scope")
    if receipt.get("source_commit") != source.get("source_commit"):
        raise ValueError("Build receipt source commit differs from current source")
    inputs = build_input_hashes(source)
    if build_input_hashes(receipt) != inputs:
        raise ValueError("Build receipt compiled/header/Metal inputs differ from current source")
    recorded = receipt.get("binaries", {})
    if not isinstance(recorded, dict):
        raise ValueError("Build receipt binary inventory is missing")
    bound = {}
    for role, name in RECEIPT_BINARIES.items():
        entry = recorded.get(name, {})
        actual = identities[role]["sha256"]
        if entry.get("arch") != "arm64" or entry.get("sha256") != actual:
            raise ValueError("Build receipt binary hash/architecture differs: " + name)
        bound[role] = {"name": name, "sha256": actual, "arch": "arm64"}
    return {"status": "VERIFIED", "receipt_path": str(path), "receipt_sha256": hashlib.sha256(raw).hexdigest(),
            "scope": "ordinary", "machine": "arm64", "source_commit": source["source_commit"],
            "build_input_changed_source_sha256": inputs, "binaries": bound,
            "comparison_scope": "CMakeLists.txt and .c/.cpp/.h/.hpp/.mm/.inc/.metal changes; Python orchestration and docs excluded",
            "metal_library": "Not present in bootstrap receipt; independently frozen and bound by Metal qualification"}


def controls(row):
    return row.get("effect_controls", []) or ([row["effect_control"]] if row.get("effect_control") else [])


def load_selection(path, root=ROOT, selected=()):
    value = json.loads(path.read_text())
    scenes = value.get("scenes", [])
    if value.get("schema") != SELECTION_SCHEMA or not scenes or value.get("expected_count") != len(scenes):
        raise ValueError("Invalid benchmark selection schema/count")
    if value.get("generator"):
        generator = value["generator"]
        if sha(relative_file(root, generator["path"])) != generator["sha256"]:
            raise ValueError("Fixture generator hash mismatch")
    ids = set()
    asset_ids = set()
    asset_targets = set()
    for asset in value.get("assets", []):
        if asset["id"] in asset_ids:
            raise ValueError("Duplicate asset ID")
        asset_ids.add(asset["id"])
        source = relative_file(root, asset["source_path"])
        target = PurePosixPath(asset["runtime_path"])
        if (not asset["runtime_path"].startswith("M3Maps/") or ".." in target.parts
                or "\\" in asset["runtime_path"] or str(target) in asset_targets
                or sha(source) != asset["sha256"]):
            raise ValueError("Invalid asset destination/hash")
        asset_targets.add(str(target))
    control_count = 0
    for row in scenes:
        name = row.get("id", "")
        if not re.fullmatch(r"[a-zA-Z0-9_-]+", name) or name in ids:
            raise ValueError("Invalid/duplicate scene ID")
        if any(token in name.lower() for token in ("montecarlo", "monte_carlo", "halton", "jit")):
            raise ValueError("Deferred scene is excluded")
        ids.add(name)
        if row.get("backends") not in (["cpu"], ["cpu", "metal"]):
            raise ValueError("Expected explicit CPU and optional Metal support")
        if "metal" not in row["backends"] and not row.get("unsupported_backends", {}).get("metal"):
            raise ValueError("CPU-only scene needs its Metal exclusion reason")
        if not set(row.get("required_stages", [])) <= CPU_STAGES:
            raise ValueError("Unknown required CPU stage")
        if not set(row.get("assets", [])) <= asset_ids:
            raise ValueError("Unknown asset ID")
        if not controls(row):
            raise ValueError("Every active-effect scene needs an effect-off control")
        for item in [row, *controls(row)]:
            file = relative_file(root, item["scene_path"])
            if file.suffix.lower() != ".m3p" or sha(file) != item["scene_sha256"]:
                raise ValueError("Scene hash/type mismatch: " + name)
        control_count += len(controls(row))
    if value.get("expected_control_count") != control_count:
        raise ValueError("Control inventory mismatch")
    if len(set(selected)) != len(selected) or set(selected) - ids:
        raise ValueError("Selection IDs must be unique and present")
    result = dict(value)
    result["full_selection_count"] = len(scenes)
    result["scenes"] = [s for s in scenes if not selected or s["id"] in selected]
    result["expected_count"] = len(result["scenes"])
    result["expected_control_count"] = sum(len(controls(s)) for s in result["scenes"])
    return result


def duration(value, name):
    if isinstance(value, bool) or not isinstance(value, (int, float)) or not math.isfinite(value) or value < 0:
        raise ValueError("Invalid nonnegative duration: " + name)
    return value


def stats(values):
    if not values:
        raise ValueError("Empty sample set")
    values = [duration(v, "sample") for v in values]
    mean = statistics.mean(values)
    return {"count": len(values), "median": statistics.median(values), "min": min(values),
            "max": max(values), "mean": mean, "population_stddev": statistics.pstdev(values),
            "cv": statistics.pstdev(values) / mean if mean else 0, "values": values}


def image_details(path, width, height):
    result = inspect_png(path, width, height)
    with Image.open(path) as im:
        result["nonconstant"] = any(lo != hi for lo, hi in im.convert("RGB").getextrema())
    return result


def compare_images(left, right):
    with Image.open(left) as im:
        size = im.size
        a = im.convert("RGB").tobytes()
    with Image.open(right) as im:
        if im.size != size:
            raise ValueError("Comparison image dimensions differ")
        b = im.convert("RGB").tobytes()
    errors = [abs(x-y) for x, y in zip(a, b)]
    return {"different_channels": sum(v != 0 for v in errors),
            "different_pixels": sum(any(errors[i:i+3]) for i in range(0, len(errors), 3)),
            "max_channel_error": max(errors, default=0), "pixels": size[0]*size[1]}


def require_stages(stages, required, absent=()):
    for name in required:
        if stages.get(name, {}).get("state") != "completed" or duration(stages[name].get("seconds"), name) <= 0:
            raise ValueError("Required active stage did not run: " + name)
    for name in absent:
        if stages.get(name, {}).get("state") != "not_run":
            raise ValueError("Disabled control stage ran: " + name)


def metal_count(value, name, maximum=None):
    if type(value) is not int or value < 0 or (maximum is not None and value > maximum):
        raise ValueError("Invalid Metal count: " + name)
    return value


def validate_metal_metadata(report, required_stages):
    if (report.get("precision") != "compatible" or report.get("backend") != "metal" or report.get("native_arm64") is not True
            or report.get("translated") is not False or report.get("exact_rgb_repeats") is not True
            or report.get("exact_stage_repeats") is not True or not report.get("device")):
        raise ValueError("Metal backend/native/repeat metadata is incomplete")
    for name in ("cpu_compute_fallback_calls", "cpu_oracle_renders", "diagnostic_dispatches"):
        if metal_count(report.get(name), name) != 0:
            raise ValueError("Render-only Metal benchmark included non-render work")
    if any(report.get("settings", {}).get(k) != v for k, v in METAL_RENDER_SETTINGS.items()):
        raise ValueError("Metal render profile differs from the qualified dispatch profile")
    features = report.get("loaded_features", {})
    if (features.get("ambient_occlusion") is not True or features.get("ao_mode") != "HiQ"
            or features.get("de_normals") is not True or features.get("formula_slots") != 1
            or type(features.get("hard_shadows")) is not bool
            or any(features.get(k) is not False for k in ("zbuffer_normals", "reflections_transparency", "volume", "dof"))):
        raise ValueError("Metal scene features do not match the supported render pipeline")
    if set(required_stages) - {"preparation", "primary", "shadows", "ao", "color"}:
        raise ValueError("Required scene stage is unsupported by the exact Metal pipeline")
    if "shadows" in required_stages and features["hard_shadows"] is not True:
        raise ValueError("Required Metal hard shadows are disabled")
    metal_count(features.get("shadow_light_mask"), "shadow light mask", 63)
    if metal_count(features.get("ao_levels"), "AO levels") == 0:
        raise ValueError("Metal AO levels are missing")
    expected_pipelines = {"exact_primary_geometry", "exact_normal_probes", "exact_normal_finish", "exact_surface",
                          "exact_ao_init", "exact_ao_sample", "exact_ao_wavelet_h", "exact_ao_wavelet_v", "exact_color"}
    if features["hard_shadows"]:
        expected_pipelines |= {"exact_shadow_prepare", "exact_shadow_trace", "exact_hard_shadow_traced"}
    pipelines = report.get("render_pipeline_names")
    if not isinstance(pipelines, list) or len(pipelines) != len(expected_pipelines) or set(pipelines) != expected_pipelines:
        raise ValueError("Metal benchmark contains missing or non-render pipelines")
    setup = report.get("setup_ms", {})
    names = {"scene_preparation", "shading_preparation", "device_library_queue", "pipeline_creation", "buffer_allocation", "total"}
    if set(setup) != names or any(duration(setup[k], "Metal setup " + k) <= 0 for k in names):
        raise ValueError("Metal setup timing inventory is incomplete")
    if setup["total"] + 0.01 < sum(setup[k] for k in names - {"total"}):
        raise ValueError("Metal setup total excludes a measured preparation phase")


def validate_metal_frame(frame, report, settings, required_stages):
    pixels = settings["width"] * settings["height"]
    faults = frame.get("faults")
    if frame.get("exact_stage_records_match_cold") is not True or not isinstance(faults, dict) or set(faults) != METAL_FAULTS:
        raise ValueError("Metal stage repeat or fault inventory is incomplete")
    for name, value in faults.items():
        value = metal_count(value, name, pixels)
        # A trace terminal fault may be suppressed by the ordered row shortcut;
        # any unsuppressed fault must appear in final_surface and fail here.
        if name != "latent_shadow_terminal_faults" and value:
            raise ValueError("Metal stage fault gate failed: " + name)
    stages = frame.get("stages", {})
    gpu_stages = frame.get("stages_gpu_ms", {})
    if set(stages) != METAL_STAGES or set(gpu_stages) != METAL_STAGES:
        raise ValueError("Metal stage timing inventory is incomplete")
    shadows = report["loaded_features"]["hard_shadows"]
    active = {"primary", "surface_packing", "ambient_occlusion", "final_color"}
    active |= {"shadow_preparation", "shadow_traversal", "hard_shadows"} if shadows else {"surface_copy"}
    require_stages(stages, active, METAL_STAGES - active)
    for name in METAL_STAGES:
        value = duration(gpu_stages[name], "GPU " + name)
        if (duration(stages[name].get("gpu_ms"), "stage GPU " + name) != value
                or (name in active and value <= 0) or (name not in active and value != 0)):
            raise ValueError("Metal GPU stage timestamps contradict executed work")
    total = duration(frame.get("gpu_timestamp_ms"), "GPU total")
    if not math.isclose(total, sum(gpu_stages.values()), rel_tol=1e-9, abs_tol=1e-6):
        raise ValueError("Metal GPU total differs from its non-overlapping stages")
    normal = frame.get("normal_pipeline_gpu_ms", {})
    if set(normal) != {"geometry", "probes", "finish"} or any(duration(v, "normal GPU") <= 0 for v in normal.values()):
        raise ValueError("Metal normal timing detail is incomplete")
    if not math.isclose(sum(normal.values()), gpu_stages["primary"], rel_tol=1e-9, abs_tol=1e-6):
        raise ValueError("Metal normal substeps differ from the primary total")
    activity = frame.get("activity", {})
    hits = metal_count(activity.get("hits"), "hits", pixels)
    misses = metal_count(activity.get("misses"), "misses", pixels)
    if hits + misses != pixels or not hits or not misses or activity.get("nonconstant_rgb") is not True:
        raise ValueError("Metal geometry activity lacks foreground/background coverage")
    shadow_changes = metal_count(activity.get("shadow_changed_pixels"), "shadow changed pixels", pixels)
    ao_changes = metal_count(activity.get("ao_changed_pixels"), "AO changed pixels", pixels)
    if ("shadows" in required_stages and not shadow_changes) or ("ao" in required_stages and not ao_changes):
        raise ValueError("Required Metal effect produced no changed surface pixels")
    if not shadows and shadow_changes:
        raise ValueError("Disabled Metal shadows changed the surface")
    render, readback, combined = (frame[k] for k in ("render_ms", "readback_ms", "render_and_readback_ms"))
    if combined + 0.01 < render + readback or render + 0.01 < 1000 * sum(s["seconds"] for s in stages.values()):
        raise ValueError("Metal frame timing excludes completed render/readback work")
    if not math.isclose(frame["end_to_end_ms"], combined + frame["png_encode_write_ms"], rel_tol=1e-9, abs_tol=1e-6):
        raise ValueError("Metal end-to-end timing excludes PNG output or includes validation")
    if frame["phase"] == "cold":
        cold = duration(frame.get("cold_total_ms"), "Metal cold total")
        end = duration(frame.get("cold_end_to_end_ms"), "Metal cold end-to-end")
        if cold + 0.01 < report["setup_ms"]["total"] + combined:
            raise ValueError("Metal cold total excludes setup or first render")
        if not math.isclose(end, cold + frame["png_encode_write_ms"], rel_tol=1e-9, abs_tol=1e-6):
            raise ValueError("Metal cold end-to-end boundary changed")


def validate_metal_qualification(summary, identities, scene, settings):
    if summary.get("schema") != "mb3d.metal-exact-shading-evidence.v1" or summary.get("status") != "PASS_SCOPED":
        raise ValueError("Numerical Metal qualification failed")
    expected = {k: settings[k] for k in ("width", "height", "threads")}
    expected.update(METAL_RENDER_SETTINGS)
    expected.pop("hard_shadows_override")
    expected.update(repeats=3, full_shading=True, hard_shadows=False,
                    primary_dispatch_sweep=False, shadow_dispatch_sweep=False)
    if any(summary.get("settings", {}).get(k) != v for k, v in expected.items()):
        raise ValueError("Metal qualification settings differ from benchmark settings")
    if (summary.get("probe", {}).get("sha256") != identities["metal"]["sha256"]
            or summary.get("library", {}).get("sha256") != identities["library"]["sha256"]
            or summary.get("scene", {}).get("scene_sha256") != scene["scene_sha256"]):
        raise ValueError("Metal qualification binary/library/scene identity differs")
    description = summary.get("native_scene_description", {}).get("scene", {})
    if description.get("scene_sha256") != scene["scene_sha256"]:
        raise ValueError("Metal qualification prepared scene identity differs")
    images = summary.get("images", [])
    if len(images) != 2 or {row.get("backend") for row in images} != {"gpu", "cpu"}:
        raise ValueError("Metal qualification lacks independent final CPU/GPU images")
    return {"probe_sha256": identities["metal"]["sha256"], "library_sha256": identities["library"]["sha256"],
            "scene_sha256": scene["scene_sha256"], "settings": expected,
            "scene_description": description, "numeric_profile": summary.get("numeric_profile")}


def validate_report(report, directory, backend, settings, required_stages=()):
    expected_schema = f"mb3d.{backend}-render-benchmark.v1"
    if report.get("schema") != expected_schema or report.get("status") != "COMPLETE":
        raise ValueError("Native benchmark is not complete with the expected schema")
    if any(report.get("settings", {}).get(k) != v for k, v in settings.items()):
        raise ValueError("Native benchmark settings changed")
    execution = report.get("execution", report.get("settings", {}))
    if backend == "cpu" and (execution.get("deterministic") is not True or execution.get("precision") != "compatible"):
        raise ValueError("CPU deterministic compatible profile is required")
    if backend == "metal":
        validate_metal_metadata(report, required_stages)
    if len(report.get("samples", [])) != settings["repeats"] or len(report.get("warmups", [])) != settings["warmups"]:
        raise ValueError("Missing warm samples/warmups")
    if not isinstance(report.get("cold"), dict):
        raise ValueError("Missing cold frame")
    frames = [report["cold"], *report["warmups"], *report["samples"]]
    identities = [("cold", 0), *[("warmup", i) for i in range(settings["warmups"])],
                  *[("warm", i) for i in range(settings["repeats"])]]
    seen = set()
    hashes = set()
    for frame, (phase, index) in zip(frames, identities):
        if frame.get("phase") != phase or frame.get("index") != index:
            raise ValueError("Missing or reordered frame identity")
        if frame.get("state", "completed") != "completed":
            raise ValueError("Native frame did not complete")
        for dimension in ("width", "height"):
            if frame.get(dimension) != settings[dimension]:
                raise ValueError("Native frame dimensions changed")
        for key in TIMINGS:
            duration(frame.get(key), key)
        if frame["render_ms"] <= 0 or frame["png_encode_write_ms"] <= 0:
            raise ValueError("Render/PNG timing must represent executed work")
        if frame.get("exact_rgb_matches_cold") is not True:
            raise ValueError("Native RGB repeat mismatch")
        if backend == "metal":
            validate_metal_frame(frame, report, settings, required_stages)
        if backend == "cpu":
            if not CPU_STAGES <= frame.get("stages", {}).keys():
                raise ValueError("CPU stage inventory is incomplete")
            require_stages(frame.get("stages", {}), required_stages)
        for name, stage in frame.get("stages", {}).items():
            seconds = duration(stage.get("seconds"), name)
            if stage.get("state") not in ("completed", "not_run") or (stage["state"] == "not_run" and seconds != 0):
                raise ValueError("Incomplete or contradictory stage: " + name)
        file = relative_file(directory, frame["image"])
        if str(file) in seen:
            raise ValueError("Repeated sample image path")
        seen.add(str(file))
        pixels = image_details(file, settings["width"], settings["height"])
        if not pixels["nonconstant"]:
            raise ValueError("Constant image cannot qualify active effects")
        frame["image_identity"] = pixels
        hashes.add(pixels["rgb_sha256"])
    if len(hashes) != 1:
        raise ValueError("Cold/warm decoded RGB differs")
    for key in ("cold_total_ms", "cold_end_to_end_ms"):
        duration(report["cold"].get(key), key)
    return next(iter(hashes))


def summarize_backend(processes):
    reports = [p["native"] for p in processes]
    # Warm frames are nested in independent processes. Report the distribution
    # of per-process medians as the primary statistic, not pseudoreplicated n.
    return {"cold_end_to_end_ms": stats([r["cold"]["cold_end_to_end_ms"] for r in reports]),
            "cold_to_rgb_ms": stats([r["cold"]["cold_total_ms"] for r in reports]),
            "warm_end_to_end_ms": stats([statistics.median(s["end_to_end_ms"] for s in r["samples"]) for r in reports]),
            "warm_to_rgb_ms": stats([statistics.median(s["render_and_readback_ms"] for s in r["samples"]) for r in reports]),
            "process_wall_seconds": stats([p["process"]["wall_seconds"] for p in processes]),
            "process_cpu_seconds": stats([p["process"]["cpu_seconds"] for p in processes]),
            "process_peak_rss_bytes": stats([p["process"]["peak_rss_bytes"] for p in processes]),
            "note": "Warm statistics use one median per fresh process. RSS covers the whole process, not isolated frames."}


def export_gallery(report, output, destination):
    destination.mkdir(parents=True, exist_ok=False)
    (destination/"images").mkdir()
    records = []
    body = ["<!doctype html><meta charset=utf-8><title>Mandelbulb CPU / Metal benchmark</title>",
            "<style>body{font:16px system-ui;background:#171923;color:#eee;margin:32px}table{border-collapse:collapse}td,th{padding:10px;border:1px solid #555}img{max-width:44vw}small{color:#bbb}</style>",
            "<h1>Full render benchmark</h1><p>Fresh process/context cold start; OS/driver caches unchanged. Warm frames reuse the process. Times include PNG encoding. Exact ARM CPU/Metal gates apply only to common scenes. x86/ARM: NOT_CHECKED in this native run.</p>"]
    for row in report["results"]:
        body.append("<h2>"+html.escape(row["id"])+"</h2><table><tr><th>Backend</th><th>Cold median ms</th><th>Warm median ms</th><th>Peak process MiB</th></tr>")
        for backend, data in row["backends"].items():
            if data.get("status") != "COMPLETE":
                body.append("<tr><td>"+backend+"</td><td colspan=3>"+html.escape(data.get("reason", data.get("status", "pending")))+"</td></tr>")
                continue
            summary = data["summary"]
            body.append(f"<tr><td>{backend}</td><td>{summary['cold_end_to_end_ms']['median']:.2f}</td><td>{summary['warm_end_to_end_ms']['median']:.2f}</td><td>{summary['process_peak_rss_bytes']['median']/2**20:.1f}</td></tr>")
        body.append("</table>")
        for backend, data in row["backends"].items():
            if data.get("status") != "COMPLETE":
                continue
            first = data["processes"][0]
            source = output/first["directory"]/first["native"]["cold"]["image"]
            name = row["id"]+"-"+backend+".png"
            shutil.copy2(source, destination/"images"/name)
            records.append({"scene": row["id"], "backend": backend, "image": "images/"+name,
                            **image_details(source, report["settings"]["width"], report["settings"]["height"])})
            body.append(f'<figure><figcaption>{backend}</figcaption><img src="images/{name}"></figure>')
    write_json(destination/"image-manifest.json", {"schema": SCHEMA, "images": records})
    shutil.copytree(output/"inputs", destination/"inputs")
    selection = json.loads((output/"inputs/selection.json").read_text())
    resources = {"schema": "mb3d.retained-render-resources.v1", "resource_root": "inputs/resources", "assets": []}
    for asset in selection.get("assets", []):
        source = relative_file(output/"runtime", asset["runtime_path"])
        if sha(source) != asset["sha256"]:
            raise ValueError("Runtime asset changed before gallery export")
        target = destination/"inputs/resources"/asset["runtime_path"]
        target.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(source, target)
        if sha(target) != asset["sha256"]:
            raise ValueError("Retained gallery asset hash mismatch")
        resources["assets"].append({"id": asset["id"], "runtime_path": asset["runtime_path"],
                                    "gallery_path": str(target.relative_to(destination)), "sha256": asset["sha256"]})
    (destination/"inputs/resources").mkdir(exist_ok=True)
    write_json(destination/"inputs/resource-manifest.json", resources)
    retained = dict(report)
    retained["retained_gallery"] = {"resource_root": resources["resource_root"],
                                    "resource_manifest": "inputs/resource-manifest.json", "image_manifest": "image-manifest.json"}
    write_json(destination/"summary.json", retained)
    (destination/"KEEP_FINAL_IMAGES.txt").write_text(
        "Final benchmark images, input scenes, runtime resources and report. Preserve during cleanup.\n"
        "Reproduce resource-backed scenes using resource root inputs/resources; see inputs/resource-manifest.json.\n")
    (destination/"index.html").write_text("\n".join(body))


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--cpu", type=Path, required=True, help="mb3d_cpu_render_benchmark")
    parser.add_argument("--renderer", type=Path, required=True, help="Production mb3d_render for untimed effect controls")
    parser.add_argument("--metal-probe", type=Path, required=True)
    parser.add_argument("--library", type=Path, required=True)
    parser.add_argument("--selection", type=Path, default=ROOT/"comparison/benchmark_scenes/effects-v1.json")
    parser.add_argument("--scene", action="append", default=[])
    parser.add_argument("--output", type=Path, required=True, help="Absent working leaf")
    parser.add_argument("--gallery", type=Path, help="Absent retained gallery directory, exported only after acceptance")
    parser.add_argument("--width", type=int, default=512)
    parser.add_argument("--height", type=int, default=384)
    parser.add_argument("--threads", type=int, default=6)
    parser.add_argument("--cold-repeats", type=int, default=3)
    parser.add_argument("--warmups", type=int, default=1)
    parser.add_argument("--repeats", type=int, default=3)
    parser.add_argument("--timeout", type=float, default=900)
    parser.add_argument("--min-free-gib", type=float, default=8)
    parser.add_argument("--bootstrap", type=Path, help="Current build/test receipt")
    args = parser.parse_args(argv)
    if (min(args.width, args.height, args.threads) < 1 or args.threads > min(64, args.height)
            or args.repeats < 3 or args.repeats > 15 or args.cold_repeats < 3 or args.warmups < 1
            or not math.isfinite(args.timeout) or args.timeout <= 0 or not math.isfinite(args.min_free_gib) or args.min_free_gib < 1):
        parser.error("Positive dimensions/timeout, >=3 cold and warm repeats, >=1 warmup, >=1 GiB free-space floor required")
    try:
        host = native_host()
        selection = load_selection(args.selection.resolve(), selected=args.scene)
        paths = {"cpu": args.cpu.resolve(strict=True), "renderer": args.renderer.resolve(strict=True),
                 "metal": args.metal_probe.resolve(strict=True), "library": args.library.resolve(strict=True)}
        identities = {k: binary_identity(v) if k != "library" else {"path": str(v), "sha256": sha(v)} for k, v in paths.items()}
        output = args.output.expanduser().resolve()
        if args.gallery and args.gallery.expanduser().exists():
            raise ValueError("Final gallery already exists; preserve it or choose an absent destination")
        if args.bootstrap and not args.bootstrap.is_file():
            raise ValueError("Build/test receipt missing")
        source = source_identity()
        receipt_binding = bootstrap_binding(args.bootstrap, source, identities)
        output.mkdir(parents=True, exist_ok=False)
    except (OSError, ValueError, subprocess.CalledProcessError) as error:
        parser.error(str(error))
    for name in ("inputs/scenes", "evidence/bin", "evidence/source", "runtime", "logs", "runs", "controls", "qualification"):
        (output/name).mkdir(parents=True)
    settings = {k: getattr(args, k) for k in ("width", "height", "threads", "warmups", "repeats")}
    report = {"schema": SCHEMA, "status": "RUNNING", "started_utc": utc_now(), "host": host,
              "settings": settings, "cold_repeats": args.cold_repeats, "source_identity": source,
              "bootstrap_binding": receipt_binding,
              "selection_sha256": sha(args.selection), "selected_count": len(selection["scenes"]),
              "full_selection_count": selection["full_selection_count"], "binaries": identities,
              "precision": "compatible",
              "method": {"cold": "fresh process/context, OS and driver caches not purged",
                         "warm_cpu": "same process and loaded input; each production Render owns fresh job resources",
                         "warm_metal": "same process, prepared scene, pipelines and buffers reused",
                         "end_to_end": "input/preparation (cold), render to RGB, final-surface/readback and PNG encode/write; process wall separately",
                         "qualification": "independent exact stage/RGB GPU qualification and CPU effect controls outside render-only samples",
                         "repetition": "sequential processes; CPU/Metal order alternates by cold repetition; warm statistics use per-process medians"},
              "x86_arm": {"status": "NOT_CHECKED", "reason": "No hash-bound x86 image references for these new benchmark inputs in this native run; ARM/Metal exactness does not establish x86 parity"},
              "disk_observations": [], "results": [], "commands": []}
    env = {k: v for k, v in os.environ.items() if not k.upper().startswith("MB3D_")}

    def save():
        write_json(output/"summary.json", report)

    def disk():
        usage = shutil.disk_usage(output)
        report["disk_observations"].append({"at": utc_now(), "free_bytes": usage.free})
        if usage.free < args.min_free_gib * 2**30:
            raise RuntimeError("Free-space floor reached; no more work launched")

    def launch(name, command, cwd):
        disk()
        record = {"name": name, "argv": list(map(str, command)), "cwd": str(cwd), "started_utc": utc_now(), "log": "logs/"+name+".log"}
        report["commands"].append(record); save()
        print(name, flush=True)
        record["process"] = execute(record["argv"], cwd, output/record["log"], args.timeout, env)
        record["finished_utc"] = utc_now(); record["log_sha256"] = sha(output/record["log"]); save()
        if record["process"]["timed_out"] or record["process"]["returncode"] != 0:
            raise RuntimeError("Command failed/timed out: " + name)
        return record["process"]

    try:
        disk()
        for key, file in paths.items():
            dest = output/"evidence/bin"/("exact.metallib" if key == "library" else key)
            shutil.copy2(file, dest)
            if sha(dest) != identities[key]["sha256"]:
                raise ValueError("Binary changed during freeze")
            identities[key]["frozen_path"] = str(dest.relative_to(output))
        (output/"evidence/source/working-tree.patch").write_bytes(subprocess.check_output(["git", "diff", "HEAD", "--binary"], cwd=ROOT))
        for name, digest in source["changed_source_sha256"].items():
            if digest:
                dest = output/"evidence/source"/name; dest.parent.mkdir(parents=True, exist_ok=True)
                shutil.copy2(ROOT/name, dest)
        if args.bootstrap:
            shutil.copy2(args.bootstrap, output/"evidence/bootstrap.json")
            if sha(output/"evidence/bootstrap.json") != receipt_binding["receipt_sha256"]:
                raise ValueError("Build receipt changed after validation")
            report["bootstrap_sha256"] = receipt_binding["receipt_sha256"]
        write_json(output/"inputs/selection.json", selection)
        if selection.get("generator"):
            shutil.copy2(ROOT/selection["generator"]["path"], output/"inputs/generate_effects_v1.py")
        for row in selection["scenes"]:
            for index, item in enumerate([row, *controls(row)]):
                item["frozen_scene"] = "inputs/scenes/"+row["id"]+f"-{index}.m3p"
                shutil.copy2(ROOT/item["scene_path"], output/item["frozen_scene"])
                if sha(output/item["frozen_scene"]) != item["scene_sha256"]:
                    raise ValueError("Input scene changed")
        for asset in selection.get("assets", []):
            dest = output/"runtime"/asset["runtime_path"]; dest.parent.mkdir(parents=True, exist_ok=True)
            shutil.copy2(ROOT/asset["source_path"], dest)
            if sha(dest) != asset["sha256"]:
                raise ValueError("Input asset changed")
        write_json(output/"inputs/selection.json", selection)
        binary = {k: output/v["frozen_path"] for k, v in identities.items()}
        for scene in selection["scenes"]:
            row = {"id": scene["id"], "scene_sha256": scene["scene_sha256"], "status": "RUNNING", "backends": {}, "effect_controls": []}
            report["results"].append(row); save()
            # Qualification is intentionally completed before timing, so cold
            # means a new process and not an empty system/driver cache.
            if "metal" in scene["backends"]:
                qualified = output/"qualification"/scene["id"]
                launch("qualify-"+scene["id"], [sys.executable, "-B", ROOT/"comparison/tools/run_metal_exact_scene.py",
                       "--probe", binary["metal"], "--library", binary["library"], "--scene", output/scene["frozen_scene"],
                       "--output", qualified, "--width", args.width, "--height", args.height, "--threads", args.threads,
                       "--repeats", 3, "--full-shading", "--timeout", args.timeout], ROOT)
                qualified_summary = json.loads((qualified/"summary.json").read_text())
                binding = validate_metal_qualification(qualified_summary, identities, scene, settings)
                row["qualification"] = {"summary": str((qualified/"summary.json").relative_to(output)), "sha256": sha(qualified/"summary.json"), "status": "PASS_SCOPED", "binding": binding}
            else:
                row["backends"]["metal"] = {"status": "UNSUPPORTED", "reason": scene["unsupported_backends"]["metal"]}
            for backend in scene["backends"]:
                row["backends"][backend] = {"status": "RUNNING", "processes": []}
            for repetition in range(args.cold_repeats):
                order = scene["backends"] if repetition % 2 == 0 else list(reversed(scene["backends"]))
                for backend in order:
                    dest = output/"runs"/(scene["id"]+f"-{backend}-{repetition:02d}")
                    command = [binary[backend], "--scene", output/scene["frozen_scene"], "--output", dest]
                    for key, value in settings.items():
                        command += ["--"+key, value]
                    if backend == "metal":
                        command += ["--benchmark-render", "--full-shading", "--library", binary["library"]]
                    else:
                        command += ["--resource-root", output/"runtime", "--precision", "compatible"]
                    process = launch(scene["id"]+f"-{backend}-{repetition:02d}", command, output/"runtime")
                    native = json.loads((dest/"report.json").read_text())
                    digest = validate_report(native, dest, backend, settings, scene["required_stages"])
                    if backend == "metal" and (native.get("scene_description") != binding["scene_description"]
                                               or native.get("numeric_profile") != binding["numeric_profile"]):
                        raise ValueError("Metal benchmark prepared scene/profile differs from independent qualification")
                    data = row["backends"][backend]
                    if data.get("rgb_sha256", digest) != digest:
                        raise ValueError("RGB differs between fresh processes")
                    data["rgb_sha256"] = digest
                    data["processes"].append({"directory": str(dest.relative_to(output)), "process": process,
                                              "report_sha256": sha(dest/"report.json"), "native": native})
                    save()
            for backend in scene["backends"]:
                data = row["backends"][backend]
                if len(data["processes"]) != args.cold_repeats:
                    raise ValueError("Incomplete fresh-process count")
                data["summary"] = summarize_backend(data["processes"]); data["status"] = "COMPLETE"
            cpu_first = row["backends"]["cpu"]["processes"][0]
            cpu_image = output/cpu_first["directory"]/cpu_first["native"]["cold"]["image"]
            if "metal" in scene["backends"]:
                gpu_first = row["backends"]["metal"]["processes"][0]
                gpu_image = output/gpu_first["directory"]/gpu_first["native"]["cold"]["image"]
                row["cpu_metal"] = compare_images(cpu_image, gpu_image)
                if row["cpu_metal"]["different_channels"]:
                    raise ValueError("CPU/Metal final RGB is not exact")
                for image in qualified_summary["images"]:
                    if compare_images(cpu_image, qualified/image["png"])["different_channels"]:
                        raise ValueError("Benchmark RGB differs from independently qualified full render")
            else:
                row["cpu_metal"] = {"status": "UNSUPPORTED", "reason": scene["unsupported_backends"]["metal"]}
            for index, control in enumerate(controls(scene)):
                stem = scene["id"]+f"-effect-off-{index:02d}"
                png = output/"controls"/(stem+".png"); profile = png.with_suffix(".json")
                launch(stem, [binary["renderer"], output/control["frozen_scene"], "--output", png,
                       "--width", args.width, "--height", args.height, "--threads", args.threads,
                       "--deterministic", "--precision", "compatible", "--profile-json", profile], output/"runtime")
                data = json.loads(profile.read_text())
                if data.get("status") != "complete" or data.get("render_status") != "completed" or data.get("precision") != "compatible":
                    raise ValueError("Effect-off control incomplete")
                require_stages(data["stages"], control.get("required_stages", []), control.get("required_not_run_stages", []))
                difference = compare_images(cpu_image, png)
                if not difference["different_channels"]:
                    raise ValueError("Enabled effect did not change RGB: " + stem)
                row["effect_controls"].append({"scene_sha256": control["scene_sha256"], "disabled_effects": control.get("disabled_effects"),
                                               "image": str(png.relative_to(output)), "difference": difference,
                                               "image_identity": image_details(png, args.width, args.height)})
            row["status"] = "COMPLETE"; save()
        report["source_identity_at_end"] = source_identity()
        if report["source_identity_at_end"] != source:
            raise ValueError("Source changed during benchmark; results not promoted")
        if sha(args.selection) != report["selection_sha256"]:
            raise ValueError("Selection changed during benchmark")
        # Recheck source fixtures/assets, which live outside source_identity's
        # code inventory, and every frozen copy used during measurement.
        for scene in selection["scenes"]:
            for item in [scene, *controls(scene)]:
                if any(sha(p) != item["scene_sha256"] for p in (ROOT/item["scene_path"], output/item["frozen_scene"])):
                    raise ValueError("Scene changed during benchmark")
        for asset in selection.get("assets", []):
            if any(sha(p) != asset["sha256"] for p in (ROOT/asset["source_path"], output/"runtime"/asset["runtime_path"])):
                raise ValueError("Asset changed during benchmark")
        for key, identity in identities.items():
            if sha(output/identity["frozen_path"]) != identity["sha256"]:
                raise ValueError("Frozen binary changed during benchmark")
        report["status"] = "COMPLETE"
    except KeyboardInterrupt:
        report["status"] = "INTERRUPTED"
    except Exception as error:
        report.update(status="FAILED", error=str(error))
        print(str(error), file=sys.stderr)
    report["finished_utc"] = utc_now()
    report["counts"] = {"complete": sum(r["status"] == "COMPLETE" for r in report["results"]),
                        "selected": len(selection["scenes"]), "effect_controls": sum(len(r["effect_controls"]) for r in report["results"])}
    save()
    if report["status"] == "COMPLETE" and args.gallery:
        export_gallery(report, output, args.gallery.expanduser().resolve())
    print(json.dumps({"status": report["status"], "counts": report["counts"], "summary": str(output/"summary.json")}))
    return 0 if report["status"] == "COMPLETE" else 1


if __name__ == "__main__":
    raise SystemExit(main())
