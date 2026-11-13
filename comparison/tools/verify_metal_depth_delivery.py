#!/usr/bin/env python3
"""Re-read full Native depth-gradient evidence and enforce delivery boundaries."""
import argparse,hashlib,json,copy,math,statistics
from pathlib import Path
from PIL import Image
from run_m3parameter_gpu import verify_inputs,require,validate_profile,work_record
ROOT=Path(__file__).resolve().parents[2]
def read(p):return json.loads(Path(p).read_text())
def sha(p):return hashlib.sha256(Path(p).read_bytes()).hexdigest()
def raw(p,size):
 with Image.open(p) as im:
  require(list(im.size)==size,'Image dimensions differ');return im.convert('RGB').tobytes()
def checked(frame,size,backend):
 p=read(frame['profile']);w=validate_profile(p,backend,size);require(w==frame['work'],'Saved work differs from actual profile')
 require(frame['event']['exit_code']==0 and not frame['event']['guard'],'Failed process')
 b=raw(Path(frame['profile']).parent/'image.png',size);require(hashlib.sha256(b).hexdigest()==frame['image']['rgb_sha256'],'Saved RGB digest differs')
 return b,w,p
def previous(new,old):
 n=copy.deepcopy(new);o=copy.deepcopy(old);require(n['execution']['color_abi_version']==4 and o['execution']['color_abi_version']==3,'Wrong ABI transition');n['execution']['color_abi_version']=3;require(n==o,'Prior sampling work changed')
def run(package,q,extras,out):
 contract,_=verify_inputs(package);scenes={s['id']:s for s in contract['scenes']}
 for rel,h in read(package/'source-manifest.json')['source_files'].items():require(sha(ROOT/rel)==h,'Compiled source changed '+rel)
 qualification=read(q/'qualification.json');require(qualification['status']=='PASS_SCOPED','Qualification incomplete')
 require(qualification['build_receipt_sha256']==sha(package.parent/'qualification-build-receipt.json'),'Qualification build identity mismatch')
 require(qualification['runner_sha256']==sha(ROOT/'comparison/tools/qualify_metal_depth_gradient.py'),'Qualification runner changed')
 full=read(q/'full-scenes.json');require(len(full)==2 and {r['id'] for r in full}=={'a203b667b15f1e47','5f2747de7764cf5f'},'Wrong new scene set')
 boot=read(extras/'bootstrap-final/bootstrap.json');require(boot['status']=='PASS_SCOPED' and len(boot['tests'])==45 and not any(t['failed'] for t in boot['tests']),'Ordinary suite failed')
 stage=(package.parent/'color-tests/process.log').read_text();require('PASS 1237 checks' in stage and stage.count('max CPU/GPU channel difference=0')==11,'Color controls incomplete')
 require(read(extras/'obsolete-library/result.json')['status']=='PASS_REJECTED_WITHOUT_IMAGE' and not (extras/'obsolete-library/image.png').exists(),'Old library published output')
 rows=[]
 for row in full:
  s=scenes[row['id']];size=s['settings']['size'];require(row['size']==size and len(row['frames'])==3,'Original scene dimensions/count changed')
  expected=None;expected_work=None
  for f in row['frames']:
   pixels,work,p=checked(f,size,'metal');require(p['input']==str(package/s['frozen']),'Wrong scene input')
   e=work['execution'];require(e['color_abi_version']==4 and e['ao_samples']>0 and e['ao_changed']>0 and e['shadow_rays']>0 and e['shadow_steps']>0 and e['shadow_changed']>0,'Effects inactive')
   if expected is None:expected=pixels;expected_work=work
   require(expected==pixels and expected_work==work,'Exact repeat mismatch')
  linear,lw,lp=checked(row['gradient_off'],size,'metal');require(lw==expected_work,'Gradient changed non-color work');different=sum(expected[i:i+3]!=linear[i:i+3] for i in range(0,len(expected),3));require(different>0 and different==row['gradient_changed_pixels'],'Gradient control inactive')
  cpu,cw,cp=checked(row['cpu'],size,'cpu');dif=sum(expected[i:i+3]!=cpu[i:i+3] for i in range(0,len(expected),3));require(dif==row['cpu_metal_different_pixels'],'CPU comparison mismatch')
  for profile in row['effect_controls']:
   p=read(profile);validate_profile(p,'metal',[64,48]);require(p['metal_execution']['engine']=='general_formula','Effect engine differs')
   stage='ao' if 'ao-off' in profile else 'shadows';require(p['stages'][stage]['state']=='not_run','Control did not disable effect')
   require(raw(Path(profile).parent/'image.png',[64,48])!=raw(q/'effect-controls'/row['id']/'on/image.png',[64,48]),'Effect control image unchanged')
  rows.append({'id':row['id'],'scene':s['path'],'size':size,'full_exact_repeats':3,'metal_seconds':[read(f['profile'])['render_total_seconds'] for f in row['frames']],'cpu_reference_seconds':cp['render_total_seconds'],'gradient_changed_pixels':different,'cpu_metal_different_pixels':dif,'cpu_metal_difference_percent':100*dif/(size[0]*size[1]),'gpu_work':expected_work})
 oldrows={r['id']:r for r in read(ROOT/'comparison/runs/current/metal-box-performance/qualification02/regression.json')}
 regression=read(q/'regression.json');require(len(regression)==16,'Missing previous scenes')
 require({r['id'] for r in regression}==set(oldrows),'Preview scene set changed')
 for row in regression:
  require(len(row['frames'])==2,'Missing preview repeat')
  ref=oldrows[row['id']]['frames'][0];rp=read(ref['profile']);size=[rp['width'],rp['height']];expected=raw(Path(ref['profile']).parent/'image.png',size);prev=None
  for f in row['frames']:
   b,w,_=checked(f,size,'metal');require(b==expected,'Prior preview RGB changed');previous(w,ref['work'])
   if prev is not None:require(w==prev,'Preview work repeat changed')
   prev=w
 oldfull={r['id']:r for r in read(ROOT/'comparison/runs/current/metal-box-performance/qualification02/full-scenes.json')}
 priorfull=read(q/'previous-full.json');require(len(priorfull)==2 and {r['id'] for r in priorfull}==set(oldfull),'Prior full scene set changed')
 for row in priorfull:
  require(len(row['frames'])==2,'Missing prior full repeat')
  ref=oldfull[row['id']];expected=raw(Path(ref['frames'][0]['profile']).parent/'image.png',ref['size'])
  for f in row['frames']:
   b,w,_=checked(f,ref['size'],'metal');require(b==expected,'Prior full RGB changed');previous(w,ref['frames'][0]['work'])
 before={x['path']:x['sha256'] for x in read(package.parent/'qualification-build-receipt.json')['products']};after={x['path']:x['sha256'] for x in read(package/'build-receipt.json')['products']};require(before==after,'Bootstrap changed qualified products')
 result={'status':'PASS_DELIVERY_RECHECK','independent_reviewer':False,'method':'Separate artifact re-read in the same task','ordinary_suites':45,'color_stage_variants':11,'color_stage_checks':1237,'new_full_frames':6,'previous_full_frames':4,'previous_preview_frames':32,'admission_counts':read(q/'admission/result.json')['counts'],'build_products_unchanged_after_bootstrap':True,'phase_one_full77':'OPEN','rows':rows,'runner_hashes':{f:sha(ROOT/'comparison/tools'/f) for f in ['qualify_metal_depth_gradient.py','verify_metal_depth_delivery.py','run_m3parameter_gpu.py']}}
 out.write_text(json.dumps(result,ensure_ascii=False,indent=2)+'\n');print('Delivery recheck PASS')
if __name__=='__main__':
 a=argparse.ArgumentParser();a.add_argument('--package',type=Path,required=True);a.add_argument('--qualification',type=Path,required=True);a.add_argument('--extras',type=Path,required=True);a.add_argument('--output',type=Path,required=True);v=a.parse_args();run(v.package.resolve(),v.qualification.resolve(),v.extras.resolve(),v.output.resolve())
