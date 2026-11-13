"""Exercise strict error contracts through the real CLI and public API test.

Expected rejections are recorded as rejected, never as numerical parity PASS.
The optional API executable tests the actual C ABI and public Render in one
process, including valid rendering after each failure. No Pascal run is needed
or safe for the known undefined/nonterminating inputs used here.
"""
from __future__ import annotations

import argparse
import hashlib
import json
from pathlib import Path
import struct
import subprocess
import time

from external_validation import ROOT, decode, write_json


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def prepare(output):
    evidence = ROOT / 'comparison/results/external-complete-20261013'
    manifests = ['gnarly-family-manifest-v1.json', 'fold-kali-tube-manifest-v1.json',
                 'trifox-manifest-v1.json']
    inventory = {}
    for filename in manifests:
        document = json.loads((evidence / filename).read_text())
        for case in document['cases'] + document.get('excluded_nonterminating_original', []):
            inventory[case['id']] = case

    def original(case_id, expected=''):
        case = inventory[case_id]
        path = ROOT / Path(case['scene'].replace('\\', '/'))
        if sha(path) != case['sha256']:
            raise ValueError('Original scene changed: ' + case_id)
        return dict(id=case_id, scene=str(path.resolve()), sha256=sha(path),
                    expected_diagnostic=expected, source_manifest=manifests)

    cases = [original('challenge_TrifoxComplexAngles_early')]
    for formula in ['gnarlyIFS', 'gnarly2IFS', 'gnarly3IFS', 'ducksIFS']:
        cases.append(original(f'challenge_{formula}_p8_alternate',
                              'FORMULA_INPUT_REJECTED[caller_dependent_flip]'))
    for option in [0, 3]:
        cases.append(original(f'challenge_TrifoxComplexAngles_p{option}_zero',
                              'FORMULA_INPUT_REJECTED[nonterminating_zero_power]'))

    base = decode(Path(cases[0]['scene']))
    scene_dir = output / 'scenes'
    scene_dir.mkdir(parents=True, exist_ok=True)

    def generated(case_id, data, diagnostic, changes):
        path = scene_dir / (case_id + '.m3p')
        path.write_bytes(data)
        cases.append(dict(id=case_id, scene=str(path.resolve()), sha256=sha(path),
                          source_sha256=cases[0]['sha256'], changes=changes,
                          expected_diagnostic=diagnostic))

    unknown = bytearray(base)
    unknown[860:892] = b'not_a_registered_external_formula'[:32]
    generated('unknown_active', unknown, 'FORMULA_INPUT_REJECTED[unknown_external_formula]',
              'Replace active slot 1 name with an unknown non-NUL 32-byte name')
    dormant = bytearray(base)
    offset = 848 + 5 * 188
    dormant[offset:offset + 188] = unknown[848:1036]
    struct.pack_into('<i', dormant, offset, 0)
    generated('unknown_dormant', dormant, '', 'Add unknown slot 6 with initial count zero')

    guarded = bytearray(base)
    guarded[841] = 0
    guarded[1036:1224] = base[848:1036]
    guarded[848:1036] = bytes(188)
    struct.pack_into('<iii', guarded, 848, 1, 20, 5)
    label = b'_CondItcountSwitch'
    guarded[860:860 + len(label)] = label
    struct.pack_into('<5d', guarded, 908, 1., 2., 0., 0., 0.)
    generated('conditional_disables_counted_partner', guarded,
              'Formula iteration failed: no-progress work budget exhausted',
              'Slot 1 conditional disables the counted Trifox partner in slot 2')
    mix_source = ROOT / 'comparison/scenes_phase3_decomb/phase3_decomb_intpow-bulbox_v0_medium.m3p'
    invalid_mix = decode(mix_source)
    invalid_mix[843] = 5
    generated('invalid_de_mix_non_difs_second', invalid_mix,
              'FORMULA_INPUT_REJECTED[invalid_de_mix]',
              'DE mix with a non-dIFS second group, explicitly invalid in Pascal HeaderTrafos')
    cases[-1]['source_sha256'] = sha(mix_source)
    cases[-1]['source_scene'] = str(mix_source)
    return cases, {str(evidence / p): sha(evidence / p) for p in manifests}


def run_cli(exe, case, threads, output, timeout, label):
    image = output / (label + '.bmp')
    if image.exists():
        raise ValueError('Refusing to reuse an existing output: ' + str(image))
    command = [str(exe), case['scene'], '-o', str(image), '--format', 'bmp',
               '-w', '16', '-h', '12', '-t', str(threads), '--max-iter', '8',
               '--no-postprocess']
    started = time.monotonic()
    result = subprocess.run(command, cwd=ROOT, capture_output=True, text=True,
                            errors='replace', timeout=timeout)
    (output / (label + '.stdout.txt')).write_text(result.stdout, encoding='utf-8')
    (output / (label + '.stderr.txt')).write_text(result.stderr, encoding='utf-8')
    row = dict(case=case['id'], threads=threads, command=command,
               returncode=result.returncode, duration_seconds=time.monotonic() - started,
               stdout_sha256=sha(output / (label + '.stdout.txt')),
               stderr_sha256=sha(output / (label + '.stderr.txt')))
    diagnostic = case['expected_diagnostic']
    if diagnostic:
        if result.returncode != 3 or diagnostic not in result.stderr or image.exists():
            raise AssertionError(f'{label}: expected clean render rejection, got exit '
                                 f'{result.returncode}, artifact={image.exists()}: {result.stderr}')
        row.update(status='rejected', expected_diagnostic=diagnostic, contract_verified=True,
                   parity_status='unverified')
    else:
        if result.returncode != 0 or not image.is_file():
            raise AssertionError(f'{label}: valid render failed: {result.stderr}')
        data = image.read_bytes()
        if data[:2] != b'BM' or len(data) < 54:
            raise AssertionError(label + ': invalid output BMP')
        width, height = struct.unpack_from('<ii', data, 18)
        if width != 16 or abs(height) != 12:
            raise AssertionError(label + ': unexpected output dimensions')
        row.update(status='rendered', output_sha256=sha(image), width=width, height=abs(height))
    return row


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--cpp', type=Path, required=True)
    parser.add_argument('--api-test', type=Path,
                        help='mb3d_tests_render_failures executable (actual linked C API)')
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--timeout', type=float, default=20.)
    args = parser.parse_args()
    exe, output = args.cpp.resolve(), args.output.resolve()
    output.mkdir(parents=True, exist_ok=True)
    if (output / 'summary.json').exists():
        parser.error('Use a new output directory; existing evidence is not overwritten')
    cases, manifests = prepare(output)
    report = dict(scope=__doc__, cpp=str(exe), cpp_sha256=sha(exe),
                  tool_sha256=sha(Path(__file__)), manifest_sha256=manifests,
                  cases=cases, results=[], finished=False, success=False)
    write_json(output / 'summary.json', report)
    try:
        for threads in [1, 4]:
            baseline = run_cli(exe, cases[0], threads, output, args.timeout, f't{threads}_baseline')
            report['results'].append(baseline)
            for case in cases[1:]:
                label = f't{threads}_{case["id"]}'
                row = run_cli(exe, case, threads, output, args.timeout, label)
                if row['status'] == 'rendered' and row['output_sha256'] != baseline['output_sha256']:
                    raise AssertionError(label + ': dormant unknown changed output')
                report['results'].append(row)
                print(label, row['status'], flush=True)
                write_json(output / 'summary.json', report)
        if args.api_test:
            api = args.api_test.resolve()
            command = [str(api), str(ROOT), str(output / 'api-scenes')]
            result = subprocess.run(command, cwd=ROOT, capture_output=True, text=True,
                                    errors='replace', timeout=max(45., args.timeout))
            for stream in ['stdout', 'stderr']:
                (output / ('api.' + stream + '.txt')).write_text(getattr(result, stream), encoding='utf-8')
            report['api'] = dict(command=command, executable_sha256=sha(api),
                                 returncode=result.returncode,
                                 stdout_sha256=sha(output / 'api.stdout.txt'),
                                 stderr_sha256=sha(output / 'api.stderr.txt'))
            if result.returncode:
                raise AssertionError('Public/C API failure/recovery test failed: ' + result.stderr)
        else:
            report['api'] = dict(status='not_run', reason='No --api-test executable supplied')
        report['success'] = True
        report['finished'] = True
    except Exception as error:
        report['error'] = str(error)
        raise
    finally:
        write_json(output / 'summary.json', report)
    print('CLI error contracts verified; expected rejections remain unverified for parity.')


if __name__ == '__main__':
    main()
