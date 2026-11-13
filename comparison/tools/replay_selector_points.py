"""Replay prepared selector witnesses and verify the actual native INTEGER field.

The original CODE uses the retained C++ parameter buffer, so independent loader
qualification remains separate. Rejected original-undefined inputs are retained
as unverified, never converted to numerical PASS. No Monte Carlo or JIT.
"""
import argparse
from collections import Counter, defaultdict
import hashlib
import json
import os
from pathlib import Path
import struct
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[2]
RUN = ROOT/'comparison/runs/stage-a-20261018'
FOLDERS = ['selectors-27-v1', 'selectors-remaining-v1', 'original-reviewed-active-v2',
           'original-direct-four-v1', 'original-final-six-v1']


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def write(path, value):
    path.write_text(json.dumps(value, indent=2)+'\n', encoding='utf-8')


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--cpp', type=Path, required=True)
    ap.add_argument('--oracle', type=Path, required=True)
    ap.add_argument('--output', type=Path, required=True)
    a = ap.parse_args()
    a.output.mkdir(parents=True, exist_ok=False)
    a.cpp, a.oracle = a.cpp.resolve(), a.oracle.resolve()
    identities = {str(p):sha(p) for p in (a.cpp, a.oracle, Path(__file__).resolve())}
    groups, by_id = defaultdict(list), {}
    for folder in FOLDERS:
        path = RUN/folder/'scenes/manifest.json'
        identities[str(path)] = sha(path)
        for case in json.loads(path.read_bytes())['cases']:
            if case['id'] in by_id:
                raise ValueError('Duplicate selector ID: '+case['id'])
            if sha(ROOT/case['scene']) != case['sha256']:
                raise ValueError('Changed selector scene: '+case['id'])
            by_id[case['id']] = case
            name = case['target_formulas'][0]
            if name.startswith('JIT'):
                raise ValueError('Deferred formula in active manifest')
            group = ('conditional' if name.startswith('_Cond') else
                     'sphere' if name in ('_MapTranslate','ABoxMapSFold','SphereHeightMap','SphereHMVoid') else
                     'height' if name in ('HeightMapIFS','HeightMap2IFS','CylinderHeightMap','CylinderHMVoid','TorusHMVoid') else 'generic')
            groups[group].append(case)
    result = dict(scope=__doc__, finished=False, identities=identities, selected_cases=len(by_id), jobs=[], results=[])
    write(a.output/'summary.json', result)
    for group, cases in sorted(groups.items()):
        manifest = a.output/(group+'.json')
        write(manifest, dict(cases=cases))
        out = a.output/group
        command = [sys.executable, '-X', 'utf8', str(ROOT/'comparison/tools/check_formula_scene_points.py'),
                   '--cpp', str(a.cpp), '--oracle', str(a.oracle), '--manifest', str(manifest),
                   '--output', str(out), '--all-cases', '--checkpoint-every', '16']
        if group=='conditional':
            command += ['--conditional-counters']
        env = {k:v for k,v in os.environ.items() if not k.startswith('MB3D_POINT_')}
        if group=='sphere': env['MB3D_POINT_SPHEREMAP_CALLBACK']='1'
        if group=='height': env['MB3D_POINT_HEIGHTMAP_CALLBACK']='1'
        with (a.output/(group+'.log')).open('w', encoding='utf-8') as log:
            completed = subprocess.run(command, cwd=ROOT, env=env, stdout=log, stderr=subprocess.STDOUT)
        batch = json.loads((out/'summary.json').read_bytes())
        if completed.returncode or not batch['finished'] or len(batch['results'])!=len(cases):
            raise ValueError('Incomplete point batch: '+group)
        for row in batch['results']:
            case = by_id[row['case']]
            blob = out/(row['case']+'.params')
            if blob.exists():
                if sha(blob)!=row['parameter_sha256']:
                    raise ValueError('Changed parameter buffer')
                selector = case['selector']
                offset = selector['pvar_offset']
                if isinstance(offset,str): offset=int(offset,0)
                value = struct.unpack_from('<i',blob.read_bytes(),512+offset)[0]
                row['native_selector_check'] = dict(expected=selector['int32_value'], loaded=value,
                    offset=offset, stored_value_index=selector['stored_value_index'])
                if value!=selector['int32_value']:
                    raise ValueError('Ineffective selector: '+row['case'])
            elif row['status']=='pass':
                raise ValueError('PASS without parameter identity')
            row['point_report'] = str(out/'summary.json')
            result['results'].append(row)
        result['jobs'].append(dict(group=group, command=command, summary_sha256=sha(out/'summary.json')))
        result['counts'] = dict(Counter(r['status'] for r in result['results']))
        write(a.output/'summary.json',result)
        print(group, result['counts'], flush=True)
    if any(sha(Path(path))!=expected for path,expected in identities.items()):
        raise ValueError('Frozen input identity changed')
    result['native_selectors_verified'] = sum('native_selector_check' in r for r in result['results'])
    result['finished']=True
    write(a.output/'summary.json',result)
    print(result['counts'])
    return int(any(r['status'] in ('fail','error') for r in result['results']))


if __name__=='__main__':
    raise SystemExit(main())
