"""Pre-device literal genIFS, typed ABI, exact-domain and caller controls."""
import copy
from fractions import Fraction as F
import json
import re
import struct
import unittest
from unittest.mock import patch

import native_group05_ifs_reference as m
from gpu_pipeline_proofs import STATE
from native_formula_observer import validate_case
from qualify_native_metal_all import current_registry


class OriginalGenIFS(unittest.TestCase):
    def test_literal_cycle_and_dense_matrix_scale_then_current_j(self):
        s = m.state()
        for rotation, expected in [('cycle', [1, -18, 4, 6]), ('dense', [5, F(-75, 4), F(75, 8), 6])]:
            out = m.point(s, m.parameters(rotation=rotation))
            self.assertEqual([m.fraction(out[f'xyz[{i}]']) for i in range(4)], expected)
            for lane in set(STATE)-{f'xyz[{i}]' for i in range(4)}-{'current_slot'}:
                self.assertEqual(out[lane], s[lane], lane)
        # The original reads J at +24/+32/+40, not immutable C or originalJulia.
        changed = dict(s)
        for a in ('c', 'original_j'):
            for i in range(3): changed[f'{a}[{i}]'] = m.fp32(123+i)
        actual = m.point(changed, m.parameters())
        self.assertEqual([actual[f'xyz[{i}]'] for i in range(4)], [m.fp32(v) for v in (1, -18, 4, 6)])

    def test_eight_observable_abs_combinations(self):
        expected = [(11, F(13, 4), F(51, 8)), (7, F(21, 4), F(43, 8)),
                    (7, F(-51, 4), F(115, 8)), (3, F(-43, 4), F(107, 8)),
                    (13, F(-19, 4), F(19, 8)), (9, F(-11, 4), F(11, 8)),
                    (9, F(-83, 4), F(83, 8)), (5, F(-75, 4), F(75, 8))]
        s = m.state(); s['xyz[1]'] = m.fp32(-4)
        for mask, literal in enumerate(expected):
            p = m.parameters(flags=tuple((mask >> i) & 1 for i in range(3)), rotation='dense')
            out = m.point(s, p)
            self.assertEqual(tuple(m.fraction(out[f'xyz[{i}]']) for i in range(3)), literal, mask)

    def test_all_nonzero_integer_encodings_take_the_same_branch(self):
        s = m.state(); s['xyz[1]'] = m.fp32(-4)
        zero = m.point(s, m.parameters(flags=(0, 0, 0)))
        for axis in range(3):
            flags = [0, 0, 0]; flags[axis] = 1
            expected = m.point(s, m.parameters(flags=flags))
            self.assertNotEqual(expected, zero)
            for value in (-1, -(1 << 31), (1 << 31)-1, 1 << 24, (1 << 24)+1):
                flags[axis] = value
                self.assertEqual(m.point(s, m.parameters(flags=flags)), expected)

    def test_original_zero_angle_words_and_signed_zero_abs(self):
        self.assertEqual(m.prepared_matrix('zero_angles'),
                         [m.ONE, m.NEG_ZERO, 0, 0, m.ONE, m.NEG_ZERO, 0, 0, m.ONE])
        s = m.state(); s.update({'xyz[0]': m.NEG_ZERO, 'xyz[1]': m.ONE, 'xyz[2]': m.fp32(-1), 'j[0]': m.NEG_ZERO})
        for flag, expected in [(0, m.NEG_ZERO), (1, 0)]:
            p = m.parameters(1, (flag, 0, 0), (0, 0, 0), 'zero_angles'); p[4] = m.NEG_ZERO
            self.assertEqual(m.point(s, p)['xyz[0]'], expected)

    def test_signed_zero_scale_is_point_only(self):
        s = m.state()
        for scale, expected_w in [(0, m.NEG_ZERO), (m.NEG_ZERO, 0)]:
            p = m.parameters(); p[0] = scale
            out = m.point(s, p)
            self.assertEqual(out['xyz[3]'], expected_w)
            for i in range(3): self.assertEqual(out[f'xyz[{i}]'], s[f'j[{i}]'])
            with self.assertRaisesRegex(m.ScopeError, 'zero denominator'):
                m.orbit(m.orbit_state(), p)

    def test_mandel_closed_form_not_repeated_point_implementation(self):
        s = m.orbit_state(julia=0, count=3)
        p = m.parameters(2, (0, 0, 0), (0, 0, 0), 'zero_angles')
        out = m.orbit(s, p)
        # a_n=2^(n+1)-1, w=2^n, |C|=13; n=4.
        self.assertEqual([m.fraction(out[f'xyz[{i}]']) for i in range(4)], [93, 124, 372, 16])
        self.assertEqual([m.fraction(out[k]) for k in ('rout', 'rold', 'otrap', 'sse[0]')], [162409, 38025, 169, F(403, 16)])
        self.assertEqual((out['first'], out['iteration'], out['tmp']), (0, 4, 2))
        for i in range(4): self.assertEqual(out[f'derivative[{i}]'], s[f'derivative[{i}]'])
        for i in range(3): self.assertEqual(out[f'j[{i}]'], s[f'c[{i}]'])

    def test_julia_cycle_and_negative_w_have_exact_distance(self):
        s = m.orbit_state(count=3); s['max_iterations'] = 3
        p = m.parameters(-2, (0, 0, 0), (0, 0, 0), 'cycle')
        out = m.orbit(s, p)
        self.assertEqual([m.fraction(out[f'xyz[{i}]']) for i in range(4)], [-24, -32, -96, -8])
        self.assertEqual([out[f'sse[{i}]'] for i in range(4)], [m.fp32(13), 0, m.fp32(8), 0])
        self.assertEqual((out['iteration'], out['tmp']), (3, 0))

    def test_offset_before_scale_closed_form(self):
        out = m.orbit(m.orbit_state(), m.parameters(F(1, 2), (0, 0, 0), (F(3, 4), 1, 3), 'zero_angles'))
        # a_n=2^-n +(1-2^-n)/4; n=4, a_n=19/64.
        self.assertEqual([m.fraction(out[f'xyz[{i}]']) for i in range(4)], [F(57, 64), F(19, 16), F(57, 16), F(1, 16)])
        self.assertEqual(m.fraction(out['sse[0]']), F(247, 4))

    def test_bailout_equality_continues(self):
        p = m.parameters(2, (0, 0, 0), (0, 0, 0), 'zero_angles')
        for stop, iteration in [(338, 1), (676, 2)]:
            s = m.orbit_state(); s['rstop'] = m.fp32(stop)
            self.assertEqual(m.orbit(s, p)['iteration'], iteration)

    def test_preparation_and_arithmetic_domain_rejections(self):
        with self.assertRaisesRegex(ValueError, 'arbitrary'):
            m.matrix.prepare_zero_angles([m.fp32(30)]*3)
        with self.assertRaisesRegex(m.ScopeError, 'product'):
            m.exact_mul(0x3f800001, 0x3f800001)
        with self.assertRaisesRegex(m.ScopeError, 'subset'):
            m.dot([m.ONE]*3, list(map(m.fp32, [1 << 24, 1, -(1 << 24)])))
        with self.assertRaisesRegex(m.ScopeError, 'sum'):
            m.exact_add(m.fp32(1 << 24), m.ONE)
        for bits in (1, 0x7f800000, 0x7fc00001):
            with self.assertRaises(ValueError): m.exact_mul(bits, m.ONE)
        with self.assertRaises(ValueError): m.exact_mul(0x7f7fffff, m.fp32(2))

    def test_irrational_and_rounded_distance_rejections(self):
        s = m.orbit_state()
        for i in range(3): s[f'c[{i}]'] = m.ONE
        with self.assertRaisesRegex(m.ScopeError, 'irrational'):
            m.orbit(s, m.parameters(1, (0, 0, 0), (0, 0, 0), 'zero_angles'))
        s = m.orbit_state(julia=0); s['max_iterations'] = 1
        with self.assertRaisesRegex(m.ScopeError, 'rounded distance'):
            m.orbit(s, m.parameters(3, (0, 0, 0), (0, 0, 0), 'zero_angles'))


class Evidence(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.cases = [r for r in m.build_cases() if r['identity'][2] == 'genIFS']; cls.registry = current_registry(m.ROOT)

    def test_fixed_84_case_coverage_preservation_and_observer_abi(self):
        self.assertEqual(len(self.cases), 84)
        self.assertEqual(sum(r['input']['kind'] == 'point' for r in self.cases), 37)
        self.assertEqual(len({r['id'] for r in self.cases}), 84)
        self.assertEqual(len({json.dumps(r['input'], sort_keys=True) for r in self.cases}), 84)
        for row in self.cases:
            validate_case(row['input'], self.registry)
            self.assertEqual(set(row['expected_state_bits']), set(STATE))
            for lane in set(STATE)-set(row['write_lanes']):
                self.assertEqual(row['expected_state_bits'][lane], row['input']['input_state_bits'][lane], (row['id'], lane))

    def test_int32_storage_and_exact_double_mirror_survive_float_collision(self):
        s = m.state(); results = []
        for value in ((1 << 24), (1 << 24)+1):
            p = m.program(m.parameters(flags=(value, 0, 0)), s); results.append(p)
            self.assertEqual(p[319], value)  # PVar-20, not a float interpretation.
            self.assertEqual(struct.unpack('<d', struct.pack('<II', *p[134:136]))[0], value)
        self.assertEqual(results[0][69], results[1][69])
        self.assertNotEqual(results[0][319], results[1][319])
        p = m.program(m.parameters(flags=(-(1 << 31), 0, 0)), s)
        self.assertEqual(p[319], 0x80000000)
        self.assertEqual(p[69], m.fp32(-(1 << 31)))

    def test_original_parameter_offsets_and_negative_alias_cache(self):
        params = m.parameters(flags=(-1, 0, -(1 << 31)))
        p = m.program(params, m.state()); memory = struct.pack('<64I', *p[260:324])
        self.assertEqual(struct.unpack_from('<d', memory, 248)[0], 0.5)
        self.assertEqual(struct.unpack_from('<d', memory, 240)[0], -2)
        self.assertEqual(struct.unpack_from('<iii', memory, 228), (-(1 << 31), 0, -1))
        for i, (offset, kind) in enumerate(m.LAYOUT):
            if kind == 'single': self.assertEqual(struct.unpack_from('<I', memory, 256+offset)[0], params[i])
        for i in range(32):
            self.assertEqual(p[324+i], m.matrix.narrow64(struct.unpack_from('<Q', memory, 248-8*i)[0]))

    def test_noncanonical_abi_cannot_be_silently_repaired(self):
        row = self.cases[0]['input']
        for index in (1, 6, 50, 58, 60, 61, 62, 63, 68, 69, 132, 134, 319, 324, 325, 388):
            bad = copy.deepcopy(row); bad['parameter_bits'][index] = m.word(int(bad['parameter_bits'][index], 16) ^ 1)
            with self.assertRaises(ValueError, msg=str(index)): m.evaluate(bad)

    def test_descriptor_matches_pinned_transport_metadata(self):
        # ABI metadata is transport, not the numerical oracle or a formula body.
        text = (m.ROOT/'src/render/native_metal/generated/formula_metadata.h').read_text()
        match = re.search(r'case 429u: slot.traits=(\d+)u; slot.wrapper_flags=(\d+)u; slot.difs_style=(\d+)u;', text)
        self.assertIsNotNone(match)
        self.assertEqual(tuple(map(int, match.groups())), (0, 0, 0))
        self.assertEqual(m.FORMULAS['genIFS'], (429, 2, 16, 0, 0, 1))
        for row in self.cases:
            self.assertEqual(row['input']['parameter_bits'][61:64], ['0x00000000']*3)

    def test_invalid_caller_and_encoding_rejected(self):
        row = next(r['input'] for r in self.cases if r['input']['kind'] == 'orbit')
        for key, value in [('julia', 2), ('inside', 1), ('calc_smooth', 1), ('max_iterations', 0), ('max_iterations', 9),
                           ('hybrid_counts[0]', 0), ('hybrid_counts[0]', 4), ('hybrid_counts[1]', 1),
                           ('hybrid_counts[1]', 0x80000001), ('vary_scale', m.fp32(2)), ('j[3]', 0)]:
            bad = copy.deepcopy(row); bad['input_state_bits'][key] = m.word(value)
            with self.assertRaises(ValueError, msg=key): m.evaluate(bad)
        bad = copy.deepcopy(row); bad['input_state_bits']['xyz[0]'] = '0X00000000'
        with self.assertRaises(ValueError): m.evaluate(bad)
        bad = copy.deepcopy(row); bad['identity'][1] += 1
        with self.assertRaises(ValueError): m.evaluate(bad)

    def test_original_double_is_exactly_widened_not_arbitrary_default(self):
        bad = copy.deepcopy(self.cases[0]['input'])
        lo, hi = struct.unpack('<II', struct.pack('<d', -1.6))
        bad['parameter_bits'][132:134] = [m.word(lo), m.word(hi)]
        with self.assertRaisesRegex(ValueError, 'ABI'): m.evaluate(bad)

    def test_source_helper_and_caller_pins_are_mandatory(self):
        manifest = m.manifest()
        self.assertEqual(manifest['code_audit']['genIFS']['bytes'], 143)
        self.assertEqual(set(manifest['dependencies']), set(m.DEPENDENCY_PINS))
        self.assertEqual({str(p.relative_to(m.ROOT)) for p in m.dependency_paths()}, set(m.DEPENDENCY_PINS))
        for pins, key in [(m.SOURCE_PINS, 'mb3d/M3Formulas/genIFS.m3f'),
                          (m.DEPENDENCY_PINS, 'comparison/tools/native_group05_scaling_reference.py'),
                          (m.ABI_PINS, 'src/render/native_metal/generated/formula_metadata.h')]:
            with patch.dict(pins, {key: '0'*64}):
                with self.assertRaisesRegex(ValueError, 'pinned source'): m.manifest()
        with patch.object(m.caller, 'CALLER_PROFILE', (m.caller.CALLER_PROFILE[0], '0'*64)):
            with self.assertRaisesRegex(ValueError, 'pinned source'): m.manifest()

    def test_manifest_survives_collection_json_roundtrip(self):
        manifest = m.manifest()
        self.assertEqual(json.loads(json.dumps(manifest)), manifest)


class OriginalSierpHilbert(unittest.TestCase):
    def test_four_independently_interpreted_literals(self):
        controls = [((3, 4, 12), m.sierp_parameters(), [-5, -2, 34, -9]),
                    ((-2, 4, -4), m.sierp_parameters(), [-8, -2, 4, -9]),
                    ((-2, 4, -4), m.sierp_parameters(-2, (1, 2, -1), (0, 0)), [-5, -2, -7, 6]),
                    ((-2, 4, -4), m.sierp_parameters(-2, (1, 2, -1), (2, 1), 'cycle', 'quarter'), [7, 10, -11, 6])]
        for xyz, params, expected in controls:
            s = m.state()
            for i, value in enumerate(xyz): s[f'xyz[{i}]'] = m.fp32(value)
            out = m.point(s, params, 'SierpHilbert')
            self.assertEqual([m.fraction(out[f'xyz[{i}]']) for i in range(4)], expected)
            for lane in set(STATE)-{f'xyz[{i}]' for i in range(4)}-{'current_slot'}:
                self.assertEqual(out[lane], s[lane], lane)

    def test_no_julia_or_c_addition(self):
        s = m.state(); params = m.sierp_parameters()
        expected = m.point(s, params, 'SierpHilbert')
        for lane in ('c', 'j', 'original_j', 'derivative', 'sse'):
            for i in range(4): s[f'{lane}[{i}]'] = m.fp32(100+i)
        actual = m.point(s, params, 'SierpHilbert')
        self.assertEqual([actual[f'xyz[{i}]'] for i in range(4)], [expected[f'xyz[{i}]'] for i in range(4)])

    def test_inclusive_fold_and_edge_signed_zero_literals(self):
        s = m.state()
        for i in range(3): s[f'xyz[{i}]'] = 0
        params = m.sierp_parameters(1, (1, 1, 1), (0, 0))
        self.assertEqual(m.sierp_folds([0, 0, 0]), [m.NEG_ZERO, m.NEG_ZERO, 0])
        self.assertEqual([m.point(s, params, 'SierpHilbert')[f'xyz[{i}]'] for i in range(3)], [0, m.NEG_ZERO, 0])
        for edge, expected_y in [(m.NEG_ZERO, m.NEG_ZERO), (m.fp32(-1), m.NEG_ZERO), (m.ONE, 0)]:
            params[22] = edge
            self.assertEqual(m.point(s, params, 'SierpHilbert')['xyz[1]'], expected_y)

    def test_strict_fold_mutation_is_detected(self):
        def strict(values):
            v = list(values)
            for a, b in ((0, 1), (0, 2), (1, 2)):
                if -m.fraction(v[a]) > m.fraction(v[b]):
                    v[a], v[b] = v[b] ^ m.NEG_ZERO, v[a] ^ m.NEG_ZERO
            return v
        s = m.state()
        for i in range(3): s[f'xyz[{i}]'] = 0
        params = m.sierp_parameters(1, (1, 1, 1), (0, 0))
        original = m.point(s, params, 'SierpHilbert')
        with patch.object(m, 'sierp_folds', strict):
            changed = m.point(s, params, 'SierpHilbert')
        self.assertEqual(original['xyz[1]'], m.NEG_ZERO)
        self.assertEqual(changed['xyz[1]'], 0)

    def test_edges_are_y_then_x_and_only_positive(self):
        for axis in (0, 1):
            for value, expected in [(1, 1), (2, 2), (3, 1)]:
                s = m.state()
                for i in range(3): s[f'xyz[{i}]'] = m.fp32(value if i == axis else 0)
                edges = (2, 0) if axis == 1 else (0, 2)
                out = m.point(s, m.sierp_parameters(1, (0, 0, 0), edges), 'SierpHilbert')
                self.assertEqual(m.fraction(out[f'xyz[{axis}]']), expected)
        s = m.state()
        baseline = m.point(s, m.sierp_parameters(1, (0, 0, 0), (0, 0)), 'SierpHilbert')
        self.assertEqual(m.point(s, m.sierp_parameters(1, (0, 0, 0), (-2, -3)), 'SierpHilbert'), baseline)

    def test_signed_zero_scale_and_nonunit_cscale(self):
        for scale, expected_w in [(0, m.NEG_ZERO), (m.NEG_ZERO, 0)]:
            params = m.sierp_parameters(0, (1, 2, -1), (0, 0)); params[0] = scale
            out = m.point(m.state(), params, 'SierpHilbert')
            self.assertEqual([m.fraction(out[f'xyz[{i}]']) for i in range(3)], [1, 2, -1])
            self.assertEqual(out['xyz[3]'], expected_w)

    def test_default_one_iteration_orbit_has_exact_distance(self):
        for julia in (0, 1):
            s = m.orbit_state(julia=julia); s['max_iterations'] = 1
            for i in range(3): s[f'c[{i}]'] = m.ONE if i == 0 else 0
            out = m.orbit(s, m.sierp_parameters(), 'SierpHilbert')
            self.assertEqual([m.fraction(out[f'xyz[{i}]']) for i in range(4)], [1, -2, -2, 3])
            self.assertEqual(m.fraction(out['rout']), 9)
            self.assertEqual([out[f'sse[{i}]'] for i in range(4)], [m.ONE, 0, m.fp32(3), 0])
            self.assertEqual((out['first'], out['iteration'], out['tmp']), (0, 1, 0))

    def test_norm_preservation_bailout_and_counter_cycle(self):
        for stop, iterations in [(338, 1), (676, 2), (1 << 16, 4)]:
            s = m.orbit_state(count=3); s['rstop'] = m.fp32(stop)
            params = m.sierp_parameters(2, (0, 0, 0), (0, 0), 'cycle', 'quarter')
            out = m.orbit(s, params, 'SierpHilbert')
            self.assertEqual(out['iteration'], iterations)
            self.assertEqual(m.fraction(out['rout']), 169*4**iterations)
            self.assertEqual(m.fraction(out['sse[0]']), 13)
            self.assertEqual(out['tmp'], 3-((iterations-1) % 3)-1)

    def test_active_edges_and_zero_radius_orbits(self):
        for axis in (0, 1):
            s = m.orbit_state()
            for i in range(3): s[f'c[{i}]'] = m.fp32(4 if i == axis else 0)
            edges = (1, 0) if axis == 1 else (0, 1)
            out = m.orbit(s, m.sierp_parameters(1, (0, 0, 0), edges), 'SierpHilbert')
            self.assertEqual(m.fraction(out['rout']), 4)
            self.assertEqual(m.fraction(out['sse[0]']), 2)
        s = m.orbit_state(); s['max_iterations'] = 3
        for i in range(3): s[f'c[{i}]'] = 0
        out = m.orbit(s, m.sierp_parameters(1, (1, 1, 1), (0, 0)), 'SierpHilbert')
        self.assertEqual((out['rout'], out['sse[0]'], out['xyz[1]']), (0, 0, m.NEG_ZERO))

    def test_rounding_at_edge_scale_minus_one_and_cscale_rejected(self):
        s = m.state()
        for i, value in enumerate((2, -1, 2)): s[f'xyz[{i}]'] = m.fp32(value)
        with self.assertRaisesRegex(m.ScopeError, 'sum'):
            m.point(s, m.sierp_parameters(1, (0, 0, 0), (1 << 24, 0)), 'SierpHilbert')
        with self.assertRaisesRegex(m.ScopeError, 'sum'):
            m.point(m.state(), m.sierp_parameters(1 << 25), 'SierpHilbert')
        params = m.sierp_parameters(F(5, 2)); params[1] = 0x3f800001
        with self.assertRaisesRegex(m.ScopeError, 'product'):
            m.point(m.state(), params, 'SierpHilbert')


class SierpEvidence(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.all_cases = m.build_cases()
        cls.cases = [r for r in cls.all_cases if r['identity'][2] == 'SierpHilbert']
        cls.registry = current_registry(m.ROOT)

    def test_86_new_and_170_combined_unique_cases(self):
        self.assertEqual(len(self.cases), 86)
        self.assertEqual(sum(r['input']['kind'] == 'point' for r in self.cases), 46)
        self.assertEqual(len(self.all_cases), 170)
        self.assertEqual(len({r['id'] for r in self.all_cases}), 170)
        self.assertEqual(len({json.dumps(r['input'], sort_keys=True) for r in self.all_cases}), 170)
        for row in self.cases:
            validate_case(row['input'], self.registry)
            self.assertEqual(set(row['expected_state_bits']), set(STATE))
            for lane in set(STATE)-set(row['write_lanes']):
                self.assertEqual(row['expected_state_bits'][lane], row['input']['input_state_bits'][lane], (row['id'], lane))

    def test_typed_double_single_offsets_and_metadata(self):
        params = m.sierp_parameters(-2, (1, 2, -1), (2, 1), 'cycle', 'quarter')
        p = m.program(params, m.state(), 'SierpHilbert'); memory = struct.pack('<64I', *p[260:324])
        self.assertEqual(p[58:64], [590, 2, 24, 0, 1, 0])
        for (offset, kind), value in zip(m.LAYOUTS['SierpHilbert'], params):
            actual = struct.unpack_from('<Q' if kind == 'double' else '<I', memory, 256+offset)[0]
            self.assertEqual(actual, m.matrix.wide32(value) if kind == 'double' else value)
        text = (m.ROOT/'src/render/native_metal/generated/formula_metadata.h').read_text()
        self.assertIn('case 590u: slot.traits=0u; slot.wrapper_flags=1u; slot.difs_style=0u;', text)

    def test_sierp_specific_metadata_and_raw_layout_mutations_rejected(self):
        base = self.cases[0]['input']
        for index in (58, 59, 60, 61, 62, 63, 69, 72, 81, 90, 91, 134, 176, 178, 292, 294, 324):
            bad = copy.deepcopy(base); bad['parameter_bits'][index] = m.word(int(bad['parameter_bits'][index], 16) ^ 1)
            with self.assertRaises(ValueError, msg=str(index)): m.evaluate(bad)

    def test_original_sierp_source_pin_is_required(self):
        self.assertEqual(m.manifest()['code_audit']['SierpHilbert']['bytes'], 289)
        with patch.dict(m.SOURCE_PINS, {'mb3d/M3Formulas/SierpHilbert.m3f': '0'*64}):
            with self.assertRaisesRegex(ValueError, 'pinned source'): m.manifest()


if __name__ == '__main__':
    unittest.main()
