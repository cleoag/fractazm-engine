#!/usr/bin/env python3
"""Assemble real incremental evidence; never manufacture missing global proofs."""
import argparse,json
from pathlib import Path
from qualify_native_metal_all import ROOT,sha,digest,source_snapshot,RECEIPT_SCHEMA,require,read_json
from qualify_gpu_parallel_pipeline import INCREMENTAL_SCHEMA,scope


def ref(path):
    path=Path(path).resolve();require(path.is_relative_to(ROOT),'Artifact outside repository archive')
    return {'path':str(path.relative_to(ROOT)),'sha256':sha(path)}


def write(path,value):
    path.parent.mkdir(parents=True,exist_ok=True)
    tmp=path.with_name(path.name+'.tmp');tmp.write_text(json.dumps(value,indent=2,ensure_ascii=False,allow_nan=False)+'\n');tmp.replace(path)


def refresh(package,contract,build):
    c,_,scenes=scope(contract);manifest=package/'evidence.json'
    ev=read_json(manifest) if manifest.exists() else {'schema':INCREMENTAL_SCHEMA,'artifact_root':'repository','formulas':[],'scenes':[],'proofs':[]}
    require(ev['schema']==INCREMENTAL_SCHEMA,'Cannot overwrite different manifest schema')
    old=ev.get('binding');b=dict(old or {})
    commands=read_json(build/'compile_commands.json')
    compiled_root=Path(next(r['file'] for r in commands if r['file'].endswith('/src/render/native_metal/general_renderer.mm'))).resolve().parents[3]
    require(compiled_root.is_dir() and compiled_root.is_relative_to(ROOT),'Missing/foreign compiled source root')
    b['candidate_compiled_source_root']=str(compiled_root.relative_to(ROOT))
    b.update(contract_sha256=sha(contract),source=source_snapshot(ROOT),threads=6,
             renderer=ref(build/'mb3d_render'),library=ref(build/'metal_native_scene.metallib'),
             runtime=str((package/'runtime').relative_to(ROOT)),
             default_resource_profile='original',resource_profiles={
                 'original':{'runtime':str((package/'runtime').relative_to(ROOT))},
                 'available':{'runtime':str((package/'runtime-selected').relative_to(ROOT))}},
             inputs={sid:ref(ROOT/s['path']) for sid,s in scenes.items()})
    if 'substitute1633' in c['resource_profiles']:
        b['resource_profiles']['substitute1633']={'runtime':str((package/'runtime-substitute1633').relative_to(ROOT))}
    selection=package/('map-selection-v4.json' if 'substitute1633' in c['resource_profiles'] else 'map-selection.json')
    if not selection.exists():
        # New packages are prepared entirely from tracked approved assets.
        # Preserve explicit historical selection records when they exist.
        selection=package/'resources.json'
        prepared=read_json(selection)
        require(prepared.get('schema')=='mb3d.prepared-scene-resources.v1' and
                prepared.get('qualification') is False and
                prepared.get('original_identity_claimed_for_selected_maps') is False,
                'Prepare approved resources before assembling a new package')
        for name,approved in c['resource_profiles'].items():
            profile=prepared.get('profiles',{}).get(name,{})
            require(profile.get('inventory_sha256')==approved['sha256'],
                    'Prepared resource inventory differs from contract: '+name)
            require(profile.get('runtime') is not None and
                    (package/profile['runtime']).resolve()==(ROOT/b['resource_profiles'][name]['runtime']).resolve(),
                    'Prepared resource runtime differs from package: '+name)
    b['map_selection']=ref(selection)
    assignments={sid:c.get('preferred_scene_resource_profiles',{}).get(sid,'available') for sid in scenes}
    assignments.update({sid:row.get('resource_profile','available') for sid,row in ((r['id'],r) for r in ev['scenes'])})
    b['scene_resources']=assignments
    if old and old!=b:
        require(not ev['formulas'] and not ev['proofs'],'Recollect global/formula bindings explicitly after source change')
        history=package/'manifest-history'/(digest(old)+'.json')
        if not history.exists():write(history,ev)
    ev['binding']=b;binding=digest(b)
    for row in ev['scenes']:
        observations=row['effect_observations'];require(sha(ROOT/observations['path'])==observations['sha256'],'Changed effect observations')
        receipt={'schema':RECEIPT_SCHEMA,'kind':'scene_effects_resources','status':'PASS','binding_sha256':binding,
                 'producer':row['producer'],'independent_reviewer':row['reviewer'],
                 'checks':[{'id':'raw_frames_are_revalidated_by_the_gate','status':'PASS'}],
                 'artifacts':[observations],'observations':observations}
        path=package/'receipts'/binding/(row['id']+'.json')
        if path.exists():require(read_json(path)==receipt,'Attempt to overwrite sealed receipt')
        else:write(path,receipt)
        row['effect_and_resource_proof']=ref(path)
    write(manifest,ev);return ev


def main():
    p=argparse.ArgumentParser(description=__doc__)
    for key in ('package','contract','build'):p.add_argument('--'+key,type=Path,required=True)
    p.add_argument('--import-fp32',type=Path);a=p.parse_args();package=a.package.resolve();build=a.build.resolve()
    if a.import_fp32:
        manifest=package/'evidence.json'
        ev=read_json(manifest) if manifest.exists() else {'schema':INCREMENTAL_SCHEMA,'artifact_root':'repository','formulas':[],'scenes':[],'proofs':[]}
        report=read_json(a.import_fp32/'report.json');require(report['status']=='PASS_FP32_CPU_SCENES','Unverified CPU scene report')
        contract,_,_=scope(a.contract.resolve())
        for entry in report['scenes']:
            profile=entry.get('resource_profile','original')
            require(profile in contract['scene_resource_profiles'][entry['id']], 'Unapproved CPU resource profile')
            payload=read_json(ROOT/entry['evidence']['path']);require(sha(ROOT/entry['evidence']['path'])==entry['evidence']['sha256'],'Changed CPU evidence')
            selected=sorted(set(contract['scene_required_maps'][entry['id']])&set(contract['resource_profiles'][profile]['candidate_map_ids']))
            data=dict(payload['row'],id=entry['id'],resource_profile=profile,selected_map_ids=selected,
                      producer=entry.get('producer','verify_fp32_cpu_acceptance.py'),
                      reviewer='fp32_overflow_witness.compare exact rational oracle and gpu_pipeline_fp32 raw-artifact validator')
            observations=package/'observations'/(entry['id']+'-effects.json');write(observations,payload['effects']);data['effect_observations']=ref(observations)
            existing=next((r for r in ev['scenes'] if r['id']==data['id']),None)
            if existing:require(existing['frames']==data['frames'],'Import would replace different CPU frames')
            else:ev['scenes'].append(data)
        write(manifest,ev)
    ev=refresh(package,a.contract.resolve(),build)
    print('Manifest assembled:',len(ev['scenes']),'scene records;',len(ev['formulas']),'formula records;',len(ev['proofs']),'global proofs. Gate must revalidate all records.')


if __name__=='__main__':main()
