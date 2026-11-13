"""Extract unchanged original two-slot dispatchers and their real SSE2 copy helper.

The observation shims use integer registers only and tail-jump into original CODE.
Plain/trace/repeat validation belongs to check_manowar_hybrids.py.
"""
import argparse,hashlib,json,subprocess,shutil
from pathlib import Path
ROOT=Path(__file__).resolve().parents[2]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def proc(text,name):
    prefix='function 'if name.endswith('DE')else'procedure '
    a=text.index(prefix+name+'(',text.index('implementation'))
    return text[a:text.index('\nend;',a)+5]
def source(formulas_path=None):
    types=(ROOT/'mb3d/TypeDefinitions.pas').read_text()
    types=types[types.index('  ThybridIteration2 ='):types.index('  ThybridIteration =')]
    math=(ROOT/'mb3d/Math3D.pas').read_text();formulas=(formulas_path or ROOT/'mb3d/formulas.pas').read_text()
    guarded='procedure InitDispatchGuard('in formulas
    result='''program manowar_hybrid_oracle;
{$mode delphi}{$asmmode intel}
uses Windows,SysUtils,Classes,Math;
type TVec3D=array[0..2]of Double;TPVec3D=^TVec3D;
     TVec4D=array[0..3]of Double;TPVec4D=^TVec4D;
     TSMatrix4=array[0..3,0..3]of Single;TPSMatrix4=^TSMatrix4;
'''+types+'\n'+proc(math,'mCopyVec')+'\n'+proc(math,'CopyVec4SSE2')+'\n'+proc(math,'Rotate4Dex')+'''
type T2Vproc4=procedure(V1,V2:TPVec4D);
var mCopyVec4:T2Vproc4=CopyVec4SSE2;
procedure CalcSmoothIterations(P:TPIteration3D;n:Integer);begin Halt(90);end;
'''+(proc(formulas,'InitDispatchGuard')if guarded else'')+ '\n'.join(proc(formulas,n)for n in ('doHybridPas','doInterpolHybridPas','doInterpolHybridPasDE','doInterpolHybridPas4D','doInterpolHybridPas4DDE'))+'''
var Codes:array[0..1]of Pointer;Params:array[0..1,0..1023]of Byte;
    Calls:LongInt;Lanes:array[0..127,0..3]of Double;SlotIds:array[0..127]of LongInt;
    It:TIteration3Dext;F:TFileStream;I,K,JM,Limit,Driver,StartSlot,RepeatSlot,EndSlot:Integer;
    Counts:array[0..1]of LongInt;CX,CY,CZ,JX,JY,JZ,StopValue,DE:Double;
'''
    for n in range(2):
        result+=f'''procedure Capture{n}(var x,y,z,w:Double;P:Pointer);assembler;nostackframe;
asm
 push ebx
 mov ebx,[Calls]
 cmp ebx,128
 jae @skip
 mov dword ptr [SlotIds+ebx*4],{n}
 shl ebx,5
 movupd [Lanes+ebx],xmm0
 movupd [Lanes+ebx+16],xmm1
@skip:
 inc dword ptr [Calls]
 pop ebx
 jmp dword ptr [Codes+{n*4}]
end;
'''
    result+='''procedure Number(V:Double);begin Write(FloatToStrF(V,ffGeneral,17,0),' ');end;
begin
 if ParamCount<>12 then Halt(2);
 if (SizeOf(Pointer)<>4)or(PtrUInt(@It.C1)-PtrUInt(@It)<>56)then Halt(3);
 DefaultFormatSettings.DecimalSeparator:='.';
 SetExceptionMask([exInvalidOp,exDenormalized,exZeroDivide,exOverflow,exUnderflow,exPrecision]);Set8087CW($133F);
 Driver:=StrToInt(ParamStr(1));StartSlot:=StrToInt(ParamStr(2));RepeatSlot:=StrToInt(ParamStr(3));EndSlot:=StrToInt(ParamStr(4));
 Counts[0]:=StrToInt(ParamStr(5));Counts[1]:=StrToInt(ParamStr(6));
 for I:=0 to 1 do begin
  F:=TFileStream.Create(ParamStr(7+I*2),fmOpenRead);
  Codes[I]:=VirtualAlloc(nil,F.Size,MEM_COMMIT or MEM_RESERVE,PAGE_EXECUTE_READWRITE);
  if Codes[I]=nil then Halt(4);F.ReadBuffer(Codes[I]^,F.Size);F.Free;
  F:=TFileStream.Create(ParamStr(8+I*2),fmOpenRead);if F.Size<>1024 then Halt(5);
  F.ReadBuffer(Params[I],1024);F.Free;
 end;
 while not EOF(Input)do begin
  ReadLn(CX,CY,CZ,JX,JY,JZ,JM,Limit,StopValue);if(Limit<1)or(Limit>32)then Halt(6);
  FillChar(It,SizeOf(It),0);FillChar(Lanes,SizeOf(Lanes),0);Calls:=0;
  It.C1:=CX;It.C2:=CY;It.C3:=CZ;It.Ju1:=JX;It.Ju2:=JY;It.Ju3:=JZ;
  It.DoJulia:=JM<>0;It.maxIt:=Limit;It.RStop:=StopValue;It.VaryScale:=1;It.Deriv1:=1;It.DEoption:=StrToInt(ParamStr(12));
  for I:=0 to 3 do It.SMatrix4[I,I]:=1;
  It.iStartFrom:=StartSlot;It.iRepeatFrom:=RepeatSlot;It.EndTo:=EndSlot;
  for I:=0 to 1 do begin It.nHybrid[I]:=Counts[I];It.fHPVar[I]:=@Params[I,512];It.fHybrid[I]:=ThybridIteration2(Codes[I]);end;
  if ParamStr(11)='trace'then begin It.fHybrid[0]:=Capture0;It.fHybrid[1]:=Capture1;end
  else if ParamStr(11)<>'plain'then Halt(7);
  DE:=0;
  case Driver of
   0:doHybridPas(TPIteration3D(@It.C1));
   1:doInterpolHybridPas(TPIteration3D(@It.C1));
   2:DE:=doInterpolHybridPasDE(TPIteration3D(@It.C1));
   3:doInterpolHybridPas4D(TPIteration3D(@It.C1));
   4:DE:=doInterpolHybridPas4DDE(TPIteration3D(@It.C1));
   else Halt(8);
  end;
  Number(It.x);Number(It.y);Number(It.z);Number(It.w);Number(It.Rout);Number(It.Rold);Number(It.OTrap);
  Number(It.Dfree1);Number(It.Dfree2);Number(It.Deriv1);Number(It.Deriv2);Number(DE);Write(It.ItResultI,' ',It.bFirstIt,' ',Calls,' ');
  for I:=0 to Calls-1 do begin Write(SlotIds[I],' ');for K:=0 to 3 do Number(Lanes[I,K]);end;WriteLn;
 end;
 for I:=0 to 1 do VirtualFree(Codes[I],0,MEM_RELEASE);
end.
'''
    return result.replace('uses Windows,SysUtils,Classes,Math;', 'uses Windows,SysUtils,Classes,Math,FormulaExecutionGuard;')if guarded else result
def main():
    ap=argparse.ArgumentParser(description=__doc__);ap.add_argument('--output',type=Path,required=True)
    ap.add_argument('--work',type=Path,default=Path('C:/work/_c/mb3d_pascal_work/mb3d_work/_reference_gaps_manowar'))
    ap.add_argument('--guarded-source',type=Path)
    a=ap.parse_args();base=Path('C:/work/_c/mb3d_pascal_work/mb3d_work/_reference_gaps_manowar').resolve()
    if not a.work.resolve().is_relative_to(base):ap.error('isolated Manowar build directory required')
    a.work.mkdir(parents=True,exist_ok=True);a.output.mkdir(parents=True,exist_ok=False)
    (a.work/'obj').mkdir(exist_ok=True);(a.work/'bin').mkdir(exist_ok=True)
    pas=a.work/'manowar_hybrid_oracle.pas';pas.write_text(source(a.guarded_source))
    if a.guarded_source:shutil.copy2(a.guarded_source.parent/'FormulaExecutionGuard.pas',a.work/'FormulaExecutionGuard.pas')
    fpc=Path('C:/FPC/3.2.2/bin/i386-Win32/fpc.exe')
    command=[str(fpc),'-Mdelphi','-CfSSE2','-O2','-al','-FU'+str(a.work/'obj'),'-FE'+str(a.work/'bin'),str(pas)]
    r=subprocess.run(command,cwd=a.work,capture_output=True,timeout=90,creationflags=subprocess.CREATE_NO_WINDOW)
    (a.output/'build.log').write_bytes(r.stdout+r.stderr)
    (a.output/'command.json').write_text(json.dumps(dict(argv=command,exit_code=r.returncode),indent=2));r.check_returncode()
    for p in [pas,a.work/'bin/manowar_hybrid_oracle.exe',*a.work.glob('obj/*.s')]: (a.output/p.name).write_bytes(p.read_bytes())
    paths=[fpc,fpc.with_name('ppc386.exe'),pas,a.work/'bin/manowar_hybrid_oracle.exe',Path(__file__),*(ROOT/'mb3d'/n for n in ('formulas.pas','Math3D.pas','TypeDefinitions.pas'))]
    if a.guarded_source:paths.extend([a.guarded_source,a.work/'FormulaExecutionGuard.pas'])
    (a.output/'identity.json').write_text(json.dumps({'scope':__doc__,'sha256':{str(p):sha(p)for p in paths}},indent=2))
if __name__=='__main__':main()
