"""Compare Manowar with explicit incoming/outgoing XMM0/XMM1 lanes.

An unseeded C++ oracle process does not reproduce Pascal register preconditions.
This profile certifies individual formula calls, not arbitrary hybrid drivers.
"""
import argparse,json,math,os,random
from pathlib import Path
from check_formula_scene_points import execute,sha
from external_validation import ROOT,write_json


def main():
 ap=argparse.ArgumentParser(description=__doc__)
 for key in ('cpp','oracle','manifest','output'):ap.add_argument('--'+key,type=Path,required=True)
 a=ap.parse_args();a.output.mkdir(parents=True,exist_ok=False)
 os.environ['MB3D_POINT_SSE01']='1';os.environ['MB3D_POINT_FIRST']='1'
 for key in ('MB3D_POINT_DERIVATIVES','MB3D_POINT_ROUT','MB3D_POINT_ITERATION'):os.environ.pop(key,None)
 profiles=[]
 for first in (-2147483648,-1,0,1,2,2147483647):
  for lanes in ([0.,0.,0.,0.],[4.,0.,2.,0.],[.2,-.3,.4,-.5],[1e12,-1e12,1e-12,-1e-12]):
   for xyz in ([.125,-.25,.5],[-.5,.25,-.125],[0.,0.,0.]):
    profiles.append(list(xyz)+[.375,.2,-.3,.4,1.,first,.375,3.]+list(lanes))
 rng=random.Random(0x535345)
 for i in range(128):
  profiles.append([rng.uniform(-2,2) for _ in range(4)]+[.2,-.3,.4,1.,(-1,0,1)[i%3],.375,3.]+[rng.uniform(-4,4) for _ in range(4)])
 stdin=''.join(' '.join(format(v,'.17g') for v in row)+'\n' for row in profiles)
 (a.output/'inputs.txt').write_text(stdin)
 asset=ROOT/'mb3d/M3Formulas/ManowarBulb.m3f';code=a.output/'ManowarBulb.bin'
 code.write_bytes(bytes.fromhex(asset.read_text().split('[CODE]')[1].split('[END]')[0]))
 report=dict(scope=__doc__,finished=False,cpp_sha256=sha(a.cpp),oracle_sha256=sha(a.oracle),asset_sha256=sha(asset),manifest_sha256=sha(a.manifest),tool_sha256=sha(Path(__file__)),results=[])
 for case in json.loads(a.manifest.read_bytes())['cases']:
  if case['target_formulas']!=['ManowarBulb']:continue
  scene=ROOT/case['scene'];assert sha(scene)==case['sha256']
  blob=a.output/(case['id']+'.params')
  actual=execute([a.cpp.resolve(),'ManowarBulb',scene.resolve(),blob.resolve()],stdin,24)
  expected=execute([a.oracle.resolve(),code.resolve(),blob.resolve()],stdin,24)
  assert len(actual)==len(expected)==len(profiles)
  failures=[]
  for i,(c,p) in enumerate(zip(actual,expected)):
   for lane,(v,e) in enumerate(zip(c,p)):
    same=math.isnan(v) if math.isnan(e) else v==e if math.isinf(e) or lane>=20 else math.isfinite(v) and abs(v-e)<=2e-12*max(1.,abs(e))
    if not same:failures.append(dict(row=i,lane=lane,expected=str(e),actual=str(v)))
  row=dict(case=case['id'],name='ManowarBulb',scene_sha256=case['sha256'],rows=len(profiles),lanes=24,status='fail' if failures else 'pass',failed_lanes=len(failures),examples=failures[:12])
  report['results'].append(row);write_json(a.output/'summary.json',report)
  write_json(a.output/(case['id']+'-golden.json'),dict(input=profiles,expected=[[str(v) for v in row] for row in expected]))
  print(case['id'],row['status'],len(failures),flush=True)
 report['finished']=True;write_json(a.output/'summary.json',report)
 return any(row['status']!='pass' for row in report['results'])

if __name__=='__main__':raise SystemExit(main())
