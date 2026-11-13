"""Original-CODE arithmetic, typed ABI and caller-state tests; no GPU output."""
import copy
from fractions import Fraction
import hashlib
import struct
import unittest
from unittest.mock import patch

import native_group03_julia_reference as m
from gpu_pipeline_proofs import STATE
from native_formula_observer import validate_case


class ArithmeticAndOriginalCode(unittest.TestCase):
    def test_sqrt_known_values_and_signed_zero(self):
        for bits, expected in [(0,0),(m.NEG_ZERO,m.NEG_ZERO),(m.ONE,m.ONE),
                               (m.fp32(2),0x3fb504f3),(m.fp32(9),m.fp32(3)),
                               (m.fp32(Fraction(1,4)),m.fp32(Fraction(1,2)))]:
            self.assertEqual(m.sqrt_bits(bits),expected)
        for bits in (m.fp32(-1),1,0x7f800000,0x7fc00000):
            with self.assertRaises(ValueError):m.sqrt_bits(bits)

    def test_sqrt_interval_nearest_proof(self):
        for bits in (m.fp32(3),m.fp32(5),0x2f123456,0x7f7fffff):
            out=m.sqrt_bits(bits);x=m.fraction(bits)
            lower=(m.fraction(out-1)+m.fraction(out))/2
            upper=(m.fraction(out)+m.fraction(out+1))/2
            self.assertLessEqual(lower*lower,x);self.assertLessEqual(x,upper*upper)

    def test_julia_sets_reads_C_and_real_nearest_even(self):
        s=m._state()
        for i,v in enumerate((1,3,5)):s[f'c[{i}]']=m.fp32(v)
        o=m._point('_JuliaSets',s,[m.fp32(2),m.fp32(Fraction(1,4))])
        self.assertEqual([o[f'xyz[{i}]'] for i in range(3)],list(map(m.fp32,(1,-1,1))))
        self.assertEqual([o[f'j[{i}]'] for i in range(3)],[0,m.fp32(Fraction(1,2)),m.fp32(Fraction(1,2))])
        s['c[0]']=m.fp32((1<<31)+256)
        o=m._point('_JuliaSets',s,[m.ONE,m.ONE])
        self.assertEqual(o['j[0]'],s['c[0]']);self.assertEqual(o['xyz[0]'],0)
        s['c[0]']=m.fp32(-1)
        o=m._point('_JuliaSets',s,[m.fp32(2),m.ONE])
        self.assertEqual(o['j[0]'],m.NEG_ZERO);self.assertEqual(o['xyz[0]'],m.fp32(-1))

    def test_partly_radius_boundary_and_C_not_xyz(self):
        s=m._state()
        for i,v in enumerate((3,4,0)):s[f'c[{i}]']=m.fp32(v)
        for radius in (5,6):
            o=m._point('_PartlyJuliaRoff',s,[0,0,0,m.fp32(radius),m.EPS])
            self.assertEqual([o[f'j[{i}]'] for i in range(4)],[s[f'j[{i}]'] for i in range(4)])
        o=m._point('_PartlyJuliaRoff',s,[0,0,0,0,m.EPS])
        self.assertEqual(o['j[0]'],m.fp32(3+m.fraction(s['j[0]'])))
        self.assertEqual(o['j[1]'],m.fp32(4+m.fraction(s['j[1]'])))
        for i in range(4):self.assertEqual(o[f'xyz[{i}]'],s[f'xyz[{i}]'])

    def test_epsilon_guard_not_divide_by_zero(self):
        s=m._state()
        for i in range(3):s[f'c[{i}]']=s[f'j[{i}]']=0
        o=m._point('_PartlyJuliaRoff',s,[0,0,0,m.fp32(-1),m.EPS])
        self.assertEqual(o['j[0]'],0)
        s['c[0]']=m.fp32(Fraction(1,1<<40))
        o=m._point('_PartlyJuliaRoff',s,[0,0,0,0,m.EPS])
        ratio=m.fp32(Fraction(1,1<<40)/m.fraction(m.EPS))
        expected=m.fp32(Fraction(1,1<<40)*m.fraction(ratio))
        self.assertEqual(o['j[0]'],expected)
        self.assertNotEqual(o['j[0]'],s['c[0]'])

    def test_updateC2_CODE_inequality_W_and_tie(self):
        s=m._state();p=m._parameters_default('_updateC2')
        for i,v in enumerate((1,2,3)):s[f'xyz[{i}]']=m.fp32(v)
        for i,v in enumerate((2,1,3)):s[f'j[{i}]']=m.fp32(v)
        o=m._point('_updateC2',s,p)
        self.assertEqual([o[f'xyz[{i}]'] for i in range(3)],list(map(m.fp32,(2,2,3))))
        self.assertEqual([o[f'j[{i}]'] for i in range(3)],list(map(m.fp32,(1,1,3))))
        p[9]=m.fp32(2);o=m._point('_updateC2',s,p)
        self.assertEqual(o['xyz[3]'],m.fp32(2*m.fraction(s['xyz[3]'])))
        # Literal status-word simulation at stack depth2. C0 is carry after
        # SHR; TOP6 keeps ZF clear, so equality performs FXCH.
        ah=(6<<3)|0x40
        self.assertFalse(bool(ah&1) or (ah>>1)==0)
        p=m._parameters_default('_updateC2')
        for i in range(3):s[f'xyz[{i}]']=0;s[f'j[{i}]']=m.NEG_ZERO
        o=m._point('_updateC2',s,p)
        self.assertEqual([o[f'j[{i}]'] for i in range(3)],[0,0,0])

    def test_updateC2_matrix_transpose_scale_and_offset(self):
        s=m._state();p=m._parameters_default('_updateC2')
        p[:9]=list(map(m.fp32,(0,1,0,0,0,1,1,0,0)))
        p[9:]=list(map(m.fp32,(2,1,2,3)))
        for i,v in enumerate((1,2,3)):s[f'xyz[{i}]']=m.fp32(v);s[f'j[{i}]']=m.fp32(-9)
        # M*x=(2,3,1); scale/add=(5,8,5); no swaps;
        # transpose gives(5,5,8), then subtract original offsets ->(4,3,5).
        o=m._point('_updateC2',s,p)
        self.assertEqual([o[f'xyz[{i}]'] for i in range(3)],list(map(m.fp32,(4,3,5))))

    def test_helispiral_typed_flags_and_independent_small_sine(self):
        s=m._state();p=m._parameters_default('_helispiral');p[10]=m.fp32(Fraction(1,1<<20))
        for i,v in enumerate((1,0,3)):s[f'xyz[{i}]']=m.fp32(v)
        for i,v in enumerate((3,4,2)):s[f'j[{i}]']=m.fp32(v)
        for h,sp,n in [(0,0,4),(1,0,3),(0,1,8),(1,1,7),(2,-1,4),(16777217,-2147483648,4)]:
            p[11:13]=[h,sp];o=m._point('_helispiral',s,p)
            self.assertEqual(o['xyz[0]'],m.ONE)
            self.assertEqual(o['xyz[1]'],m.fp32(-Fraction(n,1<<20)))
            self.assertEqual(o['xyz[2]'],m.fp32(3))
            for i in range(4):self.assertEqual(o[f'j[{i}]'],s[f'j[{i}]'])

    def test_helispiral_nonzero_fixed_angle_not_host_libm(self):
        s=m._state();p=m._parameters_default('_helispiral');p[9]=0;p[13]=m.fp32(Fraction(1,1<<12))
        for i,v in enumerate((1,0,0)):s[f'xyz[{i}]']=m.fp32(v)
        angle=m.fp32(m.fraction(m.DEG)/4096)
        lo,hi=m.tail.trig_interval(angle)
        self.assertEqual(m.fp32(lo),angle);self.assertEqual(m.fp32(hi),angle)
        o=m._point('_helispiral',s,p)
        self.assertEqual(o['xyz[0]'],m.ONE);self.assertEqual(o['xyz[1]'],angle^m.NEG_ZERO)

    def test_singular_nonfinite_and_ambiguous_arithmetic_rejected(self):
        with self.assertRaises(ValueError):m._point('_JuliaSets',m._state(),[0,m.ONE])
        s=m._state();s['xyz[3]']=0x7f7fffff;p=m._parameters_default('_updateC2');p[9]=m.fp32(2)
        with self.assertRaises(ValueError):m._point('_updateC2',s,p)
        with self.assertRaises(ValueError):m.dot([m.ONE,m.ONE,m.fp32(1<<24)],[m.ONE]*3)


class CorpusAndABI(unittest.TestCase):
    @classmethod
    def setUpClass(cls):cls.cases=m.build_cases()

    def test_complete_unique_cases_and_observer_ABI(self):
        self.assertEqual(len(self.cases),65)
        self.assertEqual(len({c['id'] for c in self.cases}),65)
        for c in self.cases:
            self.assertEqual(len(c['input']['parameter_bits']),2048)
            self.assertEqual(len(c['expected_state_bits']),47)
            validate_case(c['input'])
            self.assertEqual(m.evaluate(c['input']),c['expected_state_bits'])
        for name in m.FORMULAS:
            kinds={c['input']['kind'] for c in self.cases if c['identity'][2]==name}
            self.assertEqual(kinds,{'point','orbit'})

    def test_full_integer_sentinels_and_untouched_lanes(self):
        for c in self.cases:
            before=c['input']['input_state_bits'];after=c['expected_state_bits']
            for lane in set(STATE)-set(c['write_lanes']):self.assertEqual(after[lane],before[lane],(c['id'],lane))
            if c['variant']=='integer_sentinels':
                self.assertEqual(after['first'],'0xffffffff');self.assertEqual(after['iteration'],'0x01000001');self.assertEqual(after['tmp'],'0x80000000')

    def test_orbit_early_escape_vs_equal_threshold(self):
        for c in self.cases:
            if c['variant']=='orbit_escape':self.assertEqual(c['expected_state_bits']['iteration'],'0x00000001')
            if c['variant']=='orbit_equal_threshold':self.assertEqual(c['expected_state_bits']['iteration'],'0x00000004')

    def test_W_evolves_across_four_iterations(self):
        for c in self.cases:
            if c['variant'].startswith('orbit_W_scale_'):
                self.assertEqual(c['expected_state_bits']['xyz[3]'],m.word(m.fp32(16)))
                self.assertEqual(c['expected_state_bits']['iteration'],'0x00000004')

    def test_exact_flag_metadata_raw_and_constant_storage(self):
        c=next(c for c in self.cases if c['variant']=='flags_16777217_-2147483648');p=list(map(lambda v:int(v,16),c['input']['parameter_bits']))
        self.assertEqual(m._parameters('_helispiral',p)[11:13],[16777217,-2147483648])
        self.assertEqual(p[68+11],m.fp32(16777217))
        raw=struct.pack('<64I',*p[260:324])
        self.assertEqual(struct.unpack_from('<i',raw,256-64)[0],16777217)
        self.assertEqual(p[132+28]|p[133+28]<<32,m.tail.binary64(Fraction('0.017453293')))
        self.assertNotEqual(p[132+28]|p[133+28]<<32,m.tail.wide32(m.DEG))

    def test_reject_program_and_state_tampering(self):
        c=next(c for c in self.cases if c['variant']=='flags_16777217_-2147483648')
        # Typed metadata, raw PVar, constants alias, unused slot and header.
        for index in (132+22,260+(256-64)//4,356,56+332+9,0):
            payload=copy.deepcopy(c['input']);payload['parameter_bits'][index]=m.word(int(payload['parameter_bits'][index],16)^1)
            with self.assertRaises(ValueError,msg=str(index)):m.evaluate(payload)
        for mutation in ('nonfinite','missing_lane','extra_key','wrong_identity'):
            p=copy.deepcopy(c['input'])
            if mutation=='nonfinite':p['input_state_bits']['c[0]']='0x7f800000'
            elif mutation=='missing_lane':del p['input_state_bits']['first']
            elif mutation=='extra_key':p['unbound']=True
            else:p['identity'][1]=10014
            with self.assertRaises(ValueError,msg=mutation):m.evaluate(p)

    def test_bound_dependencies_and_original_opcode_pins(self):
        manifest=m.manifest()
        self.assertEqual(len(manifest['dependencies']),3)
        self.assertEqual(set(manifest['code_audit']),set(m.FORMULAS))
        self.assertEqual(manifest['model_sha256'],hashlib.sha256(m.Path(m.__file__).read_bytes()).hexdigest())
        with patch.dict(m.DEPENDENCY_PINS,{'native_formula_reference.py':'0'*64}):
            with self.assertRaisesRegex(ValueError,'dependency'):m.manifest()
        with patch.dict(m.CODE_AUDIT['_JuliaSets'],{'sha256':'0'*64}):
            with self.assertRaisesRegex(ValueError,'CODE'):m.manifest()


if __name__=='__main__':unittest.main()
