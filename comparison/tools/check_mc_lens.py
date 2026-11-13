"""Check lens initialization and sample scaling using extracted Pascal statements.

Independent source extraction preserves Single argument/stores and mixed Double
StepWidth arithmetic. This does not check basis construction or complete rays.
"""
import argparse
import itertools
import random
import re
import shutil
import subprocess
from pathlib import Path
from check_mc_sampling import extract
from external_validation import ROOT, digest, write_json


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--pascal-work-root', type=Path, required=True)
    ap.add_argument('--cpp', type=Path, required=True)
    ap.add_argument('--output', type=Path, required=True)
    ap.add_argument('--fpc', default='fpc')
    args = ap.parse_args()
    work, output = args.pascal_work_root.resolve(), args.output.resolve()
    if work == (ROOT/'mb3d').resolve() or (ROOT/'mb3d').resolve() in work.parents:
        raise ValueError('Immutable Pascal tree')
    output.mkdir(parents=True, exist_ok=False)
    original = ROOT/'mb3d/CalcMonteCarlo.pas'
    mathfile = work/'Math3D.pas'
    code, math = original.read_text(encoding='latin1'), mathfile.read_text(encoding='latin1')
    snippets = {
        'CalcBokeh': extract(code, 'function CalcBokeh(', '{TMCCalcThread}'),
        'MakeDiscFromHalton': extract(code, 'procedure MakeDiscFromHalton(', 'procedure GetHalton2Dshifts('),
        'NotZero': extract(math, 'function NotZero(d:', 'procedure Clamp0SvecPas('),
        'SinCosS': extract(math, 'procedure SinCosS(', 'function FracSingle('),
        'MaxCS': extract(math, 'function MaxCS(s1, s2:', 'function Max0S('),
    }
    for field in ['sDOFaperture', 'sDOFZsharp']:
        matches = re.findall(r'MCThread\[x - 1\]\.'+field+r'\s*:= ([^;]+);', code)
        if len(matches) != 1:
            raise ValueError('Initialization source shape changed: '+field)
        snippets[field] = field+' := '+matches[0]+';'
    method = extract(code, 'procedure TMCCalcThread.CalcBokehMC(', 'procedure TMCCalcThread.DoDOF;')
    start, end = method.index('      s := CalcBokeh('), method.index(' {   end')
    snippets['scale'] = method[start:end].strip()
    source = '''program mc_lens_probe_generated;
{$mode delphi}
uses Math;
const s05: Single = 0.5; sPiM025: Single = Pi * 0.25;
function pm_sin(x: Double): Double; cdecl; external 'portable_math.dll' name 'pm_sin';
function pm_cos(x: Double): Double; cdecl; external 'portable_math.dll' name 'pm_cos';
'''
    source += '\n\n'.join(snippets[k] for k in ['NotZero','SinCosS','MaxCS','CalcBokeh','MakeDiscFromHalton'])
    source += '''
var Header: record sDOFaperture,sDOFZsharp: Single; Width: Integer; end;
    MCTparas: record StepWidth: Double; end;
    xx,yy,s,sDOFaperture,sDOFZsharp: Single; iBokehNr: Integer;
begin
  while not EOF(Input) do begin
    ReadLn(xx,yy,iBokehNr,Header.sDOFaperture,Header.sDOFZsharp,Header.Width,MCTparas.StepWidth);
'''
    source += '\n'.join(snippets[k] for k in ['sDOFaperture','sDOFZsharp'])
    source += '\nMakeDiscFromHalton(xx,yy);\n'+snippets['scale']
    source += "\nWriteLn(PCardinal(@xx)^,' ',PCardinal(@yy)^,' ',PCardinal(@sDOFZsharp)^,' ',PCardinal(@sDOFaperture)^,' ',PCardinal(@s)^);\nend;\nend.\n"
    generated = work/'mc_lens_probe_generated.pas'
    if generated.exists() and not generated.read_text().startswith('program mc_lens_probe_generated;'):
        raise ValueError('Unrelated source at generated path')
    generated.write_text(source)
    command = [args.fpc,'-Mdelphi','-O2','-CfSSE2','-FE'+str(output),'-FU'+str(output),str(generated)]
    proc = subprocess.run(command,capture_output=True,timeout=60)
    (output/'build.log').write_bytes(proc.stdout+proc.stderr)
    if proc.returncode:
        raise RuntimeError('Pascal build failed: '+str(output/'build.log'))
    shutil.copyfile(work/'portable_math.dll', output/'portable_math.dll')
    pascal = output/'mc_lens_probe_generated.exe'
    rows = [(x,y,m,a,f,w,s) for (x,y),m,(a,f,w,s) in itertools.product(
        [(0.,0.),(.5,.5),(1.,1.),(.1234567,.7654321),(.94,.01)], range(8),
        [(0.,0.,32,.01),(2e-7,.01,127,.0001),(.02,.3,1920,.0023),(-.5,-.1,53,.03),(.7,.1234567,167,.007)])]
    rng = random.Random(0x24563487)
    rows += [(rng.random(),rng.random(),rng.randrange(8),rng.uniform(1e-8,.9),rng.uniform(0.,2.),rng.randrange(1,4097),rng.uniform(1e-8,.1)) for _ in range(4096)]
    stdin = ''.join(' '.join(format(v,'.17g') for v in row)+'\n' for row in rows)
    (output/'inputs.txt').write_text(stdin)
    def run(exe, flags):
        p = subprocess.run([str(exe.resolve()),*flags],input=stdin,text=True,capture_output=True,cwd=output,timeout=30)
        if p.returncode:
            raise RuntimeError(f'{exe} exit {p.returncode}: {p.stderr[:500]}')
        (output/(exe.stem+'.txt')).write_text(p.stdout)
        values = [[int(v) for v in line.split()] for line in p.stdout.splitlines()]
        if len(values)!=len(rows) or any(len(row)!=5 for row in values):
            raise ValueError('Incomplete output')
        return values
    expected, actual = run(pascal,[]), run(args.cpp,['--lens'])
    failures, examples = [0]*5, []
    for i,(ref,cpp) in enumerate(zip(expected,actual)):
        for lane,(r,c) in enumerate(zip(ref,cpp)):
            if r!=c:
                failures[lane]+=1
                if len(examples)<20:
                    examples.append(dict(row=i,lane=lane,input=rows[i],reference_bits=r,cpp_bits=c))
    report = dict(scope=__doc__, rows=len(rows), lanes=['x','y','focus','aperture','scale'],
                  failures=failures, exact=not any(failures), examples=examples, command=command,
                  sources={str(p):digest(p.read_bytes()) for p in [original,mathfile,generated]},
                  extracted={k:digest(v.encode()) for k,v in snippets.items()},
                  binaries={str(p):digest(p.read_bytes()) for p in [pascal,args.cpp,output/'portable_math.dll']},
                  inputs_sha256=digest(stdin.encode()))
    write_json(output/'summary.json',report)
    print({k:report[k] for k in ['rows','failures','exact']})
    return int(not report['exact'])


if __name__ == '__main__':
    raise SystemExit(main())
