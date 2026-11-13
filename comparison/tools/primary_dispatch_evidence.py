"""Exact same-kernel primary dispatch validation shared by both scene scopes."""
from __future__ import annotations

import html
import math
from pathlib import Path
import statistics
import struct

PIXEL_BYTES = 176
BASELINE_FILE = "gpu-baseline-primary.bin"
BASELINE_GATE = "all 176 bytes exact (same-kernel eight-row GPU baseline)"
NORMAL_BASELINE_FILE = "gpu-inline-primary.bin"
NORMAL_BASELINE_GATE = "all 176 bytes exact (inline GPU normal baseline)"
NORMAL_STAGES = ("geometry", "probes", "finish")


def count(value, label):
    if type(value) is not int or value < 0:
        raise ValueError("Invalid primary dispatch count: " + label)
    return value


def positive_time(value, label):
    if (isinstance(value, bool) or not isinstance(value, (int, float))
            or not math.isfinite(value) or value <= 0):
        raise ValueError("Missing positive primary dispatch timing: " + label)
    return value


def boolean(value, label):
    if type(value) is not bool:
        raise ValueError("Invalid primary dispatch Boolean: " + label)
    return value


def complete_maximum(samples, first):
    if not isinstance(samples, list) or not samples:
        raise ValueError("Missing complete primary timing samples")
    return max(positive_time(first, "first primary frame"),
               *(positive_time(value, "primary frame") for value in samples))


def validate_normal_controls(section):
    if not isinstance(section, dict):
        raise ValueError("Missing normal GPU controls")
    abi = section.get("abi", {})
    expected = [64, 0, 48, 56, 6, 176]
    values = abi.get("values")
    if (count(abi.get("checks"), "normal ABI checks") != 6
            or not isinstance(values, list) or len(values) != 6):
        raise ValueError("Normal ABI coverage changed")
    abi_failures = sum(count(value, "normal ABI value") != want for value, want in zip(values, expected))
    if (count(abi.get("failures"), "normal ABI failures") != abi_failures
            or boolean(abi.get("pass"), "normal ABI pass") != (abi_failures == 0)):
        raise ValueError("Normal ABI values disagree with pass")
    negative = section.get("negative_controls", {})
    failed = count(negative.get("failures"), "normal negative failures")
    if (count(negative.get("cases"), "normal negative cases") != 8
            or count(negative.get("checks"), "normal negative checks") != 240
            or failed > 240 or boolean(negative.get("pass"), "normal negative pass") != (failed == 0)):
        raise ValueError("Normal malformed-descriptor coverage or pass changed")
    synthetic = section.get("synthetic", {})
    if (count(synthetic.get("pixels"), "normal synthetic pixels") != 15
            or count(synthetic.get("active_pixels"), "normal active pixels") != 5
            or count(synthetic.get("inactive_pixels"), "normal inactive pixels") != 10):
        raise ValueError("Normal synthetic activity coverage changed")
    different = count(synthetic.get("different_pixels"), "normal synthetic differences")
    inactive = count(synthetic.get("inactive_sample_failures"), "normal inactive sample failures")
    guards = count(synthetic.get("guard_failures"), "normal guard failures")
    synthetic_failures = different + inactive + guards
    if (different > 15 or inactive > 10 or guards > 9*(176+64)
            or boolean(synthetic.get("pass"), "normal synthetic pass") != (synthetic_failures == 0)):
        raise ValueError("Normal synthetic or guard failures disagree with pass")
    failures = abi_failures + failed + synthetic_failures
    if boolean(section.get("pass"), "normal controls pass") != (failures == 0):
        raise ValueError("Normal controls pass disagrees with checks")
    return failures


def validate_normal_pipeline(section, width, height, repeats, mode, threads, samples, first):
    """Check the split path without changing the full primary timing denominator."""
    if not isinstance(section, dict):
        raise ValueError("Missing normal pipeline evidence")
    pixels = width * height
    if (mode not in ("inline", "parallel") or section.get("mode") != mode
            or count(section.get("pixels"), "normal pixels") != pixels
            or count(section.get("scratch_record_bytes"), "normal scratch record") != 64
            or count(section.get("probe_jobs"), "normal probe jobs") != (pixels * 6 if mode == "parallel" else 0)
            or count(section.get("threads_per_threadgroup"), "normal threads") != threads
            or not 1 <= count(threads, "requested normal threads") <= 1024):
        raise ValueError("Normal execution settings or scratch ABI differ from requested command")
    limits = section.get("pipeline_limits")
    if not isinstance(limits, dict) or set(limits) != {"probes", "finish"}:
        raise ValueError("Missing normal pipeline GPU limits")
    for name, pipeline in limits.items():
        if not isinstance(pipeline, dict):
            raise ValueError("Invalid normal pipeline GPU limits")
        simd = count(pipeline.get("thread_execution_width"), name + " normal SIMD width")
        maximum = count(pipeline.get("max_total_threads_per_threadgroup"), name + " normal group limit")
        if not simd or not maximum or threads % simd or threads > maximum:
            raise ValueError("Normal launch exceeds reported GPU limits")
    baseline = section.get("baseline", {})
    if (count(baseline.get("pixels"), "inline baseline pixels") != pixels
            or baseline.get("gate") != NORMAL_BASELINE_GATE):
        raise ValueError("Inline normal baseline coverage or exact gate changed")
    failures = validate_normal_controls(section.get("controls"))
    for key in ("different_pixels", "fault_pixels"):
        value = count(baseline.get(key), "inline normal baseline " + key)
        if value > pixels:
            raise ValueError("Inline normal baseline failures exceed pixel count")
        failures += value
    for field in ("gpu_ms", "median_gpu_ms", "first_gpu_ms"):
        if not isinstance(section.get(field), dict) or set(section[field]) != set(NORMAL_STAGES):
            raise ValueError("Missing normal pipeline timing denominators")
    if not isinstance(samples, list) or len(samples) != repeats:
        raise ValueError("Missing aggregate primary timing repetitions")
    for name in NORMAL_STAGES:
        values = section["gpu_ms"][name]
        if not isinstance(values, list) or len(values) != repeats:
            raise ValueError("Missing normal pipeline timing repetitions")
        measured = [section["first_gpu_ms"][name], *values]
        if any(isinstance(v, bool) or not isinstance(v, (int, float))
               or not math.isfinite(v) or v < 0 for v in measured):
            raise ValueError("Invalid normal pipeline timing sample")
        median = section["median_gpu_ms"][name]
        if (isinstance(median, bool) or not isinstance(median, (int, float))
                or not math.isfinite(median)
                or not math.isclose(median, statistics.median(values), rel_tol=1e-12, abs_tol=1e-12)):
            raise ValueError("Incorrect normal pipeline timing median")
        if mode == "inline" and name != "geometry":
            if any(measured):
                raise ValueError("Inline normals must exclude separate normal timings")
        elif any(v <= 0 for v in measured):
            raise ValueError("Selected normal pipeline must record actual GPU work")
    for index, total in enumerate([first, *samples]):
        actual = sum(section["first_gpu_ms"][name] if index == 0 else section["gpu_ms"][name][index - 1]
                     for name in NORMAL_STAGES)
        positive_time(total, "aggregate primary frame")
        if not math.isclose(actual, total, rel_tol=1e-12, abs_tol=1e-9):
            raise ValueError("Normal pipeline times disagree with complete primary timing")
    if boolean(section.get("pass"), "normal pipeline pass") != (failures == 0):
        raise ValueError("Normal pipeline pass disagrees with exact checks")
    return failures


def normal_geometry_maximum(section):
    return complete_maximum(section["gpu_ms"]["geometry"], section["first_gpu_ms"]["geometry"])


def validate_normal_buffers(folder: Path, section, width, height):
    raw = []
    for name in ("gpu-primary.bin", NORMAL_BASELINE_FILE):
        path = folder / name
        if not path.is_file() or path.stat().st_size != width * height * PIXEL_BYTES:
            raise ValueError("Raw inline normal baseline size differs from declared ABI: " + name)
        raw.append(path.read_bytes())
    selected, baseline = raw
    different = sum(selected[i:i+PIXEL_BYTES] != baseline[i:i+PIXEL_BYTES]
                    for i in range(0, len(selected), PIXEL_BYTES))
    faults = sum(bool(struct.unpack_from("<I", selected, i+152)[0]
                      or struct.unpack_from("<I", baseline, i+152)[0])
                 for i in range(0, len(selected), PIXEL_BYTES))
    expected = section["baseline"]
    if (different, faults) != (expected["different_pixels"], expected["fault_pixels"]):
        raise ValueError("Saved inline normal baseline bytes disagree with report")


def normal_gallery_html(section, division_mode):
    mode = html.escape(str(section["mode"]), quote=True)
    division = html.escape(str(division_mode), quote=True)
    labels = {"geometry": "Обход и уточнение", "probes": "Параллельные пробы нормали",
              "finish": "Упаковка нормали"}
    rows = "".join(f'<tr><td>{labels[name]}</td><td>{section["median_gpu_ms"][name]:.3f} мс</td></tr>'
                   for name in NORMAL_STAGES)
    baseline = section["baseline"]
    return (f'<p>Нормали: {mode}, {section["threads_per_threadgroup"]} потоков в группе. '
            f'Точное деление: {division}. Сравнение с исходным проходом inline: '
            f'{baseline["pixels"]} записей по 176 байт; различий {baseline["different_pixels"]}, '
            f'ошибок {baseline["fault_pixels"]}.</p>'
            '<p>Эти части уже включены во время первичной геометрии; повторно к кадру не прибавляются.</p>'
            '<table><tr><th>Часть первичного прохода</th><th>Медиана GPU</th></tr>' + rows + '</table>')


def configuration_shapes(height, selected_rows, simd, maximum):
    candidates = [(rows, 32, 32) for rows in (8, 32, 128, 256, height)]
    candidates += [(selected_rows, group, group) for group in (64, 128, 256)]
    candidates += [(selected_rows, 32, tile) for tile in (16, 8, 4)]
    return list(dict.fromkeys(shape for shape in candidates if shape[1] % simd == 0 and shape[1] <= maximum))


def validate_shape(section, threads, expected_width, label):
    tile_width = count(section.get("tile_width"), label + " tile width")
    tile_height = count(section.get("tile_height"), label + " tile height")
    if (not tile_width or not tile_height or tile_width * tile_height != threads
            or tile_width != expected_width):
        raise ValueError("Primary dispatch tile shape differs from " + label)


def validate(section, width, height, rows, threads, sweep, complete_max_ms, tile_width=8):
    """Return failures, retaining failed evidence while rejecting false PASS."""
    if not isinstance(section, dict):
        raise ValueError("Missing primary dispatch evidence")
    if (not 1 <= count(rows, "requested rows") <= 4096
            or not 1 <= count(threads, "requested threads") <= 1024
            or count(section.get("rows_per_command"), "rows") != rows
            or count(section.get("threads_per_threadgroup"), "threads") != threads):
        raise ValueError("Primary dispatch settings differ from requested command")
    requested_tile = count(tile_width, "requested tile width") or threads
    if requested_tile > threads or threads % requested_tile:
        raise ValueError("Primary dispatch requested tile width must divide its thread count")
    validate_shape(section, threads, requested_tile, "requested command")
    if (count(section.get("commands_per_frame"), "commands") != (height + rows - 1) // rows
            or count(section.get("grid_width"), "grid width") != width
            or count(section.get("grid_height"), "grid height") != min(rows, height)):
        raise ValueError("Primary dispatch grid or partial-row coverage is invalid")
    simd = count(section.get("thread_execution_width"), "SIMD width")
    maximum = count(section.get("max_total_threads_per_threadgroup"), "group limit")
    if not simd or maximum < 32 or 32 % simd or threads % simd or threads > maximum:
        raise ValueError("Primary dispatch exceeds compiled pipeline limits")
    command_ms = positive_time(section.get("max_command_gpu_ms"), "command maximum")
    total_ms = positive_time(complete_max_ms, "complete primary maximum")
    if command_ms > total_ms + max(1e-9, total_ms * 1e-12):
        raise ValueError("Primary command maximum exceeds complete primary timing")
    if command_ms + max(1e-9, total_ms * 1e-12) < total_ms / section["commands_per_frame"]:
        raise ValueError("Primary command maximum is below complete primary timing per command")
    baseline = section.get("baseline", {})
    pixels = width * height
    if (count(baseline.get("rows_per_command"), "baseline rows") != 8
            or count(baseline.get("threads_per_threadgroup"), "baseline threads") != 32
            or count(baseline.get("pixels"), "baseline pixels") != pixels
            or baseline.get("gate") != BASELINE_GATE):
        raise ValueError("Primary baseline coverage or exact acceptance gate changed")
    validate_shape(baseline, 32, 32, "baseline")
    failures = 0
    for key in ("different_pixels", "fault_pixels"):
        value = count(baseline.get(key), "baseline " + key)
        if value > pixels:
            raise ValueError("Primary baseline failures exceed checked pixels")
        failures += value
    negative = section.get("negative_controls", {})
    rejected = count(negative.get("failures"), "negative control failures")
    if (count(negative.get("cases"), "negative control cases") != 4
            or count(negative.get("checks"), "negative control checks") != 4 * pixels
            or rejected > 4 * pixels
            or boolean(negative.get("pass"), "negative controls") != (rejected == 0)):
        raise ValueError("Primary dispatch negative control coverage is incomplete")
    failures += rejected
    if sweep:
        failures += validate_trials(section.get("trials"), pixels, height, rows, simd, maximum)
    elif section.get("trials") is not None:
        raise ValueError("Unrequested primary dispatch trials in report")
    if boolean(section.get("pass"), "pass") != (failures == 0):
        raise ValueError("Primary dispatch pass flag disagrees with exact checks")
    return failures


def validate_trials(trials, pixels, height, selected_rows, simd, maximum):
    if not isinstance(trials, dict):
        raise ValueError("Requested primary dispatch trials are missing")
    if (count(trials.get("records_per_run"), "trial records per run") != pixels
            or count(trials.get("warmup_runs"), "trial warmup runs") != 1
            or count(trials.get("timed_runs"), "trial timed runs") != 3):
        raise ValueError("Primary dispatch trial run coverage is incomplete")
    configurations = trials.get("configurations")
    if not isinstance(configurations, list) or any(not isinstance(row, dict) for row in configurations):
        raise ValueError("Invalid primary dispatch trial configurations")
    observed = [(count(row.get("rows_per_command"), "trial rows"),
                 count(row.get("threads_per_threadgroup"), "trial threads"),
                 count(row.get("tile_width"), "trial tile width")) for row in configurations]
    if observed != configuration_shapes(height, selected_rows, simd, maximum):
        raise ValueError("Primary dispatch trial configurations missing, duplicated or unsupported")
    failures = 0
    for row in configurations:
        validate_shape(row, row["threads_per_threadgroup"], row["tile_width"], "trial configuration")
        if (count(row.get("checked_records"), "trial checked records") != 4 * pixels
                or count(row.get("record_bytes"), "trial record bytes") != PIXEL_BYTES):
            raise ValueError("Primary dispatch trial exact-record coverage is incomplete")
        for key in ("different_records", "fault_records"):
            value = count(row.get(key), "trial " + key)
            if value > 4 * pixels:
                raise ValueError("Primary dispatch trial failures exceed checked records")
            failures += value
        samples = row.get("gpu_ms")
        if not isinstance(samples, list) or len(samples) != 3:
            raise ValueError("Missing primary dispatch trial timing repetitions")
        for value in samples:
            positive_time(value, "trial sample")
        median = positive_time(row.get("median_gpu_ms"), "trial median")
        if not math.isclose(median, statistics.median(samples), rel_tol=1e-12, abs_tol=1e-12):
            raise ValueError("Incorrect primary dispatch trial median")
        warmup = positive_time(row.get("warmup_gpu_ms"), "trial warmup")
        command = positive_time(row.get("max_command_gpu_ms"), "trial maximum")
        maximum_time = max(warmup, *samples)
        if command > maximum_time + max(1e-9, maximum_time * 1e-12):
            raise ValueError("Primary trial command maximum exceeds complete run")
        commands = (height + row["rows_per_command"] - 1) // row["rows_per_command"]
        if command + max(1e-9, maximum_time * 1e-12) < maximum_time / commands:
            raise ValueError("Primary trial command maximum is below complete run timing per command")
    # Trial failures are included in the enclosing dispatch pass flag. The
    # driver need not duplicate that flag on the nested trials collection.
    return failures


def validate_buffers(folder: Path, section, width, height):
    raw = []
    for name in ("gpu-primary.bin", BASELINE_FILE):
        path = folder / name
        if not path.is_file() or path.stat().st_size != width * height * PIXEL_BYTES:
            raise ValueError("Raw primary baseline buffer size differs from declared ABI: " + name)
        raw.append(path.read_bytes())
    selected, baseline = raw
    different = sum(selected[i:i+PIXEL_BYTES] != baseline[i:i+PIXEL_BYTES]
                    for i in range(0, len(selected), PIXEL_BYTES))
    faults = sum(bool(struct.unpack_from("<I", selected, i+152)[0]
                      or struct.unpack_from("<I", baseline, i+152)[0])
                 for i in range(0, len(selected), PIXEL_BYTES))
    expected = section["baseline"]
    if (different, faults) != (expected["different_pixels"], expected["fault_pixels"]):
        raise ValueError("Saved primary baseline bytes disagree with report")


def gallery_html(section):
    esc = lambda value: html.escape(str(value), quote=True)
    baseline = section["baseline"]
    text = (f'<p>Первичная геометрия: {section["rows_per_command"]} строк в команде, '
            f'{section["threads_per_threadgroup"]} потоков в группе '
            f'({section["tile_width"]}×{section["tile_height"]}); '
            f'{section["commands_per_frame"]} команд на кадр. Прежний запуск по 8 строк: '
            f'{baseline["pixels"]} записей по 176 байт, различаются {baseline["different_pixels"]}, '
            f'ошибок вычисления {baseline["fault_pixels"]}. '
            f'Проверки размеров запуска: {section["negative_controls"]["checks"]}, '
            f'ошибок {section["negative_controls"]["failures"]}.</p>')
    if section.get("trials") is not None:
        rows = "".join(
            f'<tr><td>{esc(row["rows_per_command"])}</td><td>{esc(row["threads_per_threadgroup"])}</td>'
            f'<td>{esc(row["tile_width"])}×{esc(row["tile_height"])}</td>'
            f'<td>{row["median_gpu_ms"]:.3f} мс</td><td>{row["different_records"]} / {row["fault_records"]}</td></tr>'
            for row in section["trials"]["configurations"])
        text += ('<h2>Размер запуска первичной геометрии</h2>'
                 '<p>Один прогрев и три измерения каждой конфигурации; все записи проверяются побайтово. '
                 'Эти пробы исключены из времени полного рендера.</p>'
                 '<table><tr><th>Строк в команде</th><th>Потоков в группе</th><th>Форма группы</th><th>Медиана</th>'
                 '<th>Различия / ошибки</th></tr>' + rows + '</table>')
    return text
