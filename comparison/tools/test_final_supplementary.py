"""Acceptance-boundary negative controls; no external binaries are launched."""
from copy import deepcopy
from pathlib import Path
import hashlib
import tempfile
import unittest

from audit_final_supplementary import (
    Audit, DEFAULT_THRESHOLDS, FIELDS, SELECTOR_UNVERIFIED,
    check_hash, check_stdin_hash, check_selector_status, check_state, digest, same_cases,
)


class SupplementalGateTests(unittest.TestCase):
    def setUp(self):
        self.audit = Audit.__new__(Audit)
        self.audit.identities = {'renderer': {'sha256': 'a' * 64}}
        # This control stops at the artifact boundary. Artifact tampering is
        # independently exercised against a real temporary file below.
        self.audit.output = lambda *_: None
        self.prior = {'results': [{'id': 'witness', 'scene_sha256': 'scene'}]}
        self.report = dict(finished=True, cpp_sha256='a' * 64,
            thresholds=dict(DEFAULT_THRESHOLDS), counts={'PASS': 1}, results=[dict(
                id='witness', scene_sha256='scene', returncode=0, status='PASS',
                exact_rgb=True, changed_fields=dict.fromkeys(FIELDS, 0),
                metrics=dict(max_abs_delta=0, mean_abs_delta=0, p99_abs_delta=0,
                             diff_ratio=0, diff_ratio_threshold=2, psnr=float('inf'), ssim=1))])

    def image_gate(self, report):
        return self.audit.images(Path('summary.json'), report, self.prior, exact=True, state=True)

    def test_hidden_failure_despite_exit_zero(self):
        self.assertEqual(self.image_gate(self.report), 1)
        for change in ({'status': 'FAIL'}, {'metrics': dict(self.report['results'][0]['metrics'], ssim=.5)}):
            bad = deepcopy(self.report); bad['results'][0].update(change)
            with self.subTest(change=change), self.assertRaises(ValueError):
                self.image_gate(bad)

    def test_wrong_candidate_hash(self):
        bad = deepcopy(self.report); bad['cpp_sha256'] = 'b' * 64
        with self.assertRaisesRegex(ValueError, 'candidate hash'):
            self.image_gate(bad)

    def test_missing_case(self):
        bad = deepcopy(self.report); bad['results'] = []
        with self.assertRaisesRegex(ValueError, 'missing/unexpected'):
            self.image_gate(bad)

    def test_duplicate_cannot_replace_expected_case(self):
        expected = [{'id': 'a'}, {'id': 'b'}]
        with self.assertRaisesRegex(ValueError, 'duplicate'):
            same_cases([{'id': 'a'}, {'id': 'a'}], expected, ('id',), 'corpus')

    def test_changed_artifact(self):
        with tempfile.TemporaryDirectory() as folder:
            artifact = Path(folder) / 'render.bin'; artifact.write_bytes(b'qualified')
            original = digest(artifact); self.assertEqual(check_hash(artifact, original), original)
            artifact.write_bytes(b'changed')
            with self.assertRaisesRegex(ValueError, 'Changed artifact'):
                check_hash(artifact, original)

    def test_known_undefined_must_not_be_promoted_to_pass(self):
        case, (name, reason) = next(iter(SELECTOR_UNVERIFIED.items()))
        row = dict(case=case, name=name, reason=reason+' slot 1', status='unverified',
                   input_validation='rejected', cpp_exit_code=12,
                   native_selector_check={'expected': -1, 'loaded': -1})
        check_selector_status(row)
        for change in ({'status': 'pass'}, {'cpp_exit_code': 0}, {'reason': 'unrelated failure'}):
            with self.subTest(change=change), self.assertRaises(ValueError):
                check_selector_status(dict(row, **change))

    def test_stdin_hash_accepts_windows_newlines_but_rejects_content_changes(self):
        with tempfile.TemporaryDirectory() as folder:
            artifact = Path(folder) / 'points.input'
            logical = b'0.25 -1 2\n3 4 5\n'
            expected = hashlib.sha256(logical).hexdigest()
            for content in (logical, logical.replace(b'\n', b'\r\n')):
                artifact.write_bytes(content)
                check_stdin_hash(artifact, expected)
            for content in (logical.replace(b'0.25', b'0.26'),
                            logical.replace(b'\n', b'\r'), logical + b'\n'):
                artifact.write_bytes(content)
                with self.subTest(content=content), self.assertRaisesRegex(ValueError, 'Changed stdin artifact'):
                    check_stdin_hash(artifact, expected)

    def test_repeat_state_requires_every_field_and_zero_changes(self):
        row = {'changed_fields': dict.fromkeys(FIELDS, 0)}; check_state(row)
        for changed in (dict(row['changed_fields'], amb_shadow=1),
                        {k: v for k, v in row['changed_fields'].items() if k != 'zpos'}):
            with self.subTest(changed=changed), self.assertRaises(ValueError):
                check_state({'changed_fields': changed})


if __name__ == '__main__':
    unittest.main()
