#!/usr/bin/env python3
"""Reject stale or mismatched accumulation without RGB, then recover exactly."""
import argparse
import json
import os
import time
from pathlib import Path

from check_scenes import Service, save

p = argparse.ArgumentParser(description=__doc__)
p.add_argument('--package', type=Path, required=True)
a = p.parse_args()
package = a.package.resolve()
out = package / 'general-protocol'
out.mkdir(exist_ok=False)
os.environ['MB3D_VIEWER_GENERAL_PREVIEW'] = '1'
os.environ.pop('MB3D_VIEWER_ORDERED_CONTROL', None)
service = Service(package / 'final/Mandelbulb Viewer.app/Contents/MacOS/MandelbulbViewer', out, 'protocol')
row = next(r for r in json.loads((package / 'inventory.json').read_text())['ordinary'] if r['rank'] == 3)
request = json.loads((package / 'requests' / (row['id'] + '.json')).read_text())
request.update(edge=64, phase=0)
results = []

def run(label, q, expected='completed'):
    folder = out / label
    folder.mkdir()
    result, rgb = service.run(q, label, folder, time.monotonic() + 180)
    results.append(result)
    save(out / 'checks.json', results)
    assert result['status'] == expected, result
    if expected == 'failed':
        assert not rgb and 'snapshot/phase mismatch' in result['error'], result
    return result

try:
    first = run('initial', request)
    pid = first['pid']
    run('skipped-phase', dict(request, phase=2), 'failed')
    recovered = run('after-skipped-phase', request)
    assert recovered['rgb_sha256'] == first['rgb_sha256']
    moved = json.loads((package / 'moved-requests' / (row['id'] + '-orbit.json')).read_text())
    moved.update(edge=64, phase=1)
    run('changed-camera', moved, 'failed')
    run('after-camera-change', request)
    run('changed-scene-revision',dict(request,phase=1,scene_revision=request['scene_revision']+1),'failed')
    run('after-revision-change',request)
    run('changed-resource-snapshot', dict(request, phase=1, root=request['root'] + '/.'), 'failed')
    run('fast-profile', dict(request, mode='fastgeometry'))
    run('changed-quality-profile', dict(request, phase=1), 'failed')
    last = run('final-recovery', request)
    assert last['rgb_sha256'] == first['rgb_sha256'] and last['formula_work'] == first['formula_work']
    assert all(r['pid'] == pid for r in results)
    print('5 mismatched snapshots/phases rejected without RGB; exact recovery; one persistent process')
finally:
    service.close()
