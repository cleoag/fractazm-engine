"""Scene, effect and benchmark validators with process/resource identity."""
import math
import statistics
from pathlib import Path
from qualify_native_metal_all import require, artifact, read_json, unique, sha
from run_m3parameter_gpu import validate_profile, rgb, work_record
from gpu_pipeline_proofs import run_event, option, output_bound, positive
from gpu_pipeline_binding import for_scene

FLAGS={'ao':'--no-ao','shadows':'--no-hard-shadow-pass','reflections':'--no-reflections',
       'dof':'--no-dof','normals':'--no-zbuffer-normals','volume':'--no-volume'}
ACTIVITY={'ao':('execution','ao_samples'),'shadows':('execution','shadow_steps'),
          'reflections':('reflections','rays'),'dof':('execution','dof_contributing_pairs'),
          'normals':('execution','znormal_foreground'),'volume':('execution','volume_map_cells')}

def archive_state(execution):
    # A validated in-process GPU render can reuse in-memory pipelines and
    # make zero disk-archive queries. This does not validate early CPU fallback.
    state=execution['pipeline_archive']
    require(isinstance(state,list) and len(state)==2 and
            all(type(x) is int and x>=0 for x in state),
            'Missing/invalid pipeline archive observation')
    return state

def logical_work(value):
    # Compilation changes cold/warm archive counts, never fractal work.
    result=work_record(value)
    result['execution'].pop('pipeline_archive',None)
    return result

def enabled(scene):
    s=scene['settings']
    return {'ao':s['ao_kind']!='disabled','shadows':s['hard_shadows_enabled'],
            'reflections':s['reflection_enabled'],'dof':s['dof_enabled'],
            'normals':s['zbuffer_normals'],'volume':bool(s['volume_light'])}

def argv_shape(argv, mandatory, switches, input_path=None):
    require(isinstance(argv,list) and bool(argv) and all(isinstance(x,str) for x in argv), 'Empty/malformed argv')
    seen=set();i=1
    if input_path is not None:
        require(len(argv)>1 and Path(argv[1]).resolve()==input_path,'Wrong scene path');i=2
    while i<len(argv):
        a=argv[i];require(a not in seen,'Duplicate option');seen.add(a)
        if a in switches:i+=1
        else:
            require(a in mandatory and i+1<len(argv),'Unknown option/override '+a);i+=2
    require(seen==set(mandatory)|set(switches),'Incomplete option set')

def frame(base,row,scene,ctx,off=None):
    e=run_event(base,row['event'],ctx['renderer'],max_seconds=300)
    require(e['seconds']<=180,'Diagnostic deadline exceeded; further repeats prohibited')
    source=ctx['inputs'][scene['id']]
    for path,expected in ((source,scene['sha256']),(ctx['library_path'],ctx['library']['sha256'])):
        require(any(artifact(base,r)==path and r['sha256']==expected for r in e['inputs']),'Missing bound scene/library input')
    # Parity mode disables authored primary/AO jitter. Stable ordinary seeds
    # already allow exact repeats without that scene mutation.
    switches={FLAGS[off]} if off else set()
    mandatory={'--backend','--precision','--width','--height','--threads','--metal-library','--output','--profile-json'}
    argv_shape(e['argv'],mandatory,switches,source)
    require(Path(e['cwd']).resolve()==ctx['runtime'],'Wrong resource working directory')
    a=e['argv'];size=scene['settings']['size']
    require(option(a,'--backend')=='metal' and option(a,'--precision')=='native' and
            [int(option(a,'--width')),int(option(a,'--height'))]==size and int(option(a,'--threads'))==ctx['threads'],'Changed render settings')
    require(Path(option(a,'--metal-library')).resolve()==ctx['library_path'],'Wrong Metal library')
    p=output_bound(base,e,row['profile']);im=output_bound(base,e,row['image'])
    profile=read_json(p)
    require(Path(option(a,'--profile-json')).resolve()==p and Path(option(a,'--output')).resolve()==im and
            Path(profile['output']).resolve()==im and Path(profile['input']).resolve()==source and
            Path(profile['metal_library']).resolve()==ctx['library_path'],'Unbound frame IO')
    w=validate_profile(profile,'metal',size)
    for stage,on in enabled(scene).items():
        require(profile['stages'][stage]['state']==('completed' if on and stage!=off else 'not_run'),'Effect stage mismatch')
    image,_=rgb(im,size)
    require(image['unique_colors']>1,'Constant scene image')
    return {'image':image,'work':logical_work(w),'archive':archive_state(profile['metal_execution']),
            'run_id':e['run_id'],'event_path':str(artifact(base,row['event'])),
            'profile_path':str(p),'image_path':str(im),'profile':profile}

def scene_frames(base,row,scene,ctx,proof):
    from native_numeric_review import pending,validate as validate_numeric_review
    if pending(scene):
        require('numeric_review' in row and 'cpu_reference' in row,'Missing bound numerical review for known outlier')
        validate_numeric_review(base,row['numeric_review'],scene,ctx,row['cpu_reference'],row['frames'][0])
    require(len(row['frames'])==3,'Three full frames required')
    frames=[frame(base,r,scene,ctx) for r in row['frames']]
    for field in ('run_id','event_path','profile_path','image_path'):
        require(len({f[field] for f in frames})==3,'Copied/reused frame '+field)
    require(all((f['image'],f['work'])==(frames[0]['image'],frames[0]['work']) for f in frames),'Exact frame/work mismatch')
    require(all(f['archive']==frames[0]['archive'] for f in frames),'Frame repeats changed archive state')
    require(proof['scene_sha256']==scene['sha256'] and proof['runtime_files']==ctx['runtime_files'],'Wrong effect/resource proof')
    controls=unique(proof['controls'],lambda r:r['stage'],'effect control')
    required={k for k,v in enabled(scene).items() if v}
    require(set(controls)==required,'Missing/extra effect controls')
    ids={f['run_id'] for f in frames}
    for stage,c in controls.items():
        off=frame(base,c['off'],scene,ctx,stage)
        require(off['run_id'] not in ids,'Control process reused');ids.add(off['run_id'])
        require(off['work']['execution']['engine']==frames[0]['work']['execution']['engine'],'Control changed GPU engine')
        group,key=ACTIVITY[stage]
        require(key in frames[0]['work'][group] and key in off['work'][group], 'Missing measured GPU effect counter: '+key)
        require(type(frames[0]['work'][group][key]) is int and frames[0]['work'][group][key]>0,'No GPU effect activity')
        require(off['work'][group][key]==0,'Disabled effect still reports GPU activity')
        require(off['image']!=frames[0]['image'],'No visible effect control')
    ctx.setdefault('qualified_scenes',{})[scene['id']]={'image':frames[0]['image'],'work':frames[0]['work']}
    return ids

def benchmark(base,job,scene,ctx,mode):
    e=run_event(base,job['event'],ctx['benchmarks'][mode])
    path=output_bound(base,e,job['report']);r=read_json(path)
    args=e['argv'];size=scene['settings']['size']
    mandatory={'--scene','--output','--resource-root','--backend','--precision','--width','--height',
               '--threads','--warmups','--repeats','--metal-library','--frame-timeout-seconds','--rng-mode'}
    argv_shape(args,mandatory,set())
    expected={'--scene':str(ctx['inputs'][scene['id']]),'--output':str(path.parent),
              '--resource-root':str(ctx['runtime']),'--backend':'metal','--precision':'native',
              '--width':str(size[0]),'--height':str(size[1]),'--threads':str(ctx['threads']),
              '--warmups':'0','--repeats':'3','--metal-library':str(ctx['libraries'][mode]),'--rng-mode':'scene'}
    require(all(option(args,k)==v for k,v in expected.items()) and Path(e['cwd']).resolve()==ctx['runtime'],'Wrong benchmark inputs/build')
    limit=float(option(args,'--frame-timeout-seconds'));require(0<limit<=300,'Invalid frame timeout')
    require(r['schema']=='mb3d.backend-render-benchmark.v1' and r['status']=='COMPLETE' and r['error']=='' and
            r['backend']=='metal' and r['execution']['precision']=='native' and r['execution']['deterministic'] is False and
            r['execution']['rng_mode']=='scene' and
            Path(r['scene']).resolve()==ctx['inputs'][scene['id']] and Path(r['resource_root']).resolve()==ctx['runtime'], 'Wrong benchmark identity')
    features=r['loaded_features']
    require(r['effective_rng']=={'primary_first_step_jitter':bool(features['iOptions']&1),
            'ao_first_step_jitter':bool(features['bCalcAmbShadowAutomatic']&128)},'Changed scene jitter')
    require(r['settings']=={'width':size[0],'height':size[1],'threads':ctx['threads'],'warmups':0,'repeats':3}, 'Benchmark settings changed')
    require(r['warmups']==[] and len(r['samples'])==3,'Wrong sample count')
    samples=[r['cold'],*r['samples']];sigs=[];times=[];image_paths=set();warm_archives=[]
    for i,s in enumerate(samples):
        require(s['phase']==('cold' if i==0 else 'warm') and s['index']==(0 if i==0 else i-1), 'Misclassified/reordered sample')
        require(s['state']=='completed' and s['error']=='' and s['precision']=='native' and [s['width'],s['height']]==size,'Failed sample')
        d=s['deadline'];positive(d['seconds'],'Invalid frame guard observation')
        require(d['limit_seconds']==limit and d['seconds']<=limit and d['completed'] is True,'Missing/exceeded per-frame guard')
        timing=s['cold_end_to_end_ms'] if i==0 else s['end_to_end_ms'];positive(timing,'Invalid full timing')
        require(abs(timing/1000-d['seconds'])<=.001 and timing<=300000,'Underreported cold/guard time')
        backend=s['backend'];execution=backend['execution']
        require(backend['requested']=='metal' and backend['effective']=='metal' and backend['fallback'] is False and
                execution['library_profile']==[2,4,32] and execution['validated'] is True,'Benchmark fallback/profile fault')
        for k in ('faults','nonfinite','work_exhausted'):require(execution[k]==0,'GPU benchmark fault')
        cache=archive_state(execution)
        if i:
            require(cache[1]==0,'Warm frame has pipeline archive misses')
            warm_archives.append(cache)
        for stage,on in enabled(scene).items():require(s['stages'][stage]['state']==('completed' if on else 'not_run'),'Benchmark dropped effect')
        n=size[0]*size[1]
        for k,v in {'primary_records':n,'normal_records':n,'surface_records':2*n,'color_records':n}.items():
            require(type(execution[k]) is int and execution[k]==v,'Missing full benchmark records')
        require(s['stages']['final_surface']['state']=='completed' and s['stages']['save']['state']=='completed','Incomplete benchmark pipeline')
        require(Path(s['image']).name==s['image'],'Image path escape')
        im=path.parent/s['image'];require(str(im) not in image_paths,'Repeated benchmark image');image_paths.add(str(im))
        refs=[x for x in e['outputs'] if artifact(base,x)==im];require(len(refs)==1,'Image not bound to benchmark process')
        image,_=rgb(im,size)
        sigs.append((image,logical_work({'execution':execution,'reflections':backend['reflections']})))
        if i:times.append(timing/1000)
    require(all(s==sigs[0] for s in sigs),'Cold/warm RGB/work mismatch')
    require(all(s==warm_archives[0] for s in warm_archives),'Warm archive observations differ')
    require(sum(s['deadline']['seconds'] for s in samples)<=e['seconds']+.01,'Frames exceed observed process duration')
    return e['run_id'],str(path),sigs[0],times,r['loaded_features'],(e['started_ns'],e['finished_ns'])

def full_warm_timings(base,data,ctx):
    scenes=unique(data['scenes'],lambda r:r['id'],'timing scene')
    cpu=ctx.get('qualified_fp32_cpu_scenes',{})
    require(set(scenes)==set(ctx['scenes'])-set(cpu),'Missing full GPU scene timings or CPU mixed into GPU speed denominator')
    reported=unique(data.get('cpu_fallback_scenes',[]),lambda r:r['id'],'CPU fallback timing scene')
    require(set(reported)==set(cpu),'Missing separately reported CPU fallback timings')
    for sid,row in reported.items():
        for key in ('cpu_render_seconds','fallback_selection_seconds','command_seconds'):
            require(row[key]==cpu[sid][key],'CPU timing differs from qualified frame observations')
    require(bool(scenes),'No GPU timings for GPU speed gate')
    gains=[];runids=set();reports=set()
    for sid,row in scenes.items():
        scene_ctx=for_scene(ctx,sid)
        times={};sigs={};features=[];intervals={}
        require(row['order']==['baseline','candidate','candidate','baseline'],'Timing order not balanced')
        for mode in ('baseline','candidate'):
            require(len(row[mode])==2,'Two independent timing processes required')
            fs=[benchmark(base,j,ctx['scenes'][sid],scene_ctx,mode) for j in row[mode]]
            for rid,path,sig,ts,feature,_ in fs:
                require(rid not in runids and path not in reports,'Duplicate timing process/report');runids.add(rid);reports.add(path)
                features.append(feature)
            require(fs[0][2]==fs[1][2],'Cross-process RGB/work difference')
            times[mode]=statistics.median(fs[0][3]+fs[1][3]);sigs[mode]=fs[0][2]
            intervals[mode]=[f[5] for f in fs]
        order=[intervals['baseline'][0],intervals['candidate'][0],intervals['candidate'][1],intervals['baseline'][1]]
        require(all(a[1]<=b[0] for a,b in zip(order,order[1:])),'Timing processes overlap or violate balanced order')
        require(all(f==features[0] for f in features),'Changed loaded scene/effects')
        require(sid in ctx['qualified_scenes'] and sigs['candidate']==
                (ctx['qualified_scenes'][sid]['image'],ctx['qualified_scenes'][sid]['work']), 'Benchmark differs from qualified full frame')
        # Numeric optimizations may change Native images, but require a separate
        # independently reviewed before/after difference record, never a repeat waiver.
        if sigs['baseline']!=sigs['candidate']:
            review=read_json(artifact(base,row['native_difference_review']))
            require(review['baseline']==sigs['baseline'][0] and review['candidate']==sigs['candidate'][0] and
                    review['producer']!=review['independent_reviewer'] and review['cause'] and review['scope']=='native-only', 'Unreviewed Native change')
            require(review['baseline_work']==sigs['baseline'][1] and review['candidate_work']==sigs['candidate'][1], 'Unreviewed work change')
            require(sigs['baseline'][1]==sigs['candidate'][1], 'Changed logical work requires a separately versioned acceptance contract')
        require(times['candidate']<=times['baseline']*1.05,'Reproducible >5% regression')
        gains.append(times['baseline']/times['candidate'])
    require(math.exp(statistics.mean(map(math.log,gains)))>=1.25,'Full warm speedup below 1.25')
