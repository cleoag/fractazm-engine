#!/usr/bin/env python3
"""Check distance cut ownership and expression extraction independently of GPU tests."""
import subprocess,unittest
from pathlib import Path
from gen_native_distance_continuations import generate_distance,generate_distance_observer
from gen_native_metal_formulas import tokens
ROOT=Path(__file__).resolve().parents[1]
def contains(a,b):return any(a[i:i+len(b)]==b for i in range(len(a)-len(b)+1))
class DistanceRegions(unittest.TestCase):
 @classmethod
 def setUpClass(cls):
  cls.source=subprocess.check_output(['git','show','fc00c78e02b1aba530e611a07f2bf72cc5dbd099:src/render/native_metal/orbit_driver.metal.h'],cwd=ROOT,text=True)
 def test_source_binding(self):
  for generator in [generate_distance,generate_distance_observer]:
   with self.assertRaises(ValueError):generator(self.source+'\n')
   with self.assertRaises(ValueError):generator((ROOT/'src/render/native_metal/orbit_driver.metal.h').read_text())
 def test_restore_sample_before_fault_return(self):
  v=tokens(generate_distance(self.source))
  self.assertTrue(contains(v,tokens('s.c[axis]=original;if(fault.code)return distance_phase(d,99);')))
 def test_restore_orbit_after_gradient(self):
  v=tokens(generate_distance(self.source))
  self.assertTrue(contains(v,tokens('s.rout=radius;s.iteration=iterations;s.max_iterations=max_iterations;s.calc_smooth=smooth;s.rstop=p.radius_stop;')))
 def test_retry_guards_preserved(self):
  v=tokens(generate_distance(self.source))
  for code in ['if(retried||steps<=1)', 'native_range_exit(resources);limit=0;iterations=0;', 'native_range_exit(resources);s.max_iterations=--steps;']:
   self.assertTrue(contains(v,tokens(code)))
 def test_mathematical_expressions_preserved(self):
  v=tokens(generate_distance(self.source));old=tokens(self.source)
  for code in ['s.rstop_d=stop*p.stop_scale;', 'const float difference=center-s.rout;', 'delta[axis]=mb3d_native_gradient::normalize(difference,sample);', 'value=mb3d_native_gradient::distance(center,p.de_scale,delta,p.de_offset_floor);', 'value*=p.de_scale;', 'const auto interior=mb3d_inside::transform_de(value,stop,s.iteration,driver,p.de_option,calc_inside);']:
   self.assertTrue(contains(old,tokens(code)));self.assertTrue(contains(v,tokens(code)))
 def test_no_work_reset_in_retries(self):
  code=generate_distance(self.source);self.assertNotIn('fault.work',code)
  self.assertEqual(code.count('out={};out.state=lane;'),1)
  self.assertNotIn('native_run_orbit',code)
 def test_observer_retains_four_original_call_sites(self):
  v=tokens(generate_distance_observer(self.source));self.assertEqual(v.count('distance_observed_orbit'),4)
  self.assertEqual(v.count('for'),2)
 def test_no_arithmetic_policy_change(self):
  for code in [generate_distance(self.source),generate_distance_observer(self.source)]:
   for forbidden in ['fma(', 'volatile', 'fast::', '#pragma clang fp']:
    self.assertNotIn(forbidden,code)
if __name__=='__main__':unittest.main()
