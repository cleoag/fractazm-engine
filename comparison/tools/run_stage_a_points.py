"""Replay the 5,506 active point cases on frozen binaries, retaining exceptions.

Uses existing checker tolerances and dedicated callback/counter profiles. This
does not replace independent loader, full-orbit, real-map, or image checks.
"""
import argparse
from collections import Counter
from concurrent.futures import ThreadPoolExecutor
import hashlib
import json
import os
from pathlib import Path
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[2]


def sha(p): return hashlib.sha256(p.read_bytes()).hexdigest()


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--cpp', type=Path, required=True)
    ap.add_argument('--oracle', type=Path, required=True)
    ap.add_argument('--ledger', type=Path, required=True)
    ap.add_argument('--output', type=Path, required=True)
    ap.add_argument('--workers', type=int, default=4)
    a = ap.parse_args()
    if not 1 <= a.workers <= 8: ap.error('workers must be 1..8')
    a.output = a.output.resolve()
    a.cpp = a.cpp.resolve()
    a.oracle = a.oracle.resolve()
    a.output.mkdir(parents=True, exist_ok=False)
    data = json.loads(a.ledger.read_bytes())
    assert data['totals']['formulas'] == 457 and len(data['external_cases']) == 5506
    sphere = {'_MapTranslate','ABoxMapSFold','SphereHeightMap','SphereHMVoid'}
    height = {'HeightMapIFS','HeightMap2IFS','CylinderHeightMap','CylinderHMVoid','TorusHMVoid'}
    conditional = {'_CondItcountR','_CondItcountSwitch','_CondItcountSwX2','_CondLyapunov','_CondLyapunovEx'}
    grouped = {k:[] for k in ['generic','sphere','height','conditional']}
    for row in data['external_cases']:
        names = row['target_formulas']
        assert len(names) == 1 and not names[0].startswith('JIT')
        name = names[0]
        group = 'sphere' if name in sphere else 'height' if name in height else 'conditional' if name in conditional else 'generic'
        scene = ROOT / row['scene']
        if sha(scene) != row['scene_sha256']: raise ValueError('Scene identity changed: '+str(scene))
        grouped[group].append(dict(id=row['case'],scene=row['scene'],sha256=row['scene_sha256'],target_formulas=names))
    identities = {str(p):sha(p) for p in [a.cpp,a.oracle,a.ledger.resolve(),Path(__file__)]}
    jobs = []
    for group,cases in grouped.items():
        for offset in range(0,len(cases),256):
            job = f'{group}-{offset//256:02}'
            manifest = a.output/(job+'.json')
            manifest.write_text(json.dumps(dict(cases=cases[offset:offset+256]),indent=2))
            cmd = [sys.executable,'-X','utf8',str(ROOT/'comparison/tools/check_formula_scene_points.py'),
                   '--cpp',str(a.cpp),'--oracle',str(a.oracle),'--manifest',str(manifest),
                   '--output',str(a.output/job),'--all-cases','--checkpoint-every','16']
            if group == 'conditional': cmd += ['--conditional-counters']
            jobs.append(dict(name=job,group=group,command=cmd,manifest_sha256=sha(manifest)))
    (a.output/'plan.json').write_text(json.dumps(dict(scope=__doc__,identities=identities,jobs=jobs),indent=2))
    def run(job):
        if sha(a.cpp)!=identities[str(a.cpp)] or sha(a.oracle)!=identities[str(a.oracle)]:
            raise ValueError('Frozen binary identity changed')
        env = {k:v for k,v in os.environ.items() if not k.startswith('MB3D_POINT_')}
        if job['group'] == 'sphere': env['MB3D_POINT_SPHEREMAP_CALLBACK']='1'
        if job['group'] == 'height': env['MB3D_POINT_HEIGHTMAP_CALLBACK']='1'
        with (a.output/(job['name']+'.log')).open('w') as log:
            proc = subprocess.run(job['command'],cwd=ROOT,env=env,stdout=log,stderr=subprocess.STDOUT)
        if proc.returncode: raise RuntimeError(job['name']+' process failed')
        report = json.loads((a.output/job['name']/'summary.json').read_bytes())
        if not report.get('finished'): raise RuntimeError(job['name']+' incomplete')
        print(job['name'],report['counts'],flush=True)
        return report
    with ThreadPoolExecutor(max_workers=a.workers) as pool: reports = list(pool.map(run,jobs))
    rows = [row for report in reports for row in report['results']]
    if len(rows)!=5506 or len({r['case'] for r in rows})!=5506: raise ValueError('Incomplete or duplicate case coverage')
    if any(sha(Path(p))!=s for p,s in identities.items()): raise ValueError('Input identity changed during replay')
    counts = dict(Counter(r['status'] for r in rows))
    summary = dict(scope=__doc__,finished=True,counts=counts,identities=identities,
                   selected_cases=5506,results=rows,
                   limitations='Manowar incoming-register profile remains separate. Explicit rejections and unavailable references are not numerical PASS.')
    (a.output/'summary.json').write_text(json.dumps(summary,indent=2))
    print(counts)
    return int(any(r['status'] in ('fail','error') for r in rows))


if __name__ == '__main__': raise SystemExit(main())
