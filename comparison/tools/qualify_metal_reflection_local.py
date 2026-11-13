"""Full native reflection material qualification, frozen ordinary scenes only."""
import argparse,copy,sys
from pathlib import Path
from run_m3parameter_gpu import read,write,sha,require,verify_inputs,invoke,render_one,rgb,validate_profile
ROOT=Path(__file__).resolve().parents[2]
BASE=ROOT/'comparison/runs/current/metal-box-performance/qualification02'
DEPTH_IDS={'a203b667b15f1e47','5f2747de7764cf5f'}
IDS=['53465ebd7f8bd8e8','f145162b4387fbb0']
def run(p,out):
 require(not out.exists(),'Fresh output required');out.mkdir(parents=True)
 identity=sha(__file__);contract,_=verify_inputs(p);scenes={s['id']:s for s in contract['scenes']}
 def command(argv,folder,limit=180,code=0):
  e=invoke(argv,folder,p/'runtime',limit);require(e['exit_code']==code and not e['guard'],'Command failed '+str(folder));return e
 def frame(s,backend,folder,size,limit=300):
  f=render_one(p,s,backend,folder,size,limit);require(f['status']=='PREVIEW_COMPLETE','Frame failed '+str(f));return f
 command([sys.executable,'-B',ROOT/'comparison/tools/run_m3parameter_gpu.py','--package',p,'--output',out/'admission','--mode','admission'],out/'admission-process',300,1)
 admission=read(out/'admission/result.json');old=read(BASE/'admission/result.json')
 accepted=lambda r:{x['id'] for x in r['rows'] if x['status']=='PREPARATION_ACCEPTED'}
 require(accepted(admission)==accepted(old)|DEPTH_IDS|set(IDS),'Unexpected admission delta')
 print('ADMISSION 29/77; exactly two new scenes',flush=True)
 rows=[]
 for sid in IDS:
  s=scenes[sid];size=s['settings']['size'];fs=[]
  # Previews catch runtime faults before original-size work.
  # A new five-slot height-map pipeline needs bounded cold driver compilation.
  preview=frame(s,'metal',out/'preview'/sid,[64,48],600)
  for i in range(3):
   print('START FULL',sid,i+1,flush=True);f=frame(s,'metal',out/'full'/sid/str(i),size)
   require(f['image']['unique_colors']>32,'Constant frame')
   if fs:require((f['image'],f['work'])==(fs[0]['image'],fs[0]['work']),'Exact full repeat failed')
   fs.append(f);write(out/(sid+'-partial.json'),fs);print('FULL PASS',sid,i+1,read(f['profile'])['render_total_seconds'],flush=True)
  e=fs[0]['work']['execution'];pr=read(fs[0]['profile']);r=pr['metal_reflections']
  require(e['engine']=='general_formula' and e['shadow_steps']>0 and e['shadow_changed']>0 and e['ao_samples']>0 and e['ao_changed']>0,'Inactive primary effects')
  require(r['rays']>0 and r['hits']>0 and r['changed_pixels']>0 and r['ao_steps']>0 and r['shadow_steps']>0,'Inactive secondary rays/effects')
  cpu=frame(s,'cpu',out/'cpu'/sid,size,300)
  controls=[]
  for name,flag,stage in [('reflection-off','--no-reflections','reflections'),('ao-off','--no-ao','ao'),('shadow-off',None,'shadows'),('power64',None,None)]:
   d=out/'controls'/sid/name;input_scene=p/s['frozen'];extra=[flag] if flag else []
   if not flag:
    input_scene=p/'inputs/controls'/out.name/(sid+'-'+name+'.m3p');input_scene.parent.mkdir(parents=True,exist_ok=True)
    helper='make-shadow-control' if stage=='shadows' else 'make-material-control'
    command([p.parent/helper,p/s['frozen'],input_scene],out/'controls'/sid/(name+'-prepare'))
   command([p/'build/mb3d_render',input_scene,'--backend','metal','--precision','native','--width','64','--height','48','--threads','6','--deterministic',*extra,'--metal-library',p/'build/metal_native_scene.metallib','--output',d/'image.png','--profile-json',d/'profile.json'],d)
   profile=read(d/'profile.json');work=validate_profile(profile,'metal',[64,48]);im,_=rgb(d/'image.png',[64,48])
   require(work['execution']['engine']=='general_formula' and im!=preview['image'],'Inactive same-engine control '+name)
   if stage:require(profile['stages'][stage]['state']=='not_run','Disabled effect still ran')
   controls.append({'name':name,'profile':str(d/'profile.json'),'image':im,'work':work})
  _,a=rgb(Path(fs[0]['profile']).parent/'image.png',size);_,b=rgb(Path(cpu['profile']).parent/'image.png',size)
  row={'id':sid,'scene':s['path'],'size':size,'frames':fs,'cpu':cpu,'preview':preview,'controls':controls,'metal_seconds':[read(f['profile'])['render_total_seconds'] for f in fs],'cpu_seconds':read(cpu['profile'])['render_total_seconds'],'cpu_metal_different_pixels':sum(a[k:k+3]!=b[k:k+3] for k in range(0,len(a),3)),'status':'PASS_FULL_NATIVE_METAL_EXACT_REPEATS'}
  rows.append(row);write(out/'full-scenes.json',rows);print('SCENE COMPLETE',sid,flush=True)
 for name in ['regression.json','full-scenes.json']:
  prior=read(BASE/name);reg=[]
  for row in prior:
   ref=row['frames'][0];profile=read(ref['profile']);size=[profile['width'],profile['height']];fs=[]
   actual,_=rgb(Path(ref['profile']).parent/'image.png',size);require(actual==ref['image'],'Changed frozen image')
   for i in range(2):
    f=frame(scenes[row['id']],'metal',out/('prior-'+name[:-5])/row['id']/str(i),size,600)
    oldwork=copy.deepcopy(ref['work']);require(oldwork['execution']['color_abi_version']==3,'Unexpected old color ABI');oldwork['execution']['color_abi_version']=4
    require(f['image']==ref['image'] and f['work']==oldwork,'Prior RGB/work changed '+scenes[row['id']]['path']);fs.append(f)
   reg.append({'id':row['id'],'frames':fs});write(out/('prior-'+name),reg);print('REGRESSION',name,len(reg),'/',len(prior),flush=True)
 # Current depth-performance full PNGs are retained locally; its raw full
 # work records are on disconnected STORAGE. Never invent or regenerate them.
 depth=[];alias=read(ROOT/'comparison/runs/current/metal-depth-performance/main-alias-check.json')
 for row in alias['rows']:
  sid=row['id'];s=scenes[sid];preview_ref=row['frames'][0];pre=[]
  for i in range(2):
   f=frame(s,'metal',out/'depth-preview'/sid/str(i),[64,48],180)
   require(f['image']==preview_ref['image'] and f['work']==preview_ref['work'],'Latest depth preview changed');pre.append(f)
  image_path=ROOT/'comparison/results/m3parameter-metal-depth-performance-20261025'/(sid+'-metal.png')
  size=s['settings']['size'];reference,_=rgb(image_path,size);fs=[]
  for i in range(2):
   f=frame(s,'metal',out/'depth-full'/sid/str(i),size,600)
   require(f['image']==reference,'Latest depth full RGB changed')
   if fs:require(f['work']==fs[0]['work'],'Depth current work repeat failed')
   fs.append(f)
  depth.append({'id':sid,'preview':pre,'frames':fs,'full_reference_image':str(image_path),'full_reference_sha256':sha(image_path),'old_full_work_comparison':'NOT_AVAILABLE_WITHOUT_STORAGE'})
 write(out/'depth-regression.json',depth)
 verify_inputs(p)
 for rel,h in read(p/'source-manifest.json')['source_files'].items():require(sha(ROOT/rel)==h,'Source changed '+rel)
 require(sha(__file__)==identity,'Runner changed')
 write(out/'qualification.json',{'status':'PASS_SCOPED','full_scene_ids':IDS,'new_full_frames':6,'exact_repeats':True,'admission_counts':admission['counts'],'prior_preview_frames':32,'prior_full_frames':8,'latest_depth_preview_frames':4,'baseline_note':'Local frozen box RGB/work (color ABI 3->4 only) and latest depth PNG/preview work; latest depth old full work remains on STORAGE','runner_sha256':identity,'build_receipt_sha256':sha(p/'build-receipt.json'),'full77':'OPEN','native_not_compatible':True})
 print('QUALIFICATION PASS',flush=True)
if __name__=='__main__':
 a=argparse.ArgumentParser();a.add_argument('--package',type=Path,required=True);a.add_argument('--output',type=Path,required=True);v=a.parse_args();run(v.package.resolve(),v.output.resolve())
