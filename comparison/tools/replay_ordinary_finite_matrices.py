"""Replay the prepared finite ordinary witnesses on one frozen candidate.

Original invalid/unstable rows remain failures in their original reports. This
tool explicitly selects qualified references; it does not certify Stage A.
"""
import argparse
from collections import Counter
import hashlib
import json
from pathlib import Path
import shutil
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[2]
BASE = ROOT/'comparison/results/stage-a-20261018'
RUNS = ROOT/'comparison/runs/stage-a-20261018'


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def write(path, data):
    path.write_text(json.dumps(data, indent=2)+'\n', encoding='utf-8')


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--cpp-exe', type=Path, required=True)
    ap.add_argument('--output', type=Path, required=True)
    args = ap.parse_args()
    exe = args.cpp_exe.resolve()
    sha = digest(exe)
    args.output.mkdir(parents=True, exist_ok=False)
    excluded = {
        'effective_comb5': 'Original second formula is not dIFS; invalid mix now rejected. Valid mix6 is a separate matrix, not replacement PASS.',
        'effective_comb5_color1': 'Original second formula is not dIFS; invalid mix now rejected.',
        'effective_comb5_color2': 'Original second formula is not dIFS; invalid mix now rejected.',
        'effective_transmit_both2': 'Original batch completion returned prematurely. Retained old failure is not a valid oracle; corrected completion and transmission coupling evidence are separate.',
    }
    table = [
        ('additional31', 'additional-modes-candidate-v5', 'additional-modes-v1', 31),
        ('effective42', 'effective-modes-candidate-v8', 'effective-modes-v1', 42),
        ('mix6', 'de-mix-controls-candidate-v8', 'de-mix-controls-v2', 6),
        ('lighting30', 'lighting-modes-candidate-v8', 'lighting-modes-v1', 30),
        ('visible10', 'visible-lights-candidate-v13', 'visible-lights-v2', 10),
        ('maps18', 'render-maps-bmp-candidate-v15', 'render-maps-v1', 18),
        ('preprocess8', 'render-map-preprocess-candidate-v15', 'render-map-preprocess-v1', 8),
        ('heightmap4', 'formula-render-maps-candidate-v15', 'formula-render-maps-v2', 4),
    ]
    volume = set(json.loads((BASE/'lighting-modes-v1/volume-cases.json').read_bytes()))
    report = dict(scope='Prepared finite ordinary matrices; Monte Carlo/JIT excluded',
                  finished=False, stage_a_closed=False, cpp_sha256=sha,
                  tool_sha256=digest(Path(__file__)), matrices=[], excluded=excluded,
                  separately_replayed_volume_cases=sorted(volume),
                  note='Counts are matrix rows, not unique scenes or total semantic coverage.')
    plans = []
    for name, previous, folder, expected in table:
        old_path = RUNS/previous/'summary.json'
        old = json.loads(old_path.read_bytes())
        ref = Path(old['reference'])
        if digest(ref) != old['results'][0]['reference_summary_sha256']:
            raise ValueError('Original reference changed: '+str(ref))
        selected = [row['scene'] for row in old['results']
                    if row['scene'] not in excluded and
                    not (name=='lighting30' and row['scene'] in volume)]
        if len(selected) != expected or len(selected)!=len(set(selected)):
            raise ValueError('Finite matrix count changed: '+name)
        case_list = args.output/(name+'-cases.json')
        write(case_list, selected)
        plans.append((name, ref, BASE/folder/'manifest.json', case_list, expected,
                      dict(previous_report=str(old_path), previous_sha256=digest(old_path))))

    # Preserve real .25/.75 witnesses; the earlier integer 25/75 controls only
    # exercise subnormal-weight fallback. Adapt metadata, never original bytes.
    interp = BASE/'interpolation-float-v1'
    original = json.loads((interp/'summary.json').read_bytes())
    adapted = args.output/'interpolation-reference'
    (adapted/'pascal').mkdir(parents=True)
    refs = [row['reference'] for row in original['results']]
    if len(refs)!=2:
        raise ValueError('Interpolation finite count changed')
    for row in refs:
        for src, dst in [(interp/'pascal'/(row['scene']+'.png'), adapted/'pascal'/(row['scene']+'.png')),
                         (interp/row['pascal_log'], adapted/row['pascal_log'])]:
            shutil.copyfile(src, dst)
    adapted_report = dict(finished=True, pascal_sha256=original['pascal_sha256'],
        manifest_sha256=digest(interp/'manifest.json'), batches=[], results=refs,
        provenance=dict(original_summary=str(interp/'summary.json'),
                        original_sha256=digest(interp/'summary.json'),
                        note='Metadata-only adaptation; original PNG/log bytes preserved.'))
    write(adapted/'summary.json', adapted_report)
    plans.append(('interpolation2', adapted/'summary.json', interp/'manifest.json', None, 2, {}))

    write(args.output/'summary.json', report)
    for name, ref, manifest, cases, expected, previous in plans:
        if digest(exe)!=sha:
            raise ValueError('Candidate changed')
        command = [sys.executable, '-X', 'utf8', str(ROOT/'comparison/tools/replay_parity.py'),
                   '--reference', str(ref), '--manifest', str(manifest), '--cpp-exe', str(exe),
                   '--output', str(args.output/name), '--checkpoint-every', '8']
        if cases:
            command += ['--case-list', str(cases)]
        completed = subprocess.run(command, cwd=ROOT, capture_output=True, text=True, encoding='utf-8')
        (args.output/(name+'.log')).write_text(completed.stdout+completed.stderr, encoding='utf-8')
        current = json.loads((args.output/name/'summary.json').read_bytes())
        if not current['finished'] or len(current['results'])!=expected:
            raise ValueError('Incomplete replay: '+name)
        item = dict(name=name, command=command, returncode=completed.returncode,
                    reference_sha256=digest(ref), manifest_sha256=digest(manifest),
                    report=str(args.output/name/'summary.json'), expected=expected,
                    counts=dict(Counter(row['status'] for row in current['results'])),
                    exact_rgb=sum(row.get('exact_rgb',False) for row in current['results']),
                    **previous)
        report['matrices'].append(item)
        write(args.output/'summary.json', report)
        print(json.dumps(item), flush=True)
    report['finished']=True
    report['counts']=dict(sum((Counter(item['counts']) for item in report['matrices']), Counter()))
    write(args.output/'summary.json', report)
    return int(any(item['returncode'] for item in report['matrices']))


if __name__=='__main__':
    raise SystemExit(main())
