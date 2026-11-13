"""Compare ray refinement and march-step limits with extracted Pascal code.

The x87 block is extracted from immutable Calc.pas; only addresses are mapped
to a standalone record. March limiting uses the original CalcThread expression
and Math3D.MaxCS. Finite boundary/seeded cases are not universal coverage.
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
        raise ValueError('Immutable reference tree')
    out.mkdir(parents=True, exist_ok=False)
    paths = [immutable/n for n in ('Calc.pas','CalcThread.pas','Math3D.pas')]
    calc, thread, math = [p.read_text(encoding='latin1') for p in paths]
    start = calc.rindex('procedure RMdoBinSearch(')
    block = calc[calc.index('@@4:', start):calc.index('  dec  ebp', calc.index('@@4:', start))]
    block = re.sub(r'\{.*?\}', '', block, flags=re.S)
    block = re.sub(r'//[^\n]*', '', block).replace('@@4:', '')
    mappings = {
        '[esi+TMCTparameter.mZZ-$38]': '[eax+24]', '[esp]': '[eax+32]',
        '[esi+TMCTparameter.mVgradsFOV-$38]': '[eax+40]',
        '[esi+TMCTparameter.mVgradsFOV-$38 + 8]': '[eax+48]',
        '[esi+TMCTparameter.mVgradsFOV-$38 + 16]': '[eax+56]',
        '[ebx+TIteration3Dext.C1]': '[eax]',
        '[ebx+TIteration3Dext.C1 + 8]': '[eax+8]',
        '[ebx+TIteration3Dext.C1 + 16]': '[eax+16]',
        '[esi+TMCTparameter.mctDEstopFactor-$38]': '[eax+64]',
        '[esi+TMCTparameter.DEstop-$38]': '[eax+68]',
        '[esi+TMCTparameter.msDEstop-$38]': '[eax+72]',
    }
    for old, new in mappings.items():
        if old not in block:
            raise ValueError('Reference address changed: '+old)
        block = block.replace(old, new)
    if re.search(r'\b(esi|ebx|esp|ebp)\b', block, re.I):
        raise ValueError('Unmapped reference address')
    start = math.rindex('function MaxCS(')
    maxcs = math[start:math.index('end;', math.index('asm', start))+4]
    expression = 'dT1 := MaxCS(msDEstop, 0.4) * mctMH04ZSD;'
    if expression not in thread:
        raise ValueError('March expression changed')
    source = '''program ray_refinement_probe_generated;
{$mode delphi}
uses Math;
type TValues=packed record v:array[0..7] of Double; factor,stop,result:Single; end;
procedure Refine(p:Pointer); assembler;
asm
'''+block+'''
end;
'''+maxcs+'''
var data:TValues; i:Integer; dT1:Double; msDEstop,mctMH04ZSD:Single;
begin
Set8087CW($137f);
while not EOF(Input) do begin
if ParamCount=0 then begin
for i:=0 to 7 do Read(data.v[i]); ReadLn(data.factor,data.stop);
Refine(@data);
for i:=0 to 3 do Write(PQWord(@data.v[i])^,' ');
WriteLn(PLongWord(@data.result)^);
end else begin
ReadLn(msDEstop,mctMH04ZSD);
'''+expression+'''
WriteLn(PQWord(@dT1)^);
end;
end; end.
'''
    generated = work/'ray_refinement_probe_generated.pas'
    if generated.exists() and not generated.read_text().startswith('program ray_refinement_probe_generated;'):
        raise ValueError('Unrelated source')
    generated.write_text(source)
    cmd = [a.fpc,'-Mdelphi','-O2','-CfSSE2','-al','-FE'+str(out),'-FU'+str(out),str(generated)]
    p = subprocess.run(cmd,capture_output=True,timeout=60)
    (out/'build.log').write_bytes(p.stdout+p.stderr)
    if p.returncode:
        raise RuntimeError('Pascal compilation failed')
    single = lambda v: struct.unpack('<f',struct.pack('<f',v))[0]
    rng = random.Random(0x524159)
    rows = []
    for step in (-1e20,-1.,-0.,0.,1e-12,.5,1.,1e20):
        for scale in (1e-20,1.,1e20):
            rows.append([scale,-scale,scale*.3,1.,step,1e-12,-.1,1.,single(.01),single(.4)])
    for _ in range(256):
        rows.append([rng.uniform(-2,2) for _ in range(8)]+[single(rng.random()),single(rng.random())])
    limits = [[single(s),single(l)] for s in (0.,.39999999,.4,.40000003,1.,1.2470693588256836,1e10)
              for l in (0.,.1,1.,22.650032043457031,1e10)]
    pascal = out/'ray_refinement_probe_generated.exe'
    checks = {}
    fixtures = {}
    for name,inputs,pflags,cflags in [('refine',rows,[],['--refine-ray-step']),
                                     ('limit',limits,['limit'],['--march-step-limit'])]:
        stdin = ''.join(' '.join(format(v,'.17g') for v in row)+'\n' for row in inputs)
        (out/(name+'-inputs.txt')).write_text(stdin)
        outputs = []
        for side,exe,flags in [('pascal',pascal,pflags),('cpp',a.cpp,cflags)]:
            p = subprocess.run([str(exe.resolve()),*flags],input=stdin,text=True,capture_output=True,timeout=30)
            if p.returncode:
                raise RuntimeError(side+': '+p.stderr)
            (out/(name+'-'+side+'.txt')).write_text(p.stdout)
            values = [[int(w) for w in line.split()] for line in p.stdout.splitlines()]
            if len(values)!=len(inputs):
                raise ValueError('Incomplete output')
            outputs.append(values)
        failures = [dict(row=i,input=inputs[i],expected=e,actual=v)
                    for i,(e,v) in enumerate(zip(*outputs)) if e!=v]
        checks[name] = dict(rows=len(inputs),failures=len(failures),examples=failures[:10])
        fixtures[name] = [dict(input=x,expected=y) for x,y in zip(inputs,outputs[0])]
    write_json(out/'golden.json',fixtures)
    passed = all(c['failures']==0 for c in checks.values())
    write_json(out/'summary.json',dict(scope=__doc__,finished=True,passed=passed,checks=checks,
        build_command=cmd,cpp_sha256=digest(a.cpp.read_bytes()),pascal_sha256=digest(pascal.read_bytes()),
        source_sha256={str(p):digest(p.read_bytes()) for p in paths+[generated,Path(__file__).resolve()]}))
    print(json.dumps(checks))
    return not passed


if __name__=='__main__':
    raise SystemExit(main())
