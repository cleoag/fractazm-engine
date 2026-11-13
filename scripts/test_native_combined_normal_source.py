#!/usr/bin/env python3
"""Check that the two-program diagnostic changes storage, not caller arithmetic."""
import unittest
from pathlib import Path
from gen_native_orbit_continuations import function
from gen_native_metal_formulas import tokens
ROOT=Path(__file__).resolve().parents[1]
class CombinedNormalSource(unittest.TestCase):
 def test_complete_kernels_preserved(self):
  old=(ROOT/'tests/metal/p2_normal_composition.metal').read_text()
  new=(ROOT/'tests/metal/p2_combined_normal_composition.metal').read_text().replace('in.programs[0]','in.program')
  for name in ['original','observe','advance','abi']:
   with self.subTest(kernel=name):self.assertEqual(function(old,name),function(new,name))
 def test_only_input_storage_changes(self):
  old=(ROOT/'tests/metal/p2_normal_composition.metal').read_text()
  new=(ROOT/'tests/metal/p2_combined_normal_composition.metal').read_text().replace('NativeFormulaProgram programs[2];','NativeFormulaProgram program;').replace('in.programs[0]','in.program')
  self.assertEqual(tokens(old),tokens(new))
if __name__=='__main__':unittest.main()
