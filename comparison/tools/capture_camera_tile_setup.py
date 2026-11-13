"""Extract immutable Pascal camera/tile preparation into a small i386 oracle."""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess

ROOT = Path(__file__).resolve().parents[2]
WORK = Path('C:/work/_c/mb3d_pascal_work/mb3d_work/camera_tile_oracle')
FPC = Path('C:/FPC/3.2.2/bin/i386-Win32/fpc.exe')


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--output', type=Path, required=True)
    ap.add_argument('--fixture', type=Path, required=True)
    ap.add_argument('--all-stereo-bytes', action='store_true', help='Capture all256 direct-header stereo values')
    args = ap.parse_args()
    args.output.mkdir(parents=True, exist_ok=False)
    WORK.mkdir(parents=True, exist_ok=True)
    files = {n: ROOT / ('mb3d/' + n + '.pas') for n in ['HeaderTrafos', 'Math3D', 'DivUtils']}
    text = {n: p.read_text(encoding='latin1') for n, p in files.items()}
    extracts = []

    def extract(unit, first, after):
        start = text[unit].index(first, text[unit].lower().index('implementation'))
        stop = text[unit].index(after, start + len(first))
        value = text[unit][start:stop]
        extracts.append({'unit': str(files[unit]), 'start_line': text[unit][:start].count('\n') + 1,
                         'text_sha256': hashlib.sha256(value.encode('latin1')).hexdigest()})
        return value

    code = extract('Math3D', 'function MaxCD(const s1, s2: Double): Double;',
                   '//    if Abs(s1)')
    code += extract('Math3D', 'function NormaliseVectorTo(const n: Double; const V: TVec3D): TVec3D;',
                    'procedure NormaliseVectorTo(const n: Double; V: TPVec3D);')
    code += extract('Math3D', 'procedure mCopyAddVecWeight(V1, V2, V3: TPVec3D; const W: Double);',
                    'procedure CopyAddSVecWeight')
    code += extract('DivUtils', 'procedure CalcStepWidth(Header: TPMandHeader10);',
                    'function cIpol2colFlip')
    code += extract('HeaderTrafos', 'function CalcXoff(Header: TPMandHeader10): Single;',
                    'function GetRealMidPos')
    code += extract('HeaderTrafos', 'procedure StereoChange(Header: TPMandHeader10;',
                    'procedure Ini1CFfromHAddon')
    code += extract('DivUtils', 'procedure GetTilingInfosFromHeader(Header: TPMandHeader10;',
                    'procedure DisableTiling')
    pas = WORK / 'camera_tile_oracle.pas'
    pas.write_text(('''program camera_tile_oracle;
{$mode delphi}{$R-}{$Q-}
uses SysUtils, Math, Types;
type TVec3D=array[0..2] of Double; TPVec3D=^TVec3D;
 TMatrix3=array[0..2] of TVec3D; TPMatrix3=^TMatrix3;
 TMandHeader10=record Width,Height,TilingOptions:Integer; bStereoMode:Byte;
 dZoom,dStepWidth,dXmid,dYmid,dZmid:Double;
 StereoScreenWidth,StereoScreenDistance,StereoMinDistance:Single; hVGrads:TMatrix3; end;
 TPMandHeader10=^TMandHeader10;
const d1em100:Double=1e-100; s05:Single=0.5;
var SupportSSE2:Boolean=True;
''' + code + '''
var H:TMandHeader10; mode,config,crop,tx,ty,px,py,w,hei,xp,yp,c:Integer;
 v:TVec3D; xoff:Single; tile:TPoint; rect:TRect;
begin
 SetExceptionMask([exInvalidOp,exDenormalized,exZeroDivide,exOverflow,exUnderflow,exPrecision]);
 Set8087CW($027F);
 FillChar(H,SizeOf(H),0); H.Width:=97;H.Height:=73;H.dZoom:=1.25;
 H.dXmid:=0.125;H.dYmid:=-0.25;H.dZmid:=0.75;
 H.hVGrads[0,0]:=0.3;H.hVGrads[0,1]:=-0.4;H.hVGrads[0,2]:=0.5;
 for config:=0 to 2 do begin
  case config of
   0:begin H.StereoScreenWidth:=0.45;H.StereoScreenDistance:=0.65;H.StereoMinDistance:=0.08;end;
   1:begin H.StereoScreenWidth:=2.3;H.StereoScreenDistance:=1.25;H.StereoMinDistance:=0.25;end;
   2:begin H.StereoScreenWidth:=0.45;H.StereoScreenDistance:=1.25;H.StereoMinDistance:=1.25;end;
  end;
  for mode:=0 to 4 do begin
   H.bStereoMode:=mode; xoff:=CalcXoff(@H);StereoChange(@H,mode,v,@H.hVGrads);
   WriteLn('S ',config,' ',mode,' ',IntToHex(PLongWord(@xoff)^,8),' ',
    IntToHex(PQWord(@v[0])^,16),' ',IntToHex(PQWord(@v[1])^,16),' ',IntToHex(PQWord(@v[2])^,16));
  end;
 end;
 for config:=0 to 2 do begin
  case config of 0:begin tx:=1;ty:=1;end;1:begin tx:=3;ty:=2;end;2:begin tx:=7;ty:=5;end;end;
  for crop:=0 to 3 do for c:=0 to 1 do begin
   if c=0 then begin px:=0;py:=0;end else begin px:=tx-1;py:=ty-1;end;
   H.TilingOptions:=tx or (ty shl 7) or (px shl 14) or (py shl 21) or (crop shl 28);
   GetPaintTileSizes(@H,w,hei,xp,yp);tile:=GetTileSize(@H);GetTilingInfosFromHeader(@H,rect,mode);
   WriteLn('T ',H.TilingOptions,' ',w,' ',hei,' ',xp,' ',yp,' ',tile.X,' ',tile.Y,' ',
    rect.Left,' ',rect.Top,' ',rect.Right,' ',rect.Bottom);
  end;
 end;
end.
''').replace('for mode:=0 to 4 do begin', 'for mode:=0 to 255 do begin' if args.all_stereo_bytes else 'for mode:=0 to 4 do begin'), encoding='latin1')
    cmd = [str(FPC), '-Mdelphi', '-CfSSE2', '-O2', '-al', str(pas)]
    proc = subprocess.run(cmd, cwd=WORK, capture_output=True, timeout=45)
    (args.output / 'build.log').write_bytes(proc.stdout + proc.stderr)
    proc.check_returncode()
    proc = subprocess.run([str(pas.with_suffix('.exe'))], capture_output=True, timeout=15)
    (args.output / 'output.txt').write_bytes(proc.stdout + proc.stderr)
    proc.check_returncode()
    stereo, tiles = [], []
    for line in proc.stdout.decode().splitlines():
        tag, *row = line.split()
        (stereo if tag == 'S' else tiles).append(row)
    fixture = ['// Immutable Pascal source extraction, FPC i386 SSE2; see camera-tile-setup-oracle-v1.',
               'struct StereoSetupFixture { int config, mode; uint32_t xoff; uint64_t mid[3]; };',
               'static const StereoSetupFixture kStereoSetupFixtures[] = {']
    fixture += [f'    {{{r[0]}, {r[1]}, 0x{r[2]}u, {{0x{r[3]}ull, 0x{r[4]}ull, 0x{r[5]}ull}}}},' for r in stereo]
    fixture += ['};', 'struct TileSetupFixture { int options, w, h, x, y, tileW, tileH, left, top, right, bottom; };',
                'static const TileSetupFixture kTileSetupFixtures[] = {']
    fixture += ['    {' + ', '.join(r) + '},' for r in tiles]
    fixture += ['};', '']
    args.fixture.write_text('\n'.join(fixture))
    for p in [pas, pas.with_suffix('.exe'), pas.with_suffix('.s')]:
        (args.output / p.name).write_bytes(p.read_bytes())
    sha = lambda p: hashlib.sha256(p.read_bytes()).hexdigest()
    identity = {'stereo_rows': len(stereo), 'tile_rows': len(tiles), 'all_stereo_bytes': args.all_stereo_bytes, 'command': cmd, 'extracts': extracts,
                'sha256': {str(p): sha(p) for p in [*files.values(), pas, pas.with_suffix('.exe'),
                                                  FPC, FPC.with_name('ppc386.exe'), Path(__file__), args.fixture]}}
    (args.output / 'identity.json').write_text(json.dumps(identity, indent=2) + '\n')
    print(json.dumps({'stereo_rows': len(stereo), 'tile_rows': len(tiles)}))


if __name__ == '__main__':
    main()
