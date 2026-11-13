"""Independent pre-device original semantics, finite-domain and packed ABI tests."""
import copy
from fractions import Fraction
import hashlib
from itertools import permutations
import struct
import unittest

import native_group03_geometry_reference as m
from gpu_pipeline_proofs import STATE
from native_formula_observer import validate_case

DENSE = list(map(m.fp32, (Fraction(1, 2), Fraction(1, 4), Fraction(-1, 8),
                        Fraction(-1, 4), 1, Fraction(1, 2),
                        Fraction(1, 8), Fraction(-1, 2), Fraction(1, 4))))
CYCLE = list(map(m.fp32, (0, 1, 0, 0, 0, 1, 1, 0, 0)))


def vector(state, lane='xyz'):
    return [m.fraction(state[f'{lane}[{i}]']) for i in range(3)]


def state_xyz(values):
    s = m._state()
    for i, value in enumerate(values): s[f'xyz[{i}]'] = m.fp32(value)
    return s


class OriginalGeometrySemantics(unittest.TestCase):
    def test_tetra_first_disabled_forces_second_half_even_when_both_flags_zero(self):
        s = state_xyz((-2, 1, -3))
        self.assertEqual(vector(m._point('_FoldingTetra3d', s, [0, 0])), [1, -2, -3])
        self.assertEqual(vector(m._point('_FoldingTetra3d', s, [1, 0])), [3, 2, 1])
        for flags in ([-1, 0], [-(1 << 31), 0], [16777217, 0]):
            self.assertEqual(vector(m._point('_FoldingTetra3d', s, flags)), [3, 2, 1])

    def test_tetra_equal_planes_fold_and_equal_sort_swaps_signed_zeros(self):
        s = state_xyz((0, 0, 0))
        out = m._point('_FoldingTetra3d', s, [1, 0])
        self.assertEqual([out[f'xyz[{i}]'] for i in range(3)], [m.NEG_ZERO, m.NEG_ZERO, 0])
        for i in range(3): s[f'xyz[{i}]'] = m.NEG_ZERO
        out = m._point('_FoldingTetra3d', s, [1, 0])
        self.assertEqual([out[f'xyz[{i}]'] for i in range(3)], [0, 0, m.NEG_ZERO])
        s = state_xyz((0, 0, 0)); s['xyz[2]'] = m.NEG_ZERO
        out = m._point('_FoldingTetra3d', s, [0, 0])
        self.assertEqual([out[f'xyz[{i}]'] for i in range(3)], [m.NEG_ZERO, 0, 0])

    def test_tetra_both_halves_have_sorted_positive_leaders_and_preserve_radius(self):
        for values in permutations((-3, 1, -2)):
            s = state_xyz(values)
            out = m._point('_FoldingTetra3d', s, [1, 1])
            self.assertEqual(vector(out), [3, 2, 1])
            self.assertEqual(m.radius(out), m.fp32(14))

    def test_addc_reads_mutable_j_and_preserves_every_other_state_lane(self):
        s = state_xyz((1, 2, 3))
        for i, value in enumerate((4, -2, 8)): s[f'j[{i}]'] = m.fp32(value)
        out = m._point('_addC', s, list(map(m.fp32, (2, -3, Fraction(1, 2)))))
        self.assertEqual(vector(out), [9, 8, 7])
        for lane in set(STATE)-{'xyz[0]', 'xyz[1]', 'xyz[2]', 'current_slot'}:
            self.assertEqual(out[lane], s[lane], lane)

    def test_rotated_fold_uses_transpose_and_original_vector_for_each_row(self):
        out = m._point('_RotatedFolding', state_xyz((2, 3, -4)), CYCLE+[m.ONE])
        self.assertEqual(vector(out), [2, -1, -4])
        out = m._point('_RotatedFolding', state_xyz((1, 2, 3)), DENSE+[m.fp32(Fraction(1, 4))])
        # Manually: M*x=(5/8,13/4,-1/8); folded X=-1/8; M^T*f follows.
        self.assertEqual(vector(out), [Fraction(-57, 64), Fraction(105, 32), Fraction(103, 64)])

    def test_rotated_fold_keeps_original_fsubp_then_fchs_zero_sign(self):
        out = m._point('_RotatedFolding', state_xyz((0, -2, -3)), m.prepare_zero_angles([0]*3)+[m.ONE])
        self.assertEqual(out['xyz[0]'], m.NEG_ZERO)
        self.assertEqual(vector(out)[1:], [-2, -3])
        for x, expected in ((-3, 1), (-1, -1), (Fraction(1, 2), Fraction(1, 2)), (1, 1), (3, -1)):
            out = m._point('_RotatedFolding', state_xyz((x, -2, -3)), m.prepare_zero_angles([0]*3)+[m.ONE])
            self.assertEqual(vector(out)[0], expected)

    def test_transform_shift_rotate_scale_w_derivative_and_conditional_mutable_j(self):
        for julia, flag in ((0, 0), (0, 1), (0, -1), (0, -(1 << 31)), (1, 1), (1, -1)):
            with self.subTest(julia=julia, flag=flag):
                s = state_xyz((1, 2, 3)); s.update(julia=julia)
                s['xyz[3]'], s['derivative[0]'] = m.fp32(7), m.fp32(3)
                p = list(map(m.fp32, (-2, 1, -1, 2)))+CYCLE+[flag]
                out = m._point('_Transform', s, p)
                self.assertEqual(vector(out), [-2, -10, -4])
                self.assertEqual(m.fraction(out['xyz[3]']), -14)
                self.assertEqual(m.fraction(out['derivative[0]']), -6)
                self.assertEqual(vector(out, 'j'), [-2, -10, -4] if flag and not julia else vector(s, 'j'))
                changed = {'xyz[0]', 'xyz[1]', 'xyz[2]', 'xyz[3]', 'derivative[0]', 'current_slot'}
                if flag and not julia: changed |= {'j[0]', 'j[1]', 'j[2]'}
                for lane in set(STATE)-changed: self.assertEqual(out[lane], s[lane], lane)

    def test_zero_angle_preparation_is_signed_not_idealized_nonzero_rotation(self):
        self.assertEqual(m.prepare_zero_angles([0]*3), [m.ONE, m.NEG_ZERO, 0, 0, m.ONE, m.NEG_ZERO, 0, 0, m.ONE])
        self.assertEqual(m.prepare_zero_angles([m.NEG_ZERO, 0, m.NEG_ZERO]),
                         [m.ONE, 0, 0, m.NEG_ZERO, m.ONE, 0, 0, m.NEG_ZERO, m.ONE])
        with self.assertRaisesRegex(m.ScopeError, 'Euler'): m.prepare_zero_angles([0, 0, m.fp32(90)])


class ArithmeticAndABI(unittest.TestCase):
    def test_rounded_products_and_reassociated_partial_sums_fail_closed(self):
        with self.assertRaises(m.ScopeError): m.exact_mul(m.fp32(Fraction(13, 10)), m.fp32(Fraction(7, 10)))
        with self.assertRaises(m.ScopeError): m.exact_sum(list(map(m.fp32, (1 << 25, -(1 << 25), 1))))
        # Original left grouping: (1+1)+2^24 is exact, but 1+2^24 is not.
        with self.assertRaises(m.ScopeError): m.radius(state_xyz((1, 1, 4096)))
        with self.assertRaises(m.ScopeError): m.exact_mul(1, m.ONE)
        with self.assertRaises(ValueError): m.exact_mul(0x7f800000, m.ONE)
        with self.assertRaises(ValueError): m.exact_mul(0x7f7fffff, m.fp32(2))

    def test_typed_int32_preserves_full_value_and_original_packed_offsets(self):
        for name, values, index, offset in (
                ('_FoldingTetra3d', [16777217, -(1 << 31)], 0, -12),
                ('_Transform', [m.ONE, 0, 0, 0]+CYCLE+[(1 << 31)-1], 13, -116)):
            p = m._program(name, values, m._state())
            raw = struct.pack('<64I', *p[260:324])
            self.assertEqual(struct.unpack_from('<i', raw, 256+offset)[0], values[index])
            self.assertEqual(m._parameters(name, p), values)
            original = p[132+2*index] | p[133+2*index] << 32
            self.assertEqual(m.fraction64(original), values[index])
            self.assertNotEqual(m.fraction(p[68+index]), values[index])
        with self.assertRaisesRegex(ValueError, 'int32'): m._program('_FoldingTetra3d', [1 << 31, 0], m._state())

    def test_rotated_single_matrix_then_unaligned_double_and_transform_double_matrix(self):
        params = DENSE+[m.fp32(Fraction(1, 4))]
        p = m._program('_RotatedFolding', params, m._state())
        raw = struct.pack('<64I', *p[260:324])
        for i in range(9): self.assertEqual(struct.unpack_from('<I', raw, 244-4*i)[0], params[i])
        self.assertEqual(struct.unpack_from('<Q', raw, 204)[0], 0x3fd0000000000000)
        params = [m.ONE, 0, 0, 0]+DENSE+[-1]
        p = m._program('_Transform', params, m._state())
        raw = struct.pack('<64I', *p[260:324])
        for i in range(9): self.assertEqual(struct.unpack_from('<Q', raw, 208-8*i)[0], m.wide32(DENSE[i]))
        self.assertEqual(struct.unpack_from('<i', raw, 140)[0], -1)
        self.assertEqual(struct.unpack_from('<Q', raw, 248)[0], 0x3fe0000000000000)


class CorpusAndCaller(unittest.TestCase):
    @classmethod
    def setUpClass(cls): cls.rows = m.build_cases()

    def test_fixed_four_formula_corpus_all47_state_and_safe_ids(self):
        self.assertEqual(len(self.rows), 97)
        self.assertEqual(len({r['id'] for r in self.rows}), 97)
        self.assertEqual(sum(r['input']['kind']=='orbit' for r in self.rows), 22)
        registry = {('external', d[0], name) for name, d in m.FORMULAS.items()}
        for row in self.rows:
            self.assertRegex(row['id'], r'^[a-zA-Z0-9_-]+$')
            validate_case(row['input'], registry)
            self.assertEqual(set(row['expected_state_bits']), set(STATE))
            self.assertEqual(row['expected_state_bits'], m.evaluate(row['input']))
            for lane in set(STATE)-set(row['write_lanes']):
                self.assertEqual(row['expected_state_bits'][lane], row['input']['input_state_bits'][lane], (row['id'], lane))
        for name in m.FORMULAS:
            self.assertEqual({r['input']['kind'] for r in self.rows if r['identity'][2]==name}, {'point', 'orbit'})

    def test_parameter_metadata_raw_and_alias_tampering_is_rejected(self):
        row = next(r for r in self.rows if r['id']=='8481-dense_prepared')
        for index in (60, 61, 62, 132, 260+61, 324+1):
            altered = copy.deepcopy(row['input'])
            altered['parameter_bits'][index] = m.word(int(altered['parameter_bits'][index], 16)^1)
            with self.assertRaisesRegex(ValueError, 'ABI'): m.evaluate(altered)
        row = next(r for r in self.rows if r['id']=='7096-raw_integer_flags_2')
        altered = copy.deepcopy(row['input'])
        # Binary64 metadata says 1.5, not an integer, even if raw flag remains.
        altered['parameter_bits'][132:134] = ['0x00000000', '0x3ff80000']
        with self.assertRaisesRegex(ValueError, 'int32'): m.evaluate(altered)

    def test_addc_orbit_mandel_and_julia_recurrence_without_hidden_c_add(self):
        for variant, julia in (('orbit_mandel', False), ('orbit_julia', True)):
            row = next(r for r in self.rows if r['id']=='7280-'+variant)
            inp, out = row['input']['input_state_bits'], row['expected_state_bits']
            for i in range(3):
                coordinate = m.fraction(int(inp[f'c[{i}]'], 16))
                constant = m.fraction(int(inp[f'original_j[{i}]' if julia else f'c[{i}]'], 16))
                self.assertEqual(m.fraction(int(out[f'xyz[{i}]'], 16)), coordinate+4*constant)
            self.assertEqual(out['iteration'], '0x00000004')

    def test_transform_orbit_scales_w_and_derivative_each_step_and_keeps_original_c(self):
        for variant, julia in (('orbit_mandel', False), ('orbit_julia', True)):
            row = next(r for r in self.rows if r['id']=='8896-'+variant)
            inp, out = row['input']['input_state_bits'], row['expected_state_bits']
            for i, src in enumerate((1, 2, 0)):
                wanted = m.fraction(int(inp[f'c[{src}]'], 16))/16
                self.assertEqual(m.fraction(int(out[f'xyz[{i}]'], 16)), wanted)
                self.assertEqual(out[f'j[{i}]'], inp[f'original_j[{i}]'] if julia else out[f'xyz[{i}]'])
            self.assertEqual(out['xyz[3]'], m.word(m.fp32(Fraction(1, 16))))
            self.assertEqual(m.fraction(int(out['derivative[0]'], 16)), m.fraction(int(inp['derivative[0]'], 16))/16)
            for i in range(4): self.assertEqual(out[f'c[{i}]'], inp[f'c[{i}]'])

    def test_strict_escape_equal_boundary_and_positive_slot_repetition(self):
        for name, descriptor in m.FORMULAS.items():
            for variant, iterations in (('orbit_boundary', 4), ('orbit_escape', 1), ('orbit_repeat', 4)):
                row = next(r for r in self.rows if r['id']==f'{descriptor[0]}-{variant}')
                self.assertEqual(int(row['expected_state_bits']['iteration'], 16), iterations, row['id'])
            row = next(r for r in self.rows if r['id']==f'{descriptor[0]}-orbit_boundary')
            self.assertEqual(row['expected_state_bits']['rout'], row['input']['input_state_bits']['rstop'])
        row = next(r for r in self.rows if r['id']=='8896-orbit_add_shift_copy')
        self.assertEqual(m.fraction(int(row['expected_state_bits']['xyz[0]'], 16)), Fraction(25, 8))

    def test_work_and_arithmetic_scope_fail_closed(self):
        base = next(r for r in self.rows if r['id']=='7280-orbit_mandel')['input']
        for lane, value in (('max_iterations', 9), ('hybrid_counts[0]', 0), ('inside', 1), ('calc_smooth', 1)):
            altered = copy.deepcopy(base); altered['input_state_bits'][lane] = m.word(value)
            with self.assertRaises(m.ScopeError): m.evaluate(altered)
        for value in (1, 0x7f800000, 0x7fc00000):
            altered = copy.deepcopy(base); altered['input_state_bits']['c[0]'] = m.word(value)
            with self.assertRaises(ValueError): m.evaluate(altered)

    def test_zero_scale_transform_point_is_finite_but_analytic_orbit_is_rejected(self):
        payload = copy.deepcopy(next(r for r in self.rows if r['id']=='8896-orbit_mandel')['input'])
        state = {k: int(v, 16) for k, v in payload['input_state_bits'].items()}
        params = m._parameters('_Transform', [int(v, 16) for v in payload['parameter_bits']])
        params[0] = 0
        payload['parameter_bits'] = [m.word(v) for v in m._program('_Transform', params, state)]
        with self.assertRaisesRegex(m.ScopeError, 'zero W'):
            m.evaluate(payload)
        payload['kind'] = 'point'
        point = m.evaluate(payload)
        self.assertEqual(int(point['xyz[3]'], 16) & 0x7fffffff, 0)
        for lane in STATE[:33]: m.fraction(int(point[lane], 16))

    def test_original_code_source_and_primitive_identity_are_pinned(self):
        manifest = m.manifest()
        self.assertEqual(len(manifest['code_audit']), 4)
        self.assertIn('mb3d/CustomFormulas.pas', manifest['semantic_sources'])
        self.assertIn('mb3d/DivUtils.pas', manifest['semantic_sources'])
        self.assertEqual(set(manifest['abi_sources']), set(m.ABI_PINS))
        data = m.dependency_paths()[0].read_bytes().replace(b'\r\n', b'\n')
        for content in (data, data.replace(b'\n', b'\r\n')):
            self.assertEqual(m.primitive_identity(content), hashlib.sha256(content).hexdigest())
        with self.assertRaisesRegex(ValueError, 'dependency'):
            m.primitive_identity(data.replace(b'def add(', b'def altered_add(', 1))
        self.assertIn('arbitrary Euler', ' '.join(manifest['model_limits']))


if __name__ == '__main__': unittest.main()
