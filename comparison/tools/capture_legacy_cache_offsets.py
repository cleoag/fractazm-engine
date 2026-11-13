"""Independent bounded original file-loader cache-offset witnesses (no render)."""
import argparse
import json
import os
from pathlib import Path
import shutil
import struct
import subprocess

from capture_legacy_loader_tails import sha
from check_legacy_loader import legacy
from external_validation import ROOT, decode

def main():
    ap=argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--output',type=Path,required=True)
    a=ap.parse_args();out=a.output.resolve();out.mkdir(parents=True,exist_ok=False)
    original=ROOT/'comparison/windows_build/Mandelbulb3D_loader_audit.exe'
    if sha(original)!='b9cd0e578f6f41dcdd648d094c1ee3b6ff29a284985f62ea5bf566b57af7cf99':
        raise ValueError('Unexpected original loader executable')
    runtime=out/'runtime';runtime.mkdir();exe=runtime/original.name
    shutil.copy2(original,exe);shutil.copy2(original.parent/'portable_math.dll',runtime/'portable_math.dll')
    ini=(original.parent/'Mandelbulb3D.ini').read_text().replace(str(original.parent),str(runtime))
    for asset in ['M3Maps','M3Formulas']: ini=ini.replace(str(runtime/asset),str(ROOT/'mb3d'/asset))
    (runtime/'Mandelbulb3D.ini').write_text(ini)
    base=decode(ROOT/'comparison/scenes_phase2/phase2_intpow2_small.m3p');rows=[]
    # >=19 uses GetTileSize for cached-record seek; <=18 deliberately uses full dimensions.
    for mid in (18,19,44):
        for width,height in ((30001,1),(1,30001)):
            struct.pack_into('<2i',base,4,width,height)
            data=legacy(base,mid);header_size=704 if mid<20 else 840
            stored_width=min(30000,width) if mid>=19 else width
            stored_height=min(30000,height) if mid>=19 else height
            cache_size=stored_width*stored_height*(18 if mid>=19 else 16)
            data=data[:header_size]+bytes(cache_size)+data[header_size:]
            name=f'v{mid}_cache_clamp_{width}x{height}'
            scene=out/(name+'.m3p');scene.write_bytes(data);dumps=[]
            for repeat in (False,True):
                dump=out/(name+('.repeat' if repeat else '')+'.pascal.bin')
                env=dict(os.environ);env['MB3D_HEADLESS_LOADER_DUMP']=str(dump)
                proc=subprocess.run([str(exe),'--render',str(scene),'--output',str(out/'unused.png'),'--threads','1'],
                    cwd=runtime,env=env,capture_output=True,timeout=12)
                (out/(name+('.repeat' if repeat else '')+'.log')).write_bytes(proc.stdout+b'\n'+proc.stderr)
                if proc.returncode or not dump.exists(): raise ValueError('Original cache seek capture failed: '+name)
                dumps.append(dump)
            if dumps[0].read_bytes()!=dumps[1].read_bytes(): raise ValueError('Repeat differs: '+name)
            rows.append(dict(id=name,path=str(scene),sha256=sha(scene),pascal_sha256=sha(dumps[0]),
                repeat_sha256=sha(dumps[1]),pascal_exit=0,original_repeat_exact=True,
                stored_pixel_bytes=cache_size,addon_offset=header_size+cache_size))
    report=dict(scope=__doc__,pascal_sha256=sha(exe),finished=True,results=rows,
        sources={str(p.relative_to(ROOT)):sha(p) for p in [ROOT/'mb3d/FileHandling.pas',ROOT/'mb3d/DivUtils.pas',Path(__file__)]})
    (out/'summary.json').write_text(json.dumps(report,indent=2)+'\n');print('Original capture/repeat:',len(rows),'exact')

if __name__=='__main__':main()
