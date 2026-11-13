"""Qualify completed ordinary Pascal tile stages and full-frame-prefix capacity."""
from pathlib import Path
import argparse
import json
import shutil
import struct

import numpy as np

from check_stage_a_tiles import ROOT, run, save, sha, load_rgb
from compare_silight5 import read_dump, FIELD_NAMES
from external_validation import decode, slots
from pascal_formula_identity import validate_formula_load


def main(exe, output):
    if (output/'manifest.json').exists():
        raise ValueError('Reference folder already exists; preserve previous captures and use a new output folder')
    output.mkdir(parents=True, exist_ok=True)
    (output/'scenes').mkdir(exist_ok=True)
    (output/'pascal').mkdir(exist_ok=True)
    previous = output.parent/'tiles-v1'
    manifest = json.loads((previous/'manifest.json').read_text())
    cases = manifest['cases']
    for case in cases:
        target = output/'scenes'/Path(case['scene']).name
        shutil.copy2(ROOT/case['scene'], target)
        case['scene'] = str(target.relative_to(ROOT))
    control = decode(output.parent/'effective-modes-v1/scenes/effective_control.m3p')
    for name, ao, dof in [('control', 0, 0), ('ao0', 1, 0), ('ao1', 5, 0),
                          ('ao2', 9, 0), ('dof_sorted', 0, 1), ('dof_plain', 0, 9),
                          ('dof_sorted_visible', 0, 1), ('dof_plain_visible', 0, 9)]:
        data = bytearray(control)
        struct.pack_into('<I', data, 428, 1 | (1 << 7) | (1 << 28))
        data[149] = ao; data[181] = dof
        if name.endswith('_visible'): struct.pack_into('<f', data, 172, 1.25)
        scene = output/'scenes'/f'tile_safe_{name}.m3p';scene.write_bytes(data)
        cases.append(dict(id=scene.stem, scene=str(scene.relative_to(ROOT)), sha256=sha(scene),
                          slots=slots(data), expected_size=[98, 74], full_frame_prefix=[96, 72]))
    save(output/'manifest.json', dict(pascal_exe=str(exe), pascal_sha256=sha(exe), cases=cases))
    report = dict(profile='completed ordinary pipeline; pending GUI repaint disabled; full tile painting after hard shadows; DOF completion protected from early publish and unblurred repaint; every allocated tile row copied to final bitmap',
                  pascal_exe=str(exe), pascal_sha256=sha(exe), results=[], repeats=[])
    for case in cases:
        image = output/'pascal'/(case['id']+'.png'); dump = image.with_suffix('.silight')
        cmd = [str(exe), '--render', str(ROOT/case['scene']), '--output', str(image), '--threads', '1',
               '--silight5-dump', str(dump), '--silight5-dump-live']
        code, log = run(cmd);image.with_suffix('.log').write_bytes(log)
        row = dict(id=case['id'], command=cmd, returncode=code)
        if code == 0:
            rgb=load_rgb(image);w,h,records=read_dump(dump)
            row.update(status='reference_ready',png_sha256=sha(image),dump_sha256=sha(dump),
                       dimensions=[rgb.shape[1],rgb.shape[0]],dump_camera_dimensions=[w,h],records=len(records),
                       nonconstant=bool((rgb!=rgb[0,0]).any()),
                       formula_identity=validate_formula_load(log.decode('cp1252',errors='replace'),ROOT/case['scene'],case['slots']))
            assert row['dimensions']==case['expected_size'] and len(records)==rgb.shape[0]*rgb.shape[1]
        else: row['status']='reference_error'
        report['results'].append(row);save(output/'references.json',report);print(case['id'],row['status'],flush=True)
    for name in ['shadow_hard','shadow_soft','reflect','transmit','safe_ao0','safe_ao1','safe_ao2','safe_dof_sorted','safe_dof_plain',
                 'safe_dof_sorted_visible','safe_dof_plain_visible']:
        case=next(c for c in cases if c['id']=='tile_'+name)
        for trace in [False, True]:
            suffix='trace' if trace else 'repeat'
            image=output/'pascal'/f'{case["id"]}-{suffix}.png';dump=image.with_suffix('.silight')
            cmd=[str(exe),'--render',str(ROOT/case['scene']),'--output',str(image),'--threads','1',
                 '--silight5-dump',str(dump),'--silight5-dump-live']
            if trace:cmd+=['--diag-pixel','4,4','--parity-trace',str(image.with_suffix('.trace')),'--parity-trace-level','checkpoint']
            code,log=run(cmd);image.with_suffix('.log').write_bytes(log)
            original=output/'pascal'/(case['id']+'.png')
            row=dict(id=case['id'],kind=suffix,command=cmd,returncode=code)
            if code==0:
                a=load_rgb(original);b=load_rgb(image)
                ar=read_dump(original.with_suffix('.silight'))[2];br=read_dump(dump)[2]
                row.update(exact_rgb=bool((a==b).all()),changed_pixels=int(np.any(a!=b,axis=2).sum()),
                           changed_fields={name:sum(x[i]!=y[i] for x,y in zip(ar,br)) for i,name in enumerate(FIELD_NAMES)},
                           image_sha256=sha(image),dump_sha256=sha(dump))
            report['repeats'].append(row);save(output/'references.json',report);print(case['id'],suffix,row.get('exact_rgb'),flush=True)


if __name__=='__main__':
    ap=argparse.ArgumentParser(description=__doc__);ap.add_argument('--pascal-exe',type=Path,required=True);ap.add_argument('--output',type=Path,required=True)
    a=ap.parse_args();main(a.pascal_exe.resolve(),a.output.resolve())
