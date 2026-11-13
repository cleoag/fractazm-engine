"""Stage profiling must preserve pixels and expose failure without inventing stages."""
import json
import math
import os
from pathlib import Path
import subprocess
import sys
import tempfile


def main():
    if len(sys.argv) != 4:
        raise SystemExit('usage: test_render_profile_cli.py <renderer> <source-root> <scratch>')
    renderer, source, parent = (Path(x).resolve() for x in sys.argv[1:])
    parent.mkdir(parents=True, exist_ok=True)
    scene = source / 'comparison/scenes/test_01_intpow2.m3p'
    env = {k: v for k, v in os.environ.items() if not k.upper().startswith('MB3D_')}
    with tempfile.TemporaryDirectory(dir=parent, prefix='profile-') as temporary:
        scratch = Path(temporary)
        checks = 0

        def run(*extra, input_path=scene):
            return subprocess.run([str(renderer), str(input_path), '--width', '32', '--height', '24',
                                   '--threads', '2', '--deterministic', *map(str, extra)],
                                  cwd=scratch, env=env, capture_output=True, timeout=30)

        def read(path, status, code):
            nonlocal checks
            report = json.loads(path.read_text())
            assert report['schema'] == 'mb3d.cpu-stage-profile.v1'
            assert report['schema_version'] == 1
            assert report['precision'] in ('compatible', 'native')
            assert report['status'] == status and report['exit_code'] == code, report
            for field in ('render_total_seconds', 'command_total_seconds'):
                assert math.isfinite(report[field]) and report[field] >= 0
            for name, timing in report['stages'].items():
                assert timing['state'] in ('not_run', 'completed', 'interrupted')
                assert math.isfinite(timing['seconds']) and timing['seconds'] >= 0
                if timing['state'] == 'not_run':
                    assert timing['seconds'] == 0 and name not in report['stages_seconds']
                else:
                    assert timing['seconds'] == report['stages_seconds'][name]
            assert sum(report['stages_seconds'].values()) <= report['command_total_seconds']
            checks += 1
            return report

        plain, measured = scratch / 'plain.bmp', scratch / 'measured.bmp'
        result = run('--format', 'bmp', '--output', plain)
        assert result.returncode == 0, result.stderr
        path = scratch / 'complete.json'
        result = run('--format', 'bmp', '--output', measured, '--profile-json', path)
        assert result.returncode == 0, result.stderr
        assert plain.read_bytes() == measured.read_bytes(), 'Profiling changed pixels'
        checks += 2
        report = read(path, 'complete', 0)
        assert (report['width'], report['height'], report['workers']) == (32, 24, 2)
        assert report['render_status'] == 'completed'
        assert all(report['stages'][name]['state'] == 'completed'
                   for name in ('load', 'preparation', 'primary', 'color', 'final_surface', 'save'))
        checks += 3

        # Compile-selected default and both explicit overrides remain observable.
        default_mode = report['precision']
        for mode in ('compatible', 'native'):
            image = scratch / (mode + '.bmp')
            sidecar = scratch / (mode + '.json')
            result = run('--precision', mode, '--format', 'bmp', '--output', image,
                         '--profile-json', sidecar)
            assert result.returncode == 0, result.stderr
            assert read(sidecar, 'complete', 0)['precision'] == mode
            if mode == default_mode:
                assert image.read_bytes() == plain.read_bytes()
            limited_mode = scratch / (mode + '-limited.json')
            absent = scratch / (mode + '-limited.png')
            result = run('--precision', mode, '--work-budget', '1', '--output', absent,
                         '--profile-json', limited_mode)
            assert result.returncode == 6 and not absent.exists()
            assert read(limited_mode, 'work_exhausted', 6)['precision'] == mode
            checks += 4
        invalid_output = scratch / 'invalid-mode.png'
        result = run('--precision', 'typo', '--output', invalid_output)
        assert result.returncode == 1 and not invalid_output.exists()
        checks += 1

        # Existing profile, scene and output paths must never be overwritten by JSON.
        frozen = path.read_bytes()
        result = run('--output', measured, '--profile-json', path)
        assert result.returncode == 4 and path.read_bytes() == frozen
        original = scene.read_bytes()
        result = run('--profile-json', scene)
        assert result.returncode == 4 and scene.read_bytes() == original
        conflict = scratch / 'conflict.png'
        result = run('--output', conflict, '--profile-json', conflict)
        assert result.returncode == 1 and not conflict.exists()
        absent_input = scratch / 'absent-input.m3p'
        result = run('--profile-json', absent_input, input_path=absent_input)
        assert result.returncode == 1 and not absent_input.exists()
        checks += 4

        shared = scratch / 'concurrent.json'
        concurrent_outputs = [scratch / f'concurrent-{i}.png' for i in range(2)]
        processes = [subprocess.Popen(
            [str(renderer), str(scene), '--width', '64', '--height', '48', '--threads', '1',
             '--deterministic', '--output', str(output), '--profile-json', str(shared)],
            cwd=scratch, env=env, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
            for output in concurrent_outputs]
        for process in processes:
            process.communicate(timeout=30)
        codes = [process.returncode for process in processes]
        assert sorted(codes) == [0, 4], 'Concurrent profile publication must have exactly one winner'
        report = read(shared, 'complete', 0)
        assert report['output'] == str(concurrent_outputs[codes.index(0)])
        checks += 2

        limited = scratch / 'limited.json'
        no_image = scratch / 'must-not-exist.png'
        result = run('--work-budget', '1', '--output', no_image, '--profile-json', limited)
        assert result.returncode == 6 and not no_image.exists(), result.stderr
        report = read(limited, 'work_exhausted', 6)
        assert report['render_status'] == 'work_exhausted'
        assert report['stages']['save']['state'] == 'not_run'
        assert any(s['state'] == 'interrupted' for s in report['stages'].values())
        checks += 3

        failed_load = scratch / 'missing.json'
        result = run('--output', no_image, '--profile-json', failed_load,
                     input_path=scratch / 'does-not-exist.m3p')
        assert result.returncode == 2 and not no_image.exists()
        report = read(failed_load, 'load_error', 2)
        assert report['render_status'] == 'not_run'
        assert report['stages']['load']['state'] == 'interrupted'
        checks += 2

        failed_save = scratch / 'save-error.json'
        result = run('--output', scratch / 'no-directory' / 'image.png', '--profile-json', failed_save)
        assert result.returncode == 4, result.stderr
        report = read(failed_save, 'write_error', 4)
        assert report['render_status'] == 'completed'
        assert report['stages']['save']['state'] == 'interrupted'
        checks += 2

        result = run('--output', measured, '--profile-json', scratch / 'no-directory' / 'report.json')
        assert result.returncode == 4, 'Sidecar write failure falsely reported success'
        checks += 1
        print(f'CLI profile: {checks} checks passed; byte-exact image and structured failure evidence.')


if __name__ == '__main__':
    main()
