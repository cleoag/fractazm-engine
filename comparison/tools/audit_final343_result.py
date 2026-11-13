"""Seal an ordinary final343 replay against its prepared reference snapshot."""
import argparse
from collections import Counter
import hashlib
import json
from pathlib import Path

from image_diff import DEFAULT_THRESHOLDS

ROOT=Path(__file__).resolve().parents[2]


def main():
    ap=argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--preflight',type=Path,default=ROOT/'comparison/results/stage-a-20261018/final343-reference-preflight-v1.json')
    ap.add_argument('--candidate',type=Path,required=True)
    ap.add_argument('--replay',type=Path,required=True)
    ap.add_argument('--output',type=Path,required=True)
    args=ap.parse_args();identities={}
    def keep(path):
        path=path.resolve();raw=path.read_bytes()
        identities[str(path)]=hashlib.sha256(raw).hexdigest();return raw
    snapshot=json.loads(keep(args.preflight))
    for path,expected in snapshot['input_sha256'].items():
        if hashlib.sha256(keep(ROOT/path)).hexdigest()!=expected:
            raise ValueError('Prepared reference changed: '+path)
    binary=args.candidate/'mb3d_render.exe'
    binary_hash=hashlib.sha256(keep(binary)).hexdigest()
    build=json.loads(keep(args.candidate/'build-identity.json'))
    if binary_hash not in {r.get('Hash',r.get('sha256')).lower() for r in build['binaries']}:
        raise ValueError('Build identity mismatch')
    summary=json.loads(keep(args.replay/'summary.json'))
    if not summary['finished'] or summary['cpp_sha256']!=binary_hash:
        raise ValueError('Incomplete/wrong-candidate replay')
    if summary['thresholds']!=DEFAULT_THRESHOLDS:
        raise ValueError('Threshold contract differs')
    expected={row['id']:row for row in snapshot['cases']}
    seen=set();suite_counts={}
    for row in summary['results']:
        key=row['suite']+':'+row['scene']
        if key not in expected or key in seen:raise ValueError('Unexpected/repeated case: '+key)
        seen.add(key);reference=expected[key]
        if row['scene_sha256']!=snapshot['input_sha256'][reference['scene_path']]:
            raise ValueError('Scene mismatch: '+key)
        if row.get('status') in ('pass','fail'):
            if row['pascal_image_sha256']!=snapshot['input_sha256'][reference['pascal_image']]:
                raise ValueError('Original image mismatch: '+key)
            image=args.replay/'cpp'/row['suite']/(row['scene']+'.png')
            if hashlib.sha256(keep(image)).hexdigest()!=row['cpp_image_sha256']:
                raise ValueError('Candidate image changed: '+key)
        suite_counts.setdefault(row['suite'],Counter())[row['status']]+=1
    if seen!=set(expected) or len(seen)!=343:raise ValueError('Final343 denominator changed')
    counts=dict(Counter(row['status'] for row in summary['results']))
    report=dict(scope='Frozen ordinary 343-scene image regression only; Monte Carlo/JIT excluded',
        cpp_sha256=binary_hash,finished=True,measured_cases=343,counts=counts,
        regression_gate_pass=counts=={'pass':343},stage_a_closed=False,
        exact_rgb=sum(row.get('exact_rgb',False) for row in summary['results']),
        measured_cpp_seconds=sum(row.get('cpp_ms',0) for row in summary['results'])/1000,
        thresholds=summary['thresholds'],logical_threads=16,suites=suite_counts,
        original_image_hash_provenance=Counter(row['image_hash_provenance'] for row in snapshot['cases']),
        command=['python','-X','utf8','comparison/tools/replay_parity.py','--reference',snapshot['reference'],
                 '--cpp-exe',str(binary.resolve()),'--output',str(args.replay.resolve()),'--checkpoint-every','16'],
        limitations=['Original scene hashes predate this run; original PNG hashes were first recorded by the preflight, not retrospectively verified.',
            'A passing 343-case regression is separate from active external cases, finite mode/state matrices and reference blockers.'],
        input_sha256=identities)
    keep(Path(__file__))
    args.output.parent.mkdir(parents=True,exist_ok=True)
    args.output.write_text(json.dumps(report,indent=2)+'\n')
    print(json.dumps({key:report[key] for key in ['measured_cases','counts','exact_rgb','measured_cpp_seconds','regression_gate_pass']}))


if __name__=='__main__':main()
