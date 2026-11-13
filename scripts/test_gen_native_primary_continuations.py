#!/usr/bin/env python3
import subprocess,unittest
from pathlib import Path
from gen_native_primary_continuations import regions,generate_primary
from gen_native_metal_formulas import emit_text,tokens
ROOT=Path(__file__).resolve().parents[1]
class PrimaryRegions(unittest.TestCase):
 @classmethod
 def setUpClass(cls):
  cls.source=subprocess.check_output(['git','show','fc00c78e02b1aba530e611a07f2bf72cc5dbd099:src/render/native_metal/general_geometry.metal.h'],cwd=ROOT,text=True);cls.parts=regions(cls.source);cls.code=generate_primary(cls.source)
 def test_source_binding(self):
  with self.assertRaises(ValueError):generate_primary(self.source+'\n')
 def test_complete_numeric_regions(self):
  for name in ['inside','loop_begin','backup_after','move_condition','move_before','move_after','contact_after','after_loop','pack','normal_condition','normal_after','finish']:
   value=emit_text(self.parts[name]).replace('return out;','return primary_phase(f,99,point,active);').replace('break;','return primary_phase(f,60,point,active);')
   self.assertIn(value,self.code,name)
 def test_backtrack_step_is_live_across_child(self):
  self.assertIn(emit_text(self.parts['backup_before']).replace('const float step =','step ='),self.code)
  self.assertIn('thread auto& step=n.refine_step',self.code);self.assertIn('last_step = - step',self.code)
 def test_all_march_children_keep_default_contract(self):
  expected=tokens('result=general_de(point,stop,program,resources,lane,record);')
  self.assertEqual(self.parts['backup_call'],expected);self.assertEqual(self.parts['move_call'],expected)
  self.assertEqual(self.parts['initial'],tokens('if(!camera.cut_miss)')+expected)
  self.assertIn('entry_advance<7>',self.code);self.assertIn('f.de={};',self.code)
 def test_de_refinement_returns_to_hit_commit_even_on_fault(self):
  self.assertIn('if(n.header.pc==49)return primary_phase(f,30,point,active)',self.code)
  self.assertNotIn('if(record.fault)return primary_phase(f,30',self.code)
 def test_iteration_refinement_fault_exits_march(self):
  self.assertIn('if(!refinement_success(f.refinement,record))return primary_phase(f,60,point,active)',self.code)
  self.assertIn('point=float3(f.refinement.point[0],f.refinement.point[1],f.refinement.point[2]);depth=f.refinement.depth;iteration=f.refinement.iteration',self.code)
 def test_normal_child_completes_before_packing(self):
  self.assertIn('normal_result=f.normal.result;',self.code);self.assertIn('normal_record)!=99)return 80',self.code)
  self.assertEqual(self.parts['normal_call'],tokens('normal_result=general_calculate_normals(active,depth,stop,scene,program,normal_params,resources,lane,record,normal_record);'))
 def test_unaudited_capabilities_refused_before_child(self):
  guard='if(normal_params.smoothing||scene.color_on_it||scene.fog_iteration==kNativeVolumeFog||native_primary_random())'
  self.assertIn(guard,self.code);self.assertLess(self.code.index(guard),self.code.index('primary_entry_child(f,point,stop,1)'))
 def test_camera_and_orientation_are_attempt_local(self):
  self.assertIn('const auto camera=f.camera',self.code);self.assertIn('auto normal_params=f.normal_params;normal_params.inside=uint(inside_word!=0)',self.code)
  self.assertNotIn('f.normal_params.inside=',self.code);self.assertNotIn('f.camera=',self.code)
 def test_no_arithmetic_or_compiler_barrier(self):
  for value in ['fma(','volatile','noinline','fast::','#pragma clang fp']:self.assertNotIn(value,self.code)
if __name__=='__main__':unittest.main()
