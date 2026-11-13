"""Preparation and transport fixtures only; no original execution or numerical credit."""
import json
from pathlib import Path
import struct
import tempfile
import unittest
from unittest.mock import patch

import build_original_translate_callers_oracle as b


def normalized_state(row, final):
    raw = bytearray(b.state_bytes(row))
    for slot in range(row['active_slots']):
        for offset in (156,180):
            struct.pack_into('<I',raw,offset+slot*4,slot+1)
    struct.pack_into('<I',raw,104,1 if final else 0)
    return bytes(raw)


def synthetic(rows, traced=False):
    data = bytearray(struct.pack('<8s4I',b'MBTCAL01',1,len(rows),456,512))
    for index,row in enumerate(rows):
        before,after = normalized_state(row,False),normalized_state(row,True)
        data += struct.pack('<2I',index,tuple(b.PROFILES).index(row['profile']))
        data += before+after+bytes(40)+struct.pack('<I',int(traced))
        if traced:
            data += struct.pack('<I',0)+after+bytes(52)
    return bytes(data)


class OriginalExtraction(unittest.TestCase):
    def test_every_body_and_type_block_is_unchanged_original(self):
        sections=b.original_sections()
        generated=b.source(b.controls())
        for name,section in sections.items():
            with self.subTest(name=name):
                self.assertEqual(generated.count(section),1)
        for filename,names in [('formulas.pas',['CalcSmoothIterations',*dict.fromkeys(x[0] for x in b.PROFILES.values())]),
                               ('Math3D.pas',['Rotate4Dex','CopyVec4SSE2','mCopyVec'])]:
            original=(b.ROOT/'mb3d'/filename).read_text()
            for name in names:
                self.assertIn(b.extract(original,name),generated)
        self.assertNotIn('Halt(90)',generated)
        self.assertIn('fyl2x',sections['CalcSmoothIterations'])
        self.assertIn('mCopyVec4:=CopyVec4SSE2;',generated)

    def test_original_code_and_typed_parameter_layout(self):
        rows=b.controls(['numeric3d'])
        payloads=b.input_payloads(rows)
        original=(b.ROOT/'mb3d/M3Formulas/_Translate.m3f').read_text()
        self.assertEqual(payloads['original.code'],bytes.fromhex(original.split('[CODE]')[1].split('[END]')[0]))
        self.assertEqual(len(payloads['original.code']),36)
        self.assertEqual(len(payloads['controls.bin']),2512*len(rows))
        params=b.parameter_bytes([1.25,-.5,-0.])
        self.assertEqual(struct.unpack_from('<3d',params,480),(-0.,-.5,1.25))
        self.assertEqual(params[480:488],struct.pack('<d',-0.))
        self.assertEqual(struct.unpack_from('<d',params,504)[0],.5)
        self.assertEqual(params[:480]+params[512:],bytes(992))

    def test_changed_source_is_rejected(self):
        with tempfile.TemporaryDirectory() as folder:
            root=Path(folder)
            path=root/next(iter(b.SOURCE_PINS));path.parent.mkdir(parents=True)
            path.write_text('changed')
            with self.assertRaisesRegex(ValueError,'immutable source changed'):
                b.checked_sources(root)

    def test_trace_is_integer_full_state_tail_copy_with_explicit_width(self):
        for slot in range(2):
            text=b.capture_source(slot)
            self.assertIn('pushfd\n  pushad',text)
            self.assertIn(f'popad\n  popfd\n  jmp dword ptr [CodePtrs+{slot*4}]',text)
            self.assertIn('jae @overflow',text)
            self.assertIn('jmp TraceAbort',text)
            stores=[line.strip() for line in text.splitlines() if ', eax' in line]
            self.assertEqual(len(stores),114)
            self.assertTrue(all(line.startswith('mov dword ptr [Trace+') for line in stores))
            for offset in range(0,456,4):
                self.assertIn(f'mov eax, dword ptr [It+{offset}]',text)
                self.assertIn(f'mov dword ptr [Trace+ebx+{offset+4}], eax',text)
            self.assertIn('movupd [Trace+ebx+460], xmm0',text)
            self.assertIn('movupd [Trace+ebx+476], xmm1',text)
            self.assertNotIn('call ',text)
            self.assertNotIn('fld',text)
            self.assertNotIn('movupd xmm',text)

    def test_return_capture_immediate_and_numeric_has_no_x87_pop(self):
        text=b.source(b.controls())
        for index,(name,_,_,analytic,_) in enumerate(b.PROFILES.values()):
            block=text.split(f'procedure Call{index}(')[1].split('\nend;')[0]
            self.assertIn('pxor xmm0, xmm0\n  pxor xmm1, xmm1\n  call '+name,block)
            self.assertIn(f'call {name}\n  movupd [OutLanes], xmm0\n  movupd [OutLanes+16], xmm1',block)
            self.assertEqual('fstp qword ptr [Distance]' in block,analytic)
        self.assertIn('Snapshot:=It; Normalize(Snapshot);',text)
        self.assertNotIn('FloatToStr',text)


class BoundedInputs(unittest.TestCase):
    def test_case_matrix_has_strict_bailout_and_all_smoothing_paths(self):
        rows=b.controls()
        self.assertEqual(len(rows),117)
        self.assertEqual(len({r['id'] for r in rows}),117)
        for profile in b.PROFILES:
            selected=[r for r in rows if r['profile']==profile]
            self.assertTrue(any(r['id'].endswith('bailout-before-max') for r in selected))
            self.assertTrue(any(r['id'].endswith('bailout-equality') for r in selected))
            for suffix in ('smooth-low','smooth-old-below-one','smooth-old-above-one','smooth-old-equal-one'):
                self.assertTrue(any(r['id'].endswith(suffix) for r in selected))
            for row in selected:
                self.assertTrue(1<=row['max_it']<=4)
                self.assertGreater(row['rstop'],0)

    def test_4d_matrix_reaches_nonzero_w_and_negative_prefix_count_is_encoded(self):
        rows=b.controls()
        for row in rows:
            raw=b.state_bytes(row)
            self.assertEqual(len(raw),456)
            self.assertTrue(all(raw[o:o+4]==bytes(4) for o in b.POINTER_OFFSETS))
            if row['matrix']=='swap-x-w':
                matrix=struct.unpack_from('<16f',raw,312)
                self.assertEqual(sum(a*c for a,c in zip(matrix[12:15],row['point'])),-3.)
            if row['negative_prefix']:
                self.assertEqual(struct.unpack_from('<I',raw,132)[0],0x80000001)
                self.assertEqual(row['active_slots'],2)
            if row['interpolation']:
                self.assertEqual(sum(struct.unpack_from('<2f',raw,132)),1.)

    def test_unsupported_abi_and_combined_are_not_runnable(self):
        for profile in ('difs','difs-noVecIni','combined'):
            with self.assertRaisesRegex(ValueError,'remain OPEN'):
                b.controls([profile])
        for profiles in ([],['numeric3d','numeric3d']):
            with self.assertRaises(ValueError): b.controls(profiles)
        text=b.source(b.controls())
        self.assertNotIn('call doHybridIFS',text)
        self.assertNotIn('CalcDEfull',text)


class Preparation(unittest.TestCase):
    def test_prepare_does_not_compile_or_run_and_freezes_all_files(self):
        with tempfile.TemporaryDirectory() as folder,patch.object(b.subprocess,'Popen',side_effect=AssertionError('no subprocess')):
            output=Path(folder)/'new'
            manifest=b.prepare(output,['numeric4d'])
            self.assertEqual(manifest['status'],'PREPARED_NOT_COMPILED')
            self.assertFalse(manifest['qualification'])
            for name,expected in manifest['files'].items():
                self.assertEqual(b.sha((output/name).read_bytes()),expected)
            plan=json.loads((output/'process-plan.json').read_text())
            self.assertEqual(len(plan),6)
            self.assertEqual(len({r['argv'][-1] for r in plan}),6)
            self.assertTrue(all(r['fresh_process'] and r['timeout_seconds']==30 for r in plan))
            self.assertEqual([r['argv'][2] for r in plan],['plain']*3+['trace']*3)
            with self.assertRaisesRegex(ValueError,'fresh output'):
                b.prepare(output)

    def test_compile_refuses_local_checkout_without_launching(self):
        with tempfile.TemporaryDirectory() as folder,patch.object(b.subprocess,'Popen',side_effect=AssertionError('no subprocess')):
            with self.assertRaisesRegex(ValueError,'external Windows'):
                b.compile_prepared(Path(folder)/'out',Path(folder)/'work',Path(folder)/'fake-fpc')

    def test_compile_rejects_wrong_args_before_work_creation_or_process(self):
        with tempfile.TemporaryDirectory() as folder,patch.object(b,'validate_external_work'),patch.object(b.subprocess,'Popen',side_effect=AssertionError('must not launch')):
            base=Path(folder);output=base/'prepared';work=base/'work';fpc=base/'fake-fpc'
            fpc.write_bytes(b'synthetic compiler fixture only')
            with patch.object(b,'COMPILER_SHA',b.sha(fpc.read_bytes())):
                b.prepare(output,['numeric4d'])
                manifest=json.loads((output/'preparation.json').read_text())
                for args in (['-Mdelphi','-CfSSE2','-O1','-al','-va'],list(b.COMPILER_ARGS)+['-OoFASTMATH'],list(reversed(b.COMPILER_ARGS)),None):
                    manifest['compiler']['args']=args
                    (output/'preparation.json').write_text(json.dumps(manifest))
                    with self.subTest(args=args),self.assertRaisesRegex(ValueError,'exact reviewed compiler arguments'):
                        b.compile_prepared(output,work,fpc)
                    self.assertFalse(work.exists())
                    self.assertFalse((output/'compile').exists())

    def test_compile_rejects_changed_preparation_and_keeps_before_after_pins(self):
        with tempfile.TemporaryDirectory() as folder,patch.object(b,'validate_external_work'):
            base=Path(folder);output=base/'prepared';work=base/'work';fpc=base/'fake-fpc'
            fpc.write_bytes(b'synthetic compiler fixture only')
            class FakeProcess:
                pid=12345
                def wait(self,timeout):
                    self_outer.assertEqual(timeout,90)
                    path=output/'preparation.json'
                    path.write_bytes(path.read_bytes()+b'\n')
                    return 0
            self_outer=self
            with patch.object(b,'COMPILER_SHA',b.sha(fpc.read_bytes())):
                b.prepare(output,['numeric4d'])
                before=b.sha((output/'preparation.json').read_bytes())
                with patch.object(b.subprocess,'Popen',return_value=FakeProcess()):
                    with self.assertRaisesRegex(ValueError,'compile failed'):
                        b.compile_prepared(output,work,fpc)
                receipt=json.loads((output/'compile/identity.json').read_text())
                self.assertEqual(receipt['preparation_sha256'],before)
                self.assertNotEqual(receipt['preparation_sha256_after'],before)
                self.assertIn('preparation manifest changed during compilation',receipt['capture_errors'])
                self.assertEqual(receipt['status'],'COMPILE_IDENTITY_REJECTED')


class SyntheticTransport(unittest.TestCase):
    def setUp(self):
        self.rows=b.controls(['numeric4d'])[:2]

    def test_full_before_after_and_callback_records_roundtrip(self):
        direct,trace=synthetic(self.rows),synthetic(self.rows,True)
        parsed=b.parse_output(trace,self.rows)
        self.assertEqual(len(parsed),2)
        for row in parsed:
            self.assertEqual(len(bytes.fromhex(row['before_hex'])),456)
            self.assertEqual(len(bytes.fromhex(row['state_hex'])),456)
            self.assertEqual(len(bytes.fromhex(row['trace'][0]['state_hex'])),456)
        result=b.compare_outputs([direct]*3,[trace]*3,self.rows)
        self.assertFalse(result['qualification'])
        self.assertEqual(result['status'],'REPEAT_AND_TRACE_NEUTRALITY_PASS')

    def test_malformed_header_order_count_pointer_padding_trailing_rejected(self):
        raw=synthetic(self.rows,True)
        corruptions=[raw[:10],raw[:-1],raw+b'x']
        for offset,value in ((8,2),(24,4),(24+960,65),(24+464+104,999),
                             (24+964,7),(24+964+492,1)):
            changed=bytearray(raw);struct.pack_into('<I',changed,offset,value);corruptions.append(bytes(changed))
        for changed in corruptions:
            with self.subTest(size=len(changed)),self.assertRaises(ValueError):
                b.parse_output(changed,self.rows)

    def test_any_state_or_return_difference_and_missing_repeats_rejected(self):
        direct,trace=synthetic(self.rows),synthetic(self.rows,True)
        for offset in (24+8,24+464,24+920,24+952):
            changed=bytearray(trace);changed[offset]^=1
            with self.assertRaisesRegex(ValueError,'trace changed|captured input differs'):
                b.compare_outputs([direct]*3,[bytes(changed)]*3,self.rows)
        with self.assertRaisesRegex(ValueError,'three fresh'):
            b.compare_outputs([direct]*2,[trace]*3,self.rows)
        with self.assertRaisesRegex(ValueError,'exactly repeatable'):
            b.compare_outputs([direct,direct,direct+b'x'],[trace]*3,self.rows)
        with self.assertRaisesRegex(ValueError,'missing trace'):
            b.compare_outputs([direct]*3,[direct]*3,self.rows)

    def test_identically_corrupted_before_records_cannot_pass_neutrality(self):
        direct,trace=bytearray(synthetic(self.rows)),bytearray(synthetic(self.rows,True))
        for raw in (direct,trace): raw[24+8+56]^=1
        with self.assertRaisesRegex(ValueError,'captured input differs'):
            b.compare_outputs([bytes(direct)]*3,[bytes(trace)]*3,self.rows)

    def test_trace_pvar_must_match_active_slot(self):
        rows=[r for r in b.controls(['numeric4d']) if r['negative_prefix']]
        raw=bytearray(synthetic(rows,True))
        struct.pack_into('<I',raw,24+964+4+104,2)
        with self.assertRaisesRegex(ValueError,'slot and PVar disagree'):
            b.parse_output(raw,rows)


if __name__=='__main__':
    unittest.main()
