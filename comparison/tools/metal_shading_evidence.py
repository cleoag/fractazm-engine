"""Strict validation and retained gallery for the pinned exact scene shading scope."""
from __future__ import annotations

import html
import math
from pathlib import Path
import statistics
import struct
from PIL import Image
import primary_dispatch_evidence as primary_evidence
import metal_scene_manifest as scene_manifest

SCHEMA = "mb3d.metal-exact-shading-evidence.v1"
PROFILE = scene_manifest.DEFAULT_SCENE.profile(True)
SCOPE = scene_manifest.DEFAULT_SCENE.scope(True)
SCENE_SHA256 = scene_manifest.DEFAULT_SCENE.sha256
IMAGES = {"gpu": "gpu-final.ppm", "cpu": "cpu-final.ppm"}
BUFFER_BYTES = {f"{backend}-{stage}.bin": size for backend in ("gpu", "cpu")
                for stage, size in (("primary", 176), ("packed", 48), ("shadow", 48), ("ao", 48), ("color", 16))}
BUFFER_BYTES["cpu-full-surface.bin"] = 48
BUFFER_BYTES.update({"gpu-shadow-rays.bin": 96, "cpu-shadow-rays.bin": 96})
BUFFER_BYTES.update({"gpu-shadow-traces.bin": 64, "cpu-shadow-traces.bin": 64, "gpu-prepared-shadow.bin": 48})
BUFFER_BYTES[primary_evidence.BASELINE_FILE] = primary_evidence.PIXEL_BYTES
BUFFER_BYTES[primary_evidence.NORMAL_BASELINE_FILE] = primary_evidence.PIXEL_BYTES
TIMINGS = ("gpu_timestamp", "encode_submit_wait_readback", "cpu_full_render")
STAGES = ("primary", "surface_packing", "shadow_preparation", "shadow_traversal", "hard_shadows", "ambient_occlusion", "final_color")
SURFACE_GATES = ("surface_packing", "hard_shadows", "prepared_shadow_equivalence", "ambient_occlusion", "full_cpu_surface")
SURFACE_GATE = "all 48 bytes exact (all production surface fields and zero reserved/fault)"
COLOR_GATE = "RGB8 exact, zero channel error"
RAY_GATE = "all 96 bytes exact (prepared shadow rays, zero reserved/fault)"
TRACE_GATE = "all 64 bytes exact (independent shadow traces, zero record faults/reserved)"


def count(value, label):
    if type(value) is not int or value < 0:
        raise ValueError("Invalid nonnegative count: " + label)
    return value


def duration(value, label):
    if isinstance(value, bool) or not isinstance(value, (int, float)) or not math.isfinite(value) or value < 0:
        raise ValueError("Invalid timing sample: " + label)
    return value


def boolean(value, label):
    if type(value) is not bool:
        raise ValueError("Invalid Boolean evidence flag: " + label)
    return value


def timing(section, field, repeats):
    samples = section.get(field + "_ms")
    if not isinstance(samples, list) or len(samples) != repeats:
        raise ValueError("Missing timing repetitions: " + field)
    for sample in samples:
        duration(sample, field)
    median = duration(section.get(field + "_median_ms"), field + " median")
    if not math.isclose(median, statistics.median(samples), rel_tol=1e-12, abs_tol=1e-12):
        raise ValueError("Incorrect reported timing median: " + field)


def validate_report(report, width, height, threads, repeats, hard_shadows, numeric_validator, geometry_validator,
                    shadow_mode="parallel", shadow_prep_threads=32, shadow_rows_per_command=256,
                    shadow_dispatch_sweep=False, shadow_trace_threads=32, primary_rows_per_command=256,
                    primary_threads=32, primary_dispatch_sweep=False, primary_tile_width=8,
                    scene_descriptor=scene_manifest.DEFAULT_SCENE, normal_mode="parallel", normal_threads=32):
    if (report.get("schema") != "mb3d.metal-exact-shading.v1" or report.get("numeric_profile") != scene_descriptor.profile(True)
            or report.get("status") not in ("PASS_SCOPED", "FAIL")):
        raise ValueError("Unexpected exact-shading schema, profile or status")
    if (report.get("native_arm64") is not True or report.get("translated") is not False
            or count(report.get("cpu_compute_fallback_calls"), "fallback calls") != 0
            or report.get("runtime_shader_compilation") is not False
            or count(report.get("abi_checks"), "ABI checks") != 24):
        raise ValueError("Native ARM64, 24 checked ABI fields, no CPU fallback and offline Metal required")
    scene_manifest.validate_description(report, scene_descriptor)
    for field in ("scene_preparation_ms", "device_library_queue_setup_ms"):
        duration(report.get(field), field)
    shading = report.get("shading", {})
    scene_manifest.validate_identity(shading, scene_descriptor)
    shadows_enabled = bool(scene_descriptor.original_shadow_bits & 1) or hard_shadows
    for key, expected in (("width", width), ("height", height), ("cpu_reference_threads", threads)):
        if count(shading.get(key), key) != expected:
            raise ValueError("Scene or execution settings differ from requested command")
    if (shading.get("scene_sha256") != scene_descriptor.sha256
            or boolean(shading.get("hard_shadows_override"), "shadow override") != hard_shadows
            or boolean(shading.get("hard_shadows_enabled"), "enabled shadows") != shadows_enabled):
        raise ValueError("Scene or hard-shadow settings differ from requested command")
    if (shadow_mode not in ("legacy", "prepared", "parallel") or shading.get("shadow_mode") != shadow_mode
            or count(shading.get("shadow_prep_threads_per_threadgroup"), "shadow preparation group") != shadow_prep_threads
            or count(shading.get("shadow_trace_threads_per_threadgroup"), "shadow trace group") != shadow_trace_threads
            or count(shading.get("shadow_rows_per_command"), "shadow row batch") != shadow_rows_per_command
            or not 1 <= shadow_prep_threads <= 4096 or not 1 <= shadow_trace_threads <= 4096
            or not 1 <= shadow_rows_per_command <= 4096):
        raise ValueError("Shadow execution settings differ from the requested command")
    validate_gpu_metadata(report, width, height, shadow_prep_threads, shadow_rows_per_command, shadow_trace_threads)
    if "images" in report and report["images"] != IMAGES:
        raise ValueError("Unexpected final image paths")
    if "buffers" in report and report["buffers"] != BUFFER_BYTES:
        raise ValueError("Unexpected raw-buffer paths")
    numeric_failures = numeric_validator(report, include_single=True)
    failures = 0
    comparison = shading.get("primary", {})
    scene_manifest.validate_geometry_limits(comparison, scene_descriptor)
    failures += geometry_validator(comparison, width, height)
    pixels = width * height
    failures += scene_manifest.validate_formula_rejections(shading.get("formula_rejections"), pixels, scene_descriptor)
    ray = shading.get("shadow_preparation", {})
    if ray.get("gate") != RAY_GATE or count(ray.get("pixels"), "prepared shadow pixels") != pixels:
        raise ValueError("Prepared shadow ray gate or denominator changed")
    active = count(ray.get("active_rays"), "active shadow rays")
    if active > comparison["hits"] or (not shadows_enabled and active) or (shadows_enabled and not active and report["status"] == "PASS_SCOPED"):
        raise ValueError("Prepared shadow rays disagree with scene coverage")
    for key in ("different_pixels", "fault_pixels"):
        value = count(ray.get(key), "prepared shadow " + key)
        if value > pixels:
            raise ValueError("Invalid prepared shadow ray counter")
        failures += value
    trace = shading.get("shadow_traversal", {})
    if (trace.get("gate") != TRACE_GATE or count(trace.get("pixels"), "shadow trace pixels") != pixels
            or count(trace.get("active_rays"), "active shadow traces") != active
            or count(trace.get("terminal_fault_pixels"), "deferred trace faults") > active):
        raise ValueError("Independent shadow trace gate, coverage or terminal-fault counts changed")
    for key in ("different_pixels", "fault_pixels"):
        value = count(trace.get(key), "shadow trace " + key)
        if value > pixels:
            raise ValueError("Invalid independent shadow trace counter")
        failures += value
    for name in SURFACE_GATES:
        section = shading.get(name, {})
        if section.get("gate") != SURFACE_GATE or count(section.get("pixels"), name + " pixels") != pixels:
            raise ValueError("Surface gate or denominator changed: " + name)
        for key in ("different_pixels", "fault_pixels"):
            n = count(section.get(key), name + " " + key)
            if n > pixels:
                raise ValueError("Invalid surface counter: " + name)
            failures += n
    for name in ("color_stage", "final_rgb"):
        section = shading.get(name, {})
        if (section.get("gate") != COLOR_GATE or count(section.get("pixels"), name + " pixels") != pixels
                or count(section.get("channels"), name + " channels") != pixels * 3):
            raise ValueError("RGB gate or denominator changed: " + name)
        changed = count(section.get("different_pixels"), name + " different pixels")
        channels = count(section.get("different_channels"), name + " different channels")
        maximum = count(section.get("max_channel_error"), name + " maximum error")
        faults = count(section.get("fault_pixels"), name + " fault pixels")
        if (changed > pixels or not changed <= channels <= 3 * changed or maximum > 255
                or (maximum == 0) != (changed == 0) or faults > pixels):
            raise ValueError("Inconsistent RGB counters: " + name)
        failures += changed + faults
    negative = shading.get("negative_controls", {})
    checks = count(negative.get("checks"), "negative controls checks")
    rejected = count(negative.get("failures"), "negative controls failures")
    if (checks != 22 * pixels + 4 or rejected > checks or count(negative.get("cases"), "negative control cases") != 26
            or boolean(negative.get("pass"), "negative controls") != (rejected == 0)):
        raise ValueError("Negative control evidence is missing or inconsistent")
    failures += rejected
    skip = shading.get("trace_skip_controls", {})
    skip_failures = count(skip.get("failures"), "trace skip failures")
    if (count(skip.get("cases"), "trace skip cases") != 6 or count(skip.get("checks"), "trace skip checks") != 12
            or skip_failures > 12 or boolean(skip.get("pass"), "trace skip pass") != (skip_failures == 0)):
        raise ValueError("Trace shortcut boundary controls are missing or inconsistent")
    failures += skip_failures
    if shadow_dispatch_sweep:
        if not shadows_enabled or shadow_mode != "prepared":
            raise ValueError("Dispatch trials require enabled prepared shadows")
        failures += validate_dispatch_trials(shading, width, height)
    elif shading.get("dispatch_trials") is not None:
        raise ValueError("Unrequested shadow dispatch trials in report")
    for field in ("gpu_repeats_bit_exact", "cpu_repeats_bit_exact"):
        failures += not boolean(shading.get(field), field)
    changed_shadows = count(shading.get("shadow_changed_pixels"), "shadow changed pixels")
    changed_ao = count(shading.get("ao_changed_pixels"), "AO changed pixels")
    if changed_shadows > pixels or changed_ao > pixels:
        raise ValueError("Invalid changed-stage count")
    if not shadows_enabled and changed_shadows:
        raise ValueError("Original M3P has shadows disabled; the stage must be a no-op")
    if shadows_enabled and not changed_shadows and report["status"] == "PASS_SCOPED":
        raise ValueError("Enabled shadows must exercise discriminating scene pixels")
    if not changed_ao and report["status"] == "PASS_SCOPED":
        raise ValueError("AO must exercise discriminating scene pixels")
    primary_timing = shading.get("timing_stages", {}).get("primary", {})
    primary_times = primary_timing.get("gpu_ms", [])
    failures += primary_evidence.validate_normal_pipeline(
        report.get("normal_pipeline"), width, height, repeats, normal_mode, normal_threads,
        primary_times, primary_timing.get("first_gpu_ms"))
    failures += primary_evidence.validate(shading.get("primary_dispatch"), width, height,
                                          primary_rows_per_command, primary_threads, primary_dispatch_sweep,
                                          primary_evidence.normal_geometry_maximum(report["normal_pipeline"]),
                                          primary_tile_width)
    passing = failures == 0
    if (boolean(shading.get("pass"), "shading pass") != passing
            or (report["status"] == "PASS_SCOPED") != (passing and numeric_failures == 0)):
        raise ValueError("Qualification status disagrees with shading/numerical evidence")
    for field in TIMINGS:
        timing(shading, field, repeats)
    duration(shading.get("first_encode_submit_wait_readback_ms"), "first GPU frame")
    stages = shading.get("timing_stages", {})
    if set(stages) != set(STAGES):
        raise ValueError("Missing stage timing denominators")
    for name in STAGES:
        section = stages[name]
        duration(section.get("first_gpu_ms"), name + " first GPU sample")
        # Stage report spells the median in the order used by the driver.
        timing({"gpu_ms": section.get("gpu_ms"), "gpu_median_ms": section.get("median_gpu_ms")}, "gpu", repeats)
    prep = stages["shadow_preparation"]
    if shadow_mode == "legacy" and (prep["first_gpu_ms"] != 0 or any(prep["gpu_ms"])):
        raise ValueError("Legacy timing must keep separate shadow preparation at zero")
    if shadow_mode != "legacy" and (prep["first_gpu_ms"] <= 0 or any(x <= 0 for x in prep["gpu_ms"])):
        raise ValueError("Prepared shadow execution must include preparation time")
    trace_timing = stages["shadow_traversal"]
    if shadow_mode != "parallel" and (trace_timing["first_gpu_ms"] != 0 or any(trace_timing["gpu_ms"])):
        raise ValueError("Independent trace diagnostic must stay outside nonparallel render timings")
    if shadow_mode == "parallel" and (trace_timing["first_gpu_ms"] <= 0 or any(x <= 0 for x in trace_timing["gpu_ms"])):
        raise ValueError("Parallel shadow execution must include trace time")
    for field in ("max_shadow_prepare_command_ms", "max_shadow_trace_command_ms", "max_hard_shadow_command_ms"):
        measured = duration(shading.get(field), field)
        if ((field == "max_shadow_prepare_command_ms" and shadow_mode == "legacy")
                or (field == "max_shadow_trace_command_ms" and shadow_mode != "parallel")):
            if measured != 0:
                raise ValueError("Untimed shadow diagnostic must be excluded from timed command maximum")
        elif measured <= 0:
            raise ValueError("Missing actual GPU command duration: " + field)
    for field, stage in (("max_hard_shadow_command_ms", "hard_shadows"),
                         ("max_shadow_prepare_command_ms", "shadow_preparation"),
                         ("max_shadow_trace_command_ms", "shadow_traversal")):
        if ((stage == "shadow_preparation" and shadow_mode == "legacy")
                or (stage == "shadow_traversal" and shadow_mode != "parallel")):
            continue  # Extra diagnostics run outside the selected mode's benchmark.
        maximum = max(stages[stage]["first_gpu_ms"], *stages[stage]["gpu_ms"])
        if shading[field] > maximum + max(1e-9, maximum * 1e-12):
            raise ValueError("Maximum command exceeds its complete stage timing")
    for index, total in enumerate(shading["gpu_timestamp_ms"]):
        stage_total = sum(stages[name]["gpu_ms"][index] for name in STAGES)
        if not math.isclose(total, stage_total, rel_tol=1e-12, abs_tol=1e-9):
            raise ValueError("Full GPU timing disagrees with stage samples")
    return report


def validate_dispatch_trials(shading, width, height):
    trials = shading.get("dispatch_trials")
    if not isinstance(trials, dict):
        raise ValueError("Requested shadow dispatch trials are missing")
    pixels = width * height
    if (count(trials.get("records_per_run"), "trial records per run") != pixels
            or count(trials.get("warmup_runs"), "trial warmup runs") != 1
            or count(trials.get("timed_runs"), "trial timed runs") != 3
            or not isinstance(trials.get("note"), str) or not trials["note"]):
        raise ValueError("Dispatch trial run denominators or scope are invalid")
    pipeline = shading["pipeline_limits"]
    simd = pipeline["shadow_prepare"]["thread_execution_width"]
    maximum = pipeline["shadow_prepare"]["max_total_threads_per_threadgroup"]
    if (count(trials.get("prepare_thread_execution_width"), "trial preparation SIMD width") != simd
            or count(trials.get("prepare_max_total_threads_per_threadgroup"), "trial preparation group limit") != maximum
            or count(trials.get("march_thread_execution_width"), "trial march SIMD width") != pipeline["shadow_march"]["thread_execution_width"]):
        raise ValueError("Dispatch trial preparation limits differ from actual pipeline")
    candidates = [n for n in (32, 64, 128, 256) if n % simd == 0 and n <= maximum]
    row_candidates = sorted({128, 256, 512, height})
    failures = 0
    for stage, expected, key, stride in (
            ("preparation", candidates, "threads_per_threadgroup", 96),
            ("march", row_candidates, "rows_per_command", 48)):
        rows = trials.get(stage)
        if (not isinstance(rows, list) or not rows or len(rows) != len(expected)
                or any(not isinstance(row, dict) for row in rows)
                or [count(row.get(key), stage + " trial configuration") for row in rows] != expected):
            raise ValueError("Dispatch trial configurations are missing, duplicated or unsupported: " + stage)
        for row in rows:
            if (count(row.get("checked_records"), stage + " checked records") != 4 * pixels
                    or count(row.get("record_bytes"), stage + " record bytes") != stride):
                raise ValueError("Dispatch trial exact-record coverage is incomplete")
            if stage == "march" and count(row.get("threads_per_threadgroup"), "march trial threads") != pipeline["shadow_march"]["thread_execution_width"]:
                raise ValueError("March trial threads differ from pipeline SIMD width")
            for field in ("different_records", "fault_records"):
                value = count(row.get(field), stage + " trial " + field)
                if value > 4 * pixels:
                    raise ValueError("Dispatch trial failure counter exceeds checked records")
                failures += value
            timing({"gpu_ms": row.get("gpu_ms"), "gpu_median_ms": row.get("median_gpu_ms")}, "gpu", 3)
            warmup = duration(row.get("warmup_gpu_ms"), stage + " trial warmup")
            if warmup <= 0 or any(value <= 0 for value in row["gpu_ms"]):
                raise ValueError("Dispatch trial must record actual GPU work")
            if stage == "march":
                command = duration(row.get("max_command_gpu_ms"), "march trial command maximum")
                total = max(warmup, *row["gpu_ms"])
                if command <= 0 or command > total + max(1e-9, total * 1e-12):
                    raise ValueError("Dispatch trial command maximum exceeds its complete run")
    if boolean(trials.get("pass"), "dispatch trials pass") != (failures == 0):
        raise ValueError("Dispatch trial pass flag disagrees with exact-record checks")
    return failures


def validate_gpu_metadata(report, width, height, prep_threads, rows, trace_threads):
    hardware = report.get("hardware", {})
    if hardware.get("unified_memory") is not True:
        raise ValueError("Native Apple Silicon unified-memory evidence required")
    maximum_buffer = count(hardware.get("max_buffer_length"), "maximum Metal buffer length")
    if maximum_buffer < width * height * 176:
        raise ValueError("Declared GPU cannot hold the primary buffer")
    if count(hardware.get("recommended_max_working_set_size"), "recommended Metal working set") == 0:
        raise ValueError("Missing GPU working-set metadata")
    maximum_threads = hardware.get("max_threads_per_threadgroup")
    if (not isinstance(maximum_threads, list) or len(maximum_threads) != 3
            or any(count(x, "maximum GPU group dimension") == 0 for x in maximum_threads)):
        raise ValueError("Invalid GPU threadgroup dimensions")
    limits = report["shading"].get("pipeline_limits", {})
    if set(limits) != {"shadow_prepare", "shadow_trace", "shadow_march"}:
        raise ValueError("Missing shadow pipeline GPU limits")
    for name, grid in (("shadow_prepare", width * height), ("shadow_trace", width * height),
                       ("shadow_march", min(height, rows))):
        pipeline = limits[name]
        simd = count(pipeline.get("thread_execution_width"), name + " SIMD width")
        maximum = count(pipeline.get("max_total_threads_per_threadgroup"), name + " group limit")
        group = count(pipeline.get("threads_per_threadgroup"), name + " group threads")
        if (simd == 0 or group == 0 or maximum == 0 or group % simd or group > maximum
                or group > maximum_threads[0] or count(pipeline.get("grid_threads"), name + " grid threads") != grid):
            raise ValueError("Shadow launch exceeds or disagrees with reported GPU limits")
        if name == "shadow_prepare" and group != prep_threads:
            raise ValueError("Shadow preparation group differs from requested threads")
        if name == "shadow_trace" and group != trace_threads:
            raise ValueError("Shadow trace group differs from requested threads")


def _surface_counts(gpu, cpu):
    if len(gpu) != len(cpu) or len(gpu) % 48:
        raise ValueError("Incomplete surface byte records")
    different = sum(gpu[i:i+48] != cpu[i:i+48] for i in range(0, len(gpu), 48))
    faults = sum(struct.unpack_from("<I", gpu, i + 32)[0] != 0 for i in range(0, len(gpu), 48))
    return different, faults


def validate_trace_records(raw, rays, hard_shadows):
    """Check the fixed ABI and its relation to the captured GPU ray inputs."""
    for i in range(0, len(raw), 64):
        (version, active, fault, terminal, min_bits, final_bits, found_bits,
         has_loop, threshold, writes, reserved0, reserved1, reserved2) = struct.unpack_from("<4I3Q6I", raw, i)
        minimum, final, found = struct.unpack_from("<3d", raw, i+16)
        if (version != 1 or active > 1 or fault > 1 or terminal not in (0, 2, 4)
                or has_loop > 1 or threshold not in (0, 3) or writes > 1
                or reserved0 or reserved1 or reserved2 or not all(map(math.isfinite, (minimum, final, found)))
                or (has_loop and (not active or minimum < 0))
                or (not has_loop and (min_bits or terminal or threshold or writes))
                or (not writes and found_bits) or (threshold == 3 and not writes)
                or (fault and (active or has_loop or final_bits))):
            raise ValueError("Noncanonical or nonfinite independent shadow trace")
        ray_offset = (i // 64) * 96
        _, ray_active, backface, ray_fault = struct.unpack_from("<4I", rays, ray_offset)
        length = struct.unpack_from("<d", rays, ray_offset+48)[0]
        expected_active = ray_active if hard_shadows else 0
        expected_loop = bool(expected_active and not ray_fault and not backface and length > 0)
        if not fault and (active != expected_active or has_loop != expected_loop
                          or (has_loop and minimum > length)):
            raise ValueError("Independent shadow trace disagrees with its prepared ray")


def validate_primary_cpu_buffers(folder: Path, report, width, height):
    paths = [folder / name for name in ("gpu-primary.bin", "cpu-primary.bin")]
    for path in paths:
        if not path.is_file() or path.stat().st_size != width * height * primary_evidence.PIXEL_BYTES:
            raise ValueError("Raw CPU/GPU primary size differs from the 176-byte ABI: " + path.name)
    if report["status"] == "PASS_SCOPED":
        for path in paths:
            raw = path.read_bytes()
            for offset in range(0, len(raw), primary_evidence.PIXEL_BYTES):
                orbit, reserved = struct.unpack_from("<dQ", raw, offset+160)
                if reserved or not math.isfinite(orbit):
                    raise ValueError("Noncanonical orbit color or reserved primary tail: " + path.name)
        with paths[0].open("rb") as gpu, paths[1].open("rb") as cpu:
            while chunk := gpu.read(1024 * 1024):
                if chunk != cpu.read(len(chunk)):
                    raise ValueError("Saved CPU/GPU primary records differ despite exact PASS")


def validate_buffers(folder: Path, report, width, height):
    pixels = width * height
    for name, stride in BUFFER_BYTES.items():
        path = folder / name
        if not path.is_file() or path.stat().st_size != pixels * stride:
            raise ValueError("Raw buffer size differs from declared ABI: " + name)
    primary_evidence.validate_buffers(folder, report["shading"]["primary_dispatch"], width, height)
    primary_evidence.validate_normal_buffers(folder, report["normal_pipeline"], width, height)
    validate_primary_cpu_buffers(folder, report, width, height)
    gpu_rays = (folder / "gpu-shadow-rays.bin").read_bytes()
    cpu_rays = (folder / "cpu-shadow-rays.bin").read_bytes()
    different = sum(gpu_rays[i:i+96] != cpu_rays[i:i+96] for i in range(0, len(gpu_rays), 96))
    faults = sum(struct.unpack_from("<I", gpu_rays, i+12)[0] != 0 for i in range(0, len(gpu_rays), 96))
    active = sum(struct.unpack_from("<I", gpu_rays, i+4)[0] == 1 for i in range(0, len(gpu_rays), 96))
    expected_rays = report["shading"]["shadow_preparation"]
    if (different, faults, active) != tuple(expected_rays[k] for k in ("different_pixels", "fault_pixels", "active_rays")):
        raise ValueError("Saved prepared shadow rays disagree with report")
    for raw in (gpu_rays, cpu_rays):
        for i in range(0, len(raw), 96):
            version, enabled, backface, fault = struct.unpack_from("<4I", raw, i)
            scalar = struct.unpack_from("<8dfI2I", raw, i+16)
            if (version != 1 or enabled > 1 or backface > 1 or fault & ~13 or any(scalar[-2:])
                    or not all(math.isfinite(value) for value in scalar[:9])):
                raise ValueError("Noncanonical or nonfinite prepared shadow ray")
            if not fault and ((not enabled and any(raw[i+8:i+96]))
                              or (enabled and scalar[5] <= 0)
                              or (enabled and not backface and scalar[4] > 0 and scalar[8] <= 0)):
                raise ValueError("Inactive or active prepared shadow ray has invalid state")
    gpu_traces = (folder / "gpu-shadow-traces.bin").read_bytes()
    cpu_traces = (folder / "cpu-shadow-traces.bin").read_bytes()
    different = sum(gpu_traces[i:i+64] != cpu_traces[i:i+64] for i in range(0, len(gpu_traces), 64))
    faults = sum(struct.unpack_from("<I", gpu_traces, i+8)[0] != 0 for i in range(0, len(gpu_traces), 64))
    active = sum(struct.unpack_from("<I", gpu_traces, i+4)[0] == 1 for i in range(0, len(gpu_traces), 64))
    terminal = sum(struct.unpack_from("<I", gpu_traces, i+12)[0] != 0 for i in range(0, len(gpu_traces), 64))
    expected_traces = report["shading"]["shadow_traversal"]
    if (different, faults, active, terminal) != tuple(expected_traces[k] for k in
            ("different_pixels", "fault_pixels", "active_rays", "terminal_fault_pixels")):
        raise ValueError("Saved independent shadow traces disagree with report")
    # Both diagnostics consume the captured GPU rays; production CPU rendering
    # remains a separate shadow/full-surface/RGB oracle.
    for raw in (gpu_traces, cpu_traces):
        validate_trace_records(raw, gpu_rays, report["shading"]["hard_shadows_enabled"])
    gates = (("surface_packing", "gpu-packed.bin", "cpu-packed.bin"),
             ("hard_shadows", "gpu-shadow.bin", "cpu-shadow.bin"),
             ("prepared_shadow_equivalence", "gpu-shadow.bin", "gpu-prepared-shadow.bin"),
             ("ambient_occlusion", "gpu-ao.bin", "cpu-ao.bin"),
             ("full_cpu_surface", "gpu-ao.bin", "cpu-full-surface.bin"))
    for section, gpu, cpu in gates:
        counts = _surface_counts((folder / gpu).read_bytes(), (folder / cpu).read_bytes())
        expected = report["shading"][section]
        if counts != (expected["different_pixels"], expected["fault_pixels"]):
            raise ValueError("Saved surface bytes disagree with report: " + section)
    packed = (folder / "gpu-packed.bin").read_bytes()
    shadow = (folder / "gpu-shadow.bin").read_bytes()
    ao = (folder / "gpu-ao.bin").read_bytes()
    changed_shadow = sum(packed[i+24:i+28] != shadow[i+24:i+28] for i in range(0, len(packed), 48))
    changed_ao = sum(shadow[i+28:i+32] != ao[i+28:i+32] for i in range(0, len(ao), 48))
    if (changed_shadow, changed_ao) != (report["shading"]["shadow_changed_pixels"], report["shading"]["ao_changed_pixels"]):
        raise ValueError("Saved stage changes disagree with report")
    gpu = (folder / "gpu-color.bin").read_bytes()
    cpu = (folder / "cpu-color.bin").read_bytes()
    actual = _rgb_counts(_color_rgb(gpu), _color_rgb(cpu))
    faults = sum(any(struct.unpack_from("<III", gpu, i + 4)) for i in range(0, len(gpu), 16))
    section = report["shading"]["color_stage"]
    if actual != tuple(section[k] for k in ("different_pixels", "different_channels", "max_channel_error")) or faults != section["fault_pixels"]:
        raise ValueError("Saved color records disagree with report")


def _color_rgb(raw):
    return b"".join(raw[i:i+3] for i in range(0, len(raw), 16))


def _rgb_counts(gpu, cpu):
    if len(gpu) != len(cpu) or len(gpu) % 3:
        raise ValueError("RGB image lengths disagree")
    changed_pixels = sum(gpu[i:i+3] != cpu[i:i+3] for i in range(0, len(gpu), 3))
    changed_channels = sum(a != b for a, b in zip(gpu, cpu))
    maximum = max((abs(a-b) for a, b in zip(gpu, cpu)), default=0)
    return changed_pixels, changed_channels, maximum


def validate_final_images(folder: Path, report, width, height):
    data = {}
    for backend, name in IMAGES.items():
        with Image.open(folder / name) as image:
            if image.size != (width, height) or image.mode != "RGB":
                raise ValueError("Final image dimensions or mode disagree")
            data[backend] = image.tobytes()
    if data["gpu"] != _color_rgb((folder / "gpu-color.bin").read_bytes()):
        raise ValueError("Saved GPU image differs from GPU color records")
    section = report["shading"]["final_rgb"]
    actual = _rgb_counts(data["gpu"], data["cpu"])
    if actual != tuple(section[k] for k in ("different_pixels", "different_channels", "max_channel_error")):
        raise ValueError("Saved final RGB differs from reported CPU comparison")
    # Color-stage errors and full-production errors have distinct oracles.
    cpu_adapter = _color_rgb((folder / "cpu-color.bin").read_bytes())
    if (section["different_pixels"] == 0 and report["shading"]["color_stage"]["different_pixels"] == 0
            and cpu_adapter != data["cpu"]):
        raise ValueError("CPU stage and complete-render images disagree despite reported PASS")


def gallery_html(report, images, descriptor=None):
    esc = lambda value: html.escape(str(value), quote=True)
    s = report["shading"]
    descriptor = descriptor or scene_manifest.scene_for_report(s)
    mode = ("Жёсткие тени включены явным параметром --hard-shadows. Исходный M3P сохранён без изменений; "
            "CPU и GPU используют одинаковое включение теней." if s["hard_shadows_override"] else
            "Жёсткие тени включены исходной M3P-сценой; CPU и GPU сохраняют эту настройку." if s["hard_shadows_enabled"] else
            "В исходной M3P-сцене жёсткие тени отключены. Этот вариант сохраняет исходную настройку; "
            "GPU-проход теней возвращает неизменённую поверхность.")
    figures = []
    for row in images:
        label = "Metal GPU · итоговый рендер" if row["backend"] == "gpu" else "Основной CPU-рендерер · независимый результат"
        figures.append(f'<figure><a href="{esc(row["png"])}"><img src="{esc(row["png"])}" alt="{esc(label)}"></a><figcaption>{label}<br><a href="{esc(row["png"])}" download>Скачать PNG</a></figcaption></figure>')
    labels = {"primary": "Геометрия и нормали", "surface_packing": "Упаковка поверхности", "shadow_preparation": "Подготовка теневых лучей",
              "shadow_traversal": "Независимая маршировка теневых лучей",
              "hard_shadows": "Последовательное разрешение теней" if s["shadow_mode"] == "parallel" else "Жёсткие тени",
              "ambient_occlusion": "AO и фильтрация", "final_color": "Палитра, свет, туман и RGB"}
    timings = "".join(f'<tr><td>{labels[name]}</td><td>{s["timing_stages"][name]["median_gpu_ms"]:.3f} мс</td></tr>' for name in STAGES)
    primitives = "; ".join(f'{label}: {report[key]["checks"]} проверок, ошибок {report[key]["failures"]}' for key, label in (("arithmetic", "binary64"), ("extended_arithmetic", "extended80"), ("single_arithmetic", "binary32")))
    rgb = s["final_rgb"]
    preparation = s["shadow_preparation"]
    traversal = s["shadow_traversal"]
    trials_html = ""
    if s.get("dispatch_trials") is not None:
        trials = s["dispatch_trials"]
        trial_rows = "".join(
            f'<tr><td>{label}</td><td>{row[key]}</td><td>{row["median_gpu_ms"]:.3f} мс</td>'
            f'<td>{row["checked_records"]}</td><td>{row["different_records"]} / {row["fault_records"]}</td></tr>'
            for stage, label, key in (("preparation", "Подготовка: потоков в группе", "threads_per_threadgroup"),
                                      ("march", "Маршировка: строк в команде", "rows_per_command"))
            for row in trials[stage])
        trials_html = ('<h2>Подбор размеров запуска</h2><p>Изолированные GPU-проходы: один прогрев и три измерения '
                       'для каждой конфигурации. Каждый запуск проверяет все записи побайтово. Эти пробы исключены '
                       'из времени полного рендера.</p><table><tr><th>Проход</th><th>Размер</th><th>Медиана</th>'
                       '<th>Проверено записей</th><th>Различия / ошибки</th></tr>' + trial_rows + '</table>')
    shadow_mode = {
        "parallel": "Подготовка и маршировка каждого теневого луча выполняются параллельно на GPU. Лёгкий проход по строкам применяет результаты в исходном порядке и сохраняет правила пропуска открытых участков.",
        "prepared": "Подготовка каждого теневого луча выполняется отдельным параллельным GPU-проходом; маршировка сохраняет порядок пикселей внутри строки. Диагностика независимых трасс выполняется вне измеряемого рендера.",
        "legacy": "Базовый режим legacy: подготовка и маршировка выполняются последовательно внутри строки. Отдельные проверки подготовленных лучей и независимых трасс выполняются вне измеряемого рендера."
    }[s["shadow_mode"]]
    gates = "".join(f'<li>{label}: {s[key]["pixels"]} пикселей, различаются {s[key]["different_pixels"]}, с ошибкой вычисления {s[key]["fault_pixels"]}.</li>' for key, label in (("surface_packing", "Упаковка поверхности"), ("hard_shadows", "Тени"), ("prepared_shadow_equivalence", "Совпадение с прежним GPU-проходом prepared"), ("ambient_occlusion", "AO"), ("full_cpu_surface", "Поверхность полного CPU-рендера")))
    return f'''<!doctype html><html lang="ru"><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1">
<title>{esc(descriptor.formula_name)}: итоговый Metal GPU / CPU рендер</title><style>body{{background:#11161e;color:#e8edf5;font:16px/1.55 system-ui;margin:0}}main{{max-width:1400px;margin:auto;padding:30px}}a{{color:#9bd1ff}}.images{{display:grid;grid-template-columns:1fr 1fr;gap:20px}}figure{{margin:0}}img{{width:100%;display:block}}figcaption{{padding:10px 0}}.notice{{border-left:4px solid #e9ba75;background:#29241c;padding:18px}}table{{border-collapse:collapse}}td,th{{padding:8px 18px;border-bottom:1px solid #374350;text-align:left}}@media(max-width:800px){{.images{{grid-template-columns:1fr}}main{{padding:16px}}}}</style>
<main><h1>{esc(descriptor.formula_name)} · итоговый GPU и CPU рендер</h1><p><strong>{esc(report['status'])}</strong> · {s['width']}×{s['height']} · {esc(Path(descriptor.path).name)} · {esc(report.get('device','Apple Silicon'))}</p>
<p>На GPU выполняются лучи, формула {esc(descriptor.formula_name)}, DE, уточнение поверхности, нормали, упаковка, тени, AO и финальные цвета сцены. CPU подготавливает сцену и таблицы, отправляет команды и сохраняет изображение. Отдельный полный CPU-рендер служит независимой проверкой.</p>
<p class="notice">{mode} Это сравнение с текущим нативным CPU-кодом проекта, а не с оригинальным изображением Pascal. Эта проверка относится к закреплённой сцене {esc(descriptor.scene_id)}; отражения, объёмный свет, DOF и другие формулы здесь не квалифицируются.</p>
<p>Итоговый RGB: {rgb['pixels']} пикселей / {rgb['channels']} каналов; различаются {rgb['different_pixels']} пикселей и {rgb['different_channels']} каналов, максимальная ошибка {rgb['max_channel_error']}. Допуск изображения — точное совпадение каждого байта. Поверхность и сравниваемые численные значения геометрии совпадают побитово; исходные проверки допусков также сохранены. Все 176 байт каждой первичной записи CPU/GPU сравниваются точно.</p>
<p>Отказы для неизвестной формулы, чужой capability, изменённого параметра, ненулевого reserved и несовместимой специализации: {s["formula_rejections"]["checks"]} проверок, ошибок {s["formula_rejections"]["failures"]}.</p>
<p>Явный отказ для dIFS без выбранного минимума: {s["formula_rejections"]["no_minimum_checks"]} проверок, ошибок {s["formula_rejections"]["no_minimum_failures"]}. Эти случаи требуют состояния предыдущего CPU-вызова и не считаются точным совпадением рендера.</p>
{primary_evidence.gallery_html(s["primary_dispatch"])}
{primary_evidence.normal_gallery_html(report["normal_pipeline"], report["division_mode"])}
<div class="images">{''.join(figures)}</div><h2>Проверки</h2><p>{primitives}.</p><ul>{gates}</ul><p>Тени изменили {s['shadow_changed_pixels']} пикселей; AO — {s['ao_changed_pixels']}. Проверки отказов: {s['negative_controls']['checks']}, ошибок {s['negative_controls']['failures']}. Повторы GPU: {esc(s['gpu_repeats_bit_exact'])}; повторы CPU: {esc(s['cpu_repeats_bit_exact'])}.</p>
<p>Режим теней: <strong>{esc(s['shadow_mode'])}</strong>. {shadow_mode} Подготовленные лучи: {preparation['pixels']} записей по 96 байт, различаются {preparation['different_pixels']}, ошибок вычисления {preparation['fault_pixels']}, активных лучей {preparation['active_rays']}. Это дополнительная проверка общим кодом подготовки на сохранённых данных GPU; тени и полный результат также проверяются независимым основным CPU-рендерером. Размер группы подготовки: {s['shadow_prep_threads_per_threadgroup']}; строк в последовательной команде: {s['shadow_rows_per_command']}.</p>
<p>Независимые трассы: {traversal['pixels']} записей по 64 байта, различаются {traversal['different_pixels']}, ошибок записей {traversal['fault_pixels']}. Отложенных конечных ошибок: {traversal['terminal_fault_pixels']}; они становятся ошибкой рендера, только если последовательный проход действительно доходит до соответствующего вычисления. Граничные проверки пропуска: {s['trace_skip_controls']['checks']}, ошибок {s['trace_skip_controls']['failures']}. Размер группы маршировки лучей: {s['shadow_trace_threads_per_threadgroup']}. Сравнение трасс использует общий алгоритм на CPU и GPU; независимые основные CPU-проверки сохранены.</p>
<h2>Скорость</h2><p>Медианы {len(s['gpu_timestamp_ms'])} повторов после прогрева. CPU использует {s['cpu_reference_threads']} потоков. Для CPU учитывается полный Render с подготовкой параметров; для GPU отдельно показаны команды и отправка/ожидание/чтение. Запись изображений исключена. Эти измерения относятся к точной программной арифметике и данной сцене.</p>
<table><tr><th>Этап GPU</th><th>Медиана</th></tr>{timings}</table><p>GPU, сумма времени команд: {s['gpu_timestamp_median_ms']:.3f} мс; GPU с отправкой, ожиданием и чтением: {s['encode_submit_wait_readback_median_ms']:.3f} мс; полный CPU-рендер: {s['cpu_full_render_median_ms']:.3f} мс.</p>
{trials_html}
<p><a href="probe/report.json">Подробные результаты</a> · <a href="summary.json">Происхождение и хеши файлов</a> · <a href="inputs/scene.m3p">Исходная M3P</a></p></main></html>'''
