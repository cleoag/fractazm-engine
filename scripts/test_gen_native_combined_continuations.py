#!/usr/bin/env python3
"""Source/ownership checks for P1 DE combination, separate from GPU parity."""
import subprocess,unittest
from pathlib import Path
from gen_native_combined_continuations import generate_combined,generate_combined_observer,observer_numeric_regions
from gen_native_metal_formulas import tokens
ROOT=Path(__file__).resolve().parents[1]
def contains(a,b):return any(a[i:i+len(b)]==b for i in range(len(a)-len(b)+1))
class CombinedRegions(unittest.TestCase):
 @classmethod
 def setUpClass(cls):
  cls.source=subprocess.check_output(['git','show','fc00c78e02b1aba530e611a07f2bf72cc5dbd099:src/render/native_metal/orbit_driver.metal.h'],cwd=ROOT,text=True)
 def test_source_binding(self):
  for generator in [generate_combined,generate_combined_observer]:
   with self.assertRaises(ValueError):generator(self.source+'\n')
   with self.assertRaises(ValueError):generator((ROOT/'src/render/native_metal/orbit_driver.metal.h').read_text())
 def test_distinct_query_owners_and_merge(self):
  code=generate_combined(self.source)
  self.assertIn('distance_advance<0,true,EntryContext>',code);self.assertIn('distance_advance<1,true,EntryContext>',code)
  self.assertEqual(code.count('out.fault.work+=first.fault.work;'),1)
  self.assertTrue(contains(tokens(code),tokens('out=d.result;out.fault.work+=first.fault.work;if(out.fault.code||!out.completed)')))
 def test_single_query_does_not_bind_second_program(self):
  code=generate_combined(self.source)
  self.assertEqual(code.count('(&p)[1]'),3)
  self.assertGreater(code.index('(&p)[1]'),code.index('if(c.header.pc==3)'))
 def test_second_maximum_preserved(self):
  self.assertTrue(contains(tokens(generate_combined(self.source)),tokens('second_maximum=q.max_iterations+(temporary_maximum&&q.de_option!=20?1:0);')))
 def test_combination_math_preserved(self):
  v=tokens(generate_combined(self.source));old=tokens(self.source)
  for code in ['value=min(other-max(0.f,k-value),value-max(0.f,k-other));', 'value=min(other-first_cut*(k-second_cut)/k,value-second_cut*(k-first_cut)/k);', 's.smooth=(s.smooth*weight+a.smooth*other_weight)*inverse;', 's.otrap=(s.otrap*weight+a.otrap*other_weight)*inverse;', 'value=other/abs(a.derivative[0]);', 'value=other/abs(a.xyz[3]);', 'value=other*pow(abs(p.mix_power),-1.f-float(a.iteration));']:
   self.assertTrue(contains(old,tokens(code)));self.assertTrue(contains(v,tokens(code)))
 def test_observer_keeps_three_original_call_sites(self):
  v=tokens(generate_combined_observer(self.source));self.assertEqual(v.count('combined_observed_distance'),4)
  self.assertTrue(contains(v,tokens('out.fault.work+=first.fault.work;')))
 def test_observer_keeps_original_loops_and_atomic_calls(self):
  code=observer_numeric_regions(self.source)
  self.assertEqual(tokens(code).count('atomic_step_lane'),3)
  self.assertEqual(tokens(code).count('do'),1)
  self.assertTrue(contains(tokens(code),tokens('s=entry;pass_limit=exit_pass;if(!inert)native_range_exit(resources);')))
  for forbidden in ['orbit_pass_advance','orbit_replay_advance','distance_advance','wrapper_advance','body_advance']:
   self.assertNotIn(forbidden,code)
 def test_no_arithmetic_policy_change(self):
  for code in [generate_combined(self.source),generate_combined_observer(self.source)]:
   for forbidden in ['fma(', 'volatile', 'fast::', '#pragma clang fp']:
    self.assertNotIn(forbidden,code)
if __name__=='__main__':unittest.main()
