#!/usr/bin/env python3
"""Full native box-chain benchmark, exact repeats and frozen pre-optimization checks."""
import argparse, statistics
from pathlib import Path
from run_m3parameter_gpu import read,write,sha,require,verify_inputs,invoke,rgb
from benchmark_m3parameter_4d import signature
from metal_box_work_compare import same_work
ROOT=Path(__file__).resolve().parents[2]
IDS=['d94d68df7a6cca83','7e41151ddd058d0a']
def run(package,out,qualification):
 identity=sha(__file__);contract,_=verify_inputs(package);scenes={s['id']:s for s in contract['scenes']}
 require(not out.exists(),'Fresh output required');out.mkdir(parents=True)
 old={r['id']:r for r in read(ROOT/'comparison/runs/current/metal-parameters/qualification05/full-scenes.json')}
 qualified={r['id']:r for r in read(qualification/'full-scenes.json')}
 rows=[]
 for sid in IDS:
  scene=scenes[sid];size=scene['settings']['size'];sigs={};jobs=[]
  # Same process: cold + warmup + 3 samples; reverse order in a fresh process.
  for process in range(2):
   for backend in (['cpu','metal'] if process==0 else ['metal','cpu']):
    folder=out/sid/f'{backend}-{process}';dest=folder/'frames'
    argv=[package/'build/mb3d_backend_render_benchmark','--scene',package/scene['frozen'],'--output',dest,'--resource-root',package/'runtime','--backend',backend,'--precision','native','--width',size[0],'--height',size[1],'--threads','6','--warmups','1','--repeats','3']
    if backend=='metal':argv+=['--metal-library',package/'build/metal_native_scene.metallib']
    event=invoke(argv,folder,package/'runtime',600);require(event['exit_code']==0 and not event['guard'],'Benchmark failed '+str(folder))
    report=read(dest/'report.json');require(report['status']=='COMPLETE','Incomplete benchmark')
    sig=signature(report,dest,size,backend,scene['settings'])
    if backend in sigs:require(sig==sigs[backend],'Cross-process RGB/work mismatch')
    sigs[backend]=sig
    ref=qualified[sid]['frames'][0] if backend=='metal' else qualified[sid]['cpu']
    require(sig['image']==ref['image'] and sig['work']==ref['work'],'CLI/benchmark RGB/work mismatch')
    if backend=='metal':
     ref=old[sid]['frames'][0]
     # Decode both original files; do not trust saved hash fields alone.
     oldim,_=rgb(Path(ref['profile']).parent/'image.png',size)
     require(sig['image']==oldim==ref['image'],'Pre-optimization RGB changed')
     same_work(sig['work'],ref['work'])
    jobs.append({'backend':backend,'process':process,'event':event,'report':str(dest/'report.json'),'signature':sig,
     'cold_ms':report['cold']['cold_end_to_end_ms'],
     'warm_ms':[f['end_to_end_ms'] for f in report['samples']],
     'render_ms':[f['render_profile_total_ms'] for f in report['samples']] if 'render_profile_total_ms' in report['samples'][0] else []})
    print(scene['path'],backend,process,'cold',jobs[-1]['cold_ms'],'warm',jobs[-1]['warm_ms'],flush=True)
    write(out/'partial.json',{'rows':rows,'current':{'id':sid,'jobs':jobs}})
  row={'id':sid,'scene':scene['path'],'size':size,'jobs':jobs,'old_metal_exact_rgb_and_sampling_work':True,'allowed_work_change':'HiQ AO dispatch count only, independently checked from dimensions/levels/repeats',
   'historical_old_metal_render_seconds':old[sid]['metal_seconds'],'old_reference':old[sid]['frames'][0]['profile']}
  for backend in ['cpu','metal']:
   selected=[j for j in jobs if j['backend']==backend]
   row[backend]={'cold_ms':[j['cold_ms'] for j in selected],'warm_median_ms':statistics.median(v for j in selected for v in j['warm_ms'])}
  row['warm_speedup_vs_cpu']=row['cpu']['warm_median_ms']/row['metal']['warm_median_ms']
  rows.append(row)
 verify_inputs(package)
 for rel,h in read(package/'source-manifest.json')['source_files'].items():require(sha(ROOT/rel)==h,'Compiled source changed '+rel)
 require(sha(__file__)==identity,'Runner changed')
 write(out/'result.json',{'status':'PASS','scope':'Two full scenes; native exact repeatability, not CPU/Pascal parity; full77 OPEN',
  'runner_sha256':identity,'build_receipt_sha256':sha(package/'build-receipt.json'),'rows':rows})
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--package',type=Path,required=True);p.add_argument('--output',type=Path,required=True);p.add_argument('--qualification',type=Path,required=True);a=p.parse_args();run(a.package.resolve(),a.output.resolve(),a.qualification.resolve())
