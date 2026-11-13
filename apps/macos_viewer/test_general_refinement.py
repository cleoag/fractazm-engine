#!/usr/bin/env python3
"""Complete general 640-pixel accumulation -> authored Full on two bounded scenes."""
import argparse
import json
import os
import time
from pathlib import Path
from PIL import Image
from check_scenes import Service, save

p = argparse.ArgumentParser(description=__doc__)
p.add_argument('--package', type=Path, required=True)
a = p.parse_args()
package = a.package.resolve()
out = package / 'general-refinement'
out.mkdir(exist_ok=False)
os.environ['MB3D_VIEWER_GENERAL_PREVIEW'] = '1'
os.environ.pop('MB3D_VIEWER_ORDERED_CONTROL', None)
exe = package / 'final/Mandelbulb Viewer.app/Contents/MacOS/MandelbulbViewer'
preview, full = (Service(exe, out, name) for name in ('preview', 'full'))
results = []
try:
    for row in json.loads((package / 'inventory.json').read_text())['ordinary']:
        if row['rank'] not in (3, 37):
            continue
        folder = out / row['id']
        folder.mkdir()
        request = json.loads((package / 'requests' / (row['id'] + '.json')).read_text())
        request.update(edge=640, phase=0, mode='points')
        expected = None
        for repeat in range(3):
            deadline = time.monotonic() + 180
            phase, total, covered = 0, 1, 0
            while phase < total:
                label = f'accum-{repeat}-{phase}'
                dest = folder / label
                dest.mkdir()
                r, rgb = preview.run(dict(request, phase=phase), label, dest, deadline)
                assert r['status'] == 'completed' and r['engine'].startswith('general_navigation'), r
                total = r['phases']
                assert r['covered'] == covered + r['samples']
                covered = r['covered']
                results.append(dict(rank=row['rank'], **r))
                phase += 1
            assert covered == r['width'] * r['height']
            if repeat == 0:
                Image.frombytes('RGB', (r['width'], r['height']), rgb).save(folder / 'accumulated.png')
            label = f'full-{repeat}'
            dest = folder / label
            dest.mkdir()
            r, rgb = full.run(dict(request, phase=0, mode='full'), label, dest, time.monotonic() + 180)
            assert r['status'] == 'completed', r
            if expected is None:
                expected = rgb
                Image.frombytes('RGB', (r['width'], r['height']), rgb).save(folder / 'full.png')
            assert rgb == expected
            results.append(dict(rank=row['rank'], **r))
            save(out / 'checks.json', results)
            print(row['rank'], 'general accumulation -> Full', repeat + 1, 'PASS', flush=True)
finally:
    preview.close()
    full.close()
