"""Pre-device literal original-CODE, caller, preservation and rejection tests."""
import copy
from fractions import Fraction
import struct
import unittest
from unittest.mock import patch

import native_group05_scaling_reference as m
from gpu_pipeline_proofs import STATE
from native_formula_observer import validate_case
from qualify_native_metal_all import current_registry


class OriginalScaling(unittest.TestCase):
    def test_signed_scale_changes_w_not_derivative_or_j(self):
        s=m.state()
        for i,v in enumerate((1,2,-3,5)):s[f'xyz[{i}]']=m.fp32(v)
        out=m.point(s,m.fp32(-2))
        self.assertEqual([m.fraction(out[f'xyz[{i}]']) for i in range(4)],[-2,-4,6,-10])
        for lane in set(STATE)-{f'xyz[{i}]' for i in range(4)}-{'current_slot'}:
            self.assertEqual(out[lane],s[lane],lane)

    def test_zero_signs(self):
        s=m.state()
        for i,v in enumerate((0,m.NEG_ZERO,0,m.NEG_ZERO)):s[f'xyz[{i}]']=v
        out=m.point(s,m.fp32(-2))
        self.assertEqual([out[f'xyz[{i}]'] for i in range(4)],[m.NEG_ZERO,0,m.NEG_ZERO,0])

    def test_literal_rounded_products(self):
        self.assertEqual(m.product(0x3f800001,0x3f800001),0x3f800002)
        self.assertEqual(m.product(0x3fc00000,0x3f800001),0x3fc00002)

    def test_caller_initialization_counter_cycle_and_preserved_derivatives(self):
        s=m.state();s['hybrid_counts[0]']=3
        for i,v in enumerate((3,4,12)):s[f'c[{i}]']=m.fp32(v)
        out=m.orbit(s,m.fp32(-2))
        self.assertEqual([m.fraction(out[f'xyz[{i}]']) for i in range(4)],[48,64,192,16])
        self.assertEqual((out['first'],out['iteration'],out['tmp']),(0,4,2))
        self.assertEqual(m.fraction(out['otrap']),Fraction(169))
        for i in range(4):self.assertEqual(out[f'derivative[{i}]'],s[f'derivative[{i}]'])
        for i in range(3):self.assertEqual(out[f'j[{i}]'],s[f'c[{i}]'])
        self.assertEqual(out['j[3]'],s['j[3]'])

    def test_bailout_is_strict_and_julia_changes_only_j(self):
        s=m.state();s['rstop']=m.fp32(676);s['julia']=1
        for i,v in enumerate((3,4,12)):s[f'c[{i}]']=m.fp32(v)
        out=m.orbit(s,m.fp32(2))
        self.assertEqual(out['iteration'],2)
        self.assertEqual(m.fraction(out['xyz[3]']),4)
        self.assertEqual([out[f'sse[{i}]'] for i in range(4)],[m.fp32(13),0,m.fp32(4),0])
        for i in range(3):self.assertEqual(out[f'j[{i}]'],s[f'original_j[{i}]'])

    def test_irrational_distance_transport_is_rejected_before_gpu(self):
        with self.assertRaisesRegex(m.ScopeError,'irrational'):
            m.orbit(m.state(),m.fp32(2))

    def test_domain_rejects_zero_orbit_denominator_and_rounded_radius(self):
        with self.assertRaises(m.ScopeError):m.orbit(m.state(),0)
        s=m.state();s['xyz[0]']=0x3f800001
        with self.assertRaises(m.ScopeError):m.radius(s)
        with self.assertRaises(m.ScopeError):m.product(1,m.ONE)
        with self.assertRaises(ValueError):m.product(0x7f800000,m.ONE)
        with self.assertRaises(ValueError):m.product(0x7f7fffff,m.fp32(2))


class Evidence(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.cases=m.build_cases()
        cls.registry=current_registry(m.ROOT)

    def test_all_50_fixed_cases_have_full_preservation_and_observer_abi(self):
        self.assertEqual(len(self.cases),50)
        self.assertEqual(len({r['id'] for r in self.cases}),50)
        self.assertEqual({r['input']['kind'] for r in self.cases},{'point','orbit'})
        for r in self.cases:
            validate_case(r['input'],self.registry)
            self.assertEqual(set(r['expected_state_bits']),set(STATE))
            for lane in set(STATE)-set(r['write_lanes']):
                self.assertEqual(r['expected_state_bits'][lane],r['input']['input_state_bits'][lane],(r['id'],lane))

    def test_parameter_raw_double_and_alias(self):
        p=m.program(m.fp32(-2),m.state());raw=struct.pack('<64I',*p[260:324])
        self.assertEqual(struct.unpack_from('<Q',raw,240)[0],0xc000000000000000)
        self.assertEqual(struct.unpack_from('<Q',raw,248)[0],0x3fe0000000000000)
        self.assertEqual(p[132:134],[0,0xc0000000]);self.assertEqual(p[325],0xc0000000)

    def test_mismatched_metadata_raw_alias_and_caller_rejected(self):
        for i in (1,6,59,60,62,68,132,321,325,50):
            data=copy.deepcopy(self.cases[0]['input']);data['parameter_bits'][i]=m.word(int(data['parameter_bits'][i],16)^1)
            with self.assertRaises(ValueError,msg=str(i)):m.evaluate(data)
        for field in ('inside','calc_smooth','max_iterations','hybrid_counts[0]'):
            data=copy.deepcopy(self.cases[-1]['input']);data['input_state_bits'][field]='0x00000009'
            with self.assertRaises(ValueError,msg=field):m.evaluate(data)

    def test_bad_identity_width_hex_and_nonfinite_rejected(self):
        data=copy.deepcopy(self.cases[0]['input']);data['identity'][1]=9453
        with self.assertRaises(ValueError):m.evaluate(data)
        data=copy.deepcopy(self.cases[0]['input']);del data['input_state_bits']['derivative[0]']
        with self.assertRaises(ValueError):m.evaluate(data)
        data=copy.deepcopy(self.cases[0]['input']);data['parameter_bits'][68]='0xABCDEF00'
        with self.assertRaises(ValueError):m.evaluate(data)
        data=copy.deepcopy(self.cases[0]['input']);data['input_state_bits']['xyz[0]']='0x7fc00000'
        with self.assertRaises(ValueError):m.evaluate(data)

    def test_manifest_rejects_changed_caller_proof(self):
        with patch.object(m,'CALLER_PROFILE',(m.CALLER_PROFILE[0],'0'*64)):
            with self.assertRaisesRegex(ValueError,'pinned source'):m.manifest()

    def test_manifest_rejects_changed_source_and_dependency(self):
        self.assertEqual(m.manifest()['code_audit']['_Scaling']['bytes'],44)
        with patch.dict(m.SOURCE_PINS,{'mb3d/formulas.pas':'0'*64}):
            with self.assertRaisesRegex(ValueError,'pinned source'):m.manifest()
        with patch.object(m,'PRIMITIVE_SHA256','0'*64):
            with self.assertRaisesRegex(ValueError,'pinned source'):m.manifest()


if __name__=='__main__':unittest.main()
