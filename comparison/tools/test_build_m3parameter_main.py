"""Checks for historical producer closure and terminal-product integrity."""
import contextlib,io,json,tempfile,unittest
from pathlib import Path
import build_m3parameter_main as b

class VersionedBuildInputs(unittest.TestCase):
 def setUp(self):
  self.tmp=tempfile.TemporaryDirectory();self.p=Path(self.tmp.name);self.build=self.p/'build';self.records=self.p/'provenance/records';self.records.mkdir(parents=True);self.build.mkdir();self.sequence=0
  source=b.ROOT/'CMakeLists.txt';self.dep={'path':str(source),'sha256':b.sha(source),'classification':'source'}
  (self.p/'source-manifest.json').write_text(json.dumps({'source_files':{'CMakeLists.txt':b.sha(source)}}));(self.p/'contract.json').write_text('{}');(self.p/'provenance/capture.py').write_text('# fixture')
  for i,name in enumerate(b.EXPECTED_AIR_PRODUCTS):self.record(name,('AIR'+str(i)).encode(),deps=True)
  self.archive=self.build/'history.a';old=self.record('history.a',b'old header, same objects')
  self.link=self.record('fixture_executable',b'executable',inputs=[{'path':str(self.archive),'sha256':old}])
  self.record('history.a',b'new header, same objects')
  self.record('metal_native_scene.metallib',b'library')
 def tearDown(self):self.tmp.cleanup()
 def record(self,name,data,deps=False,inputs=None):
  p=self.build/name;p.write_bytes(data);self.sequence+=1;n=self.sequence
  r={'exit_code':0,'capture_errors':[],'start_time_ns':2*n,'end_time_ns':2*n+1,'explicit_inputs_before':inputs or [],'dependencies':[self.dep] if deps else [],'depfile':'fixture.d' if deps else None,'output':{'path':str(p),'sha256':b.sha(p)}}
  (self.records/f'{n}.json').write_text(json.dumps(r));(self.records/f'{n}.started.json').write_text('{}');return b.sha(p)
 def validate(self):
  with contextlib.redirect_stdout(io.StringIO()):b.validate(self.p,['fixture_executable'])
 def test_versioned_input_and_current_output(self):self.validate()
 def test_current_archive_tamper_rejected(self):
  self.archive.write_bytes(b'tampered')
  with self.assertRaisesRegex(RuntimeError,'Output identity mismatch'):self.validate()
 def test_old_link_input_without_matching_producer_rejected(self):
  p=self.records/'9.json';r=json.loads(p.read_text());r['explicit_inputs_before'][0]['sha256']='0'*64;p.write_text(json.dumps(r))
  with self.assertRaisesRegex(RuntimeError,'Producer hash mismatch'):self.validate()
 def test_failed_capture_rejected(self):
  p=self.records/'1.json';r=json.loads(p.read_text());r['exit_code']=1;p.write_text(json.dumps(r))
  with self.assertRaisesRegex(RuntimeError,'Failed capture'):self.validate()
 def test_interrupted_command_rejected(self):
  (self.records/'extra.started.json').write_text('{}')
  with self.assertRaisesRegex(RuntimeError,'Interrupted compiler'):self.validate()
 def test_missing_preview_air_rejected(self):
  (self.build/'navigation_preview.air').unlink()
  (self.records/'7.json').unlink();(self.records/'7.started.json').unlink()
  with self.assertRaisesRegex(RuntimeError,'exact seven'):self.validate()
 def test_same_count_wrong_air_identity_rejected(self):
  old=self.build/'navigation_preview.air';new=self.build/'unrelated.air';old.rename(new)
  p=self.records/'7.json';r=json.loads(p.read_text());r['output']['path']=str(new);p.write_text(json.dumps(r))
  with self.assertRaisesRegex(RuntimeError,'exact seven'):self.validate()
 def test_extra_air_rejected(self):
  self.record('unexpected.air',b'AIR',deps=True)
  with self.assertRaisesRegex(RuntimeError,'exact seven'):self.validate()
if __name__=='__main__':unittest.main()
