"""Run the current built-in corpus; optionally add the curated external sample.

Raw outputs stay in comparison/runs. Compact summaries are written only after
all suites complete, so every run records the same checkpoint Git status.
"""
import argparse
import hashlib
import json
import math
from pathlib import Path
import shutil
import subprocess
import sys
from datetime import datetime, timezone

ROOT = Path(__file__).resolve().parents[2]


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def inventory(path):
    return {p.relative_to(path).as_posix(): digest(p)
            for p in sorted(path.rglob('*')) if p.is_file()}


def json_compatible(value):
    """Keep exact-image PSNR infinity explicit in strict JSON artifacts."""
    if isinstance(value, float) and not math.isfinite(value):
        return 'NaN' if math.isnan(value) else ('Infinity' if value > 0 else '-Infinity')
    if isinstance(value, dict):
        return {key: json_compatible(item) for key, item in value.items()}
    if isinstance(value, list):
        return [json_compatible(item) for item in value]
    return value


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--cpp-exe', type=Path, required=True)
    parser.add_argument('--pascal-exe', type=Path, required=True)
    parser.add_argument('--threads', type=int, default=16)
    parser.add_argument('--timeout-ms', type=int, default=240000)
    parser.add_argument('--external-sample', action='store_true')
    args = parser.parse_args()
    stamp = datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%SZ')
    run = ROOT/'comparison/runs'/stamp
    run.mkdir(parents=True)
    suites = [ROOT/'comparison/scenes_builtin', ROOT/'comparison/scenes_phase2']
    suites += sorted((ROOT/'comparison').glob('scenes_phase3_*'))
    if args.external_sample:
        sample = run/'external_sample'
        sample.mkdir()
        for scene in sorted((ROOT/'comparison/scenes').glob('ext_*.m3p')):
            shutil.copyfile(scene, sample/scene.name)
        suites.append(sample)

    binaries = {'cpp': args.cpp_exe.resolve(), 'pascal': args.pascal_exe.resolve()}
    dll = binaries['pascal'].parent/'portable_math.dll'
    if not dll.is_file():
        raise FileNotFoundError(f'Required Pascal math runtime missing: {dll}')
    manifest = {
        'started': stamp, 'threads': args.threads, 'timeout_ms': args.timeout_ms,
        'git_commit': subprocess.check_output(['git','rev-parse','HEAD'],cwd=ROOT,text=True).strip(),
        'git_status': subprocess.check_output(['git','status','--porcelain'],cwd=ROOT,text=True).strip(),
        'binaries': {k:{'path':str(p),'sha256':digest(p)} for k,p in binaries.items()},
        'pascal_source_manifest_sha256': digest(ROOT/'patches/pascal_source_manifest.json'),
        'assets': {}, 'suites': [],
    }
    for label, directory in [('pascal_formulas', binaries['pascal'].parent/'M3Formulas'),
                             ('pascal_maps', binaries['pascal'].parent/'M3Maps'),
                             ('cpp_formulas_reference', ROOT/'mb3d/M3Formulas'),
                             ('cpp_maps', ROOT/'mb3d/M3Maps'),
                             ('cpp_maps_fallback', ROOT/'mb3d/maps')]:
        manifest['assets'][label] = inventory(directory) if directory.exists() else None
    manifest['pascal_math_dll'] = {'path':str(dll),'sha256':digest(dll)}
    (run/'manifest.json').write_text(json.dumps(manifest,indent=2))
    print(f'RUN {run}', flush=True)
    summaries = {}
    for scene_dir in suites:
        name = scene_dir.name
        output = run/name if name != 'external_sample' else run/'external_results'
        command = [sys.executable, str(ROOT/'comparison/tools/sweep.py'),
                   '--scene-dir',str(scene_dir), '--cpp-exe',str(binaries['cpp']),
                   '--pascal-exe',str(binaries['pascal']), '--threads',str(args.threads),
                   '--timeout-ms',str(args.timeout_ms), '--report-dir',str(output)]
        print(f'START {name}', flush=True)
        with (run/(name+'.log')).open('w',encoding='utf-8') as log:
            process = subprocess.run(command,cwd=ROOT,stdout=log,stderr=subprocess.STDOUT)
        paths = list(output.glob('*/summary.json'))
        if len(paths) != 1:
            raise RuntimeError(f'{name}: missing/ambiguous summary; inspect {run/(name+".log")}')
        summary = json.loads(paths[0].read_text())
        if summary.get('ended') is None:
            raise RuntimeError(f'{name}: incomplete run')
        summaries[name] = summary
        manifest['suites'].append({'name':name,'command':command,'exit_code':process.returncode,
                                   'summary':str(paths[0].relative_to(ROOT))})
        print(f"DONE {name}: default={summary['pass_count']}/{summary['scene_count']} "
              f"exact={summary['exact_rgb_count']} errors={summary['error_count']}", flush=True)
        (run/'manifest.json').write_text(json.dumps(manifest,indent=2))
    manifest['ended'] = datetime.now(timezone.utc).isoformat()
    result = ROOT/'comparison/results'/stamp
    result.mkdir(parents=True)
    for name, summary in summaries.items():
        (result/(name+'.json')).write_text(json.dumps(json_compatible(summary),indent=2,allow_nan=False))
    (result/'manifest.json').write_text(json.dumps(manifest,indent=2))
    (run/'manifest.json').write_text(json.dumps(manifest,indent=2))
    print(f'EVIDENCE {result}', flush=True)
    return int(any(s['fail_count'] or s['error_count'] for s in summaries.values()))


if __name__ == '__main__':
    raise SystemExit(main())
