#!/usr/bin/env python3
"""Full depth-scene scheduling qualification against frozen pre-optimization evidence."""
import argparse,copy,json,sys,statistics
from pathlib import Path
from metal_depth_work_compare import same_work
from run_m3parameter_gpu import read,write,sha,require,verify_inputs,invoke,render_one,rgb,validate_profile
ROOT=Path(__file__).resolve().parents[2]
BASE=Path('/Volumes/STORAGE/CodexArtifacts/mandelbulb-depth-gradient-qualification-20261025/run')
IDS=['a203b667b15f1e47','5f2747de7764cf5f']
def same_previous(new,old):
 require(new==old,'Previous sampling work changed')
 return True
def run(p,out,resume=None):
 identity=sha(__file__);contract,_=verify_inputs(p);scenes={s['id']:s for s in contract['scenes']}
 if resume:
  seal=read(resume);require(seal['package']==str(p) and seal['qualification']==str(out) and seal['build_receipt_sha256']==sha(p/'build-receipt.json'),'Resume identity mismatch')
  for name,h in seal['files'].items():require(sha(name)==h,'Changed resume artifact '+name)
 else:require(not out.exists(),'Fresh output required');out.mkdir(parents=True)
 def command(argv,folder,timeout=120,code=0):
  if resume and (folder/'event.json').exists():
   e=read(folder/'event.json');require(e['argv']==list(map(str,argv)),'Resume command mismatch')
  else:e=invoke(argv,folder,p/'runtime',timeout)
  require(e['exit_code']==code and not e['guard'],'Command failed '+str(folder));return e
 def frame(scene,backend,folder,size,timeout):
  if not (resume and (folder/'profile.json').exists()):return render_one(p,scene,backend,folder,size,timeout)
  profile=read(folder/'profile.json');event=read(folder/'event.json');work=validate_profile(profile,backend,size);im,_=rgb(folder/'image.png',size)
  require(str(folder/'profile.json') in seal['files'] and str(folder/'image.png') in seal['files'],'Unsealed resume frame')
  require(event['exit_code']==0 and not event['guard'] and profile['input']==str((p/scene['frozen']).resolve()),'Resume input/process mismatch')
  settings=scene['settings'];expected={'ao':settings['ao_kind']!='disabled','shadows':settings['hard_shadows_enabled'],'reflections':settings['reflection_enabled'],'dof':settings['dof_enabled'],'normals':settings['zbuffer_normals'],'volume':bool(settings['volume_light'])}
  for stage,on in expected.items():require(profile['stages'][stage]['state']==('completed' if on else 'not_run'),'Resume stage mismatch')
  return {'event':event,'status':'PREVIEW_COMPLETE','profile':str(folder/'profile.json'),'image':im,'work':work,'stages':{k:v['state'] for k,v in profile['stages'].items()}}
 # Validate fresh admission before expensive full frames, including every old accepted scene.
 command([sys.executable,'-B',ROOT/'comparison/tools/run_m3parameter_gpu.py','--package',p,'--output',out/'admission','--mode','admission'],out/'admission-process',300,1)
 admission=read(out/'admission/result.json');old=read(BASE/'admission/result.json')
 new_ids={r['id'] for r in admission['rows'] if r['status']=='PREPARATION_ACCEPTED'};old_ids={r['id'] for r in old['rows'] if r['status']=='PREPARATION_ACCEPTED'}
 require(new_ids==old_ids and len(new_ids)==27,'Admission inventory changed')
 print('ADMISSION unchanged 27/77',flush=True)
 oldfull={r['id']:r for r in read(BASE/'full-scenes.json')}
 rows=[]
 for sid in IDS:
  s=scenes[sid];size=s['settings']['size'];frames=[]
  for i in range(3):
   print('START FULL',sid,i+1,'/3',flush=True)
   f=frame(s,'metal',out/'full'/sid/str(i),size,1800)
   require(f['status']=='PREVIEW_COMPLETE','Full render failed '+str(f))
   require(f['image']['unique_colors']>32,'Constant frame')
   if frames:require(f['image']==frames[0]['image'] and f['work']==frames[0]['work'],'Exact full repeat failed')
   frames.append(f);write(out/f'{sid}-partial.json',frames)
   print('FULL PASS',sid,i+1,read(f['profile'])['render_total_seconds'],flush=True)
  ref=oldfull[sid]['frames'][0];old_image,_=rgb(Path(ref['profile']).parent/'image.png',size)
  require(frames[0]['image']==old_image==ref['image'],'Pre-optimization RGB changed')
  same_work(frames[0]['work'],ref['work'],size)
  e=frames[0]['work']['execution'];require(e['color_abi_version']==4 and e['shadow_rays']>0 and e['shadow_steps']>0 and e['shadow_changed']>0 and e['ao_samples']>0 and e['ao_changed']>0,'Effects not exercised')
  cpu=frame(s,'cpu',out/'cpu'/sid,size,600);require(cpu['status']=='PREVIEW_COMPLETE','CPU reference failed')
  # Full-resolution gradient-off control changes only the two original depth-gradient bits.
  dest=out/'gradient-off'/sid;dest.mkdir(parents=True,exist_ok=bool(resume))
  controlpath=p/'inputs/controls'/out.name/(sid+'-linear.m3p');controlpath.parent.mkdir(parents=True,exist_ok=True)
  command([p.parent/'make-control',p/s['frozen'],controlpath],dest/'prepare')
  control=copy.deepcopy(s);control['frozen']=str(controlpath.relative_to(p))
  off=frame(control,'metal',dest/'render',size,1800);require(off['status']=='PREVIEW_COMPLETE','Gradient control failed')
  require(off['work']==frames[0]['work'],'Gradient mode changed geometry/effect work')
  _,a=rgb(Path(frames[0]['profile']).parent/'image.png',size);_,b=rgb(Path(off['profile']).parent/'image.png',size)
  changed=sum(a[k:k+3]!=b[k:k+3] for k in range(0,len(a),3));require(changed>0,'Gradient inactive')
  # Effect-off controls at 64x48, same general engine; originals above retain all effects.
  base=frame(s,'metal',out/'effect-controls'/sid/'on',[64,48],120);require(base['status']=='PREVIEW_COMPLETE','Effect baseline failed')
  controls=[]
  for name,flag,stage in [('ao-off','--no-ao','ao'),('shadow-off-scene',None,'shadows')]:
   d=out/'effect-controls'/sid/name
   input_scene=p/s['frozen'];extra=[flag] if flag else []
   if not flag:
    input_scene=p/'inputs/controls'/out.name/(sid+'-no-shadow.m3p')
    command([p.parent/'make-shadow-control',p/s['frozen'],input_scene],out/'effect-controls'/sid/'shadow-prepare')
   command([p/'build/mb3d_render',input_scene,'--backend','metal','--precision','native','--width','64','--height','48','--threads','6','--deterministic',*extra,'--metal-library',p/'build/metal_native_scene.metallib','--output',d/'image.png','--profile-json',d/'profile.json'],d,180)
   pr=read(d/'profile.json');validate_profile(pr,'metal',[64,48]);im,_=rgb(d/'image.png',[64,48]);require(pr['metal_execution']['engine']=='general_formula' and pr['stages'][stage]['state']=='not_run' and im!=base['image'],'Inactive effect control '+name);controls.append(str(d/'profile.json'))
  _,c=rgb(Path(cpu['profile']).parent/'image.png',size)
  rows.append({'id':sid,'scene':s['path'],'size':size,'status':'PASS_FULL_NATIVE_METAL_EXACT_REPEATS','frames':frames,'cpu':cpu,'gradient_off':off,'gradient_changed_pixels':changed,'effect_controls':controls,'cpu_metal_different_pixels':sum(a[k:k+3]!=c[k:k+3] for k in range(0,len(a),3)),'cpu_metal_max_channel_delta':max(abs(x-y) for x,y in zip(a,c)),'metal_seconds':[read(f['profile'])['render_total_seconds'] for f in frames]})
  write(out/'full-scenes.json',rows);print('SCENE COMPLETE',sid,flush=True)
 prior=read(BASE/'regression.json');reg=[]
 for item in prior:
  ref=item['frames'][0];profile=read(ref['profile']);s=scenes[item['id']];fs=[]
  for i in range(2):
   f=frame(s,'metal',out/'regression'/item['id']/str(i),[profile['width'],profile['height']],180)
   require(f['status']=='PREVIEW_COMPLETE' and f['image']==ref['image'] and same_previous(f['work'],ref['work']),'Regression failed '+scenes[item['id']]['path']);fs.append(f)
  reg.append({'id':item['id'],'frames':fs});write(out/'regression.json',reg);print('REGRESSION',len(reg),'/16',flush=True)
 # Preserve recently optimized original-size scenes on the new shader library.
 oldfull=read(BASE/'previous-full.json');previous=[]
 for row in oldfull:
  pr=read(row['frames'][0]['profile']);row['size']=[pr['width'],pr['height']];fs=[]
  for i in range(2):
   f=frame(scenes[row['id']],'metal',out/'previous-full'/row['id']/str(i),row['size'],180)
   require(f['status']=='PREVIEW_COMPLETE' and f['image']==row['frames'][0]['image'] and same_previous(f['work'],row['frames'][0]['work']),'Previous full changed');fs.append(f)
  previous.append({'id':row['id'],'frames':fs})
 write(out/'previous-full.json',previous)
 verify_inputs(p)
 for rel,h in read(p/'source-manifest.json')['source_files'].items():require(sha(ROOT/rel)==h,'Compiled source changed '+rel)
 require(sha(__file__)==identity,'Runner changed')
 write(out/'qualification.json',{'status':'PASS_SCOPED','full_scene_ids':IDS,'optimized_full_scene_count':2,'full_frames':6,'exact_repeats':True,'admission_counts':admission['counts'],'old_new_rgb_exact':True,'old_new_work_change':'DE AO dispatch count only','regression_frames':32,'previous_full_frames':4,'runner_sha256':identity,'build_receipt_sha256':sha(p/'build-receipt.json'),'phase_one_full77':'OPEN','precision':'native_metal_fp32','compatible_or_pascal_parity':False,'resume_seal':str(resume) if resume else None,'resume_seal_sha256':sha(resume) if resume else None})
 print('QUALIFICATION PASS',flush=True)
if __name__=='__main__':
 a=argparse.ArgumentParser();a.add_argument('--package',type=Path,required=True);a.add_argument('--output',type=Path,required=True);a.add_argument('--resume-seal',type=Path);v=a.parse_args();run(v.package.resolve(),v.output.resolve(),v.resume_seal.resolve() if v.resume_seal else None)
