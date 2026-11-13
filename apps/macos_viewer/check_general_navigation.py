#!/usr/bin/env python3
"""Audit opt-in ordered preview on the frozen ordinary corpus (never qualification).

Checks coverage and deterministic replay at 64 px, then sparse navigation at 640.
The 180 s guard covers the entire small accumulation, including IPC. Full/effects
are deliberately not dispatched; previously slow full frames must not be retried.
"""
import argparse
import hashlib
import json
import os
from pathlib import Path
import time
from PIL import Image
from check_scenes import Service, save

p=argparse.ArgumentParser(description=__doc__)
p.add_argument('--package',type=Path,required=True)
p.add_argument('--resume',action='store_true')
p.add_argument('--mode',choices=('points','fastgeometry'),default='points')
p.add_argument('--baseline',type=Path,help='Compare every RGB/work/coverage result with a frozen prior audit')
a=p.parse_args(); package=a.package.resolve()
inventory=json.loads((package/'inventory.json').read_text())
if a.baseline:
 old_package=a.baseline.resolve()
 old_inventory=json.loads((old_package/'inventory.json').read_text())
 assert inventory['excluded_sha256']==old_inventory['excluded_sha256']
 assert [{k:r[k] for k in ('id','sha256','resource_profile')} for r in inventory['ordinary']]==[
  {k:r[k] for k in ('id','sha256','resource_profile')} for r in old_inventory['ordinary']]
 assert json.loads((package/'resources/resources.json').read_text())==json.loads((old_package/'resources/resources.json').read_text())
 for row in inventory['ordinary']:
  for path in [Path('requests')/(row['id']+'.json'),*[Path('moved-requests')/(row['id']+'-'+motion+'.json') for motion in ('orbit','fly','pan')]]:
   new_q=json.loads((package/path).read_text());old_q=json.loads((old_package/path).read_text())
   new_q.pop('root');old_q.pop('root')
   # Added transport identities do not alter the frozen numerical snapshot.
   for key in ('protocol','request_id','scene_revision','camera_revision','quality_revision','epoch','input_seconds','submit_seconds'):
    new_q.pop(key,None);old_q.pop(key,None)
   assert new_q==old_q,path
exe=package/'final/Mandelbulb Viewer.app/Contents/MacOS/MandelbulbViewer'
out=package/('general-audit' if a.mode=='points' else 'general-audit-'+a.mode);out.mkdir(exist_ok=a.resume)
os.environ['MB3D_VIEWER_GENERAL_PREVIEW']='1'
service=Service(exe,out,'general')
report={'qualification':False,'experimental':True,'mode':a.mode,'exe_sha256':hashlib.sha256(exe.read_bytes()).hexdigest(),
        'scope':'77 ordinary scenes, 64 px accumulation, 640 px sparse + orbit/fly/pan; no Full',
        'excluded':inventory['excluded'],'rows':[]}
if a.resume:
 report=json.loads((out/'report.json').read_text())
 complete=[r for r in report['rows'] if r.get('audit_complete') or (out/r['id']/'case.json').exists()]
 for r in report['rows']:
  if r not in complete:
   (out/r['id']).rename(out/(r['id']+'-interrupted'))
 report['rows']=complete
 report['runner_note']='Resumed after replacing quadratic report rewrites with an append-only phase journal; completed scenes retained.'
completed={r['id'] for r in report['rows']}
rows=sorted(inventory['ordinary'],key=lambda r:(r['priority']=='deferred_slow_tail',r['rank']))
try:
 for number,row in enumerate(rows,1):
  if row['id'] in completed:continue
  assert hashlib.sha256((package/'inputs'/Path(row['scene']).name).read_bytes()).hexdigest()==row['sha256']
  request=json.loads((package/'requests'/(row['id']+'.json')).read_text())
  request['mode']=a.mode
  folder=out/row['id'];folder.mkdir();record=dict(row,stages=[]);report['rows'].append(record)
  prior={}
  if a.baseline:
   old_folder=a.baseline.resolve()/out.name/row['id']
   prior={r['label']:r for r in map(json.loads,(old_folder/'phases.jsonl').read_text().splitlines())}
  print(f"START {number}/77 {row['rank']} {row['scene']}",flush=True)
  def run(label,q,deadline=None,keep_image=True):
   dest=folder/label;dest.mkdir()
   result,rgb=service.run(q,label,dest,deadline or time.monotonic()+180)
   with (folder/'phases.jsonl').open('a') as journal:journal.write(json.dumps(result)+'\n')
   if keep_image or result['status']!='completed':record['stages'].append(result)
   if rgb and keep_image:
    Image.frombytes('RGB',(result['width'],result['height']),rgb).save(dest/'image.png')
    result['image']=str((dest/'image.png').relative_to(package))
   if keep_image or result['status']!='completed':save(out/'report.json',report)
   if a.baseline:
    old=prior.pop(label)
    assert old['request']==result['request'] and old['status']==result['status'],(row['rank'],label,old,result)
    keys=('rgb_sha256','formula_work','samples','covered','width','height','phases','engine') if result['status']=='completed' else ('error',)
    assert all(old.get(k)==result.get(k) for k in keys),(row['rank'],label,{k:(old.get(k),result.get(k)) for k in keys})
   if keep_image or result['status']!='completed':print(f"  {label}: {result['status']} {result['wall_seconds']:.3f}s {result.get('error','')}",flush=True)
   return result
  started=time.monotonic(); first=run('small-0000',dict(request,edge=64,phase=0))
  if first['status']=='completed':
   covered=first['covered'];last=first
   for phase in range(1,first['phases']):
    last=run(f'small-{phase:04}',dict(request,edge=64,phase=phase),started+180,keep_image=phase==first['phases']-1)
    if last['status']!='completed':break
    assert last['covered']==covered+last['samples'] and last['covered']<=last['width']*last['height']
    covered=last['covered']
   record['small_accumulation_complete']=last['status']=='completed' and covered==last['width']*last['height']
   record['small_accumulation_wall_seconds']=time.monotonic()-started
   print(f"  accumulation complete={record['small_accumulation_complete']} {record['small_accumulation_wall_seconds']:.3f}s",flush=True)
   # Do not launch larger diagnostics after a small accumulation reaches its guard.
   if record['small_accumulation_complete']:
    repeat=run('small-repeat',dict(request,edge=64,phase=0));record['small_repeat_exact']=repeat.get('rgb_sha256')==first.get('rgb_sha256')
    assert record['small_repeat_exact']
    repeat2=run('small-repeat-2',dict(request,edge=64,phase=0));assert repeat2.get('rgb_sha256')==first.get('rgb_sha256')
    large=run('navigation-0',dict(request,edge=640,phase=0))
    if large['status']=='completed':
     run('navigation-1',dict(request,edge=640,phase=1))
     again=run('navigation-repeat',dict(request,edge=640,phase=0));record['large_repeat_exact']=again.get('rgb_sha256')==large.get('rgb_sha256');assert record['large_repeat_exact']
     again2=run('navigation-repeat-2',dict(request,edge=640,phase=0));assert again2.get('rgb_sha256')==large.get('rgb_sha256')
     for motion in ('orbit','fly','pan'):
      moved=json.loads((package/'moved-requests'/(row['id']+'-'+motion+'.json')).read_text())
      moved['mode']=a.mode
      run(motion,moved)
  record['audit_complete']=True
  if a.baseline:
   assert not prior,(row['rank'],'missing baseline stages',list(prior))
   record['baseline_exact']=True
   report['baseline']=str(a.baseline.resolve())
  save(folder/'case.json',record);save(out/'report.json',report)
finally:service.close()
print('DONE',len(report['rows']),flush=True)
