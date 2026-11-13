"""Fresh Native CPU FP64 versus Native Metal FP32 full-render measurements.

Same scene inputs and output dimensions do not make these identical arithmetic
profiles. RGB differences between backends are reported; each backend must
repeat exact RGB within and across its independent fresh processes.
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

from PIL import Image

import benchmark_precision as cpu
import benchmark_metal_precision as metal
from benchmark_gallery_arm64 import ROOT, binary_identity, execute, native_host, relative_file, sha, utc_now
from benchmark_render_arm64 import TIMINGS, controls, duration, image_details, require_stages, stats
from native_evidence import source_identity

SCHEMA = "mb3d.native-backend-benchmark.v1"
BACKENDS = ("cpu", "metal")
DISK_FLOOR = cpu.DISK_FLOOR
check_disk, observe_disk, write_json = cpu.check_disk, cpu.observe_disk, cpu.write_json
quiet_host = metal.quiet_host
rgb_difference = cpu.rgb_difference
verify_representative_image = metal.verify_representative_image


def backend_order(index):
    return BACKENDS if index % 2 == 0 else tuple(reversed(BACKENDS))


def admit_backend_reference(references, backend, rgb):
    if backend not in BACKENDS:
        raise ValueError("Unknown backend")
    if backend in references and references[backend] != rgb:
        raise ValueError("Fresh-process RGB differs within backend")
    references.setdefault(backend, rgb)


def validate_cpu_report(report, directory, settings, scene, runtime):
    rgb, details = cpu.validate_report(report, directory, "native", settings, scene, runtime)
    features = report["loaded_features"]
    for name in ("bCalculateHardShadow", "bCalcAmbShadowAutomatic", "bNormalsOnDE"):
        if type(features.get(name)) is not int or not features[name] & 1:
            raise ValueError("Native CPU required effect is disabled: " + name)
    setup = report.get("setup_ms", {})
    if set(setup) != {"load", "ordinary_input_check"} or duration(setup.get("load"), "CPU load") <= 0:
        raise ValueError("CPU setup timing inventory is incomplete")
    duration(setup.get("ordinary_input_check"), "CPU ordinary input check")
    images = []
    frames = [report["cold"], *report["warmups"], *report["samples"]]
    for frame in frames:
        if frame["threads"] != settings["threads"]:
            raise ValueError("CPU worker count differs from requested benchmark settings")
        if frame.get("error"):
            raise ValueError("CPU frame has an error")
        render, readback, combined, png = (frame[k] for k in ("render_ms", "readback_ms", "render_and_readback_ms", "png_encode_write_ms"))
        if render <= 0 or png <= 0 or combined + .01 < render + readback:
            raise ValueError("CPU timing excludes completed render/surface-copy work")
        preparation = duration(frame.get("preparation_ms"), "CPU preparation")
        if not math.isclose(preparation, frame["stages"]["preparation"]["seconds"] * 1000, rel_tol=1e-9, abs_tol=1e-6):
            raise ValueError("CPU preparation timing contradicts stage profile")
        if frame["end_to_end_ms"] + .01 < combined + png:
            raise ValueError("CPU end-to-end timing excludes PNG output")
        for name, stage in frame["stages"].items():
            if stage["state"] == "not_run" and stage["seconds"] != 0:
                raise ValueError("CPU inactive stage has nonzero duration: " + name)
        require_stages(frame["stages"], scene["required_stages"], ("volume", "normals", "reflections", "dof"))
        if frame["phase"] == "cold":
            if (frame["cold_total_ms"] + .01 < setup["load"] + setup["ordinary_input_check"] + combined
                    or frame["cold_end_to_end_ms"] + .01 < frame["cold_total_ms"] + png
                    or frame["end_to_end_ms"] != frame["cold_end_to_end_ms"]):
                raise ValueError("CPU cold load-to-RGB/PNG timing boundary is inconsistent")
        path = relative_file(directory, frame["image"])
        identity = image_details(path, settings["width"], settings["height"])
        if not identity["nonconstant"]:
            raise ValueError("Constant CPU image cannot prove effect activity")
        images.append({"path": frame["image"], **identity})
    if report.get("error"):
        raise ValueError("CPU report has an error")
    return rgb, images


def validate_metal_report(report, directory, library, settings, scene):
    return metal.validate_report(report, directory, "native", library, settings, scene)


def summarize_processes(processes, backend):
    if backend == "metal":
        return metal.summarize_processes(processes)
    if backend != "cpu" or not processes:
        raise ValueError("Unknown backend or empty process inventory")
    reports = [p["native"] for p in processes]
    result = {
        "cold": {key: stats([r["cold"][key] for r in reports]) for key in (*TIMINGS, "cold_total_ms", "cold_end_to_end_ms", "preparation_ms")},
        "warm": {key: stats([statistics.median(f[key] for f in r["samples"]) for r in reports]) for key in (*TIMINGS, "preparation_ms")},
        "setup_ms": {key: stats([r["setup_ms"][key] for r in reports]) for key in reports[0]["setup_ms"]},
        "stage_wall_ms": {},
        "process_wall_seconds": stats([p["process"]["wall_seconds"] for p in processes]),
        "process_cpu_seconds": stats([p["process"]["cpu_seconds"] for p in processes]),
        "process_peak_rss_bytes": stats([p["process"]["peak_rss_bytes"] for p in processes]),
        "method": "Warm distributions contain one median per independent fresh process; CPU resources/workers are recreated by each Render."}
    for name in cpu.STAGES:
        cold_state = reports[0]["cold"]["stages"][name]["state"]
        warm_state = reports[0]["samples"][0]["stages"][name]["state"]
        if (any(r["cold"]["stages"][name]["state"] != cold_state for r in reports)
                or any(f["stages"][name]["state"] != warm_state for r in reports for f in [*r["warmups"], *r["samples"]])):
            raise ValueError("CPU stage activity changed across matching phases")
        result["stage_wall_ms"][name] = {"cold_state": cold_state, "warm_state": warm_state,
            "cold": stats([r["cold"]["stages"][name]["seconds"] * 1000 for r in reports]),
            "warm": stats([statistics.median(f["stages"][name]["seconds"] * 1000 for f in r["samples"]) for r in reports])}
    return result


def validate_cli_profile(profile, required, not_run=()):
    if (profile.get("status") != "complete" or profile.get("render_status") != "completed"
            or profile.get("precision") != "native"):
        raise ValueError("Untimed Native CPU production control did not complete")
    require_stages(profile.get("stages", {}), required, not_run)


def export_gallery(report, output, destination):
    if report["status"] != "COMPLETE" or destination.exists() or not destination.parent.is_dir():
        raise ValueError("Gallery requires completed results and a new destination")
    check_disk(destination.parent)
    destination.mkdir()
    manifest = []
    def copy(source, relative, digest=None):
        if digest and sha(source) != digest:
            raise ValueError("Evidence changed before retention: " + str(relative))
        target = destination / relative
        target.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(source, target)
        if sha(source) != sha(target) or (digest and sha(target) != digest):
            raise ValueError("Evidence copy differs: " + str(relative))
        manifest.append({"file": str(relative), "sha256": sha(target), "bytes": target.stat().st_size})
    for folder in ("evidence", "inputs", "runtime"):
        for path in sorted((output / folder).rglob("*")):
            if path.is_file():
                copy(path, path.relative_to(output))
    for scene in report["scenes"]:
        for data in scene["backends"].values():
            for process in data["processes"]:
                for key in ("report", "log"):
                    copy(relative_file(output, process[key]), Path(process[key]), process[key + "_sha256"])
        for control in scene.get("untimed_controls", []):
            for key in ("report", "log"):
                copy(relative_file(output, control[key]), Path(control[key]), control[key + "_sha256"])
            verify_representative_image(output / control["image"], control["image_identity"])
            copy(output / control["image"], Path(control["image"]), control["image_identity"]["image_sha256"])
    body = ['<!doctype html><html lang="ru"><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1">',
        '<title>Native CPU / Native Metal</title><style>body{font:16px/1.5 system-ui;background:#f4f3ef;color:#202729;margin:32px}.table-scroll{overflow-x:auto;max-width:100%}table{border-collapse:collapse;background:white}th,td{padding:9px;border:1px solid #ccd3d0}.pair{display:flex;flex-wrap:wrap;gap:16px}figure{margin:8px 0}img{width:min(512px,90vw)}pre{white-space:pre-wrap;font-size:12px}details{margin:12px 0}</style>',
        '<h1>Native CPU / Native Metal</h1><p>CPU: нативная FP64/libm без FMA contraction. Metal: аппаратная FP32, возможен FMA; CPU-подготовка Native FP64. Это сравнение двух режимов исполнения, а не ускорение одинаковой арифметики. Различия RGB между ними допустимы и измерены.</p>',
        '<p>Cold: новый процесс, загрузка сцены и подготовка ресурсов до RGB; запуск процесса не включён, кэши ОС не очищались. Warm: CPU заново создаёт ресурсы и workers каждого Render; Metal повторно использует scene, pipelines и buffers. '
        + f'Для каждого backend: {report.get("cold_repeats", 3)} процессов; в каждом {report.get("settings", {}).get("warmups", 1)} warmup и {report.get("settings", {}).get("repeats", 3)} warm-кадров. '
        + 'Основная warm-оценка: медиана медиан процессов. Процессы CPU/Metal запускались последовательно с чередованием порядка.</p>',
        '<p>Внутри каждого backend все RGB-повторы точные. CPU подтверждает завершение стадий и действие AO/shadows отдельными контролями; Metal также проверяет повтор записей стадий внутри процесса и отсутствие faults/нечисловых значений. Untimed CLI reference и effect-off controls не входят в измерения.</p>',
        '<div class="table-scroll"><table><tr><th>Сцена</th><th>Cold CPU→RGB, мс</th><th>Cold Metal→RGB, мс</th><th>Cold CPU/Metal</th><th>Warm CPU→RGB, мс</th><th>Warm Metal→RGB, мс</th><th>Warm CPU/Metal</th><th>Δ пикселей</th><th>Max Δ</th><th>MAE</th></tr>']
    for row in report["scenes"]:
        c, m = [row["backends"][b]["summary"] for b in BACKENDS]
        values = [row["id"], f'{c["cold"]["cold_total_ms"]["median"]:.3f}', f'{m["cold"]["cold_total_ms"]["median"]:.3f}',
            f'{row["cold_ratio_cpu_over_metal"]:.3f}×', f'{c["warm"]["render_and_readback_ms"]["median"]:.3f}',
            f'{m["warm"]["render_and_readback_ms"]["median"]:.3f}', f'{row["warm_ratio_cpu_over_metal"]:.3f}×',
            row["cross_backend_rgb"]["different_pixels"], row["cross_backend_rgb"]["maximum_channel_error"], f'{row["cross_backend_rgb"]["mean_absolute_channel_error"]:.6f}']
        body.append('<tr>' + ''.join('<td>' + html.escape(str(v)) + '</td>' for v in values) + '</tr>')
    body.append('</table></div>')
    aggregate = report.get("aggregate", {})
    if aggregate:
        body.append(f'<p>Геометрическое среднее warm CPU/Metal по всем сценам: {aggregate["warm_speed_ratio_geometric_mean"]:.3f}×.</p>')
    for row in report["scenes"]:
        body.append('<h2>' + html.escape(row["id"]) + '</h2><div class="pair">')
        image_data = {}
        for backend in BACKENDS:
            entry = row["backends"][backend]["processes"][0]
            recorded = entry["images"][0]
            if recorded.get("path") != "cold.png":
                raise ValueError("Missing admitted representative image")
            source = output / entry["directory"] / "cold.png"
            verify_representative_image(source, recorded)
            name = Path("images") / (row["id"] + '-' + backend + '.png')
            copy(source, name, recorded["image_sha256"])
            verify_representative_image(destination / name, recorded)
            image_data[backend] = cpu.pixels(source, (recorded["width"], recorded["height"]))
            body.append('<figure><figcaption>Native ' + backend + '</figcaption><img src="' + name.as_posix() + '"></figure>')
        diff = bytes(min(255, abs(a - b) * 8) for a, b in zip(image_data["cpu"], image_data["metal"]))
        path = destination / "images" / (row["id"] + '-difference-x8.png')
        Image.frombytes("RGB", (recorded["width"], recorded["height"]), diff).save(path)
        manifest.append({"file": str(path.relative_to(destination)), "sha256": sha(path), "bytes": path.stat().st_size})
        body.append('<figure><figcaption>|CPU−Metal| ×8, значения ограничены 255</figcaption><img src="' + path.relative_to(destination).as_posix() + '"></figure></div>')
        body.append('<details><summary>Стадии, setup, readback, PNG и process wall/CPU/RSS</summary><p>CPU readback — копия финальной RGB-поверхности на CPU. CPU preparation включена в каждый Render; setup shadows/AO/color находится внутри соответствующих стадий. GPU timestamps отделены от wall time. Cold end_to_end разных драйверов имеет разные границы: сравнивается только cold_end_to_end_ms.</p><pre>' + html.escape(json.dumps({b: row["backends"][b]["summary"] for b in BACKENDS}, indent=2)) + '</pre></details>')
    body.append('<p><a href="summary.json">Полный отчёт, команды и исходные профили</a> · <a href="manifest.json">SHA256 сохранённых файлов</a></p><p>Monte Carlo/JIT исключены. Pascal/native-x86 parity этим запуском не проверяется.</p></html>')
    (destination / "index.html").write_text('\n'.join(body))
    report["retained_gallery"] = {"path": str(destination), "representative_images": "images/<scene>-<backend>.png",
        "diff_images": "images/<scene>-difference-x8.png", "retained": "All process/control reports and logs, control PNGs, frozen binaries/library/source/build evidence/inputs; six representative RGB PNGs and three deterministic difference images by default. Other raw frame PNGs remain in working output."}
    observe_disk(report, output, "end")
    report["finished_utc"] = utc_now()
    write_json(destination / "summary.json", report)
    manifest.extend({"file": name, "sha256": sha(destination / name), "bytes": (destination / name).stat().st_size} for name in ("summary.json", "index.html"))
    write_json(destination / "manifest.json", {"schema": SCHEMA, "files": manifest})
    (destination / "KEEP_FINAL_IMAGES.txt").write_text("Retained Native CPU/Metal evidence, binaries, inputs and final images. Preserve during cleanup.\n")


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    for key in ("cpu", "renderer", "metal", "native-library", "output"):
        parser.add_argument("--" + key, type=Path, required=True)
    parser.add_argument("--gallery", type=Path)
    parser.add_argument("--build-evidence", action="append", type=Path, default=[])
    parser.add_argument("--selection", type=Path, default=ROOT / "comparison/benchmark_scenes/effects-v1.json")
    parser.add_argument("--scene", action="append", default=[])
    for name, value in (("width", 512), ("height", 384), ("threads", 6), ("cold-repeats", 3), ("warmups", 1), ("repeats", 3)):
        parser.add_argument("--" + name, type=int, default=value)
    parser.add_argument("--timeout", type=float, default=900)
    args = parser.parse_args(argv)
    report = None
    try:
        output = args.output.expanduser().resolve()
        gallery = args.gallery.expanduser().resolve() if args.gallery else output / "gallery"
        if output.exists() or not output.parent.is_dir():
            raise ValueError("Output must be a new directory with an existing parent")
        if gallery.exists() or gallery == output or (args.gallery and not gallery.parent.is_dir()):
            raise ValueError("Gallery must be a separate new directory with an existing parent")
        if (min(args.width, args.height, args.threads) < 1 or args.threads > min(64, args.height)
                or not 3 <= args.cold_repeats <= 15 or not 1 <= args.warmups <= 15 or not 3 <= args.repeats <= 15
                or not math.isfinite(args.timeout) or args.timeout <= 0):
            raise ValueError("Positive dimensions/timeout, >=3 fresh/warm repeats and >=1 warmup required")
        initial_free = check_disk(output.parent)
        quiet_host()
        host = native_host()
        settings = {key: getattr(args, key) for key in ("width", "height", "threads", "warmups", "repeats")}
        selection = metal.select_scenes(args.selection, args.scene)
        for scene in selection["scenes"]:
            variants = controls(scene)
            if (len(variants) != 2 or {tuple(v.get("disabled_effects", [])) for v in variants} != {("ao",), ("shadows",)}
                    or any(v.get("expect_rgb_difference") is not True
                           or not set(v["disabled_effects"]) <= set(v.get("required_not_run_stages", [])) for v in variants)):
                raise ValueError("Each selected scene requires separate AO-off and shadows-off controls")
        binaries = {role: binary_identity(getattr(args, role).expanduser().resolve(strict=True)) for role in ("cpu", "renderer", "metal")}
        library_path = args.native_library.expanduser().resolve(strict=True)
        if not library_path.is_file() or library_path.suffix != ".metallib":
            raise ValueError("A precompiled Native .metallib is required")
        library = {"path": str(library_path), "sha256": sha(library_path), "bytes": library_path.stat().st_size}
        evidence = []
        for path in args.build_evidence:
            path = path.expanduser().resolve(strict=True)
            if not path.is_file():
                raise ValueError("Build evidence must be an existing file")
            evidence.append({"path": str(path), "sha256": sha(path), "bytes": path.stat().st_size})
        source = source_identity()
        output.mkdir()
        report = {"schema": SCHEMA, "status": "INITIALIZING", "started_utc": utc_now(),
                  "disk_free_bytes": {"floor": DISK_FLOOR, "start": initial_free, "minimum_observed": initial_free, "observations": []}}
        write_json(output / "summary.json", report)
        for name in ("evidence/bin", "evidence/source", "evidence/build", "inputs", "runtime", "runs", "controls", "logs"):
            (output / name).mkdir(parents=True)
        for role, identity in binaries.items():
            identity["frozen_path"] = "evidence/bin/" + {"cpu": "mb3d_cpu_render_benchmark", "renderer": "mb3d_render", "metal": "mb3d_metal_exact_scene"}[role]
            shutil.copy2(identity["path"], output / identity["frozen_path"])
        library["frozen_path"] = "evidence/bin/native.metallib"
        shutil.copy2(library_path, output / library["frozen_path"])
        for i, item in enumerate(evidence):
            item["frozen_path"] = "evidence/build/" + f'{i:02d}-' + Path(item["path"]).name
            shutil.copy2(item["path"], output / item["frozen_path"])
        (output / "evidence/source/working-tree.patch").write_bytes(subprocess.check_output(["git", "diff", "HEAD", "--binary"], cwd=ROOT))
        for name, digest in source["changed_source_sha256"].items():
            if digest:
                target = output / "evidence/source" / name
                target.parent.mkdir(parents=True, exist_ok=True)
                shutil.copy2(ROOT / name, target)
                if sha(target) != digest:
                    raise ValueError("Source changed during freeze: " + name)
        frozen_inputs = []
        for scene in selection["scenes"]:
            for i, item in enumerate([scene, *controls(scene)]):
                relative = "inputs/" + scene["id"] + ("" if i == 0 else f'-effect-off-{i-1:02d}') + ".m3p"
                shutil.copy2(relative_file(ROOT, item["scene_path"]), output / relative)
                item["frozen_scene"] = relative
                frozen_inputs.append(item)
        for asset in selection.get("assets", []):
            target = output / "runtime" / asset["runtime_path"]
            target.parent.mkdir(parents=True, exist_ok=True)
            shutil.copy2(relative_file(ROOT, asset["source_path"]), target)
        write_json(output / "inputs/selection.json", selection)
        report.update(status="RUNNING", host=host, settings=settings, cold_repeats=args.cold_repeats, backends=list(BACKENDS), precision="native",
            source_identity=source, binaries=binaries, library=library, build_evidence=evidence,
            source_binding="Observed source and frozen bytes; build evidence is copied and hash-bound, not inferred from observation alone",
            selection_sha256=sha(args.selection), selected_count=len(selection["scenes"]), assets=selection.get("assets", []), scenes=[], commands=[],
            cross_backend_rgb_differences_allowed=True,
            scope="Native CPU FP64 versus Native Metal FP32 operational comparison; per-backend RGB repeats and active ordinary effects; no same-arithmetic speedup or Pascal/native-x86 parity claim",
            method={"cold": "new process/context; scene load/preparation through RGB or PNG; OS caches not flushed; process launch excluded",
                "warm": "median of per-process medians; CPU each Render recreates job resources/workers, Metal reuses scene/pipelines/buffers",
                "readback": "CPU BuildFinalRGBSurface copy; Metal GPU result readback; both included in render_and_readback_ms",
                "end_to_end": "Cold compares only cold_end_to_end_ms; CPU cold end_to_end_ms includes loading while Metal frame end_to_end_ms excludes setup",
                "repeatability": "Exact decoded RGB for all frames/processes within each backend; CPU stage activity, Metal within-process stage records and activity checked",
                "process": "wait4 wall/CPU/RSS include all child work including report/validation; separate from driver timings",
                "controls": "Untimed Native production CLI reference and effect-off controls before all recorded benchmark processes",
                "sequential": "Backend order alternates by cold repetition; no running VMs/builds/renderers/tests at launch"})
        env = {k: v for k, v in os.environ.items() if not k.upper().startswith("MB3D_")}
        report["removed_environment_variable_names"] = sorted(set(os.environ) - set(env))
        runtime = output / "runtime"
        def verify_inputs():
            for identity in [*binaries.values(), library, *evidence]:
                if any(sha(path) != identity["sha256"] for path in (Path(identity["path"]), output / identity["frozen_path"])):
                    raise ValueError("Binary/library/build evidence changed: " + identity["path"])
            for item in frozen_inputs:
                if any(sha(path) != item["scene_sha256"] for path in (relative_file(ROOT, item["scene_path"]), output / item["frozen_scene"])):
                    raise ValueError("Scene/control input changed")
            for asset in selection.get("assets", []):
                if any(sha(path) != asset["sha256"] for path in (relative_file(ROOT, asset["source_path"]), runtime / asset["runtime_path"])):
                    raise ValueError("Resource input changed")
        def launch(entry, command, phase):
            quiet_host(); observe_disk(report, output, "before:" + phase); verify_inputs()
            entry.update(argv=command, status="RUNNING")
            report["commands"].append({"argv": command, "cwd": str(runtime), "purpose": phase})
            write_json(output / "summary.json", report)
            entry["process"] = execute(command, runtime, output / entry["log"], args.timeout, env)
            entry["log_sha256"] = sha(output / entry["log"])
            observe_disk(report, output, "after:" + phase)
            if entry["process"]["timed_out"] or entry["process"]["returncode"] != 0:
                raise ValueError("Process failed/timed out; see " + entry["log"])
        # Complete controls first, outside all recorded cold/warm processes.
        cpu_reference = {}
        for scene in selection["scenes"]:
            row = {"id": scene["id"], "scene_sha256": scene["scene_sha256"], "required_stages": scene["required_stages"],
                   "status": "CONTROLS", "process_order": [], "backends": {b: {"processes": []} for b in BACKENDS}, "untimed_controls": []}
            report["scenes"].append(row)
            for index, item in enumerate([scene, *controls(scene)]):
                stem = scene["id"] + ("-reference" if index == 0 else f'-effect-off-{index-1:02d}')
                entry = {"kind": "production-reference" if index == 0 else "effect-off", "scene_sha256": item["scene_sha256"],
                         "disabled_effects": item.get("disabled_effects", []), "image": "controls/" + stem + ".png",
                         "report": "controls/" + stem + ".json", "log": "logs/" + stem + ".log"}
                row["untimed_controls"].append(entry)
                command = [str(output / binaries["renderer"]["frozen_path"]), str(output / item["frozen_scene"]),
                    "--output", str(output / entry["image"]), "--width", str(args.width), "--height", str(args.height),
                    "--threads", str(args.threads), "--deterministic", "--precision", "native", "--profile-json", str(output / entry["report"])]
                launch(entry, command, stem)
                raw = json.loads((output / entry["report"]).read_text())
                validate_cli_profile(raw, item["required_stages"], item.get("required_not_run_stages", []))
                image = image_details(output / entry["image"], args.width, args.height)
                if not image["nonconstant"]:
                    raise ValueError("Untimed CPU control image is constant")
                data = cpu.pixels(output / entry["image"], (args.width, args.height))
                if index == 0:
                    cpu_reference[scene["id"]] = data
                else:
                    entry["difference_from_enabled"] = rgb_difference(cpu_reference[scene["id"]], data)
                    if not entry["difference_from_enabled"]["different_pixels"]:
                        raise ValueError("Enabled CPU effect produced no changed RGB")
                entry.update(status="PASS", image_identity=image, native=raw, report_sha256=sha(output / entry["report"]))
                write_json(output / "summary.json", report)
        for scene_source, row in zip(selection["scenes"], report["scenes"]):
            scene = dict(scene_source, frozen_path=output / scene_source["frozen_scene"])
            row["status"] = "RUNNING"
            references, metadata = {}, {}
            for index in range(args.cold_repeats):
                for backend in backend_order(index):
                    directory = output / "runs" / f'{scene["id"]}-{backend}-{index:02d}'
                    command = [str(output / binaries[backend]["frozen_path"]), "--scene", str(scene["frozen_path"]), "--output", str(directory), "--precision", "native"]
                    if backend == "cpu": command += ["--resource-root", str(runtime)]
                    else: command += ["--benchmark-render", "--full-shading", "--library", str(output / library["frozen_path"])]
                    for key, value in settings.items(): command += ["--" + key, str(value)]
                    entry = {"backend": backend, "cold_index": index, "directory": str(directory.relative_to(output)),
                        "report": str((directory / "report.json").relative_to(output)), "log": "logs/" + directory.name + ".log"}
                    row["backends"][backend]["processes"].append(entry)
                    row["process_order"].append({"cold_index": index, "backend": backend})
                    launch(entry, command, directory.name)
                    raw = json.loads((directory / "report.json").read_text())
                    if backend == "cpu":
                        rgb, images = validate_cpu_report(raw, directory, settings, scene, runtime)
                        if rgb != cpu_reference[scene["id"]]:
                            raise ValueError("CPU benchmark RGB differs from untimed Native production CLI")
                        binding = {key: raw[key] for key in ("loaded_features", "execution", "architecture")}
                    else:
                        rgb, images = validate_metal_report(raw, directory, output / library["frozen_path"], settings, scene)
                        binding = {key: raw[key] for key in ("numeric_profile", "scene_description", "loaded_features", "library_profile", "execution", "device")}
                    admit_backend_reference(references, backend, rgb)
                    if backend in metadata and metadata[backend] != binding:
                        raise ValueError("Backend prepared metadata changed between processes")
                    metadata[backend] = binding
                    entry.update(status="PASS", native=raw, images=images, exact_rgb_within_process=True,
                                 rgb_sha256=hashlib.sha256(rgb).hexdigest(), report_sha256=sha(directory / "report.json"))
                    write_json(output / "summary.json", report)
            for backend, data in row["backends"].items():
                data.update(summary=summarize_processes(data["processes"], backend), exact_rgb_within_backend=True,
                    rgb_sha256=hashlib.sha256(references[backend]).hexdigest(), metadata=metadata[backend])
            row["cross_backend_rgb"] = rgb_difference(references["cpu"], references["metal"])
            for label, phase, key in (("cold", "cold", "cold_total_ms"), ("warm", "warm", "render_and_readback_ms")):
                numerator, denominator = [row["backends"][b]["summary"][phase][key]["median"] for b in BACKENDS]
                if denominator <= 0:
                    raise ValueError("Zero Metal duration")
                row[label + "_ratio_cpu_over_metal"] = numerator / denominator
            row["status"] = "COMPLETE"
            write_json(output / "summary.json", report)
        report["source_identity_at_end"] = source_identity()
        if report["source_identity_at_end"] != source or sha(args.selection) != report["selection_sha256"]:
            raise ValueError("Source/selection changed during benchmark")
        verify_inputs(); quiet_host(); observe_disk(report, output, "complete")
        ratios = [r["warm_ratio_cpu_over_metal"] for r in report["scenes"]]
        report["aggregate"] = {"warm_speed_ratio_geometric_mean": math.exp(statistics.mean(math.log(r) for r in ratios)),
            "warm_speed_ratio_range": [min(ratios), max(ratios)], "method": "Equal-weight geometric mean of all selected scene ratios"}
        count = len(report["scenes"]) * 2 * args.cold_repeats
        report.update(status="COMPLETE", finished_utc=utc_now(), process_count=count, frame_count=count * (1 + args.warmups + args.repeats),
            cold_frame_count=count, discarded_warmup_frame_count=count * args.warmups, measured_warm_frame_count=count * args.repeats,
            untimed_control_frame_count=sum(len(r["untimed_controls"]) for r in report["scenes"]), gallery=str(gallery))
        write_json(output / "summary.json", report)
        export_gallery(report, output, gallery)
    except (OSError, ValueError, KeyError, TypeError, subprocess.SubprocessError) as error:
        if report is not None:
            report.update(status="FAILED", error=str(error))
        print("Native backend benchmark failed: " + str(error), file=sys.stderr)
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
