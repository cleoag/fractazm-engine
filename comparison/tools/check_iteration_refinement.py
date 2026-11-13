"""Check original Pascal scalar refinement using explicit, reproducible input bits.

The callback supplies SmoothItD independently of XYZ. Vector movement is stubbed;
this checks depth, control flow and restored limits, not spatial sampling.
"""
import argparse
import json
import subprocess
from pathlib import Path

from external_validation import ROOT, digest, write_json


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--cpp', type=Path, required=True)
    ap.add_argument('--inputs', type=Path, required=True,
                    help='Rows: steps maxIt depthBits count followed by Single sample bits')
    ap.add_argument('--output', type=Path, required=True)
    ap.add_argument('--pascal-work-root', type=Path, required=True)
    ap.add_argument('--fpc', default='C:/FPC/3.2.2/bin/i386-Win32/fpc.exe')
    a = ap.parse_args()
    immutable = (ROOT/'mb3d').resolve()
    work = a.pascal_work_root.resolve()
    if work == immutable or immutable in work.parents:
        raise ValueError('Immutable Pascal tree')
    rows = [list(map(int, line.split())) for line in a.inputs.read_text().splitlines()]
    if not rows:
        raise ValueError('Empty input suite')
    for row in rows:
        if len(row) < 5 or not 1 <= row[3] <= 1000 or len(row) != row[3]+4:
            raise ValueError('Invalid sample count')
        if not 0 <= row[0] <= 1000 or not 1 <= row[1] <= 1000000:
            raise ValueError('Invalid refinement limits')
        if not 0 <= row[2] < 2**64 or any(not 0 <= x < 2**32 for x in row[4:]):
            raise ValueError('Invalid floating point bit pattern')
    out = a.output.resolve()
    out.mkdir(parents=True, exist_ok=False)
    original = immutable/'Calc.pas'
    text = original.read_text(encoding='latin1')
    start = text.rindex('procedure RMdoBinSearchIt(')
    snippet = text[start:text.index('\nprocedure RMCalculateStartPos', start)]
    source = '''program iteration_refinement_generated;
{$mode delphi}
uses Math;
type TVec3D=array[0..2] of Double;TPVec3D=^TVec3D;
TIteration3Dext=record C1,C2,C3:Double;maxIt,DEoption:Integer;SmoothItD:Single;CalcSIT:Boolean;end;
TPIteration3Dext=^TIteration3Dext;
TMCTparameter=record pIt3Dext:TPIteration3Dext;iMaxIt,iMaxItF2,iDEAddSteps,DEoption2:Integer;
mVgradsFOV:TVec3D;CalcDE:function(it:TPIteration3Dext;p:Pointer):Double;end;
PMCTparameter=^TMCTparameter;
const s05:Single=0.5;
var sequence:array[0..999] of Single;index,count:Integer;
procedure mAddVecWeight(a,b:TPVec3D;w:Double);begin end;
function Controlled(it:TPIteration3Dext;p:Pointer):Double;
begin if index<count then it.SmoothItD:=sequence[index] else it.SmoothItD:=sequence[count-1];Inc(index);Result:=0;end;
''' + snippet + '''
var p:TMCTparameter;e:TIteration3Dext;z:QWord;v:Cardinal;depth:Double;steps,maxIts,i:Integer;
begin
SetExceptionMask([exInvalidOp,exDenormalized,exZeroDivide,exOverflow,exUnderflow,exPrecision]);Set8087CW($137f);
while not EOF(Input) do begin
Read(steps,maxIts,z,count);for i:=0 to count-1 do begin Read(v);Move(v,sequence[i],4);end;ReadLn;
FillChar(p,SizeOf(p),0);FillChar(e,SizeOf(e),0);index:=0;
p.pIt3Dext:=@e;p.CalcDE:=Controlled;p.iMaxIt:=maxIts;p.iMaxItF2:=maxIts;p.iDEAddSteps:=steps;e.maxIt:=maxIts;Move(z,depth,8);
RMdoBinSearchIt(@p,depth);WriteLn(PQWord(@depth)^,' ',PCardinal(@e.SmoothItD)^,' ',index,' ',e.maxIt,' ',p.iMaxIt,' ',p.iMaxItF2);
end;end.
'''
    generated = work/'iteration_refinement_generated.pas'
    generated.write_text(source)
    (out/generated.name).write_text(source)
    command = [a.fpc, '-Mdelphi', '-O2', '-CfSSE2', '-al', '-FE'+str(out),
               '-FU'+str(out), str(generated)]
    build = subprocess.run(command, capture_output=True, timeout=60)
    (out/'build.log').write_bytes(build.stdout+build.stderr)
    if build.returncode:
        raise RuntimeError('Pascal build failed')
    stdin = ''.join(' '.join(map(str, row))+'\n' for row in rows)
    (out/'inputs.txt').write_text(stdin)
    binaries = {'pascal': out/'iteration_refinement_generated.exe', 'cpp': a.cpp.resolve()}
    values = {}
    for name, exe in binaries.items():
        flags = ['--iteration-refine'] if name == 'cpp' else []
        p = subprocess.run([str(exe), *flags], input=stdin, text=True,
                           capture_output=True, timeout=30)
        (out/(name+'.txt')).write_text(p.stdout)
        (out/(name+'.stderr.txt')).write_text(p.stderr)
        if p.returncode:
            raise RuntimeError(name+' failed')
        values[name] = [list(map(int, line.split())) for line in p.stdout.splitlines()]
        if len(values[name]) != len(rows) or any(len(v) != 6 for v in values[name]):
            raise ValueError('Incomplete '+name)
    failed = [i for i, (p, c) in enumerate(zip(values['pascal'], values['cpp'])) if p != c]
    write_json(out/'golden.json', [dict(input=r, expected=e) for r, e in zip(rows, values['pascal'])])
    report = dict(finished=True, rows=len(rows), failed_rows=len(failed), exact=not failed,
                  scope=__doc__, command=command,
                  sources={str(p): digest(p.read_bytes()) for p in [original, generated, Path(__file__).resolve(), a.inputs]},
                  extracted_source_sha256=digest(snippet.encode()),
                  binaries={n: dict(path=str(p), sha256=digest(p.read_bytes())) for n, p in binaries.items()},
                  examples=[dict(row=i, input=rows[i], expected=values['pascal'][i], actual=values['cpp'][i]) for i in failed[:20]])
    write_json(out/'summary.json', report)
    print({k: report[k] for k in ['rows', 'failed_rows', 'exact']})
    return bool(failed)


if __name__ == '__main__':
    raise SystemExit(main())
