"""Synthetic diagnostic-validator tests; never device or formula evidence."""
import copy
from pathlib import Path
import unittest
from unittest.mock import patch
import native_formula_reference as reference
import verify_native_affine_controls as controls


def raw_report(batch, policy=0):
    program = batch['program']
    tail = [0, 0, 0, 0, 1, program[11], 0, 0, 0, 1, 0, 0]
    return {'schema': 'mb3d.native-formula-probe.v1', 'status': 'COMPLETE', 'profile': 'native_gpu_fp32',
            'library_profile': [2, 4, 32], 'formula_abi': [1, 8192, 188, 236, 32],
            'scene': None, 'program': '/program', 'library': '/library', 'mode': 0, 'slot': 0,
            'point_count': len(batch['states']), 'repeats': 3, 'wrapper_policy': policy,
            'invalid_descriptor_controls': 7, 'fault_count': 0, 'exact_state_repeats': True,
            'input_immutable': True, 'allocation_canaries': True, 'device': 'SYNTHETIC FIXTURE',
            'wrapper_audit_words': [1, policy, 0] * 6, 'program_words': program,
            'rows': [{'index': i, 'input_words': state, 'result_words': expected + tail, 'fault': 0, 'work': 1}
                     for i, (state, expected) in enumerate(zip(batch['states'], batch['expected']))]}


class DenseAffineControlsTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.batches = controls.build_controls()

    def validate(self, report, batch=None, policy=0):
        return controls.validate_bulk(report, batch or self.batches[0], policy, Path('/program'), Path('/library'))

    def result_map(self, batch):
        return {(side, policy): self.validate(raw_report(batch, policy), batch, policy)
                for side in ('old', 'candidate') for policy in (0, 1)}

    def test_corpus_dimensions_and_program_bound_state(self):
        original = {x['id']: x['input'] for x in reference.build_cases() if x['input']['kind'] == 'point'}
        self.assertEqual(len(self.batches), 24)
        self.assertEqual(sum(len(b['states']) for b in self.batches), 3072)
        preserved = [name for name in controls.STATE if name.startswith(('original_j[', 'hybrid_counts['))]
        preserved += ['julia', 'inside', 'calc_smooth', 'max_iterations', 'rstop']
        for batch in self.batches:
            source = original[batch['id']]
            self.assertEqual(batch['program'], [int(v, 16) for v in source['parameter_bits']])
            for lane in preserved:
                index = controls.STATE.index(lane)
                self.assertTrue(all(state[index] == int(source['input_state_bits'][lane], 16) for state in batch['states']))
            for lane in ('first', 'iteration', 'tmp'):
                index = controls.STATE.index(lane)
                self.assertTrue(set(controls.COUNTERS) <= {s[index] for s in batch['states']})
            self.assertTrue(any(s[0] == 0x80000000 for s in batch['states']))
            self.assertTrue(any(s[8] == 0x4c000000 for s in batch['states']))
            self.assertTrue(any(s[8] == 0xcc000000 for s in batch['states']))

    def test_deterministic_corpus(self):
        self.assertEqual(controls.build_controls(), self.batches)

    def test_complete_report_and_unchanged_formula(self):
        batch = self.batches[0]
        result = controls.compare_results(batch, self.result_map(batch))
        self.assertTrue(result['unaffected_59_words_exact'])
        self.assertEqual(result['old_reference_mismatches'], [])

    def test_raw_faults_integrity_and_wrong_lengths_rejected(self):
        mutations = [lambda x: x.update(status='POINT_FAULTS'), lambda x: x.update(fault_count=1),
                     lambda x: x['rows'][0].update(fault=1), lambda x: x['rows'][0]['result_words'].__setitem__(47, 1),
                     lambda x: x['rows'][0]['result_words'].pop(), lambda x: x['rows'].pop(),
                     lambda x: x.update(exact_state_repeats=False), lambda x: x.update(input_immutable=False),
                     lambda x: x.update(allocation_canaries=False), lambda x: x.update(repeats=2),
                     lambda x: x.update(wrapper_policy=1), lambda x: x['wrapper_audit_words'].__setitem__(1, 1),
                     lambda x: x['rows'][0]['input_words'].__setitem__(0, 0),
                     lambda x: x['rows'][0]['result_words'].__setitem__(51, 2)]
        for index, mutate in enumerate(mutations):
            report = copy.deepcopy(raw_report(self.batches[0]));mutate(report)
            with self.subTest(index=index), self.assertRaises(ValueError):self.validate(report)

    def test_candidate_reference_mismatch_rejected(self):
        batch = self.batches[0];results = self.result_map(batch)
        for policy in (0, 1):results['candidate', policy][0][0] ^= 1
        with self.assertRaisesRegex(ValueError, 'independent 47-word'):controls.compare_results(batch, results)

    def test_full59_policy_and_old_work_changes_rejected(self):
        batch = self.batches[0];results = self.result_map(batch)
        results['candidate', 1][0][58] = 1
        with self.assertRaisesRegex(ValueError, 'Wrapper policies'):controls.compare_results(batch, results)
        results = self.result_map(batch)
        for policy in (0, 1):results['old', policy][0][51] = 2
        with self.assertRaisesRegex(ValueError, 'metadata or work'):controls.compare_results(batch, results)
        results = self.result_map(batch)
        for policy in (0, 1):results['old', policy][0][0] ^= 1
        with self.assertRaisesRegex(ValueError, 'Unaffected formula'):controls.compare_results(batch, results)

    def test_old_c4_mismatch_preserved_without_fitting_candidate(self):
        batch = next(b for b in self.batches if b['identity'][2] == '_TranslateC4d')
        results = self.result_map(batch)
        for policy in (0, 1):results['old', policy][0][0] = 0
        actual = controls.compare_results(batch, results)
        self.assertEqual(actual['old_reference_mismatches'][0], {'row': 0, 'lanes': ['xyz[0]']})
        self.assertTrue(actual['candidate_reference_exact'])

    def test_missing_policy_and_result_words_rejected(self):
        batch = self.batches[0];results = self.result_map(batch)
        results.pop(('old', 1))
        with self.assertRaises(ValueError):controls.compare_results(batch, results)
        results = self.result_map(batch);results['candidate', 1][0].pop()
        with self.assertRaises(ValueError):controls.compare_results(batch, results)

    def test_invalid_deadline_never_launches_device(self):
        with patch.object(controls, 'invoke') as invoke:
            with self.assertRaises(ValueError):controls.collect(Path('/old'), Path('/candidate'), Path('/output'), 151)
            invoke.assert_not_called()


if __name__ == '__main__':
    unittest.main()
