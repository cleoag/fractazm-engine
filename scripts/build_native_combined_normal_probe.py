#!/usr/bin/env python3
"""Build a two-program normal fixture from an explicit frozen caller stage.

Build completion is not numerical acceptance. The legacy single-program control
is retained for independent ABI-layout controls; combined inputs need their own
same-source original reference and later full-P1 image/work acceptance.
"""
import argparse,hashlib,json,shlex,subprocess
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
def sha(p):return hashlib.sha256(Path(p).read_bytes()).hexdigest()
def main():
 ap=argparse.ArgumentParser(description=__doc__);ap.add_argument('--source-build',type=Path,required=True);ap.add_argument('--output',type=Path,required=True);a=ap.parse_args()
 prior=json.loads(a.source_build.read_text());base=a.source_build.resolve().parent;out=a.output.resolve();out.mkdir(parents=True,exist_ok=False)
 if prior['status']!='PASS':raise ValueError('Passing frozen caller build required')
 # Every input under the frozen source stage must still match its build record.
 frozen={p:h for p,h in prior['dependencies'].items()if Path(p).is_relative_to(base)}
 if any(sha(p)!=h for p,h in frozen.items()):raise ValueError('Frozen caller source changed')
 r={'scope':'two-program normal diagnostic build; numerical acceptance separate','source_commit':subprocess.check_output(['git','rev-parse','HEAD'],cwd=ROOT,text=True).strip(),'source_build_sha256':sha(a.source_build),'frozen_stage':frozen,'legacy_control':prior['frozen_control'],'commands':[],'status':'BUILDING'}
 def save():(out/'receipt.json').write_text(json.dumps(r,indent=2)+'\n')
 def run(cmd):
  row={'argv':list(map(str,cmd))};r['commands'].append(row);save();v=subprocess.run(row['argv'],capture_output=True,text=True,timeout=120);row['exit_code']=v.returncode;log=out/f'command-{len(r["commands"])}.log';log.write_text(v.stdout+v.stderr);row['log_sha256']=sha(log);save();v.check_returncode()
 try:
  shader=ROOT/'tests/metal/p2_combined_normal_composition.metal';host=ROOT/'tests/test_native_combined_normal_composition.mm';stage=base/'engine'
  for name,extra in [('reference',['-DP2_REFERENCE=1']),('probe',['-DP2_OUTLINE_OBSERVER=1'])]:
   run(['xcrun','-sdk','macosx','metal','-v','-std=metal3.1','-fno-fast-math','-ffp-contract=fast',*extra,'-MMD','-MF',out/(name+'.d'),'-I',stage,'-I',base,'-c',shader,'-o',out/(name+'.air')]);run(['xcrun','-sdk','macosx','metallib',out/(name+'.air'),'-o',out/(name+'.metallib')])
  run(['xcrun','clang++','-std=c++20','-O2','-arch','arm64','-fobjc-arc','-fno-fast-math','-ffp-contract=off','-Wall','-Wextra','-Werror','-I',ROOT,'-MMD','-MF',out/'host.d',host,'-framework','Foundation','-framework','Metal','-o',out/'probe'])
  r['dependencies']={}
  for name in ['reference','probe','host']:
   for path in shlex.split((out/(name+'.d')).read_text().replace('\\\n',' ').split(':',1)[1]):r['dependencies'][str(Path(path).resolve())]=sha(path)
  r['dependencies'][str(Path(__file__).resolve())]=sha(__file__)
  r['products']={name:sha(out/name)for name in ['probe','probe.air','probe.metallib','reference.air','reference.metallib']};r['frozen_control']={'path':str(out/'reference.metallib'),'sha256':sha(out/'reference.metallib')}
  if any(sha(p)!=h for p,h in frozen.items()):raise ValueError('Frozen source changed during build')
  r['status']='BUILT';save();print(r['status'])
 except Exception as e:r.update(status='ERROR',error=str(e));save();raise
if __name__=='__main__':main()
