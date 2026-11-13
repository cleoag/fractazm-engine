"""Seal scoped original-caller evidence, without running any renderer or oracle."""
import argparse
import hashlib
import json
import math
from pathlib import Path
from check_manowar_hybrids import equal, near, incoming_mismatches, validate_rows

ROOT = Path(__file__).resolve().parents[2]
MAIN = ['alt_mm','alt_ms','alt_sm','alt_tm','alt_mt','alt_rm','interp_mm',
        'interp_ms','interp_sm','interp_zero','interp_de_zero','interp_4d_zero',
        'interp_4dde_zero']
PRODUCERS = ['alt_ipm','alt_mip'] + ['alt_ipm_type'+str(i) for i in range(12)] + [
    'alt_ipm_negative','alt_ipm_intmin','alt_ipm_intmax','alt_ipm_third']

def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()

def read(path):
    return json.loads(path.read_bytes())

def require(ok, message):
    if not ok:
        raise ValueError(message)

def numbers(path):
    return [[math.nan if 'nan' in v.lower() else float(v) for v in line.split()]
            for line in path.read_text().splitlines()]

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--root', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    require(not args.output.exists(), 'refuse to overwrite qualification')
    base = args.root.resolve()
    evidence = {}
    def load(relative):
        path = base / relative
        evidence[str(path.relative_to(ROOT)).replace('\\','/')] = sha(path)
        return read(path)
    identity = load('candidate-v2/identity.json')
    for path, digest in identity['sha256'].items():
        require(sha(ROOT/path) == digest, 'changed frozen source/binary: '+path)
    probe = base/'candidate-v2/manowar_hybrid_probe.exe'
    results = []
    baseline = []
    for directory, before, manifest_name, names in [
        ('hybrids-final-v2','hybrids-before-fix','hybrid-manifest-v3.json',MAIN),
        ('sse-producer-final','sse-producer-branches-before','sse-producer-branches-manifest.json',PRODUCERS)]:
        summary = load(directory+'/summary.json')
        manifest = load(manifest_name)
        require(summary['finished'] and summary['rows_per_case']==96, 'incomplete run')
        require([r['id'] for r in summary['results']]==names, 'wrong exact current case set')
        require([c['id'] for c in manifest['cases']]==names, 'wrong exact manifest case set')
        require(summary['identities'].get(str(probe))==sha(probe), 'wrong current probe')
        for artifact,digest in summary['artifact_sha256'].items():
            require(sha(base/directory/artifact)==digest, 'changed raw artifact: '+artifact)
        old = load(before+'/summary.json')
        require([r['id'] for r in old['results']]==names, 'wrong baseline case set')
        for artifact,digest in old['artifact_sha256'].items():
            require(sha(base/before/artifact)==digest, 'changed baseline artifact: '+artifact)
        for case, result in zip(manifest['cases'],summary['results']):
            require(sha(ROOT/case['scene'])==case['sha256']==result['scene_sha256'], 'changed scene')
            require(result['status']=='PASS' and not result['failures'], 'failed profile')
            require(result['different_orbits']==result['different_manowar_incoming_lanes']==0,
                    'hidden state/caller failure')
            require(result['plain_trace_repeat_invariant'], 'instrumentation drift')
            require(all(r['status']=='PASS' and not r['different_consumed_offsets']
                        for r in result['independent_loader']), 'unqualified parameter buffer')
            current = {mode:numbers(base/directory/case['id']/(mode+'.stdout'))
                       for mode in ['cpp_plain','cpp_trace','pas_plain','pas_trace','pas_repeat']}
            validate_rows(current,96)
            bad_state = bad_lanes = 0
            old_rows = {mode:numbers(base/before/case['id']/(mode+'.stdout'))
                        for mode in ['cpp_plain','cpp_trace','pas_plain','pas_trace','pas_repeat']}
            validate_rows(old_rows,96)
            for cpp,pas in zip(old_rows['cpp_trace'],old_rows['pas_trace']):
                bad_state += not all(near(x,y) if i<12 else equal(x,y)
                                     for i,(x,y) in enumerate(zip(cpp[:14],pas[:14])))
                bad_lanes += len(incoming_mismatches(cpp,pas,case['names']))
            baseline.append({'id':case['id'],'different_orbits':bad_state,
                             'incoming_mismatch_events':bad_lanes,
                             'strict_status':'FAIL' if bad_state or bad_lanes else 'PASS'})
            results.append({'id':case['id'],'rows':96,'status':'PASS',
                            'exact_orbits':result['exact_orbits'],
                            'captured_calls':result['captured_calls'],
                            'runtime':result['runtime']})
    loader = load('loader13-current/summary.json')
    original_loader = load('loader13-original/summary.json')
    require(loader['finished'] and original_loader['finished'], 'unfinished loader')
    require(len(loader['results'])==13 and len({r['case'] for r in loader['results']})==13,
            'wrong independent loader count')
    require({r['case'] for r in loader['results']}=={r['case'] for r in original_loader['results']},
            'loader cases differ')
    require(loader['cpp_sha256']==sha(base/'candidate-v2/mb3d_formula_point_probe.exe'), 'wrong loader probe')
    for row in loader['results']:
        require(row['status']=='EXACT' and row['exit_code']==0, 'loader failure')
        current, pascal = Path(row['current']), Path(row['pascal'])
        require(sha(current)==row['current_sha256'] and sha(pascal)==row['pascal_sha256'], 'loader hash')
        require(len(current.read_bytes())==1024 and len(pascal.read_bytes())==512
                and current.read_bytes()[256:768]==pascal.read_bytes(), 'loader bytes')
    for name,names in [('guarded-caller-invariance',MAIN),('guarded-producer-invariance',PRODUCERS)]:
        guarded = load(name+'/summary.json')
        require(guarded['finished'] and [r['id'] for r in guarded['results']]==names,
                'wrong guarded reference cases')
        require(all(r['rows']==96 and r['status'] in ('PASS','EXACT')
                    and not r.get('different_rows') for r in guarded['results']), 'guarded drift')
    negative = load('fixture-negative-controls-v2/summary.json')
    require(len(negative['results'])==8 and len({r['id'] for r in negative['results']})==8,
            'missing failure-closed controls')
    require(all(r['status']=='PASS' and r['exit_code']==2 for r in negative['results']), 'fail-open fixture')
    for filename,expected in [('focused-native-v2.log','1248 orbits, 41693 checks, 0 failures'),
                              ('focused-producer-native-v2.log','1728 orbits, 50292 checks, 0 failures')]:
        path = base/filename
        require(path.read_text().strip()==expected, 'native regression failed')
        evidence[str(path.relative_to(ROOT)).replace('\\','/')] = sha(path)
    for filename in ['production-fixes-final.patch','source-final.zip','consumed-parameters-final.json']:
        path=base/filename
        evidence[str(path.relative_to(ROOT)).replace('\\','/')] = sha(path)
    for filename in ['oracle-v2/identity.json','oracle-guarded/identity.json']:
        load(filename)
    report = {
        'schema_version':1,'status':'PASS','scope':'Finite original FPC 3.2.2 i386 SSE2 caller qualification on native MSVC x64 C++',
        'stage_a_complete':False,'generic_point_reclassified':False,
        'candidate':identity,'profile_count':31,'orbit_count':2976,
        'native_checks':91985,'independent_original_case_buffers':13,
        'guarded_vs_immutable_orbits':2976,'negative_fixture_checks':8,
        'comparison':{'scaled_finite_tolerance':2e-12,'counters_and_call_order':'exact',
                      'nan':'classification','signed_zero_and_nan_payloads_certified':False},
        'profiles':results,'negative_baseline_reclassified_with_current_lane_checks':baseline,
        'baseline_note':'Historical hybrids-before-fix ignored incoming-only lane failures in its status. Raw outputs remain unchanged; strict statuses above include zero-weight caller observations.',
        'fixes':[
            'Eight interpolation callback sites reproduce SSE2 CopyVec4 incoming XYZW; 4DDE MOVSD additionally supplies saved Deriv1 and zero in XMM0.',
            'ABoxMod4d native helper preserves original x87 extended fold/cancellation and store order; portable fallback is unchanged and not certified.',
            '_IntPowRotateC explicitly publishes its outgoing SSE lanes, including the unscaled XMM1.high selected by the final enabled rotation.'
        ],
        'support_boundary':[
            'The 13 generic Manowar point cases remain unverified in that generic profile; independent loader and actual caller evidence are separate.',
            'All four GetMCT-selected interpolation driver classes are exercised, including zero-weight Manowar whose callback still executes.',
            'Alternating witnesses cover counted siblings, x87-only uncounted _Translate/_Rotate, and shipped DE=-1 SSE producer _IntPowRotateC with both orders and finite branch representatives.',
            'The producer inventory is restricted to shipped DE=-1 bodies with positive saved counts. GetMCT can retain negative saved counts for other DE classes; arbitrary uncounted formulas and arbitrary longer chains are not certified.',
            'The original 13 scene option tails can be overwritten by loader defaults; they are not 13 distinct effective mode branches.',
            'Other Pascal compilers, alternative copy implementations, ARM/GPU/portable branches, signed-zero/NaN payload behavior and arbitrary x87 exception state are outside this evidence.',
            'This report contains no new image acceptance claim; root runs the separate affected selection and coherent ordinary suite.'
        ],
        'evidence_sha256':evidence,'validator_sha256':sha(Path(__file__)),
        'helper_tool_sha256':{str((ROOT/'comparison/tools'/n).relative_to(ROOT)):sha(ROOT/'comparison/tools'/n)
                              for n in ['check_manowar_hybrids.py','check_manowar_sse_producer.py','test_manowar_hybrids.py']}
    }
    args.output.write_text(json.dumps(report,indent=2))
    print('PASS: 31 profiles, 2976 orbits, 91985 native checks, 13 independent buffers')

if __name__=='__main__':
    main()
