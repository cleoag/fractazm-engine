"""Extract immutable Pascal visible-light kernels into an independent i386 oracle."""
import argparse
import hashlib
import json
from pathlib import Path
import struct
import subprocess

ROOT = Path(__file__).resolve().parents[2]
WORK = Path('C:/work/_c/mb3d_pascal_work/mb3d_work/visible_light_oracle')
FPC = Path('C:/FPC/3.2.2/bin/i386-Win32/fpc.exe')


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--output', type=Path, required=True)
    ap.add_argument('--fixture', type=Path, required=True)
    ap.add_argument('--include-reserved', action='store_true', help='Also capture reachable raw default shapes10/12/14')
    args = ap.parse_args()
    args.output.mkdir(parents=True, exist_ok=False)
    WORK.mkdir(parents=True, exist_ok=True)
    math_source = ROOT / 'mb3d/Math3D.pas'
    paint_source = ROOT / 'mb3d/PaintThread.pas'
    math_text = math_source.read_text()
    paint_text = paint_source.read_text()
    start = math_text.index('function FastPow(const x, y: Single): Single;  //used by vis light 3')
    fastpow = math_text[start:math_text.index('\nend;', start) + len('\nend;')]
    start = paint_text.index('procedure CalcPosLightShape(var flux, transp: Single; LightNr: Integer; PLVals: TPLightVals);', paint_text.index('implementation'))
    shape = paint_text[start:paint_text.index('\nfunction GetDiffMapWrap3D', start)]
    pas = WORK / 'visible_light_math_oracle.pas'
    pas.write_text('''program visible_light_math_oracle;
{$mode delphi}{$R-}{$Q-}
uses SysUtils, Math;
type TLightVals = record iLightPos: array[0..5] of Integer; bCalcPixColSqr: LongBool; end;
 TPLightVals = ^TLightVals;
function Max0S(x: Single): Single;
begin if x<0 then Result:=0 else Result:=x; end;
''' + fastpow + '\n' + shape + '''
var a,b,c: LongWord; opt,sq: Integer; x,y,z: Single; lv:TLightVals;
begin
 SetExceptionMask([exInvalidOp,exDenormalized,exZeroDivide,exOverflow,exUnderflow,exPrecision]);
 while not Eof do begin
  ReadLn(a,b,opt,sq); Move(a,x,4); Move(b,y,4);
  z:=FastPow(x,50); c:=PLongWord(@z)^;
  lv.iLightPos[0]:=opt; lv.bCalcPixColSqr:=sq<>0;
  CalcPosLightShape(x,y,0,@lv);
  WriteLn(IntToHex(a,8),' ',IntToHex(b,8),' ',opt,' ',sq,' ',
    IntToHex(PLongWord(@x)^,8),' ',IntToHex(PLongWord(@y)^,8),' ',IntToHex(c,8));
 end;
end.
''')
    command = [str(FPC), '-Mdelphi', '-CfSSE2', '-O2', '-al', str(pas)]
    build = subprocess.run(command, cwd=WORK, capture_output=True, timeout=60)
    (args.output / 'build.log').write_bytes(build.stdout + build.stderr)
    build.check_returncode()
    bits = lambda x: struct.unpack('<I', struct.pack('<f', x))[0]
    cases = [(bits(radius*ratio), bits(radius), shape | positional, squared)
             for radius in (.001, .1, 1., 100.)
             for ratio in (0., .00001, .1, .5, .9, .95, .952381, .999999, 1.)
             for shape in ((0, 2, 4, 6, 8, 10, 12, 14) if args.include_reserved else (0, 2, 4, 6, 8))
             for positional in (0, 1) for squared in (0, 1)]
    inputs = ''.join(' '.join(map(str, row)) + '\n' for row in cases)
    run = subprocess.run([str(pas.with_suffix('.exe'))], input=inputs.encode(), capture_output=True, timeout=15)
    (args.output/'inputs.txt').write_text(inputs)
    (args.output/'output.txt').write_bytes(run.stdout + run.stderr)
    run.check_returncode()
    rows = [line.split() for line in run.stdout.decode().splitlines()]
    assert len(rows) == len(cases)
    fixture = ['// Independently extracted immutable Pascal Math3D/PaintThread, FPC i386 SSE2.',
      'struct VisibleLightMathFixture { uint32_t flux, radius; int option, squared; uint32_t expected_flux, expected_transparency, fast_pow50; };',
      'static const VisibleLightMathFixture kVisibleLightMathFixtures[] = {']
    for row in rows:
        values = ['0x'+v+'u' if i not in (2,3) else v for i,v in enumerate(row)]
        fixture.append('    {' + ', '.join(values) + '},')
    fixture += ['};', '']
    args.fixture.write_text('\n'.join(fixture))
    for artifact in (pas, pas.with_suffix('.exe'), pas.with_suffix('.s')):
        (args.output/artifact.name).write_bytes(artifact.read_bytes())
    sha = lambda p: hashlib.sha256(p.read_bytes()).hexdigest()
    identity = dict(scope='FastPow rational approximation and all visible-light shape branches; finite scalar inputs',
        rows=len(rows), includes_reserved_shapes=args.include_reserved, command=command, sha256={str(p):sha(p) for p in
          [math_source,paint_source,pas,pas.with_suffix('.exe'),FPC,FPC.with_name('ppc386.exe'),Path(__file__),args.fixture]})
    (args.output/'identity.json').write_text(json.dumps(identity,indent=2)+'\n')
    print(json.dumps(dict(rows=len(rows),fixture=str(args.fixture))))


if __name__ == '__main__': main()
