"""Compare Benesi ports with original x86 code, plus JIT source equations.
The native oracle is compiled for x86 from benesi_x86_oracle.cpp. All
executions are child processes with a timeout. No reference files are changed.
"""
import argparse,hashlib,itertools,json,math,random,subprocess
from pathlib import Path
ROOT=Path(__file__).resolve().parents[2]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def run(exe,arg,rows):
 p=subprocess.run([str(exe.resolve()),str(arg)],input=''.join(' '.join(format(v,'.17g') for v in r)+'\n' for r in rows),text=True,capture_output=True,timeout=30)
 if p.returncode:raise RuntimeError(f'{exe.name}: exit {p.returncode}: {p.stderr[:500]}')
 result=[[(float('nan') if 'nan' in x.lower() else float(x)) for x in line.split()] for line in p.stdout.splitlines()]
 if len(result)!=len(rows) or any(len(r)!=8 for r in result):raise RuntimeError('incomplete oracle output')
 return result
def jit(name,r):
 x,y,z,w,j1,j2,j3,p0,p1=r[:9]
 if name=='JITTrigExample':return [math.sinh(x*p0),y,z,w]
 if name=='JIT_IQ_Bulb':
  radius=(x*x+y*y+z*z)**(.5*p0);theta=math.atan2(math.hypot(x,z),y)*p0;phi=math.atan2(x,z)*p0
  return [math.sin(phi)*math.sin(theta)*radius+j1,math.cos(theta)*radius+j2,math.sin(theta)*math.cos(phi)*radius+j3,w]
 a,b,c=.816496580927726,.577350269189626,.707106781186548
 tx=x*a-z*b;x,y,z=abs((tx-y)*c),abs((tx+y)*c),abs(x*b+z*a)
 tx=(x+y)*c;x,y,z=tx*a+z*b,(-x+y)*c,-tx*b+z*a
 x,y,z=p1*x-p0,p1*y,p1*z
 t=2*x/math.hypot(y,z) if y or z else math.copysign(math.inf,x)
 return [x*x-y*y-z*z+j1,t*2*y*z,t*(y*y-z*z),w]
def main():
 ap=argparse.ArgumentParser();ap.add_argument('--cpp',type=Path,required=True);ap.add_argument('--oracle',type=Path,required=True);ap.add_argument('--output',type=Path,required=True);a=ap.parse_args()
 a.output.mkdir(parents=True,exist_ok=True);rng=random.Random(0x24563487)
 report={'scope':'One iteration, XYZ/W and J1..J4; persistent C = (x+.125,y-.25,z+.5); supplied options. No image or full-hybrid certification.','seed':0x24563487,'cpp_sha256':sha(a.cpp),'oracle_sha256':sha(a.oracle),'formulas':[]}
 for name in ['_BenesiT1','_BenesiT2','_BenesiT3','_BenesiSpheric','_BenesiUnSpheric','JIT_IQ_Bulb','JITBenesiPineTree','JITTrigExample','_Abs4d','_AbsX','_AbsY','_AbsZ','_conj','_conj2D','_flipC-xyz','_FlipXY','_FlipXZ','_FlipYZ','_FlipXYc','_FlipXZc','_FlipYZc','_Folding1','_FoldingOct','_invcylindrical','_invspherical','_invtorical','_invtorical2','_JuliaSets','_QuadrayTransform','_QuadrayTransform2','_reciprocalX','_reciprocalX2','_reciprocalY2','_reciprocalZ2','_skew','_tocylindrical','_tospherical','_totorical','_updateC','_updateC_4d','_YplusSinZ','_Rpow3','_SinePow2','_SphereInvC','_SquareR','_HopSqrtX','_HopSqrtY','_HopSqrtZ']:
  asset=ROOT/'mb3d/M3Formulas'/f'{name}.m3f';source=asset.read_text(encoding='latin1')
  rows=[]
  for xyz in itertools.product([-2.,-1.,-1e-12,0.,1e-12,1.,2.],repeat=3):
   for params in [(2.,2.),(-1.,.5),(0.,0.)] if name.startswith('_') else [(2.,2.)]:rows.append([*xyz,.375,.2,-.3,.4,*params])
  for _ in range(1000):
   rows.append([*(rng.uniform(-3,3) for _ in range(3)),rng.uniform(-1,1),*(rng.uniform(-1,1) for _ in range(3)),rng.uniform(.1,3),rng.uniform(-3,3)])
  rows=[r+[1.,1.] for r in rows]
  actual=run(a.cpp,name,rows)
  if name.startswith('_'):
   code=bytes.fromhex(source.split('[CODE]')[1].split('[END]')[0]);path=a.output/f'{name}.bin';path.write_bytes(code)
   expected=run(a.oracle,path.resolve(),rows);kind='original-x86-code'
  else:expected=[jit(name,r)+r[4:7]+[0.] for r in rows];kind='independent-Python-source-equations'
  errors=[];maximum=0
  for i,(x,y) in enumerate(zip(actual,expected)):
   for lane,(v,e) in enumerate(zip(x,y)):
    if not math.isfinite(v) or not math.isfinite(e):ok=(math.isnan(v) and math.isnan(e)) or v==e
    else:
     relative=abs(v-e)/max(1,abs(e));maximum=max(maximum,relative);ok=relative<=2e-12
    if not ok:errors.append({'row':i,'lane':lane,'input':rows[i],'cpp':str(v),'reference':str(e)})
  entry={'name':name,'reference':kind,'asset_sha256':sha(asset),'cases':len(rows),'failed_lanes':len(errors),'max_scaled_error':maximum,'examples':errors[:10]};report['formulas'].append(entry);print(name,len(rows),len(errors),maximum)
 report['passed']=all(not f['failed_lanes'] for f in report['formulas']);(a.output/'summary.json').write_text(json.dumps(report,indent=2)+'\n')
 raise SystemExit(0 if report['passed'] else 1)
if __name__=='__main__':main()
