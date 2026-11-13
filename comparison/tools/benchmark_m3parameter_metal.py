#!/usr/bin/env python3
"""Bound phase-one performance/regression run; no final corpus qualification."""
import argparse,json,statistics,hashlib
from pathlib import Path
from run_m3parameter_gpu import verify_inputs,read,write,sha,require,render_one,invoke,rgb,work_record

def main():
 runner_identity=sha(__file__)
 p=argparse.ArgumentParser();p.add_argument('--baseline',type=Path,required=True);p.add_argument('--candidate',type=Path,required=True);p.add_argument('--output',type=Path,required=True);p.add_argument('--mode',choices=['preview','original'],required=True);a=p.parse_args()
 base=a.baseline.resolve();candidate=a.candidate.resolve();out=a.output.resolve();require(not out.exists(),'Fresh output required');out.mkdir(parents=True)
 c,_=verify_inputs(candidate);verify_inputs(base)
 pinned={r['id']:r for r in read(base/'compare-01/result.json')['rows'] if r['status'].startswith('PASS')}
 rows=[]
 if a.mode=='preview':
  for s in c['scenes']:
   if s['id'] not in pinned:continue
   old=pinned[s['id']];size=old['render_size'];r={'id':s['id'],'scene':s['path'],'size':size,'frames':{},'status':'PASS'}
   for backend in ['cpu','metal']:
    fs=[]
    for i in range(3):
     f=render_one(candidate,s,backend,out/s['id']/f'{backend}-{i}',size,180);fs.append(f)
     require(f['status']=='PREVIEW_COMPLETE',str(f))
     require(f['image']==old['frames'][backend][0]['image'],'Changed RGB')
     require(f['work']==old['frames'][backend][0]['work'],'Changed work')
    r['frames'][backend]=fs
   rows.append(r);write(out/'partial.json',rows);print(json.dumps({'done':len(rows),'total':len(pinned),'scene':Path(s['path']).stem,'cpu_s':statistics.median(f['event']['seconds'] for f in r['frames']['cpu']),'metal_s':statistics.median(f['event']['seconds'] for f in r['frames']['metal']),'exact_old_rgb_and_work':True}),flush=True)
 else:
  ids=['c8ef0b59581727b2','6c86d9620df94e35','35820014b1fd2ff4']
  scenes={s['id']:s for s in c['scenes']}
  for id in ids:
   s=scenes[id];size=s['settings']['size'];r={'id':id,'scene':s['path'],'size':size,'jobs':[]};signatures={}
   for process in range(3):
    # Alternate old/candidate order, with CPU interleaved. The old Insect
    # run already exceeded 180 s/frame; retain it as censored, not a time.
    modes=['baseline-metal','candidate-metal','candidate-cpu'] if process%2==0 else ['candidate-cpu','candidate-metal','baseline-metal']
    for mode in modes:
     if id==ids[2] and mode=='baseline-metal':continue
     package=base if mode.startswith('baseline') else candidate;backend=mode.split('-')[1];folder=out/id/f'{mode}-{process}';dest=folder/'frames'
     argv=[package/'build/mb3d_backend_render_benchmark','--scene',package/s['frozen'],'--output',dest,'--resource-root',package/'runtime','--backend',backend,'--precision','native','--width',size[0],'--height',size[1],'--threads','6','--warmups','1','--repeats','3']
     if backend=='metal':argv+=['--metal-library',package/'build/metal_native_scene.metallib']
     event=invoke(argv,folder,package/'runtime',300);require(event['exit_code']==0 and not event['guard'],'Benchmark failed '+str(folder))
     report=read(dest/'report.json');require(report['status']=='COMPLETE','Benchmark report failed')
     fs=[report['cold'],*report['warmups'],*report['samples']];signature=None
     for f in fs:
      require(f['state']=='completed' and f['backend']['effective']==backend and not f['backend']['fallback'],'Wrong backend/frame')
      image,_=rgb(dest/f['image'],size);e=f['backend']['execution'];w=work_record({'execution':e,'reflections':f['backend']['reflections']})
      settings=s['settings'];expected={'ao':settings['ao_kind']!='disabled','shadows':settings['hard_shadows_enabled'],'reflections':settings['reflection_enabled'],'dof':settings['dof_enabled'],'normals':settings['zbuffer_normals'],'volume':bool(settings['volume_light'])}
      for stage,enabled in expected.items():require(f['stages'][stage]['state']==('completed' if enabled else 'not_run'),'Effect stage mismatch')
      if backend=='metal':require(e['validated'] and e['library_profile']==[2,4,32] and e['engine']=='general_formula' and not e['faults'],'Invalid GPU evidence')
      current={'image':image,'work':w}
      if signature is None:signature=current
      require(current==signature,'In-process repeat differs')
     if mode in signatures:require(signature==signatures[mode],'Cross-process repeat differs')
     signatures[mode]=signature
     if all(m in signatures for m in ['baseline-metal','candidate-metal']):require(signatures['candidate-metal']==signatures['baseline-metal'],'Old/candidate image or work differs')
     job={'mode':mode,'process':process,'event':event,'report':str(dest/'report.json'),'signature':signature,'cold_ms':report['cold']['cold_end_to_end_ms'],'warm_ms':statistics.median(f['end_to_end_ms'] for f in report['samples'])};r['jobs'].append(job)
     write(out/'partial.json',[*rows,r]);print(json.dumps({'scene':Path(s['path']).stem,'mode':mode,'process':process,'cold_ms':job['cold_ms'],'warm_ms':job['warm_ms']}),flush=True)
   rows.append(r)
 verify_inputs(candidate);verify_inputs(base);require(sha(__file__)==runner_identity,'Runner changed during execution')
 write(out/'result.json',{'status':'PASS','scope':'Performance and exact regression only; full corpus qualification remains OPEN','mode':a.mode,'runner_sha256':sha(__file__),'baseline_build':sha(base/'build-receipt.json'),'candidate_build':sha(candidate/'build-receipt.json'),'rows':rows})
if __name__=='__main__':main()
