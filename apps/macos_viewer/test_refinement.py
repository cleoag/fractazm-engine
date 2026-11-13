#!/usr/bin/env python3
"""Two-service refinement/cancellation checks; every request has a real deadline."""
import argparse,json,os,pathlib,select,statistics,struct,subprocess,time,hashlib
p=argparse.ArgumentParser();p.add_argument('--app',type=pathlib.Path,required=True);p.add_argument('--output',type=pathlib.Path,required=True)
a=p.parse_args();a.output.mkdir(parents=True,exist_ok=False)
exe=a.app/'Contents/MacOS/MandelbulbViewer';root=a.app/'Contents/Resources/runtime'
scene='comparison/results/viewer-log-diagnosis-20261101/fault-camera.m3p'
base=json.loads(subprocess.check_output([str(exe),'--request',scene,str(root),'metal','640','points'],timeout=60))
records=[]
class Client:
 def __init__(self,name):
  self.log=(a.output/(name+'.log')).open('wb');self.job=subprocess.Popen([str(exe),'--service'],stdin=subprocess.PIPE,stdout=subprocess.PIPE,stderr=self.log,bufsize=0)
 def send(self,q):
  b=json.dumps(q).encode();v=memoryview(struct.pack('<III',0x4d425633,len(b),0)+b)
  while v:n=os.write(self.job.stdin.fileno(),v);assert n>0;v=v[n:]
 def receive(self):
  deadline=time.monotonic()+60
  def read(n):
   b=bytearray()
   while len(b)<n:
    t=deadline-time.monotonic();assert t>0 and select.select([self.job.stdout],[],[],t)[0]
    part=os.read(self.job.stdout.fileno(),n-len(b));assert part;b+=part
   return bytes(b)
  magic,n,m=struct.unpack('<III',read(12));assert magic==0x4d425633 and n<1048576 and m<=1600*1600*3
  info=json.loads(read(n));rgb=read(m)
  assert info['status']=='completed' and info['backend']=='metal' and not info['fallback'] and info['faults']==0,info
  assert info['pid']==self.job.pid
  return info,rgb
 def run(self,q,name):
  start=time.monotonic();self.send(q);info,rgb=self.receive()
  records.append(dict(name=name,wall_seconds=time.monotonic()-start,rgb_sha256=hashlib.sha256(rgb).hexdigest(),**info))
  return info,rgb
 def close(self):
  if self.job.poll() is None:self.job.stdin.close()
  try:self.job.wait(timeout=3)
  except subprocess.TimeoutExpired:self.job.kill();self.job.wait(timeout=3)
  self.log.close()
preview=Client('preview');full=Client('full');full_request=dict(base,mode='full',phase=0)
try:
 expected=None
 for repeat in range(3):
  for phase in range(16):info,rgb=preview.run(dict(base,phase=phase),f'accum-{repeat}-{phase}')
  assert info['covered']==640*480
  info,rgb=full.run(full_request,f'full-after-accum-{repeat}')
  assert info['ao_samples']>0 and info['ao_active']>0
  if expected is None:expected=rgb
  assert rgb==expected
 # Interrupt a real full-effects request, then send to the already-warm preview
 # lane BEFORE waiting for the full process to exit.
 cancel_times=[]
 for repeat in range(3):
  full.send(full_request);time.sleep(.06)
  assert not select.select([full.job.stdout],[],[],0)[0], 'Full fixture finished before cancellation'
  start=time.monotonic();full.job.kill()
  info,rgb=preview.run(base,f'preview-after-cancel-{repeat}')
  cancel_times.append(time.monotonic()-start)
  assert info['phase']==0 and info['covered']==19200 and info['device_creations']==1
  full.close();full=Client(f'full-recreated-{repeat}')
 info,rgb=full.run(full_request,'full-after-final-idle');assert rgb==expected
 summary=dict(complete_accumulation_then_effects_repeats=3,active_ao=True,cancelled_full_requests=3,
  preview_pid=preview.job.pid,preview_device_creations=1,
  cancel_to_preview_seconds=cancel_times,median_cancel_to_preview_seconds=statistics.median(cancel_times),
  note='Cancellation-to-RGB reply; excludes UI display. Warm preview process survives; only full lane is recreated.')
 (a.output/'checks.json').write_text(json.dumps(records,indent=2)+'\n');(a.output/'summary.json').write_text(json.dumps(summary,indent=2)+'\n')
 print(json.dumps(summary,indent=2))
finally:preview.close();full.close()
