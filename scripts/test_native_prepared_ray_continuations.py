#!/usr/bin/env python3
"""Compare sixteen real P1 camera rays with frozen inside/primary controls.

The legacy diagnostic starts each lane at camera.origin with tmp=17. This is
not a complete frame, RGB qualification, or proof of cross-sample owner ordering.
"""
import argparse,hashlib,json,shlex,subprocess,sys
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT))
from comparison.tools.run_m3parameter_gpu import invoke
from check_native_inside_trace import compare

def sha(p):return hashlib.sha256(Path(p).read_bytes()).hexdigest()
def main():
 ap=argparse.ArgumentParser(description=__doc__);ap.add_argument('--candidate-build',type=Path,required=True);ap.add_argument('--prepared-inventory',type=Path,required=True);ap.add_argument('--scene-id',required=True);ap.add_argument('--profile',choices=['points','fastgeometry'],default='points');ap.add_argument('--output',type=Path,required=True);ap.add_argument('--schedule-profile',choices=['fixed','bounded'],default='fixed');a=ap.parse_args()
 out=a.output.resolve();out.mkdir(parents=True,exist_ok=False);build=json.loads(a.candidate_build.read_text());inventory=json.loads(a.prepared_inventory.read_text());row=next(x for x in inventory['rows']if x['id']==a.scene_id and x['profile']==a.profile)
 if build['status']!='PASS'or inventory['status']!='PASS':raise ValueError('Passing receipts required')
 if row['map_count']or row['program_count']!=1:raise ValueError('Mapped/combined ray fixture not implemented')
 base=a.candidate_build.resolve().parent;prepared=a.prepared_inventory.resolve().parent/row['id']/row['profile'];library=base/'probe.metallib';reference=Path(build['frozen_control']['path'])
 bound={str(prepared/name):digest for name,digest in row['files'].items()};bound.update({str(library):build['products']['probe.metallib'],str(reference):build['frozen_control']['sha256']})
 frozen={p:h for p,h in build['dependencies'].items()if Path(p).is_relative_to(base)};bound.update(frozen)
 if any(sha(p)!=h for p,h in bound.items()):raise ValueError('Frozen input changed')
 r={'scope':'16 P1 camera rays with the frozen diagnostic initial lane; no full frame/owner-order/qualification claim','source_commit':subprocess.check_output(['git','rev-parse','HEAD'],cwd=ROOT,text=True).strip(),'candidate_build_sha256':sha(a.candidate_build),'prepared_inventory_sha256':sha(a.prepared_inventory),'input':row,'schedule_profile':a.schedule_profile,'frozen':bound,'commands':[],'status':'BUILDING','executions':[]}
 def save():(out/'receipt.json').write_text(json.dumps(r,indent=2)+'\n')
 def run(cmd):
  item={'argv':list(map(str,cmd))};r['commands'].append(item);save();v=subprocess.run(item['argv'],capture_output=True,text=True,timeout=120);item['exit_code']=v.returncode;log=out/f'command-{len(r["commands"])}.log';log.write_text(v.stdout+v.stderr);item['log_sha256']=sha(log);save();v.check_returncode()
 try:
  run(['xcrun','-sdk','macosx','metal','-v','-std=metal3.1','-fno-fast-math','-ffp-contract=fast','-MMD','-MF',out/'camera.d','-I',base/'engine','-I',ROOT/'tests','-c',ROOT/'tests/metal/p2_prepared_ray_inputs.metal','-o',out/'camera.air'])
  run(['xcrun','-sdk','macosx','metallib',out/'camera.air','-o',out/'camera.metallib'])
  run(['xcrun','clang++','-std=c++20','-O2','-arch','arm64','-fobjc-arc','-Wall','-Wextra','-Werror','-I',ROOT,'-I',ROOT/'tests','-MMD','-MF',out/'host.d',ROOT/'tests/test_native_prepared_ray_composition.mm','-framework','Foundation','-framework','Metal','-o',out/'probe'])
  r['dependencies']={}
  for name in ['camera','host']:
   for path in shlex.split((out/(name+'.d')).read_text().replace('\\\n',' ').split(':',1)[1]):r['dependencies'][str(Path(path).resolve())]=sha(path)
  for p in [Path(__file__).resolve(),ROOT/'scripts/check_native_inside_trace.py']:r['dependencies'][str(p)]=sha(p)
  r['products']={name:sha(out/name)for name in ['probe','camera.air','camera.metallib']};r['status']='RUNNING';save()
  for repeat in range(3):
   data=out/f'data/{repeat}';data.mkdir(parents=True);execution=invoke([str(out/'probe'),str(library),str(data),str(reference),str(repeat),str(prepared),str(out/'camera.metallib'),a.schedule_profile],out/f'run-{repeat}',ROOT,timeout=150,exclusive_gpu=True);r['executions'].append(execution);save()
   if execution['exit_code']!=0:r['status']='FAIL';break
   execution['coverage']=compare((data/'observed-trace.bin').read_bytes(),(data/'candidate-trace-first.bin').read_bytes(),16);execution['last_coverage']=compare((data/'observed-trace.bin').read_bytes(),(data/'candidate-trace.bin').read_bytes(),16);save()
  else:
   for p in(out/'data/0').glob('*.bin'):
    if any(p.read_bytes()!=(out/f'data/{i}'/p.name).read_bytes()for i in [1,2]):raise ValueError('Repeat mismatch '+p.name)
   r['cross_repeat_exact']=True;r['status']='PASS'
  if any(sha(p)!=h for p,h in {**bound,**r['dependencies']}.items()):raise ValueError('Bound input changed')
  r['observations']={str(p.relative_to(out)):{'bytes':p.stat().st_size,'sha256':sha(p)}for p in(out/'data').rglob('*.bin')};save();print(r['status']);return int(r['status']!='PASS')
 except Exception as e:r.update(status='ERROR',error=str(e));save();print(str(e),file=sys.stderr);return 2
if __name__=='__main__':raise SystemExit(main())
