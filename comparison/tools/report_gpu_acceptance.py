#!/usr/bin/env python3
"""Refresh a clearly marked working report from actual common-gate counters."""
import html,json,os,re,statistics,subprocess
from pathlib import Path
from qualify_native_metal_all import ROOT,read_json,digest,artifact,require
from prepare_gpu_parallel_bundle import write


def resource_summary(binding, scene_total):
    """Summarize pinned availability metadata, never rendering qualification."""
    unknown = {'status': 'UNVERIFIED', 'qualification': False,
               'installed_maps': None, 'requested_maps': None,
               'scenes_with_available_maps': None, 'scene_total': scene_total,
               'missing_map_ids': [], 'selected_candidates': [], 'substitutions': [],
               'original_identity_claimed_for_selected_maps': False}
    if 'map_selection' not in binding:
        return unknown | {'reason': 'Resource metadata is absent'}
    selection = read_json(artifact(ROOT, binding['map_selection']))
    if selection.get('schema') == 'mb3d.prepared-scene-resources.v1':
        require(selection.get('qualification') is False and
                selection.get('original_identity_claimed_for_selected_maps') is False,
                'Prepared resources must not claim qualification or original selected-map identity')
        metadata_path = 'comparison/resources/phase1-selected/selection.json'
        metadata = read_json(artifact(ROOT, {'path': metadata_path,
                                            'sha256': selection['selection_sha256']}))
        require(metadata['contract_sha256'] == binding.get('contract_sha256'),
                'Prepared resources use a different acceptance contract')
        contract = read_json(artifact(ROOT, {'path': metadata['contract'],
                                            'sha256': metadata['contract_sha256']}))
        profiles = selection.get('profiles')
        require(isinstance(profiles, dict) and set(profiles) == set(metadata['profiles']),
                'Unknown or missing prepared resource profile')
        profile_ids = {}
        for name, profile in profiles.items():
            pin = metadata['profiles'][name]
            approved = contract['resource_profiles'][name]
            require((ROOT / pin['inventory']).resolve() ==
                    (ROOT / metadata['contract']).parent.joinpath(approved['inventory']).resolve()
                    and pin['inventory_sha256'] == approved['sha256'],
                    'Prepared resource inventory is not approved by the contract')
            inventory = read_json(artifact(ROOT, {'path': pin['inventory'], 'sha256': pin['inventory_sha256']}))
            require(profile.get('inventory') == pin['inventory'] and
                    profile.get('inventory_sha256') == pin['inventory_sha256'] and
                    profile.get('files') == inventory, 'Prepared resource inventory differs from pinned bytes')
            profile_ids[name] = {int(Path(path).stem) for path in inventory
                                 if Path(path).parent == Path('M3Maps') and Path(path).stem.isdecimal()}
        prepared = selection.get('scene_resource_profiles')
        required = contract['scene_required_maps']
        require(isinstance(prepared, dict) and set(prepared) == set(required) and
                type(selection.get('scenes_with_available_resources')) is int and
                selection['scenes_with_available_resources'] == len(prepared) == scene_total,
                'Prepared scene availability count/assignment differs')
        for sid, profile in prepared.items():
            require(profile in profiles and profile in contract['scene_resource_profiles'][sid] and
                    set(required[sid]) <= profile_ids[profile], 'Prepared scene lacks its required maps')
        assignments = binding.get('scene_resources', prepared)
        require(isinstance(assignments, dict) and set(assignments) == set(required), 'Incomplete bound scene resources')
        missing = set()
        available = 0
        for sid, profile in assignments.items():
            require(profile in profiles and profile in contract['scene_resource_profiles'][sid],
                    'Unknown or unapproved bound scene resource profile')
            absent = set(required[sid]) - profile_ids[profile]
            missing |= absent
            available += not absent
        requested = set().union(*(set(ids) for ids in required.values()))
        installed = set().union(*profile_ids.values()) & requested
        substitutions = []
        for item in metadata.get('substitutions', []):
            require(item.get('status') == 'USER_APPROVED_SUBSTITUTE' and
                    item.get('original_identity_claimed') is False, 'Unapproved or mislabelled selected substitute')
            if any(item['requested_id'] in required[sid] and
                   any(Path(path).stem == str(item['requested_id']) and h == item['sha256']
                       for path, h in profiles[profile]['files'].items())
                   for sid, profile in assignments.items()):
                substitutions.append(item)
        return unknown | {'status': 'PINNED_PREPARED_PROFILES', 'installed_maps': len(installed),
                          'requested_maps': len(requested), 'scenes_with_available_maps': available,
                          'missing_map_ids': sorted(missing), 'substitutions': substitutions,
                          'selected_candidates': metadata.get('selected_candidates', [])}
    legacy = {'installed_maps', 'scenes_with_available_maps', 'missing_map_ids'}
    if not legacy <= set(selection):
        return unknown | {'reason': 'Resource metadata schema is not recognized'}
    installed, available, requested = (selection['installed_maps'], selection['scenes_with_available_maps'],
                                       selection.get('requested_maps'))
    require(type(installed) is int and installed >= 0 and type(available) is int and
            0 <= available <= scene_total and
            (requested is None or (type(requested) is int and requested >= installed)),
            'Invalid legacy resource availability counts')
    require(isinstance(selection['missing_map_ids'], list) and
            all(type(x) is int and x >= 0 for x in selection['missing_map_ids']), 'Invalid missing-map list')
    substitutions = selection.get('substitutions', [])
    require(isinstance(substitutions, list) and all(item.get('status') == 'USER_APPROVED_SUBSTITUTE' and
            item.get('original_identity_claimed') is False for item in substitutions), 'Unapproved or mislabelled legacy substitute')
    return unknown | {'status': 'PINNED_LEGACY_METADATA', 'installed_maps': installed,
                      'requested_maps': requested, 'scenes_with_available_maps': available,
                      'missing_map_ids': selection['missing_map_ids'], 'substitutions': substitutions,
                      'selected_candidates': selection.get('selected_candidates', [])}


def resource_description(summary):
    if summary['status'] == 'UNVERIFIED':
        return 'Доступность карт не проверена: сведения о ресурсах отсутствуют или их схема не поддерживается.'
    maps = str(summary['installed_maps'])
    if summary['requested_maps'] is not None:maps += '/' + str(summary['requested_maps'])
    text = f"Доступны {maps} номеров карт; обеспечены {summary['scenes_with_available_maps']}/{summary['scene_total']} сцен."
    if summary['selected_candidates']:
        text += ' Выбранные MI-карты являются доступными заменами, а не подтверждёнными оригиналами.'
    for item in summary['substitutions']:
        text += f" Карта {item['requested_id']} явно заменена картой {item['selected_source_id']} с разрешения пользователя; сходство с оригиналом не утверждается."
    if summary['missing_map_ids']:
        text += ' В выбранных профилях недоступны карты: ' + ', '.join(map(str, summary['missing_map_ids'])) + '.'
    return text


def publish(package, *, output=None, write_plan=True):
    q=read_json(package/'qualification.json');ev=read_json(package/'evidence.json')
    queue={r['id']:r for r in read_json(ROOT/'comparison/scene-queue.json')['rows']}
    selection=resource_summary(ev['binding'],len(queue))
    resource_text=resource_description(selection)
    out=Path(output).resolve() if output is not None else ROOT/'comparison/results/native-common-acceptance-current'
    if not out.is_relative_to(ROOT/'comparison/results'):raise ValueError('Report must remain inside comparison/results')
    out.mkdir(parents=True,exist_ok=True)
    cpu_ids={r['id'] for r in ev['scenes'] if r.get('qualification')=='QUALIFIED_FP32_CPU'}
    cases=sorted([c for p in (package/'scene-runs').glob('*/case.json')
                  if (c:=read_json(p))['id'] not in cpu_ids],key=lambda c:c['rank'])
    for row in ev['scenes']:
        if row.get('qualification')=='QUALIFIED_FP32_CPU':
            entry=queue[row['id']]
            cases.append(dict(id=row['id'],rank=entry['rank'],scene=entry['scene'],status='QUALIFIED_FP32_CPU',
                              cpu_reference=row['direct_cpu'],cpu_fallback_frames=row['frames'],selected_map_ids=row.get('selected_map_ids',[])))
    cases.sort(key=lambda c:c['rank'])
    accepted=len(q.get('scene_outcomes',{}));rows=[];summaries=[]
    def link(path,label):return '<a href="'+html.escape(os.path.relpath(ROOT/path,out),quote=True)+'">'+html.escape(label)+'</a>'
    for case in cases:
        frames=case.get('frames',[]);timings=[];archives=[]
        for frame in frames:
            p=read_json(ROOT/frame['profile']['path']);timings.append(p['render_total_seconds']);archives.append(p['metal_execution']['pipeline_archive'])
        cpu=case.get('cpu_reference');cpu_seconds=read_json(ROOT/cpu['profile']['path'])['render_total_seconds'] if cpu else None
        used=case.get('selected_map_ids',[]);name=Path(case['scene']).stem
        img=link(frames[0]['image']['path'],'GPU') if frames else '—'
        if cpu:img+=' / '+link(cpu['image']['path'],'CPU')
        median=statistics.median(timings) if timings else None
        summaries.append({k:case.get(k) for k in ('rank','id','scene','status','reason','active_job','selected_map_ids','cpu_difference','cpu_reference_status')}|
                         {'gpu_render_seconds':timings,'gpu_archives':archives,'cpu_render_seconds':cpu_seconds,
                          'fp32_cpu_repeat_seconds':[read_json(ROOT/f['profile']['path'])['render_total_seconds'] for f in case.get('cpu_fallback_frames',[])]})
        rows.append('<tr><td>'+str(case['rank'])+'</td><td>'+html.escape(name)+'</td><td>'+html.escape(case['status']+(' · '+case['active_job']['label'] if case.get('active_job') else ''))+
                    '</td><td>'+('/'.join(map(str,used)) or 'исходные')+'</td><td>'+(f'{median:.3f}' if median else '—')+
                    '</td><td>'+(f'{cpu_seconds:.3f}' if cpu_seconds else '—')+'</td><td>'+img+'</td></tr>')
    report={'schema':'mb3d.common-acceptance-working-report.v1','working_report':True,'collection_source_sha256':digest(ev['binding']['source']),'compiled_source_root':ev['binding'].get('candidate_compiled_source_root'),'collection_commit':subprocess.check_output(['git','rev-parse','HEAD'],cwd=ROOT,text=True).strip(),
            'qualification':q,'renderer':ev['binding']['renderer'],'library':ev['binding']['library'],'scenes':summaries,'resources':selection,
            'limitations':['Full contract remains OPEN','Selected MI maps are replacements, not authenticated originals',resource_text,'CPU/GPU single-reference speed ratios are not the balanced performance gate']}
    write(out/'report.json',report)
    title=f'Общая приёмка: {accepted}/77 сцен — {accepted/77:.1%}'
    (out/'index.html').write_text('<!doctype html><html lang="ru"><meta charset="utf-8"><meta http-equiv="refresh" content="30"><title>'+title+
        '</title><style>body{font:16px system-ui;margin:32px;color:#dde5ee;background:#111822}table{border-collapse:collapse;width:100%}td,th{text-align:left;padding:9px;border-bottom:1px solid #34404d}a{color:#80c9ff}p{max-width:1000px}progress{width:100%;height:20px}</style>'+
        '<h1>'+title+'</h1><progress max="77" value="'+str(accepted)+'"></progress><p>GPU: '+str(q['qualified_gpu_scenes'])+' · FP32 → native CPU: '+str(q['qualified_fp32_cpu_scenes'])+
        '. Полный контракт: <b>'+q['status']+'</b>. Это обновляемый рабочий отчёт, не финальная приёмка.</p>'+
        '<p>Исходные размеры и эффекты, три точных повтора RGB и работы. '+resource_text+'</p>'+
        '<p>В таблице — медиана полного GPU-рендера и одиночный CPU-эталон, секунды. Это ещё не сбалансированный тест ускорения. Холодные прогоны вынесены в исходные записи; состояние архива есть в report.json. Незакрытые численные вопросы и пропущенные CPU-эталоны остаются OPEN.</p>'+
        '<p>'+link('docs/NATIVE_ACCEPTANCE_PLAN.md','План с отметками')+' · '+link(str((out/'report.json').relative_to(ROOT)),'Числа и оставшиеся обязательства')+'</p>'+
        '<table><tr><th>№</th><th>Сцена</th><th>Результат</th><th>Выбранные карты</th><th>GPU, с</th><th>CPU, с</th><th>Картинки</th></tr>'+''.join(rows)+'</table></html>')
    if not write_plan:return
    p=ROOT/'docs/NATIVE_ACCEPTANCE_PLAN.md';s=p.read_text()
    resource_state='UNVERIFIED: resource metadata absent or unsupported'
    if selection['status'] != 'UNVERIFIED':
        maps=str(selection['installed_maps'])
        if selection['requested_maps'] is not None:maps+='/'+str(selection['requested_maps'])
        complete=(selection['requested_maps'] == selection['installed_maps'] and
                  selection['scenes_with_available_maps'] == selection['scene_total'] and not selection['missing_map_ids'])
        resource_state=('Done: ' if complete else 'OPEN: ')+f"{maps} map IDs supplied, {selection['scenes_with_available_maps']}/{selection['scene_total']} scenes"
    s=re.sub(r'\| Resource selection \|[^\n]+',f"| Resource selection | {resource_state} | Pin selected replacements separately; preserve prior resource profiles and document original-identity limits |",s)
    s=re.sub(r'\| Full ordinary scenes \|[^\n]+',
        f'| Full ordinary scenes | {accepted}/77 ({accepted/77:.1%}): {q["qualified_gpu_scenes"]} GPU + {q["qualified_fp32_cpu_scenes"]} FP32 CPU | Original sizes/effects, three exact repeats and on/off controls on final identities |',s)
    verified=['<!-- BEGIN VERIFIED SCENES -->','', '## Qualified scenes on the current identity','']
    for sid,outcome in sorted(q.get('scene_outcomes',{}).items(),key=lambda item:queue[item[0]]['rank']):
        verified.append('- [x] '+Path(queue[sid]['scene']).stem+' — '+('GPU' if outcome=='QUALIFIED_GPU' else 'proven FP32 → native CPU'))
    verified+=['','<!-- END VERIFIED SCENES -->']
    block='\n'.join(verified)
    if '<!-- BEGIN VERIFIED SCENES -->' in s:s=re.sub(r'<!-- BEGIN VERIFIED SCENES -->.*?<!-- END VERIFIED SCENES -->',lambda _:block,s,flags=re.S)
    else:s+='\n'+block+'\n'
    p.write_text(s)


if __name__=='__main__':
    import argparse
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--package',required=True,type=Path)
    p.add_argument('--output',type=Path);p.add_argument('--no-plan',action='store_true');a=p.parse_args()
    publish(a.package.resolve(),output=a.output,write_plan=not a.no_plan)
