"""Check the original Pascal header's ray-step limiter expression bit for bit.

Extracts the typed constants, MaxCS and expression from the immutable reference;
compiles only in the external working tree using the reference SSE2 flags.
Finite selected boundaries and seeded random inputs are not universal proof.
"""
import argparse
import json
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
    a = ap.parse_args()
    work, out = a.pascal_work_root.resolve(), a.output.resolve()
    immutable = (ROOT/'mb3d').resolve()
    if work == immutable or immutable in work.parents:
        raise ValueError('Immutable Pascal tree')
    out.mkdir(parents=True, exist_ok=False)
    paths = [immutable/n for n in ('TypeDefinitions.pas','Math3D.pas','HeaderTrafos.pas')]
    types, math, header = [p.read_text(encoding='latin1') for p in paths]
    constants = []
    for name in ('s05','s0001','s001'):
        matches = re.findall(r'\b'+name+r'\s*:\s*Single\s*=\s*[^;]+;',types)
        if len(matches)!=1: raise ValueError('Constant changed: '+name)
        constants += matches
    start = math.rindex('function MaxCS(')
    maxcs = math[start:math.index('end;',math.index('asm',start))+4]
    expressions = re.findall(r'mctMH04ZSD := Max\(iMandWidth, iMandHeight\)[^;]+;',header)
    if len(expressions)!=1: raise ValueError('Expression changed')
    source = ('program ray_step_limiter_probe_generated;\n{$mode delphi}\nuses Math;\nconst '+
              '\n'.join(constants)+'\n'+maxcs+'''
var iMandWidth,iMandHeight:Integer; sZstepDiv,sRaystepLimiter,mctMH04ZSD:Single;
begin
while not EOF(Input) do begin
ReadLn(iMandWidth,iMandHeight,sZstepDiv,sRaystepLimiter);
'''+expressions[0]+'''
WriteLn(PLongWord(@mctMH04ZSD)^);
end; end.
''')
    generated = work/'ray_step_limiter_probe_generated.pas'
    if generated.exists() and not generated.read_text().startswith('program ray_step_limiter_probe_generated;'):
        raise ValueError('Unrelated source')
    generated.write_text(source)
    cmd = [a.fpc,'-Mdelphi','-O2','-CfSSE2','-al','-FE'+str(out),'-FU'+str(out),str(generated)]
    p = subprocess.run(cmd,capture_output=True,timeout=60)
    (out/'build.log').write_bytes(p.stdout+p.stderr)
    if p.returncode: raise RuntimeError('Pascal build failed')
    single = lambda x: struct.unpack('<f',struct.pack('<f',x))[0]
    rows = [(w,h,s,l) for w,h in ((64,48),(1,1),(1920,1080),(16384,8192))
            for s in (0.,1e-4,.001,.1,.25,.5,1.,2.,16.,1e6)
            for l in (0.,.001,.01,.1,1.,2.,100.)]
    rng = random.Random(0x53544550)
    rows += [(rng.randrange(1,16385),rng.randrange(1,16385),
              10**rng.uniform(-5,3),10**rng.uniform(-4,2)) for _ in range(8192)]
    stdin = ''.join(f'{w} {h} {single(s):.17g} {single(l):.17g}\n' for w,h,s,l in rows)
    (out/'inputs.txt').write_text(stdin)
    pascal = out/'ray_step_limiter_probe_generated.exe'
    def run(exe, flags):
        p = subprocess.run([str(exe.resolve()),*flags],input=stdin,text=True,capture_output=True,timeout=30)
        if p.returncode: raise RuntimeError(p.stderr)
        (out/(exe.stem+'.txt')).write_text(p.stdout)
        values = [int(x) for x in p.stdout.split()]
        if len(values)!=len(rows): raise ValueError('Incomplete output')
        return values
    expected, actual = run(pascal,[]), run(a.cpp,['--ray-step-limiter'])
    failures = [dict(input=rows[i],expected=e,actual=v) for i,(e,v) in enumerate(zip(expected,actual)) if e!=v]
    write_json(out/'summary.json',dict(scope=__doc__,finished=True,passed=not failures,
        rows=len(rows),failures=len(failures),examples=failures[:20],build_command=cmd,
        cpp_sha256=digest(a.cpp.read_bytes()),pascal_sha256=digest(pascal.read_bytes()),
        source_sha256={str(p):digest(p.read_bytes()) for p in paths+[generated,Path(__file__).resolve()]},
        input_sha256=digest((out/'inputs.txt').read_bytes())))
    print(len(rows),'rows;',len(failures),'failures')
    return bool(failures)


if __name__=='__main__':
    raise SystemExit(main())
