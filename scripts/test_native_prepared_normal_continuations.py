#!/usr/bin/env python3
"""Run bounded ordered-normal proofs using a frozen P1-prepared program.

This is a point fixture, not full-scene or formula qualification. Reuse explicitly
selected, hash-checked normal-control/candidate libraries and compile a new host.
"""
import argparse,hashlib,json,shlex,subprocess,sys
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT))
from comparison.tools.run_m3parameter_gpu import invoke
from check_native_normal_composition_trace import validate_directory

def sha(path):return hashlib.sha256(Path(path).read_bytes()).hexdigest()
def main():
 ap=argparse.ArgumentParser(description=__doc__);ap.add_argument('--candidate-build',type=Path,required=True);ap.add_argument('--prepared-inventory',type=Path,required=True);ap.add_argument('--scene-id',required=True);ap.add_argument('--profile',choices=['points','fastgeometry'],default='points');ap.add_argument('--output',type=Path,required=True);a=ap.parse_args()
 out=a.output.resolve();out.mkdir(parents=True,exist_ok=False)
 build=json.loads(a.candidate_build.read_text());inventory=json.loads(a.prepared_inventory.read_text());row=next(x for x in inventory['rows']if x['id']==a.scene_id and x['profile']==a.profile)
 if build['status']!='PASS'or inventory['status']!='PASS':raise ValueError('Explicit passing artifact receipts required')
 if row['map_count']or row['program_count']!=1:raise ValueError('Mapped/combined fixture not implemented')
 prepared=a.prepared_inventory.resolve().parent/row['id']/row['profile'];library=a.candidate_build.resolve().parent/'probe.metallib';reference=Path(build['frozen_control']['path'])
 bound={str(prepared/name):digest for name,digest in row['files'].items()};bound.update({str(library):build['products']['probe.metallib'],str(reference):build['frozen_control']['sha256']})
 if any(sha(p)!=h for p,h in bound.items()):raise ValueError('Frozen input changed')
 r={'scope':'prepared-program bounded normal points; no renderer qualification','source_commit':subprocess.check_output(['git','rev-parse','HEAD'],cwd=ROOT,text=True).strip(),'candidate_build_sha256':sha(a.candidate_build),'prepared_inventory_sha256':sha(a.prepared_inventory),'input':row,'frozen':bound,'status':'BUILDING','executions':[]}
 def save():(out/'receipt.json').write_text(json.dumps(r,indent=2)+'\n')
 source=ROOT/'tests/test_native_prepared_normal_composition.mm';command=['xcrun','clang++','-std=c++20','-O2','-arch','arm64','-fobjc-arc','-fno-fast-math','-ffp-contract=off','-Wall','-Wextra','-Werror','-I'+str(ROOT),'-MMD','-MF',str(out/'probe.d'),str(source),'-framework','Foundation','-framework','Metal','-o',str(out/'probe')];r['command']=command;save()
 try:
  v=subprocess.run(command,capture_output=True,text=True,timeout=120);(out/'compile.log').write_text(v.stdout+v.stderr);r['compile_exit']=v.returncode;save();v.check_returncode()
  deps=shlex.split((out/'probe.d').read_text().replace('\\\n',' ').split(':',1)[1]);r['dependencies']={str(Path(x).resolve()):sha(x)for x in deps}
  for p in [Path(__file__).resolve(),ROOT/'scripts/check_native_normal_composition_trace.py']:r['dependencies'][str(p)]=sha(p)
  r['host_sha256']=sha(out/'probe');r['status']='RUNNING';save()
  for repeat in range(3):
   data=out/f'data/{repeat}';data.mkdir(parents=True);execution=invoke([str(out/'probe'),str(library),str(data),str(reference),str(prepared),str(repeat)],out/f'run-{repeat}',ROOT,timeout=150,exclusive_gpu=True);r['executions'].append(execution);save()
   if execution['exit_code']!=0:r['status']='FAIL';break
   execution['coverage']=validate_directory(data,cases=16);save()
  else:r['status']='PASS'
  for path,digest in {**bound,**r['dependencies']}.items():
   if sha(path)!=digest:raise ValueError('Bound input changed '+path)
  r['observations']={str(p.relative_to(out)):{'bytes':p.stat().st_size,'sha256':sha(p)}for p in (out/'data').rglob('*.bin')};save();print(r['status']);return int(r['status']!='PASS')
 except Exception as e:r.update(status='ERROR',error=str(e));save();print(str(e),file=sys.stderr);return 2
if __name__=='__main__':raise SystemExit(main())
