"""Monte Carlo replay must bind historical PNG and linear reference artifacts."""
import json
from pathlib import Path
import tempfile
import unittest
from external_validation import digest, write_json
from validate_monte_carlo import load_pascal_reference, checked_pascal_paths

class MonteCarloReferenceTests(unittest.TestCase):
    def setUp(self):
        temp = tempfile.TemporaryDirectory()
        self.addCleanup(temp.cleanup)
        self.root = Path(temp.name)
        self.original = self.root / 'original'
        self.replay = self.root / 'replay'
        (self.original / 'pascal').mkdir(parents=True)
        self.replay.mkdir()
        self.png = self.original / 'pascal' / 'case.png'
        self.linear = self.png.with_suffix('.linear.txt')
        self.png.write_bytes(b'independent-reference-image')
        self.linear.write_bytes(b'independent-reference-linear-values')
        self.case = dict(id='case', sha256='scene-hash', samples=8)
        self.old = dict(case='case', scene_sha256='scene-hash', samples=8)
        self.recorded = dict(self.old, pascal_image_sha256=digest(self.png.read_bytes()),
                             pascal_linear_sha256=digest(self.linear.read_bytes()))
        self.base = dict(binaries={'pascal': {'sha256': 'pascal-binary'}}, results=[self.old])
        self.latest = dict(self.base, results=[self.recorded], pascal_reference=str(self.original))
        write_json(self.original / 'summary.json', self.base)
        write_json(self.replay / 'summary.json', self.latest)

    def test_replay_chain_uses_recorded_hashes_and_original_files(self):
        report, directory, evidence = load_pascal_reference(self.replay)
        self.assertEqual(directory, self.original)
        self.assertEqual(checked_pascal_paths(directory, self.case, report['results'][0]), (self.png, self.linear))
        self.assertEqual([x['sha256'] for x in evidence],
                         [digest((p / 'summary.json').read_bytes()) for p in (self.replay, self.original)])

    def test_legacy_missing_hashes_are_not_backfilled(self):
        with self.assertRaisesRegex(ValueError, 'no recorded artifact hashes'):
            checked_pascal_paths(self.original, self.case, self.old)

    def test_modified_png_rejected(self):
        self.png.write_bytes(b'changed')
        with self.assertRaisesRegex(ValueError, 'hash changed'):
            checked_pascal_paths(self.original, self.case, self.recorded)

    def test_modified_linear_rejected(self):
        self.linear.write_bytes(b'changed')
        with self.assertRaisesRegex(ValueError, 'hash changed'):
            checked_pascal_paths(self.original, self.case, self.recorded)

    def test_changed_samples_rejected(self):
        self.case['samples'] = 16
        with self.assertRaisesRegex(ValueError, 'mismatched'):
            checked_pascal_paths(self.original, self.case, self.recorded)

    def test_binary_mismatch_rejected(self):
        self.base['binaries']['pascal']['sha256'] = 'other'
        write_json(self.original / 'summary.json', self.base)
        with self.assertRaisesRegex(ValueError, 'binary differs'):
            load_pascal_reference(self.replay)

    def test_cycle_rejected(self):
        self.base['pascal_reference'] = str(self.replay)
        write_json(self.original / 'summary.json', self.base)
        with self.assertRaisesRegex(ValueError, 'Cyclic'):
            load_pascal_reference(self.replay)

if __name__ == '__main__':
    unittest.main()
