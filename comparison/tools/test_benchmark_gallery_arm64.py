"""Focused evidence-integrity checks; no renderer or performance jobs run here."""
import copy
import hashlib
import json
import os
from pathlib import Path
import sys
import tempfile
import unittest

from PIL import Image

import benchmark_gallery_arm64 as bench


def complete_profile():
    skipped = {"volume", "normals", "reflections", "dof"}
    stages = {name: {"state": "not_run" if name in skipped else "completed",
                     "seconds": 0 if name in skipped else 0.1} for name in bench.STAGES}
    return {"schema": "mb3d.cpu-stage-profile.v1", "schema_version": 1,
            "status": "complete", "exit_code": 0, "render_status": "completed",
            "precision": "compatible",
            "width": 2, "height": 2, "workers": 6, "stages": stages,
            "stages_seconds": {k: v["seconds"] for k, v in stages.items() if v["state"] == "completed"},
            "render_total_seconds": 1.0, "command_total_seconds": 1.2}


def sample(seconds, kind="measured", digest="a" * 64):
    return {"kind": kind, "status": "rendered", "rgb_sha256": digest,
            "wall_seconds": seconds, "cpu_seconds": seconds * 4,
            "cpu_user_seconds": seconds * 3, "cpu_system_seconds": seconds,
            "peak_rss_bytes": 1024, "profile": complete_profile()}


class GalleryBenchmarkIntegrityTests(unittest.TestCase):
    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory()
        self.addCleanup(self.temporary.cleanup)
        self.root = Path(self.temporary.name)

    def save_json(self, name, value):
        path = self.root / name
        path.write_text(json.dumps(value))
        return path

    def selection(self):
        scene = self.root / "scene.m3p"
        scene.write_bytes(b"original preset bytes")
        return {"schema": bench.SELECTION_SCHEMA, "expected_count": 1, "assets": [],
                "scenes": [{"id": "scene", "scene_path": "scene.m3p", "scene_sha256": bench.sha(scene)}]}

    def test_tracked_selection_has_exact_32_valid_presets(self):
        result = bench.load_selection(bench.ROOT / "comparison/benchmark_scenes/gallery32.json")
        self.assertEqual(result["expected_count"], 32)
        ids = {r["id"] for r in result["scenes"]}
        self.assertIn("extall_dIFS_tritgrid2IFS", ids)
        self.assertIn("extall_dIFS_YinYangIFS", ids)
        self.assertNotIn("extall_dIFS_ducksIFS", ids)
        self.assertNotIn("extall_dIFS_gnarlyIFS", ids)

    def test_input_hash_is_checked(self):
        selection = self.selection()
        path = self.save_json("selection.json", selection)
        self.assertEqual(bench.load_selection(path, self.root)["expected_count"], 1)
        (self.root / "scene.m3p").write_bytes(b"different preset")
        with self.assertRaisesRegex(ValueError, "SHA-256 mismatch"):
            bench.load_selection(path, self.root)

    def test_duplicate_and_unknown_ids_are_rejected(self):
        selection = self.selection()
        path = self.save_json("selection.json", selection)
        for chosen in (("missing",), ("scene", "scene")):
            with self.subTest(chosen=chosen), self.assertRaises(ValueError):
                bench.load_selection(path, self.root, chosen)
        selection["scenes"].append(copy.deepcopy(selection["scenes"][0]))
        selection["expected_count"] = 2
        with self.assertRaisesRegex(ValueError, "duplicate"):
            bench.load_selection(self.save_json("duplicate.json", selection), self.root)

    def test_traversal_absolute_and_external_symlinks_are_rejected(self):
        for path in ("../escape.m3p", "/absolute.m3p", "dir\\escape.m3p"):
            with self.subTest(path=path), self.assertRaises(ValueError):
                bench.relative_file(self.root, path)
        (self.root / "escape.m3p").symlink_to(Path(__file__).resolve())
        with self.assertRaises(ValueError):
            bench.relative_file(self.root, "escape.m3p")

    def test_subset_preserves_selection_order_and_denominator(self):
        result = bench.load_selection(bench.ROOT / "comparison/benchmark_scenes/gallery32.json",
                                      selected_ids=("extall_Ads__Rotate", "extall_3Da_ABoxVSShapes"))
        self.assertEqual(result["expected_count"], 2)
        self.assertEqual(result["full_selection_count"], 32)
        self.assertEqual(result["scenes"][0]["id"], "extall_3Da_ABoxVSShapes")

    def test_profile_accepts_explicit_skipped_stages(self):
        profile = complete_profile()
        result = bench.validate_profile(self.save_json("profile.json", profile), 2, 2, 6)
        self.assertEqual(result["stages"]["dof"]["state"], "not_run")
        self.assertNotIn("dof", result["stages_seconds"])

    def test_profile_rejects_failure_incomplete_mismatch_and_nonfinite(self):
        variants = []
        for field, value in (("exit_code", 3), ("status", "cancelled"), ("render_status", "failed"),
                             ("workers", 1), ("width", 3), ("schema", "other")):
            variant = complete_profile()
            variant[field] = value
            variants.append(variant)
        variant = complete_profile()
        variant["stages"]["primary"]["state"] = "interrupted"
        variants.append(variant)
        variant = complete_profile()
        variant["stages"]["primary"]["seconds"] = float("nan")
        variants.append(variant)
        variant = complete_profile()
        variant["stages_seconds"]["primary"] = 99
        variants.append(variant)
        variant = complete_profile()
        del variant["stages"]["save"]
        variants.append(variant)
        for index, value in enumerate(variants):
            with self.subTest(index=index), self.assertRaises(ValueError):
                bench.validate_profile(self.save_json("bad.json", value), 2, 2, 6)

    def test_png_hash_is_top_to_bottom_rgb_not_encoded_bytes(self):
        pixels = bytes((255, 0, 0, 0, 255, 0, 0, 0, 255, 255, 255, 255))
        image = Image.frombytes("RGB", (2, 2), pixels)
        path = self.root / "pixels.png"
        image.save(path)
        inspected = bench.inspect_png(path, 2, 2)
        self.assertEqual(inspected["rgb_sha256"], hashlib.sha256(pixels).hexdigest())
        with self.assertRaises(ValueError):
            bench.inspect_png(path, 3, 2)
        path.write_bytes(path.read_bytes()[:-16])
        with self.assertRaises((OSError, ValueError, SyntaxError)):
            bench.inspect_png(path, 2, 2)

    def test_observation_has_no_median_or_repeat_claim(self):
        result = bench.summarize_runs([sample(2)], 1)
        self.assertEqual(result["status"], "complete")
        self.assertEqual(result["sample_kind"], "single_observation")
        self.assertEqual(result["repeat_rgb"], "not_checked_single_sample")
        self.assertNotIn("median", result)
        self.assertEqual(result["observation"]["wall_seconds"], 2)

    def test_warmup_excluded_from_median_but_included_in_rgb(self):
        runs = [sample(99, "warmup"), sample(1), sample(5), sample(3)]
        result = bench.summarize_runs(runs, 3)
        self.assertEqual(result["median"]["wall_seconds"], 3)
        runs[0]["rgb_sha256"] = "b" * 64
        self.assertEqual(bench.summarize_runs(runs, 3)["status"], "failed")

    def test_failed_or_missing_samples_do_not_create_medians(self):
        for runs in ([sample(2)], [sample(2), {"kind": "measured", "status": "timeout"}]):
            result = bench.summarize_runs(runs, 3)
            self.assertEqual(result["status"], "failed")
            self.assertNotIn("median", result)

    def test_aggregate_denominator_is_only_measured_stage_time(self):
        row = {"summary": bench.summarize_runs([sample(2)], 1)}
        report = bench.aggregate_scenes([row, {"summary": {"status": "failed"}}], 1)
        self.assertEqual(report["included_scenes"], 1)
        self.assertAlmostEqual(sum(report["stage_fraction"].values()), 1)
        self.assertEqual(report["stage_fraction"]["dof"], 0)
        self.assertEqual(report["wall_seconds"], 2)

    @unittest.skipUnless(hasattr(os, "wait4"), "Requires POSIX wait4")
    def test_wait4_records_nonzero_exit_and_timeout_separately(self):
        env = dict(os.environ)
        failed = bench.execute([sys.executable, "-c", "raise SystemExit(6)"], self.root,
                               self.root / "failure.log", 5, env)
        self.assertEqual(failed["returncode"], 6)
        self.assertFalse(failed["timed_out"])
        self.assertGreater(failed["peak_rss_bytes"], 0)
        timed = bench.execute([sys.executable, "-c", "import time; time.sleep(10)"], self.root,
                              self.root / "timeout.log", 0.05, env)
        self.assertTrue(timed["timed_out"])
        self.assertLess(timed["returncode"], 0)
        self.assertLess(timed["wall_seconds"], 2)

    def test_historical_gallery_cannot_be_mislabeled_as_reference(self):
        selection = self.selection()
        path = self.save_json("gallery.json", {"status": "COMPLETE", "source_commit": "5d2ed53", "results": []})
        with self.assertRaisesRegex(ValueError, "not Pascal/historical"):
            bench.load_reference(path, selection, 1024, 768, 6)


if __name__ == "__main__":
    unittest.main()
