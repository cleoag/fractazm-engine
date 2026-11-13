"""Render the fixed independent Manowar hybrid fixtures with both applications.

Original-code orbit witnesses qualify the caller separately. These images use
the explicitly identified guarded Pascal application and unchanged image gates.
Every saved slot is verified in its load log, including zero-weight slots.
"""
import argparse
from dataclasses import asdict
import hashlib
import json
import os
from pathlib import Path
import subprocess
import struct
import time

import numpy as np
from image_diff import DEFAULT_THRESHOLDS, load_rgb, compute_metrics, evaluate
from pascal_formula_identity import loaded_formulas
from external_validation import decode
from compatibility_precision import bind_compatible, precision_evidence

ROOT = Path(__file__).resolve().parents[2]

# The selected profile is an explicit coverage contract, not a caller-supplied
# expected count that could silently accept an incomplete manifest.
PROFILES = {
    'hybrid13': {
        'alt_mm', 'alt_ms', 'alt_sm', 'alt_tm', 'alt_mt', 'alt_rm',
        'interp_mm', 'interp_ms', 'interp_sm', 'interp_zero',
        'interp_de_zero', 'interp_4d_zero', 'interp_4dde_zero',
    },
    'producer18': {'alt_ipm', 'alt_mip', 'alt_ipm_negative',
                   'alt_ipm_intmin', 'alt_ipm_intmax', 'alt_ipm_third'}
                  | {'alt_ipm_type' + str(i) for i in range(12)},
}

def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for key in ('cpp', 'pascal', 'manifest', 'output'):
        parser.add_argument('--' + key, type=Path, required=True)
    parser.add_argument('--profile', choices=tuple(PROFILES), default='hybrid13')
    args = parser.parse_args()
    args.output.mkdir(parents=True, exist_ok=False)
    manifest = json.loads(args.manifest.read_bytes())
    ids = [case['id'] for case in manifest['cases']]
    expected = PROFILES[args.profile]
    if len(ids) != len(expected) or set(ids) != expected:
        raise ValueError('Incomplete or duplicate qualified caller scene set')
    env = {k.upper(): v for k, v in os.environ.items()}
    for key in list(env):
        if key.startswith(('MB3D_POINT_', 'MB3D_PARITY_', 'MB3D_HEADLESS_')) or key == 'MB3D_FORMULA_BUFFER_DUMP':
            del env[key]
    report = dict(scope=__doc__, profile=args.profile, finished=False, threads=4, timeout_seconds=90,
                  thresholds=DEFAULT_THRESHOLDS,
                  identities={str(p.resolve()): sha(p) for p in (args.cpp, args.pascal, args.manifest, Path(__file__))}, results=[])
    for case in manifest['cases']:
        scene = ROOT / case['scene']
        if sha(scene) != case['sha256']:
            raise ValueError('Changed scene: ' + str(scene))
        # Interpolation weights occupy the same saved word as integer counts;
        # the application's load log prints that word as an Integer.
        saved = decode(scene)
        directory = args.output / case['id']
        directory.mkdir()
        row = dict(id=case['id'], scene=str(scene), scene_sha256=sha(scene), commands={})
        images = {}
        for language, exe in (('pascal', args.pascal.resolve()), ('cpp', args.cpp.resolve())):
            image = (directory / (language + '.png')).resolve()
            command = [str(exe)] + (['--render'] if language == 'pascal' else []) + [str(scene.resolve()), '--output', str(image), '--threads', '4']
            if language == 'cpp':
                command += ['--deterministic']
                bind_compatible(command,image)
            cwd = exe.parent if language == 'pascal' else ROOT
            start = time.monotonic()
            result = subprocess.run(command, cwd=cwd, env=env, capture_output=True, timeout=90,
                                    creationflags=subprocess.CREATE_NO_WINDOW)
            (directory / (language + '.stdout')).write_bytes(result.stdout)
            (directory / (language + '.stderr')).write_bytes(result.stderr)
            row['commands'][language] = dict(argv=command, cwd=str(cwd), exit_code=result.returncode,
                                            seconds=time.monotonic()-start)
            result.check_returncode()
            if language == 'cpp':
                row['commands'][language].update(precision_evidence(image))
            if not image.exists():
                raise ValueError('Missing completed image')
            if language == 'pascal':
                loaded = loaded_formulas(result.stdout.decode(errors='replace'), scene)
                for i, name in enumerate(case['names']):
                    actual = loaded.get(i)
                    saved_count_word = struct.unpack_from('<i', saved, 848 + 188 * i)[0]
                    if not actual or actual['number'] != 20 or actual['name'] != name or actual['iterations'] != saved_count_word:
                        raise ValueError('Pascal slot load mismatch: ' + str((i, name, actual)))
                    asset = exe.parent / 'M3Formulas' / (name + '.m3f')
                    original = ROOT / 'mb3d' / 'M3Formulas' / (name + '.m3f')
                    if sha(asset) != sha(original):
                        raise ValueError('Pascal runtime formula differs from immutable asset')
                    report['identities'][str(asset)] = sha(asset)
                    report['identities'][str(original)] = sha(original)
                row['loaded_slots'] = loaded
            images[language] = load_rgb(image)
            row[language + '_image_sha256'] = sha(image)
        metrics = compute_metrics(images['pascal'], images['cpp'], int(DEFAULT_THRESHOLDS['diff_ratio_threshold']))
        row['metrics'] = asdict(metrics)
        row['checks'] = evaluate(metrics, DEFAULT_THRESHOLDS)
        row['status'] = 'PASS' if all(row['checks'].values()) else 'FAIL'
        row['exact_rgb'] = metrics.max_abs_delta == 0
        row['reference_rgb_range'] = int(np.max(images['pascal'])) - int(np.min(images['pascal']))
        row['reference_spatially_nonconstant'] = bool(np.any(images['pascal'] != images['pascal'][0, 0]))
        report['results'].append(row)
        (args.output / 'summary.json').write_text(json.dumps(report, indent=2))
        print(case['id'], row['status'], 'exact', row['exact_rgb'], flush=True)
    report['finished'] = True
    report['counts'] = dict(images=len(report['results']), passed=sum(r['status']=='PASS' for r in report['results']),
                           exact=sum(r['exact_rgb'] for r in report['results']),
                           nonconstant=sum(r['reference_spatially_nonconstant'] for r in report['results']))
    report['artifact_sha256'] = {str(p.relative_to(args.output)): sha(p) for p in args.output.rglob('*') if p.is_file() and p.name != 'summary.json'}
    (args.output / 'summary.json').write_text(json.dumps(report, indent=2))
    return int(report['counts']['passed'] != len(expected))

if __name__ == '__main__':
    raise SystemExit(main())
