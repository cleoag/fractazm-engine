"""Pairwise 4D rotation planes, unequal centers and signed/zero scale.

All 15 pairs of six rotation angles are exercised at four scales.
Defaults to Menger4ex and MixPinski4ex; --formula also selects their base forms. This is a finite stress set, not exhaustive
parameter or floating-point coverage. Original-code tests share C++ expansion.
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
    ap.add_argument('--formula', action='append', choices=('Menger4', 'MixPinski4', 'Menger4ex', 'MixPinski4ex'))
    args = ap.parse_args()
    bases = json.loads(args.manifest.read_bytes())['cases']
    args.output.mkdir(parents=True, exist_ok=False)
    cases = []
    for name in (args.formula or ('Menger4ex', 'MixPinski4ex')):
        base = next(c for c in bases if c['id'] == 'challenge_' + name + '_early')
        source = ROOT / base['scene']
        if digest(source.read_bytes()) != base['sha256']:
            raise ValueError('Base scene changed')
        data = decode(source)
        slot = next(s for s in slots(data) if s['name'] == name)
        slot = prepare_external_options(data, slot)
        for first, second in itertools.combinations(range(6), 2):
            for index, scale in enumerate((-2.5, 0., .125, 3.)):
                options = slot['options'][:]
                options[:5] = [scale, .7, -1.3, .2, -.6]
                options[5:11] = [0.] * 6
                options[5 + first], options[5 + second] = 37., -61.
                if name.endswith('ex'):
                    options[11:15] = [.125, -.75, 1.5, -.375]
                scene = bytearray(data)
                struct.pack_into('<16d', scene, 848 + slot['slot'] * 188 + 60, *options)
                case = f'four_d_ex_{name}_r{first}_{second}_s{index}'
                path = args.output / (case + '.m3p')
                path.write_bytes(scene)
                cases.append(dict(id=case, target_formulas=[name],
                                  scene=str(path.resolve().relative_to(ROOT)), sha256=digest(scene),
                                  source=base['scene'], source_sha256=base['sha256'],
                                  angles=[first, second], scale=scale))
    write_json(args.output / 'manifest.json', dict(scope=__doc__, cases=cases))
    print(len(cases), 'scenes')


if __name__ == '__main__':
    main()
