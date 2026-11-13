"""Paired Compatible/Native Metal timings with separate, hash-bound libraries.

Cross-profile differences are permitted; each profile must repeat exact RGB.
Stage records must repeat within each process, with stable activity between
processes. This runner does not claim CPU/Pascal/native-x86 parity. The
existing benchmark_render_arm64 runner remains Compatible-only.
"""
from __future__ import annotations

import argparse
import hashlib
import html
import json
import math
import os
from pathlib import Path
import shutil
import statistics
import subprocess
import sys

from benchmark_gallery_arm64 import (ROOT, binary_identity, execute, native_host,
                                      relative_file, sha, utc_now)
from benchmark_precision import (MODES, DISK_FLOOR, admit_profile_reference,
                                 check_disk, mode_order, observe_disk, pixels,
                                 quiet_host as cpu_quiet_host, rgb_difference,
                                 write_json)
from benchmark_render_arm64 import (METAL_RENDER_SETTINGS, METAL_STAGES,
                                    TIMINGS, duration, image_details,
                                    load_selection, metal_count,
                                    stats, validate_metal_frame)
from native_evidence import source_identity

SCHEMA = "mb3d.metal-precision-benchmark.v1"
SCENE_IDS = ("sphereifs-ao-shadows", "boxifs-ao-shadows", "integerpower8-ao-shadows")


def quiet_host():
    cpu_quiet_host()
    commands = subprocess.check_output(["ps", "-axo", "comm="], text=True).splitlines()
    active = sorted({Path(c.strip()).name for c in commands
                     if Path(c.strip()).name.startswith("mb3d_")
                     or Path(c.strip()).name in {"metal", "metallib", "air-lld", "ctest"}})
    if active:
        raise ValueError("Concurrent Metal/build/test activity: " + ", ".join(active))


def validate_libraries(paths):
    if set(paths) != set(MODES):
        raise ValueError("Both explicit Metal profile libraries are required")
    records = {}
    for mode, path in paths.items():
        path = path.resolve(strict=True)
        if not path.is_file() or path.suffix != ".metallib":
            raise ValueError("Expected a precompiled .metallib for " + mode)
        records[mode] = {"path": str(path), "sha256": sha(path), "bytes": path.stat().st_size}
    if records["compatible"]["sha256"] == records["native"]["sha256"]:
        raise ValueError("Compatible/Native libraries must have distinct byte identities")
    return records


# Keep this common pipeline contract local: relaxing the Compatible-only
# production qualification wrapper would silently change its admission policy.
def validate_pipeline_metadata(report, required_stages, mode):
    if (report.get("precision") != mode or report.get("backend") != "metal" or report.get("native_arm64") is not True
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


def validate_precision(report, mode, library):
    if mode not in MODES:
        raise ValueError("Unknown requested Metal precision")
    execution = report.get("execution", {})
    expected_shader = {"compatible": "software_binary64_extended80", "native": "hardware_float32"}[mode]
    marker = {"profile_id": 1 if mode == "compatible" else 2,
              "scene_abi_version": 4, "arithmetic_bits": 80 if mode == "compatible" else 32}
    actual_marker = report.get("library_profile", {})
    if (report.get("precision") != mode or execution.get("precision") != mode
            or execution.get("shader_precision") != expected_shader
            or execution.get("cpu_preparation_precision") != mode
            or report.get("library_precision") != mode or report.get("library") != str(library)
            or actual_marker != marker or any(type(actual_marker.get(k)) is not int for k in marker)
            or type(report.get("profile_validation_dispatches")) is not int
            or report.get("profile_validation_dispatches") != 1):
        raise ValueError("Missing/mismatched precision, library or GPU profile marker")
    profile = report.get("numeric_profile", "")
    prefix = "EXACT_" if mode == "compatible" else "NATIVE_FP32_"
    if not isinstance(profile, str) or not profile.startswith(prefix) or not profile.endswith("_FULL_SHADING"):
        raise ValueError("Missing/wrong numeric profile")
    if report.get("runtime_shader_compilation") is True:
        raise ValueError("Only frozen precompiled Metal libraries are allowed")


def validate_report(report, directory, mode, library, settings, scene):
    validate_precision(report, mode, library)
    if report.get("schema") != "mb3d.metal-render-benchmark.v1" or report.get("status") != "COMPLETE":
        raise ValueError("Metal benchmark report incomplete")
    if any(report.get("settings", {}).get(key) != value for key, value in settings.items()):
        raise ValueError("Metal benchmark settings mismatch")
    if (report.get("scene") != str(scene["frozen_path"])
            or report.get("scene_description", {}).get("scene_sha256") != scene["scene_sha256"]):
        raise ValueError("Loaded Metal scene identity mismatch")
    validate_pipeline_metadata(report, scene["required_stages"], mode)
    cold, warmups, samples = report.get("cold"), report.get("warmups"), report.get("samples")
    if (not isinstance(cold, dict) or not isinstance(warmups, list) or not isinstance(samples, list)
            or len(warmups) != settings["warmups"] or len(samples) != settings["repeats"]):
        raise ValueError("Incomplete Metal frame inventory")
    frames = [(cold, "cold", 0), *[(f, "warmup", i) for i, f in enumerate(warmups)],
              *[(f, "warm", i) for i, f in enumerate(samples)]]
    reference = None
    images = []
    for frame, phase, index in frames:
        name = "cold.png" if phase == "cold" else f"{phase}-{index:03d}.png"
        if (frame.get("phase") != phase or frame.get("index") != index or frame.get("image") != name
                or frame.get("state", "completed") != "completed"
                or frame.get("width") != settings["width"] or frame.get("height") != settings["height"]
                or frame.get("exact_rgb_matches_cold") is not True):
            raise ValueError("Missing/reordered frame identity or RGB repeat claim")
        for key in TIMINGS:
            duration(frame.get(key), key)
        if frame["render_ms"] <= 0 or frame["png_encode_write_ms"] <= 0:
            raise ValueError("Render and PNG timings must represent executed work")
        validate_metal_frame(frame, report, settings, scene["required_stages"])
        if metal_count(frame.get("nonfinite_primary_values"), "nonfinite primary values") != 0:
            raise ValueError("Nonfinite primary values in Metal output")
        for name_stage, stage in frame["stages"].items():
            seconds = duration(stage.get("seconds"), name_stage)
            if stage.get("state") not in ("completed", "not_run") or (stage["state"] == "not_run" and seconds != 0):
                raise ValueError("Contradictory stage inventory")
        file = relative_file(directory, name)
        identity = image_details(file, settings["width"], settings["height"])
        if not identity["nonconstant"]:
            raise ValueError("Constant Metal image cannot prove active effects")
        rgb = pixels(file, (settings["width"], settings["height"]))
        if reference is not None and rgb != reference:
            raise ValueError("Decoded Metal RGB differs within profile process")
        reference = rgb
        images.append({"path": name, **identity})
    return reference, images


def summarize_processes(processes):
    reports = [p["native"] for p in processes]
    if not reports:
        raise ValueError("No Metal processes to summarize")
    keys = (*TIMINGS, "gpu_timestamp_ms")
    result = {
        "cold": {key: stats([r["cold"][key] for r in reports]) for key in (*keys, "cold_total_ms", "cold_end_to_end_ms")},
        "warm": {key: stats([statistics.median(f[key] for f in r["samples"]) for r in reports]) for key in keys},
        "setup_ms": {key: stats([r["setup_ms"][key] for r in reports]) for key in reports[0]["setup_ms"]},
        "stage_wall_ms": {}, "stage_gpu_ms": {},
        "process_wall_seconds": stats([p["process"]["wall_seconds"] for p in processes]),
        "process_cpu_seconds": stats([p["process"]["cpu_seconds"] for p in processes]),
        "process_peak_rss_bytes": stats([p["process"]["peak_rss_bytes"] for p in processes]),
        "method": "Warm distributions contain one median per independent fresh process; GPU timestamps and wall durations remain separate."}
    for name in METAL_STAGES:
        state = reports[0]["cold"]["stages"][name]["state"]
        if any(f["stages"][name]["state"] != state for r in reports for f in [r["cold"], *r["warmups"], *r["samples"]]):
            raise ValueError("Stage activity changed between fresh processes")
        for kind, field, multiplier in (("stage_wall_ms", "seconds", 1000), ("stage_gpu_ms", "gpu_ms", 1)):
            result[kind][name] = {"state": state,
                "cold": stats([r["cold"]["stages"][name][field] * multiplier for r in reports]),
                "warm": stats([statistics.median(f["stages"][name][field] * multiplier for f in r["samples"]) for r in reports])}
    return result


def select_scenes(path, requested=()):
    chosen = tuple(requested or SCENE_IDS)
    if len(set(chosen)) != len(chosen) or set(chosen) - set(SCENE_IDS):
        raise ValueError("Only the three admitted ordinary Metal effect scenes are allowed")
    selection = load_selection(path, selected=chosen)
    if len(selection["scenes"]) != len(chosen):
        raise ValueError("Missing selected Metal scenes")
    for row in selection["scenes"]:
        if (row["backends"] != ["cpu", "metal"]
                or not {"primary", "shadows", "ao", "color"} <= set(row["required_stages"])):
            raise ValueError("Metal scene must admit the full active AO/shadow pipeline")
    return selection


def verify_representative_image(path, recorded):
    current = image_details(path, recorded["width"], recorded["height"])
    if any(current[key] != recorded.get(key) for key in current):
        raise ValueError("Representative image changed after admission")


def export_gallery(report, output, destination):
    if report["status"] != "COMPLETE" or destination.exists() or not destination.parent.is_dir():
        raise ValueError("Gallery requires complete results and a new destination")
    check_disk(destination.parent)
    destination.mkdir()
    manifest = []
    def copy(source, relative):
        target = destination / relative
        target.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(source, target)
        if sha(source) != sha(target):
            raise ValueError("Retained evidence copy changed")
        manifest.append({"file": str(relative), "sha256": sha(target), "bytes": target.stat().st_size})
    for dirname in ("evidence", "inputs", "runtime"):
        for source in sorted((output / dirname).rglob("*")):
            if source.is_file():
                copy(source, Path(dirname) / source.relative_to(output / dirname))
    for row in report["scenes"]:
        for profile in row["profiles"].values():
            for process in profile["processes"]:
                for key, digest_key in (("report", "report_sha256"), ("log", "log_sha256")):
                    relative = Path(process[key])
                    # Validate the serialized POSIX path before using an OS-native Path.
                    source = relative_file(output, process[key])
                    if sha(source) != process[digest_key]:
                        raise ValueError("Process evidence changed before retention")
                    copy(source, relative)
    body = ['<!doctype html><html lang="ru"><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1">',
            '<title>Metal — Compatible / Native</title><style>body{font:16px/1.5 system-ui;margin:32px;background:#f4f3ef;color:#202729}table{border-collapse:collapse;background:white}th,td{padding:10px;border:1px solid #ccd3d0}.pair{display:flex;gap:20px;flex-wrap:wrap}figure{margin:12px 0}img{width:min(512px,90vw)}pre{white-space:pre-wrap}</style>',
            '<h1>Metal: Compatible / Native</h1><p>Native: аппаратная FP32 на GPU, нативная FP64/libm при CPU-подготовке. Compatible: программная binary64/extended80. Между режимами отличия RGB допустимы; внутри каждого режима требуются точные повторы.</p>',
            '<p>Cold — свежий процесс и контекст, кэши ОС не очищаются. Driver cold включает загрузку/подготовку, но не запуск процесса. Warm повторно использует ресурсы. Времена рендера/readback/PNG, GPU timestamps и process wall/CPU/RSS разделены в JSON. CPU oracle и fallback в измерениях отсутствуют.</p>',
            '<table><tr><th>Сцена</th><th>Cold C RGB, мс</th><th>Cold N RGB, мс</th><th>Warm C RGB, мс</th><th>Warm N RGB, мс</th><th>Warm C/N</th><th>Δ пикселей</th></tr>']
    for row in report["scenes"]:
        c, n = [row["profiles"][mode]["summary"] for mode in MODES]
        values = [row["id"], *[f'{x:.3f}' for x in (c["cold"]["cold_total_ms"]["median"], n["cold"]["cold_total_ms"]["median"],
                  c["warm"]["render_and_readback_ms"]["median"], n["warm"]["render_and_readback_ms"]["median"])],
                  f'{row["warm_ratio_compatible_over_native"]:.3f}×', row["cross_profile_rgb"]["different_pixels"]]
        body.append('<tr>' + ''.join('<td>' + html.escape(str(v)) + '</td>' for v in values) + '</tr>')
    body.append('</table><p>Это проверка исполнения, эффектов и повторяемости. Native не получает статус EXACT/parity. Сравнение с Pascal, CPU и нативным x86 в этот запуск не входит.</p>')
    for row in report["scenes"]:
        body.append('<h2>' + html.escape(row["id"]) + '</h2><div class="pair">')
        for mode in MODES:
            first = row["profiles"][mode]["processes"][0]
            source = output / first["directory"] / "cold.png"
            recorded = first["images"][0]
            if recorded.get("path") != "cold.png":
                raise ValueError("Missing admitted representative image")
            verify_representative_image(source, recorded)
            name = Path("images") / (row["id"] + '-' + mode + '.png')
            copy(source, name)
            verify_representative_image(destination / name, recorded)
            body.append('<figure><figcaption>' + mode + '</figcaption><img src="' + name.as_posix() + '"></figure>')
        body.append('</div>')
    body.append('<p><a href="summary.json">Полный JSON с границами таймингов и хешами</a> · <a href="manifest.json">Сохранённые файлы SHA256</a></p></html>')
    (destination / "index.html").write_text('\n'.join(body))
    report["retained_gallery"] = {"path": str(destination), "representative_images": "images/<scene>-<precision>.png",
        "retained": "Frozen binary/libraries, source patch, inputs/resources, every process report and log; six first-cold PNGs by default. Other raw frame PNGs remain in the working output."}
    observe_disk(report, output, "end")
    report["finished_utc"] = utc_now()
    write_json(destination / "summary.json", report)
    manifest.extend({"file": name, "sha256": sha(destination / name), "bytes": (destination / name).stat().st_size} for name in ("summary.json", "index.html"))
    write_json(destination / "manifest.json", {"schema": SCHEMA, "files": manifest})
    (destination / "KEEP_FINAL_IMAGES.txt").write_text("Retained Metal precision evidence, libraries, inputs and final images. Preserve during cleanup.\n")


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--metal", type=Path, required=True)
    parser.add_argument("--compatible-library", type=Path, required=True)
    parser.add_argument("--native-library", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--gallery", type=Path)
    parser.add_argument("--selection", type=Path, default=ROOT / "comparison/benchmark_scenes/effects-v1.json")
    parser.add_argument("--scene", action="append", default=[])
    for name, default in (("width", 512), ("height", 384), ("threads", 6), ("cold-repeats", 3), ("warmups", 1), ("repeats", 3)):
        parser.add_argument("--" + name, type=int, default=default)
    parser.add_argument("--timeout", type=float, default=900)
    args = parser.parse_args(argv)
    report = None
    try:
        output = args.output.expanduser().resolve()
        if output.exists() or not output.parent.is_dir():
            raise ValueError("Output must be a new directory with an existing parent")
        gallery = args.gallery.expanduser().resolve() if args.gallery else output / "gallery"
        if gallery.exists() or (args.gallery and not gallery.parent.is_dir()) or gallery == output:
            raise ValueError("Gallery must be a separate new directory with an existing parent")
        if (min(args.width, args.height, args.threads) < 1 or args.threads > min(64, args.height)
                or not 3 <= args.cold_repeats <= 15 or not 1 <= args.warmups <= 15 or not 3 <= args.repeats <= 15
                or not math.isfinite(args.timeout) or args.timeout <= 0):
            raise ValueError("Positive dimensions/timeout, >=3 fresh/warm repeats and >=1 warmup required")
        initial_free = check_disk(output.parent)
        quiet_host()
        host = native_host()
        settings = {k: getattr(args, k) for k in ("width", "height", "threads", "warmups", "repeats")}
        selection = select_scenes(args.selection, args.scene)
        metal = args.metal.resolve(strict=True)
        binary = binary_identity(metal)
        libraries = validate_libraries({"compatible": args.compatible_library, "native": args.native_library})
        source = source_identity()
        output.mkdir()
        report = {"schema": SCHEMA, "status": "INITIALIZING", "started_utc": utc_now(),
                  "disk_free_bytes": {"floor": DISK_FLOOR, "start": initial_free,
                                      "minimum_observed": initial_free, "observations": []}}
        write_json(output / "summary.json", report)
        for name in ("evidence/bin", "evidence/source", "inputs", "runtime", "runs", "logs"):
            (output / name).mkdir(parents=True)
        frozen_binary = output / "evidence/bin/mb3d_metal_exact_scene"
        shutil.copy2(metal, frozen_binary)
        binary["frozen_path"] = str(frozen_binary.relative_to(output))
        for mode, identity in libraries.items():
            target = output / "evidence/bin" / (mode + ".metallib")
            shutil.copy2(identity["path"], target)
            identity["frozen_path"] = str(target.relative_to(output))
        (output / "evidence/source/working-tree.patch").write_bytes(subprocess.check_output(["git", "diff", "HEAD", "--binary"], cwd=ROOT))
        for name, digest in source["changed_source_sha256"].items():
            if digest:
                target = output / "evidence/source" / name
                target.parent.mkdir(parents=True, exist_ok=True)
                shutil.copy2(ROOT / name, target)
                if sha(target) != digest:
                    raise ValueError("Source changed during freeze: " + name)
        write_json(output / "inputs/selection.json", selection)
        for asset in selection.get("assets", []):
            target = output / "runtime" / asset["runtime_path"]
            target.parent.mkdir(parents=True, exist_ok=True)
            shutil.copy2(relative_file(ROOT, asset["source_path"]), target)
            if sha(target) != asset["sha256"]:
                raise ValueError("Frozen resource mismatch")
        for scene in selection["scenes"]:
            target = output / "inputs" / (scene["id"] + ".m3p")
            shutil.copy2(relative_file(ROOT, scene["scene_path"]), target)
            if sha(target) != scene["scene_sha256"]:
                raise ValueError("Frozen scene mismatch")
        report = {"schema": SCHEMA, "status": "RUNNING", "started_utc": utc_now(), "host": host,
                  "settings": settings, "cold_repeats": args.cold_repeats, "modes": list(MODES),
                  "source_identity": source, "binary": binary, "libraries": libraries,
                  "source_binding": "Observed source plus frozen bytes; not a claim that observation alone proves build inputs",
                  "selection_sha256": sha(args.selection), "selected_count": len(selection["scenes"]),
                  "scenes": [], "commands": [], "assets": selection.get("assets", []),
                  "disk_free_bytes": {"floor": DISK_FLOOR, "start": initial_free, "minimum_observed": initial_free, "observations": []},
                  "cross_profile_rgb_differences_allowed": True,
                  "scope": "Per-profile repeat, fault, stage and effect-activity validation only; no CPU/Pascal/native-x86 parity qualification",
                  "method": {"cold": "new process/context; driver load/preparation to RGB; OS caches not flushed; process launch excluded",
                             "warm": "reuse scene, pipelines and buffers; median per fresh process then across processes",
                             "repeatability": "Decoded RGB exact across every frame/process within each profile; stage records exact within each process, stage activity stable across processes",
                             "sequential": "mode order alternates by cold repetition; no VMs/builds/renderers/tests active at launch",
                             "gpu": "GPU timestamp distributions are separate from encode/submit/wait wall durations",
                             "process": "wait4 wall/CPU/peakRSS cover all child work, including untimed validation and output"}}
        write_json(output / "summary.json", report)
        env = {k: v for k, v in os.environ.items() if not k.upper().startswith("MB3D_")}
        report["removed_environment_variable_names"] = sorted(set(os.environ) - set(env))
        def verify_inputs():
            if sha(metal) != binary["sha256"] or sha(frozen_binary) != binary["sha256"]:
                raise ValueError("Metal binary changed")
            for identity in libraries.values():
                if sha(Path(identity["path"])) != identity["sha256"] or sha(output / identity["frozen_path"]) != identity["sha256"]:
                    raise ValueError("Metal library changed")
        for scene_source in selection["scenes"]:
            scene = dict(scene_source, frozen_path=output / "inputs" / (scene_source["id"] + ".m3p"))
            row = {"id": scene["id"], "scene_sha256": scene["scene_sha256"], "required_stages": scene["required_stages"],
                   "status": "RUNNING", "process_order": [], "profiles": {m: {"processes": []} for m in MODES}}
            report["scenes"].append(row)
            references = {}
            metadata = {}
            for index in range(args.cold_repeats):
                for mode in mode_order(index):
                    quiet_host(); observe_disk(report, output, f"before:{scene['id']}:{mode}:{index}"); verify_inputs()
                    directory = output / "runs" / f"{scene['id']}-{mode}-{index:02d}"
                    library = output / libraries[mode]["frozen_path"]
                    command = [str(frozen_binary), "--scene", str(scene["frozen_path"]), "--output", str(directory),
                               "--benchmark-render", "--full-shading", "--library", str(library), "--precision", mode]
                    for key, value in settings.items():
                        command += ["--" + key, str(value)]
                    log = output / "logs" / (directory.name + ".log")
                    entry = {"precision": mode, "cold_index": index, "argv": command, "directory": str(directory.relative_to(output)),
                             "log": str(log.relative_to(output)), "status": "RUNNING"}
                    row["process_order"].append({"cold_index": index, "precision": mode})
                    row["profiles"][mode]["processes"].append(entry)
                    report["commands"].append({"argv": command, "cwd": str(output / "runtime")})
                    write_json(output / "summary.json", report)
                    entry["process"] = execute(command, output / "runtime", log, args.timeout, env)
                    entry["log_sha256"] = sha(log)
                    observe_disk(report, output, f"after:{scene['id']}:{mode}:{index}")
                    if entry["process"]["timed_out"] or entry["process"]["returncode"] != 0:
                        raise ValueError("Metal render failed/timed out; see " + str(log))
                    raw = json.loads((directory / "report.json").read_text())
                    rgb, images = validate_report(raw, directory, mode, library, settings, scene)
                    admit_profile_reference(references, mode, rgb)
                    binding = {key: raw[key] for key in ("numeric_profile", "scene_description", "loaded_features", "library_profile", "execution", "device")}
                    if mode in metadata and metadata[mode] != binding:
                        raise ValueError("Metal prepared profile/device changed between processes")
                    metadata[mode] = binding
                    entry.update(status="PASS", native=raw, images=images, report_sha256=sha(directory / "report.json"),
                                 report=str((directory / "report.json").relative_to(output)),
                                 exact_within_process=True, rgb_sha256=hashlib.sha256(rgb).hexdigest())
                    write_json(output / "summary.json", report)
            for mode, profile in row["profiles"].items():
                profile.update(summary=summarize_processes(profile["processes"]), exact_rgb_within_profile=True,
                               exact_stage_records_within_each_process=True, stable_stage_activity_across_processes=True,
                               rgb_sha256=hashlib.sha256(references[mode]).hexdigest(), metadata=metadata[mode])
            row["cross_profile_rgb"] = rgb_difference(references["compatible"], references["native"])
            numerator, denominator = [row["profiles"][m]["summary"]["warm"]["render_and_readback_ms"]["median"] for m in MODES]
            if denominator <= 0:
                raise ValueError("Native Metal render duration is zero")
            row.update(status="COMPLETE", warm_ratio_compatible_over_native=numerator / denominator)
            write_json(output / "summary.json", report)
        report["source_identity_at_end"] = source_identity()
        if report["source_identity_at_end"] != source or sha(args.selection) != report["selection_sha256"]:
            raise ValueError("Source/selection changed during Metal benchmark")
        verify_inputs()
        for scene in selection["scenes"]:
            if any(sha(path) != scene["scene_sha256"] for path in (relative_file(ROOT, scene["scene_path"]), output / "inputs" / (scene["id"] + ".m3p"))):
                raise ValueError("Scene changed during benchmark")
        for asset in selection.get("assets", []):
            if any(sha(path) != asset["sha256"] for path in (relative_file(ROOT, asset["source_path"]), output / "runtime" / asset["runtime_path"])):
                raise ValueError("Resource changed during benchmark")
        quiet_host(); observe_disk(report, output, "complete")
        report.update(status="COMPLETE", finished_utc=utc_now(), frame_count=len(report["scenes"]) * 2 * args.cold_repeats * (1 + args.warmups + args.repeats))
        write_json(output / "summary.json", report)
        report["gallery"] = str(gallery)
        export_gallery(report, output, gallery)
    except (OSError, ValueError, KeyError, TypeError, subprocess.SubprocessError) as error:
        if report is not None:
            report.update(status="FAILED", error=str(error))
        print("Metal precision benchmark failed: " + str(error), file=sys.stderr)
        return 1
    finally:
        if report is not None:
            if report["status"] != "COMPLETE":
                observe_disk(report, output, "end", enforce=False)
                report["finished_utc"] = utc_now()
            write_json(output / "summary.json", report)
    print(output / "summary.json")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
