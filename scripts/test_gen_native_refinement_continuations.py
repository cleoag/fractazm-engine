#!/usr/bin/env python3
"""Audit source regions and live fields; these checks do not prove numeric parity."""
import subprocess,unittest
from pathlib import Path
from gen_native_refinement_continuations import regions,generate_refinement
from gen_native_metal_formulas import tokens,emit_text
from gen_native_orbit_continuations import function
ROOT=Path(__file__).resolve().parents[1]
class RefinementRegions(unittest.TestCase):
 @classmethod
 def setUpClass(cls):
  cls.source=subprocess.check_output(['git','show','fc00c78e02b1aba530e611a07f2bf72cc5dbd099:src/render/native_metal/general_geometry.metal.h'],cwd=ROOT,text=True)
  cls.code=generate_refinement(cls.source);cls.parts=regions(cls.source)
 def test_source_binding(self):
  with self.assertRaises(ValueError):generate_refinement(self.source+'\n')
 def test_calls_have_original_temporary_contract(self):
  for name in ['call_first','call_second']:
   self.assertIn('general_de ( point , stop , program , resources , lane , record , true , temporary_max , true )',emit_text(self.parts[name]))
  self.assertIn('e.smooth=1;e.max_iterations=maximum;e.temporary_maximum=1',self.code)
 def test_complete_loop_regions_preserved(self):
  for name in ['move_first','after_first','high','move_second','after_second','finish']:
   text=emit_text(self.parts[name]).replace('return false;','return refinement_phase(n,99,point);')
   self.assertIn(text,self.code)
 def test_no_numeric_or_compiler_barrier(self):
  for value in ['fma(','volatile','fast::','noinline','#pragma clang fp']:self.assertNotIn(value,self.code)
 def test_point_saved_before_child_return(self):
  self.assertIn('refinement_child(e,point,stop,temporary_max,2);return refinement_phase(n,2,point)',self.code)
  self.assertIn('refinement_child(e,point,stop,temporary_max,4);return refinement_phase(n,4,point)',self.code)
 def test_original_false_returns_always_have_fault(self):
  v=function(self.source,'general_refine_iteration_count');self.assertEqual(emit_text(v).count('return false;'),4)
  self.assertEqual(emit_text(v).count('record . fault'),4)
  self.assertIn('return n.header.pc==99&&!record.fault',self.code)
 def test_child_is_initialized_once_and_context_is_explicit(self):
  self.assertIn('e={};e.header.parent_tag=uint(Tag::Refinement)',self.code)
  self.assertIn('entry_advance<5>',self.code);self.assertNotIn('general_de(',self.code)
 def test_live_scalar_types_and_input_iteration_preserved(self):
  self.assertIn('thread auto& temporary_max=n.temporary_max',self.code)
  self.assertNotIn('iteration=0',self.code)
  for declaration in ['const float target =','const int temporary_max =','float step =','bool first =']:self.assertNotIn(declaration,self.code)
if __name__=='__main__':unittest.main()
