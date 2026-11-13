"""Bounded tests of the actual patched Pascal dispatch procedures (no GUI)."""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess


def procedure(source, name):
    import re
    match = re.search(r'(?m)^(?:procedure|function) ' + name + r'\(', source[source.index('implementation'):])
    start = source.index('implementation') + match.start()
    return source[start:source.index('\nend;', start) + 5]


HARNESS = r'''
var It: TIteration3Dext; Params: array[0..255] of Byte;
    Calls, Checks, Driver, Mode, J: Integer; Failed: Boolean; V: Double;
procedure Check(B: Boolean; const S: string);
begin if not B then raise Exception.Create(S); Inc(Checks); end;
procedure Noop(var X,Y,Z,W: Double; P: Pointer);
begin
  Inc(Calls);
  if Mode=1 then TPIteration3D(P)^.ItResultI:=-1;
end;
procedure IFSNoop; assembler; nostackframe;
asm
  inc dword ptr [Calls]
  mov dword ptr [esi-32], 0
  mov dword ptr [esi-28], 0
end;
procedure Setup;
begin
  ClearFormulaExecutionFailure;
  FillChar(It,SizeOf(It),0); Calls:=0; Mode:=0;
  It.C1:=0.25; It.C2:=0.5; It.C3:=0.75;
  It.RStop:=100; It.maxIt:=2; It.bIsInsideRender:=True;
  for J:=0 to 3 do It.SMatrix4[J,J]:=1;
  for J:=0 to 5 do begin
    It.fHPVar[J]:=@Params[128];
    if (Driver < 2) or (Driver >= 4) then It.fHybrid[J]:=Noop
    else It.fHybrid[J]:=ThybridIteration2(@IFSNoop);
  end;
end;
procedure Run;
begin
  case Driver of
    0: doHybridPas(@It.C1);
    1: V:=doHybridPasDE(@It.C1);
    2: V:=doHybridIFS3D(@It.C1);
    3: V:=doHybridIFS3DnoVecIni(@It.C1);
    4: doHybrid4DPas(@It.C1);
    5: V:=doHybrid4DDEPas(@It.C1);
    6: do3DalternateOTrap(@It.C1);
    7: doInterpolHybridPas(@It.C1);
    8: V:=doInterpolHybridPasDE(@It.C1);
    9: doInterpolHybridPas4D(@It.C1);
    10: V:=doInterpolHybridPas4DDE(@It.C1);
  end;
end;
begin
  SetExceptionMask([exInvalidOp,exDenormalized,exZeroDivide,exOverflow,exUnderflow,exPrecision]);
  Set8087CW($133F);
  Check(SizeOf(Pointer)=4,'i386 required');
  Check(PtrUInt(@It.C1)-PtrUInt(@It)=56,'iteration ABI');
  for Driver:=0 to 6 do begin
    Setup; It.nHybrid[0]:=1; Run;
    Check(Calls=2,'healthy calls driver='+IntToStr(Driver));
    Setup; Failed:=False;
    try Run; except on E:EFormulaExecutionError do Failed:=Pos('empty_cycle',E.Message)>0; end;
    if Driver=0 then Check((not Failed) and (It.ItResultI=2),'finite original inside exit')
    else Check(Failed,'empty-cycle driver='+IntToStr(Driver));
    Setup; It.EndTo:=1;It.iRepeatFrom:=1;It.nHybrid[0]:=1;Failed:=False;
    try Run; except on E:EFormulaExecutionError do Failed:=Pos('empty_cycle',E.Message)>0; end;
    Check(Failed and (Calls=1),'repeat suffix excludes start driver='+IntToStr(Driver));
    Setup; It.nHybrid[0]:=LongInt($80000001);Failed:=False;
    try Run; except on E:EFormulaExecutionError do Failed:=Pos('no_progress_budget',E.Message)>0; end;
    Check(Failed and (Calls=FormulaCallsWithoutProgressLimit),'uncounted budget driver='+IntToStr(Driver));
    Setup; It.EndTo:=1;It.nHybrid[0]:=LongInt($800186A3);It.nHybrid[1]:=1;Run;
    Check(Calls=200008,'finite repeat100003 driver='+IntToStr(Driver));
    Setup; It.iStartFrom:=6; Failed:=False;
    try Run; except on E:EFormulaExecutionError do Failed:=Pos('invalid_slot_range',E.Message)>0; end;
    Check(Failed and (Calls=0),'invalid slot before access driver='+IntToStr(Driver));
    Setup; It.nHybrid[0]:=1;Run;
    Check(Calls=2,'healthy recovery driver='+IntToStr(Driver));
  end;
  for Driver:=7 to 10 do begin
    Setup; PSingle(@It.nHybrid[0])^:=0.5; PSingle(@It.nHybrid[1])^:=0.5; Run;
    Check(Calls=4,'healthy interpolation driver='+IntToStr(Driver));
    Setup; PSingle(@It.nHybrid[0])^:=0.5; PSingle(@It.nHybrid[1])^:=0.5;
    Mode:=1; Failed:=False;
    try Run; except on E:EFormulaExecutionError do Failed:=Pos('no_progress_budget',E.Message)>0; end;
    Check(Failed and (Calls=FormulaCallsWithoutProgressLimit),'reset-counter interpolation driver='+IntToStr(Driver));
    Setup; PSingle(@It.nHybrid[0])^:=0.5; PSingle(@It.nHybrid[1])^:=0.5; Run;
    Check(Calls=4,'healthy interpolation recovery driver='+IntToStr(Driver));
  end;
  WriteLn('PASS ',Checks);
end.
'''


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--work', type=Path, default=Path('C:/work/_c/mb3d_pascal_work/mb3d_work'))
    ap.add_argument('--output', type=Path, required=True)
    ap.add_argument('--fpc', type=Path, default=Path('C:/FPC/3.2.2/bin/i386-Win32/fpc.exe'))
    args = ap.parse_args()
    out = args.output.resolve()
    out.mkdir(parents=True, exist_ok=True)
    paths = [args.work / x for x in ('formulas.pas', 'Math3D.pas', 'TypeDefinitions.pas', 'FormulaExecutionGuard.pas', 'OTrapDEcalc.pas')]
    formulas, math, types, _, otrap = (p.read_text(encoding='latin1') for p in paths)
    types = types[types.index('  ThybridIteration2 ='):types.index('  ThybridIteration =')]
    text = '''program guard_orbit_probe;
{$mode delphi}{$asmmode intel}
uses Windows, SysUtils, Math, FormulaExecutionGuard;
type TVec3D=array[0..2] of Double; TPVec3D=^TVec3D;
     TVec4D=array[0..3] of Double; TPVec4D=^TVec4D;
     TSMatrix4=array[0..3,0..3] of Single; TPSMatrix4=^TSMatrix4;
'''+types+'''\nconst d65535: Double=65535;
function HeadlessDiagPixelActive: Boolean; begin Result:=False; end;
procedure FormulaDiagTrace(const S:string); begin end;
procedure CalcSmoothIterations(P:TPIteration3D; N:Integer); begin Halt(90); end;
'''
    text += procedure(math, 'mCopyVec') + '\n' + procedure(formulas, 'InitDispatchGuard') + '\n'
    for name in ('CopyVec4SSE2', 'Rotate4Dex', 'MinCD'):
        text += procedure(math, name) + '\n'
    text += 'var mCopyVec4: procedure(V1,V2:TPVec4D) = CopyVec4SSE2;\n'
    for name in ('doHybridPas', 'doHybridPasDE', 'doHybridIFS3D', 'doHybridIFS3DnoVecIni',
                 'doHybrid4DPas', 'doHybrid4DDEPas', 'doInterpolHybridPas', 'doInterpolHybridPasDE',
                 'doInterpolHybridPas4D', 'doInterpolHybridPas4DDE'):
        text += procedure(formulas, name) + '\n'
    text += procedure(otrap, 'CalcOTrapMode3D') + '\n' + procedure(otrap, 'do3DalternateOTrap') + '\n'
    text += HARNESS
    src = out / 'guard_orbit_probe.pas'
    src.write_text(text, encoding='latin1')
    cmd = [str(args.fpc), '-Mdelphi', '-CfSSE2', '-O2', '-al', '-Fu'+str(args.work), '-FU'+str(out), '-FE'+str(out), str(src)]
    build = subprocess.run(cmd, capture_output=True, text=True, timeout=60)
    (out/'build.log').write_text(build.stdout+build.stderr)
    build.check_returncode()
    exe = src.with_suffix('.exe')
    run = subprocess.run([str(exe)], capture_output=True, text=True, timeout=60)
    (out/'run.log').write_text(run.stdout+run.stderr)
    hashes = {str(p): hashlib.sha256(p.read_bytes()).hexdigest() for p in [*paths, Path(__file__), args.fpc, src, exe]}
    (out/'summary.json').write_text(json.dumps(dict(status='PASS' if run.returncode==0 else 'FAIL',returncode=run.returncode,command=cmd,stdout=run.stdout,stderr=run.stderr,sha256=hashes),indent=2))
    print(run.stdout+run.stderr)
    run.check_returncode()


if __name__ == '__main__':
    main()
