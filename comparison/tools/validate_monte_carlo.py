"""Serial fixed-sample Pascal/C++ Monte Carlo comparisons (not adaptive GUI parity).

Preparation enumerates all seven MCoptions bits, with DOF/reflections enabled,
plus existing geometry/effect fixtures. Failures remain in the report. Images
are compared using the unchanged regular parity gates, never statistical slack.
"""
import argparse
from collections import Counter
from dataclasses import asdict
import json
import os
from pathlib import Path
import subprocess
import time
import numpy as np

from external_validation import ROOT, decode, digest, slots, write_json
from image_diff import DEFAULT_THRESHOLDS, load_rgb, compute_metrics, evaluate
from sweep import check_stale_processes


def prepare(output):
    import struct
    output.mkdir(parents=True, exist_ok=True)
    source = ROOT / 'comparison/scenes/test_01_intpow2.m3p'
    cases = []

    def add(name, source, changes, samples=8):
        data = decode(source)
        struct.pack_into('<ii', data, 4, 32, 24)
        for offset, fmt, value in changes:
            struct.pack_into('<' + fmt, data, offset, value)
        path = output / (name + '.m3p')
        path.write_bytes(data)
        cases.append({'id': name, 'scene': str(path.resolve()),
                      'sha256': digest(data), 'source': str(source.relative_to(ROOT)),
                      'source_sha256': digest(source.read_bytes()), 'samples': samples,
                      'changes': changes, 'slots': slots(data)})

    for bits in range(128):
        add(f'mc_options_{bits:03}', source,
            [(124, 'B', bits), (181, 'B', 1), (164, 'f', .5),
             (172, 'f', .03), (336, 'B', 1), (332, 'f', .5),
             (337, 'B', 2), (125, 'B', 8), (186, 'B', 3)])
    for samples in [1, 4, 16]:
        add(f'mc_base_s{samples}', source, [], samples)
    for folder in [ROOT / 'comparison/scenes_builtin', ROOT / 'comparison/scenes_phase2',
                   *sorted((ROOT / 'comparison').glob('scenes_phase3_*'))]:
        for scene in sorted(folder.glob('*.m3p')):
            add('mc_' + folder.name + '_' + scene.stem, scene, [], 4)
    write_json(output / 'manifest.json', {'scope': __doc__, 'cases': cases,
        'unverified': ['adaptive CalcN scheduling', 'packed progressive resume',
                       'GUI noise filtering', 'unbounded sample counts and parameters']})
    print(f'Prepared {len(cases)} MC cases', flush=True)


def load_linear(path, width, height):
    with path.open(encoding='ascii') as stream:
        header = stream.readline().split()
        if len(header) not in (2, 3) or [int(x) for x in header[:2]] != [width, height]:
            raise ValueError('Linear buffer dimensions differ from PNG')
        values = np.loadtxt(stream)
    if values.shape != (width * height, 3) or not np.isfinite(values).all():
        raise ValueError('Invalid or nonfinite linear buffer')
    return values


def source_provenance():
    def git(*args):
        return subprocess.check_output(['git', *args], cwd=ROOT, text=True).strip()
    sources = {}
    for folder in ['src', 'external_formulas', 'scripts']:
        for path in sorted((ROOT / folder).rglob('*')):
            if path.is_file() and path.suffix in {'.cpp', '.h', '.inc', '.py', '.asm'}:
                sources[str(path.relative_to(ROOT))] = digest(path.read_bytes())
    sources['CMakeLists.txt'] = digest((ROOT / 'CMakeLists.txt').read_bytes())
    return {'git_commit': git('rev-parse', 'HEAD'), 'git_status': git('status', '--porcelain'),
            'sources': sources, 'threshold_policy': 'unchanged image_diff defaults; linear metrics diagnostic only'}


def load_pascal_reference(directory):
    """Resolve replay chains, retaining the top report's recorded artifact hashes."""
    directory = directory.resolve()
    seen, evidence = set(), []
    top = None
    while True:
        if directory in seen or len(seen) >= 16:
            raise ValueError('Cyclic or excessive Pascal reference chain')
        seen.add(directory)
        path = directory / 'summary.json'
        raw = path.read_bytes()
        current = json.loads(raw)
        evidence.append({'path': str(path), 'sha256': digest(raw)})
        if top is None:
            top = current
        elif current['binaries']['pascal']['sha256'] != top['binaries']['pascal']['sha256']:
            raise ValueError('Pascal binary differs in reference chain')
        parent = current.get('pascal_reference')
        if not parent:
            return top, directory, evidence
        directory = Path(parent).resolve()


def checked_pascal_paths(directory, case, recorded):
    if recorded.get('scene_sha256') != case['sha256'] or recorded.get('samples') != case['samples']:
        raise ValueError('Missing or mismatched Pascal reference case')
    path = directory / 'pascal' / (case['id'] + '.png')
    linear = path.with_suffix('.linear.txt')
    if not recorded.get('pascal_image_sha256') or not recorded.get('pascal_linear_sha256'):
        raise ValueError('Pascal reference has no recorded artifact hashes; use a hash-bearing replay report')
    if digest(path.read_bytes()) != recorded['pascal_image_sha256'] or digest(linear.read_bytes()) != recorded['pascal_linear_sha256']:
        raise ValueError('Pascal reference image or linear data hash changed')
    return path, linear


def run(args):
    if not args.pascal_reference and check_stale_processes():
        raise RuntimeError('A render process is running; serialize Pascal reference jobs')
    manifest = json.loads(args.manifest.read_text())
    cases = manifest['cases']
    if args.select:
        cases = [c for c in cases if args.select in c['id']]
    output = args.output.resolve()
    output.mkdir(parents=True, exist_ok=False)
    exes = {'pascal': args.pascal_exe.resolve(), 'cpp': args.cpp_exe.resolve()}
    report = {'scope': __doc__, 'complete': False, 'manifest_sha256': digest(args.manifest.read_bytes()),
              'binaries': {k: {'path': str(p), 'sha256': digest(p.read_bytes())} for k, p in exes.items()},
              'threads': args.threads, 'thresholds': DEFAULT_THRESHOLDS, 'results': [],
              'provenance': source_provenance(), 'unverified': manifest.get('unverified', [])}
    if args.pascal_reference:
        reference, reference_directory, reference_evidence = load_pascal_reference(args.pascal_reference)
        report['pascal_reference_evidence'] = reference_evidence
        report['pascal_reference'] = str(args.pascal_reference.resolve())
        report['binaries']['pascal'] = reference['binaries']['pascal']
        reference_cases = {r['case']: r for r in reference['results']}
    for case in cases:
        item = {'case': case['id'], 'scene_sha256': case['sha256'], 'samples': case['samples']}
        images, linear_images = {}, {}
        try:
            scene = Path(case['scene']).resolve()
            if digest(scene.read_bytes()) != case['sha256']:
                raise ValueError('Scene bytes changed since preparation')
            for language, exe in exes.items():
                if language == 'pascal' and args.pascal_reference:
                    old = reference_cases.get(case['id'], {})
                    path, linear = checked_pascal_paths(reference_directory, case, old)
                    images[language] = load_rgb(path)
                    item['pascal_image_sha256'] = digest(path.read_bytes())
                    linear_images[language] = load_linear(path.with_suffix('.linear.txt'), images[language].shape[1], images[language].shape[0])
                    item['pascal_linear_sha256'] = digest(path.with_suffix('.linear.txt').read_bytes())
                    continue
                path = output / language / (case['id'] + '.png')
                path.parent.mkdir(exist_ok=True)
                linear = path.with_suffix('.linear.txt')
                env = {k: v for k, v in os.environ.items()
                       if not k.startswith(('MB3D_MC_TRACE', 'MB3D_HEADLESS_MC_'))}
                if language == 'pascal':
                    env['MB3D_HEADLESS_MC_SAMPLES'] = str(case['samples'])
                    env['MB3D_HEADLESS_MC_DUMP'] = str(linear)
                    command = [str(exe), '--render', str(scene), '--output', str(path), '--threads', str(args.threads)]
                else:
                    command = [str(exe), str(scene), '--output', str(path), '--threads', str(args.threads),
                               '--deterministic', '--mc-samples', str(case['samples']), '--mc-linear-dump', str(linear)]
                before = time.monotonic()
                proc = subprocess.run(command, cwd=exe.parent if language == 'pascal' else ROOT,
                                      env=env, capture_output=True, timeout=args.timeout)
                item[language + '_seconds'] = time.monotonic() - before
                item[language + '_command'] = command
                path.with_suffix('.log').write_bytes(proc.stdout + b'\n' + proc.stderr)
                if proc.returncode or not path.exists() or not linear.exists():
                    raise RuntimeError(f'{language}: exit={proc.returncode}; image/linear output required; see log')
                images[language] = load_rgb(path)
                item[language + '_image_sha256'] = digest(path.read_bytes())
                linear_images[language] = load_linear(linear, images[language].shape[1], images[language].shape[0])
                item[language + '_linear_sha256'] = digest(linear.read_bytes())
            delta = np.abs(linear_images['pascal'] - linear_images['cpp'])
            worst = np.unravel_index(int(delta.argmax()), delta.shape)
            item['linear_metrics'] = {'max_abs_delta': float(delta.max()), 'mean_abs_delta': float(delta.mean()),
                'rmse': float(np.sqrt(np.mean(delta * delta))), 'p99_abs_delta': float(np.percentile(delta, 99)),
                'worst_pixel': [int(worst[0] % images['cpp'].shape[1]), int(worst[0] // images['cpp'].shape[1])],
                'worst_channel': int(worst[1])}
            metrics = compute_metrics(images['pascal'], images['cpp'], DEFAULT_THRESHOLDS['diff_ratio_threshold'])
            checks = evaluate(metrics, DEFAULT_THRESHOLDS)
            item.update(status='pass' if all(checks.values()) else 'fail', metrics=asdict(metrics),
                        exact_rgb=metrics.max_abs_delta == 0, checks=checks)
        except (OSError, RuntimeError, ValueError, subprocess.TimeoutExpired) as exc:
            item.update(status='error', error=str(exc))
        report['results'].append(item)
        report['counts'] = dict(Counter(r['status'] for r in report['results']))
        write_json(output / 'summary.json', report)
        print(f"{len(report['results'])}/{len(cases)} {case['id']} {item['status']}", flush=True)
    report['finished'] = True
    write_json(output / 'summary.json', report)
    return int(any(r['status'] != 'pass' for r in report['results']))


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    sub = parser.add_subparsers(dest='command', required=True)
    prep = sub.add_parser('prepare'); prep.add_argument('--output', type=Path, required=True)
    check = sub.add_parser('run')
    for flag in ['manifest', 'output', 'cpp-exe', 'pascal-exe']:
        check.add_argument('--' + flag, type=Path, required=True)
    check.add_argument('--threads', type=int, default=1)
    check.add_argument('--timeout', type=int, default=30)
    check.add_argument('--select')
    check.add_argument('--pascal-reference', type=Path, help='Reuse hash-matched fixed-sample reference outputs')
    args = parser.parse_args()
    if args.command == 'prepare': prepare(args.output)
    else: raise SystemExit(run(args))
