"""Sine transforms: all masked coordinate selectors crossed with signed scales.

Selectors -1..4 exercise all XYZ/W choices and mask wraparound. Offsets and
scales include negative and zero values. One-step coverage is finite; the
point checker separately supplies range-reduction boundary inputs.
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
    for name in ('_SinApprox', '_PlusSinApp'):
        base = next(c for c in bases if c['id'] == 'challenge_' + name + '_early')
        source = ROOT / base['scene']
        if digest(source.read_bytes()) != base['sha256']:
            raise ValueError('Changed source')
        data = decode(source)
        slot = next(s for s in slots(data) if s['name'] == name)
        slot = prepare_external_options(data, slot)
        count = 1 if name == '_SinApprox' else 2
        variants = itertools.product(itertools.product(range(-1, 5), repeat=count),
                                     (-1.25, 0., .75), (-2., 0., 1.5), (-.5, 0., 2.))
        for index, (selectors, offset, scale1, scale2) in enumerate(variants):
            options = slot['options'][:]
            options[:count+4] = [*selectors, offset, scale1, scale2, .25]
            scene = bytearray(data)
            struct.pack_into('<16d', scene, 848 + 188 * slot['slot'] + 60, *options)
            case = f'sine_approx_{name}_{index:04d}'
            path = a.output / (case + '.m3p')
            path.write_bytes(scene)
            cases.append(dict(id=case, target_formulas=[name], options=options,
                              scene=str(path.resolve().relative_to(ROOT)), sha256=digest(scene),
                              source=base['scene'], source_sha256=base['sha256']))
    write_json(a.output/'manifest.json', dict(scope=__doc__, cases=cases))
    print(len(cases), 'scenes')


if __name__ == '__main__':
    main()
