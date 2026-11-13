#!/usr/bin/env python3
import subprocess,unittest
from pathlib import Path
from gen_native_primary_trace import capture,original_observed,candidate_observed
from gen_native_primary_continuations import generate_primary
from gen_native_metal_formulas import tokens,matching
ROOT=Path(__file__).resolve().parents[1]
class PrimaryTrace(unittest.TestCase):
 @classmethod
 def setUpClass(cls):
  cls.source=subprocess.check_output(['git','show','fc00c78e02b1aba530e611a07f2bf72cc5dbd099:src/render/native_metal/general_geometry.metal.h'],cwd=ROOT,text=True)
  cls.original=original_observed(cls.source);cls.candidate=candidate_observed(cls.source)
 def test_six_paired_boundary_sites(self):
  for site in range(1,7):
   for phase in [1,2]:
    expected=tokens(capture(site,phase))
    for code in [self.original,self.candidate]:
     v=tokens(code);self.assertEqual(sum(v[i:i+len(expected)]==expected for i in range(len(v))),1)
 def test_candidate_changes_only_copy_hooks_and_signature(self):
  # Direct per-hook removal is less permissive than arithmetic normalization.
  s=self.candidate
  for site in range(1,7):
   for phase in [1,2]:s=s.replace(capture(site,phase),'')
  s=s.replace('template<class Observer> inline uint primary_advance(', 'inline uint primary_advance(').replace('thread NativeResources& resources,thread Observer& observer){','thread NativeResources& resources){')
  for pc,to in [(1,2),(49,30)]:s=s.replace(f'if(n.header.pc=={pc}){{return primary_phase(f,{to},point,active);}}',f'if(n.header.pc=={pc})return primary_phase(f,{to},point,active);')
  s=s.replace('if(de_limited){return primary_phase(f,40,point,active);}', 'if(de_limited)return primary_phase(f,40,point,active);')
  self.assertEqual(tokens(s),tokens(generate_primary(self.source)))
 def test_observed_original_preserves_complete_source_body(self):
  from gen_native_orbit_continuations import function
  from gen_native_continuations import locate
  v=tokens(self.original)
  while 'observer' in v:
   i=v.index('observer');end=matching(v,i+3,'(',')');del v[i:end+2]
  i=locate(v,tokens('if(!camera.cut_miss){'));opening=i+len(tokens('if(!camera.cut_miss)'));closing=matching(v,opening);del v[closing];del v[opening]
  i=locate(v,tokens('else{const bool primary_refined='));call=i+len(tokens('else{const bool primary_refined='));end=v.index(';',call)
  tail=tokens('if(!primary_refined)break;}');self.assertEqual(v[end+1:end+1+len(tail)],tail)
  v[i:end+1+len(tail)]=tokens('else if(!')+v[call:end]+tokens(')break;')
  self.assertEqual(v,function(self.source,'general_primary_camera_attempt'))
 def test_initial_snapshot_does_not_read_undeclared_values(self):
  for value in ['de','last_step','last_de','step','iteration','program_maximum','combined_maximum','step_index','ended','de_limited']:self.assertNotIn(value,tokens(capture(1,1)))
 def test_normal_snapshot_skips_ended_march_scope(self):
  for value in ['last_step','last_de','step','step_index','ended','de_limited']:self.assertNotIn(value,tokens(capture(6,1)))
 def test_only_backup_captures_live_step(self):
  for site in range(1,7):self.assertEqual('step' in tokens(capture(site,1)),site==2)
 def test_only_refinement_captures_contact_flag(self):
  for site in range(1,7):self.assertEqual('de_limited' in tokens(capture(site,1)),site in [4,5])
 def test_child_result_only_after_direct_de_return(self):
  for site in range(1,7):
   for phase in [1,2]:self.assertEqual('result' in tokens(capture(site,phase)),site in [1,2,3]and phase==2)
 def test_source_binding_remains_strict(self):
  with self.assertRaises(ValueError):candidate_observed(self.source+'\n')
  with self.assertRaises(ValueError):original_observed(self.source+'\n')
if __name__=='__main__':unittest.main()
