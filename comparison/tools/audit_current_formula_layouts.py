"""Recheck the active ordinary loader allocations against retained independent dumps.

Raw equality, transfer from a previously audited C++ allocation, and unresolved
differences are separate facts. This tool never promotes unused bytes or an old
formula result to a new parity PASS.
"""
import argparse
import ast
from collections import Counter
import json
import os
from pathlib import Path
import subprocess

from external_validation import ROOT, digest, write_json


def retained_layouts():
    source = ROOT / 'comparison/tools/prioritize_external_fixes.py'
    tree = ast.parse(source.read_text())
    phases = next(ast.literal_eval(n.iter) for n in ast.walk(tree)
                  if isinstance(n, ast.For) and isinstance(n.iter, ast.Tuple)
                  and ast.literal_eval(n.iter)[0] == 'orbit-shapes-layouts-v1')
    rows, files = {}, {str(source): digest(source.read_bytes())}
    for phase in ['parameter-layouts-v1', *phases]:
        path = ROOT / 'comparison/runs/external-complete' / phase / 'summary.json'
        if path.exists():
            files[str(path)] = digest(path.read_bytes())
            for row in json.loads(path.read_bytes())['results']:
                if not row['name'].startswith('JIT'):
                    rows[row['name']] = dict(row, retained_summary=str(path))
    return rows, files


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--cpp', type=Path, required=True)
    ap.add_argument('--output', type=Path, required=True)
    args = ap.parse_args()
    args.output.mkdir(parents=True, exist_ok=False)
    exe = args.cpp.resolve()
    binary_sha = digest(exe.read_bytes())
    layouts, inputs = retained_layouts()
    rows = []
    for i, (name, old) in enumerate(sorted(layouts.items())):
        scene = Path(old['scene'])
        if digest(scene.read_bytes()) != old['scene_sha256']:
            raise ValueError('Changed scene: ' + str(scene))
        directory = args.output / f'{i:04d}'
        directory.mkdir()
        allocation = directory / 'cpp.bin'
        env = dict(os.environ)
        if name in ('_MapTranslate', 'ABoxMapSFold', 'SphereHeightMap', 'SphereHMVoid'):
            env['MB3D_POINT_SPHEREMAP_CALLBACK'] = '1'
        if name in ('HeightMapIFS', 'HeightMap2IFS', 'CylinderHeightMap', 'CylinderHMVoid', 'TorusHMVoid'):
            env['MB3D_POINT_HEIGHTMAP_CALLBACK'] = '1'
        command = [str(exe), name, str(scene), str(allocation.resolve()), '--dump-only']
        run = subprocess.run(command, input=b'', capture_output=True, env=env, timeout=30)
        (directory / 'cpp.log').write_bytes(run.stdout + run.stderr)
        row = dict(formula=name, scene=str(scene), scene_sha256=old['scene_sha256'],
                   retained_summary=old['retained_summary'], raw_historical_status=old['status'],
                   command=command, exit_code=run.returncode, status='unresolved')
        if run.returncode == 0:
            data = allocation.read_bytes()
            if len(data) != 1024:
                raise ValueError('Invalid C++ allocation')
            row['cpp_allocation_sha256'] = digest(data)
            prior = Path(old['retained_summary']).parent / old['artifact_directory']
            old_cpp = prior / 'cpp.bin'
            if old_cpp.exists():
                before = old_cpp.read_bytes()
                if old.get('cpp_allocation_sha256') and digest(before) != old['cpp_allocation_sha256']:
                    raise ValueError('Changed retained C++ allocation: ' + str(old_cpp))
                row['retained_cpp_sha256'] = digest(before)
                row['same_as_retained_cpp'] = data == before
                row['changed_cpp_offsets'] = [k-512 for k, (a,b) in enumerate(zip(data, before)) if a != b]
            pascal = prior / f"pascal_{old['slot']}.bin"
            if pascal.exists():
                reference = pascal.read_bytes()
                if len(reference) != 512:
                    raise ValueError('Invalid Pascal allocation')
                if old.get('pascal_sha256') and digest(reference) != old['pascal_sha256']:
                    raise ValueError('Changed retained Pascal allocation: ' + str(pascal))
                row['pascal_buffer'] = str(pascal)
                row['pascal_buffer_sha256'] = digest(reference)
                row['pvar_relative_differences'] = [k-256 for k,(a,b) in enumerate(zip(reference, data[256:768])) if a != b]
                row['status'] = 'raw_exact' if not row['pvar_relative_differences'] else 'raw_difference'
            else:
                row['status'] = 'missing_independent_buffer'
        else:
            row['status'] = 'probe_error'
        rows.append(row)
    if digest(exe.read_bytes()) != binary_sha:
        raise ValueError('Binary changed during audit')
    report = dict(scope=__doc__, cpp_sha256=binary_sha, input_sha256=inputs,
                  tool_sha256=digest(Path(__file__).read_bytes()), results=rows,
                  counts=dict(Counter(r['status'] for r in rows)),
                  allocation_transfer_counts=dict(Counter(str(r.get('same_as_retained_cpp')) for r in rows)),
                  finished=True)
    write_json(args.output / 'summary.json', report)
    print(json.dumps({k:report[k] for k in ('counts', 'allocation_transfer_counts')}))
    return int(any(r['status'] == 'probe_error' for r in rows))


if __name__ == '__main__':
    raise SystemExit(main())
