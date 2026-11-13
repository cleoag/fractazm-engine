"""Compare prepared live postprocess controls, not captured-primary buffers."""
import argparse
from collections import Counter
from dataclasses import asdict
import hashlib
import json
from pathlib import Path
import subprocess
import time

from compare_silight5 import FIELD_NAMES, read_dump
from image_diff import load_rgb, compute_metrics, DEFAULT_THRESHOLDS, evaluate

ROOT=Path(__file__).resolve().parents[2]
BASE=ROOT/'comparison/results/stage-a-20261018/effective-modes-v1'


def main():
    ap=argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--cpp-exe',type=Path,required=True)
    ap.add_argument('--output',type=Path,required=True)
    args=ap.parse_args();args.output.mkdir(parents=True,exist_ok=False)
    identities={}
    def keep(path):
        path=path.resolve();sha=hashlib.sha256(path.read_bytes()).hexdigest()
        identities[str(path)]=sha;return sha
    exe=args.cpp_exe.resolve();exe_hash=keep(exe)
    keep(Path(__file__))
    qpath=BASE/'live-qualification.json';keep(qpath)
    qualification=json.loads(qpath.read_bytes())
    if not qualification['finished']:raise ValueError('Incomplete live reference')
    manifest=BASE/'manifest.json'
    if keep(manifest)!=qualification['manifest_sha256']:raise ValueError('Changed manifest')
    cases={row['id']:row for row in json.loads(manifest.read_bytes())['cases']}
    extra=qualification['additional_scene'];cases[extra['id']]=extra
    report=dict(scope='Ordinary full-frame live normals and hard/soft-shadow controls; Monte Carlo/JIT excluded',
        binary_sha256=exe_hash,finished=False,thresholds=DEFAULT_THRESHOLDS,
        original_qualification=str(qpath),original_pascal_sha256=qualification['pascal_sha256'],
        original_interventions=qualification['comparisons'],results=[],input_sha256=identities)
    def save():
        (args.output/'summary.json').write_text(json.dumps(report,indent=2)+'\n')
    for original in qualification['results']:
        name=original['scene'];case=cases[name];scene=ROOT/case['scene']
        if keep(scene)!=case['sha256']:raise ValueError('Changed scene: '+name)
        png=BASE/'live'/(name+'-live.png');packed=png.with_suffix('.silight')
        if keep(png)!=original['pascal_image_sha256'] or keep(packed)!=original['silight_sha256']:
            raise ValueError('Changed live reference: '+name)
        if keep(BASE/original['pascal_log'])!=original['pascal_log_sha256']:
            raise ValueError('Changed original log: '+name)
        if keep(exe)!=exe_hash:raise ValueError('Changed candidate')
        target=args.output/(name+'.png');dump=target.with_suffix('.silight')
        command=[str(exe),str(scene),'--output',str(target.resolve()),'--threads','1',
                 '--silight5-dump',str(dump.resolve())]
        started=time.monotonic();run=subprocess.run(command,cwd=ROOT,capture_output=True,text=True,timeout=120)
        (args.output/(name+'.log')).write_text(run.stdout+run.stderr,encoding='utf-8')
        item=dict(id=name,command=command,returncode=run.returncode,
                  seconds=time.monotonic()-started,scene_sha256=case['sha256'])
        if run.returncode:
            item.update(status='error',error=run.stderr)
        else:
            metrics=compute_metrics(load_rgb(png),load_rgb(target),DEFAULT_THRESHOLDS['diff_ratio_threshold'])
            checks=evaluate(metrics,DEFAULT_THRESHOLDS)
            wa,ha,ra=read_dump(packed);wb,hb,rb=read_dump(dump)
            if (wa,ha,len(ra))!=(wb,hb,len(rb)):raise ValueError('Packed dimensions differ')
            fields={f:dict(changed=sum(a[i]!=b[i] for a,b in zip(ra,rb)),
                           max_abs=max(abs(a[i]-b[i]) for a,b in zip(ra,rb)))
                    for i,f in enumerate(FIELD_NAMES)}
            exact=all(v['changed']==0 for v in fields.values())
            item.update(status='pass' if all(checks.values()) else 'fail',
                state_status='pass' if exact else 'fail',exact_rgb=metrics.max_abs_delta==0,
                exact_state=exact,metrics=asdict(metrics),checks=checks,fields=fields,
                cpp_image_sha256=keep(target),cpp_state_sha256=keep(dump),
                pascal_image_sha256=original['pascal_image_sha256'],pascal_state_sha256=original['silight_sha256'])
        report['results'].append(item);save()
        print(name,item['status'],item.get('state_status'),flush=True)
    for path,expected in identities.items():
        if hashlib.sha256(Path(path).read_bytes()).hexdigest()!=expected:raise ValueError('Input changed: '+path)
    report.update(finished=True,counts=dict(Counter(r['status'] for r in report['results'])),
                  state_counts=dict(Counter(r.get('state_status','error') for r in report['results'])))
    save()
    return int(any(r['status']!='pass' or r.get('state_status')!='pass' for r in report['results']))


if __name__=='__main__':raise SystemExit(main())
