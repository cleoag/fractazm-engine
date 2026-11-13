"""Focused evidence checks; synthetic PNGs only, no renderer or timings run."""
from pathlib import Path
import tempfile
import unittest
from unittest import mock

from PIL import Image

import benchmark_precision as bench


class PrecisionBenchmarkTests(unittest.TestCase):
    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory()
        self.addCleanup(self.temporary.cleanup)
        self.root = Path(self.temporary.name)
        self.settings = {"width": 2, "height": 2, "threads": 6, "warmups": 1, "repeats": 3}
        self.scene = {"frozen_path": self.root / "scene.m3p", "required_stages": ["preparation", "primary", "color"]}
        self.runtime = self.root / "runtime"
        self.rgb = bytes(range(12))

    def frame(self, phase, index):
        name = "cold.png" if phase == "cold" else f"{phase}-{index:03d}.png"
        Image.frombytes("RGB", (2, 2), self.rgb).save(self.root / name)
        active = {"preparation", "primary", "color", "final_surface", "save"}
        result = {"phase": phase, "index": index, "image": name, "state": "completed",
                  "width": 2, "height": 2, "threads": 2, "exact_rgb_matches_cold": True,
                  "render_ms": 10, "render_and_readback_ms": 11, "end_to_end_ms": 13,
                  "readback_ms": 1, "png_encode_write_ms": 2,
                  "stages": {n: {"state": "completed" if n in active else "not_run",
                                  "seconds": .001 if n in active else 0} for n in bench.STAGES}}
        if phase == "cold":
            result.update(cold_total_ms=12, cold_end_to_end_ms=14)
        return result

    def report(self, mode="compatible"):
        return {"schema": "mb3d.cpu-render-benchmark.v1", "backend": "cpu", "architecture": "arm64",
                "status": "COMPLETE", "settings": dict(self.settings), "scene": str(self.scene["frozen_path"]),
                "resource_root": str(self.runtime), "loaded_features": {"load_success": True, "MCoptions": 0,
                    "formulas": [{"slot": i, "id": 0, "name": "", "iterations": 0} for i in range(6)]},
                "execution": {"precision": mode, "deterministic": True, "profile_timing": True, "work_control": "none"},
                "cold": self.frame("cold", 0), "warmups": [self.frame("warmup", 0)],
                "samples": [self.frame("warm", i) for i in range(3)]}

    def validate(self, report, mode="compatible"):
        return bench.validate_report(report, self.root, mode, self.settings, self.scene, self.runtime)

    def test_complete_profile(self):
        rgb, details = self.validate(self.report())
        self.assertEqual(rgb, self.rgb)
        self.assertEqual(len(details["images"]), 5)
        self.assertTrue(details["exact_within_process"])

    def test_missing_wrong_and_unknown_precision_rejected(self):
        for mode in (None, "native", "extended", ""):
            report = self.report()
            if mode is None:
                del report["execution"]["precision"]
            else:
                report["execution"]["precision"] = mode
            with self.subTest(mode=mode), self.assertRaisesRegex(ValueError, "precision"):
                self.validate(report)
        with self.assertRaisesRegex(ValueError, "Unknown"):
            self.validate(self.report(), "unknown")

    def test_metadata_and_inventory_required(self):
        for field in ("architecture", "loaded_features", "execution", "settings", "scene", "resource_root", "samples"):
            report = self.report(); del report[field]
            with self.subTest(field=field), self.assertRaises(ValueError):
                self.validate(report)

    def test_actual_rgb_rechecked_even_if_driver_claims_exact(self):
        report = self.report()
        Image.new("RGB", (2, 2), (255, 255, 255)).save(self.root / "warm-001.png")
        with self.assertRaisesRegex(ValueError, "RGB repeat"):
            self.validate(report)

    def test_effect_stage_and_nonfinite_timing_rejected(self):
        report = self.report(); report["samples"][0]["stages"]["primary"]["state"] = "not_run"
        with self.assertRaisesRegex(ValueError, "effect stage"):
            self.validate(report)
        report = self.report(); report["cold"]["render_ms"] = float("nan")
        with self.assertRaisesRegex(ValueError, "duration"):
            self.validate(report)

    def test_deferred_loaded_inputs_rejected(self):
        for field, value in (("MCoptions", 1), ("formulas", [{"name": "JITFormula"}])):
            report = self.report(); report["loaded_features"][field] = value
            with self.subTest(field=field), self.assertRaises(ValueError):
                self.validate(report)

    def test_cross_mode_rgb_difference_allowed(self):
        result = bench.rgb_difference(bytes((0, 1, 2, 3, 4, 5)), bytes((0, 3, 2, 6, 4, 5)))
        self.assertEqual(result["different_pixels"], 2)
        self.assertEqual(result["different_channels"], 2)
        self.assertEqual(result["maximum_channel_error"], 3)
        self.assertTrue(result["allowed"])
        self.assertFalse(result["exact"])

    def test_medians_pool_all_fresh_processes(self):
        processes = [{"cold": {key: value for key in (*bench.TIMINGS, "cold_total_ms")},
                      "warm": {key: [value, value + 1, value + 2] for key in bench.TIMINGS}}
                     for value in (3, 6, 9)]
        result = bench.summarize_processes(processes)
        self.assertEqual(result["cold_median_ms"]["render_ms"], 6)
        self.assertEqual(result["warm_median_ms"]["render_ms"], 7)

    def test_profile_order_alternates_per_cold_repetition(self):
        self.assertEqual([bench.mode_order(i) for i in range(3)],
                         [("compatible", "native"), ("native", "compatible"), ("compatible", "native")])

    def test_fresh_process_exactness_is_separate_for_each_profile(self):
        references = {}
        bench.admit_profile_reference(references, "compatible", b"abc")
        bench.admit_profile_reference(references, "native", b"xyz")
        bench.admit_profile_reference(references, "compatible", b"abc")
        with self.assertRaisesRegex(ValueError, "fresh-process"):
            bench.admit_profile_reference(references, "compatible", b"abd")
        self.assertEqual(references, {"compatible": b"abc", "native": b"xyz"})

    def test_disk_minimum_and_end_recorded_even_on_failure(self):
        start = bench.DISK_FLOOR + 100
        report = {"disk_free_bytes": {"start": start, "minimum_observed": start, "observations": []}}
        with mock.patch.object(bench.shutil, "disk_usage", return_value=mock.Mock(free=start - 200)):
            with self.assertRaisesRegex(ValueError, "12 GiB"):
                bench.observe_disk(report, self.root, "before")
            bench.observe_disk(report, self.root, "end", enforce=False)
        self.assertEqual(report["disk_free_bytes"]["minimum_observed"], start - 200)
        self.assertEqual(report["disk_free_bytes"]["end"], start - 200)
        self.assertEqual(len(report["disk_free_bytes"]["observations"]), 2)

    def test_fresh_output_rejected_before_subprocess(self):
        with mock.patch.object(bench, "native_host") as native:
            self.assertEqual(bench.main(["--cpu", "missing", "--output", str(self.root)]), 1)
        native.assert_not_called()

    def test_low_disk_fails(self):
        with mock.patch.object(bench.shutil, "disk_usage", return_value=mock.Mock(free=bench.DISK_FLOOR - 1)):
            with self.assertRaisesRegex(ValueError, "12 GiB"):
                bench.check_disk(self.root)

    def test_running_vm_rejected_without_mutation(self):
        result = mock.Mock(stdout="UUID STATUS IP_ADDR NAME\n{uuid} running - Windows 11\n")
        with mock.patch.object(bench.shutil, "which", return_value="prlctl"), \
             mock.patch.object(bench.subprocess, "run", return_value=result) as execute:
            with self.assertRaisesRegex(ValueError, "Parallels"):
                bench.quiet_host()
        execute.assert_called_once_with(["prlctl", "list", "-a"], text=True, capture_output=True, check=True)


if __name__ == "__main__":
    unittest.main()
