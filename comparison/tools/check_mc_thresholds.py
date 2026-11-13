"""Check Pascal typed MC constants and the original DE refinement predicate.

The finite boundary/random corpus certifies these two constants and this branch,
not the complete marcher, shadow transport, or smooth-iteration refinement.
"""
import argparse
import math
import random
import re
import struct
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
    typedefs = definitions.read_text(encoding='latin1')
    constants = []
    for name in ['s0001', 's001']:
        matches = re.findall(r'\b'+name+r'\s*:\s*Single\s*=\s*[^;]+;', typedefs)
        if len(matches) != 1:
            raise ValueError('Typed constant declaration changed: '+name)
        constants.append(matches[0])
    predicates = re.findall(r'while (Abs\(dTmp - msDEstop\) > s0001) do', code)
    if len(predicates) != 1:
        raise ValueError('Original binary refinement condition changed')
    source = '''program mc_thresholds_probe_generated;
{$mode delphi}
const '''+'\n'.join(constants)+'''
var dTmp:Double; msDEstop:Single;
begin
WriteLn(PLongWord(@s0001)^,' ',PLongWord(@s001)^);
while not EOF(Input) do begin
ReadLn(dTmp,msDEstop);
WriteLn(Ord('''+predicates[0]+'''));
end; end.
'''
    generated = work/'mc_thresholds_probe_generated.pas'
    if generated.exists() and not generated.read_text().startswith('program mc_thresholds_probe_generated;'):
        raise ValueError('Unrelated generated source')
    generated.write_text(source)
    command = [args.fpc, '-Mdelphi', '-O2', '-CfSSE2', '-al', '-FE'+str(output), '-FU'+str(output), str(generated)]
    p = subprocess.run(command, capture_output=True, timeout=60)
    (output/'build.log').write_bytes(p.stdout+p.stderr)
    if p.returncode:
        raise RuntimeError('Pascal build failed')
    pascal = output/'mc_thresholds_probe_generated.exe'
    single = lambda x: struct.unpack('<f', struct.pack('<f', x))[0]
    # Include the actual threshold and adjacent Doubles, the erroneous decimal
    # interpretation, both signs, cancellation at large stop, and signed zeros.
    threshold = single(float(constants[0].split('=')[1].rstrip(';')))
    rows = []
    for stop in [0., -0., 1e-30, -.5, 1., single(1.3605704307556152), 1e5, 1e20]:
        stop = single(stop)
        for offset in [0., .0001, -.0001, .0009, -.0009, threshold, -threshold]:
            center = stop+offset
            rows += [(math.nextafter(center,-math.inf),stop), (center,stop),
                     (math.nextafter(center,math.inf),stop)]
    rng = random.Random(0x544852)
    rows += [(stop+offset,stop) for stop,offset in
             [(single(rng.uniform(-10,10)),rng.uniform(-.02,.02)) for _ in range(8192)]]
    stdin = ''.join(f'{d:.17g} {s:.17g}\n' for d,s in rows)
    (output/'inputs.txt').write_text(stdin)
    def run(exe, flags):
        p = subprocess.run([str(exe.resolve()), *flags], input=stdin, text=True,
                           capture_output=True, cwd=output, timeout=30)
        if p.returncode:
            raise RuntimeError(f'{exe}: {p.returncode}: {p.stderr[:500]}')
        (output/(exe.stem+'.txt')).write_text(p.stdout)
        lines = [[int(v) for v in line.split()] for line in p.stdout.splitlines()]
        if len(lines)!=len(rows)+1 or len(lines[0])!=2 or any(len(v)!=1 for v in lines[1:]):
            raise ValueError('Incomplete oracle output')
        return lines
    ref, actual = run(pascal,[]), run(args.cpp,['--thresholds'])
    failures = [i for i,(a,b) in enumerate(zip(ref,actual)) if a!=b]
    report = dict(scope=__doc__, rows=len(rows), constant_bits=ref[0],
                  constants=constants, predicate=predicates[0], exact=not failures,
                  failures=[len(failures)], examples=[dict(index=i,expected=ref[i],actual=actual[i]) for i in failures[:20]],
                  command=command, sources={str(p):digest(p.read_bytes()) for p in [original,definitions,generated]},
                  binaries={str(p):digest(p.read_bytes()) for p in [pascal,args.cpp]},
                  inputs_sha256=digest(stdin.encode()))
    write_json(output/'summary.json',report)
    print({k:report[k] for k in ['rows','constant_bits','failures','exact']})
    return int(bool(failures))


if __name__ == '__main__':
    raise SystemExit(main())
