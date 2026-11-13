"""Synthetic validator tests only; no device or numerical acceptance credit."""
from contextlib import ExitStack, redirect_stdout
import copy
import io
import json
from pathlib import Path
import tempfile
import unittest
from unittest.mock import patch

import native_translate_transport_controls as t


class SourceCorpus(unittest.TestCase):
    @classmethod
    def setUpClass(cls): cls.cases = t.build_cases()

    def test_declared_partition_and_unchanged_original_programs(self):
        self.assertEqual(len(self.cases), 22)
        original = {r['id']: r for r in t.mixed.build_cases()}
        mixed = [r for r in self.cases if r['rule'] == 'RETURN_SSE_ONLY']
        self.assertEqual(len(mixed), 9)
        self.assertEqual(sum(r['source_slot_visits'][1] > 0 for r in mixed), 6)
        for row in mixed:
            p, s = t.observer.validate_case(original[row['id']]['input'])
            self.assertEqual((row['program_words'], row['input_words']), (p, s))
            self.assertEqual((p[7], p[50:53]), (400, [30, 0x80000001, 5]))

    def test_exclusions_use_valid_actual_metadata_and_distinct_conditions(self):
        rows = {r['id']: r for r in self.cases}; at = 388
        self.assertEqual(rows['disabled-negative']['program_words'][at+9], 0)
        self.assertEqual(rows['outside-negative']['program_words'][5], 0)
        self.assertEqual(rows['intmin-negative']['input_words'][38], 0x80000000)
        p = rows['sse-consumer-unreachable']['program_words']
        self.assertEqual((p[at+2], p[at+5] & 8, p[at+9], p[5]), (10262, 8, 1, 0))
        self.assertEqual(tuple(p[at+3:at+8]), t.observer.descriptor_metadata()[0][10262])
        self.assertEqual(rows['smoothing-low-branch']['input_words'][45], 1)
        self.assertEqual([r['expected_fault'] for r in self.cases].count(5), 2)

    def test_every_non_sse_word_is_protected(self):
        row = next(r for r in self.cases if r['rule'] == 'RETURN_SSE_ONLY')
        old = row['input_words'].copy()+[0]*12; old[3] = 0x3f800000; old[51] = row['expected_work']; old[55] = 0x40000000
        new = old.copy(); new[20:24] = [old[55], 0, old[3], 0]
        self.assertEqual(t.compare(row, old, new)['status'], 'PASS_CONTROL')
        for index in t.OTHER:
            bad = new.copy(); bad[index] ^= 1
            with self.subTest(index=index), self.assertRaises(ValueError): t.compare(row, old, bad)
        for index in t.SSE:
            bad = new.copy(); bad[index] ^= 1
            with self.subTest(index=index), self.assertRaises(ValueError): t.compare(row, old, bad)


class SyntheticCollection(unittest.TestCase):
    @classmethod
    def setUpClass(cls): cls.source = t.build_cases()

    def setUp(self):
        self.temp = tempfile.TemporaryDirectory(); self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name).resolve(); self.output = self.root/'capture'
        self.helper = self.root/'helper.py'; self.helper.write_text('# SYNTHETIC FIXTURE\n')
        self.cases = [copy.deepcopy(next(r for r in self.source if r['id'] == name)) for name in
                      ('positive-de2-count1', 'translate-lightbulbmoon-03', 'positive-work-budget')]
        self.old = self.root/t.OLD_BUILD; self.new = self.root/'new/build'
        for build in (self.old, self.new):
            build.mkdir(parents=True)
            for name in ('mb3d_native_formula_probe', 'metal_native_scene.metallib'):
                (build/name).write_text(str(build)+' SYNTHETIC')
            (build.parent/'build-receipt.json').write_text('{}')
        self.invocations = []; self.mutate = None; self.after = None; self.reuse = False
        stack = self.enterContext(ExitStack()); stack.enter_context(redirect_stdout(io.StringIO()))
        stack.enter_context(patch.object(t, 'ROOT', self.root))
        stack.enter_context(patch.object(t, 'build_cases', side_effect=lambda: copy.deepcopy(self.cases)))
        stack.enter_context(patch.object(t, 'dependencies', return_value=[self.helper]))
        stack.enter_context(patch.object(t, 'build_binding', side_effect=self.build_binding))
        stack.enter_context(patch.object(t, 'invoke', side_effect=self.invoke))
        t.prepare(self.root/'predevice')
        self.manifest = self.root/'predevice/manifest.json'; self.review = self.root/'review.json'
        t.write(self.review, {'schema': 'mb3d.translate-transport-predevice-review.v1', 'status': 'PASS_CONTROLS_ONLY',
                             'qualification': False, 'manifest': t.ref(self.manifest), 'author': t.AUTHOR,
                             'reviewer': 'INDEPENDENT SYNTHETIC REVIEW FIXTURE'})

    def build_binding(self, build, old=False):
        paths = {'probe': build/'mb3d_native_formula_probe', 'library': build/'metal_native_scene.metallib',
                 'receipt': build.parent/'build-receipt.json'}
        return {k: t.ref(p) for k, p in paths.items()}, list(paths.values())

    def invoke(self, argv, folder, cwd, timeout, **kwargs):
        folder.mkdir(parents=True); self.invocations.append((argv, kwargs)); number = len(self.invocations)
        opts = dict(zip(argv[1::2], argv[2::2])); case = next(r for r in self.cases if r['id'] == folder.parents[1].name)
        words = case['input_words'].copy()+[0]*12
        words[3] = 0x3f800000; words[47] = case['expected_fault']; words[51] = case['expected_work']
        words[52] = case['program_words'][11]; words[55] = 0x40000000; words[56] = 1
        if folder.parent.name == 'candidate' and case['rule'] == 'RETURN_SSE_ONLY': words[20:24] = [words[55], 0, words[3], 0]
        if words[47]: words[48:51] = [8065, 0, 1]; words[55:57] = [0, 0]
        report = {'schema': 'mb3d.native-formula-probe.v1', 'status': 'POINT_FAULTS' if words[47] else 'COMPLETE',
                  'profile': 'native_gpu_fp32', 'library_profile': [2,4,32], 'formula_abi': [1,8192,188,236,32],
                  'scene': None, 'program': opts['--program'], 'library': opts['--library'], 'mode': 1,
                  'slot': case['slot'], 'point_count': 1, 'repeats': 3, 'wrapper_policy': 1,
                  'exact_state_repeats': True, 'input_immutable': True, 'allocation_canaries': True,
                  'invalid_descriptor_controls': 7, 'device': 'SYNTHETIC NO DEVICE CREDIT',
                  'wrapper_audit_words': [1,1,0]*6, 'program_words': case['program_words'],
                  'fault_count': int(bool(words[47])),
                  'rows': [{'index': 0, 'input_words': case['input_words'], 'result_words': words,
                            'fault': words[47], 'work': words[51]}]}
        event = {'schema': 'mb3d.gpu-process.v2', 'pid': 1 if self.reuse else number, 'started_ns': 1,
                 'finished_ns': 1000001, 'seconds': .001, 'effective_timeout_seconds': timeout,
                 'owned_process_reaped': True, 'run_id': kwargs['run_id'], 'argv': argv, 'cwd': str(cwd),
                 'executable_sha256_before': t.sha(argv[0]), 'executable_sha256_after': t.sha(argv[0]),
                 'exit_code': 2 if words[47] else 0, 'guard': None,
                 'inputs': [t.ref(p) for p in kwargs['input_files']]}
        if self.mutate: self.mutate(report, event, folder, number)
        t.write(Path(opts['--output']), report); (folder/'process.log').write_text('SYNTHETIC NO GPU')
        event.update(outputs=[t.ref(Path(opts['--output']))], log=t.ref(folder/'process.log'))
        t.write(folder/'event.json', event)
        if self.after: self.after(number)
        return event

    def collect(self): return t.collect(self.manifest, self.review, self.new, self.output)

    def test_complete_synthetic_capture_and_independent_replay(self):
        result = self.collect()
        self.assertEqual(result['status'], 'PASS_CONTROLS_ONLY')
        self.assertEqual(t.validate(self.output/'transport-controls.json')['actual_processes'], 18)
        self.assertTrue(all(k['exclusive_gpu'] is True for _,k in self.invocations))
        self.assertTrue(all('--range-controls' not in a for a,_ in self.invocations))

    def test_non_sse_change_fails_without_stopping_remaining_controls(self):
        def mutate(r, e, folder, n):
            if folder.parent.name == 'candidate' and 'lightbulbmoon' in folder.parents[1].name:
                r['rows'][0]['result_words'][24] ^= 1
        self.mutate = mutate
        result = self.collect()
        self.assertEqual(result['status'], 'CONTROLS_FAILED'); self.assertEqual(len(self.invocations), 18)
        with self.assertRaises(ValueError): t.validate(self.output/'transport-controls.json')

    def test_repeat_difference_aborts(self):
        self.mutate = lambda r,e,f,n: r['rows'][0]['result_words'].__setitem__(30, n)
        with self.assertRaisesRegex(ValueError, 'repeats differ'): self.collect()

    def test_duplicate_actual_process_rejected(self):
        self.reuse = True
        with self.assertRaisesRegex(ValueError, 'Reused actual process'): self.collect()

    def test_input_closure_omission_rejected(self):
        self.mutate = lambda r,e,f,n: e.__setitem__('inputs', [])
        with self.assertRaisesRegex(ValueError, 'Missing captured'): self.collect()

    def test_source_mutation_after_final_process_rejected(self):
        self.after = lambda n: self.helper.write_text('changed') if n == 18 else None
        with self.assertRaisesRegex(ValueError, 'stale artifact|changed after process'): self.collect()

    def test_wrong_executable_command_rejected(self):
        self.mutate = lambda r,e,f,n: e.__setitem__('argv', e['argv']+['--range-controls','active'])
        with self.assertRaisesRegex(ValueError, 'Wrong executed command'): self.collect()

    def test_wrong_exit_for_predeclared_fault_rejected(self):
        def mutate(r,e,f,n):
            if r['fault_count']: e['exit_code'] = 9
        self.mutate = mutate
        with self.assertRaisesRegex(ValueError, 'Unexpected probe process result'): self.collect()

    def test_fault_limit_and_fault_type_are_not_normalized_away(self):
        def mutate(r,e,f,n):
            if r['fault_count']: r['rows'][0]['result_words'][52] += 1
        self.mutate = mutate
        with self.assertRaisesRegex(ValueError, 'work-budget fault state'): self.collect()

    def test_guarded_failure_rejected(self):
        def mutate(r,e,f,n):
            if r['fault_count']: e['guard'] = 'TIMEOUT'
        self.mutate = mutate
        with self.assertRaises(ValueError): self.collect()

    def test_bool_abi_and_repeat_counts_rejected(self):
        self.mutate = lambda r,e,f,n: r.__setitem__('mode', True)
        with self.assertRaisesRegex(ValueError, 'Wrong probe dispatch'): self.collect()

    def test_missing_or_self_review_fails_before_process(self):
        for reviewer in ('', t.AUTHOR, ' '+t.AUTHOR+' ', 3):
            review = t.observer.read_json(self.review); review['reviewer'] = reviewer; t.write(self.review, review)
            with self.subTest(reviewer=reviewer), self.assertRaisesRegex(ValueError, 'Independent predevice'): self.collect()
        self.assertFalse(self.invocations)

    def test_changed_corpus_fails_before_process(self):
        path = self.root/'predevice/corpus.json'; value = t.observer.read_json(path)
        value['cases'][0]['input_words'][0] ^= 1; t.write(path, value)
        with self.assertRaises(ValueError): self.collect()
        self.assertFalse(self.invocations)

    def test_replay_does_not_trust_stored_comparison_or_case_omission(self):
        self.collect(); path = self.output/'transport-controls.json'; value = t.observer.read_json(path)
        value['cases'][0]['comparison']['exact_other_words'] = 58; t.write(path, value)
        with self.assertRaisesRegex(ValueError, 'Unreplayed comparison'): t.validate(path)
        value['cases'].pop(); t.write(path, value)
        with self.assertRaisesRegex(ValueError, 'Incomplete/reordered'): t.validate(path)


if __name__ == '__main__': unittest.main()
