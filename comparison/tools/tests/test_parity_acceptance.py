import sys
import tempfile
import unittest
from pathlib import Path

from PIL import Image

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from sweep import run_diff
from trace_diff import parse_line, diff_traces


class ParityAcceptanceTests(unittest.TestCase):
    def test_large_difference_stays_failed_even_with_legacy_allowance(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            Image.new('RGB', (16, 16), (250, 250, 250)).save(root/'pas.png')
            Image.new('RGB', (16, 16), (150, 150, 150)).save(root/'cpp.png')
            result = run_diff(root/'pas.png', root/'cpp.png', root/'diff.json',
                              'phase3_vl_bulbox_medium')
            self.assertFalse(result['pass'])
            self.assertFalse(result['exact_rgb'])
            self.assertTrue(result['legacy_profile_pass'])

    def test_identical_pixels_are_exact(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            Image.new('RGB', (16, 16), (20, 40, 60)).save(root/'same.png')
            result = run_diff(root/'same.png', root/'same.png', root/'diff.json')
            self.assertTrue(result['pass'])
            self.assertTrue(result['exact_rgb'])

    def test_missing_checksums_cannot_certify_bitwise_equality(self):
        pas = [parse_line('stage=ray_ready x=0 y=0 value=1', 1, 'pascal')]
        cpp = [parse_line('stage=ray_ready x=0 y=0 value=2', 1, 'cpp')]
        index, reason = diff_traces(pas, cpp, 'bits', 0, 0)
        self.assertEqual(index, 0)
        self.assertIn('missing checksum', reason)


if __name__ == '__main__':
    unittest.main()
