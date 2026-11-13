"""Incremental full-contract bindings; incomplete global bindings never pass.

Scene checks need final host/library, immutable inputs and pinned resources.
Baseline/probe/tool bindings remain a separate mandatory global obligation.
"""
from pathlib import Path
from qualify_native_metal_all import require,artifact,contained,sha,digest
from gpu_pipeline_proofs import binary_identity,ORDINARY


def runtime_files(path):
    require(path.is_dir(),'Missing runtime')
    require(all(not p.is_symlink() for p in path.rglob('*')),'Resource alias forbidden')
    return {str(p.relative_to(path)):sha(p) for p in path.rglob('*') if p.is_file()}


def core_context(base,b,c,formulas,scenes,root):
    require(type(b['threads']) is int and 1<=b['threads']<=256,'Invalid logical worker count')
    ctx={'root':root,'source':b['source'],'formulas':set(formulas),'scenes':scenes,'binding_id':digest(b),
         'stages':c['compute_stages'],'threads':b['threads'],'training_inputs':set(),
         'qualified_scenes':{},'qualified_fp32_cpu_scenes':{}}
    if 'map_selection' in b:artifact(base,b['map_selection'])
    ctx['renderer']=binary_identity(base,b['renderer']);ctx['library']=b['library'];ctx['library_path']=artifact(base,b['library'])
    ctx['build_dir']=Path(ctx['renderer']['path']).parent
    require(set(b['inputs'])==set(scenes),'Incomplete canonical scene mapping')
    ctx['inputs']={sid:artifact(base,r) for sid,r in b['inputs'].items()}
    for sid,p in ctx['inputs'].items():
        require(sha(p)==scenes[sid]['sha256'],'Input scene differs')
        require(not (p.parent/'M3Maps').exists(),'Scene-local resource shadowing')
    profiles=c['_resource_profiles'];require(set(b['resource_profiles'])==set(profiles),'Resource profile inventory differs')
    ctx['resource_profiles']={}
    for name,entry in b['resource_profiles'].items():
        path=contained(base,entry['runtime']);actual=runtime_files(path)
        require(actual==profiles[name]['files'],'Resource profile differs from pinned selection')
        ctx['resource_profiles'][name]={'runtime':path,'runtime_files':actual,'candidate_map_ids':profiles[name]['candidate_map_ids']}
    require(set(b['scene_resources'])==set(scenes),'Incomplete scene resource assignment')
    for sid,name in b['scene_resources'].items():require(name in c['scene_resource_profiles'][sid] and name in profiles,'Unapproved scene resource profile')
    ctx['scene_resources']=b['scene_resources'];ctx['scene_required_maps']=c['scene_required_maps']
    default=b['default_resource_profile'];require(default in profiles,'Unknown default resource profile')
    ctx.update(ctx['resource_profiles'][default]);ctx['runtime_default_profile']=default
    # Future proof validators must fail on absent tools rather than use
    # fabricated placeholders. Their missing binding remains counted globally.
    ctx['tools']={k:binary_identity(base,v) for k,v in b.get('tools',{}).items()}
    if 'ordinary_commands' in b:
        commands=b['ordinary_commands']
        require(isinstance(commands,dict) and set(commands)==ORDINARY,'Missing ordinary command mapping')
        require(all(isinstance(a,list) and a and all(isinstance(v,str) for v in a) for a in commands.values()),'Malformed ordinary command')
        ctx['ordinary_commands']=commands
    return ctx


def for_scene(ctx,sid):
    if 'resource_profiles' not in ctx:return ctx
    name=ctx['scene_resources'][sid];r=ctx['resource_profiles'][name];required=set(ctx['scene_required_maps'][sid]);available=[]
    for path in r['runtime_files']:
        p=Path(path)
        if p.parent==Path('M3Maps') and p.suffix.lower() in ('.png','.jpg','.jpeg','.bmp') and p.stem.isdigit():available.append(int(p.stem))
    require(len(available)==len(set(available)),'Ambiguous numbered map lookup')
    require(required<=set(available),'Missing required scene maps: '+str(sorted(required-set(available))))
    result=dict(ctx);result.update(r);result['resource_profile']=name
    result['selected_map_ids']=sorted(required&set(r['candidate_map_ids']))
    return result
