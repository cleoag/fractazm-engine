#!/usr/bin/env python3
"""Prove ordered normal state flow with synthetic or real internal-formula children.

Binds the pinned P1 normal caller, seven schedules, three repeats, poisoned
inactive storage and unsupported-smoothing admission. No renderer is enabled.
"""
import argparse,hashlib,json,shlex,shutil,subprocess,sys
from pathlib import Path
from gen_native_normal_continuations import generate_normal
from gen_native_orbit_continuations import function
from gen_native_metal_formulas import emit_text,tokens
from gen_native_continuations import locate
from check_native_normal_composition_trace import validate_directory
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT))
from comparison.tools.run_m3parameter_gpu import invoke

def sha(path):return hashlib.sha256(Path(path).read_bytes()).hexdigest()
def main():
 ap=argparse.ArgumentParser(description=__doc__);ap.add_argument('--output',type=Path,required=True);ap.add_argument('--baseline-library',type=Path);ap.add_argument('--smoothing-policy',type=int,choices=[0,1],default=0);ap.add_argument('--driver',type=int,choices=range(8),default=0);ap.add_argument('--legacy-zero-gradient',action='store_true',help='retain the old direct-success/DE-refusal fixture explicitly');ap.add_argument('--outline-observer',action='store_true',help='outline only the copy-only original sample snapshots');ap.add_argument('--real',action='store_true',help='real internal-formula composition in a separately frozen control library');a=ap.parse_args();out=a.output.resolve()
 if (a.smoothing_policy or a.driver or a.outline_observer or a.legacy_zero_gradient) and not a.real:ap.error('Smoothing policy/driver/observer options require --real')
 if out.exists():ap.error('New output required')
 if shutil.disk_usage(ROOT).free<12*1024**3:ap.error('12 GiB reserve unavailable')
 out.mkdir(parents=True);(out/'data').mkdir();stage=out/'engine'
 receipt={'scope':('real internal-formula normal composition; no renderer acceptance' if a.real else 'synthetic child ordering/packing; no fractal or renderer acceptance'),'source_commit':subprocess.check_output(['git','rev-parse','HEAD'],cwd=ROOT,text=True).strip(),'commands':[],'dependencies':{},'status':'BUILDING','pipeline_cache':'no explicit archive; driver-cache state unknown'}
 def save():(out/'receipt.json').write_text(json.dumps(receipt,indent=2)+'\n')
 def run(cmd):
  row={'argv':list(map(str,cmd))};receipt['commands'].append(row);save();p=subprocess.run(row['argv'],cwd=ROOT,capture_output=True,text=True,timeout=120);row['exit_code']=p.returncode
  log=out/f'command-{len(receipt["commands"])}.log';log.write_text(p.stdout+p.stderr);row['log_sha256']=sha(log);save()
  if p.returncode:raise RuntimeError(p.stderr[-1500:])
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
  if a.real:overlays+=['continuation_step.metal.h','continuation_orbit_step.metal.h','continuation_replay_step.metal.h','continuation_distance_step.metal.h','continuation_combined_step.metal.h','continuation_entry_step.metal.h','continuation_normal_sample_step.metal.h','continuation_normal_step.metal.h']
  for name in overlays:
   source=ROOT/'src/render/native_metal'/name;shutil.copy2(source,stage/'src/render/native_metal'/name);receipt['dependencies'][str(source)]=sha(source)
  source=(stage/'src/render/native_metal/general_normals.metal.h').read_text()
  (out/'normal_regions.metal.h').write_text(generate_normal(source))
  body=emit_text(function(source,'general_calculate_normals')).replace('general_normal_sample','scripted_sample')
  (out/'normal_control.metal.h').write_text('inline NativeNormalResult normal_scripted_control(float3 center,float depth,float stop,constant ExactScene& scene,constant NativeFormulaProgram& program,NativeNormalParams params,thread NativeResources& resources,thread NativeFormulaState& lane,thread NativeGeometryRecord& geometry,thread NativeNormalRecord& record,bool reflection_pack=false){'+body+'}\n')
  shader=ROOT/('tests/metal/p2_normal_composition.metal' if a.real else 'tests/metal/p2_ordered_normal_continuation.metal');host=ROOT/('tests/test_native_normal_composition.mm' if a.real else 'tests/test_native_ordered_normal_continuations.mm')
  if a.real:
   v=function(source,'general_calculate_normals')
   sites=[i for i,t in enumerate(v) if t=='general_normal_sample']
   from gen_native_metal_formulas import matching
   for i in reversed(sites):
    end=matching(v,i+1,'(',')');v[i]='normal_observed_sample';v[end:end]=tokens(',observer')
   signature='float3 center,float depth,float stop,constant ExactScene& scene,constant NativeFormulaProgram& program,NativeNormalParams params,thread NativeResources& resources,thread NativeFormulaState& lane,thread NativeGeometryRecord& geometry,thread NativeNormalRecord& record,thread Observer& observer,bool reflection_pack=false'
   helper='template<class Observer> inline GeneralNormalSample normal_observed_sample(float3 point,float stop,bool direct,bool smooth,constant NativeFormulaProgram& p,thread NativeResources& resources,thread NativeFormulaState& lane,thread NativeGeometryRecord& g,thread NativeNormalRecord& n,thread Observer& observer){observer.record(1,point,stop,direct,smooth,lane,g,n,GeneralNormalSample{});const auto result=general_normal_sample(point,stop,direct,smooth,p,resources,lane,g,n);observer.record(2,point,stop,direct,smooth,lane,g,n,result);return result;}\n'
   (out/'normal_outer_observer.metal.h').write_text(helper+'template<class Observer> inline NativeNormalResult normal_outer_observed('+signature+'){'+emit_text(v)+'}\n')
   # Test-only snapshots: source expressions/child dispatch remain unchanged.
   code=(out/'normal_regions.metal.h').read_text()
   code=code.replace('inline void normal_sample_child(thread NormalSampleFrame& s,','template<class Observer> inline void normal_sample_child(thread NormalSampleFrame& s,thread NativeFormulaState& lane,thread NativeGeometryRecord& geometry,thread NativeNormalRecord& record,thread Observer& observer,')
   code=code.replace('s.want_smooth=uint(smooth);','s.want_smooth=uint(smooth);observer.record(1,point,stop,direct,smooth,lane,geometry,record,GeneralNormalSample{});')
   code=code.replace('inline uint normal_advance(', 'template<class Observer> inline uint normal_advance(')
   code=code.replace('thread NativeNormalRecord& record){','thread NativeNormalRecord& record,thread Observer& observer){')
   code=code.replace('normal_sample_child(s,','normal_sample_child(s,lane,geometry,record,observer,')
   code=code.replace('if(child!=99)return n.header.pc;','if(child!=99)return n.header.pc;observer.record(2,float3(s.point[0],s.point[1],s.point[2]),s.stop,bool(s.direct),bool(s.want_smooth),lane,geometry,record,GeneralNormalSample{s.value,s.smooth});')
   (out/'normal_regions.metal.h').write_text(code)
  if a.real:
   for script in ['gen_native_caller_continuations.py','gen_native_entry_continuations.py','gen_native_normal_sample_continuations.py']:run([sys.executable,ROOT/'scripts'/script,'--source-root',stage,'--output',out])
   run([sys.executable,ROOT/'scripts/gen_native_continuations.py','--output',out])
   run(['xcrun','-sdk','macosx','metal','-v','-std=metal3.1','-fno-fast-math','-ffp-contract=fast','-DP2_REFERENCE=1','-MMD','-MF',out/'reference.d','-I',stage,'-I',out,'-c',shader,'-o',out/'reference.air'])
   run(['xcrun','-sdk','macosx','metallib',out/'reference.air','-o',out/'reference.metallib'])
  run(['xcrun','-sdk','macosx','metal','-v','-std=metal3.1','-fno-fast-math','-ffp-contract=fast',*(['-DP2_OUTLINE_OBSERVER=1'] if a.outline_observer else []),'-MMD','-MF',out/'probe.d','-I',stage,'-I',out,'-c',shader,'-o',out/'probe.air'])
  run(['xcrun','-sdk','macosx','metallib',out/'probe.air','-o',out/'probe.metallib'])
  run(['xcrun','clang++','-v','-std=c++20','-O2','-fobjc-arc','-Wall','-Wextra','-Werror','-MMD','-MF',out/'host.d','-I',stage,host,'-framework','Foundation','-framework','Metal','-o',out/'probe'])
  for dep in [out/'probe.d',out/'host.d']+([out/'reference.d'] if a.real else []):
   for path in shlex.split(dep.read_text().replace('\\\n',' ').split(': ',1)[1]):
    p=Path(path);p=p if p.is_absolute() else ROOT/p;receipt['dependencies'][str(p)]=sha(p)
  for p in [Path(__file__),ROOT/'scripts/gen_native_normal_continuations.py',ROOT/'scripts/gen_native_normal_sample_continuations.py',ROOT/'scripts/gen_native_orbit_continuations.py',ROOT/'scripts/gen_native_metal_formulas.py',ROOT/'scripts/gen_native_continuations.py',audit,p1]:receipt['dependencies'][str(p.resolve())]=sha(p)
  if a.real:
   for name in ['gen_native_caller_continuations.py','gen_native_entry_continuations.py','gen_native_replay_continuations.py','gen_native_distance_continuations.py','gen_native_combined_continuations.py']:
    p=ROOT/'scripts'/name;receipt['dependencies'][str(p)]=sha(p)
   p=ROOT/'scripts/check_native_normal_composition_trace.py';receipt['dependencies'][str(p)]=sha(p)
  for path in receipt['dependencies']:
   p=Path(path)
   if p.is_relative_to(ROOT):dest=out/'source'/p.relative_to(ROOT);dest.parent.mkdir(parents=True,exist_ok=True);shutil.copy2(p,dest)
  receipt['products']={name:sha(out/name) for name in (['probe','probe.air','probe.metallib']+(['reference.air','reference.metallib'] if a.real else []))};save()
  reference=a.baseline_library.resolve() if a.baseline_library else out/'reference.metallib'
  receipt['smoothing_policy']=a.smoothing_policy if a.real else None;receipt['driver']=a.driver if a.real else None;receipt['outline_observer']=a.outline_observer;receipt['nonzero_gradient']=not a.legacy_zero_gradient if a.real else None
  if a.real:receipt['frozen_control']={'path':str(reference),'sha256':sha(reference)};save()
  receipt['execution']=invoke([str(out/'probe'),str(out/'probe.metallib'),str(out/'data')]+([str(reference),str(a.smoothing_policy),str(a.driver),str(int(not a.legacy_zero_gradient))] if a.real else []),out/'run',ROOT,timeout=150,exclusive_gpu=True)
  for path,digest in receipt['dependencies'].items():
   if sha(path)!=digest:raise RuntimeError('Source changed during proof: '+path)
  if a.real and sha(reference)!=receipt['frozen_control']['sha256']:raise RuntimeError('Frozen control changed')
  receipt['observations']={p.name:{'bytes':p.stat().st_size,'sha256':sha(p)} for p in (out/'data').glob('*.bin')}
  receipt['status']='PASS' if receipt['execution']['exit_code']==0 else 'FAIL'
  if a.real and receipt['status']=='PASS':receipt['coverage']=validate_directory(out/'data')
  save();print(receipt['status']);return int(receipt['status']!='PASS')
 except Exception as e:receipt['status']='ERROR';receipt['error']=str(e);save();print(str(e),file=sys.stderr);return 2
if __name__=='__main__':raise SystemExit(main())
