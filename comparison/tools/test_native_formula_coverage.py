"""Synthetic coverage tamper controls; no numerical/device qualification."""
import copy
import importlib.util
import json
from pathlib import Path
import sys
import tempfile
import unittest
from unittest.mock import patch

import native_formula_coverage as c
from gpu_pipeline_proofs import STATE
from test_native_formula_observer import case


class CoverageFixture:
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory(); self.root = Path(self.tmp.name).resolve()
        self.source = self.put('original.txt', 'immutable original source', text=True)
        self.defaults = self.put('defaults.json', {'values':[0,0,0]})
        self.model = self.put('model.py', '# independent model', text=True)
        self.helper = self.put('helper.py', '# exact arithmetic', text=True)
        self.identity = ['external',8065,'_Translate']
        self.scene_inventory = self.put(
            'comparison/contracts/native-gpu-acceptance-v4/inventory.json',
            {'scenes':[{'id':str(i),'slots':[]} for i in range(77)],
             'formulas':[{'namespace':self.identity[0], 'id':self.identity[1],
                          'name':self.identity[2]}]})
        self.put('comparison/contracts/native-gpu-acceptance-v4/acceptance-contract.json',
                 {'inventory':'inventory.json','inventory_sha256':self.scene_inventory['sha256']})
        self.inv = {'schema':'mb3d.formula-obligation-inventory.v1','identity':self.identity,
                    'sources':[self.source],'defaults':[self.defaults],
                    'scene_inventory':self.scene_inventory,'contexts':[],'status':'INVENTORIED'}
        self.inventory = self.put('inventory.json', self.inv)
        payload = case()
        self.corpus = [dict(id='point',identity=self.identity,variant='synthetic-unit-control',input=payload,
            expected_state_bits={k:'0x00000000' for k in STATE},read_lanes=list(STATE),
            write_lanes=['xyz[0]'],source_hashes={'original':self.source['sha256']})]
        self.binding = dict(reference_model=self.model,reference_dependencies=[self.model,self.helper],
                            reference_manifest={'scope':'SYNTHETIC UNIT FIXTURE'})
        self.d = {'schema':'mb3d.formula-coverage-declaration.v1','identity':self.identity,
                  'author':'model-author','inventory':self.inventory,'sources':[self.source],
                  'defaults':[self.defaults],'context_ids':[], 'categories':{},
                  'cases':[{'id':'point','input_sha256':c.digest(payload),'capability':'single_slot',
                            'context_ids':[],'fields':list(STATE),'domains':list(c.CATEGORIES)}]}
        for name in c.CATEGORIES:
            na = name in ('scene_contexts','resources')
            self.d['categories'][name] = [dict(id=name,applicability='NOT_APPLICABLE' if na else 'REQUIRED',
                source_locations=[{'artifact':self.source,'location':'original lines 1-2'}],
                condition='Concrete original condition for '+name,context_ids=[],required_fields=[] if na or name=='source_semantics' else list(STATE),
                capability='single_slot',evidence_kind='source' if name=='source_semantics' or na else 'device',
                case_ids=[] if na or name=='source_semantics' else ['point'],
                na_reason='Source-backed absence' if na else '')]
        self.sync()

    def tearDown(self): self.tmp.cleanup()
    def put(self,name,value,text=False):
        p=self.root/name;p.parent.mkdir(parents=True,exist_ok=True)
        p.write_text(value if text else json.dumps(value))
        return c.ref(self.root,p)
    def sync(self):
        self.dr=self.put('declaration.json',self.d)
        self.review={'schema':'mb3d.formula-coverage-review.v1','declaration':self.dr,'reviewer':'independent-reviewer',
                     'status':'PASS','reviewed_obligation_ids':[
                         row['id'] for rows in self.d['categories'].values() for row in rows],
                     'approved_na_ids':[row['id'] for rows in self.d['categories'].values()
                                        for row in rows if row['applicability']=='NOT_APPLICABLE'],
                     'rationale':'Original-source completeness review'}
        self.rr=self.put('review.json',self.review)
        self.mr_data={'schema':'mb3d.formula-model-review.v1','declaration':self.dr,'reviewer':'model-reviewer',
            'status':'PASS',**self.binding,'corpus_sha256':c.digest(self.corpus),
            'reviewed_case_ids':[row['id'] for row in self.corpus],
            'derivation':'Independent original arithmetic, no candidate outputs','observed_fields':list(STATE),
            'representation':'full_state_binary32','exclusions':[]}
        self.mr=self.put('model-review.json',self.mr_data)
    def validate(self):
        return c.validate_precollection(self.root,self.dr,self.rr,self.mr,self.corpus,self.binding)


class CoverageContract(CoverageFixture, unittest.TestCase):
    def test_predeclared_closure_captures_sources_reviews_model_and_defaults(self):
        result=self.validate()
        for record in (self.dr,self.rr,self.mr,self.inventory,self.scene_inventory,self.source,self.defaults,self.model,self.helper):
            self.assertIn(record,result['coverage_inputs'])

    def test_missing_empty_categories_and_defaults_reject_after_rehash(self):
        original=copy.deepcopy(self.d)
        for name in c.CATEGORIES:
            for empty in (False,True):
                self.d=copy.deepcopy(original)
                if empty:self.d['categories'][name]=[]
                else:del self.d['categories'][name]
                self.sync()
                with self.subTest(category=name,empty=empty),self.assertRaises(ValueError):self.validate()
        self.d=copy.deepcopy(original);self.d['defaults']=[];self.sync()
        with self.assertRaises(ValueError):self.validate()

    def test_unsupported_na_and_self_review_reject(self):
        self.d['categories']['rounded_finite'][0].update(applicability='NOT_APPLICABLE',na_reason='too difficult')
        self.sync()
        with self.assertRaises(ValueError):self.validate()
        self.setUp_after_mutation()
        self.review['reviewer']='model-author';self.rr=self.put('review.json',self.review)
        with self.assertRaises(ValueError):self.validate()

    def setUp_after_mutation(self):
        self.d['categories']['rounded_finite'][0].update(applicability='REQUIRED',na_reason='');self.sync()

    def test_forward_or_circular_review_fields_reject(self):
        self.review['collection']={'path':'future','sha256':'0'*64};self.rr=self.put('review.json',self.review)
        with self.assertRaises(ValueError):self.validate()
        self.sync();self.d['future_admission']={'path':'future','sha256':'0'*64};self.sync()
        with self.assertRaises(ValueError):self.validate()

    def test_stale_model_helper_source_and_mapping_reject(self):
        for name in ('model.py','helper.py','original.txt'):
            p=self.root/name;old=p.read_bytes();p.write_bytes(old+b'changed')
            with self.subTest(name=name),self.assertRaises(ValueError):self.validate()
            p.write_bytes(old)
        self.corpus[0]['input']['parameter_bits'][68]='0x3f800000'
        with self.assertRaises(ValueError):self.validate()

    def test_wrong_identity_or_unknown_case_and_unobserved_fields_reject(self):
        for field,value in [('case_ids',['missing']),('required_fields',['not-a-state-word'])]:
            old=self.d['categories']['state'][0][field]
            self.d['categories']['state'][0][field]=value;self.sync()
            with self.assertRaises(ValueError):self.validate()
            self.d['categories']['state'][0][field]=old
        self.sync();self.corpus[0]['identity']=['external',429,'genIFS']
        with self.assertRaises(ValueError):self.validate()
        self.corpus[0]['identity']=self.identity
        self.mr_data['representation']='decimal';self.mr=self.put('model-review.json',self.mr_data)
        with self.assertRaises(ValueError):self.validate()

    def test_missing_realized_context_or_single_slot_substitution_reject(self):
        self.inv['contexts']=[{'id':'mixed','scene_ids':['0'],'parameters_sha256':'1'*64,
            'program_sha256':'2'*64,'resources_sha256':'3'*64,'capability':'mixed','artifacts':[self.source]}]
        self.inventory=self.put('inventory.json',self.inv);self.d['inventory']=self.inventory;self.sync()
        with self.assertRaises(ValueError):self.validate()
        self.d['context_ids']=['mixed'];self.d['cases'][0]['context_ids']=['mixed'];self.sync()
        with self.assertRaises(ValueError):self.validate()

    def test_case_cannot_lose_default_or_rounded_domain(self):
        for domain in ('authored_defaults','rounded_finite'):
            self.d['cases'][0]['domains'].remove(domain);self.sync()
            with self.assertRaises(ValueError):self.validate()
            self.d['cases'][0]['domains'].append(domain)

    def test_collection_requires_captured_predeclaration_in_every_event(self):
        bundle=self.validate()
        with self.assertRaises(ValueError):c.validate_event_inputs({'inputs':bundle['coverage_inputs'][1:]},bundle)
        c.validate_event_inputs({'inputs':bundle['coverage_inputs']},bundle)

    def test_rehashed_alternate_77_scene_authority_is_rejected(self):
        alternate = self.put('untrusted-scenes.json', {'scenes':[
            {'id':str(i),'slots':[]} for i in range(77)], 'formulas':[]})
        self.inv['scene_inventory'] = alternate
        self.inventory = self.put('inventory.json', self.inv)
        self.d['inventory'] = self.inventory
        self.sync()
        with self.assertRaisesRegex(ValueError, 'inventory|authority|canonical'):
            self.validate()

    def test_authored_usage_cannot_be_erased_from_the_pinned_inventory(self):
        authority = c.load(self.root, self.scene_inventory)
        authority['scenes'][0]['slots'] = [{'key':'external:8065','iterations':1}]
        self.scene_inventory = self.put(self.scene_inventory['path'], authority)
        self.put('comparison/contracts/native-gpu-acceptance-v4/acceptance-contract.json',
                 {'inventory':'inventory.json','inventory_sha256':self.scene_inventory['sha256']})
        self.inv['scene_inventory'] = self.scene_inventory
        self.d['inventory'] = self.put('inventory.json', self.inv); self.sync()
        with self.assertRaisesRegex(ValueError, 'Authored scene usage'):
            self.validate()
        authority['scenes'][0]['slots'] = []
        self.inv['scene_inventory'] = self.put('erased-scene-usage.json', authority)
        self.d['inventory'] = self.put('inventory.json', self.inv); self.sync()
        with self.assertRaisesRegex(ValueError, 'pinned acceptance authority'):
            self.validate()

    def test_valid_resource_input_cannot_claim_resources_not_applicable(self):
        import native_formula_resource_input as resources
        from test_native_formula_resource_input import ResourceInput
        fixture = ResourceInput(); fixture.setUp(); self.addCleanup(fixture.tearDown)
        payload = copy.deepcopy(fixture.payload)
        for record in payload['files']:
            target = self.root/record['path']; target.parent.mkdir(parents=True, exist_ok=True)
            target.write_bytes((fixture.root/record['path']).read_bytes())
        resources.validate(payload, self.root)
        self.corpus[0]['input']['resources'] = payload
        self.d['cases'][0]['input_sha256'] = c.digest(self.corpus[0]['input'])
        self.sync()
        with self.assertRaisesRegex(ValueError, 'resource-bearing'):
            self.validate()
        self.d['categories']['resources'][0].update(applicability='REQUIRED', na_reason='',
                                                    evidence_kind='device', case_ids=[])
        self.sync()
        with self.assertRaisesRegex(ValueError, 'resource-bearing'):
            self.validate()

    def test_unsupported_scene_adapter_cannot_be_enabled_by_a_label(self):
        self.inv['contexts'] = [{'id':'authored','scene_ids':['0'],'parameters_sha256':'1'*64,
            'program_sha256':'2'*64,'resources_sha256':'3'*64,'capability':'single_slot',
            'representation':'probe-program-v1','artifacts':[self.source]}]
        self.d['inventory'] = self.put('inventory.json', self.inv)
        self.d['context_ids'] = ['authored']
        self.d['cases'][0]['context_ids'] = ['authored']
        self.d['categories']['scene_contexts'][0].update(applicability='REQUIRED', na_reason='',
            evidence_kind='device', context_ids=['authored'], case_ids=['point'])
        self.sync()
        with self.assertRaisesRegex(ValueError, 'Unsupported scene-to-probe context adapter'):
            self.validate()

    def test_future_admission_requires_coverage_before_any_io(self):
        import verify_native_formula_group as v
        with patch.object(v,'read') as reader, self.assertRaisesRegex(ValueError,'coverage'):
            v.admit(Path('missing'),Path('missing'),Path('missing'),Path('missing'))
        reader.assert_not_called()


class CoverageDecision(CoverageFixture, unittest.TestCase):
    """Synthetic state-machine fixtures; replay/build/common-gate doubles only.

    These tests deliberately substitute the hardware/provenance boundaries to
    exercise coverage decisions. Their temporary PASS records are not device
    evidence. Reference loading, corpus checks and artifact binding stay real.
    """

    def setUp(self):
        super().setUp()
        import native_formula_observer as observer
        import prepare_gpu_parallel_bundle as bundle_writer
        import verify_native_formula_group as verifier

        self.verifier = verifier
        self.observer = observer
        orbit = copy.deepcopy(self.corpus[0])
        orbit.update(id='orbit', variant='synthetic-unit-orbit')
        orbit['input']['kind'] = 'orbit'
        orbit['input']['parameter_bits'][1] = '0x00000001'
        self.corpus.append(orbit)
        mapped = copy.deepcopy(self.d['cases'][0])
        mapped.update(id='orbit', input_sha256=c.digest(orbit['input']))
        self.d['cases'].append(mapped)
        for rows in self.d['categories'].values():
            for row in rows:
                if row['case_ids']:
                    row['case_ids'] = ['point', 'orbit']

        # A real, deliberately synthetic module exercises the normal reference
        # importer, source hashes and helper closure without invoking hardware.
        manifest = {'scope':'SYNTHETIC UNIT FIXTURE; NO NUMERICAL CREDIT'}
        self.helper = self.put('comparison/tools/unit_coverage_helper.py',
                               '# Synthetic reference dependency\n', text=True)
        model_source = (
            'import copy, json\nfrom pathlib import Path\n'
            f'CORPUS = json.loads({json.dumps(self.corpus)!r})\n'
            f'def manifest(): return {manifest!r}\n'
            'def dependency_paths():\n'
            '    return [Path(__file__).with_name("unit_coverage_helper.py")]\n'
            'def build_cases(): return copy.deepcopy(CORPUS)\n'
            'def evaluate(payload):\n'
            '    for row in CORPUS:\n'
            '        if row["input"] == payload:\n'
            '            return copy.deepcopy(row["expected_state_bits"])\n'
            '    raise ValueError("Unknown synthetic reference input")\n'
        )
        model_name = 'unit_coverage_reference'
        self.model = self.put('comparison/tools/'+model_name+'.py', model_source, text=True)
        module_spec = importlib.util.spec_from_file_location(model_name, self.root/self.model['path'])
        module = importlib.util.module_from_spec(module_spec)
        module_spec.loader.exec_module(module)
        for item in (
            patch.object(verifier, 'ROOT', self.root),
            patch.object(bundle_writer, 'ROOT', self.root),
            patch.dict(verifier.REFERENCE_MODULES, {'synthetic_unit':model_name}),
            patch.dict(sys.modules, {model_name:module}),
        ):
            item.start(); self.addCleanup(item.stop)
        self.binding = dict(reference_model=self.model,
                            reference_dependencies=sorted([self.model,self.helper], key=lambda r:r['path']),
                            reference_manifest=manifest)
        self.tool_refs = [self.put('coverage-tool.py', '# Synthetic coverage tool\n', text=True),
                          self.put('collector-tool.py', '# Synthetic collector tool\n', text=True)]
        tools_patch = patch.object(c, 'tool_inputs', side_effect=lambda root:
                                  [c.ref(root, Path(root)/r['path']) for r in self.tool_refs])
        tools_patch.start(); self.addCleanup(tools_patch.stop)
        self.sync()
        self.bundle = self.validate()
        self.candidate = {key:self.put(key+'.fixture', 'SYNTHETIC '+key+'; NO GPU\n', text=True)
                          for key in ('library','probe','observer','build_receipt')}
        self.rows = []
        for index, original in enumerate(self.corpus):
            row = {key:copy.deepcopy(original[key]) for key in
                   ('id','identity','variant','expected_state_bits','read_lanes','write_lanes','source_hashes')}
            row.update(input=self.put(original['id']+'/input.json', original['input']),
                       kind=original['input']['kind'], status='PASS_CASE',
                       gpu_state_bits=copy.deepcopy(original['expected_state_bits']),
                       result_words=[0]*59, repeats=[])
            for repeat in range(3):
                prefix = original['id']+'/repeat-'+str(repeat)
                event = self.put(prefix+'/event.json', {
                    'run_id':f'{index*3+repeat+1:032x}',
                    'inputs':self.bundle['coverage_inputs']+self.tool_refs})
                observation = self.put(prefix+'/observation.json', {
                    # The actual-device check is explicitly replaced below.
                    # Do not use this label/record outside this unit fixture.
                    'device':'UNIT TEST REPLAY DOUBLE; NO HARDWARE WAS RUN',
                    'state_bits':copy.deepcopy(row['gpu_state_bits']),
                    'result_words':row['result_words'].copy(),
                    'invalid_descriptor_controls':7,
                    'allocation_canaries':True, 'input_immutable':True})
                row['repeats'].append({'event':event,'observation':observation,'inputs':row['input']})
            self.rows.append(row)
        self.collection = dict(schema='mb3d.native-formula-group.v1',
            status='COLLECTED_REVIEW_REQUIRED', evidence_origin='actual_device_capture',
            reference_group='synthetic_unit', cases=self.rows,
            coverage_tools=self.tool_refs, **self.binding, **self.candidate, **self.bundle)
        self.disposition = dict(schema='mb3d.formula-scene-disposition.v1',
            identity=self.identity, candidate=copy.deepcopy(self.candidate),
            reviewer='independent-scene-reviewer', rationale='No authored fixture scene contexts',
            contexts=[], evidence=[])
        self.decision = dict(schema='mb3d.formula-coverage-decision.v1',
            declaration=self.dr, completeness_review=self.rr, model_review=self.mr,
            reviewer='independent-device-reviewer', identity=self.identity.copy(),
            candidate=copy.deepcopy(self.candidate), status='COVERAGE_COMPLETE',
            obligations=[dict(id=row['id'], status='NOT_APPLICABLE'
                if row['applicability']=='NOT_APPLICABLE' else 'PASS',
                case_ids=row['case_ids'].copy(), reason='')
                for rows in self.d['categories'].values() for row in rows])
        self.save_collection()
        self.save_disposition()
        self.save_decision()
        build_patch = patch.object(verifier, 'validate_build_products')
        self.build_check = build_patch.start(); self.addCleanup(build_patch.stop)
        replay_patch = patch.object(observer, 'replay_observation', side_effect=self.fake_replay)
        self.replay_check = replay_patch.start(); self.addCleanup(replay_patch.stop)

    @staticmethod
    def fake_replay(root, observation, event, payload, probe, library):
        """Unit seam only: raw/device validation is covered by observer tests."""
        return c.load(root, observation)

    def save_collection(self):
        self.cr = self.put('collection.json', self.collection)
        self.decision['collection'] = self.cr

    def save_disposition(self):
        self.sr = self.put('scene-disposition.json', self.disposition)
        self.decision['scene_disposition'] = self.sr

    def save_decision(self):
        self.decision_ref = self.put('coverage.json', self.decision)
        return self.decision_ref

    def refresh_predeclared_fixture(self):
        """Build a different predeclared unit fixture, never mutate real evidence."""
        self.sync(); self.bundle = self.validate()
        self.collection.update(self.bundle)
        self.decision.update(declaration=self.dr, completeness_review=self.rr, model_review=self.mr)
        for row in self.rows:
            for repeat in row['repeats']:
                event = c.load(self.root, repeat['event'])
                event['inputs'] = self.bundle['coverage_inputs']+self.tool_refs
                repeat['event'] = self.put(repeat['event']['path'], event)
        self.save_collection()

    def outcome(self, key):
        return next(row for row in self.decision['obligations'] if row['id']==key)

    def check(self):
        return c.validate_coverage(self.root, self.save_decision())

    def edit_repeat(self, case_index, repeat_index, key, mutate):
        repeat = self.rows[case_index]['repeats'][repeat_index]
        value = c.load(self.root, repeat[key]); mutate(value)
        repeat[key] = self.put(repeat[key]['path'], value)
        self.save_collection()

    def make_ledger(self):
        self.package_binding = {'library':self.candidate['library'],
                                'tools':{'formula_gpu':self.candidate['observer']}}
        self.proof = {'schema':'mb3d.native-metal-all.formula-observations.v1',
                      'cases':[{'case_id':row['id'], 'repeats':copy.deepcopy(row['repeats']),
                                'gpu_state_bits':copy.deepcopy(row['gpu_state_bits'])}
                               for row in self.rows]}
        self.receipt = dict(kind='formula_gpu_point_orbit', status='PASS',
                            identity=self.identity.copy(), coverage=self.save_decision(),
                            binding_sha256=c.digest(self.package_binding),
                            artifacts=[self.cr,self.candidate['build_receipt']],
                            observations=self.put('package/observations.json', self.proof))
        self.receipt_ref = self.put('package/receipt.json', self.receipt)
        self.evidence = {'binding':self.package_binding,
                         'formulas':[dict(namespace=self.identity[0], id=self.identity[1],
                                          name=self.identity[2], proof=self.receipt_ref)]}
        self.accepted = ['formulas', str(tuple(self.identity))]
        self.qualification = {'accepted':[self.accepted]}
        self.ledger = dict(schema='mb3d.formula-verified.v1', status='FORMULA_VERIFIED',
            identity=self.identity.copy(), candidate=copy.deepcopy(self.candidate),
            coverage=self.decision_ref, admission_receipt=self.receipt_ref,
            scene_disposition=self.sr, contract=self.put('package/contract.json', {'fixture':True}),
            evidence=self.put('package/evidence.json', self.evidence),
            qualification=self.put('package/qualification.json', self.qualification))
        gate_patch = patch.object(self.verifier, 'evaluate', return_value={'accepted':[self.accepted]})
        self.common_gate = gate_patch.start(); self.addCleanup(gate_patch.stop)

    def check_ledger(self):
        return c.validate_verified(self.root, self.put('verified.json', self.ledger))

    def test_complete_decision_replays_each_mapped_case_once(self):
        self.assertEqual(self.check()['status'], 'COVERAGE_COMPLETE')
        self.assertEqual(self.replay_check.call_count, 6)
        self.build_check.assert_called_once()
        self.assertEqual({call.args[3]['kind'] for call in self.replay_check.call_args_list},
                         {'point','orbit'})

    def test_case_kind_cannot_relabel_actual_point_or_orbit(self):
        self.rows[1]['kind']='point';self.save_collection()
        with self.assertRaisesRegex(ValueError,'kind differs'):
            self.check()

    def test_scoped_proof_retains_required_open_without_completion(self):
        self.outcome('rounded_finite').update(status='OPEN', reason='Original rounded domain unresolved')
        self.decision['status'] = 'SCOPED_PROOF'
        self.assertEqual(self.check()['status'], 'SCOPED_PROOF')
        self.assertEqual(self.replay_check.call_count, 6)
        self.decision['status'] = 'COVERAGE_COMPLETE'
        with self.assertRaisesRegex(ValueError, 'status'):
            self.check()

    def test_open_without_numerical_pass_does_not_replay(self):
        for row in self.decision['obligations']:
            if row['status']=='PASS' and row['id']!='source_semantics':
                row.update(status='OPEN', reason='Required device evidence not yet available')
        self.decision['status'] = 'OPEN'
        self.assertEqual(self.check()['status'], 'OPEN')
        self.replay_check.assert_not_called()
        self.decision['status'] = 'SCOPED_PROOF'
        with self.assertRaisesRegex(ValueError, 'status'):
            self.check()

    def test_wrong_identity_candidate_and_evidence_origin_reject(self):
        self.decision['identity'] = ['external',429,'genIFS']
        with self.assertRaisesRegex(ValueError, 'identity'):
            self.check()
        self.decision['identity'] = self.identity.copy()
        self.decision['candidate']['probe'] = self.put('other-probe', 'another probe', text=True)
        with self.assertRaisesRegex(ValueError, 'candidate'):
            self.check()
        self.decision['candidate'] = copy.deepcopy(self.candidate)
        for origin in ('synthetic_fixture', None):
            if origin is None:self.collection.pop('evidence_origin', None)
            else:self.collection['evidence_origin'] = origin
            self.save_collection()
            with self.subTest(origin=origin), self.assertRaisesRegex(ValueError, 'Synthetic|unmarked'):
                self.check()
        self.replay_check.assert_not_called()

    def test_synthetic_device_report_cannot_become_numerical_credit(self):
        self.edit_repeat(0, 0, 'observation', lambda r:r.update(device='SYNTHETIC TEST FIXTURE'))
        with self.assertRaisesRegex(ValueError, 'Synthetic device'):
            self.check()

    def test_changed_outcome_case_mapping_rejects(self):
        for mapping in (['point'], ['point','unknown'], ['orbit','point'], ['point','point']):
            self.outcome('rounded_finite')['case_ids'] = mapping
            with self.subTest(mapping=mapping), self.assertRaisesRegex(ValueError, 'case mapping'):
                self.check()

    def test_rehashed_input_or_full_state_target_tamper_rejects(self):
        original = c.load(self.root, self.rows[0]['input'])
        changed = copy.deepcopy(original); changed['parameter_bits'][68] = '0x3f800000'
        self.rows[0]['input'] = self.put(self.rows[0]['input']['path'], changed)
        self.save_collection()
        with self.assertRaisesRegex(ValueError, 'input changed'):
            self.check()
        self.rows[0]['input'] = self.put(self.rows[0]['input']['path'], original)
        self.rows[0]['gpu_state_bits']['xyz[0]'] = '0x3f800000'
        self.save_collection()
        with self.assertRaisesRegex(ValueError, 'full state mismatch'):
            self.check()

    def test_fewer_or_reused_fresh_processes_reject(self):
        original = copy.deepcopy(self.rows[0]['repeats'])
        self.rows[0]['repeats'].pop(); self.save_collection()
        with self.assertRaisesRegex(ValueError, 'repeats'):
            self.check()
        self.rows[0]['repeats'] = original
        first_id = c.load(self.root, original[0]['event'])['run_id']
        self.edit_repeat(0, 2, 'event', lambda r:r.update(run_id=first_id))
        with self.assertRaisesRegex(ValueError, 'Reused formula process'):
            self.check()

    def test_reused_event_across_point_and_orbit_rejects(self):
        first_id = c.load(self.root, self.rows[0]['repeats'][0]['event'])['run_id']
        self.edit_repeat(1, 0, 'event', lambda r:r.update(run_id=first_id))
        with self.assertRaisesRegex(ValueError, 'Reused formula process'):
            self.check()

    def test_59th_word_mismatch_rejects_despite_matching_full_state(self):
        self.edit_repeat(0, 2, 'observation', lambda r:r['result_words'].__setitem__(58, 1))
        with self.assertRaisesRegex(ValueError, '47/59'):
            self.check()

    def test_truncated_59_word_result_rejects_before_replay(self):
        self.rows[0]['result_words'].pop(); self.save_collection()
        with self.assertRaisesRegex(ValueError, 'repeat words'):
            self.check()
        self.replay_check.assert_not_called()

    def test_stale_candidate_and_helper_closure_reject(self):
        for record in [*self.candidate.values(), self.helper, self.dr, self.tool_refs[0]]:
            path = self.root/record['path']; previous = path.read_bytes()
            path.write_bytes(previous+b'changed')
            with self.subTest(path=record['path']), self.assertRaises(ValueError):
                self.check()
            path.write_bytes(previous)

    def test_last_event_missing_predeclared_model_or_source_rejects(self):
        record = self.rows[1]['repeats'][2]['event']
        original = c.load(self.root, record)
        for omitted in (self.dr, self.rr, self.mr, self.helper, self.source):
            changed = copy.deepcopy(original)
            changed['inputs'] = [r for r in changed['inputs'] if r!=omitted]
            self.rows[1]['repeats'][2]['event'] = self.put(record['path'], changed)
            self.save_collection()
            with self.subTest(omitted=omitted['path']), self.assertRaisesRegex(ValueError, 'captured'):
                self.check()

    def test_last_event_missing_validator_tool_capture_rejects(self):
        self.edit_repeat(1, 2, 'event', lambda event:event.update(
            inputs=[r for r in event['inputs'] if r!=self.tool_refs[0]]))
        with self.assertRaisesRegex(ValueError, 'tools were not captured'):
            self.check()

    def test_unsupported_actual_control_remains_open(self):
        self.d['categories']['range_faults'][0]['evidence_kind'] = 'actual_control'
        self.refresh_predeclared_fixture()
        with self.assertRaisesRegex(ValueError, 'Unsupported actual control'):
            self.check()
        self.outcome('range_faults').update(status='OPEN', reason='Actual refusal adapter unimplemented')
        self.decision['status'] = 'SCOPED_PROOF'
        self.assertEqual(self.check()['status'], 'SCOPED_PROOF')

    def test_probe_integrity_uses_actual_controls_not_pass_labels(self):
        self.d['categories']['range_faults'][0]['evidence_kind'] = 'probe_integrity'
        self.refresh_predeclared_fixture()
        self.assertEqual(self.check()['status'], 'COVERAGE_COMPLETE')
        self.edit_repeat(1, 2, 'observation', lambda r:r.update(invalid_descriptor_controls=6))
        with self.assertRaisesRegex(ValueError, 'integrity controls'):
            self.check()

    def test_missing_or_mismatched_scene_disposition_rejects(self):
        for change in (lambda r:r.update(identity=['external',429,'genIFS']),
                       lambda r:r['candidate'].update(probe=self.model)):
            original = copy.deepcopy(self.disposition)
            change(self.disposition); self.save_disposition()
            with self.assertRaisesRegex(ValueError, 'scene disposition binding'):
                self.check()
            self.disposition = original
        self.decision['scene_disposition'] = {'path':'absent.json','sha256':'0'*64}
        with self.assertRaisesRegex(ValueError, 'Missing/stale'):
            self.check()

    def test_declared_open_context_still_requires_scene_disposition(self):
        self.inv['contexts'] = [{'id':'pending-caller','scene_ids':['0'],'parameters_sha256':'1'*64,
            'program_sha256':'2'*64,'resources_sha256':'3'*64,'capability':'mixed',
            'artifacts':[self.source]}]
        self.d['inventory'] = self.put('inventory.json', self.inv)
        self.d['context_ids'] = ['pending-caller']
        self.d['categories']['scene_contexts'][0].update(applicability='REQUIRED', na_reason='',
            evidence_kind='actual_control', context_ids=['pending-caller'], capability='mixed')
        self.refresh_predeclared_fixture()
        self.outcome('scene_contexts').update(status='OPEN', reason='Caller observation not implemented')
        self.decision['status'] = 'SCOPED_PROOF'
        with self.assertRaisesRegex(ValueError, 'Missing affected-scene disposition'):
            self.check()
        self.disposition['contexts'] = [dict(id='pending-caller', status='UNRELATED_OPEN',
            reason='Synthetic unit disposition; caller obligation remains OPEN', evidence=[self.source])]
        self.disposition['evidence'] = [self.source]
        self.save_disposition()
        self.assertEqual(self.check()['status'], 'SCOPED_PROOF')

    def test_unknown_or_missing_obligation_and_future_fields_reject(self):
        original = copy.deepcopy(self.decision)
        for mutate in (lambda d:d['obligations'].pop(),
                       lambda d:d['obligations'].append(copy.deepcopy(d['obligations'][0])),
                       lambda d:d.update(admission_receipt={'path':'future','sha256':'0'*64}),
                       lambda d:d.update(ledger={'path':'future','sha256':'0'*64})):
            self.decision = copy.deepcopy(original); mutate(self.decision)
            with self.assertRaises(ValueError):
                self.check()

    def test_verified_requires_coverage_receipt_and_both_gate_results(self):
        self.make_ledger()
        self.assertEqual(self.check_ledger()['status'], 'FORMULA_VERIFIED')
        self.common_gate.assert_called_once_with(self.root/'package/contract.json', self.root/'package')
        self.common_gate.return_value = {'accepted':[]}
        with self.assertRaisesRegex(ValueError, 'Common gate rejected'):
            self.check_ledger()
        self.common_gate.return_value = {'accepted':[self.accepted]}
        self.ledger['qualification'] = self.put('package/qualification.json', {'accepted':[]})
        with self.assertRaisesRegex(ValueError, 'Common gate rejected'):
            self.check_ledger()

    def test_legacy_receipt_without_coverage_cannot_make_verified(self):
        self.make_ledger()
        self.receipt.pop('coverage')
        self.ledger['admission_receipt'] = self.put('package/receipt.json', self.receipt)
        with self.assertRaisesRegex(ValueError, 'coverage-bound admission'):
            self.check_ledger()
        self.common_gate.assert_not_called()

    def test_ledger_wrong_identity_candidate_or_disposition_rejects(self):
        self.make_ledger(); original = copy.deepcopy(self.ledger)
        for key, value in (('identity',['external',429,'genIFS']),
                           ('candidate',dict(self.candidate, probe=self.model)),
                           ('scene_disposition',self.dr)):
            self.ledger = copy.deepcopy(original); self.ledger[key] = value
            with self.subTest(key=key), self.assertRaisesRegex(ValueError, 'Ledger coverage binding'):
                self.check_ledger()
        self.common_gate.assert_not_called()

    def test_scoped_proof_cannot_become_verified(self):
        self.outcome('rounded_finite').update(status='OPEN', reason='Rounded domain unresolved')
        self.decision['status'] = 'SCOPED_PROOF'
        self.make_ledger()
        with self.assertRaisesRegex(ValueError, 'Incomplete formula'):
            self.check_ledger()
        self.common_gate.assert_not_called()

    def test_ledger_receipt_must_belong_to_accepted_package(self):
        self.make_ledger()
        self.evidence['formulas'][0]['proof'] = self.put('package/other-receipt.json', self.receipt)
        self.ledger['evidence'] = self.put('package/evidence.json', self.evidence)
        with self.assertRaisesRegex(ValueError, 'Receipt absent'):
            self.check_ledger()
        self.common_gate.assert_not_called()

    def test_ledger_candidate_a_cannot_reuse_candidate_b_receipt(self):
        self.make_ledger()
        self.package_binding['library'] = self.put('other-library', 'OTHER CANDIDATE', text=True)
        self.receipt['binding_sha256'] = c.digest(self.package_binding)
        self.ledger['admission_receipt'] = self.put('package/receipt.json', self.receipt)
        self.evidence['formulas'][0]['proof'] = self.ledger['admission_receipt']
        self.ledger['evidence'] = self.put('package/evidence.json', self.evidence)
        with self.assertRaisesRegex(ValueError, 'covered runtime/observer'):
            self.check_ledger()
        self.common_gate.assert_not_called()

    def test_ledger_admission_cannot_substitute_other_case_processes(self):
        self.make_ledger()
        self.proof['cases'][0]['repeats'] = copy.deepcopy(self.proof['cases'][1]['repeats'])
        self.receipt['observations'] = self.put('package/observations.json', self.proof)
        self.ledger['admission_receipt'] = self.put('package/receipt.json', self.receipt)
        self.evidence['formulas'][0]['proof'] = self.ledger['admission_receipt']
        self.ledger['evidence'] = self.put('package/evidence.json', self.evidence)
        with self.assertRaisesRegex(ValueError, 'covered probe/processes'):
            self.check_ledger()
        self.common_gate.assert_not_called()

    def test_ledger_admission_requires_covered_collection_and_build(self):
        self.make_ledger()
        original = self.receipt['artifacts'].copy()
        for missing in original:
            self.receipt['artifacts'] = [r for r in original if r!=missing]
            self.ledger['admission_receipt'] = self.put('package/receipt.json', self.receipt)
            self.evidence['formulas'][0]['proof'] = self.ledger['admission_receipt']
            self.ledger['evidence'] = self.put('package/evidence.json', self.evidence)
            with self.subTest(missing=missing['path']), self.assertRaisesRegex(ValueError, 'collection/build provenance'):
                self.check_ledger()
        self.common_gate.assert_not_called()

    def test_ledger_cannot_point_at_other_file_than_common_gate_reads(self):
        self.make_ledger()
        self.ledger['evidence'] = self.put('package/other.json', self.evidence)
        with self.assertRaisesRegex(ValueError, 'evidence|manifest|package'):
            self.check_ledger()
        self.common_gate.assert_not_called()

    def test_ledger_future_fields_and_foreign_qualification_reject(self):
        self.make_ledger()
        self.ledger['future_approval'] = self.dr
        with self.assertRaisesRegex(ValueError, 'ledger.*fields'):
            self.check_ledger()
        self.ledger.pop('future_approval')
        self.ledger['qualification'] = self.put('other-package/qualification.json', self.qualification)
        with self.assertRaisesRegex(ValueError, 'qualification package'):
            self.check_ledger()


if __name__=='__main__':unittest.main()
