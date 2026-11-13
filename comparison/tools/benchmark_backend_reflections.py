"""Native CPU/Metal opaque reflection comparison through RenderWithBackend.

Both backends execute a complete production API call for every frame. Same-
process warm calls recreate backend resources; this is not the standalone GPU
benchmark's reused-pipeline measurement. Cross-backend RGB differences are allowed.
"""
from __future__ import annotations

import argparse
import hashlib
import html
import json
import math
import os
from pathlib import Path
import re
import shutil
import statistics
import subprocess
import sys

from PIL import Image
import benchmark_precision as common
from benchmark_gallery_arm64 import ROOT, binary_identity, execute, native_host, relative_file, sha, utc_now
from benchmark_metal_precision import quiet_host
from benchmark_render_arm64 import TIMINGS, duration, image_details, require_stages, stats
from native_evidence import SOURCE_PATHS, source_identity

SCHEMA = "mb3d.backend-reflections-benchmark.v1"
DRIVER_SCHEMA = "mb3d.backend-render-benchmark.v1"
BACKENDS = ("cpu", "metal")
ORDINARY = ("sphereifs-ao-shadows", "boxifs-ao-shadows", "integerpower8-ao-shadows")
STAGES = common.STAGES
REQUIRED = ("preparation", "primary", "shadows", "ao", "color")
COUNTS = ("active_pixels", "rays", "hits", "misses", "changed_pixels", "faults", "max_bounces",
          "ray_steps", "primary_skipped", "ao_rays", "ao_steps", "shadow_rays", "shadow_steps")
write_json = common.write_json


def check(condition, message):
    if not condition:
        raise ValueError(message)


def safe_scene_id(value):
    check(isinstance(value, str) and re.fullmatch(r"[A-Za-z0-9_-]+", value) is not None,
          "Unsafe scene ID")
    return value


def selected_inputs(manifest):
    """Validate every identifier before any output directory or copy exists."""
    check(isinstance(manifest, dict) and manifest.get("schema") == "mb3d.render-benchmark-selection.v1",
          "Invalid scene selection schema")
    rows = manifest.get("scenes")
    check(isinstance(rows, list) and rows and type(manifest.get("expected_count")) is int
          and manifest["expected_count"] == len(rows), "Invalid scene selection count/type")
    by_id = {}
    for row in rows:
        check(isinstance(row, dict), "Scene selection row must be an object")
        name = safe_scene_id(row.get("id"))
        check(name not in by_id, "Duplicate scene ID")
        by_id[name] = row
    names = ("integerpower8-reflections", *ORDINARY)
    check(all(name in by_id for name in names), "Required benchmark scene is missing")
    scene = by_id[names[0]]
    off = scene.get("effect_control")
    check(isinstance(off, dict), "Reflection effect control must be an object")
    off_id = safe_scene_id(off.get("id"))
    check(off_id not in by_id, "Duplicate effect-control scene ID")
    check(off.get("disabled_effects") == ["reflections"] and off.get("expect_rgb_difference") is True
          and scene.get("assets") == [], "Reflection control contract differs")
    inputs = (scene, off, *(by_id[name] for name in ORDINARY))
    for row in inputs:
        path = relative_file(ROOT, row.get("scene_path"))
        check(path.suffix.lower() == ".m3p" and sha(path) == row.get("scene_sha256"), "Input scene hash/type mismatch")
    return scene, off, by_id, inputs


def input_destination(output, scene_id):
    name = safe_scene_id(scene_id)
    root = output.resolve()
    target = (root / "inputs" / (name + ".m3p")).resolve()
    check(target.is_relative_to(root) and target.parent == root / "inputs", "Input destination escapes output")
    return target


def backend_order(index):
    return BACKENDS if index % 2 == 0 else tuple(reversed(BACKENDS))


def admit_reference(references, backend, rgb):
    check(backend in BACKENDS, "Unknown backend")
    check(backend not in references or references[backend] == rgb, "Within-backend process RGB changed")
    references.setdefault(backend, rgb)


def validate_reflections(activity, enabled, pixel_count):
    check(isinstance(activity, dict) and activity.get("enabled") is enabled, "Reflection activity missing or wrong")
    for key in COUNTS:
        check(type(activity.get(key)) is int and activity[key] >= 0, "Invalid reflection count: " + key)
    duration(activity.get("max_command_seconds"), "reflection max command")
    check(activity["faults"] == 0, "Reflection GPU faults")
    if enabled:
        check(0 < activity["changed_pixels"] <= activity["active_pixels"] <= pixel_count,
              "Reflection did not change valid active pixels")
        eligible = activity["active_pixels"] - activity["primary_skipped"]
        check(0 <= activity["primary_skipped"] <= activity["active_pixels"]
              and 0 < activity["rays"] <= 2 * eligible and activity["changed_pixels"] <= eligible,
              "Reflection rays/skipped-primary counts are inconsistent")
        check(activity["hits"] + activity["misses"] == activity["rays"], "Reflection ray outcomes incomplete")
        check(activity["max_bounces"] == 2 and activity["hits"] > 0 and activity["misses"] > 0
              and activity["max_command_seconds"] > 0,
              "Reflection bounce count/timing invalid")
        check(activity["primary_skipped"] <= pixel_count and all(activity[k] > 0 for k in
              ("ray_steps", "ao_rays", "ao_steps", "shadow_rays", "shadow_steps")),
              "Secondary traversal/AO/shadow work is missing")
    else:
        check(not any(activity[key] for key in COUNTS) and activity["max_command_seconds"] == 0,
              "Inactive reflection pass reported GPU work")
    return {key: activity[key] for key in ("enabled", *COUNTS)}


def validate_stages(stages, reflected, cold=True):
    check(isinstance(stages, dict) and set(stages) == STAGES, "Stage inventory differs")
    for name, stage in stages.items():
        check(stage.get("state") in ("completed", "not_run"), "Incomplete stage: " + name)
        elapsed = duration(stage.get("seconds"), name)
        check(stage["state"] != "not_run" or elapsed == 0, "Inactive stage has time: " + name)
    required = (*REQUIRED, "final_surface", "save", *(("load",) if cold else ()),
                *(("reflections",) if reflected else ()))
    absent = ("volume", "normals", "dof", *(() if reflected else ("reflections",)),
              *(() if cold else ("load",)))
    require_stages(stages, required, absent)


def validate_backend(binding, backend, library, reflected, size):
    check(binding.get("requested") == backend and binding.get("effective") == backend
          and binding.get("fallback") is False and binding.get("fallback_code") == ""
          and binding.get("fallback_reason") == "", "Wrong backend or hidden fallback")
    duration(binding.get("selection_seconds"), "backend selection")
    if backend == "metal":
        check(binding.get("library") == str(library) and bool(binding.get("device")), "Metal library/device mismatch")
    else:
        check(binding.get("library") == "" and binding.get("device") == "", "CPU claimed GPU identity")
    return validate_reflections(binding.get("reflections"), reflected and backend == "metal", size[0] * size[1])


def validate_report(report, directory, backend, library, settings, scene, runtime, reflected=True):
    check(backend in BACKENDS, "Unknown backend")
    check(report.get("schema") == DRIVER_SCHEMA and report.get("status") == "COMPLETE"
          and report.get("backend") == backend and report.get("architecture") == "arm64"
          and report.get("error") == "", "Incomplete/wrong benchmark identity")
    check(report.get("settings") == settings, "Settings mismatch")
    check(report.get("scene") == str(scene) and report.get("resource_root") == str(runtime), "Input identity mismatch")
    check(report.get("execution") == {"deterministic": True, "profile_timing": True,
          "work_control": "none", "precision": "native"}, "Wrong execution/precision policy")
    features = report.get("loaded_features", {})
    check(features.get("load_success") is True and features.get("effective_bCalc3D") == 1
          and type(features.get("MCoptions")) is int and not features["MCoptions"] & 1,
          "Ordinary loaded feature metadata missing")
    check(features.get("bCalcSRautomatic") == (1 if reflected else 0), "Wrong reflection scene flags")
    check(features.get("SRreflectioncount") == 2 and math.isclose(features.get("SRamount", -1), 0.8, abs_tol=1e-7),
          "Reflection fixture parameters differ")
    for key in ("bCalculateHardShadow", "bCalcAmbShadowAutomatic", "bNormalsOnDE"):
        check(type(features.get(key)) is int and features[key] & 1, "Required effect disabled: " + key)
    formulas = features.get("formulas")
    check(isinstance(formulas, list) and len(formulas) == 6, "Formula metadata missing")
    for index, f in enumerate(formulas):
        check(f.get("slot") == index and type(f.get("id")) is int and type(f.get("iterations")) is int
              and isinstance(f.get("name"), str) and "jit" not in f["name"].lower(), "Invalid/deferred formula metadata")
    setup = report.get("setup_ms", {})
    check(set(setup) == {"load", "ordinary_input_check"}, "Setup inventory incomplete")
    check(duration(setup["load"], "load") > 0, "Cold load absent")
    duration(setup["ordinary_input_check"], "input check")
    cold, warmups, samples = report.get("cold"), report.get("warmups"), report.get("samples")
    check(isinstance(cold, dict) and isinstance(warmups, list) and isinstance(samples, list)
          and len(warmups) == settings["warmups"] and len(samples) == settings["repeats"], "Missing frame repetitions")
    inventory = [(cold, "cold", 0), *[(s, "warmup", i) for i, s in enumerate(warmups)],
                 *[(s, "warm", i) for i, s in enumerate(samples)]]
    reference, activity_reference, images = None, None, []
    size = settings["width"], settings["height"]
    for frame, phase, index in inventory:
        name = "cold.png" if phase == "cold" else f"{phase}-{index:03d}.png"
        check(frame.get("phase") == phase and frame.get("index") == index and frame.get("image") == name
              and frame.get("state") == "completed" and frame.get("error") == ""
              and frame.get("exact_rgb_matches_cold") is True and frame.get("precision") == "native"
              and (frame.get("width"), frame.get("height")) == size
              and frame.get("threads") == (0 if backend == "metal" else settings["threads"]), "Invalid frame metadata")
        activity = validate_backend(frame.get("backend", {}), backend, library, reflected, size)
        check(activity_reference is None or activity == activity_reference, "Reflection activity changed between repetitions")
        activity_reference = activity
        validate_stages(frame.get("stages"), reflected, phase == "cold")
        for key in (*TIMINGS, "load_ms", "preparation_ms", "render_profile_total_ms"):
            duration(frame.get(key), key)
        render, copy, combined, png = (frame[k] for k in TIMINGS[:4])
        check(render > 0 and png > 0 and combined + .01 >= render + copy
              and frame["end_to_end_ms"] + .01 >= combined + png, "Incomplete render/PNG timing boundary")
        check(math.isclose(frame["preparation_ms"], frame["stages"]["preparation"]["seconds"] * 1000,
                           rel_tol=1e-9, abs_tol=1e-6), "Preparation timing mismatch")
        check(frame["load_ms"] == (setup["load"] if phase == "cold" else 0), "Repeated scene loading or missing cold load")
        if phase == "cold":
            total = duration(frame.get("cold_total_ms"), "cold total")
            end = duration(frame.get("cold_end_to_end_ms"), "cold end to end")
            check(total + .01 >= setup["load"] + setup["ordinary_input_check"] + combined
                  and end + .01 >= total + png and frame["end_to_end_ms"] == end, "Cold timing boundary excludes work")
        path = relative_file(directory, name)
        identity = image_details(path, *size)
        check(identity["nonconstant"], "Constant image cannot prove reflection activity")
        rgb = common.pixels(path, size)
        check(reference is None or reference == rgb, "Within-process RGB changed")
        reference = rgb
        images.append({"path": name, **identity})
    return reference, images


def validate_cli_profile(profile, backend, library, reflected, settings, requested=None):
    check(profile.get("status") == "complete" and profile.get("render_status") == "completed"
          and profile.get("exit_code") == 0 and profile.get("precision") == "native"
          and profile.get("error") == "", "CLI reference failed")
    check(profile.get("schema") == ("mb3d.render-stage-profile.v1" if backend == "metal" else "mb3d.cpu-stage-profile.v1")
          and profile.get("numeric_profile") == ("native_metal_fp32" if backend == "metal" else "native_cpu_fp64"),
          "CLI arithmetic identity mismatch")
    binding = {"requested": profile.get("backend_requested"), "effective": profile.get("backend"),
               "fallback": profile.get("backend_fallback"), "fallback_code": profile.get("backend_fallback_code"),
               "fallback_reason": profile.get("backend_fallback_reason"), "device": profile.get("metal_device"),
               "library": profile.get("metal_library"), "selection_seconds": profile.get("backend_selection_seconds"),
               "reflections": profile.get("metal_reflections")}
    if requested == "auto":
        check(binding["requested"] == "auto", "Auto request was not exercised")
        binding["requested"] = "metal"
    validate_backend(binding, backend, library, reflected, (settings["width"], settings["height"]))
    check((profile.get("width"), profile.get("height")) == (settings["width"], settings["height"])
          and profile.get("workers") == (0 if backend == "metal" else settings["threads"])
          and profile.get("threads_requested") == settings["threads"], "CLI dimensions/workers mismatch")
    validate_stages(profile.get("stages"), reflected)


def summarize_processes(processes):
    check(bool(processes), "Missing process inventory")
    reports = [p["report_data"] for p in processes]
    result = {"cold": {key: stats([r["cold"][key] for r in reports])
                         for key in (*TIMINGS, "cold_total_ms", "cold_end_to_end_ms")},
              "warm": {key: stats([statistics.median(s[key] for s in r["samples"]) for r in reports]) for key in TIMINGS},
              "stages": {}, "method": "Median of per-process warm medians; complete new RenderWithBackend call/resources per frame"}
    for stage in sorted(STAGES):
        result["stages"][stage] = {
            "cold": stats([r["cold"]["stages"][stage]["seconds"] * 1000 for r in reports]),
            "warm": stats([statistics.median(s["stages"][stage]["seconds"] * 1000 for s in r["samples"]) for r in reports])}
    for key in ("wall_seconds", "cpu_seconds", "peak_rss_bytes"):
        result["process_" + key] = stats([p["process"][key] for p in processes])
    return result


def source_snapshot():
    source = source_identity()
    names = subprocess.check_output(["git", "ls-files", "--cached", "--others", "--exclude-standard", "-z", "--", *SOURCE_PATHS], cwd=ROOT)
    files = {}
    for raw in sorted(set(names.split(b"\0")) - {b""}):
        name = raw.decode()
        path = ROOT / name
        if path.is_file() and (path.name == "CMakeLists.txt" or path.suffix.lower() in
                (".h", ".hpp", ".cpp", ".c", ".mm", ".metal", ".inc", ".asm", ".py", ".json", ".cmake", ".txt")):
            files[name] = sha(path)
    return {**source, "files_sha256": files}


def frozen_reference(gallery, scene_id, backend, size, reflection=False):
    summary_path, manifest_path = gallery / "summary.json", gallery / "manifest.json"
    manifest = json.loads(manifest_path.read_text())
    records = {r["file"]: r for r in manifest["files"]}
    def verified(name):
        path = relative_file(gallery, name)
        row = records.get(name, {})
        check(row.get("sha256") == sha(path) and row.get("bytes") == path.stat().st_size, "Frozen reference integrity failed: " + name)
        return path
    summary = json.loads(verified("summary.json").read_text())
    check(summary.get("status") == "COMPLETE", "Prior reference report incomplete")
    scene = next((s for s in summary["scenes"] if s["id"] == scene_id), None)
    check(scene is not None, "Prior scene missing")
    name = f"images/{scene_id}-{backend}{'-0' if reflection else ''}.png"
    image = verified(name)
    identity = image_details(image, *size)
    check((summary.get("settings", {}).get("width"), summary.get("settings", {}).get("height")) == size,
          "Prior reference dimensions differ")
    if reflection:
        check(backend == "cpu" and summary.get("schema") == "mb3d.main-renderer-native-integration.v1",
              "Prior reflection reference must be the Native CPU main-renderer record")
        run = scene["routes"][backend]["runs"][0]
        profile = run["profile_data"]
        check(run["image"] == name and run["effective"] == "cpu" and profile["precision"] == "native"
              and profile["backend"] == "cpu" and run["image_identity"]["rgb_sha256"] == identity["rgb_sha256"],
              "Prior reflection reference role/RGB differs")
    else:
        check(summary.get("precision") == "native" and scene["backends"][backend]["exact_rgb_within_backend"] is True
              and scene["backends"][backend]["rgb_sha256"] == identity["rgb_sha256"],
              "Prior Native backend reference role/RGB differs")
    return {"gallery": str(gallery), "summary_sha256": sha(summary_path), "manifest_sha256": sha(manifest_path),
            "scene_sha256": scene["scene_sha256"], "image": str(image), **identity}


def verify_manifest(directory):
    value = json.loads((directory / "manifest.json").read_text())
    names = []
    for row in value["files"]:
        path = relative_file(directory, row["file"])
        check(path.stat().st_size == row["bytes"] and sha(path) == row["sha256"], "Retained evidence integrity failed")
        names.append(row["file"])
    check(len(names) == len(set(names)), "Duplicate manifest file")
    check(set(names) == {str(p.relative_to(directory)) for p in directory.rglob("*") if p.is_file() and p.relative_to(directory) != Path("manifest.json")},
          "Unlisted retained evidence")
    return len(names)


def verify_recorded_evidence(report, directory):
    """Bind retained bytes to identities admitted before copying, not new hashes."""
    def file(name, digest):
        check(sha(relative_file(directory, name)) == digest, "Admitted evidence changed: " + name)
    def image(name, identity):
        path = relative_file(directory, name)
        check(sha(path) == identity["image_sha256"], "Admitted PNG changed: " + name)
        actual = image_details(path, identity["width"], identity["height"])
        check(actual["rgb_sha256"] == identity["rgb_sha256"], "Admitted decoded RGB changed: " + name)
    for entry in [*report["identities"].values(), *report["build_evidence"]]:
        file(entry["frozen_path"], entry["sha256"])
    for name, digest in report["source_identity"]["files_sha256"].items():
        file("evidence/source/" + name, digest)
    for row in report["scene_inputs"]:
        file("inputs/" + row["id"] + ".m3p", row["scene_sha256"])
    for entry in report["controls"]:
        file(entry["report"], entry["report_sha256"])
        file(entry["log"], entry["log_sha256"])
        image(entry["image"], entry["image_identity"])
    for data in report["backends"].values():
        for process in data["processes"]:
            file(process["report"], process["report_sha256"])
            file(process["log"], process["log_sha256"])
            for row in process["images"]:
                image(process["directory"] + "/" + row["path"], row)
    for entry in report["prior_references"]:
        image(entry["frozen_image"], entry)
    for entry in report["prior_report_files"]:
        file(entry["frozen_path"], entry["sha256"])


def export_gallery(report, output, gallery):
    check(not gallery.exists() and gallery.parent.is_dir(), "Gallery must be new")
    common.check_disk(gallery.parent)
    staging = gallery.with_name(gallery.name + ".staging")
    check(not staging.exists(), "Gallery staging exists")
    verify_recorded_evidence(report, output)
    shutil.copytree(output, staging)
    verify_recorded_evidence(report, staging)
    for name, digest in report["source_identity"]["files_sha256"].items():
        check(sha(staging / "evidence/source" / name) == digest, "Frozen source differs from measured snapshot")
    manifest = {"schema": SCHEMA, "files": [{"file": str(p.relative_to(staging)), "bytes": p.stat().st_size, "sha256": sha(p)}
                for p in sorted(staging.rglob("*")) if p.is_file()]}
    write_json(staging / "manifest.json", manifest)
    verify_manifest(staging)
    common.check_disk(staging)
    staging.rename(gallery)


def html_report(report):
    parts = ['<!doctype html><html lang="ru"><meta charset="utf-8"><title>Отражения: Native CPU / Metal</title>',
             '<style>body{font:16px/1.5 system-ui;max-width:1100px;margin:32px auto;padding:0 16px;background:#f4f3ef;color:#202729}table{border-collapse:collapse}td,th{padding:9px;border:1px solid #aaa}.pair{display:flex;flex-wrap:wrap;gap:16px}img{max-width:100%;width:512px}figure{margin:8px 0}pre{white-space:pre-wrap}</style>',
             '<h1>Отражения: Native CPU / Native Metal</h1><p>Одинаковый вход 512×384 и полный RenderWithBackend. CPU — FP64; Metal — FP32. Различия между ними допустимы; каждый backend обязан точно повторить свои RGB.</p>',
             '<p>Cold — первый полный рендер нового процесса, включая загрузку сцены. Warm — новый полный вызов в том же процессе: ресурсы, GPU pipelines/buffers и CPU workers создаются заново. Кэши ОС/драйвера не очищались. Запуск процесса, проверки и PNG-кодирование показаны отдельно.</p>',
             f'<p>{report["cold_repeats"]} процессов на backend; в каждом cold, {report["settings"]["warmups"]} исключённых warmup и {report["settings"]["repeats"]} измеряемых warm-кадра. Warm — медиана медиан процессов. Это сравнение режимов, не ускорение идентичной арифметики.</p>',
             '<table><tr><th>Backend</th><th>Cold → RGB, мс</th><th>Warm → RGB, мс</th><th>Warm с PNG, мс</th></tr>']
    for backend in BACKENDS:
        data = report["backends"][backend]["summary"]
        parts.append(f'<tr><td>{backend}</td><td>{data["cold"]["cold_total_ms"]["median"]:.3f}</td><td>{data["warm"]["render_and_readback_ms"]["median"]:.3f}</td><td>{data["warm"]["end_to_end_ms"]["median"]:.3f}</td></tr>')
    parts.append(f'</table><p>CPU/Metal: cold {report["cold_ratio_cpu_over_metal"]:.3f}×; warm {report["warm_ratio_cpu_over_metal"]:.3f}×.</p><div class="pair">')
    for backend in BACKENDS:
        parts.append(f'<figure><figcaption>{backend}: отражения включены</figcaption><img src="controls/reflection-{backend}-on.png"></figure>')
    parts.append('</div><p>RGB-различия CPU/Metal: ' + html.escape(json.dumps(report["cross_backend_rgb"])) + '</p>')
    parts.append('<h2>Проверка действия отражений</h2><div class="pair">')
    for backend in BACKENDS:
        parts.append(f'<figure><figcaption>{backend}: отражения выключены</figcaption><img src="controls/reflection-{backend}-off.png"></figure>')
    parts.append('</div><pre>' + html.escape(json.dumps(report["effect_differences"], indent=2)) + '</pre>')
    parts.append('<p>Все повторы и контроль auto проверены; прежние три сцены сверены с сохранёнными RGB каждого backend. GPU activity/faults берутся из production API. Все PNG, команды, профили, исходники и хеши сохранены.</p><p><a href="summary.json">Полный отчёт</a> · <a href="manifest.json">Манифест файлов</a></p><p>Monte Carlo/JIT исключены. Pascal/native-x86 parity не проверяется.</p></html>')
    return '\n'.join(parts)


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    for key in ("driver", "renderer", "native-library", "prior-gallery", "prior-reflection-gallery", "output", "gallery"):
        parser.add_argument("--" + key, type=Path, required=True)
    parser.add_argument("--build-evidence", type=Path, action="append", default=[])
    parser.add_argument("--selection", type=Path, default=ROOT / "comparison/benchmark_scenes/effects-v1.json")
    for key, default in (("width", 512), ("height", 384), ("threads", 6), ("cold-repeats", 3), ("warmups", 1), ("repeats", 3)):
        parser.add_argument("--" + key, type=int, default=default)
    parser.add_argument("--timeout", type=float, default=900)
    args = parser.parse_args(argv)
    report, output = None, args.output.expanduser().resolve()
    try:
        gallery = args.gallery.expanduser().resolve()
        check((args.width, args.height, args.threads) == (512, 384, 6), "Frozen full-render comparison requires 512x384 and six requested threads")
        check(3 <= args.cold_repeats <= 9 and 1 <= args.warmups <= 3 and 3 <= args.repeats <= 9
              and math.isfinite(args.timeout) and args.timeout > 0, "Invalid bounded repetitions/timeout")
        check(not output.exists() and output.parent.is_dir() and not gallery.exists() and gallery.parent.is_dir()
              and output != gallery and not gallery.is_relative_to(output), "Output/gallery must be separate new directories")
        common.check_disk(output.parent); quiet_host()
        settings = {k: getattr(args, k) for k in ("width", "height", "threads", "warmups", "repeats")}
        scene, off, by_id, input_rows = selected_inputs(json.loads(args.selection.read_text()))
        # Resolve every destination before output.mkdir(), then recheck at copy.
        for row in input_rows:
            input_destination(output, row["id"])
        identities = {role: binary_identity(getattr(args, role).expanduser().resolve(strict=True)) for role in ("driver", "renderer")}
        library = args.native_library.expanduser().resolve(strict=True)
        identities["library"] = {"path": str(library), "sha256": sha(library), "bytes": library.stat().st_size}
        check(library.suffix == ".metallib", "Expected offline Metal library")
        source = source_snapshot()
        output.mkdir()
        for folder in ("evidence/bin", "evidence/build", "evidence/source", "evidence/prior", "inputs", "runtime", "controls", "runs", "logs"):
            (output / folder).mkdir(parents=True)
        report = {"schema": SCHEMA, "status": "RUNNING", "started_utc": utc_now(), "settings": settings,
                  "cold_repeats": args.cold_repeats, "host": native_host(), "source_identity": source,
                  "source_binding": "Frozen observed source; supplied build/test logs are hash-bound evidence, not an inferred compile receipt",
                  "identities": identities, "build_evidence": [], "backends": {b: {"processes": []} for b in BACKENDS},
                  "controls": [], "prior_references": [], "prior_report_files": [], "commands": [], "effect_differences": {},
                  "disk_free_bytes": {"floor": common.DISK_FLOOR, "minimum_observed": common.check_disk(output), "observations": []},
                  "scope": "Native CPU FP64 versus Native Metal FP32 main application API; no cross-backend parity claim",
                  "method": {"cold": "new process, load through first RGB; OS/driver caches not purged",
                             "warm": "same process, fresh complete RenderWithBackend resources every frame; median of process medians",
                             "png": "encode/write separately; process wall includes all validation/report work"}, "gallery": str(gallery)}
        for role, identity in identities.items():
            identity["frozen_path"] = "evidence/bin/" + {"driver": "mb3d_backend_render_benchmark", "renderer": "mb3d_render", "library": "metal_native_scene.metallib"}[role]
            shutil.copy2(identity["path"], output / identity["frozen_path"])
        for index, path in enumerate(args.build_evidence):
            path = path.resolve(strict=True)
            item = {"path": str(path), "sha256": sha(path), "frozen_path": f"evidence/build/{index:02d}-{path.name}"}
            shutil.copy2(path, output / item["frozen_path"])
            report["build_evidence"].append(item)
        for name, digest in source["files_sha256"].items():
            target = output / "evidence/source" / name
            target.parent.mkdir(parents=True, exist_ok=True); shutil.copy2(ROOT / name, target)
            check(sha(target) == digest, "Source changed during snapshot")
        (output / "evidence/source/working-tree.patch").write_bytes(subprocess.check_output(["git", "diff", "HEAD", "--binary"], cwd=ROOT))
        write_json(output / "evidence/source-identity.json", source)
        frozen_inputs = []
        for row in input_rows:
            path = relative_file(ROOT, row["scene_path"])
            check(sha(path) == row["scene_sha256"], "Input scene hash mismatch")
            target = input_destination(output, row["id"])
            shutil.copy2(path, target)
            row["frozen_path"] = str(target)
            frozen_inputs.append(row)
        write_json(output / "inputs/selection.json", {"selection_sha256": sha(args.selection), "scenes": frozen_inputs})
        report["scene"] = scene
        report["scene_inputs"] = frozen_inputs
        for role, prior in (("ordinary", args.prior_gallery.resolve()), ("reflection", args.prior_reflection_gallery.resolve())):
            for name in ("summary.json", "manifest.json"):
                origin = prior / name
                item = {"path": str(origin), "sha256": sha(origin), "frozen_path": "evidence/prior/" + role + "-" + name}
                shutil.copy2(origin, output / item["frozen_path"])
                check(sha(output / item["frozen_path"]) == item["sha256"], "Prior report changed while copying")
                report["prior_report_files"].append(item)
        env = {k: v for k, v in os.environ.items() if not k.upper().startswith("MB3D_")}
        report["removed_environment_variable_names"] = sorted(set(os.environ) - set(env))
        frozen_library, runtime = output / identities["library"]["frozen_path"], output / "runtime"
        def verify_inputs():
            for entry in [*identities.values(), *report["build_evidence"], *report["prior_report_files"]]:
                check(all(sha(p) == entry["sha256"] for p in (Path(entry["path"]), output / entry["frozen_path"])), "Binary/library/build evidence changed")
            for row in frozen_inputs:
                check(sha(Path(row["frozen_path"])) == row["scene_sha256"] == sha(ROOT / row["scene_path"]), "Scene changed")
        def launch(entry, command):
            quiet_host(); common.observe_disk(report, output, "before:" + entry["label"]); verify_inputs()
            entry["argv"] = list(map(str, command)); entry["cwd"] = str(runtime)
            report["commands"].append({"argv": entry["argv"], "cwd": str(runtime), "purpose": entry["label"]})
            write_json(output / "summary.json", report)
            entry["process"] = execute(entry["argv"], runtime, output / entry["log"], args.timeout, env)
            entry["log_sha256"] = sha(output / entry["log"])
            common.observe_disk(report, output, "after:" + entry["label"])
            check(entry["process"]["returncode"] == 0 and not entry["process"]["timed_out"], "Process failed: " + entry["log"])
        def cli(label, row, backend, reflected, expected=None):
            entry = {"label": label, "backend": backend, "image": "controls/" + label + ".png",
                     "report": "controls/" + label + ".json", "log": "logs/" + label + ".log", "scene_sha256": row["scene_sha256"]}
            report["controls"].append(entry)
            command = [output / identities["renderer"]["frozen_path"], row["frozen_path"], "--output", output / entry["image"],
                       "--width", args.width, "--height", args.height, "--threads", args.threads, "--precision", "native",
                       "--backend", backend, "--deterministic", "--profile-json", output / entry["report"]]
            if backend != "cpu": command += ["--metal-library", frozen_library]
            launch(entry, command)
            raw = json.loads((output / entry["report"]).read_text())
            check(raw.get("input") == row["frozen_path"] and raw.get("output") == str(output / entry["image"]),
                  "CLI input/output identity mismatch")
            validate_cli_profile(raw, "metal" if backend == "auto" else backend, frozen_library, reflected, settings, backend)
            rgb = common.pixels(output / entry["image"], (args.width, args.height))
            if expected is not None: check(rgb == expected, "Production CLI same-backend reference changed: " + label)
            entry.update(status="PASS", profile=raw, report_sha256=sha(output / entry["report"]), image_identity=image_details(output / entry["image"], args.width, args.height))
            return rgb
        # The prior gallery is independently frozen evidence, never rewritten.
        for name in ORDINARY:
            for backend in BACKENDS:
                ref = frozen_reference(args.prior_gallery.resolve(), name, backend, (args.width, args.height))
                check(ref["scene_sha256"] == by_id[name]["scene_sha256"], "Prior input differs")
                destination = output / "evidence/prior" / (name + "-" + backend + ".png")
                shutil.copy2(ref["image"], destination)
                check(sha(destination) == ref["image_sha256"], "Prior PNG changed while copying")
                ref["frozen_image"] = str(destination.relative_to(output))
                report["prior_references"].append(ref)
                cli("ordinary-" + name + "-" + backend, by_id[name], backend, False, common.pixels(destination, (args.width, args.height)))
        ref = frozen_reference(args.prior_reflection_gallery.resolve(), scene["id"], "cpu", (args.width, args.height), True)
        check(ref["scene_sha256"] == scene["scene_sha256"], "Prior reflection CPU input differs")
        target = output / "evidence/prior/reflection-cpu.png"; shutil.copy2(ref["image"], target)
        check(sha(target) == ref["image_sha256"], "Prior reflection PNG changed while copying")
        ref["frozen_image"] = str(target.relative_to(output)); report["prior_references"].append(ref)
        references = {}
        for backend in BACKENDS:
            references[backend] = cli("reflection-" + backend + "-on", scene, backend, True,
                common.pixels(target, (args.width, args.height)) if backend == "cpu" else None)
            off_rgb = cli("reflection-" + backend + "-off", off, backend, False)
            difference = common.rgb_difference(references[backend], off_rgb)
            check(difference["different_pixels"] > 0, "Reflection negative control did not change RGB")
            report["effect_differences"][backend] = difference
        cli("reflection-auto-on", scene, "auto", True, references["metal"])
        activity = {}
        for index in range(args.cold_repeats):
            for backend in backend_order(index):
                label = f"reflection-{backend}-{index:02d}"
                directory = output / "runs" / label
                entry = {"label": label, "backend": backend, "cold_index": index, "directory": str(directory.relative_to(output)),
                         "report": str((directory / "report.json").relative_to(output)), "log": "logs/" + label + ".log"}
                report["backends"][backend]["processes"].append(entry)
                command = [output / identities["driver"]["frozen_path"], "--scene", scene["frozen_path"], "--output", directory,
                           "--resource-root", runtime, "--backend", backend, "--precision", "native"]
                if backend == "metal": command += ["--metal-library", frozen_library]
                for key, value in settings.items(): command += ["--" + key, value]
                launch(entry, command)
                raw = json.loads((directory / "report.json").read_text())
                rgb, images = validate_report(raw, directory, backend, frozen_library, settings, Path(scene["frozen_path"]), runtime)
                admit_reference(references, backend, rgb)
                binding = {key: raw[key] for key in ("loaded_features", "execution", "architecture")}
                binding["reflection_activity"] = {k: raw["cold"]["backend"]["reflections"][k] for k in ("enabled", *COUNTS)}
                cli_activity = next(c["profile"]["metal_reflections"] for c in report["controls"]
                                    if c["label"] == "reflection-" + backend + "-on")
                check(binding["reflection_activity"] == {k: cli_activity[k] for k in ("enabled", *COUNTS)},
                      "Benchmark reflection activity differs from production CLI")
                check(backend not in activity or activity[backend] == binding, "Backend activity/input metadata changed across processes")
                activity[backend] = binding
                entry.update(status="PASS", report_data=raw, images=images, report_sha256=sha(directory / "report.json"), rgb_sha256=hashlib.sha256(rgb).hexdigest())
        for backend, data in report["backends"].items():
            data.update(summary=summarize_processes(data["processes"]), exact_rgb_all_frames=True, rgb_sha256=hashlib.sha256(references[backend]).hexdigest())
        report["cross_backend_rgb"] = common.rgb_difference(references["cpu"], references["metal"])
        for label, phase, key in (("cold", "cold", "cold_total_ms"), ("warm", "warm", "render_and_readback_ms")):
            c, m = [report["backends"][b]["summary"][phase][key]["median"] for b in BACKENDS]
            check(m > 0, "Zero Metal timing"); report[label + "_ratio_cpu_over_metal"] = c / m
        verify_inputs(); quiet_host()
        verify_recorded_evidence(report, output)
        report["source_identity_at_end"] = source_snapshot()
        check(report["source_identity_at_end"] == source, "Source changed during measurements")
        for name, digest in source["files_sha256"].items():
            check(sha(output / "evidence/source" / name) == digest, "Retained source changed")
        common.observe_disk(report, output, "complete")
        report.update(status="COMPLETE", finished_utc=utc_now(), process_count=2 * args.cold_repeats,
                      frame_count=2 * args.cold_repeats * (1 + args.warmups + args.repeats), untimed_control_frame_count=len(report["controls"]))
        write_json(output / "summary.json", report)
        (output / "index.html").write_text(html_report(report))
        export_gallery(report, output, gallery)
        check((gallery / "summary.json").read_bytes() == (output / "summary.json").read_bytes(), "Summary copies differ")
    except (OSError, ValueError, KeyError, TypeError, StopIteration, subprocess.SubprocessError) as error:
        if report is not None:
            report.update(status="FAILED", error=str(error), finished_utc=utc_now())
            write_json(output / "summary.json", report)
        print("Backend reflection benchmark failed: " + str(error), file=sys.stderr)
        return 1
    print(output / "summary.json")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
