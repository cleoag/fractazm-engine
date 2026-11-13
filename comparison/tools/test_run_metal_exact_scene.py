"""Fail-closed artifact validation; these tests do not dispatch GPU work."""
from pathlib import Path
import tempfile
import unittest
import contextlib
import io
import copy
import struct
import json
from unittest import mock
from types import SimpleNamespace
from dataclasses import replace

from PIL import Image
import run_metal_exact_scene as exact


def valid_primary_dispatch(width=16, height=16, rows=256, threads=32, sweep=False, maximum=1024, tile_width=8):
    pixels = width * height
    tile = tile_width or threads
    result = {"pass": True, "rows_per_command": rows, "threads_per_threadgroup": threads,
              "tile_width": tile, "tile_height": threads // tile,
              "commands_per_frame": (height + rows - 1) // rows, "grid_width": width,
              "grid_height": min(rows, height), "thread_execution_width": 32,
              "max_total_threads_per_threadgroup": maximum,
              "max_command_gpu_ms": 5 / ((height + rows - 1) // rows),
              "baseline": {"rows_per_command": 8, "threads_per_threadgroup": 32, "pixels": pixels,
                           "tile_width": 32, "tile_height": 1,
                           "different_pixels": 0, "fault_pixels": 0,
                           "gate": exact.primary_evidence.BASELINE_GATE},
              "negative_controls": {"cases": 4, "checks": 4*pixels, "failures": 0, "pass": True}}
    if sweep:
        result["trials"] = {"warmup_runs": 1, "timed_runs": 3, "records_per_run": pixels,
            "configurations": [{"rows_per_command": row, "threads_per_threadgroup": group,
                                "tile_width": tile, "tile_height": group // tile,
                                "gpu_ms": [3, 1, 2], "median_gpu_ms": 2, "warmup_gpu_ms": 5,
                                "max_command_gpu_ms": 5 / ((height + row - 1) // row),
                                "different_records": 0, "fault_records": 0,
                                "checked_records": 4*pixels, "record_bytes": 176}
                               for row, group, tile in exact.primary_evidence.configuration_shapes(height, rows, 32, maximum)]}
    return result


def valid_sqrt_seed_controls():
    fixed = {"exact":19712, "plus_one":19712, "minus_one":19702, "minus_four":19702,
             "plus_four":19000, "minus_five":0, "plus_five":0, "zero":10, "limit":100,
             "uint64_max":0, "random32":0, "random64":0, "invalid_high":0}
    return {"pass":True, "matrix_inputs":19712, "seed_variants":12, "invalid_high_checks":6,
            "records":236550, "checks":473100, "sqrt_checks":236550, "prefix_checks":236550,
            "nan_class_checks":84, "signed_zero_checks":24, "input_bytes":24, "record_bytes":32,
            "failures":0, "gpu_timestamp_ms":2, "gate":exact.SQRT_SEED_GATE, "oracle":exact.SQRT_SEED_ORACLE,
            "cases":[{"case":name, "records":6 if name=="invalid_high" else 19712,
                      "sqrt_failures":0, "prefix_failures":0, "accepted_prefixes":fixed[name],
                      "rejected_prefixes":(6 if name=="invalid_high" else 19712)-fixed[name]}
                     for name in exact.SQRT_SEED_CASES]}


def valid_sqrt_wide_seed_controls():
    result = {"pass":True, "matrix_inputs":19712, "boundary_prefix_inputs":169, "seed_variants":20,
              "clamp_checks":16, "records":397636, "checks":795272, "sqrt_checks":397636,
              "prefix_checks":397636, "nan_class_checks":140, "signed_zero_checks":40,
              "input_bytes":24, "record_bytes":32, "failures":0, "gpu_timestamp_ms":2,
              "gate":exact.SQRT_WIDE_GATE, "oracle":exact.SQRT_WIDE_ORACLE, "cases":[]}
    for name, accepts, rejects in zip(exact.SQRT_WIDE_CASES, exact.SQRT_WIDE_REQUIRED_ACCEPTS,
                                     exact.SQRT_WIDE_REQUIRED_REJECTIONS):
        records = 16 if name == "root_max_clamp" else 19881
        result["cases"].append({"case":name, "records":records, "required_accepts":accepts,
                                "required_rejections":rejects, "optional_prefixes":records-accepts-rejects,
                                "accepted_prefixes":accepts, "rejected_prefixes":records-accepts,
                                "sqrt_failures":0, "prefix_failures":0})
    return result


def valid_normal_pipeline(width=16, height=16, mode="parallel", threads=32):
    separate = 1 if mode == "parallel" else 0
    return {"pass": True, "mode": mode, "pixels": width*height, "scratch_record_bytes": 64,
            "probe_jobs": width*height*6 if mode == "parallel" else 0, "threads_per_threadgroup": threads,
            "pipeline_limits": {name: {"thread_execution_width": 32, "max_total_threads_per_threadgroup": 1024}
                                for name in ("probes", "finish")},
            "baseline": {"pixels": width*height, "different_pixels": 0, "fault_pixels": 0,
                         "gate": exact.primary_evidence.NORMAL_BASELINE_GATE},
            "gpu_ms": {"geometry": [3, 1, 2], "probes": [separate]*3, "finish": [separate]*3},
            "median_gpu_ms": {"geometry": 2, "probes": separate, "finish": separate},
            "first_gpu_ms": {"geometry": 5, "probes": separate, "finish": separate},
            "controls": {
                "pass": True,
                "abi": {"checks": 6, "failures": 0, "values": [64, 0, 48, 56, 6, 176], "pass": True},
                "negative_controls": {"cases": 8, "checks": 240, "failures": 0, "pass": True},
                "synthetic": {"pixels": 15, "active_pixels": 5, "inactive_pixels": 10,
                              "different_pixels": 0, "inactive_sample_failures": 0, "guard_failures": 0, "pass": True}}}


def valid_division_seed_controls():
    cases = []
    for name in exact.DIVISION_SEED_CASES:
        records = 48 if name == "invalid_domain" else 14592
        accepted = records if name in ("exact", "minus_one", "plus_one", "minus_two", "plus_two", "hardware") else 0
        cases.append({"case": name, "records": records, "required_accepts": accepted,
                      "required_rejections": records-accepted, "accepted_digits": accepted,
                      "rejected_digits": records-accepted,
                      "correction_accepts": accepted if name in ("minus_one", "plus_one", "minus_two", "plus_two") else 0,
                      "value_failures": 0, "digit_failures": 0})
    return {"pass": True, "matrix_inputs": 14592, "seed_variants": 12, "invalid_domain_checks": 48,
            "records": 175152, "checks": 700608, "value_checks": 525456, "digit_checks": 175152,
            "input_bytes": 48, "record_bytes": 48, "failures": 0, "gpu_timestamp_ms": 2, "cases": cases,
            **{key: sum(row[key] for row in cases) for key in ("required_accepts", "required_rejections", "correction_accepts")},
            "gate": exact.DIVISION_SEED_GATE, "oracle": exact.DIVISION_SEED_ORACLE}


def valid_report():
    arithmetic = lambda n: {"checks": n * 20, "failures": 0,
                           "operations": [{"operation": i, "checks": 20, "failures": 0} for i in range(n)]}
    binary64 = {"checks": 10 * exact.BINARY64_BASE_CHECKS_PER_OPERATION, "failures": 0,
                "operations": [{"operation": i, "checks": exact.BINARY64_BASE_CHECKS_PER_OPERATION,
                                "failures": 0} for i in range(10)]}
    binary64.update(targeted_sqrt_checks=19703, independent_sqrt_checks=11)
    binary64["power_two_division_checks"] = 218192
    binary64["operations"][3]["checks"] += 218192
    binary64["checks"] += 218192
    binary64.update(power_two_multiplication_checks=436384,
                    power_two_multiplication_left_checks=218192,
                    power_two_multiplication_right_checks=218192)
    binary64["operations"][2]["checks"] += 436384
    binary64["checks"] += 436384
    binary64["operations"][4]["checks"] += 19714
    binary64["checks"] += 19714
    binary64.update(exact.NORMAL_ARITHMETIC_COVERAGE)
    for operation, field in ((0, "normal_addition_boundary_add_checks"),
                             (1, "normal_addition_boundary_subtract_checks"),
                             (2, "normal_multiplication_boundary_checks")):
        binary64["operations"][operation]["checks"] += exact.NORMAL_ARITHMETIC_COVERAGE[field]
        binary64["checks"] += exact.NORMAL_ARITHMETIC_COVERAGE[field]
    primary = {"width": 16, "height": 16, **exact.DEFAULT_SCENE.report_identity(),
               "formula_rejections": {"cases": 5, "checks": 1280, "failures": 0, "pass": True,
                                      "no_minimum_cases": 1, "no_minimum_checks": 256, "no_minimum_failures": 0},
               "cpu_reference_threads": 6, "complete": True, "gpu_repeats_bit_exact": True,
               "negative_controls_pass": True, "abi_rejection_pixels": 256,
               "malformed_rejection_pixels": 768, "malformed_variants": 3,
               "budget_exhausted_pixels": 37, "diagnostic_rgb_pixel_differences": 0,
               "cpu_production_primary_reference_observation_ms": 100,
               "first_gpu_timestamp_ms": 7,
               "primary_dispatch": valid_primary_dispatch(),
               "comparison": {"pixels": 256, "hits": 100, "misses": 156,
                              "result_kind_limit": 1, "final_iteration_limit": 60,
                              "nontrivial_hit_mask": True, "coverage_failures": 0,
                              "numeric_checks": 2848, "different_numeric_bits": 0,
                              "camera_failures_2e_15": 0, "geometry_failures_2e_12": 0,
                              "normal_failures_2e_12_and_exact_packing": 0,
                              "discrete_failures": 0, "fault_pixels": 0, "failures": 0,
                              "signed_zero_failures": 0, "output_sanity_failures": 0,
                              "output_sanity_values": 256*38}}
    for field in exact.TIMINGS:
        primary[field + "_ms"] = [3, 1, 2]
        primary[field + "_median_ms"] = 2
    primary["gpu_timestamp_ms"] = [5, 3, 4]
    primary["gpu_timestamp_median_ms"] = 4
    return {"schema": "mb3d.metal-exact-scene.v1", "status": "PASS_SCOPED",
            "numeric_profile": exact.PROFILE, "native_arm64": True, "translated": False,
            "cpu_compute_fallback_calls": 0, "runtime_shader_compilation": False,
            "scene_abi_version": 4, "scene_bytes": 336, "pixel_bytes": 176,
            "scene_description": exact.DEFAULT_SCENE.description(),
            "pipeline_creation_ms": 10,
            "division_mode": "verified", "normal_pipeline": valid_normal_pipeline(),
            "division_seed_controls": valid_division_seed_controls(),
            "pipeline_specialization": {"formula_kind": 1, "builtin_power": 0, "capability_id": 1},
            "abi_checks": 14, "arithmetic": binary64, "extended_arithmetic": arithmetic(7),
            "native_log": {"checks": 441307, "log_checks": 216981, "fma_checks": 224326,
                           "failures": 0, "gpu_timestamp_ms": 2,
                           "oracle": "native Apple ARM64 std::log and std::fma; every non-NaN bit exact",
                           "profile": "Apple ARM64 libm log compatibility; fused binary64 nearest-even"},
            "sqrt_seed_controls": valid_sqrt_seed_controls(),
            "sqrt_wide_seed_controls": valid_sqrt_wide_seed_controls(),
            "box_original_fixtures": {"supported_rows": 1540, "values": 30800,
                                      "unsupported_twist_rows": 70, "failed_rows": 0,
                                      "failed_values": 0, "unsupported_rejection_failures": 0,
                                      "class_failures": 0, "signed_zero_failures": 0, "exact_bits": 30800,
                                      "gate": "2e-12 * max(1,abs(reference)); same nonfinite class and signed zero"},
            "primary": primary, "images": dict(exact.IMAGES), "buffers": dict(exact.BUFFERS)}


class ExactSceneEvidenceTests(unittest.TestCase):
    def validate(self, report):
        return exact.validate_report(report, 16, 16, 6, 3)

    def test_declared_scope_passes(self):
        self.assertEqual(self.validate(valid_report())["status"], "PASS_SCOPED")

    def test_native_offline_no_fallback_required(self):
        for field, value in (("native_arm64", False), ("translated", True),
                             ("cpu_compute_fallback_calls", 1), ("runtime_shader_compilation", True),
                             ("abi_checks", 0), ("numeric_profile", "FP32")):
            report = valid_report(); report[field] = value
            with self.subTest(field=field), self.assertRaises(ValueError):
                self.validate(report)

    def test_exact_arithmetic_cannot_be_missing_or_have_zero_denominator(self):
        for category in ("arithmetic", "extended_arithmetic"):
            for value in ({}, {"checks": 0, "failures": 0, "operations": []}):
                report = valid_report(); report[category] = value
                with self.subTest(category=category), self.assertRaises(ValueError):
                    self.validate(report)

    def test_arithmetic_failure_is_not_promoted_to_pass(self):
        report = valid_report()
        report["extended_arithmetic"]["failures"] = 1
        report["extended_arithmetic"]["operations"][0]["failures"] = 1
        with self.assertRaisesRegex(ValueError, "Qualification"):
            self.validate(report)
        report["status"] = "FAIL"
        self.validate(report)

    def test_each_operation_has_independent_checks(self):
        report = valid_report()
        report["arithmetic"]["operations"][0]["checks"] = 0
        report["arithmetic"]["checks"] -= 20
        with self.assertRaises(ValueError):
            self.validate(report)

    def test_threads_cannot_exceed_height(self):
        with contextlib.redirect_stderr(io.StringIO()), self.assertRaises(SystemExit) as error:
            exact.main(["--probe", "missing", "--library", "missing.metallib", "--output", "unused",
                        "--height", "16", "--threads", "64"])
        self.assertEqual(error.exception.code, 2)

    def test_original_fixtures_and_unsupported_cases_cannot_be_dropped(self):
        for field, value in (("supported_rows", 1539), ("values", 30799),
                             ("unsupported_twist_rows", 0), ("gate", "2e-10")):
            report = valid_report(); report["box_original_fixtures"][field] = value
            with self.subTest(field=field), self.assertRaises(ValueError):
                self.validate(report)

    def test_class_and_signed_zero_errors_cannot_hide_in_point_pass(self):
        for field in ("class_failures", "signed_zero_failures", "failed_values"):
            report = valid_report(); report["box_original_fixtures"][field] = 1
            with self.subTest(field=field), self.assertRaises(ValueError):
                self.validate(report)

    def test_scene_and_requested_settings_are_bound(self):
        for field, value in (("scene_sha256", "0"*64), ("width", 32),
                             ("height", 32), ("cpu_reference_threads", 4)):
            report = valid_report(); report["primary"][field] = value
            with self.subTest(field=field), self.assertRaises(ValueError):
                self.validate(report)

    def test_numeric_bit_differences_fail_even_inside_existing_tolerances(self):
        report = valid_report()
        report["primary"]["comparison"]["different_numeric_bits"] = 1
        with self.assertRaisesRegex(ValueError, "Qualification"):
            self.validate(report)
        report["status"] = "FAIL"
        self.validate(report)

    def test_zero_geometry_and_unchecked_output_are_rejected(self):
        for change in (lambda p: p.update(hits=0, misses=256),
                       lambda p: p.update(output_sanity_values=0),
                       lambda p: p.update(numeric_checks=0),
                       lambda p: p.update(signed_zero_failures=1)):
            report = valid_report(); change(report["primary"]["comparison"])
            with self.assertRaises(ValueError):
                self.validate(report)

    def test_faults_stay_failed_and_incomplete(self):
        report = valid_report(); primary = report["primary"]
        primary["comparison"].update(fault_pixels=1, failures=1)
        report["status"] = "FAIL"
        with self.assertRaisesRegex(ValueError, "complete"):
            self.validate(report)
        primary["complete"] = False
        self.validate(report)
        report["status"] = "PASS_SCOPED"
        with self.assertRaisesRegex(ValueError, "Qualification"):
            self.validate(report)

    def test_failed_negative_controls_and_nonrepeatable_pixels_are_rejected(self):
        for change in (lambda p: p.update(abi_rejection_pixels=255),
                       lambda p: p.update(malformed_rejection_pixels=767),
                       lambda p: p.update(malformed_variants=2),
                       lambda p: p.update(budget_exhausted_pixels=0),
                       lambda p: p.update(gpu_repeats_bit_exact=False),
                       lambda p: p.update(diagnostic_rgb_pixel_differences=1)):
            report = valid_report(); change(report["primary"])
            with self.assertRaises(ValueError):
                self.validate(report)

    def test_timing_median_samples_and_finiteness(self):
        for change in (lambda p: p.update(gpu_timestamp_median_ms=3),
                       lambda p: p.update(gpu_timestamp_ms=[1, 2]),
                       lambda p: p.update(gpu_timestamp_ms=[1, float("nan"), 2]),
                       lambda p: p.update(cpu_production_primary_reference_observation_ms=-1)):
            report = valid_report(); change(report["primary"])
            with self.assertRaises(ValueError):
                self.validate(report)
        report = valid_report()
        for field in exact.TIMINGS:
            report["primary"][field+"_ms"] = [4, 2, 1, 3]
            report["primary"][field+"_median_ms"] = 2.5
        report["normal_pipeline"]["gpu_ms"] = {"geometry": [3.5, 1.5, .5, 2.5],
                                                "probes": [.25]*4, "finish": [.25]*4}
        report["normal_pipeline"]["median_gpu_ms"] = {"geometry": 2, "probes": .25, "finish": .25}
        exact.validate_report(report, 16, 16, 6, 4)

    def test_report_cannot_escape_image_or_buffer_paths(self):
        for field in ("images", "buffers"):
            report = valid_report(); report[field]["gpu"] = "../escape"
            with self.subTest(field=field), self.assertRaises(ValueError):
                self.validate(report)

    def test_gallery_describes_normal_visualization_and_single_cpu_observation(self):
        report = valid_report(); report["device"] = "<script>bad()</script>"
        markup = exact.gallery_html(report, [{"backend": "gpu", "png": "images/gpu.png"}])
        self.assertIn("цвет показывает направление нормали", markup)
        self.assertIn("ещё не полный GPU-рендер", markup)
        self.assertIn("в одном запуске", markup)
        self.assertIn("&lt;script&gt;", markup)
        self.assertNotIn("<script>", markup)

    def test_png_conversion_preserves_bits_and_existing_files(self):
        with tempfile.TemporaryDirectory() as folder:
            root = Path(folder); source = root/"a.ppm"; png = root/"a.png"
            data = bytes([0,1,255,250,2,128,45,74,90,70,12,8])
            Image.frombytes("RGB", (2,2), data).save(source)
            exact.convert_ppm_losslessly(source, png, 2, 2)
            with Image.open(png) as image:
                self.assertEqual(image.tobytes(), data)
            with self.assertRaises(FileExistsError):
                exact.convert_ppm_losslessly(source, png, 2, 2)


def valid_shading_report(hard_shadows=False, shadow_mode="prepared", prep_threads=32, rows=256, sweep=False,
                         trace_threads=32):
    report = valid_report()
    report.update(schema="mb3d.metal-exact-shading.v1", numeric_profile=exact.shading_evidence.PROFILE,
                  abi_checks=24, scene_preparation_ms=1, device_library_queue_setup_ms=2,
                  hardware={"unified_memory": True, "max_buffer_length": 2**32,
                            "recommended_max_working_set_size": 2**33,
                            "max_threads_per_threadgroup": [1024, 1024, 1024]})
    report["single_arithmetic"] = {"checks": 240, "failures": 0,
        "operations": [{"operation": i, "checks": 20, "failures": 0} for i in range(12)]}
    report.pop("images"); report.pop("buffers")
    p = report.pop("primary")
    surface = {"pixels": 256, "different_pixels": 0, "fault_pixels": 0,
               "gate": exact.shading_evidence.SURFACE_GATE}
    color = {"pixels": 256, "channels": 768, "different_pixels": 0,
             "different_channels": 0, "max_channel_error": 0, "fault_pixels": 0,
             "gate": exact.shading_evidence.COLOR_GATE}
    shading = {"width": 16, "height": 16, "cpu_reference_threads": 6,
               **exact.DEFAULT_SCENE.report_identity(), "hard_shadows_override": hard_shadows,
               "formula_rejections": p["formula_rejections"],
               "hard_shadows_enabled": hard_shadows, "primary": p["comparison"],
               "primary_dispatch": p["primary_dispatch"],
               "negative_controls": {"pass": True, "checks": 5636, "failures": 0, "cases": 26},
               "trace_skip_controls": {"pass": True, "checks": 12, "failures": 0, "cases": 6},
               "shadow_mode": shadow_mode, "shadow_prep_threads_per_threadgroup": prep_threads,
               "shadow_trace_threads_per_threadgroup": trace_threads,
               "shadow_rows_per_command": rows, "max_shadow_prepare_command_ms": 4,
               "max_shadow_trace_command_ms": 4 if shadow_mode == "parallel" else 0,
               "max_hard_shadow_command_ms": 4,
               "shadow_preparation": {"pixels": 256, "different_pixels": 0, "fault_pixels": 0,
                                      "active_rays": 1 if hard_shadows else 0,
                                      "gate": exact.shading_evidence.RAY_GATE},
               "shadow_traversal": {"pixels": 256, "different_pixels": 0, "fault_pixels": 0,
                                    "active_rays": 1 if hard_shadows else 0, "terminal_fault_pixels": 0,
                                    "gate": exact.shading_evidence.TRACE_GATE},
               "pipeline_limits": {
                   "shadow_prepare": {"thread_execution_width": 32, "max_total_threads_per_threadgroup": 1024,
                                      "threads_per_threadgroup": prep_threads, "grid_threads": 256},
                   "shadow_trace": {"thread_execution_width": 32, "max_total_threads_per_threadgroup": 1024,
                                    "threads_per_threadgroup": trace_threads, "grid_threads": 256},
                   "shadow_march": {"thread_execution_width": 32, "max_total_threads_per_threadgroup": 1024,
                                    "threads_per_threadgroup": 32, "grid_threads": min(16, rows)}},
               "gpu_repeats_bit_exact": True, "cpu_repeats_bit_exact": True, "pass": True,
               "shadow_changed_pixels": 1 if hard_shadows else 0, "ao_changed_pixels": 1,
               "first_encode_submit_wait_readback_ms": 80,
               "timing_stages": {name: {"first_gpu_ms": 5, "gpu_ms": [3, 1, 2], "median_gpu_ms": 2}
                                 for name in exact.shading_evidence.STAGES}}
    shading["timing_stages"]["primary"] = {"first_gpu_ms": 7, "gpu_ms": [5, 3, 4], "median_gpu_ms": 4}
    for key in exact.shading_evidence.SURFACE_GATES:
        shading[key] = dict(surface)
    for key in ("color_stage", "final_rgb"):
        shading[key] = dict(color)
    if shadow_mode == "legacy":
        shading["timing_stages"]["shadow_preparation"] = {"first_gpu_ms": 0, "gpu_ms": [0, 0, 0], "median_gpu_ms": 0}
        shading["max_shadow_prepare_command_ms"] = 0
    if shadow_mode != "parallel":
        shading["timing_stages"]["shadow_traversal"] = {"first_gpu_ms": 0, "gpu_ms": [0, 0, 0], "median_gpu_ms": 0}
    total = [sum(stage["gpu_ms"][index] for stage in shading["timing_stages"].values()) for index in range(3)]
    for key, values in (("gpu_timestamp", total), ("encode_submit_wait_readback", [30, 15, 20]),
                        ("cpu_full_render", [50, 30, 40])):
        shading[key + "_ms"] = values; shading[key + "_median_ms"] = sorted(values)[1]
    report["shading"] = shading
    if sweep:
        def entry(key, value, stride):
            row = {key: value, "threads_per_threadgroup": value if key == "threads_per_threadgroup" else 32,
                   "gpu_ms": [3, 1, 2], "median_gpu_ms": 2, "warmup_gpu_ms": 5,
                   "different_records": 0, "fault_records": 0, "checked_records": 4*256,
                   "record_bytes": stride}
            if key == "rows_per_command": row["max_command_gpu_ms"] = 4
            return row
        shading["dispatch_trials"] = {
            "pass": True, "records_per_run": 256, "warmup_runs": 1, "timed_runs": 3,
            "note": "Isolated dispatch trials excluded from complete render timing.",
            "prepare_thread_execution_width": 32, "prepare_max_total_threads_per_threadgroup": 1024,
            "march_thread_execution_width": 32,
            "preparation": [entry("threads_per_threadgroup", n, 96) for n in (32, 64, 128, 256)],
            "march": [entry("rows_per_command", n, 48) for n in (16, 128, 256, 512)]}
    return report


class SceneManifestEvidenceTests(unittest.TestCase):
    def setUp(self):
        self.catalog = exact.scene_manifest
        self.text = self.catalog.MANIFEST.read_text()
        self.rows = [line for line in self.text.splitlines() if line.startswith("MB3D_EXACT_SCENE")]

    def report_for(self, scene, full=False, hard=False):
        report = valid_shading_report(hard) if full else valid_report()
        report["numeric_profile"] = scene.profile(full)
        report["scene_description"] = scene.description()
        report["pipeline_specialization"] = {"formula_kind": scene.formula_kind, "builtin_power": scene.builtin_power,
                                             "capability_id": scene.capability_id}
        section = report["shading" if full else "primary"]
        section.update(scene.report_identity())
        section["primary" if full else "comparison"].update(
            result_kind_limit=2 if scene.formula_kind == 3 else 1,
            final_iteration_limit=scene.max_iterations+(scene.formula_kind == 3))
        return report

    def validate(self, report, scene, full=False, hard=False):
        return exact.validate_report(report, 16, 16, 6, 3, full_shading=full,
                                     hard_shadows=hard, shadow_mode="prepared",
                                     scene_descriptor=scene)

    def test_shared_catalog_pins_every_input_and_profiles(self):
        self.assertEqual(len(self.catalog.SCENES), 4)
        for scene in self.catalog.SCENES:
            with self.subTest(scene=scene.scene_id):
                self.assertEqual(self.catalog.scene_for_file(exact.ROOT / scene.path), scene)
                self.assertEqual(len(scene.parameter_bits), 13)
                self.assertEqual(scene.original_shadow_bits, 6)
                self.assertEqual(scene.slot_count, 15)
                for full in (False, True):
                    self.validate(self.report_for(scene, full), scene, full)
                self.validate(self.report_for(scene, True, True), scene, True, True)
        self.assertEqual(self.catalog.SCENES[1].profile(), "EXACT_BINARY64_PRIMARY_BOXIFS")
        self.assertEqual(self.catalog.SCENES[1].profile(True), "EXACT_BOXIFS_FULL_SHADING")
        self.assertEqual(self.catalog.DEFAULT_SCENE.profile(), "EXACT_BINARY64_PRIMARY_SPHEREIFS")

    def test_scene_metadata_is_mandatory_and_bound_to_selected_input(self):
        for scene in self.catalog.SCENES:
            for full in (False, True):
                for field, expected in scene.report_identity().items():
                    for value in (None, "wrong", True, 0):
                        report = self.report_for(scene, full)
                        section = report["shading" if full else "primary"]
                        if value is None:
                            del section[field]
                        else:
                            section[field] = value
                        with self.subTest(scene=scene.scene_id, full=full, field=field, value=value):
                            with self.assertRaises(ValueError):
                                self.validate(report, scene, full)
                other = self.catalog.SCENES[(scene.capability_id) % len(self.catalog.SCENES)]
                report = self.report_for(other, full)
                # A consistent descriptor from a different pinned scene is
                # still not evidence for the requested input.
                with self.assertRaises(ValueError):
                    self.validate(report, scene, full)

    def test_formula_profile_and_abi_cannot_be_relabelled(self):
        box = self.catalog.SCENES[1]
        for full in (False, True):
            report = self.report_for(box, full)
            report["numeric_profile"] = exact.DEFAULT_SCENE.profile(full)
            with self.assertRaises(ValueError):
                self.validate(report, box, full)
            report = self.report_for(box, full)
            report["abi_checks"] = 22 if full else 12
            with self.assertRaises(ValueError):
                self.validate(report, box, full)
            # Merely changing a caller-side object cannot add a capability.
            forged = replace(box, sha256="a" * 64)
            report = self.report_for(forged, full)
            with self.assertRaises(ValueError):
                self.validate(report, forged, full)

    def test_catalog_parser_rejects_malformed_fields_and_code(self):
        changes = (
            lambda row: row.replace("MB3D_EXACT_SCENE", "OTHER_MACRO"),
            lambda row: row + "; arbitrary_call()",
            lambda row: row.replace('"sphereifs_extall"', '"../sphere"'),
            lambda row: row.replace("comparison/scenes/", "../comparison/scenes/"),
            lambda row: row.replace("comparison/scenes/", "comparison/scenes/../scenes/"),
            lambda row: row.replace("comparison/scenes/", "/comparison/scenes/"),
            lambda row: row.replace("comparison/scenes/", "comparison/scenes//"),
            lambda row: row.replace('"SphereIFS"', '"Unsupported"'),
            lambda row: row.replace(", 1, \"SphereIFS\"", ", 2, \"SphereIFS\""),
            lambda row: row.replace(", 60, 15, 6,", ", 0, 15, 6,"),
            lambda row: row.replace(", 60, 15, 6,", ", 60, 0, 6,"),
            lambda row: row.replace(", 60, 15, 6,", ", 60, 15, 256,"),
            lambda row: row.replace("0x3ff0000000000000ULL", "1.0", 1),
            lambda row: row.replace("0x3ff0000000000000ULL", "0x4000000000000000ULL", 1),
            lambda row: row.replace("0x0000000000000000ULL", "0x8000000000000000ULL", 1),
            lambda row: row.replace("0x3ff0000000000000ULL", "0x7ff0000000000000ULL", 1),
            lambda row: row.replace("0x3ff0000000000000ULL", "0x7ff8000000000000ULL", 1),
        )
        for change in changes:
            with self.subTest(change=change), self.assertRaises(ValueError):
                self.catalog.parse_manifest(change(self.rows[0]))
        with self.assertRaises(ValueError):
            self.catalog.parse_manifest("// only comments\n")

    def test_catalog_parser_rejects_duplicate_ids_paths_hashes_and_twist(self):
        first, second = self.catalog.SCENES[:2]
        for old, new in ((f'({second.capability_id},', f'({first.capability_id},'),
                         (f'"{second.scene_id}"', f'"{first.scene_id}"'),
                         (f'"{second.path}"', f'"{first.path}"'),
                         (f'"{second.sha256}"', f'"{first.sha256}"')):
            text = self.rows[0] + "\n" + self.rows[1].replace(old, new)
            with self.subTest(field=old), self.assertRaises(ValueError):
                self.catalog.parse_manifest(text)
        for index, bits in ((8, "0x8000000000000000ULL"), (8, "0x3ff0000000000000ULL"),
                            (9, "0x0000000000000000ULL"), (10, "0x3ff0000000000000ULL"),
                            (11, "0x4000000000000000ULL"), (12, "0x4000000000000000ULL")):
            fields = self.rows[1][len("MB3D_EXACT_SCENE("):-1].split(", ")
            fields[9 + index] = bits
            text = self.rows[0] + "\nMB3D_EXACT_SCENE(" + ", ".join(fields) + ")"
            with self.subTest(parameter=index, bits=bits), self.assertRaises(ValueError):
                self.catalog.parse_manifest(text)
        with self.assertRaises(ValueError):
            self.catalog.parse_manifest("\n".join(reversed(self.rows)))

    def test_mutated_input_is_not_admitted_but_unchanged_copy_is(self):
        scene = self.catalog.SCENES[1]
        with tempfile.TemporaryDirectory() as directory:
            copied = Path(directory) / "arbitrary-copy-name.m3p"
            data = (exact.ROOT / scene.path).read_bytes()
            copied.write_bytes(data)
            self.assertEqual(self.catalog.scene_for_file(copied), scene)
            copied.write_bytes(data[:-1] + bytes([data[-1] ^ 1]))
            with self.assertRaises(ValueError):
                self.catalog.scene_for_file(copied)

    def test_exact_geometry_bits_are_required_for_every_scene_and_scope(self):
        for scene in self.catalog.SCENES:
            for full in (False, True):
                report = self.report_for(scene, full)
                section = report["shading" if full else "primary"]
                section["primary" if full else "comparison"]["different_numeric_bits"] = 1
                with self.subTest(scene=scene.scene_id, full=full), self.assertRaises(ValueError):
                    self.validate(report, scene, full)
                report["status"] = "FAIL"
                if full:
                    section["pass"] = False
                self.validate(report, scene, full)

    def test_formula_rejection_controls_are_mandatory_and_fail_closed(self):
        scene = self.catalog.SCENES[1]
        for full in (False, True):
            for values in (None, {}, {"cases": 4}, {"checks": 1279}, {"checks": 1281},
                           {"cases": True}, {"checks": 1280.0}, {"failures": -1},
                           {"failures": 1281}, {"pass": 1}, {"pass": False},
                           {"no_minimum_cases": 0}, {"no_minimum_cases": True},
                           {"no_minimum_checks": 255}, {"no_minimum_failures": 257},
                           {"no_minimum_failures": 1}, {"no_minimum_failures": 1, "pass": False},
                           {"failures": 1}, {"failures": 1, "pass": False}):
                report = self.report_for(scene, full)
                section = report["shading" if full else "primary"]
                if values is None:
                    del section["formula_rejections"]
                elif not values:
                    section["formula_rejections"] = {}
                else:
                    section["formula_rejections"].update(values)
                with self.subTest(full=full, values=values), self.assertRaises(ValueError):
                    self.validate(report, scene, full)
            report = self.report_for(scene, full)
            section = report["shading" if full else "primary"]
            section["formula_rejections"].update(failures=1, **{"pass": False})
            report["status"] = "FAIL"
            if full:
                section["pass"] = False
            self.validate(report, scene, full)

    def test_raw_primary_exact_gate_covers_all_bytes_and_both_scopes(self):
        scene = self.catalog.SCENES[1]
        with tempfile.TemporaryDirectory() as directory:
            folder = Path(directory)
            raw = bytes(16 * 16 * 176)
            (folder / "gpu-primary.bin").write_bytes(raw)
            for full in (False, True):
                report = self.report_for(scene, full)
                (folder / "cpu-primary.bin").write_bytes(raw)
                exact.shading_evidence.validate_primary_cpu_buffers(folder, report, 16, 16)
                # Camera, numeric geometry, packed normal, fault/reserved and
                # the last allocated byte must all be protected by the gate.
                for offset in (0, 56, 120, 152, 159, 160, 168, len(raw) - 1):
                    changed = bytearray(raw); changed[offset] = 1
                    (folder / "cpu-primary.bin").write_bytes(changed)
                    with self.subTest(full=full, offset=offset), self.assertRaisesRegex(ValueError, "records differ|Noncanonical"):
                        exact.shading_evidence.validate_primary_cpu_buffers(folder, report, 16, 16)
                report["status"] = "FAIL"
                exact.shading_evidence.validate_primary_cpu_buffers(folder, report, 16, 16)
                (folder / "cpu-primary.bin").write_bytes(raw[:-1])
                with self.assertRaisesRegex(ValueError, "176-byte ABI"):
                    exact.shading_evidence.validate_primary_cpu_buffers(folder, report, 16, 16)
                (folder / "cpu-primary.bin").unlink()
                with self.assertRaisesRegex(ValueError, "176-byte ABI"):
                    exact.shading_evidence.validate_primary_cpu_buffers(folder, report, 16, 16)

    def test_box_gallery_identifies_scene_and_exact_cpu_scope(self):
        for full in (False, True):
            scene = self.catalog.SCENES[3]
            report = self.report_for(scene, full)
            gallery = exact.shading_evidence.gallery_html if full else exact.gallery_html
            markup = gallery(report, [])
            self.assertIn(Path(scene.path).name, markup)
            self.assertIn("формула boxIFS", markup)
            self.assertNotIn("формула SphereIFS", markup)
            self.assertIn("Pascal", markup)


class RuntimeSceneEvidenceTests(unittest.TestCase):
    def scene(self, kind=1, power=0):
        source = exact.DEFAULT_SCENE if kind != 2 else exact.scene_manifest.SCENES[1]
        cap = 100+kind
        return replace(source, capability_id=cap, formula_kind=kind,
                       scene_id=exact.scene_manifest.RUNTIME[cap][1],
                       formula_name=exact.scene_manifest.FORMULAS[kind],
                       path="/arbitrary/folder/runtime.m3p", sha256="a"*64,
                       builtin_power=power,
                       parameter_bits=((0x3ff0000000000000,0x2b2bff2ee48e0530,0x3ff0000000000000,
                                        0x4018000000000000,0x401c000000000000,0x4020000000000000,
                                        0x403c000000000000,0x4051800000000000,0,0,0,0,0)
                                       if kind == 3 else source.parameter_bits))

    def description(self, scene):
        return {"schema": exact.scene_manifest.DESCRIPTION_SCHEMA,
                "native_arm64": True, "translated": False, "gpu_dispatches": 0,
                "scene_abi_version": 4, "scene_bytes": 336, "pixel_bytes": 176,
                "scene": scene.description()}

    def report(self, scene, full=False):
        report = valid_shading_report() if full else valid_report()
        report["numeric_profile"] = scene.profile(full)
        report["scene_description"] = scene.description()
        report["pipeline_specialization"] = {"formula_kind": scene.formula_kind, "builtin_power": scene.builtin_power,
                                             "capability_id": scene.capability_id}
        section = report["shading" if full else "primary"]
        section.update(scene.report_identity())
        section["formula_rejections"].update(no_minimum_cases=int(scene.formula_kind != 3),
                                             no_minimum_checks=256*int(scene.formula_kind != 3))
        section["primary" if full else "comparison"].update(
            result_kind_limit=2 if scene.formula_kind == 3 else 1,
            final_iteration_limit=scene.max_iterations+(scene.formula_kind == 3))
        return report

    def test_native_description_admits_runtime_inputs_and_all_internal_powers(self):
        for scene in (self.scene(1), self.scene(2), *(self.scene(3,p) for p in range(2,9))):
            described = exact.scene_manifest.scene_from_description(self.description(scene), scene.sha256, scene.path)
            self.assertEqual(described, scene)
            for full in (False, True):
                report = self.report(scene, full)
                exact.validate_report(report,16,16,6,3,full_shading=full,
                                      shadow_mode="prepared",scene_descriptor=scene)
                markup = (exact.shading_evidence.gallery_html if full else exact.gallery_html)(report,[],scene)
                self.assertIn("runtime.m3p",markup)
                self.assertIn(scene.formula_name,markup)

    def test_iteration_boundary_result_bounds_are_formula_specific(self):
        for scene in (self.scene(1),self.scene(2),replace(self.scene(3,8),max_iterations=256)):
            for full in (False,True):
                report=self.report(scene,full)
                comparison=report["shading"]["primary"] if full else report["primary"]["comparison"]
                exact.validate_report(report,16,16,6,3,full_shading=full,shadow_mode="prepared",scene_descriptor=scene)
                if scene.formula_kind == 3:
                    self.assertEqual(comparison["result_kind_limit"],2)
                    self.assertEqual(comparison["final_iteration_limit"],257)
                for field in ("result_kind_limit","final_iteration_limit"):
                    original=comparison[field]
                    for value in (0,original+1,original-1,True):
                        comparison[field]=value
                        with self.subTest(kind=scene.formula_kind,field=field,value=value),self.assertRaises(ValueError):
                            exact.validate_report(report,16,16,6,3,full_shading=full,shadow_mode="prepared",scene_descriptor=scene)
                    comparison[field]=original

    def test_native_description_requires_exact_input_hash_and_abi(self):
        scene=self.scene()
        for key,value in (("schema","wrong"),("native_arm64",False),("translated",True),
                          ("gpu_dispatches",1),("gpu_dispatches",False),("scene_abi_version",2),("scene_bytes",288),("pixel_bytes",160)):
            description=self.description(scene);description[key]=value
            with self.subTest(key=key),self.assertRaises(ValueError):
                exact.scene_manifest.scene_from_description(description,scene.sha256,scene.path)
        with self.assertRaises(ValueError):
            exact.scene_manifest.scene_from_description(self.description(scene),"b"*64,scene.path)
        changed=self.description(scene);changed["scene"]["slot_count"]=True
        with self.assertRaises(ValueError):
            exact.scene_manifest.scene_from_description(changed,scene.sha256,scene.path)

    def test_known_scene_cannot_be_relabelled_as_runtime(self):
        scene=replace(self.scene(),sha256=exact.DEFAULT_SCENE.sha256)
        with self.assertRaises(ValueError):
            exact.scene_manifest.scene_from_description(self.description(scene),scene.sha256,scene.path)
        pinned=exact.DEFAULT_SCENE
        self.assertEqual(exact.scene_manifest.scene_from_description(self.description(pinned),pinned.sha256,"copy.m3p"),pinned)
        changed=self.description(pinned);changed["scene"]["parameter_bits"][0]="4000000000000000"
        with self.assertRaises(ValueError):
            exact.scene_manifest.scene_from_description(changed,pinned.sha256,pinned.path)

    def test_runtime_parameter_contract_cannot_be_forged(self):
        invalid=[]
        for scene,index,bits in ((self.scene(1),5,0),(self.scene(1),8,0x4000000000000000),
                                 (self.scene(1),9,0x4010000000000000),(self.scene(1),10,0x3fb999999999999a),
                                 (self.scene(2),8,0x8000000000000000),(self.scene(2),0,0x7ff0000000000000)):
            values=list(scene.parameter_bits);values[index]=bits
            invalid.append(replace(scene,parameter_bits=tuple(values)))
        invalid.extend((replace(self.scene(3,8),parameter_bits=(0,)*13),
                        replace(self.scene(),max_iterations=257),replace(self.scene(),slot_count=0),
                        self.scene(3,1),self.scene(3,9),replace(self.scene(),builtin_power=8)))
        for scene in invalid:
            with self.subTest(scene=scene),self.assertRaises(ValueError):
                exact.scene_manifest.scene_from_description(self.description(scene),scene.sha256,scene.path)

    def test_report_requires_the_prepared_description_for_every_scope(self):
        scene=self.scene()
        for full in (False,True):
            for field in ("scene_description","scene_abi_version","scene_bytes","pixel_bytes"):
                report=self.report(scene,full);del report[field]
                with self.subTest(full=full,field=field),self.assertRaises(ValueError):
                    exact.validate_report(report,16,16,6,3,full_shading=full,
                                          shadow_mode="prepared",scene_descriptor=scene)
            report=self.report(scene,full);report["scene_description"]["slot_count"]=15.0
            with self.assertRaises(ValueError):
                exact.validate_report(report,16,16,6,3,full_shading=full,
                                      shadow_mode="prepared",scene_descriptor=scene)

    def test_pipeline_identity_and_cold_creation_time_are_mandatory(self):
        for scene in (self.scene(1), self.scene(2), self.scene(3,4)):
            for full in (False,True):
                for field,value in (
                        ("pipeline_specialization", None),
                        ("pipeline_specialization", {"formula_kind": scene.formula_kind, "builtin_power": 99,
                                                     "capability_id": scene.capability_id}),
                        ("pipeline_specialization", {"formula_kind": True, "builtin_power": scene.builtin_power,
                                                     "capability_id": scene.capability_id}),
                        ("pipeline_specialization", {"formula_kind": scene.formula_kind,
                                                     "builtin_power": scene.builtin_power}),
                        ("pipeline_specialization", {"formula_kind": scene.formula_kind,
                                                     "builtin_power": scene.builtin_power,
                                                     "capability_id": scene.capability_id+1}),
                        ("pipeline_specialization", {"formula_kind": scene.formula_kind,
                                                     "builtin_power": scene.builtin_power, "capability_id": True}),
                        ("pipeline_creation_ms", 0), ("pipeline_creation_ms", True),
                        ("pipeline_creation_ms", float("nan")), ("pipeline_creation_ms", float("inf"))):
                    report=self.report(scene,full)
                    report[field]=value
                    with self.subTest(kind=scene.formula_kind,full=full,field=field,value=value),self.assertRaises(ValueError):
                        exact.validate_report(report,16,16,6,3,full_shading=full,
                                              shadow_mode="prepared",scene_descriptor=scene)

    def test_describe_preflight_is_bounded_native_and_hash_bound(self):
        with tempfile.TemporaryDirectory() as directory:
            path=Path(directory)/"input.m3p";path.write_bytes(b"authored source bytes")
            scene=replace(self.scene(),sha256=exact.sha(path),path=str(path))
            result=SimpleNamespace(returncode=0,stdout=json.dumps(self.description(scene)),stderr="")
            with mock.patch.object(exact.subprocess,"run",return_value=result) as run:
                described,description,command=exact.describe_scene(Path("probe"),path,16,16,6,True,True,900)
            self.assertEqual(described,scene)
            self.assertIn("--describe",command)
            self.assertNotIn("--library",command)
            self.assertEqual(run.call_args.kwargs["timeout"],60)
            result.returncode=1;result.stderr="unsupported formula"
            with mock.patch.object(exact.subprocess,"run",return_value=result),self.assertRaisesRegex(ValueError,"rejected"):
                exact.describe_scene(Path("probe"),path,16,16,6)

    def test_new_primary_tail_cannot_hide_nonfinite_or_reserved_bytes(self):
        with tempfile.TemporaryDirectory() as directory:
            folder=Path(directory)
            for offset,fmt,value in ((160,"d",float("nan")),(168,"Q",1)):
                raw=bytearray(256*176);struct.pack_into("<"+fmt,raw,offset,value)
                for backend in ("gpu","cpu"):(folder/(backend+"-primary.bin")).write_bytes(raw)
                with self.subTest(offset=offset),self.assertRaisesRegex(ValueError,"Noncanonical"):
                    exact.shading_evidence.validate_primary_cpu_buffers(folder,valid_report(),16,16)

    def test_native_log_gate_requires_exact_counts_profile_and_gpu_execution(self):
        for field,value in (("checks",441306),("log_checks",0),("fma_checks",224325),
                            ("oracle","approximate"),("profile","generic log"),
                            ("gpu_timestamp_ms",0),("failures",1)):
            for full in (False,True):
                report=valid_shading_report() if full else valid_report()
                report["native_log"][field]=value
                with self.subTest(field=field,full=full),self.assertRaises(ValueError):
                    exact.validate_report(report,16,16,6,3,full_shading=full,shadow_mode="prepared")


class PowerTwoMultiplicationEvidenceTests(unittest.TestCase):
    def test_both_profiles_require_both_operand_positions(self):
        for full in (False, True):
            report = valid_shading_report() if full else valid_report()
            exact.validate_report(report, 16, 16, 6, 3, full_shading=full, shadow_mode="prepared")
            for suffix in ("checks", "left_checks", "right_checks"):
                incomplete = copy.deepcopy(report)
                incomplete["arithmetic"].pop("power_two_multiplication_" + suffix)
                with self.subTest(full=full, suffix=suffix), self.assertRaises(ValueError):
                    exact.validate_report(incomplete, 16, 16, 6, 3, full_shading=full, shadow_mode="prepared")

    def test_matrix_counters_cannot_be_zero_reduced_or_noninteger(self):
        for suffix, expected in (("checks", 436384), ("left_checks", 218192), ("right_checks", 218192)):
            for value in (0, expected-1, expected+1, True, float(expected), float("nan"), -1):
                report = valid_report(); report["arithmetic"]["power_two_multiplication_" + suffix] = value
                with self.subTest(suffix=suffix, value=value), self.assertRaises(ValueError):
                    exact.validate_numeric_evidence(report)
        report = valid_report()
        report["arithmetic"].update(power_two_multiplication_left_checks=0,
                                    power_two_multiplication_right_checks=436384)
        with self.assertRaisesRegex(ValueError, "multiplication"):
            exact.validate_numeric_evidence(report)

    def test_operation_denominator_must_include_the_whole_matrix(self):
        report = valid_report(); arithmetic = report["arithmetic"]
        operation = next(row for row in arithmetic["operations"] if row["operation"] == 2)
        arithmetic["checks"] -= operation["checks"] - 436383
        operation["checks"] = 436383
        with self.assertRaisesRegex(ValueError, "multiplication"):
            exact.validate_numeric_evidence(report)

    def test_multiplication_failure_cannot_be_promoted_to_pass(self):
        for full in (False, True):
            report = valid_shading_report() if full else valid_report()
            report["arithmetic"]["operations"][2]["failures"] = 1
            report["arithmetic"]["failures"] = 1
            with self.subTest(full=full), self.assertRaisesRegex(ValueError, "Qualification"):
                exact.validate_report(report, 16, 16, 6, 3, full_shading=full, shadow_mode="prepared")
            report["status"] = "FAIL"
            exact.validate_report(report, 16, 16, 6, 3, full_shading=full, shadow_mode="prepared")


class NormalArithmeticEvidenceTests(unittest.TestCase):
    def test_boundary_fields_are_required_in_both_profiles(self):
        for full in (False, True):
            report = valid_shading_report() if full else valid_report()
            exact.validate_report(report, 16, 16, 6, 3, full_shading=full, shadow_mode="prepared")
            for field in exact.NORMAL_ARITHMETIC_COVERAGE:
                incomplete = copy.deepcopy(report)
                incomplete["arithmetic"].pop(field)
                with self.subTest(full=full, field=field), self.assertRaises(ValueError):
                    exact.validate_report(incomplete, 16, 16, 6, 3, full_shading=full, shadow_mode="prepared")

    def test_boundary_counters_cannot_be_empty_reduced_or_noninteger(self):
        for field, expected in exact.NORMAL_ARITHMETIC_COVERAGE.items():
            for value in (0, expected-1, expected+1, True, float(expected), float("nan"), -1):
                report = valid_report()
                report["arithmetic"][field] = value
                with self.subTest(field=field, value=value), self.assertRaises(ValueError):
                    exact.validate_numeric_evidence(report)

    def test_each_operation_must_include_its_distinct_boundary_matrix(self):
        minimums = {0: 1960532, 1: 1960532, 2: 1669332}
        for operation, minimum in minimums.items():
            report = valid_report()
            arithmetic = report["arithmetic"]
            row = next(row for row in arithmetic["operations"] if row["operation"] == operation)
            arithmetic["checks"] -= row["checks"] - (minimum-1)
            row["checks"] = minimum-1
            with self.subTest(operation=operation), self.assertRaisesRegex(ValueError, "per-operation"):
                exact.validate_numeric_evidence(report)

    def test_add_and_subtract_checks_cannot_be_assigned_to_one_operation(self):
        report = valid_report()
        arithmetic = report["arithmetic"]
        add, subtract = arithmetic["operations"][:2]
        transferred = exact.NORMAL_ARITHMETIC_COVERAGE["normal_addition_boundary_add_checks"]
        add["checks"] -= transferred
        subtract["checks"] += transferred
        with self.assertRaisesRegex(ValueError, "per-operation"):
            exact.validate_numeric_evidence(report)

    def test_boundary_failure_cannot_be_promoted_to_pass(self):
        for full in (False, True):
            for operation in (0, 1, 2):
                report = valid_shading_report() if full else valid_report()
                report["arithmetic"]["operations"][operation]["failures"] = 1
                report["arithmetic"]["failures"] = 1
                with self.subTest(full=full, operation=operation), self.assertRaisesRegex(ValueError, "Qualification"):
                    exact.validate_report(report, 16, 16, 6, 3, full_shading=full, shadow_mode="prepared")
                report["status"] = "FAIL"
                exact.validate_report(report, 16, 16, 6, 3, full_shading=full, shadow_mode="prepared")


class SqrtSeedEvidenceTests(unittest.TestCase):
    def test_controls_are_required_in_both_primary_and_full_reports(self):
        for full in (False, True):
            report = valid_shading_report() if full else valid_report()
            exact.validate_report(report, 16, 16, 6, 3, full_shading=full, shadow_mode="prepared")
            report.pop("sqrt_seed_controls")
            with self.subTest(full=full), self.assertRaises(ValueError):
                exact.validate_report(report, 16, 16, 6, 3, full_shading=full, shadow_mode="prepared")

    def test_targeted_production_sqrt_cannot_be_dropped(self):
        for field, value in (("targeted_sqrt_checks", 19702), ("targeted_sqrt_checks", True),
                             ("independent_sqrt_checks", 10)):
            report = valid_report(); report["arithmetic"][field] = value
            with self.subTest(field=field), self.assertRaisesRegex(ValueError, "sqrt"):
                exact.validate_numeric_evidence(report)
        report = valid_report(); group = report["arithmetic"]["operations"][4]
        report["arithmetic"]["checks"] -= group["checks"]-19713; group["checks"] = 19713
        with self.assertRaisesRegex(ValueError, "Targeted"):
            exact.validate_numeric_evidence(report)

    def test_seed_dimensions_case_set_and_oracle_are_fixed(self):
        for field in ("matrix_inputs", "seed_variants", "invalid_high_checks", "records", "checks",
                      "sqrt_checks", "prefix_checks", "nan_class_checks", "signed_zero_checks",
                      "input_bytes", "record_bytes"):
            for value in (0, True):
                report = valid_report(); report["sqrt_seed_controls"][field] = value
                with self.subTest(field=field, value=value), self.assertRaises(ValueError):
                    exact.validate_sqrt_seed_controls(report)
        for field, value in (("gate", "epsilon"), ("oracle", "shared helper"), ("cases", [])):
            report = valid_report(); report["sqrt_seed_controls"][field] = value
            with self.subTest(field=field), self.assertRaises(ValueError): exact.validate_sqrt_seed_controls(report)
        report = valid_report(); report["sqrt_seed_controls"]["cases"][1]["case"] = "exact"
        with self.assertRaisesRegex(ValueError, "cases"): exact.validate_sqrt_seed_controls(report)

    def test_exact_correction_boundary_and_invalid_high_counts_are_bound(self):
        for name in ("exact", "minus_one", "plus_one", "minus_four", "minus_five", "plus_five",
                     "zero", "uint64_max", "invalid_high"):
            report = valid_report(); row = next(r for r in report["sqrt_seed_controls"]["cases"] if r["case"]==name)
            row["accepted_prefixes"] += -1 if row["accepted_prefixes"] else 1
            row["rejected_prefixes"] = row["records"]-row["accepted_prefixes"]
            with self.subTest(name=name), self.assertRaisesRegex(ValueError, "boundary"):
                exact.validate_sqrt_seed_controls(report)
        report = valid_report(); report["sqrt_seed_controls"]["cases"][0]["records"] -= 1
        with self.assertRaisesRegex(ValueError, "coverage"): exact.validate_sqrt_seed_controls(report)

    def test_seed_failures_cannot_hide_in_overall_pass(self):
        for full in (False, True):
            for key in ("sqrt_failures", "prefix_failures"):
                report = valid_shading_report() if full else valid_report()
                controls = report["sqrt_seed_controls"]; controls["cases"][0][key] = 1
                with self.assertRaisesRegex(ValueError, "totals"): exact.validate_sqrt_seed_controls(report)
                controls.update(failures=1, **{"pass":False})
                with self.subTest(full=full, key=key), self.assertRaisesRegex(ValueError, "Qualification"):
                    exact.validate_report(report, 16, 16, 6, 3, full_shading=full, shadow_mode="prepared")
                report["status"] = "FAIL"
                exact.validate_report(report, 16, 16, 6, 3, full_shading=full, shadow_mode="prepared")

    def test_seed_counters_and_flags_are_strict_types(self):
        for field, value in (("accepted_prefixes", -1), ("rejected_prefixes", True),
                             ("sqrt_failures", 19713), ("prefix_failures", 1.0)):
            report = valid_report(); report["sqrt_seed_controls"]["cases"][0][field] = value
            with self.subTest(field=field), self.assertRaises(ValueError): exact.validate_sqrt_seed_controls(report)
        for value in (1, "true", None):
            report = valid_report(); report["sqrt_seed_controls"]["pass"] = value
            with self.assertRaises(ValueError): exact.validate_sqrt_seed_controls(report)

    def test_seed_gpu_timestamp_must_measure_actual_work(self):
        for value in (0, -1, True, float("nan"), float("inf"), None):
            report = valid_report(); report["sqrt_seed_controls"]["gpu_timestamp_ms"] = value
            with self.subTest(value=value), self.assertRaises(ValueError): exact.validate_sqrt_seed_controls(report)


class WideSqrtSeedEvidenceTests(unittest.TestCase):
    def test_wide_and_original_seed_evidence_are_required_in_both_scopes(self):
        for full in (False, True):
            for key in ("sqrt_seed_controls", "sqrt_wide_seed_controls"):
                report = valid_shading_report() if full else valid_report()
                exact.validate_report(report, 16, 16, 6, 3, full_shading=full, shadow_mode="prepared")
                report.pop(key)
                with self.subTest(full=full, key=key), self.assertRaises(ValueError):
                    exact.validate_report(report, 16, 16, 6, 3, full_shading=full, shadow_mode="prepared")

    def test_matrix_boundary_clamp_classes_abi_and_exact_oracle_are_required(self):
        for field in ("matrix_inputs", "boundary_prefix_inputs", "seed_variants", "clamp_checks",
                      "records", "checks", "sqrt_checks", "prefix_checks", "nan_class_checks",
                      "signed_zero_checks", "input_bytes", "record_bytes"):
            for value in (None, 0, True, 1.0):
                report = valid_report(); report["sqrt_wide_seed_controls"][field] = value
                with self.subTest(field=field, value=value), self.assertRaises(ValueError):
                    exact.validate_sqrt_wide_seed_controls(report)
            report = valid_report(); report["sqrt_wide_seed_controls"][field] -= 1
            with self.subTest(field=field), self.assertRaises(ValueError):
                exact.validate_sqrt_wide_seed_controls(report)
        for field in ("gate", "oracle"):
            report = valid_report(); report["sqrt_wide_seed_controls"][field] = "shared helper or approximate"
            with self.subTest(field=field), self.assertRaises(ValueError): exact.validate_sqrt_wide_seed_controls(report)

    def test_seed_categories_and_independent_correction_guarantees_cannot_shrink(self):
        for mutation in (lambda rows: rows.pop(), lambda rows: rows.reverse(),
                         lambda rows: rows[1].update(case="exact"), lambda rows: rows.append({})):
            report = valid_report(); mutation(report["sqrt_wide_seed_controls"]["cases"])
            with self.assertRaisesRegex(ValueError, "cases"): exact.validate_sqrt_wide_seed_controls(report)
        for index, name in enumerate(exact.SQRT_WIDE_CASES):
            for field in ("records", "required_accepts", "required_rejections", "optional_prefixes"):
                report = valid_report(); row = report["sqrt_wide_seed_controls"]["cases"][index]
                row[field] += 1
                with self.subTest(case=name, field=field), self.assertRaisesRegex(ValueError, "coverage"):
                    exact.validate_sqrt_wide_seed_controls(report)

    def test_required_acceptance_and_rejection_cannot_be_hidden_as_optional_fallback(self):
        for name, accept_delta in (("root_max_clamp", -1), ("exact", -1), ("zero", 1), ("limit", 1)):
            report = valid_report()
            row = next(row for row in report["sqrt_wide_seed_controls"]["cases"] if row["case"] == name)
            row["accepted_prefixes"] += accept_delta; row["rejected_prefixes"] -= accept_delta
            with self.subTest(case=name), self.assertRaisesRegex(ValueError, "guarantees"):
                exact.validate_sqrt_wide_seed_controls(report)
            row["prefix_failures"] = 1
            report["sqrt_wide_seed_controls"].update({"failures":1, "pass":False})
            self.assertEqual(exact.validate_sqrt_wide_seed_controls(report), 1)
        # Near the residual guard, either exact prefix admission or exact
        # fallback is valid. The separately required value-bit check remains.
        for admit_optional in (False, True):
            report = valid_report()
            row = next(row for row in report["sqrt_wide_seed_controls"]["cases"] if row["case"] == "minus_1024")
            if admit_optional:
                row["accepted_prefixes"] += row["optional_prefixes"]
                row["rejected_prefixes"] -= row["optional_prefixes"]
            self.assertEqual(exact.validate_sqrt_wide_seed_controls(report), 0)
            row["sqrt_failures"] = 1
            with self.assertRaisesRegex(ValueError, "totals"): exact.validate_sqrt_wide_seed_controls(report)

    def test_value_or_prefix_failures_cannot_be_promoted_to_pass(self):
        for full in (False, True):
            for field in ("sqrt_failures", "prefix_failures"):
                report = valid_shading_report() if full else valid_report()
                section = report["sqrt_wide_seed_controls"]; section["cases"][0][field] = 1
                with self.assertRaisesRegex(ValueError, "totals"): exact.validate_sqrt_wide_seed_controls(report)
                section["failures"] = 1; section["pass"] = False
                self.assertEqual(exact.validate_sqrt_wide_seed_controls(report), 1)
                with self.subTest(full=full, field=field), self.assertRaisesRegex(ValueError, "Qualification"):
                    exact.validate_report(report, 16, 16, 6, 3, full_shading=full, shadow_mode="prepared")
                report["status"] = "FAIL"
                exact.validate_report(report, 16, 16, 6, 3, full_shading=full, shadow_mode="prepared")

    def test_invalid_outcome_counts_flags_and_unmeasured_work_are_rejected(self):
        for field, value in (("sqrt_failures", 19882), ("prefix_failures", True),
                             ("accepted_prefixes", -1), ("rejected_prefixes", 1.0),
                             ("accepted_prefixes", 0), ("rejected_prefixes", 19882)):
            report = valid_report(); report["sqrt_wide_seed_controls"]["cases"][0][field] = value
            with self.subTest(field=field), self.assertRaises(ValueError): exact.validate_sqrt_wide_seed_controls(report)
        for value in (None, 1, "true", False):
            report = valid_report(); report["sqrt_wide_seed_controls"]["pass"] = value
            with self.subTest(value=value), self.assertRaises(ValueError): exact.validate_sqrt_wide_seed_controls(report)
        for value in (None, 0, -1, True, float("nan"), float("inf")):
            report = valid_report(); report["sqrt_wide_seed_controls"]["gpu_timestamp_ms"] = value
            with self.subTest(value=value), self.assertRaises(ValueError): exact.validate_sqrt_wide_seed_controls(report)


class PrimaryDispatchEvidenceTests(unittest.TestCase):
    def validate(self, section, width=16, height=16, rows=256, threads=32, sweep=False, tile_width=8):
        return exact.primary_evidence.validate(section, width, height, rows, threads, sweep, 5, tile_width)

    def test_default_tile_is_8_by_4_and_linear_requires_explicit_selection(self):
        section = valid_primary_dispatch()
        self.assertEqual((section["tile_width"], section["tile_height"]), (8, 4))
        self.assertEqual((section["baseline"]["tile_width"], section["baseline"]["tile_height"]), (32, 1))
        self.validate(section)
        linear = valid_primary_dispatch(tile_width=0)
        self.validate(linear, tile_width=0)
        with self.assertRaisesRegex(ValueError, "tile shape"): self.validate(linear)

    def test_primary_options_are_valid_for_both_scopes(self):
        for full in (False, True):
            for tile in (0, 64, 16, 8, 1):
                report = valid_shading_report() if full else valid_report()
                section = report["shading" if full else "primary"]
                section["primary_dispatch"] = valid_primary_dispatch(rows=8, threads=64, tile_width=tile)
                exact.validate_report(report, 16, 16, 6, 3, full_shading=full,
                                      shadow_mode="prepared", primary_rows_per_command=8, primary_threads=64,
                                      primary_tile_width=tile)
                if tile != 8:
                    with self.subTest(full=full, tile=tile), self.assertRaisesRegex(ValueError, "tile shape"):
                        exact.validate_report(report, 16, 16, 6, 3, full_shading=full,
                                              shadow_mode="prepared", primary_rows_per_command=8, primary_threads=64)

    def test_tile_shape_is_actual_positive_and_matches_requested_thread_product(self):
        for field, value in (("tile_width", None), ("tile_width", 0), ("tile_width", 4),
                             ("tile_width", True), ("tile_width", 8.0), ("tile_width", 64),
                             ("tile_height", None), ("tile_height", 0), ("tile_height", 2),
                             ("tile_height", True), ("tile_height", 4.0)):
            section = valid_primary_dispatch(tile_width=8); section[field] = value
            with self.subTest(field=field, value=value), self.assertRaises(ValueError):
                self.validate(section, tile_width=8)
        for requested in (-1, 3, 33, True, 8.0):
            with self.subTest(requested=requested), self.assertRaises(ValueError):
                self.validate(valid_primary_dispatch(), tile_width=requested)

    def test_tiled_partial_rows_keep_full_grid_and_command_ceil(self):
        section = valid_primary_dispatch(63, 47, 13, tile_width=4)
        self.validate(section, 63, 47, 13, tile_width=4)
        self.assertEqual((section["tile_width"], section["tile_height"]), (4, 8))
        self.assertEqual((section["commands_per_frame"], section["grid_width"], section["grid_height"]),
                         (4, 63, 13))
        for field, value in (("commands_per_frame", 3), ("grid_width", 64), ("grid_height", 16)):
            altered = copy.deepcopy(section); altered[field] = value
            with self.subTest(field=field), self.assertRaises(ValueError):
                self.validate(altered, 63, 47, 13, tile_width=4)

    def test_partial_row_grid_and_command_ceil_are_required(self):
        self.validate(valid_primary_dispatch(63, 47, 32), 63, 47, 32)
        for field, value in (("commands_per_frame", 1), ("commands_per_frame", 3),
                             ("grid_width", 64), ("grid_height", 47), ("rows_per_command", 47)):
            section = valid_primary_dispatch(63, 47, 32); section[field] = value
            with self.subTest(field=field, value=value), self.assertRaises(ValueError):
                self.validate(section, 63, 47, 32)

    def test_primary_settings_and_compiled_limits_are_bound(self):
        for field, value in (("rows_per_command", 8), ("threads_per_threadgroup", 64),
                             ("thread_execution_width", 0), ("thread_execution_width", 24),
                             ("max_total_threads_per_threadgroup", 16), ("max_command_gpu_ms", 0),
                             ("max_command_gpu_ms", 6), ("max_command_gpu_ms", float("nan")),
                             ("commands_per_frame", True)):
            section = valid_primary_dispatch(); section[field] = value
            with self.subTest(field=field), self.assertRaises(ValueError): self.validate(section)

    def test_maximum_command_uses_positive_first_and_repeated_frames(self):
        report = valid_report()
        report["primary"]["primary_dispatch"]["max_command_gpu_ms"] = 5
        exact.validate_report(report, 16, 16, 6, 3)
        for full in (False, True):
            for value in (None, [], [1, float("nan"), 3], [3, 0, 2], ["2", 1, 3]):
                report = valid_shading_report() if full else valid_report()
                target = report["shading"]["timing_stages"]["primary"] if full else report["primary"]
                target["gpu_ms" if full else "gpu_timestamp_ms"] = value
                with self.subTest(full=full, value=value), self.assertRaises(ValueError):
                    exact.validate_report(report, 16, 16, 6, 3, full_shading=full, shadow_mode="prepared")
            for value in (None, 0, float("inf")):
                report = valid_shading_report() if full else valid_report()
                target = report["shading"]["timing_stages"]["primary"] if full else report["primary"]
                target["first_gpu_ms" if full else "first_gpu_timestamp_ms"] = value
                with self.subTest(full=full, value=value), self.assertRaises(ValueError):
                    exact.validate_report(report, 16, 16, 6, 3, full_shading=full, shadow_mode="prepared")

    def test_command_maximum_cannot_be_below_complete_frame_per_command(self):
        for full in (False, True):
            for rows in (8, 256):
                report = valid_shading_report() if full else valid_report()
                section = report["shading" if full else "primary"]
                section["primary_dispatch"] = valid_primary_dispatch(rows=rows)
                exact.validate_report(report, 16, 16, 6, 3, full_shading=full,
                                      shadow_mode="prepared", primary_rows_per_command=rows)
                section["primary_dispatch"]["max_command_gpu_ms"] -= 1e-6
                with self.subTest(full=full, rows=rows), self.assertRaisesRegex(ValueError, "below"):
                    exact.validate_report(report, 16, 16, 6, 3, full_shading=full,
                                          shadow_mode="prepared", primary_rows_per_command=rows)
        section = valid_primary_dispatch(63, 47, 13)
        section["max_command_gpu_ms"] = 5 / 4 - 1e-10
        self.validate(section, 63, 47, 13)
        section["max_command_gpu_ms"] = 5 / 4 - 1e-6
        with self.assertRaisesRegex(ValueError, "below"): self.validate(section, 63, 47, 13)

    def test_baseline_cannot_be_omitted_reduced_or_loosened(self):
        section = valid_primary_dispatch(); section.pop("baseline")
        with self.assertRaises(ValueError): self.validate(section)
        for field, value in (("rows_per_command", 128), ("threads_per_threadgroup", 64),
                             ("tile_width", 16), ("tile_height", 2), ("tile_width", None),
                             ("tile_height", None), ("tile_width", True),
                             ("pixels", 255), ("different_pixels", 257), ("fault_pixels", True),
                             ("gate", "tolerance 2e-12")):
            section = valid_primary_dispatch(); section["baseline"][field] = value
            with self.subTest(field=field), self.assertRaises(ValueError): self.validate(section)

    def test_baseline_and_negative_failures_cannot_be_hidden_from_pass(self):
        for subsection, field in (("baseline", "different_pixels"), ("baseline", "fault_pixels"),
                                  ("negative_controls", "failures")):
            section = valid_primary_dispatch(); section[subsection][field] = 1
            if subsection == "negative_controls": section[subsection]["pass"] = False
            with self.subTest(field=field), self.assertRaisesRegex(ValueError, "pass flag"):
                self.validate(section)
            section["pass"] = False
            self.assertEqual(self.validate(section), 1)
            for full in (False, True):
                report = valid_shading_report() if full else valid_report()
                report["shading" if full else "primary"]["primary_dispatch"] = section
                with self.assertRaisesRegex(ValueError, "Qualification"):
                    exact.validate_report(report, 16, 16, 6, 3, full_shading=full, shadow_mode="prepared")
                report["status"] = "FAIL"
                if full: report["shading"]["pass"] = False
                exact.validate_report(report, 16, 16, 6, 3, full_shading=full, shadow_mode="prepared")

    def test_negative_controls_cover_larger_and_smaller_launch_dimensions(self):
        for field, value in (("cases", 2), ("checks", 512), ("checks", 1023), ("checks", 1025),
                             ("failures", 1025), ("pass", False)):
            section = valid_primary_dispatch(); section["negative_controls"][field] = value
            with self.subTest(field=field), self.assertRaises(ValueError): self.validate(section)

    def test_requested_trials_are_mandatory_and_unrequested_trials_rejected(self):
        with self.assertRaisesRegex(ValueError, "missing"): self.validate(valid_primary_dispatch(), sweep=True)
        with self.assertRaisesRegex(ValueError, "Unrequested"): self.validate(valid_primary_dispatch(sweep=True))
        report = valid_shading_report(); report["shading"]["primary_dispatch"] = valid_primary_dispatch(sweep=True)
        exact.validate_report(report, 16, 16, 6, 3, full_shading=True, shadow_mode="prepared", primary_dispatch_sweep=True)
        with self.assertRaisesRegex(ValueError, "require full shading"):
            exact.validate_report(valid_report(), 16, 16, 6, 3, primary_dispatch_sweep=True)

    def test_trial_configurations_are_deduplicated_and_limited_by_pipeline(self):
        for height in (16, 32, 128, 256, 47):
            for maximum in (32, 64, 128, 1024):
                self.validate(valid_primary_dispatch(63, height, 32, sweep=True, maximum=maximum),
                              63, height, 32, sweep=True)
        for mutation in (lambda t: t["configurations"].pop(),
                         lambda t: t["configurations"].append(copy.deepcopy(t["configurations"][0])),
                         lambda t: t["configurations"][0].update(threads_per_threadgroup=16),
                         lambda t: t["configurations"][0].update(rows_per_command=16)):
            section = valid_primary_dispatch(sweep=True); mutation(section["trials"])
            with self.assertRaisesRegex(ValueError, "configurations"): self.validate(section, sweep=True)

    def test_sweep_has_each_linear_and_two_dimensional_shape_once(self):
        configurations = exact.primary_evidence.configuration_shapes(47, 256, 32, 1024)
        self.assertEqual(configurations, [(8, 32, 32), (32, 32, 32), (128, 32, 32), (256, 32, 32),
                                          (47, 32, 32), (256, 64, 64), (256, 128, 128),
                                          (256, 256, 256), (256, 32, 16), (256, 32, 8), (256, 32, 4)])
        self.assertEqual(exact.primary_evidence.configuration_shapes(32, 32, 32, 32),
                         [(8, 32, 32), (32, 32, 32), (128, 32, 32), (256, 32, 32),
                          (32, 32, 16), (32, 32, 8), (32, 32, 4)])
        self.validate(valid_primary_dispatch(sweep=True, tile_width=8), sweep=True, tile_width=8)

    def test_trial_tile_shape_cannot_be_missing_replaced_or_misreported(self):
        for field, value in (("tile_width", None), ("tile_width", 0), ("tile_width", 32),
                             ("tile_width", True), ("tile_width", 4.0), ("tile_height", None),
                             ("tile_height", 0), ("tile_height", 1), ("tile_height", 7),
                             ("tile_height", True)):
            section = valid_primary_dispatch(sweep=True)
            section["trials"]["configurations"][-1][field] = value
            with self.subTest(field=field, value=value), self.assertRaises(ValueError):
                self.validate(section, sweep=True)
        section = valid_primary_dispatch(sweep=True)
        section["trials"]["configurations"][-1].update(tile_width=2, tile_height=16)
        with self.assertRaisesRegex(ValueError, "configurations"): self.validate(section, sweep=True)

    def test_trials_require_all_records_in_all_runs(self):
        for field, value in (("warmup_runs", 0), ("timed_runs", 2), ("records_per_run", 255)):
            section = valid_primary_dispatch(sweep=True); section["trials"][field] = value
            with self.subTest(field=field), self.assertRaises(ValueError): self.validate(section, sweep=True)
        for field, value in (("checked_records", 3*256), ("record_bytes", 48),
                             ("different_records", 4*256+1), ("fault_records", True)):
            section = valid_primary_dispatch(sweep=True); section["trials"]["configurations"][0][field] = value
            with self.subTest(field=field), self.assertRaises(ValueError): self.validate(section, sweep=True)

    def test_trial_failure_contributes_to_dispatch_pass(self):
        for field in ("different_records", "fault_records"):
            section = valid_primary_dispatch(sweep=True); section["trials"]["configurations"][0][field] = 1
            with self.assertRaisesRegex(ValueError, "pass flag"): self.validate(section, sweep=True)
            section["pass"] = False
            self.assertEqual(self.validate(section, sweep=True), 1)

    def test_trial_timings_require_real_samples_correct_median_and_bounded_command(self):
        for values in ({"gpu_ms": [1, 2]}, {"gpu_ms": [0, 1, 2], "median_gpu_ms": 1},
                       {"gpu_ms": [1, float("inf"), 2]}, {"median_gpu_ms": 3},
                       {"warmup_gpu_ms": 0}, {"max_command_gpu_ms": 0}, {"max_command_gpu_ms": 6}):
            section = valid_primary_dispatch(sweep=True); section["trials"]["configurations"][0].update(values)
            with self.subTest(values=values), self.assertRaises(ValueError): self.validate(section, sweep=True)

    def test_trial_command_maximum_uses_partial_batch_count_and_warmup(self):
        section = valid_primary_dispatch(63, 47, 13, sweep=True)
        self.validate(section, 63, 47, 13, sweep=True)
        for index, original in enumerate(section["trials"]["configurations"]):
            altered = copy.deepcopy(section); row = altered["trials"]["configurations"][index]
            commands = (47 + original["rows_per_command"] - 1) // original["rows_per_command"]
            row["max_command_gpu_ms"] = 5 / commands - 1e-6
            with self.subTest(index=index), self.assertRaisesRegex(ValueError, "below"):
                self.validate(altered, 63, 47, 13, sweep=True)
        section["trials"]["configurations"][0]["max_command_gpu_ms"] -= 1e-10
        self.validate(section, 63, 47, 13, sweep=True)

    def test_saved_primary_baseline_is_full_record_exact_including_faults(self):
        for backend in ("gpu-primary.bin", "gpu-baseline-primary.bin"):
            for offset in (0, 119, 152, 159):
                with self.subTest(backend=backend, offset=offset), tempfile.TemporaryDirectory() as name:
                    folder = Path(name); section = valid_primary_dispatch()
                    for file in ("gpu-primary.bin", "gpu-baseline-primary.bin"):
                        (folder / file).write_bytes(bytes(256*176))
                    exact.primary_evidence.validate_buffers(folder, section, 16, 16)
                    path = folder / backend; raw = bytearray(path.read_bytes()); raw[offset] = 1; path.write_bytes(raw)
                    with self.assertRaisesRegex(ValueError, "bytes disagree"):
                        exact.primary_evidence.validate_buffers(folder, section, 16, 16)
                    section["baseline"]["different_pixels"] = 1
                    section["baseline"]["fault_pixels"] = int(offset == 152)
                    exact.primary_evidence.validate_buffers(folder, section, 16, 16)

    def test_identically_faulty_primary_baseline_cannot_hide_fault_count(self):
        with tempfile.TemporaryDirectory() as name:
            folder = Path(name); section = valid_primary_dispatch()
            raw = bytearray(256*176); struct.pack_into("<I", raw, 152, 4)
            for file in ("gpu-primary.bin", "gpu-baseline-primary.bin"): (folder / file).write_bytes(raw)
            with self.assertRaisesRegex(ValueError, "bytes disagree"):
                exact.primary_evidence.validate_buffers(folder, section, 16, 16)
            section["baseline"]["fault_pixels"] = 1
            exact.primary_evidence.validate_buffers(folder, section, 16, 16)

    def test_missing_or_truncated_primary_baseline_is_rejected(self):
        with tempfile.TemporaryDirectory() as name:
            folder = Path(name); (folder / "gpu-primary.bin").write_bytes(bytes(256*176))
            with self.assertRaisesRegex(ValueError, "ABI"):
                exact.primary_evidence.validate_buffers(folder, valid_primary_dispatch(), 16, 16)
            (folder / "gpu-baseline-primary.bin").write_bytes(bytes(256*176-1))
            with self.assertRaisesRegex(ValueError, "ABI"):
                exact.primary_evidence.validate_buffers(folder, valid_primary_dispatch(), 16, 16)

    def test_primary_cli_rejects_bounds_and_diagnostic_sweep_before_execution(self):
        for option, values in (("--primary-rows-per-command", ("0", "-1", "4097")),
                               ("--primary-threads", ("0", "-1", "1025"))):
            for value in values:
                with contextlib.redirect_stderr(io.StringIO()) as output, self.assertRaises(SystemExit):
                    exact.main(["--probe", "missing", "--library", "missing.metallib", "--output", "unused", option, value])
                self.assertIn("Primary rows must", output.getvalue())
        with contextlib.redirect_stderr(io.StringIO()) as output, self.assertRaises(SystemExit):
            exact.main(["--probe", "missing", "--library", "missing.metallib", "--output", "unused", "--primary-dispatch-sweep"])
        self.assertIn("--primary-dispatch-sweep requires --full-shading", output.getvalue())

    def test_primary_cli_rejects_non_factor_tile_before_execution(self):
        for value in ("-1", "3", "33", "1025"):
            for options in ([], ["--full-shading"]):
                with contextlib.redirect_stderr(io.StringIO()) as output, self.assertRaises(SystemExit):
                    exact.main(["--probe", "missing", "--library", "missing.metallib", "--output", "unused",
                                "--primary-tile-width", value, *options])
                self.assertIn("Primary tile width must", output.getvalue())

    def test_gallery_exposes_primary_configuration_and_trial_scope(self):
        markup = exact.primary_evidence.gallery_html(valid_primary_dispatch(sweep=True, tile_width=8))
        self.assertIn("256 строк в команде", markup)
        self.assertIn("32 потоков в группе (8×4)", markup)
        self.assertIn("Форма группы", markup)
        self.assertIn("16×2", markup)
        self.assertIn("Прежний запуск по 8 строк", markup)
        self.assertIn("исключены из времени полного рендера", markup)


class ExactShadingEvidenceTests(unittest.TestCase):
    def validate(self, report, hard=False, mode="prepared", prep_threads=32, rows=256, sweep=False, trace_threads=32):
        return exact.validate_report(report, 16, 16, 6, 3, True, hard, mode, prep_threads, rows, sweep, trace_threads)

    def test_original_and_explicit_shadow_variants(self):
        self.validate(valid_shading_report())
        self.validate(valid_shading_report(True), True)
        with self.assertRaises(ValueError):
            self.validate(valid_shading_report(True))

    def test_parallel_is_default_and_preserves_all_three_modes(self):
        exact.validate_report(valid_shading_report(True, "parallel"), 16, 16, 6, 3,
                              full_shading=True, hard_shadows=True)
        for mode in ("legacy", "prepared", "parallel"):
            for hard in (False, True):
                self.validate(valid_shading_report(hard, mode, trace_threads=64), hard, mode, trace_threads=64)

    def test_independent_traces_keep_terminal_faults_separate_from_record_faults(self):
        report = valid_shading_report(True, "parallel")
        report["shading"]["shadow_traversal"]["terminal_fault_pixels"] = 1
        self.validate(report, True, "parallel")
        for field, value in (("pixels", 255), ("gate", "nonzero epsilon"), ("different_pixels", 1),
                             ("fault_pixels", 1), ("active_rays", 0), ("terminal_fault_pixels", 2)):
            report = valid_shading_report(True, "parallel"); report["shading"]["shadow_traversal"][field] = value
            with self.subTest(field=field), self.assertRaises(ValueError):
                self.validate(report, True, "parallel")

    def test_trace_shortcut_controls_require_all_boundary_and_fault_cases(self):
        for values in ({"cases": 5}, {"checks": 11}, {"checks": 13}, {"failures": 1},
                       {"pass": False}, {"cases": True}):
            report = valid_shading_report(True, "parallel"); report["shading"]["trace_skip_controls"].update(values)
            with self.subTest(values=values), self.assertRaises(ValueError):
                self.validate(report, True, "parallel")
        report = valid_shading_report(True, "parallel")
        report["status"] = "FAIL"; report["shading"]["pass"] = False
        report["shading"]["trace_skip_controls"].update({"failures": 1, "pass": False})
        self.validate(report, True, "parallel")

    def test_trace_timing_and_pipeline_limits_are_mode_bound(self):
        for mode in ("legacy", "prepared", "parallel"):
            for mutate in (
                    lambda s: s["timing_stages"]["shadow_traversal"].update(first_gpu_ms=0 if mode == "parallel" else 1),
                    lambda s: s.update(max_shadow_trace_command_ms=0 if mode == "parallel" else 1),
                    lambda s: s["pipeline_limits"]["shadow_trace"].update(threads_per_threadgroup=64),
                    lambda s: s["pipeline_limits"]["shadow_trace"].update(thread_execution_width=24),
                    lambda s: s["pipeline_limits"]["shadow_trace"].update(max_total_threads_per_threadgroup=16),
                    lambda s: s["pipeline_limits"]["shadow_trace"].update(grid_threads=255)):
                report = valid_shading_report(True, mode); mutate(report["shading"])
                with self.subTest(mode=mode, mutation=mutate), self.assertRaises(ValueError):
                    self.validate(report, True, mode)
        report = valid_shading_report(True, "parallel"); report["shading"]["max_shadow_trace_command_ms"] = 6
        with self.assertRaisesRegex(ValueError, "complete stage"):
            self.validate(report, True, "parallel")

    def test_shadow_options_require_shading_cli(self):
        for options in (["--hard-shadows"], ["--shadow-mode", "legacy"], ["--shadow-mode", "prepared"],
                        ["--shadow-mode", "parallel"], ["--shadow-trace-threads", "64"],
                        ["--shadow-prep-threads", "64"], ["--shadow-rows-per-command", "1"],
                        ["--shadow-dispatch-sweep"]):
            with self.subTest(options=options), contextlib.redirect_stderr(io.StringIO()) as output:
                with self.assertRaises(SystemExit) as error:
                    exact.main(["--probe", "missing", "--library", "missing.metallib", "--output", "unused", *options])
            self.assertEqual(error.exception.code, 2)
            self.assertIn("Shadow options require --full-shading", output.getvalue())

    def test_sweep_cli_requires_enabled_prepared_shadows(self):
        for options in ([], ["--hard-shadows"], ["--hard-shadows", "--shadow-mode", "legacy"],
                        ["--hard-shadows", "--shadow-mode", "parallel"]):
            with self.subTest(options=options), contextlib.redirect_stderr(io.StringIO()) as output:
                with self.assertRaises(SystemExit):
                    exact.main(["--probe", "missing", "--library", "missing.metallib", "--output", "unused",
                                "--full-shading", "--shadow-dispatch-sweep", *options])
            self.assertIn("requires --full-shading --hard-shadows and prepared shadow mode", output.getvalue())

    def test_dispatch_sweep_presence_and_scope_are_command_bound(self):
        self.validate(valid_shading_report(True, sweep=True), True, sweep=True)
        report = valid_shading_report(True); report["shading"]["dispatch_trials"] = None
        self.validate(report, True)
        with self.assertRaisesRegex(ValueError, "missing"):
            self.validate(report, True, sweep=True)
        with self.assertRaisesRegex(ValueError, "Unrequested"):
            self.validate(valid_shading_report(True, sweep=True), True)
        with self.assertRaisesRegex(ValueError, "missing"):
            self.validate(valid_shading_report(True), True, sweep=True)
        for hard, mode in ((False, "prepared"), (True, "legacy"), (True, "parallel")):
            with self.assertRaisesRegex(ValueError, "enabled prepared"):
                self.validate(valid_shading_report(hard, mode, sweep=True), hard, mode, sweep=True)
        with self.assertRaisesRegex(ValueError, "require full shading"):
            exact.validate_report(valid_report(), 16, 16, 6, 3, shadow_dispatch_sweep=True)

    def test_dispatch_sweep_all_candidates_and_all_records_are_required(self):
        mutations = (
            lambda t: t.update(records_per_run=255),
            lambda t: t.update(warmup_runs=0),
            lambda t: t.update(timed_runs=4),
            lambda t: t.update(prepare_thread_execution_width=64),
            lambda t: t.update(prepare_max_total_threads_per_threadgroup=512),
            lambda t: t.update(march_thread_execution_width=64),
            lambda t: t["preparation"].pop(),
            lambda t: t["preparation"][0].update(threads_per_threadgroup=16),
            lambda t: t["preparation"][0].update(checked_records=3*256),
            lambda t: t["preparation"][0].update(record_bytes=48),
            lambda t: t["march"].pop(),
            lambda t: t["march"][0].update(rows_per_command=128),
            lambda t: t["march"][0].update(threads_per_threadgroup=64),
            lambda t: t["march"][0].update(record_bytes=96),
            lambda t: t["march"][0].update(fault_records=4*256+1),
        )
        for mutate in mutations:
            report = valid_shading_report(True, sweep=True); mutate(report["shading"]["dispatch_trials"])
            with self.subTest(mutation=mutate), self.assertRaises(ValueError):
                self.validate(report, True, sweep=True)
        # A lower real pipeline limit reduces the candidate set without dropping legal groups.
        report = valid_shading_report(True, sweep=True)
        report["shading"]["pipeline_limits"]["shadow_prepare"]["max_total_threads_per_threadgroup"] = 128
        report["shading"]["dispatch_trials"]["prepare_max_total_threads_per_threadgroup"] = 128
        report["shading"]["dispatch_trials"]["preparation"].pop()
        self.validate(report, True, sweep=True)

    def test_dispatch_sweep_failure_cannot_be_promoted_to_full_pass(self):
        for stage in ("preparation", "march"):
            for key in ("different_records", "fault_records"):
                report = valid_shading_report(True, sweep=True)
                report["shading"]["dispatch_trials"][stage][0][key] = 1
                with self.subTest(stage=stage, key=key), self.assertRaisesRegex(ValueError, "pass flag"):
                    self.validate(report, True, sweep=True)
                report["shading"]["dispatch_trials"]["pass"] = False
                with self.assertRaisesRegex(ValueError, "Qualification"):
                    self.validate(report, True, sweep=True)
                report["status"] = "FAIL"; report["shading"]["pass"] = False
                self.validate(report, True, sweep=True)

    def test_dispatch_sweep_requires_complete_measured_timing(self):
        for stage in ("preparation", "march"):
            for values in ({"gpu_ms": [1, 2]}, {"gpu_ms": [0, 1, 2], "median_gpu_ms": 1},
                           {"gpu_ms": [1, float("nan"), 3]}, {"median_gpu_ms": 3},
                           {"warmup_gpu_ms": 0}):
                report = valid_shading_report(True, sweep=True)
                report["shading"]["dispatch_trials"][stage][0].update(values)
                with self.subTest(stage=stage, values=values), self.assertRaises(ValueError):
                    self.validate(report, True, sweep=True)
        for maximum in (0, 6, float("inf")):
            report = valid_shading_report(True, sweep=True)
            report["shading"]["dispatch_trials"]["march"][0]["max_command_gpu_ms"] = maximum
            with self.subTest(maximum=maximum), self.assertRaises(ValueError):
                self.validate(report, True, sweep=True)

    def test_shadow_cli_bounds_are_rejected_before_execution(self):
        for option in ("--shadow-prep-threads", "--shadow-trace-threads", "--shadow-rows-per-command"):
            for value in ("0", "-1", "4097"):
                with self.subTest(option=option, value=value), contextlib.redirect_stderr(io.StringIO()) as output:
                    with self.assertRaises(SystemExit):
                        exact.main(["--probe", "missing", "--library", "missing.metallib", "--output", "unused",
                                    "--full-shading", option, value])
                self.assertIn("must be 1..4096", output.getvalue())

    def test_prepared_and_legacy_settings_are_bound_to_requested_command(self):
        for mode in ("prepared", "legacy", "parallel"):
            self.validate(valid_shading_report(True, mode, 64, 8), True, mode, 64, 8)
            for field, value in (("shadow_mode", "other"), ("shadow_prep_threads_per_threadgroup", 32),
                                 ("shadow_rows_per_command", 256), ("shadow_trace_threads_per_threadgroup", 64)):
                report = valid_shading_report(True, mode, 64, 8); report["shading"][field] = value
                with self.subTest(mode=mode, field=field), self.assertRaisesRegex(ValueError, "Shadow execution settings"):
                    self.validate(report, True, mode, 64, 8)

    def test_prepared_rays_require_full_exact_coverage_and_valid_activity(self):
        for hard, field, value in ((False, "pixels", 255), (False, "gate", "close enough"),
                                  (False, "different_pixels", 1), (False, "fault_pixels", 1),
                                  (False, "active_rays", 1), (True, "active_rays", 101),
                                  (True, "active_rays", 0), (True, "active_rays", True)):
            report = valid_shading_report(hard); report["shading"]["shadow_preparation"][field] = value
            with self.subTest(hard=hard, field=field, value=value), self.assertRaises(ValueError):
                self.validate(report, hard)
        report = valid_shading_report(True); report["status"] = "FAIL"; report["shading"]["pass"] = False
        report["shading"]["shadow_preparation"]["different_pixels"] = 1
        self.validate(report, True)

    def test_gpu_hardware_and_pipeline_metadata_cannot_be_omitted(self):
        mutations = (
            lambda p: p.pop("hardware"),
            lambda p: p["hardware"].update(unified_memory=False),
            lambda p: p["hardware"].update(max_buffer_length=256*176-1),
            lambda p: p["hardware"].update(recommended_max_working_set_size=0),
            lambda p: p["hardware"].update(max_threads_per_threadgroup=[32, 32]),
            lambda p: p["hardware"].update(max_threads_per_threadgroup=[16, 32, 32]),
            lambda p: p["shading"].pop("pipeline_limits"),
            lambda p: p["shading"]["pipeline_limits"]["shadow_prepare"].update(thread_execution_width=0),
            lambda p: p["shading"]["pipeline_limits"]["shadow_prepare"].update(thread_execution_width=24),
            lambda p: p["shading"]["pipeline_limits"]["shadow_prepare"].update(max_total_threads_per_threadgroup=16),
            lambda p: p["shading"]["pipeline_limits"]["shadow_prepare"].update(threads_per_threadgroup=64),
            lambda p: p["shading"]["pipeline_limits"]["shadow_prepare"].update(grid_threads=255),
            lambda p: p["shading"]["pipeline_limits"]["shadow_march"].update(grid_threads=256),
        )
        for mutate in mutations:
            report = valid_shading_report(); mutate(report)
            with self.subTest(mutation=mutate), self.assertRaises(ValueError):
                self.validate(report)

    def test_preparation_timing_distinguishes_benchmark_from_diagnostic(self):
        for mode in ("legacy", "prepared"):
            self.validate(valid_shading_report(False, mode), mode=mode)
            mutations = [
                lambda p: p["timing_stages"]["shadow_preparation"].update(first_gpu_ms=1 if mode == "legacy" else 0),
                lambda p: p["timing_stages"]["shadow_preparation"].update(gpu_ms=[0, 0, 0] if mode == "prepared" else [1, 1, 1]),
                lambda p: p.update(max_shadow_prepare_command_ms=1 if mode == "legacy" else 0),
                lambda p: p.update(max_hard_shadow_command_ms=6),
            ]
            if mode == "prepared":
                mutations.append(lambda p: p.update(max_shadow_prepare_command_ms=6))
            for mutate in mutations:
                report = valid_shading_report(False, mode); mutate(report["shading"])
                with self.subTest(mode=mode, mutation=mutate), self.assertRaises(ValueError):
                    self.validate(report, mode=mode)

    def test_missing_or_loosened_intermediate_checks_fail(self):
        for stage in exact.shading_evidence.SURFACE_GATES:
            for key, value in (("pixels", 255), ("gate", "2e-6"), ("different_pixels", 1), ("fault_pixels", 1)):
                report = valid_shading_report(); report["shading"][stage][key] = value
                with self.subTest(stage=stage, key=key), self.assertRaises(ValueError):
                    self.validate(report)

    def test_rgb_requires_all_channels_at_zero_error(self):
        for stage in ("color_stage", "final_rgb"):
            for values in ({"channels": 767}, {"pixels": 255}, {"gate": "one level allowed"},
                           {"different_pixels": 1, "different_channels": 1, "max_channel_error": 1},
                           {"different_pixels": 0, "different_channels": 1}, {"fault_pixels": 1}):
                report = valid_shading_report(); report["shading"][stage].update(values)
                with self.subTest(stage=stage, values=values), self.assertRaises(ValueError):
                    self.validate(report)

    def test_primitive_and_shading_failures_keep_separate_status(self):
        report = valid_shading_report(); report["status"] = "FAIL"
        report["single_arithmetic"]["failures"] = 1
        report["single_arithmetic"]["operations"][0]["failures"] = 1
        self.validate(report)
        report["status"] = "PASS_SCOPED"
        with self.assertRaisesRegex(ValueError, "Qualification"):
            self.validate(report)
        report = valid_shading_report(); report["status"] = "FAIL"; report["shading"]["pass"] = False
        report["shading"]["hard_shadows"]["fault_pixels"] = 1
        self.validate(report)

    def test_binary32_all_operations_are_checked(self):
        for mutation in (lambda p: p.pop("single_arithmetic"),
                         lambda p: p["single_arithmetic"].update(operations=[]),
                         lambda p: p["single_arithmetic"]["operations"][0].update(failures=21),
                         lambda p: p["single_arithmetic"]["operations"][0].update(checks=0)):
            report = valid_shading_report(); mutation(report)
            with self.assertRaises(ValueError): self.validate(report)

    def test_negative_controls_repetitions_and_abi_are_required(self):
        for mutation in (lambda p: p.update(abi_checks=12),
                         lambda p: p["shading"].pop("negative_controls"),
                         lambda p: p["shading"]["negative_controls"].update(checks=0),
                         lambda p: p["shading"]["negative_controls"].update(checks=5635),
                         lambda p: p["shading"]["negative_controls"].update(checks=5637),
                         lambda p: p["shading"]["negative_controls"].update(cases=8),
                         lambda p: p["shading"]["negative_controls"].update(failures=1),
                         lambda p: p["shading"].update(gpu_repeats_bit_exact=False),
                         lambda p: p["shading"].update(cpu_repeats_bit_exact=False),
                         lambda p: p["shading"].update(pass_="true")):
            report = valid_shading_report(); mutation(report)
            if "pass_" in report["shading"]: report["shading"]["pass"] = report["shading"].pop("pass_")
            with self.assertRaises(ValueError): self.validate(report)

    def test_counts_require_integers_not_boolean_or_nan(self):
        for value in (True, 256.0, float("nan"), -1):
            report = valid_shading_report(); report["shading"]["final_rgb"]["pixels"] = value
            with self.assertRaises(ValueError): self.validate(report)

    def test_noop_shadows_and_discriminating_ao(self):
        for hard, key, value in ((False, "shadow_changed_pixels", 1), (True, "shadow_changed_pixels", 0),
                                 (False, "ao_changed_pixels", 0)):
            report = valid_shading_report(hard); report["shading"][key] = value
            with self.assertRaises(ValueError): self.validate(report, hard)

    def test_stage_and_full_timing_samples_must_agree(self):
        for mutation in (lambda s: s["timing_stages"].pop("hard_shadows"),
                         lambda s: s["timing_stages"]["hard_shadows"].update(gpu_ms=[3, 1]),
                         lambda s: s["timing_stages"]["hard_shadows"].update(median_gpu_ms=1),
                         lambda s: s.update(gpu_timestamp_ms=[16, 5, 10]),
                         lambda s: s.update(cpu_full_render_median_ms=41),
                         lambda s: s.update(first_encode_submit_wait_readback_ms=float("inf"))):
            report = valid_shading_report(); mutation(report["shading"])
            with self.assertRaises(ValueError): self.validate(report)

    def test_paths_cannot_be_replaced(self):
        for field in ("images", "buffers"):
            report = valid_shading_report(); report[field] = {"gpu": "../escape"}
            with self.assertRaises(ValueError): self.validate(report)

    def make_artifacts(self, folder, hard=False):
        report = valid_shading_report(hard)
        packed = bytearray(256 * 48)
        for i in range(256): struct.pack_into("<I", packed, i * 48 + 28, 5000)
        shadow = bytearray(packed)
        if hard: struct.pack_into("<I", shadow, 24, 1024)
        ao = bytearray(shadow); struct.pack_into("<I", ao, 28, 4999)
        color = bytes(16) * 256
        color = struct.pack("<IIII", 0xff030201, 0, 0, 0) + color[16:]
        rays = bytearray(256 * 96)
        for i in range(256): struct.pack_into("<I", rays, i * 96, 1)
        traces = bytearray(256 * 64)
        for i in range(256): struct.pack_into("<I", traces, i * 64, 1)
        if hard:
            struct.pack_into("<I", rays, 4, 1)
            struct.pack_into("<d", rays, 56, 1)  # Positive threshold for an active ray.
            struct.pack_into("<I", traces, 4, 1)
        for backend in ("gpu", "cpu"):
            for stage, data in (("primary", bytes(256*176)), ("packed", packed), ("shadow", shadow),
                                ("ao", ao), ("color", color), ("shadow-rays", rays), ("shadow-traces", traces)):
                (folder / f"{backend}-{stage}.bin").write_bytes(data)
        (folder / "gpu-baseline-primary.bin").write_bytes(bytes(256*176))
        (folder / exact.primary_evidence.NORMAL_BASELINE_FILE).write_bytes(bytes(256*176))
        (folder / "cpu-full-surface.bin").write_bytes(ao)
        (folder / "gpu-prepared-shadow.bin").write_bytes(shadow)
        rgb = bytes((1, 2, 3)) + bytes(255*3)
        for name in exact.shading_evidence.IMAGES.values():
            Image.frombytes("RGB", (16, 16), rgb).save(folder / name)
        return report

    def test_saved_stage_buffers_and_final_images_are_verified(self):
        with tempfile.TemporaryDirectory() as name:
            folder = Path(name); report = self.make_artifacts(folder, True)
            exact.shading_evidence.validate_buffers(folder, report, 16, 16)
            exact.shading_evidence.validate_final_images(folder, report, 16, 16)
            path = folder / "gpu-shadow.bin"; data = bytearray(path.read_bytes()); data[24] ^= 1; path.write_bytes(data)
            with self.assertRaisesRegex(ValueError, "surface bytes"):
                exact.shading_evidence.validate_buffers(folder, report, 16, 16)

    def test_truncated_or_missing_stage_cannot_pass(self):
        with tempfile.TemporaryDirectory() as name:
            folder = Path(name); report = self.make_artifacts(folder)
            (folder / "cpu-full-surface.bin").write_bytes(b"short")
            with self.assertRaisesRegex(ValueError, "ABI"):
                exact.shading_evidence.validate_buffers(folder, report, 16, 16)

    def test_saved_shadow_preparation_bytes_cannot_disagree_with_report(self):
        for backend in ("cpu", "gpu"):
            with self.subTest(backend=backend), tempfile.TemporaryDirectory() as name:
                folder = Path(name); report = self.make_artifacts(folder, True)
                path = folder / f"{backend}-shadow-rays.bin"; raw = bytearray(path.read_bytes())
                raw[16] ^= 1; path.write_bytes(raw)
                with self.assertRaisesRegex(ValueError, "Saved prepared shadow rays"):
                    exact.shading_evidence.validate_buffers(folder, report, 16, 16)

    def test_invalid_ray_records_do_not_pass_when_cpu_and_gpu_match(self):
        # Matching bytes alone cannot admit corrupted ABI, reserved values or nonfinite state.
        for offset, format_, value in ((0, "I", 2), (4, "I", 2), (8, "I", 2),
                                      (12, "I", 2), (16, "d", float("nan")),
                                      (80, "f", float("inf")), (88, "I", 1), (92, "I", 1),
                                      (56, "d", 0)):
            with self.subTest(offset=offset, value=value), tempfile.TemporaryDirectory() as name:
                folder = Path(name); report = self.make_artifacts(folder, True)
                for backend in ("cpu", "gpu"):
                    path = folder / f"{backend}-shadow-rays.bin"; raw = bytearray(path.read_bytes())
                    struct.pack_into("<" + format_, raw, offset, value); path.write_bytes(raw)
                if offset == 12: report["shading"]["shadow_preparation"]["fault_pixels"] = 1
                if offset == 4: report["shading"]["shadow_preparation"]["active_rays"] = 0
                with self.assertRaisesRegex(ValueError, "prepared shadow ray"):
                    exact.shading_evidence.validate_buffers(folder, report, 16, 16)

    def test_inactive_ray_payload_and_truncated_ray_buffers_are_rejected(self):
        with tempfile.TemporaryDirectory() as name:
            folder = Path(name); report = self.make_artifacts(folder)
            for backend in ("cpu", "gpu"):
                path = folder / f"{backend}-shadow-rays.bin"; raw = bytearray(path.read_bytes())
                struct.pack_into("<d", raw, 16, 1); path.write_bytes(raw)
            with self.assertRaisesRegex(ValueError, "Inactive"):
                exact.shading_evidence.validate_buffers(folder, report, 16, 16)
            (folder / "cpu-shadow-rays.bin").write_bytes(bytes(255*96))
            with self.assertRaisesRegex(ValueError, "ABI"):
                exact.shading_evidence.validate_buffers(folder, report, 16, 16)

    def test_saved_independent_traces_and_prepared_baseline_are_checked(self):
        for filename, offset in (("cpu-shadow-traces.bin", 24), ("gpu-shadow-traces.bin", 24),
                                 ("gpu-prepared-shadow.bin", 24)):
            with self.subTest(filename=filename), tempfile.TemporaryDirectory() as name:
                folder = Path(name); report = self.make_artifacts(folder, True)
                path = folder / filename; raw = bytearray(path.read_bytes()); raw[offset] ^= 1; path.write_bytes(raw)
                with self.assertRaisesRegex(ValueError, "Saved (independent shadow traces|surface bytes)"):
                    exact.shading_evidence.validate_buffers(folder, report, 16, 16)

    def test_trace_terminal_faults_have_valid_raw_records_without_becoming_render_failures(self):
        for terminal in (2, 4):
            with self.subTest(terminal=terminal), tempfile.TemporaryDirectory() as name:
                folder = Path(name); report = self.make_artifacts(folder, True)
                report["shading"]["shadow_traversal"]["terminal_fault_pixels"] = 1
                for backend in ("gpu", "cpu"):
                    path = folder / f"{backend}-shadow-rays.bin"; ray = bytearray(path.read_bytes())
                    struct.pack_into("<d", ray, 48, 2); struct.pack_into("<f", ray, 80, 1); path.write_bytes(ray)
                    path = folder / f"{backend}-shadow-traces.bin"; trace = bytearray(path.read_bytes())
                    struct.pack_into("<I", trace, 12, terminal)
                    struct.pack_into("<d", trace, 16, 1); struct.pack_into("<I", trace, 40, 1); path.write_bytes(trace)
                self.validate(report, True)
                exact.shading_evidence.validate_buffers(folder, report, 16, 16)

    def test_matching_trace_bytes_still_require_valid_flags_scalars_and_ray_relation(self):
        for offset, format_, value in ((0, "I", 2), (4, "I", 2), (8, "I", 2), (12, "I", 1),
                                      (16, "d", float("nan")), (24, "d", float("inf")), (32, "d", 1),
                                      (40, "I", 2), (44, "I", 1), (44, "I", 3), (48, "I", 2),
                                      (52, "I", 1), (56, "I", 1), (60, "I", 1), (16, "d", 1),
                                      (40, "I", 1)):
            with self.subTest(offset=offset, value=value), tempfile.TemporaryDirectory() as name:
                folder = Path(name); report = self.make_artifacts(folder, True)
                for backend in ("gpu", "cpu"):
                    path = folder / f"{backend}-shadow-traces.bin"; trace = bytearray(path.read_bytes())
                    struct.pack_into("<"+format_, trace, offset, value); path.write_bytes(trace)
                if offset == 4: report["shading"]["shadow_traversal"]["active_rays"] = 0
                if offset == 8: report["shading"]["shadow_traversal"]["fault_pixels"] = 1
                if offset == 12: report["shading"]["shadow_traversal"]["terminal_fault_pixels"] = 1
                with self.assertRaisesRegex(ValueError, "independent shadow trace|Independent shadow trace"):
                    exact.shading_evidence.validate_buffers(folder, report, 16, 16)

    def test_trace_minimum_must_be_an_entered_checkpoint_within_ray_length(self):
        for minimum in (-1, 3):
            with self.subTest(minimum=minimum), tempfile.TemporaryDirectory() as name:
                folder = Path(name); report = self.make_artifacts(folder, True)
                for backend in ("gpu", "cpu"):
                    path = folder / f"{backend}-shadow-rays.bin"; ray = bytearray(path.read_bytes())
                    struct.pack_into("<d", ray, 48, 2); struct.pack_into("<f", ray, 80, 1); path.write_bytes(ray)
                    path = folder / f"{backend}-shadow-traces.bin"; trace = bytearray(path.read_bytes())
                    struct.pack_into("<d", trace, 16, minimum); struct.pack_into("<I", trace, 40, 1); path.write_bytes(trace)
                with self.assertRaisesRegex(ValueError, "shadow trace"):
                    exact.shading_evidence.validate_buffers(folder, report, 16, 16)

    def test_image_cannot_be_substituted_after_comparison(self):
        with tempfile.TemporaryDirectory() as name:
            folder = Path(name); report = self.make_artifacts(folder)
            Image.new("RGB", (16, 16), (8, 9, 10)).save(folder / "gpu-final.ppm")
            with self.assertRaisesRegex(ValueError, "color records"):
                exact.shading_evidence.validate_final_images(folder, report, 16, 16)

    def test_gallery_labels_original_shadow_setting_and_cpu_scope(self):
        report = valid_shading_report(); report["device"] = "<script>wrong</script>"
        markup = exact.shading_evidence.gallery_html(report, [{"backend": "gpu", "png": "images/gpu-final.png"}])
        self.assertIn("жёсткие тени отключены", markup)
        self.assertIn("6 потоков", markup)
        self.assertIn("а не с оригинальным изображением Pascal", markup)
        self.assertIn("итоговый рендер", markup)
        self.assertNotIn("<script>", markup)
        enabled = exact.shading_evidence.gallery_html(valid_shading_report(True), [])
        self.assertIn("--hard-shadows", enabled)
        self.assertIn("CPU и GPU используют одинаковое включение", enabled)
        self.assertIn("отдельным параллельным GPU-проходом", enabled)
        self.assertIn("96 байт", enabled)
        legacy = exact.shading_evidence.gallery_html(valid_shading_report(True, "legacy"), [])
        self.assertIn("вне измеряемого рендера", legacy)
        sweep = exact.shading_evidence.gallery_html(valid_shading_report(True, sweep=True), [])
        self.assertIn("Подбор размеров запуска", sweep)
        self.assertIn("пробы исключены", sweep)
        parallel = exact.shading_evidence.gallery_html(valid_shading_report(True, "parallel"), [])
        self.assertIn("Лёгкий проход по строкам", parallel)
        self.assertIn("64 байта", parallel)
        self.assertIn("прежним GPU-проходом prepared", parallel)


class ExactDivisionNormalEvidenceTests(unittest.TestCase):
    def validate(self, report, full=False, **kwargs):
        return exact.validate_report(report, 16, 16, 6, 3, full_shading=full,
                                     shadow_mode="prepared", **kwargs)

    def test_division_and_normal_modes_are_bound_in_both_scopes(self):
        for full in (False, True):
            for division in ("verified", "restoring"):
                report = valid_shading_report() if full else valid_report()
                report["division_mode"] = division
                self.validate(report, full, division_mode=division)
                with self.assertRaisesRegex(ValueError, "Division execution"):
                    self.validate(report, full, division_mode="verified" if division == "restoring" else "restoring")
            for key in ("division_mode", "normal_pipeline"):
                report = valid_shading_report() if full else valid_report()
                report.pop(key)
                with self.subTest(full=full, key=key), self.assertRaises(ValueError):
                    self.validate(report, full)

    def test_inline_mode_requires_zero_separate_work(self):
        for full in (False, True):
            report = valid_shading_report() if full else valid_report()
            report["normal_pipeline"] = valid_normal_pipeline(mode="inline")
            if full:
                report["shading"]["timing_stages"]["primary"] = {
                    "first_gpu_ms": 5, "gpu_ms": [3, 1, 2], "median_gpu_ms": 2}
                report["shading"]["gpu_timestamp_ms"] = [
                    value-2 for value in report["shading"]["gpu_timestamp_ms"]]
                report["shading"]["gpu_timestamp_median_ms"] -= 2
            else:
                report["primary"].update(first_gpu_timestamp_ms=5,
                                          gpu_timestamp_ms=[3, 1, 2], gpu_timestamp_median_ms=2)
            self.validate(report, full, normal_mode="inline")
            report["normal_pipeline"]["gpu_ms"]["probes"][0] = 1
            with self.assertRaisesRegex(ValueError, "Inline normals"):
                self.validate(report, full, normal_mode="inline")

    def test_normal_abi_jobs_and_limits_are_required(self):
        changes = (
            lambda p: p.update(mode="inline"),
            lambda p: p.update(pixels=255),
            lambda p: p.update(scratch_record_bytes=63),
            lambda p: p.update(probe_jobs=256),
            lambda p: p.update(threads_per_threadgroup=64),
            lambda p: p.pop("pipeline_limits"),
            lambda p: p["pipeline_limits"]["probes"].update(thread_execution_width=24),
            lambda p: p["pipeline_limits"]["finish"].update(max_total_threads_per_threadgroup=16))
        for change in changes:
            report = valid_report(); change(report["normal_pipeline"])
            with self.subTest(change=change), self.assertRaises(ValueError): self.validate(report)

    def test_normal_baseline_cannot_be_reduced_or_ignored(self):
        for full in (False, True):
            for key, value in (("pixels", 255), ("gate", "epsilon"), ("different_pixels", 1),
                               ("fault_pixels", 1), ("different_pixels", True)):
                report = valid_shading_report() if full else valid_report()
                report["normal_pipeline"]["baseline"][key] = value
                with self.subTest(full=full, key=key), self.assertRaises(ValueError):
                    self.validate(report, full)
            report = valid_shading_report() if full else valid_report()
            report["normal_pipeline"]["baseline"]["different_pixels"] = 1
            report["normal_pipeline"]["pass"] = False
            report["status"] = "FAIL"
            if full: report["shading"]["pass"] = False
            self.validate(report, full)

    def test_normal_timing_cannot_hide_work_or_inflate_primary(self):
        changes = (
            lambda p: p["gpu_ms"].pop("finish"),
            lambda p: p["gpu_ms"].update(probes=[0]*3),
            lambda p: p["gpu_ms"].update(probes=[1, 1]),
            lambda p: p["first_gpu_ms"].update(probes=float("nan")),
            lambda p: p["median_gpu_ms"].update(probes=2),
            lambda p: p["gpu_ms"].update(geometry=[4, 1, 2]))
        for change in changes:
            report = valid_report(); change(report["normal_pipeline"])
            with self.subTest(change=change), self.assertRaises(ValueError): self.validate(report)

    def test_normal_control_coverage_and_faults_cannot_be_hidden(self):
        changes = (
            lambda s: s["abi"].update(checks=5),
            lambda s: s["abi"].update(values=[64, 0, 48, 56, 6, 160]),
            lambda s: s["negative_controls"].update(cases=7),
            lambda s: s["negative_controls"].update(checks=239),
            lambda s: s["negative_controls"].update(failures=1),
            lambda s: s["synthetic"].update(inactive_pixels=9),
            lambda s: s["synthetic"].update(active_pixels=0),
            lambda s: s["synthetic"].update(guard_failures=1),
            lambda s: s["synthetic"].update(inactive_sample_failures=1),
            lambda s: s["synthetic"].update(different_pixels=1))
        for full in (False, True):
            for change in changes:
                report = valid_shading_report() if full else valid_report()
                change(report["normal_pipeline"]["controls"])
                with self.subTest(full=full, change=change), self.assertRaises(ValueError):
                    self.validate(report, full)

    def test_saved_inline_normal_baseline_is_checked_over_all_bytes(self):
        for offset in (0, 72, 96, 152, 160, 168):
            with tempfile.TemporaryDirectory() as directory:
                folder = Path(directory)
                raw = bytearray(256*176)
                (folder / "gpu-primary.bin").write_bytes(raw)
                raw[offset] = 1
                (folder / exact.primary_evidence.NORMAL_BASELINE_FILE).write_bytes(raw)
                with self.subTest(offset=offset), self.assertRaises(ValueError):
                    exact.primary_evidence.validate_normal_buffers(folder, valid_normal_pipeline(), 16, 16)
        with tempfile.TemporaryDirectory() as directory:
            folder = Path(directory)
            (folder / "gpu-primary.bin").write_bytes(bytes(256*176))
            with self.assertRaises(ValueError):
                exact.primary_evidence.validate_normal_buffers(folder, valid_normal_pipeline(), 16, 16)

    def test_division_seed_denominators_and_oracles_cannot_be_weakened(self):
        changes = (
            lambda s: s.update(records=1), lambda s: s.update(checks=175152),
            lambda s: s.update(gate="NaN class"), lambda s: s.update(oracle="shared division"),
            lambda s: s.update(gpu_timestamp_ms=0), lambda s: s["cases"].pop(),
            lambda s: s["cases"][0].update(records=14591),
            lambda s: s["cases"][0].update(required_accepts=0, required_rejections=14592))
        for change in changes:
            report = valid_report(); change(report["division_seed_controls"])
            with self.subTest(change=change), self.assertRaises(ValueError): self.validate(report)

    def test_division_correction_and_rejection_branches_are_required(self):
        for name in ("minus_one", "plus_one", "minus_two", "plus_two"):
            report = valid_report()
            case = next(row for row in report["division_seed_controls"]["cases"] if row["case"] == name)
            case["correction_accepts"] = 0
            with self.subTest(case=name), self.assertRaises(ValueError): self.validate(report)
        for name in ("minus_three", "plus_three", "limit", "uint64_max", "invalid_domain"):
            report = valid_report()
            case = next(row for row in report["division_seed_controls"]["cases"] if row["case"] == name)
            case.update(required_accepts=1, required_rejections=case["records"]-1,
                        accepted_digits=1, rejected_digits=case["records"]-1)
            with self.subTest(case=name), self.assertRaises(ValueError): self.validate(report)

    def test_division_failures_cannot_hide_from_overall_status(self):
        for full in (False, True):
            report = valid_shading_report() if full else valid_report()
            control = report["division_seed_controls"]
            control["pass"] = False; control["failures"] = 1
            control["cases"][0]["value_failures"] = 1
            with self.assertRaises(ValueError): self.validate(report, full)
            report["status"] = "FAIL"
            self.validate(report, full)

    def test_power_two_division_denominator_is_required(self):
        for key, value in (("power_two_division_checks", 0), ("power_two_division_checks", 218191)):
            report = valid_report(); report["arithmetic"][key] = value
            with self.assertRaises(ValueError): self.validate(report)

    def test_normal_cli_invalid_bounds_and_modes_reject_before_execution(self):
        for option, values in (("--normal-threads", ("0", "-1", "1025")),
                               ("--normal-mode", ("other",)),
                               ("--division-mode", ("float",))):
            for value in values:
                with contextlib.redirect_stderr(io.StringIO()) as output, self.assertRaises(SystemExit):
                    exact.main(["--probe", "missing", "--library", "missing.metallib", "--output", "unused",
                                option, value])
                self.assertNotIn("not found", output.getvalue())


if __name__ == "__main__":
    unittest.main()
