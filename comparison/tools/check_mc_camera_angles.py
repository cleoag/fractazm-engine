"""Bitwise check of CalculateVgradsFOV angle expressions extracted from Pascal.

This verifies Single horizontal FOV scaling and Double vertical FOV scaling in the
qualified FPC SSE2 build. It does not certify basis, DOF, or full camera rays.
"""
import argparse
import itertools
import random
import re
import subprocess
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
    start = code.index('      CAFX :=', code.index('procedure TMCCalcThread.CalculateVgradsFOV'))
    statements = code[start:code.index('      if MCTCameraOptic', start)]
    constants = re.findall(r'\bs05\s*:\s*Single\s*=\s*[^;]+;', definitions.read_text(encoding='latin1'))
    if len(constants) != 1:
        raise ValueError('Typed constant source changed')
    # Preserve the field types too: FOVXmul is Single, unlike FOVy.
    declarations = []
    for name in ['FOVXmul','FOVy']:
        fields = re.findall(r'\b'+name+r'\s*:\s*(?:Single|Double)\s*;', definitions.read_text(encoding='latin1'))
        if len(fields) != 1:
            raise ValueError('Camera field declaration changed: '+name)
        declarations.append(fields[0])
    source = '''program mc_camera_angles_probe_generated;
{$mode delphi}
const ''' + constants[0] + '''
var x,y:Single; iMandWidth,iMandHeight:Integer;
    CAFX,CAFY:Double;
''' + '\n'.join(declarations) + '''
begin
while not EOF(Input) do begin
ReadLn(x,y,iMandWidth,iMandHeight,FOVXmul,FOVy);
''' + statements + '''
WriteLn(PQWord(@CAFX)^,' ',PQWord(@CAFY)^);
end; end.
'''
    generated = work/'mc_camera_angles_probe_generated.pas'
    if generated.exists() and not generated.read_text().startswith('program mc_camera_angles_probe_generated;'):
        raise ValueError('Unrelated source at generated path')
    generated.write_text(source)
    command = [args.fpc, '-Mdelphi', '-O2', '-CfSSE2', '-al', '-FE'+str(output), '-FU'+str(output), str(generated)]
    proc = subprocess.run(command, capture_output=True, timeout=60)
    (output/'build.log').write_bytes(proc.stdout+proc.stderr)
    if proc.returncode:
        raise RuntimeError('Pascal build failed: '+str(output/'build.log'))
    pascal = output/'mc_camera_angles_probe_generated.exe'
    rows = [(x,y,w,h,fx,fy) for (x,y),(w,h),(fx,fy) in itertools.product(
        [(0.,0.),(.25,-.25),(26.25,3.75),(-.75,.5),(100.1234567,34.7654321),(16777216.,-16777216.)],
        [(1,1),(32,24),(53,37),(1920,1080),(16777217,16777219)],
        [(0.,0.),(.0123456789,.832143214321),(-.004,-3.14),(1e-20,1e20)])]
    rng = random.Random(0x43414d)
    rows += [(rng.uniform(-1,4097),rng.uniform(-1,4097),rng.randrange(1,4097),rng.randrange(1,4097),rng.uniform(-.1,.1),rng.uniform(-6.3,6.3)) for _ in range(8192)]
    stdin = ''.join(' '.join(format(v,'.17g') for v in row)+'\n' for row in rows)
    (output/'inputs.txt').write_text(stdin)
    def run(exe, flags):
        p = subprocess.run([str(exe.resolve()),*flags],input=stdin,text=True,capture_output=True,cwd=output,timeout=30)
        if p.returncode:
            raise RuntimeError(f'{exe} exit {p.returncode}: {p.stderr[:500]}')
        (output/(exe.stem+'.txt')).write_text(p.stdout)
        values = [[int(v) for v in line.split()] for line in p.stdout.splitlines()]
        if len(values)!=len(rows) or any(len(row)!=2 for row in values):
            raise ValueError('Incomplete output')
        return values
    expected, actual = run(pascal,[]), run(args.cpp,['--camera-angles'])
    failures, examples = [0,0], []
    for i,(ref,cpp) in enumerate(zip(expected,actual)):
        for lane,(r,c) in enumerate(zip(ref,cpp)):
            if r!=c:
                failures[lane]+=1
                if len(examples)<20:
                    examples.append(dict(row=i,lane=lane,input=rows[i],reference_bits=r,cpp_bits=c))
    report = dict(scope=__doc__, rows=len(rows), lanes=['CAFX','CAFY'], failures=failures,
                  exact=not any(failures), examples=examples, command=command,
                  sources={str(p):digest(p.read_bytes()) for p in [original,definitions,generated]},
                  extracted_sha256=digest(statements.encode()), field_declarations=declarations,
                  binaries={str(p):digest(p.read_bytes()) for p in [pascal,args.cpp]},
                  inputs_sha256=digest(stdin.encode()))
    write_json(output/'summary.json',report)
    print({k:report[k] for k in ['rows','failures','exact']})
    return int(not report['exact'])


if __name__ == '__main__':
    raise SystemExit(main())
