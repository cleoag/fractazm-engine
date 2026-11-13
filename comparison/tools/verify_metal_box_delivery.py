#!/usr/bin/env python3
"""Re-read delivered box-chain evidence, without using the benchmark signature helper."""
import argparse,hashlib,json,math,statistics
from pathlib import Path
from PIL import Image
from run_m3parameter_gpu import verify_inputs,require
ROOT=Path(__file__).resolve().parents[2]
def read(p):return json.loads(Path(p).read_text())
def sha(p):return hashlib.sha256(Path(p).read_bytes()).hexdigest()
def work(x):
 if isinstance(x,dict):return {k:work(v) for k,v in x.items() if not k.endswith('seconds')}
 if isinstance(x,list):return list(map(work,x))
 if isinstance(x,float):require(math.isfinite(x),'Nonfinite evidence')
 return x
def pixels(p,size):
 with Image.open(p) as im:
  require(list(im.size)==size,'Wrong image dimensions');return im.convert('RGB').tobytes()
def run(root):
 p=root/'package02';verify_inputs(p)
 for rel,h in read(p/'source-manifest.json')['source_files'].items():require(sha(ROOT/rel)==h,'Changed compiled source '+rel)
 qualification=read(root/'qualification02/qualification.json');require(qualification['status'].startswith('PASS'),'Qualification failed')
 bench=read(root/'benchmark02/result.json');require(bench['status']=='PASS','Benchmark failed')
 bootstrap=read(root/'bootstrap/bootstrap.json');require(bootstrap['status']=='PASS_SCOPED' and len(bootstrap['tests'])==45 and not any(t['failed'] for t in bootstrap['tests']),'Ordinary suite failed')
 old={r['id']:r for r in read(ROOT/'comparison/runs/current/metal-parameters/qualification05/full-scenes.json')}
 new={r['id']:r for r in read(root/'qualification02/full-scenes.json')};summary=[]
 for row in bench['rows']:
  sid=row['id'];size=row['size'];n=size[0]*size[1];expected_pixels=pixels(Path(old[sid]['frames'][0]['profile']).parent/'image.png',size)
  oldprofile=read(old[sid]['frames'][0]['profile']);oldwork=work({'execution':oldprofile['metal_execution'],'reflections':oldprofile['metal_reflections']})
  cli=new[sid]['frames'];reference=read(cli[0]['profile']);referencework=work({'execution':reference['metal_execution'],'reflections':reference['metal_reflections']})
  changed=[k for k in referencework['execution'] if referencework['execution'][k]!=oldwork['execution'][k]]
  require(changed==(['ao_dispatches'] if sid=='d94d68df7a6cca83' else []),'Unexpected old/new work change')
  require(referencework['reflections']==oldwork['reflections'],'Reflection work changed')
  if changed:require(oldwork['execution']['ao_dispatches']==9455 and referencework['execution']['ao_dispatches']==600,'Wrong command regrouping')
  for f in cli:
   prof=read(f['profile']);require(work({'execution':prof['metal_execution'],'reflections':prof['metal_reflections']})==referencework,'CLI work mismatch')
   require(pixels(Path(f['profile']).parent/'image.png',size)==expected_pixels,'CLI/old pixel mismatch')
   require(prof['backend']=='metal' and not prof['backend_fallback'] and prof['precision']=='native','CLI backend mismatch')
  off=read(new[sid]['ao_off']);require(off['stages']['ao']['state']=='not_run' and off['metal_execution']['engine']==reference['metal_execution']['engine'],'Invalid AO control')
  offpixels=pixels(Path(new[sid]['ao_off']).parent/'image.png',size)
  ao_changed=sum(expected_pixels[i:i+3]!=offpixels[i:i+3] for i in range(0,len(expected_pixels),3));require(ao_changed>0,'Inactive AO')
  backend_pixels={};backend_work={};timings={'cpu':[],'metal':[]};render={'cpu':[],'metal':[]};max_ao_command=0
  for job in row['jobs']:
   report=read(job['report']);backend=job['backend'];dest=Path(job['report']).parent
   require(report['status']=='COMPLETE' and len(report['warmups'])==1 and len(report['samples'])==3,'Wrong benchmark phases')
   for frame in [report['cold'],*report['warmups'],*report['samples']]:
    require(frame['state']=='completed' and frame['precision']=='native','Wrong precision/state')
    route=frame['backend'];require(route['effective']==backend and not route['fallback'],'Backend fallback')
    require(frame['stages']['ao']['state']=='completed','AO omitted')
    raw=pixels(dest/frame['image'],size);w=work({'execution':route['execution'],'reflections':route['reflections']})
    if backend not in backend_pixels:backend_pixels[backend]=raw;backend_work[backend]=w
    require(raw==backend_pixels[backend] and w==backend_work[backend],'Benchmark exact repeat mismatch')
    if backend=='metal':
     require(raw==expected_pixels and w==referencework,'Benchmark/old pixels or CLI work mismatch')
     e=route['execution'];require(e['validated'] and e['library_profile']==[2,4,32] and e['ao_samples']>0 and not(e['faults'] or e['nonfinite'] or e['work_exhausted']),'Invalid GPU evidence')
     max_ao_command=max(max_ao_command,e['ao_max_command_seconds']);require(0<=e['ao_max_command_seconds']<1,'Unbounded AO command duration')
   timings[backend]+=[f['end_to_end_ms'] for f in report['samples']]
   render[backend]+=[f['render_profile_total_ms'] for f in report['samples']]
  a=backend_pixels['metal'];b=backend_pixels['cpu'];different=sum(a[i:i+3]!=b[i:i+3] for i in range(0,len(a),3))
  summary.append({'id':sid,'scene':row['scene'],'size':size,'metal_full_exact_frames':13,'cpu_benchmark_exact_frames':10,'old_rgb_exact':True,'old_work_changed_fields':changed,'ao_off_different_pixels':ao_changed,'cpu_metal_different_pixels':different,'cpu_metal_difference_percent':100*different/n,'cpu_metal_max_channel_delta':max(abs(x-y) for x,y in zip(a,b)),'max_ao_command_seconds':max_ao_command,'warm_ms':{k:{'median':statistics.median(v),'min':min(v),'max':max(v)} for k,v in timings.items()},'warm_render_ms':{k:statistics.median(v) for k,v in render.items()}})
 before={v['path']:v['sha256'] for v in read(root/'qualification-build-receipt.json')['products']};after={v['path']:v['sha256'] for v in read(p/'build-receipt.json')['products']}
 require(before==after,'Bootstrap changed qualified products')
 helpers=['benchmark_metal_box_chains.py','qualify_metal_box_chains.py','metal_box_work_compare.py','benchmark_m3parameter_4d.py','run_m3parameter_gpu.py','verify_metal_box_delivery.py']
 result={'status':'PASS_DELIVERY_RECHECK','independent_reviewer':False,'method':'Separate artifact re-read and explicit raw RGB/work verification in the same task','ordinary_suites':45,'build_products_unchanged_after_bootstrap':True,'source_commit':bootstrap['source_commit'],'rows':summary,'tools':{f:sha(ROOT/'comparison/tools'/f) for f in helpers}}
 (root/'delivery-verification.json').write_text(json.dumps(result,ensure_ascii=False,indent=2)+'\n');print(json.dumps(result,ensure_ascii=False,indent=2))
if __name__=='__main__':
 a=argparse.ArgumentParser();a.add_argument('--root',type=Path,required=True);run(a.parse_args().root.resolve())
