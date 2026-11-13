#!/usr/bin/env python3
"""Pinned phase-one admission, Native CPU preview and Metal smoke runner.

Package 0 readiness only. Never awards final scene qualification from a preview,
host preparation, missing-resource render, fallback, or a previous binary's PASS.
"""
from pathlib import Path
from collections import Counter
import argparse,hashlib,json,math,os,signal,shutil,struct,subprocess,sys,time,uuid
from PIL import Image
ROOT=Path(__file__).resolve().parents[2]
FRAME_DIAGNOSE_SECONDS=180
FRAME_STOP_SECONDS=300
def sha(p):return hashlib.sha256(Path(p).read_bytes()).hexdigest()
def read(p):return json.loads(Path(p).read_text())
def write(p,x):p.write_text(json.dumps(x,ensure_ascii=False,indent=2,allow_nan=False)+'\n')
def require(value,why):
 if not value:raise ValueError(why)
def artifact(root,rel):
 p=(root/rel).resolve();require(p.is_relative_to(root.resolve()),'Escaping artifact path');return p

def work_record(value):
 """Keep all work fields, including floating values; omit only timings."""
 if isinstance(value,dict):return {k:work_record(v) for k,v in value.items() if not k.endswith('seconds')}
 if isinstance(value,list):return [work_record(v) for v in value]
 if isinstance(value,float):require(math.isfinite(value),'Nonfinite work record')
 return value

def verify_inputs(package):
 c=read(package/'contract.json');receipt=read(package/'build-receipt.json')
 require(c['schema']=='mb3d.phase1.contract.v1' and len(c['scenes'])==77 and c['scene_count']==77,'Wrong scope')
 require(len({s['id'] for s in c['scenes']})==77,'Duplicate scene')
 require(receipt['status']=='PASS_CAPTURED_BUILD_CLOSURE' and receipt['contract_sha256']==sha(package/'contract.json'),'Unbound build')
 for name,expected in [('source-manifest.json',c['source_manifest_sha256']),('resources/manifest.json',c['resource_manifest_sha256'])]:require(sha(package/name)==expected,'Manifest changed')
 for s in c['scenes']:
  require(sha(artifact(package,s['frozen']))==s['sha256'],'Frozen scene changed')
  require(sha(artifact(ROOT,s['path']))==s['sha256'],'Original scene changed')
 for s in c['excluded']:require(s['sha256'] not in {x['sha256'] for x in c['scenes']},'Excluded scene selected')
 sm=read(package/'source-manifest.json')
 for rel,h in sm['source_files'].items():require(sha(artifact(package/'source',rel))==h,'Candidate source changed: '+rel)
 for p in receipt['products']:require(sha(artifact(package,p['path']))==p['sha256'],'Build output changed')
 resource=read(package/'resources/manifest.json');missing={}
 for r in resource['resources']:
  if 'file' in r:require(sha(artifact(package,r['file']))==r['sha256'],'Resource changed')
  else:
   for use in r['uses']:missing.setdefault(use['scene'],[]).append(r['id'])
 require({p.name for p in (package/'runtime/M3Maps').iterdir()}=={Path(r['file']).name for r in resource['resources'] if 'file'in r}|{'m3maps terms of use.txt'},'Unexpected resource shadowing')
 return c,missing

def invoke(argv,folder,cwd,timeout=60,evidence_root=None,*,input_files=(),exclusive_gpu=None,run_id=None,termination_grace_seconds=5):
 # Acquire before starting the process/frame clock. Waiting for another owned
 # GPU job must never become render time. CPU-only commands need no GPU slot.
 args=list(map(str,argv));gpu=exclusive_gpu
 if gpu is None:
  gpu='--backend' in args and args[args.index('--backend')+1] in ('metal','auto')
 if not gpu:return _invoke_unlocked(args,folder,cwd,timeout,evidence_root,input_files=input_files,run_id=run_id,termination_grace_seconds=termination_grace_seconds)
 import fcntl
 path=Path(folder).resolve()
 root=Path(evidence_root).resolve() if evidence_root else (ROOT if path.is_relative_to(ROOT) else path.parent)
 lock=root/'comparison/runs/.gpu-execution.lock';lock.parent.mkdir(parents=True,exist_ok=True)
 started=time.monotonic()
 with lock.open('a+b') as slot:
  fcntl.flock(slot,fcntl.LOCK_EX)
  waited=time.monotonic()-started
  try:return _invoke_unlocked(args,folder,cwd,timeout,evidence_root,input_files=input_files,gpu_queue_wait_seconds=waited,run_id=run_id,termination_grace_seconds=termination_grace_seconds)
  finally:fcntl.flock(slot,fcntl.LOCK_UN)

def _invoke_unlocked(argv,folder,cwd,timeout=60,evidence_root=None,*,input_files=(),gpu_queue_wait_seconds=None,run_id=None,termination_grace_seconds=5):
 require(type(termination_grace_seconds) in (int,float) and 0<termination_grace_seconds<=5,'Invalid termination grace')
 if run_id is None:run_id=uuid.uuid4().hex
 require(isinstance(run_id,str) and len(run_id)==32 and all(c in '0123456789abcdef' for c in run_id),'Invalid explicit process run_id')
 folder=Path(folder).resolve();cwd=Path(cwd).resolve()
 folder.mkdir(parents=True,exist_ok=False);free=shutil.disk_usage(folder).free;reason=None
 argv=list(map(str,argv));exe=Path(shutil.which(argv[0]) or (cwd/argv[0])).resolve();argv[0]=str(exe)
 root=Path(evidence_root).resolve() if evidence_root else (ROOT if folder.is_relative_to(ROOT) else folder.parent)
 def ref(p):return {'path':str(p.relative_to(root)),'sha256':sha(p)}
 inputs=[]
 for value in argv[1:]:
  p=(cwd/value).resolve()
  if p.is_file() and p.is_relative_to(root) and not p.is_relative_to(folder):
   item=ref(p)
   if item not in inputs:inputs.append(item)
 for value in input_files:
  p=Path(value).resolve();require(p.is_file() and p.is_relative_to(root),'Missing/foreign explicit process input')
  item=ref(p)
  if item not in inputs:inputs.append(item)
 # Record selected runtime resources before/after each new process. Historical
 # events retain their original format; no resource observations are invented.
 resource_files={str(p.relative_to(cwd)):sha(p) for p in (cwd/'M3Maps').rglob('*') if p.is_file()}
 for name,h in resource_files.items():
  p=(cwd/name).resolve()
  if p.is_relative_to(root):
   item={'path':str(p.relative_to(root)),'sha256':h}
   if item not in inputs:inputs.append(item)
 executable_hash=sha(exe)
 started_ns=time.time_ns();start=time.monotonic()
 single_frame=Path(str(argv[0])).name.lower() in ('mb3d_render','mb3d_render.exe')
 requested_timeout=timeout
 if single_frame:timeout=min(timeout,FRAME_STOP_SECONDS)
 warned=False
 require(free>=12*1024**3,'Disk reserve below 12 GiB')
 environment={k:v for k,v in os.environ.items() if not k.upper().startswith(('MB3D_','DYLD_'))}
 with (folder/'process.log').open('wb') as stream:
  p=subprocess.Popen(list(map(str,argv)),cwd=cwd,env=environment,stdout=stream,stderr=subprocess.STDOUT,start_new_session=True)
  try:
   while p.poll() is None:
    elapsed=time.monotonic()-start
    if single_frame and not warned and elapsed>=FRAME_DIAGNOSE_SECONDS:
     stream.write(b'\n[runner] Slow frame: diagnostic threshold reached; hard stop remains enabled.\n');stream.flush();warned=True
    free=min(free,shutil.disk_usage(folder).free)
    if free<12*1024**3:reason='DISK_RESERVE'
    elif elapsed>timeout:reason='RENDER_FRAME_TIMEOUT' if single_frame and timeout==FRAME_STOP_SECONDS else 'TIMEOUT'
    elif (folder/'process.log').stat().st_size>16*1024**2:reason='LOG_LIMIT'
    if reason:break
    try:p.wait(timeout=.25)
    except subprocess.TimeoutExpired:pass
  finally:
   if p.poll() is None:
    os.killpg(p.pid,signal.SIGTERM)
    try:p.wait(timeout=termination_grace_seconds)
    except subprocess.TimeoutExpired:os.killpg(p.pid,signal.SIGKILL);p.wait(timeout=termination_grace_seconds)
 elapsed=time.monotonic()-start;finished_ns=time.time_ns()
 if any(sha(root/r['path'])!=r['sha256'] for r in inputs):reason='INPUT_CHANGED'
 resources_after={str(p.relative_to(cwd)):sha(p) for p in (cwd/'M3Maps').rglob('*') if p.is_file()}
 if resources_after!=resource_files:reason='INPUT_CHANGED'
 after_hash=sha(exe)
 if after_hash!=executable_hash:reason='EXECUTABLE_CHANGED'
 outputs=[ref(x) for x in sorted(folder.rglob('*')) if x.is_file() and x.name!='event.json']
 event={'schema':'mb3d.gpu-process.v2','run_id':run_id,'started_ns':started_ns,'finished_ns':finished_ns,
  'executable_sha256_before':executable_hash,'executable_sha256_after':after_hash,'inputs':inputs,'outputs':outputs,
  'log':ref(folder/'process.log'),'effective_timeout_seconds':timeout,
  'resource_files_before':resource_files,'resource_files_after':resources_after,
  'argv':list(map(str,argv)),'cwd':str(cwd),'exit_code':p.returncode,'guard':reason,'pid':p.pid,'owned_process_reaped':True,'seconds':elapsed,'minimum_free_bytes':free}
 if gpu_queue_wait_seconds is not None:event['gpu_queue_wait_seconds']=gpu_queue_wait_seconds
 if single_frame:event.update(requested_timeout_seconds=requested_timeout,effective_timeout_seconds=timeout,frame_diagnostic_threshold_seconds=FRAME_DIAGNOSE_SECONDS,frame_warning_emitted=warned)
 write(folder/'event.json',event);return event

def validate_profile(p,backend,size):
 require(p.get('status')=='complete' and p.get('render_status')=='completed' and p.get('exit_code')==0 and p.get('error')=='','Render incomplete')
 require(p.get('backend')==backend and p.get('backend_requested')==backend and p.get('backend_fallback') is False and p.get('backend_fallback_code')=='' and p.get('backend_fallback_reason')=='','Backend mismatch or CPU fallback')
 require(p.get('precision')=='native' and p.get('numeric_profile')=='native_'+('metal_fp32' if backend=='metal' else 'cpu_fp64'),'Wrong precision')
 require(type(p.get('width')) is int and type(p.get('height')) is int and [p['width'],p['height']]==size,'Render dimensions mismatch')
 stages=p.get('stages',{})
 require(set(stages)=={'load','preparation','volume','primary','normals','shadows','ao','color','reflections','dof','final_surface','save'},'Stage inventory mismatch')
 for k,v in stages.items():
  require(v.get('state') in ['completed','not_run'] and type(v.get('seconds')) in [float,int] and math.isfinite(v['seconds']) and v['seconds']>=0,'Invalid stage '+k)
  if k in ['load','preparation','primary','color','final_surface','save']:require(v['state']=='completed','Missing full-render stage '+k)
 e=p.get('metal_execution',{})
 if backend=='metal':
  require(p.get('schema')=='mb3d.render-stage-profile.v1' and p.get('workers')==0 and bool(p.get('metal_device')),'Not GPU profile')
  require(e.get('validated') is True and e.get('library_profile')==[2,4,32] and e.get('engine') in ['general_formula','exact_capability'],'GPU ABI/profile invalid')
  n=size[0]*size[1]
  for k,v in {'primary_records':n,'normal_records':n,'surface_records':2*n,'color_records':n,'faults':0,'nonfinite':0,'work_exhausted':0}.items():
   require(type(e.get(k)) is int and e[k]==v,'GPU record mismatch '+k)
  require(p.get('metal_reflections',{}).get('faults')==0,'Reflection fault metadata missing')
 else:require(p.get('schema')=='mb3d.cpu-stage-profile.v1' and e.get('validated') is False,'CPU profile mismatch')
 return {'execution':work_record(e),'reflections':work_record(p.get('metal_reflections',{}))}

def rgb(p,size):
 with Image.open(p) as im:
  im.load();require(list(im.size)==size,'Image size mismatch');im=im.convert('RGB');raw=im.tobytes()
 return {'rgb_sha256':hashlib.sha256(raw).hexdigest(),'unique_colors':len(set(zip(raw[0::3],raw[1::3],raw[2::3]))),'bytes':len(raw)},raw

def loaded_scope(path):
 b=path.read_bytes();require(len(b)==1976,'Loader ABI mismatch')
 require(not b[124]&1,'Deferred header excluded')
 for i in range(6):require(b'jit' not in b[848+188*i+12:848+188*i+44].lower(),'Deferred formula excluded')
 return {'loaded_sha256':sha(path),'loaded_width':struct.unpack_from('<i',b,4)[0],'loaded_height':struct.unpack_from('<i',b,8)[0]}

def render_one(package,s,backend,folder,size,timeout):
 argv=[package/'build/mb3d_render',artifact(package,s['frozen']),'--backend',backend,'--precision','native','--width',size[0],'--height',size[1],'--threads','6','--deterministic','--output',folder/'image.png','--profile-json',folder/'profile.json']
 if backend=='metal':argv+=['--metal-library',package/'build/metal_native_scene.metallib']
 event=invoke(argv,folder,package/'runtime',timeout)
 result={'event':event,'status':'OPEN_RENDER','profile':str(folder/'profile.json')}
 try:
  require(event['exit_code']==0 and not event['guard'],'Process failed or exhausted deadline')
  p=read(folder/'profile.json');work=validate_profile(p,backend,size);im,raw=rgb(folder/'image.png',size)
  settings=s['settings']
  expected={'ao':settings['ao_kind']!='disabled','shadows':settings['hard_shadows_enabled'],
    'reflections':settings['reflection_enabled'],'dof':settings['dof_enabled'],'normals':settings['zbuffer_normals'],
    'volume':bool(settings['volume_light'])}
  for stage,enabled in expected.items():
   require(p['stages'][stage]['state']==('completed' if enabled else 'not_run'),'Effect stage differs from scene: '+stage)
  require(p.get('input')==str(artifact(package,s['frozen'])) and p.get('output')==str(folder/'image.png'),'Wrong profile input/output')
  result.update(status='PREVIEW_COMPLETE',image=im,work=work,stages={k:v['state'] for k,v in p['stages'].items()})
 except Exception as e:result['error']=str(e)
 return result

def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--package',type=Path,required=True);p.add_argument('--output',type=Path,required=True);p.add_argument('--mode',choices=['admission','cpu-baseline','smoke','compare'],required=True);p.add_argument('--edge',type=int,default=64);p.add_argument('--timeout',type=int,default=60);p.add_argument('--scene-id');p.add_argument('--original-size',action='store_true',help='Use original loaded dimensions; does not waive final effect/timing/oracle gates');a=p.parse_args()
 package=a.package.resolve();out=a.output.resolve();require(not out.exists(),'Fresh output required');require(8<=a.edge<=256 and 1<=a.timeout<=300,'Preview bounds')
 c,missing=verify_inputs(package);selected=c['scenes']
 if a.scene_id:selected=[s for s in selected if s['id']==a.scene_id];require(len(selected)==1,'Unknown scene')
 if a.mode=='smoke' and not a.scene_id:selected=[s for s in selected if Path(s['path']).stem=='Melting spot bloxx']
 out.mkdir(parents=True);rows=[];assertion=subprocess.Popen(['caffeinate','-is','-w',str(os.getpid())])
 try:
  for s in selected:
   folder=out/s['id'];row={'id':s['id'],'scene':s['path'],'status':'OPEN','full_scene_qualification':'NOT_AWARDED'}
   if a.mode=='admission':
    event=invoke([package/'build/mb3d_scene_loader_probe',artifact(package,s['frozen']),folder/'loader/loaded.bin'],folder/'loader',package/'runtime',a.timeout)
    require(event['exit_code']==0 and not event['guard'],'Loader failed');row.update(loaded_scope(folder/'loader/loaded.bin'))
    e=invoke([package/'build/mb3d_phase1_admission',artifact(package,s['frozen']),package/'runtime',folder/'admission/admission.json'],folder/'admission',package/'runtime',a.timeout)
    if e['exit_code']==0 and not e['guard']:
     obs=read(folder/'admission/admission.json');require(obs.get('render_executed') is False and obs.get('gpu_executed') is False,'Unexpected admission execution')
     row.update(status=obs['status'],preparation=obs)
    else:row.update(status='OPEN_ADMISSION',event=e)
    if s['path'] in missing:row['resource_status']='OPEN_RESOURCE';row['missing_map_ids']=missing[s['path']]
   elif s['path'] in missing:row.update(status='OPEN_RESOURCE',missing_map_ids=missing[s['path']])
   else:
    w,h=s['settings']['size'];size=[w,h] if a.original_size else [max(1,round(w*a.edge/max(w,h))),max(1,round(h*a.edge/max(w,h)))]
    row['render_size']=size;row['original_size']=a.original_size;row['frames']={}
    for backend in (['cpu','metal'] if a.mode in ['smoke','compare'] else ['cpu']):
     frames=[]
     for i in range(3):
      frame=render_one(package,s,backend,folder/f'{backend}-{i}',size,a.timeout);frames.append(frame)
      if frame['status']!='PREVIEW_COMPLETE':break
     row['frames'][backend]=frames
    complete=all(len(fs)==3 and all(f['status']=='PREVIEW_COMPLETE' for f in fs) and len({f['image']['rgb_sha256'] for f in fs})==1 and all(f['work']==fs[0]['work'] for f in fs) for fs in row['frames'].values())
    row['status']=('PASS_ORIGINAL_SIZE_REPEATS_ONLY' if a.original_size else 'PASS_PREVIEW_REPEATS_ONLY') if complete else 'OPEN_RENDER_OR_REPEAT'
    if complete and a.mode in ['smoke','compare']:
     _,cpu=rgb(folder/'cpu-0/image.png',size);_,gpu=rgb(folder/'metal-0/image.png',size)
     differences=[abs(a-b) for a,b in zip(cpu,gpu)];row['cpu_metal_difference']={'different_pixels':sum(cpu[i:i+3]!=gpu[i:i+3] for i in range(0,len(cpu),3)),'total_pixels':len(cpu)//3,'max_channel_error':max(differences),'mean_channel_error':sum(differences)/len(differences)}
   rows.append(row);write(out/'partial.json',rows);print(json.dumps({'done':len(rows),'total':len(selected),'scene':Path(s['path']).stem,'status':row['status']},ensure_ascii=False),flush=True)
  verify_inputs(package)
 finally:assertion.terminate();assertion.wait(timeout=5)
 counts=dict(Counter(r['status'] for r in rows));write(out/'result.json',{'schema':'mb3d.phase1.readiness-run.v1','mode':a.mode,'original_size':a.original_size,'scope':'Package0 preview/admission; no final phase-one PASS','runner_sha256':sha(__file__),'contract_sha256':sha(package/'contract.json'),'build_receipt_sha256':sha(package/'build-receipt.json'),'counts':counts,'rows':rows})
 print(json.dumps(counts))
 return 0 if all((r['status'].startswith('PASS_') or r['status']=='PREPARATION_ACCEPTED') and r.get('resource_status')!='OPEN_RESOURCE' for r in rows) else 1
if __name__=='__main__':sys.exit(main())
