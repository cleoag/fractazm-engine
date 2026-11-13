#!/usr/bin/env python3
"""Collect independent point/orbit evidence from three fresh Metal processes.

Collection never grants credit by itself. Admission requires a separate review,
then replays the ordinary common gate. Failed cases and raw device events remain
visible; no expected value is learned from a GPU result.
"""
import argparse
from collections import Counter, defaultdict
import importlib
import json
from pathlib import Path
import sys
import uuid
from qualify_native_metal_all import ROOT, RECEIPT_SCHEMA, artifact, digest, formula_observations, require, sha
from gpu_pipeline_proofs import formula_repeats
from gpu_pipeline_binding import core_context
from qualify_gpu_parallel_pipeline import evaluate, scope
from prepare_gpu_parallel_bundle import ref, write
from run_m3parameter_gpu import invoke
import native_formula_observer as observer
import native_formula_coverage as coverage_proof


def read(path): return json.loads(Path(path).read_text())


REFERENCE_MODULES = {'affine': 'native_formula_reference', 'affine_v2': 'native_formula_affine_v2_reference',
                     'fold': 'native_fold_reference',
                     'reciprocal': 'native_reciprocal_reference', 'matrix': 'native_matrix_reference',
                     'group02_tail': 'native_group02_tail_reference',
                     'group03_geometry': 'native_group03_geometry_reference',
                     'group03_julia': 'native_group03_julia_reference',
                     'group03_map': 'native_group03_map_reference',
                     'group04_polynomial': 'native_group04_polynomial_reference',
                     'group04_symmetry': 'native_group04_symmetry_reference',
                     'group04_projection': 'native_group04_projection_reference',
                     'group05_scaling': 'native_group05_scaling_reference',
                     'group05_ifs': 'native_group05_ifs_reference'}


def load_reference(group):
    require(isinstance(group, str) and group in REFERENCE_MODULES, 'Unknown independent reference group')
    module = importlib.import_module(REFERENCE_MODULES[group])
    require(Path(module.__file__).resolve() == ROOT/'comparison/tools'/(REFERENCE_MODULES[group]+'.py'),
            'Reference module outside audited source path')
    return module


def reference_inputs(reference):
    model = Path(reference.__file__).resolve()
    paths = set(reference.dependency_paths()) if hasattr(reference, 'dependency_paths') else {model}
    paths.add(model)
    require(all(isinstance(p, Path) and p.resolve().parent == ROOT/'comparison/tools' and
                p.suffix == '.py' and p.is_file() for p in paths), 'Invalid reference dependency closure')
    if hasattr(reference, 'data_paths'):
        data_paths = reference.data_paths()
        require(all(isinstance(p, Path) and p.resolve().is_relative_to(ROOT) and p.is_file()
                    for p in data_paths), 'Invalid reference source/profile dependency closure')
        paths.update(data_paths)
    return sorted(p.resolve() for p in paths)


def validate_reference(data, reference):
    require(ref(Path(reference.__file__).resolve()) == data['reference_model'] and
            reference.manifest() == data['reference_manifest'], 'Reference model changed')
    dependencies = [ref(p) for p in reference_inputs(reference)]
    # Existing affine collections predate the explicit dependency field. Their
    # only additional executable dependency is in the captured observer closure.
    if 'reference_dependencies' in data:
        require(data['reference_dependencies'] == dependencies, 'Reference dependencies changed')
    else:
        require(data.get('reference_group', 'affine') == 'affine' and
                len(dependencies) == 1, 'Missing reference dependency closure')
    return dependencies


def validate_reference_event(event, dependencies):
    require(all(r in event['inputs'] for r in dependencies),
            'Independent reference inputs were not captured')


def validate_build_products(receipt_path, probe, library):
    """Bind the device host to its captured build, separately from Python tools."""
    receipt = read(receipt_path)
    require(receipt['status'] == 'PASS_CAPTURED_BUILD_CLOSURE', 'Unverified device build')
    require(receipt['commands'] == len(receipt['records']) > 0 and
            receipt['objects'] > 0 and receipt['airs'] > 0, 'Empty build provenance')
    products = {str((receipt_path.parent / row['path']).resolve()): row['sha256']
                for row in receipt['products']}
    for path in (probe, library):
        require(products.get(str(path.resolve())) == sha(path), 'Device artifact absent from captured build')
    # The receipt is immutable evidence, not a request to rewrite/rebuild it.
    outputs = set()
    for row in receipt['records']:
        path = receipt_path.parent / row['path']
        require(sha(path) == row['sha256'], 'Captured compiler record changed')
        record = read(path)
        require(record['exit_code'] == 0 and not record['capture_errors'], 'Failed compiler capture')
        product = record.get('output')
        if product:
            outputs.add((str(Path(product['path']).resolve()), product['sha256']))
    require(all((str(path.resolve()), sha(path)) in outputs for path in (probe, library)),
            'Missing actual probe/library producer')


def validate_corpus(data, approved, corpus):
    expected = {row['id']: row for row in corpus if tuple(row['identity']) in approved}
    actual = [row for row in data['cases'] if tuple(row['identity']) in approved]
    require(len(actual) == len(expected) and {row['id'] for row in actual} == set(expected),
            'Incomplete or duplicate independent corpus')
    for row in actual:
        original = expected[row['id']]
        require(all(row[key] == original[key] for key in
                    ('identity', 'variant', 'expected_state_bits', 'read_lanes', 'write_lanes', 'source_hashes')),
                'Reference corpus metadata changed')
        payload = read(artifact(ROOT, row['input']))
        require(payload == original['input'], 'Reference corpus input changed')
        if 'context' in payload:
            require(all(row.get(key) == original.get(key) for key in
                        ('expected_work', 'expected_slot_visits')),
                    'Mixed reference work/schedule changed')
            if row['status'] == 'PASS_CASE':
                require(row['result_words'][51] == original['expected_work'],
                        'Mixed actual work differs from original source schedule')
    return expected


def collect(build, output, names=None, group='affine', coverage=None, coverage_review=None, model_review=None):
    require(all(x is None for x in (coverage,coverage_review,model_review)) or
            all(x is not None for x in (coverage,coverage_review,model_review)),
            'Coverage declaration, completeness review and model review are required together')
    reference = load_reference(group)
    build, output = build.resolve(), output.resolve()
    require(build.is_relative_to(ROOT) and output.is_relative_to(ROOT) and not output.exists(), 'New repository output and frozen build required')
    probe, library = build/'mb3d_native_formula_probe', build/'metal_native_scene.metallib'
    require(probe.is_file() and library.is_file(), 'Missing native device artifacts')
    cases = reference.build_cases()
    require(len({c['id'] for c in cases}) == len(cases), 'Duplicate reference case')
    known={c['identity'][2] for c in cases}
    require(names is None or (names and set(names)<=known), 'Unknown/empty reference group')
    cases=[c for c in cases if names is None or c['identity'][2] in names]
    model=Path(reference.__file__).resolve(); adapter=Path(observer.__file__).resolve()
    model_manifest=reference.manifest()
    build_receipt=build.parent/'build-receipt.json'
    validate_build_products(build_receipt,probe,library)
    result={'schema':'mb3d.native-formula-group.v1','status':'RUNNING','qualification':False,
            'evidence_origin':'actual_device_capture',
            'scope':'Independent finite point/orbit fixture domain; no scene or arbitrary-input claim',
            'reference_group':group,'reference_model':ref(model),'reference_manifest':model_manifest,
            'reference_dependencies':[ref(p) for p in reference_inputs(reference)],'observer':ref(adapter),
            'probe':ref(probe),'library':ref(library),'build_receipt':ref(build_receipt),'cases':[],'formulas':[]}
    model_hash=sha(model)
    inputs=sorted({probe,library,*reference_inputs(reference),*observer.dependency_paths()})
    bundle=None
    if coverage is not None:
        require(len({tuple(c['identity']) for c in cases})==1,'Annotated coverage collection requires one identity')
        bundle=coverage_proof.validate_precollection(ROOT,ref(coverage.resolve()),ref(coverage_review.resolve()),
                                                     ref(model_review.resolve()),cases,result)
        result.update(bundle)
        result['coverage_tools']=coverage_proof.tool_inputs(ROOT)
        # Capture the collector/coverage implementation as well as the reviewed
        # declaration and complete source/model/context input closure.
        inputs=sorted(set(inputs)|{artifact(ROOT,r) for r in result['coverage_tools']}|
                      {artifact(ROOT,r) for r in bundle['coverage_inputs']})
    for original in cases:
        require(reference.evaluate(original['input'])==original['expected_state_bits'], 'Reference corpus does not replay')
        observer.validate_case(original['input'])
        if 'context' in original['input']:
            observer.context_input.validate_schedule(original)
        inputs = sorted(set(inputs) | set(observer.dependency_paths(original['input'])))
        if 'resources' in original['input']:
            _, paths = observer.resource_input.validate(original['input']['resources'], ROOT)
            inputs = sorted(set(inputs) | set(paths))
    output.mkdir(parents=True)
    write(output/'collection.json',result)
    for index, original in enumerate(cases):
        require(sha(model)==model_hash and reference.manifest()==model_manifest, 'Independent model/source changed')
        validate_reference(result, reference)
        case_id=original['id'];require(case_id and all(c.isalnum() or c in '-_' for c in case_id), 'Unsafe case ID')
        folder=output/'cases'/case_id;folder.mkdir(parents=True)
        case_input=folder/'input.json';write(case_input,original['input'])
        row={k:original[k] for k in ('id','identity','variant','expected_state_bits','read_lanes','write_lanes','source_hashes')}
        if 'context' in original['input']:
            row.update({k:original[k] for k in ('expected_work','expected_slot_visits')})
        row.update(input=ref(case_input),kind=original['input']['kind'],status='RUNNING',repeats=[])
        result['cases'].append(row)
        for repeat in range(3):
            if bundle is not None:
                for pinned in bundle['coverage_inputs']:artifact(ROOT,pinned)
                for pinned in result['coverage_tools']:artifact(ROOT,pinned)
            run=folder/f'run-{repeat+1}';observation=run/'observation.json';run_id=uuid.uuid4().hex
            args=[adapter,'--input',case_input,'--probe',probe,'--probe-sha256',result['probe']['sha256'],
                  '--library',library,'--library-sha256',result['library']['sha256'],'--output',observation,
                  '--run-id',run_id,'--timeout','150']
            event=invoke(args,run,ROOT,180,evidence_root=ROOT,input_files=inputs,exclusive_gpu=True,run_id=run_id)
            if bundle is not None:coverage_proof.validate_event_inputs(event,bundle)
            entry={'event':ref(run/'event.json'),'inputs':ref(case_input)};row['repeats'].append(entry)
            if event['exit_code']!=0 or event['guard'] is not None or not observation.is_file():
                row.update(status='EXECUTION_FAILED',failure={'exit_code':event['exit_code'],'guard':event['guard']});break
            entry['observation']=ref(observation);actual=read(observation)
            require(actual['run_id']==run_id and actual['input_digest']==digest(original['input']), 'Observer event/input mismatch')
            mismatch={lane:{'expected':v,'actual':actual['state_bits'].get(lane)} for lane,v in original['expected_state_bits'].items() if actual['state_bits'].get(lane)!=v}
            if mismatch:
                row.update(status='REFERENCE_MISMATCH',differences=mismatch);break
            if 'context' in original['input'] and actual['work'] != original['expected_work']:
                row.update(status='WORK_MISMATCH', expected_work=original['expected_work'],
                           actual_work=actual['work']);break
            if repeat and actual['result_words']!=row['result_words']:
                row.update(status='REPEAT_MISMATCH');break
            row.update(gpu_state_bits=actual['state_bits'],gpu_invocations=actual['gpu_invocations'],result_words=actual['result_words'],device=actual['device'])
        else:row['status']='PASS_CASE'
        write(output/'collection.json',result)
        print(f"{index+1}/{len(cases)} {case_id}: {row['status']}",flush=True)
    by_formula=defaultdict(list)
    for case in result['cases']:by_formula[tuple(case['identity'])].append(case)
    for identity, group in by_formula.items():
        passed=all(c['status']=='PASS_CASE' for c in group) and {c['kind'] for c in group}=={'point','orbit'}
        result['formulas'].append({'identity':list(identity),'cases':len(group),'passing_cases':sum(c['status']=='PASS_CASE' for c in group),'status':'READY_FOR_REVIEW' if passed else 'OPEN_CASE_FAILURE'})
    result['status']='COLLECTED_REVIEW_REQUIRED';result['case_counts']=dict(Counter(c['status'] for c in result['cases']))
    require(reference.manifest()==model_manifest and sha(model)==model_hash, 'Reference inputs changed')
    validate_reference(result, reference)
    if bundle is not None:
        for pinned in bundle['coverage_inputs']:artifact(ROOT,pinned)
        for pinned in result['coverage_tools']:artifact(ROOT,pinned)
    write(output/'collection.json',result)
    return result


def admit(collection, review, package, contract, coverage=None):
    require(coverage is not None,'New formula admission requires coverage decision')
    collection, review, package, contract = (p.resolve() for p in (collection,review,package,contract))
    coverage_ref=ref(coverage.resolve())
    coverage_decision=coverage_proof.validate_coverage(ROOT,coverage_ref)
    require(coverage_decision['status']=='COVERAGE_COMPLETE' and coverage_decision['collection']==ref(collection),
            'Complete coverage for this collection required before admission')
    data=read(collection);decision=read(review);ev=read(package/'evidence.json')
    require(data['schema']=='mb3d.native-formula-group.v1' and data['status']=='COLLECTED_REVIEW_REQUIRED','Incomplete collection')
    reference = load_reference(data.get('reference_group', 'affine'))
    require(decision['status']=='PASS' and decision['collection']==ref(collection) and decision['reference_model']==data['reference_model'],'Review does not bind this collection/model')
    require(isinstance(decision['reviewer'],str) and decision['reviewer'] and decision['reviewer']!='verify_native_formula_group.py','Separate independent review required')
    require(decision['build_receipt']==data['build_receipt'], 'Review must bind compiled device probe provenance')
    validate_build_products(artifact(ROOT,data['build_receipt']),artifact(ROOT,data['probe']),artifact(ROOT,data['library']))
    model_dependencies = validate_reference(data, reference)
    b=ev['binding'];require(b['library']==data['library'] and b['tools']['formula_gpu']==data['observer'],'Wrong final library/observer binding')
    c,formulas,scenes=scope(contract);ctx=core_context(ROOT,b,c,formulas,scenes,ROOT);binding=digest(b)
    existing={(r['namespace'],r['id'],r['name']) for r in ev['formulas']}
    approved={tuple(x) for x in decision['approved_formulas']}
    require(approved=={tuple(coverage_decision['identity'])},'Admission identity differs from coverage')
    ready={tuple(r['identity']) for r in data['formulas'] if r['status']=='READY_FOR_REVIEW'}
    require(approved and approved<=ready and not approved&existing,'Invalid/repeated approved formula set')
    validate_corpus(data,approved,reference.build_cases())
    new=[]
    for identity in sorted(approved):
        observed=[]
        for row in data['cases']:
            if tuple(row['identity'])!=identity:continue
            require(row['status']=='PASS_CASE' and len(row['repeats'])==3,'Incomplete successful case')
            payload=read(artifact(ROOT,row['input']))
            expected=reference.evaluate(payload)
            require(expected==row['expected_state_bits']==row['gpu_state_bits'],'Independent reference mismatch during admission')
            for repeat in row['repeats']:
                validate_reference_event(read(artifact(ROOT, repeat['event'])), model_dependencies)
                observer.replay_observation(ROOT,repeat['observation'],repeat['event'],payload,data['probe'],data['library'])
            observed.append({'case_id':row['id'],'identity':list(identity),'kind':row['kind'],'variant':row['variant'],
                             'input_state_bits':payload['input_state_bits'],'parameter_bits':payload['parameter_bits'],
                             'expected_state_bits':expected,'gpu_state_bits':row['gpu_state_bits'],
                             'read_lanes':row['read_lanes'],'written_lanes':row['write_lanes'],
                             'gpu_invocations':row['gpu_invocations'],'faults':0,'repeats':row['repeats']})
            if 'resources' in payload:
                observed[-1]['resources'] = payload['resources']
            if 'context' in payload:
                observed[-1]['context'] = payload['context']
        folder=package/'formula-proofs'/str(identity[1]);folder.mkdir(parents=True,exist_ok=False)
        observation=folder/'observations.json'
        write(observation,{'schema':'mb3d.native-metal-all.formula-observations.v1','binding_sha256':binding,
                           'library_marker':[2,4,32],'execution':'metal_gpu','cpu_pixel_calls':0,'cases':observed})
        formula_observations(ROOT,ref(observation),identity,binding);formula_repeats(ROOT,observed,identity,ctx)
        receipt={'schema':RECEIPT_SCHEMA,'kind':'formula_gpu_point_orbit','status':'PASS','binding_sha256':binding,
                 'coverage':coverage_ref,'coverage_scope':'campaign004-complete-declared-ordinary-obligations',
                 'identity':list(identity),'producer':'verify_native_formula_group.py','independent_reviewer':decision['reviewer'],
                 'checks':[{'id':name,'status':'PASS'} for name in ('independent-full-state-fixtures','three-fresh-device-processes','reviewed-parameter-state-contract')],
                 'artifacts':[ref(collection),ref(review),coverage_ref,data['reference_model'],data['build_receipt'],ref(observation)],'observations':ref(observation),
                 'state_contract_verified':True,'parameter_contract_verified':True}
        path=folder/'receipt.json';write(path,receipt)
        new.append({'namespace':identity[0],'id':identity[1],'name':identity[2],'proof':ref(path)})
    ev['formulas'].extend(new);write(package/'evidence.json',ev)
    result=evaluate(contract,package);write(package/'qualification.json',result)
    for row in new:require(['formulas',str((row['namespace'],row['id'],row['name']))] in result['accepted'],'Common gate rejected formula')
    for row in new:
        publish_verified(package,row,ev,result,coverage_ref,coverage_decision,contract)
    return result


def publish_verified(package,row,evidence,result,coverage_ref,decision,contract):
    """Seal the accepted manifests before later admissions mutate the aggregate."""
    folder=package/'formula-proofs'/str(row['id'])
    snapshot=folder/'admission'
    snapshot.mkdir(parents=True,exist_ok=False)
    write(snapshot/'evidence.json',evidence)
    write(snapshot/'qualification.json',result)
    verified={'schema':'mb3d.formula-verified.v1','status':'FORMULA_VERIFIED',
              'identity':[row['namespace'],row['id'],row['name']],
              'candidate':decision['candidate'],'coverage':coverage_ref,
              'admission_receipt':row['proof'],'scene_disposition':decision['scene_disposition'],
              'contract':ref(contract),'evidence':ref(snapshot/'evidence.json'),
              'qualification':ref(snapshot/'qualification.json')}
    coverage_proof.validate_verified_value(ROOT,verified)
    write(folder/'verified.json',verified)
    return verified


def main():
    p=argparse.ArgumentParser(description=__doc__);sub=p.add_subparsers(dest='mode',required=True)
    collect_parser=sub.add_parser('collect');collect_parser.add_argument('--build',type=Path,required=True);collect_parser.add_argument('--output',type=Path,required=True)
    collect_parser.add_argument('--formula',action='append')
    collect_parser.add_argument('--reference-group', choices=sorted(REFERENCE_MODULES), default='affine')
    for name in ('coverage','coverage-review','model-review'):collect_parser.add_argument('--'+name,type=Path)
    admit_parser=sub.add_parser('admit')
    for name in ('collection','review','package','contract'):admit_parser.add_argument('--'+name,type=Path,required=True)
    admit_parser.add_argument('--coverage',type=Path,required=True)
    a=p.parse_args()
    if a.mode=='collect':result=collect(a.build,a.output,a.formula,a.reference_group,a.coverage,a.coverage_review,a.model_review)
    else:result=admit(a.collection,a.review,a.package,a.contract,a.coverage)
    print(json.dumps({k:result[k] for k in ('status','case_counts','remaining') if k in result},indent=2))

if __name__=='__main__':main()
