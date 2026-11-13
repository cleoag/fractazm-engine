"""Enumerate all documented _GenCWarp selector combinations, in two profiles.

24 theta selectors x 24 phi selectors x 3 recipes are exhaustive only for
these three discrete options. Rotation, powers, blending and input points
remain finite samples. Generated scenes are also usable by the image runner.
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
    base = next(c for c in manifest['cases'] if c['id'] == 'challenge__GenCWarp_once')
    source = ROOT / base['scene']
    if digest(source.read_bytes()) != base['sha256']:
        raise ValueError('Base scene changed')
    data = decode(source)
    target = next(s for s in slots(data) if s['name'] == '_GenCWarp')
    index = prepare_external_options(data, target)['slot']
    offset = 848 + index * 188 + 60
    args.output.mkdir(parents=True, exist_ok=False)
    cases = []
    profiles = {'default': [0, 0, 0, 8, 8, 0, 0, 0, 1, 0],
                'rotated_blended': [37, -29, 63, 1.5, -2.25, 0, 0, 0, .5, .25]}
    for profile, values in profiles.items():
        for theta, phi, recipe in itertools.product(range(24), range(24), range(3)):
            scene = bytearray(data)
            options = list(values)
            options[5:8] = theta, phi, recipe
            struct.pack_into('<10d', scene, offset, *options)
            name = f'gencwarp_{profile}_t{theta:02}_p{phi:02}_r{recipe}'
            path = args.output / (name + '.m3p')
            path.write_bytes(scene)
            cases.append(dict(id=name, target_formulas=['_GenCWarp'],
                              scene=str(path.resolve().relative_to(ROOT)), sha256=digest(scene),
                              source=base['scene'], source_sha256=base['sha256'],
                              profile=profile, theta=theta, phi=phi, recipe=recipe))
    write_json(args.output / 'manifest.json', dict(scope=__doc__, profiles=profiles,
               discrete_combinations=24*24*3, cases=cases))
    print(len(cases), 'scenes')


if __name__ == '__main__':
    main()
