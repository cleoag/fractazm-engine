"""Visible-geometry candidates: each valid shape, both slot orders, 1/2/4 repeats.

Combines ABoxVSShapes with Integer Power 2. Use check_formula_influence.py to
measure geometry and omission-control depth changes before accepting a scene
as useful. This generator alone does not establish usefulness or parity.
"""
import argparse
import json
import struct
from pathlib import Path
from external_validation import ROOT, decode, digest, write_json, slots, prepare_external_options


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--manifest', type=Path, required=True)
    ap.add_argument('--output', type=Path, required=True)
    args = ap.parse_args()
    original = {c['id']: c for c in json.loads(args.manifest.read_bytes())['cases']}
    args.output.mkdir(parents=True, exist_ok=False)
    cases = []
    for mode in range(1, 18):
        base = original[f'vsshapes_{mode}_mod_2_3']
        source = ROOT/base['scene']
        if digest(source.read_bytes()) != base['sha256']:
            raise ValueError('Base scene changed')
        for order in ['first', 'last']:
            for repeat in [1, 2, 4]:
                scene = decode(source)
                prepare_external_options(scene, next(s for s in slots(scene) if s['name'] == 'ABoxVSShapes'))
                target = bytearray(scene[848:1036])
                struct.pack_into('<i', target, 0, repeat)
                power = bytearray(188)
                struct.pack_into('<iii', power, 0, 1, 0, 2)
                power[12:44] = b'Integer Power'.ljust(32, b'\0')
                struct.pack_into('<2d', power, 60, 2., -1.)
                scene[848:1224] = target+power if order == 'first' else power+target
                case_id = f'vsshapes_hybrid_{mode}_{order}_{repeat}'
                path = args.output/(case_id+'.m3p')
                path.write_bytes(scene)
                cases.append(dict(id=case_id, target_formulas=['ABoxVSShapes'],
                    scene=str(path.resolve().relative_to(ROOT)), sha256=digest(scene),
                    source=base['scene'], source_sha256=base['sha256'],
                    mode=mode, order=order, repeat=repeat))
    write_json(args.output/'manifest.json', dict(scope=__doc__, cases=cases))
    print(len(cases), 'hybrid candidates')


if __name__ == '__main__':
    main()
