#!/usr/bin/env python3
"""Pinned live-camera regressions and spatial preview checks, never corpus credit.
Requires Pillow from the ordinary tooling dependencies. Every child has a real
60-second guard; source/app inputs must remain frozen throughout this run.
"""
import argparse, hashlib, json, pathlib, statistics, subprocess
from PIL import Image
p=argparse.ArgumentParser();p.add_argument('--app',type=pathlib.Path,required=True);p.add_argument('--old-app',type=pathlib.Path,required=True);p.add_argument('--cameras',type=pathlib.Path,required=True);p.add_argument('--output',type=pathlib.Path,required=True)
a=p.parse_args();a.output.mkdir(parents=True,exist_ok=False);records=[]
def run(scene,name,mode='full',edge=480,old=False):
 app=a.old_app if old else a.app
 output=a.output/f'{name}.png';meta=a.output/f'{name}.json'
 cmd=[str(app/'Contents/MacOS/MandelbulbViewer'),'--worker',str(scene),str(app/'Contents/Resources/runtime'),'metal',str(edge),str(output),str(meta)]
 if not old:cmd+=[mode]
 with (a.output/f'{name}.log').open('w') as log:r=subprocess.run(cmd,stdout=log,stderr=subprocess.STDOUT,timeout=60)
 value=json.loads(meta.read_text());value.update(name=name,exit_code=r.returncode,command=cmd,scene_sha256=hashlib.sha256(scene.read_bytes()).hexdigest())
 if value.get('status')=='completed':
  assert value['backend']=='metal' and not value['fallback'],value
  value['rgb_sha256']=hashlib.sha256(Image.open(output).tobytes()).hexdigest()
 records.append(value)
 (a.output/'checks.json').write_text(json.dumps(records,indent=2)+'\n')
 return value
cameras=sorted(a.cameras.glob('*/scene.m3p'))
assert cameras, 'No retained camera snapshots'
for i,scene in enumerate(cameras):
 values=[run(scene,f'camera{i:02}-{j}') for j in range(3)]
 assert all(v['status']=='completed' for v in values),values
 assert len({v['rgb_sha256'] for v in values})==1
 assert all(v.get('faults')==0 for v in values)
 print(f'camera {i+1}/{len(cameras)}: full-size three exact GPU repeats',flush=True)
# Preserve complete effects and verify that AO has an active contribution.
scene=pathlib.Path('comparison/results/viewer-log-diagnosis-20261101/fault-camera.m3p')
full=run(scene,'ao-on');noao=run(scene,'ao-off','noao')
assert full['ao_samples']>0 and full['ao_active']>0 and noao['ao_samples']==0
assert full['rgb_sha256']!=noao['rgb_sha256']
# Old/new valid outside frame must remain identical, on original dimensions.
demo=a.app/'Contents/Resources/demo.m3p'
old=run(demo,'outside-old',old=True);new=[run(demo,f'outside-new-{i}') for i in range(3)]
assert old['status']=='completed' and all(v['rgb_sha256']==old['rgb_sha256'] for v in new)
# Sparse values must equal the exact dense geometric pixel at full camera coordinates.
for mode in ['geometry','grid','points']:assert run(scene,'sparse-'+mode,mode,163)['status']=='completed'
dense=Image.open(a.output/'sparse-geometry.png');width,height=dense.size
for mode,block in [('grid',2),('points',4)]:
 sparse=Image.open(a.output/f'sparse-{mode}.png');cells=(width+block-1)//block
 for oy in range(0,height,block):
  for ox in range(0,width,block):
   cw=min(block,width-ox);ch=min(block,height-oy);slot=0
   if mode=='points':
    value=((oy//block)*cells+ox//block)^0x4d423344
    value^=value>>16;value=(value*0x7feb352d)&0xffffffff;value^=value>>15;value=(value*0x846ca68b)&0xffffffff;value^=value>>16
    slot=value%(cw*ch)
   expected=dense.getpixel((ox+slot%cw,oy+slot//cw))
   assert all(sparse.getpixel((x,y))==expected for y in range(oy,oy+ch) for x in range(ox,ox+cw))
# Balanced order, same 640x480 camera, separate shaders and effect sets labelled.
for mode in ['full','draft','grid','points']:run(scene,'warmup-'+mode,mode,640)
for repeat,order in enumerate([['full','draft','grid','points'],['points','grid','draft','full'],['full','draft','grid','points'],['points','grid','draft','full']]):
 for mode in order:assert run(scene,f'timing{repeat}-{mode}',mode,640)['status']=='completed'
summary={'camera_count':len(cameras),'full_size_exact_repeats':len(cameras)*3,'outside_old_new_exact':True,'ao_control_active':True,'sparse_full_camera_pixel_checks':True,'timing_medians':{}}
for mode in ['full','draft','grid','points']:
 values=[v for v in records if v['name'].startswith('timing') and v['name'].endswith('-'+mode)]
 assert all(v['archive_misses']==0 and v['archive_hits']>0 for v in values),values
 summary['timing_medians'][mode]={'seconds':statistics.median(v['seconds'] for v in values),'samples':values[0]['samples'],'quality':values[0]['preview_mode'],'archive':'warm'}
(a.output/'summary.json').write_text(json.dumps(summary,indent=2)+'\n');print(json.dumps(summary,indent=2),flush=True)
