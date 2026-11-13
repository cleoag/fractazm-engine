#!/usr/bin/env python3
"""Persistent GPU accumulation, isolation and old/new controls. Per-request 60 s guard."""
import argparse, hashlib, json, os, pathlib, select, statistics, struct, subprocess, time
from PIL import Image
p=argparse.ArgumentParser();p.add_argument('--app',type=pathlib.Path,required=True);p.add_argument('--old-app',type=pathlib.Path,required=True);p.add_argument('--cameras',type=pathlib.Path);p.add_argument('--output',type=pathlib.Path,required=True)
a=p.parse_args();a.output.mkdir(parents=True,exist_ok=False)
exe=a.app/'Contents/MacOS/MandelbulbViewer';res=a.app/'Contents/Resources';old=a.old_app/'Contents/MacOS/MandelbulbViewer'
scene=pathlib.Path('comparison/results/viewer-log-diagnosis-20261101/fault-camera.m3p')
def request(scene=scene,mode='points',edge=163):
 return json.loads(subprocess.check_output([str(exe),'--request',str(scene),str(res/'runtime'),'metal',str(edge),mode],timeout=60))
log=(a.output/'service.log').open('wb');job=subprocess.Popen([str(exe),'--service'],stdin=subprocess.PIPE,stdout=subprocess.PIPE,stderr=log,bufsize=0)
records=[]
def read(size,deadline):
 data=bytearray()
 while len(data)<size:
  timeout=deadline-time.monotonic();assert timeout>0 and select.select([job.stdout],[],[],timeout)[0], 'request deadline'
  chunk=os.read(job.stdout.fileno(),size-len(data));assert chunk, 'unexpected service EOF';data+=chunk
 return bytes(data)
def run(q,name,success=True):
 start=time.monotonic();payload=json.dumps(q).encode();packet=struct.pack('<III',0x4d425633,len(payload),0)+payload
 # Requests fit within the bounded protocol; write fully even with unbuffered pipes.
 view=memoryview(packet)
 while view:
  n=os.write(job.stdin.fileno(),view);assert n>0;view=view[n:]
 magic,n,m=struct.unpack('<III',read(12,start+60));assert magic==0x4d425633 and n<1048576 and m<=1600*1600*3
 info=json.loads(read(n,start+60));rgb=read(m,start+60);elapsed=time.monotonic()-start
 assert (info['status']=='completed')==success,info
 assert info['pid']==job.pid
 if success:assert info['backend']=='metal' and not info['fallback'] and info['faults']==0 and len(rgb)==info['width']*info['height']*3
 else:assert not rgb
 records.append(dict(name=name,wall_seconds=elapsed,rgb_sha256=hashlib.sha256(rgb).hexdigest(),**info))
 (a.output/'checks.json').write_text(json.dumps(records,indent=2)+'\n')
 return info,rgb
def mix(x):
 x^=x>>16;x=(x*0x7feb352d)&0xffffffff;x^=x>>15;x=(x*0x846ca68b)&0xffffffff;return x^(x>>16)
try:
 q=request();denseq=dict(q,mode='geometry');info,dense=run(denseq,'dense')
 w,h=info['width'],info['height'];assert (w,h)==(163,122)
 # Every intermediate pixel equals its most recent measured sample. All previous
 # exact pixels persist. Edge cells skip exhausted phases; work sums to w*h.
 for mode,block in [('points',4),('grid',2)]:
  q=dict(q,mode=mode,phase=0);expected=bytearray(w*h*3);count=0;sum_samples=0
  for phase in range(block*block):
   info,rgb=run(dict(q,phase=phase),f'{mode}-{phase}')
   samples=0
   for oy in range(0,h,block):
    for ox in range(0,w,block):
     cw=min(block,w-ox);ch=min(block,h-oy);area=cw*ch;cell=(oy//block)*((w+block-1)//block)+ox//block
     if phase>=area:continue
     start=mix(cell^0x4d423344)%area if mode=='points' else 0
     slot=(start+phase)%area;sx=ox+slot%cw;sy=oy+slot//cw;color=dense[(sy*w+sx)*3:(sy*w+sx)*3+3]
     if phase==0:
      for y in range(oy,oy+ch):
       for x in range(ox,ox+cw):expected[(y*w+x)*3:(y*w+x)*3+3]=color
     else:expected[(sy*w+sx)*3:(sy*w+sx)*3+3]=color
     samples+=1
   count+=samples;sum_samples+=info['samples']
   assert rgb==expected and info['covered']==count and info['samples']==samples
   assert info['device_creations']==1
   if phase in [0,3,7,15]:Image.frombytes('RGB',(w,h),rgb).save(a.output/f'{mode}-{phase}.png')
  assert rgb==dense and sum_samples==w*h and info['covered']==w*h
 # Wrong phase or a changed snapshot/root must fail, then phase 0 recovers.
 run(dict(q,root=None),'invalid-schema',False)
 q=dict(q,mode='points',phase=0);_,first=run(q,'reset-a')
 run(dict(q,phase=2),'wrong-phase',False)
 run(dict(q,phase=1),'poisoned-after-wrong-phase',False)
 _,reset=run(q,'reset-b');assert reset==first
 run(dict(q,phase=1,root=q['root']+'/changed'),'changed-root',False)
 run(q,'reset-c')
 other=request(res/'demo.m3p');run(dict(other,phase=1),'changed-camera',False)
 run(other,'new-camera');run(dict(other,phase=1),'new-camera-phase1')
 # Full production rendering retains the v2 RGB, three exact repeats each.
 for name,path in [('outside',res/'demo.m3p'),('inside',scene)]:
  target=a.output/f'old-{name}.png';meta=a.output/f'old-{name}.json'
  subprocess.run([str(old),'--worker',str(path),str(res/'runtime'),'metal','480',str(target),str(meta),'full'],timeout=60,check=True)
  expected=Image.open(target).tobytes();old_info=json.loads(meta.read_text());full=request(path,'full',480)
  for repeat in range(3):
   info,rgb=run(full,f'{name}-full-{repeat}');assert rgb==expected
   for key in ['samples','ao_samples','ao_active','faults','engine']:assert info[key]==old_info[key],(key,info,old_info)
 # Retained navigation camera sequence in one process, then exact repeats.
 if a.cameras:
  cameras=sorted(a.cameras.glob('*/scene.m3p'));assert cameras
  for i,path in enumerate(cameras):
   target=a.output/f'camera-old-{i}.png';meta=a.output/f'camera-old-{i}.json'
   subprocess.run([str(old),'--worker',str(path),str(res/'runtime'),'metal','480',str(target),str(meta),'full'],timeout=60,check=True)
   expected=Image.open(target).tobytes();full=request(path,'full',480)
   for repeat in range(3):assert run(full,f'camera-{i}-{repeat}')[1]==expected
  print(f'{len(cameras)} retained cameras, three exact persistent-service repeats each',flush=True)
 # Identical frozen first-phase quality, same-camera old/new PNG bytes.
 q=request(edge=640)
 target=a.output/'old-points.png';meta=a.output/'old-points.json'
 subprocess.run([str(old),'--worker',str(scene),str(res/'runtime'),'metal','640',str(target),str(meta),'points'],timeout=60,check=True)
 assert run(q,'new-points-control')[1]==Image.open(target).tobytes()
 # Balanced transport benchmark after warming both paths, same threads/camera.
 old_times=[];new_times=[]
 for repeat in range(8):
  def old_frame():
   start=time.monotonic();subprocess.run([str(old),'--worker',str(scene),str(res/'runtime'),'metal','640',str(target),str(meta),'points'],timeout=60,check=True);old_times.append(time.monotonic()-start)
  def new_frame():
   run(q,f'roundtrip-{repeat}');new_times.append(records[-1]['wall_seconds'])
  for f in ([old_frame,new_frame] if repeat%2==0 else [new_frame,old_frame]):f()
 # Accumulate the full 640x480 view with the same process, no device re-creation.
 start=time.monotonic();samples=0
 for phase in range(16):
  info,rgb=run(dict(q,phase=phase),f'full-resolution-phase-{phase}');samples+=info['samples']
 assert samples==640*480 and info['covered']==samples and info['device_creations']==1
 accumulation=time.monotonic()-start
 denseq=dict(q,mode='geometry',phase=0);assert run(denseq,'full-resolution-dense')[1]==rgb
 target=a.output/'old-dense.png';meta=a.output/'old-dense.json'
 subprocess.run([str(old),'--worker',str(scene),str(res/'runtime'),'metal','640',str(target),str(meta),'geometry'],timeout=60,check=True)
 assert rgb==Image.open(target).tobytes() and samples==json.loads(meta.read_text())['samples']
 expected_final=rgb
 for repeat in range(2):
  for phase in range(16):info,rgb=run(dict(q,phase=phase),f'accum-repeat-{repeat}-{phase}')
  assert rgb==expected_final and info['covered']==640*480
 Image.frombytes('RGB',(640,480),rgb).save(a.output/'accumulated.png')
 summary=dict(pid=job.pid,requests=len(records),clipped_grid_and_points_exact=True,full_frame_old_new_repeats=6,device_creations=info['device_creations'],samples_to_dense=samples,
  old_spawn_png_median_seconds=statistics.median(old_times),persistent_rgb_roundtrip_median_seconds=statistics.median(new_times),accumulation_16_phases_seconds=accumulation,
  note='One scene, different transport but identical first-phase RGB; includes Python request/validation bookkeeping. Not UI FPS or scene qualification.')
 (a.output/'summary.json').write_text(json.dumps(summary,indent=2)+'\n');print(json.dumps(summary,indent=2))
finally:
 job.stdin.close()
 try:job.wait(timeout=3)
 except subprocess.TimeoutExpired:job.kill();job.wait()
 log.close()
