#!/usr/bin/env python3
"""Bounded host-only planning check against a retained package's 77-scene admission.

Compiles changed host translation units. Links the diagnostic admission tool to
retained host archives; this is not a clean build or GPU/render qualification.
"""
import argparse
import json
import shlex
import sys
from collections import Counter
from pathlib import Path

from run_m3parameter_gpu import ROOT, invoke, sha


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--baseline', type=Path, required=True)
    parser.add_argument('--admission', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    base, out = args.baseline.resolve(), args.output.resolve()
    out.mkdir(parents=True, exist_ok=False)
    build = base / 'build'
    db = json.loads((build / 'compile_commands.json').read_text())

    def run(argv, name, cwd=ROOT, timeout=120):
        result = invoke(argv, out / name, cwd, timeout)
        if result['exit_code'] != 0 or result['guard']:
            raise RuntimeError(f'{name} failed: {out / name / "process.log"}')

    test = out / 'planning_test'
    run(['/usr/bin/clang++', '-std=c++20', '-O2', '-Wall', '-Wextra', '-Werror',
         '-I' + str(ROOT), '-I' + str(ROOT / 'src'),
         ROOT / 'tests/test_native_postprocess_plan.cpp', '-o', test], 'compile-test')
    run([test], 'test')
    objects = {}
    for name in ['dof_dispatch.mm', 'zbuffer_dispatch.mm', 'general_renderer.mm', 'phase1_admission.cpp']:
        entry = next(x for x in db if Path(x['file']).name == name)
        command = shlex.split(entry['command'])
        target = out / (name + '.o')
        command[command.index('-o') + 1] = str(target)
        command += ['-MD', '-MF', str(out / (name + '.d'))]
        run(command, 'compile-' + name)
        objects[name] = target
    link = shlex.split((build / 'CMakeFiles/mb3d_phase1_admission.dir/link.txt').read_text())
    link = link[link.index('--') + 1:]
    link = [str(objects['phase1_admission.cpp']) if x.endswith('phase1_admission.cpp.o') else x for x in link]
    exe = out / 'admission_probe'
    link[link.index('-o') + 1] = str(exe)
    run(link, 'link-admission-retained-archives', build)
    baseline = json.loads(args.admission.read_text())
    by_id = {row['id']: row for row in baseline['rows']}
    contract = json.loads((base / 'contract.json').read_text())
    rows = []
    corrected_budgets = 0
    for scene in contract['scenes']:
        folder = out / ('scene-' + scene['id'])
        run([exe, base / scene['frozen'], base / 'runtime', folder / 'admission.json'],
            folder.name, base / 'runtime')
        observed = json.loads((folder / 'admission.json').read_text())
        previous = by_id[scene['id']]
        if observed['status'] != previous['status']:
            raise RuntimeError(f'Admission changed for {scene["id"]}')
        old_preparation = previous['preparation']
        if observed['stages'] != old_preparation['stages']:
            raise RuntimeError(f'Stage decisions changed for {scene["id"]}')
        expected = dict(old_preparation.get('prepared', {}))
        if expected.get('dof_bytes', 0):
            # Retained candidate21 admission omitted the 64x48 inverse ranks
            # and eight spatial records. No other prepared value should change.
            expected['dof_bytes'] += 64 * 48 * 4 + 8 * 16
            corrected_budgets += 1
        if observed.get('prepared', {}) != expected:
            raise RuntimeError(f'Unexpected preparation change for {scene["id"]}')
        if observed['render_executed'] or observed['gpu_executed']:
            raise RuntimeError('Unexpected rendering')
        rows.append({'id': scene['id'], 'status': observed['status']})
    result = {'status': 'PASS_HOST_PLANNING_ONLY', 'gpu_executed': False,
              'fresh_full_build': False, 'retained_archives': str(build),
              'corrected_dof_budgets': corrected_budgets,
              'counts': dict(Counter(row['status'] for row in rows)), 'rows': rows,
              'test_output': (out / 'test/process.log').read_text().strip(),
              'changed_sources': {str(ROOT / x): sha(ROOT / x) for x in [
                  'src/render/native_metal/postprocess_plan.h',
                  'src/render/native_metal/dof_dispatch.h', 'src/render/native_metal/dof_dispatch.mm',
                  'src/render/native_metal/zbuffer_dispatch.h', 'src/render/native_metal/zbuffer_dispatch.mm',
                  'comparison/tools/phase1_admission.cpp', 'tests/test_native_postprocess_plan.cpp']}}
    (out / 'result.json').write_text(json.dumps(result, indent=2) + '\n')
    print(result['status'], result['counts'], result['test_output'], flush=True)
    return 0


if __name__ == '__main__':
    sys.exit(main())
