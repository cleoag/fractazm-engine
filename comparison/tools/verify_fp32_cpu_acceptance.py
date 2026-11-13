#!/usr/bin/env python3
"""Qualify original full scenes with proven FP32 range fallback; no map replacement."""
import argparse,json
from pathlib import Path
from qualify_native_metal_all import sha,require
from run_m3parameter_gpu import invoke
from gpu_pipeline_frames import enabled,FLAGS
from gpu_pipeline_fp32 import scene_frames,frame as validate_frame
from qualify_gpu_parallel_pipeline import scope
ROOT=Path(__file__).resolve().parents[2]


def main():
    p=argparse.ArgumentParser(description=__doc__)
    for name in ('build','resources','recorder','output'):p.add_argument('--'+name,type=Path,required=True)
    p.add_argument('--contract',type=Path,default=ROOT/'comparison/contracts/gpu-parallel-legacy-v2/acceptance-contract.json')
    p.add_argument('--reuse-exclude',type=Path);p.add_argument('--resume',action='store_true');a=p.parse_args()
    b=a.build.resolve();out=a.output.resolve();resources=a.resources.resolve();runtime=resources/'runtime'
    require(a.resume or not out.exists(),'Existing output needs --resume');out.mkdir(parents=True,exist_ok=a.resume)
    def ref(path):return {'path':str(path.resolve().relative_to(ROOT)),'sha256':sha(path)}
    def row(folder,capture=False):
        value={k:ref(folder/v) for k,v in (('event','event.json'),('profile','profile.json'),('image','image.png'))}
        if capture:value['capture']=ref(folder/'capture.gz')
        return value
    identities={n:sha(b/n) for n in ('mb3d_render','metal_native_scene.metallib')}
    files={str(x.relative_to(runtime)):sha(x) for x in runtime.rglob('*') if x.is_file()}
    queue=json.loads((ROOT/'comparison/scene-queue.json').read_text())['rows']
    manifest=json.loads((resources/'resources.json').read_text());selected=[x for x in queue if x['rank'] in (25,30)]
    require(len(selected)==2 and not {x['scene'] for x in selected}&set(manifest['blocked_scenes']),'Missing original resources')
    contract,_,contract_scenes=scope(a.contract.resolve())
    require(contract['policies'].get('whole_frame_fp32_fallback',{}).get('accepted') is True,'FP32 CPU acceptance is not approved')
    report={'contract':ref(a.contract.resolve()),'status':'OPEN','scope':'two original scenes; full 467/77 contract remains OPEN','identities':identities,'runtime_files':files,'scenes':[]}
    def save():(out/'report.json').write_text(json.dumps(report,indent=2)+'\n')
    for q in selected:
        scene=contract_scenes[q['id']];source=ROOT/q['scene'];require(sha(source)==scene['sha256'],'Scene changed')
        ctx={'renderer':{'path':str(b/'mb3d_render'),'sha256':identities['mb3d_render']},'library':ref(b/'metal_native_scene.metallib'),
             'library_path':b/'metal_native_scene.metallib','inputs':{scene['id']:source},'threads':6,'runtime':runtime,'runtime_files':files}
        folder=out/scene['id']
        def run(label,backend,off=None):
            dest=folder/label
            if not(a.resume and (dest/'event.json').exists()):
                require(not dest.exists(),'Inspect partial job before retry: '+str(dest))
                argv=[b/'mb3d_render',source,'--backend',backend,'--precision','native','--threads','6','--output',dest/'image.png','--profile-json',dest/'profile.json']
                if backend=='metal':argv+=['--metal-library',b/'metal_native_scene.metallib']
                if off:argv+=[FLAGS[off]]
                e=invoke(argv,dest,runtime,timeout=300,evidence_root=ROOT)
                require(not e['exit_code'] and not e['guard'] and e['seconds']<=180,'Stop failed/slow frame before further repeats')
            value=row(dest);validate_frame(ROOT,value,scene,ctx,fallback=backend=='metal',off=off);return value
        if q['rank']==25 and a.reuse_exclude:
            prior=a.reuse_exclude.resolve();frames=[row(prior/('repeat-'+str(i))) for i in range(3)];direct=row(prior/'direct-cpu')
        else:
            frames=[run('repeat-'+str(i),'metal') for i in range(3)];direct=run('direct-cpu','cpu')
        controls=[{'stage':k,'off':run('off-'+k,'cpu',k)} for k,v in enabled(scene).items() if v]
        proof={'scene_sha256':scene['sha256'],'runtime_files':files,'controls':controls}
        diagnostic=a.recorder.resolve()/(source.stem+'-original')
        evidence={'qualification':'QUALIFIED_FP32_CPU','frames':frames,'direct_cpu':direct,
            'fp32_overflow_proof':{'recorder':ref(a.recorder.resolve()/'recorder.json'),
                                  'baseline':row(diagnostic/'baseline',True),'witness':row(diagnostic/'witness',True)}}
        folder.mkdir(parents=True,exist_ok=True)
        (folder/'evidence.json').write_text(json.dumps({'scene':scene,'row':evidence,'effects':proof},indent=2)+'\n')
        result=scene_frames(ROOT,evidence,scene,ctx,proof)
        report['scenes'].append(dict(result,id=scene['id'],scene=q['scene'],evidence=ref(folder/'evidence.json')));save()
        print(q['rank'],result['status'],result['proven_pixels'],result['geometry_pixels'],flush=True)
    require(all(sha(b/k)==v for k,v in identities.items()),'Build changed during qualification')
    require(files=={str(x.relative_to(runtime)):sha(x) for x in runtime.rglob('*') if x.is_file()},'Resources changed during qualification')
    report['status']='PASS_FP32_CPU_SCENES';report['qualified_gpu_scenes']=0;report['qualified_fp32_cpu_scenes']=len(report['scenes']);save()
    print(report['status'],flush=True)


if __name__=='__main__':main()
