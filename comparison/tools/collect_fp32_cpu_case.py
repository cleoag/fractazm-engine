#!/usr/bin/env python3
"""Collect a pinned full FP32-CPU case only after an independent pixel proof."""
import argparse
import sys
from pathlib import Path


def main():
    p=argparse.ArgumentParser(description=__doc__)
    for k in ('repository','package','contract','recorder','output'):p.add_argument('--'+k,type=Path,required=True)
    p.add_argument('--rank',type=int,required=True);p.add_argument('--resume',action='store_true');a=p.parse_args()
    root=a.repository.resolve();out=a.output.resolve();package=a.package.resolve();record=a.recorder.resolve()
    sys.path.insert(0,str(root/'comparison/tools'))
    from qualify_native_metal_all import read_json,require,sha
    from qualify_gpu_parallel_pipeline import scope
    from gpu_pipeline_binding import core_context,for_scene
    from gpu_pipeline_fp32 import frame,scene_frames,recorder
    from gpu_pipeline_frames import enabled,FLAGS
    from fp32_overflow_witness import compare
    from run_m3parameter_gpu import invoke
    from prepare_gpu_parallel_bundle import write,ref
    c,f,scenes=scope(a.contract.resolve());binding=read_json(package/'evidence.json')['binding']
    ctx=core_context(root,binding,c,f,scenes,root)
    entry=next(r for r in read_json(root/'comparison/scene-queue.json')['rows'] if r['rank']==a.rank)
    sid=entry['id'];scene=scenes[sid];ctx=for_scene(ctx,sid);source=ctx['inputs'][sid]
    recorder(root,ref(record/'recorder.json'),ctx)
    diagnostic=record/(source.stem+'-original')
    proof=compare(diagnostic/'baseline/capture.gz',diagnostic/'witness/capture.gz')
    require(proof['size']==scene['settings']['size'],'Need original-size overflow proof before CPU repeats')
    def row(folder,capture=False):
        r={k:ref(folder/v) for k,v in (('event','event.json'),('profile','profile.json'),('image','image.png'))}
        if capture:r['capture']=ref(folder/'capture.gz')
        return r
    result={'status':'RUNNING','qualification':False,'rank':a.rank,'scene':scene,'frames':[],
            'pixel_proof':proof,'collector_sha256':sha(Path(__file__))}
    def run(label,backend,off=None):
        folder=out/label;argv=[ctx['renderer']['path'],str(source),'--backend',backend,'--precision','native',
            '--threads',str(ctx['threads']),'--output',str(folder/'image.png'),'--profile-json',str(folder/'profile.json')]
        if backend=='metal':argv+=['--metal-library',str(ctx['library_path'])]
        if off:argv.append(FLAGS[off])
        if (folder/'event.json').exists():
            require(a.resume,'Existing frame needs --resume');event=read_json(folder/'event.json')
            require(event['argv']==argv,'Changed CPU qualification command')
        else:
            require(not folder.exists(),'Inspect partial CPU frame')
            print('START FP32 CPU',a.rank,label,flush=True);event=invoke(argv,folder,ctx['runtime'],timeout=300,evidence_root=root)
        require(event['exit_code']==0 and event['guard'] is None and event['seconds']<=180,'Stop failed/slow CPU frame before repeats')
        value=row(folder);frame(root,value,scene,ctx,fallback=backend=='metal',off=off)
        print('DONE FP32 CPU',a.rank,label,round(event['seconds'],3),flush=True);return value
    try:
        for i in range(3):
            result['frames'].append(run('repeat-'+str(i),'metal'));write(out/'collection.json',result)
        direct=run('direct-cpu','cpu')
        controls=[{'stage':stage,'off':run('off-'+stage,'cpu',stage)} for stage,on in enabled(scene).items() if on]
        effects={'scene_sha256':scene['sha256'],'runtime_files':ctx['runtime_files'],'controls':controls}
        value={'id':sid,'qualification':'QUALIFIED_FP32_CPU','resource_profile':ctx['resource_profile'],
               'frames':result['frames'],'direct_cpu':direct,'fp32_overflow_proof':{'recorder':ref(record/'recorder.json'),
               'baseline':row(diagnostic/'baseline',True),'witness':row(diagnostic/'witness',True)}}
        observed=scene_frames(root,value,scene,ctx,effects)
        result.update(status='QUALIFIED_FP32_CPU',result=observed)
        write(out/'evidence.json',{'scene':scene,'row':value,'effects':effects})
    except (ValueError,KeyError,TypeError,OSError) as error:
        result.update(status='OPEN',error=str(error));print('OPEN',str(error),flush=True)
    write(out/'collection.json',result)


if __name__=='__main__':main()
