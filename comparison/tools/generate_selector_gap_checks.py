"""Generate the 27 reviewed marginal selector gaps from the Stage A scene audit.

Uses existing, identity-checked challenge scenes and the canonical M3F metadata.
Compound options are saved with their full VALUE count, so loader migration
cannot silently replace an explicitly selected final option with its default.
These are finite point/image inputs, not exhaustive branch certification.
"""
import argparse
import json
from pathlib import Path
import re
import struct

from external_validation import ROOT, decode, digest, slots, write_json
from gen_bulk_formula_table import parse_m3f_options, compute_pvar_offsets

WIDTH = {5: 3, 6: 3, 12: 6, 18: 4, 19: 2}
GAPS = [
    ('SphereIFS', 9, [2, 3]),
    ('boxIFS', 10, [2, 3]),
    ('cylinderIFS', 9, [2, 3]),
    ('Amazing Surf 2', 6, list(range(2, 16))),
    ('boardIFS', 10, [2, 3, 4]),
    ('koch_surf', 13, [2, 3]),
    ('RoundedBox', 9, [2, 3]),
]


def generate(manifest, output):
    bases = json.loads(manifest.read_bytes())['cases']
    output.mkdir(parents=True, exist_ok=False)
    cases = []
    for name, stored, values in GAPS:
        base = next(c for c in bases if c['id'] == 'challenge_' + name + '_early')
        source = ROOT / base['scene']
        if digest(source.read_bytes()) != base['sha256']:
            raise ValueError('Source scene identity changed: ' + name)
        scene = decode(source)
        slot = next(s for s in slots(scene) if s['name'] == name)
        offset = 848 + 188 * slot['slot']
        asset = ROOT / 'mb3d/M3Formulas' / (name + '.m3f')
        metadata = parse_m3f_options(asset)
        types = metadata['option_types']
        expanded_types = [6 if kind == 18 and k > 0 else kind
                          for kind in types for k in range(WIDTH.get(kind, 1))]
        if len(expanded_types) != len(metadata['option_defaults']):
            raise ValueError('Unexpected compound width: ' + name)
        if len(expanded_types) > 16:
            raise ValueError('Scene option overflow: ' + name)
        saved = 0
        declaration = None
        for index, kind in enumerate(types):
            if saved == stored:
                declaration = index
                break
            saved += WIDTH.get(kind, 1)
        if declaration is None or types[declaration] != 2:
            raise ValueError('Selector is not an INTEGER declaration: ' + name)
        runtime = len(compute_pvar_offsets(types[:declaration]))
        pvar_offset, value_type = compute_pvar_offsets(types)[runtime]
        assert value_type == 2
        # Reconstruct the options the current loader would provide, then make
        # the selected values explicit in a current full-count saved scene.
        options = slot['options'][:]
        for i in range(slot['option_count'], len(expanded_types)):
            options[i] = metadata['option_defaults'][i]
        if name == 'Amazing Surf 2':
            # Existing generate_ifs_checks finite recipe. Nonzero radius and
            # FoldXY exercise the smooth/abs bits without a singular setup.
            options[:6] = [2., .5, .5, 0., 0., 0.]
        struct.pack_into('<i', scene, offset + 8, len(expanded_types))
        scene[offset + 44:offset + 60] = bytes(expanded_types).ljust(16, b'\0')
        for value in values:
            data = bytearray(scene)
            selected = options[:]
            selected[stored] = float(value)
            struct.pack_into('<16d', data, offset + 60, *selected)
            case_id = f"selector_gap_{re.sub(r'[^A-Za-z0-9_]', '_', name)}_{stored}_{value}"
            path = output / (case_id + '.m3p')
            path.write_bytes(data)
            cases.append(dict(id=case_id, target_formulas=[name],
                scene=path.resolve().relative_to(ROOT).as_posix(), sha256=digest(data),
                source=base['scene'], source_sha256=base['sha256'],
                asset=asset.relative_to(ROOT).as_posix(), asset_sha256=digest(asset.read_bytes()),
                options=selected, saved_option_count=len(expanded_types),
                selector=dict(declaration_index=declaration, stored_value_index=stored,
                              runtime_param_index=runtime, pvar_offset=pvar_offset,
                              int32_value=value),
                image_scope='geometry' if name in ('Amazing Surf 2', 'boardIFS', 'koch_surf') else 'orbit_trap',
                required_output_lanes=list(range(20))))
    write_json(output / 'manifest.json', dict(scope=__doc__,
        source_manifest_sha256=digest(manifest.read_bytes()), cases=cases))
    print(len(cases), 'scenes')


if __name__ == '__main__':
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--manifest', type=Path, required=True)
    ap.add_argument('--output', type=Path, required=True)
    args = ap.parse_args()
    generate(args.manifest, args.output)
