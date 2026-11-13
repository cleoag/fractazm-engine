"""Finite tiling and Ruckerbulb branch/parameter crossings.

TilingIFS: seven signed int32 axis flags, 16 signed/zero/tiny size pairs,
three asymmetric offset profiles (336). Each brick transform: four rotations,
16 size pairs and three row offsets (192). Ruckerbulb: five powers per axis,
three Z multipliers and five signed mode values per selector (1875).
Shared parameter expansion and finite points do not certify all real inputs.
"""
import argparse
import itertools
import json
import struct
from pathlib import Path
from external_validation import ROOT, decode, digest, slots, write_json, prepare_external_options


def variants(name):
    sizes = (-2., 0., 1e-12, 2.)
    if name == 'TilingIFS':
        for flag, sx, sy, shift in itertools.product(
                (-2147483648, -3, -1, 0, 1, 2, 2147483647), sizes, sizes, (-.25, 0., .5)):
            yield [shift, -2*shift, 3*shift, sy, sx, 4*shift, -5*shift, flag]
    elif name.startswith('tilingbrick'):
        for rotation, sx, sy, row in itertools.product(
                ((0., 0., 0.), (37., -61., 23.), (-90., 0., 90.), (180., -180., 45.)),
                sizes, sizes, (-2., 0., .75)):
            yield [.125, -.25, .5, *rotation, sx, sy, row]
    else:
        for power, zpower, zmul, mode, atanmode in itertools.product(
                (-2., 0., 1., 2., 3.5), (-2., 0., 1., 2., 3.5), (-1., 0., 1.),
                (-2147483648, -1, 0, 1, 2147483647), (-2147483648, -1, 0, 1, 2147483647)):
            yield [power, zpower, zmul, mode, atanmode]


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--manifest', type=Path, required=True)
    ap.add_argument('--output', type=Path, required=True)
    a = ap.parse_args()
    manifest = json.loads(a.manifest.read_bytes())
    a.output.mkdir(parents=True, exist_ok=False)
    cases = []
    for formula in ('TilingIFS', 'tilingbrickIFS', 'tilingbrick2IFS', 'Ruckerbulb'):
        base = next(c for c in manifest['cases'] if c['id'] == f'challenge_{formula}_early')
        source = ROOT / base['scene']
        if digest(source.read_bytes()) != base['sha256']:
            raise ValueError('Changed source')
        data = decode(source)
        slot = next(s for s in slots(data) if s['name'] == formula)
        slot = prepare_external_options(data, slot)
        for index, values in enumerate(variants(formula)):
            options = slot['options'][:]
            options[:len(values)] = values
            scene = bytearray(data)
            struct.pack_into('<16d', scene, 848 + slot['slot'] * 188 + 60, *options)
            name = f'tiling_rucker_{formula}_{index:04d}'
            path = a.output / (name + '.m3p')
            path.write_bytes(scene)
            cases.append(dict(id=name, target_formulas=[formula], options=options,
                              scene=str(path.resolve().relative_to(ROOT)), sha256=digest(scene),
                              source=base['scene'], source_sha256=base['sha256']))
    write_json(a.output / 'manifest.json', dict(scope=__doc__, cases=cases))
    print(len(cases), 'scenes')


if __name__ == '__main__':
    main()
