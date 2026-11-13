"""Extract ordinary Pascal spline samplers and SSE2 converters as a scalar oracle."""
import argparse
import hashlib
import json
from pathlib import Path
import struct
import subprocess
from capture_map_preprocessing import function

ROOT=Path(__file__).resolve().parents[2]
WORK=Path('C:/work/_c/mb3d_pascal_work/mb3d_work/map_sampling_oracle')
FPC=Path('C:/FPC/3.2.2/bin/i386-Win32/fpc.exe')


def main():
    ap=argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--output',type=Path,required=True)
    ap.add_argument('--fixture',type=Path,required=True)
    args=ap.parse_args();args.output.mkdir(parents=True,exist_ok=False);WORK.mkdir(parents=True,exist_ok=True)
    paths=[ROOT/'mb3d/Math3D.pas',ROOT/'mb3d/ImageProcess.pas',ROOT/'mb3d/Calc.pas']
    math,img,calc=[p.read_text() for p in paths]
    helpers='\n'.join(function(math,sig) for sig in [
        'function Clamp01S(const sv: Single): Single;',
        'function MakeSplineCoeff(const xs: Double): TSVec;',
        'procedure ScaleSVectorV(V1: TPSVec; const s: Single);'])
    helpers+='\n'+'\n'.join(function(img,sig) for sig in [
        'function ColToSVecFlipRBc4(c: T4Cardinal): T4SVec;',
        'procedure ColToSVecSSE2(c: T4Cardinal; sv, svout: TPSVec);',
        'procedure ColToSVecSqrSSE2(c: T4Cardinal; sv, svout: TPSVec);',
        'procedure ColToSVecSqrSSE2_16(c: T4Cardinal; sv, svout: TPSVec);',
        'procedure ColToSVecSSE2_16(c: T4Cardinal; sv, svout: TPSVec);',
        'function ColToSVecFlipRBc4sqr(c: T4Cardinal): T4SVec;',
        'function ColToSVecFlipRBc4sqr16(c: T4Cardinal): T4SVec;',
        'function ColToSVecFlipRBc416(c: T4Cardinal): T4SVec;',
        'function GetLightMapPixel(const x, y: Single; LM: TPLightMap; bSqr: LongBool; WrapAround: Integer): TSVec;'])
    helpers+='\n'+function(calc,'function SplineIpolMap(x, y: Double; PCalcMap: TPLightMap): TVec3D;')
    pas=WORK/'map_sampling_oracle.pas'
    pas.write_text('''program map_sampling_oracle;
{$mode delphi}{$R-}{$Q-}
uses SysUtils, Math;
type TSVec=array[0..3] of Single; TPSVec=^TSVec; T4SVec=array[0..3] of TSVec;
 T3SVec=array[0..2] of TSVec; T4Cardinal=array[0..3] of Cardinal; TVec3D=array[0..2] of Double;
 TCol2SVec=procedure(c:T4Cardinal; sv,svout:TPSVec); TCol2SVecX87=function(c:T4Cardinal):T4SVec;
 TLightMap=record LMWidth,LMHeight,iMapType,iLMstart:Integer; sLMXfactor,sLMYfactor,sIntensity:Single; LMa:array of Cardinal; end;
 TPLightMap=^TLightMap;
const d1d6:Double=1/6; d1d256:Double=1/256; s05:Single=0.5; s1d255:Single=1/255;
 sva1:array[0..3]of Integer=($ff,$ff,$ff,$ff); sva16:array[0..3]of Integer=($ffff,$ffff,$ffff,$ffff);
 SupportSSE2:LongBool=True;
''' + helpers + '''
var lm:TLightMap; mode,typ,wrap,sq,x,y,c:Integer; a,b:QWord; dx,dy:Double; sx,sy:Single;
 p:PWord; sv:TSVec; dv:TVec3D;
begin
 SetExceptionMask([exInvalidOp,exDenormalized,exZeroDivide,exOverflow,exUnderflow,exPrecision]);
 while not Eof do begin
  ReadLn(mode,typ,wrap,sq,a,b); Move(a,dx,8); Move(b,dy,8);
  lm.LMWidth:=8;lm.LMHeight:=6;lm.iMapType:=typ;lm.sLMXfactor:=8;lm.sLMYfactor:=6;lm.sIntensity:=0.7;
  SetLength(lm.LMa,0);SetLength(lm.LMa,(9*7*(4+2*typ)+8+3) div4);FillChar(lm.LMa[0],Length(lm.LMa)*4,0);
  lm.iLMstart:=Integer(@lm.LMa[0]);
  for y:=0 to6 do for x:=0 to8 do
   if typ=0 then lm.LMa[y*9+x]:=((x*17+y*31) and255) or (((x*53+y*7) and255) shl8) or (((x*11+y*67) and255) shl16)
   else begin p:=PWord(lm.iLMstart+(y*9+x)*6);for c:=0 to2 do begin p^:=(x*1709+y*2741+c*13337) and65535;Inc(p);end;end;
  if mode=0 then begin sx:=dx;sy:=dy;sv:=GetLightMapPixel(sx,sy,@lm,sq<>0,wrap);
   for c:=0 to2 do dv[c]:=sv[c];end else dv:=SplineIpolMap(dx,dy,@lm);
  WriteLn(mode,' ',typ,' ',wrap,' ',sq,' ',IntToHex(a,16),' ',IntToHex(b,16),' ',
    IntToHex(PQWord(@dv[0])^,16),' ',IntToHex(PQWord(@dv[1])^,16),' ',IntToHex(PQWord(@dv[2])^,16));
 end;
end.
'''.replace('div4','div 4').replace('to6','to 6').replace('to8','to 8').replace('to2','to 2').replace('and255','and 255').replace('and65535','and 65535').replace('shl8','shl 8').replace('shl16','shl 16'))
    command=[str(FPC),'-Mdelphi','-CfSSE2','-O2','-al',str(pas)]
    build=subprocess.run(command,cwd=WORK,capture_output=True,timeout=60);(args.output/'build.log').write_bytes(build.stdout+build.stderr);build.check_returncode()
    bits=lambda x:struct.unpack('<Q',struct.pack('<d',x))[0]
    coords=(-.1,0.,.001,.3,.7,.999,1.,1.1)
    cases=[(0,t,w,s,bits(x),bits(y)) for t in (0,1) for w in (0,1,2) for s in (0,1) for x in coords for y in coords]
    cases += [(1,t,1,0,bits(x*8),bits(y*6)) for t in(0,1) for x in coords[1:-1] for y in coords[1:-1]]
    inputs=''.join(' '.join(map(str,r))+'\n' for r in cases)
    (args.output/'inputs.txt').write_text(inputs)
    run=subprocess.run([str(pas.with_suffix('.exe'))],cwd=WORK,input=inputs.encode(),capture_output=True,timeout=15)
    (args.output/'output.txt').write_bytes(run.stdout+run.stderr);run.check_returncode()
    rows=[line.split() for line in run.stdout.decode().splitlines()]; assert len(rows)==len(cases)
    fixture=['// Original Calc/ImageProcess samplers and x87/SSE2 helpers, FPC i386 SSE2.',
        'struct MapSamplingFixture { int mode,type,wrap,squared; uint64_t x,y,r,g,b; };',
        'static const MapSamplingFixture kMapSamplingFixtures[] = {']
    fixture+=['    {'+', '.join(v if i<4 else '0x'+v+'ull' for i,v in enumerate(r))+'},' for r in rows]
    args.fixture.write_text('\n'.join(fixture+['};','']))
    for p in(pas,pas.with_suffix('.exe'),pas.with_suffix('.s')):(args.output/p.name).write_bytes(p.read_bytes())
    identities={str(p):hashlib.sha256(p.read_bytes()).hexdigest() for p in paths+[pas,pas.with_suffix('.exe'),FPC,FPC.with_name('ppc386.exe'),Path(__file__),args.fixture]}
    (args.output/'identity.json').write_text(json.dumps(dict(rows=len(rows),command=command,sha256=identities,
        scope='RGB8/RGB16 ordinary light spline wrap0/1/2, linear/squared, clamps/seams and formula spline, finite asymmetric buffers'),indent=2)+'\n')
    print(json.dumps(dict(rows=len(rows),fixture=str(args.fixture))))


if __name__=='__main__':main()
