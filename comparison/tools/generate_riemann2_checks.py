"""2187 finite Riemann2 crossings; not exhaustive real-valued parameter coverage.

All three rotation axes, signed/zero scale and power, projection threshold and
signed sine fold. Asymmetric offsets expose swapped coordinates. The point
oracle shares C++ parameter expansion; complete orbit/image checks are separate.
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
                if c['id'] == 'challenge_Riemann2_early')
    source = ROOT / base['scene']
    if digest(source.read_bytes()) != base['sha256']:
        raise ValueError('Changed source')
    data = decode(source)
    slot = next(s for s in slots(data) if s['name'] == 'Riemann2')
    slot = prepare_external_options(data, slot)
    a.output.mkdir(parents=True, exist_ok=False)
    cases = []
    variants = itertools.product(itertools.product((-90., 0., 37.), repeat=3),
                                 (-1., 0., 1.), (-2., 0., 2.),
                                 (-1e-12, 0., 1e-5), (-.5, 0., .5))
    for index, (angles, scale, power, threshold, fold) in enumerate(variants):
        options = slot['options'][:]
        options[:11] = [*angles, scale, power, -.25, .125, -.375, 1., fold, threshold]
        scene = bytearray(data)
        struct.pack_into('<16d', scene, 848 + slot['slot'] * 188 + 60, *options)
        name = f'riemann2_parameters_{index:04d}'
        path = a.output / (name + '.m3p')
        path.write_bytes(scene)
        cases.append(dict(id=name, target_formulas=['Riemann2'], options=options,
                          scene=str(path.resolve().relative_to(ROOT)), sha256=digest(scene),
                          source=base['scene'], source_sha256=base['sha256']))
    write_json(a.output / 'manifest.json', dict(scope=__doc__, cases=cases))
    print(len(cases), 'scenes')


if __name__ == '__main__':
    main()
