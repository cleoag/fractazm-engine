"""Status and regression fixtures for native_gpu_coverage; no device runs."""
from pathlib import Path
import tempfile
import unittest

from PIL import Image

import native_gpu_coverage as cv


class Classify(unittest.TestCase):
    def test_only_completed_metal_frames_count(self):
        metal = {"backend": "metal", "backend_fallback": False}
        self.assertEqual(cv.classify(0, False, True, metal), ("GPU", ""))
        self.assertEqual(cv.classify(0, False, True, {**metal, "backend_fallback": True, "backend_fallback_code": "unsupported_scene",
                                                      "backend_fallback_reason": "x"}), ("FALLBACK", "unsupported_scene: x"))
        self.assertEqual(cv.classify(3, False, False, {**metal, "error": "fault"}), ("GPU_FAULT", "fault"))
        self.assertEqual(cv.classify(None, True, False, {}), ("TIMEOUT", "timeout"))
        self.assertEqual(cv.classify(0, False, False, metal)[0], "GPU_FAULT")

    def test_scenes_outside_binary32_are_fp32_range(self):
        metal = {"backend": "metal", "backend_fallback": False}
        before = {**metal, "backend_fallback": True, "backend_fallback_code": "unsupported_scene",
                  "backend_fallback_reason": "Native formula FP32 range: formula parameter"}
        self.assertEqual(cv.classify(0, False, True, before)[0], "FP32_RANGE")
        frame = {**metal, "error": "Native FP32 range: 9416 of 19200 pixels left the binary32 range"}
        self.assertEqual(cv.classify(3, False, False, frame)[0], "FP32_RANGE")

    def test_blocker_drops_numbers(self):
        self.assertEqual(cv.blocker("failed at pixel 1030 code=4"), "failed at pixel N code=N")


class Regressions(unittest.TestCase):
    def test_metal_scenes_keep_rgb(self):
        base = [{"rank": 1, "status": "GPU", "rgb_sha256": "a"}, {"rank": 2, "status": "FALLBACK"}]
        self.assertEqual(cv.regressions(base, [{"rank": 1, "status": "GPU", "rgb_sha256": "a"},
                                               {"rank": 2, "status": "GPU", "rgb_sha256": "b"}]), [])
        bad = cv.regressions(base, [{"rank": 1, "status": "GPU", "rgb_sha256": "c"}])
        self.assertEqual([b["rank"] for b in bad], [1])
        bad = cv.regressions(base, [{"rank": 1, "status": "FALLBACK", "reason": "r"}])
        self.assertEqual(bad[0]["status"], "FALLBACK")


class Difference(unittest.TestCase):
    def test_mean_max_and_pixels(self):
        with tempfile.TemporaryDirectory() as d:
            a, b, c = Path(d, "a.png"), Path(d, "b.png"), Path(d, "c.png")
            Image.new("RGB", (2, 2), (10, 10, 10)).save(a)
            other = Image.new("RGB", (2, 2), (10, 10, 10))
            other.putpixel((1, 0), (13, 10, 4))
            other.save(b)
            Image.new("RGB", (3, 2)).save(c)
            self.assertEqual(cv.image_difference(a, a), {"mean": 0.0, "max": 0, "pixels": 0})
            self.assertEqual(cv.image_difference(a, b), {"mean": 0.75, "max": 6, "pixels": 1})
            self.assertIsNone(cv.image_difference(a, c))


if __name__ == "__main__":
    unittest.main()
