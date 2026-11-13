"""Small source-qualified ordinary modes, independent Pascal refs and controls.

Does not run C++ or certify branch coverage from raw header bytes. Changed
packed state and RGB provide intervention evidence; unchanged cases stay
unqualified until a runtime trace proves the target. Existing outputs immutable.
"""
from __future__ import annotations
import argparse
from collections import Counter
import os
from pathlib import Path
import struct
import subprocess
import time

from external_validation import decode, slots, write_json
from prepare_stage_a_modes import ROOT, rel, sha
from pascal_formula_identity import loaded_formulas, validate_formula_load
from image_diff import load_rgb, DEFAULT_THRESHOLDS
from compare_silight5 import read_dump, FIELD_NAMES
from audit_ordinary_source import Audit

OUT=ROOT/'comparison/results/stage-a-20261018/effective-modes-v1'
EXE=ROOT/'comparison/windows_build/Mandelbulb3D_batch_audit.exe'
EXE_SHA='179dec31f161bad54aa9351e5a438a9709672e047573907eb3d5bd6b74408c10'

def generate(out):
    base='comparison/scenes_phase2/phase2_intpow2_small.m3p'
    comb='comparison/scenes_phase3_decomb/phase3_decomb_intpow-bulbox_v0_medium.m3p'
    defs=[]
    def add(name,changes=(),floats=(),source=base,control='control',purpose=''):
        defs.append((name,dict(changes),dict(floats),source,control,purpose))
    add('control',control=None)
    add('normals_depth',[(331,1)],purpose='post-depth normal auto stage')
    add('shadow_hard',[(133,7),(139,0)],purpose='hard shadow light0 auto enabled')
    add('shadow_soft',[(133,7),(139,1|32)],control='shadow_hard',purpose='soft shadow light0 auto enabled')
    add('comb_control',source=comb,control=None)
    for op in (1,2,3,4,5):
        add('comb'+str(op),[(843,op)],[(378,.2)],source=comb,control='comb_control',purpose='DE rule '+str(op))
    for col in (1,2):
        add('comb5_color'+str(col),[(843,5),(422,col)],[(378,.2)],source=comb,control='comb5',purpose='DE mix color '+str(col))
    add('comb_slice',[(344,0),(345,2)],source=comb,control='comb_control',purpose='2D DE combination driver')
    for inside in (1,2,3):
        add('inside'+str(inside),[(842,inside<<1)],purpose='inside/both mode '+str(inside))
    add('reflect',[(336,1),(337,2)],[(332,.6)],purpose='secondary reflect control')
    add('transmit',[(336,3),(337,3)],[(332,.6),(116,1.5),(370,.2)],control='reflect',purpose='transmission all ordinary surfaces')
    add('transmit_difs_rejected',[(336,7),(337,3)],[(332,.6),(116,1.5),(370,.2)],control='reflect',purpose='only-dIFS condition rejects builtin non-dIFS')
    add('transmit_both2',[(336,3),(337,3),(842,4)],[(332,.6),(116,1.5),(370,.2)],control='transmit',purpose='both surface mode2 forces RI1')
    add('transmit_both3',[(336,3),(337,3),(842,6)],[(332,.6),(116,1.5),(370,.2)],control='transmit_both2',purpose='both surface mode3 preserves RI1.5')
    for opt in range(1,6): add('color'+str(opt),[(342,opt)],purpose='color selector '+str(opt))
    add('color_onit1',[(19,1)],purpose='color on input')
    add('color_onit4',[(19,4)],purpose='color orbit rerun at4')
    add('disable_ade',[(842,1)],purpose='force numerical DE')
    add('fixed_de_stop',[(162,0)],purpose='disable view-varying DE stop')
    add('ao_de_dither2',[(149,13),(188,2)],purpose='DE AO quality0 positive dither2')
    add('ao_repeat2',[(149,25),(187,2)],purpose='random AO4 repeat2')
    for mask in (3,5,6,7): add('cut'+str(mask),[(176,mask)],purpose='combined cut-plane mask '+str(mask))
    add('squared',[(439,1)],purpose='squared color arithmetic')
    # Color selector bits reside in TBoptions, a uint32; handled below.
    for gamma in (16,48): add('gamma'+str(gamma),purpose='gamma class')
    add('trap_color',purpose='color from orbit trap')
    add('gradient_clamp',purpose='noncycling gradient')
    add('gradient_nearest',[(620,1)],purpose='nearest gradient interpolation')
    add('total_light2',[(588,1)],purpose='CalcTotalLight2')
    for fog in (1,2): add('fog_composite'+str(fog),[(524,fog)],purpose='fog compositing mode')
    add('fog_iteration4',[(223,4)],purpose='iteration-target dynamic fog')
    cases=[]; audit=Audit()
    for name,changes,floats,source,control,purpose in defs:
        data=decode(ROOT/source)
        struct.pack_into('<ii',data,4,96,72)
        for off in (124,126,133,139,149,163,181,331,336,343):data[off]=0
        data[344]=1
        for off,val in changes.items():data[off]=val
        for off,val in floats.items():struct.pack_into('<f',data,off,val)
        tb=struct.unpack_from('<I',data,476)[0]
        if name.startswith('gamma'):tb=(tb&~(63<<23))|(int(name[5:])<<23)
        if name=='trap_color':tb|=0x20000
        if name=='gradient_clamp':tb&=~0x4000
        struct.pack_into('<I',data,476,tb)
        active=slots(data)
        if any(x['number']>=20 for x in active):raise ValueError('Expected only builtins')
        path=out/'scenes'/('effective_'+name+'.m3p');path.parent.mkdir(parents=True,exist_ok=True);path.write_bytes(data)
        cases.append(dict(id=path.stem,scene=rel(path),sha256=sha(data),target_formulas=[],slots=active,source=source,source_sha256=sha((ROOT/source).read_bytes()),
            control=('effective_'+control) if control else None,purpose=purpose,expected_effective_selectors=audit.decode(data)))
    manifest=dict(schema_version=1,scope='ordinary finite mode interventions; no all-combinations claim',cases=cases)
    write_json(out/'manifest.json',manifest);return manifest

def capture(out,case,exe,trace=False,live=False):
    stem=case['id']+('-trace' if trace else '')+('-live' if live else '')
    target=out/('live' if live else 'traced' if trace else 'pascal')/(stem+'.png');target.parent.mkdir(parents=True,exist_ok=True)
    dump=target.with_suffix('.silight');log=out/(stem+'.log')
    command=[str(exe), '--render',str(ROOT/case['scene']), '--output',str(target),'--threads','1','--silight5-dump',str(dump)]
    if live:command+=['--silight5-dump-live']
    if trace:command+=['--diag-points','48,36','--diag-trace',str(out/(stem+'.trace'))]
    start=time.monotonic()
    try:
        r=subprocess.run(command,cwd=exe.parent,capture_output=True,timeout=60,creationflags=subprocess.CREATE_NO_WINDOW if os.name=='nt' else 0)
        raw=r.stdout+b'\n'+r.stderr;code=r.returncode
    except subprocess.TimeoutExpired as e:raw=(e.stdout or b'')+b'\n'+(e.stderr or b'');code='timeout'
    log.write_bytes(raw)
    row=dict(scene=case['id'],status='reference_error',command=command,returncode=code,pascal_ms=round((time.monotonic()-start)*1000),pascal_log=log.name,pascal_log_sha256=sha(raw))
    if code!=0 or not target.exists() or not dump.exists():return row
    text=raw.decode('cp1252',errors='replace');found=loaded_formulas(text,ROOT/case['scene'])
    row['pascal_formula_load']=validate_formula_load(text,ROOT/case['scene'],case['slots'])
    for slot in case['slots']:
        actual=found.get(slot['slot'])
        if not actual or actual['number']!=slot['number'] or actual['iterations']!=slot['iterations']:
            row['error']='formula loader identity changed';return row
    rgb=load_rgb(target);w,h,records=read_dump(dump)
    if rgb.shape[:2]!=(72,96) or (w,h)!=(96,72):raise ValueError('Reference dimensions')
    row.update(status='reference_ready',pascal_image_sha256=sha(target.read_bytes()),silight_sha256=sha(dump.read_bytes()),nonconstant_rgb=bool((rgb!=rgb[0,0]).any()),hit_pixels=sum(x[4]<32768 for x in records))
    if trace:
        plain=load_rgb(out/'pascal'/(case['id']+'.png'));row['exact_rgb_trace_invariance']=bool((plain==rgb).all())
        p=out/(stem+'.trace');row['trace_sha256']=sha(p.read_bytes()) if p.exists() else None
    return row

def render(out,manifest,exe):
    if sha(exe.read_bytes())!=EXE_SHA:raise ValueError('Unqualified original executable')
    report=dict(schema_version=1,scope='independent Pascal reference; no C++ acceptance',finished=False,pascal_sha256=EXE_SHA,manifest_sha256=sha((out/'manifest.json').read_bytes()),thresholds=DEFAULT_THRESHOLDS,results=[],trace_controls=[],influence=[])
    for case in manifest['cases']:
        if sha((ROOT/case['scene']).read_bytes())!=case['sha256']:raise ValueError('Scene changed')
        row=capture(out,case,exe);report['results'].append(row)
        control=case['control'];cp=out/'pascal'/((control or '')+'.png')
        if row['status']=='reference_ready' and control and cp.exists():
            rgb=load_rgb(out/'pascal'/(case['id']+'.png'));crgb=load_rgb(cp)
            _,_,rec=read_dump(out/'pascal'/(case['id']+'.silight'));_,_,crec=read_dump(cp.with_suffix('.silight'))
            changes={name:sum(a[i]!=b[i] for a,b in zip(rec,crec)) for i,name in enumerate(FIELD_NAMES)}
            report['influence'].append(dict(scene=case['id'],control=control,changed_rgb_pixels=int((rgb!=crgb).any(axis=2).sum()),changed_silight_fields=changes,
                qualification='intervention changes output/state; target branch attribution requires source gate plus trace' if any(changes.values()) or (rgb!=crgb).any() else 'no observed influence; target remains unqualified'))
        report['counts']=dict(Counter(x['status'] for x in report['results']));write_json(out/'summary.json',report)
        print(case['id'],row['status'],row.get('hit_pixels'),flush=True)
    for case in manifest['cases']:
        if case['id'] in {'effective_control','effective_normals_depth','effective_shadow_hard','effective_shadow_soft','effective_comb1','effective_comb2','effective_comb4','effective_comb5','effective_inside1','effective_inside2','effective_inside3','effective_transmit','effective_transmit_both2','effective_transmit_both3'}:
            row=capture(out,case,exe,True);report['trace_controls'].append(row);write_json(out/'summary.json',report)
            print(case['id'],'trace',row.get('exact_rgb_trace_invariance'),flush=True)
    report['finished']=True;report['qualification']='finite original-reference images/state; see explicit per-case influence and trace qualification'
    write_json(out/'summary.json',report)
    ready=[c['id'] for c,r in zip(manifest['cases'],report['results']) if r['status']=='reference_ready']
    write_json(out/'ready-cases.json',dict(cases=ready))
    if sha(exe.read_bytes())!=EXE_SHA:raise ValueError('Reference changed')

def main():
    ap=argparse.ArgumentParser(description=__doc__);ap.add_argument('--output',type=Path,default=OUT);ap.add_argument('--render-reference',action='store_true');ap.add_argument('--pascal-exe',type=Path,default=EXE);args=ap.parse_args()
    if (args.output/'summary.json').exists():raise ValueError('Use a fresh reference directory')
    m=generate(args.output);print('generated',len(m['cases']),flush=True)
    if args.render_reference:render(args.output,m,args.pascal_exe.resolve())

if __name__=='__main__':main()
