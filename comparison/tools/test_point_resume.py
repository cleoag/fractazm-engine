"""A restarted point audit must not reuse evidence from changed inputs."""
import copy
import tempfile
import struct
import unittest
from pathlib import Path
from unittest.mock import patch
import check_formula_scene_points as checker


class PointResumeTests(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)
        self.root = Path(self.tmp.name)
        self.output = self.root/'output'
        self.output.mkdir()
        (self.root/'mb3d/M3Formulas').mkdir(parents=True)
        (self.root/'mb3d/M3Formulas/F.m3f').write_bytes(b'original code')
        (self.root/'scene.m3p').write_bytes(b'original scene')
        (self.output/'case.params').write_bytes(b'expanded parameters')
        self.identity = dict(cpp_sha256='cpp', oracle_sha256='pascal',
            manifest_sha256='manifest', input_sha256='input', selected_case_count=2,
            tool_sha256={'checker.py':'checker', 'metadata.py':'metadata'},
            first_iteration_inputs=True, finished=False, results=[])
        self.cases = [dict(id='case', target_formulas=['F'], scene='scene.m3p',
                          sha256=checker.sha(self.root/'scene.m3p')),
                      dict(id='next', target_formulas=['F'])]
        self.previous = copy.deepcopy(self.identity)
        self.previous['results'] = [dict(case='case', name='F', status='pass',
            scene_sha256=self.cases[0]['sha256'],
            asset_sha256=checker.sha(self.root/'mb3d/M3Formulas/F.m3f'),
            parameter_sha256=checker.sha(self.output/'case.params'))]
        self.patch = patch.object(checker, 'ROOT', self.root)
        self.patch.start()
        self.addCleanup(self.patch.stop)

    def validate(self):
        return checker.validate_resume(self.previous, self.identity, self.cases, self.output)

    def test_valid_prefix_keeps_failures_and_errors(self):
        for status in ['pass','fail','error','unverified']:
            self.previous['results'][0]['status'] = status
            self.assertEqual(self.validate(), 1)

    def test_rejects_changed_identity(self):
        for key in ['cpp_sha256','oracle_sha256','manifest_sha256','input_sha256',
                    'selected_case_count','first_iteration_inputs','tool_sha256']:
            with self.subTest(key=key):
                original = self.previous[key]
                self.previous[key] = 'changed'
                with self.assertRaises(ValueError): self.validate()
                self.previous[key] = original

    def test_rejects_legacy_checkpoint_without_input_digest(self):
        del self.previous['input_sha256']
        with self.assertRaises(ValueError): self.validate()

    def test_rejects_changed_dependency(self):
        self.previous['tool_sha256']['metadata.py'] = 'changed metadata reader'
        with self.assertRaises(ValueError): self.validate()

    def test_rejects_checkpoint_without_tool_identity(self):
        del self.previous['tool_sha256']
        with self.assertRaises(ValueError): self.validate()

    def test_rejects_nonprefix(self):
        self.previous['results'][0]['case'] = 'next'
        with self.assertRaises(ValueError): self.validate()

    def test_rejects_changed_scene_asset_and_blob(self):
        for path in [self.root/'scene.m3p', self.root/'mb3d/M3Formulas/F.m3f',
                     self.output/'case.params']:
            with self.subTest(path=path):
                original = path.read_bytes()
                path.write_bytes(b'changed')
                with self.assertRaises(ValueError): self.validate()
                path.write_bytes(original)

    def test_rejects_missing_blob(self):
        (self.output/'case.params').unlink()
        with self.assertRaises(ValueError): self.validate()

    def test_rejects_invalid_status(self):
        self.previous['results'][0]['status'] = 'skipped'
        with self.assertRaises(ValueError): self.validate()

    def test_unverified_context_still_checks_parameter_identity(self):
        self.previous['results'][0]['status'] = 'unverified'
        (self.output/'case.params').write_bytes(b'changed')
        with self.assertRaises(ValueError): self.validate()

    def test_odd_gnarly_aliases_require_caller_context(self):
        for name in ('gnarlyIFS','gnarly2IFS','gnarly3IFS','ducksIFS'):
            for flip in (-2147483647,-3,-1,1,3,2147483647):
                blob=bytearray(1024);struct.pack_into('<i',blob,512-0x54,flip)
                with self.assertRaises(checker.PointContextRequired):
                    checker.require_independent_point_context(name,blob)

    def test_even_gnarly_aliases_allow_independent_oracle(self):
        for name in ('gnarlyIFS','gnarly2IFS','gnarly3IFS','ducksIFS'):
            for flip in (-2147483648,-2,0,2,2147483646):
                blob=bytearray(1024);struct.pack_into('<i',blob,512-0x54,flip)
                checker.require_independent_point_context(name,blob)

    def test_context_guard_rejects_incomplete_gnarly_buffer(self):
        with self.assertRaises(ValueError):
            checker.require_independent_point_context('gnarlyIFS',b'')
        checker.require_independent_point_context('UnrelatedFormula',b'')


if __name__ == '__main__':
    unittest.main()
