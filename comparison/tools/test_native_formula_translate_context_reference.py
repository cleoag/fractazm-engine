"""Source-model controls only; no device, build, VM or qualification credit."""
import copy
from fractions import Fraction
import unittest
from unittest.mock import patch

import native_formula_translate_context_reference as m
from gpu_pipeline_proofs import STATE


class TranslateContextReferenceTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.rows = m.build_cases()

    def test_original_source_pins_and_exact_context_program(self):
        manifest = m.source_manifest()
        self.assertEqual(manifest['status'], 'SOURCE_MODEL_READY_FOR_DIAGNOSTIC')
        p = m.program_words()
        self.assertEqual(p[:14], [1, 1, 6, 0, 0, 2, 2, 400, 1, 0, 0, 1 << 20, 1 << 20, 0])
        self.assertEqual(p[50:53], [30, 0x80000001, 5])
        self.assertEqual([p[58 + 332 * i] for i in range(3)], [1509, 8065, 7039])
        # This real scene has a legitimate infinity in a dormant prepared
        # parameter; rejecting every nonfinite parameter would corrupt it.
        self.assertEqual(p[69], 0x7f800000)

    def test_source_only_point_set_keeps_open_and_early_exit_controls(self):
        self.assertEqual(len(self.rows), 10)
        self.assertEqual(self.rows[0]['semantic_diagnostic']['status'], 'OPEN_MODEL_DOMAIN')
        for i in (1, 2, 6):
            d = self.rows[i]['semantic_diagnostic']
            self.assertFalse(d['target_and_successor_reached'])
            self.assertEqual(d['expected_slot_visits'][1:], [0] * 5)
        for i in (3, 4, 5, 7):
            self.assertTrue(self.rows[i]['semantic_diagnostic']['target_and_successor_reached'])

    def test_all47fields_now_follow_reviewed_original_return_transport(self):
        for row in self.rows[1:]:
            d = row['semantic_diagnostic']
            self.assertEqual(set(d['expected_state_bits']) | set(d['unproved_fields']), set(STATE))
            self.assertEqual(len(d['expected_state_bits']), 47)
            self.assertEqual(d['unproved_fields'], {})
            self.assertFalse(d['qualification'])
            self.assertEqual(m.evaluate(row['input']), d['expected_state_bits'])
            state = {k: m.decode(v) for k, v in d['expected_state_bits'].items()}
            w = state['xyz[3]'] & 0x7fffffff
            self.assertEqual([state[k] for k in m.SSE_FIELDS], [m.div(m.affine.sqrt32(state['rout']), w), 0, w, 0])

    def test_true_modifier_is_uncounted_and_successor_uses_own_slot(self):
        d = self.rows[3]['semantic_diagnostic']
        self.assertEqual(d['expected_slot_visits'], [30, 1, 3, 0, 0, 0])
        self.assertEqual(d['expected_work'], 34)
        self.assertEqual(d['expected_state_bits']['iteration'], '0x00000021')
        target, successor = d['trace'][30:32]
        self.assertEqual((target['slot'], target['formula_id'], target['counted'], target['iteration']), (1, 8065, False, 30))
        self.assertEqual((target['before']['tmp'], target['remaining']), ('0x00000001', 0))
        self.assertEqual((successor['slot'], successor['formula_id'], successor['iteration']), (2, 7039, 31))
        self.assertEqual(successor['before']['tmp'], '0x00000005')
        self.assertEqual(successor['before']['xyz[0]'], target['after']['xyz[0]'])
        self.assertEqual(successor['before']['rout'], target['before']['rout'])

    def test_translate_transition_preserves_every_other_known_field(self):
        for row in self.rows[1:]:
            for step in row['semantic_diagnostic']['trace']:
                if step['slot'] != 1:
                    continue
                before, after = step['before'], step['after']
                self.assertEqual(after['xyz[0]'], m.arithmetic.word(m.add(m.decode(before['xyz[0]']), m.rounded(Fraction(-7, 2)))))
                self.assertEqual(after['xyz[1]'], m.arithmetic.word(m.add(m.decode(before['xyz[1]']), 0)))
                self.assertEqual(after['xyz[2]'], m.arithmetic.word(m.add(m.decode(before['xyz[2]']), 0)))
                for lane in m.KNOWN_FIELDS:
                    if lane not in ('xyz[0]', 'xyz[1]', 'xyz[2]'):
                        self.assertEqual(after[lane], before[lane], lane)

    def test_surfs_original_counter_and_stored_inner_scale(self):
        # Original CODE loads PVar[-24], not evolving VaryScale / MinR^2,
        # on its strict inner sphere branch.
        s = dict.fromkeys(STATE, 0)
        s.update({'xyz[0]': m.rounded(Fraction(1, 4)), 'xyz[3]': m.ONE,
                  'vary_scale': m.rounded(99), 'first': 0})
        params = list(map(m.rounded, [2, 8, Fraction(1, 4), 1,
                                     1, 0, 0, 0, 1, 0, 0, 0, 1, Fraction(1, 4), 0]))
        m.surf(s, params)
        self.assertEqual(s['first'], 1)
        self.assertEqual(s['vary_scale'], m.rounded(Fraction(9, 4)))
        self.assertEqual(s['xyz[0]'], m.rounded(2))
        self.assertEqual(s['xyz[3]'], m.rounded(8))

    def test_original_box_fold_radius_and_no_julia_addition(self):
        s = dict.fromkeys(STATE, 0)
        s.update({'xyz[0]': m.rounded(3), 'xyz[3]': m.rounded(2), 'j[0]': m.rounded(123)})
        m.box(s, list(map(m.rounded, [8, Fraction(1, 4), 2])))
        self.assertEqual(s['xyz[0]'], m.rounded(-2))
        self.assertEqual(s['xyz[3]'], m.rounded(4))
        self.assertEqual(s['j[0]'], m.rounded(123))

    def test_surfs_original_three_matrix_rows(self):
        s = dict.fromkeys(STATE, 0)
        for i, value in enumerate((1, 2, 3)):
            s[f'xyz[{i}]'] = m.rounded(value)
        s['xyz[3]'] = m.ONE
        # Non-symmetric rows detect row/column swaps and the x87 stack's
        # unusual Z-row-first computation followed by X/Y/Z stores.
        params = list(map(m.rounded, [1, 4, Fraction(1, 4), 10,
                                     1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 0]))
        m.surf(s, params)
        self.assertEqual([s[f'xyz[{i}]'] for i in range(3)], list(map(m.rounded, [14, 32, 50])))

    def test_full47_carried_second_orbit_includes_source_return_sse(self):
        for source_index, point_index, carried in ((3, 4, self.rows[8]), (4, 3, self.rows[9])):
            previous = self.rows[source_index]['semantic_diagnostic']
            current = carried['semantic_diagnostic']
            for field in m.STATE:
                if field not in ('c[0]', 'c[1]', 'c[2]'):
                    self.assertEqual(carried['input']['input_state_bits'][field], previous['expected_state_bits'][field])
            self.assertEqual(carried['carry']['unproved_fields'], [])
            self.assertEqual(carried['carry']['carried_fields'], list(m.STATE))
            self.assertTrue(current['target_and_successor_reached'])
            self.assertEqual(current['expected_slot_visits'], self.rows[point_index]['semantic_diagnostic']['expected_slot_visits'])
            # First resets on entry, so the original Surf must reinitialize
            # VaryScale even though its incoming carried value is different.
            self.assertEqual(current['expected_state_bits']['vary_scale'], self.rows[point_index]['semantic_diagnostic']['expected_state_bits']['vary_scale'])

    def test_return_transport_overwrites_every_incoming_sse_lane(self):
        original = self.rows[3]['input']
        expected = m.evaluate(original)
        for seed in (1, 7, 19):
            case = copy.deepcopy(original)
            for i, name in enumerate(m.SSE_FIELDS):
                case['input_state_bits'][name] = m.arithmetic.word(m.rounded(Fraction((-1) ** i * (seed + i), 16)))
            self.assertEqual(m.evaluate(case), expected)

    def test_compiled_caller_and_sqrt_proof_dependencies_are_pinned(self):
        manifest = m.source_manifest()
        self.assertEqual(manifest['helper_hashes']['comparison/tools/native_formula_affine_v2_reference.py'], m.AFFINE_MODEL_SHA)
        self.assertEqual(manifest['data_dependencies'][m.CALLER_PROFILE['path']], m.CALLER_PROFILE['sha256'])
        self.assertTrue(any(name.endswith('/v2/compile/original_translate_context_oracle.exe') for name in manifest['data_dependencies']))
        self.assertTrue(any(name.endswith('/v2/pe-disassembly.txt') for name in manifest['data_dependencies']))
        bad = dict(m.CALLER_PROFILE, sha256='0' * 64)
        with patch.object(m, 'CALLER_PROFILE', bad), self.assertRaisesRegex(m.ScopeError, 'profile changed'):
            m.source_manifest()

    def test_rounded_signed_zero_and_domain_controls(self):
        self.assertEqual(m.add(0x3f800000, m.rounded(Fraction(1, 1 << 24))), 0x3f800000)
        self.assertEqual(m.add(0x80000000, 0x80000000), 0x80000000)
        self.assertEqual(m.mul(0x80000000, m.ONE), 0x80000000)
        with self.assertRaises(m.ScopeError):
            m.div(m.ONE, 0)
        with self.assertRaises(m.ScopeError):
            m.mul(0x7f7fffff, m.rounded(2))

    def test_original_settings_and_target_binding_cannot_change(self):
        original = self.rows[3]['input']
        mutations = [lambda c: c['parameter_bits'].__setitem__(7, '0x00000008'),
                     lambda c: c['parameter_bits'].__setitem__(51, '0x00000001'),
                     lambda c: c['context'].__setitem__('target_slot', 0),
                     lambda c: c['input_state_bits'].__setitem__('calc_smooth', '0x00000001')]
        for mutation in mutations:
            case = copy.deepcopy(original)
            mutation(case)
            with self.assertRaises((ValueError, AssertionError)):
                m.evaluate_semantics(case)


if __name__ == '__main__':
    unittest.main()
