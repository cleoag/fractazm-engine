"""Retain a native offline Metal experiment, its numerical limits and images.

This wrapper never enables a production backend or claims M3P/Pascal parity.
The negative double compiler check is offline; no runtime shader JIT is used.
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

from benchmark_gallery_arm64 import (ROOT, binary_identity, execute, native_host,
                                      sha, utc_now, write_json)
from native_evidence import source_identity

SCHEMA = "mb3d.metal-formula-experiment.v1"
NAMES = ("SphereIFS", "boxIFS", "Menger3")
BACKENDS = ("gpu", "cpu_fp32", "cpu_fp64")


def classify_double_compilation(returncode, timed_out, diagnostic):
    """An arbitrary compiler/toolchain failure is not evidence about FP64."""
    if timed_out:
        return "TIMEOUT_NOT_EVIDENCE"
    if returncode == 0:
        return "UNEXPECTED_COMPILE_SUCCESS"
    errors = [line.strip() for line in diagnostic.splitlines() if re.search(r"\b(?:fatal )?error:", line)]
    expected = [line for line in errors if "double" in line.lower()
                and ("not supported" in line.lower() or "unsupported" in line.lower())]
    return "EXPECTED_DOUBLE_UNSUPPORTED" if expected and len(expected) == len(errors) else "UNEXPECTED_FAILURE_NOT_EVIDENCE"


def finite_nonnegative(value, name):
    if isinstance(value, bool) or not isinstance(value, (int, float)) or not math.isfinite(value) or value < 0:
        raise ValueError("Invalid nonnegative number: " + name)
    return value


def checked_count(value, name):
    if isinstance(value, bool) or not isinstance(value, int) or value < 0:
        raise ValueError("Invalid count: " + name)
    return value


def validate_report(value, width, height, threads, repeats):
    if (value.get("schema_version") != 1 or value.get("infrastructure_status") != "PASS"
            or value.get("numeric_profile") != "EXPERIMENTAL_FP32"
            or not value.get("numerical_qualification", "").startswith("NOT_QUALIFIED")):
        raise ValueError("Unexpected probe schema, infrastructure failure or qualification claim")
    if (value.get("host_architecture") != "arm64" or value.get("rosetta_translated") != 0
            or value.get("cpu_compute_fallback_calls") != 0 or value.get("runtime_shader_compilation") is not False):
        raise ValueError("Probe must be native ARM64, offline compiled, with zero CPU fallback")
    scene = value.get("scene_parameters", {})
    if (scene.get("width"), scene.get("height"), value.get("cpu_threads"), value.get("measured_repeats")) != (width, height, threads, repeats):
        raise ValueError("Report settings differ from requested command")
    renders = value.get("renders", [])
    if len(renders) != 3 or {row.get("name") for row in renders} != set(NAMES):
        raise ValueError("Expected exactly the three declared synthetic formula scenes")
    unresolved_primary = unresolved_shadow = 0
    for row in renders:
        if (row.get("width"), row.get("height")) != (width, height) or row.get("gpu_repeats_bit_exact") is not True:
            raise ValueError("Wrong render dimensions or nonidentical GPU repeats")
        for field in ("gpu_timestamp", "warm_encode_submit_wait_readback", "cpu_fp32", "cpu_fp64"):
            samples = row.get(field + "_ms")
            if not isinstance(samples, list) or len(samples) != repeats:
                raise ValueError("Missing timing repetitions: " + field)
            for sample in samples:
                finite_nonnegative(sample, field)
            reported = finite_nonnegative(row.get(field + "_median_ms"), field + " median")
            if not math.isclose(reported, statistics.median(samples), rel_tol=1e-12, abs_tol=1e-12):
                raise ValueError("Incorrect reported median: " + field)
        for backend in BACKENDS:
            counts = row.get(backend, {})
            primary = checked_count(counts.get("primary_step_limit_pixels"), backend + " primary")
            shadow = checked_count(counts.get("shadow_step_limit_pixels"), backend + " shadow")
            expected = "BOUNDED_WITH_UNRESOLVED_PIXELS" if primary or shadow else "COMPLETE"
            if counts.get("completion") != expected or max(primary, shadow) > width * height:
                raise ValueError("Unresolved pixels must remain explicitly incomplete")
            if checked_count(counts.get("hits"), "hits") + checked_count(counts.get("misses"), "misses") != width * height:
                raise ValueError("Hit/miss counts disagree with image dimensions")
            if counts.get("nonconstant") is not True:
                raise ValueError("Synthetic image is constant")
            if backend == "gpu":
                unresolved_primary += primary
                unresolved_shadow += shadow
        for backend, suffix in (("gpu", "gpu"), ("cpu_fp32", "cpu-fp32"), ("cpu_fp64", "cpu-fp64")):
            if row.get(backend + "_image") != f"{row['name']}-{suffix}.ppm":
                raise ValueError("Unexpected image path")
        if row.get("gpu_pixel_buffer") != row["name"] + "-gpu-pixels.bin":
            raise ValueError("Unexpected raw buffer path")
    if (value.get("gpu_unresolved_primary_pixels"), value.get("gpu_unresolved_shadow_pixels")) != (unresolved_primary, unresolved_shadow):
        raise ValueError("Unresolved pixel totals disagree")
    fixture = value.get("original_formula_fixture", {})
    cohorts = fixture.get("cohorts", [])
    names = ("finite_original_in_x87_range", "x87_fsincos_out_of_range", "fp32_input_overflow")
    if (len(cohorts) != 3 or tuple(row.get("name") for row in cohorts) != names
            or fixture.get("primary_finite_original_in_range") != cohorts[0]):
        raise ValueError("Original fixture must preserve finite and unsupported input cohorts")
    rows = sum(checked_count(row.get("rows"), "cohort rows") for row in cohorts)
    failures = sum(checked_count(row.get("gpu_failed_rows_at_existing_gate"), "cohort failed rows") for row in cohorts)
    if rows != fixture.get("rows") or failures != fixture.get("gpu_failed_rows_at_existing_gate"):
        raise ValueError("Fixture cohort totals disagree")
    for cohort in cohorts:
        if cohort["gpu_failed_rows_at_existing_gate"] > cohort["rows"]:
            raise ValueError("Fixture failed-row count exceeds cohort")
        for key in ("gpu_vs_original_x86", "gpu_vs_cpu_fp32", "cpu_fp64_libm_vs_original_x86"):
            stats = cohort.get(key, {})
            if stats.get("values") != cohort["rows"] * fixture.get("lanes_per_row", 0):
                raise ValueError("Fixture cohort value denominator disagrees")
            if checked_count(stats.get("strict_gate_failures"), "strict failures") > stats["values"]:
                raise ValueError("Fixture failure count exceeds values")
    return value


def convert_ppm_losslessly(source, destination, width, height):
    with Image.open(source) as image:
        if image.format != "PPM" or image.mode != "RGB" or image.size != (width, height):
            raise ValueError("Expected RGB8 PPM with requested dimensions")
        original = image.tobytes()
        with destination.open("xb") as output:
            image.save(output, format="PNG")
    with Image.open(destination) as decoded:
        if decoded.mode != "RGB" or decoded.size != (width, height) or decoded.tobytes() != original:
            raise ValueError("PNG conversion changed decoded RGB")
    return {"ppm_sha256": sha(source), "png_sha256": sha(destination),
            "rgb_sha256": hashlib.sha256(original).hexdigest(),
            "rgb_hash_layout": "row-major top-to-bottom interleaved RGB8",
            "width": width, "height": height, "lossless_rgb_conversion": True}


def build_html(report, records):
    escape = lambda value: html.escape(str(value), quote=True)
    lookup = {(row["scene"], row["backend"]): row for row in records}
    sections = []
    for row in report["renders"]:
        name = row["name"]
        cards = []
        for backend, label in (("gpu", "Metal GPU · FP32"), ("cpu_fp32", "CPU · FP32"), ("cpu_fp64", "CPU · FP64")):
            image = lookup[name, backend]
            counts = row[backend]
            incomplete = counts["primary_step_limit_pixels"] + counts["shadow_step_limit_pixels"]
            status = (f"Лимит шагов: {counts['primary_step_limit_pixels']} лучей, {counts['shadow_step_limit_pixels']} теней; результат неполный."
                      if incomplete else "Все лучи завершены в пределах заданного лимита.")
            cards.append(f'<figure><a href="{escape(image["png"])}"><img loading="lazy" src="{escape(image["png"])}" alt="{escape(name + " " + label)}"></a><figcaption><strong>{label}</strong><p>{status}</p><a href="{escape(image["png"])}" download>PNG</a> · <a href="{escape(image["ppm"])}">исходный PPM</a></figcaption></figure>')
        fp32 = row["gpu_vs_cpu_fp32"]
        fp64 = row["gpu_vs_cpu_fp64_rounded_output"]
        sections.append(f'''<section><h2>{escape(name)}</h2><div class="images">{''.join(cards)}</div>
<table><thead><tr><th>Измерение</th><th>Медиана</th></tr></thead><tbody>
<tr><td>GPU: выполнение команды</td><td>{row['gpu_timestamp_median_ms']:.3f} мс</td></tr>
<tr><td>GPU: отправка, ожидание, копирование полного буфера</td><td>{row['warm_encode_submit_wait_readback_median_ms']:.3f} мс</td></tr>
<tr><td>CPU FP32, {report['cpu_threads']} потоков</td><td>{row['cpu_fp32_median_ms']:.3f} мс</td></tr>
<tr><td>CPU FP64, {report['cpu_threads']} потоков</td><td>{row['cpu_fp64_median_ms']:.3f} мс</td></tr></tbody></table>
<p>Различающиеся RGB8-пиксели GPU ↔ CPU FP32: <strong>{fp32['different_rgb8_pixels']}</strong>; GPU ↔ CPU FP64: <strong>{fp64['different_rgb8_pixels']}</strong>. Максимальная разница канала: {fp32['max_rgb8_channel_delta']} / {fp64['max_rgb8_channel_delta']}.</p>
<p>Различия попаданий: {fp32['hit_disagreements']} / {fp64['hit_disagreements']}; шагов лучей: {fp32['ray_step_disagreements']} / {fp64['ray_step_disagreements']}.</p></section>''')
    fixture = report["original_formula_fixture"]
    finite = fixture["primary_finite_original_in_range"]
    groups = {row["name"]: row for row in fixture["cohorts"]}
    return f'''<!doctype html><html lang="ru"><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1">
<title>Metal: эксперимент с формулами</title><style>
body{{margin:0;background:#101318;color:#e4e8ef;font:16px/1.5 system-ui,sans-serif}}main{{max-width:1500px;margin:auto;padding:32px}}h1{{font-size:32px}}h2{{margin-top:0}}a{{color:#92caff}}p{{max-width:1100px}}section{{background:#1a2029;border:1px solid #344050;border-radius:14px;padding:20px;margin:24px 0}}.images{{display:grid;grid-template-columns:repeat(3,minmax(0,1fr));gap:16px}}figure{{margin:0}}img{{width:100%;height:auto;display:block;background:black}}figcaption{{padding:12px 0}}figcaption p{{font-size:13px;color:#bfc9d9}}table{{border-collapse:collapse;width:100%;max-width:800px}}th,td{{text-align:left;padding:7px 12px;border-bottom:1px solid #344050}}.notice{{padding:16px;border-left:4px solid #e6aa57;background:#28231d}}@media(max-width:850px){{.images{{grid-template-columns:1fr}}main{{padding:16px}}}}
</style><main><h1>Metal: формулы и полный вычислительный кадр</h1>
<p><strong>{escape(report['device'])}</strong> · {report['scene_parameters']['width']}×{report['scene_parameters']['height']} · три синтетические сцены · {report['measured_repeats']} измерения после одного прогрева.</p>
<p class="notice"><strong>EXPERIMENTAL_FP32 — численная совместимость не подтверждена.</strong> Это отдельный прототип. Он не открывает сцены M3P и не заменяет основной CPU-рендерер. Сравнение с оригинальными изображениями Pascal не проводилось. Различия не скрываются и допуск 2e-12 не ослаблен.</p>
<p>На GPU считаются лучи, формулы и расстояния, нормали, жёсткие тени, пять выборок AO и цвет. Карты, гибриды, отражения, прозрачность, объём и DOF здесь не реализованы. CPU FP32/FP64 используют общий с прототипом алгоритм; это диагностические сравнения, не независимый эталон. Выход CPU FP64 округлён в общий FP32-буфер.</p>
<p>Оригинальные boxIFS-фикстуры с конечными результатами и входами внутри поддерживаемого диапазона: <strong>{finite['rows']} строк × {fixture['lanes_per_row']} значений</strong>; нарушений прежнего допуска: <strong>{finite['gpu_vs_original_x86']['strict_gate_failures']}</strong>, затронутых строк: <strong>{finite['gpu_failed_rows_at_existing_gate']}</strong>.</p>
<p>Отдельно сохранены неподдержанные случаи: {groups['x87_fsincos_out_of_range']['rows']} строк с выходом за диапазон x87 FSINCOS и {groups['fp32_input_overflow']['rows']} строк с переполнением при переходе к FP32. Эти группы не пересекаются. Весь набор: {fixture['rows']} строк, {fixture['gpu_vs_original_x86']['strict_gate_failures']} нарушений допуска. Их нельзя целиком объяснять обычным округлением FP32; статус PASS им не присваивается.</p>
<p>Медианы GPU включают отдельные границы выполнения и чтения; CPU включает создание и завершение потоков. Запись картинок исключена из этих медиан. Они не означают ускорение основного CPU-рендерера. Все PNG побитово сохраняют RGB исходных PPM.</p>
<p><a href="probe/report.json">Численный отчёт и все замеры</a> · <a href="summary.json">Проверка артефактов и происхождение</a> · <a href="evidence/source/comparison/metal_probe/provenance.json">Контракт прототипа</a> · <a href="evidence/double-compile.log">Проверка double в Metal</a></p>
{''.join(sections)}</main></html>'''


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--probe", type=Path, required=True)
    parser.add_argument("--library", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--width", type=int, default=1024)
    parser.add_argument("--height", type=int, default=768)
    parser.add_argument("--threads", type=int, default=6)
    parser.add_argument("--repeats", type=int, default=3)
    parser.add_argument("--timeout", type=float, default=900)
    parser.add_argument("--build-evidence", type=Path, action="append", default=[], help="Existing configure/build log to retain; repeatable")
    args = parser.parse_args(argv)
    if (not 16 <= args.width <= 4096 or not 16 <= args.height <= 4096 or not 1 <= args.threads <= 64
            or not 3 <= args.repeats <= 15 or not math.isfinite(args.timeout) or args.timeout <= 0):
        parser.error("Dimensions 16..4096, threads 1..64, repeats 3..15 and positive finite timeout required")
    try:
        host = native_host()
        probe = args.probe.expanduser().resolve(strict=True)
        library = args.library.expanduser().resolve(strict=True)
        identity = binary_identity(probe)
        build_evidence = [p.expanduser().resolve(strict=True) for p in args.build_evidence]
        if library.suffix != ".metallib" or not library.is_file():
            raise ValueError("A precompiled .metallib is required")
        output = args.output.expanduser().resolve()
        output.mkdir(parents=True, exist_ok=False)
    except (ValueError, OSError, subprocess.CalledProcessError) as error:
        parser.error(str(error))
    for name in ("evidence/bin", "evidence/source", "evidence/build", "images"):
        (output / name).mkdir(parents=True)
    (output / "KEEP_FINAL_IMAGES.txt").write_text("Retained experimental rendering images/evidence. Routine cleanup must not remove this directory. Explicit user deletion request required.\n")
    summary = {"schema": SCHEMA, "status": "RUNNING", "started_utc": utc_now(), "host": host,
               "scope": "Experimental native Metal synthetic formula rendering and diagnostics; no production M3P support or Pascal image parity",
               "numeric_profile": "EXPERIMENTAL_FP32", "numerical_qualification": "NOT_QUALIFIED",
               "settings": {"width": args.width, "height": args.height, "threads": args.threads,
                            "repeats": args.repeats, "timeout_seconds": args.timeout},
               "probe": identity, "library": {"path": str(library), "sha256": sha(library)},
               "source_observation": source_identity(),
               "source_identity_note": "Source snapshot and captured build files are provenance; an uncommitted checkout observation alone is not an attestation that a preexisting binary was built from it.",
               "artifacts": {}, "images": []}
    write_json(output / "summary.json", summary)

    def retain(source, relative):
        target = output / relative
        target.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(source, target)
        if sha(source) != sha(target):
            raise ValueError("File changed while snapshotting: " + str(source))
        summary["artifacts"][relative] = sha(target)
        return target

    try:
        retained_probe = retain(probe, "evidence/bin/metal_formula_probe")
        retained_library = retain(library, "evidence/bin/metal_formula_probe.metallib")
        if sha(retained_probe) != identity["sha256"] or sha(retained_library) != summary["library"]["sha256"]:
            raise ValueError("Binary/library changed after identity capture")
        provenance = json.loads((ROOT / "comparison/metal_probe/provenance.json").read_text())
        sources = {"CMakeLists.txt", "comparison/tools/metal_formula_probe.mm",
                   "comparison/tools/run_metal_formula_probe.py", "comparison/tools/benchmark_gallery_arm64.py",
                   "scripts/native_evidence.py", "tests/fixtures/boxifs_twist_regressions.h"}
        sources.update(str(p.relative_to(ROOT)) for p in (ROOT / "comparison/metal_probe").iterdir() if p.is_file())
        for formula in provenance["formulas"]:
            sources.update((formula["translated_source"].split(":", 1)[0], formula["original_source"]))
        sources.update(name for name, digest in summary["source_observation"]["changed_source_sha256"].items() if digest is not None)
        for name in sorted(sources):
            retain(ROOT / name, "evidence/source/" + name)
        patch = output / "evidence/source/working-tree.patch"
        patch.write_bytes(subprocess.check_output(["git", "diff", "HEAD", "--binary"], cwd=ROOT))
        summary["artifacts"][str(patch.relative_to(output))] = sha(patch)
        for relative in ("CMakeCache.txt", "compile_commands.json", "build.ninja",
                         "CMakeFiles/mb3d_metal_formula_probe.dir/flags.make", "CMakeFiles/mb3d_metal_formula_probe.dir/link.txt",
                         "CMakeFiles/mb3d_metal_formula_probe.dir/build.make", "CMakeFiles/mb3d_metal_formula_shaders.dir/build.make"):
            if (probe.parent / relative).is_file():
                retain(probe.parent / relative, "evidence/build/" + relative)
        for index, path in enumerate(build_evidence):
            retain(path, f"evidence/build/user-{index:02}-" + path.name)
        env = {k: v for k, v in os.environ.items() if not k.upper().startswith("MB3D_")}
        summary["removed_environment_names"] = sorted(k for k in os.environ if k.upper().startswith("MB3D_"))

        # A separate offline compile establishes the installed compiler's actual
        # rejection reason. Toolchain failure remains a failure, not FP64 proof.
        double_command = ["xcrun", "-sdk", "macosx", "metal", "-std=metal3.1", "-fno-fast-math", "-ffp-contract=off",
                          "-c", str(output / "evidence/source/comparison/metal_probe/unsupported_double.metal"),
                          "-o", str(output / "evidence/unsupported_double.air")]
        summary["double_compile"] = {"command": double_command, "status": "RUNNING"}
        try:
            compiler = Path(subprocess.check_output(["xcrun", "-sdk", "macosx", "--find", "metal"], text=True).strip())
            arch = subprocess.check_output(["lipo", "-archs", str(compiler)], text=True).strip()
            if not any(part.startswith("arm64") for part in arch.split()):
                raise ValueError("Offline compiler has no native ARM64 slice")
            summary["double_compile"]["compiler"] = {"path": str(compiler), "sha256": sha(compiler), "architectures": arch}
            write_json(output / "summary.json", summary)
            process = execute(double_command, output, output / "evidence/double-compile.log", 120, env)
            diagnostic = (output / "evidence/double-compile.log").read_text(errors="replace")
            summary["double_compile"].update(process, status=classify_double_compilation(process["returncode"], process["timed_out"], diagnostic))
        except (ValueError, OSError, subprocess.CalledProcessError) as error:
            summary["double_compile"].update(status="UNAVAILABLE_NOT_EVIDENCE", error=str(error))
            if not (output / "evidence/double-compile.log").exists():
                (output / "evidence/double-compile.log").write_text(str(error) + "\n")

        command = [str(retained_probe), "--library", str(retained_library), "--output", str(output / "probe"),
                   "--width", str(args.width), "--height", str(args.height), "--threads", str(args.threads), "--repeats", str(args.repeats)]
        summary["command"] = command
        summary["cwd"] = str(output)
        write_json(output / "summary.json", summary)
        print("Native Metal experiment: " + str(output), flush=True)
        summary["process"] = execute(command, output, output / "evidence/probe.log", args.timeout, env)
        if summary["process"]["timed_out"] or summary["process"]["returncode"] != 0:
            raise ValueError("Probe timed out or failed; inspect evidence/probe.log")
        report = validate_report(json.loads((output / "probe/report.json").read_text()), args.width, args.height, args.threads, args.repeats)
        for row in report["renders"]:
            raw = output / "probe" / row["gpu_pixel_buffer"]
            if raw.stat().st_size != args.width * args.height * 48:
                raise ValueError("GPU raw pixel buffer size does not match its ABI")
            for backend in BACKENDS:
                source = output / "probe" / row[backend + "_image"]
                destination = output / "images" / (source.stem + ".png")
                image = convert_ppm_losslessly(source, destination, args.width, args.height)
                summary["images"].append({"scene": row["name"], "backend": backend,
                                          "ppm": str(source.relative_to(output)), "png": str(destination.relative_to(output)), **image})
        summary["gpu_completion"] = ("BOUNDED_WITH_UNRESOLVED_PIXELS" if report["gpu_unresolved_primary_pixels"] or report["gpu_unresolved_shadow_pixels"] else "COMPLETE_WITHIN_PROBE_LIMITS")
        summary["gpu_unresolved_primary_pixels"] = report["gpu_unresolved_primary_pixels"]
        summary["gpu_unresolved_shadow_pixels"] = report["gpu_unresolved_shadow_pixels"]
        summary["probe_report"] = "probe/report.json"
        (output / "index.html").write_text(build_html(report, summary["images"]))
        summary["status"] = ("COMPLETE_DIAGNOSTIC_NOT_NUMERICALLY_QUALIFIED"
                             if summary["double_compile"]["status"] == "EXPECTED_DOUBLE_UNSUPPORTED" else "FAILED_DOUBLE_DIAGNOSTIC")
    except KeyboardInterrupt:
        summary["status"] = "INTERRUPTED"
    except Exception as error:
        summary.update(status="FAILED", error=str(error))
    summary["finished_utc"] = utc_now()
    summary["source_observation_at_end"] = source_identity()
    for path in sorted(output.rglob("*")):
        if path.is_file() and path.name not in ("summary.json", "summary.json.tmp"):
            summary["artifacts"][str(path.relative_to(output))] = sha(path)
    write_json(output / "summary.json", summary)
    print(json.dumps({"status": summary["status"], "summary": str(output / "summary.json"), "images": len(summary["images"])}))
    return 0 if summary["status"] == "COMPLETE_DIAGNOSTIC_NOT_NUMERICALLY_QUALIFIED" else 1


if __name__ == "__main__":
    raise SystemExit(main())
