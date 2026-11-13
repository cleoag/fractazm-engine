"""Prepare unchanged original caller diagnostics; compile only in a fresh external tree.

No renderer/model arithmetic is used. Preparation is not execution or admission.
The six-process plan needs an external runner and independent actual-PE review.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import os
from pathlib import Path
import re
import shutil
import struct
import subprocess
import time

ROOT = Path(__file__).resolve().parents[2]
NAME = 'original_translate_callers_oracle'
COMPILER_SHA = 'a9d918be94200928a21efc61f51946004490e42986be4b9f9b93195cc971f502'
COMPILER_ARGS = ('-Mdelphi','-CfSSE2','-O2','-al','-va')
STATE_SIZE, TRACE_SIZE, TRACE_CAPACITY = 456, 512, 64
POINTER_OFFSETS = (104, *range(156, 204, 4), 408, 412, *range(416, 440, 4))
SOURCE_PINS = {
    'mb3d/formulas.pas': '3fefcd04fe6726831f3a7904f151cc3cc192610f4532f2889b9771e5953775bd',
    'mb3d/TypeDefinitions.pas': '5ef479844d3d5ffac309b141c9f80b62ebf0f9bf8b8782b706d9f451ef096747',
    'mb3d/Math3D.pas': '0b03d5d982be1285839831dfc36b2532725f5d679e2891b474d00d750b7b6727',
    'mb3d/HeaderTrafos.pas': '52b3209608ba3360f4be7d821866c37a1845bc09f9aa2ec56d1912616f5d7f9d',
    'mb3d/CustomFormulas.pas': '9ed1c8289ebfb99202795af8daca6bef92650ba250a28c12435eb5beb4264c61',
    'mb3d/M3Formulas/_Translate.m3f': '3acdcb61881b9007cd1a66a33887d8a13ae7541bd3120472b582e2ac9a09b706',
}
# name: unchanged Pascal procedure, DEoption, dimensions, analytic, interpolation
PROFILES = {
    'numeric3d': ('doHybridPas', 0, 3, False, False),
    'analytic11': ('doHybridPasDE', 11, 3, True, False),
    'numeric4d': ('doHybrid4DPas', 4, 4, False, False),
    'analytic4d5': ('doHybrid4DDEPas', 5, 4, True, False),
    'analytic4d6': ('doHybrid4DDEPas', 6, 4, True, False),
    'interpolation3d': ('doInterpolHybridPas', 0, 3, False, True),
    'interpolation11': ('doInterpolHybridPasDE', 11, 3, True, True),
    'interpolation4d': ('doInterpolHybridPas4D', 4, 4, False, True),
    'interpolation4d5': ('doInterpolHybridPas4DDE', 5, 4, True, True),
}


def sha(raw):
    return hashlib.sha256(raw).hexdigest()


def checked_sources(root=ROOT):
    result = {}
    for path, expected in SOURCE_PINS.items():
        raw = (Path(root) / path).read_bytes()
        if sha(raw) != expected:
            raise ValueError('immutable source changed: ' + path)
        result[path] = raw.decode('utf8').replace('\r\n', '\n')
    return result


def extract(text, name):
    match = re.search(r'^(?:procedure|function) ' + re.escape(name) + r'\(', text[text.index('implementation'):], re.M)
    if not match:
        raise ValueError('original procedure absent: ' + name)
    start = text.index('implementation') + match.start()
    return text[start:text.index('\nend;', start) + 5]


def original_sections(root=ROOT):
    originals = checked_sources(root)
    types = originals['mb3d/TypeDefinitions.pas']
    result = {'types': types[types.index('  ThybridIteration2 ='):types.index('  ThybridIteration =')]}
    for name in ('s05', 'd1em100'):
        result[name] = re.search(r'^  ' + name + r':[^\n]+', types, re.M).group()
    math = originals['mb3d/Math3D.pas']
    for name in ('mCopyVec', 'Rotate4Dex', 'CopyVec4SSE2'):
        result[name] = extract(math, name)
    formulas = originals['mb3d/formulas.pas']
    for name in ('CalcSmoothIterations', *dict.fromkeys(p[0] for p in PROFILES.values())):
        result[name] = extract(formulas, name)
    return result


def controls(profiles=None):
    selected = tuple(PROFILES) if profiles is None else tuple(profiles)
    if not selected or len(set(selected)) != len(selected) or any(p not in PROFILES for p in selected):
        raise ValueError('supported unique caller profiles required; dIFS/noVecIni/combined remain OPEN')
    rows = []
    for profile in selected:
        _, de, dim, analytic, interpol = PROFILES[profile]
        base = dict(profile=profile, de_option=de, dimensions=dim, analytic=analytic,
                    interpolation=interpol, point=[3.,4.,0.], julia=False, max_it=1,
                    rstop=1048576., smooth=False, count=1, deltas=[[0.,0.,0.],[0.,0.,0.]],
                    weights=[.5,.5], matrix='identity', negative_prefix=False)
        variants = [
            ('one', {}), ('max-count3', dict(max_it=4, count=3, deltas=[[.25,0.,0.],[0.,0.,0.]])),
            ('bailout-before-max', dict(max_it=4, rstop=16.)),
            ('bailout-equality', dict(point=[1.,0.,0.], max_it=2, rstop=1.)),
            ('julia', dict(julia=True, max_it=2)),
            ('zero', dict(point=[0.,0.,0.])),
            ('signed-zero', dict(point=[-0.,-0.,-0.], deltas=[[-0.,-0.,-0.],[-0.,-0.,-0.]])),
            ('smooth-low', dict(point=[.25,0.,0.], smooth=True, max_it=2)),
            ('smooth-old-below-one', dict(point=[.5,0.,0.], smooth=True, deltas=[[1.,0.,0.],[1.,0.,0.]])),
            ('smooth-old-above-one', dict(point=[2.,0.,0.], smooth=True, deltas=[[1.,0.,0.],[1.,0.,0.]])),
            ('smooth-old-equal-one', dict(point=[1.,0.,0.], smooth=True, deltas=[[1.,0.,0.],[1.,0.,0.]])),
        ]
        if dim == 4:
            variants.append(('nonzero-w-prepared-matrix', dict(matrix='swap-x-w')))
        if interpol:
            variants += [('weight-left', dict(weights=[1.,0.], deltas=[[1.,0.,0.],[0.,1.,0.]])),
                         ('weight-right', dict(weights=[0.,1.], deltas=[[1.,0.,0.],[0.,1.,0.]]))]
        else:
            variants.append(('negative-prefix', dict(negative_prefix=True, max_it=2)))
        for label, changes in variants:
            row = {**base, **changes, 'id': f'{profile}-{label}'}
            row['active_slots'] = 2 if interpol or row['negative_prefix'] else 1
            rows.append(row)
    return rows


def parameter_bytes(delta):
    data = bytearray(1024)
    for offset, value in zip((-8,-16,-24,-32), (.5,*delta)):
        struct.pack_into('<d', data, 512 + offset, value)
    return bytes(data)


def state_bytes(row):
    """Authored caller-unit input, not a claim about scene preparation/selection."""
    data = bytearray(STATE_SIZE)
    def put(offset, fmt, *values):
        struct.pack_into('<' + fmt, data, offset, *values)
    for offset, value in ((0,-7.5),(8,-2.),(16,3.25),(24,-9.),(32,10.),(40,-11.),(48,12.),
                          (80,-13.),(88,14.),(96,-15.),(112,17.),(248,-18.),(256,1.25),
                          (272,.25),(280,-.5),(288,7.),(296,11.),(304,-13.),(448,.125)):
        put(offset,'d',value)
    put(56,'3d',*row['point'])
    put(108,'f',-23.)
    put(120,'2i',37,row['max_it'])
    put(128,'f',row['rstop'])
    if row['interpolation']:
        put(132,'2f',*row['weights'])
    elif row['negative_prefix']:
        put(132,'2I',0x80000001,row['count'])
    else:
        put(132,'i',row['count'])
    put(204,'B',int(row['smooth']))
    put(206,'H',row['active_slots']-1)
    put(208,'i',-1 if row['julia'] else 0)
    put(212,'f',.5)
    put(216,'i',row['de_option'])
    put(220,'6f',*([1.]*6))
    put(264,'2i',31,41)
    matrix = [float(i//4 == i%4) for i in range(16)]
    if row['matrix'] == 'swap-x-w':
        # Exact prepared matrix input, not an angle->SinCos preparation claim.
        matrix[0]=matrix[15]=0.; matrix[3]=1.; matrix[12]=-1.
    put(312,'16f',*matrix)
    put(376,'4d',-.25,.5,-.75,1.25)
    put(444,'i',2)
    return bytes(data)


def input_payloads(rows, root=ROOT):
    original = checked_sources(root)['mb3d/M3Formulas/_Translate.m3f']
    code = bytes.fromhex(original.split('[CODE]')[1].split('[END]')[0])
    payload = bytearray()
    for row in rows:
        payload += struct.pack('<2I', tuple(PROFILES).index(row['profile']), row['active_slots'])
        payload += state_bytes(row)
        payload += b''.join(parameter_bytes(d) for d in row['deltas'])
    return {'original.code': code, 'controls.bin': bytes(payload)}


def capture_source(slot):
    lines = [f'''procedure Capture{slot}(var x,y,z,w: Double; PIteration3D: Pointer); assembler; nostackframe;
asm
  pushfd
  pushad
  mov ebx, [Calls]
  cmp ebx, {TRACE_CAPACITY}
  jae @overflow
  shl ebx, 9
  mov dword ptr [Trace+ebx], {slot}''']
    for offset in range(0, STATE_SIZE, 4):
        lines.append(f'  mov eax, dword ptr [It+{offset}]\n  mov dword ptr [Trace+ebx+{offset+4}], eax')
    lines.append(f'''  movupd [Trace+ebx+460], xmm0
  movupd [Trace+ebx+476], xmm1
  inc dword ptr [Calls]
  popad
  popfd
  jmp dword ptr [CodePtrs+{slot*4}]
@overflow:
  popad
  popfd
  jmp TraceAbort
end;''')
    return '\n'.join(lines)


HEADER = '''program original_translate_callers_oracle;
{$mode delphi}{$asmmode intel}
uses Windows, SysUtils, Classes, Math;
type TVec3D = array[0..2] of Double;
     TPVec3D = ^TVec3D;
     TVec4D = array[0..3] of Double;
     TPVec4D = ^TVec4D;
     TSMatrix4 = array[0..3,0..3] of Single;
     TPSMatrix4 = ^TSMatrix4;
     T2Vproc4 = procedure(V1,V2: TPVec4D);
'''

HARNESS = r'''
const Magic: array[0..7] of AnsiChar = ('M','B','T','C','A','L','0','1');
type TParams = array[0..1023] of Byte;
     PParams = ^TParams;
var It, BeforeState, Snapshot: TIteration3Dext;
    Trace: array[0..63,0..511] of Byte;
    CodePtrs, ParamMemory: array[0..1] of Pointer;
    Params: array[0..1] of PParams;
    OutLanes: array[0..3] of Double;
    Distance: Double;
    Calls, Profile, ActiveSlots, RunIndex, I, J, Version, RecordCount, StateSize, TraceSize: LongInt;
    InputStream, CodeStream, OutputStream: TFileStream;
    Directory: String;
    TraceMode: Boolean;

procedure TraceAbort;
begin Halt(91); end;
__CAPTURES__
__CALLS__

procedure BindPointers;
var Slot: Integer;
begin
  for Slot:=0 to ActiveSlots-1 do It.fHPVar[Slot]:=@Params[Slot]^[512];
  if TraceMode then begin
    It.fHybrid[0]:=Capture0;
    if ActiveSlots=2 then It.fHybrid[1]:=Capture1;
  end else
    for Slot:=0 to ActiveSlots-1 do It.fHybrid[Slot]:=ThybridIteration2(CodePtrs[Slot]);
end;

procedure Normalize(var State: TIteration3Dext);
var Slot, Token: Integer;
begin
  Token:=0;
  for Slot:=0 to ActiveSlots-1 do if State.PVar=It.fHPVar[Slot] then Token:=Slot+1;
  if (State.PVar<>nil) and (Token=0) then Halt(9);
  State.PVar:=Pointer(Token);
  for Slot:=0 to ActiveSlots-1 do begin
    State.fHPVar[Slot]:=Pointer(Slot+1);
    State.fHybrid[Slot]:=ThybridIteration2(Pointer(Slot+1));
  end;
end;

begin
  if ParamCount<>3 then Halt(2);
  if (SizeOf(Pointer)<>4) or (SizeOf(It)<>456) or
     (PtrUInt(@It.C1)-PtrUInt(@It)<>56) or
     (PtrUInt(@It.PVar)-PtrUInt(@It)<>104) or
     (PtrUInt(@It.fHPVar)-PtrUInt(@It)<>156) or
     (PtrUInt(@It.fHybrid)-PtrUInt(@It)<>180) or
     (PtrUInt(@It.CalcSIT)-PtrUInt(@It)<>204) or
     (PtrUInt(@It.bTmp)-PtrUInt(@It)<>268) or
     (PtrUInt(@It.Deriv1)-PtrUInt(@It)<>288) or
     (PtrUInt(@It.SMatrix4)-PtrUInt(@It)<>312) or
     (PtrUInt(@It.Ju1)-PtrUInt(@It)<>376) or
     (PtrUInt(@It.OTrapDE)-PtrUInt(@It)<>448) then Halt(3);
  TraceMode:=ParamStr(2)='trace';
  if not TraceMode and (ParamStr(2)<>'plain') then Halt(7);
  if FileExists(ParamStr(3)) then Halt(8);
  Directory:=IncludeTrailingPathDelimiter(ParamStr(1));
  SetExceptionMask([exInvalidOp,exDenormalized,exZeroDivide,exOverflow,exUnderflow,exPrecision]);
  Set8087CW($133F);
  { Original SSE2 startup choice: Mand.pas3180, Math3D.pas4107. }
  mCopyVec4:=CopyVec4SSE2;
  CodeStream:=TFileStream.Create(Directory+'original.code',fmOpenRead);
  if CodeStream.Size<>36 then Halt(5);
  for I:=0 to 1 do begin
    CodePtrs[I]:=VirtualAlloc(nil,CodeStream.Size,MEM_COMMIT or MEM_RESERVE,PAGE_EXECUTE_READWRITE);
    if CodePtrs[I]=nil then Halt(4);
    CodeStream.Position:=0; CodeStream.ReadBuffer(CodePtrs[I]^,CodeStream.Size);
    GetMem(ParamMemory[I],SizeOf(TParams)+15);
    Params[I]:=PParams((PtrUInt(ParamMemory[I])+15) and not PtrUInt(15));
  end;
  CodeStream.Free;
  InputStream:=TFileStream.Create(Directory+'controls.bin',fmOpenRead);
  if InputStream.Size<>__COUNT__*2512 then Halt(5);
  OutputStream:=TFileStream.Create(ParamStr(3),fmCreate);
  Version:=1; RecordCount:=__COUNT__; StateSize:=456; TraceSize:=512;
  OutputStream.WriteBuffer(Magic,8); OutputStream.WriteBuffer(Version,4);
  OutputStream.WriteBuffer(RecordCount,4); OutputStream.WriteBuffer(StateSize,4);
  OutputStream.WriteBuffer(TraceSize,4);
  for RunIndex:=0 to __COUNT__-1 do begin
    InputStream.ReadBuffer(Profile,4); InputStream.ReadBuffer(ActiveSlots,4);
    if (ActiveSlots<1) or (ActiveSlots>2) then Halt(6);
    InputStream.ReadBuffer(It,456);
    for I:=0 to 1 do InputStream.ReadBuffer(Params[I]^,1024);
    if (It.maxIt<1) or (It.maxIt>4) or (It.RStop<=0) then Halt(6);
    BindPointers;
    BeforeState:=It; Normalize(BeforeState);
    FillChar(Trace,SizeOf(Trace),0); Calls:=0;
    FillChar(OutLanes,SizeOf(OutLanes),0); FillChar(Distance,SizeOf(Distance),0);
    case Profile of
__DISPATCH__
    else Halt(6);
    end;
    Snapshot:=It; Normalize(Snapshot);
    OutputStream.WriteBuffer(RunIndex,4); OutputStream.WriteBuffer(Profile,4);
    OutputStream.WriteBuffer(BeforeState,456); OutputStream.WriteBuffer(Snapshot,456);
    OutputStream.WriteBuffer(OutLanes,32); OutputStream.WriteBuffer(Distance,8);
    OutputStream.WriteBuffer(Calls,4);
    for J:=0 to Calls-1 do begin
      Move(Trace[J,4],Snapshot,456); Normalize(Snapshot);
      Move(Snapshot,Trace[J,4],456);
      OutputStream.WriteBuffer(Trace[J],512);
    end;
  end;
  InputStream.Free; OutputStream.Free;
  for I:=0 to 1 do begin VirtualFree(CodePtrs[I],0,MEM_RELEASE); FreeMem(ParamMemory[I]); end;
end.
'''


def source(rows, root=ROOT):
    sections = original_sections(root)
    calls = []
    for index, (name,de,dim,analytic,interpol) in enumerate(PROFILES.values()):
        calls.append(f'''procedure Call{index}(P: TPIteration3D); assembler; nostackframe;
asm
  pxor xmm0, xmm0
  pxor xmm1, xmm1
  call {name}
  movupd [OutLanes], xmm0
  movupd [OutLanes+16], xmm1
''' + ('  fstp qword ptr [Distance]\n' if analytic else '') + 'end;\n')
    parts = [HEADER, sections['types'], 'const', sections['s05'], sections['d1em100'],
             sections['mCopyVec'], sections['Rotate4Dex'], sections['CopyVec4SSE2'],
             'var mCopyVec4: T2Vproc4;', sections['CalcSmoothIterations']]
    parts += [sections[name] for name in dict.fromkeys(p[0] for p in PROFILES.values())]
    harness = HARNESS.replace('__COUNT__',str(len(rows)))
    harness = harness.replace('__CAPTURES__','\n'.join(capture_source(s) for s in range(2)))
    harness = harness.replace('__CALLS__','\n'.join(calls))
    harness = harness.replace('__DISPATCH__','\n'.join(f'      {i}: Call{i}(TPIteration3D(@It.C1));' for i in range(len(PROFILES))))
    return '\n'.join(parts) + harness


def _state(raw, slots, allow_nil=False):
    expected = dict.fromkeys(POINTER_OFFSETS, 0)
    actual = {o:struct.unpack_from('<I',raw,o)[0] for o in POINTER_OFFSETS}
    if actual[104] not in range(0 if allow_nil else 1, slots+1):
        raise ValueError('unmapped PVar')
    expected[104] = actual[104]
    for slot in range(slots):
        expected[156+slot*4] = expected[180+slot*4] = slot+1
    if actual != expected:
        raise ValueError('unmapped process pointer')
    return raw.hex()


def parse_output(raw, rows):
    if len(raw)<24 or struct.unpack_from('<8s4I',raw)!=(b'MBTCAL01',1,len(rows),456,512):
        raise ValueError('invalid output header')
    offset, output = 24, []
    for index,row in enumerate(rows):
        if len(raw)<offset+964:
            raise ValueError('truncated state record')
        if struct.unpack_from('<2I',raw,offset)!=(index,tuple(PROFILES).index(row['profile'])):
            raise ValueError('unexpected record identity')
        before=_state(raw[offset+8:offset+464],row['active_slots'],True)
        expected_before=bytearray(state_bytes(row))
        for slot in range(row['active_slots']):
            struct.pack_into('<I',expected_before,156+slot*4,slot+1)
            struct.pack_into('<I',expected_before,180+slot*4,slot+1)
        if bytes.fromhex(before)!=bytes(expected_before):
            raise ValueError('captured input differs from declared control')
        after=_state(raw[offset+464:offset+920],row['active_slots'])
        calls,=struct.unpack_from('<I',raw,offset+960)
        if calls>TRACE_CAPACITY or len(raw)<offset+964+calls*512:
            raise ValueError('invalid trace count or truncated trace')
        trace=[]
        for call in range(calls):
            start=offset+964+call*512
            slot,=struct.unpack_from('<I',raw,start)
            if slot>=row['active_slots'] or raw[start+492:start+512]!=bytes(20):
                raise ValueError('invalid trace slot or padding')
            if struct.unpack_from('<I',raw,start+4+104)[0]!=slot+1:
                raise ValueError('trace slot and PVar disagree')
            trace.append(dict(slot=slot,state_hex=_state(raw[start+4:start+460],row['active_slots']),
                              sse64_hex=raw[start+460:start+492].hex()))
        output.append(dict(id=row['id'],before_hex=before,state_hex=after,
                           return_sse64_hex=raw[offset+920:offset+952].hex(),
                           distance64_hex=raw[offset+952:offset+960].hex(),trace=trace))
        offset+=964+calls*512
    if offset!=len(raw):
        raise ValueError('trailing output')
    return output


def compare_outputs(plain, traced, rows):
    if len(plain)!=3 or len(traced)!=3:
        raise ValueError('three fresh-process outputs per mode required')
    for runs in (plain,traced):
        if not runs[0]==runs[1]==runs[2]:
            raise ValueError('outputs not exactly repeatable')
    left,right=parse_output(plain[0],rows),parse_output(traced[0],rows)
    for a,b in zip(left,right):
        if a['trace'] or not b['trace']:
            raise ValueError('missing trace or traced plain run')
        if {k:v for k,v in a.items() if k!='trace'}!={k:v for k,v in b.items() if k!='trace'}:
            raise ValueError('trace changed original state/return')
    return dict(status='REPEAT_AND_TRACE_NEUTRALITY_PASS',qualification=False,case_count=len(rows),
                limitations=['fresh process provenance external','no numerical/model credit','actual PE review required'])


def prepare(output, profiles=None, root=ROOT):
    output=Path(output)
    if output.exists():
        raise ValueError('fresh output directory required')
    rows=controls(profiles)
    sections=original_sections(root)
    payloads=input_payloads(rows,root)
    generated=source(rows,root)
    metadata_path=Path(root)/'src/render/native_metal/generated/formula_metadata.h'
    metadata=metadata_path.read_text()
    expected='case 8065u: slot.traits=0u; slot.wrapper_flags=1u; slot.difs_style=0u; return true;'
    if expected not in metadata:
        raise ValueError('actual Translate metadata changed')
    inventory_path=Path(root)/'src/render/native_metal/generated/inventory.json'
    inventory=json.loads(inventory_path.read_text())
    records=[r for r in inventory['formulas'] if r['id']==8065]
    typed=dict(id=8065,name='_Translate',de_option=-1,param_count=3,
               parameter_types=[0,0,0],adds_c=1,difs_style=0,traits=0)
    if len(records)!=1 or any(records[0].get(k)!=v for k,v in typed.items()):
        raise ValueError('actual Translate typed metadata changed')
    abi_path=Path(root)/'src/render/native_metal/formula_abi.h'
    output.mkdir(parents=True)
    (output/'inputs').mkdir()
    for name,data in payloads.items():
        (output/'inputs'/name).write_bytes(data)
    (output/(NAME+'.pas')).write_text(generated)
    (output/'controls.json').write_text(json.dumps(rows,indent=2)+'\n')
    plan=[dict(id=f'{mode}-{repeat}',argv=[NAME+'.exe','INPUT_DIRECTORY',mode,f'{mode}-{repeat}.bin'],
               fresh_process=True,timeout_seconds=30) for mode in ('plain','trace') for repeat in range(1,4)]
    (output/'process-plan.json').write_text(json.dumps(plan,indent=2)+'\n')
    manifest=dict(schema='mb3d.original-translate-callers-preparation.v1',status='PREPARED_NOT_COMPILED',qualification=False,
                  source_pins=SOURCE_PINS,section_sha256={k:sha(v.encode()) for k,v in sections.items()},
                  generator_sha256=sha(Path(__file__).read_bytes()),
                  metadata=dict(path=str(metadata_path.relative_to(root)),sha256=sha(metadata_path.read_bytes()),descriptor=expected),
                  typed_metadata=dict(path=str(inventory_path.relative_to(root)),sha256=sha(inventory_path.read_bytes()),fields=typed),
                  abi=dict(path=str(abi_path.relative_to(root)),sha256=sha(abi_path.read_bytes())),
                  compiler=dict(sha256=COMPILER_SHA,args=list(COMPILER_ARGS)),
                  entry_sse='XMM0/XMM1 zero before unchanged caller',state_bytes=456,trace_bytes=512,
                  profiles=list(dict.fromkeys(r['profile'] for r in rows)),case_count=len(rows),
                  limitations=['caller-unit inputs, not authored driver selection','no RN32 projection or numerical admission',
                               'prepared identity/swap matrix, no angle preparation claim',
                               'SSE2 CopyVec4SSE2 profile only','dIFS/noVecIni ABI unresolved and never executed',
                               'combined outer two-program contract OPEN','compiler read closure requires external capture'],
                  files={str(p.relative_to(output)):sha(p.read_bytes()) for p in sorted(output.rglob('*')) if p.is_file()})
    (output/'preparation.json').write_text(json.dumps(manifest,indent=2)+'\n')
    return manifest


def validate_external_work(work):
    editable=Path('C:/work/_c/mb3d_pascal_work/mb3d_work').resolve()
    if os.name!='nt' or not work.resolve().is_relative_to(editable) or work.exists():
        raise ValueError('fresh external Windows Pascal work directory required')


def validate_prepared(output):
    raw=(output/'preparation.json').read_bytes()
    manifest=json.loads(raw)
    if (manifest.get('schema')!='mb3d.original-translate-callers-preparation.v1' or
        manifest.get('generator_sha256')!=sha(Path(__file__).read_bytes()) or
        manifest.get('compiler',{}).get('sha256')!=COMPILER_SHA):
        raise ValueError('prepared generator/compiler identity mismatch')
    if manifest['compiler'].get('args')!=list(COMPILER_ARGS):
        raise ValueError('exact reviewed compiler arguments required')
    for rel,expected in manifest['files'].items():
        if sha((output/rel).read_bytes())!=expected:
            raise ValueError('prepared input changed: '+rel)
    return manifest,sha(raw)


def compile_prepared(output, work, fpc):
    """Captured diagnostic identity, not production clean-build qualification."""
    output,work,fpc=map(Path,(output,work,fpc))
    validate_external_work(work)
    if sha(fpc.read_bytes())!=COMPILER_SHA:
        raise ValueError('pinned original FPC compiler required')
    manifest,preparation_sha256=validate_prepared(output)
    if (output/'compile').exists():
        raise ValueError('new compile output required')
    ancestor=work.parent
    while not ancestor.exists(): ancestor=ancestor.parent
    if min(shutil.disk_usage(ancestor).free,shutil.disk_usage(output).free)<12*1024**3:
        raise ValueError('12 GiB disk reserve required')
    work.mkdir(parents=True); target=output/'compile'; target.mkdir()
    pas=work/(NAME+'.pas'); pas.write_bytes((output/pas.name).read_bytes())
    command=[str(fpc.resolve()),*manifest['compiler']['args'],str(pas.resolve())]
    started=time.time_ns()
    with (target/'build.log').open('wb') as log:
        process=subprocess.Popen(command,cwd=work,stdout=log,stderr=subprocess.STDOUT)
        try:
            code=process.wait(timeout=90)
        except subprocess.TimeoutExpired:
            process.kill(); process.wait(); code=-1
    receipt=dict(schema='mb3d.original-caller-compile-identity.v1',command=command,cwd=str(work.resolve()),
                 pid=process.pid,start_time_ns=started,end_time_ns=time.time_ns(),exit_code=code,
                 compiler_sha256=sha(fpc.read_bytes()),preparation_sha256=preparation_sha256,
                 status='COMPILE_FAILED' if code else 'COMPILED_PENDING_PE_AND_CLOSURE_REVIEW',qualification=False,
                 limitations=['-va verbose log preserved; external compiler-read closure still required',
                              'actual PE disassembly, extracted-body equality and neutral process runs required'])
    errors=[]
    preparation_after=sha((output/'preparation.json').read_bytes())
    receipt['preparation_sha256_after']=preparation_after
    if preparation_after!=preparation_sha256:
        errors.append('preparation manifest changed during compilation')
    if sha(fpc.read_bytes())!=COMPILER_SHA:
        errors.append('compiler changed during compilation')
    if sha(pas.read_bytes())!=manifest['files'][pas.name]:
        errors.append('generated source changed during compilation')
    for suffix in ('.pas','.exe','.s','.o'):
        p=pas.with_suffix(suffix)
        if p.exists(): shutil.copyfile(p,target/p.name)
    receipt['products']={p.name:sha(p.read_bytes()) for p in sorted(target.iterdir()) if p.is_file()}
    exe=target/(NAME+'.exe')
    if exe.exists():
        raw=exe.read_bytes()
        pe=struct.unpack_from('<I',raw,60)[0] if len(raw)>=64 else len(raw)
        if (raw[:2]!=b'MZ' or pe+26>len(raw) or raw[pe:pe+4]!=b'PE\0\0' or
            struct.unpack_from('<H',raw,pe+4)[0]!=0x14c or struct.unpack_from('<H',raw,pe+24)[0]!=0x10b):
            errors.append('product is not i386 PE32')
    receipt['capture_errors']=errors
    if errors: receipt['status']='COMPILE_IDENTITY_REJECTED'
    (target/'identity.json').write_text(json.dumps(receipt,indent=2)+'\n')
    if code or errors or any(NAME+s not in receipt['products'] for s in ('.pas','.exe','.s','.o')):
        raise ValueError('compile failed or diagnostic products missing; retained attempt must not be overwritten')
    return receipt


def main():
    ap=argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--output',type=Path,required=True)
    mode=ap.add_mutually_exclusive_group()
    mode.add_argument('--prepare-only',action='store_true')
    mode.add_argument('--compile-prepared',action='store_true',help='compile an unchanged transferred preparation package')
    ap.add_argument('--profiles',nargs='+',choices=tuple(PROFILES))
    ap.add_argument('--work',type=Path)
    ap.add_argument('--fpc',type=Path)
    a=ap.parse_args()
    if a.prepare_only:
        if a.work or a.fpc: ap.error('prepare-only cannot compile')
    elif not a.work or not a.fpc:
        ap.error('--prepare-only or explicit --work and --fpc required')
    if a.compile_prepared and a.profiles:
        ap.error('prepared profiles cannot change')
    if not a.compile_prepared: prepare(a.output,a.profiles)
    if not a.prepare_only: compile_prepared(a.output,a.work,a.fpc)
    print(a.output/'preparation.json')


if __name__=='__main__':
    main()
