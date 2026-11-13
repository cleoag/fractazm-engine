#!/usr/bin/env python3
"""Bound full-pipeline 4D benchmark; native differences are reported, not waived repeats."""
import argparse,statistics,json
from pathlib import Path
from run_m3parameter_gpu import verify_inputs,read,write,sha,require,invoke,rgb,work_record
IDS=['77923ebd629a2fb2','0f258a2abc1002ec']
def signature(report,dest,size,backend,settings):
 sig=None
 for f in [report['cold'],*report['warmups'],*report['samples']]:
  require(f['state']=='completed' and f['backend']['effective']==backend and not f['backend']['fallback'],'backend/frame failure')
  e=f['backend']['execution'];expected={'ao':settings['ao_kind']!='disabled','shadows':settings['hard_shadows_enabled'],'reflections':settings['reflection_enabled'],'dof':settings['dof_enabled'],'normals':settings['zbuffer_normals'],'volume':bool(settings['volume_light'])}
  for stage,on in expected.items():require(f['stages'][stage]['state']==('completed' if on else 'not_run'),'effect stage mismatch')
  if backend=='metal':require(e['validated'] and e['library_profile']==[2,4,32] and e['engine']=='general_formula' and not e['faults'],'GPU evidence failure')
  image,_=rgb(dest/f['image'],size);s={'image':image,'work':work_record({'execution':e,'reflections':f['backend']['reflections']})}
  if sig is None:sig=s
  require(s==sig,'Exact image/work repeat failed')
 return sig

def main():
 p=argparse.ArgumentParser();p.add_argument('--baseline',type=Path,required=True);p.add_argument('--candidate',type=Path,required=True);p.add_argument('--output',type=Path,required=True);p.add_argument('--mode',choices=['preview','original'],required=True);a=p.parse_args()
 base=a.baseline.resolve();candidate=a.candidate.resolve();out=a.output.resolve();require(not out.exists(),'Fresh output required');out.mkdir(parents=True);identity=sha(__file__)
 contract,_=verify_inputs(candidate);verify_inputs(base);scenes={s['id']:s for s in contract['scenes']};rows=[]
 for id in IDS:
  s=scenes[id];size=s['settings']['size'] if a.mode=='original' else ([64,64] if id==IDS[0] else [64,48]);row={'id':id,'scene':s['path'],'size':size,'jobs':[]};sigs={}
  # Preview: two independent processes per mode, 1 cold + 3 warm each.
  # Original: 1 process, 1 cold + 1 warmup + 3 warm (5 exact frames).
  for process in range(2 if a.mode=='preview' else 1):
   modes=['baseline-metal','candidate-cpu','candidate-metal'] if process==0 else ['candidate-metal','candidate-cpu','baseline-metal']
   for mode in modes:
    if a.mode=='original' and mode=='baseline-metal':continue # no extrapolated old full-resolution time
    package=base if mode.startswith('baseline') else candidate;backend=mode.split('-')[1];folder=out/id/f'{mode}-{process}';dest=folder/'frames'
    argv=[package/'build/mb3d_backend_render_benchmark','--scene',package/s['frozen'],'--output',dest,'--resource-root',package/'runtime','--backend',backend,'--precision','native','--width',size[0],'--height',size[1],'--threads','6','--warmups',1 if a.mode=='original' else 0,'--repeats',3]
    if backend=='metal':argv+=['--metal-library',package/'build/metal_native_scene.metallib']
    event=invoke(argv,folder,package/'runtime',600 if a.mode=='original' else 180);require(event['exit_code']==0 and not event['guard'],'benchmark failed '+str(folder))
    report=read(dest/'report.json');require(report['status']=='COMPLETE','bad report');sig=signature(report,dest,size,backend,s['settings'])
    if mode in sigs:require(sigs[mode]==sig,'cross-process repeat failed')
    sigs[mode]=sig
    job={'mode':mode,'process':process,'event':event,'report':str(dest/'report.json'),'signature':sig,'cold_ms':report['cold']['cold_end_to_end_ms'],'warm_ms':statistics.median(f['end_to_end_ms'] for f in report['samples'])};row['jobs'].append(job);write(out/'partial.json',[*rows,row]);print(json.dumps({'scene':Path(s['path']).stem,'mode':mode,'process':process,'cold_ms':job['cold_ms'],'warm_ms':job['warm_ms']}),flush=True)
  row['old_metal_exact']=sigs.get('baseline-metal')==sigs.get('candidate-metal') if a.mode=='preview' else None
  if id==IDS[0] and a.mode=='preview':require(row['old_metal_exact'],'Melting changed')
  rows.append(row)
 verify_inputs(base);verify_inputs(candidate);require(sha(__file__)==identity,'runner changed')
 write(out/'result.json',{'status':'PASS','scope':'4D performance and exact repeatability; full77 remains OPEN','mode':a.mode,'runner_sha256':identity,'baseline_build':sha(base/'build-receipt.json'),'candidate_build':sha(candidate/'build-receipt.json'),'rows':rows})
if __name__=='__main__':main()
