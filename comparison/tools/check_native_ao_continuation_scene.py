#!/usr/bin/env python3
"""Original-size old/new AO ownership check against sealed production evidence."""
import argparse
import copy
import json
import math
import sys
from pathlib import Path


def normalize(work,size,threads):
    work=copy.deepcopy(work);e=work['execution']
    for key in ('volume_map_cells','volume_map_dispatches','volume_map_range_exit_cells'):
        assert e.pop(key)==0,'This AO-only comparison does not erase active volume work'
    plan=e['execution_plan'];owners=min(threads,size[1]);old=f'rng:owner=2,order=3,batch={owners},group=0,quota=128,collective=0,unit=2,semantics=2'
    new=f'rng:owner=2,order=3,batch={owners},group={owners},quota=128,collective=1,unit=2,semantics=2,chunks=8'
    assert plan.count(new)==1,'Unexpected collective execution contract'
    plan=plan.replace(new,old).replace(',chunks=1','')
    assert ',chunks=' not in plan
    e['execution_plan']=plan
    chunks=math.ceil(math.ceil(size[1]/owners)*size[0]/128)
    saved=(chunks-math.ceil(chunks/8))*e['ao_certificate_levels']
    assert saved>0
    e['ao_commands']+=saved;e['ao_dispatches']+=saved
    return work,saved


def main():
    p=argparse.ArgumentParser(description=__doc__)
    for k in ('repository','package','contract','candidate-build','output'):p.add_argument('--'+k,type=Path,required=True)
    p.add_argument('--rank',type=int,default=7);p.add_argument('--resume',action='store_true')
    a=p.parse_args();root=a.repository.resolve();out=a.output.resolve();package=a.package.resolve();build=a.candidate_build.resolve()
    sys.path.insert(0,str(root/'comparison/tools'))
    from qualify_native_metal_all import read_json,require,sha
    from qualify_gpu_parallel_pipeline import scope
    from gpu_pipeline_binding import core_context,for_scene
    from gpu_pipeline_proofs import binary_identity
    from gpu_pipeline_frames import frame,scene_frames,enabled,FLAGS
    from run_m3parameter_gpu import invoke
    from prepare_gpu_parallel_bundle import ref,write
    c,formulas,scenes=scope(a.contract.resolve());binding=read_json(package/'evidence.json')['binding']
    ctx=core_context(root,binding,c,formulas,scenes,root)
    entry=next(r for r in read_json(root/'comparison/scene-queue.json')['rows'] if r['rank']==a.rank)
    sid=entry['id'];scene=scenes[sid];ctx=for_scene(ctx,sid);size=scene['settings']['size']
    def row(folder):return {k:ref(folder/v) for k,v in (('event','event.json'),('profile','profile.json'),('image','image.png'))}
    baseline=row(package/'scene-runs'/sid/'repeat-0');old=frame(root,baseline,scene,ctx)
    ctx=dict(ctx,renderer=binary_identity(root,ref(build/'mb3d_render')),library=ref(build/'metal_native_scene.metallib'),
             library_path=build/'metal_native_scene.metallib',build_dir=build)
    report={'qualification':False,'rank':a.rank,'scene':scene['path'],'size':size,'baseline':baseline,
            'checker_sha256':sha(Path(__file__)),
            'renderer':ref(build/'mb3d_render'),'library':ref(build/'metal_native_scene.metallib'),'frames':[],'controls':[]}
    def job(label,off=None):
        folder=out/label;argv=[ctx['renderer']['path'],str(ctx['inputs'][sid]),'--backend','metal','--precision','native',
            '--width',str(size[0]),'--height',str(size[1]),'--threads',str(ctx['threads']),
            '--output',str(folder/'image.png'),'--profile-json',str(folder/'profile.json'),
            '--metal-library',str(ctx['library_path'])]
        if off:argv.append(FLAGS[off])
        if (folder/'event.json').exists():
            require(a.resume,'Existing run requires --resume');e=read_json(folder/'event.json')
            require(e['argv']==argv,'Changed AO job')
        else:
            require(not folder.exists(),'Inspect partial AO job')
            print('START',label,size,flush=True);e=invoke(argv,folder,ctx['runtime'],timeout=300,evidence_root=root)
        require(e['exit_code']==0 and e['guard'] is None and e['seconds']<=180 and not e.get('frame_warning_emitted'),
                'Frame failed or exceeded diagnostic threshold; no further repeats')
        r=row(folder);f=frame(root,r,scene,ctx,off)
        print('DONE',label,round(e['seconds'],3),f['archive'],flush=True)
        return r,f
    try:
        archives=[]
        for i in range(3):
            r,f=job('repeat-'+str(i));canonical,saved=normalize(f['work'],size,ctx['threads'])
            require(f['image']==old['image'],'AO continuation changed original-size RGB')
            require(canonical==old['work'],'AO continuation changed logical work outside the explicit physical mapping')
            report['frames'].append(r);archives.append(f['archive']);report['saved_commands_per_frame']=saved;write(out/'report.json',report)
        if archives[0]!=archives[1]:
            require(archives[0][1]>0 and archives[1]==archives[2] and archives[1][1]==0,'Unexpected cache-state drift')
            r,f=job('repeat-3');canonical,_=normalize(f['work'],size,ctx['threads'])
            require(f['image']==old['image'] and canonical==old['work'] and f['archive']==archives[1],
                    'Additional warm frame differs')
            report['cold_warmup']=report['frames'].pop(0);report['frames'].append(r)
            write(out/'report.json',report)
        for stage,on in enabled(scene).items():
            if on:
                r,f=job('off-'+stage,stage);report['controls'].append({'stage':stage,'off':r});write(out/'report.json',report)
        proof={'scene_sha256':scene['sha256'],'runtime_files':ctx['runtime_files'],'controls':report['controls']}
        scene_frames(root,{'frames':report['frames']},scene,ctx,proof)
        report.update(status='PASS_FULL_AO_OWNERSHIP_AND_REPEATS',old_new_rgb_and_logical_work_exact=True,effects=proof)
    except (ValueError,AssertionError,KeyError,TypeError,OSError) as error:
        report.update(status='OPEN',error=str(error));print('OPEN',str(error),flush=True)
    write(out/'report.json',report)


if __name__=='__main__':main()
