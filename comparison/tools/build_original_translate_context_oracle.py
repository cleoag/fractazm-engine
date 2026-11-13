"""Build a bounded original mixed-count FPC caller diagnostic in a fresh tree.

Only unchanged original Pascal bodies and original M3F CODE execute. Original
negative PVar bytes survive unchanged; Julia is explicitly the prepared binary32
projection widened to Double. This is NOT an original-scene/Metal/RN32 oracle.
No VM execution is performed by this builder. Run the resulting executable as
``original_translate_context_oracle.exe INPUT_DIRECTORY plain|trace NEW.bin``.
"""
from __future__ import annotations

import argparse
import hashlib
import json
from pathlib import Path
import shutil
import struct
import subprocess

from build_original_hybrid_oracle import HEADER, STUB, ROOT

PROGRAM = 'comparison/runs/formula-contexts-20261104/original-contexts-v2/ecc0b336d49fa5e9/primary.bin'
PROGRAM_SHA = '04e705d6a1826330c6ef1101d54f0013bb112e7e8cb080463791b91520a462f0'
ASSETS = (
    ('Amazing Surf', '19f5cfa2cdd127a34851f7312c0226078aea675ca9d17e6079b46b9ec70be03d'),
    ('_Translate', '3acdcb61881b9007cd1a66a33887d8a13ae7541bd3120472b582e2ac9a09b706'),
    ('_AmazingBox', 'a3e18ad5fb97eaf275534d24e5b4bece03110384940edfa90b8fe93d3f2e06ce'),
)
POINTS = ((0, 0, 0), (1, 0, 0), (-1, 0, 0), (0, 1, 0),
          (0, 0, 1), (1, 1, 1), (-1, -1, -1), (4, 2, 1))
# The last four records prove actual original carry, never RN32-derived carry.
RUNS = tuple((i, False) for i in range(8)) + ((3, False), (4, True), (4, False), (3, True))
STATE_SIZE, TRACE_SIZE, TRACE_CAPACITY = 456, 128, 1024
TRACE_DOUBLES = ('vary_scale', 'x', 'y', 'z', 'w', 'rout', 'rold', 'otrap',
                 'xmm0_low', 'xmm0_high', 'xmm1_low', 'xmm1_high', 'rstopd', 'deriv1')
# Absolute byte offsets from packed TIteration3Dext, checked by generated code.
TRACE_OFFSETS = {0: 256, 1: 24, 2: 32, 3: 40, 4: 48, 5: 112, 6: 8, 7: 248,
                 12: 16, 13: 288}
POINTER_OFFSETS = (104, *range(156, 180, 4), *range(180, 204, 4),
                   408, 412, *range(416, 440, 4))


def digest(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def checked(path, expected):
    raw = Path(path).read_bytes()
    if hashlib.sha256(raw).hexdigest() != expected:
        raise ValueError('immutable diagnostic input changed: ' + str(path))
    return raw


def input_payloads(root=ROOT):
    root = Path(root)
    raw = checked(root / PROGRAM, PROGRAM_SHA)
    words = struct.unpack('<2048I', raw)
    if words[1:8] != (1, 6, 0, 0, 2, 2, 400) or words[50:53] != (30, 0x80000001, 5):
        raise ValueError('unsupported caller profile')
    result = {'julia.bin': struct.pack('<4d', *struct.unpack('<4f', raw[96:112]))}
    for slot, (name, sha) in enumerate(ASSETS):
        asset = checked(root / 'mb3d/M3Formulas' / (name + '.m3f'), sha)
        text = asset.decode('latin1')
        code = bytes.fromhex(text.split('[CODE]')[1].split('[END]')[0])
        base = (56 + 332 * slot + 204) * 4
        params = bytearray(1024)
        params[256:512] = raw[base:base + 256]
        result[f'code{slot}.bin'] = code
        result[f'params{slot}.bin'] = bytes(params)
    return result


def capture_source(slot):
    lines = [f'''procedure Capture{slot}(var x,y,z,w: Double; PIteration3D: Pointer); assembler; nostackframe;
asm
  pushfd
  pushad
  mov ebx, [Calls]
  cmp ebx, {TRACE_CAPACITY}
  jae @overflow
  shl ebx, 7
  mov dword ptr [Trace+ebx], {slot}
  mov eax, [It+268]
  mov dword ptr [Trace+ebx+4], eax
  mov eax, [It+120]
  mov dword ptr [Trace+ebx+8], eax
  mov eax, [It+264]
  mov dword ptr [Trace+ebx+12], eax''']
    for index, offset in TRACE_OFFSETS.items():
        for half in (0, 4):
            lines.append(f'  mov eax, [It+{offset + half}]\n  mov dword ptr [Trace+ebx+{16 + index * 8 + half}], eax')
    lines.append(f'''  movupd [Trace+ebx+80], xmm0
  movupd [Trace+ebx+96], xmm1
  inc dword ptr [Calls]
  popad
  popfd
  jmp dword ptr [CodePtrs+{slot * 4}]
@overflow:
  popad
  popfd
  jmp TraceAbort
end;
''')
    return '\n'.join(lines)


HARNESS = r'''
const Points: array[0..7,0..2] of Double =
 ((0,0,0),(1,0,0),(-1,0,0),(0,1,0),(0,0,1),(1,1,1),(-1,-1,-1),(4,2,1));
      PointIndices: array[0..11] of Integer = (0,1,2,3,4,5,6,7,3,4,4,3);
      Magic: array[0..7] of AnsiChar = ('M','B','T','C','T','X','0','1');
type TParams = array[0..1023] of Byte;
     PParams = ^TParams;
var CodePtrs: array[0..2] of Pointer;
    ParamMemory: array[0..2] of Pointer;
    Params: array[0..2] of PParams;
    CodeSizes: array[0..2] of LongInt;
    Julia: array[0..3] of Double;
    It, Snapshot: TIteration3Dext;
    Trace: array[0..1023,0..127] of Byte;
    Calls: LongInt;
    OutLanes: array[0..3] of Double;
    Distance: Double;
    Stream, OutputStream: TFileStream;
    I, J, RunIndex, PointIndex, Token, Version, RecordCount, RecordSize, TraceSize: LongInt;
    TraceMode: Boolean;
    Directory, Filename: String;

procedure TraceAbort;
begin Halt(91); end; { overflow is a failed diagnostic, never a partial trace }

__CAPTURE__

procedure CallAndCapture(P: TPIteration3D); assembler; nostackframe;
asm
  call doHybridPasDE
  movupd [OutLanes], xmm0
  movupd [OutLanes+16], xmm1
  fstp qword ptr [Distance]
end;

procedure BindPointers;
var Slot: Integer;
begin
  for Slot:=0 to 2 do It.fHPVar[Slot]:=@Params[Slot]^[512];
  if TraceMode then begin
    It.fHybrid[0]:=Capture0; It.fHybrid[1]:=Capture1; It.fHybrid[2]:=Capture2;
  end else
    for Slot:=0 to 2 do It.fHybrid[Slot]:=ThybridIteration2(CodePtrs[Slot]);
end;

begin
  if ParamCount<>3 then Halt(2);
  if (SizeOf(Pointer)<>4) or (SizeOf(It)<>456) or
     (PtrUInt(@It.C1)-PtrUInt(@It)<>56) or
     (PtrUInt(@It.bTmp)-PtrUInt(@It)<>268) or
     (PtrUInt(@It.Deriv1)-PtrUInt(@It)<>288) or
     (PtrUInt(@It.Ju1)-PtrUInt(@It)<>376) then Halt(3);
  Directory:=IncludeTrailingPathDelimiter(ParamStr(1));
  TraceMode:=ParamStr(2)='trace';
  if not TraceMode and (ParamStr(2)<>'plain') then Halt(7);
  if FileExists(ParamStr(3)) then Halt(8);
  SetExceptionMask([exInvalidOp,exDenormalized,exZeroDivide,exOverflow,exUnderflow,exPrecision]);
  Set8087CW($133F);
  Stream:=TFileStream.Create(Directory+'julia.bin',fmOpenRead);
  if Stream.Size<>32 then Halt(5);
  Stream.ReadBuffer(Julia,32); Stream.Free;
  for I:=0 to 2 do begin
    Filename:=Directory+'code'+IntToStr(I)+'.bin';
    Stream:=TFileStream.Create(Filename,fmOpenRead);
    if (Stream.Size<1) or (Stream.Size>4096) then Halt(5);
    CodeSizes[I]:=Stream.Size;
    CodePtrs[I]:=VirtualAlloc(nil,CodeSizes[I],MEM_COMMIT or MEM_RESERVE,PAGE_EXECUTE_READWRITE);
    if CodePtrs[I]=nil then Halt(4);
    Stream.ReadBuffer(CodePtrs[I]^,CodeSizes[I]); Stream.Free;
    GetMem(ParamMemory[I],SizeOf(TParams)+15);
    Params[I]:=PParams((PtrUInt(ParamMemory[I])+15) and not PtrUInt(15));
    Stream:=TFileStream.Create(Directory+'params'+IntToStr(I)+'.bin',fmOpenRead);
    if Stream.Size<>SizeOf(TParams) then Halt(5);
    Stream.ReadBuffer(Params[I]^,SizeOf(TParams)); Stream.Free;
  end;
  OutputStream:=TFileStream.Create(ParamStr(3),fmCreate);
  Version:=1; RecordCount:=12; RecordSize:=SizeOf(It); TraceSize:=128;
  OutputStream.WriteBuffer(Magic,8);
  OutputStream.WriteBuffer(Version,4); OutputStream.WriteBuffer(RecordCount,4);
  OutputStream.WriteBuffer(RecordSize,4); OutputStream.WriteBuffer(TraceSize,4);
  for RunIndex:=0 to 11 do begin
    if (RunIndex<>9) and (RunIndex<>11) then begin
      FillChar(It,SizeOf(It),0);
      It.Ju1:=Julia[0]; It.Ju2:=Julia[1]; It.Ju3:=Julia[2]; It.Ju4:=Julia[3];
      It.J4:=Julia[3];
      It.VaryScale:=1; It.Deriv1:=1;
      It.maxIt:=400; It.RStop:=1048576; It.DoJulia:=True; It.DEoption:=2;
      It.nHybrid[0]:=30; It.nHybrid[1]:=LongInt($80000001); It.nHybrid[2]:=5;
      It.iStartFrom:=0; It.iRepeatFrom:=0; It.EndTo:=2;
      BindPointers;
    end;
    PointIndex:=PointIndices[RunIndex];
    It.C1:=Points[PointIndex,0]; It.C2:=Points[PointIndex,1]; It.C3:=Points[PointIndex,2];
    FillChar(Trace,SizeOf(Trace),0); Calls:=0;
    CallAndCapture(TPIteration3D(@It.C1));
    Snapshot:=It;
    { Normalize only process addresses; every numeric byte remains verbatim. }
    Token:=0;
    for J:=0 to 2 do if It.PVar=It.fHPVar[J] then Token:=J+1;
    if Token=0 then Halt(9);
    Snapshot.PVar:=Pointer(Token);
    for J:=0 to 2 do begin
      Snapshot.fHPVar[J]:=Pointer(J+1);
      Snapshot.fHybrid[J]:=ThybridIteration2(Pointer(J+1));
    end;
    OutputStream.WriteBuffer(RunIndex,4); OutputStream.WriteBuffer(PointIndex,4);
    OutputStream.WriteBuffer(Snapshot,SizeOf(Snapshot));
    OutputStream.WriteBuffer(OutLanes,32); OutputStream.WriteBuffer(Distance,8);
    OutputStream.WriteBuffer(Calls,4);
    if Calls>0 then OutputStream.WriteBuffer(Trace,Calls*128);
  end;
  OutputStream.Free;
  for I:=0 to 2 do begin VirtualFree(CodePtrs[I],0,MEM_RELEASE); FreeMem(ParamMemory[I]); end;
end.
'''


def source():
    types = (ROOT / 'mb3d/TypeDefinitions.pas').read_text()
    types = types[types.index('  ThybridIteration2 ='):types.index('  ThybridIteration =')]
    math = (ROOT / 'mb3d/Math3D.pas').read_text()
    start = math.index('procedure mCopyVec(', math.index('implementation'))
    copy = math[start:math.index('\nend;', start) + 5]
    formulas = (ROOT / 'mb3d/formulas.pas').read_text()
    start = formulas.index('function doHybridPasDE(', formulas.index('implementation'))
    hybrid = formulas[start:formulas.index('\nend;', start) + 5]
    harness = HARNESS.replace('__CAPTURE__', '\n'.join(capture_source(i) for i in range(3)))
    return HEADER.replace('original_hybrid_oracle', 'original_translate_context_oracle') + types + '\n' + copy + STUB + hybrid + harness


def parse_output(raw):
    """Decode raw bits without float conversion; never grant numerical credit."""
    if len(raw) < 24 or struct.unpack_from('<8s4I', raw) != (b'MBTCTX01', 1, 12, STATE_SIZE, TRACE_SIZE):
        raise ValueError('unsupported or truncated caller output')
    offset, rows = 24, []
    for index, (point, carry) in enumerate(RUNS):
        if len(raw) < offset + 508:
            raise ValueError('truncated caller record')
        run_id, point_id = struct.unpack_from('<2I', raw, offset)
        if (run_id, point_id) != (index, point):
            raise ValueError('unexpected caller record order')
        state = raw[offset + 8:offset + 464]
        pointer_values = {pos: struct.unpack_from('<I', state, pos)[0] for pos in POINTER_OFFSETS}
        expected_pointers = dict.fromkeys(POINTER_OFFSETS, 0)
        expected_pointers[104] = pointer_values[104]
        for slot in range(3):
            expected_pointers[156 + 4 * slot] = slot + 1
            expected_pointers[180 + 4 * slot] = slot + 1
        if pointer_values[104] not in (1, 2, 3) or pointer_values != expected_pointers:
            raise ValueError('unexpected pointer normalization')
        lanes = raw[offset + 464:offset + 496]
        distance = raw[offset + 496:offset + 504]
        calls, = struct.unpack_from('<I', raw, offset + 504)
        if calls > TRACE_CAPACITY or len(raw) < offset + 508 + calls * TRACE_SIZE:
            raise ValueError('invalid or truncated trace count')
        trace = []
        for call in range(calls):
            values = struct.unpack_from('<4I14Q', raw, offset + 508 + call * TRACE_SIZE)
            if values[0] > 2 or values[1] < 1 or values[1] > 30 or values[2] > 400:
                raise ValueError('invalid callback schedule')
            trace.append({'slot': values[0], 'remaining': values[1], 'iteration': values[2],
                          'first': values[3], 'raw64': dict(zip(TRACE_DOUBLES, map(lambda v: f'{v:016x}', values[4:])))})
        rows.append({'run': index, 'point': list(POINTS[point]), 'carry': carry,
                     'state_hex': state.hex(), 'return_sse64': [f'{v:016x}' for v in struct.unpack('<4Q', lanes)],
                     'distance64': distance.hex(), 'calls': calls, 'trace': trace})
        offset += 508 + calls * TRACE_SIZE
    if offset != len(raw):
        raise ValueError('unexpected trailing caller output')
    return rows


def compare_outputs(plain_runs, traced_runs):
    """Check three fresh-process captures supplied by the runner, not run them.

    Files do not establish fresh process identity themselves; the external
    runner must pin argv/process/repeat evidence. This helper proves only byte
    repeatability, observation neutrality and the bounded original schedule.
    """
    if len(plain_runs) != 3 or len(traced_runs) != 3:
        raise ValueError('three outputs per mode required; process provenance is external')
    for runs in (plain_runs, traced_runs):
        if runs[0] != runs[1] or runs[0] != runs[2]:
            raise ValueError('original caller outputs are not exactly repeatable')
    plain, traced = parse_output(plain_runs[0]), parse_output(traced_runs[0])
    summaries = []
    for left, right in zip(plain, traced):
        if left['calls'] or not 1 <= right['calls'] <= 411:
            raise ValueError('missing or unexpected callback trace')
        for key in ('state_hex', 'return_sse64', 'distance64'):
            if left[key] != right[key]:
                raise ValueError('trace changed original output: ' + key)
        slot, remaining, iteration = 0, 30, 0
        visits = [0, 0, 0]
        for event in right['trace']:
            if remaining == 0:
                slot = (slot + 1) % 3
                remaining = (30, 1, 5)[slot]
            if (event['slot'], event['remaining'], event['iteration']) != (slot, remaining, iteration):
                raise ValueError('captured schedule contradicts unchanged original caller')
            visits[slot] += 1
            remaining -= 1
            iteration += slot != 1
            if iteration > 400:
                raise ValueError('captured schedule exceeds original iteration limit')
        state = bytes.fromhex(right['state_hex'])
        if (struct.unpack_from('<I', state, 120)[0], struct.unpack_from('<I', state, 268)[0],
            struct.unpack_from('<I', state, 104)[0]) != (iteration, remaining, slot + 1):
            raise ValueError('final original counters disagree with actual trace')
        summaries.append({'run': right['run'], 'point': right['point'], 'carry': right['carry'],
                          'calls': right['calls'], 'visits': visits, 'iteration': iteration,
                          'target_and_successor_reached': all(visits),
                          'return_sse64': right['return_sse64']})
    return {'status': 'REPEAT_AND_TRACE_NEUTRALITY_PASS', 'qualification': False,
            'process_provenance': 'external runner evidence required', 'runs': summaries}


def manifest():
    return {'schema': 'mb3d.original-translate-context-diagnostic.v1',
            'status': 'BUILT_NOT_EXECUTED', 'qualification': False,
            'profile': 'NEW FPC i386 SSE2 original mixed negative-count caller',
            'projection': 'Julia projected binary32 prepared words widened exactly to Double; raw original negative PVar bytes unchanged',
            'limitations': ['not original scene Double parity', 'not RN32 or Metal arithmetic',
                            'finite compiled caller profile only', 'fresh cases have zero auxiliary fields, Deriv1=VaryScale=1',
                            'carry cases use actual preceding original state, not RN32 model output'],
            'points': POINTS, 'runs': [{'point': i, 'carry_previous_original': carry} for i, carry in RUNS],
            'output': {'endianness': 'little', 'state_size': STATE_SIZE,
                       'state': 'complete packed TIteration3Dext, active PVar/fHPVar/fHybrid addresses replaced by 1-based slot tokens',
                       'trace_capacity': TRACE_CAPACITY, 'trace_size': TRACE_SIZE,
                       'trace_integer_fields': ['slot', 'remaining', 'iteration', 'first'],
                       'trace_raw64_fields': TRACE_DOUBLES},
            'runtime_argv': ['original_translate_context_oracle.exe', 'INPUT_DIRECTORY', 'plain|trace', 'NEW_OUTPUT.bin'],
            'runtime_timeout_seconds': 30, 'repeat_requirement': '3 fresh processes per mode; compare complete raw state/return bits plain versus traced'}


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--work', type=Path, required=True)
    ap.add_argument('--output', type=Path, required=True)
    ap.add_argument('--fpc', type=Path, required=True)
    a = ap.parse_args()
    editable = Path('C:/work/_c/mb3d_pascal_work/mb3d_work').resolve()
    if not a.work.resolve().is_relative_to(editable) or a.work.exists() or a.output.exists():
        ap.error('fresh external Pascal work and new output directories required')
    for path in (a.work, a.output):
        existing = path.resolve()
        while not existing.exists():
            existing = existing.parent
        if shutil.disk_usage(existing).free < 12 * 1024 ** 3:
            ap.error('12 GiB free disk space is required')
    payloads = input_payloads()
    a.work.mkdir(parents=True); a.output.mkdir(parents=True)
    inputs = a.output / 'inputs'; inputs.mkdir()
    for name, data in payloads.items():
        (inputs / name).write_bytes(data)
    pas = a.work / 'original_translate_context_oracle.pas'
    pas.write_text(source())
    command = [str(a.fpc), '-Mdelphi', '-CfSSE2', '-O2', '-al', str(pas.resolve())]
    run = subprocess.run(command, cwd=a.work, capture_output=True, text=True, timeout=90)
    (a.output / 'build.log').write_text(run.stdout + run.stderr)
    run.check_returncode()
    products = [pas.with_suffix(ext) for ext in ('.pas', '.exe', '.s', '.o')]
    for path in products:
        (a.output / path.name).write_bytes(path.read_bytes())
    paths = [a.fpc, *products, Path(__file__), Path(__file__).with_name('build_original_hybrid_oracle.py'),
             ROOT / PROGRAM, *(ROOT / 'mb3d' / p for p in ('formulas.pas', 'TypeDefinitions.pas', 'Math3D.pas')),
             *(ROOT / 'mb3d/M3Formulas' / (name + '.m3f') for name, _ in ASSETS), *inputs.iterdir()]
    identity = manifest()
    identity.update(command=command, sha256={str(path): digest(path) for path in paths})
    (a.output / 'identity.json').write_text(json.dumps(identity, indent=2) + '\n')
    print(a.output / 'original_translate_context_oracle.exe')


if __name__ == '__main__':
    main()
