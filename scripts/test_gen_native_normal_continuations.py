#!/usr/bin/env python3
import unittest
from pathlib import Path
from gen_native_normal_continuations import generate_normal,regions
from gen_native_metal_formulas import tokens,emit_text
ROOT=Path(__file__).resolve().parents[1]
class OrderedNormalSourceTests(unittest.TestCase):
 @classmethod
 def setUpClass(cls):
  cls.source=(ROOT/'src/render/native_metal/general_normals.metal.h').read_text()
  cls.code=generate_normal(cls.source);cls.regions=regions(cls.source)
 def test_changed_source_rejected(self):
  with self.assertRaises(ValueError):generate_normal(self.source+'\n')
 def test_smoothing_refused_before_child(self):
  self.assertLess(self.code.index('if(params.smoothing)'),self.code.index('normal_sample_child(s,center'))
 def test_axis_and_coordinate_expressions_preserved(self):
  expected=emit_text(self.regions['sample_setup']).replace('const uint axis =','axis =')
  self.assertIn(expected,self.code)
  self.assertIn('sample == 0 ? 2 : sample == 1 ? 0 : 1',self.code)
 def test_pair_combine_original(self):self.assertIn(emit_text(self.regions['after_pair']),self.code)
 def test_finish_original(self):
  self.assertIn(emit_text(self.regions['tail']).replace('return out;','return normal_phase(n,99);'),self.code)
 def test_first_then_plus_minus_and_fault_boundary(self):
  self.assertIn('normal_sample_child(s,center,stop,bool(direct),true,1)',self.code)
  self.assertIn('normal_sample_child(s,plus,stop,bool(direct),bool(direct),3)',self.code)
  self.assertIn('if(!(sample<3&&!record.fault))',self.code)
  # A plus fault must still invoke the guarded minus sample, as in P1.
  self.assertIn('if(n.header.pc==3){n.a=s.value;normal_sample_child',self.code)
 def test_no_inactive_normal_load(self):self.assertIn('n.header.pc>=2?float3(n.normal',self.code)
 def test_first_child_has_separate_storage_context(self):
  self.assertIn('if(n.header.pc==1)child=normal_sample_advance<true>',self.code)
  self.assertIn('else child=normal_sample_advance(s,',self.code)
 def test_no_child_reconstruction(self):
  self.assertEqual(self.code.count('normal_sample_advance('),1)
  self.assertNotIn('general_normal_sample(',self.code)
  self.assertNotIn('general_de(',self.code)
 def test_no_arithmetic_switch(self):
  for value in ('volatile','fma(','fast-math','fp-contract','noinline'):self.assertNotIn(value,self.code)
if __name__=='__main__':unittest.main()
