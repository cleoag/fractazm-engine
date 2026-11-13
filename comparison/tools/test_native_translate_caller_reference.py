"""Literal/source-bound partial caller diagnostics; no GPU or complete proof."""
import copy
from fractions import Fraction
import unittest
from unittest.mock import patch

import native_translate_caller_reference as m
import native_formula_observer as observer


class CallerSource(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.cases=m.build_source_cases()
        cls.registry=observer.current_registry(observer.ROOT)

    def case(self,profile,tag='j0-c1-n4'):
        return copy.deepcopy(next(c['source_case'] for c in self.cases if c['id']==f'translate-caller-{profile}-{tag}'))

    def result(self,profile,tag='j0-c1-n4'):
        return m.evaluate_source(self.case(profile,tag))['expected_memory_bits']

    def test_valid_current_metadata_and_program_shapes(self):
        self.assertEqual(len(self.cases),60)
        self.assertEqual(len({c['id'] for c in self.cases}),60)
        for row in self.cases:
            with self.subTest(case=row['id']):
                observer.validate_case(row['source_case']['input'],self.registry)
                self.assertEqual(row['expected'],m.evaluate_source(row['source_case']))

    def test_numeric_and_analytic_reset_w_differ(self):
        numeric=self.result('numeric3d');analytic=self.result('analytic11')
        self.assertEqual([numeric[f'xyz[{i}]'] for i in range(4)],['0x40e00000','0x40800000','0x00000000','0x00000000'])
        self.assertEqual(analytic['xyz[3]'],'0x3f800000')
        self.assertEqual(numeric['rout'],'0x42820000') # 7^2+4^2=65
        self.assertEqual(numeric['rold'],'0x42500000') # 6^2+4^2=52
        self.assertEqual(numeric['otrap'],'0x41c80000') # initial25
        self.assertEqual(numeric['iteration'],'0x00000004')
        self.assertEqual(numeric['first'],'0x00000000')

    def test_julia_and_4d_j4_initialization(self):
        source=self.case('numeric3d')['input']['input_state_bits']
        for profile in m.PROFILES:
            out=self.result(profile,'j1-c1-n4')
            for i in range(3):self.assertEqual(out[f'j[{i}]'],source[f'original_j[{i}]'])
        self.assertEqual(self.result('numeric4d')['j[3]'],'0x00000000')
        self.assertEqual(self.result('analytic4d5','j1-c1-n4')['j[3]'],source['original_j[3]'])
        self.assertEqual(self.result('numeric3d')['j[3]'],source['j[3]'])

    def test_analytic4d_derivative_reset_and_other_preservation(self):
        source=self.case('analytic4d5')['input']['input_state_bits']
        for profile in ('analytic4d5','analytic4d6'):
            out=self.result(profile)
            self.assertEqual(out['derivative[0]'],'0x3f800000')
            for lane in ('derivative[1]','derivative[2]','derivative[3]','vary_scale','dfree1','dfree2','smooth'):
                self.assertEqual(out[lane],source[lane])
        self.assertEqual(self.result('numeric4d')['derivative[0]'],source['derivative[0]'])

    def test_count_cycles_and_strict_bailout_equality(self):
        for profile in m.PROFILES:
            out=self.result(profile,'j0-c3-n4')
            self.assertEqual(out['tmp'],'0x00000002')
            out=self.result(profile,'bailout-equality')
            self.assertEqual(out['iteration'],'0x00000002')
            self.assertEqual(out['rout'],'0x41c80000')

    def test_low_smoothing_and_real_high_dword_boundary(self):
        for profile in m.PROFILES:
            self.assertEqual(self.result(profile,'smooth-low-j0')['smooth'],'0x40000000')
        # Original CMP high DWORD admits seven FP32 steps above1; step8 is
        # first high DWORD3ff00001. Exact Rout<=1 would get this wrong.
        for value in (0,m.affine.ONE,m.affine.ONE+1,m.affine.ONE+7):
            self.assertEqual(m.smooth_low_branch(value,3),0x40400000)
        with self.assertRaisesRegex(ValueError,'logarithmic'):
            m.smooth_low_branch(m.affine.ONE+8,3)

    def test_identity_rotation_uses_operations_and_signed_zero(self):
        for profile in ('numeric4d','analytic4d5','analytic4d6'):
            out=self.result(profile,'signed-zero')
            # Fourth row has three (-0)*(+0) products; all-negative zero
            # reduction remains -0. Translate adds +0 to xyz only.
            self.assertEqual([out[f'xyz[{i}]'] for i in range(4)],['0x00000000']*3+['0x80000000'])

    def test_unknown_sse_never_presented_as_expected_or_admissible(self):
        self.assertFalse(hasattr(m,'evaluate'))
        self.assertFalse(hasattr(m,'build_cases'))
        for row in self.cases:
            out=row['expected']
            self.assertEqual(set(out['expected_memory_bits']),set(m.STATE)-set(m.SSE_FIELDS))
            self.assertEqual(out['unproved_fields'],list(m.SSE_FIELDS))
            self.assertFalse(out['qualification'])
            self.assertTrue(out['compiled_transport_required'])
            self.assertEqual(len(out['expected_memory_bits']),43)

    def test_typed_metadata_and_program_mutations_reject(self):
        source=self.case('numeric3d')
        for index in (1,6,13,28,59,60,61,62,68,132,320,2047):
            case=copy.deepcopy(source);p=case['input']['parameter_bits'];p[index]=m.affine.word(int(p[index],16)^1)
            with self.subTest(index=index),self.assertRaises(ValueError):m.evaluate_source(case)

    def test_unproved_profiles_and_domain_reject(self):
        for profile in m.OPEN_PROFILES:
            case=self.case('numeric3d');case['profile']=profile
            with self.assertRaises(ValueError):m.evaluate_source(case)
        for key,value in (('inside',1),('rstop',0),('max_iterations',9),('hybrid_counts[0]',0x80000001)):
            case=self.case('numeric3d');s={k:int(v,16) for k,v in case['input']['input_state_bits'].items()};s[key]=value
            case['input']['input_state_bits']={k:m.affine.word(v) for k,v in s.items()}
            case['input']['parameter_bits']=list(map(m.affine.word,m.program('numeric3d',s,[m.affine.binary64(1),0,0])))
            with self.subTest(key=key),self.assertRaises(ValueError):m.evaluate_source(case)

    def test_rounded_radius_stays_open(self):
        s=m.affine.state();s['xyz[0]']=0x3f800001
        with self.assertRaisesRegex(ValueError,'radius product'):m.exact_radius(s,3)

    def test_manifest_pins_and_json_roundtrip(self):
        manifest=m.manifest()
        self.assertEqual(manifest,m.json.loads(m.json.dumps(manifest)))
        self.assertFalse(manifest['collector_compatible'])
        pins=dict(m.PINNED_PATHS);pins['mb3d/formulas.pas']='0'*64
        with patch.object(m,'PINNED_PATHS',pins),self.assertRaisesRegex(ValueError,'pin changed'):m.manifest()


if __name__=='__main__':unittest.main()
