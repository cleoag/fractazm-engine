"""Compare the iteration-boundary step with the extracted Pascal expression.
Finite special-value and seeded cases; preserves NaN signs and input bits.
"""
import argparse,struct,random,subprocess,json
from pathlib import Path
from external_validation import ROOT,digest,write_json

def main():
    ap=argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--cpp',type=Path,required=True)
    ap.add_argument('--output',type=Path,required=True)
    ap.add_argument('--pascal-work-root',type=Path,required=True)
    ap.add_argument('--fpc',default='C:/FPC/3.2.2/bin/i386-Win32/fpc.exe')
    a=ap.parse_args();out=a.output.resolve();out.mkdir(parents=True,exist_ok=False)
    work=a.pascal_work_root.resolve();immutable=(ROOT/'mb3d').resolve()
    if work==immutable or immutable in work.parents:raise ValueError('Immutable Pascal tree')
    original=immutable/'Calc.pas';s=original.read_text(encoding='latin1')
    start=s.index('          R := LastSI - SmoothItD;',s.rindex('procedure RMdoBinSearchIt'))
    snippet=s[start:s.index('\n        end;',start)]
    source="""program iteration_step_generated;
{$mode delphi}{$asmmode intel}
uses Math;
const s05:Single=0.5;
function Step(YP,LastSI,SmoothItD:Single;dmul:Double):Double;
var R:Single;dT1:Double;
begin
"""+snippet+"""
Result:=dT1;end;
var a,b,c:Cardinal;d:QWord;yp,ls,si:Single;mulv,res:Double;
begin
SetExceptionMask([exInvalidOp,exDenormalized,exZeroDivide,exOverflow,exUnderflow,exPrecision]);
Set8087CW($137f);
while not EOF(Input) do begin
ReadLn(a,b,c,d);Move(a,yp,4);Move(b,ls,4);Move(c,si,4);Move(d,mulv,8);
res:=Step(yp,ls,si,mulv);WriteLn(PQWord(@res)^);
end;end.
"""
    generated=work/'iteration_step_generated.pas';generated.write_text(source)
    cmd=[a.fpc,'-Mdelphi','-O2','-CfSSE2','-al','-FE'+str(out),'-FU'+str(out),str(generated)]
    p=subprocess.run(cmd,capture_output=True);(out/'build.log').write_bytes(p.stdout+p.stderr)
    if p.returncode:raise RuntimeError('Pascal build failed')
    fb=lambda x:struct.unpack('<I',struct.pack('<f',x))[0]
    db=lambda x:struct.unpack('<Q',struct.pack('<d',x))[0]
    special=[0,0x80000000,1,0x80000001,fb(1e-30),fb(60),fb(-462045770153984.0),0x7f7fffff,0xff7fffff,0x7f800000,0xff800000,0x7fc00000,0xffc00000,0x7f800001,0xff800001]
    rows=[(fb(59.01),x,y,db(m)) for x in special for y in special for m in [0.,0.7,1.]]
    rng=random.Random(0x17E2)
    for _ in range(256):rows.append((fb(rng.uniform(0,100)),fb(rng.uniform(-1e4,1e4)),fb(rng.uniform(-1e4,1e4)),db(rng.random())))
    stdin=''.join(' '.join(map(str,row))+'\n' for row in rows);(out/'inputs.txt').write_text(stdin)
    pascal=out/'iteration_step_generated.exe';values=[]
    for label,exe,flags in [('pascal',pascal,[]),('cpp',a.cpp.resolve(),['--iteration-step'])]:
        p=subprocess.run([str(exe),*flags],input=stdin,text=True,capture_output=True,timeout=30)
        (out/(label+'.txt')).write_text(p.stdout);(out/(label+'.stderr.txt')).write_text(p.stderr)
        if p.returncode:raise RuntimeError(label+' failed')
        v=[int(x) for x in p.stdout.splitlines()]
        if len(v)!=len(rows):raise ValueError('Incomplete '+label)
        values.append(v)
    failed=[i for i,(x,y) in enumerate(zip(*values)) if x!=y]
    write_json(out/'golden.json',[dict(input=r,expected=e) for r,e in zip(rows,values[0])])
    report=dict(finished=True,rows=len(rows),failed_rows=len(failed),exact=not failed,command=cmd,
        examples=[dict(row=i,input=rows[i],expected=values[0][i],actual=values[1][i]) for i in failed[:20]],
        extracted_source_sha256=digest(snippet.encode()),sources={str(p):digest(p.read_bytes()) for p in [original,generated,Path(__file__).resolve()]},
        binaries={str(p):digest(p.read_bytes()) for p in [pascal,a.cpp.resolve()]},scope=__doc__)
    write_json(out/'summary.json',report);print({k:report[k] for k in ['rows','failed_rows','exact']})
    return bool(failed)
if __name__=='__main__':raise SystemExit(main())
