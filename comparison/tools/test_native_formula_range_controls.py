"""Synthetic range-protocol and source controls; no build/VM/device execution."""
import copy
from pathlib import Path
import unittest

import native_formula_range_controls as v

ROOT = Path(__file__).resolve().parents[2]


def fixture(fault=0, mode=1, exits=1):
    result = [0] * 59
    result[47], result[51], result[56] = fault, 1, int(fault == 0)
    return {'schema': 'mb3d.native-formula-probe.v1', 'profile': 'native_gpu_fp32',
            'status': 'POINT_FAULTS' if fault else 'COMPLETE', 'library_profile': [2, 4, 32],
            'formula_abi': [1, 8192, 188, 236, 32], 'point_count': 1, 'repeats': 3, 'mode': mode,
            'exact_state_repeats': True, 'input_immutable': True, 'allocation_canaries': True,
            'invalid_descriptor_controls': 7, 'fault_count': int(bool(fault)),
            'rows': [{'index': 0, 'input_words': [0] * 47, 'result_words': result, 'fault': fault, 'work': 1}],
            'range_controls': {'schema': 'mb3d.native-formula-range-controls.v1', 'qualification': False,
                               'status': 'EXACT_NONINTERFERENCE', 'kernel': 'native_formula_range_probe',
                               'repeats': 3, 'exact_result_matches_baseline': True, 'exact_control_repeats': True,
                               'allocation_canaries': True, 'input_immutable': True, 'program_immutable': True,
                               'resources_immutable': True, 'runs': [
                                   {'repeat': i + 1, 'rows': [{'index': 0, 'range_exits': exits,
                                    'result_words': result.copy(), 'fault': fault, 'work': 1,
                                    'completed': int(fault == 0)}]} for i in range(3)]}}


class PassiveControls(unittest.TestCase):
    def test_observed_range_count_has_no_numerical_or_frame_credit(self):
        got = v.validate_range_controls(fixture(), [1])
        self.assertEqual(got['counts'], [1]); self.assertEqual(got['work'], [1])
        self.assertFalse(got['qualification'])

    def test_strict_step_fault_retained_and_has_no_orbit_range_count(self):
        got = v.validate_range_controls(fixture(fault=4, mode=0, exits=0), [0])
        self.assertEqual(got['faults'], [4]); self.assertFalse(got['qualification'])

    def test_mixed_multi_input_rows_preserve_counts_and_errors(self):
        raw = fixture(); other = fixture(fault=5, exits=0)
        raw['point_count'] = 2; raw['fault_count'] = 1; raw['status'] = 'POINT_FAULTS'
        raw['rows'].append(other['rows'][0]); raw['rows'][1]['index'] = 1
        for run, error in zip(raw['range_controls']['runs'], other['range_controls']['runs']):
            run['rows'].append(error['rows'][0]); run['rows'][1]['index'] = 1
        got = v.validate_range_controls(raw, [1, 0])
        self.assertEqual(got['faults'], [0, 5]); self.assertEqual(got['counts'], [1, 0])

    def test_any_of_59_words_changed_rejects_even_if_summary_claims_match(self):
        for index in (0, 20, 33, 47, 51, 52, 55, 56, 58):
            raw = fixture(); raw['range_controls']['runs'][1]['rows'][0]['result_words'][index] ^= 1
            with self.subTest(index=index), self.assertRaises(ValueError):
                v.validate_range_controls(raw)

    def test_missing_reused_or_extra_repeats_reject(self):
        for change in (lambda c: c['runs'].pop(), lambda c: c['runs'].append(copy.deepcopy(c['runs'][0])),
                       lambda c: c['runs'][1].update(repeat=1), lambda c: c.update(repeats=2)):
            raw = fixture(); change(raw['range_controls'])
            with self.assertRaises(ValueError): v.validate_range_controls(raw)

    def test_count_mismatch_and_impossible_or_noninteger_counts_reject(self):
        for value in (0, 2, -1, True, 1.0, 1 << 32):
            raw = fixture(); raw['range_controls']['runs'][1]['rows'][0]['range_exits'] = value
            with self.subTest(value=value), self.assertRaises(ValueError): v.validate_range_controls(raw)
        with self.assertRaises(ValueError): v.validate_range_controls(fixture(), [0])
        with self.assertRaises(ValueError): v.validate_range_controls(fixture(mode=0))

    def test_canary_mutable_input_or_failed_noninterference_reject(self):
        for field in ('allocation_canaries', 'input_immutable', 'program_immutable', 'resources_immutable',
                      'exact_control_repeats', 'exact_result_matches_baseline'):
            raw = fixture(); raw['range_controls'][field] = False
            with self.subTest(field=field), self.assertRaises(ValueError): v.validate_range_controls(raw)

    def test_fault_is_not_silently_relabelled_success(self):
        raw = fixture(fault=4, exits=0); raw['status'] = 'COMPLETE'
        with self.assertRaises(ValueError): v.validate_range_controls(raw)
        raw = fixture(); raw['range_controls']['qualification'] = True
        with self.assertRaises(ValueError): v.validate_range_controls(raw)

    def test_wrong_abi_and_future_fields_reject(self):
        raw = fixture(); raw['formula_abi'][3] = 240
        with self.assertRaises(ValueError): v.validate_range_controls(raw)
        raw = fixture(); raw['formula_abi'][0] = True
        with self.assertRaises(ValueError): v.validate_range_controls(raw)
        raw = fixture(); raw['range_controls']['future_allow_mismatch'] = True
        with self.assertRaises(ValueError): v.validate_range_controls(raw)


class SourceControls(unittest.TestCase):
    @staticmethod
    def body(text, name):
        start = text.index('kernel void ' + name + '(')
        start = text.index('{', start)
        depth = 1
        for i in range(start + 1, len(text)):
            depth += (text[i] == '{') - (text[i] == '}')
            if depth == 0:
                return text[start + 1:i]
        raise AssertionError('unclosed kernel')

    def test_only_counter_sidechannel_differs_from_ordinary_kernel_body(self):
        source = (ROOT / 'src/render/native_metal/orbit_driver.metal.h').read_text()
        ordinary = self.body(source, 'native_formula_probe')
        observed = self.body(source, 'native_formula_range_probe')
        self.assertNotIn('range_outputs', ordinary)
        self.assertNotIn('range_count', ordinary)
        self.assertEqual(observed.replace('    uint range_count=0;resources.range_exits=&range_count;\n', '')
                         .replace('    range_outputs[tid]=range_count;\n', ''), ordinary)
        self.assertLess(observed.index('if(tid>=launch.x || tid>=launch.w)return;'),
                        observed.index('resources.range_exits=&range_count'))
        self.assertLess(source.index('#ifdef MB3D_NATIVE_FORMULA_PROBE_KERNELS'),
                        source.index('kernel void native_formula_range_probe('))

    def test_host_opt_in_three_repeats_full_record_comparison_and_guarded_count_buffer(self):
        source = (ROOT / 'src/render/native_metal/formula_probe.mm').read_text()
        self.assertIn('bool map_controls=false,range_controls=false', source)
        self.assertIn('!o.range_controls||o.repeats>=3', source)
        self.assertIn('repeat<options.repeats', source)
        self.assertIn('std::memcmp(baseline.data(),captured.rows.data(),baseline.size()*sizeof(NativeOrbitResult))', source)
        self.assertIn('captured.counts==rangeControls[0].counts', source)
        self.assertIn('bytes[i]==0xa5&&bytes[rangeGuard+rangePayload+i]==0xa5', source)
        self.assertIn('offset:rangeGuard atIndex:7', source)
        self.assertIn('RANGE_CONTROL_MISMATCH', source)
        self.assertIn('return !rangeMatches||!rangeRepeats?3:faults?2:0', source)


if __name__ == '__main__':
    unittest.main()
