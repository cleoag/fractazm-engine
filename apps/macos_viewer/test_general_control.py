#!/usr/bin/env python3
"""Cancel active general sparse commands without dropping their service/device."""
import argparse,json,os,signal,threading,time
from pathlib import Path
from check_scenes import Service,save
p=argparse.ArgumentParser();p.add_argument('--package',type=Path,required=True);a=p.parse_args()
package=a.package.resolve();out=package/'general-control';out.mkdir(exist_ok=False)
os.environ['MB3D_VIEWER_GENERAL_PREVIEW']='1'
service=Service(package/'final/Mandelbulb Viewer.app/Contents/MacOS/MandelbulbViewer',out,'control')
row=next(x for x in json.loads((package/'inventory.json').read_text())['ordinary'] if x['rank']==56)
request=json.loads((package/'requests'/(row['id']+'.json')).read_text());request.update(edge=640,phase=0)
results=[]
def run(label,q):
 folder=out/label;folder.mkdir();r,rgb=service.run(q,label,folder,time.monotonic()+180);results.append(r);save(out/'checks.json',results);return r,rgb
try:
 first,rgb=run('warm',request);assert first['status']=='completed',first
 pid=first['pid'];devices=first['device_creations'];expected=first['rgb_sha256']
 for i in range(3):
  sent=[]
  def cancel():sent.append(time.monotonic());os.kill(pid,signal.SIGUSR1)
  timer=threading.Timer(.03,cancel);timer.start()
  try:r,rgb=run(f'cancel-{i}',dict(request,phase=1));finished=time.monotonic()
  finally:timer.join()
  assert r['status']=='cancelled' and not rgb and r['pid']==pid,r
  assert r['protocol']==1 and r['presentation']=='unpresented' and r['scene_revision']==request['scene_revision'],r
  r['signal_to_reply_seconds']=finished-sent[0]
  recovered,rgb=run(f'recover-{i}',request)
  assert recovered['status']=='completed' and recovered['pid']==pid and recovered['device_creations']==devices,recovered
  assert recovered['rgb_sha256']==expected
  assert recovered['scene_preparations']==first['scene_preparations'] and recovered['resource_uploads']==first['resource_uploads'],recovered
 save(out/'checks.json',results)
 print(json.dumps({'cancelled':3,'same_pid':pid,'device_creations':devices,'recovery_exact':True,'signal_to_reply_seconds':[r['signal_to_reply_seconds'] for r in results if 'signal_to_reply_seconds' in r]},indent=2))
finally:service.close()
