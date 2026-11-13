#!/usr/bin/env python3
"""Predeclared reciprocal point controls under both wrapper policies; diagnostic only.

Expected 47-word states are computed before any device process. Each bulk run
uses three in-process repeats. These controls do not replace the independently
reviewed three-fresh-process formula collector or affected full-scene checks.
"""
from __future__ import annotations

import argparse
from collections import Counter
import json
from pathlib import Path
import shutil
import struct

import native_reciprocal_reference as reference
import native_formula_observer as observer
from gpu_pipeline_proofs import STATE
from qualify_native_metal_all import ROOT, require, sha
from prepare_gpu_parallel_bundle import ref, write
from run_m3parameter_gpu import invoke
from verify_native_formula_group import validate_build_products

UNCHANGED = {6030, 7007}
POINT_COUNTS = {6030: 14, 7007: 11, 4812: 16, 5823: 16}


def build_controls():
    """Group the frozen 57 point cases by their complete 2048-word program."""
    cases = [c for c in reference.build_cases() if c['input']['kind'] == 'point']
    require(len(cases) == 57 and len({c['id'] for c in cases}) == 57 and
            Counter(c['identity'][1] for c in cases) == POINT_COUNTS,
            'Missing, duplicate or changed predeclared point corpus')
    registry = {tuple(c['identity']) for c in cases}
    groups = {}
    for case in cases:
        program, state = observer.validate_case(case['input'], registry)
        expected = reference.evaluate(case['input'])
        require(expected == case['expected_state_bits'], 'Independent case does not replay')
        key = tuple(program)
        if key not in groups:
            groups[key] = {'id': f'{case["identity"][1]}-program-{len(groups):02d}',
                           'identity': case['identity'], 'case_ids': [], 'variants': [],
                           'program': program, 'states': [], 'expected': []}
        batch = groups[key]
        require(batch['identity'] == case['identity'], 'Program aliases different identities')
        batch['case_ids'].append(case['id']);batch['variants'].append(case['variant'])
        batch['states'].append(state)
        batch['expected'].append([int(expected[lane], 16) for lane in STATE])
    require(len(groups) == 26, 'Predeclared program grouping changed')
    return list(groups.values())


def wrapper_audit(batch, policy):
    require(type(policy) is int and policy in (0, 1), 'Invalid wrapper policy')
    # The raw ABI reports a specialized formula ID, not a Boolean flag.
    selected = 5823 if batch['identity'][1] == 5823 and policy == 1 else 0
    return [1, policy, selected] + [1, policy, 0] * 5


def validate_bulk(report, batch, policy, program_path, library_path):
    """Replay raw probe integrity, all 59 output words and exact executed inputs."""
    count = len(batch['states'])
    require(report['schema'] == 'mb3d.native-formula-probe.v1' and
            report['status'] == 'COMPLETE' and report['profile'] == 'native_gpu_fp32',
            'Probe did not complete Native Metal execution')
    require(observer.words(report['library_profile'], 3) == [2, 4, 32] and
            observer.words(report['formula_abi'], 5) == [1, 8192, 188, 236, 32], 'Wrong device ABI')
    require(report['scene'] is None and report['program'] == str(program_path) and
            report['library'] == str(library_path), 'Wrong probe inputs')
    for key, value in {'mode': 0, 'slot': 0, 'point_count': count, 'repeats': 3,
                       'wrapper_policy': policy, 'invalid_descriptor_controls': 7, 'fault_count': 0}.items():
        require(type(report[key]) is int and report[key] == value, 'Wrong probe field: ' + key)
    require(all(report[k] is True for k in ('exact_state_repeats', 'input_immutable', 'allocation_canaries')),
            'Repeat/input/allocation integrity failure')
    require(isinstance(report['device'], str) and report['device'].strip(), 'Missing GPU identity')
    require(observer.words(report['wrapper_audit_words'], 18) == wrapper_audit(batch, policy),
            'Wrong observed wrapper policy or unexpected specialized formula')
    require(observer.words(report['program_words'], 2048) == batch['program'], 'Executed program changed')
    require(isinstance(report['rows'], list) and len(report['rows']) == count, 'Wrong raw result length')
    results = []
    for i, row in enumerate(report['rows']):
        require(type(row['index']) is int and row['index'] == i and
                observer.words(row['input_words'], 47) == batch['states'][i], 'Executed state differs')
        result = observer.words(row['result_words'], 59)
        require(type(row['fault']) is int and row['fault'] == 0 and
                type(row['work']) is int and row['work'] == 1 and
                result[47:] == [0, 0, 0, 0, 1, batch['program'][11], 0, 0, 0, 1, 0, 0],
                'Faulted/incomplete/changed point result metadata or work')
        results.append(result)
    return results


def compare_results(batch, results):
    require(set(results) == {('old', 0), ('old', 1), ('candidate', 0), ('candidate', 1)},
            'Missing old/candidate wrapper-policy control')
    count = len(batch['states'])
    require(len(batch['expected']) == len(batch['case_ids']) == count,
            'Missing expected cases')
    for expected in batch['expected']:
        observer.words(expected, 47)
    for rows in results.values():
        require(len(rows) == count, 'Control result row count differs')
        for row in rows:
            observer.words(row, 59)
    require(results['candidate', 0] == results['candidate', 1],
            'Candidate wrapper policies differ in full 59-word results')
    mismatches = {}
    for policy in (0, 1):
        old, candidate = results['old', policy], results['candidate', policy]
        require(all(row[:47] == expected for row, expected in zip(candidate, batch['expected'])),
                'Candidate differs from independent 47-word reference')
        require(all(a[47:] == b[47:] for a, b in zip(old, candidate)),
                'Old/candidate metadata or work changed')
        if batch['identity'][1] in UNCHANGED:
            require(old == candidate, 'Unaffected formula changed in full 59-word results')
        differences = []
        for index, (row, expected) in enumerate(zip(old, batch['expected'])):
            lanes = [lane for lane, actual, wanted in zip(STATE, row, expected) if actual != wanted]
            if lanes:
                differences.append({'row': index, 'case_id': batch['case_ids'][index], 'lanes': lanes})
        mismatches[str(policy)] = differences
    return {'points': count, 'candidate_reference_exact': True,
            'candidate_policies_59_words_exact': True,
            'old_policies_59_words_exact': results['old', 0] == results['old', 1],
            'metadata_work_unchanged': True,
            'unaffected_59_words_exact': batch['identity'][1] in UNCHANGED,
            'old_reference_mismatches_by_policy': mismatches}


def dependency_paths(model_manifest, builds):
    paths = {Path(__file__).resolve(), Path(reference.__file__).resolve(),
             *reference.dependency_paths(), *observer.dependency_paths(),
             ROOT/'comparison/tools/verify_native_formula_group.py',
             ROOT/'comparison/tools/prepare_gpu_parallel_bundle.py'}
    for key in ('semantic_sources', 'abi_sources', 'dependencies'):
        for name, expected in model_manifest[key].items():
            path = ROOT/name
            require(sha(path) == expected, 'Independent dependency differs from manifest')
            paths.add(path)
    for build in builds.values():
        paths.update(build.values())
        receipt = observer.read_json(build['receipt'])
        for record in receipt['records']:
            path = (build['receipt'].parent/record['path']).resolve()
            require(sha(path) == record['sha256'], 'Compiled receipt record changed')
            paths.add(path)
        source_manifest = build['receipt'].parent/'source-manifest.json'
        require(sha(source_manifest) == receipt['source_manifest_sha256'], 'Build source manifest changed')
        paths.add(source_manifest)
    return paths


def collect(old_build, candidate_build, output, timeout=150):
    require(type(timeout) is int and 0 < timeout <= 150, 'Diagnostic deadline must be at most 150 seconds')
    old_build, candidate_build, output = (Path(p).resolve() for p in (old_build, candidate_build, output))
    require(all(p.is_relative_to(ROOT) for p in (old_build, candidate_build, output)) and not output.exists(),
            'Frozen repository builds and a new output directory are required')
    require(shutil.disk_usage(ROOT).free >= 12 * 1024 ** 3, 'Disk reserve below 12 GiB')
    builds = {}
    for side, build in (('old', old_build), ('candidate', candidate_build)):
        probe, library, receipt = build / 'mb3d_native_formula_probe', build / 'metal_native_scene.metallib', build.parent / 'build-receipt.json'
        validate_build_products(receipt, probe, library)
        builds[side] = {'probe': probe, 'library': library, 'receipt': receipt}
    # Everything below this line is independently computed BEFORE the first invoke.
    batches = build_controls()
    model_manifest = reference.manifest()
    dependencies = dependency_paths(model_manifest, builds)
    pinned = {p: sha(p) for p in dependencies}
    output.mkdir(parents=True)
    result = {'schema': 'mb3d.native-reciprocal-policy-controls.v1', 'status': 'RUNNING', 'qualification': False,
              'scope': '57 predeclared point states per side/policy; three in-process repeats, not three fresh-process qualification',
              'reference_manifest': model_manifest, 'builds': {k: {n: ref(p) for n, p in v.items()} for k, v in builds.items()},
              'points_per_side_policy': 57, 'wrapper_policies': [0, 1], 'repeats_per_process': 3,
              'timeout_seconds': timeout, 'dependencies': [ref(p) for p in sorted(dependencies)], 'batches': []}
    files = {}
    for batch in batches:
        folder = output / batch['id'];folder.mkdir()
        program_path, states_path, expected_path = (folder / name for name in ('program.bin', 'states.bin', 'expected-state.bin'))
        program_path.write_bytes(struct.pack('<2048I', *batch['program']))
        states_path.write_bytes(b''.join(struct.pack('<47I', *s) for s in batch['states']))
        expected_path.write_bytes(b''.join(struct.pack('<47I', *s) for s in batch['expected']))
        files[batch['id']] = (folder, program_path, states_path, expected_path)
        pinned.update({p: sha(p) for p in (program_path, states_path, expected_path)})
    write(output / 'controls.json', result)
    try:
        for number, batch in enumerate(batches, 1):
            folder, program_path, states_path, expected_path = files[batch['id']]
            row = {k: batch[k] for k in ('id', 'identity', 'case_ids', 'variants')}
            row.update(status='RUNNING', program=ref(program_path), states=ref(states_path), expected=ref(expected_path), runs=[])
            result['batches'].append(row)
            outputs = {}
            for side in ('old', 'candidate'):
                for policy in (0, 1):
                    require(all(sha(p) == h for p, h in pinned.items()), 'Pinned diagnostic input changed')
                    build = builds[side];run = folder / f'{side}-policy{policy}';raw = run / 'raw-probe.json'
                    argv = [build['probe'], '--program', program_path, '--states', states_path,
                            '--library', build['library'], '--output', raw, '--mode', 'step',
                            '--slot', '0', '--repeats', '3', '--wrapper-policy', str(policy)]
                    event = invoke(argv, run, ROOT, timeout=timeout, evidence_root=ROOT,
                                   input_files=[*dependencies, expected_path], exclusive_gpu=True,
                                   termination_grace_seconds=1)
                    entry = {'side': side, 'policy': policy, 'event': ref(run / 'event.json')};row['runs'].append(entry)
                    if raw.exists():entry['raw'] = ref(raw)
                    require(event['exit_code'] == 0 and event['guard'] is None and raw.is_file(), 'Diagnostic process failed or guarded')
                    require(all(sha(p) == h for p, h in pinned.items()), 'Pinned diagnostic input changed during process')
                    report = observer.read_json(raw)
                    outputs[side, policy] = validate_bulk(report, batch, policy, program_path, build['library'])
                    entry['device'] = report['device']
                    entry['specialized'] = batch['identity'][1] == 5823 and policy == 1
                    write(output / 'controls.json', result)
            row.update(compare_results(batch, outputs), status='PASS_DIAGNOSTIC')
            write(output / 'controls.json', result)
            print(f'{number}/{len(batches)} ({number * 100 // len(batches)}%) {batch["id"]}: PASS_DIAGNOSTIC', flush=True)
        result['old_mismatch_counts_by_policy'] = {
            str(policy): sum(len(b['old_reference_mismatches_by_policy'][str(policy)])
                             for b in result['batches']) for policy in (0, 1)}
        result['processes'] = len(batches) * 4
        result['device_invocations'] = 57 * 4 * 3
        result['status'] = 'PASS_DIAGNOSTIC'
    except Exception as error:
        result.update(status='FAIL_DIAGNOSTIC', error=str(error))
        if result['batches']:result['batches'][-1]['status'] = 'FAIL_DIAGNOSTIC'
        write(output / 'controls.json', result)
        raise
    write(output / 'controls.json', result)
    return result


def main():
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument('--old-build', type=Path, required=True)
    parser.add_argument('--candidate-build', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--timeout', type=int, default=150)
    args = parser.parse_args()
    collect(args.old_build, args.candidate_build, args.output, args.timeout)


if __name__ == '__main__':
    main()
