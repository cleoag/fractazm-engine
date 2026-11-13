#!/usr/bin/env python3
"""Collect one fixed mixed-context diagnostic; never admit or qualify formulas.

Ten source-declared cases, one unchanged-probe baseline and three fresh observer
processes each. Unproved fields and domain failures remain OPEN. This is a leaf
diagnostic using the existing process/GPU lock and build/observer validators.
"""
import argparse
from collections import Counter
import json
from pathlib import Path
import struct
import sys
import uuid

import native_formula_observer as observer
import native_formula_translate_context_reference as model
import verify_native_formula_group as group
from gpu_pipeline_proofs import option
from prepare_gpu_parallel_bundle import write
from qualify_native_metal_all import ROOT, require, sha, digest
from run_m3parameter_gpu import invoke


def ref(path):
    path = Path(path).resolve()
    require(path.is_relative_to(ROOT), 'Diagnostic artifact outside repository')
    return {'path': str(path.relative_to(ROOT)), 'sha256': sha(path)}


def dependency_closure(manifest, cases):
    paths = {Path(__file__).resolve(), Path(model.__file__).resolve()}
    for section in ('source_hashes', 'helper_hashes', 'context_dependencies', 'data_dependencies'):
        records = manifest.get(section, {}) if section == 'data_dependencies' else manifest[section]
        require(isinstance(records, dict), 'Malformed model dependency section')
        for name, expected in records.items():
            path = (ROOT / name).resolve()
            require(path.is_relative_to(ROOT) and sha(path) == expected, 'Model source/data changed')
            paths.add(path)
    for case in cases:
        paths.update(observer.dependency_paths(case['input']))
    # Capture the imported repository helper closure, including the reused build
    # validator and its dependencies. No external Python module is copied.
    for module in list(sys.modules.values()):
        filename = getattr(module, '__file__', None)
        if filename:
            path = Path(filename).resolve()
            if path.parent == ROOT / 'comparison/tools' and path.suffix == '.py':
                paths.add(path)
    return sorted(paths)


def raw_capture(path, event, program, state, program_path, library):
    """Fault reports stay diagnostic data; only COMPLETE uses strict validation."""
    require(path.is_file() and ref(path) in event['outputs'], 'Missing/unbound raw probe report')
    report = observer.read_json(path)
    if report.get('status') == 'COMPLETE':
        words = observer.validate_report(report, program, state, 'orbit', program_path, library, 1)
        require(event['exit_code'] == 0 and event['guard'] is None, 'Failed complete probe process')
    else:
        require(report.get('schema') == 'mb3d.native-formula-probe.v1' and
                report.get('status') == 'POINT_FAULTS' and report.get('profile') == 'native_gpu_fp32' and
                report.get('program') == str(program_path) and report.get('library') == str(library) and
                report.get('program_words') == program and report.get('mode') == 1 and report.get('slot') == 1 and
                report.get('point_count') == 1 and report.get('repeats') == 3 and
                report.get('wrapper_policy') == 1 and report.get('fault_count') == 1 and
                len(report.get('rows', [])) == 1, 'Unbound/malformed fault diagnostic')
        require(report.get('library_profile') == [2, 4, 32] and
                report.get('formula_abi') == [1, 8192, 188, 236, 32] and report.get('scene') is None and
                all(report.get(key) is True for key in ('exact_state_repeats', 'input_immutable', 'allocation_canaries')) and
                type(report.get('invalid_descriptor_controls')) is int and report['invalid_descriptor_controls'] == 7 and
                isinstance(report.get('device'), str) and bool(report['device'].strip()), 'Fault diagnostic integrity failed')
        audit = observer.words(report['wrapper_audit_words'], 18)
        require(all(audit[i:i+2] == [1, 1] for i in range(0, 18, 3)), 'Fault wrapper audit failed')
        row = report['rows'][0]
        words = observer.words(row['result_words'], 59)
        require(row.get('index') == 0 and row.get('input_words') == state and
                row.get('fault') == words[47] != 0 and row.get('work') == words[51] and
                event['exit_code'] != 0 and event['guard'] is None, 'Inconsistent fault diagnostic')
    return {'raw': ref(path), 'report_status': report['status'], 'result_words': words,
            'work': report['rows'][0]['work'], 'fault': report['rows'][0]['fault']}


def collect_context(build, output, model_review):
    build, output = Path(build).resolve(), Path(output).resolve()
    require(build.is_relative_to(ROOT) and output.is_relative_to(ROOT) and not output.exists(),
            'Frozen repository build and fresh diagnostic output required')
    probe, library = build / 'mb3d_native_formula_probe', build / 'metal_native_scene.metallib'
    receipt = build.parent / 'build-receipt.json'
    group.validate_build_products(receipt, probe, library)
    manifest, cases = model.source_manifest(), model.build_cases()
    model_review = Path(model_review).resolve()
    reviewed = observer.read_json(model_review)
    require(reviewed.get('schema') == 'mb3d.native-formula-context-model-review.v1' and
            reviewed.get('status') == 'PASS_SCOPED_SOURCE_MODEL' and reviewed.get('qualification') is False and
            all(isinstance(reviewed.get(key), str) and reviewed[key].strip()
                for key in ('reviewer', 'model_author')) and
            reviewed['reviewer'].strip() != reviewed['model_author'].strip() and
            reviewed.get('reference_model') == ref(Path(model.__file__)) and
            reviewed.get('reference_manifest_sha256') == digest(manifest) and
            reviewed.get('corpus_sha256') == digest(cases) and
            reviewed.get('reviewed_case_ids') == [c['id'] for c in cases],
            'Missing/changed independent pre-device context model review')
    require(len(cases) == 10 and len({c['id'] for c in cases}) == 10, 'Expected ten predeclared cases')
    for case in cases:
        require(case['id'] and all(c.isalnum() or c in '-_' for c in case['id']), 'Unsafe case ID')
        observer.validate_case(case['input'])
        diagnostic = case['semantic_diagnostic']
        known = diagnostic.get('expected_state_bits', {})
        require(set(known) <= set(observer.STATE), 'Unknown model state fields')
        if known:
            require(set(known) in (set(model.KNOWN_FIELDS), set(observer.STATE)) and
                    set(known) | set(diagnostic['unproved_fields']) == set(observer.STATE) and
                    not set(known) & set(diagnostic['unproved_fields']), 'Incomplete model field disposition')
    inputs = sorted({probe, library, receipt, model_review, *dependency_closure(manifest, cases)})
    before = [ref(p) for p in inputs]
    result = {'schema': 'mb3d.native-formula-context-diagnostic-collection.v1', 'status': 'RUNNING',
              'qualification': False, 'verified_formulas': 0, 'evidence_origin': 'actual_device_capture',
              'reference_manifest': manifest, 'model_review': ref(model_review), 'dependencies_before': before,
              'probe': ref(probe), 'library': ref(library), 'build_receipt': ref(receipt), 'cases': []}
    output.mkdir(parents=True)
    write(output / 'source-corpus.json', cases)
    result['source_corpus'] = ref(output / 'source-corpus.json')
    inputs.append(output / 'source-corpus.json')
    pins = [ref(p) for p in inputs]
    result['dependencies_before'] = pins
    adapter, processes = Path(observer.__file__).resolve(), set()

    def unchanged():
        require([ref(p) for p in inputs] == pins and model.source_manifest() == manifest,
                'Diagnostic source/helper/data/build closure changed')

    def capture(argv, folder, extra, run_id=None):
        unchanged()
        run_id = run_id or uuid.uuid4().hex
        event = invoke(argv, folder, ROOT, 180, evidence_root=ROOT,
                       input_files=[*inputs, *extra], exclusive_gpu=True, run_id=run_id)
        event = observer.run_event(ROOT, ref(folder / 'event.json'),
                                   {'path': str(argv[0]), 'sha256': sha(argv[0])},
                                   success=event['exit_code'] == 0, max_seconds=180)
        identity = (event['pid'], event['started_ns'])
        require(identity not in processes and event['owned_process_reaped'] is True and
                event['run_id'] == run_id and event['argv'] == list(map(str, argv)),
                'Duplicate/unreaped diagnostic process')
        processes.add(identity)
        require(all(p in event['inputs'] for p in pins), 'Uncaptured diagnostic closure')
        unchanged()
        return event

    try:
        write(output / 'diagnostic.json', result)
        for index, original in enumerate(cases):
            folder = output / 'cases' / original['id']; folder.mkdir(parents=True)
            semantic, diagnostic = original['input'], original['semantic_diagnostic']
            inp = folder / 'input.json'; write(inp, semantic)
            program, state = observer.validate_case(semantic)
            pp, sp = folder / 'program.bin', folder / 'state.bin'
            pp.write_bytes(struct.pack('<2048I', *program)); sp.write_bytes(struct.pack('<47I', *state))
            expected = diagnostic.get('expected_state_bits', {})
            row = {'id': original['id'], 'identity': original['identity'], 'input': ref(inp),
                   'model_diagnostic': diagnostic, 'compared_fields': sorted(expected),
                   'unproved_fields': diagnostic.get('unproved_fields', dict.fromkeys(observer.STATE, 'OPEN_MODEL_DOMAIN')),
                   'status': 'RUNNING', 'repeats': []}
            result['cases'].append(row)
            base_folder = folder / 'baseline'; raw = base_folder / 'raw-probe.json'
            argv = [probe, '--program', pp, '--states', sp, '--library', library, '--output', raw,
                    '--mode', 'orbit', '--slot', '1', '--repeats', '3', '--wrapper-policy', '1']
            event = capture(argv, base_folder, [inp, pp, sp])
            row['baseline'] = {'event': ref(base_folder / 'event.json')}
            try:
                row['baseline'].update(raw_capture(raw, event, program, state, pp, library))
            except (ValueError, KeyError, TypeError, OSError) as exc:
                row['baseline']['error'] = str(exc)
            for repeat in range(3):
                run = folder / f'run-{repeat+1}'; observed = run / 'observation.json'
                run_id = uuid.uuid4().hex
                argv = [adapter, '--input', inp, '--probe', probe, '--probe-sha256', result['probe']['sha256'],
                        '--library', library, '--library-sha256', result['library']['sha256'],
                        '--output', observed, '--run-id', run_id, '--timeout', '150']
                event = capture(argv, run, [inp], run_id)
                entry = {'event': ref(run / 'event.json')}; row['repeats'].append(entry)
                try:
                    require(event['guard'] is None, 'Guarded observer execution')
                    nested = run / 'observation.probe/process'
                    child = observer.read_json(nested / 'event.json')
                    child = observer.run_event(ROOT, ref(nested / 'event.json'),
                                               {'path': str(probe), 'sha256': result['probe']['sha256']},
                                               success=child['exit_code'] == 0, max_seconds=180)
                    child_id = (child['pid'], child['started_ns'])
                    require(child_id not in processes and child['pid'] != event['pid'] and
                            child['run_id'] != event['run_id'] and
                            event['started_ns'] <= child['started_ns'] < child['finished_ns'] <= event['finished_ns'],
                            'Unbound/reused nested probe process')
                    processes.add(child_id)
                    expected_args = {'--program': str(nested.parent / 'program.bin'), '--states': str(nested.parent / 'state.bin'),
                                     '--library': str(library), '--output': str(nested / 'raw-probe.json'),
                                     '--mode': 'orbit', '--slot': '1', '--repeats': '3', '--wrapper-policy': '1'}
                    require(all(option(child['argv'], key) == value for key, value in expected_args.items()) and
                            (nested.parent / 'program.bin').read_bytes() == pp.read_bytes() and
                            (nested.parent / 'state.bin').read_bytes() == sp.read_bytes() and
                            all(ref(p) in child['inputs'] for p in observer.dependency_paths(semantic)),
                            'Nested probe input/dependency changed')
                    entry.update(raw_capture(nested / 'raw-probe.json', child, program, state,
                                             run / 'observation.probe/program.bin', library))
                    entry['probe_event'] = ref(nested / 'event.json')
                    require(event['exit_code'] == (1 if entry['fault'] else 0), 'Unexpected observer exit')
                    if entry['fault'] == 0:
                        actual = observer.replay_observation(ROOT, ref(observed), entry['event'], semantic,
                                                             result['probe'], result['library'])
                        entry['observation'] = ref(observed)
                        entry['differences'] = {k: {'expected': v, 'actual': actual['state_bits'].get(k)}
                                                for k, v in expected.items() if actual['state_bits'].get(k) != v}
                        entry['work_matches_model'] = not expected or actual['work'] == diagnostic['expected_work']
                except (ValueError, KeyError, TypeError, OSError) as exc:
                    entry['error'] = str(exc)
                write(output / 'diagnostic.json', result)
            captures = [row['baseline'], *row['repeats']]
            valid = all('result_words' in item and 'error' not in item for item in captures)
            same = valid and all(item['result_words'] == captures[0]['result_words'] and
                                 item['work'] == captures[0]['work'] for item in captures[1:])
            row['observation_noninterference'] = 'EXACT_RAW59_AND_WORK' if same else 'OPEN'
            row['status'] = ('DEVICE_EXECUTION_FAILED' if not valid else 'OBSERVATION_CHANGED' if not same else
                             'OPEN_MODEL_DOMAIN' if not expected else 'DEVICE_FAULT' if captures[0]['fault'] else
                             'MODEL_STATE_MISMATCH' if any(r['differences'] or not r['work_matches_model'] for r in row['repeats']) else
                             'OPEN_CALLER_FIELDS' if row['unproved_fields'] else 'SCOPED_DIAGNOSTIC_MATCH_ONLY')
            write(output / 'diagnostic.json', result)
            print(f"{index+1}/10 ({(index+1)*10}%) {row['id']}: {row['status']}; verified formulas 0", flush=True)
        unchanged()
        group.validate_build_products(receipt, probe, library)
        result['status'] = 'DIAGNOSTIC_ONLY'
    except Exception as exc:
        result.update(status='COLLECTION_ABORTED', error=str(exc))
        raise
    finally:
        result['dependencies_after'] = [ref(p) for p in inputs if p.is_file()]
        result['case_counts'] = dict(Counter(row['status'] for row in result['cases']))
        write(output / 'diagnostic.json', result)
    return result


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--build', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--model-review', type=Path, required=True)
    args = parser.parse_args()
    collect_context(args.build, args.output, args.model_review)


if __name__ == '__main__':
    main()
