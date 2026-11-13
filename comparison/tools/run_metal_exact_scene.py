"""Retain native Metal arithmetic and pinned scene evidence.

The default visualizes primary normals. --full-shading compares the final GPU
render with the production CPU pipeline; --hard-shadows explicitly enables the
original scene's disabled shadow pass without changing the input M3P.
"""
from __future__ import annotations

import argparse
import html
import json
import math
import os
from pathlib import Path
import shutil
import statistics
import subprocess

from benchmark_gallery_arm64 import (ROOT, binary_identity, execute, native_host,
                                      sha, utc_now, write_json)
from native_evidence import source_identity
from run_metal_formula_probe import convert_ppm_losslessly
import metal_shading_evidence as shading_evidence
import primary_dispatch_evidence as primary_evidence
import metal_scene_manifest as scene_manifest

SCHEMA = "mb3d.metal-exact-primary-evidence.v1"
DEFAULT_SCENE = scene_manifest.DEFAULT_SCENE
PROFILE = DEFAULT_SCENE.profile()
SCENE = ROOT / DEFAULT_SCENE.path
SCENE_SHA256 = DEFAULT_SCENE.sha256
PIXEL_BYTES = 176
IMAGES = {"gpu": "gpu-primary-normals.ppm", "cpu": "cpu-primary-normals.ppm"}
BUFFERS = {"gpu": "gpu-primary.bin", "cpu": "cpu-primary.bin",
           "gpu_baseline": primary_evidence.BASELINE_FILE,
           "gpu_inline": primary_evidence.NORMAL_BASELINE_FILE}
TIMINGS = ("gpu_timestamp", "encode_submit_wait_readback")
SQRT_SEED_CASES = ("exact", "minus_one", "plus_one", "minus_four", "plus_four", "minus_five",
                   "plus_five", "zero", "limit", "uint64_max", "random32", "random64", "invalid_high")
SQRT_SEED_GATE = "sqrt non-NaN bits exact; NaN class; prefix root/remainder/valid exact"
SQRT_SEED_ORACLE = "native ARM64 std::sqrt(double); independent integer prefix binary search"
POWER_TWO_MULTIPLICATION_PER_ORDER = 2098 * 26 * 4
# Shared native generator: every binary64 exponent, cancellation/alignment
# boundaries in both operand orders, and explicit nearest-even product ties.
NORMAL_ARITHMETIC_COVERAGE = {
    "normal_addition_boundary_checks": 3820096,
    "normal_addition_boundary_add_checks": 1910048,
    "normal_addition_boundary_subtract_checks": 1910048,
    "normal_addition_boundary_swapped_checks": 1795360,
    "normal_multiplication_boundary_checks": 1182464,
    "normal_multiplication_halfway_checks": 224,
}
BINARY64_BASE_CHECKS_PER_OPERATION = 50000 + 22 * 22
SQRT_WIDE_CASES = ("exact", "minus_one", "plus_one", "minus_four", "plus_four", "minus_256", "plus_256",
                   "minus_1023", "plus_1023", "minus_1024", "plus_1024", "minus_1025", "plus_1025",
                   "zero", "root_max", "limit", "uint64_max", "hardware32", "random32", "random64", "root_max_clamp")
# These coverage counts come from the input bits and independent integer
# acceptance/rejection guarantees. Optional seeds may use the exact fallback.
SQRT_WIDE_REQUIRED_ACCEPTS = (19881, 19782, 16683, 19782, 16676, 19782, 16676, 0, 0, 0, 0, 0, 0,
                              13, 3106, 0, 0, 16775, 0, 0, 16)
SQRT_WIDE_REQUIRED_REJECTIONS = (0, 23, 3099, 40, 3106, 45, 3106, 45, 3106, 45, 3106, 45, 3106,
                                 19868, 16775, 19881, 19881, 3106, 19881, 19881, 0)
SQRT_WIDE_GATE = "sqrt non-NaN bits exact; NaN class; accepted prefix exact bracket/remainder; rejected prefix zero"
SQRT_WIDE_ORACLE = "native ARM64 std::sqrt(double); independent unsigned __int128 prefix binary search"


def count(value, name):
    if isinstance(value, bool) or not isinstance(value, int) or value < 0:
        raise ValueError("Invalid nonnegative count: " + name)
    return value


def duration(value, name):
    if isinstance(value, bool) or not isinstance(value, (int, float)) or not math.isfinite(value) or value < 0:
        raise ValueError("Invalid timing sample: " + name)
    return value


def validate_sqrt_seed_controls(report):
    section = report.get("sqrt_seed_controls", {})
    expected = {"matrix_inputs": 19712, "seed_variants": 12, "invalid_high_checks": 6,
                "records": 236550, "checks": 473100, "sqrt_checks": 236550,
                "prefix_checks": 236550, "nan_class_checks": 84, "signed_zero_checks": 24,
                "input_bytes": 24, "record_bytes": 32}
    if (any(count(section.get(key), "sqrt seed " + key) != value for key, value in expected.items())
            or section.get("gate") != SQRT_SEED_GATE or section.get("oracle") != SQRT_SEED_ORACLE):
        raise ValueError("Sqrt seed coverage, independent oracle or exact gate changed")
    cases = section.get("cases")
    if (not isinstance(cases, list) or len(cases) != len(SQRT_SEED_CASES)
            or any(not isinstance(row, dict) for row in cases)
            or [row.get("case") for row in cases] != list(SQRT_SEED_CASES)):
        raise ValueError("Sqrt seed control cases missing, reordered or duplicated")
    failures = 0
    for row in cases:
        name = row["case"]
        records = 6 if name == "invalid_high" else 19712
        accepted = count(row.get("accepted_prefixes"), name + " accepted prefixes")
        rejected = count(row.get("rejected_prefixes"), name + " rejected prefixes")
        if count(row.get("records"), name + " records") != records or accepted + rejected != records:
            raise ValueError("Sqrt seed per-case coverage disagrees")
        # These categories have fixed independent correction outcomes. In
        # particular +/-5 must reject; +/-4 are still admitted when in range.
        expected_accepted = {"exact": records, "plus_one": records, "minus_one": records-10,
                             "minus_four": records-10, "minus_five": 0, "plus_five": 0,
                             "zero": 10, "uint64_max": 0, "invalid_high": 0}.get(name)
        if expected_accepted is not None and accepted != expected_accepted:
            raise ValueError("Sqrt seed correction boundary coverage changed")
        for key in ("sqrt_failures", "prefix_failures"):
            failed = count(row.get(key), name + " " + key)
            if failed > records:
                raise ValueError("Sqrt seed failures exceed checked records")
            failures += failed
    if (count(section.get("failures"), "sqrt seed failures") != failures
            or type(section.get("pass")) is not bool or section["pass"] != (failures == 0)):
        raise ValueError("Sqrt seed pass flag or failure totals disagree")
    if duration(section.get("gpu_timestamp_ms"), "sqrt seed GPU timestamp") <= 0:
        raise ValueError("Sqrt seed controls require actual GPU work")
    return failures


def validate_sqrt_wide_seed_controls(report):
    section = report.get("sqrt_wide_seed_controls", {})
    expected = {"matrix_inputs": 19712, "boundary_prefix_inputs": 169, "seed_variants": 20, "clamp_checks": 16,
                "records": 397636, "checks": 795272, "sqrt_checks": 397636, "prefix_checks": 397636,
                "nan_class_checks": 140, "signed_zero_checks": 40, "input_bytes": 24, "record_bytes": 32}
    if (any(count(section.get(key), "wide sqrt " + key) != value for key, value in expected.items())
            or section.get("gate") != SQRT_WIDE_GATE or section.get("oracle") != SQRT_WIDE_ORACLE):
        raise ValueError("Wide sqrt coverage, independent oracle or exact gate changed")
    cases = section.get("cases")
    if (not isinstance(cases, list) or len(cases) != len(SQRT_WIDE_CASES)
            or any(not isinstance(row, dict) for row in cases)
            or [row.get("case") for row in cases] != list(SQRT_WIDE_CASES)):
        raise ValueError("Wide sqrt cases missing, reordered or duplicated")
    failures = 0
    for row, required, rejected in zip(cases, SQRT_WIDE_REQUIRED_ACCEPTS, SQRT_WIDE_REQUIRED_REJECTIONS):
        records = 16 if row["case"] == "root_max_clamp" else 19881
        if (count(row.get("records"), "wide sqrt case records") != records
                or count(row.get("required_accepts"), "wide sqrt required accepts") != required
                or count(row.get("required_rejections"), "wide sqrt required rejections") != rejected
                or count(row.get("optional_prefixes"), "wide sqrt optional prefixes") != records - required - rejected):
            raise ValueError("Wide sqrt correction boundary or per-case coverage changed")
        failed = {key: count(row.get(key), "wide sqrt " + key) for key in ("sqrt_failures", "prefix_failures")}
        if any(value > records for value in failed.values()):
            raise ValueError("Wide sqrt failures exceed checked records")
        accepted_count = count(row.get("accepted_prefixes"), "wide sqrt accepted prefixes")
        rejected_count = count(row.get("rejected_prefixes"), "wide sqrt rejected prefixes")
        if (accepted_count + rejected_count != records
                or accepted_count + failed["prefix_failures"] < required
                or rejected_count + failed["prefix_failures"] < rejected):
            raise ValueError("Wide sqrt prefix outcomes disagree with checked guarantees")
        failures += sum(failed.values())
    if (count(section.get("failures"), "wide sqrt failures") != failures
            or type(section.get("pass")) is not bool or section["pass"] != (failures == 0)):
        raise ValueError("Wide sqrt pass flag or failure totals disagree")
    if duration(section.get("gpu_timestamp_ms"), "wide sqrt GPU timestamp") <= 0:
        raise ValueError("Wide sqrt controls require a positive GPU timestamp")
    return failures


DIVISION_SEED_CASES = ("exact", "minus_one", "plus_one", "minus_two", "plus_two",
                       "minus_three", "plus_three", "zero", "limit_minus_one",
                       "limit", "uint64_max", "hardware", "invalid_domain")
DIVISION_SEED_GATE = "all value bits exact with canonical NaNs; digit exact quotient/remainder or zero rejection"
DIVISION_SEED_ORACLE = "native ARM64 hardware binary64 division; independent unsigned __int128 quotient/remainder"


def validate_division_seed_controls(report):
    section = report.get("division_seed_controls")
    if not isinstance(section, dict):
        raise ValueError("Missing division seed controls")
    required = {"matrix_inputs": 14592, "seed_variants": 12, "invalid_domain_checks": 48,
                "records": 175152, "checks": 700608, "value_checks": 525456, "digit_checks": 175152,
                "input_bytes": 48, "record_bytes": 48}
    if any(count(section.get(key), "division " + key) != value for key, value in required.items()):
        raise ValueError("Division seed control coverage or ABI changed")
    if section.get("gate") != DIVISION_SEED_GATE or section.get("oracle") != DIVISION_SEED_ORACLE:
        raise ValueError("Division seed exact gate or independent oracle changed")
    if duration(section.get("gpu_timestamp_ms"), "division seed GPU time") <= 0:
        raise ValueError("Division seed controls must record GPU work")
    cases = section.get("cases")
    if (not isinstance(cases, list) or any(not isinstance(row, dict) for row in cases)
            or [row.get("case") for row in cases] != list(DIVISION_SEED_CASES)):
        raise ValueError("Division seed cases are missing, duplicated or reordered")
    value_failures = digit_failures = accepts = rejections = corrections = 0
    for row in cases:
        name = row["case"]
        records = 48 if name == "invalid_domain" else 14592
        if count(row.get("records"), name + " division records") != records:
            raise ValueError("Division seed case denominator changed")
        a = count(row.get("required_accepts"), name + " required accepts")
        r = count(row.get("required_rejections"), name + " required rejections")
        c = count(row.get("correction_accepts"), name + " correction accepts")
        actual_a = count(row.get("accepted_digits"), name + " accepted digits")
        actual_r = count(row.get("rejected_digits"), name + " rejected digits")
        vf = count(row.get("value_failures"), name + " value failures")
        df = count(row.get("digit_failures"), name + " digit failures")
        if (a+r != records or c > a or actual_a+actual_r > records
                or vf > records or df > records
                or (df == 0 and (actual_a != a or actual_r != r))):
            raise ValueError("Division seed branch or failure totals disagree")
        if name == "exact" and (a != records or c):
            raise ValueError("Exact division hints must be accepted without correction")
        if name in ("minus_one", "plus_one", "minus_two", "plus_two") and (not a or c != a):
            raise ValueError("Division correction branches require accepted corrected hints")
        if name in ("minus_three", "plus_three", "limit", "uint64_max", "invalid_domain") and (a or c):
            raise ValueError("Invalid division hints must be rejected")
        accepts += a; rejections += r; corrections += c
        value_failures += vf; digit_failures += df
    for key, value in (("required_accepts", accepts), ("required_rejections", rejections),
                       ("correction_accepts", corrections)):
        if count(section.get(key), "division " + key) != value or not value:
            raise ValueError("Division control branch totals are incomplete")
    failures = count(section.get("failures"), "division failures")
    if (not value_failures+digit_failures <= failures <= 3*value_failures+digit_failures
            or type(section.get("pass")) is not bool or section["pass"] != (failures == 0)):
        raise ValueError("Division seed pass flag or failure totals disagree")
    return failures


def validate_numeric_evidence(report, include_single=False):
    logarithm = report.get("native_log", {})
    if (count(logarithm.get("checks"), "native log checks") != 441307
            or count(logarithm.get("log_checks"), "native logarithm cases") != 216981
            or count(logarithm.get("fma_checks"), "native fused cases") != 224326
            or logarithm.get("oracle") != "native Apple ARM64 std::log and std::fma; every non-NaN bit exact"
            or logarithm.get("profile") != "Apple ARM64 libm log compatibility; fused binary64 nearest-even"):
        raise ValueError("Native log/FMA coverage or exact oracle changed")
    failures = count(logarithm.get("failures"), "native log failures")
    if failures > logarithm["checks"] or duration(logarithm.get("gpu_timestamp_ms"), "native log GPU timestamp") <= 0:
        raise ValueError("Native log/FMA failures or GPU timing invalid")
    for category in (("arithmetic", "extended_arithmetic", "single_arithmetic") if include_single else ("arithmetic", "extended_arithmetic")):
        section = report.get(category, {})
        total = count(section.get("checks"), category + " checks")
        failed = count(section.get("failures"), category + " failures")
        if total == 0 or failed > total:
            raise ValueError("Invalid comparison denominator: " + category)
        operations = section.get("operations", [])
        expected_operations = {"arithmetic": 10, "extended_arithmetic": 7, "single_arithmetic": 12}[category]
        if (len(operations) != expected_operations or
                {r.get("operation") for r in operations} != set(range(expected_operations)) or
                any(count(r.get("checks"), "operation checks") == 0 or
                    count(r.get("failures"), "operation failures") > r["checks"] for r in operations) or
                sum(count(r.get("checks"), "operation checks") for r in operations) != total or
                sum(count(r.get("failures"), "operation failures") for r in operations) != failed):
            raise ValueError("Arithmetic operation totals disagree")
        failures += failed
    arithmetic = report["arithmetic"]
    if (count(arithmetic.get("power_two_division_checks"), "power-two division checks") != 218192
            or next(row["checks"] for row in arithmetic["operations"] if row["operation"] == 3) < 218192):
        raise ValueError("Targeted power-two division coverage is incomplete")
    failures += validate_division_seed_controls(report)
    # Every finite representable power, 26 boundary operands and four sign
    # combinations, separately with the power on the left and on the right.
    multiplication = 2 * POWER_TWO_MULTIPLICATION_PER_ORDER
    if (count(arithmetic.get("power_two_multiplication_checks"), "power-two multiplication checks") != multiplication
            or any(count(arithmetic.get("power_two_multiplication_" + side + "_checks"),
                         "power-two multiplication " + side + " checks") != POWER_TWO_MULTIPLICATION_PER_ORDER
                   for side in ("left", "right"))
            or next(row["checks"] for row in arithmetic["operations"] if row["operation"] == 2) < multiplication):
        raise ValueError("Targeted power-two multiplication coverage is incomplete")
    if any(count(arithmetic.get(key), key) != expected
           for key, expected in NORMAL_ARITHMETIC_COVERAGE.items()):
        raise ValueError("Normal arithmetic boundary coverage is incomplete")
    operation_minimums = {
        0: BINARY64_BASE_CHECKS_PER_OPERATION + NORMAL_ARITHMETIC_COVERAGE["normal_addition_boundary_add_checks"],
        1: BINARY64_BASE_CHECKS_PER_OPERATION + NORMAL_ARITHMETIC_COVERAGE["normal_addition_boundary_subtract_checks"],
        2: (BINARY64_BASE_CHECKS_PER_OPERATION + multiplication
            + NORMAL_ARITHMETIC_COVERAGE["normal_multiplication_boundary_checks"]),
    }
    if any(next(row["checks"] for row in arithmetic["operations"] if row["operation"] == op) < minimum
           for op, minimum in operation_minimums.items()):
        raise ValueError("Normal arithmetic per-operation denominator is incomplete")
    if (count(arithmetic.get("targeted_sqrt_checks"), "targeted sqrt checks") != 19703
            or count(arithmetic.get("independent_sqrt_checks"), "independent sqrt checks") != 11
            or next(row["checks"] for row in arithmetic["operations"] if row["operation"] == 4) < 19714):
        raise ValueError("Targeted production sqrt coverage is incomplete")
    failures += validate_sqrt_seed_controls(report)
    failures += validate_sqrt_wide_seed_controls(report)
    boxes = report.get("box_original_fixtures", {})
    if (boxes.get("supported_rows"), boxes.get("values"), boxes.get("unsupported_twist_rows")) != (1540, 30800, 70):
        raise ValueError("Original fixture rows or unsupported twist cases were omitted")
    for field, maximum in (("failed_rows", 1540), ("failed_values", 30800),
                           ("unsupported_rejection_failures", 70), ("class_failures", 30800),
                           ("signed_zero_failures", 30800), ("exact_bits", 30800)):
        if count(boxes.get(field), field) > maximum:
            raise ValueError("Invalid original fixture counter: " + field)
    if boxes.get("gate") != "2e-12 * max(1,abs(reference)); same nonfinite class and signed zero":
        raise ValueError("Original point acceptance gate changed")
    if (boxes["class_failures"] + boxes["signed_zero_failures"] > boxes["failed_values"] or
            boxes["failed_values"] > boxes["failed_rows"] * 20):
        raise ValueError("Original fixture failure totals disagree")
    failures += boxes["failed_rows"] + boxes["unsupported_rejection_failures"]
    return failures


def validate_geometry(comparison, width, height):
    if count(comparison.get("pixels"), "pixels") != width * height:
        raise ValueError("Pixel count disagrees with dimensions")
    hits = count(comparison.get("hits"), "hits")
    misses = count(comparison.get("misses"), "misses")
    unresolved = count(comparison.get("fault_pixels"), "fault pixels")
    if hits + misses != width * height or unresolved > width * height:
        raise ValueError("Hit or unresolved-pixel totals disagree")
    check_total = count(comparison.get("numeric_checks"), "numeric checks")
    if check_total == 0 or count(comparison.get("different_numeric_bits"), "different bits") > check_total:
        raise ValueError("Invalid scene numerical denominator")
    categories = ("camera_failures_2e_15", "geometry_failures_2e_12",
                  "normal_failures_2e_12_and_exact_packing", "discrete_failures", "fault_pixels",
                  "output_sanity_failures", "coverage_failures")
    if sum(count(comparison.get(key), key) for key in categories) != comparison.get("failures"):
        raise ValueError("Primary failure totals disagree")
    if (comparison.get("output_sanity_values") != 38 * width * height or
            comparison.get("nontrivial_hit_mask") is not (0 < hits < width * height) or
            comparison["coverage_failures"] != (0 if hits and misses else 1)):
        raise ValueError("Missing complete output sanity or nontrivial-geometry evidence")
    if count(comparison.get("signed_zero_failures"), "signed zero failures") > comparison["failures"]:
        raise ValueError("Signed zero failures were hidden from primary acceptance")
    if not 8 * width * height <= check_total <= 16 * width * height:
        raise ValueError("Incomplete primary numeric comparison count")
    if comparison["failures"] == 0 and check_total != 8 * width * height + 8 * hits:
        raise ValueError("Incomplete hit geometry comparison count")
    return comparison["failures"] + comparison["different_numeric_bits"]


def validate_report(report, width, height, threads, repeats, full_shading=False, hard_shadows=False,
                    shadow_mode="parallel", shadow_prep_threads=32, shadow_rows_per_command=256,
                    shadow_dispatch_sweep=False, shadow_trace_threads=32, primary_rows_per_command=256,
                    primary_threads=32, primary_dispatch_sweep=False, primary_tile_width=8,
                    scene_descriptor=DEFAULT_SCENE, normal_mode="parallel", normal_threads=32,
                    division_mode="verified"):
    if division_mode not in ("restoring", "verified") or report.get("division_mode") != division_mode:
        raise ValueError("Division execution mode differs from requested command")
    if full_shading:
        return shading_evidence.validate_report(report, width, height, threads, repeats, hard_shadows,
                                                validate_numeric_evidence, validate_geometry, shadow_mode,
                                                shadow_prep_threads, shadow_rows_per_command, shadow_dispatch_sweep,
                                                shadow_trace_threads, primary_rows_per_command, primary_threads,
                                                primary_dispatch_sweep, primary_tile_width, scene_descriptor,
                                                normal_mode, normal_threads)
    scene_manifest.validate_description(report, scene_descriptor)
    if hard_shadows or shadow_dispatch_sweep or primary_dispatch_sweep:
        raise ValueError("Hard shadows and dispatch sweeps require full shading")
    if (report.get("schema") != "mb3d.metal-exact-scene.v1" or report.get("numeric_profile") != scene_descriptor.profile()
            or report.get("status") not in ("PASS_SCOPED", "FAIL")):
        raise ValueError("Unexpected exact-primary report schema, profile or status")
    if (report.get("native_arm64") is not True or report.get("translated") is not False
            or report.get("cpu_compute_fallback_calls") != 0
            or report.get("runtime_shader_compilation") is not False or report.get("abi_checks") != 14):
        raise ValueError("Native ARM64, checked ABI, no CPU fallback and offline Metal required")
    primary = report.get("primary", {})
    scene_manifest.validate_identity(primary, scene_descriptor)
    settings = (primary.get("width"), primary.get("height"), primary.get("cpu_reference_threads"),
                primary.get("scene_sha256"))
    if settings != (width, height, threads, scene_descriptor.sha256):
        raise ValueError("Scene or execution settings differ from the requested command")
    if "images" in report and report["images"] != IMAGES:
        raise ValueError("Unexpected image paths")
    if "buffers" in report and report["buffers"] != BUFFERS:
        raise ValueError("Unexpected raw-buffer paths")
    failures = validate_numeric_evidence(report)
    failures += scene_manifest.validate_formula_rejections(primary.get("formula_rejections"), width * height, scene_descriptor)
    comparison = primary.get("comparison", {})
    scene_manifest.validate_geometry_limits(comparison, scene_descriptor)
    failures += validate_geometry(comparison, width, height)
    failures += primary_evidence.validate_normal_pipeline(
        report.get("normal_pipeline"), width, height, repeats, normal_mode, normal_threads,
        primary.get("gpu_timestamp_ms"), primary.get("first_gpu_timestamp_ms"))
    hits = comparison["hits"]; misses = comparison["misses"]; unresolved = comparison["fault_pixels"]
    if primary.get("complete") is not (unresolved == 0):
        raise ValueError("Unresolved pixels cannot count as complete")
    for field in ("gpu_repeats_bit_exact", "negative_controls_pass"):
        if type(primary.get(field)) is not bool:
            raise ValueError("Evidence flags must be Boolean")
    rejected = count(primary.get("abi_rejection_pixels"), "ABI rejection pixels")
    malformed = count(primary.get("malformed_rejection_pixels"), "malformed scene rejection pixels")
    exhausted = count(primary.get("budget_exhausted_pixels"), "budget exhaustion pixels")
    if (primary.get("malformed_variants") != 3 or malformed > 3 * width * height or
            rejected > width * height or exhausted > width * height or
            primary["negative_controls_pass"] != (rejected == width * height and
                                                  malformed == 3 * width * height and exhausted > 0)):
        raise ValueError("Negative controls disagree with completion evidence")
    rgb_differences = count(primary.get("diagnostic_rgb_pixel_differences"), "RGB differences")
    if rgb_differences > width * height:
        raise ValueError("Invalid RGB pixel counter")
    failures += primary_evidence.validate(primary.get("primary_dispatch"), width, height,
                                          primary_rows_per_command, primary_threads, False,
                                          primary_evidence.normal_geometry_maximum(report["normal_pipeline"]),
                                          primary_tile_width)
    passing = (failures == 0 and comparison["failures"] == 0 and
               primary["gpu_repeats_bit_exact"] and primary["negative_controls_pass"] and rgb_differences == 0)
    if (report["status"] == "PASS_SCOPED") != passing:
        raise ValueError("Qualification status disagrees with numeric/completion evidence")
    if passing and not 0 < hits < width * height:
        raise ValueError("Qualified scene must contain both geometry and background")
    for key in TIMINGS:
        samples = primary.get(key + "_ms")
        if not isinstance(samples, list) or len(samples) != repeats:
            raise ValueError("Missing timing repetitions: " + key)
        for sample in samples:
            duration(sample, key)
        reported = duration(primary.get(key + "_median_ms"), key + " median")
        if not math.isclose(reported, statistics.median(samples), rel_tol=1e-12, abs_tol=1e-12):
            raise ValueError("Incorrect reported timing median")
    duration(primary.get("cpu_production_primary_reference_observation_ms"), "CPU observation")
    return report


def gallery_html(report, images, descriptor=None):
    escape = lambda x: html.escape(str(x), quote=True)
    cards = []
    for row in images:
        label = "Metal GPU · первичная геометрия" if row["backend"] == "gpu" else "Основной CPU-код · независимая проверка"
        cards.append(f'<figure><a href="{escape(row["png"])}"><img src="{escape(row["png"])}" alt="{escape(label)}"></a><figcaption>{label}<br><a href="{escape(row["png"])}" download>Скачать PNG</a></figcaption></figure>')
    primary = report["primary"]
    descriptor = descriptor or scene_manifest.scene_for_report(primary)
    comparison = primary["comparison"]
    arithmetic = report["arithmetic"]
    extended = report["extended_arithmetic"]
    boxes = report["box_original_fixtures"]
    timing_rows = []
    for field, label in (("gpu_timestamp", "GPU: время команд"),
                         ("encode_submit_wait_readback", "GPU: отправка, ожидание и чтение")):
        timing_rows.append(f'<tr><td>{label}</td><td>{statistics.median(primary[field + "_ms"]):.3f} мс</td></tr>')
    return f'''<!doctype html><html lang="ru"><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1">
<title>Metal: точная арифметика и геометрия M3P</title><style>
body{{background:#11161e;color:#e8edf5;font:16px/1.5 system-ui;margin:0}}main{{max-width:1400px;margin:auto;padding:30px}}a{{color:#9bd1ff}}.images{{display:grid;grid-template-columns:1fr 1fr;gap:20px}}figure{{margin:0}}img{{width:100%;display:block}}figcaption{{padding:10px 0}}.notice{{border-left:4px solid #e9ba75;background:#29241c;padding:18px}}table{{border-collapse:collapse}}td,th{{padding:8px 18px;border-bottom:1px solid #374350;text-align:left}}@media(max-width:800px){{.images{{grid-template-columns:1fr}}main{{padding:16px}}}}
</style><main><h1>Точная арифметика Metal · реальная M3P-сцена</h1>
<p><strong>{escape(report['status'])}</strong> · {primary['width']}×{primary['height']} · {escape(Path(descriptor.path).name)} · {escape(report.get('device', 'Apple Silicon'))}</p>
<p class="notice">Это диагностическое изображение первичной геометрии: цвет показывает направление нормали. На GPU вычисляются лучи, формула {escape(descriptor.formula_name)}, расстояния, уточнение пересечения и нормали. Оригинальные цвета сцены, жёсткие тени, AO, отражения и DOF здесь не воспроизводятся. Это ещё не полный GPU-рендер сцены и не сравнение с изображением Pascal.</p>
<p>CPU подготавливает параметры M3P и таблицы синуса/косинуса для строк и столбцов. GPU сам строит каждый луч. Геометрия отдельно проверяется основными CPU-функциями проекта. Исходные проверки допусков сохранены; дополнительно требуется точное совпадение всех сравниваемых численных битов.</p>
<p>Проверки binary64: {arithmetic['checks']}, ошибок {arithmetic['failures']}; extended: {extended['checks']}, ошибок {extended['failures']}. Независимые точки boxIFS без Twist: {boxes['supported_rows']} строк / {boxes['values']} значений, ошибок {boxes['failed_values']}; {boxes['unsupported_twist_rows']} неподдержанных строк проверены на явный отказ, а не включены в численный PASS.</p>
<p>Первичная геометрия: {comparison['numeric_checks']} численных проверок, ошибок {comparison['failures']}. Незавершённые пиксели: {comparison['fault_pixels']}. Попадания: {comparison['hits']}; фон: {comparison['pixels'] - comparison['hits']}. Различающиеся биты численных значений: {comparison['different_numeric_bits']}; для PASS этот счётчик должен быть нулевым. Все 176 байт каждой первичной записи CPU/GPU также сравниваются точно.</p>
<p>Отказы для неизвестной формулы, чужой capability, изменённого параметра, ненулевого reserved и несовместимой специализации: {primary["formula_rejections"]["checks"]} проверок, ошибок {primary["formula_rejections"]["failures"]}.</p>
<p>Явный отказ для dIFS без выбранного минимума: {primary["formula_rejections"]["no_minimum_checks"]} проверок, ошибок {primary["formula_rejections"]["no_minimum_failures"]}. Эти случаи требуют состояния предыдущего CPU-вызова и не считаются точным совпадением рендера.</p>
{primary_evidence.gallery_html(primary["primary_dispatch"])}
{primary_evidence.normal_gallery_html(report["normal_pipeline"], report["division_mode"])}
<div class="images">{''.join(cards)}</div><h2>Измерения</h2>
<p>Медианы {len(primary['gpu_timestamp_ms'])} измерений GPU. CPU-проверка заняла {primary['cpu_production_primary_reference_observation_ms']:.3f} мс в одном запуске и включает дополнительные диагностические выборки. Сравнение скорости с ней не приводится; это не измерение ускорения полного рендера.</p>
<table><tr><th>Граница измерения</th><th>Медиана</th></tr>{''.join(timing_rows)}</table>
<p><a href="probe/report.json">Численные результаты</a> · <a href="summary.json">Происхождение и проверка файлов</a> · <a href="inputs/scene.m3p">Исходная M3P-сцена</a></p>
</main></html>'''



def describe_scene(probe, scene, width, height, threads, full_shading=False, hard_shadows=False, timeout=60):
    """Run only native preparation; bind its result to the exact selected bytes."""
    digest = sha(scene)
    command = [str(probe), "--describe", "--scene", str(scene), "--width", str(width),
               "--height", str(height), "--threads", str(threads)]
    if full_shading:
        command.append("--full-shading")
    if hard_shadows:
        command.append("--hard-shadows")
    env = {key: value for key, value in os.environ.items() if not key.upper().startswith("MB3D_")}
    process = subprocess.run(command, capture_output=True, text=True, check=False,
                             timeout=min(timeout, 60), cwd=ROOT, env=env)
    if process.returncode:
        raise ValueError("Native scene preparation rejected the input: " + process.stderr.strip())
    if sha(scene) != digest:
        raise ValueError("Scene changed during native preparation")
    try:
        description = json.loads(process.stdout)
    except json.JSONDecodeError as error:
        raise ValueError("Native scene description was not valid JSON") from error
    descriptor = scene_manifest.scene_from_description(description, digest, scene)
    return descriptor, description, command

def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--probe", type=Path, required=True)
    parser.add_argument("--library", type=Path, required=True)
    parser.add_argument("--scene", type=Path, default=SCENE)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--width", type=int, default=320)
    parser.add_argument("--height", type=int, default=240)
    parser.add_argument("--threads", type=int, default=6)
    parser.add_argument("--repeats", type=int, default=3)
    parser.add_argument("--timeout", type=float, default=900)
    parser.add_argument("--full-shading", action="store_true")
    parser.add_argument("--hard-shadows", action="store_true")
    parser.add_argument("--shadow-mode", choices=("legacy", "prepared", "parallel"))
    parser.add_argument("--shadow-prep-threads", type=int)
    parser.add_argument("--shadow-trace-threads", type=int)
    parser.add_argument("--shadow-rows-per-command", type=int)
    parser.add_argument("--shadow-dispatch-sweep", action="store_true")
    parser.add_argument("--primary-rows-per-command", type=int, default=256)
    parser.add_argument("--primary-threads", type=int, default=32)
    parser.add_argument("--primary-tile-width", type=int, default=8,
                        help="0 selects a linear group; otherwise a positive factor of --primary-threads")
    parser.add_argument("--primary-dispatch-sweep", action="store_true")
    parser.add_argument("--normal-mode", choices=("inline", "parallel"), default="parallel")
    parser.add_argument("--normal-threads", type=int, default=32)
    parser.add_argument("--division-mode", choices=("restoring", "verified"), default="verified")
    parser.add_argument("--build-evidence", type=Path, action="append", default=[])
    args = parser.parse_args(argv)
    if (not 16 <= args.width <= 4096 or not 16 <= args.height <= 4096 or not 1 <= args.threads <= 64
            or args.threads > args.height or not 3 <= args.repeats <= 15 or not math.isfinite(args.timeout) or args.timeout <= 0):
        parser.error("Dimensions 16..4096, threads 1..64, repeats 3..15 and positive timeout required")
    if not args.full_shading and (args.hard_shadows or args.shadow_mode is not None
                                  or args.shadow_prep_threads is not None or args.shadow_trace_threads is not None
                                  or args.shadow_rows_per_command is not None
                                  or args.shadow_dispatch_sweep):
        parser.error("Shadow options require --full-shading")
    if args.primary_dispatch_sweep and not args.full_shading:
        parser.error("--primary-dispatch-sweep requires --full-shading")
    if not 1 <= args.primary_rows_per_command <= 4096 or not 1 <= args.primary_threads <= 1024:
        parser.error("Primary rows must be 1..4096 and threads 1..1024; the probe additionally validates actual GPU limits")
    if not 1 <= args.normal_threads <= 1024:
        parser.error("Normal threads must be 1..1024; the probe additionally validates actual GPU limits")
    if (args.primary_tile_width < 0 or args.primary_tile_width > args.primary_threads
            or (args.primary_tile_width and args.primary_threads % args.primary_tile_width)):
        parser.error("Primary tile width must be zero or a positive factor of --primary-threads")
    args.shadow_mode = args.shadow_mode or "parallel"
    args.shadow_prep_threads = 32 if args.shadow_prep_threads is None else args.shadow_prep_threads
    args.shadow_trace_threads = 32 if args.shadow_trace_threads is None else args.shadow_trace_threads
    args.shadow_rows_per_command = 256 if args.shadow_rows_per_command is None else args.shadow_rows_per_command
    if args.shadow_dispatch_sweep and (not args.hard_shadows or args.shadow_mode != "prepared"):
        parser.error("--shadow-dispatch-sweep requires --full-shading --hard-shadows and prepared shadow mode")
    if (not 1 <= args.shadow_prep_threads <= 4096 or not 1 <= args.shadow_trace_threads <= 4096
            or not 1 <= args.shadow_rows_per_command <= 4096):
        parser.error("Shadow preparation/trace threads and rows per command must be 1..4096; the probe additionally validates actual GPU limits")
    try:
        host = native_host()
        probe = args.probe.expanduser().resolve(strict=True)
        library = args.library.expanduser().resolve(strict=True)
        scene = args.scene.expanduser().resolve(strict=True)
        identity = binary_identity(probe)
        evidence = [p.expanduser().resolve(strict=True) for p in args.build_evidence]
        if library.suffix != ".metallib" or not library.is_file():
            raise ValueError("An offline compiled .metallib is required")
        descriptor, description, description_command = describe_scene(
            probe, scene, args.width, args.height, args.threads,
            args.full_shading, args.hard_shadows, args.timeout)
        output = args.output.expanduser().resolve()
        if output.is_relative_to(ROOT):
            raise ValueError("Final images must be retained outside the source checkout")
        output.mkdir(parents=True, exist_ok=False)
    except (ValueError, OSError, subprocess.SubprocessError) as error:
        parser.error(str(error))
    summary = {"schema": shading_evidence.SCHEMA if args.full_shading else SCHEMA, "status": "RUNNING", "started_utc": utc_now(), "host": host,
               "numeric_profile": descriptor.profile(args.full_shading),
               "scope": descriptor.scope(args.full_shading),
               "settings": {"width": args.width, "height": args.height, "threads": args.threads,
                            "repeats": args.repeats, "timeout_seconds": args.timeout,
                            "full_shading": args.full_shading, "hard_shadows": args.hard_shadows,
                            "shadow_mode": args.shadow_mode if args.full_shading else None,
                            "shadow_prep_threads": args.shadow_prep_threads if args.full_shading else None,
                            "shadow_trace_threads": args.shadow_trace_threads if args.full_shading else None,
                            "shadow_rows_per_command": args.shadow_rows_per_command if args.full_shading else None,
                            "shadow_dispatch_sweep": args.shadow_dispatch_sweep,
                            "primary_rows_per_command": args.primary_rows_per_command,
                            "primary_threads": args.primary_threads,
                            "primary_tile_width": args.primary_tile_width,
                            "normal_mode": args.normal_mode,
                            "normal_threads": args.normal_threads,
                            "division_mode": args.division_mode,
                            "primary_dispatch_sweep": args.primary_dispatch_sweep},
               "probe": identity, "library": {"path": str(library), "sha256": sha(library)},
               "scene": {"path": str(scene), "sha256": descriptor.sha256, **descriptor.report_identity(),
                         "catalog_path": descriptor.path, "manifest_sha256": sha(scene_manifest.MANIFEST)},
               "native_scene_description": description, "description_command": description_command,
               "source_observation": source_identity(),
               "source_identity_note": "The source snapshot and build files document provenance; checkout observation alone does not prove how a preexisting binary was built.",
               "artifacts": {}, "images": []}
    for name in ("evidence/bin", "evidence/source", "evidence/build", "images", "inputs"):
        (output / name).mkdir(parents=True)
    (output / "KEEP_FINAL_IMAGES.txt").write_text(
        "Preserve final images and gallery before reusing this working directory. "
        "Temporary buffers, copied binaries/sources and test logs are disposable after verification.\n")
    write_json(output / "summary.json", summary)

    def retain(source, relative):
        target = output / relative
        target.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(source, target)
        if sha(source) != sha(target):
            raise ValueError("Input changed while snapshotting: " + str(source))
        summary["artifacts"][relative] = sha(target)
        return target

    try:
        write_json(output / "evidence/scene-description.json", description)
        binary = retain(probe, "evidence/bin/metal_exact_scene")
        metal = retain(library, "evidence/bin/metal_exact_scene.metallib")
        source_scene = retain(scene, "inputs/scene.m3p")
        if sha(binary) != identity["sha256"] or sha(metal) != summary["library"]["sha256"] or sha(source_scene) != descriptor.sha256:
            raise ValueError("Executable, library or scene changed after identity capture")
        sources = {"CMakeLists.txt", "comparison/tools/run_metal_exact_scene.py",
                   "comparison/tools/test_run_metal_exact_scene.py", "comparison/tools/metal_shading_evidence.py",
                   "comparison/tools/run_metal_formula_probe.py", "comparison/tools/primary_dispatch_evidence.py",
                   "comparison/tools/benchmark_gallery_arm64.py", "comparison/tools/metal_scene_manifest.py",
                   "scripts/native_evidence.py",
                   "external_formulas/Formulas/dIFS/SphereIFS.inc", "external_formulas/Formulas/dIFS/boxIFS.inc",
                   "mb3d/M3Formulas/SphereIFS.m3f", "mb3d/M3Formulas/boxIFS.m3f",
                   "tests/fixtures/boxifs_regressions.h", "tests/fixtures/boxifs_twist_regressions.h"}
        sources.update(str(p.relative_to(ROOT)) for p in (ROOT / "comparison/metal_exact").rglob("*") if p.is_file())
        sources.update(str(p.relative_to(ROOT)) for p in (ROOT / "comparison/tools").glob("*metal_exact*.mm"))
        sources.update(name for name, digest in summary["source_observation"]["changed_source_sha256"].items() if digest is not None)
        for name in sorted(sources):
            retain(ROOT / name, "evidence/source/" + name)
        (output / "evidence/source/working-tree.patch").write_bytes(subprocess.check_output(["git", "diff", "HEAD", "--binary"], cwd=ROOT))
        for relative in ("CMakeCache.txt", "compile_commands.json", "build.ninja"):
            if (probe.parent / relative).is_file():
                retain(probe.parent / relative, "evidence/build/" + relative)
        for target in ("mb3d_metal_exact_scene", "mb3d_metal_exact_shaders", "mb3d_tests_metal_exact_scene_host"):
            for filename in ("flags.make", "link.txt", "build.make"):
                relative = f"CMakeFiles/{target}.dir/{filename}"
                if (probe.parent / relative).is_file():
                    retain(probe.parent / relative, "evidence/build/" + relative)
        for index, path in enumerate(evidence):
            retain(path, f"evidence/build/user-{index:02}-" + path.name)
        env = {key: value for key, value in os.environ.items() if not key.upper().startswith("MB3D_")}
        summary["removed_environment_names"] = sorted(set(os.environ) - set(env))
        command = [str(binary), "--library", str(metal), "--scene", str(source_scene),
                   "--output", str(output / "probe"), "--width", str(args.width), "--height", str(args.height),
                   "--threads", str(args.threads), "--repeats", str(args.repeats),
                   "--primary-rows-per-command", str(args.primary_rows_per_command),
                   "--primary-threads", str(args.primary_threads),
                   "--primary-tile-width", str(args.primary_tile_width),
                   "--normal-mode", args.normal_mode, "--normal-threads", str(args.normal_threads),
                   "--division-mode", args.division_mode]
        if args.full_shading:
            command.extend(["--full-shading", "--shadow-mode", args.shadow_mode,
                            "--shadow-prep-threads", str(args.shadow_prep_threads),
                            "--shadow-trace-threads", str(args.shadow_trace_threads),
                            "--shadow-rows-per-command", str(args.shadow_rows_per_command)])
        if args.hard_shadows:
            command.append("--hard-shadows")
        if args.shadow_dispatch_sweep:
            command.append("--shadow-dispatch-sweep")
        if args.primary_dispatch_sweep:
            command.append("--primary-dispatch-sweep")
        summary["command"] = command; summary["cwd"] = str(output)
        write_json(output / "summary.json", summary)
        print("Native exact Metal scene: " + str(output), flush=True)
        summary["process"] = execute(command, output, output / "evidence/probe.log", args.timeout, env)
        if summary["process"]["timed_out"]:
            raise ValueError("Probe timed out; incomplete output retained")
        report = validate_report(json.loads((output / "probe/report.json").read_text()), args.width, args.height, args.threads, args.repeats, args.full_shading, args.hard_shadows,
                                 args.shadow_mode, args.shadow_prep_threads, args.shadow_rows_per_command,
                                 args.shadow_dispatch_sweep, args.shadow_trace_threads, args.primary_rows_per_command,
                                 args.primary_threads, args.primary_dispatch_sweep, args.primary_tile_width, descriptor,
                                 args.normal_mode, args.normal_threads, args.division_mode)
        if args.full_shading:
            shading_evidence.validate_buffers(output / "probe", report, args.width, args.height)
        else:
            primary_evidence.validate_buffers(output / "probe", report["primary"]["primary_dispatch"],
                                               args.width, args.height)
            primary_evidence.validate_normal_buffers(output / "probe", report["normal_pipeline"],
                                                      args.width, args.height)
            shading_evidence.validate_primary_cpu_buffers(output / "probe", report, args.width, args.height)
        image_paths = shading_evidence.IMAGES if args.full_shading else IMAGES
        for backend in ("gpu", "cpu"):
            raw = output / "probe" / BUFFERS[backend]
            if raw.stat().st_size != args.width * args.height * PIXEL_BYTES:
                raise ValueError("Raw primary buffer size does not match the versioned ABI")
            ppm = output / "probe" / image_paths[backend]
            png = output / "images" / (ppm.stem + ".png")
            image = convert_ppm_losslessly(ppm, png, args.width, args.height)
            summary["images"].append({"backend": backend, "ppm": str(ppm.relative_to(output)), "png": str(png.relative_to(output)), **image})
        if args.full_shading:
            shading_evidence.validate_final_images(output / "probe", report, args.width, args.height)
        timing_section = report["shading"] if args.full_shading else report["primary"]
        timing_fields = shading_evidence.TIMINGS if args.full_shading else TIMINGS
        summary["timing_medians_ms"] = {field: statistics.median(timing_section[field + "_ms"]) for field in timing_fields}
        summary["probe_report"] = "probe/report.json"
        (output / "index.html").write_text((shading_evidence.gallery_html if args.full_shading else gallery_html)(report, summary["images"], descriptor))
        if summary["process"]["returncode"] != (0 if report["status"] == "PASS_SCOPED" else 1):
            raise ValueError("Process exit status disagrees with its report")
        summary["status"] = report["status"]
    except KeyboardInterrupt:
        summary["status"] = "INTERRUPTED"
    except Exception as error:
        summary.update(status="FAILED", error=str(error))
    summary["finished_utc"] = utc_now()
    summary["source_observation_at_end"] = source_identity()
    if summary["status"] == "PASS_SCOPED" and summary["source_observation_at_end"] != summary["source_observation"]:
        summary.update(status="FAILED_SOURCE_CHANGED", error="Checkout source changed during the evidence run")
    for path in sorted(output.rglob("*")):
        if path.is_file() and path.name not in ("summary.json", "summary.json.tmp"):
            summary["artifacts"][str(path.relative_to(output))] = sha(path)
    write_json(output / "summary.json", summary)
    print(json.dumps({"status": summary["status"], "summary": str(output / "summary.json"), "images": len(summary["images"])}))
    return 0 if summary["status"] == "PASS_SCOPED" else 1


if __name__ == "__main__":
    raise SystemExit(main())
