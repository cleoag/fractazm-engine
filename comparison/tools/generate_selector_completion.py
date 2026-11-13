"""Prepare the missing finite selector representatives from an effective audit.

This is a minimal marginal input set, not an exhaustive interaction proof. Saved
compound options use the actual value count; the checker must additionally verify
the selected INTEGER in the native expanded parameter buffer.
"""
import argparse
import json
from pathlib import Path
import re
import struct

from external_validation import ROOT, decode, digest, prepare_external_options, slots, write_json
from generate_external_challenges import ensure_conditional_partners


def generate(a):
    audit = json.loads(a.audit.read_bytes())
    ledger = json.loads(a.ledger.read_bytes())
    previous = [json.loads(path.read_bytes()) for path in a.already_prepared]
    prepared = {(c['target_formulas'][0], c['selector']['stored_value_index'],
                 c['selector']['int32_value']) for m in previous for c in m['cases']}
    base_cases = {c['case']: c for c in ledger['external_cases']}
    a.output.mkdir(parents=True, exist_ok=False)
    cases = []
    for option in audit['options']:
        name = option['formula']
        values = [v for v in option['missing_representatives_in_scene_catalog']
                  if (name, option['stored_value_index'], v) not in prepared]
        if not values:
            continue
        if name.startswith('JIT'):
            raise ValueError('Deferred formula unexpectedly included')
        base = base_cases['challenge_' + name + '_early']
        source = ROOT / base['scene']
        if digest(source.read_bytes()) != base['scene_sha256']:
            raise ValueError('Source identity changed: ' + str(source))
        scene = decode(source)
        slot = next(s for s in slots(scene) if s['name'] == name)
        slot = prepare_external_options(scene, slot)
        offset = 848 + 188 * slot['slot']
        for value in values:
            data = bytearray(scene)
            struct.pack_into('<d', data, offset + 60 + 8 * option['stored_value_index'], float(value))
            partners = ensure_conditional_partners(data, name)
            case_id = 'selector_complete_' + re.sub(r'[^A-Za-z0-9_]', '_', name)
            case_id += f"_{option['stored_value_index']}_{value}"
            path = a.output / (case_id + '.m3p')
            path.write_bytes(data)
            selected = next(s for s in slots(data) if s['name'] == name)
            cases.append(dict(id=case_id, target_formulas=[name],
                scene=path.resolve().relative_to(ROOT).as_posix(), sha256=digest(data),
                source=base['scene'], source_sha256=base['scene_sha256'],
                asset=option['asset'], options=selected['options'],
                saved_option_count=selected['option_count'],
                selector=dict(declaration_index=option['declaration_index'],
                    stored_value_index=option['stored_value_index'],
                    runtime_param_index=option['runtime_param_index'],
                    pvar_offset=option['pvar_offset'], int32_value=value),
                domain=option['domain'], conditional_partners=partners,
                required_output_lanes=list(range(26 if partners else 20))))
    report = dict(scope=__doc__, cases=cases,
        input_sha256={str(p.resolve().relative_to(ROOT)): digest(p.read_bytes())
                      for p in [a.audit, a.ledger, *a.already_prepared]},
        selected_cases=len(cases), previously_prepared=len(prepared))
    write_json(a.output / 'manifest.json', report)
    for kind in ('generic', 'conditional'):
        write_json(a.output / (kind + '.json'), {**report, 'cases': [c for c in cases
            if bool(c['conditional_partners']) == (kind == 'conditional')]})
    print(len(cases), 'new scenes', len(prepared), 'already prepared')


if __name__ == '__main__':
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--audit', type=Path, required=True)
    ap.add_argument('--ledger', type=Path, required=True)
    ap.add_argument('--already-prepared', type=Path, action='append', default=[])
    ap.add_argument('--output', type=Path, required=True)
    generate(ap.parse_args())
