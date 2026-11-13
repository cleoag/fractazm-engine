#!/usr/bin/env python3
import subprocess,unittest
from pathlib import Path
from gen_native_inside_continuations import regions,generate_inside
from gen_native_metal_formulas import tokens,emit_text
ROOT=Path(__file__).resolve().parents[1]
class InsideRegions(unittest.TestCase):
 @classmethod
 def setUpClass(cls):
  cls.source=subprocess.check_output(['git','show','fc00c78e02b1aba530e611a07f2bf72cc5dbd099:src/render/native_metal/primary_inside_bridge.metal.h'],cwd=ROOT,text=True);cls.parts=regions(cls.source);cls.code=generate_inside(cls.source)
 def test_source_binding(self):
  with self.assertRaises(ValueError):generate_inside(self.source+'\n')
 def test_complete_owner_regions_preserved(self):
  for name in ['before_attempt','after_attempt','after_loop','smooth_condition','before_smooth','after_smooth','finish']:
   self.assertIn(emit_text(self.parts[name]).replace('return pixel;','return inside_phase(n,99);').replace('break;','return inside_phase(n,3);'),self.code)
 def test_attempt_restarts_primary_not_camera_or_lane(self):
  self.assertIn('f.primary={};',self.code);self.assertIn('f.primary.inside_word=state.inside',self.code)
  for field in ['f.camera=','f.lane=','f.geometry=','f.normal_params=']:self.assertNotIn(field,self.code)
 def test_signed_inside_word_is_not_booleanized(self):
  self.assertNotIn('bool(state.inside)',self.code);self.assertNotIn('uint(state.inside)',self.code)
  self.assertIn('thread auto& state=n.retry',self.code)
 def test_at_most_two_attempts_and_advance_after_retry(self):
  self.assertIn('if(!(attempt<2))return inside_phase(n,3)',self.code)
  self.assertLess(self.code.index('mb3d_inside :: retry'),self.code.index('++attempt'))
 def test_post_smooth_default_maximum_and_no_temporary_query(self):
  self.assertEqual(self.parts['smooth_call'],tokens('const auto result=general_de(point,as_type<float>(pixel.final_stop),program,resources,lane,geometry,true);'))
  self.assertIn('f.de={};',self.code);self.assertIn('f.de.smooth=1;',self.code);self.assertIn('entry_advance<8>',self.code)
  self.assertNotIn('f.de.temporary_maximum=',self.code)
 def test_post_smooth_keeps_earlier_trap_and_radius(self):
  self.assertNotIn('pixel . orbit_trap =',self.code);self.assertNotIn('geometry . initial_rout =',self.code)
  self.assertIn('pixel . smooth_iterations = as_type < uint > ( result . state . smooth )',self.code)
 def test_child_finishes_before_owner_consumes_result(self):
  self.assertIn('if(primary_advance(f,scene,program,resources)!=99)return 2',self.code)
  self.assertIn('const auto result=f.de.child;',self.code)
 def test_no_arithmetic_or_compiler_change(self):
  for value in ['fma(','volatile','noinline','fast::','#pragma clang fp']:self.assertNotIn(value,self.code)
if __name__=='__main__':unittest.main()
