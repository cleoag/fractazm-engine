"""Measure the actual saved-register qword read by odd gnarly/ducks CODE.

Uses unchanged doHybridIFS3D assembly from immutable Pascal. A tail shim records
the real caller stack without substituting it. Separate optional seeded calls
demonstrate ABI dependence through that same original caller (not just a mock).
"""
import argparse
import json
import subprocess
from pathlib import Path
from build_original_hybrid_oracle import ROOT, HEADER, digest


HARNESS = '''
type TParams = array[0..1023] of Byte;
     PParams = ^TParams;
var CodePtr, ParamAllocation: Pointer;
    StackWords: array[0..1] of Cardinal;
    Params: PParams;
    It: TIteration3Dext;
    Stream: TFileStream;
    CX,CY,CZ,Distance: Double;
    SeedLo,SeedHi: Cardinal;

procedure Capture; assembler; nostackframe;
asm
  push eax
  mov eax, [esp+8]
  mov [StackWords], eax
  mov eax, [esp+12]
  mov [StackWords+4], eax
  pop eax
  jmp dword ptr [CodePtr]
end;

function SeededCall(PIt: TPIteration3D): Double; assembler; nostackframe;
asm
  push esi
  push edi
  mov esi, [SeedHi]
  mov edi, [SeedLo]
  call doHybridIFS3D
  pop edi
  pop esi
end;

procedure Number(V: Double);
begin Write(FloatToStrF(V,ffGeneral,17,0),' '); end;

begin
  if ParamCount <> 3 then Halt(2);
  if (SizeOf(Pointer) <> 4) or (PtrUInt(@It.C1)-PtrUInt(@It) <> 56) then Halt(3);
  DefaultFormatSettings.DecimalSeparator:='.';
  SetExceptionMask([exInvalidOp,exDenormalized,exZeroDivide,exOverflow,exUnderflow,exPrecision]);
  Set8087CW($133F);
  Stream:=TFileStream.Create(ParamStr(1),fmOpenRead);
  CodePtr:=VirtualAlloc(nil,Stream.Size,MEM_COMMIT or MEM_RESERVE,PAGE_EXECUTE_READWRITE);
  if CodePtr=nil then Halt(4);
  Stream.ReadBuffer(CodePtr^,Stream.Size);Stream.Free;
  Stream:=TFileStream.Create(ParamStr(2),fmOpenRead);
  if Stream.Size<>SizeOf(TParams) then Halt(5);
  GetMem(ParamAllocation,SizeOf(TParams)+15);
  Params:=PParams((PtrUInt(ParamAllocation)+15) and not PtrUInt(15));
  Stream.ReadBuffer(Params^,SizeOf(TParams));Stream.Free;
  while not EOF(Input) do begin
    ReadLn(CX,CY,CZ,SeedLo,SeedHi);
    FillChar(It,SizeOf(It),0);FillChar(StackWords,SizeOf(StackWords),0);
    It.C1:=CX;It.C2:=CY;It.C3:=CZ;
    It.maxIt:=1;It.RStopD:=0.001;It.nHybrid[0]:=1;
    It.fHPVar[0]:=@Params^[512];
    if ParamStr(3)='plain' then It.fHybrid[0]:=ThybridIteration2(CodePtr)
    else if (ParamStr(3)='trace') or (ParamStr(3)='seeded') then It.fHybrid[0]:=ThybridIteration2(@Capture)
    else Halt(6);
    if ParamStr(3)='seeded' then Distance:=SeededCall(TPIteration3D(@It.C1))
    else Distance:=doHybridIFS3D(TPIteration3D(@It.C1));
    Number(Distance);Number(It.x);Number(It.y);Number(It.z);
    Number(It.Rout);Number(It.OTrap);Number(It.VaryScale);Number(It.Dfree1);Number(It.Dfree2);
    WriteLn(It.ItResultI,' ',StackWords[0],' ',StackWords[1]);
  end;
  VirtualFree(CodePtr,0,MEM_RELEASE);
  FreeMem(ParamAllocation);
end.
'''


def main():
    ap=argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--output',type=Path,required=True)
    a=ap.parse_args()
    a.output.mkdir(parents=True,exist_ok=False)
    work=Path('C:/work/_c/mb3d_pascal_work/mb3d_work/ifs_caller_oracle')
    work.mkdir(parents=True,exist_ok=True)
    types=(ROOT/'mb3d/TypeDefinitions.pas').read_text()
    types=types[types.index('  ThybridIteration2 ='):types.index('  ThybridIteration =')]
    formulas=(ROOT/'mb3d/formulas.pas').read_text()
    start=formulas.index('function doHybridIFS3D(',formulas.index('implementation'))
    proc=formulas[start:formulas.index('\nend;',start)+5]
    pas=work/'original_ifs_caller_oracle.pas'
    pas.write_text(HEADER.replace('original_hybrid_oracle','original_ifs_caller_oracle')+types+'\nconst d65535: Double = 65535;\n'+proc+HARNESS)
    fpc=Path('C:/FPC/3.2.2/bin/i386-Win32/fpc.exe')
    cmd=[str(fpc),'-Mdelphi','-CfSSE2','-O2','-al',str(pas)]
    run=subprocess.run(cmd,cwd=work,capture_output=True,text=True,timeout=90)
    (a.output/'build.log').write_text(run.stdout+run.stderr);run.check_returncode()
    for p in (pas,pas.with_suffix('.exe'),pas.with_suffix('.s')):
        (a.output/p.name).write_bytes(p.read_bytes())
    paths=[fpc,fpc.with_name('ppc386.exe'),pas,pas.with_suffix('.exe'),Path(__file__),
           Path(__file__).with_name('build_original_hybrid_oracle.py'),ROOT/'mb3d/formulas.pas',ROOT/'mb3d/TypeDefinitions.pas']
    (a.output/'identity.json').write_text(json.dumps(dict(command=cmd,sha256={str(p):digest(p) for p in paths}),indent=2))


if __name__=='__main__':main()
