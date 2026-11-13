#!/usr/bin/env python3
"""Reject both mixed host/shader ABI directions before navigation dispatch."""
import argparse
import json
import os
from pathlib import Path
import shutil
import time

from check_scenes import Service, save

p = argparse.ArgumentParser(description=__doc__)
p.add_argument('--baseline', type=Path, required=True)
p.add_argument('--package', type=Path, required=True)
a = p.parse_args()
baseline, package = a.baseline.resolve(), a.package.resolve()
out = package / 'schedule-abi'
out.mkdir(exist_ok=False)
os.environ['MB3D_VIEWER_GENERAL_PREVIEW'] = '1'
os.environ.pop('MB3D_VIEWER_ORDERED_CONTROL', None)
row = next(r for r in json.loads((baseline / 'inventory.json').read_text())['ordinary'] if r['rank'] == 3)
# Use the current private envelope for both hosts; the older host ignores it.
q = json.loads((package / 'requests' / (row['id'] + '.json')).read_text())
q.update(mode='points', edge=64, phase=0)
results = []
for name, host, shader in [('old-host-new-shader', baseline, package), ('new-host-old-shader', package, baseline)]:
    folder = out / name
    folder.mkdir()
    app = folder / 'Mandelbulb Viewer.app'
    shutil.copytree(host / 'final/Mandelbulb Viewer.app', app)
    shader_path = Path('Contents/Resources/general_navigation.metallib')
    shutil.copyfile(shader / 'final/Mandelbulb Viewer.app' / shader_path, app / shader_path)
    service = Service(app / 'Contents/MacOS/MandelbulbViewer', folder, name)
    try:
        r, rgb = service.run(q, name, folder, time.monotonic() + 180)
        results.append(r)
        save(out / 'checks.json', results)
        assert r['status'] == 'failed' and not rgb and r['error'] == 'General navigation shader ABI mismatch', r
    finally:
        service.close()
print('PASS both mixed ABI directions reject without RGB')
