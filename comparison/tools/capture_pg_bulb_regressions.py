"""Capture original PG-bulb powers, angle branches and cancellation regressions.

Expected results come only from immutable M3F CODE through the x86 oracle.
The C++ probe supplies packed parameters; this is not a loader parity proof.
"""
import argparse
import hashlib
import json
import math
import os
from pathlib import Path
import struct

from capture_formula_regressions import literal
from check_formula_scene_points import execute, sha
from external_validation import ROOT, decode, slots, write_json, prepare_external_options


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for key in ('cpp', 'oracle', 'base', 'output', 'header'):
        parser.add_argument('--' + key, type=Path, required=True)
    args = parser.parse_args()
    args.output.mkdir(parents=True, exist_ok=False)
    asset = ROOT / 'mb3d/M3Formulas/PG-bulb.m3f'
    code = args.output / 'original-code.bin'
    code.write_bytes(bytes.fromhex(asset.read_text().split('[CODE]')[1].split('[END]')[0]))
    data = decode(args.base)
    slot = next(s for s in slots(data) if s['name'] == 'PG-bulb')
    prepare_external_options(data, slot)
    offset = 848 + 188 * slot['slot']
    # These two rows exposed cancellation after repeated products at powers10/11.
    inputs = [
        [-1.6881121273612618, 2.497941324534125, -1.890485888505075, -.9964311313664636,
         .20659375880868347, .09669982477566297, .900455238309702],
        [-2.799724630955355, 2.6712770120848965, -.656797130811035, -.15145224208822872,
         -.9688481423606135, .34651719482765286, -.9957979133788728],
    ]
    coordinates = [(-2., -.125, .5, .375), (.125, -.25, 0., 3.),
                   (0., 0., 2., .375), (-0., -0., -.25, -.375),
                   (5e-324, -5e-324, .5, .75), (1e-160, -1e-160, .5, .75),
                   (1e160, -1e160, .5, .75), (1e308, -1e308, .5, .75),
                   (math.inf, .25, .5, .75), (math.nan, .25, .5, .75)]
    inputs += [list(xyz) + [.2, -.3, .4] for xyz in coordinates]
    inputs += [[.125, -.25, .5, .375] + list(j)
               for j in ((0., 0., .4), (-0., -0., .4), (1e-160, -1e-160, .4),
                         (1e160, -1e160, .4))]
    stdin = ''.join(' '.join(format(v, '.17g') for v in row + [1., -2., .375, 3.])
                    + '\n' for row in inputs)
    os.environ['MB3D_POINT_DERIVATIVES'] = '1'
    rows, cases, parameter_checks = [], [], []
    powers = [-2147483648, -2, 0, *range(1, 17), 17, 2147483647]
    for power in powers:
        for angles in (0, 1):
            for wadd in (0., .75):
                options = [wadd, float(power), float(angles)] + [0.] * 13
                scene = bytearray(data)
                struct.pack_into('<16d', scene, offset + 60, *options)
                case_id = f'pg_power_{power}_angles_{angles}_wadd_{int(wadd != 0)}'
                path = args.output / (case_id + '.m3p')
                path.write_bytes(scene)
                blob = args.output / (case_id + '.params')
                execute([args.cpp.resolve(), 'PG-bulb', path.resolve(), blob.resolve(), '--dump-only'], '')
                actual_power = struct.unpack_from('<i', blob.read_bytes(), 512 - 0x14)[0]
                actual_angles = struct.unpack_from('<i', blob.read_bytes(), 512 - 0x18)[0]
                if (actual_power, actual_angles) != (power, angles):
                    raise ValueError('Loader changed PG-bulb selectors')
                parameter_checks.append(dict(case=case_id, power=actual_power, old_angles=actual_angles,
                                             parameter_sha256=sha(blob)))
                expected = execute([args.oracle.resolve(), code.resolve(), blob.resolve()], stdin)
                if len(expected) != len(inputs) or any(len(output) != 20 for output in expected):
                    raise ValueError('Incomplete original oracle output')
                cases.append(dict(id=case_id, target_formulas=['PG-bulb'],
                                  scene=path.resolve().relative_to(ROOT).as_posix(), sha256=sha(path)))
                for xyz, output in zip(inputs, expected):
                    rows.append(dict(case=case_id, name='PG-bulb', option_count=3,
                                     options=options, input=xyz,
                                     output=[format(x, '.17g') for x in output]))
    report = dict(scope=__doc__, cpp_parameter_probe_sha256=sha(args.cpp),
                  oracle_sha256=sha(args.oracle), asset_sha256=sha(asset),
                  original_code_sha256=sha(code), source_scene_sha256=sha(args.base),
                  generator_sha256=sha(Path(__file__)),
                  input_sha256=hashlib.sha256(stdin.encode()).hexdigest(),
                  cases=cases, parameter_checks=parameter_checks, rows=rows)
    write_json(args.output / 'golden.json', report)
    write_json(args.output / 'manifest.json', dict(scope=__doc__, cases=cases))
    header = '// Independent original x86 outputs; capture_pg_bulb_regressions.py.\n'
    header += 'struct FormulaRegression { const char* name; int option_count; double options[16], input[7], output[20]; const char* case_id; };\n'
    header += 'static const FormulaRegression formulaRegressions[] = {\n'
    for row in rows:
        arrays = ', '.join('{' + ', '.join(map(literal, row[key])) + '}' for key in ('options', 'input', 'output'))
        header += '    {"PG-bulb", 3, ' + arrays + ', ' + json.dumps(row['case']) + '},\n'
    args.header.write_text(header + '};\n', encoding='utf-8')
    print(len(rows), 'independent original-code rows;', len(parameter_checks), 'verified parameter sets')


if __name__ == '__main__':
    main()
