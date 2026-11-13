"""Checkpoint integrity tests: resume must reject altered evidence."""
import copy
import json
from pathlib import Path
import tempfile
import unittest
import subprocess
from unittest.mock import patch

from batch_formula_sweep import validate_checkpoint, retry_pascal_with_identity
from compatibility_precision import precision_evidence, profile_path
from external_validation import digest, write_json


class CheckpointTests(unittest.TestCase):
    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory()
        self.addCleanup(self.temporary.cleanup)
        self.root = Path(self.temporary.name)
        self.identity = dict(pascal_sha256='pas', cpp_sha256='cpp',
                             manifest_sha256='manifest', qualification_sha256='qualification',
                             thresholds={'mean': 2},
                             timeouts={'render_ms': 15000, 'batch_seconds': 45})
        self.cases = [dict(id='first', sha256='scene'), dict(id='second', sha256='next')]
        item = dict(scene='first', scene_sha256='scene', status='fail')
        for kind in ('pascal', 'cpp'):
            path = self.root / kind / 'first.png'
            path.parent.mkdir()
            path.write_bytes(kind.encode())
            item[kind + '_image_sha256'] = digest(path.read_bytes())
        image = self.root / 'cpp' / 'first.png'
        profile_path(image).write_text(json.dumps(dict(
            schema='mb3d.cpu-stage-profile.v1', precision='compatible',
            status='complete', exit_code=0, render_status='completed')))
        item.update(precision_evidence(image))
        self.report = dict(self.identity, results=[item])

    def test_keeps_failed_measurement_and_resumes_next_case(self):
        self.assertEqual(validate_checkpoint(self.report, self.identity, self.cases, self.root), 1)

    def test_rejects_changed_identity(self):
        for key in self.identity:
            with self.subTest(key=key):
                changed = dict(self.identity, **{key: 'altered'})
                with self.assertRaises(ValueError):
                    validate_checkpoint(self.report, changed, self.cases, self.root)

    def test_rejects_changed_image_and_order(self):
        with self.assertRaises(ValueError):
            validate_checkpoint(self.report, self.identity, list(reversed(self.cases)), self.root)
        (self.root / 'pascal' / 'first.png').write_bytes(b'truncated')
        with self.assertRaises(ValueError):
            validate_checkpoint(self.report, self.identity, self.cases, self.root)

    def test_rejects_changed_or_missing_timeouts(self):
        for key in self.identity['timeouts']:
            changed = copy.deepcopy(self.identity)
            changed['timeouts'][key] *= 2
            with self.subTest(key=key), self.assertRaises(ValueError):
                validate_checkpoint(self.report, changed, self.cases, self.root)
        legacy = copy.deepcopy(self.report)
        del legacy['timeouts']
        with self.assertRaises(ValueError):
            validate_checkpoint(legacy, self.identity, self.cases, self.root)

    def test_rejects_missing_hash_on_accepted_image(self):
        report = copy.deepcopy(self.report)
        del report['results'][0]['cpp_image_sha256']
        with self.assertRaises(ValueError):
            validate_checkpoint(report, self.identity, self.cases, self.root)

    def test_failed_atomic_replace_preserves_checkpoint(self):
        path = self.root / 'summary.json'
        write_json(path, {'completed': 1})
        with patch('external_validation.os.replace', side_effect=OSError('interrupted')):
            with self.assertRaises(OSError):
                write_json(path, {'completed': 2})
        self.assertEqual(json.loads(path.read_text()), {'completed': 1})
        self.assertFalse(list(self.root.glob('*.tmp')))
        write_json(path, {'completed': 2})
        self.assertEqual(json.loads(path.read_text()), {'completed': 2})

    def test_retry_preserves_full_identity_log_in_fresh_process(self):
        listing = self.root / 'retry.tsv'
        output = self.root / 'retry.png'
        output.write_bytes(b'incomplete earlier image')
        identity = b'FORMULA IDENTITY\n' + b'x' * 3000
        result = subprocess.CompletedProcess([], 0, identity, b'')
        with patch('batch_formula_sweep.subprocess.run', return_value=result) as run:
            code, _, log = retry_pascal_with_identity(
                self.root / 'Mandelbulb3D.exe', self.root / 'scene.m3p', output, listing, 15000)
        self.assertEqual(code, 0)
        self.assertTrue(log.startswith('FORMULA IDENTITY\n'))
        self.assertGreater(len(log), 3000)
        self.assertEqual(listing.read_bytes(), b'')
        self.assertFalse(output.exists())
        self.assertEqual(run.call_args.kwargs['env']['MB3D_HEADLESS_BATCH'], str(listing.resolve()))
        self.assertEqual(run.call_args.kwargs['timeout'], 15)

    def test_retry_timeout_retains_partial_diagnostic(self):
        error = subprocess.TimeoutExpired([], 1, output=b'loaded slot 0', stderr=b'waiting')
        with patch('batch_formula_sweep.subprocess.run', side_effect=error):
            code, _, log = retry_pascal_with_identity(self.root / 'Mandelbulb3D.exe',
                self.root / 'scene.m3p', self.root / 'retry.png', self.root / 'retry.tsv', 1000)
        self.assertEqual(code, -1)
        self.assertIn('loaded slot 0', log)
        self.assertIn('TIMEOUT', log)
        self.assertIn('waiting', log)


if __name__ == '__main__':
    unittest.main()
