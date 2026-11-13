"""Bitwise check of CalcThread's integer-row camera angle against Pascal.

Extracts the expression and field/constant types from the immutable sources.
The oracle uses the qualified FPC SSE2 flags; this is finite input coverage,
not a claim about every camera mode or the original Delphi compiler.
"""
import argparse
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
    a = ap.parse_args()
    work, out = a.pascal_work_root.resolve(), a.output.resolve()
    immutable = (ROOT/'mb3d').resolve()
    if work == immutable or immutable in work.parents:
        raise ValueError('Immutable Pascal tree')
    out.mkdir(parents=True, exist_ok=False)
    paths = [immutable/n for n in ('TypeDefinitions.pas', 'CalcThread.pas')]
    types, code = [p.read_text(encoding='latin1') for p in paths]
    expressions = re.findall(r'CAFY := \(y / iMandHeight[^;]+;', code)
    if len(expressions) != 5 or len(set(expressions)) != 1:
        raise ValueError('Camera row expressions changed')
    declarations = []
    for name, kind in [('s05', 'Single'), ('CAFY', 'Double'), ('FOVy', 'Double')]:
        pattern = r'\b'+name+r'\s*:\s*'+kind+(r'\s*=\s*[^;]+;' if name == 's05' else r'\s*;')
        matches = re.findall(pattern, types)
        if len(matches) != 1:
            raise ValueError('Declaration changed: '+name)
        declarations += matches
    source = ('program camera_row_angle_probe_generated;\n{$mode delphi}\nconst '+
              declarations[0]+'\nvar y,iMandHeight:Integer;\n'+
              '\n'.join(declarations[1:])+'''
begin
while not EOF(Input) do begin
ReadLn(y,iMandHeight,FOVy);
'''+expressions[0]+'''
WriteLn(PQWord(@CAFY)^);
end; end.
''')
    generated = work/'camera_row_angle_probe_generated.pas'
    if generated.exists() and not generated.read_text().startswith('program camera_row_angle_probe_generated;'):
        raise ValueError('Unrelated source')
    generated.write_text(source)
    cmd = [a.fpc, '-Mdelphi', '-O2', '-CfSSE2', '-al', '-FE'+str(out), '-FU'+str(out), str(generated)]
    p = subprocess.run(cmd, capture_output=True, timeout=60)
    (out/'build.log').write_bytes(p.stdout+p.stderr)
    if p.returncode:
        raise RuntimeError('Pascal build failed')
    rows = [(y,h,f) for h in (1,3,48,1080,16777217,2147483647)
            for y in (-1,0,1,10,h//2,h-1,h)
            for f in (0.,.5235987755982988,-3.141592653589793,1e-100,1e100)]
    rng = random.Random(0x43414659)
    rows += [(rng.randrange(-16384,16385),rng.randrange(1,16385),rng.uniform(-6.3,6.3)) for _ in range(8192)]
    stdin = ''.join(f'{y} {h} {f:.17g}\n' for y,h,f in rows)
    (out/'inputs.txt').write_text(stdin)
    pascal = out/'camera_row_angle_probe_generated.exe'
    def run(exe, flags):
        p = subprocess.run([str(exe.resolve()), *flags], input=stdin, text=True, capture_output=True, timeout=30)
        if p.returncode:
            raise RuntimeError(p.stderr)
        (out/(exe.stem+'.txt')).write_text(p.stdout)
        values = [int(x) for x in p.stdout.split()]
        if len(values) != len(rows):
            raise ValueError('Incomplete output')
        return values
    expected, actual = run(pascal, []), run(a.cpp, ['--camera-row-angle'])
    failures = [dict(input=rows[i],expected=e,actual=v) for i,(e,v) in enumerate(zip(expected,actual)) if e!=v]
    write_json(out/'summary.json', dict(scope=__doc__,finished=True,passed=not failures,
        rows=len(rows),failures=len(failures),examples=failures[:20],build_command=cmd,
        cpp_sha256=digest(a.cpp.read_bytes()),pascal_sha256=digest(pascal.read_bytes()),
        source_sha256={str(p):digest(p.read_bytes()) for p in paths+[generated,Path(__file__).resolve()]},
        input_sha256=digest((out/'inputs.txt').read_bytes())))
    print(len(rows), 'rows;', len(failures), 'failures')
    return bool(failures)


if __name__ == '__main__':
    raise SystemExit(main())
