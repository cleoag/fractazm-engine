"""Qualify the edited Pascal loader's exact-CODE guards without building the GUI.

The generated i386 harness and all compiler products stay in the editable Pascal
tree. Original and guarded CODE run through identical ABIs. No unsafe original
case is executed; rejected cases must recover and match a fresh healthy call.
"""
import argparse
import hashlib
import json
import random
import struct
import subprocess
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
NAMES = ('TrifoxComplexAngles', '_e_3Dtess', '_IFS_tess', 'Knots2IFS')
WORK = Path('C:/work/_c/mb3d_pascal_work/mb3d_work')
FPC = Path('C:/FPC/3.2.2/bin/i386-Win32/fpc.exe')
HARNESS = r'''program code_guard_qualification;
{$mode delphi}{$asmmode intel}
uses Windows, SysUtils, Classes, Math, KnownFormulaCodeGuard, FormulaExecutionGuard;
type TBytes = array[0..1023] of Byte;
var Params, State, BaseParams: TBytes;
    CodePtr, ClonePtr, ItPtr, VarPtr: Pointer;
    S: TFileStream;
    N,I: Integer;
    Guarded, DIFS: Boolean;
    X,Y,Z: Double;
    Name: string;
    BeforeCW, AfterCW: Word;

procedure InvokeStandard; assembler;
asm
  push ebx
  push esi
  push edi
  mov eax, [ItPtr]
  sub eax, 8
  push eax
  mov eax, [ItPtr]
  push eax
  sub eax, 32
  mov edx, [ItPtr]
  sub edx, 24
  mov ecx, [ItPtr]
  sub ecx, 16
  call dword ptr [CodePtr]
  pop edi
  pop esi
  pop ebx
end;

procedure InvokeDIFS; assembler;
asm
  push ebx
  push esi
  push edi
  mov esi, [ItPtr]
  add esi, 88
  mov edi, [VarPtr]
  xor ebx, ebx
  mov ecx, 1
  call dword ptr [CodePtr]
  pop edi
  pop esi
  pop ebx
end;

procedure InitState;
begin
  FillChar(State,SizeOf(State),0);
  PDouble(PtrUInt(ItPtr)-32)^:=X;
  PDouble(PtrUInt(ItPtr)-24)^:=Y;
  PDouble(PtrUInt(ItPtr)-16)^:=Z;
  PDouble(PtrUInt(ItPtr)-8)^:=0.375;
  PPointer(PtrUInt(ItPtr)+48)^:=VarPtr;
  PDouble(PtrUInt(ItPtr)+56)^:=X*X+Y*Y+Z*Z;
  PDouble(PtrUInt(ItPtr)+200)^:=1;
  PDouble(PtrUInt(ItPtr)+232)^:=1;
end;

procedure EmitState(const Prefix: string);
var J: Integer;
begin
  { Exclude the ASLR-dependent pointer; include all other formula-visible bytes. }
  PPointer(PtrUInt(ItPtr)+48)^:=nil;
  Write(Prefix,' ');
  for J:=0 to High(State) do Write(IntToHex(State[J],2));
  WriteLn;
end;

procedure HealthyParameters;
begin
  if Name='TrifoxComplexAngles' then begin
    PLongInt(PtrUInt(VarPtr)-12)^:=2;
    PLongInt(PtrUInt(VarPtr)-28)^:=2;
  end;
  if Name='_e_3Dtess' then PLongInt(PtrUInt(VarPtr)-64)^:=5;
  if Name='_IFS_tess' then PLongInt(PtrUInt(VarPtr)-68)^:=10;
  if Name='Knots2IFS' then PLongInt(PtrUInt(VarPtr)-100)^:=3;
end;

begin
  if ParamCount<>4 then Halt(2);
  SetErrorMode(SEM_FAILCRITICALERRORS or SEM_NOGPFAULTERRORBOX);
  DefaultFormatSettings.DecimalSeparator:='.';
  SetExceptionMask([exInvalidOp,exDenormalized,exZeroDivide,exOverflow,exUnderflow,exPrecision]);
  Set8087CW($133F);
  Name:=ParamStr(3); Guarded:=ParamStr(4)<>'original'; DIFS:=Name='Knots2IFS';
  ItPtr:=@State[256]; VarPtr:=@Params[512];
  S:=TFileStream.Create(ParamStr(1),fmOpenRead); N:=S.Size;
  CodePtr:=VirtualAlloc(nil,4096,MEM_COMMIT or MEM_RESERVE,PAGE_EXECUTE_READWRITE);
  if (N>3968) or (CodePtr=nil) then Halt(3);
  S.ReadBuffer(CodePtr^,N); S.Free;
  try
    if ParamStr(4)='identity' then begin
      WriteLn('INSTALLED ',Ord(InstallKnownFormulaCodeGuard(CodePtr,N,4096,Name)));
      Halt(0);
    end;
    if Guarded then InstallKnownFormulaCodeGuard(CodePtr,N,4096,Name);
  except
    on E: EFormulaExecutionError do begin WriteLn('INSTALL_REJECT ',E.Message); Halt(12); end;
  end;
  if ParamStr(4)='clone' then begin
    ClonePtr:=VirtualAlloc(nil,4096,MEM_COMMIT or MEM_RESERVE,PAGE_EXECUTE_READWRITE);
    Move(CodePtr^,ClonePtr^,4096); VirtualFree(CodePtr,0,MEM_RELEASE); CodePtr:=ClonePtr;
    FlushInstructionCache(GetCurrentProcess,CodePtr,4096);
  end;
  S:=TFileStream.Create(ParamStr(2),fmOpenRead);
  if S.Size<>SizeOf(Params) then Halt(4);
  S.ReadBuffer(BaseParams,SizeOf(BaseParams)); S.Free;
  while not EOF(Input) do begin
    ReadLn(X,Y,Z); Params:=BaseParams; InitState;
    asm fnstcw BeforeCW end;
    try
      if DIFS then InvokeDIFS else InvokeStandard;
      EmitState('OK');
    except
      on E: EFormulaExecutionError do begin
        asm fnstcw AfterCW end;
        if BeforeCW<>AfterCW then Halt(10);
        WriteLn('REJECT ',E.Message,' cw=',AfterCW);
        HealthyParameters; InitState;
        if DIFS then InvokeDIFS else InvokeStandard;
        EmitState('RECOVERY');
      end;
    end;
    asm fnstcw AfterCW end;
    if BeforeCW<>AfterCW then Halt(11);
  end;
  VirtualFree(CodePtr,0,MEM_RELEASE);
end.
'''


def digest(p):
    return hashlib.sha256(p.read_bytes()).hexdigest()


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--work', type=Path, default=WORK)
    ap.add_argument('--output', type=Path, required=True)
    a = ap.parse_args()
    a.output.mkdir(parents=True, exist_ok=True)
    generated = a.work / '_code_guard_qualification'
    generated.mkdir(exist_ok=True)
    pas = generated / 'code_guard_qualification.pas'
    pas.write_text(HARNESS)
    command = [str(FPC), '-B', '-Mdelphi', '-CfSSE2', '-O2', '-Fu'+str(a.work),
               '-FU'+str(generated), '-FE'+str(generated), str(pas)]
    built = subprocess.run(command, capture_output=True, text=True, timeout=45)
    (a.output/'build.log').write_text(built.stdout+built.stderr)
    if built.returncode:
        raise RuntimeError(built.stdout+built.stderr)
    exe = generated/'code_guard_qualification.exe'
    report = {'profile': 'modified Pascal exact-CODE guard, actual body budget 1048576',
              'inputs': [], 'results': [], 'build_command': command,
              'binary_sha256': digest(exe), 'scope_excludes': ['Monte Carlo qualification', 'JIT']}
    for p in (pas, a.work/'KnownFormulaCodeGuard.pas', a.work/'KnownFormulaCodeBytes.inc',
              a.work/'FormulaExecutionGuard.pas', a.work/'CustomFormulas.pas'):
        report['inputs'].append({'path': str(p), 'sha256': digest(p)})
    params = {}
    for name in NAMES:
        asset = ROOT/'mb3d/M3Formulas'/f'{name}.m3f'
        code = bytes.fromhex(asset.read_text().split('[CODE]')[1].split('[END]')[0])
        (a.output/f'{name}.bin').write_bytes(code)
        found = list((ROOT/'comparison/runs/stage-a-20261018/final-external-points-v20').glob(f'generic-*/challenge_{name}_early.params'))
        assert len(found)==1, (name,found)
        params[name] = found[0].read_bytes()
        report['inputs'].append({'path':str(asset),'sha256':digest(asset),'code_sha256':hashlib.sha256(code).hexdigest(),
                                 'prepared_path':str(found[0]),'prepared_sha256':digest(found[0])})

    def execute(case, name, data, points, mode):
        param = a.output/f'{case}.params'; param.write_bytes(data)
        cmd = [str(exe),str((a.output/f'{name}.bin').resolve()),str(param.resolve()),name,mode]
        r = subprocess.run(cmd,input=''.join(' '.join(map(str,p))+'\n' for p in points),
                           capture_output=True,text=True,timeout=15)
        (a.output/f'{case}-{mode}.log').write_text(r.stdout+r.stderr)
        if r.returncode: raise RuntimeError((case,mode,r.returncode,r.stdout,r.stderr))
        return r.stdout.splitlines()

    def compare(case,name,data,points):
        before = execute(case,name,data,points,'original')
        after = execute(case,name,data,points,'guarded')
        cloned = execute(case,name,data,points,'clone')
        assert before==after==cloned, case
        report['results'].append({'case':case,'kind':'exact original/guarded/copied CODE state',
                                  'rows':len(points),'status':'pass'})

    rng = random.Random(1048576)
    points = [(0.,0.,0.),(2.,2.,0.),(-.5,.25,1.)]+[tuple(rng.uniform(-4,4) for _ in range(3)) for _ in range(29)]
    for name in NAMES:
        compare('ordinary-'+name,name,params[name],points)
    for name,offset in [('TrifoxComplexAngles',-12),('TrifoxComplexAngles',-28),
                        ('_e_3Dtess',-64),('_IFS_tess',-68),('Knots2IFS',-100)]:
        for value in (-2147483648,-1,1,257,513):
            data = bytearray(params[name]); struct.pack_into('<i',data,512+offset,value)
            # Unsigned IFS depth can be huge; the origin provably exits before its loop.
            compare(f'boundary-{name}-{offset}-{value}',name,data,[(0.,0.,0.)] if name=='_IFS_tess' else points[:3])
    for name,offset in [('_e_3Dtess',-64),('_IFS_tess',-68)]:
        data = bytearray(params[name]); struct.pack_into('<i',data,512+offset,2147483647)
        compare('huge-early-exit-'+name,name,data,[(0.,0.,0.)])

    for name in NAMES:
        codepath=a.output/f'{name}.bin'
        original=codepath.read_bytes()
        mutant=a.output/f'{name}-unknown.bin'
        changed=bytearray(original);changed[-1]^=1;mutant.write_bytes(changed)
        cmd=[str(exe),str(mutant.resolve()),'unused',name,'identity']
        r=subprocess.run(cmd,capture_output=True,text=True,timeout=5)
        assert r.returncode==12 and 'unrecognized_code_variant' in r.stdout,(name,r)
        r=subprocess.run([str(exe),str(codepath.resolve()),'unused','renamed','identity'],capture_output=True,text=True,timeout=5)
        assert r.returncode==0 and r.stdout.strip()=='INSTALLED 1',(name,r)
        r=subprocess.run([str(exe),str(mutant.resolve()),'unused','unrelated','identity'],capture_output=True,text=True,timeout=5)
        assert r.returncode==0 and r.stdout.strip()=='INSTALLED 0',(name,r)
        assert codepath.read_bytes()==original
        report['results'].append({'case':'identity-'+name,'kind':'full CODE identity, renamed exact accepted, unknown named variant rejected, unrelated unmodified','status':'pass'})

    rejected = []
    for offset in (-12,-28):
        data=bytearray(params['TrifoxComplexAngles']);struct.pack_into('<i',data,512+offset,0)
        rejected.append((f'zero-power-{offset}','TrifoxComplexAngles',data,(.25,.5,.75)))
    data=bytearray(params['Knots2IFS']);struct.pack_into('<i',data,412,1048577)
    rejected.append(('knots-budget','Knots2IFS',data,(.25,.5,.75)))
    edge=bytearray(data);struct.pack_into('<i',edge,412,1048576)
    compare('knots-budget-last-allowed','Knots2IFS',edge,[(.25,.5,.75)])
    data=bytearray(params['_IFS_tess']);struct.pack_into('<i',data,444,-1)
    struct.pack_into('<d',data,512-48,0.) # dispersion zero
    struct.pack_into('<d',data,512-64,0.) # rotation zero: one transverse coordinate stays zero
    rejected.append(('ifs-budget','_IFS_tess',data,(0.,2.,0.)))
    edge=bytearray(data);struct.pack_into('<i',edge,444,1048575)
    compare('ifs-budget-last-allowed','_IFS_tess',edge,[(0.,2.,0.)])
    data=bytearray(params['_e_3Dtess']);struct.pack_into('<i',data,448,2147483647)
    struct.pack_into('<d',data,512+32,0.) # direct prepared-constant stress, not a file option
    rejected.append(('e3-prepared-constant-budget','_e_3Dtess',data,(0.,0.,0.)))
    edge=bytearray(data);struct.pack_into('<i',edge,448,1048576)
    compare('e3-prepared-constant-last-allowed','_e_3Dtess',edge,[(0.,0.,0.)])
    for case,name,data,point in rejected:
        for mode in ('guarded','clone'):
            lines=execute(case,name,data,[point],mode)
            assert len(lines)==2 and lines[0].startswith('REJECT ') and lines[1].startswith('RECOVERY '), (case,lines)
            healthy=bytearray(data)
            for offset,value in {'TrifoxComplexAngles':[(-12,2),(-28,2)],'_e_3Dtess':[(-64,5)],
                                 '_IFS_tess':[(-68,10)],'Knots2IFS':[(-100,3)]}[name]:
                struct.pack_into('<i',healthy,512+offset,value)
            fresh=execute(case+'-fresh',name,healthy,[point],'original')
            assert lines[1][9:]==fresh[0][3:],case
        report['results'].append({'case':case,'kind':'bounded rejection + exact healthy recovery + CW + copied CODE',
                                  'status':'pass','qualification':'prepared constant stress only' if case.startswith('e3-') else 'saved option input'})

    report['status']='pass'
    report['counts']={'cases':len(report['results']),'failed':0}
    (a.output/'summary.json').write_text(json.dumps(report,indent=2)+'\n')
    print(json.dumps(report['counts']))


if __name__=='__main__': main()
