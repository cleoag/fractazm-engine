"""Separate whole-frame native CPU acceptance for proven binary32 overflow.

This never grants GPU qualification or a GPU speedup sample. The diagnostic
recorder must leave every ordinary primary buffer bit-identical, and an exact
rational oracle must establish overflow on more than 1% of full-frame pixels.
"""
import json,shutil,tempfile
from pathlib import Path
from qualify_native_metal_all import require,artifact,read_json,sha,unique
from gpu_pipeline_proofs import run_event,output_bound,option,positive
from gpu_pipeline_frames import argv_shape,enabled,FLAGS,logical_work
from run_m3parameter_gpu import validate_profile,rgb
from fp32_overflow_witness import compare
from capture_fp32_frame_witnesses import instrument_native,instrument_host


def frame(base,row,scene,ctx,*,fallback=True,off=None,executable=None,library=None,reference_only=False):
    raw=read_json(artifact(base,row['event']));a=raw['argv']
    # Frozen copies with exactly the final bytes are the same host identity.
    identity=executable or ctx['renderer'];binary={'path':a[0],'sha256':identity['sha256']}
    e=run_event(base,row['event'],binary,max_seconds=300)
    require(not reference_only or (fallback is False and off is None),'Only a direct CPU reference may use reference-only validation')
    if not reference_only:require(e['seconds']<=180,'Diagnostic deadline exceeded; further repeats prohibited')
    source=ctx['inputs'][scene['id']];size=scene['settings']['size'];switches={FLAGS[off]} if off else set()
    mandatory={'--backend','--precision','--threads','--output','--profile-json'}
    if '--width' in a or '--height' in a:mandatory|={'--width','--height'}
    if fallback:mandatory.add('--metal-library')
    argv_shape(a,mandatory,switches,source)
    require(Path(e['cwd']).resolve()==ctx['runtime'],'Wrong resource directory')
    require(option(a,'--backend')==('metal' if fallback else 'cpu') and option(a,'--precision')=='native' and
            int(option(a,'--threads'))==ctx['threads'],'Changed CPU render settings')
    if '--width' in a:require([int(option(a,'--width')),int(option(a,'--height'))]==size,'Resized scene')
    require(any(artifact(base,r)==source and r['sha256']==scene['sha256'] for r in e['inputs']),'Scene not bound to process')
    p=output_bound(base,e,row['profile']);im=output_bound(base,e,row['image']);profile=read_json(p)
    require(Path(option(a,'--profile-json')).resolve()==p and Path(option(a,'--output')).resolve()==im and
            Path(profile['input']).resolve()==source and Path(profile['output']).resolve()==im,'Unbound frame IO')
    require(profile['threads_requested']==ctx['threads'] and profile['workers']==ctx['threads'],'CPU workers differ')
    for key in ('render_total_seconds','command_total_seconds'):positive(profile[key],'Invalid CPU full-frame time')
    positive(profile['backend_selection_seconds'],'Invalid backend selection time',zero=True)
    require(profile['render_total_seconds']+profile['backend_selection_seconds']<=profile['command_total_seconds']+.01 and
            profile['command_total_seconds']<=e['seconds']+.01,'CPU timing exceeds observed process')
    m=profile['metal_execution']
    require(all(m[k]==0 for k in ('faults','nonfinite','work_exhausted')) and profile['metal_reflections']['faults']==0,'GPU fault hidden by fallback')
    if fallback:
        lib=Path(option(a,'--metal-library')).resolve();expected=library or ctx['library']
        require(Path(profile['metal_library']).resolve()==lib and sha(lib)==expected['sha256'] and
                any(artifact(base,r)==lib and r['sha256']==expected['sha256'] for r in e['inputs']),'Wrong/unbound native library')
        require(profile['backend_requested']=='metal' and profile['backend_fallback'] is True and
                profile['backend_fallback_code']=='fp32_range' and bool(profile['backend_fallback_reason']),'Unaccepted fallback')
        require(m['library_profile']==[2,4,32] and m['fp32_range_gate_stage']=='primary' and
                m['fp32_range_gate_units']==size[0]*size[1] and m['fp32_range_gate_percent']==1 and
                all(type(m[k]) is int for k in ('fp32_range_gate_units','fp32_range_gate_percent','fp32_range_gate_exceeded')) and size[0]*size[1]<100*m['fp32_range_gate_exceeded']<=100*size[0]*size[1],
                'Missing/exceeded range gate observation')
        normalized=dict(profile,backend_requested='cpu',backend_fallback=False,backend_fallback_code='',backend_fallback_reason='')
    else:normalized=profile
    work=validate_profile(normalized,'cpu',size)
    for stage,on in enabled(scene).items():
        value=profile['stages'][stage];active=on and stage!=off
        require(value['state']==('completed' if active else 'not_run'),'Changed CPU effect')
        require(value['seconds']>0 if active else value['seconds']==0,'Missing CPU effect activity')
    image,_=rgb(im,size);require(image['unique_colors']>1,'Constant CPU frame')
    return {'image':image,'work':logical_work(work),'profile':profile,'event':e,'reference_only':reference_only,
            'paths':tuple(str(x) for x in (artifact(base,row['event']),p,im)), 'run_id':e['run_id']}


def recorder(base,record,ctx):
    path=artifact(base,record);r=read_json(path);folder=path.parent;b=Path(r['production_build'])
    require(r['qualification'] is False and r['production_renderer_sha256']==ctx['renderer']['sha256'] and
            r['production_library_sha256']==ctx['library']['sha256'],'Recorder final identity mismatch')
    require(sha(b/'mb3d_render')==r['production_renderer_sha256'] and sha(b/'metal_native_scene.metallib')==r['production_library_sha256'],'Changed recorder input build')
    require(sha(folder/'recorder')==r['recorder_sha256'] and sha(folder/'instrumented.metallib')==r['instrumented_library_sha256'],'Changed recorder outputs')
    commands=read_json(b/'compile_commands.json');origin=Path(next(c['file'] for c in commands if c['file'].endswith('/general_renderer.mm'))).parents[3]
    original={str(x.relative_to(origin)):sha(x) for x in (origin/'src/render/native_metal').rglob('*') if x.is_file()}
    require(original==r['original_native_sources'],'Changed original native source closure')
    root=Path(__file__).resolve().parents[2]
    require(all(sha(root/k)==v for k,v in original.items()),'Diagnostic source differs from current production GPU source')
    # Reconstruct the reviewed observer from original source. A receipt cannot
    # self-authorize a different shader that manufactures convenient operands.
    with tempfile.TemporaryDirectory(prefix='mb3d-witness-check-') as td:
        local=Path(td);native=local/'src/render/native_metal'
        shutil.copytree(origin/'src/render/native_metal',native);shutil.copytree(origin/'comparison/metal_exact',local/'comparison/metal_exact')
        instrument_native(native)
        expected={str(x.relative_to(local)):sha(x) for x in native.rglob('*') if x.is_file()}
    require(expected==r['instrumented_sources'],'Recorder instrumentation differs from reviewed implementation')
    require(all(sha(folder/'source'/k)==v for k,v in expected.items()),'Changed recorder source')
    shared=origin/'comparison/metal_exact'
    require({str(x.relative_to(shared)):sha(x) for x in shared.rglob('*') if x.is_file()}==
            {str(x.relative_to(folder/'source/comparison/metal_exact')):sha(x) for x in (folder/'source/comparison/metal_exact').rglob('*') if x.is_file()},'Changed shared recorder headers/fixtures')
    require((folder/'general_renderer.mm').read_text()==instrument_host((origin/'src/render/native_metal/general_renderer.mm').read_text()),'Changed recorder host')
    require(len(r['commands'])==9 and all(c['exit_code']==0 for c in r['commands']),'Incomplete recorder compiler/link transcript')
    airs=[c for c in r['commands'] if 'metal' in c['argv'] and '-c' in c['argv']]
    require(len(airs)==6 and len({c['argv'][c['argv'].index('-o')+1] for c in airs})==6,'Missing AIR closure')
    production_commands=[read_json(p) for p in (b.parent/'provenance/records').glob('*.json')]
    for c in airs:
        src=Path(c['argv'][c['argv'].index('-c')+1]);require(src.is_relative_to(folder/'source') or
            (src.is_relative_to(origin/'comparison/metal_exact') and sha(src)==sha(root/src.relative_to(origin))),'Unbound shader source')
        flags=[x for x in c['argv'] if x.startswith('-f')]
        matches=[p['argv'] for p in production_commands if 'metal' in p.get('argv',[]) and '-c' in p['argv'] and Path(p['argv'][p['argv'].index('-c')+1]).name==src.name]
        require(matches and all(flags==[x for x in a if x.startswith('-f')] for a in matches),'Recorder changed arithmetic compiler flags')
    return r,folder


def scene_frames(base,row,scene,ctx,proof):
    require(proof['scene_sha256']==scene['sha256'] and proof['runtime_files']==ctx['runtime_files'],'Wrong effect/resource proof')
    require(len(row['frames'])==3,'Three full CPU fallback frames required')
    frames=[frame(base,x,scene,ctx) for x in row['frames']]
    ids=set();paths=set()
    def add(f):
        require(f['run_id'] not in ids and not set(f['paths'])&paths,'Copied/reused CPU evidence');ids.add(f['run_id']);paths.update(f['paths'])
    for f in frames:add(f)
    require(all((f['image'],f['work'])==(frames[0]['image'],frames[0]['work']) for f in frames),'CPU fallback RGB/work repeats differ')
    direct=frame(base,row['direct_cpu'],scene,ctx,fallback=False);add(direct)
    require(direct['image']==frames[0]['image'],'Fallback differs from direct native CPU')
    controls=unique(proof['controls'],lambda r:r['stage'],'CPU effect control')
    require(set(controls)=={k for k,v in enabled(scene).items() if v},'Missing/extra CPU effect controls')
    for stage,c in controls.items():
        off=frame(base,c['off'],scene,ctx,fallback=False,off=stage);add(off)
        require(off['image']!=direct['image'],'CPU effect control has no visible effect')
    evidence=row['fp32_overflow_proof'];rec,folder=recorder(base,evidence['recorder'],ctx)
    captures=[]
    for mode in ('baseline','witness'):
        lib=ctx['library'] if mode=='baseline' else {'sha256':rec['instrumented_library_sha256']}
        f=frame(base,evidence[mode],scene,ctx,executable={'sha256':rec['recorder_sha256']},library=lib);add(f)
        require(f['image']==frames[0]['image'] and f['work']==frames[0]['work'],'Diagnostic frame differs from production fallback')
        capture=output_bound(base,f['event'],evidence[mode]['capture'])
        require(f['event']['diagnostic_environment']=={'NATIVE_RANGE_CAPTURE':str(capture)},'Unbound diagnostic destination')
        captures.append(capture)
    result=compare(*captures)
    require(result['size']==scene['settings']['size'] and result['range_pixels']==frames[0]['profile']['metal_execution']['fp32_range_gate_exceeded'],'Proof differs from full scene gate')
    result.update(status='QUALIFIED_FP32_CPU',image=frames[0]['image'],effects=sorted(controls),
        cpu_render_seconds=[f['profile']['render_total_seconds'] for f in frames],
        fallback_selection_seconds=[f['profile']['backend_selection_seconds'] for f in frames],
        command_seconds=[f['event']['seconds'] for f in frames],pipeline_archive='unreported_before_early_fallback',
        production_renderer_sha256=ctx['renderer']['sha256'],production_library_sha256=ctx['library']['sha256'])
    ctx.setdefault('qualified_fp32_cpu_scenes',{})[scene['id']]=result
    return result
