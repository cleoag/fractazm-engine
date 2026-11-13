"""Synthetic unit tamper controls only; no device/scene qualification fixtures."""
import copy
import json
from pathlib import Path
import struct
import tempfile
import unittest
from unittest.mock import patch

import native_translate_coverage_controls as c


class StateControls(unittest.TestCase):
    def setUp(self):
        self.target = {key: '0x00000000' for key in c.STATE}
        self.actual = dict(result_words=[0]*59, work=36, faults=0, state_bits=self.target.copy(), device='unit-test-adapter')
        self.raw = dict(result_words=[0]*59, work=36, fault=0)
        self.item = dict(result_words=[0]*59, work=36, fault=0, differences={}, work_matches_model=True)
        self.expected = dict(expected_state_bits=self.target.copy(), expected_work=36)

    def check(self): c.exact_state(self.actual, self.raw, self.item, self.expected)

    def test_all47_and_work_guard_accepts_matching_unit_values_only(self): self.check()

    def test_every_state_lane_mismatch_rejects_despite_pass_summaries(self):
        for field in c.STATE:
            with self.subTest(field=field):
                self.actual['state_bits'] = dict(self.target, **{field: '0x00000001'})
                with self.assertRaisesRegex(ValueError, 'full47'): self.check()

    def test_partial_model_cannot_supply_full_state_credit(self):
        del self.expected['expected_state_bits']['sse[3]']
        with self.assertRaisesRegex(ValueError, 'full47'): self.check()

    def test_raw59_direct_observer_mismatch_rejects(self):
        for index in range(59):
            with self.subTest(word=index):
                self.raw['result_words'] = [0]*59; self.raw['result_words'][index] = 1
                with self.assertRaisesRegex(ValueError, 'raw59'): self.check()

    def test_incomplete_raw59_rejects(self):
        self.actual['result_words'] = self.raw['result_words'] = self.item['result_words'] = [0]*58
        with self.assertRaisesRegex(ValueError, 'raw59'): self.check()

    def test_equal_wrong_work_rejects_source_target(self):
        self.actual['work'] = self.raw['work'] = self.item['work'] = 37
        with self.assertRaisesRegex(ValueError, 'full47'): self.check()

    def test_fault_cannot_be_promoted_by_equal_repeats(self):
        self.actual['faults'] = self.raw['fault'] = self.item['fault'] = 1
        with self.assertRaisesRegex(ValueError, 'raw59'): self.check()

    def test_synthetic_and_unmarked_device_reject(self):
        for device in ('Synthetic Apple', '', None):
            self.actual['device'] = device
            with self.subTest(device=device), self.assertRaisesRegex(ValueError, 'device'): self.check()

    def test_zero_without_target_only_checks_transport_and_returns_no_credit(self):
        self.expected = {'status': 'OPEN_MODEL_DOMAIN'}
        self.assertIsNone(self.check())


class HeaderControls(unittest.TestCase):
    def setUp(self):
        dummy = dict(path='unused', sha256='0'*64)
        self.proof = dict(schema=c.SCHEMA, identity=c.context_input.IDENTITY, declaration=dict(dummy, sha256=c.DECLARATION_SHA),
                          context_id=c.context_input.CONTEXT_ID, obligation_ids=list(c.SUPPORTED),
                          candidate={k:dummy.copy() for k in ('probe', 'library', 'renderer', 'build_receipt')},
                          mixed_collection=dummy, scene_evidence=dummy,
                          scene_contract=c.coverage.ref(c.ROOT, c.ROOT/c.CONTRACT_PATH),
                          tool_inputs=c.tool_inputs(), evidence_origin='actual_device_capture')
        self.proof['candidate']['observer'] = c.coverage.ref(c.ROOT, Path(c.observer.__file__))

    def test_exact_structural_record_has_no_self_asserted_status(self):
        c.validate_header(c.ROOT, self.proof)
        self.proof['status'] = 'PASS'
        with self.assertRaisesRegex(ValueError, 'fields'): c.validate_header(c.ROOT, self.proof)

    def test_unknown_schema_identity_and_context_reject(self):
        for key, value in [('schema', 'unknown'), ('identity', ['external',1,'other']), ('context_id', 'other')]:
            p=copy.deepcopy(self.proof); p[key]=value
            with self.subTest(key=key), self.assertRaises(ValueError): c.validate_header(c.ROOT, p)

    def test_open_profiles_cannot_be_promoted_and_obligations_cannot_disappear(self):
        for ids in (list(c.SUPPORTED)+[c.OPEN[0]], list(c.SUPPORTED[1:]), list(c.SUPPORTED)+[c.SUPPORTED[0]]):
            self.proof['obligation_ids']=ids
            with self.assertRaisesRegex(ValueError, 'obligation'): c.validate_header(c.ROOT, self.proof)

    def test_synthetic_origin_and_rehashed_declaration_reject(self):
        for key, value in [('evidence_origin','synthetic_fixture'), ('declaration',dict(path='changed',sha256='1'*64))]:
            p=copy.deepcopy(self.proof); p[key]=value
            with self.subTest(key=key), self.assertRaises(ValueError): c.validate_header(c.ROOT, p)

    def test_mixed_candidate_and_synthetic_collection_reject_before_replay(self):
        candidate=self.proof['candidate']
        base=dict(schema='mb3d.native-formula-context-diagnostic-collection.v1', status='DIAGNOSTIC_ONLY',
                  qualification=False, verified_formulas=0, evidence_origin='actual_device_capture',
                  **{k:candidate[k] for k in ('probe','library','build_receipt')})
        changed=copy.deepcopy(base); changed['library']['sha256']='7'*64
        for data in (changed,dict(base,evidence_origin='synthetic_fixture')):
            with patch.object(c.Replay,'read',return_value=data),self.assertRaises(ValueError):
                c.validate_mixed(c.Replay(c.ROOT,c.ROOT/'unused-proof'),self.proof)

    def test_tool_or_observer_swap_rejects(self):
        for key in ('tool_inputs', 'observer'):
            p=copy.deepcopy(self.proof)
            if key=='tool_inputs': p[key]=[]
            else: p['candidate'][key]['sha256']='2'*64
            with self.subTest(key=key), self.assertRaises(ValueError): c.validate_header(c.ROOT,p)


class ArtifactAndProcessControls(unittest.TestCase):
    def setUp(self):
        self.temp=tempfile.TemporaryDirectory(); self.addCleanup(self.temp.cleanup)
        self.root=Path(self.temp.name).resolve(); self.a=c.Replay(self.root,self.root/'proof.json')

    def put(self,name,value=b'{}'):
        p=self.root/name; p.write_bytes(value)
        return c.coverage.ref(self.root,p)

    def test_circular_and_escaping_reference_reject(self):
        proof=self.put('proof.json')
        with self.assertRaisesRegex(ValueError,'circular'): self.a.bind(proof)
        with self.assertRaisesRegex(ValueError,'Escaping'): self.a.bind(dict(path='../escape',sha256='0'*64))

    def test_artifact_mutation_and_unknown_schema_reject(self):
        r=self.put('record.json',b'{"schema":"unknown"}')
        with self.assertRaisesRegex(ValueError,'schema'): self.a.read(r,c.SCHEMA)
        (self.root/'record.json').write_text('{}')
        with self.assertRaisesRegex(ValueError,'Changed'): self.a.stable()

    def test_unique_events_cannot_reuse_pid_start_or_run_id(self):
        exe=self.put('exe'); events=[self.put(f'e{i}.json') for i in range(3)]
        base=dict(seconds=1,pid=71,started_ns=4,run_id='a')
        with patch.object(c.observer,'run_event',return_value=base): self.a.event(events[0],exe)
        for value in (dict(base,run_id='b'),dict(base,pid=72)):
            with patch.object(c.observer,'run_event',return_value=value), self.assertRaisesRegex(ValueError,'Reused process'):
                self.a.event(events[1],exe)
        with self.assertRaisesRegex(ValueError,'Reused event'): self.a.event(events[0],exe)

    def test_scene_hard_guard_does_not_waive_180_diagnostic_threshold(self):
        e=self.put('event.json'); exe=self.put('exe')
        with patch.object(c.observer,'run_event',return_value=dict(seconds=181)) as replay:
            with self.assertRaisesRegex(ValueError,'threshold'): self.a.event(e,exe,scene=True)
            self.assertEqual(replay.call_args.kwargs['max_seconds'],300)

    def direct(self):
        probe=self.put('probe'); lib=self.put('library'); raw=self.put('raw'); inp=self.put('input.json')
        pp=self.put('program.bin',struct.pack('<2048I',*([0]*2048)))
        sp=self.put('state.bin',struct.pack('<47I',*([0]*47)))
        argv=[str(self.root/'probe'),'--program',str(self.root/'program.bin'),'--states',str(self.root/'state.bin'),
              '--library',str(self.root/'library'),'--output',str(self.root/'raw'),'--mode','orbit','--slot','1','--repeats','3','--wrapper-policy','1']
        return dict(argv=argv,inputs=[pp,sp,inp]),dict(input=inp,baseline=dict(raw=raw)),dict(probe=probe,library=lib)

    def verify_direct(self,event,row,candidate):
        c.validate_direct_inputs(self.a,event,row,candidate,[0]*2048,[0]*47,self.root/'program.bin')

    def test_direct_raw_cannot_replace_actual_program_state_inputs(self):
        event,row,candidate=self.direct(); self.verify_direct(event,row,candidate)
        for name in ('program.bin','state.bin','input.json'):
            changed=copy.deepcopy(event); changed['inputs']=[r for r in changed['inputs'] if r['path']!=name]
            with self.subTest(input=name),self.assertRaisesRegex(ValueError,'input'): self.verify_direct(changed,row,candidate)
        (self.root/'state.bin').write_bytes(struct.pack('<47I',*([1]*47)))
        event['inputs'][1]=c.coverage.ref(self.root,self.root/'state.bin')
        with self.assertRaisesRegex(ValueError,'input bytes'): self.verify_direct(event,row,candidate)

    def test_changed_mode_or_duplicate_option_rejects_even_with_passing_raw(self):
        event,row,candidate=self.direct()
        for argv in (event['argv']+['--slot','0'], [x if x!='orbit' else 'point' for x in event['argv']]):
            with self.assertRaisesRegex(ValueError,'arguments'): self.verify_direct(dict(event,argv=argv),row,candidate)

    def test_scene_common_gate_failure_not_overridden_by_summary(self):
        renderer=self.put('renderer'); lib=self.put('library'); contract=self.put('contract.json')
        ev=dict(schema='mb3d.gpu-parallel-evidence.v2',artifact_root='repository',binding=dict(renderer=renderer,library=lib))
        ref=self.put('evidence.json',json.dumps(ev).encode())
        proof=dict(scene_evidence=ref,scene_contract=contract,candidate=dict(renderer=renderer,library=lib))
        with patch.object(c.gate,'scope',return_value=({}, {}, {})), patch.object(c.gate,'evaluate',return_value={'errors':['Stale source/contract'],'accepted':[['scenes',c.SCENE]]}):
            with self.assertRaisesRegex(ValueError,'common scene gate'): c.validate_scene(self.a,proof)


if __name__=='__main__': unittest.main()
