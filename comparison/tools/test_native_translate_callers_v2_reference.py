"""Source/literal full47 tests; original replay is validation, never targets."""
import copy
from fractions import Fraction
import hashlib
import json
from pathlib import Path
import struct
import unittest
from unittest.mock import patch

import native_translate_callers_v2_reference as m


class CallersV2(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.cases=m.build_source_cases()
        cls.rows={c['id']:c for c in cls.cases}

    def expected(self,identity):
        return self.rows[identity]['expected']

    def words(self,identity):
        return {k:int(v,16) for k,v in self.expected(identity)['expected_state_bits'].items()}

    def values(self,identity,prefix='sse'):
        out=self.words(identity)
        return [m.affine.fraction(out[f'{prefix}[{i}]']) for i in range(4)]

    def test_source_inventory_complete_with_explicit_open_dispositions(self):
        self.assertEqual(len(self.cases),117)
        self.assertEqual(len(self.rows),117)
        self.assertEqual(len({r['control']['profile'] for r in self.cases}),9)
        ready=[r for r in self.cases if 'expected_state_bits' in r['expected']]
        self.assertEqual(len(ready),78)
        for row in self.cases:
            expected=row['expected']
            self.assertFalse(expected['qualification'])
            if row in ready:
                self.assertEqual(set(expected['expected_state_bits']),set(m.STATE))
                self.assertEqual(expected['unproved_fields'],[])
                self.assertFalse(expected['original_pascal_parity'])
                self.assertFalse(expected['collector_compatible'])
            else:
                self.assertEqual(expected['status'],'OPEN_MODEL_DOMAIN')
                self.assertNotIn('expected_work',expected)
        self.assertFalse(hasattr(m,'evaluate'))
        self.assertFalse(hasattr(m,'build_cases'))

    def test_numeric_short_circuit_changes_return_lane0(self):
        self.assertEqual(self.values('numeric3d-one'),[25,0,25,0])
        self.assertEqual(self.values('numeric3d-bailout-before-max'),[16,0,25,0])
        self.assertEqual(self.words('numeric3d-bailout-before-max')['iteration'],1)
        self.assertEqual(self.words('numeric3d-bailout-equality')['iteration'],2)

    def test_numeric4d_retains_partial_xyz_radius(self):
        identity='numeric4d-nonzero-w-prepared-matrix'
        self.assertEqual(self.values(identity,'xyz'),[0,4,0,-3])
        self.assertEqual(self.values(identity),[25,0,16,0])
        self.assertEqual(m.affine.fraction(self.words(identity)['rout']),25)

    def test_numeric_interpolation_retains_pre_normalization_partial_sum(self):
        self.assertEqual(self.values('interpolation3d-one'),[25,0,25,0])
        self.assertEqual(self.values('interpolation4d-nonzero-w-prepared-matrix'),[25,0,16,0])
        for profile in ('interpolation3d','interpolation4d'):
            self.assertEqual(self.words(profile+'-one')['tmp'],41)
            self.assertEqual(self.words(profile+'-one')['current_slot'],1)
            self.assertEqual(self.expected(profile+'-one')['expected_work'],2)

    def test_analytic_returns_distance_and_own_denominator(self):
        for profile in ('analytic11','analytic4d5','analytic4d6','interpolation11','interpolation4d5'):
            with self.subTest(profile=profile):
                self.assertEqual(self.values(profile+'-one'),[5,0,1,0])
        self.assertEqual(m.affine.fraction(self.words('analytic11-one')['derivative[0]']),7)
        self.assertEqual(m.affine.fraction(self.words('analytic4d5-one')['derivative[0]']),1)
        self.assertEqual(self.values('analytic4d6-nonzero-w-prepared-matrix'),[5,0,1,0])

    def test_positive_count_cycle_and_negative_prefix_work(self):
        for profile in ('numeric3d','numeric4d','analytic11','analytic4d5','analytic4d6'):
            with self.subTest(profile=profile):
                positive=self.expected(profile+'-max-count3')
                self.assertEqual(positive['expected_work'],4)
                self.assertEqual(int(positive['expected_state_bits']['tmp'],16),2)
                negative=self.expected(profile+'-negative-prefix')
                self.assertEqual([t['slot'] for t in negative['trace']],[0,1,0,1])
                self.assertEqual([t['iteration'] for t in negative['trace']],[0,0,1,1])
                self.assertEqual(negative['expected_work'],4)
                self.assertEqual(int(negative['expected_state_bits']['iteration'],16),2)

    def test_julia_and_adapter_owned_preserved_fields(self):
        for row in self.cases:
            if 'expected_state_bits' not in row['expected']: continue
            for key in ('c[3]','derivative[3]','rstop_d','vary_scale','dfree1','dfree2','original_j[0]'):
                self.assertEqual(row['expected']['expected_state_bits'][key],row['initial_state_bits'][key])
        self.assertEqual(self.values('numeric3d-julia','j'),[Fraction(-1,4),Fraction(1,2),Fraction(-3,4),Fraction(-15,2)])
        self.assertEqual(self.values('numeric4d-julia','j'),[Fraction(-1,4),Fraction(1,2),Fraction(-3,4),Fraction(5,4)])

    def test_smoothing_low_changes_memory_but_not_xmm_transport(self):
        for profile in m.original.PROFILES:
            out=self.words(profile+'-smooth-low')
            self.assertEqual(out['smooth'],m.affine.fp32(2))
            expected0=Fraction(1,4) if m.original.PROFILES[profile][3] else Fraction(1,16)
            expected2=1 if m.original.PROFILES[profile][3] else Fraction(1,16)
            self.assertEqual(self.values(profile+'-smooth-low'),[expected0,0,expected2,0])
        for word in (0,m.affine.ONE,m.affine.ONE+7):self.assertEqual(m.low_smooth(word,3),0x40400000)
        with self.assertRaisesRegex(m.ScopeError,'logarithmic'):m.low_smooth(m.affine.ONE+8,3)

    def test_logarithmic_smoothing_never_receives_targets(self):
        rows=[r for r in self.cases if 'smooth-old-' in r['id']]
        self.assertEqual(len(rows),27)
        for row in rows:
            self.assertEqual(row['expected']['reason'],'logarithmic smoothing remains OPEN')
            self.assertNotIn('expected_state_bits',row['expected'])

    def test_rounded_interpolation_controls_remain_explicit(self):
        rows=[r for r in self.cases if r['expected'].get('reason')=='rounded interpolation square root remains OPEN']
        self.assertEqual(len(rows),12)
        self.assertTrue(all(r['control']['interpolation'] for r in rows))
        self.assertEqual({r['id'].rsplit('-',1)[-1] for r in rows},{'count3','left','right'})

    def test_zero_and_signed_zero_are_not_conflated(self):
        for profile in m.original.PROFILES:
            zero=self.words(profile+'-zero');negative=self.words(profile+'-signed-zero')
            for i in range(3):
                self.assertEqual(zero[f'xyz[{i}]'],0)
                self.assertEqual(negative[f'xyz[{i}]'],0x80000000)
            self.assertEqual(zero['rout'],0)
            self.assertEqual(negative['rout'],0)

    def test_outside_source_inventory_and_mutated_fields_reject(self):
        original=self.rows['numeric3d-one']['control']
        for key,value in [('profile','difs'),('max_it',9),('rstop',0),('count',0),('point',[3.,4.,1.]),
                          ('matrix','arbitrary'),('smooth',0),('de_option',2),('extra',1)]:
            row=copy.deepcopy(original);row[key]=value
            with self.subTest(key=key),self.assertRaisesRegex(m.ScopeError,'undeclared/altered'):m.evaluate_source(row)

    def test_exact_arithmetic_guard_rejects_rounded_radius(self):
        self.assertEqual(m.radius([m.affine.fp32(3),m.affine.fp32(4),0]),m.affine.fp32(25))
        with self.assertRaisesRegex(m.ScopeError,'product'):m.radius([0x3f800001,0,0])
        with self.assertRaisesRegex(m.ScopeError,'sum'):m.exact_add(m.affine.ONE,1)
        with self.assertRaisesRegex(m.ScopeError,'square root'):m.exact_sqrt(m.affine.fp32(2))

    def test_targets_do_not_read_observed_original_or_gpu_outputs(self):
        # Target generation needs source-authored builder data only; forbid all
        # file reads after module import, including any numerical evidence.
        with patch.object(Path,'read_bytes',side_effect=AssertionError('unexpected evidence read')), \
             patch.object(Path,'read_text',side_effect=AssertionError('unexpected evidence read')):
            self.assertEqual(m.build_source_cases(),self.cases)

    def test_manifest_detects_changed_source_and_no_admission_adapter(self):
        data=m.manifest()
        self.assertFalse(data['collector_compatible'])
        self.assertFalse(data['qualification'])
        pins=dict(m.PINS);pins['mb3d/formulas.pas']='0'*64
        with patch.object(m,'PINS',pins),self.assertRaisesRegex(m.ScopeError,'pin changed'):m.manifest()


if __name__=='__main__':unittest.main()
