#!/usr/bin/env python3
"""Passive full467/77 parallel-pipeline gate. Missing evidence stays OPEN.

Evidence manifest: binding (current source snapshot, candidate and baseline
binary/library artifacts), formula proofs, scene frame sets, and reviewed
receipts for dependency/reuse/holdout/regression obligations. All references
are relative, contained and SHA256 bound. No commands from evidence execute.
Package diagnostics are intentionally insufficient to pass this full gate.
"""
import argparse
from pathlib import Path
from qualify_native_metal_all import (ROOT, require, read_json, sha, digest, artifact,
    contained, unique, formula_key, current_registry, source_snapshot, receipt,
    formula_observations)
from gpu_pipeline_proofs import VALIDATORS, STATE, ORDINARY, binary_identity, run_event, output_bound, formula_repeats
from gpu_pipeline_frames import scene_frames, full_warm_timings
from gpu_pipeline_fp32 import scene_frames as fp32_scene_frames
from gpu_pipeline_binding import core_context,for_scene

SCHEMA = 'mb3d.gpu-parallel-evidence.v1'
INCREMENTAL_SCHEMA = 'mb3d.gpu-parallel-evidence.v2'
PROOFS = ('dependency_analysis', 'scheduling', 'resource_reuse', 'result_reuse',
          'build_provenance', 'cpu_ordinary', 'metal_compatible', 'guard_failures',
          'independent_heldout', 'no_cpu_pixel_work', 'full_warm_timings')

VALIDATORS['full_warm_timings']=full_warm_timings
IMPLEMENTED_PROOF_VALIDATORS = frozenset(VALIDATORS)|{'scene_effects_resources'}
RESOURCE_KEYS=('device','library_sha256','profile','kernel','formula_namespace','formula_ids','orbit_driver','smoothing_policy','specialization_constants')
RESULT_KEYS=RESOURCE_KEYS+('program_bytes','incoming_state','maps','camera','lights','effect_flags')

def scope(path):
    c = read_json(path)
    require(c['status'] == 'APPROVED_FULL_CONTRACT', 'Unapproved contract')
    require(c['formula_count'] == 467 and c['ordinary_scene_count'] == 77, 'Reduced scope')
    require(c['policies']['frame_hard_stop_seconds'] == 300, 'Deadline changed')
    p = contained(path.parent, c['inventory'])
    require(sha(p) == c['inventory_sha256'], 'Inventory changed')
    inv = read_json(p)
    formulas = unique(inv['formulas'], formula_key, 'formula')
    scenes = unique(inv['scenes'], lambda s: s['id'], 'scene')
    require(len(formulas) == 467 and set(formulas) == current_registry(ROOT), 'Registry mismatch')
    require(len(scenes) == 77, 'Scene denominator changed')
    for s in scenes.values():
        require(sha(contained(ROOT, s['path'])) == s['sha256'], 'Original scene changed')
    resources=contained(path.parent,c['resource_inventory'])
    require(sha(resources)==c['resource_inventory_sha256'],'Pinned resource inventory changed')
    c['_resources']=read_json(resources)
    if 'resource_profiles' in c:
        require(c['policies'].get('selected_available_maps',{}).get('accepted') is True,'Selected maps not approved')
        require(set(c['scene_required_maps'])==set(c['scene_resource_profiles'])==set(scenes),'Incomplete resource requirements')
        c['_resource_profiles']={}
        for name,profile in c['resource_profiles'].items():
            p=contained(path.parent,profile['inventory']);require(sha(p)==profile['sha256'],'Resource profile inventory changed')
            require(isinstance(profile['candidate_map_ids'],list) and all(type(i) is int and i>0 for i in profile['candidate_map_ids']),'Invalid selected map IDs')
            c['_resource_profiles'][name]=dict(profile,files=read_json(p))
    return c, formulas, scenes

def context(base,b,c,formulas,scenes):
    ctx={'root':ROOT,'source':b['source'],'formulas':set(formulas),'scenes':scenes,'binding_id':digest(b),
         'stages':c['compute_stages'],'threads':b['threads'],'training_inputs':set(),'qualified_scenes':{},
         'cache_keys':{'resource_reuse':RESOURCE_KEYS,'result_reuse':RESULT_KEYS}}
    require(type(b['threads']) is int and 1<=b['threads']<=256,'Invalid logical worker count')
    ctx['renderer']=binary_identity(base,b['renderer']);ctx['library']=b['library']
    ctx['library_path']=artifact(base,b['library']);ctx['compatible_library']=b['compatible_library']
    artifact(base,b['compatible_library'])
    ctx['benchmarks']={m:binary_identity(base,b['benchmark' if m=='candidate' else 'baseline_benchmark']) for m in ('baseline','candidate')}
    ctx['libraries']={m:artifact(base,b['library' if m=='candidate' else 'baseline_library']) for m in ('baseline','candidate')}
    ctx['build_dir']=Path(ctx['renderer']['path']).parent
    ctx['baseline_build_dir']=artifact(base,b['baseline_renderer']).parent
    ctx['build_products']={m:[b[k] for k in (('renderer','library','benchmark') if m=='candidate' else
                                            ('baseline_renderer','baseline_library','baseline_benchmark'))] for m in ('baseline','candidate')}
    ctx['library_air_counts']={b['library']['path']:6,b['baseline_library']['path']:6,b['compatible_library']['path']:1}
    for values in ctx['build_products'].values():
        for r in values:artifact(base,r)
    ctx['baseline_source']=read_json(artifact(base,b['baseline_source_manifest']))['source_files']
    ctx['baseline_source_root']=contained(base,b['baseline_source_root'])
    ctx['baseline_compiled_source_root']=Path(b['baseline_compiled_source_root']).resolve()
    ctx['candidate_compiled_source_root']=contained(base,b['candidate_compiled_source_root']) if 'candidate_compiled_source_root' in b else ROOT
    require(ctx['candidate_compiled_source_root'].is_dir(),'Missing candidate compiled source root')
    require(ctx['baseline_source'] and all(sha(contained(ctx['baseline_source_root'],n))==h for n,h in ctx['baseline_source'].items()),'Changed baseline source')
    ctx['tools']=b['tools'];require(set(ctx['tools'])==(set(PROOFS)-{'build_provenance','no_cpu_pixel_work','full_warm_timings'})|{'oracle','formula_gpu'},'Incomplete observation tools')
    for tool in ctx['tools'].values():
        require(Path(tool['path']).is_file() and sha(tool['path'])==tool['sha256'],'Changed observation tool')
    # Project probes, the independent oracle and ordinary binaries are part of
    # the captured source/build closure too. CTest itself is an external tool.
    for name,tool in ctx['tools'].items():
        if name!='cpu_ordinary':
            ctx['build_products']['candidate'].append({'path':str(Path(tool['path']).resolve().relative_to(base)), 'sha256':tool['sha256']})
    ctx['build_products']['candidate'].append(b['compatible_library'])
    ctx['ordinary_commands']=b['ordinary_commands']
    require(set(ctx['ordinary_commands'])==ORDINARY,'Missing ordinary command mapping')
    project_binaries={}
    for argv in ctx['ordinary_commands'].values():
        require(isinstance(argv,list) and argv and all(isinstance(a,str) for a in argv),'Malformed ordinary command')
        for a in argv:
            p=Path(a)
            if p.is_absolute() and p.is_file() and p.is_relative_to(ctx['build_dir']):
                r={'path':str(p.relative_to(base)),'sha256':sha(p)};project_binaries[r['path']]=r
    require(project_binaries,'Missing ordinary project binaries')
    ctx['build_products']['candidate'].extend(project_binaries.values())
    ctx['runtime']=contained(base,b['runtime'])
    actual={str(p.relative_to(ctx['runtime'])):sha(p) for p in ctx['runtime'].rglob('*') if p.is_file()}
    require(actual==c['_resources'],'Resource closure differs from pinned originals')
    require(all(not p.is_symlink() for p in ctx['runtime'].rglob('*')),'Resource alias forbidden')
    ctx['runtime_files']=actual
    require(set(b['inputs'])==set(scenes),'Incomplete canonical scene mapping')
    ctx['inputs']={sid:artifact(base,r) for sid,r in b['inputs'].items()}
    for sid,p in ctx['inputs'].items():
        require(sha(p)==scenes[sid]['sha256'],'Input scene differs')
        require(not (p.parent/'M3Maps').exists(),'Scene-local resource shadowing')
    return ctx

def evaluate(contract_path, base):
    c, formulas, scenes = scope(contract_path)
    remaining = {'binding': 1, 'formulas': len(formulas), 'scenes': len(scenes), 'proofs': len(PROOFS)}
    result = {'schema': 'mb3d.gpu-parallel-qualification.v1', 'errors': [], 'accepted': [],
              'contract_sha256': sha(contract_path), 'remaining_by_kind': remaining}
    manifest = base/'evidence.json'
    if not manifest.is_file():
        result['errors'].append('No evidence manifest; previews/builds do not qualify full scope')
    else:
        try:
            ev = read_json(manifest)
            require(ev['schema'] in (SCHEMA,INCREMENTAL_SCHEMA), 'Unknown evidence schema')
            incremental=ev['schema']==INCREMENTAL_SCHEMA
            if incremental:
                require(ev.get('artifact_root')=='repository','Unknown artifact root')
                base=ROOT

            b = ev['binding']
            require(b['contract_sha256'] == sha(contract_path) and b['source'] == source_snapshot(ROOT), 'Stale source/contract')
            if incremental:
                ctx=core_context(base,b,c,formulas,scenes,ROOT)
                ctx['cache_keys']={'resource_reuse':RESOURCE_KEYS,'result_reuse':RESULT_KEYS}
                try:
                    complete=context(base,b,c,formulas,scenes);ctx.update(complete);remaining['binding']=0
                except (ValueError,KeyError,TypeError,OSError,IndexError,AttributeError) as e:
                    result['binding_errors']=['Full binding incomplete: '+str(e)]
            else:
                ctx=context(base,b,c,formulas,scenes);remaining['binding']=0
            binding = digest(b)
            fs = unique(ev.get('formulas', []), formula_key, 'formula evidence')
            ss = unique(ev.get('scenes', []), lambda s:s['id'], 'scene evidence')
            ps = unique(ev.get('proofs', []), lambda s:s['kind'], 'proof')
            require(set(fs) <= set(formulas) and set(ss) <= set(scenes) and set(ps) <= set(PROOFS), 'Unknown scope member')
            def check(kind, identity, action):
                try:
                    action(); remaining[kind] -= 1; result['accepted'].append([kind, identity])
                except (ValueError, KeyError, TypeError, OSError, IndexError, AttributeError, OverflowError) as e:
                    result['errors'].append(str(identity)+': '+str(e))
            for ident, row in fs.items():
                def formula(row=row, ident=ident):
                    proof = receipt(base, row['proof'], 'formula_gpu_point_orbit', binding)
                    require(tuple(proof['identity']) == ident, 'Wrong formula proof')
                    formula_observations(base, proof['observations'], ident, binding)
                    observations=read_json(artifact(base,proof['observations']))
                    formula_repeats(base,observations['cases'],ident,ctx)
                    require(proof['state_contract_verified'] is True and proof['parameter_contract_verified'] is True,
                            'Unverified formula state/parameters')
                check('formulas', str(ident), formula)
            for ident, row in ss.items():
                def scene(row=row, ident=ident):
                    proof = receipt(base, row['effect_and_resource_proof'], 'scene_effects_resources', binding)
                    observations=read_json(artifact(base,proof['observations']))
                    scene_ctx=for_scene(ctx,ident)
                    outcome=row.get('qualification','QUALIFIED_GPU')
                    require(outcome in ('QUALIFIED_GPU','QUALIFIED_FP32_CPU'),'Unknown scene qualification')
                    if outcome=='QUALIFIED_FP32_CPU':
                        policy=c['policies'].get('whole_frame_fp32_fallback',{})
                        require(policy.get('accepted') is True and policy.get('other_fallbacks_accepted') is False,'CPU range acceptance not approved')
                        fp32_scene_frames(base,row,scenes[ident],scene_ctx,observations)
                    else:scene_frames(base,row,scenes[ident],scene_ctx,observations)
                    result.setdefault('scene_outcomes',{})[ident]=outcome
                    if incremental:result.setdefault('scene_resource_choices',{})[ident]={'profile':scene_ctx['resource_profile'],'selected_map_ids':scene_ctx['selected_map_ids']}
                check('scenes', ident, scene)
            for ident, row in ps.items():
                def proof(row=row, ident=ident):
                    p=receipt(base, row['artifact'], ident, binding)
                    data=read_json(artifact(base,p['observations']))
                    require(data['schema']=='mb3d.gpu-pipeline-proof.v1' and data['kind']==ident and data['binding_sha256']==binding,'Wrong observation schema/kind/binding')
                    VALIDATORS[ident](base,data,ctx)
                check('proofs', ident, proof)
        except (ValueError, KeyError, TypeError, OSError, IndexError, AttributeError, OverflowError) as e:
            result['errors'].append('manifest: '+str(e)); remaining['binding'] = 1
    outcomes=result.get('scene_outcomes',{})
    result['qualified_gpu_scenes']=sum(v=='QUALIFIED_GPU' for v in outcomes.values())
    result['qualified_fp32_cpu_scenes']=sum(v=='QUALIFIED_FP32_CPU' for v in outcomes.values())
    result['remaining'] = sum(remaining.values())
    result['status'] = 'PASS' if not result['remaining'] and not result['errors'] else 'OPEN'
    return result

def main():
    p = argparse.ArgumentParser(description=__doc__)
    for name in ('contract','evidence','output'): p.add_argument('--'+name, type=Path, required=True)
    a = p.parse_args(); a.output.mkdir(parents=True, exist_ok=True)
    try: result = evaluate(a.contract.resolve(), a.evidence.resolve())
    except (ValueError, KeyError, TypeError, OSError, IndexError, AttributeError, OverflowError) as e:
        result = {'status':'INVALID', 'remaining':556, 'errors':[str(e)]}
    import json
    (a.output/'qualification.json').write_text(json.dumps(result, indent=2, ensure_ascii=False)+'\n')
    print(result['status']+' remaining='+str(result['remaining']))
    return 0 if result['status'] == 'PASS' else 1

if __name__ == '__main__': raise SystemExit(main())
