import copy
import tempfile
import unittest
from pathlib import Path
import run_m3parameter_gpu as r

def profile():
 stages={k:{'state':'not_run','seconds':0} for k in ['load','preparation','volume','primary','normals','shadows','ao','color','reflections','dof','final_surface','save']}
 for key in ['load','preparation','primary','color','final_surface','save']:stages[key]={'state':'completed','seconds':.01}
 return dict(status='complete',render_status='completed',exit_code=0,error='',backend='metal',backend_requested='metal',backend_fallback=False,backend_fallback_code='',backend_fallback_reason='',precision='native',numeric_profile='native_metal_fp32',width=2,height=2,stages=stages,schema='mb3d.render-stage-profile.v1',workers=0,metal_device='test',metal_execution=dict(validated=True,library_profile=[2,4,32],engine='general_formula',primary_records=4,normal_records=4,surface_records=8,color_records=4,faults=0,nonfinite=0,work_exhausted=0),metal_reflections=dict(faults=0))
class Validation(unittest.TestCase):
 def test_secondary_work_repeat_fields(self):
  p=profile();p['metal_reflections'].update(ray_steps=21,max_command_seconds=.1)
  first=r.validate_profile(p,'metal',[2,2])
  p['metal_reflections']['max_command_seconds']=.2
  self.assertEqual(first,r.validate_profile(p,'metal',[2,2]))
  p['metal_reflections']['ray_steps']=22
  self.assertNotEqual(first,r.validate_profile(p,'metal',[2,2]))
 def test_floating_work_is_not_timing(self):
  p=profile();p['metal_execution']['dof_maximum_radius']=1.25
  first=r.validate_profile(p,'metal',[2,2]);p['metal_execution']['dof_maximum_radius']=1.5
  self.assertNotEqual(first,r.validate_profile(p,'metal',[2,2]))
  p['metal_execution']['dof_maximum_radius']=float('nan')
  with self.assertRaises(ValueError):r.validate_profile(p,'metal',[2,2])
 def test_valid_gpu(self):r.validate_profile(profile(),'metal',[2,2])
 def test_cpu_fallback(self):
  p=profile();p.update(backend='cpu',backend_fallback=True)
  with self.assertRaises(ValueError):r.validate_profile(p,'metal',[2,2])
 def test_missing_fallback_field(self):
  p=profile();del p['backend_fallback']
  with self.assertRaises(ValueError):r.validate_profile(p,'metal',[2,2])
 def test_silent_fallback_reason(self):
  p=profile();p['backend_fallback_reason']='unsupported'
  with self.assertRaises(ValueError):r.validate_profile(p,'metal',[2,2])
 def test_wrong_precision(self):
  p=profile();p['precision']='compatible'
  with self.assertRaises(ValueError):r.validate_profile(p,'metal',[2,2])
 def test_wrong_marker(self):
  p=profile();p['metal_execution']['library_profile']=[1,8,64]
  with self.assertRaises(ValueError):r.validate_profile(p,'metal',[2,2])
 def test_missing_work(self):
  p=profile();del p['metal_execution']['primary_records']
  with self.assertRaises(ValueError):r.validate_profile(p,'metal',[2,2])
 def test_wrong_work_count(self):
  p=profile();p['metal_execution']['primary_records']=3
  with self.assertRaises(ValueError):r.validate_profile(p,'metal',[2,2])
 def test_faults(self):
  for key in ['faults','nonfinite','work_exhausted']:
   p=profile();p['metal_execution'][key]=1
   with self.subTest(key=key),self.assertRaises(ValueError):r.validate_profile(p,'metal',[2,2])
 def test_unvalidated(self):
  p=profile();p['metal_execution']['validated']=False
  with self.assertRaises(ValueError):r.validate_profile(p,'metal',[2,2])
 def test_missing_primary_stage(self):
  p=profile();p['stages']['primary']['state']='not_run'
  with self.assertRaises(ValueError):r.validate_profile(p,'metal',[2,2])
 def test_unknown_stage(self):
  p=profile();p['stages']['unknown']={'state':'completed','seconds':0}
  with self.assertRaises(ValueError):r.validate_profile(p,'metal',[2,2])
 def test_nan_time(self):
  p=profile();p['stages']['primary']['seconds']=float('nan')
  with self.assertRaises(ValueError):r.validate_profile(p,'metal',[2,2])
 def test_wrong_dimensions(self):
  with self.assertRaises(ValueError):r.validate_profile(profile(),'metal',[4,4])
 def test_path_escape(self):
  with self.assertRaises(ValueError):r.artifact(Path('/tmp/bound'), '../escape')
 def test_deferred_screen(self):
  with tempfile.TemporaryDirectory() as tmp:
   p=Path(tmp)/'loaded.bin';data=bytearray(1976);data[124]=1;p.write_bytes(data)
   with self.assertRaises(ValueError):r.loaded_scope(p)
   data[124]=0;data[860:863]=b'JiT';p.write_bytes(data)
   with self.assertRaises(ValueError):r.loaded_scope(p)
if __name__=='__main__':unittest.main()
