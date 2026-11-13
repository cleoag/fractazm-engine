"""625 finite signed/zero center and radius combinations for _hyc3Dinv.

These include the singular centered diagonal and tiny offsets. Parameter
expansion is shared with the x86 oracle; no exhaustive real-domain claim.
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
                if c['id'] == 'challenge__hyc3Dinv_early')
    source = ROOT / base['scene']
    if digest(source.read_bytes()) != base['sha256']:
        raise ValueError('Changed source')
    data = decode(source)
    slot = next(s for s in slots(data) if s['name'] == '_hyc3Dinv')
    slot = prepare_external_options(data, slot)
    a.output.mkdir(parents=True, exist_ok=False)
    cases = []
    for index, values in enumerate(itertools.product((-2., -1e-12, 0., 1e-12, 2.), repeat=4)):
        options = slot['options'][:]
        options[:4] = values
        scene = bytearray(data)
        struct.pack_into('<16d', scene, 848 + slot['slot'] * 188 + 60, *options)
        name = f'hycinv_parameters_{index:04d}'
        path = a.output / (name + '.m3p')
        path.write_bytes(scene)
        cases.append(dict(id=name, target_formulas=['_hyc3Dinv'], options=options,
                          scene=str(path.resolve().relative_to(ROOT)), sha256=digest(scene),
                          source=base['scene'], source_sha256=base['sha256']))
    write_json(a.output / 'manifest.json', dict(scope=__doc__, cases=cases))
    print(len(cases), 'scenes')


if __name__ == '__main__':
    main()
