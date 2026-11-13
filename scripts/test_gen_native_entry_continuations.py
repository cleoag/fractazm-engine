#!/usr/bin/env python3
import unittest
from pathlib import Path
from gen_native_entry_continuations import generate_entry,generate_original,generate_observer,regions
from gen_native_orbit_continuations import function
from gen_native_metal_formulas import tokens
ROOT=Path(__file__).resolve().parents[1]
class EntrySourceTests(unittest.TestCase):
 @classmethod
 def setUpClass(cls):
  cls.source=(ROOT/'src/render/native_metal/general_geometry.metal.h').read_text();cls.code=generate_entry(cls.source)
 def test_control_is_original_body(self):
  self.assertEqual(function(generate_original(self.source),'entry_original'),function(self.source,'general_de_inline'))
 def test_changed_source_rejected(self):
  with self.assertRaises(ValueError):generate_entry(self.source+'\n')
 def test_restore_precedes_pending_return(self):
  for target in ('smoothing','range_exits','carried_reads'):
   self.assertLess(self.code.index('resources . '+target+' = previous_'),self.code.index('if(child!=99)'))
 def test_no_pointer_persistence(self):
  self.assertNotIn('e.previous=',self.code);self.assertNotIn('reinterpret_cast',self.code)
 def test_work_and_fault_mapping_are_original(self):
  original=tokens(regions(self.source)[4]);expected=original[:-3] # original terminal return result;
  self.assertEqual(function(self.code,'entry_finish'),expected)
 def test_child_retained_before_accounting(self):
  self.assertIn('e.child=c.result;return entry_phase(e,2)',self.code)
  self.assertIn('if(e.header.pc==2){entry_finish(e.child,record,e.range_clamps)',self.code)
 def test_original_observer_not_cps(self):
  observed=generate_observer(self.source)
  self.assertIn('native_distance_combined',observed);self.assertNotIn('combined_advance',observed)
  self.assertEqual(observed.count('observer.record('),2)
 def test_no_arithmetic_switch(self):
  for forbidden in ('volatile','fma(','fast-math','fp-contract','noinline'):
   self.assertNotIn(forbidden,self.code)
if __name__=='__main__':unittest.main()
