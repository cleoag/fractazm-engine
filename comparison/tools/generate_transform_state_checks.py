"""Finite parameter crossings for 4D transforms and saved-position Julia offset.

Enumerates signed/zero strength and coefficients in the three chaotic systems,
all 15 pairs of six rotation axes at three signed angles, and radial offset
boundaries. The independent point oracle checks XYZ/W and sixteen state lanes.
These are one-step checks, not exhaustive orbits or floating-point coverage.
Optional iteration rotations cross all three axes at -90/0/37 degrees,
four multipliers and (for the power variant) three subtractions/four powers.
Optional random transforms vary each axis independently over range boundaries
and signed/word seed boundaries; they are not all Cartesian seed/range triples.
"""
import argparse
import itertools
import json
import math
import struct
from pathlib import Path
from external_validation import ROOT, decode, digest, slots, write_json, prepare_external_options


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--manifest', type=Path, required=True)
    ap.add_argument('--output', type=Path, required=True)
    ap.add_argument('--formula', action='append', choices=(
        '_lorenz', '_rossler', '_cosmartin', '_Rotate4d', '_PartlyJuliaRoff',
        '_RotatePlaneOnIts', '_RotateXYonIts', '_RotateXYonItsP', '_RotateRnd', '_TranslateRnd'))
    a = ap.parse_args()
    bases = json.loads(a.manifest.read_bytes())['cases']
    a.output.mkdir(parents=True, exist_ok=False)
    cases = []
    for name in a.formula or ('_lorenz', '_rossler', '_cosmartin', '_Rotate4d', '_PartlyJuliaRoff'):
        base = next(c for c in bases if c['id'] == 'challenge_' + name + '_early')
        source = ROOT / base['scene']
        if digest(source.read_bytes()) != base['sha256']:
            raise ValueError('Changed source')
        data = decode(source)
        slot = next(s for s in slots(data) if s['name'] == name)
        slot = prepare_external_options(data, slot)
        variants = []
        if name in ('_RotateRnd', '_TranslateRnd'):
            ranges = (-3., math.nextafter(-2., -math.inf), -2., -1., -0., 0., 1., 2.,
                      math.nextafter(2., math.inf), 3.)
            for axis, value, seed in itertools.product(range(3), ranges,
                    (-2147483648, -65536, -1, 0, 1, 16383, 65535, 2147483647)):
                options = slot['options'][:]
                options[axis] = value
                options[3 + axis] = seed
                variants.append(options)
        elif name in ('_RotatePlaneOnIts', '_RotateXYonIts', '_RotateXYonItsP'):
            for angles, multiplier in itertools.product(
                    itertools.product((-90., 0., 37.), repeat=3), (-360., 0., .5, 360.)):
                for sub, power in (itertools.product((0., .5, 1.), (-.8, 0., .5, 2.))
                                   if name.endswith('P') else ((0., 0.),)):
                    options = slot['options'][:]
                    options[:4] = [*angles, multiplier]
                    if name.endswith('P'):
                        options[4:6] = [sub, power]
                    variants.append(options)
        elif name == '_Rotate4d':
            for axes, angles in itertools.product(itertools.combinations(range(6), 2),
                                                  itertools.product((-61., 0., 37.), repeat=2)):
                options = slot['options'][:]
                options[:6] = [0.] * 6
                for axis, angle in zip(axes, angles):
                    options[axis] = angle
                variants.append(options)
        elif name == '_PartlyJuliaRoff':
            for center, radius in itertools.product(((0., 0., 0.), (.125, -.25, .5), (.25, -.5, 1.)),
                                                   (-2., -1e-10, -1e-12, 0., 1e-12, 1e-10, 1., 2.)):
                options = slot['options'][:]
                options[:4] = [*center, radius]
                variants.append(options)
        else:
            for values in itertools.product((-1.25, 0., .75), repeat=5):
                options = slot['options'][:]
                options[:5] = values
                variants.append(options)
        for index, options in enumerate(variants):
            scene = bytearray(data)
            offset = 848 + 188 * slot['slot']
            struct.pack_into('<16d', scene, offset+60, *options)
            case = f'transform_state_{name}_{index:04d}'
            path = a.output / (case + '.m3p')
            path.write_bytes(scene)
            cases.append(dict(id=case, target_formulas=[name], options=options,
                              scene=str(path.resolve().relative_to(ROOT)), sha256=digest(scene),
                              source=base['scene'], source_sha256=base['sha256']))
    write_json(a.output/'manifest.json', dict(scope=__doc__, cases=cases))
    print(len(cases), 'scenes')


if __name__ == '__main__':
    main()
