"""Evidence integrity tests for full CPU/Metal benchmarks; no renders run here."""
import copy
import contextlib
import hashlib
import io
import json
from pathlib import Path
import shutil
import tempfile
import unittest
from unittest import mock

from PIL import Image

import benchmark_render_arm64 as bench


class FullRenderBenchmarkTests(unittest.TestCase):
    def setUp(self):
        temporary = tempfile.TemporaryDirectory()
        self.addCleanup(temporary.cleanup)
        self.root = Path(temporary.name)
        self.settings = {"width": 2, "height": 2, "threads": 2, "warmups": 1, "repeats": 3}
        self.rgb = bytes((255, 0, 0, 0, 255, 0, 0, 0, 255, 255, 255, 255))

    def write_json(self, value):
        path = self.root / "selection.json"
        path.write_text(json.dumps(value))
        return path

    def selection(self):
        (self.root / "scene.m3p").write_bytes(b"pinned ordinary scene")
        (self.root / "control.m3p").write_bytes(b"pinned effect-off control")
        (self.root / "height.pgm").write_bytes(b"P5\n2 2\n255\n\x00\x20\x40\xff")
        return {"schema": bench.SELECTION_SCHEMA, "expected_count": 1,
                "expected_control_count": 1,
                "assets": [{"id": "map", "source_path": "height.pgm", "runtime_path": "M3Maps/1.pgm",
                            "sha256": bench.sha(self.root / "height.pgm")}],
                "scenes": [{"id": "ordinary_map", "scene_path": "scene.m3p",
                            "scene_sha256": bench.sha(self.root / "scene.m3p"),
                            "backends": ["cpu"], "unsupported_backends": {"metal": "Map sampling unsupported"},
                            "required_stages": ["primary", "ao", "color"], "assets": ["map"],
                            "effect_controls": [{"scene_path": "control.m3p",
                                                 "scene_sha256": bench.sha(self.root / "control.m3p"),
                                                 "required_not_run_stages": ["ao"]}]}]}

    def frame(self, phase, index):
        image = "cold.png" if phase == "cold" else f"{phase}-{index:03d}.png"
        Image.frombytes("RGB", (2, 2), self.rgb).save(self.root / image)
        active = {"preparation", "primary", "ao", "color"}
        result = {"phase": phase, "index": index, "image": image, "state": "completed",
                  "width": 2, "height": 2, "exact_rgb_matches_cold": True,
                  "render_ms": 10.0, "readback_ms": 1.0, "render_and_readback_ms": 11.0,
                  "png_encode_write_ms": 2.0, "end_to_end_ms": 13.0,
                  "stages": {name: {"state": "completed" if name in active else "not_run",
                                    "seconds": 0.001 if name in active else 0.0}
                             for name in bench.CPU_STAGES}}
        if phase == "cold":
            result.update(cold_total_ms=20.0, cold_end_to_end_ms=22.0, end_to_end_ms=22.0)
        return result

    def report(self, backend="cpu"):
        result = {"schema": f"mb3d.{backend}-render-benchmark.v1", "backend": backend,
                  "status": "COMPLETE", "settings": dict(self.settings),
                  "execution": {"precision": "compatible", "deterministic": True, "profile_timing": True, "work_control": "none"},
                  "cold": self.frame("cold", 0), "warmups": [self.frame("warmup", 0)],
                  "samples": [self.frame("warm", i) for i in range(3)]}
        if backend == "metal":
            result["settings"].update(bench.METAL_RENDER_SETTINGS)
            result.update(precision="compatible", native_arm64=True, translated=False, device="test GPU",
                          exact_rgb_repeats=True, exact_stage_repeats=True,
                          cpu_compute_fallback_calls=0, cpu_oracle_renders=0, diagnostic_dispatches=0,
                          loaded_features={"ambient_occlusion": True, "ao_mode": "HiQ", "de_normals": True,
                                           "formula_slots": 1, "hard_shadows": True, "zbuffer_normals": False,
                                           "reflections_transparency": False, "volume": False, "dof": False,
                                           "shadow_light_mask": 1, "ao_levels": 1},
                          render_pipeline_names=["exact_primary_geometry", "exact_normal_probes", "exact_normal_finish",
                                                 "exact_surface", "exact_shadow_prepare", "exact_shadow_trace",
                                                 "exact_hard_shadow_traced", "exact_ao_init", "exact_ao_sample",
                                                 "exact_ao_wavelet_h", "exact_ao_wavelet_v", "exact_color"],
                          setup_ms={"scene_preparation": 1.0, "shading_preparation": 1.0, "device_library_queue": 1.0,
                                    "pipeline_creation": 1.0, "buffer_allocation": 1.0, "total": 5.0})
            for frame in [result["cold"], *result["warmups"], *result["samples"]]:
                frame["exact_stage_records_match_cold"] = True
                frame["faults"] = {"primary": 0, "surface": 0, "final_surface": 0, "color": 0,
                                   "shadow_rays": 0, "shadow_traces": 0, "normal_scratch": 0,
                                   "latent_shadow_terminal_faults": 1}
                frame["stages"] = {name: {"state": "completed", "seconds": 0.001, "gpu_ms": 1.0}
                                   for name in ("primary", "surface_packing", "shadow_preparation", "shadow_traversal",
                                                "hard_shadows", "ambient_occlusion", "final_color")}
                frame["stages"]["surface_copy"] = {"state": "not_run", "seconds": 0, "gpu_ms": 0}
                frame["stages_gpu_ms"] = {k: v["gpu_ms"] for k, v in frame["stages"].items()}
                frame["gpu_timestamp_ms"] = 7.0
                frame["normal_pipeline_gpu_ms"] = {"geometry": 0.5, "probes": 0.25, "finish": 0.25}
                frame["activity"] = {"hits": 2, "misses": 2, "nonconstant_rgb": True,
                                     "shadow_changed_pixels": 1, "ao_changed_pixels": 2}
                frame["end_to_end_ms"] = 13.0
        return result

    def validate(self, report, backend="cpu"):
        return bench.validate_report(report, self.root, backend, self.settings, ("primary", "ao", "color"))

    def receipt_fixture(self):
        source = {"source_commit": "a" * 40, "changed_source_sha256": {
            "CMakeLists.txt": "1" * 64, "src/core/renderer.cpp": "2" * 64,
            "comparison/metal_exact/kernels.metal": "3" * 64, "src/removed.h": None,
            "comparison/tools/benchmark_render_arm64.py": "4" * 64, "docs/BUILD.md": "5" * 64}}
        identities = {role: {"sha256": str(index + 6) * 64} for index, role in enumerate(bench.RECEIPT_BINARIES)}
        receipt = {"status": "PASS_SCOPED", "scope": "ordinary", "machine": "arm64",
                   "platform": "macOS-26.0-arm64-arm-64bit", **copy.deepcopy(source),
                   "binaries": {name: {"arch": "arm64", "sha256": identities[role]["sha256"]}
                                for role, name in bench.RECEIPT_BINARIES.items()}}
        return source, identities, receipt

    def test_receipt_binds_build_inputs_and_all_three_binaries(self):
        source, identities, receipt = self.receipt_fixture()
        # Python/runbook edits made after compilation do not change build inputs.
        source["changed_source_sha256"]["comparison/tools/benchmark_render_arm64.py"] = "b" * 64
        source["changed_source_sha256"]["docs/BUILD.md"] = "c" * 64
        source["changed_source_sha256"]["tests/new_python_test.py"] = "d" * 64
        path = self.write_json(receipt)
        result = bench.bootstrap_binding(path, source, identities)
        self.assertEqual(result["status"], "VERIFIED")
        self.assertEqual(result["receipt_sha256"], bench.sha(path))
        self.assertEqual(set(result["binaries"]), {"cpu", "renderer", "metal"})
        self.assertNotIn("docs/BUILD.md", result["build_input_changed_source_sha256"])
        self.assertIsNone(result["build_input_changed_source_sha256"]["src/removed.h"])

    def test_receipt_rejects_failed_wrong_scope_arch_or_commit(self):
        source, identities, base = self.receipt_fixture()
        for field, value in (("status", "FAIL"), ("status", "RUNNING"), ("scope", "qualified"),
                             ("machine", "x86_64"), ("platform", "Windows-ARM64"),
                             ("source_commit", "b" * 40)):
            receipt = copy.deepcopy(base)
            receipt[field] = value
            with self.subTest(field=field, value=value), self.assertRaises(ValueError):
                bench.bootstrap_binding(self.write_json(receipt), source, identities)

    def test_receipt_rejects_stale_missing_and_added_compiled_inputs(self):
        source, identities, receipt = self.receipt_fixture()
        for path in ("CMakeLists.txt", "src/core/renderer.cpp", "comparison/metal_exact/kernels.metal",
                     "src/new.h", "src/new.hpp", "src/new.c", "src/new.mm", "src/new.inc"):
            changed = copy.deepcopy(source)
            changed["changed_source_sha256"][path] = "f" * 64
            with self.subTest(path=path), self.assertRaisesRegex(ValueError, "inputs differ"):
                bench.bootstrap_binding(self.write_json(receipt), changed, identities)
        changed = copy.deepcopy(source)
        del changed["changed_source_sha256"]["src/removed.h"]
        with self.assertRaisesRegex(ValueError, "inputs differ"):
            bench.bootstrap_binding(self.write_json(receipt), changed, identities)
        bad = copy.deepcopy(receipt)
        del bad["changed_source_sha256"]
        with self.assertRaisesRegex(ValueError, "inventory"):
            bench.bootstrap_binding(self.write_json(bad), source, identities)

    def test_receipt_rejects_missing_changed_or_non_native_executable(self):
        source, identities, base = self.receipt_fixture()
        for name in bench.RECEIPT_BINARIES.values():
            for kind in ("missing", "hash", "arch"):
                receipt = copy.deepcopy(base)
                if kind == "missing":
                    del receipt["binaries"][name]
                elif kind == "hash":
                    receipt["binaries"][name]["sha256"] = "f" * 64
                else:
                    receipt["binaries"][name]["arch"] = "x86_64"
                with self.subTest(name=name, kind=kind), self.assertRaisesRegex(ValueError, "binary"):
                    bench.bootstrap_binding(self.write_json(receipt), source, identities)

    def test_missing_optional_receipt_is_explicitly_not_supplied(self):
        source, identities, _ = self.receipt_fixture()
        self.assertEqual(bench.bootstrap_binding(None, source, identities)["status"], "NOT_SUPPLIED")

    def test_failed_receipt_rejected_before_creating_output_or_launching(self):
        source, identities, receipt = self.receipt_fixture()
        receipt["status"] = "FAIL"
        receipt_path = self.write_json(receipt)
        paths = {}
        for role in ("cpu", "renderer", "metal", "library"):
            paths[role] = self.root / role
            paths[role].write_bytes(role.encode())
        output = self.root / "must-not-exist"
        argv = ["--cpu", str(paths["cpu"]), "--renderer", str(paths["renderer"]),
                "--metal-probe", str(paths["metal"]), "--library", str(paths["library"]),
                "--bootstrap", str(receipt_path), "--output", str(output)]
        with mock.patch.object(bench, "native_host", return_value={}), \
             mock.patch.object(bench, "load_selection", return_value={"scenes": []}), \
             mock.patch.object(bench, "binary_identity", side_effect=lambda p: identities[p.name]), \
             mock.patch.object(bench, "source_identity", return_value=source), \
             mock.patch.object(bench, "execute") as execute, \
             contextlib.redirect_stderr(io.StringIO()), self.assertRaises(SystemExit):
            bench.main(argv)
        self.assertFalse(output.exists())
        execute.assert_not_called()

    def test_gallery_retains_runtime_assets_and_resource_mapping_after_cleanup(self):
        selection = self.selection()
        output = self.root / "work"
        (output / "inputs/scenes").mkdir(parents=True)
        (output / "inputs/selection.json").write_text(json.dumps(selection))
        (output / "inputs/scenes/input.m3p").write_bytes(b"preserved scene")
        runtime = output / "runtime/M3Maps/1.pgm"
        runtime.parent.mkdir(parents=True)
        shutil.copy2(self.root / "height.pgm", runtime)
        gallery = self.root / "gallery"
        bench.export_gallery({"results": [], "settings": self.settings}, output, gallery)
        shutil.rmtree(output)
        manifest = json.loads((gallery / "inputs/resource-manifest.json").read_text())
        self.assertEqual(manifest["resource_root"], "inputs/resources")
        row = manifest["assets"][0]
        self.assertEqual(row["runtime_path"], "M3Maps/1.pgm")
        self.assertEqual(bench.sha(gallery / row["gallery_path"]), selection["assets"][0]["sha256"])
        retained = json.loads((gallery / "summary.json").read_text())["retained_gallery"]
        self.assertEqual(retained["resource_manifest"], "inputs/resource-manifest.json")
        self.assertEqual((gallery / "inputs/scenes/input.m3p").read_bytes(), b"preserved scene")

    def test_gallery_rejects_mutated_runtime_asset(self):
        selection = self.selection()
        output = self.root / "work"
        (output / "inputs").mkdir(parents=True)
        (output / "inputs/selection.json").write_text(json.dumps(selection))
        runtime = output / "runtime/M3Maps/1.pgm"
        runtime.parent.mkdir(parents=True)
        runtime.write_bytes(b"wrong map")
        with self.assertRaisesRegex(ValueError, "asset changed"):
            bench.export_gallery({"results": [], "settings": self.settings}, output, self.root / "gallery")

    def test_valid_scene_control_and_asset_hashes(self):
        selection = self.selection()
        loaded = bench.load_selection(self.write_json(selection), self.root)
        self.assertEqual(loaded["expected_count"], 1)
        self.assertEqual(loaded["expected_control_count"], 1)
        for name in ("scene.m3p", "control.m3p", "height.pgm"):
            original = (self.root / name).read_bytes()
            (self.root / name).write_bytes(original + b"changed")
            with self.subTest(name=name), self.assertRaisesRegex(ValueError, "hash"):
                bench.load_selection(self.write_json(selection), self.root)
            (self.root / name).write_bytes(original)

    def test_duplicate_asset_destination_and_escaping_assets_rejected(self):
        selection = self.selection()
        duplicate = copy.deepcopy(selection["assets"][0])
        duplicate["id"] = "other-map"
        selection["assets"].append(duplicate)
        with self.assertRaisesRegex(ValueError, "destination"):
            bench.load_selection(self.write_json(selection), self.root)
        for target in ("../escape.pgm", "M3Maps/../escape.pgm", "M3Maps\\escape.pgm", "/M3Maps/1.pgm"):
            selection = self.selection()
            selection["assets"][0]["runtime_path"] = target
            with self.subTest(target=target), self.assertRaises(ValueError):
                bench.load_selection(self.write_json(selection), self.root)

    def test_missing_effect_controls_assets_and_exclusion_reason_rejected(self):
        for field in ("effect_controls", "unsupported_backends"):
            selection = self.selection()
            del selection["scenes"][0][field]
            with self.subTest(field=field), self.assertRaises(ValueError):
                bench.load_selection(self.write_json(selection), self.root)
        selection = self.selection()
        selection["scenes"][0]["assets"] = ["absent"]
        with self.assertRaisesRegex(ValueError, "Unknown asset"):
            bench.load_selection(self.write_json(selection), self.root)

    def test_deferred_ids_and_wrong_inventory_rejected(self):
        for identifier in ("montecarlo", "monte_carlo", "halton", "jit_formula"):
            selection = self.selection()
            selection["scenes"][0]["id"] = identifier
            with self.subTest(identifier=identifier), self.assertRaisesRegex(ValueError, "Deferred"):
                bench.load_selection(self.write_json(selection), self.root)
        for field in ("expected_count", "expected_control_count"):
            selection = self.selection()
            selection[field] += 1
            with self.subTest(field=field), self.assertRaises(ValueError):
                bench.load_selection(self.write_json(selection), self.root)

    def test_subset_keeps_original_order_and_control_denominator(self):
        selection = self.selection()
        for identifier in ("second", "third"):
            row = copy.deepcopy(selection["scenes"][0])
            row["id"] = identifier
            selection["scenes"].append(row)
        selection.update(expected_count=3, expected_control_count=3)
        result = bench.load_selection(self.write_json(selection), self.root, ("third", "ordinary_map"))
        self.assertEqual([r["id"] for r in result["scenes"]], ["ordinary_map", "third"])
        self.assertEqual(result["expected_count"], 2)
        self.assertEqual(result["expected_control_count"], 2)
        self.assertEqual(result["full_selection_count"], 3)
        for selected in (("missing",), ("second", "second")):
            with self.subTest(selected=selected), self.assertRaises(ValueError):
                bench.load_selection(self.write_json(selection), self.root, selected)

    def test_complete_cold_and_warm_report_uses_decoded_rgb_hash(self):
        report = self.report()
        digest = self.validate(report)
        self.assertEqual(digest, hashlib.sha256(self.rgb).hexdigest())
        self.assertEqual(report["samples"][0]["image_identity"]["rgb_sha256"], digest)
        self.assertEqual(report["cold"]["stages"]["reflections"]["state"], "not_run")

    def test_native_or_unspecified_cpu_precision_cannot_qualify_exactness(self):
        for mode in (None, "native", "unknown"):
            report = self.report()
            if mode is None:
                del report["execution"]["precision"]
            else:
                report["execution"]["precision"] = mode
            with self.subTest(mode=mode), self.assertRaises(ValueError):
                self.validate(report)

    def test_missing_cold_warmup_or_sample_rejected(self):
        for kind in ("cold", "warmups", "samples"):
            report = self.report()
            if kind == "cold":
                del report[kind]
            else:
                report[kind].pop()
            with self.subTest(kind=kind), self.assertRaises(ValueError):
                self.validate(report)

    def test_failed_status_settings_and_nondeterministic_cpu_rejected(self):
        variants = []
        for field, value in (("status", "FAILED"), ("schema", "other")):
            report = self.report()
            report[field] = value
            variants.append(report)
        report = self.report()
        report["settings"]["threads"] += 1
        variants.append(report)
        report = self.report()
        report["execution"]["deterministic"] = False
        variants.append(report)
        for index, report in enumerate(variants):
            with self.subTest(index=index), self.assertRaises(ValueError):
                self.validate(report)

    def test_missing_reordered_or_skipped_frame_not_accepted(self):
        for key, value in (("state", "not_run"), ("phase", "warmup"), ("index", 2), ("width", 3)):
            report = self.report()
            report["samples"][0][key] = value
            with self.subTest(key=key), self.assertRaises(ValueError):
                self.validate(report)

    def test_disabled_required_or_interrupted_stages_rejected(self):
        for state in ("not_run", "interrupted"):
            report = self.report()
            report["warmups"][0]["stages"]["ao"] = {"state": state, "seconds": 0}
            with self.subTest(state=state), self.assertRaisesRegex(ValueError, "active stage"):
                self.validate(report)
        report = self.report()
        del report["samples"][1]["stages"]["volume"]
        with self.assertRaisesRegex(ValueError, "inventory"):
            self.validate(report)
        report = self.report()
        report["samples"][1]["stages"]["volume"]["seconds"] = 1
        with self.assertRaisesRegex(ValueError, "contradictory"):
            self.validate(report)

    def test_effect_off_control_requires_disabled_stage(self):
        bench.require_stages({"ao": {"state": "not_run", "seconds": 0}}, (), ("ao",))
        with self.assertRaisesRegex(ValueError, "Disabled control"):
            bench.require_stages({"ao": {"state": "completed", "seconds": 0.1}}, (), ("ao",))

    def test_missing_duplicate_and_escaping_images_rejected(self):
        for image in ("missing.png", "cold.png", "../escape.png", "/absolute.png"):
            report = self.report()
            report["samples"][0]["image"] = image
            with self.subTest(image=image), self.assertRaises(ValueError):
                self.validate(report)

    def test_actual_rgb_mismatch_detected_even_with_true_native_claim(self):
        report = self.report()
        replacement = bytearray(self.rgb)
        replacement[0] -= 1
        Image.frombytes("RGB", (2, 2), bytes(replacement)).save(self.root / report["warmups"][0]["image"])
        with self.assertRaisesRegex(ValueError, "decoded RGB differs"):
            self.validate(report)
        result = bench.compare_images(self.root / "cold.png", self.root / "warmup-000.png")
        self.assertEqual(result["different_pixels"], 1)
        self.assertEqual(result["max_channel_error"], 1)

    def test_repeat_flag_constant_and_wrong_size_images_rejected(self):
        report = self.report()
        report["samples"][0]["exact_rgb_matches_cold"] = False
        with self.assertRaisesRegex(ValueError, "repeat mismatch"):
            self.validate(report)
        report = self.report()
        Image.new("RGB", (2, 2), (0, 0, 0)).save(self.root / "cold.png")
        with self.assertRaisesRegex(ValueError, "Constant image"):
            self.validate(report)
        report = self.report()
        Image.new("RGB", (3, 2), (0, 0, 0)).save(self.root / "cold.png")
        with self.assertRaises(ValueError):
            self.validate(report)

    def test_nonfinite_missing_boolean_negative_and_zero_work_timings_rejected(self):
        for value in (float("nan"), float("inf"), -1, True, None, 0):
            report = self.report()
            report["samples"][0]["render_ms"] = value
            with self.subTest(value=value), self.assertRaises(ValueError):
                self.validate(report)
        report = self.report()
        del report["cold"]["cold_total_ms"]
        with self.assertRaises(ValueError):
            self.validate(report)

    def test_metal_active_stages_repeat_and_fault_gates(self):
        report = self.report("metal")
        self.assertEqual(self.validate(report, "metal"), hashlib.sha256(self.rgb).hexdigest())
        report["samples"][0]["faults"]["primary"] = 1
        with self.assertRaisesRegex(ValueError, "fault gate"):
            self.validate(report, "metal")
        report = self.report("metal")
        report["samples"][0]["exact_stage_records_match_cold"] = False
        with self.assertRaisesRegex(ValueError, "fault inventory"):
            self.validate(report, "metal")
        report = self.report("metal")
        report["cold"]["stages"]["ambient_occlusion"]["state"] = "not_run"
        with self.assertRaisesRegex(ValueError, "active stage"):
            self.validate(report, "metal")

    def test_metal_missing_fault_inventory_and_invalid_latent_fault_counts_rejected(self):
        for name in ("final_surface", "normal_scratch", "shadow_traces", "latent_shadow_terminal_faults"):
            report = self.report("metal")
            del report["cold"]["faults"][name]
            with self.subTest(name=name), self.assertRaisesRegex(ValueError, "inventory"):
                self.validate(report, "metal")
        for value in (float("nan"), -1, True, 5):
            report = self.report("metal")
            report["cold"]["faults"]["latent_shadow_terminal_faults"] = value
            with self.subTest(value=value), self.assertRaisesRegex(ValueError, "count"):
                self.validate(report, "metal")
        report = self.report("metal")
        report["cold"]["faults"] = {}
        with self.assertRaisesRegex(ValueError, "inventory"):
            self.validate(report, "metal")

    def test_metal_render_only_profile_rejects_oracles_controls_and_extra_pipelines(self):
        for field in ("cpu_compute_fallback_calls", "cpu_oracle_renders", "diagnostic_dispatches"):
            report = self.report("metal")
            report[field] = 1
            with self.subTest(field=field), self.assertRaisesRegex(ValueError, "non-render work"):
                self.validate(report, "metal")
        report = self.report("metal")
        report["render_pipeline_names"].append("exact_arithmetic")
        with self.assertRaisesRegex(ValueError, "pipelines"):
            self.validate(report, "metal")
        report = self.report("metal")
        report["settings"]["division_mode"] = "restoring"
        with self.assertRaisesRegex(ValueError, "profile"):
            self.validate(report, "metal")

    def test_metal_effect_activity_stage_inventory_and_timing_boundaries(self):
        for field in ("ao_changed_pixels", "hits", "misses"):
            report = self.report("metal")
            report["cold"]["activity"][field] = 0
            with self.subTest(field=field), self.assertRaises(ValueError):
                self.validate(report, "metal")
        for field in ("stages", "stages_gpu_ms"):
            report = self.report("metal")
            del report["cold"][field]["surface_packing"]
            with self.subTest(field=field), self.assertRaisesRegex(ValueError, "inventory"):
                self.validate(report, "metal")
        for field in ("gpu_timestamp_ms", "end_to_end_ms", "cold_total_ms", "cold_end_to_end_ms"):
            report = self.report("metal")
            report["cold"][field] = 1.0
            with self.subTest(field=field), self.assertRaises(ValueError):
                self.validate(report, "metal")

    def test_metal_qualification_is_bound_to_binary_scene_and_dispatch_settings(self):
        settings = {k: self.settings[k] for k in ("width", "height", "threads")}
        settings.update(bench.METAL_RENDER_SETTINGS)
        settings.pop("hard_shadows_override")
        settings.update(repeats=3, full_shading=True, hard_shadows=False,
                        primary_dispatch_sweep=False, shadow_dispatch_sweep=False)
        summary = {"schema": "mb3d.metal-exact-shading-evidence.v1", "status": "PASS_SCOPED", "settings": settings,
                   "probe": {"sha256": "probe-hash"}, "library": {"sha256": "library-hash"},
                   "scene": {"scene_sha256": "scene-hash"},
                   "native_scene_description": {"scene": {"scene_sha256": "scene-hash"}},
                   "numeric_profile": "EXACT_SPHEREIFS_FULL_SHADING", "images": [{"backend": "gpu"}, {"backend": "cpu"}]}
        identities = {"metal": {"sha256": "probe-hash"}, "library": {"sha256": "library-hash"}}
        scene = {"scene_sha256": "scene-hash"}
        binding = bench.validate_metal_qualification(summary, identities, scene, self.settings)
        self.assertEqual(binding["probe_sha256"], "probe-hash")
        for field in ("probe", "library", "scene"):
            changed = copy.deepcopy(summary)
            changed[field] = {"sha256": "other", "scene_sha256": "other"}
            with self.subTest(field=field), self.assertRaisesRegex(ValueError, "identity"):
                bench.validate_metal_qualification(changed, identities, scene, self.settings)
        for name, value in (("normal_mode", "inline"), ("hard_shadows", True), ("width", 8), ("threads", 1)):
            changed = copy.deepcopy(summary)
            changed["settings"][name] = value
            with self.subTest(name=name), self.assertRaisesRegex(ValueError, "settings"):
                bench.validate_metal_qualification(changed, identities, scene, self.settings)
        changed = copy.deepcopy(summary)
        changed["images"].pop()
        with self.assertRaisesRegex(ValueError, "independent"):
            bench.validate_metal_qualification(changed, identities, scene, self.settings)

    def test_warm_summary_uses_per_process_medians_and_excludes_warmups(self):
        processes = []
        for values in ((1, 2, 1000), (3, 4, 2000), (5, 6, 3000)):
            native = self.report()
            native["warmups"][0]["end_to_end_ms"] = 999999
            for frame, value in zip(native["samples"], values):
                frame["end_to_end_ms"] = value
                frame["render_and_readback_ms"] = value / 2
            processes.append({"native": native, "process": {"wall_seconds": 3,
                              "cpu_seconds": 4, "peak_rss_bytes": 123456}})
        summary = bench.summarize_backend(processes)
        self.assertEqual(summary["warm_end_to_end_ms"]["count"], 3)
        self.assertEqual(summary["warm_end_to_end_ms"]["values"], [2, 4, 6])
        self.assertEqual(summary["warm_end_to_end_ms"]["median"], 4)
        self.assertEqual(summary["warm_to_rgb_ms"]["median"], 2)
        self.assertEqual(summary["process_peak_rss_bytes"]["median"], 123456)

    def test_stats_reject_empty_invalid_values_and_preserve_dispersion(self):
        for values in ([], [True], [float("nan")], [-1]):
            with self.subTest(values=values), self.assertRaises(ValueError):
                bench.stats(values)
        result = bench.stats([1, 2, 3])
        self.assertEqual(result["median"], 2)
        self.assertEqual(result["min"], 1)
        self.assertEqual(result["max"], 3)
        self.assertGreater(result["population_stddev"], 0)


if __name__ == "__main__":
    unittest.main()
