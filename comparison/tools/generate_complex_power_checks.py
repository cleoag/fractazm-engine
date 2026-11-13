"""GenIntPower rotation and radius-power branch coverage.

All 12^3 rotation-type triples; each documented rotation power in each slot;
radius-power signed branch boundaries and enable flags. This finite set does
not exhaust continuous inputs or the product of every type and every power.
Original-code comparisons share the port's parameter expansion.
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
    base = next(c for c in json.loads(args.manifest.read_bytes())['cases']
                if c['id'] == 'challenge_GenIntPower_early')
    source = ROOT / base['scene']
    if digest(source.read_bytes()) != base['sha256']:
        raise ValueError('Base scene changed')
    data = decode(source)
    slot = next(s for s in slots(data) if s['name'] == 'GenIntPower')
    slot = prepare_external_options(data, slot)
    args.output.mkdir(parents=True, exist_ok=False)
    variants = []
    for types in itertools.product(range(12), repeat=3):
        variants.append(('triple_' + '_'.join(map(str, types)),
                         [8, 3, types[0], 1, 5, types[1], 1, 7, types[2], .75]))
    for rotation, power, index in itertools.product(range(12), range(2, 12), range(3)):
        options = [8, 3, 1, int(index >= 1), 5, 4, int(index >= 2), 7, 8, -.75]
        options[1 + index*3], options[2 + index*3] = power, rotation
        variants.append((f'slot{index}_type{rotation}_power{power}', options))
    boundaries = [-2147483648, -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 2147483647]
    for power, flags in itertools.product(boundaries, itertools.product((0, 1), repeat=2)):
        variants.append((f'radius{power}_enabled{flags[0]}{flags[1]}',
                         [power, 3, 1, flags[0], 5, 4, flags[1], 7, 8, .75]))
    for index, power, kind in itertools.product(range(3), boundaries, (-1, 0, 5, 6, 11, 12)):
        options = [8, 3, 1, int(index >= 1), 5, 4, int(index >= 2), 7, 8, .75]
        options[1 + index*3], options[2 + index*3] = power, kind
        variants.append((f'boundary_slot{index}_type{kind}_power{power}', options))
    cases = []
    for suffix, options in variants:
        scene = bytearray(data)
        struct.pack_into('<10d', scene, 848 + slot['slot']*188 + 60, *options)
        name = 'genintpower_' + suffix
        path = args.output / (name + '.m3p')
        path.write_bytes(scene)
        cases.append(dict(id=name, target_formulas=['GenIntPower'], options=options,
                          scene=str(path.resolve().relative_to(ROOT)), sha256=digest(scene),
                          source=base['scene'], source_sha256=base['sha256']))
    write_json(args.output / 'manifest.json', dict(scope=__doc__, cases=cases))
    print(len(cases), 'scenes')


if __name__ == '__main__':
    main()
