#!/usr/bin/env python3
import subprocess,unittest
from pathlib import Path
from gen_native_de_refinement_continuations import regions,generate_de_refinement,generate_control
from gen_native_metal_formulas import emit_text,tokens
from gen_native_orbit_continuations import function
ROOT=Path(__file__).resolve().parents[1]
class DeRefinementRegions(unittest.TestCase):
 @classmethod
 def setUpClass(cls):
  cls.source=subprocess.check_output(['git','show','fc00c78e02b1aba530e611a07f2bf72cc5dbd099:src/render/native_metal/general_geometry.metal.h'],cwd=ROOT,text=True);cls.parts=regions(cls.source);cls.code=generate_de_refinement(cls.source)
 def test_source_guard(self):
  with self.assertRaises(ValueError):generate_de_refinement(self.source+'\n')
 def test_complete_original_block_in_control(self):
  r=self.parts;expected=tokens('NativeOrbitResult result{};')+r['init']+r['loop']+['{']+r['before']+r['call']+r['after']+['}']
  self.assertEqual(function(generate_control(self.source),'de_refinement_control'),expected)
 def test_numeric_regions_are_verbatim(self):
  for name in ['before','after']:self.assertIn(emit_text(self.parts[name]).replace('break;','return de_refinement_phase(n,49,point);'),self.code)
 def test_last_move_precedes_exit_without_query(self):
  branch=self.code.index('if ( remaining <= 1 )');self.assertLess(self.code.index('depth += step'),branch);self.assertLess(branch,self.code.index('de_refinement_child(e,point,stop)'))
 def test_zero_default_query_contract(self):
  self.assertEqual(self.parts['call'],tokens('result=general_de(point,stop,program,resources,lane,record);'))
  self.assertIn('e={};e.header.parent_tag=uint(Tag::Primary);e.header.parent_pc=42',self.code)
  self.assertNotIn('e.smooth=1',self.code);self.assertNotIn('e.max_iterations=',self.code)
 def test_existing_primary_storage_and_distinct_context(self):
  self.assertIn('thread auto& step=n.refine_step',self.code);self.assertIn('thread auto& remaining=n.remaining',self.code);self.assertIn('entry_advance<6>',self.code)
  self.assertIn('if(!(remaining>0&&abs(de-stop)>.001f))',self.code)
 def test_no_new_fault_or_numeric_barrier(self):
  for v in ['fma(','volatile','noinline','fast::','#pragma clang fp','record.fault=']:self.assertNotIn(v,self.code)
 def test_child_completes_before_result_consumed(self):
  self.assertIn('n.child=e.child;return de_refinement_phase(n,43,point)',self.code);self.assertIn('const auto result=n.child;',self.code)
if __name__=='__main__':unittest.main()
