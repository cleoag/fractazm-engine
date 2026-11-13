#!/usr/bin/env python3
import subprocess,unittest
from pathlib import Path
from gen_native_inside_trace import capture,original_observed,candidate_observed
from gen_native_inside_continuations import generate_inside
from gen_native_orbit_continuations import function
from gen_native_metal_formulas import tokens,matching
ROOT=Path(__file__).resolve().parents[1]
class InsideTrace(unittest.TestCase):
 @classmethod
 def setUpClass(cls):
  cls.source=subprocess.check_output(['git','show','fc00c78e02b1aba530e611a07f2bf72cc5dbd099:src/render/native_metal/primary_inside_bridge.metal.h'],cwd=ROOT,text=True);cls.original=original_observed(cls.source);cls.candidate=candidate_observed(cls.source)
 def test_complete_original_is_unchanged_without_copies(self):
  v=tokens(self.original)
  while 'observer' in v:
   i=v.index('observer');end=matching(v,i+3,'(',')');del v[i:end+2]
  self.assertEqual(v,function(self.source,'general_primary_inside'))
 def test_complete_candidate_is_unchanged_without_copies(self):
  s=self.candidate
  for site in [1,2]:
   for phase in [1,2]:s=s.replace(capture(site,phase,True),'')
  s=s.replace('template<class Observer> inline uint inside_advance(', 'inline uint inside_advance(').replace('thread NativeResources& resources,thread Observer& observer){','thread NativeResources& resources){')
  self.assertEqual(tokens(s),tokens(generate_inside(self.source)))
 def test_paired_sites(self):
  for site in [1,2]:
   for phase in [1,2]:
    for candidate,code in [(False,self.original),(True,self.candidate)]:
     expected=tokens(capture(site,phase,candidate));v=tokens(code);self.assertEqual(sum(v[i:i+len(expected)]==expected for i in range(len(v))),1)
 def test_attempt_not_read_after_for_scope_ends(self):
  self.assertNotIn('attempt',tokens(capture(2,1)));self.assertIn('attempt',tokens(capture(1,1)))
 def test_smooth_point_uses_saved_child_arguments(self):
  self.assertIn('float3(f.de.point[0],f.de.point[1],f.de.point[2])',capture(2,2,True));self.assertNotIn('general_double',capture(2,2,True))
 def test_child_result_only_after_post_smooth(self):
  for site in [1,2]:
   for phase in [1,2]:self.assertEqual('result' in tokens(capture(site,phase)),site==2 and phase==2)
 def test_signed_retry_state_copied_without_normalization(self):
  self.assertIn(',state,',capture(1,1));self.assertNotIn('bool(',capture(1,1))
 def test_source_binding(self):
  with self.assertRaises(ValueError):original_observed(self.source+'\n')
  with self.assertRaises(ValueError):candidate_observed(self.source+'\n')
if __name__=='__main__':unittest.main()
