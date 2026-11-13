"""Extract immutable ordinary Pascal SSE volume lookups for finite face/seam fixtures."""
import argparse
import hashlib
import json
from pathlib import Path
import struct
import subprocess
from capture_map_preprocessing import function

ROOT=Path(__file__).resolve().parents[2]
WORK=Path('C:/work/_c/mb3d_pascal_work/mb3d_work/volume_sampling_oracle')
FPC=Path('C:/FPC/3.2.2/bin/i386-Win32/fpc.exe')


def main():
    ap=argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--output',type=Path,required=True)
    ap.add_argument('--fixture',type=Path,required=True)
    args=ap.parse_args();args.output.mkdir(parents=True,exist_ok=False);WORK.mkdir(parents=True,exist_ok=True)
    source=ROOT/'mb3d/maps/MB3DMaps.pas';text=source.read_text()
    math_source=ROOT/'mb3d/Math3D.pas'
    record=text[text.index('  TVolumetricLightMap = packed record'):text.index('  TCalcVolumetricLightMapThread =')]
    kernels='\n'.join(function(text,sig) for sig in [
        'function VolLightMapPosSSE(vd: TPVec3D): LongBool;',
        'function GetVolLightMapVecSSE(vd: TPSVec): Single;'])
    kernels+='\n'+function(math_source.read_text(),'function SqrLengthOfSVec(const V: TSVec): Single;')
    pas=WORK/'volume_sampling_oracle.pas'
    pas.write_text('''program volume_sampling_oracle;
{$mode delphi}{$R-}{$Q-}
uses SysUtils, Math;
type TSVec=array[0..3] of Single; TPSVec=^TSVec; TVec3D=array[0..2] of Double; TPVec3D=^TVec3D;
 TSMatrix3=array[0..2] of TSVec;
'''+record+'''
const cAbsSVec:array[0..3] of Cardinal=($7fffffff,$7fffffff,$7fffffff,$7fffffff);
var VolumeLightMap:TVolumetricLightMap;
'''+kernels+'''
var mode,size,rotation,x,y,k:Integer; stretch:Cardinal; a,b,c:QWord; vd:TVec3D; vs:TSVec; value:Single; answer:LongBool; distanceSquared:Double;
begin
 SetExceptionMask([exInvalidOp,exDenormalized,exZeroDivide,exOverflow,exUnderflow,exPrecision]);
 while not Eof do begin
  ReadLn(mode,size,rotation,stretch,a,b,c);Move(a,vd[0],8);Move(b,vd[1],8);Move(c,vd[2],8);
  with VolumeLightMap do begin
   CubeSize:=size;HalfSize:=size div 2;SizeFactor:=HalfSize;CsizeS:=size-1;HSizeS:=HalfSize;
   Move(stretch,StretchSide1,4);SideCount:=6;
   LightPos[0]:=0.125;LightPos[1]:=-0.25;LightPos[2]:=0.5;
   FillChar(RotMatrix,SizeOf(RotMatrix),0);
   if rotation=0 then begin RotMatrix[0,0]:=1;RotMatrix[1,1]:=1;RotMatrix[2,2]:=1;end
   else begin RotMatrix[0,0]:=0.8;RotMatrix[0,2]:=-0.6;RotMatrix[1,1]:=1;RotMatrix[2,0]:=0.6;RotMatrix[2,2]:=0.8;end;
   for k:=0 to 5 do begin SetLength(CubeSides[k],size*size);
    for y:=0 to size-1 do for x:=0 to size-1 do
     if mode=0 then CubeSides[k,x+y*size]:=k*10000+y*100+x+0.25
     else CubeSides[k,x+y*size]:=((x*11+y*7) mod 17)*0.25-2;
   end;
  end;
  for k:=0 to 2 do vs[k]:=vd[k];vs[3]:=0;distanceSquared:=SqrLengthOfSVec(vs);
  if mode=0 then begin value:=GetVolLightMapVecSSE(@vs);end
  else begin answer:=VolLightMapPosSSE(@vd);if answer then value:=1 else value:=0;end;
  WriteLn(mode,' ',size,' ',rotation,' ',IntToHex(stretch,8),' ',IntToHex(a,16),' ',IntToHex(b,16),' ',IntToHex(c,16),' ',IntToHex(PCardinal(@value)^,8),' ',IntToHex(PQWord(@distanceSquared)^,16));
 end;
end.
''')
    command=[str(FPC),'-Mdelphi','-CfSSE2','-O2','-al',str(pas)]
    build=subprocess.run(command,cwd=WORK,capture_output=True,timeout=60)
    (args.output/'build.log').write_bytes(build.stdout+build.stderr);build.check_returncode()
    fbits=lambda x:struct.unpack('<I',struct.pack('<f',x))[0]
    dbits=lambda x:struct.unpack('<Q',struct.pack('<d',x))[0]
    cases=[]
    for size in (7,9,17):
        coords=(-1,-.875,-.5,-.125,0,.125,.5,.875,1)
        for axis in range(3):
            for sign in (-1,1):
                other=[i for i in range(3) if i!=axis]
                for x in coords:
                    for y in coords:
                        d=[0.,0.,0.];d[axis]=sign;d[other[0]]=x;d[other[1]]=y
                        cases.append((0,size,0,fbits(1.),*map(dbits,d)))
        # Values immediately across a Double-to-Single boundary must select
        # the face from rounded Single components, as the actual caller does.
        for d in [(1.,1.+2**-25,.125),(-1.,-1.-2**-25,-.125),(.5,1.,1.+2**-25),
                  (1.00000001,.50000001,.12500001),(-1.00000001,.50000001,-.12500001)]:
            cases.append((0,size,0,fbits(1.),*map(dbits,d)))
    coords=(-20,-5.5,-3.5,-2.5,-1.5,-.5,0,.5,1.5,2.5,3.5,5.5,20)
    for size in (7,9,17):
        for rotation in (0,1):
            for stretch in (.5,1.):
                for i,x in enumerate(coords):
                    for y in (coords[(i+3)%len(coords)],coords[(i+7)%len(coords)]):
                        for z in (-4,-.75,0,.75,4):
                            d=(x+.125,y-.25,z+.5)
                            cases.append((1,size,rotation,fbits(stretch),*map(dbits,d)))
    inputs=''.join(' '.join(map(str,row))+'\n' for row in cases)
    (args.output/'inputs.txt').write_text(inputs)
    run=subprocess.run([str(pas.with_suffix('.exe'))],cwd=WORK,input=inputs.encode(),capture_output=True,timeout=15)
    (args.output/'output.txt').write_bytes(run.stdout+run.stderr);run.check_returncode()
    rows=[line.split() for line in run.stdout.decode().splitlines()];assert len(rows)==len(cases)
    fixture=['// Immutable MB3DMaps SSE samplers; original i386 FPC3.2.2 code.',
       'struct VolumeSamplingFixture { int mode,size,rotation; uint32_t stretch; uint64_t x,y,z; uint32_t expected; uint64_t squaredLength; };',
       'static const VolumeSamplingFixture kVolumeSamplingFixtures[] = {']
    fixture += ['    {'+', '.join(v if i<3 else '0x'+v+('ull' if 4<=i<=6 or i==8 else 'u') for i,v in enumerate(row))+'},' for row in rows]
    args.fixture.write_text('\n'.join(fixture+['};','']))
    for p in (pas,pas.with_suffix('.exe'),pas.with_suffix('.s')):(args.output/p.name).write_bytes(p.read_bytes())
    identities={str(p):hashlib.sha256(p.read_bytes()).hexdigest() for p in [source,math_source,pas,pas.with_suffix('.exe'),FPC,FPC.with_name('ppc386.exe'),Path(__file__),args.fixture]}
    (args.output/'identity.json').write_text(json.dumps(dict(rows=len(rows),positional=sum(r[0]==0 for r in cases),
        directional=sum(r[0]==1 for r in cases),command=command,sha256=identities,
        scope='Six signed faces, axis tie priority, half-even texels and Single conversion; directional rotated/unrotated clamps and odd/even halfsizes'),indent=2)+'\n')
    print(json.dumps(dict(rows=len(rows),fixture=str(args.fixture))))


if __name__=='__main__':main()
