"""Finite Julia-state checks: every documented shape, mode and transition profile.

Crosses nine shape selectors (seven documented and two default fallbacks),
four inside-mode selectors, five speeds, three signed/zero radii and two
center/recenter profiles. Matrix checks cover all permutations and signed
diagonal/shear profiles. Does not exhaust continuous inputs or whole orbits.
"""
import argparse
import itertools
import json
import struct
from pathlib import Path
from external_validation import ROOT, decode, digest, slots, write_json, prepare_external_options


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--manifest', type=Path, required=True)
    ap.add_argument('--output', type=Path, required=True)
    ap.add_argument('--formula', action='append', choices=('_LinCombineCxyz', '_PartlyJuliaR', '_PartlyJulia'))
    args = ap.parse_args()
    bases = json.loads(args.manifest.read_bytes())['cases']
    args.output.mkdir(parents=True, exist_ok=False)
    cases = []
    for name in args.formula or ('_LinCombineCxyz', '_PartlyJuliaR'):
        base = next(c for c in bases if c['id'] == 'challenge_' + name + '_early')
        source = ROOT / base['scene']
        if digest(source.read_bytes()) != base['sha256']:
            raise ValueError('Base scene changed')
        data = decode(source)
        slot = next(s for s in slots(data) if s['name'] == name)
        slot = prepare_external_options(data, slot)
        variants = []
        if name == '_PartlyJuliaR':
            for shape, mode, speed, radius, profile in itertools.product(
                    (0, 1, 2, -2, 3, 4, -4, -1, 5), (-1, 0, 1, 2),
                    (-.25, 0., .5, 1., 1.25), (-1., 0., 1.), (0, 1)):
                o = slot['options'][:]
                o[:11] = [*( (.125, -.25, .5) if profile else (0., 0., 0.) ),
                          radius, speed, .7, -1.3, .2, mode, 1. if profile else 0., shape]
                variants.append(o)
        elif name == '_PartlyJulia':
            # Match the checker's saved P offsets, independently of current xyz.
            # Every axis crosses upper/lower at all three grid positions; include
            # normal, reversed and equal bounds and both zero/nonzero offsets.
            bounds = list(itertools.product((-2., 0., 2.), repeat=2))
            for axes, add in itertools.product(itertools.product(bounds, repeat=3), (0., .375)):
                o = slot['options'][:]
                for axis, ((upper, lower), origin) in enumerate(zip(axes, (.125, -.25, .5))):
                    o[3*axis:3*axis+3] = [upper+origin, lower+origin, add]
                variants.append(o)
        else:
            for permutation, scale, shear in itertools.product(
                    itertools.permutations(range(3)), (-2., 0., .5, 1.), (0., .375)):
                o = slot['options'][:]
                o[:9] = [0.] * 9
                for row, col in enumerate(permutation):
                    o[3*row+col] = scale
                    o[3*row+(col+1)%3] = shear
                variants.append(o)
        for index, options in enumerate(variants):
            scene = bytearray(data)
            offset = 848 + slot['slot'] * 188

            struct.pack_into('<16d', scene, offset+60, *options)
            case = f'julia_state_{name}_{index:04d}'
            path = args.output / (case + '.m3p')
            path.write_bytes(scene)
            cases.append(dict(id=case, target_formulas=[name], options=options,
                              scene=str(path.resolve().relative_to(ROOT)), sha256=digest(scene),
                              source=base['scene'], source_sha256=base['sha256']))
    write_json(args.output / 'manifest.json', dict(scope=__doc__, cases=cases))
    print(len(cases), 'scenes')


if __name__ == '__main__':
    main()
