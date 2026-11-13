"""Strict passive proof of five bounded Translate mixed/scene controls.

No dispatch, build, repair, dynamic model loading or formula admission. A proof
record names evidence; its status is computed from raw processes and source
models. Historical scene snapshots are not waived when tooling changes.
"""
from pathlib import Path
import argparse
import json
import struct
from unittest.mock import patch

import build_m3parameter_main as build_validator
import diagnose_native_formula_context as collector
import gpu_pipeline_binding as binding
import gpu_pipeline_frames as frames
import native_cpu_reference as cpu
import native_formula_context_input as context_input
import native_formula_coverage as coverage
import native_formula_observer as observer
import native_formula_translate_context_reference as model
import qualify_gpu_parallel_pipeline as gate
import verify_native_formula_group as group
from gpu_pipeline_proofs import STATE
from qualify_native_metal_all import ROOT, require, sha, digest, receipt, source_snapshot

SCHEMA = 'mb3d.translate-mixed-scene-controls.v1'
SUPPORTED = (
    'translate-realized-parameter-binding', 'translate-lightbulbmoon-mixed-context',
    'translate-mixed-carried-state', 'translate-negative-mixed-caller',
    'translate-general-rounded-radius')
OPEN = ('translate-other-caller-profiles', 'translate-actual-range-nonfinite-controls')
SCENE = 'ecc0b336d49fa5e9'
DECLARATION_SHA = 'dac95b087c2c45ba54f19d72928875605a74ecdb492dd327993010eb9e045e93'
MODEL_REVIEW_SHA = '7530085d77e5fa3fe191c0bb0f027c1aa621024630a0943dd329d593a685bed8'
CONTRACT_PATH = 'comparison/contracts/native-gpu-acceptance-v4/acceptance-contract.json'


def tool_inputs(root=ROOT):
    modules = (build_validator, collector, binding, frames, cpu, context_input,
               coverage, observer, model, gate, group)
    paths = {Path(__file__).resolve(), *(Path(m.__file__).resolve() for m in modules)}
    return [coverage.ref(root, p) for p in sorted(paths)]


class Replay:
    def __init__(self, root, proof_path):
        self.root = Path(root).resolve()
        self.proof_path = Path(proof_path).resolve()
        self.pins, self.events, self.processes, self.run_ids = {}, set(), set(), set()

    def bind(self, record):
        coverage.fields(record, 'path sha256', 'control artifact')
        require(isinstance(record['path'], str) and not Path(record['path']).is_absolute(), 'Absolute control artifact')
        path = (self.root / record['path']).resolve()
        require(path.is_relative_to(self.root) and path != self.proof_path, 'Escaping/circular control artifact')
        require(coverage.ref(self.root, path) == record, 'Changed control artifact: ' + record['path'])
        self.pins[record['path']] = record['sha256']
        return path

    def read(self, record, schema=None):
        value = observer.read_json(self.bind(record))
        if schema:
            require(isinstance(value, dict) and value.get('schema') == schema, 'Unknown control evidence schema')
        return value

    def refs(self, value):
        if isinstance(value, dict):
            if set(value) == {'path', 'sha256'}:
                self.bind(value)
            else:
                for v in value.values(): self.refs(v)
        elif isinstance(value, list):
            for v in value: self.refs(v)

    def event(self, record, executable, scene=False):
        require(record['path'] not in self.events, 'Reused event file')
        value = observer.run_event(self.root, record,
            {'path': str(self.bind(executable)), 'sha256': executable['sha256']},
            max_seconds=300 if scene else 180)
        require(value['seconds'] <= 180, 'Diagnostic time threshold exceeded')
        process = value['pid'], value['started_ns']
        require(process not in self.processes and value['run_id'] not in self.run_ids, 'Reused process/run identity')
        self.processes.add(process); self.run_ids.add(value['run_id']); self.events.add(record['path'])
        self.bind(record); self.refs(value)
        return value

    def stable(self):
        for path, expected in list(self.pins.items()):
            self.bind({'path': path, 'sha256': expected})


def exact_state(actual, raw, item, expected):
    require(len(actual['result_words']) == 59 and
            actual['result_words'] == raw['result_words'] == item['result_words'] and
            actual['work'] == raw['work'] == item['work'] and
            actual['faults'] == raw['fault'] == item['fault'] == 0,
            'Mixed raw59/work/noninterference mismatch')
    require(isinstance(actual.get('device'), str) and actual['device'].strip() and
            'synthetic' not in actual['device'].lower(), 'Synthetic/unmarked device evidence')
    if 'expected_state_bits' in expected:
        require(set(expected['expected_state_bits']) == set(STATE) and
                actual['state_bits'] == expected['expected_state_bits'] and
                actual['work'] == expected['expected_work'] and
                item['differences'] == {} and item['work_matches_model'] is True,
                'Mixed original full47/work mismatch')


def validate_header(root, proof):
    coverage.fields(proof, 'schema identity declaration context_id obligation_ids candidate mixed_collection scene_evidence scene_contract tool_inputs evidence_origin', 'mixed controls proof')
    require(proof['schema'] == SCHEMA and proof['identity'] == context_input.IDENTITY and
            proof['context_id'] == context_input.CONTEXT_ID, 'Unsupported control schema/identity/context')
    require(proof['evidence_origin'] == 'actual_device_capture', 'Synthetic/unmarked controls forbidden')
    require(proof['obligation_ids'] == list(SUPPORTED), 'Unsupported/omitted control obligation')
    require(proof['declaration']['sha256'] == DECLARATION_SHA, 'Unreviewed declaration revision')
    coverage.fields(proof['candidate'], 'probe library observer build_receipt renderer', 'control candidate')
    require(proof['candidate']['observer'] == coverage.ref(root, Path(observer.__file__)), 'Observer identity differs')
    require(proof['tool_inputs'] == tool_inputs(root), 'Stale control validator tools')
    require(proof['scene_contract'] == coverage.ref(root, Path(root) / CONTRACT_PATH), 'Wrong ordinary contract')


def validate_direct_inputs(a, event, row, candidate, program, state, program_path):
    argv = event['argv']
    state_path = Path(argv[argv.index('--states') + 1])
    raw_path = a.bind(row['baseline']['raw'])
    require(argv == [str(a.bind(candidate['probe'])), '--program', str(program_path),
                    '--states', str(state_path), '--library', str(a.bind(candidate['library'])),
                    '--output', str(raw_path), '--mode', 'orbit', '--slot', '1',
                    '--repeats', '3', '--wrapper-policy', '1'], 'Direct capture arguments differ')
    for path, values, size in ((program_path, program, 2048), (state_path, state, 47)):
        ref = coverage.ref(a.root, path)
        require(ref in event['inputs'] and a.bind(ref).read_bytes() == struct.pack(f'<{size}I', *values),
                'Direct capture missing/changed prepared input bytes')
    require(row['input'] in event['inputs'], 'Direct capture missing semantic input')


def validate_mixed(a, proof):
    data = a.read(proof['mixed_collection'], 'mb3d.native-formula-context-diagnostic-collection.v1')
    require(data['status'] == 'DIAGNOSTIC_ONLY' and data['qualification'] is False and
            data['verified_formulas'] == 0 and data['evidence_origin'] == 'actual_device_capture', 'Wrong mixed origin/credit')
    candidate = proof['candidate']
    require(all(data[k] == candidate[k] for k in ('probe', 'library', 'build_receipt')), 'Mismatched mixed candidate')
    a.refs(data)
    source, manifest = model.build_cases(), model.source_manifest()
    review = a.read(data['model_review'], 'mb3d.native-formula-context-model-review.v1')
    require(data['model_review']['sha256'] == MODEL_REVIEW_SHA and
            review['reviewer'] != review['model_author'] and review['status'] == 'PASS_SCOPED_SOURCE_MODEL' and
            review['qualification'] is False, 'Unreviewed mixed source model')
    require(coverage.ref(a.root, Path(model.__file__)) == review['reference_model'] and
            manifest == data['reference_manifest'] and digest(manifest) == review['reference_manifest_sha256'] and
            digest(source) == review['corpus_sha256'] and source == a.read(data['source_corpus']) and
            [c['id'] for c in source] == [c['id'] for c in data['cases']] == review['reviewed_case_ids'],
            'Changed original corpus/model/targets')
    required = [review['reference_model'], data['model_review'], data['source_corpus'],
                candidate['observer'], candidate['probe'], candidate['library'], candidate['build_receipt']]
    required += [dict(path=p, sha256=h) for section in
                 ('source_hashes', 'helper_hashes', 'context_dependencies', 'data_dependencies')
                 for p, h in manifest[section].items()]
    required += [coverage.ref(a.root, Path(collector.__file__))]
    require(data['dependencies_before'] == data['dependencies_after'] and
            all(r in data['dependencies_before'] for r in required), 'Missing/stale original dependency closure')
    finite, visits, carries = [], [], []
    for original, row in zip(source, data['cases'], strict=True):
        payload, expected = original['input'], original['semantic_diagnostic']
        require(a.read(row['input']) == payload and row['identity'] == original['identity'] and
                row['model_diagnostic'] == expected and len(row['repeats']) == 3, 'Wrong input/target/repeat binding')
        program, state = observer.validate_case(payload)
        context_input.validate(payload, program, state, a.root)
        baseline = a.event(row['baseline']['event'], candidate['probe'])
        pp = Path(baseline['argv'][baseline['argv'].index('--program') + 1])
        validate_direct_inputs(a, baseline, row, candidate, program, state, pp)
        raw = collector.raw_capture(a.bind(row['baseline']['raw']), baseline, program, state, pp, a.bind(candidate['library']))
        require(all(raw[k] == row['baseline'][k] for k in raw), 'Summary differs from actual direct capture')
        require(all(r in baseline['inputs'] for r in data['dependencies_before']), 'Direct capture missing fresh source inputs')
        raw_device = a.read(row['baseline']['raw']).get('device')
        require(isinstance(raw_device, str) and raw_device.strip() and
                'synthetic' not in raw_device.lower(), 'Synthetic/unmarked direct device evidence')
        for item in row['repeats']:
            actual = observer.replay_observation(a.root, item['observation'], item['event'], payload, candidate['probe'], candidate['library'])
            outer = a.event(item['event'], candidate['observer'])
            a.event(actual['probe_event'], candidate['probe'])
            require(all(r in outer['inputs'] for r in data['dependencies_before']), 'Observer missing fresh source inputs')
            require(actual['probe_event'] == item['probe_event'], 'Copied probe identity differs')
            a.refs(actual); a.read(item['observation'])
            require(actual['device'] == raw_device, 'Direct/observed devices differ')
            exact_state(actual, raw, item, expected)
        known = 'expected_state_bits' in expected
        require(row['compared_fields'] == sorted(expected.get('expected_state_bits', {})) and
                row['unproved_fields'] == expected.get('unproved_fields', dict.fromkeys(STATE, 'OPEN_MODEL_DOMAIN')),
                'Misrepresented observed/unproved field scope')
        require(row['status'] == ('SCOPED_DIAGNOSTIC_MATCH_ONLY' if known else 'OPEN_MODEL_DOMAIN') and
                row['observation_noninterference'] == 'EXACT_RAW59_AND_WORK', 'Wrong numerical scope disposition')
        if known:
            finite.append(row['id'])
            if all(expected['expected_slot_visits'][i] for i in (0, 1, 2)):
                context_input.validate_schedule(expected); visits.append(row['id'])
            if 'carry' in original: carries.append(row['id'])
    require(len(source) == 10 and len(finite) == 9 and len(visits) == 6 and len(carries) == 2 and
            data['case_counts'] == {'OPEN_MODEL_DOMAIN': 1, 'SCOPED_DIAGNOSTIC_MATCH_ONLY': 9}, 'Mixed coverage incomplete')
    return {'finite_case_ids': finite, 'neighbor_schedule_case_ids': visits, 'carried_case_ids': carries,
            'open_case_ids': [r['id'] for r in data['cases'] if r['status'] == 'OPEN_MODEL_DOMAIN']}


def validate_scene(a, proof):
    path = a.bind(proof['scene_evidence'])
    require(path.name == 'evidence.json', 'Scene evidence must be common gate package')
    ev = a.read(proof['scene_evidence'], 'mb3d.gpu-parallel-evidence.v2'); a.refs(ev)
    candidate = proof['candidate']
    require(ev['artifact_root'] == 'repository' and ev['binding']['renderer'] == candidate['renderer'] and
            ev['binding']['library'] == candidate['library'], 'Mixed/scene candidate differs')
    contract_path = a.bind(proof['scene_contract'])
    contract, formulas, scenes = gate.scope(contract_path)
    outcome = gate.evaluate(contract_path, path.parent)
    require(not outcome['errors'] and ['scenes', SCENE] in outcome['accepted'], 'Current common scene gate failed')
    ctx = binding.for_scene(binding.core_context(a.root, ev['binding'], contract, formulas, scenes, a.root), SCENE)
    scene = scenes[SCENE]
    require(scene['settings']['size'] == [600, 330] and ctx['threads'] == 6 and
            ctx['selected_map_ids'] == [], 'Changed original scene/settings/resources')
    rows = [r for r in ev['scenes'] if r['id'] == SCENE]
    require(len(rows) == 1, 'Missing/duplicate original fullscene')
    row = rows[0]
    effects_receipt = receipt(a.root, row['effect_and_resource_proof'], 'scene_effects_resources', digest(ev['binding']))
    a.refs(effects_receipt); effects = a.read(effects_receipt['observations']); a.refs(effects)
    frame_ids = frames.scene_frames(a.root, row, scene, ctx, effects)
    require(len(row['frames']) == 3 and len(effects['controls']) == 3 and
            {r['stage'] for r in effects['controls']} == {'ao', 'shadows', 'reflections'} and
            len(frame_ids) == 6, 'Missing full repeats/active effect controls')
    for frame in [*row['frames'], *(c['off'] for c in effects['controls'])]:
        event = a.event(frame['event'], candidate['renderer'], scene=True)
        require(event['resource_files_before'] == event['resource_files_after'] == ctx['runtime_files'], 'Resource drift')
    require(cpu.saved_reference(a.root, a.root / row['cpu_reference_origin']['package'], scene, ctx) ==
            (row['cpu_reference'], row['cpu_reference_origin']), 'Invalid independently bound CPU reference reuse')
    require(source_snapshot(a.root) == ev['binding']['source'], 'Source changed during scene replay')
    return {'scene_id': SCENE, 'full_frames': 3, 'effect_controls': 3, 'common_gate': outcome}


def validate(root, proof_record):
    root = Path(root).resolve()
    require(root == ROOT.resolve(), 'Only current repository original/model closure supported')
    proof_path = root / proof_record['path']
    require(coverage.ref(root, proof_path) == proof_record, 'Changed proof record')
    proof = observer.read_json(proof_path)
    validate_header(root, proof)
    a = Replay(root, proof_path); a.refs(proof)
    declaration, obligations, _, contexts, _ = coverage.validate_declaration(root, proof['declaration'])
    require(declaration['identity'] == proof['identity'] and set(contexts) == {context_input.CONTEXT_ID} and
            {k for k, r in obligations.items() if r['evidence_kind'] == 'actual_control'} == set(SUPPORTED + OPEN),
            'Changed control declaration scope')
    candidate = proof['candidate']; receipt_path = a.bind(candidate['build_receipt'])
    captured = a.read(candidate['build_receipt'])
    for executable in ('probe', 'renderer'):
        group.validate_build_products(receipt_path, a.bind(candidate[executable]), a.bind(candidate['library']))
    computed = []
    with patch.object(build_validator, 'write', side_effect=lambda p, v: computed.append((p, v))):
        build_validator.validate(receipt_path.parent, captured['targets'])
    require(computed == [(receipt_path, captured)], 'Captured build dependency/producer closure changed')
    mixed = validate_mixed(a, proof)
    scene = validate_scene(a, proof)
    require(len(a.events) == len(a.processes) == len(a.run_ids) == 76, 'Wrong fresh process closure')
    a.stable()
    require(coverage.ref(root, proof_path) == proof_record and proof['tool_inputs'] == tool_inputs(root), 'Proof/tool drift')
    return dict(schema='mb3d.translate-mixed-scene-controls-result.v1', status='PASS_BOUNDED_ACTUAL_CONTROLS',
                qualification=False, verified_formulas=0, proof=proof_record, candidate=candidate,
                obligations=[dict(id=k, status='PASS') for k in SUPPORTED] +
                            [dict(id=k, status='OPEN') for k in OPEN],
                mixed=mixed, scene=scene, fresh_processes=len(a.processes),
                checked_artifacts=[dict(path=p, sha256=h) for p, h in sorted(a.pins.items())])


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('proof', type=Path)
    args = parser.parse_args()
    print(json.dumps(validate(ROOT, coverage.ref(ROOT, args.proof.resolve())), indent=2))


if __name__ == '__main__':
    main()
