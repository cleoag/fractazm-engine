#!/usr/bin/env python3
"""Matched same-process warm measurements, cold + three exact repeats.

The entire four-frame job has a 300-second wall deadline, a stricter bound
than 300 seconds per frame. Use only already completed short cases. A job
timing out is OPEN; never extend its deadline for a slow individual frame.
"""
import argparse
import statistics
from pathlib import Path
from run_m3parameter_gpu import read,write,require,verify_inputs,invoke,sha
from benchmark_m3parameter_4d import signature

def run(baseline,candidate,output,ids,size):
    require(not output.exists(),'Fresh output required');output.mkdir(parents=True)
    cs=[verify_inputs(p)[0] for p in (baseline,candidate)]
    scenes=[{s['id']:s for s in c['scenes']} for c in cs]
    rows=[]
    for sid in ids:
        scene=scenes[0][sid];require(scene==scenes[1][sid],'Scene differs')
        dimensions=size or scene['settings']['size'];jobs=[];ref=None
        for process in range(2):
            for name,package in ([('baseline',baseline),('candidate',candidate)] if process==0
                                 else [('candidate',candidate),('baseline',baseline)]):
                folder=output/sid/(name+'-'+str(process));dest=folder/'frames'
                argv=[package/'build/mb3d_backend_render_benchmark','--scene',package/scene['frozen'],
                      '--output',dest,'--resource-root',package/'runtime','--backend','metal',
                      '--precision','native','--width',dimensions[0],'--height',dimensions[1],
                      '--threads','6','--warmups','0','--repeats','3','--frame-timeout-seconds','300','--metal-library',package/'build/metal_native_scene.metallib']
                print('START WARM',sid,name,process,flush=True)
                event=invoke(argv,folder,package/'runtime',300)
                require(event['exit_code']==0 and not event['guard'],'Whole-job deadline or benchmark failure')
                report=read(dest/'report.json');require(report['status']=='COMPLETE','Incomplete benchmark')
                sig=signature(report,dest,dimensions,'metal',scene['settings'])
                if ref is not None:require(sig==ref,'Old/new or cross-process RGB/work difference')
                ref=sig
                job={'mode':name,'process':process,'report':str(dest/'report.json'),'event':event,
                     'deadline_policy':'entire four-frame job <=300 seconds; includes cold preparation',
                     'binary_sha256':sha(argv[0]),'library_sha256':sha(package/'build/metal_native_scene.metallib'),
                     'signature':sig,'cold_seconds':report['cold']['cold_end_to_end_ms']/1000,
                     'warm_seconds':[f['end_to_end_ms']/1000 for f in report['samples']]}
                jobs.append(job);write(output/'partial.json',{'rows':rows,'current':{'id':sid,'jobs':jobs}})
                print('PASS WARM',name,job['warm_seconds'],flush=True)
        row={'id':sid,'scene':scene['path'],'size':dimensions,'original_size':dimensions==scene['settings']['size'],'jobs':jobs}
        for name in ('baseline','candidate'):
            row[name+'_median_seconds']=statistics.median(t for j in jobs if j['mode']==name for t in j['warm_seconds'])
        row['speedup']=row['baseline_median_seconds']/row['candidate_median_seconds']
        rows.append(row);write(output/'result.json',{'scope':'Scoped cold/warm A/B; full467/77 contract remains OPEN','rows':rows})
    verify_inputs(baseline);verify_inputs(candidate)

if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__)
    for name in ('baseline','candidate','output'):p.add_argument('--'+name,type=Path,required=True)
    p.add_argument('--scene',action='append',required=True);p.add_argument('--size',nargs=2,type=int)
    a=p.parse_args();run(a.baseline.resolve(),a.candidate.resolve(),a.output.resolve(),a.scene,a.size)
