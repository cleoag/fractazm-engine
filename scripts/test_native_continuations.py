#!/usr/bin/env python3
"""Build an isolated, source-bound P2 raw-body proof and run under GPU guard.

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

ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT))
from comparison.tools.run_m3parameter_gpu import invoke


def sha(p):return hashlib.sha256(Path(p).read_bytes()).hexdigest()

def main():
    ap=argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--output',type=Path,required=True)
    ap.add_argument('--baseline-library',type=Path,help='frozen original kernel; never replace reference outputs')
    ap.add_argument('--cohort',choices=['core','gnarly'],default='core')
    ap.add_argument('--cases',type=int,default=1024)
    ap.add_argument('--contract',choices=['fast','off'],default='fast',help='off is a diagnostic control, never P1/P2 acceptance')
    args=ap.parse_args();out=args.output.resolve()
    if out.exists():ap.error('Output must be new; preserve frozen attempts')
    if shutil.disk_usage(ROOT).free<12*1024**3:ap.error('12 GiB reserve unavailable')
    out.mkdir(parents=True);(out/'data').mkdir()
    receipt={'scope':'raw-body continuation experiment, not renderer qualification','source_commit':subprocess.check_output(['git','rev-parse','HEAD'],cwd=ROOT,text=True).strip(),'commands':[],'dependencies':{},'products':{},'status':'BUILDING','pipeline_cache':'no explicit archive; driver-cache state unknown','contraction':args.contract,'platform':platform.platform(),'cases':args.cases}
    def capture():
        (out/'receipt.json').write_text(json.dumps(receipt,indent=2)+'\n')
    def run(cmd):
        record={'argv':list(map(str,cmd))};receipt['commands'].append(record);capture()
        result=subprocess.run(record['argv'],cwd=ROOT,capture_output=True,text=True,timeout=120)
        index=len(receipt['commands']);(out/f'command-{index}.log').write_text(result.stdout+result.stderr);record['returncode']=result.returncode;capture()
        if result.returncode:raise RuntimeError(f'Command {index} failed: {result.stderr[-2000:]}')
    try:
        if args.cohort=='gnarly' and not args.baseline_library:
            run(['xcrun','-sdk','macosx','metal','-v','-std=metal3.1','-fno-fast-math','-ffp-contract='+args.contract,'-MMD','-MF',out/'reference.d','-I',ROOT,'-c',ROOT/'tests/metal/p2_gnarly_reference.metal','-o',out/'reference.air'])
            run(['xcrun','-sdk','macosx','metallib',out/'reference.air','-o',out/'reference.metallib'])
            args.baseline_library=out/'reference.metallib'
        receipt['cohort']=args.cohort
        if args.baseline_library:
            baseline=args.baseline_library.resolve();receipt['frozen_control']={'path':str(baseline),'sha256':sha(baseline)}
        run([sys.executable,ROOT/'scripts/gen_native_continuations.py','--output',out/'generated'])
        shader=ROOT/'tests/metal/p2_formula_continuation.metal';host=ROOT/'tests/test_native_continuations.mm'
        run(['xcrun','-sdk','macosx','metal','-v','-std=metal3.1','-fno-fast-math','-ffp-contract='+args.contract,'-MMD','-MF',out/'probe.d','-I',ROOT,'-I',out/'generated','-c',shader,'-o',out/'probe.air'])
        run(['xcrun','-sdk','macosx','metallib',out/'probe.air','-o',out/'probe.metallib'])
        run(['xcrun','clang++','-v','-std=c++20','-O2','-fobjc-arc','-Wall','-Wextra','-Werror','-UNDEBUG','-MMD','-MF',out/'host.d','-I',ROOT/'src',host,'-framework','Foundation','-framework','Metal','-o',out/'probe'])
        for dep in [out/'probe.d',out/'host.d']+([out/'reference.d'] if (out/'reference.d').exists() else []):
            content=dep.read_text().replace('\\\n',' ');paths=shlex.split(content.split(': ',1)[1])
            for path in paths:
                p=Path(path);p=p if p.is_absolute() else ROOT/p
                receipt['dependencies'][str(p)]=sha(p)
        for path in [ROOT/'scripts/gen_native_continuations.py',ROOT/'scripts/gen_native_metal_formulas.py',Path(__file__)]:receipt['dependencies'][str(path.resolve())]=sha(path)
        for name in receipt['dependencies']:
            source=Path(name)
            if source.is_relative_to(ROOT):
                dest=out/'source'/source.relative_to(ROOT);dest.parent.mkdir(parents=True,exist_ok=True);shutil.copy2(source,dest)
        receipt['products']={p.name:sha(p) for p in [out/'probe',out/'probe.air',out/'probe.metallib',out/'generated/manifest.json']};receipt['status']='BUILT';capture()
        argv=[str(out/'probe'),str(out/'probe.metallib'),str(out/'data')]
        if args.baseline_library:argv.extend([str(args.baseline_library.resolve()),str(args.cases),args.cohort])
        elif args.cases!=1024:raise ValueError('Expanded cohort requires frozen control library')
        event=invoke(argv,out/'run',ROOT,timeout=150,exclusive_gpu=True)
        receipt['execution']=event;receipt['status']='EXECUTED';capture()
        # Every binding must still agree after execution; never use timestamps.
        for name,digest in receipt['dependencies'].items():
            if sha(name)!=digest:raise RuntimeError(f'Input changed during proof: {name}')
        if args.baseline_library and sha(baseline)!=receipt['frozen_control']['sha256']:raise RuntimeError('Frozen control changed during proof')
        receipt['observations']={p.name:{'bytes':p.stat().st_size,'sha256':sha(p)} for p in sorted((out/'data').glob('*.bin'))};capture()
        rc=event.get('returncode',event.get('exit_code',None))
        print(json.dumps({'output':str(out),'status':receipt['status'],'execution':event},indent=2))
        return 0 if rc==0 else 1
    except Exception as e:
        receipt['status']='FAILED';receipt['error']=str(e);capture();print(str(e),file=sys.stderr);return 2

if __name__=='__main__':raise SystemExit(main())
