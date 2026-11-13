"""Passive campaign004 coverage validation. Never dispatches or grants scene credit.

Review records are auditable judgments, not cryptographic attestations. Numerical
proof still replays the captured device processes and the ordinary common gate.
Unknown observation/control capabilities remain OPEN, never implicitly supported.
"""
from pathlib import Path
import re
from qualify_native_metal_all import artifact, digest, read_json, require, sha
from gpu_pipeline_proofs import STATE

CATEGORIES = ('source_semantics','parameters_preparation','authored_defaults','scene_contexts',
              'branches_boundaries','state','callers_hybrids','rounded_finite','resources','range_faults')
HASH = re.compile(r'[0-9a-f]{64}\Z')
PRODUCERS = {'verify_native_formula_group.py','native_formula_coverage.py'}


def ref(root,path):
    path=Path(path).resolve();root=Path(root).resolve()
    require(path.is_relative_to(root),'Coverage artifact escapes root')
    return {'path':str(path.relative_to(root)),'sha256':sha(path)}


def fields(value,names,label):
    require(isinstance(value,dict) and set(value)==set(names.split()),'Invalid '+label+' fields')


def text(value,label):
    require(isinstance(value,str) and bool(value.strip()),'Missing '+label)


def strings(values,label,empty=True):
    require(isinstance(values,list) and all(isinstance(x,str) and x for x in values)
            and len(set(values))==len(values) and (empty or bool(values)),'Invalid '+label)
    return set(values)


def load(root,record,schema=None):
    fields(record,'path sha256','artifact reference')
    data=read_json(artifact(root,record))
    if schema:require(isinstance(data,dict) and data.get('schema')==schema,'Wrong coverage artifact schema')
    return data


def refs(root,values,empty=False):
    require(isinstance(values,list) and (empty or bool(values)),'Missing coverage source references')
    for value in values:
        fields(value,'path sha256','artifact reference');artifact(root,value)
    require(len({r['path'] for r in values})==len(values),'Duplicate coverage artifact')
    return values


def reviewer(data,author):
    text(data['reviewer'],'independent reviewer')
    require(data['reviewer']!=author and data['reviewer'] not in PRODUCERS,'Self-produced coverage approval')


def unique(rows,label):
    require(isinstance(rows,list),'Expected '+label+' list')
    result={}
    for row in rows:
        require(isinstance(row,dict),'Malformed '+label)
        text(row.get('id'),'obligation/case ID')
        require(row['id'] not in result,'Duplicate '+label+' ID');result[row['id']]=row
    return result


def authoritative_inventory(root):
    path=Path(root)/'comparison/contracts/native-gpu-acceptance-v4/acceptance-contract.json'
    contract=read_json(path);inventory=(path.parent/contract['inventory']).resolve()
    record=ref(root,inventory)
    require(record['sha256']==contract['inventory_sha256'],'Pinned acceptance inventory changed')
    return record,read_json(inventory),ref(root,path)


def tool_inputs(root):
    return [ref(root,Path(__file__).resolve()),
            ref(root,Path(__file__).with_name('verify_native_formula_group.py').resolve())]


def validate_declaration(root,record):
    d=load(root,record,'mb3d.formula-coverage-declaration.v1')
    fields(d,'schema identity author inventory sources defaults context_ids categories cases','declaration')
    text(d['author'],'declaration author')
    identity=d['identity']
    require(isinstance(identity,list) and len(identity)==3 and identity[0] in ('external','internal')
            and type(identity[1]) is int and isinstance(identity[2],str),'Malformed coverage identity')
    inventory=load(root,d['inventory'],'mb3d.formula-obligation-inventory.v1')
    require(inventory['identity']==identity,'Inventory identity differs')
    refs(root,d['sources']);refs(root,d['defaults'])
    require(d['sources']==inventory['sources'] and d['defaults']==inventory['defaults'],
            'Source/default inventory removed or changed')
    canonical,authority,contract=authoritative_inventory(root)
    require(inventory['scene_inventory']==canonical,'Scene inventory differs from pinned acceptance authority')
    require(any([row['namespace'],row['id'],row['name']]==identity for row in authority['formulas']),
            'Unknown ordinary formula identity')
    scenes=authority['scenes']
    require(len(scenes)==77 and len({s['id'] for s in scenes})==77,'Full77 scene inventory required')
    contexts=unique(inventory['contexts'],'realized context')
    require(strings(d['context_ids'],'declared contexts')==set(contexts),'Realized scene contexts omitted')
    inputs=[record,d['inventory'],inventory['scene_inventory'],contract,*d['sources'],*d['defaults']]
    if 'scene_contexts_index' in inventory:
        inputs+=refs(root,[inventory['scene_contexts_index']])
        index=load(root,inventory['scene_contexts_index'],'mb3d.original-settings-scene-contexts.v1')
        require(index['scene_count']==77 and index['scene_inventory']==canonical,'Wrong realized scene index authority')
        all_contexts=unique(index['contexts'],'scene index context')
        key=f'{identity[0]}:{identity[1]}'
        expected={cid:ctx for cid,ctx in all_contexts.items() if key in ctx['formula_keys']}
        require(contexts==expected,'Formula contexts differ from full realized scene index')
    for context in contexts.values():
        require(strings(context['scene_ids'],'context scenes',False)<={s['id'] for s in scenes},'Unknown context scene')
        for key in ('parameters_sha256','program_sha256','resources_sha256'):
            require(isinstance(context[key],str) and HASH.fullmatch(context[key]),'Invalid context hash')
        require(context['capability'] in ('single_slot','mixed','combined'),'Unknown context capability')
        inputs+=refs(root,context['artifacts'])
    authored={scene['id'] for scene in scenes for slot in scene['slots']
              if slot['key']==f'{identity[0]}:{identity[1]}' and slot['iterations']>0}
    realized={scene for ctx in contexts.values() for scene in ctx['scene_ids']}
    require(authored<=realized,'Authored scene usage missing from realized inventory')
    require(isinstance(d['categories'],dict) and set(d['categories'])==set(CATEGORIES),'Missing mandatory categories')
    cases=unique(d['cases'],'declared case');obligations={}
    for case in cases.values():
        fields(case,'id input_sha256 capability context_ids fields domains','case mapping')
        require(isinstance(case['input_sha256'],str) and HASH.fullmatch(case['input_sha256']),'Invalid input mapping hash')
        require(strings(case['context_ids'],'case contexts')<=set(contexts),'Undeclared case context')
        require(strings(case['fields'],'observed fields',False)<=set(STATE),'Unobserved case field')
        require(strings(case['domains'],'case domains',False)<=set(CATEGORIES),'Unknown case domain')
        text(case['capability'],'observation capability')
        for ctx in case['context_ids']:
            require(case['capability']==contexts[ctx]['capability'],'Wrong capability for realized context')
    for category,rows in d['categories'].items():
        require(isinstance(rows,list) and rows,'Empty required category')
        for row in rows:
            fields(row,'id applicability source_locations condition context_ids required_fields capability evidence_kind case_ids na_reason','obligation')
            text(row['id'],'obligation ID');text(row['condition'],'source condition')
            require(row['id'] not in obligations,'Duplicate obligation ID')
            row=dict(row,category=category);obligations[row['id']]=row
            require(row['applicability'] in ('REQUIRED','NOT_APPLICABLE'),'Unknown applicability')
            require(isinstance(row['source_locations'],list) and row['source_locations'],'Missing source locations')
            for loc in row['source_locations']:
                fields(loc,'artifact location','source location');text(loc['location'],'original location')
                require(loc['artifact'] in d['sources']+d['defaults'],'Source location outside declared originals')
            require(strings(row['context_ids'],'obligation contexts')<=set(contexts),'Unknown obligation context')
            require(strings(row['required_fields'],'required fields')<=set(STATE),'Unobserved required field')
            require(strings(row['case_ids'],'mapped cases')<=set(cases),'Unknown case ID')
            text(row['capability'],'required capability')
            require(row['evidence_kind'] in ('source','device','probe_integrity','actual_control'),'Unknown evidence kind')
            if row['applicability']=='NOT_APPLICABLE':
                text(row['na_reason'],'source-backed N/A reason')
                require(category in ('scene_contexts','resources') and not row['case_ids'] and not row['required_fields'],
                        'Unsupported N/A for ordinary required behavior')
                if category=='scene_contexts':require(not contexts,'Realized scene context cannot be N/A')
            else:
                require(row['na_reason']=='','Required obligation has N/A reason')
                require(row['evidence_kind']!='source' or category=='source_semantics','Numerical obligation needs device evidence')
            for key in row['case_ids']:
                case=cases[key]
                require(category in case['domains'] and row['capability']==case['capability'] and
                        set(row['context_ids'])<=set(case['context_ids']) and set(row['required_fields'])<=set(case['fields']),
                        'Case domain/capability/field mapping differs')
    if contexts:
        required={ctx for row in obligations.values() if row['category']=='scene_contexts'
                  and row['applicability']=='REQUIRED' for ctx in row['context_ids']}
        require(required==set(contexts),'Required scene context obligation omitted')
    return d,obligations,cases,contexts,inputs


def validate_precollection(root,declaration,completeness,model_review,corpus,binding):
    d,obligations,cases,contexts,inputs=validate_declaration(root,declaration)
    r=load(root,completeness,'mb3d.formula-coverage-review.v1')
    fields(r,'schema declaration reviewer status reviewed_obligation_ids approved_na_ids rationale','completeness review')
    reviewer(r,d['author']);text(r['rationale'],'completeness rationale')
    require(r['declaration']==declaration and r['status']=='PASS','Completeness review does not bind declaration')
    require(strings(r['reviewed_obligation_ids'],'reviewed obligations')==set(obligations) and
            strings(r['approved_na_ids'],'approved N/A')=={k for k,v in obligations.items() if v['applicability']=='NOT_APPLICABLE'},
            'Incomplete obligation or N/A review')
    m=load(root,model_review,'mb3d.formula-model-review.v1')
    fields(m,'schema declaration reviewer status reference_model reference_manifest reference_dependencies corpus_sha256 reviewed_case_ids derivation observed_fields representation exclusions','model review')
    reviewer(m,d['author']);text(m['derivation'],'independent reference derivation')
    require(m['status']=='PASS' and m['declaration']==declaration,'Model review does not bind declaration')
    require(all(m[key]==binding[key] for key in ('reference_model','reference_manifest','reference_dependencies')),
            'Model/helper/manifest changed after review')
    inputs+=refs(root,m['reference_dependencies'])
    require(m['reference_model'] in m['reference_dependencies'],'Model absent from executable dependency closure')
    require(m['representation']=='full_state_binary32' and strings(m['observed_fields'],'model fields')==set(STATE),
            'Full raw state derivation required; partial/decimal observation insufficient')
    strings(m['exclusions'],'model exclusions')
    originals=unique(corpus,'corpus case')
    require(set(originals)==set(cases)==strings(m['reviewed_case_ids'],'reviewed cases') and
            m['corpus_sha256']==digest(corpus),'Changed/incomplete reviewed corpus')
    for key,original in originals.items():
        require(original['identity']==d['identity']==original['input']['identity'],'Wrong case identity')
        require(digest(original['input'])==cases[key]['input_sha256'],'Case mapping input changed')
        require(set(original['expected_state_bits'])==set(STATE),'Incomplete model target state')
        if cases[key]['capability'] == 'mixed':
            import native_formula_observer as observer
            import native_formula_context_input as context_input
            observer.validate_case(original['input'])
            require(original['input'].get('context') == context_input.descriptor(root) and
                    cases[key]['context_ids'] == [context_input.CONTEXT_ID],
                    'Mixed case lacks exact original context binding')
            context_input.validate_schedule(original)
            inputs += [ref(root,p) for p in observer.dependency_paths(original['input'])]
        else:
            require(cases[key]['capability']=='single_slot','Unsupported case observation capability')
            require(not cases[key]['context_ids'] and 'context' not in original['input'],
                    'Unsupported scene-to-probe context adapter; required context remains OPEN')
        if 'resources' in original['input']:
            require(any(row['category']=='resources' and row['applicability']=='REQUIRED' and
                        key in row['case_ids'] for row in obligations.values()),
                    'Actual resource-bearing case cannot omit required resource obligation')
    inputs+=[completeness,model_review]
    by_path={}
    for value in inputs:
        require(value['path'] not in by_path or value==by_path[value['path']],'Conflicting coverage input hashes')
        by_path[value['path']]=value
    return {'coverage_declaration':declaration,'coverage_review':completeness,'model_review':model_review,
            'corpus_sha256':digest(corpus),'coverage_inputs':[by_path[k] for k in sorted(by_path)]}


def validate_event_inputs(event,bundle):
    require(all(r in event['inputs'] for r in bundle['coverage_inputs']),
            'Predeclared coverage/model/source inputs were not captured')


def validate_scene_disposition(root,record,d,candidate):
    value=load(root,record,'mb3d.formula-scene-disposition.v1')
    fields(value,'schema identity candidate reviewer rationale contexts evidence','scene disposition')
    reviewer(value,d['author']);text(value['rationale'],'affected-scene disposition')
    require(value['identity']==d['identity'] and value['candidate']==candidate,'Wrong scene disposition binding')
    rows=unique(value['contexts'],'scene disposition')
    require(set(rows)==set(d['context_ids']),'Missing affected-scene disposition')
    refs(root,value['evidence'],empty=not rows)
    for row in rows.values():
        fields(row,'id status reason evidence','context disposition');text(row['reason'],'scene attribution')
        require(row['status'] in ('NO_NEW_CONTROL_REQUIRED','UNRELATED_OPEN'),'Actual scene controls need a supported proof validator')
        refs(root,row['evidence'])
    return value


def validate_coverage(root,record):
    """Replay actual numerical evidence, then derive the supplied decision status."""
    import verify_native_formula_group as v
    import native_formula_observer as observer
    decision=load(root,record,'mb3d.formula-coverage-decision.v1')
    fields(decision,'schema declaration completeness_review model_review collection reviewer identity candidate obligations scene_disposition status','coverage decision')
    d,obligations,cases,contexts,_=validate_declaration(root,decision['declaration'])
    reviewer(decision,d['author']);require(decision['identity']==d['identity'],'Wrong coverage identity')
    data=load(root,decision['collection'],'mb3d.native-formula-group.v1')
    require(data.get('evidence_origin')=='actual_device_capture','Synthetic/unmarked numerical evidence forbidden')
    require(data['status']=='COLLECTED_REVIEW_REQUIRED','Incomplete collection')
    candidate={key:data[key] for key in ('library','probe','observer','build_receipt')}
    require(data.get('coverage_tools')==tool_inputs(root),'Coverage/collector tool closure changed')
    require(decision['candidate']==candidate,'Coverage candidate differs from actual collection')
    for r in candidate.values():artifact(root,r)
    reference=v.load_reference(data['reference_group'])
    v.validate_reference(data,reference)
    corpus=[r for r in reference.build_cases() if r['identity']==d['identity']]
    bundle=validate_precollection(root,decision['declaration'],decision['completeness_review'],decision['model_review'],corpus,data)
    require(all(data.get(k)==val for k,val in bundle.items()),'Collection lacks exact predeclared coverage binding')
    v.validate_build_products(artifact(root,data['build_receipt']),artifact(root,data['probe']),artifact(root,data['library']))
    v.validate_corpus(data,{tuple(d['identity'])},corpus)
    rows=unique(decision['obligations'],'coverage outcome')
    require(set(rows)==set(obligations),'Missing/unknown obligation outcomes')
    collected=unique(data['cases'],'collected case');observed={};used=set()
    require(set(collected)==set(cases) and all(row['identity']==d['identity'] for row in collected.values()),
            'Collection contains undeclared or misidentified cases')
    def check_case(key):
        if key in observed:return
        row=collected[key];payload=load(root,row['input'])
        require(row['kind']==payload['kind'],'Collected case kind differs from actual input')
        require(row['status']=='PASS_CASE' and len(row['repeats'])==3,'Incomplete successful device repeats')
        require(reference.evaluate(payload)==row['expected_state_bits']==row['gpu_state_bits'] and
                set(row['gpu_state_bits'])==set(STATE),'Independent full state mismatch')
        require(len(row['result_words'])==59,'Incomplete repeat words')
        for repeat in row['repeats']:
            event=load(root,repeat['event']);validate_event_inputs(event,bundle)
            require(all(r in event['inputs'] for r in data['coverage_tools']),'Coverage tools were not captured')
            v.validate_reference_event(event,data['reference_dependencies'])
            require(event['run_id'] not in used,'Reused formula process');used.add(event['run_id'])
            actual=observer.replay_observation(root,repeat['observation'],repeat['event'],payload,data['probe'],data['library'])
            require('synthetic' not in actual['device'].lower(),'Synthetic device fixture is not numerical evidence')
            require(actual['state_bits']==row['gpu_state_bits'] and actual['result_words']==row['result_words'],
                    'Actual47/59 state or repeat words differ')
        observed[key]=row
    passed_device=False;complete=True
    for key,row in rows.items():
        fields(row,'id status case_ids reason','coverage outcome')
        declared=obligations[key]
        require(row['case_ids']==declared['case_ids'],'Changed obligation case mapping')
        if declared['applicability']=='NOT_APPLICABLE':
            require(row['status']=='NOT_APPLICABLE','N/A outcome differs');continue
        require(row['status'] in ('PASS','OPEN'),'Invalid required obligation outcome')
        if row['status']=='OPEN':text(row['reason'],'unresolved obligation');complete=False;continue
        kind=declared['evidence_kind']
        if kind=='source':continue  # Reviewed original-source judgment, not device credit.
        require(kind in ('device','probe_integrity'),'Unsupported actual control evidence remains OPEN')
        require(row['case_ids'],'No actual cases for required obligation')
        for cid in row['case_ids']:
            check_case(cid)
            if kind=='probe_integrity':
                for repeat in collected[cid]['repeats']:
                    actual=load(root,repeat['observation'])
                    require(actual['invalid_descriptor_controls']==7 and actual['allocation_canaries'] is True
                            and actual['input_immutable'] is True,'Actual probe integrity controls missing')
        passed_device=True
    if complete:
        # Completion retains the existing whole-corpus point AND orbit gate;
        # extra reviewed cases cannot be silently omitted from actual replay.
        for cid in cases:check_case(cid)
        require({row['kind'] for row in collected.values()}=={'point','orbit'},
                'Complete formula coverage requires point and orbit observations')
    validate_scene_disposition(root,decision['scene_disposition'],d,candidate)
    expected='COVERAGE_COMPLETE' if complete else ('SCOPED_PROOF' if passed_device else 'OPEN')
    require(decision['status']==expected,'Coverage status not derived from required obligations')
    return decision


def validate_verified(root,record):
    """A ledger entry needs both coverage and actual common-gate admission."""
    value=load(root,record,'mb3d.formula-verified.v1')
    return validate_verified_value(root,value)


def validate_verified_value(root,value):
    """Validate a proposed post-admission entry before publishing its file."""
    import verify_native_formula_group as v
    fields(value,'schema status identity candidate coverage admission_receipt scene_disposition contract evidence qualification','verified ledger')
    require(value['schema']=='mb3d.formula-verified.v1','Wrong verified ledger schema')
    decision=validate_coverage(root,value['coverage'])
    require(decision['status']=='COVERAGE_COMPLETE' and value['status']=='FORMULA_VERIFIED','Incomplete formula cannot be VERIFIED')
    require(all(value[k]==decision[k] for k in ('identity','candidate','scene_disposition')),'Ledger coverage binding differs')
    receipt=load(root,value['admission_receipt'])
    require(receipt.get('coverage')==value['coverage'] and receipt.get('identity')==value['identity'] and
            receipt.get('status')=='PASS' and receipt.get('kind')=='formula_gpu_point_orbit','Missing coverage-bound admission receipt')
    evidence_path=artifact(root,value['evidence']);qualification=load(root,value['qualification'])
    require(evidence_path.name=='evidence.json','Common gate must read the exact canonical evidence file')
    require(value['qualification']['path']==str((evidence_path.parent/'qualification.json').relative_to(root)),
            'Wrong qualification package')
    evidence=read_json(evidence_path)
    binding=evidence['binding']
    require(binding['library']==decision['candidate']['library'] and
            binding['tools']['formula_gpu']==decision['candidate']['observer'] and
            receipt.get('binding_sha256')==digest(binding),'Admission candidate differs from covered runtime/observer')
    require(decision['collection'] in receipt.get('artifacts',[]) and
            decision['candidate']['build_receipt'] in receipt.get('artifacts',[]),
            'Admission lacks covered collection/build provenance')
    collection=load(root,decision['collection'])
    proof=load(root,receipt['observations'],'mb3d.native-metal-all.formula-observations.v1')
    proof_rows={row['case_id']:row for row in proof['cases']}
    covered_rows={row['id']:row for row in collection['cases'] if row['identity']==value['identity']}
    require(len(proof_rows)==len(proof['cases']) and set(proof_rows)==set(covered_rows),
            'Admission case set differs from covered collection')
    for key,row in covered_rows.items():
        require(proof_rows[key]['repeats']==row['repeats'] and proof_rows[key]['gpu_state_bits']==row['gpu_state_bits'],
                'Admission observations differ from covered probe/processes')
    require(any([r['namespace'],r['id'],r['name']]==value['identity'] and r['proof']==value['admission_receipt']
                for r in evidence['formulas']),'Receipt absent from accepted package')
    result=v.evaluate(artifact(root,value['contract']),evidence_path.parent)
    accepted=['formulas',str(tuple(value['identity']))]
    require(accepted in result['accepted'] and accepted in qualification['accepted'],'Common gate rejected formula ledger')
    return value
