"""Independent FPC Math.Power/Single-store boundary fixture for absorption.

Expected bits come from a compiled Pascal program, not Python arithmetic.
This finite fixture is not certification of every libm input or NaN payload.
"""
import argparse
import hashlib
from pathlib import Path
import subprocess
from external_validation import ROOT,write_json

SOURCE='''program transmission_power;
{$mode delphi}
uses Math, SysUtils;
var x,p,y:Single; xb,pb,yb:LongWord; i,j:Integer; d:Double; db:QWord;
const bases:array[0..6]of Single=(0,1e-30,1e-20,0.25,1,-0.25,-1);
      powers:array[0..4]of Single=(0,0.5,1,2,3);
      distances:array[0..6]of Double=(0,1,1.0000000596046448,1.000000059604645,1e-30,1e30,-0.125);
      coefficients:array[0..3]of Single=(0,0.1,0.8,1.0000001192092896);
begin
 SetExceptionMask([exInvalidOp,exDenormalized,exZeroDivide,exOverflow,exUnderflow,exPrecision]);
 for i:=0 to High(bases)do for j:=0 to High(powers)do begin
  x:=bases[i]; p:=powers[j]; y:=Power(x,p);
  Move(x,xb,4);Move(p,pb,4);Move(y,yb,4);
  Writeln('P ',IntToHex(xb,8),' ',IntToHex(pb,8),' ',IntToHex(yb,8));
 end;
 for i:=0 to High(distances)do for j:=0 to High(coefficients)do begin
  d:=distances[i];p:=coefficients[j];y:=d*p;
  Move(d,db,8);Move(p,pb,4);Move(y,yb,4);
  Writeln('E ',IntToHex(db,16),' ',IntToHex(pb,8),' ',IntToHex(yb,8));
 end;
end.
'''

def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()

def main():
    ap=argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--output',type=Path,required=True)
    ap.add_argument('--header',type=Path,required=True)
    a=ap.parse_args();a.output.mkdir(parents=True,exist_ok=True)
    work=Path('C:/work/_c/mb3d_pascal_work/mb3d_work/audit_openair');work.mkdir(parents=True,exist_ok=True)
    fpc=Path('C:/FPC/3.2.2/bin/i386-Win32/fpc.exe');src=work/'transmission_power.pas';src.write_text(SOURCE,encoding='ascii')
    cmd=[str(fpc),'-Mdelphi','-O2','-CfSSE2',str(src)]
    build=subprocess.run(cmd,capture_output=True,check=True);exe=src.with_suffix('.exe')
    run=subprocess.run([str(exe)],capture_output=True,check=True)
    rows=[line.split() for line in run.stdout.decode('ascii').splitlines()]
    if len(rows)!=63:raise ValueError('Incomplete oracle output')
    (a.output/'source.pas').write_text(SOURCE,encoding='ascii');(a.output/'output.txt').write_bytes(run.stdout)
    (a.output/'build.log').write_bytes(build.stdout+build.stderr)
    write_json(a.output/'identity.json',dict(scope=__doc__,source_sha256=sha(src),binary_sha256=sha(exe),compiler_sha256=sha(fpc),
        command=cmd,original_math3d_sha256=sha(ROOT/'mb3d/Math3D.pas'),original_calcsr_sha256=sha(ROOT/'mb3d/CalcSR.pas'),rows=len(rows)))
    header='// Independent original Pascal Math.Power and Double*Single -> Single outputs.\n'
    header+='struct TransmissionPowerFixture { unsigned base, exponent, expected; };\nstatic const TransmissionPowerFixture kTransmissionPowers[] = {\n'
    header+=''.join('    {'+', '.join('0x'+v+'u' for v in r[1:])+'},\n' for r in rows if r[0]=='P')+'};\n'
    header+='struct TransmissionExponentFixture { unsigned long long distance; unsigned absorption, expected; };\nstatic const TransmissionExponentFixture kTransmissionExponents[] = {\n'
    header+=''.join('    {0x'+r[1]+'ull, 0x'+r[2]+'u, 0x'+r[3]+'u},\n' for r in rows if r[0]=='E')+'};\n'
    a.header.write_text(header,encoding='ascii');print(len(rows),'independent Pascal rows')

if __name__=='__main__':main()
