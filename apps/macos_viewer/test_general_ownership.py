#!/usr/bin/env python3
"""Same-artifact ordered/pixel ownership RGB+work controls on all ordinary scenes."""
import argparse,json,os,time
from pathlib import Path
from check_scenes import Service,save
p=argparse.ArgumentParser();p.add_argument('--package',type=Path,required=True);p.add_argument('--mode',choices=('points','fastgeometry'),default='points');a=p.parse_args();package=a.package.resolve()
out=package/('ownership' if a.mode=='points' else 'ownership-'+a.mode);out.mkdir(exist_ok=False);exe=package/'final/Mandelbulb Viewer.app/Contents/MacOS/MandelbulbViewer'
os.environ['MB3D_VIEWER_GENERAL_PREVIEW']='1'
clients={k:Service(exe,out,k) for k in ('ordered','selected')};results=[]
def run(lane,q,folder,label):
 if lane=='ordered':os.environ['MB3D_VIEWER_ORDERED_CONTROL']='1'
 else:os.environ.pop('MB3D_VIEWER_ORDERED_CONTROL',None)
 dest=folder/label;dest.mkdir();r,rgb=clients[lane].run(q,label,dest,time.monotonic()+180);return r
try:
 for row in json.loads((package/'inventory.json').read_text())['ordinary']:
  q=json.loads((package/'requests'/(row['id']+'.json')).read_text());q.update(edge=64,phase=0,mode=a.mode)
  folder=out/row['id'];folder.mkdir();case={'rank':row['rank'],'scene':row['scene'],'checks':[]};results.append(case)
  for phase in range(3):
   q['phase']=phase
   old=run('ordered',q,folder,f'ordered-{phase}');new=run('selected',q,folder,f'selected-{phase}')
   case['checks'].append({'ordered':old,'selected':new})
   assert old['status']==new['status'],case
   if new['status']!='completed':
    assert old.get('error')==new.get('error'),case
    break
   assert old['rgb_sha256']==new['rgb_sha256'] and old['formula_work']==new['formula_work'] and old['samples']==new['samples'],case
  if case['checks'][0]['selected']['status']=='completed':
   expected=case['checks'][0]['selected']
   for i in range(2):
    repeat=run('selected',dict(q,phase=0),folder,f'repeat-{i}');case['checks'].append({'repeat':repeat})
    assert repeat['rgb_sha256']==expected['rgb_sha256'] and repeat['formula_work']==expected['formula_work'],case
  case['passed']=True;save(out/'checks.json',results);print(row['rank'],'PASS',case['checks'][0]['selected'].get('engine',case['checks'][0]['selected']['status']),flush=True)
finally:
 for client in clients.values():client.close()
 os.environ.pop('MB3D_VIEWER_ORDERED_CONTROL',None)
