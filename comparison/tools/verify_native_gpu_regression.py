#!/usr/bin/env python3
"""Three exact repeats and old/new RGB/work on selected ordinary GPU scenes.

Original resources and settings are mandatory. This is scoped regression
coverage, not independent whole-project or effect qualification.
"""
import argparse
import json
from pathlib import Path
from native_gpu_coverage import rgb_sha256, sha256, image_difference
from run_m3parameter_gpu import invoke
from verify_native_range_fallback import work
ROOT=Path(__file__).resolve().parents[2]


def main():
    p=argparse.ArgumentParser(description=__doc__)
    for k in ('baseline','candidate','resources','output'):p.add_argument('--'+k,type=Path,required=True)
    p.add_argument('--ranks',type=int,nargs='+',required=True);p.add_argument('--original',action='store_true')
    a=p.parse_args();out=a.output.resolve();assert not out.exists();out.mkdir(parents=True)
    queue=json.loads((ROOT/'comparison/scene-queue.json').read_text())['rows']
    rows=[r for r in queue if r['rank'] in a.ranks];assert len(rows)==len(set(a.ranks))
    resources=a.resources.resolve();manifest=json.loads((resources/'resources.json').read_text());runtime=resources/'runtime'
    assert not set(r['scene'] for r in rows)&set(manifest['blocked_scenes'])
    if a.original:assert all(r['priority']!='deferred_slow_tail' for r in rows)
    for item in manifest['resources']:
        if item['status']=='AVAILABLE_DECODED':assert sha256(resources/item['path'])==item['sha256']
    inventory={r['path']:r for r in json.loads((ROOT/'comparison/results/m3parameter-gpu-priority-20261024/inventory.json').read_text())['scenes']}
    builds={k:getattr(a,k).resolve() for k in ('baseline','candidate')}
    identities={str(b/n):sha256(b/n) for b in builds.values() for n in ('mb3d_render','metal_native_scene.metallib')}
    report={'qualification':False,'original_size':a.original,'identities':identities,'resource_manifest_sha256':sha256(resources/'resources.json'),'results':[]}

    def frame(row,label,build):
        folder=out/str(row['rank'])/label;scene=ROOT/row['scene'];assert sha256(scene)==inventory[row['scene']]['sha256']
        cmd=[build/'mb3d_render',scene,'--backend','metal','--precision','native','--threads','6',
            '--metal-library',build/'metal_native_scene.metallib','--output',folder/'image.png','--profile-json',folder/'profile.json']
        if not a.original:cmd+=['--width','160','--height','120']
        event=invoke(cmd,folder,runtime,timeout=300,evidence_root=ROOT)
        assert not event['exit_code'] and not event['guard'] and not event['frame_warning_emitted'], 'Failed/slow frame: '+str(folder)
        d=json.loads((folder/'profile.json').read_text());m=d['metal_execution']
        assert d['status']=='complete' and d['backend']=='metal' and not d['backend_fallback'] and d['precision']=='native'
        assert m['validated'] and not m['faults'] and not m['nonfinite']
        assert [d['width'],d['height']]==(inventory[row['scene']]['settings']['size'] if a.original else [160,120])
        return folder,d
    for row in rows:
        result={'rank':row['rank'],'scene':row['scene'],'status':'OPEN'};report['results'].append(result)
        try:
            frames=[frame(row,'repeat-'+str(i),builds['candidate']) for i in range(3)]
            hashes=[rgb_sha256(f/'image.png') for f,_ in frames]
            assert len(set(hashes))==1 and all(work(d)==work(frames[0][1]) for _,d in frames), 'Repeat mismatch'
            old,oldp=frame(row,'baseline',builds['baseline'])
            difference=image_difference(old/'image.png',frames[0][0]/'image.png')
            same_work=work(oldp)==work(frames[0][1]);same_rgb=rgb_sha256(old/'image.png')==hashes[0]
            result.update(exact_repeats=3,old_new_rgb_exact=same_rgb,old_new_work_exact=same_work,
                difference=difference,rgb_sha256=hashes[0],gpu_seconds=[d['render_total_seconds'] for _,d in frames],
                archive_states=[d['metal_execution']['pipeline_archive'] for _,d in frames])
            assert same_rgb and same_work,'Old/new regression: inspect evidence before changing reference'
            result['status']='PASS_ORIGINAL_REGRESSION' if a.original else 'PASS_PREVIEW_ONLY'
        except (AssertionError,OSError,ValueError,KeyError) as error:result['error']=str(error)
        (out/'report.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(result),flush=True)
    assert all(sha256(Path(f))==h for f,h in identities.items())
    assert sha256(resources/'resources.json')==report['resource_manifest_sha256']
    for item in manifest['resources']:
        if item['status']=='AVAILABLE_DECODED':assert sha256(resources/item['path'])==item['sha256']
    return int(any(r['status']=='OPEN' for r in report['results']))


if __name__=='__main__':raise SystemExit(main())
