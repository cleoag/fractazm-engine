"""Generate finite hybrid candidates for geometry/influence qualification.

Combine a named challenge formula with Integer Power 2 (or transformIFS for
KnotsIFS/KleinBotTIFS, plus a SphereIFS control shape) in both slot orders,
using 1/2/4 target repeats. Original and selected nonsingular parameter variants
are retained separately. Generation alone proves neither usefulness nor parity.
"""
import argparse
import json
import struct
from pathlib import Path
from external_validation import ROOT, decode, digest, write_json, slots, prepare_external_options


VARIANTS = {
    'General': ({0: 3., 1: 2., 3: 12., 4: 1., 5: 19., 6: 3.},
                {0: 2., 1: 3., 7: -1., 8: 1., 9: -1., 10: 2.}),
    'KleinBotTIFS': ({0: .8, 5: .3, 7: .1},
                      {1: 1.25, 2: .125, 3: -.25, 4: .5, 6: 0.}),
    'ABoxPlatinumB': ({3: 17., 4: -23., 6: .125, 8: -.25},
                       {10: 17., 11: -23., 12: 31., 13: -47., 14: 5.}),
    'KnotsIFS': ({7: 3., 8: 2., 9: 1.},
                 {0: 1.5, 5: .2, 7: 5., 8: -1., 9: 2., 10: -.35}),
    'Mandalex': ({1: .5}, {1: .5, 3: 17., 4: -23., 6: 1., 7: .5, 9: 2.}),
    'Mandalay-KIFS': ({1: .5}, {1: .5, 3: 17., 4: -23., 6: .5, 7: .5, 8: 1.5}),
    'BulboxP-2': ({5: .125, 6: -.25}, {2: .8, 5: .125, 7: .25, 8: -.5}),
    '_CelticMode': ({2: .5, 3: .25}, {0: .5, 1: .125, 2: -.5, 3: -.25}),
    'ABoxSphereOffset4d': ({3: .125, 4: -.25, 6: .25},
                           {1: .75, 3: -.25, 5: .125, 6: -.5, 7: .25}),
}


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--manifest', type=Path, required=True)
    ap.add_argument('--output', type=Path, required=True)
    ap.add_argument('--formula', action='append', choices=tuple(VARIANTS))
    a = ap.parse_args()
    bases = {c['id']: c for c in json.loads(a.manifest.read_bytes())['cases']}
    a.output.mkdir(parents=True, exist_ok=False)
    cases = []
    for name in a.formula or VARIANTS:
        difs = name in ('KnotsIFS', 'KleinBotTIFS')
        base = bases['challenge_' + name + '_early']
        source = ROOT / base['scene']
        if digest(source.read_bytes()) != base['sha256']:
            raise ValueError('Base scene changed: ' + name)
        original = decode(source)
        prepare_external_options(original, next(s for s in slots(original) if s['name'] == name))
        for variant, changes in enumerate(({}, *VARIANTS[name])):
            for order in ('first', 'last'):
                for repeat in (1, 2, 4):
                    scene = bytearray(original)
                    target = bytearray(scene[848:1036])
                    struct.pack_into('<i', target, 0, repeat)
                    for option, value in changes.items():
                        struct.pack_into('<d', target, 60 + option * 8, value)
                    power = bytearray(188)
                    struct.pack_into('<iii', power, 0, 1, 0, 2)
                    power[12:44] = b'Integer Power'.ljust(32, b'\0')
                    struct.pack_into('<2d', power, 60, 2., -1.)
                    if difs:
                        # dIFS partner: affine rotation/fold, rather than an
                        # escape-time power formula in a distance composition.
                        struct.pack_into('<iii', power, 0, 1, 25, 10)
                        power[12:44] = b'transformIFS'.ljust(32, b'\0')
                        struct.pack_into('<10d', power, 60, .125, -.25, .5,
                                         17., -23., 31., 0., 1., 0., 1.125)
                    scene[848:1976] = bytes(1128)
                    scene[848:1224] = target + power if order == 'first' else power + target
                    if difs:
                        # Omitting the target must leave a distance-producing shape.
                        # A transform alone is an invalid control, not evidence
                        # that the omitted formula affected the image.
                        sphere_base = bases['challenge_SphereIFS_early']
                        sphere_path = ROOT / sphere_base['scene']
                        if digest(sphere_path.read_bytes()) != sphere_base['sha256']:
                            raise ValueError('Base scene changed: SphereIFS')
                        sphere_scene = decode(sphere_path)
                        prepare_external_options(sphere_scene, next(s for s in slots(sphere_scene) if s['name'] == 'SphereIFS'))
                        sphere = bytearray(sphere_scene[848:1036])
                        struct.pack_into('<i', sphere, 0, 1)
                        struct.pack_into('<13d', sphere, 60, .25, 1., 0., 0.,
                                         -1.5, 1., 1., 1., 0., 0., 1., 1., -1.)
                        scene[1224:1412] = sphere
                        # Fit the target and its control shape into the viewport.
                        struct.pack_into('<d', scene, 84,
                                         struct.unpack_from('<d', original, 84)[0] * .35)
                    case_id = f'formula_hybrid_{name}_{variant}_{order}_{repeat}'
                    path = a.output / (case_id + '.m3p')
                    path.write_bytes(scene)
                    cases.append(dict(id=case_id, target_formulas=[name],
                        scene=str(path.resolve().relative_to(ROOT)), sha256=digest(scene),
                        source=base['scene'], source_sha256=base['sha256'],
                        mode=name, variant=variant, changed_options=changes,
                        order=order, repeat=repeat, partner='transformIFS' if difs else 'Integer Power',
                        control_shape='SphereIFS' if difs else None,
                        zoom_factor=.35 if difs else 1.))
    write_json(a.output / 'manifest.json', dict(scope=__doc__, cases=cases))
    print(len(cases), 'hybrid candidates')


if __name__ == '__main__':
    main()
