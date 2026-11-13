#!/usr/bin/env python3
"""Predeclare/replay bounded old/new SSE transport controls; no admission.

This is a leaf probe runner, not a formula model. In eligible negative-count
orbits the compiled-original epilogue relates the four return lanes to the
unchanged distance and W. It never substitutes GPU values for frozen numerical
reference targets. All other55 words must remain exact, including work/fault.
"""
import argparse
import copy
from pathlib import Path
import struct
import sys
import uuid

import native_formula_affine_v2_reference as affine
import native_formula_translate_context_reference as mixed
import native_formula_observer as observer
import verify_native_formula_group as group
from prepare_gpu_parallel_bundle import write
from qualify_native_metal_all import ROOT, artifact, digest, require, sha
from run_m3parameter_gpu import invoke

AUTHOR = '/root/formula_execution_audit'
OLD_BUILD = 'comparison/runs/native-formula-group05-20261104/candidate-build-v3/build'
OLD_PINS = {'probe': 'a31be1d2ab616f04fb9ef74d65071ff66d114b0def1aad8dbbe3cb1573ab01ae',
            'library': '6c8ecafd0c3987d2123e91b395006355ff79ea195bd7ccce3e69b9a17d628b95',
            'receipt': 'f0f12f4f57b0d86f680fcd11a6d6287381b718147edb98fd68ef6d3d6d17a9d7'}
SSE = tuple(range(20, 24))
OTHER = tuple(i for i in range(59) if i not in SSE)
LIMITATIONS = [
    'Transport/ownership controls only; no formula admission, new numerical target, or scene qualification.',
    'Nine finite original mixed cases retain original400 iterations and allthree slots; pointzero range case remains separate OPEN.',
    'Synthetic caller predicate controls are labelled mutations, never authenticated original scenes.',
    'SSE consumer is enabled but unreachable: tests the conservative metadata exclusion, not consumer arithmetic.',
    'Full-scene RGB/work/effect ownership controls remain separately required.',
    'Run before geometry contraction correction; any nonSSE change fails this fixed comparison.',
]


def ref(path):
    path = Path(path).resolve()
    require(path.is_relative_to(ROOT), 'Artifact outside repository')
    return {'path': str(path.relative_to(ROOT)), 'sha256': sha(path)}


def build_cases():
    """Only source/prepared inputs; no previous GPU captures are consulted."""
    rows = []

    def append(name, p, s, rule='EXACT59', fault=0, work=None, origin='synthetic-caller-unit', slot=0):
        observer.words(p, 2048); observer.words(s, 47); observer.validate_program_scalars(p)
        require(p[0] == 1 and p[2] == 6 and p[6] in observer.ORBIT_OPTIONS[p[1]], 'Invalid caller ABI')
        external, _ = observer.descriptor_metadata()
        for i in range(6):
            at = 56 + 332*i
            require(p[at] == 1 and p[at+9] in (0, 1) and p[at+10:at+12] == [0, 0], 'Bad slot ABI')
            if p[at+9]:
                require(p[at+1] == 1 and p[at+2] in external and
                        tuple(p[at+3:at+8]) == external[p[at+2]] and p[at+8] == 0,
                        'Nonproduction enabled-slot metadata')
        require(s[36] == p[7] and s[37:43] == p[50:56] and s[43:46] == p[8:11], 'Caller/state mismatch')
        rows.append({'id': name, 'origin': origin, 'rule': rule, 'expected_fault': fault,
                     'expected_work': work, 'slot': slot, 'program_words': p, 'input_words': s})

    def base(count=1, maximum=4, stop=None):
        s = affine.state(); s['hybrid_counts[0]'] = count; s['max_iterations'] = maximum
        if stop is not None: s['rstop'] = affine.fp32(stop)
        p = affine.program([affine.binary64(x) for x in (1, -2, 3)], s)
        return p, [s[k] for k in observer.STATE]

    for count in (1, 3):
        p, s = base(count); append(f'positive-de2-count{count}', p, s, work=4)
    p, s = base(stop=16); append('positive-de2-early-exit', p, s, work=1)
    for row in mixed.build_cases():
        diagnostic = row['semantic_diagnostic']
        if diagnostic['status'] == 'OPEN_MODEL_DOMAIN':
            require(row['id'] == 'translate-lightbulbmoon-00', 'Unexpected unmodelled mixed case')
            continue
        p, s = observer.validate_case(row['input'])
        append(row['id'], p, s, 'RETURN_SSE_ONLY', work=diagnostic['expected_work'],
               origin='original-prepared-LightBulbMoon', slot=1)
        rows[-1]['source_slot_visits'] = diagnostic['expected_slot_visits']

    for name in ('disabled-negative', 'outside-negative', 'intmin-negative', 'sse-consumer-unreachable'):
        p, s = base(); at = 56+332
        p[at:at+332] = p[56:56+332]
        p[51] = s[38] = 0x80000001
        if name == 'disabled-negative': p[at+9] = 0; p[5] = 1
        if name == 'intmin-negative': p[51] = s[38] = 0x80000000; p[5] = 1
        if name == 'sse-consumer-unreachable':
            # Never dispatched (end_slot0). Parameter zeros are deliberately
            # unused, not purported prepared ManowarBulb defaults.
            p[at:at+332] = [0]*332
            p[at:at+12] = [1, 1, 10262, *observer.descriptor_metadata()[0][10262], 0, 1, 0, 0]
            p[51] = s[38] = 0
        append(name, p, s, work=7 if name == 'disabled-negative' else 4)
    for name, driver, de in (('other-driver-numeric3d', 0, 0), ('global-de11', 1, 11),
                             ('other-driver-analytic4d', 3, 5)):
        p, s = base(); p[1], p[6] = driver, de
        append(name, p, s, work=4)
    p, s = base(); p[10] = s[45] = 1
    # Exact zero radius uses the low smoothing branch, avoiding logarithms.
    state = dict(zip(observer.STATE, s))
    for i in range(3): state[f'c[{i}]'] = 0
    p = affine.program([affine.binary64(0)]*3, state)
    append('smoothing-low-branch', p, [state[k] for k in observer.STATE], work=4)
    p, s = base(); p[11] = 1
    append('positive-work-budget', p, s, fault=5, work=1)
    original = next(r for r in rows if r['id'] == 'translate-lightbulbmoon-03')
    p, s = copy.deepcopy(original['program_words']), original['input_words'].copy(); p[11] = 1
    append('negative-work-budget-before-return', p, s, fault=5, work=1, slot=1)
    require(len(rows) == 22 and len({r['id'] for r in rows}) == 22, 'Control corpus changed')
    return rows


def dependencies():
    manifest = mixed.source_manifest()
    paths = set(observer.dependency_paths()) | {Path(__file__).resolve(), Path(mixed.__file__).resolve()}
    for section in ('source_hashes', 'helper_hashes', 'context_dependencies', 'data_dependencies'):
        for name, pin in manifest[section].items():
            path = ROOT/name; require(sha(path) == pin, 'Changed source proof dependency'); paths.add(path)
    paths.add(ROOT/'mb3d/M3Formulas/ManowarBulb.m3f')
    for module in list(sys.modules.values()):
        filename = getattr(module, '__file__', None)
        if filename:
            path = Path(filename).resolve()
            if path.parent == ROOT/'comparison/tools' and path.suffix == '.py': paths.add(path)
    return sorted(paths)


def corpus():
    return {'schema': 'mb3d.translate-transport-corpus.v1', 'author': AUTHOR, 'qualification': False,
            'cases': build_cases(), 'limitations': LIMITATIONS,
            'return_rule': 'compiled original epilogue: SSE=[unchanged distance,+0,abs(unchanged W),+0]; no geometry target substitution'}


def prepare(folder):
    folder = Path(folder).resolve()
    require(folder.is_relative_to(ROOT) and not folder.exists(), 'Fresh repository predevice directory required')
    value = corpus(); pins = [ref(p) for p in dependencies()]
    write(folder/'corpus.json', value)
    write(folder/'manifest.json', {'schema': 'mb3d.translate-transport-predevice.v1',
          'qualification': False, 'corpus': ref(folder/'corpus.json'), 'dependencies': pins})
    return ref(folder/'manifest.json')


def predevice(manifest_path, review_path):
    manifest_path, review_path = Path(manifest_path).resolve(), Path(review_path).resolve()
    value = observer.read_json(manifest_path)
    require(value.get('schema') == 'mb3d.translate-transport-predevice.v1' and value.get('qualification') is False,
            'Wrong predevice manifest')
    saved = observer.read_json(artifact(ROOT, value['corpus']))
    require(saved == corpus() and value['dependencies'] == [ref(p) for p in dependencies()], 'Changed predeclared corpus/source closure')
    review = observer.read_json(review_path)
    require(review.get('schema') == 'mb3d.translate-transport-predevice-review.v1' and
            review.get('status') == 'PASS_CONTROLS_ONLY' and review.get('qualification') is False and
            review.get('manifest') == ref(manifest_path) and review.get('author') == AUTHOR and
            isinstance(review.get('reviewer'), str) and bool(review['reviewer'].strip()) and
            review['reviewer'].strip() != AUTHOR, 'Independent predevice review missing/changed')
    return saved['cases'], [artifact(ROOT, p) for p in value['dependencies']] + [
        manifest_path, review_path, artifact(ROOT, value['corpus'])]


def build_binding(build, old=False):
    build = Path(build).resolve()
    require(build.is_relative_to(ROOT), 'Build outside repository')
    paths = {'probe': build/'mb3d_native_formula_probe', 'library': build/'metal_native_scene.metallib',
             'receipt': build.parent/'build-receipt.json'}
    group.validate_build_products(paths['receipt'], paths['probe'], paths['library'])
    if old:
        require(build == ROOT/OLD_BUILD and all(sha(paths[k]) == pin for k, pin in OLD_PINS.items()), 'Wrong frozen baseline')
    records = observer.read_json(paths['receipt'])['records']
    return {k: ref(v) for k, v in paths.items()}, [*paths.values(), *(build.parent/r['path'] for r in records)]


def arguments(binding, folder, pp, sp, case):
    return list(map(str, [ROOT/binding['probe']['path'], '--program', pp, '--states', sp,
           '--library', ROOT/binding['library']['path'], '--output', folder/'raw-probe.json',
           '--mode', 'orbit', '--slot', case['slot'], '--repeats', 3, '--wrapper-policy', 1]))


def parse_report(report, event, case, pp, library):
    # Reuse every ordinary strict check even for a predeclared work-budget
    # fault; normalize only its outcome fields for validation, then recheck the
    # original fault fields and exact exit2. No other fault is admitted.
    require(event['guard'] is None and type(event['exit_code']) is int and
            event['exit_code'] == (2 if case['expected_fault'] else 0), 'Unexpected probe process result')
    raw = copy.deepcopy(report)
    require(isinstance(raw.get('rows'), list) and len(raw['rows']) == 1, 'Missing raw row')
    result = observer.words(raw['rows'][0]['result_words'], 59).copy()
    require(result[47] == case['expected_fault'] and
            type(raw.get('fault_count')) is int and raw['fault_count'] == int(bool(result[47])) and
            type(raw['rows'][0].get('fault')) is int and raw['rows'][0]['fault'] == result[47], 'Unexpected device fault')
    if result[47]:
        require(report['status'] == 'POINT_FAULTS' and result[56] == 0 and result[55] == 0 and
                result[52] == case['program_words'][11] == result[51] == 1 and
                result[48] in (8065, 1509) and result[49] == 0 and result[50] == 1,
                'Unexpected work-budget fault state')
        raw['status'] = 'COMPLETE'; raw['fault_count'] = 0; raw['rows'][0]['fault'] = 0
        raw['rows'][0]['result_words'][47] = 0; raw['rows'][0]['result_words'][56] = 1
    observer.validate_report(raw, case['program_words'], case['input_words'], 'orbit', pp, library, case['slot'])
    require(result[51] == case['expected_work'] and result[52] == case['program_words'][11], 'Work/limit differs from source schedule')
    return result


def compare(case, baseline, candidate):
    observer.words(baseline, 59); observer.words(candidate, 59)
    require(baseline[47] == candidate[47] == case['expected_fault'] and
            baseline[51] == candidate[51] == case['expected_work'], 'Wrong fault/work')
    changed = [i for i in range(59) if baseline[i] != candidate[i]]
    if case['rule'] == 'EXACT59': require(not changed, 'Expected exact full59 old/new result')
    else:
        require(case['rule'] == 'RETURN_SSE_ONLY' and not case['expected_fault'], 'Unknown comparison rule')
        require(all(baseline[i] == candidate[i] for i in OTHER), 'Changed nonSSE result word')
        require(baseline[20:24] == case['input_words'][20:24], 'Baseline negative caller SSE differs from declared preserved entry')
        require(all((candidate[i] & 0x7f800000) != 0x7f800000 for i in (3, 24, 55)) and
                candidate[3] & 0x7fffffff != 0, 'Nonfinite/zero-divisor return is outside this proof')
        require(candidate[20:24] == [baseline[55], 0, baseline[3] & 0x7fffffff, 0], 'Wrong source-bound return SSE')
        require(changed, 'Eligible negative control did not exercise changed transport')
    return {'status': 'PASS_CONTROL', 'changed_result_words': changed,
            'exact_other_words': 55 if case['rule'] == 'RETURN_SSE_ONLY' else 59}


def replay_capture(entry, binding, case, folder, pp, sp, pins, seen):
    require(entry['event'] == ref(folder/'event.json'), 'Wrong process event location')
    event = observer.run_event(ROOT, entry['event'],
                {'path': str(ROOT/binding['probe']['path']), 'sha256': binding['probe']['sha256']},
                success=not case['expected_fault'], max_seconds=180)
    require(event['argv'] == arguments(binding, folder, pp, sp, case) and event['cwd'] == str(ROOT), 'Wrong executed command')
    key = (event['pid'], event['started_ns']); run = event['run_id']
    require(key not in seen and run not in seen, 'Reused actual process'); seen.update((key, run))
    require(all(p in event['inputs'] for p in [*pins, ref(pp), ref(sp)]), 'Missing captured input/closure')
    require(pp.read_bytes() == struct.pack('<2048I', *case['program_words']) and
            sp.read_bytes() == struct.pack('<47I', *case['input_words']), 'Wrong uploaded bytes')
    raw = ref(folder/'raw-probe.json')
    require(entry['raw'] == raw and raw in event['outputs'], 'Unbound raw report')
    return parse_report(observer.read_json(ROOT/raw['path']), event, case, pp, ROOT/binding['library']['path'])


def collect(manifest_path, review_path, candidate_build, output):
    cases, inputs = predevice(manifest_path, review_path)
    output = Path(output).resolve()
    require(output.is_relative_to(ROOT) and not output.exists(), 'Fresh repository output required')
    bindings = {}
    for side, path in (('baseline', ROOT/OLD_BUILD), ('candidate', candidate_build)):
        bindings[side], extra = build_binding(path, side == 'baseline'); inputs += extra
    require(all(bindings['baseline'][key]['sha256'] != bindings['candidate'][key]['sha256']
                for key in ('probe', 'library')), 'Old/new builds must be distinct')
    inputs = sorted(set(inputs)); pins = [ref(p) for p in inputs]
    result = {'schema': 'mb3d.translate-transport-controls.v1', 'status': 'RUNNING',
              'qualification': False, 'verified_formulas': 0, 'evidence_origin': 'actual_device_capture',
              'manifest': ref(manifest_path), 'review': ref(review_path), 'bindings': bindings,
              'dependencies_before': pins, 'cases': [], 'limitations': LIMITATIONS}
    seen = set()
    try:
        write(output/'transport-controls.json', result)
        for case in cases:
            folder = output/'cases'/case['id']; folder.mkdir(parents=True)
            pp, sp = folder/'program.bin', folder/'state.bin'
            pp.write_bytes(struct.pack('<2048I', *case['program_words'])); sp.write_bytes(struct.pack('<47I', *case['input_words']))
            row = {'id': case['id'], 'captures': {}}; result['cases'].append(row); values = {}
            for side, binding in bindings.items():
                row['captures'][side] = []; values[side] = []
                for repeat in range(3):
                    require([ref(p) for p in inputs] == pins, 'Captured closure changed before process')
                    run = folder/side/f'run-{repeat+1}'
                    invoke(arguments(binding, run, pp, sp, case), run, ROOT, 180, evidence_root=ROOT,
                           input_files=[*inputs, pp, sp], exclusive_gpu=True, run_id=uuid.uuid4().hex)
                    entry = {'event': ref(run/'event.json'), 'raw': ref(run/'raw-probe.json')}
                    row['captures'][side].append(entry)
                    values[side].append(replay_capture(entry, binding, case, run, pp, sp, pins, seen))
                    require([ref(p) for p in inputs] == pins, 'Captured closure changed after process')
                    write(output/'transport-controls.json', result)
                require(values[side] == [values[side][0]]*3, 'Fresh-process full59 repeats differ')
            try: row['comparison'] = compare(case, values['baseline'][0], values['candidate'][0])
            except ValueError as error: row['comparison'] = {'status': 'CONTROL_FAILED', 'error': str(error)}
            write(output/'transport-controls.json', result)
            print(f"{len(result['cases'])}/{len(cases)} {case['id']}: {row['comparison']['status']}; verified formulas0", flush=True)
        result['status'] = 'PASS_CONTROLS_ONLY' if all(r['comparison']['status'] == 'PASS_CONTROL' for r in result['cases']) else 'CONTROLS_FAILED'
    except Exception as error:
        result.update(status='COLLECTION_ABORTED', error=str(error)); raise
    finally:
        result['dependencies_after'] = [ref(p) for p in inputs if p.is_file()]
        write(output/'transport-controls.json', result)
    if result['status'] == 'PASS_CONTROLS_ONLY': validate(output/'transport-controls.json')
    return result


def validate(path):
    path = Path(path).resolve(); data = observer.read_json(path)
    require(data.get('schema') == 'mb3d.translate-transport-controls.v1' and data.get('status') == 'PASS_CONTROLS_ONLY' and
            data.get('qualification') is False and data.get('verified_formulas') == 0 and
            data.get('evidence_origin') == 'actual_device_capture', 'Not a completed transport control capture')
    cases, inputs = predevice(artifact(ROOT, data['manifest']), artifact(ROOT, data['review']))
    require(set(data['bindings']) == {'baseline', 'candidate'} and [r['id'] for r in data['cases']] == [r['id'] for r in cases], 'Incomplete/reordered capture')
    require(all(data['bindings']['baseline'][key]['sha256'] != data['bindings']['candidate'][key]['sha256']
                for key in ('probe', 'library')), 'Old/new builds must be distinct')
    for side, binding in data['bindings'].items():
        observed, extra = build_binding(artifact(ROOT, binding['probe']).parent, side == 'baseline')
        require(binding == observed, 'Changed build binding'); inputs += extra
    pins = [ref(p) for p in sorted(set(inputs))]
    require(data['dependencies_before'] == data['dependencies_after'] == pins, 'Changed/missing captured closure')
    seen = set()
    for case, row in zip(cases, data['cases']):
        folder = path.parent/'cases'/case['id']; pp, sp = folder/'program.bin', folder/'state.bin'
        require(set(row['captures']) == {'baseline', 'candidate'}, 'Wrong capture sides')
        values = {}
        for side, binding in data['bindings'].items():
            entries = row['captures'][side]; require(len(entries) == 3, 'Three fresh processes required per side')
            values[side] = [replay_capture(e, binding, case, folder/side/f'run-{i+1}', pp, sp, pins, seen) for i, e in enumerate(entries)]
            require(values[side] == [values[side][0]]*3, 'Fresh-process full59 repeats differ')
        require(row['comparison'] == compare(case, values['baseline'][0], values['candidate'][0]), 'Unreplayed comparison')
    return {'status': 'PASS_CONTROLS_ONLY', 'cases': len(cases), 'actual_processes': len(seen)//2,
            'qualification': False, 'verified_formulas': 0}


def main():
    parser = argparse.ArgumentParser(description=__doc__); commands = parser.add_subparsers(dest='command', required=True)
    p = commands.add_parser('prepare'); p.add_argument('--output', type=Path, required=True)
    p = commands.add_parser('collect')
    for key in ('manifest', 'review', 'candidate-build', 'output'): p.add_argument('--'+key, type=Path, required=True)
    p = commands.add_parser('validate'); p.add_argument('--capture', type=Path, required=True)
    args = parser.parse_args()
    if args.command == 'prepare': print(prepare(args.output))
    elif args.command == 'validate': print(validate(args.capture))
    else: return 0 if collect(args.manifest, args.review, args.candidate_build, args.output)['status'] == 'PASS_CONTROLS_ONLY' else 1
    return 0


if __name__ == '__main__': sys.exit(main())
