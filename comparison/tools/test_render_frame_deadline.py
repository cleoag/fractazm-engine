"""Real process checks for the ordinary single-frame render deadline."""
import os
import json,subprocess,sys,time
import tempfile
import unittest
from pathlib import Path
from unittest.mock import patch
import run_m3parameter_gpu as r

@unittest.skipUnless(os.name=='posix','POSIX process-group runner')
class Deadline(unittest.TestCase):
 def executable(self,root,name,seconds):
  p=root/name;p.write_text('#!/bin/sh\nexec /bin/sleep '+str(seconds)+'\n');p.chmod(0o700);return p
 def test_long_render_request_is_capped_and_reaped(self):
  with tempfile.TemporaryDirectory() as tmp:
   root=Path(tmp);exe=self.executable(root,'mb3d_render',30)
   with patch.object(r,'FRAME_DIAGNOSE_SECONDS',.01),patch.object(r,'FRAME_STOP_SECONDS',.1):
    event=r.invoke([exe],root/'run',root,3600)
   self.assertEqual(event['guard'],'RENDER_FRAME_TIMEOUT')
   self.assertEqual(event['requested_timeout_seconds'],3600)
   self.assertEqual(event['effective_timeout_seconds'],.1)
   self.assertTrue(event['frame_warning_emitted'])
   self.assertNotEqual(event['exit_code'],0)
   with self.assertRaises(ProcessLookupError):os.kill(event['pid'],0)
 def test_shorter_render_deadline_still_applies(self):
  with tempfile.TemporaryDirectory() as tmp:
   root=Path(tmp);exe=self.executable(root,'mb3d_render.exe',30)
   event=r.invoke([exe],root/'run',root,.05)
   self.assertEqual(event['guard'],'TIMEOUT')
   self.assertEqual(event['effective_timeout_seconds'],.05)
 def test_build_command_keeps_its_own_deadline(self):
  with tempfile.TemporaryDirectory() as tmp:
   root=Path(tmp);exe=self.executable(root,'build_tool',.1)
   with patch.object(r,'FRAME_STOP_SECONDS',.01):event=r.invoke([exe],root/'run',root,2)
   self.assertEqual(event['exit_code'],0)
   self.assertIsNone(event['guard'])
  self.assertEqual(event['effective_timeout_seconds'],2)
  self.assertNotIn('frame_diagnostic_threshold_seconds',event)
 def test_explicit_input_captured_before_execution(self):
  with tempfile.TemporaryDirectory() as tmp:
   root=Path(tmp);exe=self.executable(root,'read_tool',.01);data=root/'data';data.write_bytes(b'original')
   event=r.invoke([exe],root/'run',root,2,evidence_root=root,input_files=[data])
   self.assertEqual(event['inputs'],[{'path':'data','sha256':r.sha(data)}]);self.assertIsNone(event['guard'])
 def test_explicit_input_mutation_rejected(self):
  with tempfile.TemporaryDirectory() as tmp:
   root=Path(tmp);data=root/'data';data.write_bytes(b'original');exe=root/'change_tool'
   exe.write_text('#!/bin/sh\nprintf changed > data\n');exe.chmod(0o700)
   event=r.invoke([exe],root/'run',root,2,evidence_root=root,input_files=[data])
   self.assertEqual(event['guard'],'INPUT_CHANGED')
 def test_gpu_slot_wait_is_outside_frame_clock(self):
  import fcntl
  with tempfile.TemporaryDirectory() as tmp:
   root=Path(tmp);exe=self.executable(root,'mb3d_render',.01);lock=root/'comparison/runs/.gpu-execution.lock'
   lock.parent.mkdir(parents=True)
   code='import sys;sys.path.insert(0,sys.argv[1]);from run_m3parameter_gpu import invoke;invoke([sys.argv[2],"--backend","metal"],sys.argv[3],sys.argv[4],2,evidence_root=sys.argv[4])'
   with lock.open('a+b') as slot:
    fcntl.flock(slot,fcntl.LOCK_EX)
    child=subprocess.Popen([sys.executable,'-B','-c',code,str(Path(r.__file__).parent),str(exe),str(root/'run'),str(root)])
    try:
     time.sleep(.3);self.assertFalse((root/'run').exists());released=time.time_ns();fcntl.flock(slot,fcntl.LOCK_UN)
     self.assertEqual(child.wait(timeout=5),0)
    finally:
     if child.poll() is None:child.kill();child.wait()
   event=json.loads((root/'run/event.json').read_text())
   self.assertGreaterEqual(event['started_ns'],released)
   self.assertGreater(event['gpu_queue_wait_seconds'],.05)

if __name__=='__main__':unittest.main()
