"""Synthetic policy-control validation; no GPU or qualification evidence."""
import copy
import json
from pathlib import Path
import struct
import tempfile
from types import SimpleNamespace
import unittest
from unittest.mock import patch

import verify_native_reciprocal_policies as controls


def raw_report(batch, policy):
    marker = 5823 if batch['identity'][1] == 5823 and policy == 1 else 0
    tail = [0, 0, 0, 0, 1, batch['program'][11], 0, 0, 0, 1, 0, 0]
    return {'schema': 'mb3d.native-formula-probe.v1', 'status': 'COMPLETE',
            'profile': 'native_gpu_fp32', 'library_profile': [2, 4, 32],
            'formula_abi': [1, 8192, 188, 236, 32], 'scene': None,
            'program': '/program', 'library': '/library', 'mode': 0, 'slot': 0,
            'point_count': len(batch['states']), 'repeats': 3, 'wrapper_policy': policy,
            'invalid_descriptor_controls': 7, 'fault_count': 0,
            'exact_state_repeats': True, 'input_immutable': True, 'allocation_canaries': True,
            'device': 'SYNTHETIC FIXTURE', 'wrapper_audit_words': [1, policy, marker] + [1, policy, 0] * 5,
            'program_words': batch['program'],
            'rows': [{'index': i, 'input_words': state, 'result_words': expected + tail,
                      'fault': 0, 'work': 1}
                     for i, (state, expected) in enumerate(zip(batch['states'], batch['expected']))]}


class ReciprocalPolicyTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.batches = controls.build_controls()

    def batch(self, fid):
        return next(b for b in self.batches if b['identity'][1] == fid)

    def validate(self, report, batch, policy):
        return controls.validate_bulk(report, batch, policy, Path('/program'), Path('/library'))

    def results(self, batch):
        return {(side, policy): self.validate(raw_report(batch, policy), batch, policy)
                for side in ('old', 'candidate') for policy in (0, 1)}

    def test_exact_predeclared_cases_grouped_by_entire_program(self):
        cases = {c['id']: c for c in controls.reference.build_cases() if c['input']['kind'] == 'point'}
        self.assertEqual(len(cases), 57)
        self.assertEqual(len(self.batches), 26)
        self.assertEqual(len({tuple(b['program']) for b in self.batches}), 26)
        seen = []
        for batch in self.batches:
            for i, cid in enumerate(batch['case_ids']):
                case = cases[cid];seen.append(cid)
                self.assertEqual(batch['program'], [int(w, 16) for w in case['input']['parameter_bits']])
                self.assertEqual(batch['states'][i], [int(case['input']['input_state_bits'][n], 16) for n in controls.STATE])
                self.assertEqual(batch['expected'][i], [int(case['expected_state_bits'][n], 16) for n in controls.STATE])
        self.assertEqual(set(seen), set(cases));self.assertEqual(len(seen), 57)
        self.assertEqual(controls.build_controls(), self.batches)

    def test_missing_duplicate_or_changed_expected_case_rejected(self):
        original = controls.reference.build_cases()
        for cases in (original[1:], [original[1]] + original[1:]):
            with patch.object(controls.reference, 'build_cases', return_value=cases):
                with self.assertRaisesRegex(ValueError, 'predeclared point corpus'):
                    controls.build_controls()
        cases = copy.deepcopy(original)
        cases[0]['expected_state_bits']['xyz[0]'] = '0x12345678'
        with patch.object(controls.reference, 'build_cases', return_value=cases):
            with self.assertRaisesRegex(ValueError, 'does not replay'):controls.build_controls()

    def test_real_specialization_id_and_all_inactive_slots(self):
        for batch in self.batches:
            for policy in (0, 1):
                report = raw_report(batch, policy)
                self.validate(report, batch, policy)
                self.assertEqual(report['wrapper_audit_words'][2],
                                 5823 if batch['identity'][1] == 5823 and policy else 0)
                report['wrapper_audit_words'][5] = 5823
                with self.assertRaises(ValueError):self.validate(report, batch, policy)
        batch = self.batch(5823)
        for marker in (0, 1, 6030):
            report = raw_report(batch, 1);report['wrapper_audit_words'][2] = marker
            with self.assertRaisesRegex(ValueError, 'wrapper policy'):self.validate(report, batch, 1)

    def test_abi_inputs_canaries_and_result_integrity_rejected(self):
        batch = self.batch(5823)
        mutations = [lambda r: r.update(status='POINT_FAULTS'), lambda r: r.update(fault_count=1),
                     lambda r: r['rows'][0].update(work=2), lambda r: r['rows'][0].update(fault=1),
                     lambda r: r['rows'][0].update(index=True), lambda r: r['rows'].pop(),
                     lambda r: r['rows'][0]['result_words'].pop(),
                     lambda r: r['rows'][0]['input_words'].__setitem__(0, 123),
                     lambda r: r['program_words'].__setitem__(11, 1),
                     lambda r: r['formula_abi'].__setitem__(3, 188),
                     lambda r: r['library_profile'].__setitem__(2, 64),
                     lambda r: r.update(exact_state_repeats=False), lambda r: r.update(input_immutable=False),
                     lambda r: r.update(allocation_canaries=False), lambda r: r.update(repeats=2),
                     lambda r: r.update(wrapper_policy=0), lambda r: r.update(invalid_descriptor_controls=6),
                     lambda r: r.update(library='/wrong')]
        mutations += [lambda r, i=i: r['rows'][0]['result_words'].__setitem__(i, 123) for i in range(47, 59)]
        for i, mutate in enumerate(mutations):
            report = copy.deepcopy(raw_report(batch, 1));mutate(report)
            with self.subTest(mutation=i), self.assertRaises(ValueError):self.validate(report, batch, 1)

    def test_candidate_must_match_reference_and_both_policies(self):
        batch = self.batch(4812);results = self.results(batch)
        for policy in (0, 1):results['candidate', policy][0][0] ^= 1
        with self.assertRaisesRegex(ValueError, 'independent 47-word'):controls.compare_results(batch, results)
        results = self.results(batch);results['candidate', 1][0][58] ^= 1
        with self.assertRaisesRegex(ValueError, 'Candidate wrapper policies'):controls.compare_results(batch, results)

    def test_old_policy_mismatches_remain_separate_and_need_not_agree(self):
        batch = self.batch(5823);results = self.results(batch)
        results['old', 0][0][0] ^= 1
        results['old', 1][0][1] ^= 1
        result = controls.compare_results(batch, results)
        self.assertFalse(result['old_policies_59_words_exact'])
        self.assertEqual(result['old_reference_mismatches_by_policy']['0'][0]['lanes'], ['xyz[0]'])
        self.assertEqual(result['old_reference_mismatches_by_policy']['1'][0]['lanes'], ['xyz[1]'])
        self.assertEqual(result['old_reference_mismatches_by_policy']['0'][0]['case_id'], batch['case_ids'][0])
        self.assertTrue(result['candidate_reference_exact'])

    def test_unchanged_formulas_and_old_work_must_stay_exact(self):
        for fid in (6030, 7007):
            batch = self.batch(fid);results = self.results(batch)
            self.assertTrue(controls.compare_results(batch, results)['unaffected_59_words_exact'])
            results['old', 1][0][0] ^= 1
            with self.assertRaisesRegex(ValueError, 'Unaffected'):controls.compare_results(batch, results)
        batch = self.batch(4812);results = self.results(batch);results['old', 0][0][51] = 2
        with self.assertRaisesRegex(ValueError, 'metadata or work'):controls.compare_results(batch, results)

    def test_missing_policy_or_raw_case_rejected(self):
        batch = self.batch(4812)
        for action in (lambda r: r.pop(('old', 1)), lambda r: r['candidate', 0].pop(),
                       lambda r: r['old', 1][0].pop()):
            results = self.results(batch);action(results)
            with self.assertRaises(ValueError):controls.compare_results(batch, results)
        altered = copy.deepcopy(batch);altered['expected'].pop()
        with self.assertRaisesRegex(ValueError, 'Missing expected'):controls.compare_results(altered, self.results(batch))

    def test_guards_precede_any_device_launch(self):
        with patch.object(controls, 'invoke') as invoke:
            with self.assertRaises(ValueError):controls.collect('/old', '/candidate', '/output', 151)
            with patch.object(controls.shutil, 'disk_usage', return_value=SimpleNamespace(free=0)):
                with self.assertRaisesRegex(ValueError, 'Disk reserve'):
                    controls.collect(controls.ROOT/'old', controls.ROOT/'candidate', controls.ROOT/'unused-reciprocal-controls-test')
            invoke.assert_not_called()

    def test_all_expected_arrays_exist_before_first_invoke_and_closure_is_captured(self):
        with tempfile.TemporaryDirectory(dir=controls.ROOT/'comparison/runs') as td:
            root = Path(td);builds = []
            for side in ('old', 'candidate'):
                base = root/side;base.mkdir();build = base/'build';build.mkdir();builds.append(build)
                for name in ('mb3d_native_formula_probe', 'metal_native_scene.metallib'):
                    (build/name).write_bytes(b'SYNTHETIC NOT EXECUTABLE')
                (base/'compile.json').write_text('{}');(base/'source-manifest.json').write_text('{}')
                (base/'build-receipt.json').write_text(json.dumps({
                    'records': [{'path': 'compile.json', 'sha256': controls.sha(base/'compile.json')}],
                    'source_manifest_sha256': controls.sha(base/'source-manifest.json')}))
            output = root/'output'
            def intercepted(argv, run, cwd, **kwargs):
                expected = list(output.glob('*/expected-state.bin'))
                self.assertEqual(len(expected), 26)
                self.assertEqual(sum(p.stat().st_size for p in expected), 57*47*4)
                for batch in self.batches:
                    folder = output/batch['id']
                    self.assertEqual((folder/'program.bin').read_bytes(), struct.pack('<2048I', *batch['program']))
                    self.assertEqual((folder/'expected-state.bin').read_bytes(),
                                     b''.join(struct.pack('<47I', *s) for s in batch['expected']))
                deps = set(kwargs['input_files'])
                self.assertIn(Path(controls.reference.__file__).resolve(), deps)
                self.assertTrue(set(controls.reference.dependency_paths()) <= deps)
                self.assertTrue(set(controls.observer.dependency_paths()) <= deps)
                self.assertIn(root/'old/compile.json', deps)
                self.assertIn(root/'candidate/source-manifest.json', deps)
                self.assertTrue(kwargs['exclusive_gpu']);self.assertEqual(kwargs['timeout'], 150)
                self.assertEqual(kwargs['termination_grace_seconds'], 1)
                raise RuntimeError('INTENTIONAL NO GPU')
            with patch.object(controls, 'validate_build_products'), patch.object(controls, 'invoke', side_effect=intercepted) as invoke:
                with patch.object(controls.shutil, 'disk_usage', return_value=SimpleNamespace(free=99*1024**3)):
                    with self.assertRaisesRegex(RuntimeError, 'INTENTIONAL NO GPU'):
                        controls.collect(*builds, output)
                self.assertEqual(invoke.call_count, 1)
            report = json.loads((output/'controls.json').read_text())
            self.assertEqual(report['status'], 'FAIL_DIAGNOSTIC');self.assertFalse(report['qualification'])


if __name__ == '__main__':
    unittest.main()
