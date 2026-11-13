#!/usr/bin/env python3
"""Freeze and compare canonical real primary caller boundaries."""
import argparse,hashlib,json,shlex,shutil,subprocess,sys
from pathlib import Path
from gen_native_refinement_continuations import generate_refinement
from gen_native_de_refinement_continuations import generate_de_refinement
from gen_native_primary_continuations import generate_primary
from gen_native_primary_trace import candidate_observed,original_observed
from gen_native_orbit_continuations import function
from gen_native_metal_formulas import emit_text
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT))
from comparison.tools.run_m3parameter_gpu import invoke

def sha(path):return hashlib.sha256(Path(path).read_bytes()).hexdigest()
def main():
 ap=argparse.ArgumentParser(description=__doc__);ap.add_argument('--output',type=Path,required=True);ap.add_argument('--baseline-library',type=Path);a=ap.parse_args();a.real=True;out=a.output.resolve()
 if out.exists():ap.error('New output required')
 if shutil.disk_usage(ROOT).free<12*1024**3:ap.error('12 GiB reserve unavailable')
 out.mkdir(parents=True);(out/'data').mkdir();stage=out/'engine'
 receipt={'scope':('real primary canonical-boundary composition; no renderer acceptance' if a.real else 'synthetic primary control flow only; no native DE or renderer acceptance'),'source_commit':subprocess.check_output(['git','rev-parse','HEAD'],cwd=ROOT,text=True).strip(),'commands':[],'dependencies':{},'status':'BUILDING','pipeline_cache':'no explicit archive; driver-cache state unknown'}
 def save():(out/'receipt.json').write_text(json.dumps(receipt,indent=2)+'\n')
 def run(cmd):
  row={'argv':list(map(str,cmd))};receipt['commands'].append(row);save();p=subprocess.run(row['argv'],cwd=ROOT,capture_output=True,text=True,timeout=120);row['exit_code']=p.returncode
  log=out/f'command-{len(receipt["commands"])}.log';log.write_text(p.stdout+p.stderr);row['log_sha256']=sha(log);save()
  if p.returncode:raise RuntimeError(p.stderr[-2000:])
 try:
  audit=ROOT/'comparison/results/p2-state-audit-20261103/capabilities.json';closure=json.loads(audit.read_text())['selected_dependency_closure']
  # Match the shared ExactScene headers to the pinned P1 build, too.
  p1=ROOT/'comparison/results/p1-prepared-scene-20261103/build-record.json'
  for path,digest in json.loads(p1.read_text())['dependencies'].items():
   marker='/comparison/metal_exact/'
   if marker not in path:continue
   name='comparison/metal_exact/'+path.split(marker,1)[1]
   for revision in ['fc00c78e02b1aba530e611a07f2bf72cc5dbd099','11d13296']:
    blob=subprocess.check_output(['git','cat-file','blob',revision+':'+name],cwd=ROOT)
    if hashlib.sha256(blob).hexdigest()==digest:closure[name]={'sha256':digest,'revision':revision};break
   else:raise ValueError('Unbound shared P1 header: '+name)
  receipt['selected_source_closure']=closure
  for name,item in closure.items():
   blob=subprocess.check_output(['git','cat-file','blob',item['revision']+':'+name],cwd=ROOT);assert hashlib.sha256(blob).hexdigest()==item['sha256']
   target=stage/name;target.parent.mkdir(parents=True,exist_ok=True);target.write_bytes(blob)
  overlays=['continuation_abi.h','continuation_frames.h']
  if a.real:overlays+=['continuation_step.metal.h','continuation_orbit_step.metal.h','continuation_replay_step.metal.h','continuation_distance_step.metal.h','continuation_combined_step.metal.h','continuation_entry_step.metal.h','continuation_normal_sample_step.metal.h','continuation_normal_step.metal.h','continuation_refinement_step.metal.h','continuation_de_refinement_step.metal.h','continuation_primary_step.metal.h']
  for name in overlays:
   p=ROOT/'src/render/native_metal'/name;shutil.copy2(p,stage/'src/render/native_metal'/name);receipt['dependencies'][str(p)]=sha(p)
  source=(stage/'src/render/native_metal/general_geometry.metal.h').read_text()
  (out/'refinement_regions.metal.h').write_text(generate_refinement(source))
  body=emit_text(function(source,'general_refine_iteration_count'))
  signature='thread float3& point,float3 ray,thread float& depth,float stop,constant ExactScene& scene,constant NativeFormulaProgram& program,thread NativeResources& resources,thread NativeFormulaState& lane,thread NativeGeometryRecord& record,thread uint& iteration,uint refinement_steps'
  (out/'refinement_control.metal.h').write_text('inline bool refinement_control('+signature+'){'+body+'}\n')
  (out/'de_refinement_regions.metal.h').write_text(generate_de_refinement(source))
  (out/'primary_regions.metal.h').write_text(generate_primary(source))
  signature='constant ExactScene& scene,constant NativeFormulaProgram& program,mb3d_camera::Ray camera,uint x,uint y,thread NativeResources& resources,thread NativeGeometryRecord& record,thread NativeFormulaState& lane,NativeNormalParams normal_params,thread NativeNormalRecord& normal_record,thread NativeNormalResult& normal_result,int inside_word,thread PrimaryRandomState* random=nullptr'
  (out/'primary_control.metal.h').write_text('inline ExactPixel primary_control('+signature+'){'+emit_text(function(source,'general_primary_camera_attempt'))+'}\n')
  signature='constant ExactScene& scene,constant NativeFormulaProgram& program,mb3d_camera::Ray camera,uint x,uint y,thread NativeResources& resources,thread NativeGeometryRecord& record,thread NativeFormulaState& lane,NativeNormalParams normal_params,thread NativeNormalRecord& normal_record,thread NativeNormalResult& normal_result,int inside_word,thread Observer& observer,thread PrimaryRandomState* random=nullptr'
  (out/'primary_observer.metal.h').write_text('template<class Observer> inline ExactPixel primary_observed('+signature+'){'+original_observed(source)+'}\n')
  (out/'primary_regions.metal.h').write_text(candidate_observed(source))
  if a.real:
   for name in ['gen_native_caller_continuations.py','gen_native_entry_continuations.py','gen_native_normal_sample_continuations.py','gen_native_normal_continuations.py']:run([sys.executable,ROOT/'scripts'/name,'--source-root',stage,'--output',out])
   run([sys.executable,ROOT/'scripts/gen_native_continuations.py','--output',out])
  shader=ROOT/('tests/metal/p2_primary_observed_composition.metal' if a.real else 'tests/metal/p2_primary_continuation.metal');host=ROOT/('tests/test_native_primary_observed_composition.mm' if a.real else 'tests/test_native_primary_continuations.mm')
  flags=['xcrun','-sdk','macosx','metal','-v','-std=metal3.1','-fno-fast-math','-ffp-contract=fast','-I',stage,'-I',out,'-I',ROOT/'tests']
  for name,extra in [('reference',['-DP2_REFERENCE=1']),('probe',[])]:
   run([*flags,*extra,'-MMD','-MF',out/(name+'.d'),'-c',shader,'-o',out/(name+'.air')])
   run(['xcrun','-sdk','macosx','metallib',out/(name+'.air'),'-o',out/(name+'.metallib')])
  run(['xcrun','clang++','-v','-std=c++20','-O2','-fobjc-arc','-Wall','-Wextra','-Werror','-MMD','-MF',out/'host.d','-I',stage,host,'-framework','Foundation','-framework','Metal','-o',out/'probe'])
  for dep in [out/'probe.d',out/'reference.d',out/'host.d']:
   for path in shlex.split(dep.read_text().replace('\\\n',' ').split(': ',1)[1]):
    p=Path(path);p=p if p.is_absolute() else ROOT/p;receipt['dependencies'][str(p)]=sha(p)
  for p in [Path(__file__),audit,p1]+[ROOT/'scripts'/name for name in ['gen_native_primary_trace.py','gen_native_primary_continuations.py','gen_native_de_refinement_continuations.py','gen_native_refinement_continuations.py','gen_native_entry_continuations.py','gen_native_orbit_continuations.py','gen_native_continuations.py','gen_native_metal_formulas.py']]:receipt['dependencies'][str(p.resolve())]=sha(p)
  if a.real:
   for name in ['gen_native_caller_continuations.py','gen_native_replay_continuations.py','gen_native_distance_continuations.py','gen_native_combined_continuations.py','gen_native_normal_sample_continuations.py','gen_native_normal_continuations.py']:
    p=ROOT/'scripts'/name;receipt['dependencies'][str(p)]=sha(p)
  for path in receipt['dependencies']:
   p=Path(path)
   if p.is_relative_to(ROOT):dest=out/'source'/p.relative_to(ROOT);dest.parent.mkdir(parents=True,exist_ok=True);shutil.copy2(p,dest)
  receipt['products']={name:sha(out/name)for name in ['probe','probe.air','probe.metallib','reference.air','reference.metallib']}
  reference=a.baseline_library.resolve() if a.baseline_library else out/'reference.metallib';receipt['frozen_control']={'path':str(reference),'sha256':sha(reference)};save()
  if a.real:
   receipt['executions']=[]
   for repeat in range(3):
    data=out/'data'/str(repeat);data.mkdir()
    execution=invoke([str(out/'probe'),str(out/'probe.metallib'),str(data),str(reference),str(repeat)],out/f'run-{repeat}',ROOT,timeout=150,exclusive_gpu=True)
    receipt['executions'].append(execution);save()
    if execution['exit_code']!=0:break
   receipt['execution']=receipt['executions'][-1]
  else:
   receipt['execution']=invoke([str(out/'probe'),str(out/'probe.metallib'),str(out/'data'),str(reference)],out/'run',ROOT,timeout=150,exclusive_gpu=True)
  for path,digest in receipt['dependencies'].items():
   if sha(path)!=digest:raise RuntimeError('Source changed during proof: '+path)
  if sha(reference)!=receipt['frozen_control']['sha256']:raise RuntimeError('Frozen control changed')
  receipt['observations']={str(p.relative_to(out/'data')):{'bytes':p.stat().st_size,'sha256':sha(p)}for p in(out/'data').rglob('*.bin')}
  if a.real and len(receipt['executions'])==3 and all(x['exit_code']==0 for x in receipt['executions']):
   for name in ['input.bin','control.bin','observed.bin','observed-trace.bin','candidate-first.bin','candidate.bin','candidate-trace-first.bin','candidate-trace.bin']:
    if len({sha(out/'data'/str(i)/name)for i in range(3)})!=1:raise RuntimeError('Cross-process repeat mismatch: '+name)
  receipt['status']='PASS'if receipt['execution']['exit_code']==0 else 'FAIL';save();print(receipt['status']);return int(receipt['status']!='PASS')
 except Exception as e:receipt['status']='ERROR';receipt['error']=str(e);save();print(str(e),file=sys.stderr);return 2
if __name__=='__main__':raise SystemExit(main())
