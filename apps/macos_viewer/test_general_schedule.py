#!/usr/bin/env python3
"""Balanced old/new ordered-preview comparison on four bounded ordinary scenes.

Both frozen executables receive the same frozen inputs, four logical workers
and authored Points settings. Warm-up is separate; each measured sample uses
a fresh service. A/B/B/A controls temperature/order, not UI presentation latency.
No Full, MC/JIT or production qualification is run.
"""
import argparse
import hashlib
import json
import math
import os
from pathlib import Path
import statistics
import time

from check_scenes import Service, save


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--baseline', type=Path, required=True)
    p.add_argument('--package', type=Path, required=True)
    a = p.parse_args()
    baseline, package = a.baseline.resolve(), a.package.resolve()
    out = package / 'schedule-comparison'
    out.mkdir(exist_ok=False)
    os.environ['MB3D_VIEWER_GENERAL_PREVIEW'] = '1'
    os.environ.pop('MB3D_VIEWER_ORDERED_CONTROL', None)
    executables = {name: root / 'final/Mandelbulb Viewer.app/Contents/MacOS/MandelbulbViewer'
                   for name, root in [('old', baseline), ('new', package)]}
    report = dict(qualification=False, threads=4, mode='points', order=['old', 'new', 'new', 'old'],
                  threshold=dict(geometric_mean_speedup=1.25, maximum_scene_regression=0.05),
                  executable_sha256={k: hashlib.sha256(v.read_bytes()).hexdigest()
                                     for k, v in executables.items()}, scenes=[])
    for row in json.loads((baseline / 'inventory.json').read_text())['ordinary']:
        if row['rank'] not in (10, 32, 56, 70):
            continue
        q = json.loads((baseline / 'requests' / (row['id'] + '.json')).read_text())
        q.update(mode='points', phase=0)
        case = dict(rank=row['rank'], scene=row['scene'], runs=[])
        report['scenes'].append(case)
        expected = {}
        for trial, lane in enumerate(report['order']):
            folder = out / f"{row['rank']:02}-{trial}-{lane}"
            folder.mkdir()
            service = Service(executables[lane], folder, lane)
            try:
                for label, edge in [('compile-preflight', 64), ('warm-up', 640), ('measured', 640)]:
                    dest = folder / label
                    dest.mkdir()
                    r, rgb = service.run(dict(q, edge=edge), label, dest, time.monotonic() + 180)
                    case['runs'].append(dict(trial=trial, artifact=lane, **r))
                    save(out / 'checks.json', report)
                    assert r['status'] == 'completed' and r['engine'] == 'general_navigation_ordered', r
                    signature = tuple(r[k] for k in ('rgb_sha256', 'formula_work', 'samples', 'covered'))
                    if edge in expected:
                        assert signature == expected[edge], (row['rank'], lane, label, signature, expected[edge])
                    else:
                        expected[edge] = signature
                    if label == 'measured':
                        assert r['archive_misses'] == 0, r
                    print(row['rank'], lane, label, f"{r['wall_seconds']:.4f}s", flush=True)
            finally:
                service.close()
        timings = {lane: statistics.median(r['wall_seconds'] for r in case['runs']
                   if r['artifact'] == lane and r['label'] == 'measured') for lane in executables}
        case.update(median_seconds=timings, speedup=timings['old'] / timings['new'], exact=True)
        save(out / 'checks.json', report)
    report['geometric_mean_speedup'] = math.exp(statistics.mean(math.log(r['speedup']) for r in report['scenes']))
    report['speed_gate'] = (report['geometric_mean_speedup'] >= 1.25 and
                           all(r['median_seconds']['new'] <= r['median_seconds']['old'] * 1.05
                               for r in report['scenes']))
    save(out / 'checks.json', report)
    assert report['speed_gate'], report
    print('PASS exact old/new and balanced warm timing gate', flush=True)


if __name__ == '__main__':
    main()
