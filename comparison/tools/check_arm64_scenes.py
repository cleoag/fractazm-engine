"""Bounded native scene smoke/throughput checks, never a Pascal parity claim."""
import argparse
import concurrent.futures
import hashlib
import json
import os
from pathlib import Path
import platform
import struct
import subprocess
import sys
import time

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'scripts'))
from native_evidence import source_identity


def sha(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def native_identity(renderer):
    if platform.system() != 'Darwin' or platform.machine() != 'arm64':
        raise RuntimeError('Requires native Apple Silicon Python')
    translated = subprocess.run(['sysctl', '-in', 'sysctl.proc_translated'], capture_output=True, text=True)
    if translated.stdout.strip() == '1':
        raise RuntimeError('Translated execution is excluded')
    arch = subprocess.check_output(['lipo', '-archs', str(renderer)], text=True).strip()
    if arch != 'arm64':
        raise RuntimeError('Renderer must contain only arm64: ' + arch)
    cache = renderer.parent / 'CMakeCache.txt'
    configuration = {}
    if cache.exists():
        keys = {'CMAKE_BUILD_TYPE', 'CMAKE_CXX_COMPILER', 'CMAKE_CXX_FLAGS',
                'CMAKE_INTERPROCEDURAL_OPTIMIZATION', 'MB3D_ENABLE_PARITY_TRACE', 'MB3D_DEFAULT_PRECISION'}
        for line in cache.read_text().splitlines():
            if ':' in line and '=' in line:
                key = line.split(':', 1)[0]
                if key in keys: configuration[key] = line.split('=', 1)[1]
    return {'machine': platform.machine(), 'os': platform.platform(), 'binary_arch': arch,
            'configuration': configuration,
            'cpu': subprocess.check_output(['sysctl', '-n', 'machdep.cpu.brand_string'], text=True).strip(),
            'renderer': str(renderer), 'renderer_sha256': sha(renderer),
            **source_identity()}


def inspect_bmp(path):
    data = path.read_bytes()
    if data[:2] != b'BM' or len(data) < 54:
        raise ValueError('Missing BMP header')
    offset = struct.unpack_from('<I', data, 10)[0]
    width, height = struct.unpack_from('<ii', data, 18)
    bits = struct.unpack_from('<H', data, 28)[0]
    compression = struct.unpack_from('<I', data, 30)[0]
    if width < 1 or height == 0 or bits != 24 or compression != 0:
        raise ValueError('Expected uncompressed RGB24 BMP')
    stride = (width * 3 + 3) & ~3
    if len(data) < offset + stride * abs(height):
        raise ValueError('Truncated BMP')
    pixels = b''.join(data[offset+y*stride:offset+y*stride+width*3] for y in range(abs(height)))
    return {'width': width, 'height': abs(height), 'rgb_sha256': hashlib.sha256(pixels).hexdigest(),
            'distinct_colors': len({pixels[i:i+3] for i in range(0, len(pixels), 3)})}


def render(renderer, scene, output, width, height, threads, budget, timeout, precision='compatible'):
    command = [str(renderer), str(scene), '--width', str(width), '--height', str(height),
               '--threads', str(threads), '--deterministic', '--precision', precision,
               '--format', 'bmp', '--output', str(output)]
    if budget is not None:
        command += ['--work-budget', str(budget)]
    env = {k: v for k, v in os.environ.items() if not k.upper().startswith('MB3D_')}
    start = time.perf_counter()
    try:
        result = subprocess.run(command, capture_output=True, timeout=timeout, env=env, cwd=ROOT)
        status = {0: 'rendered', 3: 'rejected', 6: 'budget_exhausted'}.get(result.returncode, 'error')
        log = result.stdout + result.stderr
        row = {'status': status, 'returncode': result.returncode}
    except subprocess.TimeoutExpired as error:
        row = {'status': 'timeout', 'returncode': None}
        log = (error.stdout or b'') + (error.stderr or b'')
    row.update({'scene': str(scene.relative_to(ROOT)), 'scene_sha256': sha(scene),
                'command': command, 'seconds': time.perf_counter()-start})
    output.with_suffix('.log').write_bytes(log)
    if row['status'] == 'rendered':
        try:
            row.update(inspect_bmp(output))
        except (OSError, ValueError) as error:
            row.update(status='invalid_image', error=str(error))
    return row


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--renderer', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--workers', type=int, default=4)
    parser.add_argument('--threads', type=int, default=2)
    parser.add_argument('--width', type=int, default=64)
    parser.add_argument('--height', type=int, default=48)
    parser.add_argument('--timeout', type=float, default=30)
    parser.add_argument('--work-budget', type=int, default=20000000)
    parser.add_argument('--precision', choices=('compatible', 'native'), default='compatible')
    args = parser.parse_args()
    if min(args.workers, args.threads, args.width, args.height, args.timeout) <= 0 or args.work_budget < 0:
        parser.error('Workers, threads, dimensions and timeout must be positive; budget must be nonnegative')
    renderer = args.renderer.resolve()
    identity = native_identity(renderer)
    args.output.mkdir(parents=True, exist_ok=False)
    scenes = sorted((ROOT/'comparison/scenes').glob('extall_*.m3p'))
    if any('JIT' in p.name or 'MonteCarlo' in p.name for p in scenes):
        raise RuntimeError('Deferred input in ordinary scene selection')
    report = {'scope': 'Ordinary external-formula scenes; completion/valid RGB only, no numerical parity claim',
              'identity': identity, 'precision': args.precision, 'status': 'RUNNING', 'listed': len(scenes), 'results': []}
    if not scenes: raise RuntimeError('No scenes selected')
    def run(scene):
        return render(renderer, scene, args.output/(scene.stem+'.bmp'), args.width, args.height,
                      args.threads, args.work_budget, args.timeout, args.precision)
    with concurrent.futures.ThreadPoolExecutor(max_workers=args.workers) as pool:
        for row in pool.map(run, scenes):
            report['results'].append(row)
            (args.output/'summary.json.tmp').write_text(json.dumps(report, indent=2)+'\n')
            (args.output/'summary.json.tmp').replace(args.output/'summary.json')
    print(json.dumps({status: sum(r['status']==status for r in report['results'])
                      for status in sorted({r['status'] for r in report['results']})}))
    failed = any(r['status'] != 'rendered' for r in report['results'])
    report['status'] = 'FAIL' if failed else 'PASS_RENDER_ONLY'
    (args.output/'summary.json').write_text(json.dumps(report, indent=2)+'\n')
    return int(failed)


if __name__ == '__main__':
    raise SystemExit(main())
