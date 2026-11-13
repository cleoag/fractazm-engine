"""Prepare the 287 finite selector images from original and recovery references.

The first two historical numerical failures are included, not filtered. The
50 absent original PNGs use the separate recovery capture with its own identity.
"""
import argparse
from collections import Counter
import hashlib
import json
from pathlib import Path
import sys

ROOT=Path(__file__).resolve().parents[2]
BASE=ROOT/'comparison/runs/stage-a-20261018/selectors-remaining-v1'


def main():
    ap=argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--cpp-exe',type=Path,required=True)
    ap.add_argument('--output',type=Path,required=True)
    args=ap.parse_args();args.output.mkdir(parents=True,exist_ok=False)
    inputs={}
    def keep(path):
        path=path.resolve();raw=path.read_bytes()
        inputs[str(path)]=hashlib.sha256(raw).hexdigest()
        return inputs[str(path)]
    keep(Path(__file__))
    for tool in ['replay_parity.py','external_validation.py','run_stage_a_replay.py']:
        keep(ROOT/'comparison/tools'/tool)
    jobs=[];all_ids=set();historical=[]
    for index,(folder,manifest_name,expected) in enumerate([
        ('reference-and-v7','image-manifest.json',237),
        ('recovery-v9','recovery-manifest-v1.json',50)]):
        ref=BASE/folder/'summary.json';manifest=BASE/manifest_name
        keep(ref);manifest_hash=keep(manifest)
        data=json.loads(ref.read_bytes());cases=json.loads(manifest.read_bytes())['cases']
        if not data['finished'] or data['manifest_sha256']!=manifest_hash:
            raise ValueError('Unfinished/changed reference')
        by_name={row['id']:row for row in cases};selected=[]
        for row in data['results']:
            if folder=='reference-and-v7' and row['status']=='error':
                continue
            name=row['scene'];case=by_name[name]
            if keep(ROOT/case['scene'])!=case['sha256'] or case['sha256']!=row['scene_sha256']:
                raise ValueError('Scene identity changed: '+name)
            if keep(ref.parent/'pascal'/(name+'.png'))!=row['pascal_image_sha256']:
                raise ValueError('Original PNG identity changed: '+name)
            if keep(ref.parent/row['pascal_log'])!=row['pascal_log_sha256']:
                raise ValueError('Original log identity changed: '+name)
            selected.append(name)
        if len(selected)!=expected or all_ids.intersection(selected):
            raise ValueError('Selector count/overlap differs')
        all_ids.update(selected)
        case_list=args.output/(folder+'-cases.json')
        case_list.write_text(json.dumps(selected,indent=2)+'\n');keep(case_list)
        out=args.output/folder
        command=[sys.executable,'-X','utf8',str(ROOT/'comparison/tools/replay_parity.py'),
                 '--reference',str(ref),'--manifest',str(manifest),'--case-list',str(case_list.resolve()),
                 '--cpp-exe',str(args.cpp_exe.resolve()),'--output',str(out.resolve()),'--checkpoint-every','16']
        jobs.append(dict(index=index,output=str(out.resolve()),command=command,cases=selected))
        historical.append(dict(report=str(ref),counts=dict(Counter(row['status'] for row in data['results'])),
            retained_numerical_failures=[row['scene'] for row in data['results'] if row['status']=='fail']))
    if len(all_ids)!=287:raise ValueError('Expected 287 selectors')
    plan=dict(scope='Finite ordinary selector287; not generic external-suite replacement',
        cpp_exe=str(args.cpp_exe.resolve()),cpp_sha256=keep(args.cpp_exe),input_sha256=inputs,
        total_original_cases=287,scheduled_cases=287,excluded_from_numeric_replay=[],unresolved=[],
        jobs=jobs,historical_reports=historical,
        note='Original 235PASS/2FAIL/50ERROR report retained unchanged; only missing references use the separately captured recovery50.')
    (args.output/'plan.json').write_text(json.dumps(plan,indent=2)+'\n')
    print(json.dumps(dict(scheduled_cases=287,inputs=len(inputs),jobs=2)))


if __name__=='__main__':main()
