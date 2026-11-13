"""Consolidate one frozen ordinary CPU qualification without inventing closure.

All original exceptions, caller limitations and historical failed runs remain
separate. No renderer, Monte Carlo or JIT evaluation is performed by this audit.
"""
import argparse
from collections import Counter, defaultdict
import hashlib
import json
from pathlib import Path
import re
import xml.etree.ElementTree as ET

ROOT = Path(__file__).resolve().parents[2]

ORDINARY_TESTS = frozenset({
    'math3d_tests', 'm3p_parser_tests', 'renderer_tests', 'pas_lcg_tests',
    'thread_pool_tests', 'external_registry_tests', 'portable_sqrt_tests',
    'integer_options_tests', 'formula_validation_tests',
    'formula_iteration_guard_tests', 'zbuffer_normals_tests',
    'camera_tile_setup_tests', 'ao_wavelet_lanes_tests', 'dof_tests',
    'render_failures_tests',
})
DEFERRED_TESTS = frozenset({'monte_carlo_tests', 'halton_tests'})

# Fixed, versioned corpus policy. These are input manifests, not candidate
# outcomes. Changing the corpus requires an explicit policy update, never a
# new result silently becoming an allowed exception.
POLICY_INPUTS = {
    'comparison/results/stage-a-20261018/coverage-ledger-v1.json':
        'c317f9bef9dbfe5368282dac952a77c4b649a379eb2eb276254fbf9fdedae9f8',
    'comparison/results/stage-a-20261018/final343-reference-preflight-v1.json':
        '7bb72ecc2b62d6b30fcb18dc6434232d710381788e197be466575cb852d1cb6f',
}
SELECTOR_MANIFESTS = {
    'selectors-27-v1': '1204eb2566203942b4f38d9db2d8992a224d9a58f69b19a279379655c009c0bf',
    'selectors-remaining-v1': 'a1e6cb2439c67ae6af47cedacc21c39c1862433e2c28ae5410eb50e6ee7509e5',
    'original-reviewed-active-v2': 'b45a937375a8fa33628da5452c65a1f87489ccdf431b72b923b2889eadf0d7b2',
    'original-direct-four-v1': '4a6d62d3c7f90319cf93c6ba8d0e6ec7f7b19a742038c6a8059c77aff69a7cef',
    'original-final-six-v1': '20422b600540bbd91c82881bddf8ba83b89a862228dea2bf93f2352fcab8455b',
}
POLICY_INPUTS.update({f'comparison/runs/stage-a-20261018/{folder}/scenes/manifest.json': digest
                      for folder, digest in SELECTOR_MANIFESTS.items()})

MANOWAR_REASON = ('Incoming XMM0/XMM1 require check_formula_sse_state.py; '
                  'unseeded oracle state is not a Pascal precondition')
POINT_UNVERIFIED = {
    'challenge_ManowarBulb_' + suffix: ('ManowarBulb', MANOWAR_REASON)
    for suffix in ('early', 'julia', 'p0_alternate', 'p0_zero', 'p1_alternate',
                   'p1_zero', 'p2_alternate', 'p2_zero', 'p3_alternate', 'p3_zero',
                   'p4_alternate', 'p5_alternate', 'p6_alternate')
}
POINT_UNVERIFIED.update({
    f'challenge_{name}_p8_alternate': (name, 'FORMULA_INPUT_REJECTED[caller_dependent_flip]')
    for name in ('ducksIFS', 'gnarlyIFS', 'gnarly2IFS', 'gnarly3IFS')
})
POINT_UNVERIFIED.update({
    f'challenge_TrifoxComplexAngles_p{option}_zero':
        ('TrifoxComplexAngles', 'FORMULA_INPUT_REJECTED[nonterminating_zero_power]')
    for option in (0, 3)
})
SELECTOR_UNVERIFIED = {
    f'selector_complete__GenCWarp_{option}_-1':
        ('_GenCWarp', 'FORMULA_INPUT_REJECTED[uninitialized_angle_stack]')
    for option in (5, 6)
}


def case_index(rows, fields, label):
    indexed = {}
    for row in rows:
        key = tuple(row.get(field) for field in fields)
        if any(not isinstance(value, str) or not value for value in key) or key in indexed:
            raise ValueError(label + ': missing or duplicate case identity: ' + repr(key))
        indexed[key] = row
    return indexed


def validate_case_identities(rows, expected, fields, label):
    """Require the exact case set and scene bytes, not merely a row count."""
    actual = case_index(rows, fields, label)
    if set(actual) != set(expected):
        raise ValueError(label + ': missing/unexpected cases: ' + repr(dict(
            missing=sorted(set(expected) - set(actual)),
            unexpected=sorted(set(actual) - set(expected)))))
    for key, scene_hash in expected.items():
        if actual[key].get('scene_sha256') != scene_hash:
            raise ValueError(label + ': scene identity changed: ' + repr(key))
    return actual


def allowed_unverified(surface, row):
    policy = {'points': POINT_UNVERIFIED, 'selector_points': SELECTOR_UNVERIFIED}.get(surface, {})
    expected = policy.get(row.get('case'))
    if not expected or row.get('name') != expected[0]:
        return False
    reason = row.get('reason')
    if expected[0] == 'ManowarBulb':
        return reason == expected[1] and not row.get('input_validation')
    return (isinstance(reason, str) and reason.startswith(expected[1] + ' ') and
            row.get('input_validation') == 'rejected' and row.get('cpp_exit_code') == 12)


def validate_ordinary_tests(tree, additional=()):
    """Require the declared suites exactly, with no duplicates or skipped tests."""
    additional = list(additional)
    if (len(additional) != len(set(additional)) or
            set(additional) & (ORDINARY_TESTS | DEFERRED_TESTS) or
            any(not name for name in additional)):
        raise ValueError('Invalid additional ordinary test declaration')
    expected = ORDINARY_TESTS | set(additional)
    tests = tree.findall('.//testcase')
    names = [test.get('name') for test in tests]
    failed = [test.get('name') for test in tests
              if any(test.find(tag) is not None for tag in ('failure', 'error', 'skipped'))]
    if len(names) != len(set(names)) or set(names) != expected or failed:
        raise ValueError('Ordinary CTest gate not passed: ' + str(dict(
            missing=sorted(expected - set(names)),
            unexpected=sorted(str(name) for name in set(names) - expected),
            duplicates=len(names) != len(set(names)), failed=failed)))
    return sorted(expected)


def primary_failures(surfaces):
    """Allow only the recorded original point limitations, with their reason."""
    failures = []
    for surface, rows in surfaces:
        for row in rows:
            if row.get('status') != 'pass' and not (
                    row.get('status') == 'unverified' and allowed_unverified(surface, row)):
                failures.append(dict(surface=surface, case=row.get('case', row.get('scene')),
                                     status=row.get('status', 'missing_status')))
    return failures


def identity(path):
    path=path.resolve()
    return dict(path=path.relative_to(ROOT).as_posix(),sha256=hashlib.sha256(path.read_bytes()).hexdigest())


def write(path,value):
    path.write_text(json.dumps(value,indent=2)+'\n',encoding='utf-8')


def main():
    ap=argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--candidate',type=Path,required=True)
    ap.add_argument('--images',type=Path,required=True)
    ap.add_argument('--points',type=Path,required=True)
    ap.add_argument('--ordinary',type=Path,required=True)
    ap.add_argument('--selectors',type=Path,required=True)
    ap.add_argument('--integer-ledger',type=Path,required=True)
    ap.add_argument('--option-audit',type=Path,
                    help='Historical effective-loader option audit, kept separate from current numerical results')
    ap.add_argument('--supplementary',type=Path,action='append',default=[],
                    help='Additional evidence report; retained separately from primary denominators')
    ap.add_argument('--additional-ordinary-test',action='append',default=[],
                    help='Declare an additional required ordinary CTest suite; the baseline 15 remain required')
    ap.add_argument('--output',type=Path,required=True)
    a=ap.parse_args()
    if a.output.exists():
        raise ValueError('Refusing to replace an existing qualification directory')
    recorded=[]
    def read(path):
        recorded.append(identity(path));return json.loads(path.read_bytes())
    def read_policy(path):
        if identity(ROOT/path)['sha256'] != POLICY_INPUTS[path]:
            raise ValueError('Fixed corpus policy input changed: ' + path)
        return read(ROOT/path)
    build=read(a.candidate/'build-identity.json')
    for row in build['sources']+build['binaries']:
        if identity(ROOT/row['path'])['sha256']!=row['sha256']:
            raise ValueError('Frozen source/binary changed: '+row['path'])
    renderer=identity(a.candidate/'mb3d_render.exe')
    point_probe=identity(a.candidate/'mb3d_formula_point_probe.exe')
    compiler=read(a.candidate/'compiler-identity.json')
    compiler_metadata=identity(a.candidate/'CMakeCXXCompiler.cmake')
    if compiler_metadata['sha256']!=compiler['cmake_compiler_metadata_sha256']:
        raise ValueError('Frozen compiler metadata changed')
    recorded.append(compiler_metadata)
    ledger=read_policy('comparison/results/stage-a-20261018/coverage-ledger-v1.json')
    ordinary_policy=read_policy('comparison/results/stage-a-20261018/final343-reference-preflight-v1.json')
    selector_cases=[]
    for folder in SELECTOR_MANIFESTS:
        selector_cases.extend(read_policy(f'comparison/runs/stage-a-20261018/{folder}/scenes/manifest.json')['cases'])
    points,images,ordinary,selectors,integers=[read(p) for p in
        (a.points,a.images,a.ordinary,a.selectors,a.integer_ledger)]
    if not all(d.get('finished') for d in (points,images,ordinary,selectors)):
        raise ValueError('A required replay is unfinished')
    if images['cpp_sha256']!=renderer['sha256'] or ordinary['cpp_sha256']!=renderer['sha256']:
        raise ValueError('Wrong renderer candidate')
    probe_path=str((a.candidate/'mb3d_formula_point_probe.exe').resolve())
    if any(report['identities'].get(probe_path)!=point_probe['sha256'] for report in (points,selectors)):
        raise ValueError('Wrong point probe candidate')
    image_rows=[r for job in images['jobs'] for r in job['results']]
    point_rows=points['results'];ordinary_rows=ordinary['results']
    for job in images['jobs']:
        path=Path(job['summary'])
        if not path.is_absolute():path=ROOT/path
        if identity(path)['sha256']!=job['summary_sha256']:
            raise ValueError('Image child report changed')
        child=read(path)
        fields=('scene','scene_sha256','status','exact_rgb','cpp_ms','error')
        if (not child.get('finished') or child.get('cpp_sha256')!=renderer['sha256'] or
                [{key: row.get(key) for key in fields} for row in child['results']]!=job['results']):
            raise ValueError('Image combined rows differ from the finished child report')
    if len(image_rows)!=5478 or len(point_rows)!=5506 or len(ordinary_rows)!=343:
        raise ValueError('Wrong primary qualification denominator')
    external=case_index(ledger['external_cases'],('case',),'external policy')
    expected_points={key:row['scene_sha256'] for key,row in external.items()}
    expected_excluded={key:row['scene_sha256'] for key,row in external.items()
                       if row['strict_input_rejection'] or row['image']['status']!='pass'}
    expected_images={key:scene_hash for key,scene_hash in expected_points.items() if key not in expected_excluded}
    pby={key[0]:row for key,row in validate_case_identities(point_rows,expected_points,('case',),'points').items()}
    iby={key[0]:row for key,row in validate_case_identities(image_rows,expected_images,('scene',),'images').items()}
    excluded={key[0]:row for key,row in validate_case_identities(
        images['excluded_from_numeric_replay'],expected_excluded,('case',),'excluded images').items()}
    if len(excluded)!=28:
        raise ValueError('Wrong fixed image exclusion denominator')
    expected_ordinary={key:ordinary_policy['input_sha256'][row['scene_path']]
                       for key,row in case_index(ordinary_policy['cases'],('suite','scene'),'ordinary policy').items()}
    validate_case_identities(ordinary_rows,expected_ordinary,('suite','scene'),'ordinary343')
    expected_selectors={key:row['sha256'] for key,row in case_index(selector_cases,('id',),'selector policy').items()}
    validate_case_identities(selectors['results'],expected_selectors,('case',),'selector points')
    if len(expected_selectors)!=591:
        raise ValueError('Wrong fixed selector denominator')
    if selectors['counts']!=dict(Counter(row['status'] for row in selectors['results'])):
        raise ValueError('Selector aggregate differs from its rows')
    groups=defaultdict(list)
    for case in ledger['external_cases']:
        name=case['target_formulas'][0];key=case['case'];p=pby[key]
        if identity(ROOT/case['scene'])['sha256']!=case['scene_sha256'] or p['scene_sha256']!=case['scene_sha256']:
            raise ValueError('Changed external scene: '+key)
        im=iby.get(key)
        if im and im['scene_sha256']!=case['scene_sha256']:
            raise ValueError('Image/point scene mismatch')
        groups[name].append(dict(case=key,scene=case['scene'],scene_sha256=case['scene_sha256'],
            point_status=p['status'],point_reason=p.get('reason',p.get('error')),
            image_status=im['status'] if im else 'excluded_original_reference_or_undefined_input',
            exact_rgb=im.get('exact_rgb') if im else None,
            image_reason=im.get('error') if im else case.get('strict_input_rejection') or case['reference'].get('reason'),
            image_profile='file-load migrated formula, original direct kernel checked separately' if name in ('_FlipXY','_FlipXZ','_FlipYZ') else 'ordinary file-load pipeline'))
    formula_rows=[]
    for name,cases in sorted(groups.items()):
        formula_rows.append(dict(formula=name,cases=cases,point_counts=dict(Counter(r['point_status'] for r in cases)),
            image_counts=dict(Counter(r['image_status'] for r in cases)),
            all_listed_point_and_image_pass=all(r['point_status']=='pass' and r['image_status']=='pass' for r in cases)))
    if len(formula_rows)!=457:raise ValueError('Wrong formula denominator')
    formula_status=dict(scope='Current generic case acceptance; separate caller profiles are not promoted to generic PASS.',formulas=formula_rows)
    xml=a.candidate/'ordinary-tests.xml';recorded.append(identity(xml));tree=ET.parse(xml).getroot()
    test_names=validate_ordinary_tests(tree,a.additional_ordinary_test)
    render_log=a.candidate/'renderer-tests.log';recorded.append(identity(render_log))
    match=re.search(r'Results:\s*(\d+) passed,\s*(\d+) failed',render_log.read_text())
    if not match or int(match[2]):raise ValueError('Renderer unit result not passed')
    python_log=a.candidate/'python-tools-tests.log';recorded.append(identity(python_log))
    python_text=python_log.read_text(encoding='utf-8-sig')
    pycount=re.search(r'Ran (\d+) tests',python_text)
    if not pycount or not python_text.rstrip().endswith('OK'):raise ValueError('Tool tests not passed')
    failures=primary_failures([('points',point_rows),('images',image_rows),
                              ('ordinary343',ordinary_rows),('selector_points',selectors['results'])])
    supplementary=[]
    for path in a.supplementary:
        data=read(path)
        supplementary.append(dict(**identity(path),scope=data.get('scope',data.get('purpose')),
            qualification='reference_only_not_validated',
            interpretation='Historical/reference link only: this collector does not validate its success, candidate, completeness or metrics. No current-candidate PASS claim or primary denominator credit.'))
    historical_options=None
    if a.option_audit:
        old_options=read(a.option_audit)
        short_slots=old_options['short_saved_option_slots']
        historical_options=dict(catalog_scenes=old_options['totals']['unique_catalog_entries'],
            short_saved_option_slots=len(short_slots),
            unique_scenes_with_short_option_count=len({r['scene'] for r in short_slots}),
            overwritten_integer_observations=old_options['totals']['overwritten_integer_observations'],
            scenes_with_overwritten_integer_values=old_options['totals']['scenes_with_overwritten_integer_values'],
            interpretation='Historical input-construction defects, not numerical failures. Original scenes are retained; added cases use expanded counts and verify actual loaded PVar values.')
    report=dict(schema_version=1,scope=__doc__,candidate=renderer,point_probe=point_probe,
        compiler_identity=compiler,
        production_source_hashes_verified=len(build['sources']),frozen_binaries_verified=len(build['binaries']),
        ordinary_tests=dict(suites=len(test_names),passed=len(test_names),names=test_names,
            renderer_assertions_passed=int(match[1]),renderer_assertions_failed=0,tool_tests_passed=int(pycount[1])),
        ordinary343=dict(counts=dict(Counter(r['status'] for r in ordinary_rows)),exact_rgb=sum(r.get('exact_rgb',False) for r in ordinary_rows)),
        external=dict(formulas=457,cases=5506,points=dict(Counter(r['status'] for r in point_rows)),
            scheduled_images=dict(Counter(r['status'] for r in image_rows)),exact_rgb=sum(r.get('exact_rgb',False) for r in image_rows),
            excluded_images=len(excluded),generic_groups_both_listed_pass=sum(r['all_listed_point_and_image_pass'] for r in formula_rows)),
        selector_points=dict(counts=selectors['counts'],native_selectors_verified=selectors['native_selectors_verified']),
        integer_domain_totals=integers['totals'],integer_ledger_scope='Historical source/domain inventory; current guarded behavioral evidence is separate.',
        large_work_obligations=[dict(formula=r['formula'],stored_value_index=r['stored_value_index'],**r['large_work_obligation']) for r in integers['options'] if r.get('large_work_obligation')],
        unexplained_numerical_failures=failures,executed_primary_gates_passed=not failures,
        full_pascal_parity_complete=False,stage_a_complete=False,overall_project_percentage=None,
        supplementary_evidence=supplementary,
        historical_option_generation_audit=historical_options,
        limitations=['Original blocked/undefined cases remain explicit; strict rejection is not numerical parity.',
          'Source classification, effective input presence, dynamic execution and independent loader coverage are different denominators.',
          'The 343 historical PNGs were snapshotted before replay; their old reports did not retain per-image hashes. See final343 preflight.',
          'Arbitrary hybrid/state combinations, large-work cancellation, concurrent scene ownership and reserved direct-header inputs are not certified by these finite gates.',
          'GPU/ARM execution is not performed; Monte Carlo and JIT remain deferred, not complete.'],
        inputs=recorded,audit_tool=identity(Path(__file__)))
    a.output.mkdir(parents=True,exist_ok=False)
    write(a.output/'formula-status.json',formula_status)
    report['formula_status']=identity(a.output/'formula-status.json')
    write(a.output/'checkpoint.json',report)
    print(json.dumps({k:report[k] for k in ('ordinary_tests','ordinary343','external','selector_points','executed_primary_gates_passed')}))
    return int(bool(failures))


if __name__=='__main__':raise SystemExit(main())
