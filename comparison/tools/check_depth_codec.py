"""Exact packed depth/roughness comparison against unmodified Calc.pas assembly.

Covers nonfinite signs, overflow/wrap, rounding boundaries, roughness and a
seeded finite sample. This is finite evidence, not exhaustive real arithmetic.
"""
import argparse
import math
import random
import struct
import subprocess
from pathlib import Path
from external_validation import ROOT, digest, write_json


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--cpp', type=Path, required=True)
    ap.add_argument('--pascal-work-root', type=Path, required=True)
    ap.add_argument('--output', type=Path, required=True)
    ap.add_argument('--fpc', required=True)
    a = ap.parse_args()
    work, out = a.pascal_work_root.resolve(), a.output.resolve()
    immutable = (ROOT/'mb3d').resolve()
    if work == immutable or immutable in work.parents:
        raise ValueError('Immutable Pascal tree')
    out.mkdir(parents=True, exist_ok=False)
    original = ROOT/'mb3d/Calc.pas'
    text = original.read_text(encoding='latin1')
    start = text.index('procedure CalcZposAndRough(siLight: TPsiLight5;')
    # Skip the interface declaration: select the implementation containing ASM.
    start = text.index('procedure CalcZposAndRough(siLight: TPsiLight5;', start+1)
    end = text.index('\nprocedure DelayCalcPart', start)
    snippet = text[start:end]
    source = '''program depth_codec_probe_generated;
{$mode delphi}
{$asmmode intel}
uses Math;
type TsiLight5 = packed record bytes: array[0..17] of Byte; end;
     TPsiLight5 = ^TsiLight5;
     TMCTparameter = packed record Zcorr,ZcMul: Double;
       iSmNormals: Word; sRoughness: Single; end;
     PMCTparameter = ^TMCTparameter;
const s255: Single = 255;
''' + snippet + '''
var z,c,m: QWord; r,s,i: Cardinal; depth: Double;
    light: TsiLight5; params: TMCTparameter; guard: Boolean;
begin
SetExceptionMask([exInvalidOp,exDenormalized,exZeroDivide,exOverflow,exUnderflow,exPrecision]);
Set8087CW($137F);
while not EOF(Input) do begin
ReadLn(z,c,m,r,s); Move(z,depth,8); Move(c,params.Zcorr,8);
Move(m,params.ZcMul,8); Move(r,params.sRoughness,4); params.iSmNormals:=s;
FillChar(light,SizeOf(light),$A5); CalcZposAndRough(@light,@params,depth);
guard:=True;
for i:=0 to 17 do if ((i<6) or (i>=10)) and (light.bytes[i]<>$A5) then guard:=False;
WriteLn(PCardinal(@light.bytes[6])^,' ',Ord(guard));
end; end.
'''
    generated = work/'depth_codec_probe_generated.pas'
    if generated.exists() and not generated.read_text().startswith('program depth_codec_probe_generated;'):
        raise ValueError('Unrelated generated source')
    generated.write_text(source)
    command = [a.fpc,'-Mdelphi','-O2','-CfSSE2','-al','-FE'+str(out),'-FU'+str(out),str(generated)]
    built = subprocess.run(command,capture_output=True,timeout=60)
    (out/'build.log').write_bytes(built.stdout+built.stderr)
    if built.returncode: raise RuntimeError('Pascal build failed')
    db = lambda x: struct.unpack('<Q',struct.pack('<d',x))[0]
    fb = lambda x: struct.unpack('<I',struct.pack('<f',x))[0]
    special = [0,1,0x8000000000000000,0x8000000000000001,
               0x7ff0000000000000,0xfff0000000000000,0x7ff8000000000000,
               0xfff8000000000000,0x7ff0000000000001,0xfff0000000000001,
               0x7fefffffffffffff,0xffefffffffffffff,db(1),db(-1)]
    rows = [(z,c,m,fb(.5),s) for z in special for c,m in
            [(db(.020831826325),db(2342313.590271704)),(db(1),db(1)),(db(-1),db(1)),(db(0),db(0))]
            for s in [0,1,8,127,128,255,256,257]]
    for n in [0,1,2,255,256,32767,8388351,8388352,2147483646,2147483647,2147483648]:
        for t in [float(n),n+.5]:
            for m in [math.nextafter(t,-math.inf),t,math.nextafter(t,math.inf)]:
                rows.append((db(3),db(1),db(m),0,0))
    for r in [0,fb(-1),fb(.5),fb(1),fb(2),0x7f800000,0xff800000,0x7fc00000,0xffc00000]:
        for s in [0,1,127,128,255,256,257]: rows.append((db(3),db(1),db(1),r,s))
    rng=random.Random(0xD3C0DE)
    for _ in range(8192):
        rows.append((db(rng.uniform(-10,1e5)),db(rng.uniform(-.001,.1)),
                     db(rng.uniform(-1e7,1e7)),fb(rng.uniform(0,1)),rng.choice([0,1,8])))
    stdin=''.join(' '.join(map(str,r))+'\n' for r in rows)
    (out/'inputs.txt').write_text(stdin)
    pascal=out/'depth_codec_probe_generated.exe'
    def run(exe,flags,label):
        result=subprocess.run([str(exe.resolve()),*flags],input=stdin,text=True,capture_output=True,timeout=60)
        (out/(label+'.txt')).write_text(result.stdout)
        (out/(label+'.stderr.txt')).write_text(result.stderr)
        if result.returncode: raise RuntimeError(label+' failed')
        values=[tuple(map(int,line.split())) for line in result.stdout.splitlines()]
        if len(values)!=len(rows): raise ValueError('Incomplete '+label)
        return values
    expected=run(pascal,[],'pascal'); actual=run(a.cpp,['--depth-codec'],'cpp')
    failures=[i for i,(x,y) in enumerate(zip(expected,actual)) if x!=y or x[1]!=1]
    report=dict(scope=__doc__,rows=len(rows),failed_rows=len(failures),exact=not failures,
                examples=[dict(input=rows[i],expected=expected[i],actual=actual[i]) for i in failures[:30]],
                command=command,inputs_sha256=digest(stdin.encode()),
                extracted_assembly_sha256=digest(snippet.encode()),
                sources={str(p):digest(p.read_bytes()) for p in [original,generated,Path(__file__)]},
                binaries={str(p):digest(p.read_bytes()) for p in [pascal,a.cpp]})
    write_json(out/'summary.json',report)
    print({k:report[k] for k in ['rows','failed_rows','exact']})
    return int(bool(failures))


if __name__=='__main__':
    raise SystemExit(main())
