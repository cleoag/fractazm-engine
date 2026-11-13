"""Build an i386 FPC caller oracle from unchanged immutable Pascal procedures.

Generated Pascal and build products live exclusively in the editable Pascal tree.
The assembly tail shim records incoming SSE lanes without changing them. The
unmodified doHybridPas calls original m3f CODE, never translated C++ arithmetic.
This is an FPC SSE2 caller profile, not a claim about Delphi or arbitrary chains.
"""
import argparse
import hashlib
import json
import subprocess
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def source():
    types = (ROOT / 'mb3d/TypeDefinitions.pas').read_text()
    types = types[types.index('  ThybridIteration2 ='):types.index('  ThybridIteration =')]
    math = (ROOT / 'mb3d/Math3D.pas').read_text()
    start = math.index('procedure mCopyVec(', math.index('implementation'))
    copy = math[start:math.index('\nend;', start) + 5]
    formulas = (ROOT / 'mb3d/formulas.pas').read_text()
    start = formulas.index('procedure doHybridPas(', formulas.index('implementation'))
    hybrid = formulas[start:formulas.index('\nend;', start) + 5]
    return HEADER + types + '\n' + copy + STUB + hybrid + HARNESS


HEADER = '''program original_hybrid_oracle;
{$mode delphi}{$asmmode intel}
uses Windows, SysUtils, Classes, Math;
type TVec3D = array[0..2] of Double;
     TPVec3D = ^TVec3D;
     TSMatrix4 = array[0..3,0..3] of Single;
'''
STUB = '''
procedure CalcSmoothIterations(PIt3D: TPIteration3D; n: Integer);
begin Halt(90); end; { guarded: CalcSIT must remain false }
'''
HARNESS = '''
var CodePtr: Pointer;
    Calls: LongInt;
    Lanes: array[0..63,0..3] of Double;
    Params: array[0..1023] of Byte;
    It: TIteration3Dext;
    Stream: TFileStream;
    I, K, JuliaMode, Limit: Integer;
    StopValue, CX, CY, CZ, JX, JY, JZ: Double;

procedure Capture(var x,y,z,w: Double; PIteration3D: Pointer); assembler; nostackframe;
asm
  push ebx
  mov ebx, [Calls]
  cmp ebx, 64
  jae @skip
  shl ebx, 5
  movupd [Lanes+ebx], xmm0
  movupd [Lanes+ebx+16], xmm1
@skip:
  inc dword ptr [Calls]
  pop ebx
  jmp dword ptr [CodePtr]
end;

procedure Number(V: Double);
begin Write(FloatToStrF(V, ffGeneral, 17, 0), ' '); end;

begin
  if ParamCount <> 3 then Halt(2);
  if (SizeOf(Pointer) <> 4) or
     (PtrUInt(@It.C1)-PtrUInt(@It) <> 56) or
     (PtrUInt(@It.bFirstIt)-PtrUInt(@It.C1) <> 208) then Halt(3);
  DefaultFormatSettings.DecimalSeparator := '.';
  SetExceptionMask([exInvalidOp,exDenormalized,exZeroDivide,exOverflow,exUnderflow,exPrecision]);
  Set8087CW($133F);
  Stream := TFileStream.Create(ParamStr(1), fmOpenRead);
  CodePtr := VirtualAlloc(nil, Stream.Size, MEM_COMMIT or MEM_RESERVE, PAGE_EXECUTE_READWRITE);
  if CodePtr = nil then Halt(4);
  Stream.ReadBuffer(CodePtr^, Stream.Size); Stream.Free;
  Stream := TFileStream.Create(ParamStr(2), fmOpenRead);
  if Stream.Size <> SizeOf(Params) then Halt(5);
  Stream.ReadBuffer(Params, SizeOf(Params)); Stream.Free;
  while not EOF(Input) do begin
    ReadLn(CX,CY,CZ,JX,JY,JZ,JuliaMode,Limit,StopValue);
    if (Limit < 1) or (Limit > 64) then Halt(6);
    FillChar(It, SizeOf(It), 0); FillChar(Lanes, SizeOf(Lanes), 0); Calls := 0;
    It.C1:=CX; It.C2:=CY; It.C3:=CZ;
    It.Ju1:=JX; It.Ju2:=JY; It.Ju3:=JZ;
    It.DoJulia:=JuliaMode<>0; It.maxIt:=Limit; It.RStop:=StopValue;
    It.VaryScale:=1; It.Deriv1:=1;
    It.nHybrid[0]:=1; It.fHPVar[0]:=@Params[512];
    if ParamStr(3)='trace' then It.fHybrid[0]:=Capture
    else if ParamStr(3)='plain' then It.fHybrid[0]:=ThybridIteration2(CodePtr)
    else Halt(7);
    doHybridPas(TPIteration3D(@It.C1));
    Number(It.x); Number(It.y); Number(It.z); Number(It.w);
    Number(It.Rout); Number(It.Rold); Number(It.OTrap);
    Number(It.Dfree1); Number(It.Dfree2); Number(It.Deriv1); Number(It.Deriv2);
    Write(It.ItResultI,' ',It.bFirstIt,' ',Calls,' ');
    for I:=0 to Calls-1 do for K:=0 to 3 do Number(Lanes[I,K]);
    WriteLn;
  end;
  VirtualFree(CodePtr,0,MEM_RELEASE);
end.
'''


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--output', type=Path, required=True)
    ap.add_argument('--work', type=Path, default=Path('C:/work/_c/mb3d_pascal_work/mb3d_work/caller_oracle'))
    ap.add_argument('--fpc', type=Path, default=Path('C:/FPC/3.2.2/bin/i386-Win32/fpc.exe'))
    a = ap.parse_args()
    editable = Path('C:/work/_c/mb3d_pascal_work/mb3d_work').resolve()
    if not a.work.resolve().is_relative_to(editable):
        ap.error('Pascal build directory must be inside the editable Pascal working copy')
    a.work.mkdir(parents=True, exist_ok=True)
    a.output.mkdir(parents=True, exist_ok=False)
    pas = a.work / 'original_hybrid_oracle.pas'
    pas.write_text(source())
    command = [str(a.fpc), '-Mdelphi', '-CfSSE2', '-O2', '-al', str(pas.resolve())]
    run = subprocess.run(command, cwd=a.work, capture_output=True, text=True, timeout=90)
    (a.output/'build.log').write_text(run.stdout + run.stderr)
    run.check_returncode()
    exe = pas.with_suffix('.exe')
    for p in (pas, exe, pas.with_suffix('.s')):
        (a.output/p.name).write_bytes(p.read_bytes())
    paths = [a.fpc, a.fpc.with_name('ppc386.exe'), pas, exe, Path(__file__), *(ROOT/'mb3d'/p for p in ('formulas.pas','TypeDefinitions.pas','Math3D.pas'))]
    (a.output/'identity.json').write_text(json.dumps({'command': command, 'sha256': {str(p): digest(p) for p in paths}}, indent=2))
    print(exe)


if __name__ == '__main__':
    main()
