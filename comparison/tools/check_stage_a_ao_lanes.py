"""Qualify ordinary HiQ random AO wavelet/lane state against the completed Pascal profile."""
from __future__ import annotations

import argparse
from collections import Counter
from dataclasses import asdict
import json
from pathlib import Path

import numpy as np

from check_stage_a_tiles import ROOT, run, save, sha
from compare_silight5 import FIELD_NAMES, read_dump
from external_validation import decode, slots
from image_diff import DEFAULT_THRESHOLDS, compute_metrics, evaluate, load_rgb
from pascal_formula_identity import validate_formula_load


def capture(exe, output, pano=False):
    if (output/'manifest.json').exists():
        raise ValueError('Use a fresh reference directory')
    (output/'scenes').mkdir(parents=True, exist_ok=True)
    (output/'pascal').mkdir()
    source=ROOT/'comparison/results/stage-a-20261018/effective-modes-v1/scenes/effective_control.m3p'
    cases=[]
    for mode in (4,5):
        for repeat in (1,2):
            for threads in (1,2):
                data=decode(source)
                data[149]=1+2*mode
                data[187]=repeat
                data[148]=2 if pano else 0
                name=('pano_' if pano else '')+f'ao{mode}_repeat{repeat}_threads{threads}'
                scene=output/'scenes'/(name+'.m3p');scene.write_bytes(data)
                cases.append(dict(id=name,mode=mode,repeat=repeat,threads=threads,
                                  scene=str(scene.relative_to(ROOT)),sha256=sha(scene),slots=slots(data)))
    save(output/'manifest.json',dict(pascal_exe=str(exe),pascal_sha256=sha(exe),source=str(source.relative_to(ROOT)),
                                   source_sha256=sha(source),cases=cases,
                                   profile='completed ordinary profile v3 plus existing deterministic Pascal LCG initialization'))
    report=dict(results=[],repeats=[])
    for case in cases:
        for kind in ('plain','repeat','trace'):
            image=output/'pascal'/(case['id']+('' if kind=='plain' else '-'+kind)+'.png')
            dump=image.with_suffix('.silight')
            command=[str(exe),'--render',str(ROOT/case['scene']),'--output',str(image),
                     '--threads',str(case['threads']),'--silight5-dump',str(dump),'--silight5-dump-live']
            if kind=='trace':
                command+=['--diag-pixel','48,36','--parity-trace',str(image.with_suffix('.trace')),
                          '--parity-trace-level','checkpoint']
            code,log=run(command);image.with_suffix('.log').write_bytes(log)
            row=dict(id=case['id'],kind=kind,command=command,returncode=code,status='ERROR')
            if code==0:
                rgb=load_rgb(image);w,h,records=read_dump(dump)
                identity=validate_formula_load(log.decode('cp1252',errors='replace'),ROOT/case['scene'],case['slots'])
                assert identity['verified'] and rgb.shape[:2]==(h,w) and len(records)==w*h
                row.update(status='reference_ready',formula_identity=identity,png_sha256=sha(image),dump_sha256=sha(dump),
                           nonconstant=bool((rgb!=rgb[0,0]).any()),dimensions=[w,h])
                if kind!='plain':
                    base=output/'pascal'/(case['id']+'.png')
                    ar=read_dump(base.with_suffix('.silight'))[2]
                    fields={n:sum(a[i]!=b[i] for a,b in zip(ar,records)) for i,n in enumerate(FIELD_NAMES)}
                    row.update(exact_rgb=bool(np.array_equal(load_rgb(base),rgb)),changed_fields=fields)
                    assert row['exact_rgb'] and not any(fields.values())
            report['results' if kind=='plain' else 'repeats'].append(row)
            save(output/'references.json',report);print(case['id'],kind,row['status'],flush=True)


def replay(exe, references, output):
    output.mkdir(parents=True,exist_ok=True)
    manifest=json.loads((references/'manifest.json').read_text())
    refs=json.loads((references/'references.json').read_text())
    report=dict(cpp_exe=str(exe),cpp_sha256=sha(exe),reference_dir=str(references),
                reference_manifest_sha256=sha(references/'manifest.json'),thresholds=DEFAULT_THRESHOLDS,results=[],repeats=[])
    for case,ref in zip(manifest['cases'],refs['results']):
        assert case['id']==ref['id'] and ref['formula_identity']['verified']
        original=references/'pascal'/(case['id']+'.png')
        assert sha(original)==ref['png_sha256'] and sha(original.with_suffix('.silight'))==ref['dump_sha256']
        scene=ROOT/case['scene'];assert sha(scene)==case['sha256']
        for kind in ('plain','repeat','trace'):
            image=output/(case['id']+('' if kind=='plain' else '-'+kind)+'.png');dump=image.with_suffix('.silight')
            command=[str(exe),str(scene),'--output',str(image),'--threads',str(case['threads']),'--silight5-dump',str(dump)]
            if kind=='trace':
                command+=['--diag-points','48,36','--parity-trace',str(image.with_suffix('.trace')),
                          '--parity-trace-schema','--parity-trace-level','checkpoint']
            code,log=run(command);image.with_suffix('.log').write_bytes(log)
            row=dict(id=case['id'],kind=kind,command=command,returncode=code,status='ERROR')
            if code==0:
                original=(references/'pascal' if kind=='plain' else output)/(case['id']+'.png')
                a=load_rgb(original);b=load_rgb(image)
                ar=read_dump(original.with_suffix('.silight'))[2];br=read_dump(dump)[2]
                assert len(ar)==len(br)
                fields={n:sum(x[i]!=y[i] for x,y in zip(ar,br)) for i,n in enumerate(FIELD_NAMES)}
                metrics=compute_metrics(a,b,DEFAULT_THRESHOLDS['diff_ratio_threshold'])
                exact=bool(np.array_equal(a,b))
                accepted=all(evaluate(metrics,DEFAULT_THRESHOLDS).values()) if kind=='plain' else exact and not any(fields.values())
                row.update(status='PASS' if accepted else 'FAIL',exact_rgb=exact,changed_fields=fields,
                           exact_state=not any(fields.values()),state_status='PASS' if not any(fields.values()) else 'FAIL',
                           metrics=asdict(metrics),image_sha256=sha(image),dump_sha256=sha(dump))
            report['results' if kind=='plain' else 'repeats'].append(row)
            report['counts']=dict(Counter(r['status'] for r in report['results']))
            report['repeat_counts']=dict(Counter(r['status'] for r in report['repeats']))
            report['state_counts']=dict(Counter(r.get('state_status','ERROR') for r in report['results']))
            save(output/'summary.json',report);print(case['id'],kind,row['status'],row.get('changed_fields'),flush=True)


if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--pascal-exe',type=Path);p.add_argument('--cpp-exe',type=Path)
    p.add_argument('--reference-dir',type=Path);p.add_argument('--output',type=Path,required=True)
    p.add_argument('--pano',action='store_true',help='Capture the panoramic counterparts of the same RNG lane families')
    a=p.parse_args()
    if a.pascal_exe:capture(a.pascal_exe.resolve(),a.output.resolve(),a.pano)
    if a.cpp_exe:replay(a.cpp_exe.resolve(),a.reference_dir.resolve(),a.output.resolve())
