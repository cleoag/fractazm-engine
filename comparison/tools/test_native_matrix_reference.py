"""Pre-device original-state, arithmetic and packed-ABI controls."""
import copy
from fractions import Fraction
import hashlib
import struct
import unittest

import native_matrix_reference as m
from gpu_pipeline_proofs import STATE
from native_formula_observer import validate_case


DENSE = list(map(m.fp32, (Fraction(1, 2), Fraction(1, 4), Fraction(-1, 8),
                        Fraction(-1, 4), 1, Fraction(1, 2),
                        Fraction(1, 8), Fraction(-1, 2), Fraction(1, 4))))


class ExactDomainTests(unittest.TestCase):
    def test_nontrivial_exact_dot_and_signed_zero(self):
        self.assertEqual(m.fraction(m.exact_dot(list(map(m.fp32, (1, 2, 3))), DENSE[:3])), Fraction(5, 8))
        self.assertEqual(m.exact_dot([m.NEG_ZERO, m.NEG_ZERO], [m.ONE, m.ONE]), m.NEG_ZERO)
        self.assertEqual(m.exact_dot([m.NEG_ZERO, 0], [m.ONE, m.ONE]), 0)

    def test_rejects_rounded_products_even_when_final_cancellation_is_exact(self):
        with self.assertRaises(m.ScopeError):
            m.exact_dot([m.fp32(Fraction(13, 10))]*2, [m.fp32(Fraction(7, 10)), m.fp32(Fraction(-7, 10))])

    def test_all_subset_sums_must_be_exact_not_only_left_association(self):
        # (2^25-2^25)+1 is exact, but 2^25+1 is not. Accepting only the
        # original partial sums would incorrectly promise grouping invariance.
        with self.assertRaises(m.ScopeError):
            m.exact_dot(list(map(m.fp32, (1 << 25, -(1 << 25), 1))), [m.ONE]*3)

    def test_subnormal_and_nonfinite_domain_is_explicit(self):
        with self.assertRaises(m.ScopeError): m.exact_mul(1, m.ONE)
        with self.assertRaises(ValueError): m.exact_mul(0x7f800000, m.ONE)
        with self.assertRaises(ValueError): m.exact_mul(0x7f7fffff, m.fp32(2))


class OriginalSemanticsTests(unittest.TestCase):
    def test_zero_angle_preparation_preserves_negative_coefficients(self):
        self.assertEqual(m.prepare_zero_angles([0, 0, 0]),
                         [m.ONE, m.NEG_ZERO, 0, 0, m.ONE, m.NEG_ZERO, 0, 0, m.ONE])
        self.assertEqual(m.prepare_zero_angles([m.NEG_ZERO, 0, m.NEG_ZERO]),
                         [m.ONE, 0, 0, m.NEG_ZERO, m.ONE, 0, 0, m.NEG_ZERO, m.ONE])
        with self.assertRaisesRegex(m.ScopeError, 'Euler'):
            m.prepare_zero_angles([0, 0, m.fp32(90)])

    def test_update_changes_mutable_j_not_c_or_original_j(self):
        s = m._state()
        for i, (x, j) in enumerate(((1, 8), (2, -4), (3, 2))):
            s[f'xyz[{i}]'], s[f'j[{i}]'] = m.fp32(x), m.fp32(j)
        out = m._point('_updateC', s, [m.fp32(2), m.fp32(Fraction(1, 2))])
        self.assertEqual([m.fraction(out[f'j[{i}]']) for i in range(3)], [6, 2, 7])
        for lane in set(STATE)-{'j[0]', 'j[1]', 'j[2]', 'current_slot'}:
            self.assertEqual(out[lane], s[lane], lane)

    def test_row_major_matrix_and_j_target_use_original_vector(self):
        for name, lane in [('_Rotate', 'xyz'), ('_RotateC', 'j')]:
            s = m._state()
            for i, v in enumerate((1, 2, 3)): s[f'{lane}[{i}]'] = m.fp32(v)
            out = m._point(name, s, DENSE)
            self.assertEqual([m.fraction(out[f'{lane}[{i}]']) for i in range(3)],
                             [Fraction(5, 8), Fraction(13, 4), Fraction(-1, 8)])
            for untouched in set(STATE)-{f'{lane}[{i}]' for i in range(3)}-{'current_slot'}:
                self.assertEqual(out[untouched], s[untouched], (name, untouched))

    def test_basis_vectors_expose_transposition(self):
        for column in range(3):
            s = m._state()
            for i in range(3): s[f'xyz[{i}]'] = m.ONE if i == column else 0
            out = m._point('_Rotate', s, DENSE)
            self.assertEqual([out[f'xyz[{i}]'] for i in range(3)], [DENSE[3*i+column] for i in range(3)])
        self.assertNotEqual(DENSE[1], DENSE[3])

    def test_identity_matrix_still_executes_signed_zero_dot_operations(self):
        s = m._state()
        for i in range(3): s[f'xyz[{i}]'] = m.NEG_ZERO
        out = m._point('_Rotate', s, m.prepare_zero_angles([0, 0, 0]))
        self.assertEqual([out[f'xyz[{i}]'] for i in range(3)], [0, 0, m.NEG_ZERO])


class PackedABIAndCorpusTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls): cls.rows = m.build_cases()

    def test_matrix_uses_four_byte_stride_and_widened_metadata(self):
        params = m.prepare_zero_angles([0, 0, 0])
        p = m._program('_Rotate', params, m._state())
        memory = struct.pack('<64I', *p[56+204:56+268])
        for i, value in enumerate(params):
            self.assertEqual(struct.unpack_from('<I', memory, 244-4*i)[0], value)
            metadata = p[56+76+2*i] | p[56+77+2*i] << 32
            expected = struct.unpack('<Q', struct.pack('<d', struct.unpack('<f', struct.pack('<I', value))[0]))[0]
            self.assertEqual(metadata, expected)
        self.assertEqual(struct.unpack_from('<Q', memory, 248)[0], 0x3fe0000000000000)
        self.assertEqual(p[56+268], 0x3f000000)  # Original PVar-8 = Double .5.
        self.assertEqual(p[56+269], 0x3c000004)  # Packed m01/m00, not Double m00.
        self.assertEqual(p[56+78:56+80], [0, 0x80000000])  # Single -0 widened to Double -0.

    def test_update_uses_double_stride_not_single_matrix_stride(self):
        params = [m.fp32(Fraction(1, 2)), m.fp32(-2)]
        p = m._program('_updateC', params, m._state())
        raw = struct.pack('<64I', *p[56+204:56+268])
        self.assertEqual(struct.unpack_from('<Q', raw, 240)[0], 0x3fe0000000000000)
        self.assertEqual(struct.unpack_from('<Q', raw, 232)[0], 0xc000000000000000)

    def test_raw_single_metadata_and_alias_tampering_is_rejected(self):
        row = next(r for r in self.rows if r['id'] == '7060-dense_prepared')
        for index in (56+204+61, 56+76, 56+269):
            altered = copy.deepcopy(row['input'])
            altered['parameter_bits'][index] = m.word(int(altered['parameter_bits'][index], 16)^1)
            with self.assertRaisesRegex(ValueError, 'ABI'):
                m.evaluate(altered)

    def test_fixed_corpus_and_all_unwritten_state(self):
        self.assertEqual(len(self.rows), 51)
        self.assertEqual(len({r['id'] for r in self.rows}), 51)
        self.assertEqual(sum(r['input']['kind']=='orbit' for r in self.rows), 17)
        registry = {('external', d[0], name) for name, d in m.FORMULAS.items()}
        for row in self.rows:
            validate_case(row['input'], registry)
            self.assertEqual(set(row['expected_state_bits']), set(STATE))
            self.assertEqual(row['expected_state_bits'], m.evaluate(row['input']))
            for lane in set(STATE)-set(row['write_lanes']):
                self.assertEqual(row['expected_state_bits'][lane], row['input']['input_state_bits'][lane], (row['id'], lane))
        for name in m.FORMULAS:
            self.assertEqual({r['input']['kind'] for r in self.rows if r['identity'][2]==name}, {'point', 'orbit'})

    def test_update_orbit_recurrence_mandel_and_julia(self):
        for variant, julia in [('orbit_mandel', False), ('orbit_julia', True)]:
            row = next(r for r in self.rows if r['id'] == '7055-'+variant)
            inp, out = row['input']['input_state_bits'], row['expected_state_bits']
            for i in range(3):
                initial = m.fraction(int(inp[f'original_j[{i}]' if julia else f'c[{i}]'], 16))
                coordinate = m.fraction(int(inp[f'c[{i}]'], 16))
                self.assertEqual(m.fraction(int(out[f'j[{i}]'], 16)), initial+4*coordinate)
                self.assertEqual(out[f'xyz[{i}]'], inp[f'c[{i}]'])
            self.assertEqual(out['xyz[3]'], '0x3f800000')
            self.assertEqual(out['j[3]'], inp['j[3]'])

    def test_cycle_orbits_and_strict_escape_keep_original_c(self):
        for name in m.FORMULAS:
            ident = m.FORMULAS[name][0]
            repeat = next(r for r in self.rows if r['id'] == f'{ident}-orbit_repeat')
            escape = next(r for r in self.rows if r['id'] == f'{ident}-orbit_escape')
            self.assertEqual(repeat['expected_state_bits']['iteration'], '0x00000004')
            self.assertEqual(escape['expected_state_bits']['iteration'], '0x00000001')
            for i in range(4):
                self.assertEqual(repeat['expected_state_bits'][f'c[{i}]'], repeat['input']['input_state_bits'][f'c[{i}]'])
        # Four applications of a three-cycle equal one application. This
        # detects state reset each iteration and in-place overwrites.
        for ident, lane in [(7060, 'xyz'), (7062, 'j')]:
            row = next(r for r in self.rows if r['id'] == f'{ident}-orbit_mandel')
            inp, out = row['input']['input_state_bits'], row['expected_state_bits']
            self.assertEqual([out[f'{lane}[{i}]'] for i in range(3)], [inp[f'c[{i}]'] for i in (1, 2, 0)])

    def test_source_abi_pins_and_dependency_line_endings(self):
        manifest = m.manifest()
        self.assertEqual(len(manifest['code_audit']), 3)
        self.assertEqual(set(manifest['abi_sources']), set(m.ABI_PINS))
        data = m.dependency_paths()[0].read_bytes().replace(b'\r\n', b'\n')
        for content in (data, data.replace(b'\n', b'\r\n')):
            self.assertEqual(m.primitive_identity(content), hashlib.sha256(content).hexdigest())
        with self.assertRaisesRegex(ValueError, 'dependency'):
            m.primitive_identity(data.replace(b'def add(', b'def altered_add(', 1))
        self.assertIn('Euler', ' '.join(manifest['model_limits']))


if __name__ == '__main__': unittest.main()
