#!/usr/bin/env python3
"""Check source binding and preservation of arithmetic regions, not GPU parity."""
import hashlib,subprocess,unittest
from pathlib import Path
from gen_native_metal_formulas import tokens,matching
from gen_native_continuations import locate
from gen_native_orbit_continuations import generate_orbit,generate_step,generate_observer,function,ORBIT_SHA
ROOT=Path(__file__).resolve().parents[1]
def contains(haystack,needle):return any(haystack[i:i+len(needle)]==needle for i in range(len(haystack)-len(needle)+1))
class OrbitRegions(unittest.TestCase):
 @classmethod
 def setUpClass(cls):
  cls.source=subprocess.check_output(['git','show','fc00c78e02b1aba530e611a07f2bf72cc5dbd099:src/render/native_metal/orbit_driver.metal.h'],cwd=ROOT,text=True)
 def test_p1_binding(self):
  self.assertEqual(hashlib.sha256(self.source.encode()).hexdigest(),ORBIT_SHA)
  for generator in [generate_orbit,generate_step,generate_observer]:
   with self.assertRaises(ValueError):generator(self.source+'\n')
   with self.assertRaises(ValueError):generator((ROOT/'src/render/native_metal/orbit_driver.metal.h').read_text())
 def test_initial_rotation_expression_preserved(self):
  needle=tokens('s.xyz[i]=((s.c[0]*p.rotation[i][0]+s.c[1]*p.rotation[i][1])+s.c[2]*p.rotation[i][2]);')
  self.assertTrue(contains(tokens(self.source),needle));self.assertTrue(contains(tokens(generate_orbit(self.source)),needle))
 def test_first_four_binding_preserves_complete_initial_arithmetic(self):
  old=function(self.source,'native_orbit_passes');start=tokens('if(driver!=HybridDifsNoInitialization)')
  expected=old[locate(old,start):locate(old,tokens('uint slot='))]
  output=generate_orbit(self.source);actual=function(output,'orbit_initial_first_four')
  actual=actual[locate(actual,start):locate(actual,tokens('return s;'))]
  bound=tokens('if(four)for(uint order=0;order<4;++order){const uint indices[4]={1,0,2,3};const uint i=indices[order];')
  i=locate(actual,bound);end=actual.index(';',i+len(bound))+1
  self.assertEqual(actual[end],'}')
  actual[i:end+1]=tokens('if(four)for(uint i=0;i<4;++i)')+actual[i+len(bound):end]
  radius=tokens('ordered_radius({s.xyz[1],s.xyz[0],s.xyz[2],s.xyz[3]},four)');i=locate(actual,radius)
  actual[i:i+len(radius)]=tokens('native_radius(s,four)')
  self.assertEqual(actual,expected)
  self.assertIn('EntryContext==4&&driver==Hybrid4D&&native_formula_id0==(kInternalFormulaSelector|2u)',output)
 def test_blend_and_normalization_expressions_preserved(self):
  for text in ['s.xyz[i]=(i==3&&analytic&&!four)?abs(first[i])*a+abs(s.xyz[i])*b:first[i]*a+s.xyz[i]*b;',
               's.derivative[0]=abs(first_derivative)*a+abs(s.derivative[0])*b;']:
   needle=tokens(text);self.assertTrue(contains(tokens(self.source),needle));self.assertTrue(contains(tokens(generate_orbit(self.source)),needle))
 def test_normalization_arithmetic_and_radius_binding(self):
  original=tokens('float factor=target/sqrt(native_radius(s,four)+1e-40f);')
  self.assertTrue(contains(tokens(self.source),original))
  output=generate_orbit(self.source);v=tokens(output)
  sites=[i for i in range(len(v)-2) if v[i:i+3]==['float','factor','=']]
  self.assertEqual(len(sites),2)
  for i in sites:
   self.assertEqual(v[i+3:i+7],tokens('target/sqrt('))
   end=matching(v,i+6,'(',')')
   self.assertEqual(v[end-2:end+2],tokens('+1e-40f);'))
   radius=v[i+7:end-2]
   self.assertTrue(contains(radius,tokens('native_radius(s,four)')))
   self.assertTrue(contains(radius,tokens('ordered_radius({s.xyz[1],s.xyz[0],s.xyz[2],s.xyz[3]},four)')))
  # Both bindings use the original radius expression with the same operands.
  radius=function(output,'ordered_radius')
  self.assertTrue(contains(radius,tokens('float r=(x*x+y*y)+z*z;return four?r+w*w:r;')))
 def test_no_orbit_or_body_replay_to_reconstruct_checkpoint(self):
  output=generate_orbit(self.source)
  self.assertNotIn('native_orbit_passes',output);self.assertNotIn('native_run_orbit',output)
 def test_observer_retains_original_loop_and_calls(self):
  v=tokens(generate_observer(self.source));old=function(self.source,'native_orbit_passes')
  self.assertEqual(v.count('native_step'),old.count('native_step'))
  self.assertEqual(v.count('do'),1);self.assertEqual(v.count('while'),1)
 def test_no_arithmetic_switch_or_rounding_barrier(self):
  for output in [generate_step(self.source),generate_orbit(self.source),generate_observer(self.source)]:
   for forbidden in ['fma(', 'volatile', 'fast::', 'noinline', '#pragma clang fp']:
    self.assertNotIn(forbidden,output)
 def test_ordered_radius_preserves_complete_source_expression(self):
  actual=function(generate_orbit(self.source),'ordered_radius')
  # Only bind the same coordinates in Y/X/Z/W member order; the full source
  # expression, grouping and branch are compared after removing copy aliases.
  actual=actual[actual.index(';')+1:]
  replacements={name:tokens('s.xyz['+str(i)+']') for i,name in enumerate(['x','y','z','w'])}
  restored=[]
  for token in actual:restored.extend(replacements.get(token,[token]))
  self.assertEqual(restored,function(self.source,'native_radius'))
 def test_step_increments_work_once(self):
  output=tokens(generate_step(self.source));needle=tokens('++f.work;')
  self.assertEqual(sum(output[i:i+len(needle)]==needle for i in range(len(output))),1)
  self.assertIn('atomic_step_lane',output)
if __name__=='__main__':unittest.main()
