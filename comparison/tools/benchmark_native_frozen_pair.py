#!/usr/bin/env python3
"""Guarded archive-warm A/B/B/A/A/B frames with authored RNG and active effects.

Each frame is a fresh CLI process. This is not a same-process warm benchmark or
full-contract qualification. Default comparison requires exact RGB and all
logical work; compilation observations are reported separately.
"""
import argparse
import json
import statistics
import sys
from pathlib import Path


def main():
    p=argparse.ArgumentParser(description=__doc__)
    for key in ('repository','package','contract','baseline_build','candidate_build','output'):
        p.add_argument('--'+key.replace('_','-'),type=Path,required=True)
    p.add_argument('--change',choices=('identical','gpu-seeds','ao-chunks'),default='identical')
    p.add_argument('--ranks',required=True);p.add_argument('--size',nargs=2,type=int)
    a=p.parse_args();root=a.repository.resolve();sys.path.insert(0,str(root/'comparison/tools'))
    from qualify_gpu_parallel_pipeline import scope
    from gpu_pipeline_binding import core_context,for_scene
    from gpu_pipeline_frames import logical_work,enabled
    from run_m3parameter_gpu import invoke,validate_profile,rgb,require,read,sha
    from prepare_gpu_parallel_bundle import ref,write
    out=a.output.resolve();require(not out.exists(),'Fresh output required');out.mkdir(parents=True)
    checker_inputs=[ref(Path(__file__))];normalizer=None
    if a.change!='identical':
        import importlib
        module=importlib.import_module({'gpu-seeds':'native_seed_scan_evidence','ao-chunks':'native_ao_chunk_evidence'}[a.change])
        normalizer=module.normalize;checker_inputs.append(ref(module.__file__))
    c,formulas,scenes=scope(a.contract.resolve());binding=read(a.package/'evidence.json')['binding']
    ctx=core_context(root,binding,c,formulas,scenes,root)
    builds={'A':a.baseline_build.resolve(),'B':a.candidate_build.resolve()}
    products={}
    for name,build in builds.items():
        receipt=read(build.parent/'build-receipt.json');require(receipt['status']=='PASS_CAPTURED_BUILD_CLOSURE','Uncaptured build')
        indexed={r['path']:r['sha256'] for r in receipt['products']}
        products[name]={}
        for file in ('mb3d_render','metal_native_scene.metallib'):
            require(sha(build/file)==indexed['build/'+file],'Changed build product')
            products[name][file]=ref(build/file)
        products[name]['receipt']=ref(build.parent/'build-receipt.json')
    results=[];ranks=set(map(int,a.ranks.split(',')))
    for row in read(root/'comparison/scene-queue.json')['rows']:
        if row['rank'] not in ranks:continue
        sid=row['id'];scene=scenes[sid];context=for_scene(ctx,sid);size=a.size or scene['settings']['size'];jobs=[];signature=None
        for index,name in enumerate(('A','B','A','B','B','A','A','B')):
            warmup=index<2;folder=out/sid/(str(index)+'-'+name);build=builds[name]
            command=[build/'mb3d_render',context['inputs'][sid],'--backend','metal','--precision','native',
                '--width',size[0],'--height',size[1],'--threads',context['threads'],
                '--metal-library',build/'metal_native_scene.metallib','--output',folder/'image.png','--profile-json',folder/'profile.json']
            print('START',row['rank'],name,index,size,flush=True)
            event=invoke(command,folder,context['runtime'],300,evidence_root=root,input_files=[root/v['path'] for v in checker_inputs])
            require(event['exit_code']==0 and event['guard'] is None and not event.get('frame_warning_emitted') and event['seconds']<=180,'Frame failed or exceeded diagnostic threshold')
            profile=read(folder/'profile.json');validated=validate_profile(profile,'metal',size)
            require(event['resource_files_before']==event['resource_files_after']==context['runtime_files'],'Resource change')
            for stage,on in enabled(scene).items():require(profile['stages'][stage]['state']==('completed' if on else 'not_run'),'Effect changed')
            archive=profile['metal_execution']['pipeline_archive']
            if not warmup:require(archive[0]>0 and archive[1]==0,'Measurement archive not warm')
            image,_=rgb(folder/'image.png',size);work=logical_work(validated)
            if name=='B' and normalizer is not None:
                work,_=normalizer(work,size,context['threads'])
            current=(image,work)
            if signature is None:signature=current
            require(current==signature,'Old/new exact RGB or work differs')
            jobs.append({'side':name,'warmup':warmup,'event':ref(folder/'event.json'),'profile':ref(folder/'profile.json'),
                'image':ref(folder/'image.png'),'archive':archive,'cli_seconds':event['seconds'],'render_seconds':profile['render_total_seconds']})
            write(out/'partial.json',{'results':results,'current':{'rank':row['rank'],'jobs':jobs}})
            print('PASS',name,round(profile['render_total_seconds'],6),archive,flush=True)
        medians={name:statistics.median(j['render_seconds'] for j in jobs if j['side']==name and not j['warmup']) for name in builds}
        results.append({'rank':row['rank'],'scene':scene['path'],'size':size,'original_size':size==scene['settings']['size'],
            'status':'PASS_EXACT_RGB_WORK','jobs':jobs,'medians_seconds':medians,'speedup':medians['A']/medians['B']})
        write(out/'report.json',{'qualification':False,'status':'PASS_SCOPED','process_mode':'fresh CLI processes; warm archive for six measured frames',
            'order':'warm A, warm B, A/B/B/A/A/B','products':products,'rows':results,'checker_sha256':sha(Path(__file__)),'checker_inputs':checker_inputs})
    require(results,'No scenes selected')
    for item in checker_inputs:require(sha(root/item['path'])==item['sha256'],'Checker changed during benchmark')
    for side,build in builds.items():
        for file in ('mb3d_render','metal_native_scene.metallib'):require(sha(build/file)==products[side][file]['sha256'],'Product changed during benchmark')


if __name__=='__main__':main()
