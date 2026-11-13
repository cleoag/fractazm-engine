"""Independent rational/ABI controls; no GPU, renderer or learned golden values."""
import copy
from fractions import Fraction
import hashlib
import unittest
import native_reciprocal_reference as r
from gpu_pipeline_proofs import STATE
from native_formula_observer import validate_case


class Arithmetic(unittest.TestCase):
    def test_division_rounds_and_preserves_zero_sign(self):
        self.assertEqual(r.divide(r.ONE,r.fp32(3)),0x3eaaaaab)
        self.assertEqual(r.divide(r.fp32(-1),r.fp32(3)),0xbeaaaaab)
        self.assertEqual(r.divide(0x80000000,r.fp32(2)),0x80000000)
        self.assertEqual(r.divide(0,r.fp32(-2)),0x80000000)
        self.assertEqual(r.divide(0x80000000,r.fp32(-2)),0)
        self.assertEqual(r.divide(0x3f800001,r.fp32(2)),0x3f000001)

    def test_division_singular_and_range_classifications(self):
        for numerator,denominator,classification in [(r.ONE,0,'SINGULAR_DIVISION'),
                (0,0x80000000,'SINGULAR_DIVISION'),(0x7f7fffff,0x00800000,'FP32_RANGE')]:
            with self.assertRaises(r.DomainError) as caught:r.divide(numerator,denominator)
            self.assertEqual(caught.exception.classification,classification)

    def test_binary64_preparation_rounding_is_independent(self):
        self.assertEqual(r.binary64(Fraction(1,3)),0x3fd5555555555555)
        self.assertEqual(r.binary64(Fraction(1,5)),0x3fc999999999999a)
        self.assertEqual(r.binary64(1+Fraction(1,1<<53)),0x3ff0000000000000)
        self.assertEqual(r.binary64(1+Fraction(3,1<<53)),0x3ff0000000000002)
        self.assertEqual(r.binary64(Fraction(1,10**40)),r.DRECI_FLOOR64)


class FormulaSemantics(unittest.TestCase):
    def test_sphere_consumes_stored_radius_and_keeps_it(self):
        s=r._state();s['rout']=r.fp32(4)
        params=r.prepare('_SphereFolding1',[r.ONE])[1]
        out=r._point('_SphereFolding1',s,params)
        self.assertEqual([r.fraction(out[f'xyz[{i}]']) for i in range(3)],
                         [Fraction(-3,16),Fraction(1,4),Fraction(-5,16)])
        self.assertEqual(out['rout'],s['rout'])
        # Radius is intentionally unrelated to the squared coordinate norm.
        self.assertNotEqual(r.fraction(s['rout']),sum(r.fraction(s[f'xyz[{i}]'])**2 for i in range(3)))

    def test_sphere_boundaries_zero_noop_and_singular_zero(self):
        s=r._state();params=r.prepare('_SphereFolding1',[r.ONE])[1]
        for radius in (0,0x80000000,r.ONE,r.fp32(-1)):
            s['rout']=radius;expected=dict(s);expected['current_slot']=0
            self.assertEqual(r._point('_SphereFolding1',s,params),expected)
        for R in (0,r.fp32(-1)):
            s['rout']=0
            with self.assertRaises(r.DomainError) as caught:
                r._point('_SphereFolding1',s,r.prepare('_SphereFolding1',[R])[1])
            self.assertEqual(caught.exception.classification,'SINGULAR_DIVISION')
        s['rout']=r.fp32(4);s['xyz[0]'],s['xyz[1]']=0x80000000,0
        out=r._point('_SphereFolding1',s,params)
        self.assertEqual((out['xyz[0]'],out['xyz[1]']),(0,0x80000000))

    def test_x2_xor_sign_includes_negative_denominator(self):
        s=r._state()
        for x,limiter,expected in [(0,r.fp32(2),0x3f000000),(0x80000000,r.fp32(2),0xbf000000),
                (0,r.fp32(-2),0xbf000000),(0x80000000,r.fp32(-2),0x3f000000),
                (0x3f7fffff,r.fp32(-1),0xcb800000),(0x3f800001,r.fp32(-1),0x4b000000)]:
            s['xyz[0]']=x
            self.assertEqual(r._point('_reciprocalX2',s,[limiter])['xyz[0]'],expected)
        s['xyz[0]']=r.ONE
        with self.assertRaises(r.DomainError) as caught:r._point('_reciprocalX2',s,[r.fp32(-1)])
        self.assertEqual(caught.exception.classification,'SINGULAR_DIVISION')

    def test_x3b_y3b_terms_axis_and_original_sign(self):
        params=r.prepare('_reciprocalX3b',[r.fp32(2),r.fp32(4),r.ONE,r.fp32(-1)])[1]
        for name,lane in [('_reciprocalX3b','xyz[0]'),('_reciprocalY3b','xyz[1]')]:
            for value,expected in [(2,Fraction(3,8)),(-2,Fraction(-3,8))]:
                s=r._state();s[lane]=r.fp32(value);out=r._point(name,s,params)
                self.assertEqual(r.fraction(out[lane]),expected)
                for untouched in set(STATE)-{lane,'current_slot'}:self.assertEqual(out[untouched],s[untouched])
            s=r._state();s[lane]=0x80000000
            self.assertEqual(r._point(name,s,params)[lane],0x80000000)

    def test_square_first_controls_distinguish_final_results(self):
        # Fixed inputs from original-code operation review, chosen before GPU.
        params=r.prepare('_reciprocalX3b',[r.ONE,r.fp32(Fraction(1,2)),0,r.fp32(Fraction(7,10))])[1]
        value=r.fp32(Fraction(13,10));s=r._state();s['xyz[0]']=value
        original=r._point('_reciprocalX3b',s,params)['xyz[0]']
        self.assertEqual(original,0x3fb3f201)
        # Deliberate wrong association mimics the diagnosed translation risk.
        wrong_product=r.mul(r.mul(params[5],value),value)
        wrong_term=r.sub(params[3],r.divide(r.ONE,r.add(r.absolute(wrong_product),params[2])))
        self.assertEqual(wrong_term,0x3fb3f202)
        self.assertNotEqual(original,wrong_term)

    def test_intermediate_overflow_not_erased_by_later_small_scale(self):
        s=r._state();s['xyz[0]']=r.fp32(1<<80)
        params=r.prepare('_reciprocalX3b',[r.ONE,r.ONE,0,r.fp32(Fraction(1,1<<100))])[1]
        with self.assertRaises(r.DomainError) as caught:r._point('_reciprocalX3b',s,params)
        self.assertEqual(caught.exception.classification,'FP32_RANGE')


class PreparationAndCorpus(unittest.TestCase):
    @classmethod
    def setUpClass(cls):cls.cases=r.build_cases()

    def test_dreci2_preserves_prepared_binary64_then_narrows(self):
        original,prepared=r.prepare('_reciprocalX3b',[r.fp32(3),r.fp32(5),r.ONE,r.ONE])
        self.assertEqual(original[1],0x3fd5555555555555)
        self.assertEqual(original[3],0x3fc999999999999a)
        self.assertEqual((prepared[1],prepared[3]),(0x3eaaaaab,0x3e4ccccd))
        p=r._program('_reciprocalX3b',[r.fp32(3),r.fp32(5),r.ONE,r.ONE],r._state());base=56
        self.assertEqual(p[base+78]|p[base+79]<<32,original[1])
        self.assertEqual(p[base+204+58]|p[base+204+59]<<32,original[1])
        self.assertEqual(p[base+12+1],prepared[1])

    def test_rejected_preparation_is_not_a_finite_pass(self):
        for limiter in (0,r.fp32(-1),r.fp32(Fraction(1,1<<128))):
            with self.assertRaises(r.DomainError) as caught:
                r.prepare('_reciprocalX3b',[limiter,r.ONE,r.ONE,r.ONE])
            self.assertEqual(caught.exception.classification,'PARAMETER_RANGE')
        with self.assertRaises(r.DomainError) as caught:r.prepare('_SphereFolding1',[0x7f7fffff])
        self.assertEqual(caught.exception.classification,'PARAMETER_RANGE')
        with self.assertRaises(r.DomainError) as caught:r.prepare('_reciprocalX2',[0x7f800000])
        self.assertEqual(caught.exception.classification,'NONFINITE_INPUT')

    def test_relationship_tampering_is_rejected_even_if_float_unchanged(self):
        row=next(c for c in self.cases if c['id']=='4812-prepared_non_dyadic')
        for index in (69,56+78,56+204+58):
            altered=copy.deepcopy(row['input'])
            altered['parameter_bits'][index]=r.word(int(altered['parameter_bits'][index],16)^1)
            with self.assertRaises(ValueError):r.evaluate(altered)

    def test_corpus_is_complete_finite_and_preserves_unwritten_state(self):
        self.assertEqual(len(self.cases),75)
        self.assertEqual(len({c['id'] for c in self.cases}),75)
        registry={('external',v[0],name) for name,v in r.FORMULAS.items()}
        for c in self.cases:
            validate_case(c['input'],registry)
            self.assertEqual(c['expected_state_bits'],r.evaluate(c['input']))
            self.assertEqual(set(c['expected_state_bits']),set(STATE))
            for lane in set(STATE)-set(c['write_lanes']):
                self.assertEqual(c['expected_state_bits'][lane],c['input']['input_state_bits'][lane],(c['id'],lane))
        for name in r.FORMULAS:
            self.assertEqual({c['input']['kind'] for c in self.cases if c['identity'][2]==name},{'point','orbit'})
        for ident in (4812,5823):
            controls=[c for c in self.cases if c['id'].startswith(f'{ident}-square_order_')]
            self.assertEqual(len(controls),6)

    def test_orbit_cycle_collapse_and_strict_radius_equality(self):
        for name in r.FORMULAS:
            ident=r.FORMULAS[name][0]
            row=next(c for c in self.cases if c['id']==f'{ident}-orbit_repeat')
            self.assertEqual(row['expected_state_bits']['iteration'],'0x00000004')
            row=next(c for c in self.cases if c['id']==f'{ident}-orbit_escape')
            self.assertEqual(row['expected_state_bits']['iteration'],'0x00000001')
        cycle=next(c for c in self.cases if c['id']=='7007-orbit_two_cycle')
        self.assertEqual(cycle['expected_state_bits']['xyz[0]'],'0xc0000000')
        collapse=next(c for c in self.cases if c['id']=='6030-orbit_collapse_to_zero')
        self.assertEqual(collapse['expected_state_bits']['rout'],'0x00000000')
        self.assertEqual(collapse['expected_state_bits']['iteration'],'0x00000004')

    def test_provenance_line_endings_and_actual_dependency_bytes(self):
        m=r.manifest();self.assertEqual(len(m['code_audit']),4)
        data=r.dependency_paths()[0].read_bytes().replace(b'\r\n',b'\n');crlf=data.replace(b'\n',b'\r\n')
        self.assertEqual(r.primitive_identity(data),hashlib.sha256(data).hexdigest())
        self.assertEqual(r.primitive_identity(crlf),hashlib.sha256(crlf).hexdigest())
        changed=data.replace(b'def add(',b'def changed_add(',1)
        with self.assertRaises(ValueError):r.primitive_identity(changed)
        self.assertEqual(m['dependencies'][str(r.dependency_paths()[0].relative_to(r.ROOT))],
                         hashlib.sha256(r.dependency_paths()[0].read_bytes()).hexdigest())


if __name__=='__main__':unittest.main()
