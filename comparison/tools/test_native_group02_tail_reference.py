"""Independent pre-device math/ABI/state controls; no renderer or GPU outputs."""
import copy
from fractions import Fraction
import hashlib
import struct
import unittest

import native_group02_tail_reference as m
from gpu_pipeline_proofs import STATE
from native_formula_observer import validate_case


class IntervalAndArithmeticTests(unittest.TestCase):
    def test_machin_enclosure_and_axis_rounding(self):
        # Independent decimal enclosure, not a host-libm or device result.
        decimal = '3.14159265358979323846264338327950288419716939937510'
        lower = Fraction(decimal); upper = lower+Fraction(1, 10**len(decimal.split('.')[1]))
        lo, hi = m.pi_interval()
        self.assertLess(lower, lo); self.assertLess(hi, upper)
        self.assertEqual(m.rounded_interval((lo, hi)), 0x40490fdb)
        self.assertEqual(m.atan2_axis(m.ONE, 0), 0x3fc90fdb)
        self.assertEqual(m.atan2_axis(m.fp32(-1), m.NEG_ZERO), 0xbfc90fdb)

    def test_signed_axis_fpatan_does_not_use_host_atan2(self):
        for x in (m.ONE, 0):
            self.assertEqual(m.atan2_axis(0, x), 0)
            self.assertEqual(m.atan2_axis(m.NEG_ZERO, x), m.NEG_ZERO)
        for x in (m.fp32(-1), m.NEG_ZERO):
            self.assertEqual(m.atan2_axis(0, x), 0x40490fdb)
            self.assertEqual(m.atan2_axis(m.NEG_ZERO, x), 0xc0490fdb)
        with self.assertRaisesRegex(m.ScopeError, 'atan2'): m.atan2_axis(m.ONE, m.ONE)

    def test_tiny_nonzero_sine_cosine_are_interval_proven(self):
        for multiplier in (1, 2, 3, 9, 27, 81):
            value = m.fp32(Fraction(multiplier, 1 << 20))
            self.assertEqual(m.sin_bits(value), value)
            self.assertEqual(m.sin_bits(m.neg(value)), m.neg(value))
            self.assertEqual(m.cos_bits(value), m.ONE)
            for cosine in (False, True):
                lo, hi = m.trig_interval(value, cosine)
                self.assertLessEqual(lo, hi)
                self.assertEqual(m.fp32(lo), m.fp32(hi))
        self.assertEqual(m.sin_bits(m.NEG_ZERO), m.NEG_ZERO)
        self.assertEqual(m.cos_bits(m.NEG_ZERO), m.ONE)

    def test_literal_two_pi_is_not_an_exact_turn(self):
        self.assertEqual(m.TWO_PI, 0x40c90fdb)
        self.assertEqual(m.DEG, 0x3c8efa35)
        self.assertGreater(m.fraction(m.TWO_PI), 2*m.pi_interval()[1])
        self.assertEqual(m.sin_bits(m.TWO_PI), 0x343bbd2e)
        self.assertEqual(m.sin_bits(m.neg(m.TWO_PI)), 0xb43bbd2e)
        self.assertEqual(m.cos_bits(m.TWO_PI), m.ONE)
        self.assertNotEqual(m.sin_bits(m.TWO_PI), 0)

    def test_interval_and_domain_ambiguity_fail_closed(self):
        with self.assertRaisesRegex(m.ScopeError, 'unique'):
            m.rounded_interval((Fraction(1), 1+Fraction(1, 1 << 23)))
        with self.assertRaises(m.ScopeError): m.sin_bits(m.fp32(17))
        with self.assertRaises(m.ScopeError): m.sin_bits(1)  # Subnormal input.
        with self.assertRaises(m.ScopeError): m.cos_bits(0x7f800000)
        with self.assertRaises(m.ScopeError): m.divide(m.ONE, 0)

    def test_fma_and_radius_counterexamples_are_rejected(self):
        with self.assertRaisesRegex(m.ScopeError, 'grouping/FMA'):
            m.sum_products([(m.fp32(1), m.ONE), (m.fp32(1), m.ONE), (m.fp32(1 << 24), m.ONE)])
        s = m._state()
        for i, x in enumerate((1, 1, 4096)): s[f'xyz[{i}]'] = m.fp32(x)
        with self.assertRaisesRegex(m.ScopeError, 'grouping/FMA'): m.radius(s)
        # FMA can retain a product's discarded low bit through cancellation.
        with self.assertRaisesRegex(m.ScopeError, 'grouping/FMA'):
            m.sum_products([(0x3f800001, 0x3f7ffffe), (m.fp32(-1), m.ONE)])

    def test_radius_dominance_is_strictly_below_half_ulp(self):
        for x in (2, 3):
            s = m._state(); s['xyz[0]'], s['xyz[1]'], s['xyz[2]'] = m.fp32(x), m.fp32(Fraction(1, 1 << 20)), 0
            for iteration in range(5):
                expected_y = Fraction(x**iteration, 1 << 20)
                self.assertEqual(m.fraction(s['xyz[1]']), expected_y)
                # At every step, both the exact and rounded small square
                # remain below half the next representable radius increment.
                dominant = m.fp32(x*x)
                half_ulp = (m.fraction(dominant+1)-m.fraction(dominant))/2
                self.assertLess(expected_y**2, half_ulp)
                self.assertLess(m.fraction(m.mul(s['xyz[1]'], s['xyz[1]'])), half_ulp)
                self.assertEqual(m.radius(s), dominant)
                if iteration < 4: s = m._point('_invcylindrical', s, [])

    def test_rounding_real_and_int32_contracts_are_distinct(self):
        for value, expected in [(Fraction(1, 2), 0), (Fraction(3, 2), 2),
                                (Fraction(5, 2), 2), (Fraction(-1, 2), 0), (Fraction(-3, 2), -2)]:
            self.assertEqual(m.nearest_integer(m.fp32(value)), expected)
        self.assertEqual(m.frndint(m.fp32(Fraction(-1, 2))), m.NEG_ZERO)
        self.assertEqual(m.fistp_int32(m.fp32(Fraction(-1, 2))), 0)
        self.assertEqual(m.frndint(m.fp32(1 << 31)), 0x4f000000)
        self.assertEqual(m.fistp_int32(m.fp32(1 << 31)), m.INT_MIN)
        self.assertEqual(m.fistp_int32(m.fp32((1 << 31)-128)), (1 << 31)-128)
        self.assertEqual(m.fistp_int32(m.fp32(-(1 << 31))), m.INT_MIN)


class OriginalStateAndABITests(unittest.TestCase):
    @classmethod
    def setUpClass(cls): cls.rows = m.build_cases()

    def row(self, ident): return next(r for r in self.rows if r['id'] == ident)

    def test_poly_half_sector_control_has_large_discriminating_output(self):
        positive = self.row('7065-half_sector_1')['expected_state_bits']
        negative = self.row('7065-half_sector_-1')['expected_state_bits']
        self.assertEqual((positive['xyz[0]'], positive['xyz[1]']), ('0x00000000', '0x3f800000'))
        self.assertEqual((negative['xyz[0]'], negative['xyz[1]']), ('0x80000000', '0xbf800000'))
        # RN32 axis atan / original narrowed constant really is half-sector.
        for sign in (1, -1):
            q = m.mul(m.divide(m.atan2_axis(m.fp32(sign), 0), m.TWO_PI), m.fp32(2))
            self.assertEqual(m.fraction(q), Fraction(sign, 2))
            self.assertEqual(m.fraction(m.frndint(q)), 0)

    def test_original_fsubrp_sign_and_shifted_symmetric_state(self):
        row = self.row('7065-sector_zero_dense')
        original = row['input']['input_state_bits']; out = row['expected_state_bits']
        self.assertEqual(out['xyz[0]'], m.word(int(original['xyz[0]'], 16)^m.NEG_ZERO))
        self.assertEqual(out['xyz[1]'], original['xyz[1]'])
        out = self.row('7076-sector_zero_shifted')['expected_state_bits']
        self.assertEqual(m.fraction(int(out['xyz[0]'], 16)), Fraction(-7, 8))
        self.assertEqual(m.fraction(int(out['xyz[1]'], 16)), Fraction(1, 4))

    def test_finite_fistp_overflow_changes_ngon_y_not_range_status(self):
        for flag, expected in [(0, 0xc1490fdb), (1, m.TWO_PI), (-1, m.TWO_PI), (2147483647, m.TWO_PI)]:
            name = '7081-integer_positive_overflow' if flag == 0 else f'7081-no_yfold_{flag}'
            state = self.row(name)['expected_state_bits']
            self.assertEqual(int(state['xyz[1]'], 16), expected)
            self.assertEqual(state['xyz[0]'], '0xbf800000')
        sym = self.row('7076-integer_positive_overflow')['expected_state_bits']
        self.assertEqual(int(sym['xyz[1]'], 16), m.sin_bits(m.neg(m.TWO_PI)))
        before = self.row('7076-integer_below')['expected_state_bits']
        self.assertEqual(int(before['xyz[1]'], 16), m.sin_bits(m.TWO_PI))

    def test_ngon_fild_zero_is_positive_even_for_negative_zero_q(self):
        row = self.row('7081-signed_zero_integer_reload')
        self.assertEqual(row['input']['input_state_bits']['xyz[1]'], '0x00000000')
        self.assertEqual(row['expected_state_bits']['xyz[1]'], '0x80000000')
        q, order = m.NEG_ZERO, m.fp32(-2)
        original = m.mul(m.divide(m.neg(m.sub(q, m.fp32(m.fistp_int32(q)))), order), m.TWO_PI)
        wrong_float_reload = m.mul(m.divide(m.neg(m.sub(q, m.frndint(q))), order), m.TWO_PI)
        self.assertEqual((original, wrong_float_reload), (m.NEG_ZERO, 0))

    def test_siny_exact_axis_does_not_round_integer_through_float(self):
        for index in (16777217, -16777217, 16777219, 2147483647, m.INT_MIN, m.INT_MIN+1):
            row = self.row(f'7010-raw_index_{index}')
            out, before = row['expected_state_bits'], row['input']['input_state_bits']
            selected = f'xyz[{index & 3}]'
            self.assertEqual(out[selected], '0x40000000')
            for axis in range(4):
                if f'xyz[{axis}]' != selected: self.assertEqual(out[f'xyz[{axis}]'], before[f'xyz[{axis}]'])
        self.assertNotEqual(int(m.fraction(m.fp32(16777217))) & 3, 16777217 & 3)

    def test_siny_packed_integer_and_single_parameter_layout(self):
        params = [16777217, 0, m.ONE, m.fp32(2), m.fp32(-1)]
        p = m._program('_SinY', params, m._state())
        raw = struct.pack('<64I', *p[56+204:56+268])
        self.assertEqual(struct.unpack_from('<i', raw, 244)[0], 16777217)
        for i, value in enumerate(params[1:]):
            self.assertEqual(struct.unpack_from('<I', raw, 240-4*i)[0], value)
        self.assertEqual(m._parameters('_SinY', p)[0], 16777217)
        self.assertEqual(m.fraction(p[68]), 16777216)
        metadata = p[56+76] | p[56+77] << 32
        self.assertEqual(m.fraction64(metadata), 16777217)

    def test_ngon_integer_offset_and_true_decimal_constants(self):
        params = m.polygon_params('_ngon', flag=-1)
        p = m._program('_ngon', params, m._state())
        raw = struct.pack('<64I', *p[56+204:56+268])
        self.assertEqual(struct.unpack_from('<i', raw, 204)[0], -1)  # PVar-52.
        for i, offset in enumerate((0, 8), start=6):
            metadata = p[56+76+2*i] | p[56+77+2*i] << 32
            self.assertEqual(metadata, m.CONSTANTS[offset])
        self.assertEqual(p[56+300:56+302], [m.TWO_PI, m.DEG])
        self.assertLess(m.fraction64(m.CONSTANTS[0]), 2*m.pi_interval()[0])
        self.assertGreater(m.fraction64(m.CONSTANTS[8]), m.pi_interval()[1]/180)

    def test_metadata_raw_alias_and_constant_tampering_fail_closed(self):
        for row_id, offsets in [('7010-raw_index_16777217', (56+76, 56+204+61, 56+269)),
                                ('7081-integer_positive_overflow', (56+76+12, 56+300))]:
            for index in offsets:
                altered = copy.deepcopy(self.row(row_id)['input'])
                altered['parameter_bits'][index] = m.word(int(altered['parameter_bits'][index], 16)^1)
                with self.assertRaises(ValueError): m.evaluate(altered)

    def test_fixed_corpus_all_state_lanes_and_unchanged_fields(self):
        self.assertEqual(len(self.rows), 77)
        self.assertEqual(len({r['id'] for r in self.rows}), 77)
        self.assertEqual(sum(r['input']['kind']=='orbit' for r in self.rows), 21)
        registry = {('external', d[0], name) for name, d in m.FORMULAS.items()}
        for row in self.rows:
            validate_case(row['input'], registry)
            self.assertEqual(set(row['expected_state_bits']), set(STATE))
            self.assertEqual(row['expected_state_bits'], m.evaluate(row['input']))
            for lane in set(STATE)-set(row['write_lanes']):
                self.assertEqual(row['expected_state_bits'][lane], row['input']['input_state_bits'][lane], (row['id'], lane))
        for name in m.FORMULAS:
            self.assertEqual({r['input']['kind'] for r in self.rows if r['identity'][2]==name}, {'point', 'orbit'})

    def test_orbits_change_real_coordinates_and_preserve_original_state(self):
        for ident, x in [(4522, 2)]:
            row = self.row(f'{ident}-orbit_mandel'); out = row['expected_state_bits']
            self.assertEqual(m.fraction(int(out['xyz[1]'], 16)), Fraction(x**4, 1 << 20))
            self.assertEqual(m.fraction(int(out['rout'], 16)), x*x)
        out = self.row('7010-orbit_mandel')['expected_state_bits']
        self.assertEqual(m.fraction(int(out['xyz[1]'], 16)), Fraction(1, 1 << 24))
        self.assertEqual(self.row('7010-orbit_w_write')['expected_state_bits']['xyz[3]'], '0x40000000')
        ngon = self.row('7081-orbit_mandel')['expected_state_bits']
        self.assertEqual((ngon['xyz[0]'], ngon['xyz[1]']), ('0xbf800000', '0x00000000'))
        for ident, *_ in m.FORMULAS.values():
            row = self.row(f'{ident}-orbit_repeat')
            self.assertEqual(row['expected_state_bits']['iteration'], '0x00000004')
            self.assertEqual(self.row(f'{ident}-orbit_escape')['expected_state_bits']['iteration'], '0x00000001')
            for lane in (f'c[{i}]' for i in range(4)):
                self.assertEqual(row['expected_state_bits'][lane], row['input']['input_state_bits'][lane])

    def test_singular_and_unproven_polygon_domains_are_not_successes(self):
        for name in ('_PolyFolding', '_PolyFold-sym', '_ngon'):
            with self.assertRaisesRegex(m.ScopeError, 'singular'):
                m._point(name, m._state(), m.polygon_params(name, order=0))
            with self.assertRaisesRegex(m.ScopeError, 'atan2'):
                m._point(name, m._state(), m.polygon_params(name, order=2))
        with self.assertRaisesRegex(m.ScopeError, 'int32'):
            m._program('_SinY', [1 << 31, 0, m.ONE, m.ONE, 0], m._state())

    def test_source_and_dependency_pins_and_non_universal_accuracy_claim(self):
        manifest = m.manifest()
        self.assertEqual(len(manifest['code_audit']), 5)
        self.assertIn('mb3d/DivUtils.pas', manifest['semantic_sources'])
        self.assertEqual(set(manifest['abi_sources']), set(m.ABI_PINS))
        data = m.dependency_paths()[0].read_bytes().replace(b'\r\n', b'\n')
        for content in (data, data.replace(b'\n', b'\r\n')):
            self.assertEqual(m.primitive_identity(content), hashlib.sha256(content).hexdigest())
        with self.assertRaises(ValueError): m.primitive_identity(data.replace(b'def add(', b'def changed_add(', 1))
        limits = ' '.join(manifest['model_limits'])
        self.assertIn('not guaranteed universally RN32', limits)
        self.assertIn('one-ULP difference alone', limits)


if __name__ == '__main__': unittest.main()
