"""Evidence checks for the Metal wrapper; no GPU dispatch or compilation."""
import copy
import hashlib
from pathlib import Path
import tempfile
import unittest

from PIL import Image

import run_metal_formula_probe as probe


def valid_report():
    renders = []
    for name in probe.NAMES:
        row = {"name": name, "width": 16, "height": 16, "gpu_repeats_bit_exact": True,
               "gpu_image": name + "-gpu.ppm", "cpu_fp32_image": name + "-cpu-fp32.ppm",
               "cpu_fp64_image": name + "-cpu-fp64.ppm", "gpu_pixel_buffer": name + "-gpu-pixels.bin"}
        for stage in ("gpu_timestamp", "warm_encode_submit_wait_readback", "cpu_fp32", "cpu_fp64"):
            row[stage + "_ms"] = [1, 3, 2]
            row[stage + "_median_ms"] = 2
        for backend in probe.BACKENDS:
            row[backend] = {"hits": 100, "misses": 156, "primary_step_limit_pixels": 0,
                            "shadow_step_limit_pixels": 0, "completion": "COMPLETE", "nonconstant": True}
        renders.append(row)
    cohorts = []
    for name in ("finite_original_in_x87_range", "x87_fsincos_out_of_range", "fp32_input_overflow"):
        stats = {"values": 7, "strict_gate_failures": 2}
        cohorts.append({"name": name, "rows": 1, "gpu_failed_rows_at_existing_gate": 1,
                        "gpu_vs_original_x86": stats, "gpu_vs_cpu_fp32": stats,
                        "cpu_fp64_libm_vs_original_x86": stats})
    return {"schema_version": 1, "infrastructure_status": "PASS", "numeric_profile": "EXPERIMENTAL_FP32",
            "numerical_qualification": "NOT_QUALIFIED", "host_architecture": "arm64", "rosetta_translated": 0,
            "cpu_compute_fallback_calls": 0, "runtime_shader_compilation": False,
            "scene_parameters": {"width": 16, "height": 16}, "cpu_threads": 6, "measured_repeats": 3,
            "gpu_unresolved_primary_pixels": 0, "gpu_unresolved_shadow_pixels": 0, "renders": renders,
            "original_formula_fixture": {"rows": 3, "lanes_per_row": 7, "gpu_failed_rows_at_existing_gate": 3,
                                         "cohorts": cohorts, "primary_finite_original_in_range": cohorts[0]}}


class MetalProbeEvidenceTests(unittest.TestCase):
    def test_exact_double_diagnostic_is_required(self):
        expected = "probe.metal:4:38: error: type 'double' is not supported\n1 error generated.\n"
        self.assertEqual(probe.classify_double_compilation(1, False, expected), "EXPECTED_DOUBLE_UNSUPPORTED")
        self.assertEqual(probe.classify_double_compilation(0, False, expected), "UNEXPECTED_COMPILE_SUCCESS")
        self.assertEqual(probe.classify_double_compilation(1, True, expected), "TIMEOUT_NOT_EVIDENCE")

    def test_toolchain_and_secondary_errors_are_not_fp64_proof(self):
        diagnostics = ("xcrun: error: unable to find utility metal", "fatal error: metal_stdlib file not found",
                       "error: type 'double' is not supported\nerror: malformed option", "warning: double is unsupported")
        for value in diagnostics:
            with self.subTest(value=value):
                self.assertEqual(probe.classify_double_compilation(1, False, value), "UNEXPECTED_FAILURE_NOT_EVIDENCE")

    def test_valid_report_remains_unqualified(self):
        value = probe.validate_report(valid_report(), 16, 16, 6, 3)
        self.assertEqual(value["numerical_qualification"], "NOT_QUALIFIED")

    def test_native_no_fallback_and_no_runtime_compilation_are_required(self):
        for field, value in (("host_architecture", "x86_64"), ("rosetta_translated", 1),
                             ("cpu_compute_fallback_calls", 1), ("runtime_shader_compilation", True),
                             ("numerical_qualification", "PASS"), ("cpu_threads", 1)):
            report = valid_report()
            report[field] = value
            with self.subTest(field=field), self.assertRaises(ValueError):
                probe.validate_report(report, 16, 16, 6, 3)

    def test_bounded_pixels_are_recorded_and_cannot_be_complete(self):
        report = valid_report()
        report["renders"][0]["gpu"]["primary_step_limit_pixels"] = 5
        report["gpu_unresolved_primary_pixels"] = 5
        with self.assertRaisesRegex(ValueError, "incomplete"):
            probe.validate_report(report, 16, 16, 6, 3)
        report["renders"][0]["gpu"]["completion"] = "BOUNDED_WITH_UNRESOLVED_PIXELS"
        probe.validate_report(report, 16, 16, 6, 3)
        report["gpu_unresolved_primary_pixels"] = 0
        with self.assertRaisesRegex(ValueError, "totals"):
            probe.validate_report(report, 16, 16, 6, 3)

    def test_dimensions_duplicate_names_and_escaping_paths_are_rejected(self):
        for change in (lambda r: r["renders"][0].update(width=32),
                       lambda r: r["renders"][0].update(name="boxIFS"),
                       lambda r: r["renders"][0].update(gpu_image="../escape.ppm"),
                       lambda r: r["renders"][0].update(gpu_pixel_buffer="/tmp/escape.bin")):
            report = valid_report()
            change(report)
            with self.assertRaises(ValueError):
                probe.validate_report(report, 16, 16, 6, 3)

    def test_unsupported_cohorts_cannot_be_hidden_or_relabelled(self):
        report = valid_report()
        report["original_formula_fixture"]["cohorts"].pop()
        with self.assertRaisesRegex(ValueError, "cohorts"):
            probe.validate_report(report, 16, 16, 6, 3)
        report = valid_report()
        report["original_formula_fixture"]["rows"] = 1
        with self.assertRaisesRegex(ValueError, "totals"):
            probe.validate_report(report, 16, 16, 6, 3)

    def test_timing_samples_and_true_median_checked(self):
        variants = []
        bad = valid_report()
        bad["renders"][0]["gpu_timestamp_ms"] = [1, 2]
        variants.append(bad)
        bad = valid_report()
        bad["renders"][0]["gpu_timestamp_median_ms"] = 99
        variants.append(bad)
        bad = valid_report()
        bad["renders"][0]["gpu_timestamp_ms"][0] = float("nan")
        variants.append(bad)
        for report in variants:
            with self.assertRaises(ValueError):
                probe.validate_report(report, 16, 16, 6, 3)
        even = valid_report()
        even["measured_repeats"] = 4
        for row in even["renders"]:
            for field in ("gpu_timestamp", "warm_encode_submit_wait_readback", "cpu_fp32", "cpu_fp64"):
                row[field + "_ms"] = [4, 2, 1, 3]
                row[field + "_median_ms"] = 2.5
        probe.validate_report(even, 16, 16, 6, 4)

    def test_ppm_to_png_preserves_rgb_and_refuses_overwrite(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            source, destination = root / "original.ppm", root / "image.png"
            pixels = bytes((255, 1, 0, 0, 254, 2, 3, 0, 253, 252, 251, 250))
            Image.frombytes("RGB", (2, 2), pixels).save(source)
            result = probe.convert_ppm_losslessly(source, destination, 2, 2)
            self.assertEqual(result["rgb_sha256"], hashlib.sha256(pixels).hexdigest())
            self.assertTrue(result["lossless_rgb_conversion"])
            before = destination.read_bytes()
            with self.assertRaises(FileExistsError):
                probe.convert_ppm_losslessly(source, destination, 2, 2)
            self.assertEqual(before, destination.read_bytes())

    def test_ppm_conversion_rejects_wrong_dimensions_and_grayscale(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            source = root / "image.ppm"
            Image.new("RGB", (2, 2)).save(source)
            with self.assertRaises(ValueError):
                probe.convert_ppm_losslessly(source, root / "wrong.png", 3, 2)
            Image.new("L", (2, 2)).save(source)
            with self.assertRaises(ValueError):
                probe.convert_ppm_losslessly(source, root / "gray.png", 2, 2)


if __name__ == "__main__":
    unittest.main()
