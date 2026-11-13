"""Build independent original-Pascal map preparation fixtures (no renderer)."""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess

ROOT = Path(__file__).resolve().parents[2]
WORK = Path('C:/work/_c/mb3d_pascal_work/mb3d_work/map_preprocessing_oracle')
FPC = Path('C:/FPC/3.2.2/bin/i386-Win32/fpc.exe')


def function(text, signature):
    start = text.index(signature, text.index('implementation'))
    # These helpers end at an unindented end; their inner blocks are indented.
    return text[start:text.index('\nend;', start)+5]


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--output', type=Path, required=True)
    ap.add_argument('--fixture', type=Path, required=True)
    args = ap.parse_args()
    args.output.mkdir(parents=True, exist_ok=False)
    WORK.mkdir(parents=True, exist_ok=True)
    paths = [ROOT/'mb3d/maps/MB3DMaps.pas', ROOT/'mb3d/Math3D.pas', ROOT/'mb3d/DivUtils.pas']
    maps, math, div = (p.read_text() for p in paths)
    helpers = '\n'.join(function(math, sig) for sig in [
        'function ArcSinSafe(const d: Double): Double;',
        'function MinCS(const s1, s2: Single): Single;',
        'function MinMaxSVecSSE(const smin, smax: Single; const V1: TSVec): TSVec;',
        'function AddSVectors(const V1, V2: TSVec): TSVec;  overload;'])
    helpers += '\n' + '\n'.join(function(div, sig) for sig in [
        'function ColToSVecScale(c: Cardinal; scale: Single): TSVec;',
        'function WordColToSVecScale(w3: TP3word; scale: Single): TSVec;',
        'function SVecToWordCol(sv: TSVec): T3word;',
        'function SVecToColNoScale(sv: TSVec): Cardinal;'])
    kernels = maps[maps.index('function ClipR(s: Single)'):maps.index('procedure Make3Word')]
    source = '''program map_preprocessing_oracle;
{$mode delphi}{$R-}{$Q-}
uses SysUtils, Math;
type TSVec=array[0..3] of Single; T3word=array[0..2] of Word; TP3word=^T3word;
 TLightMap=record LMWidth,LMHeight,iMapType,iLMstart:Integer; LMa:array of Cardinal; end;
 TMinMax=function(const smin,smax:Single; const V1:TSVec):TSVec;
const dNearOne:Double=0.9999999; dNearMinusOne:Double=-0.9999999;
var mMinMaxSVec:TMinMax;
procedure FastMove(const src; var dst; count:Integer); begin Move(src,dst,count); end;
''' + helpers + '\n' + kernels + '''
var lm:TLightMap; sizeIndex,typ,flags,w,h,n,i,x,y,c,value:Integer; p:PWord; f:File; name:String;
begin
 SetExceptionMask([exInvalidOp,exDenormalized,exZeroDivide,exOverflow,exUnderflow,exPrecision]);
 mMinMaxSVec:=MinMaxSVecSSE;
 for sizeIndex:=0 to 1 do for typ:=0 to 1 do for flags:=0 to 7 do begin
  if sizeIndex=0 then begin w:=8; h:=8; end else begin w:=32; h:=16; end;
  n:=(w+1)*(h+1)*(4+2*typ)+8;
  SetLength(lm.LMa,0); SetLength(lm.LMa,(n+3) div 4); FillChar(lm.LMa[0],Length(lm.LMa)*4,0);
  lm.LMWidth:=w; lm.LMHeight:=h; lm.iMapType:=typ; lm.iLMstart:=Integer(@lm.LMa[0]);
  for y:=0 to h do for x:=0 to w do begin
   if typ=0 then lm.LMa[y*(w+1)+x]:=((x mod w)*17+(y mod h)*31) and 255 or
    ((((x mod w)*53+(y mod h)*7) and 255) shl 8) or ((((x mod w)*11+(y mod h)*67) and 255) shl 16)
   else begin p:=PWord(lm.iLMstart+(y*(w+1)+x)*6);
    for c:=0 to 2 do begin p^:=((x mod w)*1709+(y mod h)*2741+c*13337) and 65535; Inc(p); end;
   end;
  end;
  SmoothLightMap(lm,(flags and 1)<>0,(flags and 2)<>0); FitLMborder(lm,flags and 4);
  name:=Format('%d_%d_%d.bin',[sizeIndex,typ,flags]); AssignFile(f,name); Rewrite(f,1);
  BlockWrite(f,lm.LMa[0],n); CloseFile(f); WriteLn(name,' ',n);
 end;
end.
'''
    pas=WORK/'map_preprocessing_oracle.pas'
    pas.write_text(source)
    command=[str(FPC),'-Mdelphi','-CfSSE2','-O2','-al',str(pas)]
    build=subprocess.run(command,cwd=WORK,capture_output=True,timeout=60)
    (args.output/'build.log').write_bytes(build.stdout+build.stderr)
    build.check_returncode()
    run=subprocess.run([str(pas.with_suffix('.exe'))],cwd=WORK,capture_output=True,timeout=15)
    (args.output/'output.txt').write_bytes(run.stdout+run.stderr)
    run.check_returncode()
    rows=[]
    for line in run.stdout.decode().splitlines():
        name,n=line.split(); data=(WORK/name).read_bytes(); assert len(data)==int(n)
        (args.output/name).write_bytes(data)
        size,typ,flags=map(int,Path(name).stem.split('_'))
        value=14695981039346656037
        for byte in data: value=((value^byte)*1099511628211)&0xffffffffffffffff
        rows.append(dict(size=size,type=typ,flags=flags,bytes=len(data),fnv1a64=value,sha256=hashlib.sha256(data).hexdigest()))
    fixture=['// Original immutable MB3DMaps kernels, FPC i386 SSE2; full-buffer hashes.',
       'struct MapPreprocessingFixture { int size, type, flags, bytes; uint64_t hash; };',
       'static const MapPreprocessingFixture kMapPreprocessingFixtures[] = {']
    fixture += [f'    {{{r["size"]}, {r["type"]}, {r["flags"]}, {r["bytes"]}, 0x{r["fnv1a64"]:016x}ull}},' for r in rows]
    args.fixture.write_text('\n'.join(fixture+['};','']))
    for p in (pas,pas.with_suffix('.exe'),pas.with_suffix('.s')): (args.output/p.name).write_bytes(p.read_bytes())
    identities={str(p):hashlib.sha256(p.read_bytes()).hexdigest() for p in paths+[pas,pas.with_suffix('.exe'),FPC,FPC.with_name('ppc386.exe'),Path(__file__),args.fixture]}
    (args.output/'identity.json').write_text(json.dumps(dict(rows=rows,command=command,sha256=identities,
        scope='32 finite full buffers: 2 dimensions x RGB8/RGB16 x all smooth/spherical/border flag combinations; no universal numeric proof'),indent=2)+'\n')
    print(json.dumps(dict(rows=len(rows),fixture=str(args.fixture))))


if __name__=='__main__': main()
