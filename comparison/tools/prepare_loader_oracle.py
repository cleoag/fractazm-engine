"""Install a load-only diagnostic hook in the external editable Pascal tree.

The immutable FileHandling conversion code is not modified. The dump precedes
headless render overrides and normalizes only process pointers in a copy.
"""
import hashlib
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
WORK = Path(r'C:\work\_c\mb3d_pascal_work\mb3d_work')
OUT = ROOT / 'comparison/runs/stage-a-20261018/scene-loader-v1'

def patch(path, before, after):
    raw = path.read_bytes()
    old, new = before.replace('\n','\r\n').encode(), after.replace('\n','\r\n').encode()
    if new in raw:
        return
    if raw.count(old) != 1:
        raise ValueError('Non-unique patch anchor: '+str(path))
    OUT.mkdir(parents=True, exist_ok=True)
    (OUT/(path.name+'.before')).write_bytes(raw)
    path.write_bytes(raw.replace(old,new))

def main():
    unit = WORK/'HeadlessRender.pas'
    declaration = 'procedure HeadlessStart;'
    # Only the interface occurrence is followed immediately by this procedure.
    patch(unit, declaration+'\nprocedure HeadlessOnRenderComplete',
          declaration+'\nprocedure HeadlessDumpLoadedParameters(const Header: TMandHeader10);\nprocedure HeadlessOnRenderComplete')
    implementation = '''procedure HeadlessDumpLoadedParameters(const Header: TMandHeader10);
var
  Path: String;
  Stream: TFileStream;
  CopyHeader: TMandHeader10;
begin
  Path := GetEnvironmentVariable('MB3D_HEADLESS_LOADER_DUMP');
  if Path = '' then Exit;
  CopyHeader := Header;
  FillChar(CopyHeader.PHCustomF, SizeOf(CopyHeader.PHCustomF), 0);
  CopyHeader.PCFAddon := nil;
  Stream := TFileStream.Create(Path, fmCreate);
  try
    Stream.WriteBuffer(CopyHeader, SizeOf(CopyHeader));
    Stream.WriteBuffer(PTHeaderCustomAddon(Header.PCFAddon)^, SizeOf(THeaderCustomAddon));
  finally
    Stream.Free;
  end;
  Halt(0);
end;

'''
    patch(unit, 'procedure HeadlessStart;\nbegin', implementation+'procedure HeadlessStart;\nbegin')
    patch(WORK/'Mand.pas', '      // Log formula configuration for test diagnostics',
          '      HeadlessDumpLoadedParameters(MHeader);\n      // Log formula configuration for test diagnostics')
    files = [unit,WORK/'Mand.pas',WORK/'FileHandling.pas',ROOT/'mb3d/FileHandling.pas',Path(__file__)]
    (OUT/'oracle-hook-identity.json').write_text(json.dumps({str(p):hashlib.sha256(p.read_bytes()).hexdigest() for p in files},indent=2))

if __name__ == '__main__': main()
