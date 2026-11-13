"""Enumerate BT1Pinehedron / BT1Pinegon loop-count bytes and integer boundaries.

Finite one-option sweeps, not a Cartesian product or floating-point proof.
Scenes use nonzero rotation and unequal C scales to expose branch selection.
"""
import argparse
import json
import struct
from pathlib import Path
from external_validation import ROOT, decode, digest, slots, write_json, prepare_external_options


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--formula', choices=['BT1Pinehedron','BT1Pinegon'], default='BT1Pinehedron')
    ap.add_argument('--manifest', type=Path, required=True)
    ap.add_argument('--output', type=Path, required=True)
    args = ap.parse_args()
    base = next(c for c in json.loads(args.manifest.read_bytes())['cases']
                if c['id'] == f'challenge_{args.formula}_early')
    source = ROOT/base['scene']
    if digest(source.read_bytes()) != base['sha256']: raise ValueError('Base scene changed')
    data = decode(source)
    slot = next(s for s in slots(data) if s['name'] == args.formula)
    slot = prepare_external_options(data, slot)
    args.output.mkdir(parents=True, exist_ok=False)
    variants = [(8, n) for n in range(256)]
    boundaries = [-2147483648, -257, -129, -128, -1, 0, 1, 127, 128, 255, 256, 257, 2147483647]
    variants += [(field, n) for field in [6,7,8,9,10] for n in boundaries
                 if field != 8 or n not in range(256)]
    cases = []
    for field, value in variants:
        options = slot['options'][:]
        # Keep 256 T1 repeats bounded while preserving cancellation and folds.
        options[:11] = [1.01,.125,.7,-.3,.4,.371,62,0,1,2,62]
        options[field] = value
        scene = bytearray(data)
        struct.pack_into('<16d', scene, 848+slot['slot']*188+60, *options)
        name = f'{args.formula.lower()}_p{field}_{value}'
        path = args.output/(name+'.m3p');path.write_bytes(scene)
        cases.append(dict(id=name, target_formulas=[args.formula],
                          scene=str(path.resolve().relative_to(ROOT)), sha256=digest(scene),
                          source=base['scene'], source_sha256=base['sha256'], field=field, value=value))
    write_json(args.output/'manifest.json',dict(scope=__doc__,cases=cases))
    print(len(cases), 'scenes')


if __name__ == '__main__': main()
