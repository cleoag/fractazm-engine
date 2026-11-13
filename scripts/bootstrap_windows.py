"""Build, test and smoke-render a Git checkout without retained local runs.

Run: python scripts/bootstrap_windows.py --jobs 4
Requires Windows x64, VS 2022 C++/SDK tools, CMake >=3.28, Git, Python 3.12-3.14.
Default: compact release checks and three smoke renders. --scope ordinary
retains the optional historical regression suite and pinned Python packages.
"""
from __future__ import annotations

import argparse
from datetime import datetime, timezone
import hashlib
import json
import os
from pathlib import Path
import platform
import shutil
import subprocess
import sys
import time
import xml.etree.ElementTree as ET
from release_smoke import check_disk

ROOT = Path(__file__).resolve().parents[1]
MODULES = (
    'test_batch_resume', 'test_checkpoint_write', 'test_coverage_evidence',
    'test_discrete_selector_domains', 'test_external_constant_layout',
    'test_formula_input_rejection', 'test_formula_parameter_layouts',
    'test_formula_point_contexts', 'test_formula_views', 'test_generated_option_schema',
    'test_pascal_formula_identity', 'test_pascal_legacy_formula_identity',
    'test_point_resume', 'test_qualify_legacy_loader', 'test_replay_integrity',
    'test_stage_a_release', 'tests.test_parity_acceptance', 'tests.test_trace_validation',
    'tests.test_windows_helper_local_paths', 'tests.test_windows_path_hygiene',
    'tests.test_windows_workspace', 'test_formula_hang_guards', 'test_final_supplementary',
    'test_gencwarp_reference_domain', 'test_manowar_hybrids', 'test_benchmark_precision',
    'test_compatibility_precision', 'test_benchmark_metal_precision',
    'test_benchmark_native_backends',
)
STANDALONE = ('test_windows_workspace.py', 'test_windows_path_hygiene.py',
              'test_windows_helper_local_paths.py')
def digest(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def output_path(value: Path, name: str) -> Path:
    path = value.resolve()
    if path == ROOT or path == ROOT / 'mb3d' or ROOT / 'mb3d' in path.parents:
        raise ValueError(f'{name} must not overwrite the repository or immutable mb3d: {path}')
    return path


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--scope', choices=('release', 'ordinary'), default='release')
    parser.add_argument('--jobs', type=int, default=4)
    parser.add_argument('--build-dir', type=Path, default=ROOT / 'build-vs-bootstrap')
    parser.add_argument('--venv-dir', type=Path, default=ROOT / '.venv-ordinary')
    parser.add_argument('--output-dir', type=Path)
    parser.add_argument('--zlib-source', type=Path, help='Existing zlib 1.3.1 sources for offline configuration')
    parser.add_argument('--skip-install', action='store_true', help='Use an existing venv with the pinned packages')
    args = parser.parse_args()
    if os.name != 'nt' or platform.machine().upper() not in ('AMD64', 'X86_64'):
        parser.error('This verified bootstrap requires native Windows x64; ARM/GPU qualification is pending.')
    if not (3, 12) <= sys.version_info[:2] <= (3, 14) or sys.maxsize <= 2**32:
        parser.error('Use 64-bit CPython 3.12, 3.13 or 3.14 (checked: 3.14.2).')
    if args.jobs < 1:
        parser.error('--jobs must be positive')
    build = output_path(args.build_dir, 'build directory')
    venv = output_path(args.venv_dir, 'venv directory')
    out = output_path(args.output_dir or ROOT / 'comparison/runs' /
                      ('bootstrap-' + datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ')), 'output directory')
    out.mkdir(parents=True, exist_ok=False)
    # Windows subprocess/MSBuild reject duplicate environment names with different case.
    env = {key.upper(): value for key, value in os.environ.items()
           if not key.upper().startswith(('MB3D_', 'PYTHONPATH', 'PYTHONHOME'))}
    env['PYTHONUTF8'] = '1'
    env['PYTHONNOUSERSITE'] = '1'
    env['PYTHONOPTIMIZE'] = '0'
    env['MSBUILDDISABLENODEREUSE'] = '1'
    receipt = dict(schema_version=1, status='running', scope=args.scope + ' CPU bootstrap',
                   started_utc=datetime.now(timezone.utc).isoformat(), repository=str(ROOT),
                   build=str(build), output=str(out), python=sys.version,
                   excluded=['Monte Carlo', 'Halton', 'JIT', 'full corpus parity', 'GPU', 'Apple Silicon'],
                   steps=[], smoke_images=[])

    def save() -> None:
        (out / 'bootstrap.json').write_text(json.dumps(receipt, indent=2) + '\n', encoding='utf-8')

    def run(name: str, command: list[str | Path], *, cwd: Path = ROOT, timeout: int = 1800) -> str:
        check_disk(out)
        argv = [str(x) for x in command]
        print(f'[{len(receipt["steps"]) + 1}] {name} (log: {out / (name + ".log")})', flush=True)
        step = dict(name=name, command=argv, cwd=str(cwd), status='running')
        receipt['steps'].append(step)
        save()
        started = time.monotonic()
        log = out / (name + '.log')
        try:
            with log.open('wb') as stream:
                process = subprocess.run(argv, cwd=cwd, env=env, stdout=stream,
                                         stderr=subprocess.STDOUT, timeout=timeout, check=False)
            step.update(exit_code=process.returncode, seconds=round(time.monotonic() - started, 3),
                        log_sha256=digest(log), status='pass' if process.returncode == 0 else 'fail')
            save()
            text = log.read_text(encoding='utf-8', errors='replace')
            if process.returncode:
                print('\n'.join(text.splitlines()[-70:]), flush=True)
                raise RuntimeError(f'{name} failed ({process.returncode}); see {log}')
            return text
        except (OSError, subprocess.TimeoutExpired) as error:
            step.update(status='fail', error=str(error), seconds=round(time.monotonic() - started, 3))
            save()
            raise

    try:
        for name in ('cmake', 'ctest', 'git'):
            if not shutil.which(name):
                raise RuntimeError(f'{name} is missing from PATH; see docs/BUILD.md#fresh-windows-machine')
        vswhere = Path(os.environ.get('ProgramFiles(x86)', 'C:/Program Files (x86)')) / \
            'Microsoft Visual Studio/Installer/vswhere.exe'
        if not vswhere.is_file():
            raise RuntimeError('Install Visual Studio 2022 / Build Tools with Desktop development with C++ and a Windows SDK.')
        vs = run('visual-studio', [vswhere, '-products', '*', '-version', '[17.0,18.0)',
                                  '-requires', 'Microsoft.VisualStudio.Component.VC.Tools.x86.x64',
                                  '-property', 'installationPath'])
        if not vs.strip():
            raise RuntimeError('VS 2022 x86/x64 C++ tools were not found. Modify the VS 2022 installation.')
        receipt['git_commit'] = run('git-commit', ['git', 'rev-parse', 'HEAD']).strip()
        receipt['git_status_before'] = run('git-status', ['git', 'status', '--porcelain', '--untracked-files=no']).splitlines()
        receipt['cmake_version'] = run('cmake-version', ['cmake', '--version']).splitlines()[0]
        python = Path(sys.executable)
        if args.scope == 'ordinary':
            if venv.exists() and not (venv / 'pyvenv.cfg').is_file():
                raise RuntimeError(f'Refusing to reuse a non-venv directory: {venv}')
            if not venv.exists():
                if args.skip_install:
                    raise RuntimeError('--skip-install requires an existing prepared venv')
                run('create-venv', [sys.executable, '-m', 'venv', venv])
            python = venv / 'Scripts/python.exe'
            requirements = ROOT / 'comparison/requirements-ordinary.txt'
            if not args.skip_install:
                run('install-python-tools', [python, '-m', 'pip', 'install', '--disable-pip-version-check',
                                             '--only-binary=:all:', '-r', requirements])
            package_check = (
                "import importlib.metadata as m,json,pathlib,sys; "
                "pins=[s.strip().split('==') for s in pathlib.Path(sys.argv[1]).read_text().splitlines() "
                "if s.strip() and not s.lstrip().startswith('#')]; "
                "actual={p:m.version(p) for p,v in pins}; "
                "assert all(actual[p]==v for p,v in pins),actual; print(json.dumps(actual))"
            )
            receipt['packages'] = json.loads(run('python-packages', [python, '-c', package_check, requirements]))
        configure = ['cmake', '-S', ROOT, '-B', build, '-G', 'Visual Studio 17 2022', '-A', 'x64',
                     '-DMB3D_ENABLE_PARITY_TRACE=ON', '-DCMAKE_DISABLE_FIND_PACKAGE_ZLIB=ON',
                     '-DMB3D_ENABLE_METAL_RENDERER=OFF', '-DMB3D_BUILD_DIAGNOSTICS=OFF',
                     f'-DPython3_EXECUTABLE={python}']
        if args.zlib_source:
            source = args.zlib_source.resolve()
            if not (source / 'CMakeLists.txt').is_file():
                raise RuntimeError(f'--zlib-source is not a zlib source directory: {source}')
            configure.append(f'-DFETCHCONTENT_SOURCE_DIR_ZLIB={source}')
        run('configure', configure)
        run('build', ['cmake', '--build', build, '--config', 'Release', '--parallel', str(args.jobs),
                      '--target', 'mb3d_release_checks' if args.scope == 'release' else 'mb3d_ordinary', '--', '/nr:false'])
        plan = json.loads(run('ctest-plan', ['ctest', '--test-dir', build, '-C', 'Release',
                                            '-L', '^' + args.scope + '$', '--show-only=json-v1']))
        names = [test['name'] for test in plan['tests']]
        required = {'render_control_api_tests', 'render_control_cli_tests', 'render_profile_cli_tests'}
        if not required.issubset(names) or any('monte' in name or 'halton' in name for name in names):
            raise RuntimeError(f'Incomplete or out-of-scope test plan: {names}')
        receipt['native_tests'] = names
        try:
            run('ctest', ['ctest', '--test-dir', build, '-C', 'Release', '-L', '^' + args.scope + '$',
                          '--output-on-failure', '--timeout', '180', '--output-junit', out / 'ctest.xml'])
        finally:
            native_log = build / 'Testing/Temporary/LastTest.log'
            if native_log.is_file():
                shutil.copyfile(native_log, out / 'LastTest.log')
        cases = ET.parse(out / 'ctest.xml').getroot().findall('.//testcase')
        if len(cases) != len(names) or any(c.find('skipped') is not None or c.find('failure') is not None
                                          or c.find('error') is not None for c in cases):
            raise RuntimeError('CTest did not produce a complete passing result')
        tool_dir = ROOT / 'comparison/tools'
        if args.scope == 'ordinary':
            run('python-tests', [python, '-m', 'unittest', *MODULES], cwd=tool_dir, timeout=300)
            for script in STANDALONE:
                run(Path(script).stem, [python, tool_dir / 'tests' / script], timeout=60)
        renderer = build / 'Release/mb3d_render.exe'
        receipt['renderer_sha256'] = digest(renderer)
        run('smoke', [python, '-B', ROOT / 'scripts/release_smoke.py', '--build', build,
                      '--output', out / 'smoke', '--backend', 'cpu'], timeout=600)
        receipt['smoke_images'] = json.loads((out / 'smoke/smoke.json').read_text())['images']
        receipt['status'] = 'pass'
        print(f'PASS: {len(names)} {args.scope} suites, {len(receipt["smoke_images"])} smoke images.\n'
              f'Renderer: {renderer}\nReceipt: {out / "bootstrap.json"}', flush=True)
        return 0
    except Exception as error:
        receipt.update(status='fail', error=str(error))
        print(f'FAIL: {error}\nReceipt: {out / "bootstrap.json"}', file=sys.stderr, flush=True)
        return 1
    finally:
        receipt['finished_utc'] = datetime.now(timezone.utc).isoformat()
        save()


if __name__ == '__main__':
    raise SystemExit(main())
