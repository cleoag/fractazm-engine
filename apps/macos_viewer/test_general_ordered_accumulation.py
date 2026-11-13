#!/usr/bin/env python3
"""Exact old/new full 640-pixel sparse accumulation with carried owner state.

Rank 66 is a bounded ordinary ordered scene. Compare every phase, then repeat
the candidate three times. Guard each entire accumulation at 180 seconds.
This is approximate navigation geometry, not an authored Full render.
"""
import argparse
import json
import os
from pathlib import Path
import time

from PIL import Image
from check_scenes import Service, save


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--baseline', type=Path, required=True)
    p.add_argument('--package', type=Path, required=True)
    a = p.parse_args()
    baseline, package = a.baseline.resolve(), a.package.resolve()
    out = package / 'ordered-accumulation'
    out.mkdir(exist_ok=False)
    os.environ['MB3D_VIEWER_GENERAL_PREVIEW'] = '1'
    os.environ.pop('MB3D_VIEWER_ORDERED_CONTROL', None)
    row = next(r for r in json.loads((baseline / 'inventory.json').read_text())['ordinary'] if r['rank'] == 66)
    q = json.loads((baseline / 'requests' / (row['id'] + '.json')).read_text())
    q.update(edge=640, phase=0, mode='points')
    expected, runs = [], []
    for name, root in [('old', baseline), ('new-0', package), ('new-1', package), ('new-2', package)]:
        folder = out / name
        folder.mkdir()
        service = Service(root / 'final/Mandelbulb Viewer.app/Contents/MacOS/MandelbulbViewer', folder, name)
        started = time.monotonic()
        covered, total, phase, work = 0, 1, 0, 0
        try:
            with (folder / 'phases.jsonl').open('w') as journal:
                while phase < total:
                    r, rgb = service.run(dict(q, phase=phase), f'phase-{phase}', folder, started + 180)
                    journal.write(json.dumps(r) + '\n')
                    journal.flush()
                    assert r['status'] == 'completed' and r['engine'] == 'general_navigation_ordered', r
                    total = r['phases']
                    assert r['covered'] == covered + r['samples']
                    covered = r['covered']
                    work += r['formula_work']
                    signature = tuple(r[k] for k in ('rgb_sha256', 'formula_work', 'samples', 'covered'))
                    if name == 'old':
                        expected.append(signature)
                    else:
                        assert signature == expected[phase], (name, phase, signature, expected[phase])
                    phase += 1
                    if phase % 128 == 0:
                        print(name, phase, '/', total, f'{time.monotonic()-started:.1f}s', flush=True)
            assert covered == r['width'] * r['height'] and len(expected) == total
            Image.frombytes('RGB', (r['width'], r['height']), rgb).save(folder / 'accumulated.png')
            runs.append(dict(artifact=name, phases=total, covered=covered, formula_work=work,
                             rgb_sha256=r['rgb_sha256'], wall_seconds=time.monotonic()-started, exact=True))
            save(out / 'checks.json', dict(rank=66, scene=row['scene'], threads=4,
                 qualification=False, scope='640-pixel approximate ordered accumulation', runs=runs))
        finally:
            service.close()
    print('PASS all 1024 phases old/new, three exact complete candidate repeats', flush=True)


if __name__ == '__main__':
    main()
