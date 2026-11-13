#!/usr/bin/env python3
"""Admit existing final-identity pair frames after fresh same-engine controls.

This avoids re-rendering an already sealed exact triple. No event is rewritten
or copied, and all frame/resource/CPU-reference checks run through the common
gate. Only original-size non-numerical-outlier cases are accepted here.
"""
import argparse
import sys
from pathlib import Path


def main():
    p=argparse.ArgumentParser(description=__doc__)
    for key in ('repository','package','contract','build','pair','reference_package','report_output'):
        p.add_argument('--'+key.replace('_','-'),type=Path,required=True)
    p.add_argument('--retain-unqualified',action='store_true',help='Keep exact full triples/controls without a CPU reference; never add them to acceptance')
    a=p.parse_args();root=a.repository.resolve();sys.path.insert(0,str(root/'comparison/tools'))
    from qualify_gpu_parallel_pipeline import scope,evaluate
    from gpu_pipeline_binding import core_context,for_scene
    from gpu_pipeline_frames import frame,scene_frames,enabled,FLAGS
    from run_m3parameter_gpu import invoke,require,read,sha
    from prepare_gpu_parallel_bundle import refresh,ref,write
    from native_cpu_reference import saved_reference
    from native_numeric_review import pending
    from native_gpu_coverage import image_difference
    from report_gpu_acceptance import publish
    package=a.package.resolve();build=a.build.resolve();contract=a.contract.resolve()
    pair=read(a.pair);require(pair['status']=='PASS_SCOPED','Unfinished pair')
    c,formulas,scenes=scope(contract);ev=refresh(package,contract,build)
    ctx=core_context(root,ev['binding'],c,formulas,scenes,root)
    require(pair['products']['B']['mb3d_render']==ref(build/'mb3d_render') and
        pair['products']['B']['metal_native_scene.metallib']==ref(build/'metal_native_scene.metallib'),'Different candidate')
    ranks={x['rank']:x['id'] for x in read(root/'comparison/scene-queue.json')['rows']}
    for record in pair['rows']:
        sid=ranks[record['rank']];scene=scenes[sid];context=for_scene(ctx,sid)
        require(record['original_size'] and record['size']==scene['settings']['size'] and record['scene']==scene['path'],'Not original scene')
        require(not pending(scene),'Numerical review required separately')
        require(not any(x['id']==sid for x in ev['scenes']),'Already admitted')
        out=package/'scene-runs'/sid;require(not out.exists(),'Fresh case directory required');out.mkdir(parents=True)
        result={'rank':record['rank'],'id':sid,'scene':scene['path'],'status':'RUNNING','resource_profile':context['resource_profile'],
            'selected_map_ids':context['selected_map_ids'],'runtime_files':context['runtime_files'],'stages':[], 'adopted_from_pair':ref(a.pair)}
        def save():write(out/'case.json',result)
        save()
        try:
            jobs=[x for x in record['jobs'] if x['side']=='B' and not x['warmup']]
            require(len(jobs)==3,'Three measured candidate frames required')
            frames=[{k:x[k] for k in ('event','profile','image')} for x in jobs]
            for x in frames:frame(root,x,scene,context)
            reused=saved_reference(root,a.reference_package.resolve(),scene,context)
            require(reused is not None or a.retain_unqualified,'No reusable CPU reference')
            reference,origin=reused if reused else (None,None)
            controls=[]
            for stage,on in enabled(scene).items():
                if not on:continue
                folder=out/('off-'+stage);argv=[build/'mb3d_render',context['inputs'][sid],'--backend','metal','--precision','native',
                    '--width',record['size'][0],'--height',record['size'][1],'--threads',context['threads'],
                    '--metal-library',context['library_path'],'--output',folder/'image.png','--profile-json',folder/'profile.json',FLAGS[stage]]
                print('CONTROL',record['rank'],stage,flush=True)
                event=invoke(argv,folder,context['runtime'],300,evidence_root=root)
                require(event['exit_code']==0 and event['guard'] is None and event['seconds']<=180 and not event.get('frame_warning_emitted'),'Control failed or slow')
                off={k:ref(folder/name) for k,name in (('event','event.json'),('profile','profile.json'),('image','image.png'))}
                frame(root,off,scene,context,stage);controls.append({'stage':stage,'off':off})
            proof={'scene_sha256':scene['sha256'],'runtime_files':context['runtime_files'],'controls':controls}
            if reference is None:
                scene_frames(root,{'frames':frames},scene,context,proof)
                result.update(status='OPEN_CPU_REFERENCE',frames=frames,controls=controls,cpu_reference=None,
                    cpu_reference_status='MISSING_NOT_QUALIFIED',cpu_difference=None,
                    reason='Exact full GPU repeats/effects only; missing CPU reference keeps this outside acceptance')
                save();qualification=evaluate(contract,package);write(package/'qualification.json',qualification)
                publish(package,output=a.report_output,write_plan=False)
                print('RETAINED UNQUALIFIED GPU TRIPLE',record['rank'],flush=True);continue
            row={'id':sid,'qualification':'QUALIFIED_GPU','resource_profile':context['resource_profile'],'frames':frames,
                'cpu_reference':reference,'cpu_reference_origin':origin,'producer':'admit_native_pair_frames.py',
                'reviewer':'Common raw-frame validator; no event or image rewritten'}
            scene_frames(root,row,scene,context,proof)
            observations=package/'observations'/(sid+'-effects.json');write(observations,proof);row['effect_observations']=ref(observations)
            ev=read(package/'evidence.json');ev['scenes'].append(row);write(package/'evidence.json',ev);refresh(package,contract,build)
            qualification=evaluate(contract,package);write(package/'qualification.json',qualification)
            require(sid in qualification['scene_outcomes'],'Common gate did not admit scene')
            result.update(status='QUALIFIED_GPU',frames=frames,controls=controls,cpu_reference=reference,cpu_reference_origin=origin,
                cpu_reference_status='REUSED_COMPLETE',cpu_difference=image_difference(root/reference['image']['path'],root/frames[0]['image']['path']))
            save();publish(package,output=a.report_output,write_plan=False);print('ACCEPTED SEALED TRIPLE',record['rank'],flush=True)
        except (ValueError,KeyError,TypeError,OSError) as error:
            result.update(status='OPEN_CHECK_FAILED',reason=str(error));save();raise


if __name__=='__main__':main()
