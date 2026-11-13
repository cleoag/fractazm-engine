"""Check Halton shifts, Word conversion and RNG against extracted Pascal.

All 65536 Word values are checked at representative shifts, plus pixel/seed
streams. This is finite helper evidence, not complete Monte Carlo transport.
"""
import argparse
import itertools
import random
import re
import subprocess
from check_mc_sampling import extract
from pathlib import Path
from external_validation import ROOT, digest, write_json


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--pascal-work-root', type=Path, required=True)
    ap.add_argument('--cpp', type=Path, required=True)
    ap.add_argument('--output', type=Path, required=True)
    ap.add_argument('--fpc', default='fpc')
    args = ap.parse_args()
    work, output = args.pascal_work_root.resolve(), args.output.resolve()
    immutable = (ROOT/'mb3d').resolve()
    if work == immutable or immutable in work.parents:
        raise ValueError('Immutable Pascal tree')
    output.mkdir(parents=True, exist_ok=False)
    original = ROOT/'mb3d/CalcMonteCarlo.pas'
    definitions = ROOT/'mb3d/TypeDefinitions.pas'
    code = original.read_text(encoding='latin1')
    mathfile = ROOT/'mb3d/Math3D.pas'
    snippets = {
        'shifts': extract(code, 'procedure GetHalton2Dshifts(', 'function TMCCalcThread.GetRand:'),
        'random': extract(code, 'function TMCCalcThread.GetRand:', 'function TMCCalcThread.GenSphereSVecOm:'),
        'fraction': extract(mathfile.read_text(encoding='latin1'), 'function FracSingle(', 'procedure ChangeMathFuncsToSSE2;'),
    }
    constants = []
    for name in ['d1d65535','dSeedMul']:
        found = re.findall(r'\b'+name+r'\s*:\s*Double\s*=\s*[^;]+;', definitions.read_text(encoding='latin1'))
        if len(found) != 1: raise ValueError('Constant source changed: '+name)
        constants.append(found[0])
    source = """program mc_halton_shifts_probe_generated;
{$mode delphi}
{$asmmode intel}
{$Q-}{$R-}
const """ + '\n'.join(constants) + """
var seed: Cardinal;
"""
    source += snippets['shifts']+'\n'+snippets['fraction']+'\n'
    # Only the receiver address is changed: the original seed field becomes a
    # standalone variable. Arithmetic and x87 load/multiply remain original.
    source += snippets['random'].replace('TMCCalcThread.GetRand','GetRand').replace('[eax + seed]', '[seed]')
    start = code.index('      xx := FracSingle(', code.index('procedure TMCCalcThread.CalcBokehMC'))
    statement = code[start:code.index(';',start)+1]
    statement = statement.replace('HaltonSequence[iActRayNr].HDx','wordValue').replace('HaltonShiftX','shift')
    source += """
var x,y:Cardinal; wordValue:Word; shift,s1,s2,xx:Single; randomValue:Double;
begin
while not EOF(Input) do begin
ReadLn(x,y,seed,wordValue,shift);
GetHalton2Dshifts(s1,s2,Integer(x),Integer(y));
""" + statement + """
randomValue := GetRand;
WriteLn(PCardinal(@s1)^,' ',PCardinal(@s2)^,' ',PCardinal(@xx)^,' ',PQWord(@randomValue)^,' ',seed);
end; end.
"""
    generated = work/'mc_halton_shifts_probe_generated.pas'
    if generated.exists() and not generated.read_text().startswith('program mc_halton_shifts_probe_generated;'):
        raise ValueError('Unrelated source at generated path')
    generated.write_text(source)
    command = [args.fpc, '-Mdelphi', '-O2', '-CfSSE2', '-al', '-FE'+str(output), '-FU'+str(output), str(generated)]
    proc = subprocess.run(command, capture_output=True, timeout=60)
    (output/'build.log').write_bytes(proc.stdout+proc.stderr)
    if proc.returncode:
        raise RuntimeError('Pascal build failed: '+str(output/'build.log'))
    pascal = output/'mc_halton_shifts_probe_generated.exe'
    # Positive and signed shifts catch truncation-vs-floor behavior; runtime
    # shifts are nonnegative. Every Word is checked for each listed shift.
    rows = [(i%4096, i//4096, (i*0x324594a1)&0xffffffff, i, shift)
            for shift in [0.,.1234567,.5,.99999994,-.125] for i in range(65536)]
    rng = random.Random(0x48414c)
    rows += [(rng.getrandbits(32),rng.getrandbits(32),rng.getrandbits(32),rng.randrange(65536),rng.random()) for _ in range(8192)]
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
    expected, actual = run(pascal,[]), run(args.cpp,['--halton-shifts'])
    failures, examples = [0]*5, []
    for i,(ref,cpp) in enumerate(zip(expected,actual)):
        for lane,(r,c) in enumerate(zip(ref,cpp)):
            if r!=c:
                failures[lane]+=1
                if len(examples)<20:
                    examples.append(dict(row=i,lane=lane,input=rows[i],reference_bits=r,cpp_bits=c))
    report = dict(scope=__doc__, rows=len(rows), lanes=['shift_x','shift_y','shifted_word','random','seed'], failures=failures,
                  exact=not any(failures), examples=examples, command=command,
                  sources={str(p):digest(p.read_bytes()) for p in [original,definitions,mathfile,generated]},
                  extracted={k:digest(v.encode()) for k,v in snippets.items()}, shifted_word_statement_sha256=digest(statement.encode()),
                  binaries={str(p):digest(p.read_bytes()) for p in [pascal,args.cpp]},
                  inputs_sha256=digest(stdin.encode()))
    write_json(output/'summary.json',report)
    print({k:report[k] for k in ['rows','failures','exact']})
    return int(not report['exact'])


if __name__ == '__main__':
    raise SystemExit(main())
