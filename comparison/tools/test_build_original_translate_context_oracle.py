"""Source/transport controls only: no compiler, VM, GPU or numerical credit."""
import hashlib
from pathlib import Path
import struct
import tempfile
import unittest

import build_original_translate_context_oracle as b


def synthetic_output(traced=False, calls=1):
    """Artificial protocol fixture; deliberately not formula observations."""
    raw = bytearray(struct.pack('<8s4I', b'MBTCTX01', 1, len(b.RUNS), 456, 128))
    for run, (point, _) in enumerate(b.RUNS):
        state = bytearray(456)
        for slot in range(3):
            struct.pack_into('<I', state, 156 + 4 * slot, slot + 1)
            struct.pack_into('<I', state, 180 + 4 * slot, slot + 1)
        slot, remaining, iteration = 0, 30, 0
        trace = bytearray()
        for _ in range(calls):
            if not remaining:
                slot = (slot + 1) % 3
                remaining = (30, 1, 5)[slot]
            trace += struct.pack('<4I14Q', slot, remaining, iteration, 0, *([0] * 14))
            remaining -= 1
            iteration += slot != 1
        struct.pack_into('<I', state, 104, slot + 1)
        struct.pack_into('<I', state, 120, iteration)
        struct.pack_into('<I', state, 268, remaining)
        raw += struct.pack('<2I', run, point) + state + bytes(40)
        raw += struct.pack('<I', calls if traced else 0)
        if traced:
            raw += trace
    return bytes(raw)


class SourceContract(unittest.TestCase):
    def test_original_caller_copy_types_are_byte_exact(self):
        generated = b.source()
        for filename, needle in (('formulas.pas', 'function doHybridPasDE('),
                                 ('Math3D.pas', 'procedure mCopyVec(')):
            original = (b.ROOT / 'mb3d' / filename).read_text()
            start = original.index(needle, original.index('implementation'))
            body = original[start:original.index('\nend;', start) + 5]
            self.assertEqual(generated.count(body), 1)
        original = (b.ROOT / 'mb3d/TypeDefinitions.pas').read_text()
        self.assertIn(original[original.index('  ThybridIteration2 ='):original.index('  ThybridIteration =')], generated)
        self.assertNotIn('procedure doHybridPas(', generated)

    def test_three_tail_shims_preserve_machine_state_and_fail_on_overflow(self):
        for slot in range(3):
            source = b.capture_source(slot)
            self.assertIn('pushfd\n  pushad', source)
            self.assertIn(f'popad\n  popfd\n  jmp dword ptr [CodePtrs+{slot * 4}]', source)
            self.assertIn('cmp ebx, 1024\n  jae @overflow', source)
            self.assertIn('@overflow:\n  popad\n  popfd\n  jmp TraceAbort', source)
            self.assertIn('movupd [Trace+ebx+80], xmm0', source)
            self.assertIn('movupd [Trace+ebx+96], xmm1', source)
            self.assertNotIn('call ', source)
            self.assertNotIn('@skip', source)
            # There must be no x87 operation or write to an SSE register.
            instructions = [line.strip().split(' ', 1)[0] for line in source.split('asm\n')[1].splitlines() if line.strip()]
            self.assertTrue(set(instructions) <= {'pushfd', 'pushad', 'mov', 'cmp', 'jae', 'shl',
                                                  'movupd', 'inc', 'popad', 'popfd', 'jmp', '@overflow:', 'end;'})

    def test_byte_array_trace_destination_has_explicit_dword_width(self):
        # FPC otherwise infers Byte from the destination array and emits movb
        # even when EAX is written in the inline assembler source.
        for slot in range(3):
            source = b.capture_source(slot)
            stores = [line.strip() for line in source.splitlines()
                      if 'Trace+' in line and ', eax' in line]
            self.assertEqual(len(stores), 23)
            self.assertTrue(all(line.startswith('mov dword ptr [Trace+') for line in stores))
            self.assertNotIn('mov [Trace+', source)

    def test_profile_keeps_original_negative_count_and_actual_carry(self):
        generated = b.source()
        self.assertIn('It.maxIt:=400; It.RStop:=1048576', generated)
        self.assertIn('It.nHybrid[0]:=30; It.nHybrid[1]:=LongInt($80000001); It.nHybrid[2]:=5', generated)
        self.assertIn('It.iStartFrom:=0; It.iRepeatFrom:=0; It.EndTo:=2', generated)
        self.assertIn('if (RunIndex<>9) and (RunIndex<>11) then begin\n      FillChar(It', generated)
        self.assertIn('begin Halt(90); end;', generated)
        self.assertIn('begin Halt(91); end;', generated)
        self.assertNotIn('CalcSIT:=True', generated)
        self.assertEqual(b.RUNS[8:], ((3, False), (4, True), (4, False), (3, True)))

    def test_return_capture_immediately_precedes_all_serialization(self):
        generated = b.source()
        self.assertIn('call doHybridPasDE\n  movupd [OutLanes], xmm0\n  movupd [OutLanes+16], xmm1\n  fstp qword ptr [Distance]', generated)
        self.assertIn('CallAndCapture(TPIteration3D(@It.C1));\n    Snapshot:=It;', generated)
        self.assertIn('if FileExists(ParamStr(3)) then Halt(8)', generated)
        self.assertIn('(SizeOf(It)<>456)', generated)
        self.assertNotIn('FloatToStr', generated)

    def test_manifest_never_claims_original_scene_or_rn32_parity(self):
        manifest = b.manifest()
        self.assertIs(manifest['qualification'], False)
        self.assertEqual(manifest['status'], 'BUILT_NOT_EXECUTED')
        self.assertIn('not RN32 or Metal arithmetic', manifest['limitations'])
        self.assertIn('project', manifest['projection'])


class PreparedInputContract(unittest.TestCase):
    def test_original_code_and_negative_parameter_bytes_preserved_exactly(self):
        payloads = b.input_payloads()
        program = (b.ROOT / b.PROGRAM).read_bytes()
        for slot, (name, sha) in enumerate(b.ASSETS):
            source = (b.ROOT / 'mb3d/M3Formulas' / (name + '.m3f')).read_bytes()
            self.assertEqual(hashlib.sha256(source).hexdigest(), sha)
            code = bytes.fromhex(source.decode('latin1').split('[CODE]')[1].split('[END]')[0])
            self.assertEqual(payloads[f'code{slot}.bin'], code)
            params = payloads[f'params{slot}.bin']
            offset = (56 + 332 * slot + 204) * 4
            self.assertEqual(params[256:512], program[offset:offset + 256])
            self.assertEqual(params[:256] + params[512:], bytes(768))
        # Actual authored Double min radius survives: binary32 params underflow.
        surf = payloads['params0.bin']
        self.assertGreater(struct.unpack_from('<d', surf, 512 - 32)[0], 0)
        self.assertGreater(struct.unpack_from('<d', surf, 512 - 24)[0], 1e70)
        self.assertEqual(struct.unpack_from('<d', payloads['params1.bin'], 512 - 16)[0], -3.5)

    def test_julia_is_explicit_binary32_projection_widened_exactly(self):
        payloads = b.input_payloads()
        program = (b.ROOT / b.PROGRAM).read_bytes()
        self.assertEqual(struct.unpack('<4d', payloads['julia.bin']), struct.unpack('<4f', program[96:112]))

    def test_changed_program_or_original_code_rejected_before_output(self):
        with tempfile.TemporaryDirectory() as folder:
            root = Path(folder)
            program = root / b.PROGRAM
            program.parent.mkdir(parents=True)
            original = (b.ROOT / b.PROGRAM).read_bytes()
            program.write_bytes(b'x' + original[1:])
            with self.assertRaisesRegex(ValueError, 'immutable diagnostic input changed'):
                b.input_payloads(root)
            program.write_bytes(original)
            for name, _ in b.ASSETS:
                asset = root / 'mb3d/M3Formulas' / (name + '.m3f')
                asset.parent.mkdir(parents=True, exist_ok=True)
                asset.write_bytes((b.ROOT / 'mb3d/M3Formulas' / asset.name).read_bytes())
            asset.write_bytes(asset.read_bytes() + b'\n')
            with self.assertRaisesRegex(ValueError, 'immutable diagnostic input changed'):
                b.input_payloads(root)


class SyntheticBinaryTransport(unittest.TestCase):
    def test_complete_roundtrip_and_plain_trace_comparison_no_numerical_credit(self):
        plain = synthetic_output(calls=36)
        traced = synthetic_output(True, calls=36)
        rows = b.parse_output(traced)
        self.assertEqual(len(rows), 12)
        self.assertEqual(len(bytes.fromhex(rows[0]['state_hex'])), 456)
        self.assertEqual(rows[0]['trace'][30]['slot'], 1)
        self.assertEqual(rows[0]['trace'][31]['iteration'], 30)
        report = b.compare_outputs([plain] * 3, [traced] * 3)
        self.assertFalse(report['qualification'])
        self.assertEqual(report['runs'][0]['visits'], [30, 1, 5])
        self.assertEqual(report['runs'][0]['iteration'], 35)

    def test_truncation_trailing_bytes_and_unknown_version_reject(self):
        raw = synthetic_output(True)
        for bad in (raw[:20], raw[:-1], raw + b'x', raw[:8] + struct.pack('<I', 2) + raw[12:]):
            with self.subTest(size=len(bad)), self.assertRaises(ValueError):
                b.parse_output(bad)

    def test_trace_overflow_rejects_instead_of_truncating(self):
        raw = bytearray(synthetic_output(True))
        struct.pack_into('<I', raw, 24 + 504, 1025)
        with self.assertRaisesRegex(ValueError, 'trace count'):
            b.parse_output(raw)

    def test_unmapped_process_pointer_reject(self):
        raw = bytearray(synthetic_output())
        struct.pack_into('<I', raw, 24 + 8 + 104, 0x12340000)
        with self.assertRaisesRegex(ValueError, 'pointer normalization'):
            b.parse_output(raw)

    def test_record_order_and_slot_corruption_reject(self):
        for offset, value in ((24, 1), (24 + 508, 3)):
            raw = bytearray(synthetic_output(True))
            struct.pack_into('<I', raw, offset, value)
            with self.assertRaises(ValueError):
                b.parse_output(raw)

    def test_repeats_missing_or_different_reject(self):
        plain, traced = synthetic_output(), synthetic_output(True)
        with self.assertRaisesRegex(ValueError, 'three outputs'):
            b.compare_outputs([plain] * 2, [traced] * 3)
        with self.assertRaisesRegex(ValueError, 'not exactly repeatable'):
            b.compare_outputs([plain, plain, plain + b'x'], [traced] * 3)

    def test_any_numeric_byte_or_return_lane_changed_by_trace_rejects(self):
        plain, traced = synthetic_output(), synthetic_output(True)
        for offset in (24 + 8, 24 + 8 + 316, 24 + 464, 24 + 496):
            changed = bytearray(traced)
            changed[offset] ^= 1
            with self.subTest(offset=offset), self.assertRaisesRegex(ValueError, 'trace changed original output'):
                b.compare_outputs([plain] * 3, [bytes(changed)] * 3)

    def test_negative_slot_must_not_increment_iteration(self):
        plain, traced = synthetic_output(calls=36), bytearray(synthetic_output(True, calls=36))
        struct.pack_into('<I', traced, 24 + 508 + 31 * 128 + 8, 31)
        with self.assertRaisesRegex(ValueError, 'captured schedule'):
            b.compare_outputs([plain] * 3, [bytes(traced)] * 3)

    def test_exact_400_iteration_boundary_is_411_callbacks(self):
        plain, traced = synthetic_output(calls=411), synthetic_output(True, calls=411)
        report = b.compare_outputs([plain] * 3, [traced] * 3)
        self.assertEqual(report['runs'][0]['iteration'], 400)
        self.assertEqual(report['runs'][0]['visits'], [345, 11, 55])
        plain, traced = synthetic_output(calls=412), synthetic_output(True, calls=412)
        with self.assertRaisesRegex(ValueError, 'missing or unexpected'):
            b.compare_outputs([plain] * 3, [traced] * 3)

    def test_final_counter_disagreement_and_absent_trace_reject(self):
        plain, traced = bytearray(synthetic_output()), bytearray(synthetic_output(True))
        for raw in (plain, traced):
            struct.pack_into('<I', raw, 24 + 8 + 268, 28)
        with self.assertRaisesRegex(ValueError, 'final original counters'):
            b.compare_outputs([bytes(plain)] * 3, [bytes(traced)] * 3)
        raw = synthetic_output()
        with self.assertRaisesRegex(ValueError, 'missing or unexpected'):
            b.compare_outputs([raw] * 3, [raw] * 3)


if __name__ == '__main__':
    unittest.main()
