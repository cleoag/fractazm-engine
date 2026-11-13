#!/usr/bin/env python3
"""Exercise real profile binding and reject native use as an exact oracle."""
import json
from pathlib import Path
import subprocess
import sys
import tempfile

binary, compatible, native, scene = map(lambda p: str(Path(p).resolve()), sys.argv[1:])
base = [binary, '--scene', scene, '--width', '64', '--height', '48', '--threads', '6']
checks = 0

def run(args, ok=True, error=None):
    global checks
    result = subprocess.run(base + args, text=True, capture_output=True, timeout=90)
    assert (result.returncode == 0) == ok, result.stdout + result.stderr
    if error:
        assert error in result.stderr, result.stderr
    checks += 1
    return result

for mode in ('compatible', 'native'):
    result = run(['--describe', '--precision', mode])
    data = json.loads(result.stdout)
    assert data['precision'] == mode and data['gpu_dispatches'] == 0
run(['--describe', '--precision', 'typo'], False, 'invalid precision')
run(['--precision', 'native'], False, 'exact qualification is compatible-only')
run(['--describe', '--precision', 'native', '--division-mode', 'restoring'], False,
    'restoring software division is unavailable')
with tempfile.TemporaryDirectory(prefix='mb3d-metal-profile-') as temp:
    for mode, library in [('compatible', native), ('native', compatible)]:
        output = Path(temp) / mode
        run(['--precision', mode, '--library', library, '--output', str(output),
             '--benchmark-render', '--full-shading'], False, 'offline library precision/profile mismatch')
        report = json.loads((output / 'report.json').read_text())
        assert report['status'] == 'FAILED_INFRASTRUCTURE'
        assert not list(output.glob('*.png')), 'wrong library must fail before rendering'
print(f'{checks} Metal CLI/profile rejection checks passed')
