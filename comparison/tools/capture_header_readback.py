"""Compare raw LoadParameter output with actual Pascal GUI MakeHeader readback.

Requires the diagnostic-only hook recorded in direct-header-contract-v1/file-readback:
HeadlessDumpLoadedParameters calls Mand3DForm.MakeHeader when
MB3D_HEADLESS_AFTER_READBACK=1, before the existing pointer-normalized dump/exit.
The immutable Pascal tree is never modified; normal rendering is unaffected.
"""
import argparse
import hashlib
import json
import os
from pathlib import Path
import struct
import subprocess

from external_validation import ROOT, decode


def main():
    ap=argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--pascal-exe',type=Path,required=True)
    ap.add_argument('--output',type=Path,required=True)
    args=ap.parse_args()
    out=args.output.resolve();out.mkdir(parents=True,exist_ok=False)
    exe=args.pascal_exe.resolve()
    sha=lambda b:hashlib.sha256(b).hexdigest()
    base=decode(ROOT/'comparison/scenes/test_21_hybrid.m3p')
    struct.pack_into('<ii',base,4,32,24);base[124]=0
    rows=[]
    for mode in (0,3,7,255):
        b=bytearray(base);b[841]=mode
        scene=out/f'hybrid_{mode}.m3p';scene.write_bytes(b)
        for after in (False,True):
            stem=f'mode{mode}_'+('readback' if after else 'loaded')
            dump=out/(stem+'.bin')
            env=dict(os.environ)
            env['MB3D_HEADLESS_LOADER_DUMP']=str(dump)
            env['MB3D_HEADLESS_AFTER_READBACK']='1' if after else '0'
            command=[str(exe),'--render',str(scene),'--output',str(out/'unused.png'),'--threads','1']
            p=subprocess.run(command,cwd=exe.parent,env=env,capture_output=True,timeout=20)
            log=p.stdout+p.stderr;(out/(stem+'.log')).write_bytes(log)
            if p.returncode or not dump.exists():raise ValueError((stem,p.returncode))
            d=dump.read_bytes()
            if len(d)!=1976:raise ValueError('Unexpected packed header/addon size')
            rows.append(dict(id=stem,raw_mode=mode,after_makeheader=after,effective_mode=d[841],
                hyb1=d[845],hyb2=struct.unpack_from('<H',d,846)[0],command=command,
                scene_sha256=sha(b),dump_sha256=sha(d),log_sha256=sha(log),exit_code=p.returncode))
            print(stem,d[841],flush=True)
    report=dict(scope='Original file load vs GUI MakeHeader readback; distinct from direct GetMCT',
                pascal_exe=str(exe),pascal_sha256=sha(exe.read_bytes()),
                tool_sha256=sha(Path(__file__).read_bytes()),results=rows,
                source_sha256={p:sha((ROOT/p).read_bytes()) for p in
                    ('mb3d/Mand.pas','mb3d/formula/FormulaGUI.pas','mb3d/formula/FormulaGUI.lfm')})
    (out/'summary.json').write_text(json.dumps(report,indent=2)+'\n')


if __name__=='__main__':main()
