"""Compare the CalcN decision kernel with the original extracted Pascal body.

Explicit eight-byte tails define the reference's out-of-image reads. These
measurements certify the decision kernel for those buffers, not GUI scheduling,
undefined tail contents of real Pascal images, or progressive render parity.
"""
import argparse
from collections import Counter
import random
import re
import struct
import subprocess
from pathlib import Path
from check_mc_sampling import extract
from external_validation import ROOT, digest, write_json


def main():
    ap=argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--pascal-work-root',type=Path,required=True)
    ap.add_argument('--cpp',type=Path,required=True)
    ap.add_argument('--output',type=Path,required=True)
    ap.add_argument('--fpc',default='fpc')
    a=ap.parse_args()
    work,out=a.pascal_work_root.resolve(),a.output.resolve()
    immutable=(ROOT/'mb3d').resolve()
    if work==immutable or immutable in work.parents: raise ValueError('Immutable Pascal tree')
    out.mkdir(parents=True,exist_ok=False)
    paths=[ROOT/'mb3d'/p for p in ['CalcMonteCarlo.pas','DivUtils.pas','Math3D.pas','TypeDefinitions.pas']]
    mc,div,maths,types=[p.read_text(encoding='latin1') for p in paths]
    snippets={
        'decision':extract(mc,'function TMCCalcThread.CalcN(', 'procedure TMCCalcThread.CalcNormalsOnCutMC('),
        'decode':extract(div,'function MCRGBtoDouble(', 'function DoubleToMCRGB('),
        'clamp':extract(maths,'function Clamp0D(', 'function Clamp01S('),
        'minmax':extract(maths,'function MinMaxCD(', 'function MinCD('),
        'minimum':extract(maths,'function MinCD(', 'function MaxCD('),
        'maximum':extract(maths,'function MaxCD(', 'function MaxAbsCD('),
    }
    constants=[]
    for name,kind in [('enat','Double'),('s05','Single')]:
        found=re.findall(r'\b'+name+r'\s*:\s*'+kind+r'\s*=\s*[^;]+;',types)
        if len(found)!=1: raise ValueError('Source constant changed: '+name)
        constants.append(found[0])
    record=types[types.index('  TMCrecord = packed record'):types.index('  TMCrecordNew =')]
    source='''program mc_adaptive_probe_generated;
{$mode delphi}
uses Math, SysUtils;
type TRGB = array[0..2] of Byte; TPRGB = ^TRGB;
'''+record+'\nconst '+'\n'.join(constants)+'''
var MCTparas: record iMandWidth,iMandHeight: Integer; end;
    PMCrecord: TPMCrecord; AvrgVari: Double; AvrgRCount: Single;
    randomValue: Double; used: Integer;
function GetRand: Double;
begin Inc(used); Result:=randomValue; end;
'''+ '\n'.join(snippets[k] for k in ['decode','clamp','minmax','minimum','maximum'])+'\n'+snippets['decision'].replace('TMCCalcThread.CalcN','CalcN',1)+'''
var buffer: array[0..4095] of Byte; x,y,n,i,resultCount: Integer; v: Cardinal;
begin
  while not EOF(Input) do begin
    Read(MCTparas.iMandWidth,MCTparas.iMandHeight,x,y,AvrgVari,AvrgRCount,randomValue,n);
    if (n<18) or (n>4096) then Halt(2);
    for i:=0 to n-1 do begin Read(v); if v>255 then Halt(2); buffer[i]:=v; end;
    ReadLn;
    PMCrecord:=TPMCrecord(@buffer[(y*MCTparas.iMandWidth+x)*18]); used:=0;
    resultCount:=CalcN(x+1,y+1,PMCrecord);
    WriteLn(resultCount,' ',used);
  end;
end.
'''
    generated=work/'mc_adaptive_probe_generated.pas'
    if generated.exists() and not generated.read_text().startswith('program mc_adaptive_probe_generated;'):
        raise ValueError('Unrelated generated file')
    generated.write_text(source)
    command=[a.fpc,'-Mdelphi','-O2','-CfSSE2','-FE'+str(out),'-FU'+str(out),str(generated)]
    p=subprocess.run(command,capture_output=True,timeout=60)
    (out/'build.log').write_bytes(p.stdout+p.stderr)
    if p.returncode: raise RuntimeError('Pascal build failed; see build.log')
    rng=random.Random(0x24563487)
    rows=[]
    def add(w,h,x,y,variance,average,u,buffer,group):
        rows.append((f'{w} {h} {x} {y} {variance:.17g} {average:.9g} {u:.17g} {len(buffer)} '+
                     ' '.join(map(str,buffer)),group))
    # Every edge/corner/interior topology, hit flag and all-zero counts.
    for w,h in [(1,1),(1,3),(3,1),(2,2),(3,3),(7,5)]:
        for y in range(h):
            for x in range(w):
                for flag in [0,128]:
                    for mean,square in [(0,0),(2097152,2097152),(4194303,4194303),(6291456,6291456),(4194303,6291456),(0xffffff,0xffffff)]:
                        for tail in [0,255]:
                            buffer=bytearray(w*h*18+8)
                            for pixel in range(w*h):
                                at=pixel*18
                                buffer[at+9:at+12]=mean.to_bytes(3,'little')
                                buffer[at+12:at+15]=square.to_bytes(3,'little')
                                buffer[at+15:at+17]=(4).to_bytes(2,'little')
                                buffer[at+17]=flag
                            buffer[-8:]=bytes([tail])*8
                            for u in [0.,1/11,0.5,1.]:
                                add(w,h,x,y,.002,4,u,buffer,'topology')
    # Same two real pixels, different explicit tail. The final neighbor's
    # displaced RayCount read depends on bytes outside the image itself.
    for tail in [0,255]:
        buffer=bytearray(2*18+8)
        for pixel,mean in [(0,2097152),(1,4194304)]:
            at=pixel*18
            buffer[at+9:at+12]=mean.to_bytes(3,'little')
            buffer[at+12:at+15]=mean.to_bytes(3,'little')
            buffer[at+15:at+17]=(4).to_bytes(2,'little')
            buffer[at+17]=128
        buffer[-8:]=bytes([tail])*8
        add(2,1,0,0,.002,4,.5,buffer,'explicit_tail')
    for _ in range(10000):
        w,h=rng.choice([(1,1),(1,3),(3,1),(2,2),(3,3),(7,5)])
        buffer=bytes(rng.randrange(256) for _ in range(w*h*18+8))
        add(w,h,rng.randrange(w),rng.randrange(h),10**rng.uniform(-3,4),rng.uniform(0,65535),rng.random(),buffer,'random')
    # Uniform high variance forces the count cap. Counts 1 and 65535 put its
    # Round at half-integers, distinguishing Pascal ties-to-even from round().
    for w,h in [(1,1),(1,3),(3,1),(2,2),(3,3),(7,5)]:
        for y in range(h):
            for x in range(w):
                for flag in [0,128]:
                    for count in [0,1,65535]:
                        buffer=bytearray(w*h*18+8)
                        for pixel in range(w*h):
                            at=pixel*18
                            buffer[at+9:at+12]=(2097152).to_bytes(3,'little')
                            buffer[at+12:at+15]=(0xffffff).to_bytes(3,'little')
                            buffer[at+15:at+17]=count.to_bytes(2,'little')
                            buffer[at+17]=flag
                        add(w,h,x,y,.002,count,.5,buffer,'count_boundary')
    stdin='\n'.join(r[0] for r in rows)+'\n'
    (out/'inputs.txt').write_text(stdin)
    pascal=out/'mc_adaptive_probe_generated.exe'
    def run(exe,flags):
        p=subprocess.run([str(exe.resolve()),*flags],input=stdin,text=True,capture_output=True,timeout=60,cwd=out)
        (out/(exe.stem+'.txt')).write_text(p.stdout)
        if p.returncode: raise RuntimeError(f'{exe} exited {p.returncode}: {p.stderr[:500]}')
        results=[[int(x) for x in r.split()] for r in p.stdout.splitlines()]
        if len(results)!=len(rows) or any(len(r)!=2 for r in results): raise ValueError('Incomplete output')
        return results
    expected,actual=run(pascal,[]),run(a.cpp,['--adaptive'])
    failures=[0,0]; examples=[]; branches=Counter(); tail_results=[]
    for i,(ref,cpp) in enumerate(zip(expected,actual)):
        branches['random_'+str(ref[1])+'_samples_'+('zero' if ref[0]==0 else 'one' if ref[0]==1 else 'many')]+=1
        if rows[i][1]=='explicit_tail': tail_results.append(dict(row=i,reference=ref,cpp=cpp))
        for lane in range(2):
            if ref[lane]!=cpp[lane]:
                failures[lane]+=1
                if len(examples)<20: examples.append(dict(row=i,group=rows[i][1],reference=ref,cpp=cpp))
    report=dict(scope=__doc__,rows=len(rows),exact=not any(failures),failures=failures,
        lanes=['sample_count','random_values_consumed'],branches=dict(branches),examples=examples,explicit_tail=tail_results,
        sources={str(p):digest(p.read_bytes()) for p in paths+[generated,Path(__file__),ROOT/'src/core/calc_monte_carlo.cpp',ROOT/'src/core/calc_monte_carlo.h',ROOT/'src/core/win64_de_x87_helpers.asm',ROOT/'comparison/tools/mc_sampling_probe.cpp']},
        extracted={k:digest(v.encode()) for k,v in snippets.items()},
        binaries={str(p):digest(p.read_bytes()) for p in [pascal,a.cpp]},command=command,inputs_sha256=digest(stdin.encode()))
    write_json(out/'summary.json',report)
    print({k:report[k] for k in ['rows','exact','failures','branches','examples']})
    return int(not report['exact'])


if __name__=='__main__': raise SystemExit(main())
