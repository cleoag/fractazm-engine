"""Independent model checks; these do not execute or qualify device formulas."""
import copy
from fractions import Fraction
import struct
import unittest
import native_formula_reference as ref


class ExactBinary32Tests(unittest.TestCase):
    def test_nearest_even_and_third(self):
        self.assertEqual(ref.binary32(Fraction(1, 3)), 0x3eaaaaab)
        self.assertEqual(ref.binary32(1 + Fraction(1, 1 << 24)), 0x3f800000)
        self.assertEqual(ref.binary32(1 + Fraction(3, 1 << 24)), 0x3f800002)
        self.assertEqual(ref.binary32(-1 - Fraction(3, 1 << 24)), 0xbf800002)

    def test_subnormal_boundary_and_overflow(self):
        self.assertEqual(ref.binary32(Fraction(1, 1 << 149)), 1)
        self.assertEqual(ref.binary32(Fraction(1, 1 << 150)), 0)
        self.assertEqual(ref.binary32(Fraction(3, 1 << 150)), 2)
        self.assertEqual(ref.binary32(Fraction((1 << 24) - 1, 1 << 150)), 0x00800000)
        self.assertEqual(ref.multiply(0x7f7fffff, 0x40000000), 0x7f800000)

    def test_signed_zero_rules(self):
        self.assertEqual(ref.add(0x80000000, 0x80000000), 0x80000000)
        self.assertEqual(ref.add(0, 0x80000000), 0)
        self.assertEqual(ref.add(0x3f800000, 0xbf800000), 0)
        self.assertEqual(ref.multiply(0, 0xbf800000), 0x80000000)
        self.assertEqual(ref.multiply(0x80000000, 0xbf800000), 0)

    def test_bit_roundtrip_across_exponents(self):
        for exponent in range(255):
            for tail in (0, 1, 0x400000, 0x7fffff):
                for sign in (0, 0x80000000):
                    bits = sign | (exponent << 23) | tail
                    self.assertEqual(ref.binary32(ref.fraction(bits), bits == 0x80000000), bits)
        with self.assertRaises(ValueError):
            ref.fraction(0x7fc00000)


class AffineReferenceTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.cases = ref.build_cases()
        cls.lookup = {(c['identity'][2], c['variant']): c for c in cls.cases}

    def test_counts_identity_and_complete_state(self):
        self.assertEqual(len(self.cases), 40)
        self.assertEqual(len({c['id'] for c in self.cases}), 40)
        self.assertEqual(sum(c['input']['kind'] == 'point' for c in self.cases), 24)
        for case in self.cases:
            self.assertEqual(set(case['input']), {'identity', 'kind', 'input_state_bits', 'parameter_bits'})
            self.assertEqual(len(case['input']['parameter_bits']), 2048)
            self.assertEqual(set(case['expected_state_bits']), set(ref.STATE))
            self.assertEqual(ref.evaluate(case['input']), case['expected_state_bits'])
            for lane in set(ref.STATE) - set(case['write_lanes']):
                self.assertEqual(case['input']['input_state_bits'][lane], case['expected_state_bits'][lane], (case['id'], lane))

    def test_original_assembly_affine_examples(self):
        expected = {
            '_Translate': ([Fraction(1, 2), Fraction(-3, 4), 1, Fraction(-3, 4)], [Fraction(3, 4), -1, Fraction(5, 4), Fraction(-3, 2)]),
            '_TranslateC4d': ([Fraction(1, 2), Fraction(-3, 4), 1, Fraction(-5, 4)], [Fraction(11, 8), Fraction(-7, 4), Fraction(17, 8), Fraction(-5, 2)]),
            '_scaleC': ([Fraction(3, 8), Fraction(-1, 2), Fraction(5, 8), Fraction(-3, 4)], [Fraction(-3, 2), Fraction(-1, 2), 0, Fraction(-3, 2)]),
            '_ScaleC4d': ([Fraction(-3, 16), Fraction(1, 4), Fraction(-5, 16), Fraction(3, 8)], [Fraction(3, 2), -2, Fraction(5, 2), -3]),
        }
        for name, groups in expected.items():
            actual = self.lookup[name, 'asymmetric']['expected_state_bits']
            for prefix, values in zip(('xyz', 'j'), groups):
                self.assertEqual([actual[f'{prefix}[{i}]'] for i in range(4)], [ref.word(ref.binary32(x)) for x in values])

    def test_large_c_does_not_erase_original_position(self):
        for name in ('_TranslateC4d', '_ScaleC4d'):
            case = self.lookup[name, 'large_c_cancellation']
            self.assertEqual(case['expected_state_bits']['xyz[0]'], '0x3f800000')
            # Deliberately demonstrate the translation compensation is NOT the oracle.
            large = 0x4c000000
            compensated = ref.add(ref.add(0x3f800000, large | 0x80000000), large)
            self.assertEqual(compensated, 0)

    def test_pascal_single_slot_orbit_controls(self):
        state = self.lookup['_Translate', 'orbit_mandel']['expected_state_bits']
        for lane, expected in {'xyz[0]': Fraction(7, 8), 'xyz[1]': Fraction(-3, 2), 'xyz[2]': Fraction(17, 8),
                               'rout': Fraction(241, 32), 'rold': Fraction(83, 16), 'otrap': Fraction(25, 32)}.items():
            self.assertEqual(state[lane], ref.word(ref.binary32(expected)), lane)
        self.assertEqual(state['iteration'], '0x00000004')
        self.assertEqual(state['tmp'], '0x00000000')
        for name in ref.FORMULAS:
            escape = self.lookup[name, 'orbit_escape']['expected_state_bits']
            self.assertEqual(escape['iteration'], '0x00000001')
            self.assertEqual(escape['tmp'], '0x00000002')
            julia = self.lookup[name, 'orbit_julia']['expected_state_bits']
            self.assertEqual(julia['iteration'], '0x00000005')
            self.assertEqual(julia['tmp'], '0x00000001')
        scale = self.lookup['_ScaleC4d', 'orbit_mandel']['expected_state_bits']
        self.assertEqual(scale['rout'], ref.word(ref.binary32(Fraction(25, 8192))))
        self.assertEqual(scale['otrap'], scale['rout'])

    def test_rotate_identity_preserves_original_signed_zero(self):
        case = self.lookup['_ScaleC4d', 'orbit_mandel']
        for i in range(3):
            self.assertTrue(int(case['input']['input_state_bits'][f'c[{i}]'], 16) >> 31)
        actual = case['expected_state_bits']
        self.assertEqual(actual['xyz[3]'], '0x80000000')
        self.assertEqual(actual['j[3]'], '0x80000000')
        zero = copy.deepcopy(case['input'])
        for i in range(3):zero['input_state_bits'][f'c[{i}]'] = '0x80000000'
        output = ref.evaluate(zero)
        for i in range(4):
            self.assertEqual(output[f'xyz[{i}]'], '0x80000000')
            self.assertEqual(output[f'j[{i}]'], '0x80000000')

    def test_nonfinite_outputs_are_outside_reference_domain(self):
        bad = copy.deepcopy(self.lookup['_scaleC', 'asymmetric']['input'])
        bad['input_state_bits']['j[0]'] = '0x7f7fffff'
        with self.assertRaisesRegex(ValueError, 'nonfinite arithmetic result'):
            ref.evaluate(bad)

    def test_radius_exit_and_exact_threshold_are_discriminating(self):
        for name in ref.FORMULAS:
            case = self.lookup[name, 'orbit_escape']
            self.assertEqual(case['input']['input_state_bits']['max_iterations'], '0x00000004')
            self.assertEqual(case['expected_state_bits']['iteration'], '0x00000001')
            threshold = self.lookup[name, 'orbit_repeat']['expected_state_bits']
            self.assertEqual(threshold['iteration'], '0x00000002' if name.startswith('_Translate') else '0x00000004')

    def test_binary64_parameter_and_pvar_views_agree(self):
        for case in self.cases:
            p = [int(x, 16) for x in case['input']['parameter_bits']]
            for i in range(p[60]):
                f32 = struct.unpack('<f', struct.pack('<I', p[68 + i]))[0]
                expected = struct.unpack('<II', struct.pack('<d', f32))
                self.assertEqual(tuple(p[132 + i * 2:134 + i * 2]), expected)
                self.assertEqual(tuple(p[56 + 264 - i * 2:56 + 266 - i * 2]), expected)
                self.assertEqual(p[325 + i], p[68 + i])

    def test_domain_and_identity_fail_closed(self):
        base = self.cases[0]['input']
        for key in ('schema', 'metadata'):
            bad = copy.deepcopy(base);bad[key] = 1
            with self.assertRaises(ValueError):ref.evaluate(bad)
        for index in (1, 6, 56, 58, 61, 62, 65, 131, 388):
            bad = copy.deepcopy(base);bad['parameter_bits'][index] = '0xffffffff'
            with self.assertRaises(ValueError):ref.evaluate(bad)
        bad = copy.deepcopy(base);bad['identity'][2] = '_addC'
        with self.assertRaises(ValueError):ref.evaluate(bad)
        bad = copy.deepcopy(base);bad['input_state_bits']['xyz[0]'] = '0x-0000001'
        with self.assertRaises(ValueError):ref.evaluate(bad)


if __name__ == '__main__':
    unittest.main()
