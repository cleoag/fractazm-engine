"""Capture original Pascal parameter-capacity behavior, without rendering.

The unmodified FillCustomVBufWithVars body is extracted from the reference.
Unused matrix/reciprocal dependencies abort if accidentally exercised.
"""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess

ROOT = Path(__file__).resolve().parents[2]
WORK = Path('C:/work/_c/mb3d_pascal_work/mb3d_work/option_capacity_oracle')
FPC = Path('C:/FPC/3.2.2/bin/i386-Win32/fpc.exe')


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--output', type=Path, required=True)
    ap.add_argument('--fixture', type=Path, required=True)
    a = ap.parse_args()
    a.output.mkdir(parents=True, exist_ok=False)
    WORK.mkdir(parents=True, exist_ok=True)
    ref = ROOT / 'mb3d/CustomFormulas.pas'
    text = ref.read_text()
    start = text.index('procedure FillCustomVBufWithVars(CF1: PTCustomFormula; dOptionValues: array of Double);', text.index('implementation'))
    end = text.index('\nend;', start) + len('\nend;')
    body = text[start:end]
    assert 'MemNeeded[byOptionTypes[i]]) < 257' in body
    # Every store is four-byte aligned: the nearest possible overflow is four
    # bytes, not one. Cases cover exact fit, skips, compound cursor and count16.
    cases = [
        ('exact_256', [11]*7+[14,1,1]),
        ('skip_double_then_single', [11]*7+[14,1,0,1]),
        ('skip_compound_then_followers', [11]*7+[5,1,2,0,1,2,0,1,2]),
        ('skip_zero_cost_then_small', [11]*7+[14,1,10,20,2]),
        ('folding_reservation', [11]*6+[14,1,8,2]),
        ('folding_pointer_then_capacity', [0,0,8]+[11]*5+[14,1,0,1,2]),
        ('count_limit', [11]*7+[1]*9+[14,14]),
    ]
    prefix = '''program option_capacity_oracle;
{$mode delphi}{$R-}{$Q-}
uses SysUtils, Math;
type TMatrix3 = array[0..2,0..2] of Double;
 TSMatrix4 = array[0..15] of Single;
 ThybridIteration = procedure;
 TPhybridIteration = ^ThybridIteration;
 TCustomFormula = record
   pConstPointer16: Pointer;
   iCFOptionCount: Integer;
   byOptionTypes: array[0..15] of Byte;
 end;
 PTCustomFormula = ^TCustomFormula;
var fHIntFunctions: array[0..8] of ThybridIteration;
const Pid180 = 0.017453292519943295; s1em30: Single = 1e-30;
 d1em40: Double = 1e-40;
procedure BuildRotMatrix(x,y,z: Double; p: Pointer); begin raise Exception.Create('unused matrix'); end;
procedure ScaleMatrix(x: Double; p: Pointer); begin raise Exception.Create('unused scale'); end;
procedure BuildRotMatrix4d(a: array of Double; var m: TSMatrix4); begin raise Exception.Create('unused matrix4'); end;
function MaxAbsCD(a,b: Double): Double; begin raise Exception.Create('unused reciprocal'); end;
function MaxCD(a,b: Double): Double; begin raise Exception.Create('unused reciprocal'); end;
function MaxCS(a,b: Single): Single; begin raise Exception.Create('unused reciprocal'); end;
'''
    suffix = '''
var cf: TCustomFormula; buf: array[0..1023] of Byte;
 values: array[0..15] of Double; n,i,k,t: Integer;
begin
 while not Eof do begin
  Read(n); FillChar(cf,SizeOf(cf),0); FillChar(buf,SizeOf(buf),0);
  cf.pConstPointer16 := @buf[512]; cf.iCFOptionCount := n;
  for i:=0 to n-1 do begin Read(t); if i<16 then cf.byOptionTypes[i]:=t; end;
  ReadLn;
  for i:=0 to 15 do values[i]:=i+2;
  FillCustomVBufWithVars(@cf,values);
  for k:=256 to 511 do Write(IntToHex(buf[k],2));
  WriteLn;
 end;
end.
'''
    pas = WORK / 'option_capacity_oracle.pas'
    pas.write_text(prefix + body + suffix)
    cmd = [str(FPC), '-Mdelphi', '-CfSSE2', '-O2', str(pas)]
    build = subprocess.run(cmd, cwd=WORK, capture_output=True, timeout=60)
    (a.output/'build.log').write_bytes(build.stdout+build.stderr)
    build.check_returncode()
    inputs = ''.join(str(len(types))+' '+' '.join(map(str,types))+'\n' for _,types in cases)
    run = subprocess.run([str(pas.with_suffix('.exe'))], input=inputs.encode(), capture_output=True, timeout=15)
    (a.output/'inputs.txt').write_text(inputs)
    (a.output/'output.txt').write_bytes(run.stdout+run.stderr)
    run.check_returncode()
    rows = run.stdout.decode().splitlines()
    assert len(rows) == len(cases) and all(len(s)==512 for s in rows)
    out = ['// Original FillCustomVBufWithVars capacity fixtures; pointer bytes are zero.',
           'struct OptionCapacityFixture { const char* name; int count; uint8_t types[16]; const char* bytes; };',
           'static const OptionCapacityFixture kOptionCapacityFixtures[] = {']
    for (name,types),row in zip(cases,rows):
        out.append('    {"'+name+'", '+str(len(types))+', {'+','.join(map(str,types[:16]))+'}, "'+row+'"},')
    out += ['};', '']
    a.fixture.write_text('\n'.join(out))
    for p in [pas,pas.with_suffix('.exe')]: (a.output/p.name).write_bytes(p.read_bytes())
    sha = lambda p: hashlib.sha256(p.read_bytes()).hexdigest()
    (a.output/'identity.json').write_text(json.dumps(dict(rows=len(rows), command=cmd,
        scope='Original parameter capacity and skipped option cursor; no matrix or reciprocal arithmetic exercised',
        sha256={str(p):sha(p) for p in [ref,pas,pas.with_suffix('.exe'),FPC,FPC.with_name('ppc386.exe'),Path(__file__),a.fixture]}),indent=2))
    print('Captured',len(rows),'original capacity fixtures')


if __name__ == '__main__': main()
