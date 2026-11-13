"""Evidence failures must not become successful image comparisons."""
import json
from pathlib import Path
import tempfile
import unittest
from PIL import Image
import native_extall_coverage as ext


class SummaryTests(unittest.TestCase):
    def fixture(self, base):
        images = base / "scenes/01"
        images.mkdir(parents=True)
        data = {"size": [2, 2], "binary_sha256": "b", "library_sha256": "l",
                "statuses": {"GPU": 1}, "results": [
                    {"rank": 1, "scene": "comparison/scenes/extall_3D_sample.m3p", "status": "GPU"}]}
        (base / "coverage.json").write_text(json.dumps(data))
        return images, data

    def test_missing_images_are_not_matches(self):
        with tempfile.TemporaryDirectory() as temp:
            base = Path(temp)
            self.fixture(base)
            result = ext.summarize(base)
            self.assertEqual(result["observed_comparisons"], 0)
            self.assertNotIn("exact_rgb", result["rows"][0])

    def test_observed_difference_and_dimension_rejection(self):
        with tempfile.TemporaryDirectory() as temp:
            base = Path(temp)
            images, _ = self.fixture(base)
            Image.new("RGB", (2, 2), (0, 0, 0)).save(images / "cpu.png")
            Image.new("RGB", (2, 2), (17, 0, 0)).save(images / "image.png")
            row = ext.summarize(base)["rows"][0]
            self.assertEqual(row["strong_pixels"], 4)
            self.assertFalse(row["exact_rgb"])
            Image.new("RGB", (1, 1)).save(images / "image.png")
            with self.assertRaisesRegex(ValueError, "dimensions"):
                ext.summarize(base)

    def test_rejects_duplicate_ranks(self):
        with tempfile.TemporaryDirectory() as temp:
            base = Path(temp)
            _, data = self.fixture(base)
            data["results"] *= 2
            (base / "coverage.json").write_text(json.dumps(data))
            with self.assertRaisesRegex(ValueError, "duplicate"):
                ext.summarize(base)


if __name__ == "__main__":
    unittest.main()
