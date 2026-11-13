#!/usr/bin/env python3
"""Replay full-frame old/new AO mappings; never transfer scene qualification."""
import argparse
import copy
import json
import math
import sys
from pathlib import Path


def normalize(work, size, threads):
    w=copy.deepcopy(work);e=w['execution']
    volume={k:e.pop(k) for k in ('volume_map_cells','volume_map_dispatches','volume_map_range_exit_cells')}
    assert all(type(v) is int and v>=0 for v in volume.values())
    owners=min(threads,size[1]);saved=0;plan=e['execution_plan']
    new=f'rng:owner=2,order=3,batch={owners},group={owners},quota=128,collective=1,unit=2,semantics=2,chunks=8'
    old=f'rng:owner=2,order=3,batch={owners},group=0,quota=128,collective=0,unit=2,semantics=2'
    if ',chunks=8' in plan:
        assert plan.count(new)==1,'Unexpected AO collective contract'
        plan=plan.replace(new,old)
        chunks=math.ceil(math.ceil(size[1]/owners)*size[0]/128)
        saved=(chunks-math.ceil(chunks/8))*e['ao_certificate_levels']
        e['ao_commands']+=saved;e['ao_dispatches']+=saved
    plan=plan.replace(',chunks=1','');assert ',chunks=' not in plan
    e['execution_plan']=plan
    return w,{'saved_commands':saved,'volume_observations':volume}


def main():
    p=argparse.ArgumentParser(description=__doc__)
    for key in ('repository','baseline','candidate','contract','output'):
        p.add_argument('--'+key,type=Path,required=True)
    p.add_argument('--change',choices=('ao-continuation','gpu-seeds'),default='ao-continuation')
    p.add_argument('--ranks',help='Optional comma-separated completed queue ranks')
    a=p.parse_args();root=a.repository.resolve();sys.path.insert(0,str(root/'comparison/tools'))
    from qualify_gpu_parallel_pipeline import scope
    from gpu_pipeline_binding import core_context,for_scene
    from gpu_pipeline_frames import frame,enabled,ACTIVITY
    from prepare_gpu_parallel_bundle import ref
    from qualify_native_metal_all import require,sha
    c,formulas,scenes=scope(a.contract.resolve())
    def context(pkg):
        ev=json.loads((pkg/'evidence.json').read_text())
        return core_context(root,ev['binding'],c,formulas,scenes,root)
    oldctx=context(a.baseline.resolve());newctx=context(a.candidate.resolve())
    normalizer_source=Path(__file__)
    if a.change=='gpu-seeds':
        import native_seed_scan_evidence
        normalize_work=native_seed_scan_evidence.normalize
        normalizer_source=Path(native_seed_scan_evidence.__file__)
    else:normalize_work=normalize
    wanted=set(map(int,a.ranks.split(','))) if a.ranks else None
    rows=[]
    for path in sorted((a.candidate/'scene-runs').glob('*/case.json')):
        case=json.loads(path.read_text());sid=case['id']
        if wanted is not None and case['rank'] not in wanted:continue
        if len(case.get('frames',[]))!=3 or case.get('active_job'):continue
        result={'rank':case['rank'],'id':sid,'scene':case['scene'],'status':'OPEN'}
        try:
            scene=scenes[sid];oc=for_scene(oldctx,sid);nc=for_scene(newctx,sid)
            require(oc['threads']==nc['threads'] and oc['runtime_files']==nc['runtime_files'], 'Changed workers or resource bytes')
            folder=a.baseline/'scene-runs'/sid/'repeat-0'
            baseline={k:ref(folder/v) for k,v in (('event','event.json'),('profile','profile.json'),('image','image.png'))}
            old=frame(root,baseline,scene,oc)
            observed=[]
            for r in case['frames']:
                current=frame(root,r,scene,nc);logical,physical=normalize_work(current['work'],scene['settings']['size'],nc['threads'])
                observed.append(current)
                require(current['image']==old['image'],'Changed original-size RGB')
                require(logical==old['work'],'Changed logical work outside explicit physical mapping')
            for field in ('run_id','event_path','profile_path','image_path'):
                require(len({f[field] for f in observed})==3,'Copied/reused repeat '+field)
            require(all((f['work'],f['archive'])==(observed[0]['work'],observed[0]['archive']) for f in observed),'Candidate work/cache repeat differs')
            controls={r['stage']:r for r in case['controls']}
            require(len(controls)==len(case['controls']) and set(controls)=={k for k,v in enabled(scene).items() if v},'Incomplete/duplicate controls')
            ids={f['run_id'] for f in observed}
            for stage,control in controls.items():
                off=frame(root,control['off'],scene,nc,stage);group,key=ACTIVITY[stage]
                require(off['run_id'] not in ids,'Reused control');ids.add(off['run_id'])
                require(off['work']['execution']['engine']==observed[0]['work']['execution']['engine'],'Changed control engine')
                require(type(observed[0]['work'][group][key]) is int and observed[0]['work'][group][key]>0 and off['work'][group][key]==0,'No observed effect activity')
                require(off['image']!=observed[0]['image'],'No visible effect control')
            result.update(status='PASS_OWNERSHIP_AND_EFFECTS',baseline=baseline,frames=case['frames'],controls=case['controls'],physical_mapping=physical)
        except (ValueError,AssertionError,KeyError,TypeError,OSError) as error:
            result['error']=str(error)
        rows.append(result)
    out={'qualification':False,'checker_sha256':sha(Path(__file__)),'normalizer_sha256':sha(normalizer_source),'change':a.change,'status':'PASS_SCOPED' if rows and all(r['status']=='PASS_OWNERSHIP_AND_EFFECTS' for r in rows) else 'OPEN',
         'rows':sorted(rows,key=lambda r:r['rank']),'limits':['No CPU/GPU numeric review or common-scene qualification is inferred.','Historical counts are not transferred to the candidate.']}
    a.output.parent.mkdir(parents=True,exist_ok=True)
    with a.output.open('x') as f:json.dump(out,f,indent=2);f.write('\n')
    print(out['status'],len(rows),'full scenes')


if __name__=='__main__':main()
