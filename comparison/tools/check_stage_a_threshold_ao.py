"""Finite threshold AO scalar/SSE/MMX boundaries against completed Pascal."""
from pathlib import Path
import argparse,json,struct
import numpy as np
from check_stage_a_tiles import ROOT,run,save,sha
from check_stage_a_ao_lanes import replay
from compare_silight5 import FIELD_NAMES,read_dump
from external_validation import decode,slots
from image_diff import load_rgb
from pascal_formula_identity import validate_formula_load

def capture(exe,out):
    assert not (out/'manifest.json').exists()
    (out/'scenes').mkdir(parents=True);(out/'pascal').mkdir()
    source=ROOT/'comparison/results/stage-a-20261018/effective-modes-v1/scenes/effective_control.m3p'
    cases=[]
    for mode in (0,1):
        for width,height,threshold in ((32,24,2.),(96,72,2.),(97,73,2.),(96,72,.001)):
            data=decode(source);data[149]=1+2*mode
            struct.pack_into('<ii',data,4,width,height);struct.pack_into('<f',data,319,threshold)
            name=f'threshold_ao{mode}_{width}x{height}_'+('floor' if threshold<.01 else 'normal')
            scene=out/'scenes'/(name+'.m3p');scene.write_bytes(data)
            cases.append(dict(id=name,mode=mode,repeat=1,threads=1,threshold=threshold,width=width,height=height,
                scene=str(scene.relative_to(ROOT)),sha256=sha(scene),slots=slots(data)))
    save(out/'manifest.json',dict(pascal_exe=str(exe),pascal_sha256=sha(exe),source=str(source.relative_to(ROOT)),
         source_sha256=sha(source),cases=cases,profile='completed ordinary v3, original SSE/MMX enabled'))
    report=dict(results=[],repeats=[])
    for case in cases:
        for kind in ('plain','repeat','trace'):
            image=out/'pascal'/(case['id']+('' if kind=='plain' else '-'+kind)+'.png');dump=image.with_suffix('.silight')
            cmd=[str(exe),'--render',str(ROOT/case['scene']),'--output',str(image),'--threads','1',
                 '--silight5-dump',str(dump),'--silight5-dump-live']
            if kind=='trace':cmd+=['--diag-pixel','16,12','--parity-trace',str(image.with_suffix('.trace')),'--parity-trace-level','checkpoint']
            code,log=run(cmd);image.with_suffix('.log').write_bytes(log);assert code==0,(case['id'],code)
            identity=validate_formula_load(log.decode('cp1252',errors='replace'),ROOT/case['scene'],case['slots']);assert identity['verified']
            rgb=load_rgb(image);w,h,records=read_dump(dump);assert(w,h)==(case['width'],case['height']) and len(records)==w*h
            row=dict(id=case['id'],kind=kind,command=cmd,returncode=code,status='reference_ready',formula_identity=identity,
                     png_sha256=sha(image),dump_sha256=sha(dump),nonconstant=bool((rgb!=rgb[0,0]).any()),dimensions=[w,h])
            if kind!='plain':
                base=out/'pascal'/(case['id']+'.png');ar=read_dump(base.with_suffix('.silight'))[2]
                fields={n:sum(a[i]!=b[i]for a,b in zip(ar,records))for i,n in enumerate(FIELD_NAMES)}
                row.update(exact_rgb=bool(np.array_equal(load_rgb(base),rgb)),changed_fields=fields)
                assert row['exact_rgb'] and not any(fields.values())
            report['results'if kind=='plain'else'repeats'].append(row);save(out/'references.json',report)
            print(case['id'],kind,flush=True)
if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--pascal-exe',type=Path);p.add_argument('--cpp-exe',type=Path)
    p.add_argument('--reference-dir',type=Path);p.add_argument('--output',type=Path,required=True);a=p.parse_args()
    if a.pascal_exe:capture(a.pascal_exe.resolve(),a.output.resolve())
    if a.cpp_exe:replay(a.cpp_exe.resolve(),a.reference_dir.resolve(),a.output.resolve())
