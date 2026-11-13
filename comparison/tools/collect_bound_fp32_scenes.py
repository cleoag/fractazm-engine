#!/usr/bin/env python3
"""Collect final-identity FP32 CPU outcomes using approved resource profiles.

Independent full-frame recorder captures must exist first. CPU controls use the
same exclusive measurement slot as GPU attempts; no shared manifest is edited.
"""
import argparse,json
from pathlib import Path
from qualify_native_metal_all import ROOT,require,read_json,sha
from qualify_gpu_parallel_pipeline import scope
from gpu_pipeline_binding import core_context,for_scene
from gpu_pipeline_frames import enabled,FLAGS
from gpu_pipeline_fp32 import frame,scene_frames
from run_m3parameter_gpu import invoke
from prepare_gpu_parallel_bundle import ref,write


def main():
    p=argparse.ArgumentParser(description=__doc__)
    for name in ('package','contract','recorder','output'):p.add_argument('--'+name,type=Path,required=True)
    p.add_argument('--ranks',default='25,30,50');p.add_argument('--resume',action='store_true');a=p.parse_args()
    package=a.package.resolve();out=a.output.resolve();recorder=a.recorder.resolve()
    require(not out.exists() or a.resume,'Existing collection requires --resume');out.mkdir(parents=True,exist_ok=a.resume)
    c,f,scenes=scope(a.contract.resolve());binding=read_json(package/'evidence.json')['binding']
    ctx=core_context(ROOT,binding,c,f,scenes,ROOT);ranks=set(map(int,a.ranks.split(',')))
    require(ranks and ranks<={25,30,50},'This collector covers only the three proven short cases')
    queue=[q for q in read_json(ROOT/'comparison/scene-queue.json')['rows'] if q['rank'] in ranks]
    report={'status':'OPEN','qualification_scope':'three previously proved cases on the new identity',
            'renderer':binding['renderer'],'library':binding['library'],'scenes':[]}
    def row(folder,capture=False):
        r={k:ref(folder/n) for k,n in (('event','event.json'),('profile','profile.json'),('image','image.png'))}
        if capture:r['capture']=ref(folder/'capture.gz')
        return r
    for q in queue:
        sid=q['id'];scene=scenes[sid];profile='available' if q['rank']==50 else 'original'
        require(profile in c['scene_resource_profiles'][sid],'Unapproved resource profile')
        scoped=dict(ctx,scene_resources=dict(ctx['scene_resources'],**{sid:profile}));scoped=for_scene(scoped,sid)
        folder=out/sid;folder.mkdir(exist_ok=a.resume)
        def run(label,backend,off=None):
            dest=folder/label;argv=[scoped['renderer']['path'],scoped['inputs'][sid],'--backend',backend,
                '--precision','native','--threads','6','--output',dest/'image.png','--profile-json',dest/'profile.json']
            if backend=='metal':argv+=['--metal-library',scoped['library_path']]
            if off:argv+=[FLAGS[off]]
            if (dest/'event.json').exists():
                require(a.resume,'Existing frame');event=read_json(dest/'event.json')
                require(event['argv']==list(map(str,argv)),'Changed saved command')
            else:
                require(not dest.exists(),'Inspect partial frame before retry')
                print('START',q['rank'],label,flush=True)
                event=invoke(argv,dest,scoped['runtime'],300,evidence_root=ROOT,exclusive_gpu=True)
            require(event['exit_code']==0 and event['guard'] is None and event['seconds']<=180,'Failed/slow frame; do not repeat blindly')
            result=row(dest);frame(ROOT,result,scene,scoped,fallback=backend=='metal',off=off)
            print('DONE',q['rank'],label,round(event['seconds'],3),flush=True);return result
        frames=[run('repeat-'+str(i),'metal') for i in range(3)];direct=run('direct-cpu','cpu')
        controls=[{'stage':k,'off':run('off-'+k,'cpu',k)} for k,v in enabled(scene).items() if v]
        effects={'scene_sha256':scene['sha256'],'runtime_files':scoped['runtime_files'],'controls':controls}
        diagnostic=recorder/(Path(scene['path']).stem+'-original')
        evidence={'qualification':'QUALIFIED_FP32_CPU','frames':frames,'direct_cpu':direct,
            'fp32_overflow_proof':{'recorder':ref(recorder/'recorder.json'),'baseline':row(diagnostic/'baseline',True),
                                   'witness':row(diagnostic/'witness',True)}}
        payload=folder/'evidence.json';write(payload,{'scene':scene,'row':evidence,'effects':effects})
        result=scene_frames(ROOT,evidence,scene,scoped,effects)
        report['scenes'].append(dict(result,id=sid,scene=scene['path'],resource_profile=profile,
            producer='collect_bound_fp32_scenes.py',evidence=ref(payload)));write(out/'report.json',report)
        print('VERIFIED',q['rank'],result['status'],flush=True)
    report['status']='PASS_FP32_CPU_SCENES';report['qualified_gpu_scenes']=0;report['qualified_fp32_cpu_scenes']=len(report['scenes'])
    write(out/'report.json',report);print(report['status'],len(report['scenes']),flush=True)

if __name__=='__main__':main()
