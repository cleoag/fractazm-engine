"""Independent legacy M3I record/read witnesses; does not render or alter production.

The native i386 oracle extracts immutable FileHandling.pas conversion and raw
LoadFullM3I reads verbatim. Header migration and GUI allocation are explicit
harness inputs, not independently verified by this codec-only oracle.
"""
import argparse
from collections import Counter
import hashlib
import json
from pathlib import Path
import struct
import subprocess

from check_legacy_loader import legacy
from external_validation import ROOT, decode


def sha(path): return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def prepare_oracle(out, work):
    original = ROOT/'mb3d/FileHandling.pas'
    text = original.read_text(encoding='latin1')
    a = text.index('procedure ConvertSiLight4To5(PSL4:')
    b = text.index('\nprocedure UpdateLightVersion8', a)
    conversion = text[a:b]
    a = text.index('        BlockRead(f, MID, 4);', text.index('procedure LoadFullM3I(var Header:', text.index('implementation')))
    b = text.index('          UpdateScaledImage(0, tp.Y);', a)
    reads = text[a:b] + '\n        end;\n'
    types = (ROOT/'mb3d/TypeDefinitions.pas').read_text(encoding='latin1')
    a = types.index('  TsiLight5 = packed record')
    b = types.index('  TsiLightOld = packed record', a)
    record_types = types[a:b]
    divutils = (ROOT/'mb3d/DivUtils.pas').read_text(encoding='latin1')
    a = divutils.index('function GetTileSize(Header:', divutils.index('implementation'))
    b = divutils.index('\nprocedure DisableTiling', a)
    tile_size = divutils[a:b]
    source = '''program m3i_codec_oracle;
{$mode delphi}{$asmmode intel}
uses SysUtils, Math, Types;
type
''' + record_types + '''
  TPsiLight4=^TsiLight4; TPsiLight5=^TsiLight5;
  TMandHeader9=packed record data:array[0..703]of Byte;end;
  TMandHeader10=packed record Width,Height,TilingOptions:Integer;bCalculateHardShadow,bDFogIt:Byte;pad:array[0..825]of Byte;end;
  TPMandHeader10=^TMandHeader10;
  THeaderCustomAddon=packed record data:array[0..1135]of Byte;end;
  TMandForm=class
    siLight5:array of TsiLight5; mSLoffset:Integer;
    procedure OutMessage(s:String);
  end;
const Pid16384:Double=Pi/16384;
procedure TMandForm.OutMessage(s:String); begin raise Exception.Create(s);end;
''' + conversion + '\n' + tile_size + '''
var f,g:file; MID,i,j:Integer; SI3:array of TsiLight3; SI4:array of TsiLight4;
    PSI3,PSI4:PInteger; PW:PWord; tp:TPoint; Header:TMandHeader10;
    Mand3DForm:TMandForm; fullSizeImage:array of Cardinal; hasImage:Boolean; imageStart:Int64;
begin
SetExceptionMask([exInvalidOp,exDenormalized,exZeroDivide,exOverflow,exUnderflow,exPrecision]);
Set8087CW($137F);
j:=StrToInt(ParamStr(3));tp.X:=j;tp.Y:=1;
Header.bCalculateHardShadow:=StrToInt(ParamStr(4));Header.bDFogIt:=StrToInt(ParamStr(5));
SetLength(fullSizeImage,StrToInt(ParamStr(6)));Mand3DForm:=TMandForm.Create;
if ParamCount>=9 then begin
 Header.Width:=StrToInt(ParamStr(7));Header.Height:=StrToInt(ParamStr(8));Header.TilingOptions:=StrToInt(ParamStr(9));
 tp:=GetTileSize(@Header);j:=tp.X*tp.Y;
 if Header.TilingOptions<>0 then SetLength(fullSizeImage,j);
end;
try
AssignFile(f,ParamStr(1));Reset(f,1);
''' + reads + '''
if MID<4 then imageStart:=200 else if MID<7 then imageStart:=272 else
if MID<8 then imageStart:=580 else if MID<20 then imageStart:=704 else imageStart:=840;
if MID<18 then imageStart:=imageStart+j*12 else if MID=18 then imageStart:=imageStart+j*16 else imageStart:=imageStart+j*18;
imageStart:=imageStart+SizeOf(THeaderCustomAddon);
hasImage:=(Length(fullSizeImage)=j) and (FileSize(f)>=imageStart+j*4);
CloseFile(f);
AssignFile(g,ParamStr(2)+'.silight');Rewrite(g,1);
if j>0 then BlockWrite(g,Mand3DForm.siLight5[0],j*18);CloseFile(g);
if hasImage then begin AssignFile(g,ParamStr(2)+'.image');Rewrite(g,1);BlockWrite(g,fullSizeImage[0],j*4);CloseFile(g);end;
WriteLn('OK ',j,' ',Ord(hasImage));
except on e:Exception do begin WriteLn('ERROR ',e.ClassName,' ',e.Message);Halt(2);end;end;
end.
'''
    work.mkdir(parents=True, exist_ok=True)
    generated = work/'m3i_codec_oracle.pas'; generated.write_text(source, encoding='latin1')
    (out/'m3i_codec_oracle.pas').write_text(source, encoding='latin1')
    fpc = Path('C:/FPC/3.2.2/bin/i386-Win32/fpc.exe')
    cmd = [str(fpc),'-Mdelphi','-O2','-CfSSE2','-al','-FE'+str(out),'-FU'+str(out),str(generated)]
    r = subprocess.run(cmd,capture_output=True,timeout=60)
    (out/'pascal-build.log').write_bytes(r.stdout+r.stderr)
    if r.returncode: raise RuntimeError((r.stdout+r.stderr).decode(errors='replace'))
    return dict(command=cmd, snippets=dict(conversion=hashlib.sha256(conversion.encode('latin1')).hexdigest(),
                raw_reads=hashlib.sha256(reads.encode('latin1')).hexdigest(),tile_size=hashlib.sha256(tile_size.encode('latin1')).hexdigest()),
                source={str(p):sha(p) for p in [original,ROOT/'mb3d/TypeDefinitions.pas',ROOT/'mb3d/DivUtils.pas',generated]},
                exe=str(out/'m3i_codec_oracle.exe'),exe_sha256=sha(out/'m3i_codec_oracle.exe'))


CPP = r'''#include "io/m3p_parser.h"
#include <fstream>
#include <iostream>
int main(int argc,char**argv) {
  if(argc!=3)return 64;
  try {
    auto r=mb3d::LoadM3IFile(argv[1]);
    std::cout<<r.success<<" "<<r.has_silight<<" "<<r.has_image<<" "<<r.siLight.size()<<" "
      <<int(r.params.header.bCalculateHardShadow)<<" "<<int(r.params.header.bDFogIt)<<" "
      <<r.params.header.Width<<" "<<r.params.header.Height<<"\n"<<r.error<<"\n";
    if(!r.success)return 2;
    std::ofstream f(std::string(argv[2])+".silight",std::ios::binary);
    f.write(reinterpret_cast<const char*>(r.siLight.data()),r.siLight.size()*sizeof(TsiLight5));
    if(r.has_image){std::ofstream g(std::string(argv[2])+".image",std::ios::binary);
      g.write(reinterpret_cast<const char*>(r.image.data()),r.image.size()*4);}
    return 0;
  }catch(const std::exception&e){std::cout<<"EXCEPTION "<<e.what()<<"\n";return 3;}
}'''


def prepare_cpp(out, compile_parser=False):
    p=out/'m3i_load_probe.cpp';p.write_text(CPP)
    libs=[ROOT/'build-vs-fixes/Release/mb3d_io.lib',ROOT/'build-vs-fixes/Release/mb3d_core.lib',
          ROOT/'build-vs-fixes/_deps/zlib-build/Release/zlibstatic.lib']
    vc=Path('C:/Program Files/Microsoft Visual Studio/2022/Professional/VC/Auxiliary/Build/vcvars64.bat')
    script=out/'build-probe.cmd'
    flags='/nologo /std:c++20 /EHsc /MD /O2 /I"'+str(ROOT/'src')+'" /I"'+str(ROOT/'include')+'"'
    parser_cmd=''
    parser_obj=''
    if compile_parser:
        parser_obj=' "'+str(out/'m3p_parser.obj')+'"'
        parser_cmd='cl '+flags+' /I"'+str(ROOT/'build-vs-release/_deps/zlib-src')+'" /I"'+str(ROOT/'build-vs-fixes/_deps/zlib-build')+'" /c "'+str(ROOT/'src/io/m3p_parser.cpp')+'" /Fo:"'+str(out/'m3p_parser.obj')+'"\nif errorlevel 1 exit /b 1\n'
    script.write_text('@echo off\ncall "'+str(vc)+'" >nul\n'+parser_cmd+'cl '+flags+' "'+str(p)+'" /Fe:"'+str(out/'m3i_load_probe.exe')+'" /Fo:"'+str(out/'m3i_load_probe.obj')+'" /link'+parser_obj+' '+ ' '.join('"'+str(x)+'"' for x in libs)+'\n')
    r=subprocess.run(['cmd.exe','/d','/c',str(script)],capture_output=True,timeout=60)
    (out/'cpp-build.log').write_bytes(r.stdout+r.stderr)
    if r.returncode:raise RuntimeError((r.stdout+r.stderr).decode(errors='replace'))
    return dict(libraries={str(p):sha(p) for p in libs},compile_parser=compile_parser,parser_source_sha256=sha(ROOT/'src/io/m3p_parser.cpp'),exe=str(out/'m3i_load_probe.exe'),
                exe_sha256=sha(out/'m3i_load_probe.exe'),source_sha256=sha(p))


def main():
    ap=argparse.ArgumentParser(description=__doc__);ap.add_argument('--output',type=Path,required=True)
    ap.add_argument('--cpp-exe',type=Path);ap.add_argument('--reuse-oracle',type=Path)
    ap.add_argument('--compile-parser',action='store_true',help='Compile only live parser into standalone diagnostic probe; do not touch CMake build.')
    a=ap.parse_args();out=a.output.resolve();out.mkdir(parents=True,exist_ok=True)
    pas=prepare_oracle(out,Path('C:/work/_c/mb3d_pascal_work/mb3d_work/m3i_codec_oracle')) if not a.reuse_oracle else dict(exe=str(a.reuse_oracle.resolve()),exe_sha256=sha(a.reuse_oracle))
    cpp=prepare_cpp(out,a.compile_parser) if not a.cpp_exe else dict(exe=str(a.cpp_exe.resolve()),exe_sha256=sha(a.cpp_exe))
    base=decode(ROOT/'comparison/scenes_phase2/phase2_intpow2_small.m3p')
    rows=[]
    def case(name,mid,count=48,hs=0,fog=0,image=True,truncate=None,oldha=False,parameter_only=False,width=None,height=None,tiling=0):
        b=bytearray(base);struct.pack_into('<2i',b,4,256 if count==32768 else 8,128 if count==32768 else 6)
        if width is not None:struct.pack_into('<i',b,4,width)
        if height is not None:struct.pack_into('<i',b,8,height)
        struct.pack_into('<i',b,428,tiling)
        b[133]=hs;b[223]=fog
        params=legacy(b,mid,cache=False,oldha=oldha)
        header=200 if mid<4 else 272 if mid<7 else 580 if mid<8 else 704 if mid<20 else 840
        size=12 if mid<18 else 16 if mid==18 else 18
        raw=params[:header]+bytes(count*size)+(params[header:] if mid>=16 else bytes(1136))
        for i in range(count):
            x=(i if count==32768 else [0,1,8191,8192,16383,16384,24575,24576,32767,32768,65535,12345][i%12])
            y=(i*17+8192)&0xFFFF
            rest=[(i*1297+0x1234)&65535,(i*2711+0x4321)&65535,[0,32766,32767,32768,65535][i%5],(i*71+65500)&65535,(i*93+32760)&65535,(i*197+123)&65535]
            vals=([x,y,rest[1],rest[2],rest[3],rest[4]] if size==12 else [x,y,*rest] if size==16 else [x,y,(i*7+32768)&65535,*rest])
            raw[header+i*size:header+(i+1)*size]=struct.pack('<'+'H'*len(vals),*vals)
        if image:raw+=b''.join(struct.pack('<I',0xff000000|((i*73013)&0xffffff)) for i in range(count))
        if parameter_only:raw=legacy(b,mid,cache=False)
        if truncate is not None:raw=raw[:truncate if truncate>=0 else len(raw)+truncate]
        scene=out/(name+'.m3i');scene.write_bytes(raw)
        r=subprocess.run([cpp['exe'],str(scene),str(out/(name+'.cpp'))],capture_output=True,text=True,timeout=15)
        (out/(name+'.cpp.log')).write_text(r.stdout+r.stderr)
        row=dict(id=name,mid=mid,record_size=size,source_records=count,scene_sha256=sha(scene),cpp_exit=r.returncode,cpp_log=r.stdout.strip())
        fields=r.stdout.splitlines()[0].split()
        # Header readback is an input to this codec oracle; migration is audited separately.
        if len(fields)==8:
            n=int(fields[3]) or count;hs2=int(fields[4]);fog2=int(fields[5])
            p=subprocess.run([pas['exe'],str(scene),str(out/(name+'.pascal')),str(n),str(hs2),str(fog2),str(int(fields[6])*int(fields[7])),fields[6],fields[7],str(tiling if mid>=35 else 0)],capture_output=True,text=True,timeout=15)
            row.update(pascal_exit=p.returncode,pascal_log=p.stdout.strip(),normalized_hs=hs2,normalized_fog=fog2)
            (out/(name+'.pascal.log')).write_text(p.stdout+p.stderr)
            if r.returncode==0 and p.returncode==0:
                expected=(out/(name+'.pascal.silight')).read_bytes();actual=(out/(name+'.cpp.silight')).read_bytes()
                diffs=[i for i,(x,y) in enumerate(zip(expected,actual)) if x!=y]
                records=sorted(set(i//18 for i in diffs))
                row.update(status='EXACT' if expected==actual else 'DIFFERENT',different_records=len(records),
                   field_counts=dict(Counter((i%18)//2 for i in diffs)),examples=[dict(index=i,pascal=struct.unpack_from('<9H',expected,i*18),cpp=struct.unpack_from('<9H',actual,i*18)) for i in records[:12]],
                   pascal_silight_sha256=sha(out/(name+'.pascal.silight')),cpp_silight_sha256=sha(out/(name+'.cpp.silight')))
                expected_image=p.stdout.strip().split()[-1]=='1';actual_image=fields[2]=='1'
                row.update(pascal_has_image=expected_image,cpp_has_image=actual_image,image_status='EXACT' if expected_image==actual_image else 'DIFFERENT')
                if expected_image and actual_image:
                    row['image_status']='EXACT' if (out/(name+'.cpp.image')).read_bytes()==(out/(name+'.pascal.image')).read_bytes() else 'DIFFERENT'
            else:row['status']='READ_STATUS_MATCH' if r.returncode!=0 and p.returncode!=0 else 'READ_STATUS_DIFFERENT'
        else:row['status']='CPP_EXCEPTION'
        rows.append(row);print(name,row['status'],row.get('different_records',''),flush=True)
    for mid in (0,4,7,16,17,18,19,20,35,44):
        case(f'v{mid:02d}_records',mid)
    for mid in (16,18,19):case(f'v{mid:02d}_hard_shadow',mid,hs=1)
    case('v35_dfog',35,fog=1)
    case('v18_all_x_angles',18,count=32768)
    case('v44_no_image',44,image=False)
    case('v16_old_addon_no_image',16,image=False,oldha=True)
    for mid in (16,18,44):
        header=704 if mid<20 else 840
        case(f'v{mid:02d}_truncated_pixel',mid,truncate=header+11)
        case(f'v{mid:02d}_truncated_image',mid,truncate=-3)
        case(f'v{mid:02d}_parameter_only_as_cache',mid,count=32768,parameter_only=True)
    case('v44_tile_3x2',44,count=20,tiling=3|(2<<7)|(1<<28))
    case('v44_tile_1x1',44,count=80,tiling=1|(1<<7)|(1<<28))
    case('v44_width_clamp',44,count=30000,width=30001,height=1)
    case('v44_height_clamp',44,count=30000,width=1,height=30001)
    report=dict(scope=__doc__,oracle=pas,cpp=cpp,cases=rows,counts=dict(Counter(r['status'] for r in rows)),
                script_sha256=sha(__file__),production_modified=False)
    (out/'summary.json').write_text(json.dumps(report,indent=2));print(report['counts'])


if __name__=='__main__':main()
