"""Capture original x86 signed-overflow dispatch with every rotation enabled.

Only immutable M3F CODE produces expected outputs. The C++ probe supplies the
packed parameter buffer; selector/enable words are verified, not loader parity.
"""
import argparse
import hashlib
import json
import os
from pathlib import Path
import struct

from capture_formula_regressions import literal
from check_formula_scene_points import execute, sha
from external_validation import ROOT, decode, slots, write_json, prepare_external_options


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    for key in ('cpp', 'oracle', 'manifest', 'output', 'header'):
        ap.add_argument('--'+key, type=Path, required=True)
    args = ap.parse_args()
    args.output.mkdir(parents=True, exist_ok=False)
    inputs = [
        [.125, -.25, .5, .375, .2, -.3, .4],
        [-1.6881121273612618, 2.497941324534125, -1.890485888505075, -.9964311313664636,
         .20659375880868347, .09669982477566297, .900455238309702],
        [-2.799724630955355, 2.6712770120848965, -.656797130811035, -.15145224208822872,
         -.9688481423606135, .34651719482765286, -.9957979133788728],
        [0., 0., 2., .375, .2, -.3, .4],
        [-0., -0., -.25, -.375, -.2, 0., .4],
        [5e-324, -5e-324, .5, .75, .2, -.3, .4],
        [1e-160, -1e-160, .5, .75, 1e-160, -1e-160, .4],
        [1e160, -1e160, .5, .75, 1e160, -1e160, .4],
    ]
    stdin = ''.join(' '.join(format(v, '.17g') for v in row+[1., -2., .375, 3.])+'\n' for row in inputs)
    os.environ['MB3D_POINT_DERIVATIVES'] = '1'
    rows, cases, checks, assets = [], [], [], {}
    for base in json.loads(args.manifest.read_bytes())['cases']:
        name = base['target_formulas'][0]
        selected = base['selector']['stored_value_index']
        data = decode(ROOT/base['scene'])
        slot = next(s for s in slots(data) if s['name'] == name)
        prepare_external_options(data, slot)
        offset = 848+188*slot['slot']
        asset = ROOT/base['asset']['path']
        code = args.output/(name+'.code.bin')
        if name not in assets:
            code.write_bytes(bytes.fromhex(asset.read_text().split('[CODE]')[1].split('[END]')[0]))
            assets[name] = dict(path=base['asset']['path'], sha256=sha(asset), code_sha256=sha(code))
        for value in (-2147483648, -2147483647, -1, 0, 1, 2147483647):
            scene = bytearray(data)
            options = list(struct.unpack_from('<16d', scene, offset+60))
            if name != 'GeneralQuat':
                options[0] = 1. if name == '_IntPowRotateC' else options[0]
                options[3] = options[6] = 1.
            options[selected] = float(value)
            struct.pack_into('<16d', scene, offset+60, *options)
            case_id = f'overflow_active_{name}_{selected}_{value}'
            path = args.output/(case_id+'.m3p')
            path.write_bytes(scene)
            blob = args.output/(case_id+'.params')
            execute([args.cpp.resolve(), name, path.resolve(), blob.resolve(), '--dump-only'], '')
            packed = blob.read_bytes()
            actual = struct.unpack_from('<i', packed, 512+base['selector']['pvar_offset'])[0]
            if actual != value: raise ValueError('Selected INTEGER changed while loading')
            enabled = {}
            if name != 'GeneralQuat':
                for i in ((0,3,6) if name == '_IntPowRotateC' else (3,6)):
                    enabled[i] = struct.unpack_from('<i', packed, 512-12-i*4)[0]
                if any(v != 1 for v in enabled.values()): raise ValueError('Dormant rotation')
            expected = execute([args.oracle.resolve(), code.resolve(), blob.resolve()], stdin)
            if len(expected) != len(inputs) or any(len(r) != 20 for r in expected): raise ValueError('Incomplete original output')
            checks.append(dict(case=case_id, selector=actual, enabled=enabled, parameter_sha256=sha(blob)))
            cases.append(dict(id=case_id, target_formulas=[name], scene=path.resolve().relative_to(ROOT).as_posix(), sha256=sha(path)))
            for xyz, output in zip(inputs, expected):
                rows.append(dict(case=case_id, name=name, option_count=base['saved_option_count'], options=options,
                                 input=xyz, output=[format(v, '.17g') for v in output]))
    report = dict(scope=__doc__, cpp_parameter_probe_sha256=sha(args.cpp), oracle_sha256=sha(args.oracle),
                  source_manifest_sha256=sha(args.manifest), generator_sha256=sha(Path(__file__)),
                  input_sha256=hashlib.sha256(stdin.encode()).hexdigest(), assets=assets,
                  cases=cases, parameter_checks=checks, rows=rows)
    write_json(args.output/'golden.json', report)
    write_json(args.output/'manifest.json', dict(scope=__doc__, cases=cases))
    header = '// Independent original x86 outputs; capture_selector_overflow_regressions.py.\n'
    header += 'struct FormulaRegression { const char* name; int option_count; double options[16], input[7], output[20]; const char* case_id; };\n'
    header += 'static const FormulaRegression formulaRegressions[] = {\n'
    for row in rows:
        arrays = ', '.join('{'+', '.join(map(literal,row[k]))+'}' for k in ('options','input','output'))
        header += '    {'+json.dumps(row['name'])+', '+str(row['option_count'])+', '+arrays+', '+json.dumps(row['case'])+'},\n'
    args.header.write_text(header+'};\n', encoding='utf-8')
    print(len(rows), 'independent original rows;', len(checks), 'active parameter sets')


if __name__ == '__main__': main()
