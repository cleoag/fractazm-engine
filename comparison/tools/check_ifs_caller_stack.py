"""Audit gnarly/ducks stack dependence through the original Pascal IFS caller.

Passing means the experiment reproduced and localized the reference ambiguity;
it does NOT mean the C++ odd-Flip fallback has acquired numerical certification.
"""
import argparse
import json
import math
import struct
import subprocess
from pathlib import Path
from check_manowar_caller import ROOT, sha, run, equal


def main():
    ap=argparse.ArgumentParser(description=__doc__)
    for key in ('probe','oracle','manifest','output'):ap.add_argument('--'+key,type=Path,required=True)
    a=ap.parse_args();a.output.mkdir(parents=True,exist_ok=False)
    cases=json.loads(a.manifest.read_bytes())['cases']
    points=[[.2,.1,.05],[-.125,.25,.5],[1,-1,.125],[0,0,0]]
    report=dict(scope=__doc__,finished=False,results=[],identities={str(p):sha(p) for p in (a.probe,a.oracle,a.manifest,Path(__file__))})
    for name in ('gnarlyIFS','gnarly2IFS','gnarly3IFS','ducksIFS'):
        selected=[c for c in cases if c['target_formulas']==[name] and (c['id'].endswith('_early') or c['id'].endswith('_p8_alternate'))]
        assert len(selected)==2
        asset=ROOT/'mb3d/M3Formulas'/f'{name}.m3f'
        report['identities'][str(asset)]=sha(asset)
        code=a.output/(name+'.bin');code.write_bytes(bytes.fromhex(asset.read_text().split('[CODE]')[1].split('[END]')[0]))
        for case in selected:
            scene=ROOT/case['scene'];assert sha(scene)==case['sha256']
            directory=a.output/case['id'];directory.mkdir();blob=directory/'params.bin'
            subprocess.run([str(a.probe),name,str(scene),str(blob)],input='',text=True,capture_output=True,check=True,timeout=10)
            stdin=''.join(' '.join(map(str,p+[0,0]))+'\n' for p in points)
            values={mode:run([a.oracle,code,blob,'trace' if mode=='repeat' else mode],stdin,directory/(mode+'.txt')) for mode in ('plain','trace','repeat')}
            failures=[]
            for mode in values:
                assert len(values[mode])==len(points) and all(len(row)==12 for row in values[mode])
            for i,(p,t,r) in enumerate(zip(values['plain'],values['trace'],values['repeat'])):
                if not all(equal(x,y) for x,y in zip(p[:10],t[:10])):failures.append(dict(kind='instrumentation',row=i))
                if not all(equal(x,y) for x,y in zip(t[:10],r[:10])):failures.append(dict(kind='result_repeat',row=i))
            seeded={}
            for seed in (0.,2.,-2.):
                lo,hi=struct.unpack('<II',struct.pack('<d',seed))
                inp=''.join(' '.join(map(str,p+[lo,hi]))+'\n' for p in points)
                rows=run([a.oracle,code,blob,'seeded'],inp,directory/(f'seed_{seed}.txt'))
                assert len(rows)==len(points)
                for row in rows:
                    if row[-2:]!=[lo,hi]:failures.append(dict(kind='saved_register_mapping',seed=seed))
                seeded[seed]=rows
            changes=sum(not equal(seeded[0.][i][0],seeded[s][i][0]) for i in range(len(points)) for s in (2.,-2.))
            odd=case['id'].endswith('_p8_alternate')
            if (odd and changes==0) or (not odd and changes!=0):failures.append(dict(kind='odd_even_control',changed_distances=changes))
            measured=[struct.unpack('<d',struct.pack('<II',int(row[-2]),int(row[-1])))[0] for row in values['trace']]
            report['results'].append(dict(case=case['id'],formula=name,odd_flip=odd,status='fail' if failures else 'pass',
                changed_distances=changes,measured_caller_qwords=measured,examples=failures,scene_sha256=sha(scene),params_sha256=sha(blob)))
            print(case['id'],report['results'][-1]['status'],changes,flush=True)
    report['finished']=True
    report['artifact_sha256']={str(p.relative_to(a.output)):sha(p) for p in sorted(a.output.rglob('*')) if p.is_file()}
    (a.output/'summary.json').write_text(json.dumps(report,indent=2))
    return any(r['status']!='pass' for r in report['results'])


if __name__=='__main__':raise SystemExit(main())
