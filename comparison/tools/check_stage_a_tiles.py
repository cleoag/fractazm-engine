"""Finite ordinary tile references/replay; preserve original undefined postprocessor paths."""
from __future__ import annotations
import argparse
from collections import Counter
from dataclasses import asdict
import hashlib
import json
import os
from pathlib import Path
import struct
import subprocess

from image_diff import load_rgb, compute_metrics, evaluate, DEFAULT_THRESHOLDS
from compare_silight5 import read_dump, FIELD_NAMES
from external_validation import decode, slots
from pascal_formula_identity import validate_formula_load

ROOT = Path(__file__).resolve().parents[2]
OUT = ROOT / 'comparison/results/stage-a-20261018/tiles-v1'
sha = lambda p: hashlib.sha256(p.read_bytes()).hexdigest()

def save(path, value):
    path.write_text(json.dumps(value, indent=2) + '\n', encoding='utf-8')

def run(command, timeout=45):
    env = os.environ.copy()
    env.pop('MB3D_HEADLESS_BATCH', None)
    env.pop('MB3D_HEADLESS_LOADER_DUMP', None)
    try:
        p = subprocess.run(command, cwd=Path(command[0]).parent, env=env,
                           capture_output=True, timeout=timeout,
                           creationflags=subprocess.CREATE_NO_WINDOW if os.name == 'nt' else 0)
        return p.returncode, p.stdout + b'\n' + p.stderr
    except subprocess.TimeoutExpired as e:
        return 'timeout', (e.stdout or b'') + b'\n' + (e.stderr or b'')

def references(exe):
    if (OUT/'manifest.json').exists(): raise ValueError('Reference folder already exists')
    (OUT/'scenes').mkdir(parents=True, exist_ok=True)
    (OUT/'pascal').mkdir(exist_ok=True)
    definitions = [('control', 1, 1, 1, {}), ('first_crop0', 0, 0, 0, {}),
                   ('first_crop2', 0, 0, 2, {}), ('last_crop3', 2, 1, 3, {}),
                   ('planar', 1, 1, 1, {148: 1}), ('panorama', 1, 1, 1, {148: 2}),
                   ('normals_depth', 1, 1, 1, {}), ('shadow_hard', 1, 1, 1, {}),
                   ('shadow_soft', 1, 1, 1, {}), ('ao_de_dither2', 1, 1, 1, {}),
                   ('reflect', 1, 1, 1, {}), ('transmit', 1, 1, 1, {}),
                   ('slice', 1, 1, 1, {344: 0})]
    cases = []
    for name, px, py, crop, patches in definitions:
        source = OUT.parent/'effective-modes-v1/scenes'/('effective_'+name+'.m3p')
        if not source.exists(): source = OUT.parent/'effective-modes-v1/scenes/effective_control.m3p'
        data = decode(source)
        for off, value in patches.items(): data[off] = value
        options = 3 | (2 << 7) | (px << 14) | (py << 21) | (crop << 28)
        struct.pack_into('<I', data, 428, options)
        scene = OUT/'scenes'/('tile_'+name+'.m3p'); scene.write_bytes(data)
        cases.append(dict(id=scene.stem, scene=str(scene.relative_to(ROOT)), sha256=sha(scene),
                          source=str(source.relative_to(ROOT)), source_sha256=sha(source),
                          slots=slots(data), expected_size=[32+2*max(1,crop),36+2*max(1,crop)]))
    manifest = dict(pascal_exe=str(exe), pascal_sha256=sha(exe), cases=cases)
    save(OUT/'manifest.json', manifest)
    report = dict(results=[], scope='finite ordinary tiles; MC/JIT excluded',
                  dump_contract='Pascal dump width/height describe full camera; count describes tile. Compare records with explicit PNG dimensions.')
    for case in cases:
        image = OUT/'pascal'/(case['id']+'.png'); dump=image.with_suffix('.silight')
        command=[str(exe),'--render',str(ROOT/case['scene']),'--output',str(image),'--threads','1',
                 '--silight5-dump',str(dump),'--silight5-dump-live']
        code, log=run(command); image.with_suffix('.log').write_bytes(log)
        row=dict(id=case['id'], command=command, returncode=code, status='reference_error')
        if code==0 and image.exists() and dump.exists():
            rgb=load_rgb(image); w,h,records=read_dump(dump)
            row.update(status='reference_ready',png_sha256=sha(image),dump_sha256=sha(dump),
                       dimensions=[rgb.shape[1],rgb.shape[0]],dump_camera_dimensions=[w,h],records=len(records),
                       nonconstant=bool((rgb!=rgb[0,0]).any()),
                       formula_identity=validate_formula_load(log.decode('cp1252',errors='replace'),ROOT/case['scene'],case['slots']))
            if row['dimensions']!=case['expected_size'] or len(records)!=rgb.shape[0]*rgb.shape[1]:
                raise ValueError('Tile dimension/count mismatch')
        report['results'].append(row);save(OUT/'references.json',report)
        print(case['id'],row['status'],flush=True)

def replay(exe, output, repeats=False):
    output.mkdir(parents=True,exist_ok=True)
    manifest=json.loads((OUT/'manifest.json').read_text())
    refs=json.loads((OUT/'references.json').read_text())
    qualified={r['id']:r for r in refs['results']}
    report=dict(cpp_exe=str(exe),cpp_sha256=sha(exe),thresholds=DEFAULT_THRESHOLDS,
                reference_dir=str(OUT),reference_manifest_sha256=sha(OUT/'manifest.json'),
                reference_report_sha256=sha(OUT/'references.json'),
                pascal_sha256=manifest['pascal_sha256'],results=[])
    for case in manifest['cases']:
        scene=ROOT/case['scene'];assert sha(scene)==case['sha256']
        reference=qualified[case['id']]
        assert reference['status']=='reference_ready' and reference['formula_identity']['verified']
        assert sha(OUT/'pascal'/(case['id']+'.png'))==reference['png_sha256']
        assert sha(OUT/'pascal'/(case['id']+'.silight'))==reference['dump_sha256']
        image=output/(case['id']+'.png');dump=image.with_suffix('.silight')
        command=[str(exe),str(scene),'--output',str(image),'--threads','1','--silight5-dump',str(dump)]
        code,log=run(command);image.with_suffix('.log').write_bytes(log)
        row=dict(id=case['id'],command=command,returncode=code,status='ERROR')
        if code==0:
            a=load_rgb(OUT/'pascal'/image.name);b=load_rgb(image)
            metrics=compute_metrics(a,b,DEFAULT_THRESHOLDS['diff_ratio_threshold'])
            accepted=evaluate(metrics,DEFAULT_THRESHOLDS)
            ar=read_dump(OUT/'pascal'/dump.name)[2];w,h,br=read_dump(dump)
            assert len(ar)==len(br)==w*h
            row.update(status='PASS' if all(accepted.values()) else 'FAIL',metrics=asdict(metrics),exact_rgb=bool((a==b).all()),
                       changed_fields={name:sum(x[i]!=y[i] for x,y in zip(ar,br)) for i,name in enumerate(FIELD_NAMES)},
                       image_sha256=sha(image),dump_sha256=sha(dump))
        report['results'].append(row);report['counts']=dict(Counter(r['status'] for r in report['results']))
        save(output/'summary.json',report);print(case['id'],row['status'],row.get('metrics',{}).get('mean_abs_delta'),flush=True)
    if repeats:
        report['repeats']=[]
        for reference in refs.get('repeats',[]):
            case=next(c for c in manifest['cases'] if c['id']==reference['id'])
            image=output/(case['id']+'-'+reference['kind']+'.png');dump=image.with_suffix('.silight')
            command=[str(exe),str(ROOT/case['scene']),'--output',str(image),'--threads','1','--silight5-dump',str(dump)]
            if reference['kind']=='trace':
                command+=['--diag-points','4,4','--parity-trace',str(image.with_suffix('.trace')),
                          '--parity-trace-schema','--parity-trace-level','checkpoint']
            code,log=run(command);image.with_suffix('.log').write_bytes(log)
            row=dict(id=case['id'],kind=reference['kind'],command=command,returncode=code,status='ERROR')
            if code==0:
                original=output/(case['id']+'.png')
                a=load_rgb(original);b=load_rgb(image)
                ar=read_dump(original.with_suffix('.silight'))[2];br=read_dump(dump)[2]
                assert len(ar)==len(br)
                fields={name:sum(x[i]!=y[i] for x,y in zip(ar,br)) for i,name in enumerate(FIELD_NAMES)}
                exact=bool((a==b).all())
                row.update(status='PASS' if exact and not any(fields.values()) else 'FAIL',
                           exact_rgb=exact,changed_fields=fields,image_sha256=sha(image),dump_sha256=sha(dump))
            report['repeats'].append(row)
            report['repeat_counts']=dict(Counter(r['status'] for r in report['repeats']))
            save(output/'summary.json',report);print(case['id'],reference['kind'],row['status'],flush=True)

if __name__=='__main__':
    ap=argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--pascal-exe',type=Path);ap.add_argument('--cpp-exe',type=Path);ap.add_argument('--output',type=Path)
    ap.add_argument('--reference-dir',type=Path,default=OUT)
    ap.add_argument('--repeats',action='store_true',help='Repeat the reference-qualified plain/trace control set on C++')
    args=ap.parse_args()
    OUT=args.reference_dir.resolve()
    if args.cpp_exe and not args.output:ap.error('--cpp-exe requires --output')
    if args.pascal_exe:references(args.pascal_exe.resolve())
    if args.cpp_exe:replay(args.cpp_exe.resolve(),args.output.resolve(),args.repeats)
