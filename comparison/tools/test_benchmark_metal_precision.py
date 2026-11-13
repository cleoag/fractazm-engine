"""Evidence-admission tests; synthetic PNGs only, no Metal/CPU rendering."""
import contextlib
import copy
import io
import json
from pathlib import Path
import shutil
import tempfile
import unittest
from unittest import mock

from PIL import Image

import benchmark_metal_precision as bench
import test_benchmark_render_arm64 as fixture_module


class MetalPrecisionTests(unittest.TestCase):
    def setUp(self):
        # Reuse the complete synthetic stage/fault inventory of the production
        # pipeline tests, then add the distinct profile admission contract.
        helper = fixture_module.FullRenderBenchmarkTests("runTest")
        helper.setUp()
        self.addCleanup(helper.doCleanups)
        self.helper = helper
        self.root, self.settings = helper.root, helper.settings
        self.scene = {"frozen_path": self.root / "scene.m3p", "scene_sha256": "a" * 64,
                      "required_stages": ["preparation", "primary", "shadows", "ao", "color"]}

    def report(self, mode="compatible"):
        result = self.helper.report("metal")
        result.update(precision=mode, library_precision=mode, library=str(self.root / (mode + ".metallib")),
                      library_profile={"profile_id": 1 if mode == "compatible" else 2,
                                       "scene_abi_version": 4, "arithmetic_bits": 80 if mode == "compatible" else 32},
                      profile_validation_dispatches=1,
                      numeric_profile=("EXACT_" if mode == "compatible" else "NATIVE_FP32_") + "SPHEREIFS_FULL_SHADING",
                      scene=str(self.scene["frozen_path"]), scene_description={"scene_sha256": self.scene["scene_sha256"]},
                      execution={"precision": mode, "cpu_preparation_precision": mode,
                                 "shader_precision": "software_binary64_extended80" if mode == "compatible" else "hardware_float32"})
        for frame in [result["cold"], *result["warmups"], *result["samples"]]:
            frame["nonfinite_primary_values"] = 0
        return result

    def validate(self, value, mode="compatible"):
        return bench.validate_report(value, self.root, mode, self.root / (mode + ".metallib"), self.settings, self.scene)

    def test_both_profiles_have_identical_full_pipeline_admission(self):
        for mode in bench.MODES:
            with self.subTest(mode=mode):
                rgb, images = self.validate(self.report(mode), mode)
                self.assertEqual(rgb, self.helper.rgb)
                self.assertEqual(len(images), 5)

    def test_reject_missing_or_swapped_precision_and_gpu_marker(self):
        for key in ("precision", "execution", "library", "library_precision", "library_profile", "profile_validation_dispatches", "numeric_profile"):
            report = self.report("native")
            del report[key]
            with self.subTest(missing=key), self.assertRaises(ValueError):
                self.validate(report, "native")
        for key, value in (("precision", "compatible"), ("library", "/other.metallib"),
                           ("numeric_profile", "EXACT_SPHEREIFS_FULL_SHADING"),
                           ("profile_validation_dispatches", True), ("profile_validation_dispatches", 2),
                           ("runtime_shader_compilation", True)):
            report = self.report("native")
            report[key] = value
            with self.subTest(key=key, value=value), self.assertRaises(ValueError):
                self.validate(report, "native")
        for key in ("profile_id", "scene_abi_version", "arithmetic_bits"):
            report = self.report("native")
            report["library_profile"][key] = self.report()["library_profile"][key] if key != "scene_abi_version" else 3
            with self.subTest(marker=key), self.assertRaises(ValueError):
                self.validate(report, "native")
        report = self.report()
        report["library_profile"]["profile_id"] = True
        with self.assertRaises(ValueError):
            self.validate(report)

    def test_cpu_preparation_and_gpu_arithmetic_must_match_requested_mode(self):
        for key in ("precision", "cpu_preparation_precision", "shader_precision"):
            report = self.report("native")
            report["execution"][key] = self.report()["execution"][key]
            with self.subTest(key=key), self.assertRaises(ValueError):
                self.validate(report, "native")

    def test_same_library_bytes_are_not_two_profiles(self):
        paths = {mode: self.root / (mode + ".metallib") for mode in bench.MODES}
        for path in paths.values():
            path.write_bytes(b"same library")
        with self.assertRaisesRegex(ValueError, "distinct"):
            bench.validate_libraries(paths)
        paths["native"].write_bytes(b"native profile library")
        self.assertEqual(set(bench.validate_libraries(paths)), set(bench.MODES))

    def test_reject_hidden_oracle_fallback_or_diagnostics(self):
        for key in ("cpu_oracle_renders", "cpu_compute_fallback_calls", "diagnostic_dispatches"):
            report = self.report("native")
            report[key] = 1
            with self.subTest(key=key), self.assertRaises(ValueError):
                self.validate(report, "native")

    def test_reject_faults_nonfinite_values_and_missing_stage_inventory(self):
        for mode in bench.MODES:
            for change in ("fault", "nonfinite", "missing_finite_count", "missing_stage", "inactive_ao", "invalid_time"):
                report = self.report(mode)
                frame = report["samples"][1]
                if change == "fault": frame["faults"]["primary"] = 1
                if change == "nonfinite": frame["nonfinite_primary_values"] = 1
                if change == "missing_finite_count": del frame["nonfinite_primary_values"]
                if change == "missing_stage": del frame["stages"]["primary"]
                if change == "inactive_ao": frame["activity"]["ao_changed_pixels"] = 0
                if change == "invalid_time": frame["render_ms"] = float("nan")
                with self.subTest(mode=mode, change=change), self.assertRaises(ValueError):
                    self.validate(report, mode)

    def test_reject_changed_scene_and_unsupported_effects(self):
        for change in ("path", "sha", "reflection", "ao"):
            report = self.report()
            if change == "path": report["scene"] = "other.m3p"
            if change == "sha": report["scene_description"]["scene_sha256"] = "b" * 64
            if change == "reflection": report["loaded_features"]["reflections_transparency"] = True
            if change == "ao": report["loaded_features"]["ambient_occlusion"] = False
            with self.subTest(change=change), self.assertRaises(ValueError):
                self.validate(report)

    def test_rgb_is_decoded_instead_of_trusting_claimed_repeat(self):
        report = self.report("native")
        rgb = bytearray(self.helper.rgb)
        rgb[0] -= 1
        Image.frombytes("RGB", (2, 2), bytes(rgb)).save(self.root / "warm-002.png")
        with self.assertRaisesRegex(ValueError, "Decoded Metal RGB"):
            self.validate(report, "native")

    def test_representative_image_mutation_after_admission_is_rejected(self):
        self.report()
        path = self.root / "cold.png"
        recorded = bench.image_details(path, 2, 2)
        bench.verify_representative_image(path, recorded)
        rgb = bytearray(self.helper.rgb)
        rgb[0] -= 1
        Image.frombytes("RGB", (2, 2), bytes(rgb)).save(path)
        with self.assertRaisesRegex(ValueError, "changed after admission"):
            bench.verify_representative_image(path, recorded)

    def test_missing_reordered_and_escaping_frames_rejected(self):
        for change in ("missing", "order", "escape"):
            report = self.report()
            if change == "missing": report["samples"].pop()
            if change == "order": report["samples"][0]["index"] = 2
            if change == "escape": report["samples"][0]["image"] = "../cold.png"
            with self.subTest(change=change), self.assertRaises(ValueError):
                self.validate(report)

    def test_cross_profile_difference_allowed_but_each_process_must_repeat(self):
        references = {}
        bench.admit_profile_reference(references, "compatible", b"\x00\x01\x02")
        bench.admit_profile_reference(references, "native", b"\x00\x01\x03")
        bench.admit_profile_reference(references, "native", b"\x00\x01\x03")
        self.assertEqual(bench.rgb_difference(references["compatible"], references["native"])["different_pixels"], 1)
        with self.assertRaises(ValueError):
            bench.admit_profile_reference(references, "native", b"\x00\x01\x04")

    def test_aggregation_uses_median_per_process_and_keeps_gpu_separate(self):
        processes = []
        for values in ([1, 1, 101], [2, 2, 101], [3, 101, 101]):
            report = self.report()
            for frame, value in zip(report["samples"], values):
                frame["render_and_readback_ms"] = value
            processes.append({"native": report, "process": {"wall_seconds": 1, "cpu_seconds": 2, "peak_rss_bytes": 3}})
        summary = bench.summarize_processes(processes)
        self.assertEqual(summary["warm"]["render_and_readback_ms"]["median"], 2)
        self.assertEqual(summary["warm"]["gpu_timestamp_ms"]["median"], 7)
        self.assertEqual(bench.mode_order(0), ("compatible", "native"))
        self.assertEqual(bench.mode_order(1), ("native", "compatible"))

    def test_quiet_guard_rejects_metal_and_tests_without_terminating(self):
        for command in ("/tools/metal", "/tools/metallib", "/tools/ctest", "/build/mb3d_metal_exact_scene"):
            with self.subTest(command=command), mock.patch.object(bench, "cpu_quiet_host"), \
                 mock.patch.object(bench.subprocess, "check_output", return_value=command), self.assertRaises(ValueError):
                bench.quiet_host()
        with mock.patch.object(bench, "cpu_quiet_host", side_effect=ValueError("build active")), self.assertRaises(ValueError):
            bench.quiet_host()

    def test_only_admitted_scene_selection_and_new_output(self):
        for names in (("other",), ("sphereifs-ao-shadows",) * 2, ("MonteCarlo",), ("jit",)):
            with self.subTest(names=names), self.assertRaises(ValueError):
                bench.select_scenes(Path("unused"), names)
        with mock.patch.object(bench, "execute") as execute, contextlib.redirect_stderr(io.StringIO()):
            result = bench.main(["--metal", "unused", "--compatible-library", "unused", "--native-library", "unused", "--output", str(self.root)])
        self.assertEqual(result, 1)
        execute.assert_not_called()

    def test_retained_reports_and_logs_survive_working_directory_cleanup(self):
        output, gallery = self.root / "work", self.root / "gallery"
        for dirname in ("evidence/bin", "inputs", "runtime", "runs", "logs"):
            (output / dirname).mkdir(parents=True, exist_ok=True)
        (output / "evidence/bin/native.metallib").write_bytes(b"frozen native library")
        row = {"id": "sphereifs-ao-shadows", "profiles": {}, "warm_ratio_compatible_over_native": 1,
               "cross_profile_rgb": {"different_pixels": 1}}
        for mode in bench.MODES:
            directory = output / "runs" / mode
            directory.mkdir()
            raw = self.report(mode)
            (directory / "report.json").write_text(json.dumps(raw))
            (output / "logs" / (mode + ".log")).write_text("retained process log")
            shutil.copy2(self.root / "cold.png", directory / "cold.png")
            process = {"directory": "runs/" + mode, "report": "runs/" + mode + "/report.json",
                       "log": "logs/" + mode + ".log", "native": raw,
                       "images": [{"path": "cold.png", **bench.image_details(directory / "cold.png", 2, 2)}],
                       "process": {"wall_seconds": 1, "cpu_seconds": 1, "peak_rss_bytes": 1}}
            process["report_sha256"] = bench.sha(output / process["report"])
            process["log_sha256"] = bench.sha(output / process["log"])
            row["profiles"][mode] = {"processes": [process], "summary": bench.summarize_processes([process])}
        report = {"status": "COMPLETE", "scenes": [row], "disk_free_bytes": {"minimum_observed": 100 * 1024**3, "observations": []}}
        with mock.patch.object(bench, "check_disk"), mock.patch.object(bench, "observe_disk"):
            bench.export_gallery(report, output, gallery)
        shutil.rmtree(output)
        retained = json.loads((gallery / "summary.json").read_text())
        for profile in retained["scenes"][0]["profiles"].values():
            for process in profile["processes"]:
                self.assertEqual(bench.sha(gallery / process["report"]), process["report_sha256"])
                self.assertEqual(bench.sha(gallery / process["log"]), process["log_sha256"])
        for entry in json.loads((gallery / "manifest.json").read_text())["files"]:
            self.assertEqual(bench.sha(gallery / entry["file"]), entry["sha256"])
        self.assertEqual(retained, report)


if __name__ == "__main__":
    unittest.main()
