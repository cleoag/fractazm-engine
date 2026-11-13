"""Synthetic observer/child fixtures only; these tests prove no GPU arithmetic."""
import argparse
import json
import os
import signal
import sys
from pathlib import Path
import struct
import tempfile
import unittest
from unittest.mock import patch

import native_formula_observer as o
import run_m3parameter_gpu as runner

IDENTITY = ['external', 8065, '_Translate']
REGISTRY = {tuple(IDENTITY)}


def case():
    program = [0] * 2048
    program[0], program[2], program[6] = 1, 6, 2
    program[7], program[11], program[12] = 4, 256, 256
    program[16], program[21], program[22] = 0x41800000, 0x3f800000, 0x3f800000
    for i in range(6):
        program[56 + i * 332] = 1
    program[57:59] = [1, 8065]
    program[59:64] = [0xffffffff, 3, 0, 1, 0]
    program[65] = 1
    return {'identity': IDENTITY.copy(), 'kind': 'point',
            'input_state_bits': {name: '0x00000000' for name in o.STATE},
            'parameter_bits': [f'0x{x:08x}' for x in program]}


def report(payload, program_path, library_path):
    program, state = o.validate_case(payload, REGISTRY)
    result = state + [0] * 12
    result[0] = 0x3f800000
    result[51] = result[56] = 1
    return {'schema': 'mb3d.native-formula-probe.v1', 'status': 'COMPLETE',
            'profile': 'native_gpu_fp32', 'library_profile': [2, 4, 32],
            'formula_abi': [1, 8192, 188, 236, 32], 'scene': None,
            'program': str(program_path), 'library': str(library_path),
            'mode': int(payload['kind'] == 'orbit'), 'slot': payload.get('context',{}).get('target_slot',0), 'point_count': 1,
            'repeats': 3, 'wrapper_policy': 1, 'exact_state_repeats': True,
            'input_immutable': True, 'allocation_canaries': True,
            'invalid_descriptor_controls': 7, 'device': 'SYNTHETIC TEST FIXTURE',
            'wrapper_audit_words': [1, 1, 0] * 6, 'program_words': program,
            'fault_count': 0, 'rows': [{'index': 0, 'input_words': state,
             'result_words': result, 'fault': 0, 'work': 1}]}


class SemanticValidation(unittest.TestCase):
    def test_invalid_global_program_scalars_refuse_before_child(self):
        controls=[(7,0),(7,(1<<20)+1),(11,0),(11,(1<<24)+1),
                  (12,0),(12,(1<<20)+1),(16,0),(16,0x80000000),
                  (21,0),(22,0),(17,0x7f800000),(18,0xff800000),
                  (24,0x7fc00000),(28,0x7f800000),(15,0x80000000),
                  (14,1),(23,0x3f800000)]
        with patch.object(o,'invoke') as child:
            for index,value in controls:
                payload=case();payload['parameter_bits'][index]=f'0x{value:08x}'
                with self.subTest(index=index,value=value),self.assertRaises(ValueError):
                    o.validate_case(payload,REGISTRY)
            child.assert_not_called()

    def test_point_and_orbit_accept_full_program(self):
        for kind in ('point', 'orbit'):
            payload = case(); payload['kind'] = kind
            if kind == 'orbit': payload['parameter_bits'][1] = '0x00000001'
            p, s = o.validate_case(payload, REGISTRY)
            self.assertEqual(len(struct.pack('<2048I', *p)), 8192)
            self.assertEqual(len(struct.pack('<47I', *s)), 188)
            raw = report(payload, Path('/program'), Path('/library'))
            actual = o.validate_report(raw, p, s, kind, Path('/program'), Path('/library'))
            self.assertEqual(actual[0], 0x3f800000)

    def test_invalid_semantic_inputs(self):
        mutations = [lambda c: c.update(extra=1), lambda c: c.update(kind='distance'),
                     lambda c: c['identity'].__setitem__(2, 'wrong name'),
                     lambda c: c['identity'].__setitem__(1, True),
                     lambda c: c['input_state_bits'].pop('rout'),
                     lambda c: c['input_state_bits'].__setitem__('rout', 'nan'),
                     lambda c: c['parameter_bits'].pop(),
                     lambda c: c['parameter_bits'].__setitem__(2, '0x00000001'),
                     lambda c: c['parameter_bits'].__setitem__(58, '0x00000000'),
                     lambda c: c['parameter_bits'].__setitem__(397, '0x00000001'),
                     lambda c: c['parameter_bits'].__setitem__(13, '0x00000001'),
                     lambda c: c['parameter_bits'].__setitem__(3, '0x00000001')]
        for mutate in mutations:
            payload = case(); mutate(payload)
            with self.subTest(mutation=mutations.index(mutate)), self.assertRaises(ValueError):
                o.validate_case(payload, REGISTRY)

    def test_duplicate_json_rejected(self):
        with tempfile.TemporaryDirectory() as temp:
            path = Path(temp) / 'case.json'; path.write_text('{"kind":1,"kind":2}')
            with self.assertRaises(ValueError): o.read_json(path)

    def test_raw_report_rejections(self):
        payload = case(); p, s = o.validate_case(payload, REGISTRY)
        mutations = [lambda r: r.update(status='POINT_FAULTS'),
                     lambda r: r.update(library_profile=[1, 8, 64]),
                     lambda r: r.update(formula_abi=[1, 8192, 188, 232, 32]),
                     lambda r: r.update(program='/elsewhere'),
                     lambda r: r.update(mode=1), lambda r: r.update(point_count=2),
                     lambda r: r.update(repeats=2), lambda r: r.update(wrapper_policy=0),
                     lambda r: r.update(allocation_canaries=False),
                     lambda r: r.update(exact_state_repeats=1),
                     lambda r: r.update(input_immutable=False),
                     lambda r: r.update(invalid_descriptor_controls=6),
                     lambda r: r.update(fault_count=1), lambda r: r.update(rows=[]),
                     lambda r: r['rows'][0]['input_words'].__setitem__(0, 1),
                     lambda r: r['rows'][0]['result_words'].pop(),
                     lambda r: r['rows'][0]['result_words'].__setitem__(47, 1),
                     lambda r: r['rows'][0]['result_words'].__setitem__(51, 0),
                     lambda r: r['rows'][0]['result_words'].__setitem__(56, 0),
                     lambda r: r['rows'][0]['result_words'].__setitem__(0, True),
                     lambda r: r['program_words'].__setitem__(58, 0)]
        for mutate in mutations:
            raw = report(payload, Path('/program'), Path('/library')); mutate(raw)
            with self.subTest(mutation=mutations.index(mutate)), self.assertRaises(ValueError):
                o.validate_report(raw, p, s, 'point', Path('/program'), Path('/library'))


class SyntheticChildCapture(unittest.TestCase):
    def run_fixture(self, *, child_exit=0, changes=None, postprocess=None, payload=None):
        root = Path(self.temp.name).resolve()
        inp, library, probe, output = [root / n for n in ('case.json', 'library.bin', 'fake-probe.py', 'observation.json')]
        payload = case() if payload is None else payload
        inp.write_text(json.dumps(payload)); library.write_bytes(b'SYNTHETIC LIBRARY')
        raw = report(payload, root / 'observation.probe/program.bin', library)
        if changes: changes(raw)
        probe.write_text('#!/usr/bin/env python3\nimport json,sys\nfrom pathlib import Path\n'
                         'args=dict(zip(sys.argv[1::2],sys.argv[2::2]))\n'
                         f'Path(args["--output"]).write_text(json.dumps({raw!r}))\n'
                         f'raise SystemExit({child_exit})\n')
        probe.chmod(0o755)
        args = argparse.Namespace(input=str(inp), probe=str(probe), library=str(library),
                                  output=str(output), run_id='a'*32, slot=0, timeout=30,
                                  probe_sha256=o.sha(probe), library_sha256=o.sha(library))
        def capture(*a, **kw):
            event = runner.invoke(*a, **kw)
            if postprocess: postprocess(library)
            return event
        self.args, self.output = args, output
        context_validate=o.context_input.validate; original_root=o.ROOT
        with patch.object(o, 'ROOT', root), patch.object(o, 'current_registry', return_value=REGISTRY), \
             patch.object(o, 'dependency_paths', return_value=[]), patch.object(o, 'invoke', side_effect=capture), \
             patch.object(o.context_input,'validate',side_effect=lambda c,p,s,r:context_validate(c,p,s,original_root)), \
             patch.object(runner.shutil, 'disk_usage', return_value=argparse.Namespace(free=32*1024**3)):
            return o.observe(args)

    def setUp(self): self.temp = tempfile.TemporaryDirectory()
    def tearDown(self): self.temp.cleanup()

    def test_success_captures_real_child_and_raw_report(self):
        got = self.run_fixture()
        root = Path(self.temp.name).resolve()
        self.assertEqual(got['run_id'], 'a'*32)
        self.assertEqual(got['gpu_invocations'], 3)
        self.assertEqual(got['state_bits']['xyz[0]'], '0x3f800000')
        self.assertEqual(got['input_digest'], o.digest(case()))
        self.assertNotIn('cpu_pixel_calls', got)
        event = json.loads((root / got['probe_event']['path']).read_text())
        self.assertEqual(event['exit_code'], 0)
        self.assertEqual(event['executable_sha256_before'], self.args.probe_sha256)
        self.assertEqual(event['owned_process_reaped'], True)
        self.assertEqual(o.sha(root / got['raw_probe']['path']), got['raw_probe']['sha256'])

    def test_nonzero_child_retains_fault_report_and_event(self):
        def fault(raw): raw.update(status='POINT_FAULTS', fault_count=1)
        with self.assertRaises(ValueError): self.run_fixture(child_exit=2, changes=fault)
        root = Path(self.temp.name).resolve() / 'observation.probe/process'
        self.assertEqual(json.loads((root / 'raw-probe.json').read_text())['fault_count'], 1)
        self.assertEqual(json.loads((root / 'event.json').read_text())['exit_code'], 2)
        self.assertFalse(self.output.exists())

    def test_success_exit_with_incomplete_result_rejected(self):
        def incomplete(raw): raw['rows'][0]['result_words'][56] = 0
        with self.assertRaises(ValueError): self.run_fixture(changes=incomplete)
        self.assertFalse(self.output.exists())

    def test_library_changed_after_capture_rejected(self):
        with self.assertRaises(ValueError):
            self.run_fixture(postprocess=lambda p: p.write_bytes(b'CHANGED'))
        self.assertFalse(self.output.exists())


    def replay_fixture(self, mutate=None, payload=None):
        payload=case() if payload is None else payload
        self.run_fixture(payload=payload)
        root = Path(self.temp.name).resolve()
        def ref(path): return {'path': str(path.relative_to(root)), 'sha256': o.sha(path)}
        observed = json.loads(self.output.read_text())
        nested_path = root / observed['probe_event']['path']
        nested = json.loads(nested_path.read_text())
        adapter = Path(o.__file__).resolve()
        # A labelled synthetic outer capture; the nested fake child really ran.
        outer = dict(nested)
        outer.update(run_id='a'*32, pid=nested['pid']+1,
                     started_ns=nested['started_ns']-1000, finished_ns=nested['finished_ns']+1000,
                     executable_sha256_before=o.sha(adapter), executable_sha256_after=o.sha(adapter),
                     effective_timeout_seconds=180,
                     argv=[str(adapter), '--input', self.args.input, '--probe', self.args.probe,
                           '--probe-sha256', self.args.probe_sha256, '--library', self.args.library,
                           '--library-sha256', self.args.library_sha256, '--output', str(self.output),
                           '--run-id', 'a'*32],
                     inputs=[*nested['inputs'], ref(Path(self.args.probe))])
        if mutate: mutate(root, observed, nested)
        nested_path.write_text(json.dumps(nested))
        observed['probe_event'] = ref(nested_path)
        self.output.write_text(json.dumps(observed))
        outer['outputs'] = [ref(self.output)]
        outer_path = root / 'synthetic-outer-event.json'; outer_path.write_text(json.dumps(outer))
        with patch.object(o, 'current_registry', return_value=REGISTRY), \
             patch.object(o, 'dependency_paths', return_value=[]):
            return o.replay_observation(root, ref(self.output), ref(outer_path), payload,
                                        ref(Path(self.args.probe)), ref(Path(self.args.library)))

    def test_replay_checks_nested_actual_capture(self):
        actual = self.replay_fixture()
        self.assertEqual(actual['state_bits']['xyz[0]'], '0x3f800000')

    def test_replay_rejects_tampered_raw_probe(self):
        def mutate(root, observed, nested):
            path = root / observed['raw_probe']['path']
            raw = json.loads(path.read_text()); raw['rows'][0]['result_words'][0] = 0
            path.write_text(json.dumps(raw))
        with self.assertRaises(ValueError): self.replay_fixture(mutate)

    def test_replay_rejects_wrong_nested_executable_identity(self):
        def mutate(root, observed, nested): nested['executable_sha256_before'] = '0'*64
        with self.assertRaises(ValueError): self.replay_fixture(mutate)

    def test_replay_rejects_rehashed_wrapper_state_lie(self):
        def mutate(root, observed, nested): observed['state_bits']['xyz[0]'] = '0x00000000'
        with self.assertRaises(ValueError): self.replay_fixture(mutate)

    def test_replay_rejects_wrong_dependency_closure(self):
        def mutate(root, observed, nested):
            observed['dependencies'] = [{'path': 'fake-probe.py', 'sha256': self.args.probe_sha256}]
        with self.assertRaises(ValueError): self.replay_fixture(mutate)

    def test_mixed_nested_slot_and_context_replay(self):
        from test_native_formula_context_input import mixed_case
        payload=mixed_case(); actual=self.replay_fixture(payload=payload)
        self.assertEqual(actual['context'],payload['context'])
        root=Path(self.temp.name).resolve()
        nested=json.loads((root/actual['probe_event']['path']).read_text())
        self.assertEqual(nested['argv'][nested['argv'].index('--slot')+1],'1')

    def test_mixed_context_rehashed_slot_lie_rejected(self):
        from test_native_formula_context_input import mixed_case
        def mutate(root,observed,nested): observed['context']['target_slot']=0
        with self.assertRaisesRegex(ValueError,'context differs'):
            self.replay_fixture(mutate,payload=mixed_case())

    def test_mixed_nested_slot_zero_substitution_rejected(self):
        from test_native_formula_context_input import mixed_case
        def mutate(root,observed,nested): nested['argv'][nested['argv'].index('--slot')+1]='0'
        with self.assertRaisesRegex(ValueError,'dispatch arguments'):
            self.replay_fixture(mutate,payload=mixed_case())


class ExplicitProcessId(unittest.TestCase):
    def test_explicit_and_generated_run_id(self):
        with tempfile.TemporaryDirectory() as temp, \
             patch.object(runner.shutil, 'disk_usage', return_value=argparse.Namespace(free=32*1024**3)):
            root = Path(temp)
            first = runner.invoke(['/usr/bin/true'], root/'first', root, run_id='b'*32)
            second = runner.invoke(['/usr/bin/true'], root/'second', root)
            self.assertEqual(first['run_id'], 'b'*32)
            self.assertRegex(second['run_id'], r'^[a-f0-9]{32}$')
            self.assertNotEqual(first['run_id'], second['run_id'])

    def test_invalid_explicit_id_does_not_launch(self):
        with tempfile.TemporaryDirectory() as temp:
            root = Path(temp)
            for value in ('', '../bad', 'A'*32, 123):
                with self.subTest(value=value), self.assertRaises(ValueError):
                    runner.invoke(['/usr/bin/true'], root/'bad', root, run_id=value)
                self.assertFalse((root/'bad').exists())

    def test_invalid_termination_grace_does_not_launch(self):
        with tempfile.TemporaryDirectory() as temp:
            root = Path(temp)
            for value in (0, -1, 6, float('nan'), True):
                with self.subTest(value=value), self.assertRaises(ValueError):
                    runner.invoke(['/usr/bin/true'], root/'bad', root,
                                  termination_grace_seconds=value)
                self.assertFalse((root/'bad').exists())

    def test_cancelled_observer_reaps_sigterm_resistant_child(self):
        # This executes a Python fake probe only. Its signal resistance tests
        # ownership cleanup; it has no Metal code and creates no GPU evidence.
        with tempfile.TemporaryDirectory() as temp, \
             patch.object(runner.shutil, 'disk_usage', return_value=argparse.Namespace(free=32*1024**3)):
            root = Path(temp).resolve()
            inp, library, probe, bootstrap = [root / n for n in
                                            ('case.json', 'library.bin', 'fake-long-probe.py', 'wrapper.py')]
            inp.write_text(json.dumps(case())); library.write_bytes(b'SYNTHETIC')
            probe.write_text('#!/usr/bin/env python3\nimport os,signal,sys,time\nfrom pathlib import Path\n'
                             'signal.signal(signal.SIGTERM, signal.SIG_IGN)\n'
                             'args=dict(zip(sys.argv[1::2],sys.argv[2::2]))\n'
                             'Path(args["--states"]).with_name("child.pid").write_text(str(os.getpid()))\n'
                             'time.sleep(60)\n')
            probe.chmod(0o755)
            output = root/'outer/observation.json'
            argv = ['observer', '--input', str(inp), '--probe', str(probe),
                    '--probe-sha256', o.sha(probe), '--library', str(library),
                    '--library-sha256', o.sha(library), '--output', str(output),
                    '--run-id', 'd'*32, '--timeout', '30']
            bootstrap.write_text('import sys\nfrom pathlib import Path\n'
                                 f'sys.path.insert(0,{str(Path(o.__file__).parent)!r})\n'
                                 'import native_formula_observer as o\n'
                                 f'o.ROOT=Path({str(root)!r})\n'
                                 f'o.current_registry=lambda root: {REGISTRY!r}\n'
                                 'o.dependency_paths=lambda case=None: []\n'
                                 f'sys.argv={argv!r}\n'
                                 'raise SystemExit(o.main())\n')
            child_pid = None
            try:
                event = runner.invoke([sys.executable, bootstrap], root/'outer', root,
                                      timeout=1.5, evidence_root=root)
                pid_file = root/'outer/observation.probe/child.pid'
                self.assertTrue(pid_file.is_file(), 'Fake child never reached cancellation test')
                child_pid = int(pid_file.read_text())
                self.assertEqual(event['guard'], 'TIMEOUT')
                self.assertEqual(event['exit_code'], 1)
                self.assertLess(event['seconds'], 5)
                with self.assertRaises(ProcessLookupError): os.kill(child_pid, 0)
                self.assertFalse(output.exists())
            finally:
                # Prevent a failed regression test itself from leaking a child.
                if child_pid is None:
                    pid_file = root/'outer/observation.probe/child.pid'
                    if pid_file.is_file(): child_pid = int(pid_file.read_text())
                if child_pid is not None:
                    try: os.killpg(child_pid, signal.SIGKILL)
                    except ProcessLookupError: pass

    def test_exclusive_lock_forwards_explicit_id(self):
        with tempfile.TemporaryDirectory() as temp, patch.object(runner, '_invoke_unlocked') as capture:
            root = Path(temp)
            runner.invoke(['/usr/bin/true'], root/'unused', root, evidence_root=root,
                          exclusive_gpu=True, run_id='c'*32)
            self.assertEqual(capture.call_args.kwargs['run_id'], 'c'*32)


class DescriptorValidation(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.registry = o.current_registry(o.ROOT)
        cls.rows = {r['id']: r for r in json.loads((o.ROOT/'src/render/native_metal/generated/inventory.json').read_text())['formulas']}

    def external(self, ident=429, driver=1, option=2, kind='orbit'):
        # Production metadata is test input, never a numerical oracle.
        row = self.rows[ident]
        p = case(); p['identity'] = ['external', ident, row['name']]; p['kind'] = kind
        count = 2 if 95001 <= ident <= 95003 else len(row['parameter_types'])
        words = [driver, option & 0xffffffff]
        p['parameter_bits'][1], p['parameter_bits'][6] = (f'0x{w:08x}' for w in words)
        slot = [1, 1, ident, row['de_option'] & 0xffffffff, count, row['traits'], row['adds_c'], row['difs_style'], 0, 1, 0, 0]
        p['parameter_bits'][56:68] = [f'0x{w:08x}' for w in slot]
        return p

    def test_wrong_enabled_descriptor_fields_reject(self):
        for index, value in [(59,11), (60,0), (61,8), (62,1), (63,1), (64,2), (66,1), (67,1)]:
            p = self.external(); p['parameter_bits'][index] = f'0x{value:08x}'
            with self.subTest(word=index), self.assertRaises(ValueError):
                o.validate_case(p, self.registry)

    def test_incompatible_orbit_driver_options_reject(self):
        for driver, option in [(1,5), (3,2), (2,11), (8,0), (0,2), (1,-2)]:
            with self.subTest(driver=driver, option=option), self.assertRaises(ValueError):
                o.validate_case(self.external(driver=driver, option=option), self.registry)

    def test_point_profile_and_distinct_valid_analytic_global_are_preserved(self):
        # Global11 alone is not malformed for analytic3D: model/context
        # validation, rather than the general transport guard, owns that scope.
        for p in [self.external(driver=0, option=2, kind='point'), self.external(option=11)]:
            o.validate_case(p, self.registry)

    def test_modifier_analytic_numeric_four_d_and_difs_profiles(self):
        profiles = [(8065,1,2), (8065,3,5), (1509,1,11), (1509,0,0), (429,1,2)]
        for de, driver, option in [(4,2,4), (5,3,5), (6,3,6), (20,8,20), (21,8,20)]:
            row = next(r for r in self.rows.values() if r['de_option'] == de)
            profiles.append((row['id'], driver, option))
        for ident, driver, option in profiles:
            with self.subTest(identity=ident): o.validate_case(self.external(ident, driver, option), self.registry)

    def test_uploaded_counts_preserve_benesi_and_native_exceptions(self):
        self.assertEqual(self.rows[7083]['param_count'], 3)
        self.assertEqual(self.rows[7083]['parameter_types'], [])
        p = self.external(7083, 0, 0)
        self.assertEqual(p['parameter_bits'][60], '0x00000000')
        o.validate_case(p, self.registry)
        p['parameter_bits'][60] = '0x00000003'
        with self.assertRaises(ValueError): o.validate_case(p, self.registry)
        for ident in (95001,95002,95003):
            p = self.external(ident)
            self.assertEqual(p['parameter_bits'][60], '0x00000002')
            o.validate_case(p, self.registry)

    def test_internal_metadata_power_and_box_callback_options(self):
        options = (0,0,4,0,11,0,0,-1,20,0)
        names = {r[1]:r[2] for r in self.registry if r[0] == 'internal'}
        for ident, option in enumerate(options):
            for flag in ((0,2) if ident == 4 else (0,)):
                p = case(); p['identity'] = ['internal',ident,names[ident]]
                slot = [1,0,ident,option & 0xffffffff,0,0,flag,0,2 if ident in (0,6) else 0,1,0,0]
                p['parameter_bits'][56:68] = [f'0x{w:08x}' for w in slot]
                o.validate_case(p,self.registry)
                for index, wrong in [(60,1),(61,1),(63,1),(64,9),(62,4)]:
                    q = dict(p, parameter_bits=p['parameter_bits'].copy()); q['parameter_bits'][index]=f'0x{wrong:08x}'
                    with self.subTest(identity=ident, word=index), self.assertRaises(ValueError): o.validate_case(q,self.registry)

    def test_unused_descriptor_metadata_is_not_accepted_as_production_input(self):
        p = self.external(); p['parameter_bits'][56+332+2]='0x000001ad'
        with self.assertRaises(ValueError): o.validate_case(p,self.registry)

    def test_map_resources_and_raw_alias_bits_are_preserved(self):
        from native_group03_map_reference import resources
        p = self.external(10006, 0, 0, 'point'); p['resources'] = resources()
        # Descriptor validation does not reinterpret raw/typed parameter words
        # as floats. Per-formula preparation/range proofs own these values.
        for at, value in [(68,0x7f800000), (69,0xff800000), (196,0xffffffff), (324,0xffffffff)]:
            p['parameter_bits'][at] = f'0x{value:08x}'
        program, _ = o.validate_case(p,self.registry)
        self.assertEqual([program[i] for i in (68,69,196,324)],
                         [0x7f800000,0xff800000,0xffffffff,0xffffffff])

    def test_internal_callback_pointer_is_not_uploadable(self):
        name = next(r[2] for r in self.registry if r[:2] == ('internal',6))
        p = case(); p['identity'] = ['internal',6,name]
        p['parameter_bits'][56:68] = [f'0x{w:08x}' for w in [1,0,6,0,0,0,0,0,2,1,0,0]]
        for at in (310,311,330):
            q = dict(p,parameter_bits=p['parameter_bits'].copy()); q['parameter_bits'][at] = '0x00000001'
            with self.subTest(word=at), self.assertRaises(ValueError): o.validate_case(q,self.registry)

    def test_bad_metadata_or_changed_dependency_stops_before_child(self):
        with tempfile.TemporaryDirectory() as temp:
            root = Path(temp).resolve()
            inp, probe, lib, dep = [root/n for n in ('input.json','probe','lib','metadata')]
            probe.write_bytes(b'fake probe'); lib.write_bytes(b'fake library'); dep.write_bytes(b'metadata')
            args = argparse.Namespace(input=str(inp),probe=str(probe),library=str(lib),output=str(root/'out.json'),
                                      probe_sha256=o.sha(probe),library_sha256=o.sha(lib),run_id='a'*32,timeout=30,slot=0)
            with patch.object(o,'ROOT',root), patch.object(o,'current_registry',return_value=self.registry), \
                 patch.object(o,'dependency_paths',return_value=[dep]), patch.object(o,'invoke') as child:
                p = self.external(); p['parameter_bits'][59] = '0x0000000b'; inp.write_text(json.dumps(p))
                with self.assertRaisesRegex(ValueError,'production metadata'): o.observe(args)
                child.assert_not_called()
                inp.write_text(json.dumps(self.external()))
                original = o.validate_case
                def change_during_validation(payload):
                    result = original(payload); dep.write_bytes(b'changed metadata'); return result
                with patch.object(o,'validate_case',side_effect=change_during_validation), \
                     self.assertRaisesRegex(ValueError,'changed during validation'): o.observe(args)
                child.assert_not_called()
                self.assertFalse((root/'out.probe').exists())

    def test_metadata_sources_are_in_observation_dependency_closure(self):
        required = {'src/render/native_metal/generated/inventory.json',
                    'src/render/native_metal/generated/formula_metadata.h',
                    'src/core/formula_registry.cpp', 'src/render/native_metal/formula_host.cpp',
                    'src/core/header_trafos.cpp', 'src/core/external_formula_table.cpp',
                    'src/render/native_metal/formula_abi.h'}
        paths = {str(p.relative_to(o.ROOT)) for p in o.dependency_paths()}
        self.assertTrue(required <= paths, required-paths)


if __name__ == '__main__': unittest.main()
