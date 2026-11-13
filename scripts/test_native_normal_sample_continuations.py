#!/usr/bin/env python3
"""Build an isolated, source-bound P2 normal sample proof and run under GPU guard.

Does not rebuild selected engines or qualify formulas/scenes. Failures retain
commands, outputs and the exact experimental artifacts for first-divergence work.
"""
from __future__ import annotations
import argparse
import hashlib
import json
import platform
from pathlib import Path
import shlex
import shutil
import subprocess
import sys
import zlib

ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT))
from comparison.tools.run_m3parameter_gpu import invoke
from check_native_normal_sample_trace import validate_directory


def sha(p):return hashlib.sha256(Path(p).read_bytes()).hexdigest()

def main():
    ap=argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--output',type=Path,required=True)
    ap.add_argument('--baseline-library',type=Path,help='frozen original kernel; never replace reference outputs')
    ap.add_argument('--cohort',choices=['core','gnarly'],default='core')
    ap.add_argument('--combination',type=int,default=1)
    ap.add_argument('--second-driver',type=int,default=1)
    ap.add_argument('--driver',type=int,default=0)
    ap.add_argument('--observer',choices=['default','direct','captured'],default='default')
    ap.add_argument('--cases',type=int,default=1024)
    ap.add_argument('--smoothing-policy',type=int,choices=[0,1],default=0)

    args=ap.parse_args();args.contract='fast';out=args.output.resolve()
    if out.exists():ap.error('Output must be new; preserve frozen attempts')
    if shutil.disk_usage(ROOT).free<12*1024**3:ap.error('12 GiB reserve unavailable')
    out.mkdir(parents=True);(out/'data').mkdir()
    layer_flags=[]
    receipt={'layer':'normal_sample','scope':'normal sample continuation experiment, not renderer qualification','source_commit':subprocess.check_output(['git','rev-parse','HEAD'],cwd=ROOT,text=True).strip(),'commands':[],'dependencies':{},'products':{},'status':'BUILDING','pipeline_cache':'no explicit archive; driver-cache state unknown','contraction':args.contract,'platform':platform.platform(),'cases':args.cases}
    def capture():
        (out/'receipt.json').write_text(json.dumps(receipt,indent=2)+'\n')
    def run(cmd):
        record={'argv':list(map(str,cmd))};receipt['commands'].append(record);capture()
        result=subprocess.run(record['argv'],cwd=ROOT,capture_output=True,text=True,timeout=120)
        index=len(receipt['commands']);(out/f'command-{index}.log').write_text(result.stdout+result.stderr);record['returncode']=result.returncode;capture()
        if result.returncode:raise RuntimeError(f'Command {index} failed: {result.stderr[-2000:]}')
    try:
        # Recreate the audited selected P1 closure directly from committed blobs.
        audit=ROOT/'comparison/results/p2-state-audit-20261103/capabilities.json'
        closure=json.loads(audit.read_text())['selected_dependency_closure'];stage=out/'engine'
        p1_record=ROOT/'comparison/results/p1-prepared-scene-20261103/build-record.json'
        for path,digest in json.loads(p1_record.read_text())['dependencies'].items():
            marker='/comparison/metal_exact/'
            if marker in path:
                name='comparison/metal_exact/'+path.split(marker,1)[1]
                for revision in ['fc00c78e02b1aba530e611a07f2bf72cc5dbd099','11d13296']:
                    blob=subprocess.check_output(['git','cat-file','blob',revision+':'+name],cwd=ROOT)
                    if hashlib.sha256(blob).hexdigest()==digest:
                        closure[name]={'sha256':digest,'revision':revision};break
                else:raise ValueError('P1 shared header not found in pinned revisions: '+name)
        receipt['selected_source_closure']=closure
        for name,item in closure.items():
            data=subprocess.check_output(['git','cat-file','blob',item['revision']+':'+name],cwd=ROOT)
            if hashlib.sha256(data).hexdigest()!=item['sha256']:raise ValueError('Selected source mismatch: '+name)
            target=stage/name;target.parent.mkdir(parents=True,exist_ok=True);target.write_bytes(data)
        overlays=['continuation_abi.h','continuation_frames.h','continuation_step.metal.h','continuation_orbit_step.metal.h','continuation_replay_step.metal.h','continuation_distance_step.metal.h','continuation_combined_step.metal.h','continuation_entry_step.metal.h','continuation_normal_sample_step.metal.h']
        for name in overlays:shutil.copy2(ROOT/'src/render/native_metal'/name,stage/'src/render/native_metal'/name)
        run([sys.executable,ROOT/'scripts/gen_native_caller_continuations.py','--source-root',stage,'--output',out/'generated'])
        run([sys.executable,ROOT/'scripts/gen_native_entry_continuations.py','--source-root',stage,'--output',out/'generated'])
        run([sys.executable,ROOT/'scripts/gen_native_normal_sample_continuations.py','--source-root',stage,'--output',out/'generated'])
        if not args.baseline_library:
            run(['xcrun','-sdk','macosx','metal',*layer_flags,'-v','-std=metal3.1','-fno-fast-math','-ffp-contract=fast','-DP2_REFERENCE=1','-MMD','-MF',out/'reference.d','-I',stage,'-I',out/'generated','-c',ROOT/'tests/metal/p2_normal_sample_continuation.metal','-o',out/'reference.air'])
            run(['xcrun','-sdk','macosx','metallib',out/'reference.air','-o',out/'reference.metallib'])
            args.baseline_library=out/'reference.metallib'
        receipt['cohort']=args.cohort
        if args.baseline_library:
            baseline=args.baseline_library.resolve();receipt['frozen_control']={'path':str(baseline),'sha256':sha(baseline)}
        run([sys.executable,ROOT/'scripts/gen_native_continuations.py','--output',out/'generated'])
        shader=ROOT/'tests/metal/p2_normal_sample_continuation.metal';host=ROOT/'tests/test_native_normal_sample_continuations.mm'
        run(['xcrun','-sdk','macosx','metal',*layer_flags,'-v','-std=metal3.1','-fno-fast-math','-ffp-contract='+args.contract,'-MMD','-MF',out/'probe.d','-I',stage,'-I',out/'generated','-c',shader,'-o',out/'probe.air'])
        run(['xcrun','-sdk','macosx','metallib',out/'probe.air','-o',out/'probe.metallib'])
        run(['xcrun','clang++',*layer_flags,'-v','-std=c++20','-O2','-fobjc-arc','-Wall','-Wextra','-Werror','-UNDEBUG','-MMD','-MF',out/'host.d','-I',stage/'src','-I',stage,host,'-lz','-framework','Foundation','-framework','Metal','-o',out/'probe'])
        for dep in [out/'probe.d',out/'host.d']+([out/'reference.d'] if (out/'reference.d').exists() else []):
            content=dep.read_text().replace('\\\n',' ');paths=shlex.split(content.split(': ',1)[1])
            for path in paths:
                p=Path(path);p=p if p.is_absolute() else ROOT/p
                receipt['dependencies'][str(p)]=sha(p)
        for path in [ROOT/'scripts/gen_native_continuations.py',ROOT/'scripts/gen_native_metal_formulas.py',ROOT/'scripts/gen_native_caller_continuations.py',ROOT/'scripts/gen_native_orbit_continuations.py',ROOT/'scripts/gen_native_replay_continuations.py',ROOT/'scripts/gen_native_distance_continuations.py',ROOT/'scripts/gen_native_combined_continuations.py',ROOT/'scripts/check_native_normal_sample_trace.py',ROOT/'scripts/gen_native_entry_continuations.py',ROOT/'scripts/gen_native_normal_sample_continuations.py',audit,p1_record,Path(__file__)]:receipt['dependencies'][str(path.resolve())]=sha(path)
        for name in receipt['dependencies']:
            source=Path(name)
            if source.is_relative_to(ROOT):
                dest=out/'source'/source.relative_to(ROOT);dest.parent.mkdir(parents=True,exist_ok=True);shutil.copy2(source,dest)
        for name in overlays:receipt['dependencies'][str(ROOT/'src/render/native_metal'/name)]=sha(ROOT/'src/render/native_metal'/name)
        if (out/'generated/continuation_frames.h').read_bytes()!=(stage/'src/render/native_metal/continuation_frames.h').read_bytes():raise ValueError('Stale shared frames')
        receipt['products']={p.name:sha(p) for p in [out/'probe',out/'probe.air',out/'probe.metallib',out/'generated/manifest.json']};receipt['status']='BUILT';capture()
        argv=[str(out/'probe'),str(out/'probe.metallib'),str(out/'data')]
        if args.baseline_library:argv.extend([str(args.baseline_library.resolve()),str(args.cases),args.cohort,str(args.driver),str(args.combination),str(args.second_driver)])
        elif args.cases!=1024:raise ValueError('Expanded cohort requires frozen control library')
        argv.append(args.observer)
        argv.append(str(args.smoothing_policy))
        receipt['smoothing_policy']=args.smoothing_policy
        receipt['observer_mode']=args.observer
        event=invoke(argv,out/'run',ROOT,timeout=150,exclusive_gpu=True)
        receipt['execution']=event;receipt['status']='EXECUTED';capture()
        # Every binding must still agree after execution; never use timestamps.
        for name,digest in receipt['dependencies'].items():
            if sha(name)!=digest:raise RuntimeError(f'Input changed during proof: {name}')
        if args.baseline_library and sha(baseline)!=receipt['frozen_control']['sha256']:raise RuntimeError('Frozen control changed during proof')
        receipt['observations']={p.name:{'bytes':p.stat().st_size,'sha256':sha(p)} for p in sorted((out/'data').glob('*.zlib'))};capture()
        for name,item in receipt['observations'].items():
            decoded=zlib.decompress((out/'data'/name).read_bytes());item['encoding']='zlib';item['decoded_bytes']=len(decoded);item['decoded_sha256']=hashlib.sha256(decoded).hexdigest()
        capture()
        rc=event.get('returncode',event.get('exit_code',None))
        if rc==0:
            receipt['coverage']=validate_directory(out/'data',args.cases);capture()
        print(json.dumps({'output':str(out),'status':receipt['status'],'execution':event},indent=2))
        return 0 if rc==0 else 1
    except Exception as e:
        receipt['status']='FAILED';receipt['error']=str(e);capture();print(str(e),file=sys.stderr);return 2

if __name__=='__main__':raise SystemExit(main())
