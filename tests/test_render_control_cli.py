"""Ordinary CLI work-budget contract; no Pascal or deferred execution paths."""
import os
from pathlib import Path
import subprocess
import sys


def main():
    if len(sys.argv) != 4:
        raise SystemExit('usage: test_render_control_cli.py <renderer> <source-root> <scratch>')
    renderer, source, scratch = (Path(x).resolve() for x in sys.argv[1:])
    scratch.mkdir(parents=True, exist_ok=True)
    scene = source / 'comparison/scenes/test_01_intpow2.m3p'
    env = {k: v for k, v in os.environ.items() if not k.upper().startswith('MB3D_')}
    checks = 0

    def run(*extra):
        return subprocess.run([str(renderer), str(scene), '--width', '32', '--height', '24',
                               '--threads', '2', '--no-postprocess', *map(str, extra)],
                              cwd=scratch, env=env, capture_output=True, timeout=30)

    for value in ('-1', '+1', '', '1x', ' 1', '18446744073709551616'):
        result = run('--work-budget', value)
        assert result.returncode == 1, (value, result.returncode, result.stderr)
        checks += 1
    result = run('--work-budget')
    assert result.returncode == 1, result.stderr
    checks += 1

    outputs = [scratch / name for name in ('image.png', 'depth.png', 'surface.bin', 'diagnostics.txt')]
    args = ['--output', outputs[0], '--zbuffer', outputs[1], '--silight5-dump', outputs[2],
            '--diag-dump', outputs[3]]
    for output in outputs:
        output.unlink(missing_ok=True)
    result = run('--work-budget', '1', *args)
    assert result.returncode == 6, (result.returncode, result.stderr)
    assert all(not output.exists() for output in outputs), 'Exhaustion published output'
    checks += 2

    for output in outputs:
        output.write_bytes(b'existing output must survive failed job\n')
    result = run('--work-budget', '1', *args)
    assert result.returncode == 6, (result.returncode, result.stderr)
    assert all(output.read_bytes() == b'existing output must survive failed job\n' for output in outputs)
    checks += 2

    baseline = scratch / 'baseline.png'
    result = run('--output', baseline)
    assert result.returncode == 0, result.stderr
    checks += 1
    for name, value in (('zero', '0'), ('maximum', '18446744073709551615')):
        output = scratch / (name + '.png')
        result = run('--work-budget', value, '--output', output)
        assert result.returncode == 0, (value, result.stderr)
        assert output.read_bytes() == baseline.read_bytes(), 'Unlimited/ample budget changed image'
        checks += 2
    print(f'CLI work budget: {checks} checks passed; behavioral evidence only.')


if __name__ == '__main__':
    main()
