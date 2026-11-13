"""Re-read delivered images, work and timings; separate from qualification runner."""
import argparse,json,hashlib,copy,statistics
import xml.etree.ElementTree as ET
from pathlib import Path
from run_m3parameter_gpu import read,write,sha,require,verify_inputs,validate_profile,rgb
from metal_depth_work_compare import same_work
from benchmark_m3parameter_4d import signature
ROOT=Path(__file__).resolve().parents[2]
BASE=Path('/Volumes/STORAGE/CodexArtifacts/mandelbulb-depth-gradient-qualification-20261025/run')
IDS={'a203b667b15f1e47','5f2747de7764cf5f'}
def frame(f,size,backend):
 p=read(f['profile']);w=validate_profile(p,backend,size);im,_=rgb(Path(f['profile']).parent/'image.png',size)
 require(w==f['work'] and im==f['image'],'Saved frame evidence differs')
 require(f['event']['exit_code']==0 and not f['event']['guard'],'Failed process')
 return im,w,p

def run(p,q,bench,extras,out):
 contract,_=verify_inputs(p);scenes={s['id']:s for s in contract['scenes']}
 for rel,h in read(p/'source-manifest.json')['source_files'].items():require(sha(ROOT/rel)==h,'Source changed '+rel)
 receipt=read(p/'build-receipt.json');qualified=read(extras/'qualified-build-receipt.json');before={x['path']:x['sha256'] for x in qualified['products']};after={x['path']:x['sha256'] for x in receipt['products']}
 require(set(before)==set(after),'Product inventory changed')
 changed=[k for k in before if before[k]!=after[k]]
 require(changed==['build/libmb3d_render_backend.a'],'Unexpected changed qualified code product')
 audit=read(extras/'archive-metadata-audit.json');require(audit['status']=='PASS_TIMESTAMP_ONLY_ARCHIVE_CHANGE','Missing archive metadata proof')
 archive=bytearray((p/changed[0]).read_bytes());require(sha(p/changed[0])==after[changed[0]]==audit['current_sha256'],'Changed archive after audit')
 require(len(audit['only_changed_fields'])==2,'Unexpected archive change count')
 for field in audit['only_changed_fields']:
  offset=field['offset'];require(bytes(archive[offset:offset+12]).decode()==field['current_date'],'Archive date differs');archive[offset:offset+12]=field['qualified_date'].encode()
 require(hashlib.sha256(archive).hexdigest()==before[changed[0]]==audit['qualified_sha256'],'Archive contains non-date changes')
 qual=read(q/'qualification.json');require(qual['status']=='PASS_SCOPED' and qual['build_receipt_sha256']==sha(extras/'qualified-build-receipt.json'),'Qualification build mismatch')
 require(qual['runner_sha256']==sha(ROOT/'comparison/tools/qualify_metal_depth_performance.py'),'Qualification runner changed')
 require(read(extras/'orbit-independence/result.json')['status']=='PASS','Orbit invariance failed')
 require('PASS 50400 checks' in (extras/'de-ao-tests-final/process.log').read_text(),'GPU guards incomplete')
 boot=read(extras/'bootstrap-final/bootstrap.json');ordinary=read(extras/'ordinary-verification.json')
 require(len(boot['tests'])==45 and len({t['name'] for t in boot['tests']})==45 and [t['name'] for t in boot['tests'] if t['failed']]==['renderer_tests'],'Unexpected original ordinary result')
 retry=read(extras/'renderer-retry/event.json');cases=ET.parse(extras/'renderer-retry/ctest.xml').getroot().findall('.//testcase')
 require(retry['exit_code']==0 and not retry['guard'] and retry['argv'][retry['argv'].index('--timeout')+1]=='180','Renderer retry failed or changed deadline')
 require(len(cases)==1 and cases[0].attrib['name']=='renderer_tests' and not any(cases[0].find(tag) is not None for tag in ['failure','error','skipped']),'Retry does not prove renderer PASS')
 require(ordinary['status']=='PASS_45_UNIQUE_SUITES_AFTER_TARGETED_RETRY','Ordinary aggregation failed')
 for path,h in ordinary['evidence'].items():require(sha(Path(path))==h,'Ordinary evidence changed')
 require(read(extras/'provenance-unit-tests/event.json')['exit_code']==0,'Build provenance tests failed')
 full=read(q/'full-scenes.json');require(len(full)==2 and {r['id'] for r in full}==IDS,'Wrong full scene set');old={r['id']:r for r in read(BASE/'full-scenes.json')}
 for row in full:
  s=scenes[row['id']];size=s['settings']['size'];require(size==row['size'] and len(row['frames'])==3,'Wrong full size/repeats')
  prior_image,prior_work,_=frame(old[row['id']]['frames'][0],size,'metal');first=None
  for f in row['frames']:
   im,w,pr=frame(f,size,'metal');require(pr['input']==str(p/s['frozen']),'Wrong full input');require(im==prior_image,'Old Metal RGB changed');same_work(w,prior_work,size)
   if first:require((im,w)==first,'Exact repeat failed')
   first=(im,w);e=w['execution'];require(e['ao_samples']>0 and e['ao_changed']>0 and e['shadow_steps']>0 and e['shadow_changed']>0,'Inactive effects')
  for path in row['effect_controls']:
   pr=read(path);validate_profile(pr,'metal',[64,48]);stage='ao' if 'ao-off' in path else 'shadows';require(pr['stages'][stage]['state']=='not_run','Bad effect control')
   im,_=rgb(Path(path).parent/'image.png',[64,48]);on,_=rgb(q/'effect-controls'/row['id']/'on/image.png',[64,48]);require(im!=on,'Effect inactive')
  frame(row['cpu'],size,'cpu');im,w,_=frame(row['gradient_off'],size,'metal');require(w==first[1] and im!=first[0],'Gradient control failed')
 for name,expected_count in [('regression.json',16),('previous-full.json',2)]:
  rows=read(q/name);olds={r['id']:r for r in read(BASE/name)};require(len(rows)==expected_count and {r['id'] for r in rows}==set(olds),'Wrong prior regression set')
  for row in rows:
   ref=olds[row['id']]['frames'][0];pr=read(ref['profile']);size=[pr['width'],pr['height']];im,w,_=frame(ref,size,'metal');require(len(row['frames'])==2,'Missing repeat')
   for f in row['frames']:
    a,b,_=frame(f,size,'metal');require(a==im and b==w,'Previous RGB/work changed')
 benchmark=read(bench/'result.json');require(benchmark['status']=='PASS' and benchmark['runner_sha256']==sha(ROOT/'comparison/tools/benchmark_metal_depth_scenes.py'),'Benchmark runner mismatch')
 require(len(benchmark['rows'])==2 and {r['id'] for r in benchmark['rows']}==IDS,'Wrong benchmark set')
 rows=[]
 for row in benchmark['rows']:
  sid=row['id'];s=scenes[sid];size=s['settings']['size'];require(row['size']==size,'Benchmark resolution changed');signatures={};times={}
  require(len(row['jobs'])==4,'Missing benchmark processes')
  for job in row['jobs']:
   report=read(job['report']);backend=job['backend'];dest=Path(job['report']).parent
   require(report['status']=='COMPLETE' and len(report['samples'])==3 and len(report['warmups'])==1,'Incomplete benchmark samples')
   sig=signature(report,dest,size,backend,s['settings']);require(sig==job['signature'],'Benchmark actual records differ')
   if backend in signatures:require(sig==signatures[backend],'Cross-process repeat failed')
   signatures[backend]=sig;times.setdefault(backend,[]).extend(f['end_to_end_ms'] for f in report['samples'])
   require(job['warm_ms']==[f['end_to_end_ms'] for f in report['samples']] and job['cold_ms']==report['cold']['cold_end_to_end_ms'],'Saved timings differ')
  require(signatures['metal']['image']==old[sid]['frames'][0]['image'],'Benchmark old image differs');same_work(signatures['metal']['work'],old[sid]['frames'][0]['work'],size)
  speed=statistics.median(times['cpu'])/statistics.median(times['metal']);require(speed==row['warm_speedup_vs_cpu'],'Speed calculation differs')
  rows.append({'id':sid,'warm_speedup_vs_cpu':speed,'cpu_warm_median_ms':statistics.median(times['cpu']),'metal_warm_median_ms':statistics.median(times['metal']),'exact_frames_per_backend':10})
 write(out,{'status':'PASS_DELIVERY_RECHECK','independent_reviewer':False,'scope':'Same-task separate artifact reread, not Pascal parity','ordinary_suites':45,'new_full_frames':6,'benchmark_frames':40,'preview_regression_frames':32,'previous_full_frames':4,'old_new_rgb_exact':True,'old_new_work_change':'ABox DE AO dispatches only','full77':'OPEN','rows':rows})
 print('DELIVERY RECHECK PASS',flush=True)
if __name__=='__main__':
 a=argparse.ArgumentParser();a.add_argument('--package',type=Path,required=True);a.add_argument('--qualification',type=Path,required=True);a.add_argument('--benchmark',type=Path,required=True);a.add_argument('--extras',type=Path,required=True);a.add_argument('--output',type=Path,required=True);v=a.parse_args();run(v.package.resolve(),v.qualification.resolve(),v.benchmark.resolve(),v.extras.resolve(),v.output.resolve())
