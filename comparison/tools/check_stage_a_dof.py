"""Finite completed ordinary DOF comparisons, with retained depth-region evidence.

The Pascal headless driver must wait for DOF and preserve its final image. These
images do not reuse historical captures made before that completion correction.
Depth-region classification is source-derived from the retained original depth;
it is not instrumented branch coverage or a floating-point oracle.
"""
import argparse
from collections import Counter
from dataclasses import asdict
import json
import math
from pathlib import Path
import struct

from check_stage_a_tiles import ROOT, run, sha
from compare_silight5 import read_dump, packed_z
from external_validation import decode, slots, write_json
from image_diff import load_rgb, compute_metrics, evaluate, DEFAULT_THRESHOLDS
from pascal_formula_identity import validate_formula_load


def generate(output):
    manifest = json.loads((output/'manifest.json').read_bytes())
    if len(manifest['cases']) != 1:
        return manifest
    control = manifest['cases'][0]
    data = decode(ROOT/control['scene'])
    definitions = [(f'{driver}_mixed_pass{mode}', bit | 1 | (mode << 1), .38, .55, 25.)
                   for driver, bit in [('sorted', 0), ('plain', 8)] for mode in range(3)]
    for driver, bit in [('sorted', 0), ('plain', 8)]:
        for region, near, far, clip in [('foreground',1.2,1.2,25.), ('background',0.,0.,25.),
                                      ('clipped',.38,.55,.1)]:
            definitions.append((driver+'_'+region, bit|1, near, far, clip))
    for tag, mode, near, far, clip in definitions:
        h = bytearray(data); h[181] = mode
        struct.pack_into('<ff',h,164,near,clip)
        struct.pack_into('<f',h,410,far)
        struct.pack_into('<f',h,172,1.25)
        scene=output/'scenes'/('dof_'+tag+'.m3p'); scene.write_bytes(h)
        manifest['cases'].append(dict(id=scene.stem,scene=str(scene.relative_to(ROOT)),sha256=sha(scene),
            slots=slots(h),source=control['scene'],source_sha256=control['sha256'],
            selector=dict(driver='plain' if mode&8 else 'sorted',passes=((mode>>1)&3)+1),
            focus=[near,far],clip=clip,aperture=1.25))
    manifest['generator_sha256']=sha(Path(__file__))
    write_json(output/'manifest.json',manifest)
    return manifest


def regions(header, dump):
    width,height,records=read_dump(dump)
    get=lambda fmt,offset:struct.unpack_from(fmt,header,offset)[0]
    fov=get('<d',108);step=2.1345/(get('<d',84)*width)
    corr=math.sin(max(fov*math.pi/180,1)/height)
    mul=(math.sqrt((get('<d',28)-get('<d',20))*corr/step+1)-.999999999)/8388352
    near,far=sorted([get('<f',164)*width,get('<f',410)*width])
    factor=max(0,fov*math.pi/180)/height
    ratio=(1+near*factor)/(1+far*factor); focus=(ratio*far+near)/(1+ratio)
    counts=Counter();witness={}
    for index,record in enumerate(records):
        packed=packed_z(record)
        depth=((1+(8388353 if packed is None else 8388352-packed)*mul)**2-1)/corr
        tag='no_hit' if packed is None else ('foreground' if depth<near else 'background' if depth>far else 'focus_band')
        counts[tag]+=1;witness.setdefault(tag,[index%width,index//width])
        # First pass, sufficient to establish edge/clip candidates; exact
        # kernel branch outcomes at rounding boundaries are not claimed.
        mode=(header[181]>>1)&3
        aperture=get('<f',172)*(.5 if mode==0 else .125 if header[181]&8==0 and mode==1 else .15 if mode==1 else 1/22 if header[181]&8==0 else 1/12)
        radius=max(0,abs((depth-focus)/(1+depth*factor)*aperture)-abs((near-focus)/(1+near*factor)*aperture))
        if radius>get('<f',168): counts['clip_radius_candidate']+=1;witness.setdefault('clip_radius_candidate',[index%width,index//width])
        if radius>.5 and (index%width in (0,width-1) or index//width in (0,height-1)):
            counts['clipped_image_edge_candidate']+=1;witness.setdefault('clipped_image_edge_candidate',[index%width,index//width])
    return dict(counts=dict(counts),witness_pixels=witness,qualification='source-derived regions; not instrumented exact branch counts')


def main():
    ap=argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--output',type=Path,required=True)
    ap.add_argument('--pascal-exe',type=Path)
    ap.add_argument('--cpp-exe',type=Path)
    a=ap.parse_args();a.output=a.output.resolve()
    manifest=generate(a.output)
    reference=a.pascal_exe is not None
    exe=(a.pascal_exe or a.cpp_exe).resolve()
    target=a.output/('pascal' if reference else exe.parent.name);target.mkdir(exist_ok=True)
    report=dict(scope=__doc__,binary_sha256=sha(exe),manifest_sha256=sha(a.output/'manifest.json'),
                thresholds=DEFAULT_THRESHOLDS,results=[],finished=False)
    control=load_rgb(a.output/'pascal/dof_control.png')
    for case in manifest['cases']:
        scene=ROOT/case['scene'];assert sha(scene)==case['sha256']
        image=target/(case['id']+'.png');dump=image.with_suffix('.silight')
        cmd=[str(exe)]+(['--render'] if reference else [])+[str(scene),'--output',str(image),'--threads','1','--silight5-dump',str(dump)]+(['--silight5-dump-live'] if reference else [])
        code,log=run(cmd);image.with_suffix('.log').write_bytes(log)
        row=dict(id=case['id'],command=cmd,returncode=code,status='ERROR',scene_sha256=sha(scene))
        if code==0 and image.exists() and dump.exists():
            rgb=load_rgb(image)
            row.update(status='reference_ready' if reference else 'PASS',image_sha256=sha(image),depth_sha256=sha(dump),
                       depth_regions=regions(decode(scene),dump),changed_from_reference_control=int((rgb!=control).any(axis=2).sum()))
            if reference:
                row['formula_identity']=validate_formula_load(log.decode('cp1252',errors='replace'),scene,case['slots'])
            else:
                ref=a.output/'pascal'/image.name
                reference_rgb=load_rgb(ref)
                m=compute_metrics(reference_rgb,rgb,DEFAULT_THRESHOLDS['diff_ratio_threshold'])
                row.update(metrics=asdict(m),status='PASS' if all(evaluate(m,DEFAULT_THRESHOLDS).values()) else 'FAIL',
                           exact_rgb=bool((rgb==reference_rgb).all()),reference_sha256=sha(ref),
                           exact_depth=read_dump(dump)[2]==read_dump(ref.with_suffix('.silight'))[2])
        report['results'].append(row);report['counts']=dict(Counter(x['status'] for x in report['results']))
        write_json(target/'summary.json',report);print(case['id'],row['status'],flush=True)
    report['finished']=True;write_json(target/'summary.json',report)


if __name__=='__main__':main()
