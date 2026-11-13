"""Scoped full Makin acceptance: preserved effects/RGB, exact repeats and fresh CPU timing."""
import sys,statistics,copy,subprocess
from pathlib import Path
sys.path.insert(0,str(Path.cwd()/'comparison/tools'))
from run_m3parameter_gpu import *
from benchmark_m3parameter_4d import signature
import argparse
parser=argparse.ArgumentParser();parser.add_argument('--candidate',type=Path,required=True);parser.add_argument('--output',type=Path,required=True);args=parser.parse_args()
r=args.output.resolve();require(not r.exists(),'Fresh output required');r.mkdir(parents=True)
p=args.candidate.resolve();base=Path('comparison/runs/m3parameter-makin-effects-20261024/candidate04').resolve();repo=Path.cwd();identity=sha(__file__)
c,_=verify_inputs(p);verify_inputs(base);
for rel,h in read(p/'source-manifest.json')['source_files'].items():require(sha(repo/rel)==h,'Main source changed: '+rel)
scenes={s['id']:s for s in c['scenes']};s=scenes['0f258a2abc1002ec'];size=s['settings']['size'];rows=[]
for name,argv in [('ao',[p/'build/mb3d_tests_native_ao_seed_gpu',p/'build/metal_native_scene.metallib']),('dof',[p/'build/mb3d_tests_native_dof_gpu',p/'build/metal_native_scene.metallib']),('backend',[p/'build/mb3d_tests_render_backend',repo/'comparison/runs/m3parameter-package0-20261024/inputs/regression','--metal-library',p/'build/metal_native_scene.metallib']),('smoothing',[p/'build/mb3d_tests_native_smoothing_readback']),('color',[p/'build/mb3d_tests_phase1_color',p/'build/metal_native_scene.metallib',p/'inputs/scenes/Melting spot bloxx.m3p',p/'runtime'])]:
 e=invoke(argv,r/('checks-'+name),p/'runtime',300);print('CHECK',name,e['exit_code'],flush=True);require(e['exit_code']==0 and not e['guard'],'test failed '+name)
old_path=repo/'comparison/runs/m3parameter-makin-effects-20261024/benchmark/metal-0/frames/report.json';old=read(old_path);old_sig=signature(old,old_path.parent,size,'metal',s['settings'])
changed_work=set()
def semantic(w):
 v=copy.deepcopy(w)
 for k in changed_work:v['execution'].pop(k,None)
 return v
# Interleave fresh Metal/CPU processes; CPU is measured on this final binary.
sigs={}
for backend,process in [('metal',0),('cpu',0),('metal',1)]:
 folder=r/'benchmark'/f'{backend}-{process}';dest=folder/'frames';argv=[p/'build/mb3d_backend_render_benchmark','--scene',p/s['frozen'],'--output',dest,'--resource-root',p/'runtime','--backend',backend,'--precision','native','--width',size[0],'--height',size[1],'--threads',6,'--warmups',1,'--repeats',3]
 if backend=='metal':argv+=['--metal-library',p/'build/metal_native_scene.metallib']
 e=invoke(argv,folder,p/'runtime',600);require(e['exit_code']==0 and not e['guard'],'benchmark failed');raw=read(dest/'report.json');require(raw['status']=='COMPLETE','incomplete benchmark');sig=signature(raw,dest,size,backend,s['settings']);require(backend not in sigs or sigs[backend]==sig,'cross-process repeat mismatch');sigs[backend]=sig
 if backend=='metal':
  require(sig['image']==old_sig['image'],'AO/DOF optimization changed RGB');require(semantic(sig['work'])==semantic(old_sig['work']),'Semantic work changed')
  ex=sig['work']['execution'];require(ex['ao_seed_planning_draws']==ex['ao_rng_advances'] and ex['ao_seed_shortcuts']>0,'Inactive seed plan');require(ex['dof_candidate_scan_checks']==old_sig['work']['execution']['dof_candidate_scan_checks'],'DOF scan count changed during promotion')
 j={'backend':backend,'process':process,'event':e,'report':str(dest/'report.json'),'signature':sig,'cold_ms':raw['cold']['cold_end_to_end_ms'],'warm_ms':statistics.median(f['end_to_end_ms'] for f in raw['samples']),'build_receipt_sha256':sha(p/'build-receipt.json')};rows.append(j);write(r/'benchmark/partial.json',rows);print('BENCHMARK',backend,process,j['cold_ms'],j['warm_ms'],flush=True)
cpu=next(j['warm_ms'] for j in rows if j['backend']=='cpu');metal=statistics.median(f['end_to_end_ms'] for j in rows if j['backend']=='metal' for f in read(Path(j['report']))['samples']);require(metal<cpu,'Metal is not faster than fresh Native CPU')
write(r/'benchmark/result.json',{'status':'PASS','candidate':str(p),'candidate_build_sha256':sha(p/'build-receipt.json'),'baseline':str(base),'baseline_build_sha256':sha(base/'build-receipt.json'),'baseline_report':str(old_path),'baseline_report_sha256':sha(old_path),'runner_sha256':identity,'allowed_old_new_work_changes':sorted(changed_work),'fresh_cpu_warm_ms':cpu,'fresh_metal_warm_ms':metal,'cpu_over_metal':cpu/metal,'rows':rows})
old_reg=read(repo/'comparison/runs/m3parameter-makin-effects-20261024/regression/result.json');reg=[]
for v in old_reg['rows']:
 scene=scenes[v['id']];ref=v['frames'][0];profile=read(Path(ref['profile']));frames=[]
 for i in range(3):
  f=render_one(p,scene,'metal',r/'regression'/v['id']/str(i),[profile['width'],profile['height']],180);require(f['status']=='PREVIEW_COMPLETE','Regression failed');frames.append(f)
  require(f['image']==frames[0]['image'] and f['work']==frames[0]['work'],'Regression repeat mismatch')
  require(f['image']==ref['image'] and semantic(f['work'])==semantic(ref['work']),'Regression changed semantic work or RGB')
 reg.append({'id':v['id'],'scene':v['scene'],'frames':frames});write(r/'regression/partial.json',reg);print('REGRESSION',len(reg),v['scene'],flush=True)
write(r/'regression/result.json',{'status':'PASS','rows':reg,'baseline':'Makin candidate04: exact RGB and ALL non-timing work fields'})
e=invoke([sys.executable,'-B',repo/'comparison/tools/run_m3parameter_gpu.py','--package',p,'--output',r/'admission','--mode','admission'],r/'admission-driver',repo,180);require(e['exit_code']==1 and not e['guard'],'admission failed');a=read(r/'admission/result.json');require(a['counts']=={'PREPARATION_ACCEPTED':23,'PREPARATION_REJECTED':54},'Admission changed')
verify_inputs(p)
for rel,h in read(p/'source-manifest.json')['source_files'].items():require(sha(repo/rel)==h,'Main source changed: '+rel)
require(sha(__file__)==identity,'Runner changed');write(r/'qualification.json',{'status':'PASS_MAIN_METAL_PROMOTION_SCOPED','candidate_build_sha256':sha(p/'build-receipt.json'),'runner_sha256':identity,'benchmark_frames':15,'fresh_metal_frames':10,'fresh_cpu_frames':5,'regression_frames':48,'cpu_over_metal':cpu/metal,'old_new_rgb':'EXACT','effect_semantic_work':'EXACT','full77_ledger':'OPEN'});print('QUALIFICATION PASS',cpu/metal,flush=True)
