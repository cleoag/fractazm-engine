"""Join original INTEGER semantics with effective inputs and immutable evidence.

This is a source/obligation ledger, not a claim that all numerical inputs or
interactions pass. Presence, execution and final-candidate qualification differ.
"""
import argparse
from collections import Counter, defaultdict
import hashlib
import json
from pathlib import Path
import re

from audit_original_integer_domains import predicate
from review_original_integer_domains import rule
from external_validation import ROOT, write_json


def identity(path):
    path = path.resolve()
    return dict(path=path.relative_to(ROOT).as_posix(), sha256=hashlib.sha256(path.read_bytes()).hexdigest())


def translated_unused_review(row):
    """Concrete complete-function review, not an empty alias-analysis result."""
    name=row['formula']; index=row['stored_value_index']
    if name=='_CondLyapunov':
        path=ROOT/'src/core/external_formulas.h'
        return dict(source=identity(path),anchor='int low=ID==10357',
            justification='ID10356 selects literal0/1; INTEGER loads at-24/-28 occur only in ID10357. Sequence/targets at-12/-16/-20 are the only inputs of this branch.')
    path=ROOT/'external_formulas/Formulas/3D'/(name+'.inc')
    text=path.read_text(encoding='utf-8');code=re.sub(r'/\*.*?\*/|//[^\n]*','',text,flags=re.S)
    literal_reads=sorted({int(v) for v in re.findall(r'params\[(\d+)\]',code)})
    if index in literal_reads:raise ValueError('Unused declaration now read in C++: '+name)
    result=dict(source=identity(path),literal_param_indices=literal_reads)
    if name=='BT1Pinehedron':
        result['justification']='Complete function dynamic lowByte indices6/7/9 and fixed count8 omit10. Native T1 helper receives only coordinates, scale, offset and three constants, not params.'
    else:
        helper=re.search(r'extern "C" void (\w+Original_x87)',text)[1]
        asm=ROOT/'src/core/win64_de_x87_helpers.asm';asm_text=asm.read_text(encoding='utf-8')
        body=asm_text.split(helper+' PROC',1)[1].split(helper+' ENDP',1)[0]
        offsets=sorted({int(v) for v in re.findall(r'\[rdx \+ (\d+)\]',body)})
        if index*8 in offsets:raise ValueError('Unused declaration now read by native helper: '+name)
        result.update(native_source=identity(asm),native_function=helper,parameter_offsets=offsets,
            justification='Complete native helper uses rdx only for listed fixed qword accesses; no parameter aliases/calls. Portable formula reads parameters0..2 and C6..8, omitting3/4.')
    return result


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--audit', type=Path, required=True)
    ap.add_argument('--selectors', type=Path, required=True)
    ap.add_argument('--manifest', type=Path, action='append', default=[])
    ap.add_argument('--evidence', type=Path, action='append', default=[])
    ap.add_argument('--boundary-evidence', type=Path, action='append', default=[])
    ap.add_argument('--output', type=Path, required=True)
    ap.add_argument('--delta', type=Path, required=True)
    a = ap.parse_args()
    report = json.loads(a.audit.read_bytes())
    original = {(r['formula'],r['stored_value_index']):r for r in json.loads(a.selectors.read_bytes())['options']}
    additions = defaultdict(list)
    boundaries = defaultdict(list)
    for path in a.boundary_evidence:
        batch=json.loads(path.read_bytes())
        if not batch['finished']:raise ValueError('Incomplete boundary batch: '+str(path))
        for case in batch['results']:
            scene=ROOT/case['scene']
            if identity(scene)['sha256']!=case['scene_sha256']:raise ValueError('Changed boundary scene')
            for check in case.get('native_parameter_checks',[]):
                if check['loaded']!=check['expected']:continue
                boundaries[(case['name'],check['index'])].append(dict(value=check['loaded'],
                    scene=case['scene'],scene_sha256=case['scene_sha256'],parameter_sha256=case['parameter_sha256'],
                    case=case['case'],execution_status=case['status'],cpp_sha256=batch['cpp_sha256'],
                    oracle_sha256=batch['oracle_sha256'],rows=case['rows'],lanes=case['lanes'],
                    evidence=identity(path),scope=case['reason']))
    for path in a.manifest:
        for case in json.loads(path.read_bytes())['cases']:
            if not case.get('selector'): continue
            key = case['target_formulas'][0],case['selector']['stored_value_index']
            scene = ROOT/case['scene']
            if identity(scene)['sha256'] != case['sha256']: raise ValueError('Changed scene: '+str(scene))
            additions[key].append(dict(value=case['selector']['int32_value'], case=case['id'],
                                       scene=case['scene'], sha256=case['sha256'],
                                       activation_override=case.get('activation_override')))
    delta, headers = [], {}
    for row in report['options']:
        key = row['formula'],row['stored_value_index']
        source = report['original_sources'][row['formula']]
        if identity(ROOT/source['asm']['path'])['sha256'] != source['asm']['sha256']:
            raise ValueError('Changed original disassembly: '+row['formula'])
        row['source_identity'] = dict(asm=source['asm'], original_code_sha256=source['original_code_sha256'])
        row['additional_effective_inputs'] = additions[key]
        row['native_verified_boundary_inputs'] = boundaries[key]
        values = row['scene_values']+row['fixture_values']+[r['value'] for r in additions[key]]+[r['value'] for r in boundaries[key]]
        row['effective_input_values'] = sorted(set(values))
        for path in row['fixture_headers']:
            headers.setdefault(path, identity(ROOT/path))
        row['source_classified'] = row['classification'] != 'requires_dataflow_review'
        row['source_review_limit'] = ('Complete-code manual review for unused declarations; empty static accesses alone are never evidence of unused input.'
            if row['classification']=='unused_original_declaration' else
            'Original use classification and explicitly recorded call/mask review; numerical correctness and coupled state remain separate.')
        if row['domain_complete']:
            if row['classification']=='reviewed_finite_original_domain': signature = rule(row)['signature']
            else: signature = lambda value:tuple(predicate(value,p['bound'],p['branch']) for p in row['predicates'])
            seen = {signature(v) for v in values}
            missing = [v for v in row['representatives'] if signature(v) not in seen]
            row['missing_input_classes'] = missing
            row['finite_class_count'] = len({signature(v) for v in row['representatives']})
            row['undefined_input_representatives'] = row.get('review',{}).get('undefined_representatives',[])
            row['defined_finite_class_count'] = row['finite_class_count']-len(row['undefined_input_representatives'])
            if row['undefined_input_representatives']:
                row['undefined_input_obligation'] = 'Retain original diagnosis and separately test strict rejection; never count a rejected or NaN-only original case as numerical parity PASS.'
            row['closure_status'] = 'finite_inputs_present_execution_and_reachability_separate' if not missing else 'missing_finite_input_classes'
            if missing:
                d = dict(original[key]);d['missing_representatives_in_scene_catalog']=missing
                d['domain'] = dict(kind='original_complete_partition', semantics=row.get('review',row.get('equivalence')), source=row['source_identity'])
                delta.append(d)
        elif row['classification']=='unused_original_declaration':
            row['closure_status']='source_proves_no_original_or_translated_parameter_use'
            row['translated_unused_review']=translated_unused_review(row)
            row['closure_obligation']='Recheck complete original and translated functions if their retained source hashes change; no selector mode exists.'
        elif row['classification']=='resource_identifier':
            row['closure_status']='resource_semantics_not_integer_mode_enumeration'
            row['closure_obligation']='Valid/missing IDs; real-map dimensions/edge/address behavior and callback state are resource checks. No finite map-ID enumeration proves this.'
        elif row['classification']=='defined_even_undefined_odd_caller_stack':
            row['closure_status']='defined_even_profile_and_explicit_odd_rejection'
            row['closure_obligation']='Reuse unchanged Pascal caller evidence; strict rejection of odd inputs is not parity PASS.'
        else:
            row['closure_status']='algorithmic_boundaries_and_state_obligations'
            requested_boundaries=row.get('review',{}).get('boundary_inputs',[])
            row['missing_boundary_input_presence']=[v for v in requested_boundaries if v not in values]
            row['closure_obligation']=row.get('review',{}).get('semantics',
                'Original FILD/FIMUL/FIDIV arithmetic or packed-overlap input; no finite selector equivalence. Reuse signed/zero/extreme/singular fixtures and identify uncovered numerical boundaries.')
            if (row['formula'] in ('_IFS_tess','_e_3Dtess','Knots2IFS')
                    and row['missing_boundary_input_presence']):
                row['large_work_obligation']=dict(status='not_executed',
                    values=row['missing_boundary_input_presence'],
                    reason='These counts repeat an already qualified loop body; there is no new mode. Large-work termination/cancellation remains an explicit Stage B obligation, not a numerical PASS or permission to clamp silently.')
    report['scope'] = __doc__
    report['evidence']=[identity(p) for p in a.evidence]
    report['effective_manifests']=[identity(p) for p in a.manifest]
    report['boundary_evidence']=[identity(p) for p in a.boundary_evidence]
    report['fixture_identities']=list(headers.values())
    report['ledger_generator']=identity(Path(__file__))
    report['input_audit']=identity(a.audit)
    report['totals']=dict(options=len(report['options']), source_classified=sum(r['source_classified'] for r in report['options']),
        classifications=dict(Counter(r['classification'] for r in report['options'])),
        closure_statuses=dict(Counter(r['closure_status'] for r in report['options'])),
        finite_domains=sum(r['domain_complete'] for r in report['options']),
        finite_classes=sum(r.get('finite_class_count',0) for r in report['options']),
        defined_finite_classes=sum(r.get('defined_finite_class_count',0) for r in report['options']),
        undefined_finite_classes=sum(len(r.get('undefined_input_representatives',[])) for r in report['options']),
        missing_finite_classes=sum(len(r.get('missing_input_classes',[])) for r in report['options']),
        algorithmic_parameters_with_missing_boundary_input_presence=sum(bool(r.get('missing_boundary_input_presence')) for r in report['options']),
        parameters_with_native_boundary_checks=sum(bool(r['native_verified_boundary_inputs']) for r in report['options']),
        retained_boundary_failures=sum(x['execution_status']!='pass' for r in report['options'] for x in r['native_verified_boundary_inputs']),
        parity_passes_produced=0)
    report['limitations'] += ['Evidence belongs to its recorded executable; final candidate replay remains independent.',
        'Historical fixture input presence does not prove selector reachability. Explicit active rotation replacement scenes remove the identified dormant-rotation gap.',
        'Pointer-spill alias analysis is incomplete; unused annotations require manual complete-CODE review.',
        'Suggested numerical boundary lists are obligations, not independent bugs, and are not a demand to enumerate every integer.']
    write_json(a.output,report)
    write_json(a.delta,dict(scope='Remaining finite source-equivalence input classes only; no parity assertion.',options=delta))
    print(json.dumps(report['totals']))


if __name__=='__main__':main()
