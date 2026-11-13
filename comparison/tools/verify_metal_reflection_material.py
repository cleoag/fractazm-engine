"""Separate delivery re-read: actual files and work, never native/Pascal parity."""
import argparse,sys
from pathlib import Path
from run_m3parameter_gpu import read,write,sha,require,verify_inputs,validate_profile,rgb
from qualify_metal_reflection_material import IDS,BASE,ROOT

def run(p,q,e,out):
 c,_=verify_inputs(p);scenes={s['id']:s for s in c['scenes']};receipt=sha(p/'build-receipt.json')
 qual=read(q/'qualification.json');require(qual['status']=='PASS_SCOPED' and qual['build_receipt_sha256']==receipt,'Qualification/build mismatch')
 require(qual['runner_sha256']==sha(ROOT/'comparison/tools/qualify_metal_reflection_material.py'),'Runner changed')
 for rel,h in read(p/'source-manifest.json')['source_files'].items():require(sha(ROOT/rel)==h,'Source changed '+rel)
 boot=read(e/'bootstrap/bootstrap.json');require(boot['status']=='PASS_SCOPED' and len(boot['tests'])==45 and not any(t['failed'] for t in boot['tests']),'Ordinary tests not complete')
 for name,ident in boot['binaries'].items():require(sha(p/'build'/name)==ident['sha256'],'Bootstrap runtime mismatch')
 for name,h in read(e/'material-proof.json')['evidence'].items():require(sha(Path(name))==h,'Material proof input/output changed')
 material=read(e/'material-test02-run/event.json');require(material['exit_code']==0 and not material['guard'],'Material test failed')
 require('PASS 11535 checks; 768 CPU/GPU material cases x3 exact repeats, 12 invalid descriptors' in (e/'material-test02-run/process.log').read_text(),'Material checks incomplete')
 stale=read(e/'stale-library/event.json');require(stale['exit_code']!=0 and not stale['guard'] and not (e/'stale-library/image.png').exists(),'Stale library published image')
 require('Native camera/inside reflection ABI mismatch' in (e/'stale-library/process.log').read_text(),'Wrong stale-library rejection')
 def frame(f,size,backend,s=None):
  profile=read(f['profile']);work=validate_profile(profile,backend,size);im,_=rgb(Path(f['profile']).parent/'image.png',size)
  require((im,work)==(f['image'],f['work']),'Stored frame differs from actual files')
  event=read(Path(f['profile']).parent/'event.json');require(event['exit_code']==0 and not event['guard'],'Unsuccessful frame')
  if s:
   require(profile['input']==str(p/s['frozen']),'Wrong scene')
   require(all(profile['stages'][k]['state']=='completed' for k in ['reflections','ao','shadows']),'Missing effects')
  return im,work,profile
 rows=read(q/'full-scenes.json');require(len(rows)==2 and {r['id'] for r in rows}==set(IDS),'Wrong scene set')
 for row in rows:
  s=scenes[row['id']];size=s['settings']['size'];require(row['size']==size and len(row['frames'])==3,'Wrong full dimensions/repeats');base=None
  for f in row['frames']:
   im,w,pr=frame(f,size,'metal',s)
   if base:require((im,w)==base,'Exact full repeat failed')
   base=(im,w);r=pr['metal_reflections'];x=pr['metal_execution']
   require(all(r[k]>0 for k in ['rays','hits','changed_pixels','ao_steps','shadow_steps']) and all(x[k]>0 for k in ['ao_changed','shadow_changed']),'Effects inactive')
  frame(row['cpu'],size,'cpu',s)
  preview,_,_=frame(row['preview'],[64,48],'metal',s)
  require({r['name'] for r in row['controls']}=={'reflection-off','ao-off','shadow-off','power64'},'Missing controls')
  for r in row['controls']:
   im,w,pr=frame(r,[64,48],'metal');require(im!=preview and w['execution']['engine']=='general_formula','No same-engine influence')
   if r['name']!='power64':require(pr['stages'][{'reflection-off':'reflections','ao-off':'ao','shadow-off':'shadows'}[r['name']]]['state']=='not_run','Bad off control')
 for name,count in [('regression.json',16),('previous-full.json',2),('full-scenes.json',2)]:
  prior={r['id']:r for r in read(BASE/name)};current=read(q/('prior-'+name));require(len(current)==count and {r['id'] for r in current}==set(prior),'Wrong regression set')
  for row in current:
   f=prior[row['id']]['frames'][0];pr=read(f['profile']);size=[pr['width'],pr['height']];ref=frame(f,size,'metal')[:2]
   require(len(row['frames'])==2,'Missing prior repeats')
   for f in row['frames']:require(frame(f,size,'metal')[:2]==ref,'Prior image/work differs')
 accepted=lambda r:{x['id'] for x in r['rows'] if x['status']=='PREPARATION_ACCEPTED'}
 a=accepted(read(q/'admission/result.json'));b=accepted(read(BASE/'admission/result.json'))
 require(a-b==set(IDS) and b<=a and len(a)==29,'Admission delta differs')
 write(out,{'status':'PASS_DELIVERY_RECHECK','independent_reviewer':False,'method':'Separate script rereads actual images/profiles/work and build bindings','qualified_scene_ids':IDS,'full_frames':6,'prior_frames':40,'ordinary_suites':45,'admitted':29,'rejected':48,'full77':'OPEN','build_receipt_sha256':receipt,'qualifier_sha256':qual['runner_sha256'],'verifier_sha256':sha(__file__)})
 print('DELIVERY RECHECK PASS',flush=True)
if __name__=='__main__':
 a=argparse.ArgumentParser();a.add_argument('--package',type=Path,required=True);a.add_argument('--qualification',type=Path,required=True);a.add_argument('--extras',type=Path,required=True);a.add_argument('--output',type=Path,required=True);v=a.parse_args();run(v.package.resolve(),v.qualification.resolve(),v.extras.resolve(),v.output.resolve())
