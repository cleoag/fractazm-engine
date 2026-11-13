"""810 finite power/axiom/shift/C-weight cases for the signed MagVsXYZ variants.

The last documented C weight is unused by the shipped code. We vary it too;
these checks target executable behavior, with shared C++ parameter expansion.
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
    manifest = json.loads(a.manifest.read_bytes())
    a.output.mkdir(parents=True, exist_ok=False)
    cases = []
    for formula in ('MagVsXYZabs3', 'MagVsXYZmsw'):
        base = next(c for c in manifest['cases'] if c['id'] == f'challenge_{formula}_early')
        source = ROOT / base['scene']
        if digest(source.read_bytes()) != base['sha256']:
            raise ValueError('Changed source')
        data = decode(source)
        slot = next(s for s in slots(data) if s['name'] == formula)
        slot = prepare_external_options(data, slot)
        variants = itertools.product((-2., 0., 1., 2., 3.5), (-2., 0., 1.5707963),
                                     (-2., 0., 1.5707963), (-1., 0., .5), (-2., 0., 1.))
        for index, (power, axiom1, axiom2, shift, weight) in enumerate(variants):
            options = slot['options'][:]
            options[:7] = [power, axiom1, axiom2, shift, weight, -weight, 2 * weight]
            scene = bytearray(data)
            struct.pack_into('<16d', scene, 848 + slot['slot'] * 188 + 60, *options)
            name = f'mag_sign_{formula}_{index:04d}'
            path = a.output / (name + '.m3p')
            path.write_bytes(scene)
            cases.append(dict(id=name, target_formulas=[formula], options=options,
                              scene=str(path.resolve().relative_to(ROOT)), sha256=digest(scene),
                              source=base['scene'], source_sha256=base['sha256']))
    write_json(a.output / 'manifest.json', dict(scope=__doc__, cases=cases))
    print(len(cases), 'scenes')


if __name__ == '__main__':
    main()
