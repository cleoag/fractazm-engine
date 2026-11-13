"""Check native radius and numerical DE against extracted Pascal expressions.

Finite boundaries, mixed NaN signs/payloads, and seeded exponent ranges. The
reference uses FPC -CfSSE2, with the original Single scale and offset types.
"""
import argparse
import itertools
import json
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
    ap.add_argument('--header', type=Path)
    ap.add_argument('--fpc', default='fpc')
    a = ap.parse_args()
    work, out = a.pascal_work_root.resolve(), a.output.resolve()
    immutable = (ROOT/'mb3d').resolve()
    if work == immutable or immutable in work.parents:
        raise ValueError('Immutable Pascal tree')
    out.mkdir(parents=True, exist_ok=False)
    paths = [immutable/n for n in ('formulas.pas','Calc.pas','TypeDefinitions.pas')]
    formulas, calc, types = [p.read_text(encoding='latin1') for p in paths]
    start = formulas.rindex('procedure doHybridPas(')
    radius = re.search(r'Rout\s*:=\s*([^;]+);',formulas[start:])[1]
    de = re.findall(r'Result := bufRout \* Ln\(bufRout\)[^;]+;',calc)
    if len(de)!=1:
        raise ValueError('DE expression changed')
    denominator = re.search(r'/ \((.+)\);',de[0])[1]
    for name in ('dDEscale','mctDEoffset006'):
        if not re.search(r'\b'+name+r':\s*Single;',types):
            raise ValueError('Parameter type changed: '+name)
    pas = '''program radius_de_probe_generated;
{$mode delphi}
uses Math;
type TData=array[0..5] of Double;
function Radius(x,y,z:Double):Double;
begin Result := '''+radius+'''; end;
function RawDE(bufRout,dt,wt,Rst:Double; dDEscale,mctDEoffset006:Single):Double;
begin '''+de[0]+''' end;
function Denominator(dt,wt,Rst:Double; mctDEoffset006:Single):Double;
begin Result := '''+denominator+'''; end;
var data:TData; bits:QWord; i:Integer; r,d,e:Double;
begin
SetExceptionMask([exInvalidOp,exDenormalized,exZeroDivide,exOverflow,exUnderflow,exPrecision]);
Set8087CW($137f);
while not EOF(Input) do begin
for i:=0 to 5 do begin Read(bits); Move(bits,data[i],8); end; ReadLn;
r:=Radius(data[0],data[1],data[2]);
d:=RawDE(data[0],data[1],data[2],data[3],data[4],data[5]);
e:=Denominator(data[1],data[2],data[3],data[5]);
WriteLn(PQWord(@r)^,' ',PQWord(@d)^,' ',PQWord(@e)^);
end; end.
'''
    generated = work/'radius_de_probe_generated.pas'
    if generated.exists() and not generated.read_text().startswith('program radius_de_probe_generated;'):
        raise ValueError('Unrelated source')
    generated.write_text(pas)
    (out/'reference.pas').write_text(pas)
    cmd = [a.fpc,'-Mdelphi','-O2','-CfSSE2','-al','-FE'+str(out),'-FU'+str(out),str(generated)]
    p = subprocess.run(cmd,capture_output=True,timeout=60)
    (out/'build.log').write_bytes(p.stdout+p.stderr)
    if p.returncode:
        raise RuntimeError('Pascal build failed')
    db = lambda x: struct.unpack('<Q',struct.pack('<d',x))[0]
    single = lambda x: struct.unpack('<f',struct.pack('<f',x))[0]
    special = [db(x) for x in (0.,-0.,1.,-1.,math.inf,-math.inf,1e-200,1e200)]
    special += [0x7ff8000000000001,0xfff8000000000002,0x7ff0000000000003,0xfff0000000000004]
    tail = [db(1.),db(single(.004)),db(single(.000133406254463))]
    rows = [list(xyz)+tail for xyz in itertools.product(special,repeat=3)]
    rng = random.Random(0x524144)
    rows += [[db(10**rng.uniform(-150,150)) for _ in range(4)]+
             [db(single(10**rng.uniform(-20,10))),db(single(10**rng.uniform(-20,0)))]
             for _ in range(512)]
    stdin = ''.join(' '.join(map(str,row))+'\n' for row in rows)
    (out/'inputs.txt').write_text(stdin)
    pascal = out/'radius_de_probe_generated.exe'
    results = []
    for side,exe,flags in [('pascal',pascal,[]),('cpp',a.cpp,['--radius-de'])]:
        p = subprocess.run([str(exe.resolve()),*flags],input=stdin,text=True,capture_output=True,timeout=30)
        if p.returncode:
            raise RuntimeError(side+': '+p.stderr)
        (out/(side+'.txt')).write_text(p.stdout)
        values = [[int(w) for w in v.split()] for v in p.stdout.splitlines()]
        if len(values)!=len(rows) or any(len(v)!=3 for v in values):
            raise ValueError('Incomplete '+side+' output')
        results.append(values)
    failures = [dict(row=i,input=rows[i],expected=e,actual=v)
                for i,(e,v) in enumerate(zip(*results)) if e!=v]
    summary = dict(scope=__doc__,finished=True,passed=not failures,rows=len(rows),
        failures=len(failures),examples=failures[:20],build_command=cmd,
        lane_failures={name:sum(e[i]!=v[i] for e,v in zip(*results))
                       for i,name in enumerate(('radius','raw_de','denominator'))},
        cpp_sha256=digest(a.cpp.read_bytes()),pascal_sha256=digest(pascal.read_bytes()),
        source_sha256={str(p):digest(p.read_bytes()) for p in paths+[generated,Path(__file__).resolve()]})
    write_json(out/'golden.json',[dict(input=x,expected=y) for x,y in zip(rows,results[0])])
    write_json(out/'summary.json',summary)
    if a.header:
        text = '// Extracted Pascal expressions; see check_radius_de.py.\n'
        text += 'struct RadiusDEReference { uint64_t input[6], expected[3]; };\n'
        text += 'static constexpr RadiusDEReference radiusDEReferences[] = {\n'
        text += ''.join('{{'+','.join(str(v)+'ULL' for v in row)+'},{'+
                        ','.join(str(v)+'ULL' for v in expected)+'}},\n'
                        for row,expected in zip(rows,results[0]))
        a.header.write_text(text+'};\n')
    print(json.dumps({k:summary[k] for k in ('rows','failures','passed','lane_failures','examples')}))
    return bool(failures)


if __name__=='__main__':
    raise SystemExit(main())
