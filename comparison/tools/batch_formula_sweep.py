"""Serial Pascal batch runner, gated by exact-RGB qualification against separate processes.
Every requested case is retained. A failed/timed-out batch retries missing images
in separate processes. Batch mode never runs two Pascal renderers concurrently.
"""
import argparse,json,os,subprocess,time,hashlib
from collections import Counter
from dataclasses import asdict
from pathlib import Path
from external_validation import ROOT,write_json,digest
from image_diff import DEFAULT_THRESHOLDS,load_rgb,compute_metrics,evaluate
from sweep import run_render
from compatibility_precision import precision_evidence, verify_precision_evidence
from validate_monte_carlo import source_provenance
from pascal_formula_identity import validate_formula_load


def retry_pascal_with_identity(exe, scene, image, listing, timeout_ms):
 """A fresh process still needs batch logging to identify loaded formulas.

 An empty continuation list renders exactly the requested scene. Calling the
 ordinary GUI-subsystem entry without this flag loses stdout on Windows, so a
 successful image alone cannot qualify its formula identity.
 """
 listing.write_text('', encoding='utf-8')
 env=dict(os.environ);env['MB3D_HEADLESS_BATCH']=str(listing.resolve())
 image.unlink(missing_ok=True)
 command=[str(exe.resolve()),'--render',str(scene.resolve()),'--output',str(image.resolve()),'--threads','1']
 start=time.monotonic()
 try:
  result=subprocess.run(command,cwd=exe.parent,env=env,capture_output=True,timeout=timeout_ms/1000)
  return result.returncode,round((time.monotonic()-start)*1000),(result.stdout+b'\n'+result.stderr).decode('cp1252',errors='replace')
 except subprocess.TimeoutExpired as error:
  return -1,round((time.monotonic()-start)*1000),((error.stdout or b'')+b'\nTIMEOUT\n'+(error.stderr or b'')).decode('cp1252',errors='replace')

def validate_checkpoint(report, identity, cases, output):
 for key in ('pascal_sha256','cpp_sha256','manifest_sha256','qualification_sha256','thresholds','timeouts'):
  if report.get(key)!=identity[key]:raise ValueError('Resume identity changed: '+key)
 results=report['results']
 if len(results)>len(cases):raise ValueError('Checkpoint exceeds manifest')
 for item,c in zip(results,cases):
  if item['scene']!=c['id'] or item['scene_sha256']!=c['sha256']:raise ValueError('Checkpoint order/scene changed')
  for kind in ('pascal','cpp'):
   recorded=item.get(kind+'_image_sha256')
   if item['status'] in ('pass','fail') and not recorded:raise ValueError('Missing image identity')
   if recorded and digest((output/kind/(c['id']+'.png')).read_bytes())!=recorded:raise ValueError('Checkpoint image changed: '+item['scene'])
  if item['status'] in ('pass','fail'):verify_precision_evidence(output/'cpp'/(c['id']+'.png'),item)
 return len(results)

def main():
 ap=argparse.ArgumentParser(description=__doc__);ap.add_argument('--manifest',type=Path,required=True);ap.add_argument('--pascal-exe',type=Path,required=True);ap.add_argument('--cpp-exe',type=Path,required=True);ap.add_argument('--output',type=Path,required=True);ap.add_argument('--qualification',type=Path);ap.add_argument('--qualify-reference',type=Path);ap.add_argument('--batch-size',type=int,default=32);ap.add_argument('--resume',action='store_true');ap.add_argument('--render-timeout-ms',type=int,default=15000);ap.add_argument('--batch-timeout-seconds',type=int,default=45);a=ap.parse_args()
 if a.render_timeout_ms<1 or a.batch_timeout_seconds<1:ap.error('Timeouts must be positive')
 # Resume is explicit: existing directories otherwise remain protected.
 a.output.mkdir(parents=True,exist_ok=a.resume);pas=a.pascal_exe.resolve();cpp=a.cpp_exe.resolve();manifest=json.loads(a.manifest.read_text());cases=manifest['cases'];references={}
 if a.qualify_reference:
  for p in sorted(a.qualify_reference.rglob('summary.json')):
   old=json.loads(p.read_text())
   if not old.get('ended'):continue
   for c in old['results']:
    image=p.parent/'pascal'/(c['scene']+'.png')
    if image.exists() and not c['status'].endswith('error'):references[c['scene']]={'image':str(image.resolve()),'scene_sha256':c.get('scene_sha256'),'summary':str(p.resolve()),'summary_sha256':digest(p.read_bytes())}
  candidates=[c for c in cases if c['id'] in references and c['sha256']==references[c['id']]['scene_sha256']]
  if len(candidates)<50:raise ValueError('At least 50 independent reference images are required')
  cases=[candidates[i*len(candidates)//50] for i in range(50)]
 else:
  if not a.qualification:raise ValueError('Qualification is required')
  q=json.loads(a.qualification.read_text())
  if not q.get('qualified') or q['pascal_sha256']!=digest(pas.read_bytes()):raise ValueError('Batch binary lacks exact independent-process qualification')
 report={'scope':__doc__,'provenance':source_provenance(),'pascal_sha256':digest(pas.read_bytes()),'cpp_sha256':digest(cpp.read_bytes()),'manifest_sha256':digest(a.manifest.read_bytes()),'thresholds':DEFAULT_THRESHOLDS,'timeouts':{'render_ms':a.render_timeout_ms,'batch_seconds':a.batch_timeout_seconds},'qualification_sha256':digest(a.qualification.read_bytes()) if a.qualification else None,'results':[],'batches':[],'finished':False}
 if a.batch_size<1:raise ValueError('Batch size must be positive')
 for c in cases:
  if digest((ROOT/c['scene']).read_bytes())!=c['sha256']:raise ValueError('Changed scene '+c['id'])
 start_offset=0
 if a.resume:
  if a.qualify_reference:raise ValueError('Qualification must run without resuming')
  saved=json.loads((a.output/'summary.json').read_text())
  start_offset=validate_checkpoint(saved,report,cases,a.output)
  report=saved
  report.setdefault('resumes',[]).append({'completed':start_offset,'time':time.time()})
  print('Verified checkpoint:',start_offset,'/',len(cases),flush=True)
 for offset in range(start_offset,len(cases),a.batch_size):
  group=cases[offset:offset+a.batch_size];pairs=[]
  for c in group:
   scene=(ROOT/c['scene']).resolve()
   if digest(scene.read_bytes())!=c['sha256']:raise ValueError('Changed scene '+str(scene))
   image=(a.output/'pascal'/(c['id']+'.png')).resolve();image.parent.mkdir(parents=True,exist_ok=True)
   # An uncommitted image can be partial after a reboot. Render it afresh.
   image.unlink(missing_ok=True)
   (a.output/'cpp'/(c['id']+'.png')).unlink(missing_ok=True)
   pairs.append((scene,image))
  listing=a.output/f'batch-{offset:05d}.tsv';listing.write_text(''.join(str(s)+'\t'+str(p)+'\n' for s,p in pairs[1:]),encoding='utf-8')
  env=dict(os.environ);env['MB3D_HEADLESS_BATCH']=str(listing.resolve());start=time.monotonic()
  command=[str(pas),'--render',str(pairs[0][0]),'--output',str(pairs[0][1]),'--threads','1']
  batch={'command':command,'listing_sha256':digest(listing.read_bytes()),'cases':[c['id'] for c in group]}
  with (a.output/f'batch-{offset:05d}.log').open('w') as log:
   try:r=subprocess.run(command,cwd=pas.parent,env=env,stdout=log,stderr=subprocess.STDOUT,timeout=a.batch_timeout_seconds);batch['returncode']=r.returncode
   except subprocess.TimeoutExpired:batch['error']='TIMEOUT'
  batch['ms']=round((time.monotonic()-start)*1000);report['batches'].append(batch)
  batch_log_path=a.output/f'batch-{offset:05d}.log'
  batch_log=batch_log_path.read_text(errors='replace')
  if digest(pas.read_bytes())!=report['pascal_sha256'] or digest(cpp.read_bytes())!=report['cpp_sha256']:raise ValueError('Binary changed during run')
  for c,(scene,image) in zip(group,pairs):
   item={'scene':c['id'],'scene_sha256':c['sha256'],'target_formulas':c['target_formulas'],'batch':offset,'status':'error'}
   try:
    reference_log=batch_log;reference_log_path=batch_log_path
    if not image.exists():
     retry_listing=a.output/(c['id']+'-pascal-retry.tsv')
     code,elapsed,log=retry_pascal_with_identity(pas,scene,image,retry_listing,a.render_timeout_ms);item['retry_ms']=elapsed
     item['retry_listing_sha256']=digest(retry_listing.read_bytes())
     reference_log=log;reference_log_path=a.output/(c['id']+'-pascal-retry.log')
     reference_log_path.write_text(log,encoding='utf-8')
     if code:raise RuntimeError('Pascal retry: '+log)
    item['pascal_log']=reference_log_path.name
    item['pascal_log_sha256']=digest(reference_log_path.read_bytes())
    item['pascal_formula_load']=validate_formula_load(reference_log,scene,c['slots'])
    reference=load_rgb(image);item['pascal_image_sha256']=digest(image.read_bytes())
    if a.qualify_reference:
     ref=references[c['id']];item['independent_reference']=ref;actual=load_rgb(Path(ref['image']))
    else:
     target=a.output/'cpp'/(c['id']+'.png');code,elapsed,log=run_render(cpp,scene,target,1,a.render_timeout_ms,[]);item['cpp_ms']=elapsed
     if code:raise RuntimeError('C++: '+log)
     item.update(precision_evidence(target))
     actual=load_rgb(target);item['cpp_image_sha256']=digest(target.read_bytes())
    metrics=compute_metrics(reference,actual,DEFAULT_THRESHOLDS['diff_ratio_threshold']);checks=evaluate(metrics,DEFAULT_THRESHOLDS)
    item.update(status='pass' if all(checks.values()) else 'fail',exact_rgb=metrics.max_abs_delta==0,diff=asdict(metrics),checks=checks)
   except (OSError,ValueError,RuntimeError) as e:item['error']=str(e)
   report['results'].append(item);report['counts']=dict(Counter(x['status'] for x in report['results']));write_json(a.output/'summary.json',report);print(len(report['results']),item['scene'],item['status'],flush=True)
 report['finished']=True
 if a.qualify_reference:report['qualified']=len(report['results'])==50 and all(r.get('exact_rgb',False) and 'retry_ms' not in r for r in report['results'])
 write_json(a.output/'summary.json',report);print(report['counts'],report.get('qualified'))
if __name__=='__main__':main()
