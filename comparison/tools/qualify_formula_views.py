"""Qualify rendered geometry candidates; C++ usefulness is not Pascal parity."""
import argparse
import json
from collections import Counter
from pathlib import Path
from PIL import Image
from check_formula_influence import geometry
from external_validation import ROOT, digest, write_json


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--manifest', type=Path, required=True)
    ap.add_argument('--influence', type=Path, required=True, help='Influence summary.json')
    ap.add_argument('--output', type=Path, required=True)
    args = ap.parse_args()
    manifest = json.loads(args.manifest.read_bytes())
    influence = json.loads(args.influence.read_bytes())
    if influence['manifest_sha256'] != digest(args.manifest.read_bytes()):
        raise ValueError('Manifest differs from measured influence')
    evidence = {r['case']: r for r in influence['results']}
    if len(evidence) != len(influence['results']):
        raise ValueError('Duplicate influence result')
    results = []
    for case in manifest['cases']:
        row = dict(case=case['id'], mode=case.get('mode'), status='error')
        try:
            record = evidence[case['id']]
            scene = ROOT/case['scene']
            if digest(scene.read_bytes()) != case['sha256'] or record['scene_sha256'] != case['sha256']:
                raise ValueError('Scene hash differs')
            if record.get('error'):
                raise ValueError(record['error'])
            stem = args.influence.parent/case['id']
            dump = stem.with_suffix('.silight')
            png = stem.with_suffix('.png')
            depth = geometry(dump)
            hits = [v for v in depth if v < 32768]
            with Image.open(png) as image:
                rgb = image.convert('RGB')
                if rgb.width*rgb.height != len(depth):
                    raise ValueError('Image and depth dimensions differ')
                pixels = rgb.tobytes()
                colors = len({pixels[i:i+3] for i in range(0, len(pixels), 3)})
            changed = []
            controls = []
            for formula in record['formulas']:
                if formula['control'] != 'omit-target':
                    raise ValueError('Requires an omission control, not a substitute formula')
                control = args.influence.parent/(case['id']+'_without_'+formula['name'])
                scene_control = control.with_suffix('.m3p')
                if digest(scene_control.read_bytes()) != formula['control_sha256']:
                    raise ValueError('Control scene changed')
                control_dump = control.with_suffix('.silight')
                other = geometry(control_dump)
                if len(other) != len(depth):
                    raise ValueError('Control dimensions differ')
                count = sum(x != y for x, y in zip(depth, other))
                if count != formula['changed_depth_pixels']:
                    raise ValueError('Control depth result changed')
                changed.append(count)
                controls.append(dict(name=formula['name'], scene_sha256=formula['control_sha256'],
                                     depth_sha256=digest(control_dump.read_bytes())))
            useful = (0 < len(hits) < len(depth) and len(set(hits)) > 8 and
                      colors > 8 and bool(changed) and all(changed))
            row.update(status='useful' if useful else 'not-qualified',
                geometry_pixels=len(hits), pixels=len(depth), hit_depth_values=len(set(hits)),
                rgb_colors=colors, changed_depth_pixels=changed, controls=controls,
                scene_sha256=case['sha256'], depth_sha256=digest(dump.read_bytes()),
                png_sha256=digest(png.read_bytes()))
        except (OSError, ValueError, KeyError) as error:
            row['error'] = str(error)
        results.append(row)
    summary = dict(Counter(r['status'] for r in results))
    useful_modes = sorted({r['mode'] for r in results if r['status'] == 'useful' and r['mode'] is not None})
    write_json(args.output, dict(scope=__doc__, manifest_sha256=digest(args.manifest.read_bytes()),
        influence_sha256=digest(args.influence.read_bytes()), cpp_sha256=influence['cpp_sha256'],
        tool_sha256=digest(Path(__file__).read_bytes()), summary=summary,
        useful_modes=useful_modes, results=results, pascal_parity=False))
    print(summary, 'useful modes:', useful_modes)


if __name__ == '__main__':
    main()
