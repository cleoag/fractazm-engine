#!/usr/bin/env python3
"""Validate P1 replay extraction and fault/work ownership, not GPU parity."""
import subprocess,unittest
from pathlib import Path
from gen_native_replay_continuations import generate_replay,generate_replay_observer
from gen_native_metal_formulas import tokens
ROOT=Path(__file__).resolve().parents[1]
class ReplayRegions(unittest.TestCase):
 @classmethod
 def setUpClass(cls):
  cls.source=subprocess.check_output(['git','show','fc00c78e02b1aba530e611a07f2bf72cc5dbd099:src/render/native_metal/orbit_driver.metal.h'],cwd=ROOT,text=True)
 def test_source_binding(self):
  for generator in [generate_replay,generate_replay_observer]:
   with self.assertRaises(ValueError):generator(self.source+'\n')
   with self.assertRaises(ValueError):generator((ROOT/'src/render/native_metal/orbit_driver.metal.h').read_text())
 def test_restore_preserves_fault_and_work(self):
  code=generate_replay(self.source);v=tokens(code)
  self.assertEqual(code.count('r.entry=s;'),1)
  self.assertEqual(v.count('native_range_exit'),1)
  self.assertIn('s = entry;',code);self.assertIn('pass_limit = exit_pass;',code)
  self.assertNotIn('f.work',code);self.assertNotIn('f =',code)
  self.assertNotIn('native_run_orbit',code)
  self.assertNotIn('native_orbit_passes',code)
 def test_observer_keeps_original_child_and_loop(self):
  code=generate_replay_observer(self.source);v=tokens(code)
  self.assertEqual(v.count('native_orbit_passes'),1)
  self.assertEqual(v.count('for'),1)
  self.assertIn('observer.outer(201',code);self.assertIn('observer.outer(202',code)
  self.assertNotIn('orbit_original_observed',code)
 def test_no_arithmetic_policy_change(self):
  for code in [generate_replay(self.source),generate_replay_observer(self.source)]:
   for forbidden in ['fma(', 'volatile', 'fast::', '#pragma clang fp']:
    self.assertNotIn(forbidden,code)
if __name__=='__main__':unittest.main()
