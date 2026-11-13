"""Pre-device original-CODE and transport checks; no executable or GPU needed."""
from copy import deepcopy
from fractions import Fraction
import hashlib
import struct
import unittest
from unittest.mock import patch
import native_group04_polynomial_reference as m


class PolynomialReferenceTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.cases = m.build_cases()
        cls.by_variant = {(c['identity'][2], c['variant']): c for c in cls.cases}

    def payload(self, name, xyz, params, j=(7, -8, 9), *, kind='point'):
        state = m._state()
        for prefix, values in (('xyz', xyz), ('j', j)):
            for i, value in enumerate(values): state[f'{prefix}[{i}]'] = m.fp32(value)
        return {'identity': ['external', m.FORMULAS[name][0], name], 'kind': kind,
                'input_state_bits': {k: m.word(v) for k, v in state.items()},
                'parameter_bits': [m.word(v) for v in m._program(name, list(map(m.fp32, params)), state)]}

    def xyz(self, payload):
        out = m.evaluate(payload)
        return tuple(m.fraction(int(out[f'xyz[{i}]'], 16)) for i in range(3))

    def case(self, name, variant): return self.by_variant[name, variant]

    def test_original_source_and_frozen_primitive_closure(self):
        manifest = m.manifest()
        self.assertEqual(set(manifest['code_audit']), set(m.FORMULAS))
        self.assertIn('mb3d/CustomFormulas.pas', manifest['semantic_sources'])
        self.assertIn('mb3d/formulas.pas', manifest['semantic_sources'])
        self.assertEqual(len(m.dependency_paths()), 1)
        self.assertEqual(m.primitive_identity(m.dependency_paths()[0].read_bytes()),
                         manifest['dependencies']['comparison/tools/native_formula_reference.py'])
        with patch.object(m, 'SOURCE_PINS', {'mb3d/formulas.pas': '0'*64}):
            with self.assertRaisesRegex(ValueError, 'pinned original'): m.manifest()

    def test_primitive_line_endings_and_semantic_change(self):
        original = m.dependency_paths()[0].read_bytes().replace(b'\r\n', b'\n')
        windows = original.replace(b'\n', b'\r\n')
        self.assertEqual(m.primitive_identity(windows), hashlib.sha256(windows).hexdigest())
        with self.assertRaisesRegex(ValueError, 'frozen arithmetic'): m.primitive_identity(original+b'changed')

    def test_complete_fixed_corpus_and_all47_lanes(self):
        self.assertEqual(len(self.cases), 86)
        self.assertEqual(sum(c['input']['kind']=='orbit' for c in self.cases), 20)
        self.assertEqual(len({c['id'] for c in self.cases}), 86)
        for c in self.cases:
            self.assertEqual(set(c['expected_state_bits']), set(m.STATE))
            self.assertEqual(len(c['input']['parameter_bits']), 2048)
            self.assertEqual(c['expected_state_bits'], m.evaluate(c['input']))
            if c['input']['kind']=='point':
                for lane in set(m.STATE)-{'xyz[0]', 'xyz[1]', 'xyz[2]', 'current_slot'}:
                    self.assertEqual(c['expected_state_bits'][lane], c['input']['input_state_bits'][lane], (c['id'], lane))

    def test_authored_double_ABI_and_no_synthetic_J_parameters(self):
        for name, count in (('PseudoXDB', 1), ('IdesFormula', 5), ('Quadrat3D', 5)):
            payload = self.case(name, 'asymmetric_sentinel_state')['input']
            words = [int(v, 16) for v in payload['parameter_bits']]
            self.assertEqual(words[1], 0)  # Hybrid3D, not analytic3D.
            self.assertEqual(words[6], 0)
            self.assertEqual(words[56:63], [1, 1, m.FORMULAS[name][0], 0, count, 0, int(name=='Quadrat3D')])
            raw = struct.pack('<64I', *words[260:324])
            self.assertEqual(struct.unpack_from('<d', raw, 248)[0], .5)
            for i in range(count):
                value = struct.unpack_from('<d', raw, 240-8*i)[0]
                original = struct.unpack('<d', struct.pack('<2I', *words[132+2*i:134+2*i]))[0]
                self.assertEqual(value, original)
                self.assertEqual(Fraction(value), m.fraction(words[68+i]))
            self.assertEqual(words[68+count:132], [0]*(64-count))
            # No invented PVar storage for the caller's mutable J.
            self.assertEqual(raw[:248-8*count], bytes(248-8*count))

    def test_program_aliases_descriptor_and_metadata_fail_closed(self):
        original = self.case('Quadrat3D', 'asymmetric_sentinel_state')['input']
        for word_index in (1, 6, 58, 60, 62, 132, 320, 324, 356):
            payload = deepcopy(original)
            payload['parameter_bits'][word_index] = m.word(int(payload['parameter_bits'][word_index], 16)^1)
            with self.assertRaises(ValueError, msg=str(word_index)): m.evaluate(payload)
        payload = deepcopy(original); payload['identity'][1] += 1
        with self.assertRaises(ValueError): m.evaluate(payload)
        payload = deepcopy(original); payload['parameter_bits'].pop()
        with self.assertRaises(ValueError): m.evaluate(payload)
        payload = deepcopy(original); payload['input_state_bits'].pop('j[3]')
        with self.assertRaises(ValueError): m.evaluate(payload)

    def test_pseudoxdb_original_stack_sign_absZ_and_pre_store_inputs(self):
        self.assertEqual(self.xyz(self.payload('PseudoXDB', (3, 4, 5), [1])), (-7, 40, 9))
        self.assertEqual(self.xyz(self.payload('PseudoXDB', (3, 4, -5), [1])), (-7, 40, 9))
        self.assertEqual(self.xyz(self.payload('PseudoXDB', (3, 4, 10), [-2])), (-21, 88, -141))
        # Original Y store cannot replace input Y in X's retained squared term.
        self.assertNotEqual(-7, 2*(3*3-40*40)+7)

    def test_ides_original_monomials_and_parameter_order(self):
        self.assertEqual(self.xyz(self.payload('IdesFormula', (2, -3, 4), [1, 2, 1, Fraction(1, 2), Fraction(1, 2)])),
                         (Fraction(-3, 2), -56, Fraction(37, 2)))
        self.assertEqual(self.xyz(self.payload('IdesFormula', (2, -3, 4), [2, -1, -2, 3, -4])), (-60, 16, 29))
        self.assertEqual(self.xyz(self.payload('IdesFormula', (2, -3, 4), [0]*5)), (7, -8, 9))

    def test_quadrat_positive_branch_root_division_and_parameter_order(self):
        self.assertEqual(self.xyz(self.payload('Quadrat3D', (3, 4, 10), [1, 2, -2, 0, 1])), (28, -80, -91))
        self.assertEqual(self.xyz(self.payload('Quadrat3D', (3, 0, 2), [1, 2, -2, 7, 1])),
                         (Fraction(55, 4), -8, -7))

    def test_quadrat_negative_and_zero_H_have_distinct_outputs(self):
        self.assertEqual(self.xyz(self.payload('Quadrat3D', (1, 0, 0), [1, 2, -2, -2, 1])), (0, 0, 0))
        self.assertEqual(self.xyz(self.payload('Quadrat3D', (1, 0, 0), [1, 2, -2, -1, 1])), (7, -8, 9))
        positive = self.case('Quadrat3D', 'H_branch_positive_ulp')
        negative = self.case('Quadrat3D', 'H_branch_negative_ulp')
        self.assertEqual(self.xyz(negative['input']), (0, 0, 0))
        self.assertEqual(self.xyz(positive['input']), (Fraction(7, 4), -1, Fraction(5, 4)))
        self.assertEqual(m.exact_sqrt(m.fp32(Fraction(1, 1<<24))), m.fp32(Fraction(1, 1<<12)))

    def test_signed_zeros_are_operation_results_not_canonicalized(self):
        ide = self.case('IdesFormula', 'negative_zero_product_and_addition')['expected_state_bits']
        self.assertEqual([ide[f'xyz[{i}]'] for i in range(3)], ['0x00000000', '0x80000000', '0x00000000'])
        for h, expected in ((-1, '0x00000000'), (0, '0x80000000'), (1, '0x00000000')):
            out = self.case('Quadrat3D', f'zero_sign_H_{h}')['expected_state_bits']
            self.assertEqual([out[f'xyz[{i}]'] for i in range(3)], [expected]*3)
        self.assertEqual(m.exact_divide(m.NEG_ZERO, m.ONE), m.NEG_ZERO)
        self.assertEqual(m.exact_divide(0, m.ONE|m.NEG_ZERO), m.NEG_ZERO)
        self.assertEqual(m.exact_sqrt(m.NEG_ZERO), m.NEG_ZERO)

    def test_mutable_J_is_used_not_C_or_originalJ(self):
        for name in m.FORMULAS:
            baseline = self.case(name, 'asymmetric_sentinel_state')
            changed = self.case(name, 'mutable_J_not_C_or_originalJ')
            for i in range(3):
                lane = f'xyz[{i}]'; j = f'j[{i}]'
                delta = m.fraction(int(changed['expected_state_bits'][lane],16))-m.fraction(int(baseline['expected_state_bits'][lane],16))
                expected = m.fraction(int(changed['input']['input_state_bits'][j],16))-m.fraction(int(baseline['input']['input_state_bits'][j],16))
                self.assertEqual(delta, expected)

    def test_nonanalytic_mandel_orbits_have_hand_derived_results(self):
        for name, variant, xyz, rout, rold, iterations in (
                ('PseudoXDB','orbit_mandel_four',(19,0,80),6761,82,4),
                ('Quadrat3D','orbit_mandel_four',(677,0,0),458329,676,4),
                ('IdesFormula','orbit_mandel_dyadic_two',(Fraction(135,128),0,Fraction(-17,64)),Fraction(19381,16384),Fraction(37,64),2)):
            c = self.case(name, variant); out = c['expected_state_bits']
            self.assertEqual(self.xyz(c['input']), xyz)
            self.assertEqual(out['rout'], m.word(m.fp32(rout)))
            self.assertEqual(out['rold'], m.word(m.fp32(rold)))
            self.assertEqual(out['iteration'], m.word(iterations))
            self.assertEqual(out['xyz[3]'], '0x00000000')
            self.assertEqual(out['first'], '0x00000000')
            for i in range(3): self.assertEqual(out[f'j[{i}]'], c['input']['input_state_bits'][f'c[{i}]'])

    def test_compiled_original_caller_maximum_precedes_bailout(self):
        # Hand-derived PseudoXDB: (1,0,0) -> (1,0,-1) -> (3,0,0).
        for maximum, stop, expected_xmm0, expected_rout, iterations in (
                (1, 64, 2, 2, 1), (2, 64, 9, 9, 2),
                (4, 1, 1, 2, 1), (1, 1, 2, 2, 1), (2, 2, 9, 9, 2)):
            s = m._state()
            for i, value in enumerate((1, 0, 0)): s[f'c[{i}]'] = m.fp32(value)
            s.update(julia=0, max_iterations=maximum, rstop=m.fp32(stop))
            out = m._orbit('PseudoXDB', s, [m.ONE])
            self.assertEqual(out['iteration'], iterations)
            self.assertEqual([out[f'sse[{i}]'] for i in range(4)],
                             [m.fp32(expected_xmm0), 0, m.fp32(expected_rout), 0])
        for c in self.cases:
            if c['input']['kind'] == 'point':
                for i in range(4):
                    self.assertEqual(c['expected_state_bits'][f'sse[{i}]'], c['input']['input_state_bits'][f'sse[{i}]'])

    def test_julia_positive_zero_and_negative_branches_in_orbits(self):
        c = self.case('Quadrat3D', 'orbit_julia_pythagorean_two')
        self.assertEqual(self.xyz(c['input']), (-527,-336,0))
        self.assertEqual(c['expected_state_bits']['rout'], m.word(m.fp32(390625)))
        self.assertEqual(self.xyz(self.case('Quadrat3D', 'orbit_positive_then_zero_H')['input']), (0,0,0))
        self.assertEqual(self.xyz(self.case('Quadrat3D', 'orbit_negative_H_ignores_J')['input']), (0,0,0))
        self.assertEqual(self.xyz(self.case('Quadrat3D', 'orbit_zero_H_mutable_J')['input']), (0,1,0))
        for c in self.cases:
            if c['input']['kind']=='orbit' and c['input']['input_state_bits']['julia']=='0x00000001':
                for i in range(3):
                    self.assertEqual(c['expected_state_bits'][f'j[{i}]'], c['input']['input_state_bits'][f'original_j[{i}]'])

    def test_strict_escape_and_repeat_cursor(self):
        for name in m.FORMULAS:
            self.assertEqual(self.case(name, 'orbit_strict_escape')['expected_state_bits']['iteration'], '0x00000001')
            self.assertEqual(self.case(name, 'orbit_equality_continues')['expected_state_bits']['iteration'], '0x00000002')
        c = self.case('PseudoXDB', 'orbit_negative_multiplier')
        self.assertEqual(c['expected_state_bits']['tmp'], '0x00000002')  # 4 iterations with count3.
        self.assertEqual(self.xyz(c['input']), (19,0,-80))

    def test_invalid_numeric_domains_never_become_successful_fixtures(self):
        cases = [('PseudoXDB',(0,0,1),[1]),
                 ('PseudoXDB',(1,1,1),[1]),
                 ('PseudoXDB',(3,4,1),[1]),
                 ('Quadrat3D',(1,1,0),[1,2,-2,0,1]),
                 ('Quadrat3D',(3,4,1),[1,2,-2,0,1])]
        for name, xyz, params in cases:
            with self.assertRaises(m.ScopeError, msg=(name,xyz)): m.evaluate(self.payload(name,xyz,params))
        with self.assertRaises(m.ScopeError): m.exact_sqrt(m.fp32(-1))
        with self.assertRaises(m.ScopeError): m.exact_divide(m.ONE, m.fp32(3))
        with self.assertRaises(m.ScopeError): m.normal_or_zero(1)
        with self.assertRaises(ValueError): m.normal_or_zero(0x7f800000)
        with self.assertRaises(ValueError): m.exact_mul(m.fp32(1<<100), m.fp32(1<<100))

    def test_FMA_and_radius_guards_check_partial_subsets(self):
        with self.assertRaises(m.ScopeError): m.exact_sum([m.fp32(1),m.fp32(1),m.fp32(1<<24)])
        s = m._state()
        for i,v in enumerate((1,1,4096)): s[f'xyz[{i}]']=m.fp32(v)
        with self.assertRaises(m.ScopeError): m.radius(s)
        # Original grouping can multiply by1 after an exact square, while an
        # alternative subset is nonrepresentable; the product guard rejects it.
        a=m.fp32(Fraction(4097,4096))
        with self.assertRaises(m.ScopeError): m.exact_product([a,a,m.fp32(2)])

    def test_caller_scope_and_work_limits_fail_closed(self):
        original = self.case('IdesFormula', 'orbit_julia_three')['input']
        for lane, value in (('inside',1),('calc_smooth',1),('max_iterations',0),('max_iterations',9),('hybrid_counts[0]',0),('rstop',0)):
            p=deepcopy(original);p['input_state_bits'][lane]=m.word(value)
            with self.assertRaises(ValueError):m.evaluate(p)
        p=deepcopy(original);p['input_state_bits']['xyz[0]']='0x7fc00001'
        with self.assertRaises(ValueError):m.evaluate(p)
        p=deepcopy(original);p['extra']=True
        with self.assertRaises(ValueError):m.evaluate(p)


if __name__=='__main__': unittest.main()
