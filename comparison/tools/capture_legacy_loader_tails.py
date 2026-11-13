"""Capture independent fresh-application legacy external default-tail witnesses."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import shutil
import struct
import subprocess

from check_legacy_loader import legacy
from external_validation import ROOT, decode

def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()

def main():
    ap=argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--output',type=Path,required=True)
    a=ap.parse_args();out=a.output.resolve();out.mkdir(parents=True,exist_ok=False)
    runtime=out/'runtime';runtime.mkdir()
    original=ROOT/'comparison/windows_build/Mandelbulb3D_loader_audit.exe'
    if sha(original)!='b9cd0e578f6f41dcdd648d094c1ee3b6ff29a284985f62ea5bf566b57af7cf99':
        raise ValueError('Unexpected Pascal loader oracle')
    exe=runtime/'Mandelbulb3D_loader_audit.exe';shutil.copy2(original,exe)
    shutil.copy2(original.parent/'portable_math.dll',runtime/'portable_math.dll')
    ini=(original.parent/'Mandelbulb3D.ini').read_text()
    ini=ini.replace(str(original.parent),str(runtime))
    ini=ini.replace(str(runtime/'M3Formulas'),str(ROOT/'mb3d/M3Formulas'))
    ini=ini.replace(str(runtime/'M3Maps'),str(ROOT/'mb3d/M3Maps'))
    (runtime/'Mandelbulb3D.ini').write_text(ini)
    base=decode(ROOT/'comparison/scenes_phase2/phase2_intpow2_small.m3p')
    struct.pack_into('<2i',base,4,8,6)
    rows=[]
    for hybrid in (False,True):
        name='v15_ABoxMod1_'+('hybrid' if hybrid else 'named')
        data=legacy(base,15,fn=9 if hybrid else 11)
        data[328:344]=b'ABoxMod1'+bytes(8)
        struct.pack_into('<d',data,370,6.25)
        struct.pack_into('<2d',data,215,.75,42.)
        if hybrid:
            data[231:234]=bytes([8,8,8]);struct.pack_into('<3i',data,234,1,2,3)
        scene=out/(name+'.m3p');scene.write_bytes(data)
        dumps=[]
        for repeat in (False,True):
            dump=out/(name+('.repeat' if repeat else '')+'.pascal.bin')
            env=dict(os.environ);env['MB3D_HEADLESS_LOADER_DUMP']=str(dump)
            command=[str(exe),'--render',str(scene),'--output',str(out/'unused.png'),'--threads','1']
            proc=subprocess.run(command,cwd=runtime,env=env,capture_output=True,timeout=12)
            (out/(name+('.repeat' if repeat else '')+'.log')).write_bytes(proc.stdout+b'\n'+proc.stderr)
            if proc.returncode or not dump.exists():raise ValueError('Original load-only capture failed')
            dumps.append(dump)
        if dumps[0].read_bytes()!=dumps[1].read_bytes():raise ValueError('Original repeat is not invariant')
        rows.append(dict(id=name,path=str(scene),sha256=sha(scene),pascal_sha256=sha(dumps[0]),
            repeat_sha256=sha(dumps[1]),pascal_exit=0,original_repeat_exact=True))
    report=dict(scope=__doc__,pascal_sha256=sha(exe),results=rows,finished=True,
        source_sha256={str(p.relative_to(ROOT)):sha(p) for p in [
            ROOT/'mb3d/FileHandling.pas',ROOT/'mb3d/HeaderTrafos.pas',ROOT/'mb3d/Mand.pas',
            ROOT/'mb3d/M3Formulas/ABoxMod1.m3f',Path(__file__)]})
    (out/'summary.json').write_text(json.dumps(report,indent=2)+'\n')
    print(json.dumps(rows))

if __name__=='__main__':main()
