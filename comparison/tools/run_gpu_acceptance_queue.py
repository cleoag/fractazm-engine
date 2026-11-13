#!/usr/bin/env python3
"""Serial guarded original-scene queue on a frozen build; incremental raw evidence.

Preserve authored jitter/effects, three independent full GPU frames and each
active effect's same-engine off control. Keep CPU differences and unresolved
numerical reviews visible. Never promote a preview, missing map or timeout.
"""
import argparse,json,statistics
from pathlib import Path
from qualify_native_metal_all import ROOT,require,sha,read_json
from qualify_gpu_parallel_pipeline import scope,evaluate
from gpu_pipeline_binding import core_context,for_scene
from gpu_pipeline_frames import frame,scene_frames,enabled,FLAGS
from gpu_pipeline_fp32 import frame as cpu_frame
from run_m3parameter_gpu import invoke,validate_profile
from native_gpu_coverage import image_difference
from native_numeric_review import pending as numeric_pending,approved as numeric_approval
from prepare_gpu_parallel_bundle import refresh,ref,write
from report_gpu_acceptance import publish
from native_cpu_reference import saved_reference, previous_slow_reference


def publish_safely(package, report_output=None, write_plan=True):
    """A display failure must never relabel a validated renderer outcome."""
    if not (package/'qualification.json').exists():return
    try:publish(package,output=report_output,write_plan=write_plan)
    except (ValueError,KeyError,TypeError,OSError,IndexError) as error:
        print('REPORT UPDATE FAILED:',str(error),flush=True)


def main():
    p=argparse.ArgumentParser(description=__doc__)
    for k in ('package','contract','build'):p.add_argument('--'+k,type=Path,required=True)
    p.add_argument('--ranks',help='Comma-separated queue ranks; otherwise all non-deferred cases')
    p.add_argument('--reference-package',type=Path,help='Reuse only validated native CPU comparisons with identical scene/resources/workers')
    p.add_argument('--report-output',type=Path,help='Separate working report for an unpromoted candidate')
    p.add_argument('--no-plan',action='store_true',help='Keep the incumbent plan while checking a candidate')
    p.add_argument('--resume',action='store_true');p.add_argument('--retry',action='store_true');a=p.parse_args()
    require(not a.retry or a.resume,'Retry requires revalidation of existing artifacts with --resume')
    package=a.package.resolve();contract=a.contract.resolve();build=a.build.resolve()
    def publish_current():publish_safely(package,a.report_output,not a.no_plan)
    reference_package=a.reference_package.resolve() if a.reference_package else None
    if reference_package:require(reference_package.is_relative_to(ROOT) and reference_package!=package,'CPU reference package must be a separate repository archive')
    c,formulas,scenes=scope(contract);queue=read_json(ROOT/'comparison/scene-queue.json')['rows']
    ranks=set(map(int,a.ranks.split(','))) if a.ranks else {r['rank'] for r in queue if r['priority']!='deferred_slow_tail'}
    ev=refresh(package,contract,build);accepted={r['id'] for r in ev['scenes']}
    require(ranks<={r['rank'] for r in queue},'Unknown rank')
    for q in queue:
        if q['rank'] not in ranks or q['id'] in accepted:continue
        sid=q['id'];scene=scenes[sid];out=package/'scene-runs'/sid;out.mkdir(parents=True,exist_ok=True)
        if (out/'case.json').exists():
            previous=read_json(out/'case.json')
            if previous['status'] not in ('RUNNING',) and not a.retry:
                print('SKIP inspected case',q['rank'],previous['status'],flush=True);continue
        if (out/'case.json').exists():
            old=read_json(out/'case.json');history=out/'case-history'/(sha(out/'case.json')+'.json')
            if not history.exists():write(history,old)
        result={'id':sid,'rank':q['rank'],'scene':scene['path'],'status':'RUNNING','stages':[]}
        def save():
            write(out/'case.json',result);publish_current()
        def job(label,backend,size,off=None):
            result['active_job']={'label':label,'backend':backend,'size':size};save()
            folder=out/label;argv=[build/'mb3d_render',ctx['inputs'][sid],'--backend',backend,'--precision','native',
                '--width',size[0],'--height',size[1],'--threads',ctx['threads'],'--output',folder/'image.png','--profile-json',folder/'profile.json']
            if backend=='metal':argv+=['--metal-library',ctx['library_path']]
            if off:argv+=[FLAGS[off]]
            if (folder/'event.json').exists():
                require(a.resume,'Existing frame needs --resume');e=read_json(folder/'event.json')
                require(e['argv']==list(map(str,argv)) and Path(e['cwd']).resolve()==ctx['runtime'],'Changed frame command')
                require(e['executable_sha256_before']==e['executable_sha256_after']==ctx['renderer']['sha256'],'Changed host')
                require(all(sha(ROOT/r['path'])==r['sha256'] for r in e['inputs']+e['outputs']),'Changed saved frame artifact')
            else:
                require(not folder.exists(),'Inspect partial frame before retry')
                print('START',q['rank'],label,size,flush=True)
                e=invoke(argv,folder,ctx['runtime'],timeout=300,evidence_root=ROOT)
            result['active_job']=None
            profile=read_json(folder/'profile.json') if (folder/'profile.json').exists() else {}
            result['stages'].append({'label':label,'seconds':e['seconds'],'exit_code':e['exit_code'],'guard':e['guard'],'profile_status':profile.get('status'),'error':profile.get('error')});save()
            require(e['exit_code']==0 and e['guard'] is None,'Frame failure/guard: '+str(e['guard'])+' '+str(profile.get('error','')))
            if label!='reference-cpu':require(not e.get('frame_warning_emitted',False) and e['seconds']<=180,'Slow frame: investigate before any further full repeats')
            row={k:ref(folder/v) for k,v in (('event','event.json'),('profile','profile.json'),('image','image.png'))}
            print('DONE',q['rank'],label,round(e['seconds'],3),flush=True);return row,read_json(folder/'profile.json')
        try:
            ev=refresh(package,contract,build);ctx=for_scene(core_context(ROOT,ev['binding'],c,formulas,scenes,ROOT),sid)
            result.update(resource_profile=ctx['resource_profile'],selected_map_ids=ctx['selected_map_ids'],runtime_files=ctx['runtime_files']);save()
            size=scene['settings']['size'];small=[min(160,size[0]),max(1,round(size[1]*min(160,size[0])/size[0]))]
            preview,pp=job('preview-metal','metal',small)
            if pp['backend']!='metal':
                result['status']='OPEN_FP32_PROOF' if pp.get('backend_fallback_code')=='fp32_range' else 'OPEN_BACKEND'
                result['backend_reason']=pp.get('backend_fallback_reason');save();continue
            validate_profile(pp,'metal',small)
            reused=saved_reference(ROOT,reference_package,scene,ctx) if reference_package else None
            reference=None
            if reused:
                reference,origin=reused;result['cpu_reference_origin']=origin
                cp_profile=read_json(ROOT/reference['profile']['path'])
                result['cpu_reference_status']='REUSED_COMPLETE'
                print('REUSED native CPU reference',q['rank'],flush=True)
            elif previous_slow_reference(ROOT,reference_package,scene):
                result['cpu_reference_status']='DEFERRED_BY_PREVIOUS_SLOW_FRAME'
                result['previous_slow_cpu']=previous_slow_reference(ROOT,reference_package,scene)
                save()
            else:
                cp,cpp=job('preview-cpu','cpu',small);validate_profile(cpp,'cpu',small)
                estimate=cpp['render_total_seconds']*size[0]*size[1]/(small[0]*small[1]);result['cpu_linear_estimate_seconds']=estimate
                if estimate>180:
                    result['cpu_reference_status']='DEFERRED_BY_SMALL_DIAGNOSTIC'
                else:
                    reference,cp_profile=job('reference-cpu','cpu',size)
                    cpu_frame(ROOT,reference,scene,ctx,fallback=False,reference_only=True)
                    result['cpu_reference_status']='COMPLETE'
            if reference:
                if read_json(ROOT/reference['event']['path'])['seconds']>180:
                    result['cpu_reference_status']='COMPLETE_SLOW_REFERENCE_NOT_REPEATED'
                    dominant=max(cp_profile['stages_seconds'],key=cp_profile['stages_seconds'].get)
                    diagnostic={'profile':reference['profile'],'dominant_stage':dominant,'stage_seconds':cp_profile['stages_seconds'][dominant],
                                'cpu_frame_not_repeated':True,'qualification_credit':False}
                    warm,warm_profile=job('warm-after-slow-cpu','metal',small)
                    validate_profile(warm_profile,'metal',small)
                    archive=warm_profile['metal_execution']['pipeline_archive'];require(archive[0]>0 and archive[1]==0,'Need a confirmed warm GPU diagnostic')
                    gpu_estimate=warm_profile['render_total_seconds']*size[0]*size[1]/(small[0]*small[1])
                    diagnostic.update(gpu_warm_diagnostic=warm,gpu_linear_estimate_seconds=gpu_estimate)
                    result['slow_cpu_diagnosis']=diagnostic;save()
                    require(gpu_estimate<180,'GPU diagnostic also projects beyond frame limit')

            frames=[]
            for i in range(3):
                row,profile=job('repeat-'+str(i),'metal',size)
                require(profile['backend']=='metal','Full-size backend differs from preview')
                frame(ROOT,row,scene,ctx);frames.append(row)
                if i:require(frames[0]['image']['sha256']==row['image']['sha256'],'Exact PNG repeat mismatch')
            controls=[]
            for stage,on in enabled(scene).items():
                if on:
                    off,_=job('off-'+stage,'metal',size,stage);frame(ROOT,off,scene,ctx,stage)
                    controls.append({'stage':stage,'off':off})
            proof={'scene_sha256':scene['sha256'],'runtime_files':ctx['runtime_files'],'controls':controls}
            row={'id':sid,'qualification':'QUALIFIED_GPU','resource_profile':ctx['resource_profile'],'frames':frames,'cpu_reference':reference,
                 'producer':'run_gpu_acceptance_queue.py','reviewer':'gpu_pipeline_frames raw process/image/work validator with same-engine effect controls'}
            if reused:row['cpu_reference_origin']=origin
            result['cpu_difference']=image_difference(ROOT/reference['image']['path'],ROOT/frames[0]['image']['path']) if reference else None
            result['frames']=frames;result['cpu_reference']=reference;result['controls']=controls
            # These known unresolved differences are already explicit in G5/G6;
            # frame completion alone cannot silently close their numerical review.
            pending=numeric_pending(scene)
            if reference is None:
                result['status']='OPEN_CPU_REFERENCE';result['reason']='GPU full frames/effects pass; original-size CPU comparison remains open';save();continue
            review=numeric_approval(ROOT,scene,ctx,reference,frames) if pending else None
            if review:row['numeric_review']=review;result['numeric_review']=review
            if pending and review is None:
                result['status']='OPEN_NUMERIC_REVIEW';result['reason']='Existing G5/G6 numerical attribution remains open';save();continue
            scene_frames(ROOT,row,scene,ctx,proof)
            observations=package/'observations'/(sid+'-effects.json');write(observations,proof);row['effect_observations']=ref(observations)
            ev=read_json(package/'evidence.json');require(all(r['id']!=sid for r in ev['scenes']),'Duplicate scene publication')
            ev['scenes'].append(row);write(package/'evidence.json',ev);refresh(package,contract,build)
            qualification=evaluate(contract,package);write(package/'qualification.json',qualification)
            require(sid in qualification.get('scene_outcomes',{}),'Common gate rejected newly completed scene: '+str(qualification.get('errors')))
            result['status']='QUALIFIED_GPU';result['full_gate_status']=qualification['status'];save()
            publish_current()
            print('ACCEPTED',q['rank'],len(qualification['scene_outcomes']),'/77',flush=True)
        except (ValueError,KeyError,TypeError,OSError,IndexError) as e:
            result['status']='OPEN_CHECK_FAILED';result['reason']=str(e);save();print('OPEN',q['rank'],str(e),flush=True)
            if 'Disk reserve' in str(e) or any(x.get('guard') in ('DISK_RESERVE','INPUT_CHANGED','EXECUTABLE_CHANGED') for x in result['stages']):break
    qualification=evaluate(contract,package);write(package/'qualification.json',qualification)
    publish_current()
    print('BATCH DONE',qualification['status'],'GPU',qualification['qualified_gpu_scenes'],'FP32_CPU',qualification['qualified_fp32_cpu_scenes'],flush=True)


if __name__=='__main__':main()
