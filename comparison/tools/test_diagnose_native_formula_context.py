"""Labelled synthetic collection controls only; no device/numerical credit."""
from contextlib import ExitStack, redirect_stdout
import copy
import io
import json
from pathlib import Path
import struct
import tempfile
import unittest
from unittest.mock import patch

import diagnose_native_formula_context as d


class SourceClosure(unittest.TestCase):
    def test_compiled_caller_data_dependencies_are_pinned_and_rechecked(self):
        with tempfile.TemporaryDirectory() as temp:
            root = Path(temp).resolve()
            profile = root / 'original-profile.json'; profile.write_text('{"synthetic":true}')
            manifest = {'source_hashes': {}, 'helper_hashes': {}, 'context_dependencies': {},
                        'data_dependencies': {profile.name: d.sha(profile)}}
            with patch.object(d, 'ROOT', root):
                self.assertIn(profile, d.dependency_closure(manifest, []))
                profile.write_text('{"synthetic":"changed"}')
                with self.assertRaisesRegex(ValueError, 'Model source/data changed'):
                    d.dependency_closure(manifest, [])


class SyntheticCollection(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name).resolve()
        self.build = self.root / 'candidate/build'; self.build.mkdir(parents=True)
        for name in ('mb3d_native_formula_probe', 'metal_native_scene.metallib'):
            (self.build / name).write_bytes(b'SYNTHETIC TEST ARTIFACT')
        (self.build.parent / 'build-receipt.json').write_text('{}')
        self.helper = self.root / 'synthetic-model.py'; self.helper.write_text('# synthetic fixture\n')
        self.adapter = self.root / 'synthetic-observer.py'; self.adapter.write_text('# synthetic fixture\n')
        self.output = self.root / 'diagnostic'
        self.program, self.state = [0] * 2048, [0] * 47
        semantic = {'identity': ['external', 8065, '_Translate'], 'kind': 'orbit',
                    'input_state_bits': dict.fromkeys(d.observer.STATE, '0x00000000'),
                    'parameter_bits': ['0x00000000'] * 2048,
                    'context': {'target_slot': 1}}
        known = {k: '0x00000000' for k in d.observer.STATE if not k.startswith('sse[')}
        unknown = {k: 'UNPROVED' for k in d.observer.STATE if k.startswith('sse[')}
        self.cases = [{'id': f'synthetic-{i:02d}', 'identity': semantic['identity'],
                       'input': copy.deepcopy(semantic), 'semantic_diagnostic': {
                           'status': 'OPEN_CALLER_SSE_PROFILE', 'expected_state_bits': dict(known),
                           'unproved_fields': dict(unknown), 'expected_work': 1}}
                      for i in range(10)]
        self.cases[0]['semantic_diagnostic'] = {'status': 'OPEN_MODEL_DOMAIN', 'reason': 'synthetic domain control'}
        self.invocations, self.mutate, self.after, self.reuse = [], None, None, False
        self.child_mutate = None
        self.review_mutate = None
        stack = self.enterContext(ExitStack())
        stack.enter_context(patch.object(d, 'ROOT', self.root))
        stack.enter_context(patch.object(d.group, 'validate_build_products'))
        stack.enter_context(patch.object(d.model, 'source_manifest', return_value={'fixture': 'SYNTHETIC_NOT_NUMERICAL'}))
        stack.enter_context(patch.object(d.model, '__file__', str(self.helper)))
        stack.enter_context(patch.object(d.model, 'build_cases', side_effect=lambda: copy.deepcopy(self.cases)))
        stack.enter_context(patch.object(d, 'dependency_closure', return_value=[self.helper, self.adapter]))
        stack.enter_context(patch.object(d.observer, '__file__', str(self.adapter)))
        stack.enter_context(patch.object(d.observer, 'validate_case', return_value=(self.program, self.state)))
        stack.enter_context(patch.object(d.observer, 'dependency_paths', return_value=[self.helper, self.adapter]))
        stack.enter_context(patch.object(d.observer, 'replay_observation', side_effect=self.replay))
        stack.enter_context(patch.object(d, 'invoke', side_effect=self.invoke))
        stack.enter_context(redirect_stdout(io.StringIO()))

    def replay(self, root, observation, *_):
        return json.loads((root / observation['path']).read_text())

    def invoke(self, argv, folder, cwd, timeout, **kwargs):
        argv = list(map(str, argv)); folder.mkdir(parents=True)
        self.invocations.append((argv, kwargs))
        options = dict(zip(argv[1::2], argv[2::2]))
        baseline = '--program' in options
        if baseline:
            raw_path, pp = Path(options['--output']), Path(options['--program'])
            nested = folder
        else:
            nested = folder / 'observation.probe/process'; nested.mkdir(parents=True)
            pp = nested.parent / 'program.bin'; pp.write_bytes(struct.pack('<2048I', *self.program))
            (nested.parent / 'state.bin').write_bytes(struct.pack('<47I', *self.state))
            raw_path = nested / 'raw-probe.json'
        words = self.state.copy() + [0] * 12
        words[51] = words[56] = 1
        raw = {'schema': 'mb3d.native-formula-probe.v1', 'status': 'COMPLETE', 'profile': 'native_gpu_fp32',
               'library_profile': [2, 4, 32], 'formula_abi': [1, 8192, 188, 236, 32], 'scene': None,
               'program': str(pp), 'library': str(self.build / 'metal_native_scene.metallib'),
               'mode': 1, 'slot': 1, 'point_count': 1, 'repeats': 3, 'wrapper_policy': 1,
               'exact_state_repeats': True, 'input_immutable': True, 'allocation_canaries': True,
               'invalid_descriptor_controls': 7, 'device': 'SYNTHETIC FIXTURE',
               'wrapper_audit_words': [1, 1, 0] * 6, 'program_words': self.program, 'fault_count': 0,
               'rows': [{'index': 0, 'input_words': self.state, 'result_words': words, 'fault': 0, 'work': 1}]}
        if self.mutate:
            self.mutate(raw, baseline, len(self.invocations))
        raw_path.write_text(json.dumps(raw))
        (folder / 'process.log').write_text('SYNTHETIC FIXTURE')
        event = {'pid': 1 if self.reuse else len(self.invocations), 'started_ns': 1,
                 'run_id': kwargs['run_id'], 'owned_process_reaped': True,
                 'inputs': [d.ref(p) for p in kwargs['input_files']], 'outputs': [d.ref(raw_path)],
                 'exit_code': 2 if raw['fault_count'] else 0, 'guard': None,
                 'schema': 'mb3d.gpu-process.v2', 'finished_ns': 1000001, 'seconds': .001,
                 'effective_timeout_seconds': 180, 'argv': argv, 'cwd': str(cwd),
                 'executable_sha256_before': d.sha(argv[0]), 'executable_sha256_after': d.sha(argv[0]),
                 'log': d.ref(folder / 'process.log')}
        if not baseline:
            child = copy.deepcopy(event)
            child.update(pid=1000 + len(self.invocations), started_ns=2, finished_ns=1000000,
                         run_id=f'{1000 + len(self.invocations):032x}',
                         argv=[str(self.build / 'mb3d_native_formula_probe'), '--program', str(pp),
                               '--states', str(nested.parent / 'state.bin'), '--library', raw['library'],
                               '--output', str(raw_path), '--mode', 'orbit', '--slot', '1',
                               '--repeats', '3', '--wrapper-policy', '1'],
                         executable_sha256_before=d.sha(self.build / 'mb3d_native_formula_probe'),
                         executable_sha256_after=d.sha(self.build / 'mb3d_native_formula_probe'))
            if self.child_mutate:
                self.child_mutate(child)
            (nested / 'event.json').write_text(json.dumps(child))
            event['exit_code'] = 1 if raw['fault_count'] else 0
            if not raw['fault_count']:
                observation = {'state_bits': dict(zip(d.observer.STATE, (f'0x{w:08x}' for w in words))),
                               'result_words': words, 'work': raw['rows'][0]['work']}
                Path(options['--output']).write_text(json.dumps(observation))
        (folder / 'event.json').write_text(json.dumps(event))
        if self.after:
            self.after()
        return event

    def collect(self):
        review={'schema':'mb3d.native-formula-context-model-review.v1','status':'PASS_SCOPED_SOURCE_MODEL',
                'qualification':False,'reviewer':'synthetic-independent-reviewer','model_author':'synthetic-author',
                'reference_model':d.ref(self.helper),'reference_manifest_sha256':d.digest(d.model.source_manifest()),
                'corpus_sha256':d.digest(self.cases),'reviewed_case_ids':[c['id'] for c in self.cases]}
        if self.review_mutate:self.review_mutate(review)
        path=self.root/'review.json';path.write_text(json.dumps(review))
        return d.collect_context(self.build, self.output,path)

    def test_changed_predevice_review_refuses_before_dispatch(self):
        self.review_mutate=lambda r:r.update(corpus_sha256='0'*64)
        with self.assertRaisesRegex(ValueError,'pre-device context model review'):self.collect()
        self.assertEqual(self.invocations,[])

    def test_missing_author_or_reviewer_refuses_before_dispatch(self):
        for field in ('reviewer', 'model_author'):
            with self.subTest(field=field):
                self.review_mutate = lambda review: review.pop(field)
                with self.assertRaisesRegex(ValueError, 'pre-device context model review'):
                    self.collect()
                self.assertFalse(self.output.exists())
        self.assertEqual(self.invocations, [])

    def test_empty_or_whitespace_author_or_reviewer_refuses_before_dispatch(self):
        for field in ('reviewer', 'model_author'):
            for value in ('', ' ', '\t\n'):
                with self.subTest(field=field, value=value):
                    self.review_mutate = lambda review: review.update({field: value})
                    with self.assertRaisesRegex(ValueError, 'pre-device context model review'):
                        self.collect()
                    self.assertFalse(self.output.exists())
        self.assertEqual(self.invocations, [])

    def test_non_string_author_or_reviewer_refuses_before_dispatch(self):
        for field in ('reviewer', 'model_author'):
            for value in (None, False, True, 0, 7, [], ['name'], {}, {'name': 'person'}):
                with self.subTest(field=field, value=value):
                    self.review_mutate = lambda review: review.update({field: value})
                    with self.assertRaisesRegex(ValueError, 'pre-device context model review'):
                        self.collect()
                    self.assertFalse(self.output.exists())
        self.assertEqual(self.invocations, [])

    def test_same_author_and_reviewer_including_padding_refuses_before_dispatch(self):
        for prefix, suffix in (('', ''), (' ', '\t')):
            with self.subTest(prefix=prefix, suffix=suffix):
                self.review_mutate = lambda review: review.update(
                    reviewer=prefix + review['model_author'] + suffix)
                with self.assertRaisesRegex(ValueError, 'pre-device context model review'):
                    self.collect()
                self.assertFalse(self.output.exists())
        self.assertEqual(self.invocations, [])

    def test_independent_review_is_captured_in_every_process(self):
        result=self.collect()
        for argv,kwargs in self.invocations:self.assertIn(self.root/'review.json',kwargs['input_files'])
        self.assertEqual(result['model_review'],d.ref(self.root/'review.json'))

    def test_ten_cases_three_new_observers_and_unchanged_baseline_each(self):
        got = self.collect()
        self.assertEqual(len(self.invocations), 40)
        self.assertEqual(len(got['cases']), 10)
        self.assertEqual(got['status'], 'DIAGNOSTIC_ONLY')
        self.assertFalse(got['qualification']); self.assertEqual(got['verified_formulas'], 0)
        self.assertEqual(got['cases'][0]['status'], 'OPEN_MODEL_DOMAIN')
        self.assertEqual(got['cases'][1]['status'], 'OPEN_CALLER_FIELDS')
        self.assertEqual(len(got['cases'][1]['compared_fields']), 43)
        self.assertEqual(len(got['cases'][1]['unproved_fields']), 4)
        self.assertTrue(all(kw['exclusive_gpu'] for _, kw in self.invocations))
        self.assertEqual(len({kw['run_id'] for _, kw in self.invocations}), 40)
        for argv, kw in self.invocations:
            if '--run-id' in argv:
                self.assertEqual(argv[argv.index('--run-id') + 1], kw['run_id'])
        self.assertEqual(got['dependencies_after'], got['dependencies_before'])

    def test_faulted_domain_is_retained_in_all_four_raw_captures(self):
        def mutate(raw, baseline, index):
            if index <= 4:
                raw.update(status='POINT_FAULTS', fault_count=1)
                raw['rows'][0]['fault'] = raw['rows'][0]['result_words'][47] = 7
        self.mutate = mutate
        row = self.collect()['cases'][0]
        self.assertEqual(row['status'], 'OPEN_MODEL_DOMAIN')
        self.assertEqual(row['observation_noninterference'], 'EXACT_RAW59_AND_WORK')
        self.assertTrue(all(c['fault'] == 7 for c in [row['baseline'], *row['repeats']]))
        self.assertTrue(all('observation' not in c for c in row['repeats']))

    def test_full59_noninterference_detects_non_state_tail_change(self):
        def mutate(raw, baseline, index):
            if not baseline:
                raw['rows'][0]['result_words'][52] = 2
        self.mutate = mutate
        got = self.collect()
        self.assertTrue(all(c['status'] == 'OBSERVATION_CHANGED' for c in got['cases']))

    def test_known43_mismatch_is_kept_without_target_learning(self):
        def mutate(raw, *_):
            raw['rows'][0]['result_words'][0] = 1
        self.mutate = mutate
        got = self.collect()
        self.assertEqual(got['cases'][1]['status'], 'MODEL_STATE_MISMATCH')
        self.assertEqual(got['cases'][1]['repeats'][0]['differences']['xyz[0]'],
                         {'expected': '0x00000000', 'actual': '0x00000001'})
        self.assertEqual(self.cases[1]['semantic_diagnostic']['expected_state_bits']['xyz[0]'], '0x00000000')

    def test_unproved_sse_difference_is_explicitly_not_compared(self):
        index = d.observer.STATE.index('sse[0]')
        self.mutate = lambda raw, *_: raw['rows'][0]['result_words'].__setitem__(index, 123)
        row = self.collect()['cases'][1]
        self.assertEqual(row['status'], 'OPEN_CALLER_FIELDS')
        self.assertNotIn('sse[0]', row['compared_fields'])
        self.assertIn('sse[0]', row['unproved_fields'])

    def test_work_mismatch_is_independent_of_state_match(self):
        def mutate(raw, *_):
            raw['rows'][0]['work'] = raw['rows'][0]['result_words'][51] = 2
        self.mutate = mutate
        self.assertEqual(self.collect()['cases'][1]['status'], 'MODEL_STATE_MISMATCH')

    def test_invalid_raw_report_remains_open_and_all_repeats_retained(self):
        self.mutate = lambda raw, *_: raw.update(program='wrong-program')
        got = self.collect()
        self.assertTrue(all(c['status'] == 'DEVICE_EXECUTION_FAILED' and len(c['repeats']) == 3 for c in got['cases']))

    def test_duplicate_process_identity_aborts_without_qualification(self):
        self.reuse = True
        with self.assertRaisesRegex(ValueError, 'Duplicate/unreaped'):
            self.collect()
        got = json.loads((self.output / 'diagnostic.json').read_text())
        self.assertEqual(got['status'], 'COLLECTION_ABORTED'); self.assertFalse(got['qualification'])

    def test_source_drift_aborts_and_captures_after_hash(self):
        self.after = lambda: self.helper.write_text('changed')
        with self.assertRaisesRegex(ValueError, 'closure changed|stale artifact'):
            self.collect()
        got = json.loads((self.output / 'diagnostic.json').read_text())
        self.assertEqual(got['status'], 'COLLECTION_ABORTED')
        self.assertNotEqual(got['dependencies_before'], got['dependencies_after'])

    def test_refuses_overwrite_and_incomplete_corpus_before_process(self):
        self.output.mkdir()
        with self.assertRaisesRegex(ValueError, 'fresh diagnostic'):
            self.collect()
        self.output.rmdir(); self.cases.pop()
        with self.assertRaisesRegex(ValueError, 'ten predeclared'):
            self.collect()
        self.assertEqual(self.invocations, [])

    def test_wrong_nested_executable_or_dispatch_stays_open(self):
        self.child_mutate = lambda child: child.update(executable_sha256_before='0' * 64)
        got = self.collect()
        self.assertTrue(all(row['status'] == 'DEVICE_EXECUTION_FAILED' for row in got['cases']))
        self.assertIn('Executable identity mismatch', got['cases'][0]['repeats'][0]['error'])

    def test_fault_canary_failure_is_not_noninterference_proof(self):
        def mutate(raw, *_):
            raw.update(status='POINT_FAULTS', fault_count=1, allocation_canaries=False)
            raw['rows'][0]['fault'] = raw['rows'][0]['result_words'][47] = 7
        self.mutate = mutate
        got = self.collect()
        self.assertTrue(all(row['status'] == 'DEVICE_EXECUTION_FAILED' and
                            row['observation_noninterference'] == 'OPEN' for row in got['cases']))


if __name__ == '__main__':
    unittest.main()
