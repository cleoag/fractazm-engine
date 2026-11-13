"""Prepare-to-bundle integration without a compiler, renderer or old raw runs.

Only source/build identity discovery is stubbed. These fixtures do not satisfy
the qualification gate and do not claim a rendering or provenance result.
"""
import json
from pathlib import Path
import shutil
import tempfile
import unittest
from unittest.mock import patch

import prepare_gpu_parallel_bundle as bundle
import prepare_native_scene_resources as resources


class PreparedResourceBundle(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)
        self.root = Path(self.tmp.name).resolve()
        self.contract = resources.ROOT / "comparison/contracts/native-gpu-acceptance-v4/acceptance-contract.json"
        _, _, scenes = bundle.scope(self.contract)
        for scene in scenes.values():
            target = self.root / scene["path"]
            target.parent.mkdir(parents=True, exist_ok=True)
            shutil.copyfile(resources.ROOT / scene["path"], target)
        self.package = self.root / "package"
        resources.prepare_profiles(self.package)
        self.build = self.root / "build"
        self.build.mkdir()
        (self.build / "mb3d_render").write_bytes(b"non-executable fixture")
        (self.build / "metal_native_scene.metallib").write_bytes(b"not a Metal library")
        (self.build / "compile_commands.json").write_text(json.dumps([
            {"file": str(self.root / "src/render/native_metal/general_renderer.mm")}
        ]))

    def refresh(self):
        with patch.object(bundle, "ROOT", self.root), patch.object(
            bundle, "source_snapshot", return_value={"fixture": "not qualification"}
        ):
            return bundle.refresh(self.package, self.contract, self.build)

    def test_prepared_profiles_assemble_without_archived_selection(self):
        evidence = self.refresh()
        binding = evidence["binding"]
        self.assertEqual(binding["map_selection"]["path"], "package/resources.json")
        self.assertEqual(binding["map_selection"]["sha256"], bundle.sha(self.package / "resources.json"))
        self.assertEqual(binding["runtime"], "package/runtime")
        self.assertEqual(len(binding["scene_resources"]), 77)
        self.assertEqual(evidence["scenes"], [])
        self.assertEqual(evidence["proofs"], [])
        for profile in binding["resource_profiles"].values():
            self.assertTrue((self.root / profile["runtime"] / "M3Maps").is_dir())

    def test_explicit_historical_selection_keeps_precedence(self):
        selection = self.package / "map-selection-v4.json"
        selection.write_text('{"fixture": "historical selection"}\n')
        evidence = self.refresh()
        self.assertEqual(evidence["binding"]["map_selection"]["path"], "package/map-selection-v4.json")
        self.assertEqual(evidence["binding"]["map_selection"]["sha256"], bundle.sha(selection))

    def test_mislabelled_preparation_rejected_before_manifest(self):
        path = self.package / "resources.json"
        report = json.loads(path.read_text())
        report["profiles"]["available"]["inventory_sha256"] = "0" * 64
        path.write_text(json.dumps(report))
        with self.assertRaisesRegex(ValueError, "Prepared resource inventory differs"):
            self.refresh()
        self.assertFalse((self.package / "evidence.json").exists())


if __name__ == "__main__":
    unittest.main()
