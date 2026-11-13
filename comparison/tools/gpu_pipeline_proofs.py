"""Concrete passive observation validators for the full GPU contract.

Receipts provide independent review, not a substitute for these raw checks.
This format is an audit bundle, not cryptographic remote attestation. Test
fixtures are synthetic and must never be published as device qualification.
"""
import math
import re
import statistics
import xml.etree.ElementTree as ET
from pathlib import Path
from qualify_native_metal_all import require, artifact, contained, read_json, sha, digest, unique

STATE = tuple([f'{a}[{i}]' for a in ('xyz','c','j','original_j','derivative','sse') for i in range(4)] +
              ['rout','rold','rstop','rstop_d','otrap','vary_scale','dfree1','dfree2','smooth',
               'iteration','first','tmp','max_iterations'] + [f'hybrid_counts[{i}]' for i in range(6)] +
              ['julia','inside','calc_smooth','current_slot'])
CASE_SETS = {
 'scheduling': {'uneven_rows','group1','group2','group31','group32','group33','continuation',
                'inside','integer_counts','normal_chain','reflection_chain','faulted_continuation'},
 'resource_reuse': {'hit','library_replace','device_key','profile_key','formula_namespace',
                   'formula_ids','driver_key','smoothing_key','eviction','concurrent','failure_retry','lifetime'},
 'result_reuse': {'hit','incoming_state','outgoing_state','integer_counts','parameters','maps',
                 'camera','lights','effect_flags','eviction','concurrent','lifetime'},
 'metal_compatible': {'profile','primary','normals','shadows','ao','color','invalid_profile'},
 'guard_failures': {'bad_library','wrong_profile','bad_abi','nonfinite','work_exhaustion',
                    'cancel','allocation','out_of_bounds','deadline'},
 'independent_heldout': {'parameter_perturbation','state_poison','mixed_chain','cache_invalidation','concurrent'},
}
KEY_CHANGE={'library_replace':'library_sha256','device_key':'device','profile_key':'profile',
            'formula_namespace':'formula_namespace','formula_ids':'formula_ids','driver_key':'orbit_driver',
            'smoothing_key':'smoothing_policy','incoming_state':'incoming_state','outgoing_state':'incoming_state',
            'integer_counts':'incoming_state','parameters':'program_bytes','maps':'maps','camera':'camera',
            'lights':'lights','effect_flags':'effect_flags'}
ORDINARY = set('thread_pool render_context map_resources render_control render_control_api_lifecycle portable_formula_control render_control_image_process pas_lcg portable_sqrt sse_estimates arm64_numerics extended_sine extended_small_division extended_product extended_series render_backend native_reflection_host native_reflection_gpu numerical_de numerical_camera native_precision render_precision manowar_hybrid manowar_sse_producer math3d m3p_parser m3i_parser renderer external_registry integer_options formula_validation formula_iteration_guard formula_work_guard zbuffer_normals camera_tile_setup ao_wavelet_lanes dof render_failures render_control_api render_profile render_control_cli render_profile_cli render_backend_cli render_isolation legacy_loader'.split())
ORDINARY.update(('native_render_plan','native_resource_entry','native_formula_resources',
                 'native_material_palette','native_postprocess_plan','background_resources'))
ORDINARY = {n+'_tests' for n in ORDINARY}

def positive(x, label, zero=False):
    require(type(x) in (int,float) and abs(x)<1e100 and math.isfinite(x) and (x>=0 if zero else x>0), label)

def semantic_input(row):
    # Descriptive labels cannot turn a training vector into an unseen vector.
    payload = {k:row[k] for k in ('identity','kind','input_state_bits','parameter_bits')}
    if 'resources' in row:
        payload['resources'] = row['resources']
    if 'context' in row:
        payload['context'] = row['context']
    return payload

def formula_repeats(base, cases, ident, ctx):
    used=set()
    for case in cases:
        require(set(case['gpu_state_bits'])==set(STATE) and set(case['input_state_bits'])==set(STATE), 'Incomplete formula state observation')
        require(len(case['repeats'])==3,'Three formula observations required')
        payload=semantic_input(case)
        require(payload['identity']==list(ident),'Wrong formula input identity')
        for repeat in case['repeats']:
            event=run_event(base,repeat['event'],ctx['tools']['formula_gpu'])
            require(event['run_id'] not in used,'Reused formula execution');used.add(event['run_id'])
            inp=artifact(base,repeat['inputs'])
            require(repeat['inputs'] in event['inputs'] and str(inp) in event['argv'] and
                    read_json(inp)==payload,'Unbound formula input')
            actual=read_json(output_bound(base,event,repeat['observation']))
            require(actual['run_id']==event['run_id'] and actual['state_bits']==case['gpu_state_bits'] and
                    actual['input_digest']==digest(payload) and actual['library_sha256']==ctx['library']['sha256'] and
                    actual['faults']==0 and actual['gpu_invocations']>0 and actual['identity']==list(ident),
                    'Unbound/mismatched formula repeat')
            if 'resources' in payload:
                from native_formula_resource_input import validate_report as validate_map_report
                require(all(record in event['inputs'] for record in payload['resources']['files']),
                        'Formula execution lacks captured resource files')
                validate_map_report(actual, payload['resources'], base)
        ctx['training_inputs'].add(digest(payload))

def option(argv, name):
    require(isinstance(argv,list) and all(isinstance(x,str) for x in argv), 'Malformed argv')
    require(argv.count(name)==1 and argv.index(name)+1<len(argv), 'Missing/duplicate '+name)
    return argv[argv.index(name)+1]

def run_event(base, record, executable, *, success=True, max_seconds=None):
    path=artifact(base,record); e=read_json(path)
    require(e['schema']=='mb3d.gpu-process.v2' and re.fullmatch(r'[0-9a-f]{32}',e['run_id']), 'Missing process identity')
    require(type(e['pid']) is int and e['pid']>0 and e['owned_process_reaped'] is True, 'Unreaped process')
    require(type(e['started_ns']) is int and type(e['finished_ns']) is int and
            0<e['started_ns']<e['finished_ns'], 'Invalid process interval')
    positive(e['seconds'],'Invalid process duration')
    require(abs((e['finished_ns']-e['started_ns'])/1e9-e['seconds'])<=max(.1,e['seconds']*.01), 'Process clock/duration mismatch')
    positive(e['effective_timeout_seconds'],'Invalid process deadline')
    require(e['seconds']<=e['effective_timeout_seconds']+1, 'Process exceeded effective deadline')
    if max_seconds is not None:
        require(e['seconds']<=max_seconds and e['effective_timeout_seconds']<=max_seconds, 'Over-deadline process')
    binary=Path(e['argv'][0]).resolve()
    require(binary==Path(executable['path']).resolve() and binary.is_file() and
            sha(binary)==executable['sha256']==e['executable_sha256_before']==e['executable_sha256_after'], 'Executable identity mismatch')
    require(Path(e['cwd']).is_dir(), 'Missing working directory')
    require(e['exit_code']==0 and e['guard'] is None if success else e['exit_code']!=0, 'Unexpected process outcome')
    outputs=unique(e['outputs'],lambda x:x['path'],'process output')
    for r in outputs.values():artifact(base,r)
    for r in e['inputs']:artifact(base,r)
    artifact(base,e['log'])
    return e

def output_bound(base, event, record):
    p=artifact(base,record)
    require(record in event['outputs'], 'Output not in captured process outputs')
    return p

def binary_identity(base, record):
    return {'path':str(artifact(base,record)), 'sha256':record['sha256']}

def compare_buffers(base, comparison, required, count):
    pairs=unique(comparison,lambda r:r['kind'],'buffer comparison')
    require(set(pairs)==set(required),'Incomplete observation buffers')
    for p in pairs.values():
        a=artifact(base,p['expected']);b=artifact(base,p['actual'])
        width={'state':188,'logical_work':16,'rgb':3}[p['kind']]
        require(p['records']==count and p['bytes_per_record']==width and count>0 and
                a!=b and a.stat().st_size==count*width and a.read_bytes()==b.read_bytes(), 'Observed typed buffer differs from independent reference')

def probe_cases(base, data, ctx, kind):
    cases=unique(data['cases'],lambda x:x['id'],'probe case')
    require(set(cases)>=CASE_SETS[kind], 'Missing mandatory '+kind+' cases')
    identities=set(); used=set()
    for name,c in cases.items():
        e=run_event(base,c['event'],ctx['tools'][kind])
        require(e['run_id'] not in used,'Probe process reused');used.add(e['run_id'])
        p=output_bound(base,e,c['observation']);o=read_json(p)
        require(o['schema']=='mb3d.gpu-probe-observation.v1' and o['case_id']==name and
                o['run_id']==e['run_id'] and o['binding_sha256']==ctx['binding_id'], 'Unbound probe observation')
        expected_marker=[1,4,80] if kind=='metal_compatible' else [2,4,32]
        library=ctx['compatible_library'] if kind=='metal_compatible' else ctx['library']
        require(o['library_marker']==expected_marker and o['library_sha256']==library['sha256'] and
                o['cpu_pixel_calls']==0, 'Wrong probe library or CPU work')
        require(type(o['gpu_invocations']) is int and o['gpu_invocations']>0,'No actual GPU probe work')
        require(o['faults']==o['expected_faults'] and type(o['faults']) is int and o['faults']>=0, 'Unexpected probe faults')
        ref=run_event(base,c['reference_event'],ctx['tools']['oracle'])
        require(ref['executable_sha256_before']!=e['executable_sha256_before'],'Reference shares probe executable')
        inputs=read_json(artifact(base,c['inputs']))
        require(c['inputs'] in e['inputs'] and c['inputs'] in ref['inputs'] and
                str(artifact(base,c['inputs'])) in e['argv'] and str(artifact(base,c['inputs'])) in ref['argv'],'Unbound probe/reference inputs')
        require(isinstance(inputs,list) and bool(inputs),'Missing record inputs')
        require(o['input_digest']==digest(inputs),'Probe input digest mismatch')
        require([x['identity'] for x in inputs]==o['formula_identities'],'Unmapped formula records')
        for b in o['comparisons']:
            for r in (b['expected'],b['actual']):artifact(base,r)
            output_bound(base,e,b['actual'])
            output_bound(base,ref,b['expected'])
        expected=('state','logical_work','rgb') if kind in ('result_reuse','independent_heldout') else ('state','logical_work')
        compare_buffers(base,o['comparisons'],expected,len(inputs))
        ids={tuple(x) for x in o['formula_identities']}
        require(ids and ids<=ctx['formulas'],'Unknown/missing probe formula');identities |= ids
        if kind in ('resource_reuse','result_reuse'):
            m=o['cache']; required_keys=ctx['cache_keys'][kind]
            require(set(m['key_fields'])==set(required_keys),'Incomplete cache key')
            for key in ('hits','misses','evictions','physical_work','uncached_physical_work','peak_entries','entry_limit','peak_bytes','byte_limit'):
                require(type(m[key]) is int and m[key]>=0,'Invalid cache metric')
            require(0<m['entry_limit']<=256 and 0<m['byte_limit']<=1024**3 and
                    m['peak_entries']<=m['entry_limit'] and m['peak_bytes']<=m['byte_limit'], 'Unbounded cache')
            if name=='hit':require(m['hits']>0 and m['physical_work']<m['uncached_physical_work'],'No demonstrated reuse')
            if name=='eviction':require(m['evictions']>0,'Eviction not exercised')
            requests=m['requests'];require(len(requests)>=2,'Missing ordered cache requests')
            require(all(set(r['key'])==set(required_keys) and type(r['hit']) is bool for r in requests),'Incomplete observed request keys')
            require(requests[0]['hit'] is False,'Cache test not primed from empty state')
            if name=='hit':require(requests[1]['key']==requests[0]['key'] and requests[1]['hit'] is True,'Warm reuse not observed')
            if name not in ('hit','eviction','lifetime','concurrent','failure_retry'):
                changed={k for k in required_keys if requests[1]['key'][k]!=requests[0]['key'][k]}
                require(changed=={KEY_CHANGE[name]} and requests[1]['hit'] is False,'Wrong/stale cache key reused after priming')
            if name=='concurrent':require(m['overlapping_clients']>=2 and m['duplicate_compiles']==0,'Concurrent reuse not exercised')
            if name=='lifetime':require(m['evicted_while_in_use']>0 and m['live_handle_reads_after_eviction']>0,'Lifetime not exercised')
            if name=='failure_retry':require(m['failed_attempts']>0 and m['successful_retries']>0,'Failure retry not exercised')
        if kind=='scheduling':
            require(set(o['stages'])==set(ctx['stages']) and o['active_tasks']>=2 and o['preserved_task_order'] is True,
                    'Incomplete scheduling observation')
        if kind=='independent_heldout':
            for row in inputs:
                payload=semantic_input(row)
                require(digest(payload) not in ctx['training_inputs'],'Training input used as holdout')
    if kind in ('scheduling','independent_heldout'):
        require(identities==ctx['formulas'],'Incomplete formula scheduling/holdout coverage')

def dependency_analysis(base, data, ctx):
    rows=unique(data['formulas'],lambda r:tuple(r['identity']),'dependency identity')
    require(set(rows)==ctx['formulas'],'Incomplete dependency inventory')
    for ident,r in rows.items():
        require(r['classification'] in ('independent','ordered'),'Unknown dependency classification')
        for key in ('reads','writes','initialized'):
            require(isinstance(r[key],list) and set(r[key])<=set(STATE),'Unknown state lane')
        require(r['writes'],'Empty formula write audit')
        live=set(r['reads'])-set(r['initialized'])
        require(set(r['live_in'])==live,'Read-before-write analysis mismatch')
        require(not live if r['classification']=='independent' else bool(r['preserved_dependencies']), 'Discarded dependency')
        require(r['drivers'] and set(r['drivers'])<=set(range(10)), 'Missing orbit drivers')
        require(r['source_files'],'Missing formula source binding')
        for name in r['source_files']:require(name in ctx['source'],'Unbound formula/driver source')
        e=run_event(base,r['event'],ctx['tools']['dependency_analysis'])
        o=read_json(output_bound(base,e,r['observation']))
        require(o['identity']==list(ident) and o['run_id']==e['run_id'] and o['read_before_write']==r['live_in'] and
                o['writes']==r['writes'] and o['source_hashes']=={n:ctx['source'][n] for n in r['source_files']},'Audit/observation mismatch')

def build_provenance(base,data,ctx):
    from collections import defaultdict
    for mode in ('baseline','candidate'):
        proof=data[mode]; recs=[read_json(artifact(base,r)) for r in proof['commands']]
        require(recs and all(type(r.get('pid')) is int and r['pid']>0 for r in recs) and
                len({(r['pid'],r['start_time_ns']) for r in recs})==len(recs),'Duplicate/empty compiler records or missing process identity')
        products=defaultdict(list)
        for r in recs:
            require(r['schema']=='mb3d.actual-build-command.v1' and r['exit_code']==0 and not r['capture_errors'] and
                    r['end_time_ns']>=r['start_time_ns'] and r['argv'], 'Failed/interrupted compiler command')
            require(Path(r['tool']['path']).is_file() and sha(r['tool']['path'])==r['tool']['sha256'],'Changed compiler tool')
            if r.get('output'):products[r['output']['path']].append(r)
        expected=ctx['build_products'][mode]
        require(expected and all(str(artifact(base,r)) in products for r in expected),'Missing build output producer')
        compiled=0;airs=0
        for r in recs:
            out=r.get('output');bound_sources=0
            compiled_root=ctx.get('candidate_compiled_source_root',ctx['root']) if mode=='candidate' else ctx['baseline_compiled_source_root']
            if out and Path(out['path']).suffix in ('.o','.air'):
                require(r.get('depfile') and r.get('dependencies'),'Missing compiler dependency closure')
                compiled+=Path(out['path']).suffix=='.o';airs+=Path(out['path']).suffix=='.air'
            for d in [*r.get('dependencies',[]),*r.get('explicit_inputs_before',[])]:
                root=ctx['build_dir'] if mode=='candidate' else ctx['baseline_build_dir']
                if Path(d['path']).is_relative_to(root):
                    require(d['path'] in products,'Build-owned input has no producer')
                if d['path'] in products:
                    prior=[x for x in products[d['path']] if x['end_time_ns']<=r['start_time_ns']]
                    require(prior and max(prior,key=lambda x:x['end_time_ns'])['output']['sha256']==d['sha256'], 'Broken build producer edge')
                else:
                    if d.get('classification')=='repository':require(Path(d['path']).is_relative_to(compiled_root),'Repository input outside declared compiled root')
                    if Path(d['path']).is_relative_to(compiled_root):
                        rel=str(Path(d['path']).relative_to(compiled_root))
                        expected_source=ctx['source'] if mode=='candidate' else ctx['baseline_source']
                        require(expected_source.get(rel)==d['sha256'],'Unbound compiled source');bound_sources+=1
                        p=ctx['root']/rel if mode=='candidate' else ctx['baseline_source_root']/rel
                        require(p.is_file() and sha(p)==d['sha256'],'Changed compiled source copy')
                    else:
                        require(Path(d['path']).is_file() and sha(d['path'])==d['sha256'],'Changed compiler input')
            if out and Path(out['path']).suffix in ('.o','.air'):require(bound_sources>0,'Compiled object/AIR has no bound project source')
            if out:
                latest=max(products[out['path']],key=lambda x:x['end_time_ns'])
                require(Path(out['path']).is_file() and sha(out['path'])==latest['output']['sha256'],'Changed build product')
        require(compiled>0 and airs>=6,'Incomplete object/AIR build')
        for library in (r for r in expected if r['path'].endswith('.metallib')):
            producer=max(products[str(artifact(base,library))],key=lambda x:x['end_time_ns'])
            inputs=[d for d in producer.get('explicit_inputs_before',[]) if d['path'].endswith('.air')]
            needed=ctx['library_air_counts'][library['path']]
            require(len({d['path'] for d in inputs})==needed,'Incomplete per-library AIR inputs')

def ordinary_input_files(commands, root, build):
    root=Path(root).resolve();build=Path(build).resolve()
    paths={Path(a).resolve() for argv in commands.values() for a in argv
           if Path(a).is_absolute() and Path(a).is_file() and Path(a).resolve().is_relative_to(root)}
    paths.update(build.glob('*.metallib'))
    paths.update(p for p in (build/'CTestTestfile.cmake',build/'CTestCustom.cmake') if p.is_file())
    return sorted(paths)

def cpu_ordinary(base,data,ctx):
    listing=run_event(base,data['listing_event'],ctx['tools']['cpu_ordinary'])
    require('--show-only=json-v1' in listing['argv'] and option(listing['argv'],'-L')=='ordinary' and
            Path(option(listing['argv'],'--test-dir')).resolve()==ctx['build_dir'],'Wrong CTest inventory invocation')
    tests=read_json(output_bound(base,listing,data['listing']))
    commands={t['name']:t['command'] for t in tests['tests']}
    require(len(tests['tests'])==len(ORDINARY) and commands==ctx['ordinary_commands'],'Changed ordinary test commands')
    e=run_event(base,data['event'],ctx['tools']['cpu_ordinary'])
    require(option(e['argv'],'-L')=='ordinary' and Path(option(e['argv'],'--test-dir')).resolve()==ctx['build_dir'], 'Wrong test selection/build')
    for path in ordinary_input_files(commands,base,ctx['build_dir']):
        require({'path':str(path.relative_to(base)), 'sha256':sha(path)} in e['inputs'],'Unbound ordinary test input')
    p=output_bound(base,e,data['junit'])
    require(Path(option(e['argv'],'--output-junit')).resolve()==p,'Unbound CTest output')
    try:xml=ET.parse(p).getroot()
    except ET.ParseError as e:raise ValueError('Malformed JUnit') from e
    cases=list(xml.iter('testcase'))
    require(len(cases)==len(ORDINARY) and {c.attrib['name'] for c in cases}==ORDINARY,'Wrong ordinary suite set')
    require(all(not list(c.iter('failure')) and not list(c.iter('error')) and not list(c.iter('skipped')) and
                c.attrib.get('status','run')=='run' for c in cases),'Failed/skipped ordinary suite')

def guard_failures(base,data,ctx):
    cases=unique(data['cases'],lambda c:c['id'],'guard case')
    require(set(cases)>=CASE_SETS['guard_failures'],'Missing mandatory guard')
    used=set()
    for name,c in cases.items():
        e=run_event(base,c['event'],ctx['tools']['guard_failures'],success=False,max_seconds=300)
        require(e['run_id'] not in used,'Guard run reused');used.add(e['run_id'])
        o=read_json(output_bound(base,e,c['observation']))
        require(o['case_id']==name and o['run_id']==e['run_id'] and o['detected'] is True and o['error_code']==name,
                'Guard not observed')
        output=contained(base,o['image_path'])
        require(not output.exists() and not any(r['path']==o['image_path'] for r in e['outputs']), 'Failed render published image')

def no_cpu_pixel_work(base,data,ctx):
    from gpu_pipeline_binding import for_scene
    rows=unique(data['scenes'],lambda r:r['id'],'CPU-work trace scene')
    # Only fully replayed overflow proofs enter this context. A scene label or
    # a producer-supplied fallback claim cannot exempt a GPU trace.
    cpu=ctx.get('qualified_fp32_cpu_scenes',{})
    require(set(cpu)<=set(ctx['scenes']) and
            all(r['status']=='QUALIFIED_FP32_CPU' for r in cpu.values()),'Unqualified CPU trace exception')
    require(set(rows)==set(ctx['scenes'])-set(cpu),'Missing no-CPU-work scene or CPU outcome reported as GPU')
    reported=unique(data.get('cpu_fallback_scenes',[]),lambda r:r['id'],'CPU-work trace exception')
    require(set(reported)==set(cpu),'Missing separately reported FP32 CPU outcome')
    for sid,r in reported.items():
        require(r['qualification']=='QUALIFIED_FP32_CPU' and r['result_sha256']==digest(cpu[sid]),
                'CPU trace exception differs from replayed full-scene proof')
    for sid,r in rows.items():
        from gpu_pipeline_frames import frame
        rendered=frame(base,r,ctx['scenes'][sid],for_scene(ctx,sid))
        e=run_event(base,r['event'],ctx['renderer'],max_seconds=300)
        o=read_json(output_bound(base,e,r['trace']))
        require(o['schema']=='mb3d.gpu-stage-trace.v1' and o['run_id']==e['run_id'] and
                o['scene_sha256']==ctx['scenes'][sid]['sha256'] and o['library_sha256']==ctx['library']['sha256'] and
                o['profile_sha256']==r['profile']['sha256'],'Unbound GPU trace')
        require(set(o['cpu_calls'])==set(ctx['stages']) and all(type(v) is int and v==0 for v in o['cpu_calls'].values()),'CPU pixel work')
        require(set(o['gpu_calls'])==set(ctx['stages']) and all(type(v) is int and v>=0 for v in o['gpu_calls'].values()) and
                o['gpu_calls']['formula_and_distance']>0,'Missing GPU trace')

VALIDATORS={'dependency_analysis':dependency_analysis,'build_provenance':build_provenance,
            'cpu_ordinary':cpu_ordinary,'guard_failures':guard_failures,'no_cpu_pixel_work':no_cpu_pixel_work}
for _kind in ('scheduling','resource_reuse','result_reuse','metal_compatible','independent_heldout'):
    VALIDATORS[_kind]=lambda base,data,ctx,kind=_kind:probe_cases(base,data,ctx,kind)
