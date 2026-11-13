#!/usr/bin/env python3
"""Capture one real Native Metal formula case, without awarding qualification.

The parent collector owns the exclusive GPU lock and supplies its captured
outer-process run_id. This adapter captures the actual probe as a nested
process without reacquiring that lock. Three distinct outer processes and an
independent arithmetic reference remain the collector's responsibilities.
"""
from __future__ import annotations

import argparse
import importlib
import json
import math
from pathlib import Path
import re
import signal
import struct
import sys

from gpu_pipeline_proofs import STATE, binary_identity, output_bound, run_event
from qualify_native_metal_all import ROOT, artifact, current_registry, digest, require, sha
from audit_backend_readiness import strip_comments
from run_m3parameter_gpu import invoke
import native_formula_resource_input as resource_input
import native_formula_context_input as context_input

HEX32 = re.compile(r'0x[0-9a-f]{8}\Z')
RUN_ID = re.compile(r'[0-9a-f]{32}\Z')
HASH = re.compile(r'[0-9a-f]{64}\Z')
METADATA_INPUTS = (
    'src/render/native_metal/generated/inventory.json',
    'src/render/native_metal/generated/formula_metadata.h',
    'src/core/formula_registry.cpp', 'src/core/external_formula_table.cpp',
    'src/render/native_metal/formula_host.cpp', 'src/core/header_trafos.cpp',
    'src/render/native_metal/formula_abi.h', 'mb3d/HeaderTrafos.pas',
    'src/render/native_metal/orbit_driver.metal.h',
)

# HeaderTrafos selects the global caller after reducing slot DE options; forced
# numerical DE and modifier slots make global == slot an invalid requirement.
ORBIT_OPTIONS = {0: {-1, 0, 1}, 1: {2, 11}, 2: {4}, 3: {5, 6},
                 4: {-1, 0, 1}, 5: {2, 11}, 6: {4}, 7: {5, 6},
                 8: {20}, 9: {20}}


def read_json(path):
    def unique(pairs):
        result = {}
        for key, value in pairs:
            require(key not in result, 'Duplicate JSON key: ' + key)
            result[key] = value
        return result
    return json.loads(Path(path).read_text(), object_pairs_hook=unique)


def words(value, count):
    require(isinstance(value, list) and len(value) == count and
            all(type(x) is int and 0 <= x <= 0xffffffff for x in value),
            'Malformed uint32 word array')
    return value


def descriptor_metadata(root=ROOT):
    """Read actual upload metadata, without interpreting formula arithmetic.

    The inventory's registry arity can include appended C values. Parameter-map
    types describe uploaded PVar parameters instead. Both generated tables must
    agree; every input and the source of the caller/internal rules is captured.
    """
    inventory = read_json(root / METADATA_INPUTS[0])['formulas']
    configured = re.findall(r'case (\d+)u: slot.traits=(\d+)u; slot.wrapper_flags=(\d+)u; slot.difs_style=(\d+)u; return true;',
                            (root / METADATA_INPUTS[1]).read_text())
    device = {int(row[0]): tuple(map(int, row[1:])) for row in configured}
    require(len(device) == len(configured) == len(inventory) and
            set(device) == {row['id'] for row in inventory}, 'Inconsistent external metadata identities')
    external = {}
    for row in inventory:
        ident = row['id']; types = row['parameter_types']
        require(isinstance(types, list) and all(type(t) is int and t in (0, 1, 2, 3) for t in types),
                'Invalid parameter-map type metadata')
        if 95001 <= ident <= 95003:
            require(not types and row['param_count'] == 2, 'Native parameter metadata changed')
            count = 2  # NativeFormulaHost's explicit no-paramMap preparation.
        else:
            count = len(types)
            require(row['param_count'] == count or
                    (ident == 7083 and count == 0 and row['param_count'] == 3),
                    'Unreviewed registry arity/uploaded-count difference')
        traits = (row['traits'], row['adds_c'], row['difs_style'])
        require(device[ident] == traits and 0 <= count <= 58,
                'Inconsistent generated slot metadata')
        external[ident] = ((row['de_option'] & 0xffffffff), count, *traits)
    source = strip_comments((root / 'src/core/formula_registry.cpp').read_text())
    match = re.search(r'InternFormulasDEfunc\s*=\s*\{\{([^}]+)\}\}', source)
    require(match is not None, 'Missing internal DE metadata')
    internal = tuple(int(value.strip()) for value in match[1].split(','))
    require(len(internal) == 10, 'Internal descriptor inventory changed')
    return external, internal


def validate_descriptor(program, identity, kind):
    external, internal = descriptor_metadata()
    driver = program[1]
    option = program[6] if program[6] < (1 << 31) else program[6]-(1 << 32)
    # A canonical point profile invokes native_step only, not the orbit driver.
    allowed = set.union(*ORBIT_OPTIONS.values()) if kind == 'point' and driver == 0 else ORBIT_OPTIONS[driver]
    require(option in allowed, 'Incompatible orbit driver/global DE option')
    require(program[66:68] == [0, 0], 'Reserved slot descriptor words must be zero')
    ident = identity[1]
    if identity[0] == 'external':
        require(ident in external and tuple(program[59:64]) == external[ident] and program[64] == 0,
                'External slot descriptor differs from production metadata')
    else:
        require(0 <= ident < len(internal) and program[59] == (internal[ident] & 0xffffffff) and
                program[60:62] == [0, 0] and program[63] == 0,
                'Internal slot descriptor differs from production metadata')
        require(program[62] in ((0, 2) if ident == 4 else (0,)), 'Invalid internal wrapper flags')
        require(2 <= program[64] <= 8 if ident in (0, 6) else program[64] == 0,
                'Invalid internal power selector')
        if ident == 6:
            require(program[310:312] == [0, 0] and program[330] == 0,
                    'Internal callback address must be removed from uploaded views')
    for slot in range(1, 6):
        at = 56 + slot * 332
        require(all(value == 0 for value in program[at+1:at+332]),
                'Expected canonical unused slot descriptor/storage')


def validate_case(case, registry=None):
    fields = {'identity', 'kind', 'input_state_bits', 'parameter_bits'}
    require(isinstance(case, dict) and set(case) in
            (fields, fields | {'resources'}, fields | {'context'}),
            'Expected semantic formula input fields and explicit resources or context')
    if 'resources' in case:
        resource_input.validate(case['resources'], ROOT)
    identity = case['identity']
    require(isinstance(identity, list) and len(identity) == 3 and
            identity[0] in ('internal', 'external') and type(identity[1]) is int and
            isinstance(identity[2], str), 'Malformed identity')
    if registry is None:
        registry = current_registry(ROOT)
    require(tuple(identity) in registry, 'Unknown ordinary formula identity')
    require(case['kind'] in ('point', 'orbit'), 'Unsupported formula case kind')
    state = case['input_state_bits']
    require(isinstance(state, dict) and set(state) == set(STATE), 'Incomplete STATE')
    bits = case['parameter_bits']
    require(isinstance(bits, list) and len(bits) == 2048, 'Expected full 8192-byte program')
    require(all(isinstance(x, str) and HEX32.fullmatch(x)
                for x in list(state.values()) + bits), 'Expected canonical hex32 bits')
    program = [int(x, 16) for x in bits]
    require(program[0] == 1 and program[1] <= 9 and program[2] == 6,
            'Wrong program ABI/driver/slot count')
    validate_program_scalars(program)
    if 'context' in case:
        state_words = [int(state[x], 16) for x in STATE]
        context_input.validate(case, program, state_words, ROOT)
        # The exact pinned program binds all slots, including the two neighbours.
        # A mixed orbit dispatch executes the entire chain, not only target_slot.
        return program, state_words
    require(program[13] == 0, 'Combined programs are outside this observer')
    require(all(program[x] == 0 for x in (3, 4, 5)), 'Expected single-slot hybrid bounds')
    for slot in range(6):
        at = 56 + slot * 332
        require(program[at] == 1 and program[at+1] in (0, 1) and
                program[at+4] <= 64 and program[at+9] == int(slot == 0),
                'Expected one enabled slot zero with valid ABI')
    require(program[57:59] == [int(identity[0] == 'external'), identity[1]],
            'Program slot and named identity disagree')
    validate_descriptor(program, identity, case['kind'])
    return program, [int(state[x], 16) for x in STATE]


def validate_program_scalars(program):
    """Mirror source-bound native_program_valid controls before any child.

    This validates program scalars only. Typed formula parameters are not all
    floating-point and may contain legitimately dormant infinity aliases.
    """
    require(1 <= program[7] <= (1 << 20) and
            1 <= program[11] <= (1 << 24) and 1 <= program[12] <= (1 << 20),
            'Invalid program iteration/work bounds')
    require(program[3] <= program[5] < 6 and program[4] <= program[5],
            'Invalid program hybrid bounds')
    def value(at):
        return struct.unpack('<f', struct.pack('<I', program[at]))[0]
    require(all(math.isfinite(value(at)) for at in range(16, 23)) and
            all(value(at) > 0 for at in (16, 21, 22)) and
            all(math.isfinite(value(at)) for at in range(24, 44)),
            'Invalid program radius/scale/Julia/rotation scalars')
    require(program[13:16] == [0, 0, 0] and value(23) == 0,
            'Unsupported/noncanonical single-program combination fields')


def dependency_paths(case=None):
    """Executable Python and descriptor-source closure, captured by both processes."""
    # Fixed imported closure, independent of unrelated modules loaded by the
    # collector. Include the registry helper even before its first lazy use.
    names = ('gpu_pipeline_proofs', 'qualify_native_metal_all',
             'run_m3parameter_gpu', 'audit_backend_readiness', 'native_formula_resource_input',
             'native_formula_context_input')
    paths = {Path(__file__).resolve(), *(ROOT / p for p in METADATA_INPUTS), *(
        Path(importlib.import_module(name).__file__).resolve() for name in names)}
    if case is not None and 'context' in case:
        paths.update(context_input.dependency_paths(ROOT))
    return sorted(paths)



def validate_report(report, program, state, kind, program_path, library_path, slot=0):
    require(report['schema'] == 'mb3d.native-formula-probe.v1' and
            report['status'] == 'COMPLETE' and report['profile'] == 'native_gpu_fp32',
            'Probe did not complete native device execution')
    require(words(report['library_profile'], 3) == [2, 4, 32] and
            words(report['formula_abi'], 5) == [1, 8192, 188, 236, 32], 'Wrong device ABI')
    require(report['scene'] is None and report['program'] == str(program_path) and
            report['library'] == str(library_path), 'Wrong probe inputs')
    require(type(report['mode']) is int and report['mode'] == int(kind == 'orbit') and
            type(report['slot']) is int and report['slot'] == slot, 'Wrong probe dispatch')
    require(type(report['point_count']) is int and report['point_count'] == 1 and
            type(report['repeats']) is int and report['repeats'] == 3 and
            type(report['wrapper_policy']) is int and report['wrapper_policy'] == 1,
            'Wrong point/repeat/wrapper configuration')
    require(all(report[k] is True for k in
                ('exact_state_repeats', 'input_immutable', 'allocation_canaries')) and
            type(report['invalid_descriptor_controls']) is int and
            report['invalid_descriptor_controls'] == 7, 'Probe integrity/repeats failed')
    require(isinstance(report['device'], str) and bool(report['device'].strip()), 'Missing Metal device')
    audits = words(report['wrapper_audit_words'], 18)
    require(all(audits[i:i+2] == [1, 1] for i in range(0, 18, 3)), 'Wrapper audit failed')
    require(words(report['program_words'], 2048) == program, 'Executed program differs from case')
    require(type(report['fault_count']) is int and report['fault_count'] == 0, 'Device faults')
    require(isinstance(report['rows'], list) and len(report['rows']) == 1, 'Missing/extra probe rows')
    row = report['rows'][0]
    require(type(row['index']) is int and row['index'] == 0 and
            words(row['input_words'], 47) == state, 'Executed input state differs from case')
    result = words(row['result_words'], 59)
    require(type(row['fault']) is int and row['fault'] == result[47] == 0 and
            type(row['work']) is int and row['work'] == result[51] > 0 and
            result[53:55] == [0, 0] and result[56] == 1 and result[57] in (0, 1) and
            result[58] == 0, 'Faulted/incomplete/malformed NativeOrbitResult')
    return result


def observe(args):
    require(RUN_ID.fullmatch(args.run_id) is not None, 'Invalid outer run_id')
    require(0 < args.timeout <= 180, 'Probe timeout must be at most 180 seconds')
    inp, probe, library, output = (Path(x).resolve() for x in
                                 (args.input, args.probe, args.library, args.output))
    require(all(p.is_relative_to(ROOT) for p in (inp, probe, library, output)),
            'Observer artifacts must be inside the evidence root')
    require(not output.exists(), 'Refusing to overwrite observer output')
    for path, expected in ((probe, args.probe_sha256), (library, args.library_sha256)):
        require(HASH.fullmatch(expected) is not None and path.is_file() and sha(path) == expected,
                'Pinned probe/library identity differs')
    input_sha = sha(inp)
    case = read_json(inp)
    deps = dependency_paths(case)
    dep_hashes = {p: sha(p) for p in deps}
    program, state = validate_case(case)
    slot = case.get('context', {}).get('target_slot', 0)
    require(args.slot in (0, slot), 'CLI slot contradicts explicit context')
    require(sha(inp) == input_sha and all(sha(p) == h for p, h in dep_hashes.items()),
            'Observer input changed during validation')
    work = output.parent / (output.stem + '.probe')
    work.mkdir(parents=True, exist_ok=False)
    program_path, state_path = work / 'program.bin', work / 'state.bin'
    program_path.write_bytes(struct.pack('<2048I', *program))
    state_path.write_bytes(struct.pack('<47I', *state))
    folder = work / 'process'
    raw = folder / 'raw-probe.json'
    argv = [str(probe), '--program', str(program_path), '--states', str(state_path),
            '--library', str(library), '--output', str(raw), '--mode',
            'step' if case['kind'] == 'point' else 'orbit', '--slot', str(slot),
            '--repeats', '3', '--wrapper-policy', '1']
    resource_paths = []
    if 'resources' in case:
        _, resource_paths = resource_input.validate(case['resources'], ROOT)
        argv += resource_input.arguments(case['resources'], ROOT)
    # Failures retain the actual child log, report (including POINT_FAULTS), and
    # captured event. Never turn a failed child into a zero-fault observation.
    event = invoke(argv, folder, work, timeout=args.timeout, evidence_root=ROOT,
                   input_files=[inp, *deps, *resource_paths], exclusive_gpu=False, termination_grace_seconds=1)
    require(event['exit_code'] == 0 and event['guard'] is None, 'Native probe execution failed')
    require(sha(inp) == input_sha and sha(probe) == args.probe_sha256 and
            sha(library) == args.library_sha256 and
            all(sha(p) == h for p, h in dep_hashes.items()), 'Observer input changed during execution')
    report = read_json(raw)
    result = validate_report(report, program, state, case['kind'], program_path, library, slot)
    if 'resources' in case:
        resource_input.validate_arguments(event, case['resources'], ROOT)
        resource_input.validate_report(report, case['resources'], ROOT)
    def ref(path):
        return {'path': str(path.relative_to(ROOT)), 'sha256': sha(path)}
    observation = {
        'schema': 'mb3d.native-formula-observer.v1', 'run_id': args.run_id,
        'identity': case['identity'], 'input_digest': digest(case),
        'library_sha256': args.library_sha256, 'probe_sha256': args.probe_sha256,
        'state_bits': {name: f'0x{word:08x}' for name, word in zip(STATE, result)},
        'gpu_invocations': report['point_count'] * report['repeats'],
        'faults': report['fault_count'], 'work': report['rows'][0]['work'],
        'device': report['device'], 'result_words': result,
        'raw_probe': ref(raw), 'probe_event': ref(folder / 'event.json'),
        'dependencies': [ref(p) for p in deps],
        'exact_state_repeats': report['exact_state_repeats'],
        'allocation_canaries': report['allocation_canaries'],
        'input_immutable': report['input_immutable'],
        'invalid_descriptor_controls': report['invalid_descriptor_controls'],
    }
    if 'resources' in case:
        observation['resources'] = report['resources']
        observation['map_abi'] = report['map_abi']
    if 'context' in case:
        observation['context'] = case['context']
    with output.open('x') as stream:
        json.dump(observation, stream, indent=2, allow_nan=False)
        stream.write('\n')
    return observation


def replay_observation(root, observation_ref, outer_event_ref, semantic_input,
                       expected_probe_ref, expected_library_ref):
    """Replay captured nested execution; callers bind the compiled probe build.

    This verifies the observation against the actual raw probe/event, rather
    than trusting its copied state and metadata. It grants no reference-model,
    formula-domain, full-scene or build-provenance qualification on its own.
    """
    root = Path(root).resolve()
    probe = binary_identity(root, expected_probe_ref)
    library = artifact(root, expected_library_ref)
    adapter = Path(__file__).resolve()
    outer = run_event(root, outer_event_ref,
                      {'path': str(adapter), 'sha256': sha(adapter)}, max_seconds=180)
    observed_path = output_bound(root, outer, observation_ref)
    actual = read_json(observed_path)
    require(actual['schema'] == 'mb3d.native-formula-observer.v1' and
            actual['run_id'] == outer['run_id'], 'Wrong outer observation identity')
    program, state = validate_case(semantic_input)
    slot = semantic_input.get('context', {}).get('target_slot', 0)

    def option(event, name):
        args = event['argv']
        require(args.count(name) == 1 and args.index(name)+1 < len(args),
                'Missing/ambiguous captured argument: ' + name)
        return args[args.index(name)+1]

    def input_path(event, name):
        path = Path(option(event, name)).resolve()
        require(path.is_relative_to(root) and path.is_file(), 'Escaping/missing captured input')
        record = {'path': str(path.relative_to(root)), 'sha256': sha(path)}
        require(record in event['inputs'], 'Uncaptured actual probe input')
        return path

    require(input_path(outer, '--probe') == Path(probe['path']) and
            input_path(outer, '--library') == library and
            option(outer, '--probe-sha256') == expected_probe_ref['sha256'] and
            option(outer, '--library-sha256') == expected_library_ref['sha256'] and
            option(outer, '--run-id') == outer['run_id'] and
            Path(option(outer, '--output')).resolve() == observed_path and
            read_json(input_path(outer, '--input')) == semantic_input,
            'Outer CLI differs from bound case/probe/library')
    deps = [{'path': str(p.relative_to(root)), 'sha256': sha(p)} for p in dependency_paths(semantic_input)]
    require(actual['dependencies'] == deps and all(r in outer['inputs'] for r in deps),
            'Observer dependencies were not captured unchanged')
    nested = run_event(root, actual['probe_event'], probe, max_seconds=180)
    require(nested['run_id'] != outer['run_id'] and nested['pid'] != outer['pid'] and
            outer['started_ns'] <= nested['started_ns'] < nested['finished_ns'] <= outer['finished_ns'],
            'Probe event is not a distinct nested execution')
    require(all(r in nested['inputs'] for r in deps), 'Probe event lacks observer dependencies')
    raw_path = output_bound(root, nested, actual['raw_probe'])
    program_path = input_path(nested, '--program')
    state_path = input_path(nested, '--states')
    require(input_path(nested, '--library') == library and
            Path(option(nested, '--output')).resolve() == raw_path and
            option(nested, '--mode') == ('step' if semantic_input['kind'] == 'point' else 'orbit') and
            option(nested, '--slot') == str(slot) and option(nested, '--repeats') == '3' and
            option(nested, '--wrapper-policy') == '1', 'Wrong nested dispatch arguments')
    require(program_path.read_bytes() == struct.pack('<2048I', *program) and
            state_path.read_bytes() == struct.pack('<47I', *state), 'Raw device input differs from semantic case')
    report = read_json(raw_path)
    result = validate_report(report, program, state, semantic_input['kind'], program_path, library, slot)
    require(actual.get('context') == semantic_input.get('context'),
            'Observed context differs from bound semantic input')
    if 'resources' in semantic_input:
        resources = semantic_input['resources']
        require(all(record in outer['inputs'] for record in resources['files']),
                'Outer formula event lacks actual resource files')
        resource_input.validate_arguments(nested, resources, root)
        resource_input.validate_report(report, resources, root)
        require(actual.get('resources') == report['resources'] and
                actual.get('map_abi') == report['map_abi'],
                'Observed resources differ from actual probe report')
    else:
        require(not any(option in nested['argv'] for option in
                        ('--resource-root', '--map-id', '--map-controls')) and
                'resources' not in actual, 'Unbound resource execution')
    require(actual['identity'] == semantic_input['identity'] and
            actual['input_digest'] == digest(semantic_input) and
            actual['probe_sha256'] == expected_probe_ref['sha256'] and
            actual['library_sha256'] == expected_library_ref['sha256'] and
            actual['result_words'] == result and
            actual['state_bits'] == {name: f'0x{word:08x}' for name, word in zip(STATE, result)} and
            actual['gpu_invocations'] == report['point_count'] * report['repeats'] and
            actual['faults'] == report['fault_count'] and actual['work'] == report['rows'][0]['work'] and
            actual['device'] == report['device'], 'Observer metadata differs from actual raw device result')
    require(all(actual[key] is report[key] for key in
                ('exact_state_repeats', 'allocation_canaries', 'input_immutable')) and
            actual['invalid_descriptor_controls'] == report['invalid_descriptor_controls'],
            'Observer integrity metadata differs from probe')
    return actual


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for option in ('input', 'probe', 'probe-sha256', 'library', 'library-sha256', 'output', 'run-id'):
        parser.add_argument('--' + option, required=True)
    parser.add_argument('--slot', type=int, default=0)
    parser.add_argument('--timeout', type=float, default=30)
    args = parser.parse_args()
    def cancelled(signum, frame):
        # A second cancellation must not interrupt the child's bounded reap.
        signal.signal(signal.SIGTERM, signal.SIG_IGN)
        raise InterruptedError('Observer cancelled by SIGTERM')
    previous = signal.signal(signal.SIGTERM, cancelled)
    try:
        observe(args)
    except (ValueError, KeyError, TypeError, OSError, RuntimeError) as exc:
        print('Native formula observer failed: ' + str(exc), file=sys.stderr)
        return 1
    finally:
        signal.signal(signal.SIGTERM, previous)
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
