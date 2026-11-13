"""Compare shared color packing with the original Pascal SSE assembly.

Covers every half-integer in the 15-bit domain and neighboring Single values,
special encodings and a seeded sample of arbitrary float bits. The historical
non-SSE x87 branch remains outside this check.
"""
import argparse
import random
import re
import struct
import subprocess
from pathlib import Path
from external_validation import ROOT, digest, write_json


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--cpp', type=Path, required=True)
    ap.add_argument('--pascal-work-root', type=Path, required=True)
    ap.add_argument('--output', type=Path, required=True)
    ap.add_argument('--fpc', default='fpc')
    args = ap.parse_args()
    work, out = args.pascal_work_root.resolve(), args.output.resolve()
    immutable = (ROOT/'mb3d').resolve()
    if work == immutable or immutable in work.parents:
        raise ValueError('Immutable Pascal tree')
    out.mkdir(parents=True, exist_ok=False)
    original = ROOT/'mb3d/DivUtils.pas'
    code = original.read_text(encoding='latin1')
    matches = re.findall(r'procedure MinMaxClip15bit\(var s: Single; var w: Word\);\s*const.*?\nend;', code, re.S)
    if len(matches) != 1:
        raise ValueError('Original procedure changed')
    source = '''program clip15_probe_generated;
{$mode delphi}
{$asmmode intel}
uses Math;
var SupportSSE: LongBool = True;
''' + matches[0] + '''
var bits: LongWord; s: Single; w: Word;
begin
SetExceptionMask([exInvalidOp,exDenormalized,exZeroDivide,exOverflow,exUnderflow,exPrecision]);
while not EOF(Input) do begin
ReadLn(bits); Move(bits,s,4); MinMaxClip15bit(s,w); WriteLn(w);
end; end.
'''
    generated = work/'clip15_probe_generated.pas'
    if generated.exists() and not generated.read_text().startswith('program clip15_probe_generated;'):
        raise ValueError('Unrelated generated source')
    generated.write_text(source)
    command = [args.fpc,'-Mdelphi','-O2','-CfSSE2','-al','-FE'+str(out),'-FU'+str(out),str(generated)]
    build = subprocess.run(command,capture_output=True,timeout=60)
    (out/'build.log').write_bytes(build.stdout+build.stderr)
    if build.returncode: raise RuntimeError('Pascal build failed')
    bits_of = lambda f: struct.unpack('<I',struct.pack('<f',f))[0]
    inputs = [0,0x80000000,1,0x80000001,0x007fffff,0x807fffff,
              0x7f7fffff,0xff7fffff,0x7f800000,0xff800000,
              0x7fc00000,0xffc00000,0x7f800001,0xff800001,0x7fffffff,0xffffffff]
    for integer in range(32768):
        for value in (float(integer),integer+.5):
            bits = bits_of(value)
            inputs.extend(b for b in (bits-1,bits,bits+1) if b>=0)
    rng = random.Random(0x15B17)
    inputs += [rng.getrandbits(32) for _ in range(32768)]
    stdin = ''.join(str(i)+'\n' for i in inputs)
    (out/'inputs.txt').write_text(stdin)
    pascal = out/'clip15_probe_generated.exe'
    def run(exe,flags,label):
        result = subprocess.run([str(exe.resolve()),*flags],input=stdin,text=True,
                                capture_output=True,timeout=60)
        (out/(label+'.txt')).write_text(result.stdout)
        (out/(label+'.stderr.txt')).write_text(result.stderr)
        if result.returncode: raise RuntimeError(label+' execution failed')
        rows = [int(row) for row in result.stdout.splitlines()]
        if len(rows)!=len(inputs): raise ValueError('Incomplete '+label+' output')
        return rows
    expected, actual = run(pascal,[],'pascal'),run(args.cpp,['--clip15'],'cpp')
    failures = [i for i,(a,b) in enumerate(zip(expected,actual)) if a!=b]
    report = dict(scope=__doc__,rows=len(inputs),exact=not failures,failed_rows=len(failures),
                  examples=[dict(bits=inputs[i],expected=expected[i],actual=actual[i]) for i in failures[:20]],
                  command=command,inputs_sha256=digest(stdin.encode()),
                  sources={str(p):digest(p.read_bytes()) for p in [original,generated,Path(__file__)]},
                  binaries={str(p):digest(p.read_bytes()) for p in [pascal,args.cpp]})
    write_json(out/'summary.json',report)
    print({k:report[k] for k in ['rows','exact','failed_rows']})
    return int(bool(failures))


if __name__ == '__main__':
    raise SystemExit(main())
