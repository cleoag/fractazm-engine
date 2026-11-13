"""Synthetic admission tamper controls; no device qualification or GPU use."""
import copy
import json
from pathlib import Path
import tempfile
from types import SimpleNamespace
import unittest
from unittest.mock import patch

import verify_native_formula_group as verifier


class CorpusAdmission(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.root = Path(self.tmp.name)
        self.identity = ('external', 8065, '_Translate')
        self.corpus = []
        rows = []
        for kind in ('point', 'orbit'):
            payload = {'identity': list(self.identity), 'kind': kind, 'input_state_bits': {}, 'parameter_bits': []}
            path = self.root / (kind + '.json')
            path.write_text(json.dumps(payload))
            original = dict(id=kind, identity=list(self.identity), variant=kind,
                            input=payload, expected_state_bits={}, read_lanes=['xyz[0]'],
                            write_lanes=['xyz[0]'], source_hashes={})
            self.corpus.append(original)
            rows.append(dict(original, input={'path': path.name, 'sha256': verifier.sha(path)}))
        self.data = {'cases': rows}

    def tearDown(self):
        self.tmp.cleanup()

    def validate(self):
        with patch.object(verifier, 'ROOT', self.root):
            return verifier.validate_corpus(self.data, {self.identity}, self.corpus)

    def test_complete_exact_corpus(self):
        self.assertEqual(set(self.validate()), {'point', 'orbit'})

    def test_missing_case(self):
        self.data['cases'].pop()
        with self.assertRaisesRegex(Exception, 'Incomplete'):
            self.validate()

    def test_duplicate_cannot_replace_missing_case(self):
        self.data['cases'][1] = copy.deepcopy(self.data['cases'][0])
        with self.assertRaisesRegex(Exception, 'duplicate'):
            self.validate()

    def test_changed_contract_lanes(self):
        self.data['cases'][0]['write_lanes'] = ['j[0]']
        with self.assertRaisesRegex(Exception, 'metadata'):
            self.validate()

    def test_rehashed_wrong_semantic_input(self):
        path = self.root / 'point.json'
        payload = json.loads(path.read_text())
        payload['kind'] = 'orbit'
        path.write_text(json.dumps(payload))
        self.data['cases'][0]['input']['sha256'] = verifier.sha(path)
        with self.assertRaisesRegex(Exception, 'input changed'):
            self.validate()


class CompiledProductBinding(unittest.TestCase):
    def test_receipt_cannot_name_arbitrary_device_host(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            probe, library = root/'probe', root/'library'
            probe.write_bytes(b'synthetic original host')
            library.write_bytes(b'synthetic library')
            records = []
            for path in (probe, library):
                record = root/(path.name + '.json')
                record.write_text(json.dumps({'exit_code': 0, 'capture_errors': [],
                    'output': {'path': str(path), 'sha256': verifier.sha(path)}}))
                records.append({'path': record.name, 'sha256': verifier.sha(record)})
            receipt = root/'receipt.json'
            receipt.write_text(json.dumps({'status': 'PASS_CAPTURED_BUILD_CLOSURE',
                'commands': 2, 'objects': 1, 'airs': 1, 'records': records,
                'products': [{'path': p.name, 'sha256': verifier.sha(p)} for p in (probe, library)]}))
            verifier.validate_build_products(receipt, probe, library)
            probe.write_bytes(b'different host')
            with self.assertRaisesRegex(Exception, 'absent from captured build'):
                verifier.validate_build_products(receipt, probe, library)


class ReferenceClosure(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.root = Path(self.tmp.name).resolve()
        self.folder = self.root/'comparison/tools'
        self.folder.mkdir(parents=True)
        self.model = self.folder/'model.py'; self.model.write_text('# independent fixture model\n')
        self.primitive = self.folder/'primitive.py'; self.primitive.write_text('# independently checked operations\n')
        self.reference = SimpleNamespace(__file__=str(self.model), manifest=lambda: {'scope': 'synthetic'},
                                         dependency_paths=lambda: [self.primitive])
        self.patches = [patch.object(verifier, 'ROOT', self.root),
                        patch.object(verifier, 'ref', side_effect=self.record)]
        for item in self.patches: item.start()
        self.data = {'reference_group': 'fold', 'reference_model': self.record(self.model),
                     'reference_manifest': self.reference.manifest(),
                     'reference_dependencies': [self.record(p) for p in verifier.reference_inputs(self.reference)]}

    def tearDown(self):
        for item in reversed(self.patches): item.stop()
        self.tmp.cleanup()

    def record(self, path):
        return {'path': str(Path(path).relative_to(self.root)), 'sha256': verifier.sha(path)}

    def test_full_imported_reference_closure(self):
        self.assertEqual(verifier.validate_reference(self.data, self.reference), self.data['reference_dependencies'])

    def test_changed_imported_primitive_rejected(self):
        self.primitive.write_text('# changed expected-value computation\n')
        with self.assertRaisesRegex(Exception, 'dependencies changed'):
            verifier.validate_reference(self.data, self.reference)

    def test_missing_closure_not_legacy_fold(self):
        del self.data['reference_dependencies']
        with self.assertRaisesRegex(Exception, 'Missing reference'):
            verifier.validate_reference(self.data, self.reference)

    def test_legacy_affine_collection_still_supported(self):
        del self.reference.dependency_paths
        del self.data['reference_dependencies']
        del self.data['reference_group']
        self.assertEqual(verifier.validate_reference(self.data, self.reference), [self.record(self.model)])

    def test_uncaptured_expected_value_dependency_rejected(self):
        dependencies = self.data['reference_dependencies']
        with self.assertRaisesRegex(Exception, 'not captured'):
            verifier.validate_reference_event({'inputs': dependencies[:-1]}, dependencies)
        verifier.validate_reference_event({'inputs': dependencies}, dependencies)

    def test_unregistered_module_does_not_import(self):
        with patch.object(verifier.importlib, 'import_module') as importer:
            for value in ('untrusted.module', '../model', [], None):
                with self.assertRaisesRegex(Exception, 'Unknown independent'):
                    verifier.load_reference(value)
            importer.assert_not_called()

    def test_import_shadow_is_rejected(self):
        with patch.object(verifier.importlib, 'import_module', return_value=self.reference):
            with self.assertRaisesRegex(Exception, 'outside audited'):
                verifier.load_reference('affine')

    def test_dependency_outside_tools_is_rejected(self):
        self.reference.dependency_paths = lambda: [self.root/'arbitrary.py']
        with self.assertRaisesRegex(Exception, 'Invalid reference dependency'):
            verifier.reference_inputs(self.reference)

    def test_explicit_original_profile_data_is_captured_and_pinned(self):
        profile=self.root/'original-profile.bin';profile.write_bytes(b'original compiler evidence')
        self.reference.data_paths=lambda:[profile]
        deps=[self.record(p) for p in verifier.reference_inputs(self.reference)]
        self.assertIn(self.record(profile),deps)
        self.data['reference_dependencies']=deps
        verifier.validate_reference(self.data,self.reference)
        profile.write_bytes(b'changed')
        with self.assertRaisesRegex(Exception,'dependencies changed'):
            verifier.validate_reference(self.data,self.reference)

    def test_external_original_profile_data_is_rejected(self):
        self.reference.data_paths=lambda:[self.root.parent/'outside.bin']
        with self.assertRaisesRegex(Exception,'source/profile dependency'):
            verifier.reference_inputs(self.reference)



class RequiredCoverageBeforeAdmission(unittest.TestCase):
    def test_missing_coverage_fails_before_read_or_package_mutation(self):
        with patch.object(verifier,'read') as reader:
            with self.assertRaisesRegex(ValueError,'coverage'):
                verifier.admit(Path('missing'),Path('missing'),Path('missing'),Path('missing'))
            reader.assert_not_called()

class AnnotatedCollectorCapture(unittest.TestCase):
    """Synthetic collect integration: invoke is replaced; no GPU process runs."""
    def setUp(self):
        from test_native_formula_coverage import CoverageDecision
        self.fixture=CoverageDecision('test_complete_decision_replays_each_mapped_case_once')
        self.fixture.setUp()
        self.addCleanup(self.fixture.tearDown)
        self.addCleanup(self.fixture.doCleanups)
        f=self.fixture
        adapter=patch.object(verifier.observer,'__file__',str(f.root/f.candidate['observer']['path']))
        adapter.start();self.addCleanup(adapter.stop)
        self.build=f.root/'build';self.build.mkdir()
        for name in ('mb3d_native_formula_probe','metal_native_scene.metallib'):
            (self.build/name).write_bytes(b'SYNTHETIC; NOT DEVICE EVIDENCE')
        (f.root/'build-receipt.json').write_text('{}')
        self.output=f.root/'new-collection'

    def collect(self):
        f=self.fixture
        return verifier.collect(self.build,self.output,[f.identity[2]],'synthetic_unit',
                                f.root/f.dr['path'],f.root/f.rr['path'],f.root/f.mr['path'])

    def synthetic_invoke(self,args,run,*positional,**kwargs):
        f=self.fixture;run.mkdir(parents=True)
        payload=json.loads(Path(args[args.index('--input')+1]).read_text())
        row=next(row for row in f.corpus if row['input']==payload)
        observed={'run_id':kwargs['run_id'],'input_digest':verifier.digest(payload),
                  'state_bits':row['expected_state_bits'],'result_words':[0]*59,
                  'gpu_invocations':3,'device':'SYNTHETIC UNIT FIXTURE; NO GPU'}
        Path(args[args.index('--output')+1]).write_text(json.dumps(observed))
        event={'run_id':kwargs['run_id'],'exit_code':0,'guard':None,
               'inputs':[verifier.ref(p) for p in kwargs['input_files']]}
        (run/'event.json').write_text(json.dumps(event))
        return event

    def test_collect_captures_predeclared_sources_reviews_and_tools_each_time(self):
        f=self.fixture
        with patch.object(verifier.observer,'dependency_paths',return_value=[]), \
             patch.object(verifier,'invoke',side_effect=self.synthetic_invoke) as invoke:
            result=self.collect()
        self.assertEqual(invoke.call_count,6)
        self.assertEqual(result['case_counts'],{'PASS_CASE':2})
        for row in result['cases']:
            for repeat in row['repeats']:
                event=json.loads((f.root/repeat['event']['path']).read_text())
                for record in result['coverage_inputs']+result['coverage_tools']:
                    self.assertIn(record,event['inputs'])

    def test_changed_review_rejects_before_dispatch_or_output_creation(self):
        f=self.fixture;f.mr_data['corpus_sha256']='0'*64
        f.put('model-review.json',f.mr_data)
        with patch.object(verifier,'invoke') as invoke,self.assertRaisesRegex(ValueError,'reviewed corpus'):self.collect()
        invoke.assert_not_called();self.assertFalse(self.output.exists())

    def test_changed_review_during_capture_rejects_before_next_dispatch(self):
        f=self.fixture
        def changed(*args,**kwargs):
            event=self.synthetic_invoke(*args,**kwargs)
            (f.root/f.rr['path']).write_text('{"changed":true}')
            return event
        with patch.object(verifier.observer,'dependency_paths',return_value=[]), \
             patch.object(verifier,'invoke',side_effect=changed) as invoke,self.assertRaises(ValueError):self.collect()
        self.assertEqual(invoke.call_count,1)


    def test_sealed_ledger_survives_later_aggregate_manifest_change(self):
        f=self.fixture;f.make_ledger();package=f.root/'package'
        row={'namespace':f.identity[0],'id':f.identity[1],'name':f.identity[2],'proof':f.receipt_ref}
        verified=verifier.publish_verified(package,row,f.evidence,f.qualification,f.decision_ref,
                                            f.decision,package/'contract.json')
        f.put('package/evidence.json',{'later':'different aggregate manifest'})
        f.put('package/qualification.json',{'accepted':[]})
        import native_formula_coverage as coverage
        ledger=coverage.ref(f.root,package/'formula-proofs/8065/verified.json')
        self.assertEqual(coverage.validate_verified(f.root,ledger),verified)
        self.assertEqual(f.common_gate.call_args.args[1],package/'formula-proofs/8065/admission')
        with self.assertRaises(FileExistsError):
            verifier.publish_verified(package,row,f.evidence,f.qualification,f.decision_ref,
                                        f.decision,package/'contract.json')

    def test_failed_common_gate_never_publishes_verified_entry(self):
        f=self.fixture;f.make_ledger();f.common_gate.return_value={'accepted':[]};package=f.root/'package'
        row={'namespace':f.identity[0],'id':f.identity[1],'name':f.identity[2],'proof':f.receipt_ref}
        with self.assertRaisesRegex(ValueError,'Common gate'):
            verifier.publish_verified(package,row,f.evidence,f.qualification,f.decision_ref,
                                        f.decision,package/'contract.json')
        self.assertFalse((package/'formula-proofs/8065/verified.json').exists())


if __name__ == '__main__':
    unittest.main()
