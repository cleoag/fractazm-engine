"""Finite writeback checks: signed powers/weights, rotations, scales and flags.

_setw has three tested mode values, seven powers and every triple of three
weights. _Transform covers every triple of three angles, five scales and both
copy flags. _updateC2 covers the same angles/scales with three signed offsets.
The point runner separately varies DoJulia and derivative state.
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
    bases = json.loads(a.manifest.read_bytes())['cases']
    a.output.mkdir(parents=True, exist_ok=False)
    cases = []
    for name in ('_setw', '_Transform', '_updateC2'):
        base = next(c for c in bases if c['id'] == 'challenge_' + name + '_early')
        source = ROOT / base['scene']
        if digest(source.read_bytes()) != base['sha256']:
            raise ValueError('Changed source')
        data = decode(source)
        slot = next(s for s in slots(data) if s['name'] == name)
        slot = prepare_external_options(data, slot)
        variants = []
        if name == '_setw':
            for mode, power, weights in itertools.product((-1, 0, 1),
                    (-2., -.5, 0., .5, 1., 2., 3.), itertools.product((-1., 0., 1.), repeat=3)):
                variants.append([mode, power, *weights])
        elif name == '_Transform':
            for angles, scale, copy in itertools.product(itertools.product((-61., 0., 37.), repeat=3),
                                                         (-2., -.5, 0., .5, 2.), (0, 1)):
                variants.append([scale, .25, -.5, .75, *angles, copy])
        else:
            for angles, scale, offset in itertools.product(itertools.product((-61., 0., 37.), repeat=3),
                                                           (-2., -.5, 0., .5, 2.), (-.75, 0., .25)):
                variants.append([*angles, scale, offset, -offset, offset])
        for index, values in enumerate(variants):
            options = slot['options'][:]
            options[:len(values)] = values
            scene = bytearray(data)
            struct.pack_into('<16d', scene, 848 + 188 * slot['slot'] + 60, *options)
            case = f'state_writeback_{name}_{index:04d}'
            path = a.output / (case + '.m3p')
            path.write_bytes(scene)
            cases.append(dict(id=case, target_formulas=[name], options=options,
                              scene=str(path.resolve().relative_to(ROOT)), sha256=digest(scene),
                              source=base['scene'], source_sha256=base['sha256']))
    write_json(a.output/'manifest.json', dict(scope=__doc__, cases=cases))
    print(len(cases), 'scenes')


if __name__ == '__main__':
    main()
