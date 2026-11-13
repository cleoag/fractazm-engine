"""Compare lens helpers against extracted Pascal functions, with source provenance.
The standalone helper is generated only in the editable Pascal working tree.
This checks sampling math, not full Monte Carlo transport or GUI scheduling.
"""
import argparse
import itertools
import json
from pathlib import Path
import random
import shutil
import struct
import subprocess
from external_validation import ROOT, digest, write_json

def extract(source, start, stop):
    section = source[source.index('implementation'):]
    a = section.index(start)
    return section[a:section.index(stop, a)].strip()

def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--pascal-work-root', type=Path, required=True)
    ap.add_argument('--cpp', type=Path, required=True)
    ap.add_argument('--output', type=Path, required=True)
    ap.add_argument('--fpc', default='fpc')
    args = ap.parse_args()
    work = args.pascal_work_root.resolve()
    if work == (ROOT / 'mb3d').resolve() or (ROOT / 'mb3d').resolve() in work.parents:
        raise ValueError('Immutable Pascal reference cannot be used as working tree')
    output = args.output.resolve()
    output.mkdir(parents=True, exist_ok=False)
    code = (ROOT / 'mb3d/CalcMonteCarlo.pas').read_text(encoding='latin1')
    math = (work / 'Math3D.pas').read_text(encoding='latin1')
    snippets = {
        'CalcBokeh': extract(code, 'function CalcBokeh(', '{TMCCalcThread}'),
        'MakeDiscFromHalton': extract(code, 'procedure MakeDiscFromHalton(', 'procedure GetHalton2Dshifts('),
        'NotZero': extract(math, 'function NotZero(d:', 'procedure Clamp0SvecPas('),
        'SinCosS': extract(math, 'procedure SinCosS(', 'function FracSingle('),
    }
    generated = work / 'mc_sampling_probe_generated.pas'
    source = '''program mc_sampling_probe_generated;
{$mode delphi}
uses Math;
const s05: Single = 0.5; sPiM025: Single = Pi * 0.25;
function pm_sin(x: Double): Double; cdecl; external 'portable_math.dll' name 'pm_sin';
function pm_cos(x: Double): Double; cdecl; external 'portable_math.dll' name 'pm_cos';
'''
    source += '\n\n'.join(snippets[k] for k in ('NotZero', 'SinCosS', 'CalcBokeh', 'MakeDiscFromHalton'))
    source += '''
var x,y,b: Single; mode: Integer;
begin
  while not EOF(Input) do begin
    ReadLn(x,y,mode);
    b := CalcBokeh(x,y,mode);
    MakeDiscFromHalton(x,y);
    WriteLn(PCardinal(@x)^,' ',PCardinal(@y)^,' ',PCardinal(@b)^);
  end;
end.
'''
    if generated.exists() and generated.read_text() != source:
        # This is this tool's dedicated generated file, never an application unit.
        if not generated.read_text().startswith('program mc_sampling_probe_generated;'):
            raise ValueError('Refusing to overwrite an unrelated Pascal source')
    generated.write_text(source)
    command = [args.fpc, '-Mdelphi', '-O2', '-CfSSE2', '-FE'+str(output), '-FU'+str(output), str(generated)]
    compiled = subprocess.run(command, capture_output=True, timeout=60)
    (output / 'build.log').write_bytes(compiled.stdout + compiled.stderr)
    if compiled.returncode:
        raise RuntimeError('Pascal helper build failed; see build.log')
    shutil.copyfile(work / 'portable_math.dll', output / 'portable_math.dll')
    pascal = output / 'mc_sampling_probe_generated.exe'
    rng = random.Random(0x24563487)
    grid = list(itertools.product([i / 20 for i in range(21)], repeat=2))
    grid += [(rng.random(), rng.random()) for _ in range(512)]
    # Sector boundaries, disk center, and bokeh radial branch thresholds.
    for value in [0., .5, .94, .96, 1.]:
        for delta in [-1e-7, 0., 1e-7]:
            grid += [(value+delta,0.), (0.,value+delta), (value+delta,value), (value,1-value+delta)]
    rows = [(x,y,m) for x,y in grid for m in range(8)]
    stdin = ''.join(f'{x:.17g} {y:.17g} {m}\n' for x,y,m in rows)
    (output / 'inputs.txt').write_text(stdin)
    def run(exe):
        proc = subprocess.run([str(exe.resolve())], input=stdin, text=True, capture_output=True, timeout=30, cwd=output)
        if proc.returncode:
            raise RuntimeError(f'{exe.name} exit {proc.returncode}: {proc.stderr[:500]}')
        (output / (exe.stem+'.txt')).write_text(proc.stdout)
        values = [[struct.unpack('<f', struct.pack('<I', int(v)))[0] for v in line.split()] for line in proc.stdout.splitlines()]
        if len(values)!=len(rows) or any(len(v)!=3 for v in values):
            raise ValueError('Incomplete sampling output')
        return values
    expected, actual = run(pascal), run(args.cpp)
    failures = [0,0,0]; maximum = [0.,0.,0.]; examples = []
    for row,(ref,cpp) in enumerate(zip(expected,actual)):
        for lane,(r,c) in enumerate(zip(ref,cpp)):
            maximum[lane]=max(maximum[lane],abs(r-c))
            if struct.pack('<f',r)!=struct.pack('<f',c):
                failures[lane]+=1
                if len(examples)<30: examples.append(dict(row=row,lane=lane,input=rows[row],reference=r,cpp=c))
    report = dict(scope=__doc__, rows=len(rows), failures=failures, max_abs_delta=maximum,
                  exact=not any(failures), examples=examples, command=command,
                  sources={str(p):digest(p.read_bytes()) for p in [ROOT/'mb3d/CalcMonteCarlo.pas',work/'Math3D.pas',generated]},
                  extracted={k:digest(v.encode()) for k,v in snippets.items()},
                  binaries={str(p):digest(p.read_bytes()) for p in [pascal,args.cpp,output/'portable_math.dll']},
                  inputs_sha256=digest(stdin.encode()))
    write_json(output/'summary.json',report)
    print({k:report[k] for k in ('rows','failures','max_abs_delta','exact')})
    return int(not report['exact'])
if __name__=='__main__':
    raise SystemExit(main())
