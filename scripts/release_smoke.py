"""Three small real renders per backend for the library release check.

Uses tracked scenes/maps and the existing stdlib PNG reader. This is a smoke
check, not full-corpus numerical qualification.
"""
import argparse
from contextlib import contextmanager
import hashlib
import json
import os
from pathlib import Path
import shutil
import subprocess
import sys
import time

ROOT = Path(__file__).resolve().parents[1]
SCENES = (
    ('builtin', 'comparison/scenes/test_01_intpow2.m3p'),
    ('external', 'comparison/scenes/ext_01_bristorbrot.m3p'),
    ('heightmap', 'comparison/scenes_validation/heightmap_real_asset.m3p'),
)


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def check_disk(path):
    if shutil.disk_usage(path).free < 12 * 1024**3:
        raise RuntimeError('Less than 12 GiB free; release check stopped')


@contextmanager
def gpu_slot(enabled):
    if not enabled:
        yield
        return
    import fcntl
    path = ROOT / 'comparison/runs/.gpu-execution.lock'
    path.parent.mkdir(parents=True, exist_ok=True)
    with path.open('a+b') as lock:
        fcntl.flock(lock, fcntl.LOCK_EX)
        try:
            yield
        finally:
            fcntl.flock(lock, fcntl.LOCK_UN)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--build', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--backend', choices=('cpu', 'metal', 'all'), default='cpu')
    args = parser.parse_args()
    if not __debug__:
        parser.error('Run Python without -O: the existing PNG checks use assertions')
    build, out = args.build.resolve(), args.output.resolve()
    renderer = build / ('Release/mb3d_render.exe' if os.name == 'nt' else 'mb3d_render')
    library = build / 'metal_native_scene.metallib'
    backends = ('cpu', 'metal') if args.backend == 'all' else (args.backend,)
    if not renderer.is_file() or ('metal' in backends and not library.is_file()):
        parser.error('Build mb3d_release_checks for the requested backend first')
    if out == ROOT or out.is_relative_to(ROOT / 'mb3d'):
        parser.error('Output must be a new directory outside immutable mb3d/')
    out.mkdir(parents=True, exist_ok=False)
    sys.path.insert(0, str(ROOT / 'tests'))
    from test_render_backend_cli import rgb
    report = {'status': 'RUNNING', 'scope': 'release smoke, 64x48, native precision',
              'renderer_sha256': sha(renderer), 'images': []}
    if 'metal' in backends:
        report['library_sha256'] = sha(library)
    env = {k: v for k, v in os.environ.items()
           if not k.upper().startswith(('MB3D_', 'DYLD_'))}

    def save():
        (out / 'smoke.json').write_text(json.dumps(report, indent=2) + '\n')

    try:
        check_disk(out)
        maps = out / 'M3Maps'
        maps.mkdir()
        shutil.copyfile(ROOT / 'comparison/fixtures/M3Maps/9001.bmp', maps / '9001.bmp')
        for backend in backends:
            for name, relative in SCENES:
                folder = out / (backend + '-' + name)
                folder.mkdir()
                image, profile = folder / 'image.png', folder / 'profile.json'
                argv = [str(renderer), str(ROOT / relative), '--backend', backend,
                        '--precision', 'native', '--width', '64', '--height', '48',
                        '--threads', '1', '--output', str(image), '--profile-json', str(profile)]
                if backend == 'metal':
                    argv += ['--metal-library', str(library)]
                item = {'name': name, 'backend': backend, 'scene': relative,
                        'scene_sha256': sha(ROOT / relative), 'argv': argv, 'status': 'RUNNING'}
                report['images'].append(item)
                save()
                with gpu_slot(backend == 'metal'), (folder / 'process.log').open('wb') as log:
                    started = time.monotonic()
                    process = subprocess.Popen(argv, cwd=out, env=env, stdout=log, stderr=subprocess.STDOUT)
                    try:
                        while process.poll() is None:
                            check_disk(out)
                            if time.monotonic() - started > 180:
                                raise RuntimeError('Smoke frame exceeded 180 seconds; no repeat launched')
                            try:
                                process.wait(timeout=0.25)
                            except subprocess.TimeoutExpired:
                                pass
                    finally:
                        if process.poll() is None:
                            process.kill()
                            process.wait()
                    item.update(seconds=time.monotonic() - started, exit_code=process.returncode)
                    if process.returncode:
                        raise RuntimeError(f'{backend}/{name} failed; see {folder / "process.log"}')
                p = json.loads(profile.read_text())
                if not (p.get('status') == 'complete' and p.get('render_status') == 'completed'
                        and p.get('exit_code') == 0 and p.get('error') == ''
                        and p.get('backend') == p.get('backend_requested') == backend
                        and p.get('backend_fallback') is False and p.get('precision') == 'native'
                        and (p.get('width'), p.get('height')) == (64, 48)):
                    raise RuntimeError(f'{backend}/{name}: incomplete render or unexpected backend')
                if backend == 'metal':
                    execution = p.get('metal_execution', {})
                    if execution.get('validated') is not True or any(
                            execution.get(k) != 0 for k in ('faults', 'nonfinite', 'work_exhausted')):
                        raise RuntimeError(f'{name}: invalid GPU result')
                    item['pipeline_archive'] = execution.get('pipeline_archive')
                pixels = rgb(image)
                if len(set(zip(pixels[::3], pixels[1::3], pixels[2::3]))) < 2:
                    raise RuntimeError(f'{backend}/{name}: constant image')
                item.update(status='PASS', image=str(image), png_sha256=sha(image))
                save()
                print(f'{len(report["images"])}/{3 * len(backends)} {backend}/{name} PASS', flush=True)
        if sha(renderer) != report['renderer_sha256'] or (
                'metal' in backends and sha(library) != report['library_sha256']):
            raise RuntimeError('Build changed during release smoke')
        report['status'] = 'PASS_RELEASE_SMOKE'
        return 0
    except Exception as error:
        report.update(status='FAIL', error=str(error))
        print(error, file=sys.stderr)
        return 1
    finally:
        save()


if __name__ == '__main__':
    raise SystemExit(main())
