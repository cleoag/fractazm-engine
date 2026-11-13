"""ABoxVSShapes: every shape and invalid mode crossed with finite modifiers.

Also crosses signed/zero box folds with scale variation. These are finite
parameter checks, not exhaustive coverage of all real inputs or whole orbits.
"""
import argparse
import json
import struct
from pathlib import Path
from external_validation import ROOT, decode, digest, slots, write_json, prepare_external_options


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--manifest', type=Path, required=True)
    ap.add_argument('--output', type=Path, required=True)
    args = ap.parse_args()
    original = json.loads(args.manifest.read_bytes())['cases']
    base = next(c for c in original if c['id'] == 'challenge_ABoxVSShapes_early')
    source = ROOT/base['scene']
    if digest(source.read_bytes()) != base['sha256']:
        raise ValueError('Base scene changed')
    data = decode(source)
    slot = next(s for s in slots(data) if s['name'] == 'ABoxVSShapes')
    slot = prepare_external_options(data, slot)
    args.output.mkdir(parents=True, exist_ok=False)
    cases, state_cases = [], []
    modes = [-2147483648, -1, 0, *range(1, 19), 2147483647]
    for mode in modes:
        variants = [('nominal', {})]
        variants += [(f'mod_{i}_{j}', {5: a, 6: b})
                     for i, a in enumerate([-1., 0., .25, 1., 1.5])
                     for j, b in enumerate([-.5, 0., .125, .666, 2.])]
        variants += [(f'fold_{i}_vary_{j}', {2: fold, 3: vary})
                     for i, fold in enumerate([-1., 0., .25])
                     for j, vary in enumerate([-.5, .5])]
        for tag, changes in variants:
            options = slot['options'][:]
            changes = {4: mode, **changes}
            for field, value in changes.items():
                options[field] = value
            scene = bytearray(data)
            struct.pack_into('<16d', scene, 848+slot['slot']*188+60, *options)
            case_id = f'vsshapes_{mode}_{tag}'
            path = args.output/(case_id+'.m3p')
            path.write_bytes(scene)
            case = dict(id=case_id, target_formulas=['ABoxVSShapes'],
                        scene=str(path.resolve().relative_to(ROOT)),
                        sha256=digest(scene), source=base['scene'],
                        source_sha256=base['sha256'], changes=changes)
            cases.append(case)
            if tag == 'nominal':
                state_cases.append(case)
    write_json(args.output/'manifest.json', dict(scope=__doc__, cases=cases))
    write_json(args.output/'state-manifest.json', dict(scope=__doc__, cases=state_cases))
    write_json(args.output/'challenges-manifest.json', dict(
        scope='Original broad challenges for ABoxVSShapes',
        cases=[c for c in original if c['target_formulas'] == ['ABoxVSShapes']]))
    print(len(cases), 'scenes;', len(state_cases), 'nominal state cases')


if __name__ == '__main__':
    main()
