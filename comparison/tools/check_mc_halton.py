"""Compare the entire Pascal Halton table and finite radical-inverse inputs.

Extracts the immutable Pascal bodies unchanged; table words and Double bits
must match exactly. This does not verify ray scheduling or full MC images.
Bases below two are excluded (Pascal base one does not terminate).
"""
import argparse
import random
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
    a = ap.parse_args()
    work, out = a.pascal_work_root.resolve(), a.output.resolve()
    immutable = (ROOT/'mb3d').resolve()
    if work == immutable or immutable in work.parents:
        raise ValueError('Immutable Pascal tree')
    out.mkdir(parents=True, exist_ok=False)
    original = immutable/'CalcMonteCarlo.pas'
    text = original.read_text(encoding='latin1')
    snippet = extract(text, 'function halton(', 'function CalcMCT(')
    record = text[text.index('  HaltonRec = packed record'):text.index('  PHaltonRec =')]
    source = '''program mc_halton_probe_generated;
{$mode delphi}
uses Math, SysUtils;
type
''' + record + '''
var HaltonSequence: array[0..65535] of HaltonRec;
''' + snippet + '''
var i,idx,base: Integer; value: Double;
begin
  PreComputeHaltonSequence;
  for i := 0 to 65535 do
    with HaltonSequence[i] do WriteLn(HRx,' ',HRy,' ',HDx,' ',HDy);
  while not EOF(Input) do begin
    ReadLn(idx,base);
    if base < 2 then Halt(2);
    value := halton(idx,base);
    WriteLn(PQWord(@value)^);
  end;
end.
'''
    generated = work/'mc_halton_probe_generated.pas'
    if generated.exists() and not generated.read_text().startswith('program mc_halton_probe_generated;'):
        raise ValueError('Unrelated source at generated path')
    generated.write_text(source)
    command = [a.fpc, '-Mdelphi', '-O2', '-CfSSE2', '-FE'+str(out), '-FU'+str(out), str(generated)]
    proc = subprocess.run(command, capture_output=True, timeout=60)
    (out/'build.log').write_bytes(proc.stdout+proc.stderr)
    if proc.returncode: raise RuntimeError('Pascal build failed; see build.log')
    pascal = out/'mc_halton_probe_generated.exe'
    # All indices consumed by the fixed table, plus signed/large inputs used
    # by deeper-path calls and base powers and adjacent values.
    rows = [(i,b) for b in [2,3,5,7] for i in range(1,65537)]
    bases = [2,3,5,7,11,13,17,31,127,255,256,257,65535,65536,2147483647]
    for b in bases:
        indices = {-2147483648,-1,0,1,2,65535,65536,65537,2147483646,2147483647}
        power = b
        while power <= 2147483647:
            indices.update(i for i in [power-1,power,power+1] if i<=2147483647)
            power *= b
        rows.extend((i,b) for i in sorted(indices))
    rng = random.Random(0x48414c54)
    rows += [(rng.randint(0,2147483647),rng.choice(bases)) for _ in range(10000)]
    stdin = ''.join(f'{i} {b}\n' for i,b in rows)
    (out/'inputs.txt').write_text(stdin)
    def run(exe, flags):
        p = subprocess.run([str(exe.resolve()),*flags], input=stdin, text=True,
                           capture_output=True, cwd=out, timeout=90)
        if p.returncode: raise RuntimeError(f'{exe} exit {p.returncode}: {p.stderr[:500]}')
        (out/(exe.stem+'.txt')).write_text(p.stdout)
        result = [[int(v) for v in line.split()] for line in p.stdout.splitlines()]
        if len(result)!=65536+len(rows): raise ValueError('Incomplete output')
        if any(len(r)!=(4 if i<65536 else 1) for i,r in enumerate(result)):
            raise ValueError('Invalid output shape')
        return result
    expected, actual = run(pascal,[]), run(a.cpp,['--halton'])
    failures = [0]*5; examples = []
    for i,(ref,cpp) in enumerate(zip(expected,actual)):
        for lane,(r,c) in enumerate(zip(ref,cpp)):
            if r != c:
                failures[lane if i<65536 else 4] += 1
                if len(examples)<30:
                    examples.append(dict(row=i,input=i if i<65536 else rows[i-65536],lane=lane,reference=r,cpp=c))
    report = dict(scope=__doc__, rows=len(expected), table_rows=65536, inverse_rows=len(rows),
                  lanes=['HRx','HRy','HDx','HDy','inverse_bits'], failures=failures,
                  exact=not any(failures), examples=examples, command=command,
                  sources={str(p):digest(p.read_bytes()) for p in [original,generated,ROOT/'src/core/calc_monte_carlo.cpp',ROOT/'comparison/tools/mc_sampling_probe.cpp']},
                  extracted_sha256=digest(snippet.encode()),
                  binaries={str(p):digest(p.read_bytes()) for p in [pascal,a.cpp]}, inputs_sha256=digest(stdin.encode()))
    write_json(out/'summary.json',report)
    print({k:report[k] for k in ['rows','table_rows','inverse_rows','failures','exact']})
    return int(not report['exact'])


if __name__=='__main__': raise SystemExit(main())
