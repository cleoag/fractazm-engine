"""Finite branch/rotation checks for SierpinskiFull and helistairsIFS.

Exercises both fold orders, signed/zero scale, paired rotation axes, both
edges, both stair styles (including signed zero), radial selectors, Apply,
and step-count boundaries. Does not exhaust continuous floating-point inputs.
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
    args = ap.parse_args()
    bases = json.loads(args.manifest.read_bytes())['cases']
    args.output.mkdir(parents=True, exist_ok=False)
    cases = []
    for name in ('SierpinskiFull', 'helistairsIFS'):
        base = next(c for c in bases if c['id'] == 'challenge_' + name + '_early')
        source = ROOT / base['scene']
        if digest(source.read_bytes()) != base['sha256']:
            raise ValueError('Base scene changed')
        data = decode(source)
        slot = next(s for s in slots(data) if s['name'] == name)
        slot = prepare_external_options(data, slot)
        variants = []
        if name == 'SierpinskiFull':
            for axes, reverse, scale, edges in itertools.product(
                    itertools.combinations(range(6), 2), (-1, 0, 1),
                    (-2.5, 0., 2.), ((0., 0.), (.25, -.5), (.75, 1.25))):
                o = slot['options'][:]
                o[:4] = [scale, .7, -1.3, .2]
                o[4:10] = [0.] * 6
                o[4+axes[0]], o[4+axes[1]] = 37., -61.
                o[10:13] = [*edges, reverse]
                variants.append(o)
        else:
            for n, height, position, apply in itertools.product(
                    (-2147483648, -1, 0, 1, 2, 9, 12, 2147483647),
                    (-.35, -0., 0., .35), (-1., 0., 1.), (0, 1)):
                o = slot['options'][:]
                o[5], o[10], o[11], o[13] = apply, n, height, position
                o[0], o[6] = -.125, -.25
                variants.append(o)
            for axis, scale, height, position in itertools.product(range(3), (-2., 0., .5), (-.35, .35), (-1., 1.)):
                o = slot['options'][:]
                o[7+axis], o[1], o[11], o[13] = 37., scale, height, position
                o[2:5] = [.125, -.25, .375]
                variants.append(o)
        for index, options in enumerate(variants):
            scene = bytearray(data)
            offset = 848 + slot['slot'] * 188

            struct.pack_into('<16d', scene, offset+60, *options)
            case = f'fold_stairs_{name}_{index:04d}'
            path = args.output / (case + '.m3p')
            path.write_bytes(scene)
            cases.append(dict(id=case, target_formulas=[name], options=options,
                              scene=str(path.resolve().relative_to(ROOT)), sha256=digest(scene),
                              source=base['scene'], source_sha256=base['sha256']))
    write_json(args.output / 'manifest.json', dict(scope=__doc__, cases=cases))
    print(len(cases), 'scenes')


if __name__ == '__main__':
    main()
