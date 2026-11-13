#!/usr/bin/env python3
import unittest
from pathlib import Path
from gen_native_normal_sample_continuations import generate_sample,generate_observer,regions
from gen_native_orbit_continuations import function
from gen_native_metal_formulas import tokens
ROOT=Path(__file__).resolve().parents[1]
class NormalSampleSourceTests(unittest.TestCase):
 @classmethod
 def setUpClass(cls):
  cls.source=(ROOT/'src/render/native_metal/general_normals.metal.h').read_text();cls.code=generate_sample(cls.source)
 def test_source_guard(self):
  with self.assertRaises(ValueError):generate_sample(self.source+'\n')
 def test_both_returns_are_original(self):
  parts=regions(self.source)
  for name,tail in [('normal_sample_de_finish',parts[2]),('normal_sample_direct_finish',parts[6])]:
   self.assertEqual(function(self.code,name),tokens('GeneralNormalSample out{};'+tail))
 def test_default_maximum_for_de(self):
  self.assertIn('e={}',self.code);self.assertNotIn('e.max_iterations=',self.code)
 def test_direct_original_replay(self):
  self.assertIn('orbit_replay_advance<0,false>(r,o,step,w,b,p,lane,resources,s.fault,nullptr,0xffffffffu)',self.code)
 def test_first_ordered_context_is_opt_in(self):
  self.assertIn('template<bool OrderedFirst=false>',self.code)
  self.assertIn('OrderedFirst&&driver==Hybrid3D?',self.code)
  self.assertIn('orbit_replay_advance<0,false,false,false,3>',self.code)
 def test_first_four_context_is_scoped(self):
  self.assertIn('OrderedFirst&&driver==Hybrid4D&&native_formula_id0==(kInternalFormulaSelector|2u)?',self.code)
  self.assertIn('orbit_replay_advance<0,false,false,false,4>',self.code)
 def test_restore_before_yield(self):
  for field in ('smoothing','range_exits','carried_reads'):
   self.assertLess(self.code.index('resources . '+field+' = previous_'),self.code.index('if(child!=99)'))
 def test_no_persisted_pointer(self):self.assertNotIn('s.previous=',self.code)
 def test_original_observer_calls(self):
  code=generate_observer(self.source);self.assertIn('general_de (',code);self.assertIn('native_run_orbit (',code)
  self.assertEqual(code.count('observer . record'),4);self.assertNotIn('_advance',code)
 def test_math_flags_unchanged(self):
  for forbidden in ('volatile','fma(','fast-math','fp-contract','noinline'):
   self.assertNotIn(forbidden,self.code)
if __name__=='__main__':unittest.main()
