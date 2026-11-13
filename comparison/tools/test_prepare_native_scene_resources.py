"""Resource availability and rejection checks; no rendering or prior raw runs."""
import json
from pathlib import Path
import shutil
import tempfile
import unittest

import prepare_native_scene_resources as resources


class SelectedResources(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)
        self.root = Path(self.tmp.name)

    def test_all_scene_profiles_and_exact_substitute(self):
        out = self.root / "runtime-package"
        report = resources.prepare_profiles(out)
        self.assertEqual(report["scenes_with_available_resources"], 77)
        self.assertFalse(report["qualification"])
        self.assertFalse((out / "runtime/M3Maps/1633.jpg").exists())
        self.assertFalse((out / "runtime-selected/M3Maps/1633.jpg").exists())
        self.assertEqual((out / "runtime-substitute1633/M3Maps/1633.jpg").read_bytes(),
                         (out / "runtime/M3Maps/3038.jpg").read_bytes())
        for profile in report["profiles"].values():
            for rel, expected in profile["files"].items():
                self.assertEqual(resources.sha(out / profile["runtime"] / rel), expected)

    def test_changed_image_rejected_before_output(self):
        source = self.root / "source"
        shutil.copytree(resources.SELECTED_RESOURCES, source)
        (source / "M3Maps/18.jpg").write_bytes(b"unapproved replacement")
        out = self.root / "output"
        with self.assertRaisesRegex(ValueError, "Selected resource bytes changed"):
            resources.prepare_profiles(out, source)
        self.assertFalse(out.exists())

    def test_relabelled_inventory_rejected(self):
        source = self.root / "source"
        shutil.copytree(resources.SELECTED_RESOURCES, source)
        selection = json.loads((source / "selection.json").read_text())
        selection["profiles"]["original"] = selection["profiles"]["available"]
        (source / "selection.json").write_text(json.dumps(selection))
        with self.assertRaisesRegex(ValueError, "differs from approved contract"):
            resources.prepare_profiles(self.root / "output", source)

    def test_existing_runtime_is_not_overwritten(self):
        marker = self.root / "preserve"
        marker.write_bytes(b"historical evidence")
        with self.assertRaisesRegex(ValueError, "new output"):
            resources.prepare_profiles(self.root)
        self.assertEqual(marker.read_bytes(), b"historical evidence")


if __name__ == "__main__":
    unittest.main()
