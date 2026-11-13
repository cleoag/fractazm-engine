"""Enumerate 12 rotation types x 10 documented powers x 3 slots x 2 flip flags.

Each selected slot varies independently; preceding slots have fixed operations.
This does not enumerate the Cartesian product of all three rotations.
Scenes can also be rendered by the image comparison runner.
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
    base = next(c for c in manifest['cases'] if c['id'] == 'challenge__IntPowRotateC_once')
    source = ROOT / base['scene']
    if digest(source.read_bytes()) != base['sha256']:
        raise ValueError('Base scene changed')
    data = decode(source)
    target = next(s for s in slots(data) if s['name'] == '_IntPowRotateC')
    index = prepare_external_options(data, target)['slot']
    offset = 848 + index * 188 + 60
    args.output.mkdir(parents=True, exist_ok=False)
    cases = []
    for rotation, power, slot, flip in itertools.product(range(12), range(2, 12), range(3), range(2)):
        scene = bytearray(data)
        options = [flip, 3, 1, int(slot >= 1), 5, 4, int(slot >= 2), 7, 8, .75]
        options[1 + slot*3] = power
        options[2 + slot*3] = rotation
        struct.pack_into('<10d', scene, offset, *options)
        name = f'intpow_c_slot{slot+1}_type{rotation:02}_power{power:02}_flip{flip}'
        path = args.output / (name + '.m3p')
        path.write_bytes(scene)
        cases.append(dict(id=name, target_formulas=['_IntPowRotateC'],
                          scene=str(path.resolve().relative_to(ROOT)), sha256=digest(scene),
                          source=base['scene'], source_sha256=base['sha256'],
                          rotation=rotation, power=power, slot=slot+1, flip=flip))
    write_json(args.output / 'manifest.json', dict(scope=__doc__, cases=cases))
    print(len(cases), 'scenes')


if __name__ == '__main__':
    main()
