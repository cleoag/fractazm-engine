"""Compare uniform/Gaussian AA and random consumption with extracted Pascal.

Uses the complete GetSubPixelShift body and typed source constants. The input
stream is supplied independently, so this checks rejection and sampling math,
not the random generator, camera rays or full Monte Carlo transport.
"""
import argparse
import itertools
import random
import re
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
    immutable = (ROOT/'mb3d').resolve()
    if work == immutable or immutable in work.parents:
        raise ValueError('Immutable Pascal tree')
    output.mkdir(parents=True, exist_ok=False)
    original, definitions = ROOT/'mb3d/CalcMonteCarlo.pas', ROOT/'mb3d/TypeDefinitions.pas'
    method = extract(original.read_text(encoding='latin1'),
                     'procedure TMCCalcThread.GetSubPixelShift(', 'procedure TMCCalcThread.Execute;')
    constants = []
    for name in ['s05', 's001']:
        matches = re.findall(r'\b'+name+r'\s*:\s*Single\s*=\s*[^;]+;', definitions.read_text(encoding='latin1'))
        if len(matches) != 1: raise ValueError('Constant source changed: '+name)
        constants.append(matches[0])
    source = '''program mc_subpixel_probe_generated;
{$mode delphi}
uses Math, SysUtils;
const ''' + '\n'.join(constants) + '''
var stream: array[0..4095] of Double;
    used, count, i, mode: Integer;
    bGaussAA: Boolean;
    xx,yy: Single;
function GetRand: Double;
begin
  if used >= count then Halt(3);
  Result := stream[used]; Inc(used);
end;
''' + method.replace('TMCCalcThread.GetSubPixelShift', 'GetSubPixelShift', 1) + '''
begin
  while not EOF(Input) do begin
    Read(mode,count);
    if (count < 2) or (count > 4096) then Halt(2);
    for i := 0 to count-1 do Read(stream[i]);
    ReadLn;
    used := 0; bGaussAA := mode <> 0;
    GetSubPixelShift(xx,yy);
    WriteLn(PCardinal(@xx)^,' ',PCardinal(@yy)^,' ',used);
  end;
end.
'''
    generated = work/'mc_subpixel_probe_generated.pas'
    if generated.exists() and not generated.read_text().startswith('program mc_subpixel_probe_generated;'):
        raise ValueError('Unrelated source at generated path')
    generated.write_text(source)
    command = [args.fpc, '-Mdelphi', '-O2', '-CfSSE2', '-FE'+str(output), '-FU'+str(output), str(generated)]
    proc = subprocess.run(command, capture_output=True, timeout=60)
    (output/'build.log').write_bytes(proc.stdout+proc.stderr)
    if proc.returncode: raise RuntimeError('Pascal build failed; see build.log')
    pascal = output/'mc_subpixel_probe_generated.exe'
    rng = random.Random(0x24563487)
    # Includes center, exact circle boundary, adjacent values, multiple rejected
    # pairs, and unused trailing values. Every stream ends in an accepted pair.
    values = [0., 1e-12, .1, .5-1e-7, .5, .5+1e-7, .9, 1.-1e-12, 1.]
    rows = [(mode, [x,y,.5,.5]) for mode,x,y in itertools.product(range(2),values,values)]
    rows += [(mode, [0.,0.,1.,1.,0.,1.,1.,0.,.7,.3,.8,.8]) for mode in range(2)]
    rows += [(mode, [rng.random() for _ in range(32)]+[.5,.5]) for mode in range(2) for _ in range(4096)]
    stdin = ''.join(f'{mode} {len(stream)} '+ ' '.join(format(v,'.17g') for v in stream)+'\n' for mode,stream in rows)
    (output/'inputs.txt').write_text(stdin)
    def run(exe, flags):
        p = subprocess.run([str(exe.resolve()),*flags], input=stdin, text=True,
                           capture_output=True, cwd=output, timeout=30)
        if p.returncode: raise RuntimeError(f'{exe} exit {p.returncode}: {p.stderr[:500]}')
        (output/(exe.stem+'.txt')).write_text(p.stdout)
        result = [[int(v) for v in line.split()] for line in p.stdout.splitlines()]
        if len(result)!=len(rows) or any(len(row)!=3 for row in result): raise ValueError('Incomplete output')
        return result
    expected, actual = run(pascal, []), run(args.cpp, ['--subpixel'])
    failures, examples = [0]*3, []
    modes = {str(m): {'rows': 0, 'failures': [0]*3} for m in range(2)}
    for i,(ref,cpp) in enumerate(zip(expected,actual)):
        group = modes[str(rows[i][0])]; group['rows'] += 1
        for lane,(r,c) in enumerate(zip(ref,cpp)):
            if r != c:
                failures[lane] += 1; group['failures'][lane] += 1
                if len(examples)<20: examples.append(dict(row=i,lane=lane,input=rows[i],reference=r,cpp=c))
    report = dict(scope=__doc__, rows=len(rows), lanes=['x_bits','y_bits','random_values_consumed'],
                  failures=failures, modes=modes, exact=not any(failures), examples=examples, command=command,
                  sources={str(p):digest(p.read_bytes()) for p in [original,definitions,generated]},
                  extracted={'method':digest(method.encode()),'constants':digest('\n'.join(constants).encode())},
                  binaries={str(p):digest(p.read_bytes()) for p in [pascal,args.cpp]}, inputs_sha256=digest(stdin.encode()))
    write_json(output/'summary.json', report)
    print({k:report[k] for k in ['rows','failures','modes','exact']})
    return int(not report['exact'])


if __name__ == '__main__': raise SystemExit(main())
