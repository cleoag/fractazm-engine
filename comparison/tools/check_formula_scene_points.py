"""Compare registered formulas with shipped x86 code at identical expanded parameters.
Diagnostic only: parameter expansion, formula initialization and whole orbits are
not independently validated. Maps and pointer-bearing folding are explicit skips.
"""
import argparse,hashlib,itertools,json,math,random,subprocess,sys,os,struct
from pathlib import Path
ROOT=Path(__file__).resolve().parents[2]
sys.path.insert(0,str(ROOT/'scripts'))
from gen_bulk_formula_table import parse_m3f_options
from external_validation import write_json
from formula_point_contexts import scalar_map_context

def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()

class PointContextRequired(ValueError):
 """The generic point process cannot provide the original caller context."""

class FormulaInputRejected(RuntimeError):
 """The strict C++ probe rejected an unsupported input before evaluation."""

def require_independent_point_context(name, parameters):
 if name in ('gnarlyIFS','gnarly2IFS','gnarly3IFS','ducksIFS'):
  if len(parameters)!=1024:raise ValueError('Incomplete original parameter buffer')
  if struct.unpack_from('<i',parameters,512-0x54)[0]&1:
   raise PointContextRequired(name+' odd Flip XZ reads the Pascal caller stack; generic point oracle context is not an independent reference')

def rounding_coordinates():
 """Probe both sides of half tiles and signed 32-bit conversion limits."""
 values=[0.,-0.,1e-300,-1e-300]
 for magnitude in (.5,1.,1.5,2.,3.,4.5,6.,2147483647.5,2147483648.,4294967295.,6442450942.5):
  for sign in (-1.,1.):
   v=sign*magnitude
   values.extend((math.nextafter(v,-math.inf),v,math.nextafter(v,math.inf)))
 return [[v,-v,v,.375] for v in values]
def orbit_scale_coordinates():
 """Selected large orbit magnitudes and both sides of FSINCOS's 2^63 limit."""
 values=[math.ldexp(1.,n) for n in range(0,1024,8)]
 values += [math.nextafter(2.**63,-math.inf),2.**63,math.nextafter(2.**63,math.inf)]
 return [[sign*v,-sign*v,sign*v,.375] for v in values for sign in (-1.,1.)]

def nonfinite_coordinates():
 """All XYZ placements of finite, NaN and signed infinite values."""
 return [[*xyz,.375] for xyz in itertools.product((.25,float('nan'),float('inf'),-float('inf')),repeat=3)]

def quaternion_boundary_coordinates():
 """QuatPow's real/vector zero, log threshold and normalization boundaries."""
 result=[[x,*v] for x in (-1.,-0.,0.,1.) for v in itertools.product((0.,-0.),repeat=3)]
 for threshold in (1e-200,1e-100):
  for value in (math.nextafter(threshold,0.),threshold,math.nextafter(threshold,math.inf)):
   result.append([value,0.,0.,0.])
   for axis in (1,2,3):
    p=[0.,0.,0.,0.];p[axis]=value;result.append(p)
 for axis in range(4):
  for value in (float('nan'),float('inf'),-float('inf')):
   p=[1.,0.,0.,0.];p[axis]=value;result.append(p)
 return result

def validate_resume(previous, current, cases, output):
 """Accept only a verified prefix from the exact binaries, inputs and profile."""
 for key,value in current.items():
  if key not in ('finished','results') and previous.get(key)!=value:
   raise ValueError('Resume identity differs: '+key)
 rows=previous.get('results',[])
 if len(rows)>len(cases):raise ValueError('Resume exceeds selected cases')
 for case,row in zip(cases,rows):
  name=case['target_formulas'][0]
  if row.get('case')!=case['id'] or row.get('name')!=name:
   raise ValueError('Resume is not the selected case prefix')
  if row.get('scene_sha256')!=case['sha256'] or sha(ROOT/case['scene'])!=case['sha256']:
   raise ValueError('Resume scene changed: '+case['id'])
  if sha(ROOT/'mb3d/M3Formulas'/f'{name}.m3f')!=row.get('asset_sha256'):
   raise ValueError('Resume formula asset changed: '+name)
  if row.get('status') not in ('pass','fail','error','unverified'):
   raise ValueError('Invalid saved result status')
  if row['status'] in ('pass','fail') or row.get('parameter_sha256'):
   blob=output/(case['id']+'.params')
   if not blob.exists() or sha(blob)!=row.get('parameter_sha256'):
    raise ValueError('Resume parameter blob changed: '+case['id'])
 return len(rows)

def execute(args,rows,lanes=20):
 p=subprocess.run([str(x) for x in args],input=rows,text=True,capture_output=True,timeout=15)
 if p.returncode==12 and p.stderr.startswith('FORMULA_INPUT_REJECTED['):
  raise FormulaInputRejected(p.stderr.strip())
 if p.returncode:raise RuntimeError(f'exit {p.returncode}: {p.stderr[:300]}')
 vals=[float('nan') if 'nan' in x.lower() else float(x) for x in p.stdout.split()]
 return [vals[i:i+lanes] for i in range(0,len(vals),lanes)]
def main():
 ap=argparse.ArgumentParser();ap.add_argument('--cpp',type=Path,required=True);ap.add_argument('--oracle',type=Path,required=True);ap.add_argument('--manifest',type=Path,required=True);ap.add_argument('--output',type=Path,required=True);ap.add_argument('--all-cases',action='store_true');ap.add_argument('--julia-inputs',action='store_true');ap.add_argument('--conditional-counters',action='store_true');ap.add_argument('--rout-inputs',action='store_true');ap.add_argument('--first-iteration-inputs',action='store_true');ap.add_argument('--iteration-inputs',action='store_true');ap.add_argument('--derivative-inputs',action='store_true');ap.add_argument('--resume',action='store_true',help='Continue a hash-verified checkpoint prefix with the same inputs and binaries');ap.add_argument('--sine-inputs',action='store_true',help='Sine range-reduction boundaries on all four coordinates');ap.add_argument('--julia-mode-inputs',action='store_true');ap.add_argument('--counter-word-inputs',action='store_true',help='Every low counter word at two fixed coordinate tuples');ap.add_argument('--checkpoint-every',type=int,default=1,help='Atomically save each N results; an interrupted uncommitted tail is rerun');ap.add_argument('--rounding-inputs',action='store_true',help='Half-tile neighbors and signed integer conversion limits');ap.add_argument('--nonfinite-inputs',action='store_true',help='All XYZ combinations of finite, NaN, and signed infinities');ap.add_argument('--orbit-scale-inputs',action='store_true',help='Large orbit magnitudes and FSINCOS range boundaries');ap.add_argument('--map-context-inputs',action='store_true',help='Vary incoming Rold and trap for analytic sphere map callbacks');ap.add_argument('--quaternion-inputs',action='store_true',help='QuatPow signed-zero, epsilon-neighbor and exceptional XYZW inputs');a=ap.parse_args()
 if a.checkpoint_every<1:ap.error('--checkpoint-every must be positive')
 a.output.mkdir(parents=True,exist_ok=True);rng=random.Random(0x24563487)
 inputs=[[*xyz,.375,.2,-.3,.4,1.,1.,1.,1.] for xyz in itertools.product([-2.,-1e-12,0.,1e-12,2.],repeat=3)]
 inputs += [[*(rng.uniform(-3,3) for _ in range(3)),rng.uniform(-1,1),*(rng.uniform(-1,1) for _ in range(3)),1.,1.,1.,1.] for _ in range(256)]
 if a.counter_word_inputs:
  if any((a.first_iteration_inputs,a.iteration_inputs,a.derivative_inputs,a.conditional_counters,a.rout_inputs,a.julia_mode_inputs)):
   raise ValueError('Counter-word profile cannot share overloaded input lanes')
  os.environ['MB3D_POINT_FIRST']='1'
  inputs=[[*xyz,.375,.2,-.3,.4,1.,counter,1.,1.]
          for xyz in ((.125,-.25,.5),(-2.,1e-12,0.)) for counter in range(65536)]
 if a.quaternion_inputs:
  inputs += [xyz+[.2,-.3,.4,1.,1.,1.,1.] for xyz in quaternion_boundary_coordinates()]
 if a.orbit_scale_inputs:
  inputs += [xyz+[.2,-.3,.4,1.,1.,1.,1.] for xyz in orbit_scale_coordinates()]
 if a.nonfinite_inputs:
  inputs += [xyz+[.2,-.3,.4,1.,1.,1.,1.] for xyz in nonfinite_coordinates()]
 if a.rounding_inputs:
  inputs += [xyz+[.2,-.3,.4,1.,1.,1.,1.] for xyz in rounding_coordinates()]
 if a.sine_inputs:
  # Neighboring doubles around half/whole cycles, signed zero and large inputs.
  values=[0.,-0.,1e-300,-1e-300,1e12,-1e12]
  for multiple in range(-32,33):
   v=multiple*math.pi/2
   values.extend((math.nextafter(v,-math.inf),v,math.nextafter(v,math.inf)))
  inputs += [[v,-v,v,v,.2,-.3,.4,1.,1.,1.,1.] for v in values]
 if a.julia_inputs:
  # Singular axes and zero C are missing from the random Julia samples.
  inputs += [[1.,-2.,3.,.375,*j,1.,1.,1.,1.] for j in
             [(0.,0.,0.),(1.,0.,0.),(0.,1.,0.),(0.,0.,1.),
              (-1.,0.,0.),(0.,-1.,0.),(0.,0.,-1.),(1e-160,1e-160,1e-160)]]
 if a.julia_mode_inputs:
  if a.derivative_inputs or a.conditional_counters:raise ValueError('Julia mode profile shares an input lane with derivatives/counters')
  os.environ['MB3D_POINT_JULIA_MODE']='1'
  inputs=[row[:10]+[mode] for row in inputs for mode in (-2147483648,-1,0,1,2147483647)]
 if a.rout_inputs:
  if a.conditional_counters:raise ValueError('Rout and counter input profiles are mutually exclusive')
  os.environ['MB3D_POINT_ROUT']='1'
  inputs=[row[:7]+[radius]+row[8:] for row in inputs for radius in [0.,1e-30,.25,13.5,-1.,float('inf'),float('nan')]]
 if a.first_iteration_inputs:
  if a.conditional_counters:raise ValueError('First-state and counter profiles are mutually exclusive')
  os.environ['MB3D_POINT_FIRST']='1'
  inputs=[row[:8]+[first]+row[9:] for row in inputs for first in [-2147483648,-1,0,1,2,2147483647]]
 if a.iteration_inputs or a.derivative_inputs:
  if a.conditional_counters or a.rout_inputs or a.first_iteration_inputs:raise ValueError('Iteration/derivative profiles cannot share overloaded input lanes with other profiles')
 if a.iteration_inputs:
  os.environ['MB3D_POINT_ITERATION']='1'
  inputs=[row[:7]+[iteration]+row[8:] for row in inputs for iteration in [-2147483648,-1,0,1,2,20,2147483647]]
 if a.derivative_inputs:
  os.environ['MB3D_POINT_DERIVATIVES']='1'
  inputs=[row[:8]+list(deriv) for row in inputs for deriv in [(0.,0.,0.),(1.,0.,0.),(-2.,.375,3.),(1e-12,-1e-12,.5)]]
 spheremap=os.environ.get('MB3D_POINT_SPHEREMAP_CALLBACK')=='1'
 heightmap=os.environ.get('MB3D_POINT_HEIGHTMAP_CALLBACK')=='1'
 if a.map_context_inputs:
  if not spheremap or heightmap:raise ValueError('Map context profile requires sphere callback only')
  os.environ['MB3D_POINT_MAP_CONTEXT']='1'
 if heightmap or a.map_context_inputs:
  if any((a.conditional_counters,a.rout_inputs,a.first_iteration_inputs,a.iteration_inputs,a.derivative_inputs,a.counter_word_inputs,a.julia_mode_inputs)):raise ValueError('Heightmap profile owns Rold/Dfree input lanes')
  inputs=[row[:7]+[rold,trap]+row[9:] for row in inputs for rold in (-1.,0.,.001,1.,float('nan')) for trap in (0.,-.375)]
 if spheremap and (heightmap or a.conditional_counters):raise ValueError("Incompatible map/counter profiles")
 lanes=25 if spheremap else (24 if heightmap else (26 if a.conditional_counters else 20))
 if a.conditional_counters:
  os.environ['MB3D_POINT_COUNTERS']='1'
  inputs=[row[:7]+[counter,rng.randrange(64),rng.randrange(100),1.] for row in inputs for counter in [-2147483648,-2,-1,*range(18),98,99,100,101,2147483647]]
 stdin=''.join(' '.join(format(v,'.17g') for v in row)+'\n' for row in inputs)
 report={'scope':__doc__,'oracle_abi':'DE20/21: ESI=PIt3D+88, EDI=PVar, plain RET; otherwise Delphi register/RET8','cpp_sha256':sha(a.cpp),'oracle_sha256':sha(a.oracle),'manifest_sha256':sha(a.manifest),'seed':0x24563487,'finished':False,'results':[]}
 report['tool_sha256']={str(p.relative_to(ROOT)):sha(p) for p in
  [Path(__file__).resolve(),ROOT/'comparison/tools/external_validation.py',ROOT/'comparison/tools/formula_point_contexts.py',ROOT/'scripts/gen_bulk_formula_table.py']}
 cases=json.loads(a.manifest.read_text())['cases']
 if not a.all_cases:cases=[c for c in cases if c['id'].endswith('_early')]
 if a.conditional_counters:cases=[c for c in cases if c['target_formulas'][0] in ('_CondItcountR','_CondItcountSwitch','_CondItcountSwX2','_CondLyapunov','_CondLyapunovEx')]
 report['checkpoint_every']=a.checkpoint_every
 report['lanes']=lanes
 report['counter_word_inputs']=a.counter_word_inputs
 report['iteration_inputs']=a.iteration_inputs
 report['derivative_inputs']=a.derivative_inputs
 report['julia_inputs']=a.julia_inputs
 report['sine_inputs']=a.sine_inputs
 report['rounding_inputs']=a.rounding_inputs
 report['nonfinite_inputs']=a.nonfinite_inputs
 report['orbit_scale_inputs']=a.orbit_scale_inputs
 report['quaternion_inputs']=a.quaternion_inputs
 report['julia_mode_inputs']=a.julia_mode_inputs
 report['rout_inputs']=a.rout_inputs
 report['first_iteration_inputs']=a.first_iteration_inputs
 report['conditional_counters']=a.conditional_counters
 report['all_cases']=a.all_cases
 report['map_context_inputs']=a.map_context_inputs
 report['spheremap_callback']=spheremap
 if spheremap:report['map_reference']='Original x86 CODE with Delphi callback RGB=(z,x,mapNr); verifies XYZ/ABI, not sphere projection/interpolation'
 report['heightmap_callback']=heightmap
 if heightmap:report['map_reference']='Original x86 CODE with Delphi callback RGB=(u,v,mapNr); verifies formula/ABI, not image interpolation'
 report['input_sha256']=hashlib.sha256(stdin.encode('ascii')).hexdigest()
 report['selected_case_count']=len(cases)
 start=0
 if a.resume:
  previous=json.loads((a.output/'summary.json').read_bytes())
  start=validate_resume(previous,report,cases,a.output)
  report['results']=previous['results']
  print('Resuming verified prefix:',start,'of',len(cases),flush=True)
 write_json(a.output/'summary.json',report)
 for c in cases[start:]:
  name=c['target_formulas'][0];asset=ROOT/'mb3d/M3Formulas'/f'{name}.m3f';entry={'case':c['id'],'name':name,'asset_sha256':sha(asset),'scene_sha256':sha(ROOT/c['scene']),'status':'error'}
  try:
   if entry['scene_sha256']!=c['sha256']:raise ValueError('Scene differs from manifest')
   meta=parse_m3f_options(asset);source=asset.read_text(encoding='latin1')
   scalar_map=scalar_map_context(name,entry['asset_sha256'])
   if scalar_map:entry['map_scope']='Audited scalar Log2MapPower; no callback. Pascal map-loading metadata preserved.'
   if (meta['map_value_mask'] and not (scalar_map or (heightmap and name in ('HeightMapIFS','HeightMap2IFS','CylinderHeightMap','CylinderHMVoid','TorusHMVoid')) or (spheremap and name in ('_MapTranslate','ABoxMapSFold','SphereHeightMap','SphereHMVoid')))) or (8 in meta['option_types'] and name not in ('_SphereFolding1','_SphereFolding2','_Folding2')):entry.update(status='unverified',reason='map callback or pointer-bearing folding requires dedicated oracle')
   elif name=='ManowarBulb':entry.update(status='unverified',reason='Incoming XMM0/XMM1 require check_formula_sse_state.py; unseeded oracle state is not a Pascal precondition')
   elif '[CODE]' not in source:entry.update(status='unverified',reason='JIT source has no x86 code')
   else:
    blob=a.output/(c['id']+'.params');code=a.output/(name+'.bin');code.write_bytes(bytes.fromhex(source.split('[CODE]')[1].split('[END]')[0]))
    actual=execute([a.cpp.resolve(),name,(ROOT/c['scene']).resolve(),blob.resolve()],stdin,lanes)
    entry['parameter_sha256']=sha(blob)
    require_independent_point_context(name,blob.read_bytes())
    expected=execute([a.oracle.resolve(),code.resolve(),blob.resolve()]+(['difs'] if meta['de_option'] in (20,21) else []),stdin,lanes)
    if len(actual)!=len(inputs) or len(expected)!=len(inputs) or any(len(r)!=lanes for r in actual+expected):raise ValueError('incomplete output')
    failures=[];counts=[0]*lanes;maximum=0.;c_delta=0
    if heightmap or spheremap:
     from collections import Counter
     entry['callback_counts']=dict(Counter(str(int(r[24 if spheremap else 23])) for r in expected))
    for i,(row,ref) in enumerate(zip(actual,expected)):
     if all(math.isfinite(row[k]) and math.isfinite(ref[k]) and abs(row[k]-ref[k]-inputs[i][4+k])<2e-12*max(1,abs(ref[k])) for k in range(3)):c_delta+=1
     for k,(v,e) in enumerate(zip(row,ref)):
      if math.isfinite(v) and math.isfinite(e):error=abs(v-e)/max(1,abs(e));maximum=max(maximum,error);ok=error<=2e-12
      else:ok=(math.isnan(v) and math.isnan(e)) or v==e
      if not ok:
       counts[k]+=1
       if len(failures)<5:failures.append({'row':i,'lane':k,'input':inputs[i],'cpp':str(v),'reference':str(e)})
    entry.update(status='fail' if any(counts) else 'pass',cases=len(inputs),failed_lanes=sum(counts),lane_failures=counts,max_scaled_error=maximum,extra_julia_rows=c_delta,examples=failures,parameter_sha256=sha(blob))
  except FormulaInputRejected as e:
   entry.update(status='unverified',input_validation='rejected',cpp_exit_code=12,reason=str(e))
   if blob.exists():entry['parameter_sha256']=sha(blob)
  except PointContextRequired as e:entry.update(status='unverified',reason=str(e))
  except (Exception,) as e:entry['error']=str(e)
  report['results'].append(entry);print(c['id'],entry['status'],entry.get('lane_failures',entry.get('reason',entry.get('error',''))),flush=True)
  if len(report['results']) % a.checkpoint_every == 0:write_json(a.output/'summary.json',report)
 report['finished']=True
 from collections import Counter
 report['counts']=dict(Counter(x['status'] for x in report['results']));write_json(a.output/'summary.json',report);print(report['counts'])
if __name__=='__main__':main()
