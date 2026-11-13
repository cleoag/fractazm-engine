"""Compare packed normals with independently extracted Pascal x87 assembly.

Includes singular/nonfinite vectors, large/small exponents, coordinate axes,
near half-integer packing boundaries and deterministic random vectors.
"""
import argparse
import itertools
import math
import random
import re
import struct
import subprocess
from pathlib import Path
from external_validation import ROOT, digest, write_json


def main():
    ap=argparse.ArgumentParser(description=__doc__)
    for name in ['cpp','pascal-work-root','output']:
        ap.add_argument('--'+name,type=Path,required=True)
    ap.add_argument('--fpc',default='fpc')
    args=ap.parse_args()
    work,out=args.pascal_work_root.resolve(),args.output.resolve()
    immutable=(ROOT/'mb3d').resolve()
    if work==immutable or immutable in work.parents: raise ValueError('Immutable Pascal tree')
    out.mkdir(parents=True,exist_ok=False)
    original=ROOT/'mb3d/Math3D.pas'; typedefs=ROOT/'mb3d/TypeDefinitions.pas'
    code=original.read_text(encoding='latin1')
    start=code.index('procedure MakeWNormalsFromDVec(',code.index('implementation'))
    procedure=code[start:code.index('\nend;',start)+5]
    constants=[]
    for name in ['d1em100','d32767']:
        matches=re.findall(r'\b'+name+r'\s*:\s*Double\s*=\s*[^;]+;',typedefs.read_text(encoding='latin1'))
        if len(matches)!=1: raise ValueError('Constant changed: '+name)
        constants.append(matches[0])
    source='''program packed_normals_probe_generated;
{$mode delphi}
{$asmmode intel}
uses Math;
type TVec3D=array[0..2] of Double; TPVec3D=^TVec3D;
TLNormals=array[0..2] of SmallInt; TPLNormals=^TLNormals;
const '''+'\n'.join(constants)+'\n'+procedure+'''
var bits:array[0..2] of QWord; v:TVec3D; n:TLNormals;
begin
SetExceptionMask([exInvalidOp,exDenormalized,exZeroDivide,exOverflow,exUnderflow,exPrecision]);
Set8087CW($137F);
while not EOF(Input) do begin
ReadLn(bits[0],bits[1],bits[2]); Move(bits,v,24); MakeWNormalsFromDVec(@n,@v);
WriteLn(n[0],' ',n[1],' ',n[2]);
end; end.
'''
    generated=work/'packed_normals_probe_generated.pas'
    if generated.exists() and not generated.read_text().startswith('program packed_normals_probe_generated;'):
        raise ValueError('Unrelated generated source')
    generated.write_text(source)
    command=[args.fpc,'-Mdelphi','-O2','-CfSSE2','-al','-FE'+str(out),'-FU'+str(out),str(generated)]
    result=subprocess.run(command,capture_output=True,timeout=60)
    (out/'build.log').write_bytes(result.stdout+result.stderr)
    if result.returncode: raise RuntimeError('Pascal build failed')
    values=[0.,-0.,1.,-1.,1e-300,-1e-300,1e-50,1e150,1e300,float('inf'),-float('inf'),float('nan')]
    rows=list(itertools.product(values,repeat=3))
    for integer in range(0,32767,17):
        x=(integer+.5)/32767
        for value in (math.nextafter(x,-math.inf),x,math.nextafter(x,math.inf)):
            rows.append((value,math.sqrt(1-value*value),0.))
    rng=random.Random(0x4e4f524d)
    rows += [tuple(rng.uniform(-1,1)*10**rng.randrange(-300,301) for _ in range(3)) for _ in range(8192)]
    bits=lambda x:struct.unpack('<Q',struct.pack('<d',x))[0]
    stdin=''.join(' '.join(str(bits(x)) for x in row)+'\n' for row in rows)
    (out/'inputs.txt').write_text(stdin)
    pascal=out/'packed_normals_probe_generated.exe'
    def run(exe,flags,label):
        result=subprocess.run([str(exe.resolve()),*flags],input=stdin,text=True,capture_output=True,timeout=60)
        (out/(label+'.txt')).write_text(result.stdout)
        (out/(label+'.stderr.txt')).write_text(result.stderr)
        if result.returncode: raise RuntimeError(label+' execution failed')
        output=[tuple(map(int,line.split())) for line in result.stdout.splitlines()]
        if len(output)!=len(rows) or any(len(v)!=3 for v in output): raise ValueError('Incomplete output')
        return output
    expected,actual=run(pascal,[],'pascal'),run(args.cpp,['--pack-normals'],'cpp')
    failures=[i for i,(a,b) in enumerate(zip(expected,actual)) if a!=b]
    report=dict(scope=__doc__,rows=len(rows),exact=not failures,failed_rows=len(failures),
                examples=[dict(input_bits=[bits(v) for v in rows[i]],expected=expected[i],actual=actual[i]) for i in failures[:20]],
                command=command,inputs_sha256=digest(stdin.encode()),
                sources={str(p):digest(p.read_bytes()) for p in [original,typedefs,generated,Path(__file__)]},
                binaries={str(p):digest(p.read_bytes()) for p in [pascal,args.cpp]})
    write_json(out/'summary.json',report)
    print({k:report[k] for k in ['rows','exact','failed_rows']})
    return int(bool(failures))


if __name__=='__main__': raise SystemExit(main())
