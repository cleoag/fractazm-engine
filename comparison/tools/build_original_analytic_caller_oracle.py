"""Build a NEW bounded FPC analytic caller diagnostic outside immutable mb3d.

The original doHybridPasDE and mCopyVec bodies are extracted unchanged. The
harness runs original M3F CODE, capturing actual callback/return SSE lanes.
This does not recreate historical executables or qualify x87 arithmetic.
"""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess
from build_original_hybrid_oracle import HEADER, STUB, HARNESS, ROOT


def source():
    types=(ROOT/'mb3d/TypeDefinitions.pas').read_text()
    types=types[types.index('  ThybridIteration2 ='):types.index('  ThybridIteration =')]
    math=(ROOT/'mb3d/Math3D.pas').read_text()
    start=math.index('procedure mCopyVec(',math.index('implementation'))
    copy=math[start:math.index('\nend;',start)+5]
    formulas=(ROOT/'mb3d/formulas.pas').read_text()
    start=formulas.index('function doHybridPasDE(',formulas.index('implementation'))
    hybrid=formulas[start:formulas.index('\nend;',start)+5]
    harness=HARNESS
    substitutions=[
        ('    Calls: LongInt;', '    Calls: LongInt;\n    OutLanes: array[0..3] of Double;\n    Distance, Scale: Double;\n    Count: Integer;'),
        ('procedure Number(V: Double);', '''procedure CallAndCapture(P: TPIteration3D); assembler; nostackframe;
asm
  call doHybridPasDE
  movupd [OutLanes], xmm0
  movupd [OutLanes+16], xmm1
  fstp qword ptr [Distance]
end;

procedure Number(V: Double);'''),
        ('ReadLn(CX,CY,CZ,JX,JY,JZ,JuliaMode,Limit,StopValue);',
         'ReadLn(CX,CY,CZ,JX,JY,JZ,JuliaMode,Limit,StopValue,Count,Scale);'),
        ('(Limit > 64)', '(Limit > 8) or (Count < 1) or (Count > 3)'),
        ('It.VaryScale:=1; It.Deriv1:=1;',
         'It.VaryScale:=1; It.Deriv1:=3.5; It.Deriv2:=-2; It.Deriv3:=7; It.DEoption:=2;'),
        ('It.nHybrid[0]:=1; It.fHPVar[0]:=@Params[512];',
         'It.nHybrid[0]:=Count; It.fHPVar[0]:=@Params[512]; PDouble(@Params[496])^:=Scale;'),
        ('    doHybridPas(TPIteration3D(@It.C1));',
         "    CallAndCapture(TPIteration3D(@It.C1));\n    Write('RETURN ');\n    for I:=0 to 3 do Write(IntToHex(PQWord(@OutLanes[I])^,16),' ');\n    Write('STATE ');"),
        ('Number(It.Deriv1); Number(It.Deriv2);',
         'Number(It.Deriv1); Number(It.Deriv2); Number(It.Deriv3); Number(Distance);'),
        ("Write(It.ItResultI,' ',It.bFirstIt,' ',Calls,' ');",
         "Write(It.ItResultI,' ',It.bFirstIt,' ',It.bTmp,' ',Calls,' ');"),
        ('for I:=0 to Calls-1 do for K:=0 to 3 do Number(Lanes[I,K]);',
         "for I:=0 to Calls-1 do for K:=0 to 3 do Write(IntToHex(PQWord(@Lanes[I,K])^,16),' ');")]
    for a,b in substitutions:
        if harness.count(a)!=1:raise ValueError('original harness changed: '+a)
        harness=harness.replace(a,b)
    return HEADER.replace('original_hybrid_oracle','original_analytic_oracle')+types+'\n'+copy+STUB+hybrid+harness


def main():
    ap=argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--work',type=Path,required=True)
    ap.add_argument('--output',type=Path,required=True)
    ap.add_argument('--fpc',type=Path,required=True)
    a=ap.parse_args()
    editable=Path('C:/work/_c/mb3d_pascal_work/mb3d_work').resolve()
    if not a.work.resolve().is_relative_to(editable) or a.work.exists() or a.output.exists():
        ap.error('fresh external Pascal work and new output directories required')
    a.work.mkdir(parents=True);a.output.mkdir(parents=True)
    pas=a.work/'original_analytic_oracle.pas';pas.write_text(source())
    command=[str(a.fpc),'-Mdelphi','-CfSSE2','-O2','-al',str(pas.resolve())]
    run=subprocess.run(command,cwd=a.work,capture_output=True,text=True,timeout=90)
    (a.output/'build.log').write_text(run.stdout+run.stderr)
    run.check_returncode()
    products=[pas.with_suffix(ext) for ext in ('.pas','.exe','.s','.o')]
    for p in products:(a.output/p.name).write_bytes(p.read_bytes())
    paths=[a.fpc,*products,Path(__file__),Path(__file__).with_name('build_original_hybrid_oracle.py'),
           *(ROOT/'mb3d'/p for p in ('formulas.pas','TypeDefinitions.pas','Math3D.pas'))]
    receipt={'profile':'NEW FPC i386 SSE2 analytic caller, not historical/x87 qualification',
             'command':command,'sha256':{str(p):hashlib.sha256(p.read_bytes()).hexdigest() for p in paths}}
    (a.output/'identity.json').write_text(json.dumps(receipt,indent=2)+'\n')
    print(pas.with_suffix('.exe'))


if __name__=='__main__':main()
