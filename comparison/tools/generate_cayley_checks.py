"""Finite Cayley dIFS crossings: rotation, signed/zero scale, apply and cubic weight.

972 crossings plus 36 radius/offset boundary cases. These exercise local queries
versus orbit writeback and both sign accumulators, not all real parameters or
complete orbit combinations. Parameter expansion is shared with the point oracle.
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
    a = ap.parse_args()
    base = next(c for c in json.loads(a.manifest.read_bytes())['cases']
                if c['id'] == 'challenge_cayleySurfIFS_early')
    source = ROOT / base['scene']
    if digest(source.read_bytes()) != base['sha256']:
        raise ValueError('Changed source')
    data = decode(source)
    slot = next(s for s in slots(data) if s['name'] == 'cayleySurfIFS')
    slot = prepare_external_options(data, slot)
    variants = []
    for angles, scale, apply, coefficient in itertools.product(
            itertools.product((-90., 0., 37.), repeat=3),
            (-2., 0., 1e-12, 2.), (-1, 0, 1), (-1.6, 0., 1.6)):
        options = slot['options'][:]
        options[:10] = [1.5, scale, .125, -.25, .5, coefficient, apply, *angles]
        variants.append(options)
    for radius, offset, apply in itertools.product((-1.5, 0., 1e-12, 1.5),
                                                   (-1e-12, 0., 1e-12), (-1, 0, 1)):
        options = slot['options'][:]
        options[:10] = [radius, 2., offset, -offset, offset, 1.6, apply, 37., -61., 23.]
        variants.append(options)
    a.output.mkdir(parents=True, exist_ok=False)
    cases = []
    for index, options in enumerate(variants):
        scene = bytearray(data)
        struct.pack_into('<16d', scene, 848 + slot['slot'] * 188 + 60, *options)
        name = f'cayley_parameters_{index:04d}'
        path = a.output / (name + '.m3p')
        path.write_bytes(scene)
        cases.append(dict(id=name, target_formulas=['cayleySurfIFS'], options=options,
                          scene=str(path.resolve().relative_to(ROOT)), sha256=digest(scene),
                          source=base['scene'], source_sha256=base['sha256']))
    write_json(a.output / 'manifest.json', dict(scope=__doc__, cases=cases))
    print(len(cases), 'scenes')


if __name__ == '__main__':
    main()
