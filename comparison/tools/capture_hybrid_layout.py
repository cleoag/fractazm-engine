"""Capture original direct CheckHybridOptions/GetMCT layout, independently of GUI loading."""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess

ROOT = Path(__file__).resolve().parents[2]
WORK = Path('C:/work/_c/mb3d_pascal_work/mb3d_work/hybrid_layout_oracle')
FPC = Path('C:/FPC/3.2.2/bin/i386-Win32/fpc.exe')


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--output', type=Path, required=True)
    ap.add_argument('--fixture', type=Path, required=True)
    args = ap.parse_args()
    args.output.mkdir(parents=True, exist_ok=False)
    WORK.mkdir(parents=True, exist_ok=True)
    source = ROOT / 'mb3d/CustomFormulas.pas'
    header = ROOT / 'mb3d/HeaderTrafos.pas'
    raw = source.read_text(encoding='latin1')
    start = raw.index('procedure CheckHybridOptions(pHCA: PTHeaderCustomAddon);', raw.index('implementation'))
    code = raw[start:raw.index('procedure InsertDescription', start)]
    text = header.read_text(encoding='latin1')
    start = text.index('      CheckHybridOptions(PCFA);', text.index('function GetMCTparasFromHeader', text.index('implementation')))
    setup = text[start:text.index('      Result.CalcDE', start)]
    pas = WORK / 'hybrid_layout_oracle.pas'
    pas.write_text('''program hybrid_layout_oracle;
{$mode delphi}{$R-}{$Q-}
uses SysUtils, Math;
type TFormula=record iItCount:Integer; end;
 THeaderCustomAddon=record bOptions1,bHybOpt1:Byte; bHybOpt2:Word;
  Formulas:array[0..5] of TFormula; end;
 PTHeaderCustomAddon=^THeaderCustomAddon;
 TLayout=record StartFrom1,wEndTo,RepeatFrom1,StartFrom2,iEnd2,RepeatFrom2:Integer; end;
''' + code + '''
var A:THeaderCustomAddon; PCFA:PTHeaderCustomAddon; Result:TLayout;
 mode,opt1,opt2,i,c:Integer;
begin
 while not Eof do begin
  Read(mode,opt1,opt2);FillChar(A,SizeOf(A),0);
  A.bOptions1:=mode;A.bHybOpt1:=opt1;A.bHybOpt2:=opt2;
  for i:=0 to 5 do begin Read(c);A.Formulas[i].iItCount:=c;end;
  ReadLn;PCFA:=@A;
''' + setup + '''
  WriteLn(A.bHybOpt1,' ',A.bHybOpt2,' ',Result.StartFrom1,' ',Result.wEndTo,' ',
   Result.RepeatFrom1,' ',Result.StartFrom2,' ',Result.iEnd2,' ',Result.RepeatFrom2);
 end;
end.
''', encoding='latin1')
    cmd = [str(FPC), '-Mdelphi', '-CfSSE2', '-O2', '-al', str(pas)]
    p = subprocess.run(cmd, cwd=WORK, capture_output=True, timeout=45)
    (args.output / 'build.log').write_bytes(p.stdout + p.stderr)
    p.check_returncode()
    patterns = [(0,1,[1,1,0,0,0,0]), (0x50,0x552,[2,1,1,0,0,0]),
                (0xF7,0x777,[1,0,-2,0,1,3]), (0x40,0x543,[1,1,0,0,0,-1]),
                (0x30,0x111,[1,0,0,0,0,0]), (0xF7,0x777,[0,0,0,0,0,0])]
    cases = [(mode,a,b,c) for mode in (0,1,2,3,4,5,6,7,252,253,254,255)
             for a,b,c in patterns]
    inputs = ''.join(' '.join(map(str,[mode,a,b,*c]))+'\n' for mode,a,b,c in cases)
    p = subprocess.run([str(pas.with_suffix('.exe'))], input=inputs.encode(), capture_output=True, timeout=10)
    p.check_returncode()
    rows = [list(map(int,line.split())) for line in p.stdout.decode().splitlines()]
    assert len(rows)==len(cases) and all(len(r)==8 for r in rows)
    (args.output/'inputs.txt').write_text(inputs)
    (args.output/'output.txt').write_bytes(p.stdout)
    fixture = ['// Extracted immutable Pascal CheckHybridOptions and GetMCT layout assignments.',
               '// Scope: direct integer layout preparation; no GUI normalization or full-orbit claim.',
               'struct HybridLayoutFixture { int mode, opt1, opt2, counts[6], normalized1, normalized2, layout[6]; };',
               'static const HybridLayoutFixture kHybridLayoutFixtures[] = {']
    for (mode,a,b,c), r in zip(cases,rows):
        fixture.append('    {'+f'{mode}, {a}, {b}, '+'{'+', '.join(map(str,c))+'}, '+f'{r[0]}, {r[1]}, '+'{'+', '.join(map(str,r[2:]))+'}},')
    fixture += ['};','']
    args.fixture.write_text('\n'.join(fixture))
    for path in [pas,pas.with_suffix('.exe'),pas.with_suffix('.s')]:
        (args.output/path.name).write_bytes(path.read_bytes())
    sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
    identity=dict(scope='Exact extracted original integer layout plus GetMCT field assignments; direct caller only',
                  rows=len(rows),command=cmd,results=[dict(input=[m,a,b,*c],output=r) for (m,a,b,c),r in zip(cases,rows)],
                  sha256={str(p):sha(p) for p in [source,header,pas,pas.with_suffix('.exe'),FPC,FPC.with_name('ppc386.exe'),Path(__file__),args.fixture]})
    (args.output/'identity.json').write_text(json.dumps(identity,indent=2)+'\n')
    print(json.dumps({'rows':len(rows),'fixture':str(args.fixture)}))


if __name__=='__main__':main()
