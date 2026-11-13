#!/usr/bin/env python3
"""Bounded ABoxMod1 package: original full scenes, GPU controls and exact repeats."""
import argparse,hashlib,json,math,statistics,struct,sys
from pathlib import Path
from run_m3parameter_gpu import read,write,sha,require,verify_inputs,invoke,render_one,rgb,validate_profile
ROOT=Path(__file__).resolve().parents[2]
IDS=['d94d68df7a6cca83','7e41151ddd058d0a']
def run(p,out,resume=None):
 identity=sha(__file__);contract,_=verify_inputs(p);scenes={s['id']:s for s in contract['scenes']}
 require(not out.exists(),'fresh output required');out.mkdir(parents=True)
 def command(args,folder,timeout=300,exit_code=0):
  event=invoke(args,folder,p/'runtime',timeout);require(event['exit_code']==exit_code and not event['guard'],'Failed command '+str(folder));return event
 for sid in IDS:
  scene=scenes[sid];dest=out/'points'/sid
  formula_slot=next(x['slot'] for x in scene['slots'] if x['name']=='ABoxMod1')
  command([p/'build/mb3d_native_formula_probe','--scene',p/scene['frozen'],'--library',p/'build/metal_native_scene.metallib','--resource-root',p/'runtime','--mode','step','--slot',str(formula_slot),'--repeats','3','--output',dest/'result.json'],dest)
  probe=read(dest/'result.json');require(probe['fault_count']==0 and probe['exact_state_repeats'],'Point step fault')
 # Independent branch witnesses using a single formula and simple constants.
 # ABI offsets are pinned by the probe's size/version marker and declared structs.
 seed=read(out/'points'/IDS[0]/'result.json');require(seed['formula_abi']==[1,8192,188,236,32],'Unexpected point ABI')
 program=bytearray(struct.pack('<2048I',*seed['program_words']));slot=224
 require(struct.unpack_from('<I',program,slot+8)[0]==8024,'Wrong formula identity')
 state=bytearray(struct.pack('<47I',*seed['rows'][0]['input_words']))
 def putf(b,offset,v):struct.pack_into('<f',b,offset,v)
 def parameter(b,index,value,uploaded=None):
  putf(b,slot+48+4*index,value if uploaded is None else uploaded)
  struct.pack_into('<d',b,slot+304+8*index,value)
 for i in range(11):parameter(program,i,0.)
 parameter(program,0,2.);parameter(program,1,8.);parameter(program,2,.25);parameter(program,3,1.)
 # Make the first call initialize VaryScale from Scale, with derivative 1.
 struct.pack_into('<i',state,136,0)
 putf(state,64,1.);putf(state,12,1.)
 for offset in range(16,64,4):putf(state,offset,0.)
 controls=[]
 for name,point,scale,threshold,expected_fault in [
  ('finite-inner',(.125,.25,0.),8.,.25,0),
  ('finite-outer',(1.,0.,0.),8.,.25,0),
  ('deferred-outer',(1.,0.,0.),2e80,1e-80,0),
  ('deferred-origin',(0.,0.,0.),2e80,1e-80,6),
  ('finite-origin',(0.,0.,0.),8.,.25,0),
  ('nonfinite-scale',(0.,0.,0.),math.inf,.25,6)]:
  dest=out/'controls'/name;dest.mkdir(parents=True)
  b=bytearray(program);st=bytearray(state)
  parameter(b,1,scale,0. if abs(scale)>3.4e38 else scale);parameter(b,2,threshold)
  for i,x in enumerate(point):putf(st,i*4,x)
  (dest/'program.bin').write_bytes(b);(dest/'states.bin').write_bytes(st)
  command([p/'build/mb3d_native_formula_probe','--program',dest/'program.bin','--states',dest/'states.bin','--library',p/'build/metal_native_scene.metallib','--mode','step','--slot','0','--repeats','3','--output',dest/'result.json'],dest/'process',exit_code=2 if expected_fault else 0)
  result=read(dest/'result.json');require(result['rows'][0]['fault']==expected_fault,'Wrong branch fault '+name)
  if not expected_fault:
   words=result['rows'][0]['result_words'];xyz=struct.unpack('<3f',struct.pack('<3I',*words[:3]));expected=tuple(x*(8. if name=='finite-inner' else 2.) for x in point)
   require(xyz==expected,'Independent exact arithmetic witness failed '+name+str(xyz))
  controls.append({'name':name,'expected_fault':expected_fault,'exact_repeats':result['exact_state_repeats'],'result':str(dest/'result.json')})
 write(out/'controls.json',controls);print('CONTROLS PASS',len(controls),flush=True)
 rows=[];saved={}
 if resume:
  seal=read(resume);require(seal['package']==str(p) and seal['build_receipt_sha256']==sha(p/'build-receipt.json'),'Resume build differs')
  for filename,digest in seal['files'].items():require(sha(filename)==digest,'Resume evidence changed '+filename)
  for row in read(Path(seal['completed_scene_rows'])):
   sid=row['id'];require(sid in IDS and row['status']=='PASS_FULL_NATIVE_METAL_EXACT_REPEATS','Unqualified resume scene')
   scene=scenes[sid];size=scene['settings']['size'];require(row['size']==size and len(row['frames'])==3,'Resume dimensions/count')
   for frame in row['frames']:
    profile=read(Path(frame['profile']));work=validate_profile(profile,'metal',size)
    im,_=rgb(Path(frame['profile']).parent/'image.png',size)
    require(im==frame['image']==row['frames'][0]['image'] and work==frame['work']==row['frames'][0]['work'],'Resume RGB/work mismatch')
    require(frame['event']['exit_code']==0 and frame['event']['guard'] is None,'Interrupted resume frame')
    require(profile['input']==str(p/scene['frozen']) and profile['stages']['ao']['state']=='completed','Resume scene/effect mismatch')
   off=read(Path(row['ao_off']));validate_profile(off,'metal',size)
   require(off['stages']['ao']['state']=='not_run','Resume AO control invalid')
   offim,_=rgb(Path(row['ao_off']).parent/'image.png',size);require(offim['rgb_sha256']!=row['frames'][0]['image']['rgb_sha256'],'Resume AO inactive')
   cp=read(Path(row['cpu']['profile']));validate_profile(cp,'cpu',size)
   cpuim,_=rgb(Path(row['cpu']['profile']).parent/'image.png',size);require(cpuim==row['cpu']['image'],'Resume CPU reference changed')
   saved[sid]=row
 for sid in IDS:
  if sid in saved:
   rows.append(saved[sid]);write(out/'full-scenes.json',rows);print('VERIFIED COMPLETED SCENE',sid,flush=True);continue
  scene=scenes[sid];size=scene['settings']['size'];frames=[]
  for repeat in range(3):
   f=render_one(p,scene,'metal',out/'full'/sid/str(repeat),size,600)
   require(f['status']=='PREVIEW_COMPLETE','Full render failed '+str(f))
   require(f['image']['unique_colors']>32,'Empty/constant frame')
   if frames:require(f['image']==frames[0]['image'] and f['work']==frames[0]['work'],'Full RGB/work repeat mismatch')
   frames.append(f);print('FULL',sid,repeat+1,'/3',flush=True)
  e=frames[0]['work']['execution'];require(e['ao_active']>0 and e['ao_changed']>0 and e['ao_samples']>0,'AO not exercised')
  # Same engine AO-off is a diagnostic control, never a qualified scene render.
  dest=out/'ao-off'/sid
  command([p/'build/mb3d_render',p/scene['frozen'],'--backend','metal','--precision','native','--width',size[0],'--height',size[1],'--threads','6','--deterministic','--no-ao','--metal-library',p/'build/metal_native_scene.metallib','--output',dest/'image.png','--profile-json',dest/'profile.json'],dest,600)
  off=read(dest/'profile.json');validate_profile(off,'metal',size)
  require(off['stages']['ao']['state']=='not_run' and off['metal_execution']['engine']==e['engine'],'Invalid AO control')
  off_im,_=rgb(dest/'image.png',size);require(off_im['rgb_sha256']!=frames[0]['image']['rgb_sha256'],'AO image control inactive')
  cpu=render_one(p,scene,'cpu',out/'cpu'/sid,size,600);require(cpu['status']=='PREVIEW_COMPLETE','CPU reference failed')
  _,a=rgb(Path(frames[0]['profile']).parent/'image.png',size);_,b=rgb(Path(cpu['profile']).parent/'image.png',size)
  differences=sum(a[i:i+3]!=b[i:i+3] for i in range(0,len(a),3));delta=max(abs(x-y) for x,y in zip(a,b))
  seconds=[read(Path(f['profile']))['render_total_seconds'] for f in frames]
  rows.append({'id':sid,'scene':scene['path'],'size':size,'status':'PASS_FULL_NATIVE_METAL_EXACT_REPEATS','frames':frames,'ao_off':str(dest/'profile.json'),'cpu':cpu,'cpu_metal_differing_pixels':differences,'cpu_metal_max_channel_delta':delta,'metal_seconds':seconds})
  write(out/'full-scenes.json',rows);print('SCENE PASS',scene['path'],flush=True)
 # Existing scene results are fresh checks against frozen previous image/work.
 prior=read(ROOT/'comparison/runs/current/metal-promotion/qualification/regression/result.json');reg=[]
 for item in prior['rows']:
  ref=item['frames'][0];profile=read(Path(ref['profile']));scene=scenes[item['id']];frames=[]
  for repeat in range(2):
   f=render_one(p,scene,'metal',out/'regression'/item['id']/str(repeat),[profile['width'],profile['height']],180)
   require(f['status']=='PREVIEW_COMPLETE','Regression failed')
   require(f['image']==ref['image'] and f['work']==ref['work'],'Previous RGB/work changed')
   frames.append(f)
  reg.append({'id':item['id'],'scene':item['scene'],'frames':frames});write(out/'regression.json',reg);print('REGRESSION',len(reg),'/16',flush=True)
 command([sys.executable,'-B',ROOT/'comparison/tools/run_m3parameter_gpu.py','--package',p,'--output',out/'admission','--mode','admission'],out/'admission-process',exit_code=1)
 admission=read(out/'admission/result.json');require(admission['counts']=={'PREPARATION_ACCEPTED':25,'PREPARATION_REJECTED':52},'Unexpected admission inventory')
 verify_inputs(p)
 for rel,h in read(p/'source-manifest.json')['source_files'].items():require(sha(ROOT/rel)==h,'Main source changed '+rel)
 require(sha(__file__)==identity,'Runner changed')
 write(out/'qualification.json',{'status':'PASS_SCOPED','full_scene_ids':IDS,'full_scene_count':2,'full_frames':6,'exact_rgb_and_all_nontiming_work':True,'admission_counts':admission['counts'],'regression_frames':len(reg)*2,'runner_sha256':identity,'build_receipt_sha256':sha(p/'build-receipt.json'),'resumed_full_scenes':sorted(saved),'resume_seal_sha256':sha(resume) if resume else None,'phase_one_full77':'OPEN','precision':'native','compatible_or_pascal_parity':'NOT_CLAIMED'})
 print('QUALIFICATION PASS',flush=True)
if __name__=='__main__':
 a=argparse.ArgumentParser();a.add_argument('--package',type=Path,required=True);a.add_argument('--output',type=Path,required=True);a.add_argument('--resume',type=Path);args=a.parse_args();run(args.package.resolve(),args.output.resolve(),args.resume.resolve() if args.resume else None)
