"""Small ordinary transmission/state matrix using completed Pascal captures.

The retained legacy trace is used only to identify executed control paths. It
is not schema-certified numeric parity. TIR is witnessed when the first body
of an entered transmitting ray already has bCalcT=0: the sole assignment before
that first exit/recurse checkpoint is the original ZZ2<=0 branch. Later bodies
are deliberately excluded because they can be ordinary Fresnel reflection.
"""
import argparse
from collections import Counter
from dataclasses import asdict
import json
from pathlib import Path
import re
import struct

from check_stage_a_tiles import ROOT, run, sha
from external_validation import decode, slots, write_json
from image_diff import load_rgb, compute_metrics, evaluate, DEFAULT_THRESHOLDS
from pascal_formula_identity import validate_formula_load


def witness(path):
    text=path.read_text(errors='replace')
    if not re.search(r'\bsr_ray_enter\b',text,re.I):
        path=path.with_suffix('.log');text=path.read_text(errors='replace')
    entries=[]; pending={}; terminals=[]
    for line_no,line in enumerate(text.splitlines(),1):
        m=re.search(r'\b(sr_ray_enter|sr_recurse_ready|sr_ray_exit)\b',line,re.I)
        if not m:continue
        fields={k:float(v) for k,v in re.findall(r'(\w+)=(-?[\d.]+)',line)}
        rit=int(fields['Rit']); kind=m[1].lower()
        if kind=='sr_ray_enter':
            row=dict(line=line_no,depth=rit,transmitting=bool(fields['bCalcT']))
            entries.append(row); pending[rit]=row
        elif rit in pending:
            row=pending.pop(rit); row['first_body_line']=line_no
            row['first_body_stage']=kind; row['first_body_transmitting']=bool(fields['bCalcT'])
            row['tir']=row['transmitting'] and not row['first_body_transmitting']
        if kind=='sr_ray_exit':terminals.append(dict(line=line_no,depth=rit,open_air=bool(fields['OpenAir']),transmitting=bool(fields['bCalcT'])))
    return dict(path=str(path.relative_to(ROOT)),sha256=sha(path),entries=entries,first_body_tir_count=sum(r.get('tir',False) for r in entries),
                max_depth=max((r['depth'] for r in entries),default=0),terminals=terminals)


def main():
    ap=argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--output',type=Path,required=True)
    ap.add_argument('--zero-power',action='store_true',help='Additional zero diffuse-channel/exponent witnesses; separate output directory required')
    ap.add_argument('--reflection-controls',action='store_true',help='Additional original GUI readback boundary witnesses; separate output directory required')
    ap.add_argument('--pascal-exe',type=Path);ap.add_argument('--cpp-exe',type=Path)
    a=ap.parse_args();a.output=a.output.resolve();a.output.mkdir(parents=True,exist_ok=True)
    if a.zero_power and a.reflection_controls:ap.error('Choose one separate witness matrix')
    source=ROOT/'comparison/results/stage-a-20261018/effective-modes-v1/scenes/effective_transmit.m3p'
    manifest_path=a.output/'manifest.json'
    if not manifest_path.exists():
        (a.output/'scenes').mkdir(exist_ok=True);cases=[]
        # RI<1 supplies the entry-side TIR path without depending on finding a
        # rare exit grazing angle. It is accepted by original caller unchanged.
        defs=[('normal_depth0',1.5,0.,0,0),('normal_depth1',1.5,0.,1,0),
              ('normal_depth3',1.5,0.,3,0),('absorb_depth3',1.5,.8,3,0),
              ('strong_absorb_depth3',1.5,4.,3,0),('tir_depth1',.2,.8,1,0),
              ('tir_depth3',.2,.8,3,0),('cut_absorb_depth3',1.5,.8,3,1)]
        if a.zero_power:
            defs=[('zero_red_absorption0',1.5,0.,3,0),('zero_red_absorption1',1.5,1.,3,0),('zero_rgb_absorption0',1.5,0.,3,0)]
        if a.reflection_controls:
            defs=[('controls_low',0.,0.,0,0),('controls_high',11.,.8,255,0)]
        for name,ri,absorb,depth,cut in defs:
            data=decode(source);data[337]=depth;data[176]=cut
            struct.pack_into('<f',data,116,ri);struct.pack_into('<f',data,370,absorb)
            if a.reflection_controls:
                struct.pack_into('<f',data,120,-1. if name=='controls_low' else 1.)
            if cut:struct.pack_into('<d',data,346,0.)
            zero_channels=[]
            if a.zero_power:
                zero_channels=[0,1,2] if 'zero_rgb' in name else [0]
                # TMandHeader10.Light.LCols (offset692, stride10), and
                # ICols (offset792, stride6): preserve positions/alpha.
                for start,stride,count in [(692,10,10),(792,6,4)]:
                    for index in range(count):
                        for channel in zero_channels:data[start+stride*index+2+channel]=0
            scene=a.output/'scenes'/('transmission_'+name+'.m3p');scene.write_bytes(data)
            cases.append(dict(id=scene.stem,scene=str(scene.relative_to(ROOT)),sha256=sha(scene),
                slots=slots(data),refraction=ri,absorption=absorb,bounces=depth,cut_mask=cut,zero_diffuse_channels=zero_channels))
        write_json(manifest_path,dict(source=str(source.relative_to(ROOT)),source_sha256=sha(source),cases=cases,generator_sha256=sha(Path(__file__))))
    manifest=json.loads(manifest_path.read_bytes());reference=a.pascal_exe is not None
    exe=(a.pascal_exe or a.cpp_exe).resolve();out=a.output/('pascal' if reference else exe.parent.name);out.mkdir(exist_ok=True)
    report=dict(scope=__doc__,binary_sha256=sha(exe),manifest_sha256=sha(manifest_path),thresholds=DEFAULT_THRESHOLDS,results=[],finished=False)
    for case in manifest['cases']:
        scene=ROOT/case['scene'];assert sha(scene)==case['sha256']
        image=out/(case['id']+'.png'); trace=image.with_suffix('.trace')
        cmd=[str(exe)]+(['--render'] if reference else [])+[str(scene),'--output',str(image),'--threads','1','--diag-points','48,36','--parity-trace',str(trace)]
        code,log=run(cmd);image.with_suffix('.log').write_bytes(log)
        row=dict(id=case['id'],command=cmd,returncode=code,status='ERROR')
        if code==0 and image.exists() and trace.exists():
            row.update(status='reference_ready' if reference else 'PASS',image_sha256=sha(image),trace_sha256=sha(trace),witness=witness(trace))
            if reference:row['formula_identity']=validate_formula_load(log.decode('cp1252',errors='replace'),scene,case['slots'])
            else:
                ref=a.output/'pascal'/image.name;a_rgb,b_rgb=load_rgb(ref),load_rgb(image)
                metrics=compute_metrics(a_rgb,b_rgb,DEFAULT_THRESHOLDS['diff_ratio_threshold'])
                original_witness=witness(ref.with_suffix('.trace'))
                branch_equal=[(r['depth'],r.get('tir')) for r in row['witness']['entries']]==[(r['depth'],r.get('tir')) for r in original_witness['entries']]
                row.update(metrics=asdict(metrics),exact_rgb=bool((a_rgb==b_rgb).all()),reference_sha256=sha(ref),original_witness=original_witness,branch_witness_equal=branch_equal,
                           status='PASS' if all(evaluate(metrics,DEFAULT_THRESHOLDS).values()) and branch_equal else 'FAIL')
        report['results'].append(row);report['counts']=dict(Counter(r['status'] for r in report['results']));write_json(out/'summary.json',report)
        print(case['id'],row['status'],'TIR',row.get('witness',{}).get('first_body_tir_count'),flush=True)
    report['finished']=True;write_json(out/'summary.json',report)


if __name__=='__main__':main()
