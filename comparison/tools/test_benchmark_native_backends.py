"""Native CPU/Metal evidence admission; synthetic PNGs, no renderer runs."""
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

import benchmark_native_backends as bench
import test_benchmark_precision as cpu_fixtures
import test_benchmark_metal_precision as metal_fixtures


class NativeBackendTests(unittest.TestCase):
    def setUp(self):
        temporary = tempfile.TemporaryDirectory()
        self.addCleanup(temporary.cleanup)
        self.root = Path(temporary.name)
        self.settings = {"width": 2, "height": 2, "threads": 2, "warmups": 1, "repeats": 3}
        self.runtime = self.root / "runtime"
        self.runtime.mkdir()
        scene = self.root / "scene.m3p"
        scene.write_bytes(b"ordinary AO and hard-shadow scene")
        self.scene = {"id": "sphereifs-ao-shadows", "frozen_path": scene,
                      "scene_sha256": bench.sha(scene),
                      "required_stages": ["preparation", "primary", "shadows", "ao", "color"]}
        self.library = self.root / "native.metallib"
        self.library.write_bytes(b"separate native FP32 library")
        self.directories = {backend: self.root / backend for backend in ("cpu", "metal")}
        for directory in self.directories.values():
            directory.mkdir()
        self.cpu = cpu_fixtures.PrecisionBenchmarkTests("runTest")
        self.cpu.setUp()
        self.addCleanup(self.cpu.doCleanups)
        self.cpu.root = self.directories["cpu"]
        self.cpu.settings = dict(self.settings)
        self.cpu.scene = self.scene
        self.cpu.runtime = self.runtime
        self.metal = metal_fixtures.MetalPrecisionTests("runTest")
        self.metal.setUp()
        self.addCleanup(self.metal.doCleanups)
        self.metal.root = self.directories["metal"]
        self.metal.helper.root = self.directories["metal"]
        self.metal.settings = dict(self.settings)
        self.metal.helper.settings = dict(self.settings)
        self.metal.scene = self.scene

    def report(self, backend):
        if backend == "metal":
            report = self.metal.report("native")
            report["library"] = str(self.library)
            return report
        report = self.cpu.report("native")
        report["setup_ms"] = {"load": 1.0, "ordinary_input_check": 0.1}
        report["loaded_features"].update(bCalculateHardShadow=7, bCalcAmbShadowAutomatic=21,
                                         bNormalsOnDE=1, bCalcSRautomatic=0, bCalcDOFtype=0,
                                         byCalcNsOnZBufAuto=0, bVolLightNr=0)
        for frame in [report["cold"], *report["warmups"], *report["samples"]]:
            frame["preparation_ms"] = frame["stages"]["preparation"]["seconds"] * 1000
            for name in ("shadows", "ao"):
                frame["stages"][name] = {"state": "completed", "seconds": 0.001}
            if frame["phase"] == "cold":
                frame["cold_total_ms"] = 13.0
                frame["cold_end_to_end_ms"] = 15.0
                frame["end_to_end_ms"] = frame["cold_end_to_end_ms"]
                frame["stages"]["load"] = {"state": "completed", "seconds": 0.001}
        return report

    def validate(self, report, backend):
        if backend == "cpu":
            return bench.validate_cpu_report(report, self.directories[backend], self.settings,
                                              self.scene, self.runtime)
        return bench.validate_metal_report(report, self.directories[backend], self.library,
                                            self.settings, self.scene)

    def test_native_full_pipeline_reports_admitted_for_both_backends(self):
        for backend in ("cpu", "metal"):
            with self.subTest(backend=backend):
                rgb, images = self.validate(self.report(backend), backend)
                self.assertEqual(len(rgb), 12)
                self.assertEqual(len(images), 5)

    def test_precision_backend_and_architecture_cannot_be_swapped(self):
        for backend in ("cpu", "metal"):
            for change in ("precision", "missing_precision", "backend", "schema", "status", "architecture"):
                report = self.report(backend)
                if change == "precision": report["execution"]["precision"] = "compatible"
                if change == "missing_precision": del report["execution"]["precision"]
                if change == "backend": report["backend"] = "metal" if backend == "cpu" else "cpu"
                if change == "schema": report["schema"] = "mb3d.other-render-benchmark.v1"
                if change == "status": report["status"] = "FAILED"
                if change == "architecture":
                    if backend == "cpu": report["architecture"] = "x86_64"
                    else: report["translated"] = True
                with self.subTest(backend=backend, change=change), self.assertRaises(ValueError):
                    self.validate(report, backend)

    def test_metal_library_marker_and_cpu_preparation_must_be_native(self):
        for change in ("library", "library_precision", "marker_id", "marker_bits", "marker_abi", "preparation", "shader"):
            report = self.report("metal")
            if change == "library": report["library"] = str(self.root / "compatible.metallib")
            if change == "library_precision": report["library_precision"] = "compatible"
            if change == "marker_id": report["library_profile"]["profile_id"] = 1
            if change == "marker_bits": report["library_profile"]["arithmetic_bits"] = 80
            if change == "marker_abi": report["library_profile"]["scene_abi_version"] = 3
            if change == "preparation": report["execution"]["cpu_preparation_precision"] = "compatible"
            if change == "shader": report["execution"]["shader_precision"] = "software_binary64_extended80"
            with self.subTest(change=change), self.assertRaises(ValueError):
                self.validate(report, "metal")

    def test_required_stages_must_exist_complete_and_have_finite_times(self):
        for backend in ("cpu", "metal"):
            for change in ("missing", "failed", "not_run", "nan", "negative", "bool"):
                report = self.report(backend)
                stage = report["samples"][1]["stages"]["primary"]
                if change == "missing": del report["samples"][1]["stages"]["primary"]
                if change == "failed": stage["state"] = "failed"
                if change == "not_run": stage.update(state="not_run", seconds=0)
                if change == "nan": stage["seconds"] = float("nan")
                if change == "negative": stage["seconds"] = -1
                if change == "bool": stage["seconds"] = True
                with self.subTest(backend=backend, change=change), self.assertRaises(ValueError):
                    self.validate(report, backend)

    def test_missing_cold_and_wrong_repetition_inventory_rejected(self):
        for backend in ("cpu", "metal"):
            for change in ("cold", "warmup", "sample", "reordered", "escape"):
                report = self.report(backend)
                if change == "cold": del report["cold"]
                if change == "warmup": report["warmups"].pop()
                if change == "sample": report["samples"].pop()
                if change == "reordered": report["samples"][1]["index"] = 0
                if change == "escape": report["samples"][0]["image"] = "../cold.png"
                with self.subTest(backend=backend, change=change), self.assertRaises(ValueError):
                    self.validate(report, backend)

    def test_nonfinite_timing_and_impossible_boundaries_rejected(self):
        for backend in ("cpu", "metal"):
            for key in ("render_ms", "readback_ms", "render_and_readback_ms", "png_encode_write_ms", "end_to_end_ms"):
                for bad in (float("nan"), float("inf"), -1, True):
                    report = self.report(backend)
                    report["samples"][1][key] = bad
                    with self.subTest(backend=backend, key=key, bad=bad), self.assertRaises(ValueError):
                        self.validate(report, backend)
            for key in ("cold_total_ms", "cold_end_to_end_ms"):
                report = self.report(backend)
                report["cold"][key] = 0.1
                with self.subTest(backend=backend, key=key), self.assertRaises(ValueError):
                    self.validate(report, backend)

    def test_metal_faults_and_nonfinite_primary_cannot_qualify(self):
        for field in ("primary", "surface", "final_surface", "color", "shadow_rays", "shadow_traces", "normal_scratch"):
            report = self.report("metal")
            report["samples"][1]["faults"][field] = 1
            with self.subTest(field=field), self.assertRaises(ValueError):
                self.validate(report, "metal")
        for missing in (False, True):
            report = self.report("metal")
            if missing: del report["samples"][0]["nonfinite_primary_values"]
            else: report["samples"][0]["nonfinite_primary_values"] = 1
            with self.subTest(missing=missing), self.assertRaises(ValueError):
                self.validate(report, "metal")

    def test_cpu_cold_clock_gap_is_allowed_but_preparation_inventory_is_bound(self):
        report = self.report("cpu")
        report["cold"]["cold_end_to_end_ms"] += 0.05
        report["cold"]["end_to_end_ms"] += 0.05
        self.validate(report, "cpu")
        for change in ("missing_setup", "nan_setup", "preparation", "inactive_duration", "error", "workers"):
            report = self.report("cpu")
            if change == "missing_setup": del report["setup_ms"]["ordinary_input_check"]
            if change == "nan_setup": report["setup_ms"]["ordinary_input_check"] = float("nan")
            if change == "preparation": report["samples"][0]["preparation_ms"] += 1
            if change == "inactive_duration": report["samples"][0]["stages"]["volume"]["seconds"] = 1
            if change == "error": report["samples"][0]["error"] = "stage failure"
            if change == "workers": report["samples"][0]["threads"] = 1
            with self.subTest(change=change), self.assertRaises(ValueError):
                self.validate(report, "cpu")
    def test_disabled_ao_or_shadows_cannot_enter_common_effect_comparison(self):
        for backend in ("cpu", "metal"):
            fields = ("bCalculateHardShadow", "bCalcAmbShadowAutomatic") if backend == "cpu" else ("hard_shadows", "ambient_occlusion")
            for field in fields:
                report = self.report(backend)
                report["loaded_features"][field] = 0 if backend == "cpu" else False
                with self.subTest(backend=backend, field=field), self.assertRaises(ValueError):
                    self.validate(report, backend)

    def test_actual_rgb_rechecked_instead_of_claimed_repeat(self):
        for backend in ("cpu", "metal"):
            report = self.report(backend)
            path = self.directories[backend] / "warm-001.png"
            with Image.open(path) as image:
                changed = bytearray(image.convert("RGB").tobytes())
            changed[0] ^= 1
            Image.frombytes("RGB", (2, 2), bytes(changed)).save(path)
            with self.subTest(backend=backend), self.assertRaises(ValueError):
                self.validate(report, backend)

    def test_cross_backend_difference_allowed_but_fresh_process_repeat_exact(self):
        references = {}
        for backend in ("cpu", "metal"):
            rgb, _ = self.validate(self.report(backend), backend)
            bench.admit_backend_reference(references, backend, rgb)
            bench.admit_backend_reference(references, backend, rgb)
        difference = bench.rgb_difference(references["cpu"], references["metal"])
        self.assertGreater(difference["different_pixels"], 0)
        self.assertTrue(difference["allowed"])
        changed = bytearray(references["cpu"])
        changed[0] ^= 1
        with self.assertRaises(ValueError):
            bench.admit_backend_reference(references, "cpu", bytes(changed))
        with self.assertRaises(ValueError):
            bench.admit_backend_reference(references, "compatible", b"abc")

    def test_warm_aggregation_uses_median_of_process_medians(self):
        # Pooled nine-sample median is 3; the median of [1,2,101] is 2.
        for backend in ("cpu", "metal"):
            processes = []
            for values in ([1, 1, 101], [2, 2, 101], [3, 101, 101]):
                report = self.report(backend)
                for frame, value in zip(report["samples"], values):
                    frame["render_and_readback_ms"] = value
                processes.append({"native": report, "process": {"wall_seconds": 1, "cpu_seconds": 2, "peak_rss_bytes": 3}})
            summary = bench.summarize_processes(processes, backend)
            with self.subTest(backend=backend):
                self.assertEqual(summary["warm"]["render_and_readback_ms"]["median"], 2)
                if backend == "metal": self.assertEqual(summary["warm"]["gpu_timestamp_ms"]["median"], 7)
                else: self.assertNotIn("gpu_timestamp_ms", summary["warm"])
        self.assertEqual(bench.backend_order(0), ("cpu", "metal"))
        self.assertEqual(bench.backend_order(1), ("metal", "cpu"))

    def retention_fixture(self):
        output = self.root / "work"
        for dirname in ("evidence/bin", "inputs", "runtime", "runs", "logs"):
            (output / dirname).mkdir(parents=True, exist_ok=True)
        (output / "evidence/bin/native.metallib").write_bytes(b"retained native library")
        row = {"id": "sphereifs-ao-shadows", "backends": {}, "warm_ratio_cpu_over_metal": 1,
               "cold_ratio_cpu_over_metal": 1,
               "cross_backend_rgb": {"different_pixels": 1, "maximum_channel_error": 1,
                                     "mean_absolute_channel_error": 0.5}}
        for backend in ("cpu", "metal"):
            raw = self.report(backend)
            _, images = self.validate(raw, backend)
            directory = output / "runs" / backend
            directory.mkdir()
            (directory / "report.json").write_text(json.dumps(raw))
            (output / "logs" / (backend + ".log")).write_text("retained process log")
            for image in images:
                shutil.copy2(self.directories[backend] / image["path"], directory / image["path"])
            process = {"directory": "runs/" + backend, "report": "runs/" + backend + "/report.json",
                       "log": "logs/" + backend + ".log", "native": raw, "images": images,
                       "process": {"wall_seconds": 1, "cpu_seconds": 1, "peak_rss_bytes": 1}}
            process["report_sha256"] = bench.sha(output / process["report"])
            process["log_sha256"] = bench.sha(output / process["log"])
            row["backends"][backend] = {"processes": [process], "summary": bench.summarize_processes([process], backend)}
        report = {"status": "COMPLETE", "scenes": [row],
                  "disk_free_bytes": {"minimum_observed": 100 * 1024**3, "observations": []}}
        return output, report

    def test_retained_reports_and_manifest_images_survive_workdir_cleanup(self):
        output, report = self.retention_fixture()
        gallery = self.root / "gallery"
        with mock.patch.object(bench, "check_disk"), mock.patch.object(bench, "observe_disk"):
            bench.export_gallery(report, output, gallery)
        shutil.rmtree(output)
        retained = json.loads((gallery / "summary.json").read_text())
        for backend in retained["scenes"][0]["backends"].values():
            for process in backend["processes"]:
                self.assertEqual(bench.sha(gallery / process["report"]), process["report_sha256"])
                self.assertEqual(bench.sha(gallery / process["log"]), process["log_sha256"])
        entries = json.loads((gallery / "manifest.json").read_text())["files"]
        self.assertGreaterEqual(sum(row["file"].endswith(".png") for row in entries), 2)
        for entry in entries:
            self.assertEqual(bench.sha(gallery / entry["file"]), entry["sha256"])

    def test_mutated_representative_image_or_report_is_rejected_before_export(self):
        for changed in ("image", "report"):
            with self.subTest(changed=changed):
                output, report = self.retention_fixture()
                if changed == "image":
                    Image.new("RGB", (2, 2), (100, 100, 100)).save(output / "runs/cpu/cold.png")
                else:
                    (output / "runs/cpu/report.json").write_text("{}")
                gallery = self.root / ("gallery-" + changed)
                with mock.patch.object(bench, "check_disk"), mock.patch.object(bench, "observe_disk"), self.assertRaises(ValueError):
                    bench.export_gallery(report, output, gallery)
                shutil.rmtree(output)

    def test_existing_output_fails_before_any_renderer_launch(self):
        with mock.patch.object(bench, "execute") as execute, contextlib.redirect_stderr(io.StringIO()):
            result = bench.main(["--cpu", "unused", "--renderer", "unused", "--metal", "unused",
                                 "--native-library", "unused", "--output", str(self.root)])
        self.assertEqual(result, 1)
        execute.assert_not_called()


if __name__ == "__main__":
    unittest.main()
