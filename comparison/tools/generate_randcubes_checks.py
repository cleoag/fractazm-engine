"""All 7 x 7 shape selectors and invalid-selector fallbacks across six profiles.

Profiles exercise translation/flight, apply off, negative scale, zero variation,
and variation outside [0,1]. This is finite branch coverage, not all real inputs.
The point runner independently executes the shipped x86 bytecode on these scenes.
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
    manifest = json.loads(args.manifest.read_text())
    base = next(c for c in manifest['cases'] if c['id'] == 'challenge_RandCubesIFS_early')
    source = ROOT / base['scene']
    if digest(source.read_bytes()) != base['sha256']:
        raise ValueError('Base scene changed')
    data = decode(source)
    slot = next(s for s in slots(data) if s['name'] == 'RandCubesIFS')
    slot = prepare_external_options(data, slot)
    offset = 848 + slot['slot'] * 188 + 60
    profiles = {
        'default': {},
        'translated': {2: -.375, 3: .625, 4: -.125, 7: 1.25, 12: .75},
        'apply_off': {1: 1.5, 6: 0, 12: -.5},
        'negative_scale': {1: -2, 6: 0, 7: -.75, 8: -127, 9: 2147483647, 12: .5},
        'zero_variation': {0: 0, 5: 0, 12: 1},
        'outside_variation': {0: -1.25, 5: 2.5, 12: -1},
    }
    selectors = list(itertools.product(range(7), repeat=2))
    selectors += [(-1, -1), (7, 7), (2147483647, -2147483648), (-1, 6), (5, 7)]
    args.output.mkdir(parents=True, exist_ok=False)
    cases = []
    for profile, changes in profiles.items():
        for first, second in selectors:
            scene = bytearray(data)
            options = slot['options'][:]
            for index, value in changes.items():
                options[index] = value
            options[10:12] = first, second
            struct.pack_into('<16d', scene, offset, *options)
            name = f'randcubes_{profile}_a{first}_b{second}'
            path = args.output / (name + '.m3p')
            path.write_bytes(scene)
            cases.append(dict(id=name, target_formulas=['RandCubesIFS'],
                              scene=str(path.resolve().relative_to(ROOT)), sha256=digest(scene),
                              source=base['scene'], source_sha256=base['sha256'],
                              profile=profile, shape1=first, shape2=second))
    write_json(args.output / 'manifest.json', dict(scope=__doc__, cases=cases))
    print(len(cases), 'scenes')


if __name__ == '__main__':
    main()
