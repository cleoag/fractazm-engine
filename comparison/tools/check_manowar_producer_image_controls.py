"""Bounded visible-geometry witnesses for both orders of the SSE producer.

Creates fresh scenes/evidence only. Existing sealed qualification is untouched.
At most twelve final-C++ previews, followed by sequential Pascal and pre-fix C++
for the first meaningful witness of each order. Image thresholds are unchanged.
"""
import argparse
from dataclasses import asdict
import hashlib
import json
import os
from pathlib import Path
import struct
import subprocess
import time
import numpy as np
from external_validation import slots
from compatibility_precision import bind_compatible, precision_evidence
from image_diff import DEFAULT_THRESHOLDS, load_rgb, compute_metrics, evaluate
from pascal_formula_identity import loaded_formulas

ROOT = Path(__file__).resolve().parents[2]

def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()

def main():
    ap=argparse.ArgumentParser(description=__doc__)
    for name in ['cpp','before','pascal','output']:
        ap.add_argument('--'+name,type=Path,required=True)
    ap.add_argument('--source-directed',action='store_true',help='One source-directed bailout/feedback pair per order, at most two previews')
    a=ap.parse_args()
    a.output=a.output.resolve();a.output.mkdir(parents=True,exist_ok=False)
    env={k:v for k,v in os.environ.items() if not k.upper().startswith(
        ('MB3D_POINT_','MB3D_PARITY_','MB3D_HEADLESS_','MB3D_FORMULA_BUFFER_DUMP'))}
    report={'scope':__doc__,'finished':False,'thresholds':DEFAULT_THRESHOLDS,
            'threads':4,'timeout_seconds':90,'max_initial_cpp_previews':12,
            'identities':{str(p.resolve()):sha(p)for p in [a.cpp,a.before,a.pascal,Path(__file__)]},
            'previews':[],'results':[]}
    def save():
        (a.output/'summary.json').write_text(json.dumps(report,indent=2))
    def run(exe,scene,directory,tag,pascal=False,dump=False):
        image=directory/(tag+'.png');command=[str(exe.resolve())]
        if pascal:command+=['--render']
        command += [str(scene),'--output',str(image),'--threads','4']
        if not pascal:command+=['--deterministic']
        if not pascal:bind_compatible(command,image)
        if dump:command+=['--silight5-dump',str(directory/(tag+'.silight'))]
        cwd=exe.resolve().parent if pascal else ROOT
        start=time.monotonic()
        p=subprocess.run(command,cwd=cwd,env=env,capture_output=True,timeout=90,
                         creationflags=subprocess.CREATE_NO_WINDOW)
        (directory/(tag+'.stdout')).write_bytes(p.stdout)
        (directory/(tag+'.stderr')).write_bytes(p.stderr)
        cmd={'argv':command,'cwd':str(cwd),'exit_code':p.returncode,
             'seconds':time.monotonic()-start,'exe_sha256':sha(exe)}
        (directory/(tag+'.command.json')).write_text(json.dumps(cmd,indent=2))
        p.check_returncode()
        if not pascal:cmd.update(precision_evidence(image))
        (directory/(tag+'.command.json')).write_text(json.dumps(cmd,indent=2))
        return load_rgb(image),p,cmd
    # The old alternating camera is retained. Smaller previous-state feedback
    # keeps the RStop term in view; a single modest rotation avoids unnecessary
    # high-power C oscillation. Later candidates are used only if needed.
    choices=[(2.,-.025,False),(2.,-.075,False),(3.,-.025,False),
             (2.,-.025,True),(4.,-.025,False),(2.,-.01,False)]
    if a.source_directed:
        choices=[(2.,-.005,False)]
        report['source_directed_bailout']=1.1
        report['max_initial_cpp_previews']=2
        report['rationale']='Numerical-gradient callers use saved bailout^4*64; lower bailout keeps explicit Manowar feedback in the visible scene range.'
    selected=[]
    for order in ['ipm','mip']:
        source=ROOT/'comparison/runs/reference-gaps-20261018/manowar/sse-producer-scenes'/('alt_'+order+'.m3p')
        for index,(power,feedback,julia) in enumerate(choices):
            data=bytearray(source.read_bytes());producer=0 if order=='ipm'else 1;manowar=1-producer
            for slot,options in [(producer,{1:2,2:1,3:0,6:0}),
                                 (manowar,{0:power,2:feedback,3:.5})]:
                for k,v in options.items():struct.pack_into('<d',data,848+188*slot+60+8*k,float(v))
            if a.source_directed:struct.pack_into('<d',data,92,1.1)
            data[190]=int(julia)
            if julia:struct.pack_into('<3d',data,191,.2,-.3,.4)
            ident='visible_'+order+'_'+str(index)
            directory=a.output/ident;directory.mkdir();scene=directory/(ident+'.m3p');scene.write_bytes(data)
            rgb,p,command=run(a.cpp,scene,directory,'cpp',dump=True)
            payload=(directory/'cpp.silight').read_bytes()
            magic,version,w,h,size,count=struct.unpack_from('<8s5I',payload)
            if magic!=b'SIL5DMP1' or version!=1 or size!=18 or count!=w*h or len(payload)!=28+count*18:
                raise ValueError('invalid current geometry dump')
            records=np.frombuffer(payload,dtype='<u2',offset=28).reshape(-1,9)
            depths=records[:,4];normal_count=len(np.unique(records[:,:3],axis=0))
            nonconstant=bool(np.any(rgb!=rgb[0,0]));depth_count=len(np.unique(depths))
            meaningful=nonconstant and depth_count>8 and normal_count>8
            case={'id':ident,'order':order,'scene':str(scene.relative_to(ROOT)),
                  'scene_sha256':sha(scene),'source_sha256':sha(source),'slots':slots(data),
                  'power':power,'feedback':feedback,'julia':julia,'command':command,
                  'spatially_nonconstant':nonconstant,'unique_depths':depth_count,
                  'unique_normals':normal_count,'meaningful_geometry':meaningful,
                  'cpp_image_sha256':sha(directory/'cpp.png'),'cpp_silight_sha256':sha(directory/'cpp.silight')}
            report['previews'].append(case);save();print(ident,meaningful,depth_count,normal_count,flush=True)
            if meaningful:
                selected.append((case,scene,directory,rgb));break
    if not selected:
        report['finished']=True
        report['status']='NO_MEANINGFUL_WITNESS'
        report['counts']={'cpp_previews':len(report['previews']),'qualified':0}
        report['artifact_sha256']={str(p.relative_to(a.output)):sha(p)for p in a.output.rglob('*')if p.is_file()and p.name!='summary.json'}
        save()
        return 2
    for case,scene,directory,cpp in selected:
        pascal,p,pc=run(a.pascal,scene,directory,'pascal',pascal=True)
        actual=loaded_formulas(p.stdout.decode(errors='replace'),scene)
        for slot in case['slots']:
            i=slot['slot'];name=slot['name'];wanted={'number':20,'iterations':1,'name':name}
            if actual.get(i)!=wanted:raise ValueError('Pascal loaded the wrong formula')
            asset=a.pascal.resolve().parent/'M3Formulas'/(name+'.m3f');original=ROOT/'mb3d/M3Formulas'/(name+'.m3f')
            if sha(asset)!=sha(original):raise ValueError('changed Pascal formula asset')
            report['identities'][str(asset)]=sha(asset);report['identities'][str(original)]=sha(original)
        before,_,bc=run(a.before,scene,directory,'before')
        metrics=compute_metrics(pascal,cpp,int(DEFAULT_THRESHOLDS['diff_ratio_threshold']))
        oldmetrics=compute_metrics(pascal,before,int(DEFAULT_THRESHOLDS['diff_ratio_threshold']))
        checks=evaluate(metrics,DEFAULT_THRESHOLDS);oldchecks=evaluate(oldmetrics,DEFAULT_THRESHOLDS)
        row={**case,'pascal_command':pc,'before_command':bc,'loaded_slots':actual,
             'pascal_image_sha256':sha(directory/'pascal.png'),'before_image_sha256':sha(directory/'before.png'),
             'metrics':asdict(metrics),'checks':checks,'status':'PASS'if all(checks.values())else'FAIL',
             'exact_rgb':metrics.max_abs_delta==0,'before_metrics':asdict(oldmetrics),
             'before_checks':oldchecks,'before_status':'PASS'if all(oldchecks.values())else'FAIL',
             'reference_spatially_nonconstant':bool(np.any(pascal!=pascal[0,0]))}
        report['results'].append(row);save();print(row['id'],row['status'],'before',row['before_status'],flush=True)
    (a.output/'manifest.json').write_text(json.dumps({'cases':[r for r,_,_,_ in selected]},indent=2))
    report['finished']=True
    report['counts']={'cpp_previews':len(report['previews']),'qualified':len(report['results']),
                      'passed':sum(r['status']=='PASS'for r in report['results']),
                      'exact':sum(r['exact_rgb']for r in report['results']),
                      'discriminating_before_failures':sum(r['before_status']=='FAIL'for r in report['results'])}
    report['artifact_sha256']={str(p.relative_to(a.output)):sha(p)for p in a.output.rglob('*')if p.is_file()and p.name!='summary.json'}
    save()
    return int(not all(r['status']=='PASS' and r['reference_spatially_nonconstant']for r in report['results']))

if __name__=='__main__':
    raise SystemExit(main())
