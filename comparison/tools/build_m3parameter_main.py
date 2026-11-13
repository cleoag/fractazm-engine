#!/usr/bin/env python3
"""Fresh main-checkout phase-one build with actual compiler/link/AIR provenance."""
from pathlib import Path
import argparse,hashlib,json,os,shlex,shutil,signal,subprocess,sys,time
ROOT=Path(__file__).resolve().parents[2]
EXPECTED_AIR_PRODUCTS=('metal_native_scene.air','metal_native_general.air',
 'metal_native_dof.air','metal_native_zbuffer.air','metal_native_ao_screen.air',
 'metal_native_ao_de.air','navigation_preview.air')
def sha(p):return hashlib.sha256(Path(p).read_bytes()).hexdigest()
def read(p):return json.loads(Path(p).read_text())
def write(p,x):p.parent.mkdir(parents=True,exist_ok=True);p.write_text(json.dumps(x,indent=2)+'\n')
def need(x,s):
 if not x:raise RuntimeError(s)
def env():
 bad={'CC','CXX','OBJCXX','CPPFLAGS','CFLAGS','CXXFLAGS','LDFLAGS','CPATH','C_INCLUDE_PATH','CPLUS_INCLUDE_PATH','OBJC_INCLUDE_PATH','LIBRARY_PATH','SDKROOT','MACOSX_DEPLOYMENT_TARGET'}
 return {k:v for k,v in os.environ.items() if k.upper() not in bad and not k.upper().startswith(('MB3D_','DYLD_','CMAKE_','CCACHE','SCCACHE'))}
def run(argv,out,environment,timeout):
 start=time.monotonic();free_min=shutil.disk_usage(out).free
 with (out/'process.log').open('wb') as stream:
  p=subprocess.Popen(list(map(str,argv)),cwd=ROOT,env=environment,stdout=stream,stderr=subprocess.STDOUT,start_new_session=True)
  reason=None
  try:
   while p.poll() is None:
    free_min=min(free_min,shutil.disk_usage(out).free)
    if free_min<12*1024**3:reason='DISK_RESERVE'
    if time.monotonic()-start>timeout:reason='TIMEOUT'
    if (out/'process.log').stat().st_size>64*1024**2:reason='LOG_LIMIT'
    if reason:break
    try:p.wait(timeout=.5)
    except subprocess.TimeoutExpired:pass
  finally:
   if p.poll() is None:
    os.killpg(p.pid,signal.SIGTERM)
    try:p.wait(timeout=5)
    except subprocess.TimeoutExpired:os.killpg(p.pid,signal.SIGKILL);p.wait(timeout=5)
  result={'argv':list(map(str,argv)),'exit_code':p.returncode,'guard':reason,'seconds':time.monotonic()-start,'minimum_free_bytes':free_min}
  write(out/'event.json',result)
  need(p.returncode==0 and not reason,'Command failed '+str(out))
 return result

def validate(out,targets):
 source=ROOT;build=out/'build';recorddir=out/'provenance/records';sm=read(out/'source-manifest.json')
 for rel,h in sm['source_files'].items():need(sha(source/rel)==h,'Source changed '+rel)
 records=[(p,read(p)) for p in sorted(recorddir.glob('*.json')) if not p.name.endswith('.started.json')]
 need(len(records)==len(list(recorddir.glob('*.started.json'))),'Interrupted compiler command')
 latest={};edges=0;objects=0;airs=0;deps=0;seen={}
 for path,r in records:
  need(r['exit_code']==0 and not r['capture_errors'],'Failed capture '+str(path))
  for item in r.get('explicit_inputs_before',[]):
   p=Path(item['path'])
   if p.is_relative_to(build) and p.suffix in ['.o','.a','.air','.metallib']:
    producers=[v for _,v in records if v.get('output',{}).get('path')==str(p) and v['end_time_ns']<=r['start_time_ns']]
    need(producers,'Missing compile/archive producer '+str(p));producer=max(producers,key=lambda x:x['end_time_ns'])
    need(producer['output']['sha256']==item['sha256'],'Producer hash mismatch');edges+=1
   else:need(sha(p)==item['sha256'],'Changed link input '+str(p))
   # Build inputs are versioned by their actual preceding producer. A later
   # archive refresh may change ar header dates without changing its objects;
   # compare current bytes only to the latest producer in the product loop.
  for d in r.get('dependencies',[]):
   p=Path(d['path'])
   if str(p) not in seen:seen[str(p)]=sha(p)
   need(seen[str(p)]==d['sha256'],'Dependency changed '+str(p));deps+=1
   if p.is_relative_to(source):need(sm['source_files'].get(str(p.relative_to(source)))==d['sha256'],'Unbound source '+str(p))
   elif d['classification'] in ['build_generated','downloaded_dependency_source']:raise RuntimeError('Unproven generated dependency '+str(p))
  product=r.get('output')
  if product:
   suffix=Path(product['path']).suffix
   if suffix in ['.o','.air']:
    need(r.get('dependencies') and r.get('depfile'),'Missing dependency closure');objects+=suffix=='.o';airs+=suffix=='.air'
   old=latest.get(product['path'])
   if not old or r['end_time_ns']>old['end_time_ns']:latest[product['path']]=r
 products=[]
 for p in build.rglob('*'):
  if p.is_file() and (p.suffix in ['.o','.a','.air','.metallib'] or p.name in targets):
   r=latest.get(str(p));need(r is not None,'Unproven output '+str(p));need(sha(p)==r['output']['sha256'],'Output identity mismatch')
   products.append({'path':str(p.relative_to(out)),'sha256':sha(p)})
 for name in targets+['metal_native_scene.metallib']:need((build/name).is_file(),'Missing target '+name)
 actual_airs={Path(p).name for p in latest if Path(p).suffix=='.air'}
 need(airs==len(EXPECTED_AIR_PRODUCTS) and actual_airs==set(EXPECTED_AIR_PRODUCTS),
      'Expected exact seven offline AIR producers including navigation preview')
 receipt={'schema':'mb3d.phase1.build.v1','status':'PASS_CAPTURED_BUILD_CLOSURE','source_manifest_sha256':sha(out/'source-manifest.json'),'contract_sha256':sha(out/'contract.json'),'capture_sha256':sha(out/'provenance/capture.py'),'commands':len(records),'objects':objects,'airs':airs,'dependencies':deps,'producer_edges':edges,'products':products,'targets':targets,'records':[{'path':str(p.relative_to(out)),'sha256':sha(p)} for p,_ in records],'compiled_source_root':str(ROOT),'scope':'Fresh main checkout compile including ordinary targets; no inherited PASS; main-source and captured dependency closure checked'}
 write(out/'build-receipt.json',receipt);print(json.dumps({k:receipt[k] for k in ['status','commands','objects','airs','dependencies','producer_edges']}),flush=True)

def main():
 parser=argparse.ArgumentParser();parser.add_argument('--output',type=Path,required=True);parser.add_argument('--extra-target',action='append',default=[]);a=parser.parse_args();out=a.output.resolve()
 need(not (out/'build').exists(),'Build must be fresh');need(shutil.disk_usage(out).free>=(12*1024**3+384*1024**2),'Need 12 GiB reserve plus 384 MiB build headroom')
 targets=['mb3d_render','mb3d_backend_render_benchmark','mb3d_scene_loader_probe','mb3d_native_formula_probe','mb3d_phase1_admission','mb3d_tests_render_backend','mb3d_tests_native_smoothing_readback','mb3d_tests_native_general_gpu','mb3d_tests_native_dof_render','mb3d_tests_native_zbuffer_render','mb3d_tests_native_general_normals_gpu','mb3d_tests_native_smoothing_render','mb3d_tests_native_logical_lanes_gpu','mb3d_tests_native_zbuffer_gpu']
 for target in a.extra_target:
  need(target in ['mb3d_tests_phase1_color','mb3d_tests_native_dof_gpu','mb3d_tests_native_ao_seed_gpu','mb3d_tests_native_pipeline_cache'],'Unapproved extra target')
  if target not in targets:targets.append(target)
 sm=read(out/'source-manifest.json')
 for rel,h in sm['source_files'].items():need(sha(ROOT/rel)==h,'Source changed '+rel)
 capture=out/'provenance/capture.py';need(sha(capture)==sm['capture_sha256'],'Capture changed')
 instrument=out/'provenance/instrument.cmake';prefix=shlex.join([sys.executable,str(capture)])
 instrument.write_text('set_property(GLOBAL PROPERTY RULE_LAUNCH_CUSTOM [==['+prefix+' --kind custom --]==])\nset_property(GLOBAL PROPERTY RULE_LAUNCH_LINK [==['+prefix+' --kind link --]==])\n')
 context=out/'provenance/context.json';write(context,{'root':str(ROOT),'build':str(out/'build'),'records':str(out/'provenance/records'),'phase':'phase1-clean-unified-build'})
 environment=env();environment['MB3D_CLEAN_CAPTURE_CONTEXT']=str(context)
 launcher=';'.join([sys.executable,str(capture),'--kind','compile','--'])
 configure=['cmake','-S',ROOT,'-B',out/'build','-G','Unix Makefiles','-DCMAKE_BUILD_TYPE=Release','-DCMAKE_OSX_ARCHITECTURES=arm64','-DCMAKE_INTERPROCEDURAL_OPTIMIZATION=OFF','-DCMAKE_EXPORT_COMPILE_COMMANDS=ON','-DMB3D_DEFAULT_PRECISION=compatible','-DMB3D_NATIVE_TESTS_ONLY=OFF','-DMB3D_TEST_TIMEOUT_SCALE=1','-DCMAKE_C_FLAGS=','-DCMAKE_CXX_FLAGS=','-DMB3D_ENABLE_PARITY_TRACE=OFF','-DMB3D_ENABLE_METAL_RENDERER=ON','-DMB3D_BUILD_METAL_EXACT_SCENE=OFF','-DMB3D_BUILD_METAL_PROBE=OFF','-DMB3D_BUILD_METAL_FORMULA_PROBE=OFF','-DCMAKE_PROJECT_TOP_LEVEL_INCLUDES='+str(instrument)]
 for language in ['C','CXX','OBJCXX']:configure+=['-DCMAKE_'+language+'_COMPILER_LAUNCHER='+launcher]
 assertion=subprocess.Popen(['caffeinate','-is','-w',str(os.getpid())])
 try:
  for i,command in enumerate([configure,['cmake','--build',out/'build','--parallel','4','--verbose','--target',*targets,'mb3d_ordinary']]):
   log=out/'build-events'/str(i);log.mkdir(parents=True);r=run(command,log,environment,1200);print(json.dumps(r),flush=True)
  validate(out,targets)
 finally:assertion.terminate();assertion.wait(timeout=5)
if __name__=='__main__':main()
