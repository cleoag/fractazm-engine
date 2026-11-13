#!/usr/bin/env python3
"""Revalidate and import sealed GPU scene rows without moving their artifacts.

Use only while the target queue is idle. An unsuccessful previous capture stays
at its original paths and its case metadata is archived before replacement.
"""
import argparse,copy,sys
from pathlib import Path


def main():
    p=argparse.ArgumentParser(description=__doc__)
    for key in ('repository','source','target','contract','build','report_output'):
        p.add_argument('--'+key.replace('_','-'),type=Path,required=True)
    p.add_argument('--ranks',required=True);a=p.parse_args();root=a.repository.resolve()
    sys.path.insert(0,str(root/'comparison/tools'))
    from qualify_gpu_parallel_pipeline import scope,evaluate
    from gpu_pipeline_binding import core_context,for_scene
    from gpu_pipeline_frames import scene_frames
    from prepare_gpu_parallel_bundle import refresh,write,ref
    from run_m3parameter_gpu import read,require,sha
    from report_gpu_acceptance import publish
    source=a.source.resolve();target=a.target.resolve();contract=a.contract.resolve()
    require(source!=target and source.is_relative_to(root) and target.is_relative_to(root),'Distinct repository packages required')
    c,formulas,scenes=scope(contract);accepted=evaluate(contract,source).get('scene_outcomes',{})
    source_ev=read(source/'evidence.json');ev=refresh(target,contract,a.build.resolve())
    ctx=core_context(root,ev['binding'],c,formulas,scenes,root)
    ranks={r['rank']:r['id'] for r in read(root/'comparison/scene-queue.json')['rows']}
    requested=set(map(int,a.ranks.split(',')));require(requested<=set(ranks),'Unknown rank')
    pending=[]
    for rank in sorted(requested):
        sid=ranks[rank];require(accepted.get(sid)=='QUALIFIED_GPU','Source scene is not qualified')
        require(not any(r['id']==sid for r in ev['scenes']),'Target scene already published')
        row=copy.deepcopy(next(r for r in source_ev['scenes'] if r['id']==sid))
        obs=row['effect_observations'];require(sha(root/obs['path'])==obs['sha256'],'Source observations changed')
        scene_frames(root,row,scenes[sid],for_scene(ctx,sid),read(root/obs['path']))
        case_path=source/'scene-runs'/sid/'case.json';case=read(case_path)
        require(case['status']=='QUALIFIED_GPU' and case['frames']==row['frames'],'Source metadata disagrees')
        case['adopted_from_package']={'manifest':ref(source/'evidence.json'),'case':ref(case_path)}
        pending.append((sid,row,case))
    for sid,row,case in pending:
        out=target/'scene-runs'/sid;out.mkdir(parents=True,exist_ok=True)
        previous=out/'case.json'
        if previous.exists():
            history=out/'case-history'/(sha(previous)+'.json')
            if not history.exists():write(history,read(previous))
        ev['scenes'].append(row);write(previous,case)
    write(target/'evidence.json',ev);refresh(target,contract,a.build.resolve())
    q=evaluate(contract,target);write(target/'qualification.json',q)
    require(all(q.get('scene_outcomes',{}).get(sid)=='QUALIFIED_GPU' for sid,_,_ in pending),'Target qualification failed')
    publish(target,output=a.report_output,write_plan=False)
    print('ADMITTED',len(pending),'sealed scenes; original paths and events retained')

if __name__=='__main__':main()
