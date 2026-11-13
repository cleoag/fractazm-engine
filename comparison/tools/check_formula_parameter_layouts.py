"""Compare independently loaded Pascal/C++ parameter buffers, not formula results.

The instrumented Pascal executable writes PVar[-256:256] after loading a
scene. The C++ probe writes its own 1024-byte allocation, with PVar at 512.
Pointer parameters remain unverified. Supported sphere-map formulas may opt in
to comparing numeric options (including the integer map identifier), without
claiming parity of the referenced image resource or its interpolation.
"""
import argparse
from collections import Counter
import os
from pathlib import Path
import subprocess

from external_validation import ROOT, decode, slots, digest, write_json
import json
from pascal_formula_identity import validate_formula_load


def compare_buffers(pascal, cpp):
    if len(pascal) != 512 or len(cpp) != 1024:
        raise ValueError('Expected Pascal 512-byte and C++ 1024-byte allocations')
    actual = cpp[256:768]
    differences = [i - 256 for i, (a, b) in enumerate(zip(pascal, actual)) if a != b]
    return {'status': 'fail' if differences else 'pass',
            'different_bytes': len(differences), 'pvar_relative_offsets': differences,
            'pascal_sha256': digest(pascal), 'cpp_allocation_sha256': digest(cpp),
            'cpp_comparable_sha256': digest(actual)}


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--cpp', type=Path, required=True)
    ap.add_argument('--pascal', type=Path, required=True)
    ap.add_argument('--manifest', type=Path, required=True)
    ap.add_argument('--output', type=Path, required=True)
    ap.add_argument('--case', action='append', default=[])
    ap.add_argument('--all-cases', action='store_true')
    ap.add_argument('--width', type=int, default=1)
    ap.add_argument('--height', type=int, default=1)
    ap.add_argument('--timeout', type=float, default=60)
    ap.add_argument('--numeric-sphere-map-options', action='store_true',
                    help='Compare numeric buffers for _MapTranslate/ABoxMapSFold; excludes map resource semantics')
    a = ap.parse_args()
    cpp, pascal, output = a.cpp.resolve(), a.pascal.resolve(), a.output.resolve()
    if a.width < 1 or a.height < 1:
        ap.error('Image dimensions must be positive')
    manifest_bytes = a.manifest.read_bytes()
    cases = json.loads(manifest_bytes)['cases']
    if a.case:
        missing = set(a.case) - {c['id'] for c in cases}
        if missing:
            ap.error('Unknown cases: ' + ', '.join(sorted(missing)))
        cases = [c for c in cases if c['id'] in a.case]
    selected, seen = [], set()
    for case in cases:
        for name in case['target_formulas']:
            if a.all_cases or name not in seen:
                selected.append((case, name))
                seen.add(name)
    output.mkdir(parents=True, exist_ok=False)
    report = {'schema_version': 1, 'scope': 'Independent scene loader buffers only; no orbit or image parity claim',
              'cpp_sha256': digest(cpp.read_bytes()), 'pascal_sha256': digest(pascal.read_bytes()),
              'tool_sha256': digest(Path(__file__).read_bytes()),
              'manifest_sha256': digest(manifest_bytes), 'selected_case_count': len(selected),
              'render_dimensions': [a.width, a.height], 'compared_pvar_range': [-256, 256],
              'numeric_sphere_map_options': a.numeric_sphere_map_options,
              'finished': False, 'results': []}
    for index, (case, name) in enumerate(selected):
        directory = output / f'{index:04d}'
        directory.mkdir()
        scene = (ROOT / case['scene']).resolve()
        row = {'case': case['id'], 'name': name, 'scene': str(scene),
               'scene_sha256': digest(scene.read_bytes()), 'artifact_directory': directory.name}
        try:
            if row['scene_sha256'] != case['sha256']:
                raise ValueError('Scene differs from manifest')
            # Generated selector manifests may omit redundant slot metadata.
            # Derive it only after checking the exact scene digest above.
            case_slots = case.get('slots')
            if case_slots is None:
                case_slots = slots(decode(scene))
            slot = next(s['slot'] for s in case_slots if s['name'] == name)
            row['slot'] = slot
            blob = directory / 'cpp.bin'
            command = [str(cpp), name, str(scene), str(blob)]
            row['cpp_command'] = command
            probe_env = dict(os.environ)
            if a.numeric_sphere_map_options and name in ('_MapTranslate', 'ABoxMapSFold', 'SphereHeightMap', 'SphereHMVoid'):
                # These formulas store scalar numbers, not process pointers.
                # The callback profile lets the probe load the buffer safely;
                # empty stdin deliberately executes no formula iterations.
                probe_env['MB3D_POINT_SPHEREMAP_CALLBACK'] = '1'
                row['map_scope'] = 'Numeric options and integer map identifier only; image resource/projection/interpolation excluded'
            if a.numeric_sphere_map_options and name in ('HeightMapIFS','HeightMap2IFS','CylinderHeightMap','CylinderHMVoid','TorusHMVoid'):
                probe_env['MB3D_POINT_HEIGHTMAP_CALLBACK']='1'
                row['map_scope']='Numeric options and integer map identifier only; resource and interpolation excluded'
            result = subprocess.run(command, input=b'', capture_output=True, timeout=a.timeout, env=probe_env)
            (directory / 'cpp.log').write_bytes(result.stdout + result.stderr)
            row['cpp_exit_code'] = result.returncode
            if result.returncode == 4:
                row.update(status='unverified', reason='Map or pointer options require semantic comparison')
            else:
                if result.returncode:
                    raise ValueError(f'C++ probe exit {result.returncode}')
                env = dict(os.environ)
                env['MB3D_FORMULA_BUFFER_DUMP'] = str(directory / 'pascal')
                command = [str(pascal), '--render', str(scene), '--output', str(directory / 'pascal.png'),
                           '--threads', '1', '--width', str(a.width), '--height', str(a.height)]
                row['pascal_command'] = command
                result = subprocess.run(command, cwd=pascal.parent, env=env,
                                        capture_output=True, timeout=a.timeout)
                (directory / 'pascal.log').write_bytes(result.stdout + result.stderr)
                row['pascal_exit_code'] = result.returncode
                if result.returncode:
                    raise ValueError(f'Pascal render exit {result.returncode}')
                row['pascal_formula_load'] = validate_formula_load(
                    result.stdout.decode(errors='replace'), scene, case_slots)
                row.update(compare_buffers((directory / f'pascal_{slot}.bin').read_bytes(), blob.read_bytes()))
        except (OSError, ValueError, StopIteration, subprocess.TimeoutExpired) as error:
            row.update(status='error', error=str(error))
        report['results'].append(row)
        report['counts'] = dict(Counter(r['status'] for r in report['results']))
        write_json(output / 'summary.json', report)
        print(f'{index + 1}/{len(selected)} {name} {row["status"]}', flush=True)
    report['finished'] = True
    write_json(output / 'summary.json', report)
    return int(any(r['status'] in ('fail', 'error') for r in report['results']))


if __name__ == '__main__':
    raise SystemExit(main())
