"""Capture original Pascal interpolation setup expressions in an isolated oracle."""
import argparse
import hashlib
import json
from pathlib import Path
import struct
import subprocess

ROOT = Path(__file__).resolve().parents[2]
WORK = Path('C:/work/_c/mb3d_pascal_work/mb3d_work/interpolation_setup_oracle')
FPC = Path('C:/FPC/3.2.2/bin/i386-Win32/fpc.exe')


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--output', type=Path, required=True)
    ap.add_argument('--fixture', type=Path, required=True)
    args = ap.parse_args()
    args.output.mkdir(parents=True, exist_ok=False)
    WORK.mkdir(parents=True, exist_ok=True)
    source = ROOT / 'mb3d/HeaderTrafos.pas'
    text = source.read_text()
    start = text.index('        x1 := PSingle(@PCFA.Formulas[0].iItCount)^;')
    end = text.index('PSingle(@Result.nHybrid[1])^ := x2 * y1;', start)
    normalization = text[start:end + len('PSingle(@Result.nHybrid[1])^ := x2 * y1;')]
    blend = 'result.fHln[0] := fHln[0] * PSingle(@nHybrid[0])^ + fHln[1] * PSingle(@nHybrid[1])^;'
    assert blend in text
    scale = 'if IsCustomDE then dDEscale := dDEscale / dStepWidth\n                        else dDEscale := dDEscale * mctDEoffset;'
    assert scale in text
    color = 'dColPlus := dColPlus + i * 0.1;'
    assert color in text
    pas = WORK / 'interpolation_params_oracle.pas'
    pas.write_text('''program interpolation_params_oracle;
{$mode delphi}{$R-}{$Q-}
uses SysUtils, Math;
type TFormula = record iItCount: Integer; end;
 TAddon = record Formulas: array[0..1] of TFormula; end;
 TParams = record nHybrid: array[0..1] of Integer; fHln: array[0..1] of Single;
 dDEscale, mctDEoffset, dColPlus: Single; IsCustomDE: Boolean; end;
var PCFA: TAddon; Result: TParams; x1,x2,y1,dStepWidth: Double;
 line: string; a,b: LongWord; i: Integer;
begin
 SetExceptionMask([exInvalidOp,exDenormalized,exZeroDivide,exOverflow,exUnderflow,exPrecision]);
 while not Eof do begin
  ReadLn(line); a:=StrToDWord('$'+Copy(line,1,8)); b:=StrToDWord('$'+Copy(line,10,8));
  Move(a,PCFA.Formulas[0].iItCount,4); Move(b,PCFA.Formulas[1].iItCount,4);
''' + normalization + '''
  Result.fHln[0]:=0.9102392266; Result.fHln[1]:=1.4426950216;
  Result.dDEscale:=1.7; Result.mctDEoffset:=0.003; Result.IsCustomDE:=True;
  Result.dColPlus:=0; i:=13; dStepWidth:=0.05100827205882353;
  with Result do begin
''' + blend + '\n' + scale + '\n' + color + '''
  end;
  WriteLn(IntToHex(a,8),' ',IntToHex(b,8),' ',IntToHex(LongWord(Result.nHybrid[0]),8),' ',
   IntToHex(LongWord(Result.nHybrid[1]),8),' ',IntToHex(PLongWord(@Result.fHln[0])^,8),' ',
   IntToHex(PLongWord(@Result.dDEscale)^,8),' ',IntToHex(PLongWord(@Result.dColPlus)^,8));
 end;
end.
''')
    command = [str(FPC), '-Mdelphi', '-CfSSE2', '-O2', '-al', str(pas)]
    build = subprocess.run(command, cwd=WORK, capture_output=True, timeout=60)
    (args.output / 'build.log').write_bytes(build.stdout + build.stderr)
    build.check_returncode()
    bits = lambda v: struct.unpack('<I', struct.pack('<f', v))[0]
    cases = [(bits(a), bits(b)) for a,b in [(.25,.75),(.75,.25),(1.,0.),(0.,1.),
        (0.,0.),(-0.,1.),(1.,-0.),(-1.,2.),(2.,-1.),(-1.,1.),(1.,3.),(3.,7.),
        (.1,.2),(1e20,3e20),(1e-11,1e-11),(1e-10,0.)]]
    cases += [(25,75),(75,25),(0x7f7fffff,0x7f7fffff),(0x7f7fffff,0x7f000000),
              (bits(1e-10)-1,0),(bits(1e-10),0),(bits(1e-10)+1,0),
              (0x3f000001,0x3eaaaaab),(0x40000001,0x3dcccccd)]
    cases = list(dict.fromkeys(cases))
    inputs = ''.join(f'{a:08X} {b:08X}\n' for a,b in cases)
    run = subprocess.run([str(pas.with_suffix('.exe'))], input=inputs.encode(), capture_output=True, timeout=15)
    (args.output / 'inputs.txt').write_text(inputs)
    (args.output / 'output.txt').write_bytes(run.stdout + run.stderr)
    run.check_returncode()
    rows = [line.split() for line in run.stdout.decode().splitlines()]
    assert len(rows) == len(cases)
    fixture = ['// Original HeaderTrafos.pas normalization and Single fHln blend; FPC i386 SSE2.',
               'struct InterpolationSetupFixture { uint32_t a, b, normalized_a, normalized_b, blended_log; };',
               'static const InterpolationSetupFixture kInterpolationSetupFixtures[] = {']
    fixture += ['    {' + ', '.join('0x'+v+'u' for v in row[:5]) + '},' for row in rows]
    fixture += ['};',
                'static constexpr uint32_t kInterpolationFinalScale = 0x'+rows[0][5]+'u;',
                'static constexpr uint32_t kInterpolationColorOffset = 0x'+rows[0][6]+'u;', '']
    args.fixture.write_text('\n'.join(fixture))
    for artifact in [pas,pas.with_suffix('.exe'),pas.with_suffix('.s')]:
        (args.output / artifact.name).write_bytes(artifact.read_bytes())
    sha = lambda p: hashlib.sha256(p.read_bytes()).hexdigest()
    identity = {'scope':'Extracted original interpolation normalization, blend, final DE scale and color offset',
                'rows':len(rows),'command':command,'sha256':{str(p):sha(p) for p in
                    [source,pas,pas.with_suffix('.exe'),FPC,FPC.with_name('ppc386.exe'),Path(__file__),args.fixture]}}
    (args.output / 'identity.json').write_text(json.dumps(identity,indent=2)+'\n')
    print(json.dumps({'rows':len(rows),'fixture':str(args.fixture)}))


if __name__ == '__main__':
    main()
