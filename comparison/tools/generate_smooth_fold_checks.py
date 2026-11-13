"""Finite sharpness sweeps for both smooth-fold formulas.

Includes zero/one, odd/even, power-of-two boundaries and signed int32 limits.
This is not a Cartesian product of all options or exhaustive orbit coverage.
"""
import argparse
import json
import struct
from pathlib import Path
from external_validation import ROOT, decode, digest, slots, write_json, prepare_external_options


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--manifest', type=Path, required=True)
    ap.add_argument('--output', type=Path, required=True)
    args = ap.parse_args()
    original = json.loads(args.manifest.read_bytes())['cases']
    args.output.mkdir(parents=True, exist_ok=False)
    values = [-2147483648, -1, *range(17), 31, 32, 63, 64, 127, 128, 2147483647]
    cases = []
    for name in ['ABoxSmoothFold', 'ASurfSmoothFold']:
        base = next(c for c in original if c['id'] == f'challenge_{name}_early')
        source = ROOT/base['scene']
        if digest(source.read_bytes()) != base['sha256']:
            raise ValueError('Base scene changed')
        data = decode(source)
        slot = next(s for s in slots(data) if s['name'] == name)
        slot = prepare_external_options(data, slot)
        for field in [4, 6]:
            for value in values:
                options = slot['options'][:]
                options[field] = value
                scene = bytearray(data)
                struct.pack_into('<16d', scene, 848+slot['slot']*188+60, *options)
                case = f'smooth_{name}_p{field}_{value}'
                path = args.output/(case+'.m3p')
                path.write_bytes(scene)
                cases.append(dict(id=case, target_formulas=[name],
                    scene=str(path.resolve().relative_to(ROOT)), sha256=digest(scene),
                    source=base['scene'], source_sha256=base['sha256'], field=field, value=value))
    write_json(args.output/'manifest.json', dict(scope=__doc__, cases=cases))
    print(len(cases), 'scenes')


if __name__ == '__main__':
    main()
