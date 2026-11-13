"""Original-derived literals, typed transport and caller proof before device use."""
import copy
from fractions import Fraction
from pathlib import Path
import unittest
from unittest.mock import patch

import native_formula_affine_v2_reference as m
import native_formula_reference as old
from native_formula_observer import validate_case
from qualify_native_metal_all import current_registry


class Arithmetic(unittest.TestCase):
    def test_authored_decimal_double_encoding_and_fp32_projection(self):
        self.assertEqual(m.binary64(Fraction(1,10)), 0x3fb999999999999a)
        self.assertEqual(m.project_double(0x3fb999999999999a), 0x3dcccccd)
        self.assertEqual(m.binary64(Fraction(-1,5)), 0xbfc999999999999a)
        self.assertEqual(m.project_double(1 << 63), m.NEG_ZERO)
        self.assertEqual(m.project_double(1), 0)
        self.assertEqual(m.project_double((1 << 63)|1), m.NEG_ZERO)

    def test_binary64_roundtrip_normal_subnormal_and_ties(self):
        for exponent in (0,1,2,1022,1023,1024,2046):
            for mantissa in (0,1,(1<<51),(1<<52)-1):
                for sign in (0,1 << 63):
                    bits = sign | (exponent << 52) | mantissa
                    self.assertEqual(m.binary64(m.fraction64(bits), bits == 1 << 63), bits)
        self.assertEqual(m.binary64(Fraction(1,1 << 1075)),0)
        self.assertEqual(m.binary64(Fraction(3,1 << 1075)),2)
        with self.assertRaises(ValueError): m.fraction64(0x7ff0000000000000)

    def test_sqrt_known_literals_and_signedzero(self):
        for argument, expected in ((0,0),(m.NEG_ZERO,m.NEG_ZERO),(m.ONE,m.ONE),
                                   (0x40000000,0x3fb504f3),(0x40400000,0x3fddb3d7),
                                   (m.fp32(169),m.fp32(13)),(m.fp32(Fraction(25,32)),0x3f624630)):
            self.assertEqual(m.sqrt32(argument),expected)
            self.assertEqual(m.sqrt_transport(argument),expected)
        with self.assertRaises(ValueError):m.sqrt32(0xbf800000)
        with self.assertRaises(ValueError):m.sqrt32(0x7f800000)

    def test_exact_midpoint_square_selects_even_neighbor(self):
        for low in (0x3f800000,0x3f800001):
            midpoint=(m.fraction(low)+m.fraction(low+1))/2
            self.assertEqual(m._sqrt_round(midpoint**2,32),low+(low&1))
            self.assertEqual(m._sqrt_round(midpoint**2-Fraction(1,1<<80),32),low)
            self.assertEqual(m._sqrt_round(midpoint**2+Fraction(1,1<<80),32),low+1)

    def test_sqrt_witness_brackets_exact_result_across_exponents(self):
        for bits in (1,0x007fffff,0x00800000,0x00800001,0x3dcccccd,0x3f800001,0x7f7fffff):
            result=m.sqrt_transport(bits); value=m.fraction(bits)
            lo=(m.fraction(result-1)+m.fraction(result))/2
            hi=(m.fraction(result)+m.fraction(result+1))/2
            self.assertLessEqual(lo*lo,value)
            self.assertGreaterEqual(hi*hi,value)


class TranslateSemantics(unittest.TestCase):
    def test_three_source_adds_and_all_other_words_preserved(self):
        s=m.state(); params=[m.binary64(v) for v in (Fraction(1,8),Fraction(-1,4),Fraction(3,8))]
        out=m.translate_step(s,params,slot=1)
        self.assertEqual([out[f'xyz[{i}]'] for i in range(3)], [m.fp32(Fraction(25,8)),m.fp32(Fraction(15,4)),m.fp32(Fraction(99,8))])
        for lane in set(m.STATE)-{'xyz[0]','xyz[1]','xyz[2]','current_slot'}:
            self.assertEqual(out[lane],s[lane],lane)
        self.assertEqual(out['current_slot'],1)

    def test_defaults_are_zero_add_not_unconditional_copy(self):
        s=m.state();s['xyz[0]']=m.NEG_ZERO
        self.assertEqual(m.translate_step(s,[0,0,0])['xyz[0]'],0)
        self.assertEqual(m.translate_step(s,[1<<63,0,0])['xyz[0]'],m.NEG_ZERO)

    def test_parameter_projection_precedes_addition(self):
        s=m.state();s['xyz[0]']=m.ONE
        original=Fraction(1,1<<24)+Fraction(1,1<<53)
        out=m.translate_step(s,[m.binary64(original),0,0])
        self.assertEqual(out['xyz[0]'],m.ONE)
        # Direct Double-original sum rounds upward. Native RN32 parameter
        # projection first produces an exact tie that rounds to even1.
        self.assertEqual(m.fp32(Fraction(1)+original),m.ONE+1)

    def test_subnormal_and_maxfinite_boundary_literals(self):
        s=m.state();s['xyz[0]']=0x00800000
        self.assertEqual(m.translate_step(s,[m.binary64(-m.fraction(0x007fffff)),0,0])['xyz[0]'],1)
        s['xyz[0]']=0x7f7fffff
        self.assertEqual(m.translate_step(s,[0,0,0])['xyz[0]'],0x7f7fffff)
        self.assertEqual(m.translate_step(s,[m.binary64(-m.fraction(0x7f7fffff)),0,0])['xyz[0]'],0)
        with self.assertRaises(ValueError):m.translate_step(s,[m.binary64(m.fraction(0x7f7fffff)),0,0])
        with self.assertRaises(ValueError):m.translate_step(s,[0x7ff0000000000000,0,0])

    def test_typed_raw_aliases_cannot_collide_as_same_float(self):
        s=m.state();params=[m.binary64(Fraction(1,10)),0,0];p=m.program(params,s)
        self.assertEqual(m.prepared_translate_parameters(p),params)
        # Distinct binary64 values may share one float mirror, but every raw
        # parameter and PVar representation still must match its original word.
        other=list(params);other[0]+=1
        self.assertEqual(m.project_double(params[0]),m.project_double(other[0]))
        bad=list(p);bad[132]+=1
        with self.assertRaisesRegex(ValueError,'alias'):m.prepared_translate_parameters(bad)
        for index in (68,56+268+1,56+204+60,56+204+62):
            bad=list(p);bad[index]^=1
            with self.subTest(index=index),self.assertRaises(ValueError):m.prepared_translate_parameters(bad)

    def test_mixed_slot_helper_preserves_state_and_unused_context_words(self):
        s=m.state(); params=[m.binary64(Fraction(-7,2)),0,0];p=m.program(params,s)
        p[56+332:56+664]=p[56:56+332]
        p[56+332+300]=0xdeadbeef  # Unread constants are context data, not operands.
        self.assertEqual(m.prepared_translate_parameters(p,1),params)
        self.assertEqual(m.translate_step(s,params,1)['xyz[0]'],m.fp32(Fraction(-1,2)))
        p[56+332+4]=4
        with self.assertRaises(ValueError):m.prepared_translate_parameters(p,1)


class Caller(unittest.TestCase):
    def test_source_initialization_count_cycle_preservation_and_return_sse(self):
        s=m.state();s['hybrid_counts[0]']=3
        out=m.orbit(s,[m.binary64(v) for v in (3,4,12)])
        self.assertEqual([m.fraction(out[f'xyz[{i}]']) for i in range(4)],[15,20,60,1])
        self.assertEqual((out['first'],out['iteration'],out['tmp']),(0,4,2))
        self.assertEqual(out['rout'],m.fp32(4225));self.assertEqual(out['otrap'],m.fp32(169))
        self.assertEqual([out[f'sse[{i}]'] for i in range(4)],[m.fp32(65),0,m.ONE,0])
        for lane in ('j[3]','vary_scale','derivative[0]','derivative[1]','derivative[2]','derivative[3]'):
            self.assertEqual(out[lane],s[lane])

    def test_strict_bailout_and_julia(self):
        s=m.state();s['rstop']=m.fp32(676);s['julia']=1
        out=m.orbit(s,[m.binary64(v) for v in (3,4,12)])
        self.assertEqual(out['iteration'],2)
        for i in range(3):self.assertEqual(out[f'j[{i}]'],s[f'original_j[{i}]'])
        self.assertEqual(out['sse[0]'],m.fp32(39))

    def test_zero_radius_and_irrational_radius_have_defined_sse(self):
        s=m.state()
        for i in range(3):s[f'c[{i}]']=0
        self.assertEqual(m.orbit(s,[0]*3)['sse[0]'],0)
        s['c[0]']=s['c[1]']=m.ONE
        self.assertEqual(m.orbit(s,[0]*3)['sse[0]'],0x3fb504f3)

    def test_prepared_program_rejects_nonpositive_stop_but_zero_radius_is_valid(self):
        for stop in (0, m.NEG_ZERO, m.fp32(-16)):
            s=m.state();s['rstop']=stop
            with self.subTest(stop=stop), self.assertRaisesRegex(ValueError,'radius_stop'):
                m.orbit(s,[0]*3)
            for kind in ('point','orbit'):
                payload={'identity':m.IDENTITY,'kind':kind,
                         'input_state_bits':{k:m.word(v) for k,v in s.items()},
                         'parameter_bits':list(map(m.word,m.program([0]*3,s)))}
                with self.subTest(stop=stop,kind=kind),self.assertRaisesRegex(ValueError,'radius_stop'):
                    m.evaluate(payload)
        case=next(c for c in m.build_cases() if c['variant']=='orbit-zero-radius')
        self.assertEqual(case['input']['input_state_bits']['rstop'],m.word(m.fp32(16)))
        self.assertEqual(case['expected_state_bits']['rout'],m.word(0))
        self.assertEqual(case['expected_state_bits']['iteration'],m.word(2))

    def test_unproved_driver_states_and_rounded_radius_reject(self):
        for key,value in (('calc_smooth',1),('inside',1),('vary_scale',0),('j[3]',0),
                          ('hybrid_counts[0]',0x80000001),('hybrid_counts[1]',1),('max_iterations',9)):
            s=m.state();s[key]=value
            with self.subTest(key=key),self.assertRaises(ValueError):m.orbit(s,[0]*3)
        s=m.state();s['c[0]']=0x3f800001
        with self.assertRaisesRegex(ValueError,'radius product'):m.orbit(s,[0]*3)


class Evidence(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.cases=m.build_cases();cls.registry=current_registry(m.ROOT)

    def test_corpus_full_state_metadata_preservation_and_json_roundtrip(self):
        self.assertEqual(len(self.cases),50)
        self.assertEqual(sum(c['input']['kind']=='point' for c in self.cases),21)
        self.assertEqual(len({c['id'] for c in self.cases}),50)
        for case in self.cases:
            validate_case(case['input'],self.registry)
            self.assertEqual(set(case['expected_state_bits']),set(m.STATE))
            for lane in set(m.STATE)-set(case['write_lanes']):
                self.assertEqual(case['expected_state_bits'][lane],case['input']['input_state_bits'][lane],(case['id'],lane))
        self.assertEqual(m.manifest(),m.json.loads(m.json.dumps(m.manifest())))

    def test_old_translate_inputs_exact_outputs_change_only_sse(self):
        cases={c['variant']:c for c in self.cases}
        for previous in old.build_cases():
            if previous['identity'] != m.IDENTITY or previous['input']['kind'] != 'orbit':continue
            current=cases['legacy-'+previous['variant']]
            self.assertEqual(current['input'],previous['input'])
            changes={k for k in m.STATE if current['expected_state_bits'][k]!=previous['expected_state_bits'][k]}
            self.assertEqual(changes,{f'sse[{i}]' for i in range(4)})

    def test_full_metadata_and_unused_storage_mutations_reject(self):
        case=copy.deepcopy(self.cases[0]['input'])
        for index in (1,6,13,59,60,61,62,63,64,389,2047):
            bad=copy.deepcopy(case);bad['parameter_bits'][index]=m.word(int(bad['parameter_bits'][index],16)^1)
            with self.subTest(index=index),self.assertRaises(ValueError):m.evaluate(bad)

    def test_source_dependency_and_caller_profile_pins_reject(self):
        for target in ('SOURCE_PINS','ABI_PINS','DEPENDENCY_PINS','CALLER_PINS','DATA_PINS'):
            pins=dict(getattr(m,target));pins[next(iter(pins))]='0'*64
            with self.subTest(target=target),patch.object(m,target,pins),self.assertRaises(ValueError):m.manifest()
        self.assertEqual(m.manifest()['caller_members']['analytic-caller/compile/original_analytic_oracle.exe'],
                         '104baaf1738143ddb6a5f9c0aa504ed1eb230bc005bbf80190d339b2d40e4ec6')


if __name__=='__main__':unittest.main()
