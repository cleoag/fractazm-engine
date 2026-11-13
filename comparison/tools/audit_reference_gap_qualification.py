"""Audit the fixed October 18 reference-gap scope without executing renderers.

This creates a new scoped qualification, never changes the historical full-corpus
checkpoint, and never promotes caller-specific or rejection evidence to generic
numerical parity. All paths are resolved against the repository or run directory.
"""
from __future__ import annotations
import argparse
import datetime
import hashlib
import json
from pathlib import Path
import re
import struct
import subprocess
import xml.etree.ElementTree as ET

import numpy as np
from image_diff import DEFAULT_THRESHOLDS, load_rgb, compute_metrics, evaluate
from check_manowar_hybrid_images import PROFILES

ROOT = Path(__file__).resolve().parents[2]
OLD = ROOT/'comparison/results/compatibility-final-20261018'
CHANGED = {'CMakeLists.txt', 'src/core/formulas.cpp',
           'src/core/win64_de_x87_helpers.asm',
           'external_formulas/Formulas/4Da/ABoxMod4d.inc',
           'external_formulas/Formulas/Ads/_IntPowRotateC.inc'}
ODD = {'challenge_'+n+'_p8_alternate' for n in ['ducksIFS','gnarlyIFS','gnarly2IFS','gnarly3IFS']}
GEN = {'selector_complete__GenCWarp_5_-1','selector_complete__GenCWarp_6_-1'}
MANO = {'challenge_ManowarBulb_'+n for n in ['early','julia','p0_alternate','p0_zero',
        'p1_alternate','p1_zero','p2_alternate','p2_zero','p3_alternate','p3_zero',
        'p4_alternate','p5_alternate','p6_alternate']}
ABOX = {'challenge_ABoxMod4d_'+n for n in ['early','julia','p0_zero',
        'p0_alternate','p1_zero','p1_alternate','p2_alternate']}
LIVE_DOCS = ['README.md','docs/ARCHITECTURE.md','docs/BUILD.md',
             'docs/PARITY_TRACE.md']

def require(ok, message):
    if not ok:
        raise ValueError(message)

def sha(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()

def read(path):
    return json.loads(Path(path).read_bytes())

def validate_abox_cases(points):
    rows = points['results']
    require(len(rows)==7 and {r['case'] for r in rows}==ABOX,
            'Wrong ABox case identities')
    require(all(r['cases']==2092 for r in rows), 'Incomplete ABox inputs')

class Audit:
    def __init__(self, run):
        self.run = run.resolve()
        self.verified = {}

    def checked(self, path, digest=None):
        path = Path(path).resolve()
        value = sha(path)
        require(digest is None or value == digest, 'Changed artifact: '+str(path))
        self.verified[str(path)] = value
        return path

    def load(self, relative):
        return read(self.checked(self.run/relative))

    def hashes(self, mapping, base):
        for path, digest in mapping.items():
            self.checked(Path(base)/path, digest)

    def receipt(self, name):
        data = self.load('logs/'+name+'.json')
        require(data['finished'] and data['exit_code'] == 0, 'Failed command: '+name)
        self.checked(self.run/'logs'/(name+'.log'), data['log_sha256'])
        return data

    def artifacts(self, relative):
        s = self.load(relative)
        self.hashes(s.get('artifact_sha256', {}), (self.run/relative).parent)
        self.hashes(s.get('identities', {}), ROOT)
        return s

    def baseline_difference(self, lsb, current):
        """Bind the sole tolerated mismatch to this exact retained baseline case."""
        case = current['scene']
        require(case==lsb['case']=='challenge__IntPowRotateC_p4_zero',
                'Wrong one-LSB case identity')
        require(lsb['old_vs_new_rgb_exact'], 'Unexplained new pixel difference')
        paths = {key:self.checked(ROOT/lsb[key],lsb[key+'_sha256'])
                 for key in ['old_summary','old_image','new_image']}
        old = read(paths['old_summary'])
        matches = [r for r in old['results'] if r['scene']==case]
        require(len(matches)==1 and matches[0]==lsb['old_result'],
                'One-LSB receipt not bound to historical result')
        previous = matches[0]
        require(previous['suite']==current['suite'] and
                previous['scene_sha256']==current['scene_sha256'] and
                previous['pascal_image_sha256']==current['pascal_image_sha256'],
                'Different one-LSB scene or Pascal reference')
        require(paths['new_image']==(self.run/'affected-images-final/cpp'/current['suite']/(case+'.png')).resolve(),
                'One-LSB receipt names a different current image')
        require(paths['old_image']==(paths['old_summary'].parent/'cpp'/previous['suite']/(case+'.png')).resolve(),
                'One-LSB receipt names a different historical image')
        require(lsb['new_image_sha256']==current['cpp_image_sha256'] and
                lsb['old_image_sha256']==previous['cpp_image_sha256'],
                'One-LSB image hash not bound to result')
        require(np.array_equal(load_rgb(paths['old_image']),load_rgb(paths['new_image'])),
                'Baseline preservation claim failed')

    def run_audit(self):
        identity = self.load('manowar/candidate-v2/identity.json')
        self.hashes(identity['sha256'], ROOT)
        renderer = self.checked(self.run/'manowar/candidate-v2/mb3d_render.exe')
        probe = self.checked(self.run/'manowar/candidate-v2/mb3d_formula_point_probe.exe')
        # All production sources from the preceding full run are accounted for.
        oldfreeze = read(self.checked(ROOT/'comparison/runs/compatibility-final-20261018/candidate/application-freeze.json'))
        production = {p:sha(self.checked(ROOT/p)) for p in oldfreeze['production_sources']}
        changes = {p for p,h in production.items() if h != oldfreeze['production_sources'][p]}
        require(changes == CHANGED, 'Unexpected production delta: '+str(changes))
        require(not subprocess.check_output(['git','status','--porcelain','--','mb3d'],cwd=ROOT).strip(), 'Immutable reference modified')

        mano = self.load('manowar/final-revalidation.json')
        require((mano['profile_count'],mano['orbit_count'],mano['native_checks'],mano['independent_original_case_buffers'],mano['negative_fixture_checks']) == (31,2976,91985,13,8), 'Incomplete caller qualification')
        require(not mano['stage_a_complete'] and not mano['generic_point_reclassified'], 'Unsound generic promotion')
        self.hashes(mano['evidence_sha256'], ROOT)
        self.receipt('manowar-final-audit')
        loader = self.load('manowar/loader13-current/summary.json')
        require({r['case'] for r in loader['results']} == MANO, 'Wrong original Manowar IDs')

        gen = self.artifacts('gencwarp/summary.json')
        self.hashes(gen['build']['sha256'], ROOT)
        require(gen['finished'] and gen['status']=='PASS_REFERENCE_DOMAIN_PROOF', 'GenCWarp proof failed')
        require(gen['native_experiments']==2352 and gen['counts']['valid_controls']==2304, 'GenCWarp coverage incomplete')
        require(not gen['numerical_parity_for_invalid_selectors'] and {r['case'] for r in gen['rejected_cases']}==GEN, 'Invalid selector incorrectly accepted')

        odd = self.artifacts('odd-flip/summary.json')
        require(odd['finished'] and set(odd['case_ids'])==ODD, 'Wrong odd-Flip cases')
        require(odd['totals']==dict(application_processes=72,odd_processes=60,even_control_processes=12,original_caller_captured_rows=3584,captured_replay_exact_context_rows=3584,strict_cpp_rejections=8), 'Incomplete odd-Flip matrix')
        require(not odd['seeded_registers_used_in_application'] and not odd['caller_memory_substitution_in_application'], 'Synthetic original caller')
        for rel,count in [('application-v1',56),('original-caller-v2',16)]:
            s = self.artifacts('odd-flip/'+rel+'/summary.json')
            require(s['finished'] and len(s['results'])==count, 'Incomplete original process matrix')
        self.artifacts('odd-flip/captured-input-replay/summary.json')
        for row in odd['results']:
            require(row['rgb_and_full_live_state_invariant'], 'Unstable original image/state')
            if row['odd_flip']:
                require(row['cpp_numerical_parity_pass'] is False, 'Unsupported odd-Flip promoted')

        nonterm = self.load('nontermination/final-revalidation.json')
        self.receipt('nontermination-final-audit')
        require(nonterm['finished'] and nonterm['counts']['cases']==24 and nonterm['counts']['source_nontermination_orbit_proofs']==18 and nonterm['counts']['excessive_finite_orbit_witnesses']==6, 'Wrong nontermination classification')
        require(not nonterm['original_numerical_parity_created'] and not nonterm['universal_image_nontermination_proven'], 'Overstated source proof')
        self.checked(nonterm['witness_report']['path'], nonterm['witness_report']['sha256'])
        require(len(nonterm['validator_negative_controls'])==10 and all(v['rejected'] for v in nonterm['validator_negative_controls']), 'Fail-open witness validator')

        rejections = self.artifacts('rejections-final/summary.json')
        self.hashes(rejections['artifacts'],self.run/'rejections-final')
        require(rejections['finished'] and len(rejections['results'])==10 and all(r['status']=='PASS_EXPECTED_REJECTION' for r in rejections['results']), 'Current rejection regression failed')
        require(set(r['case'] for r in rejections['results'])==ODD|GEN, 'Incomplete current rejection cases')
        self.receipt('rejections-final')

        impact = self.load('affected-corpus-final.json')
        require(len(impact['external'])==5506 and len(impact['ordinary'])==343, 'Incomplete scene impact scan')
        for row in impact['external']+impact['ordinary']:
            self.checked(row['scene'],row['sha256'])
        affected = {r['case'] for r in impact['external'] if r['affected']}
        require(len(affected)==39 and not any(r['affected'] for r in impact['ordinary']), 'Wrong affected set')
        images = self.load('affected-images-final/summary.json')
        require(images['finished'] and images['cpp_sha256']==sha(renderer), 'Wrong image candidate')
        require(len(images['results'])==39 and {r['scene'] for r in images['results']}==affected, 'Wrong replay coverage')
        require(images['thresholds']==DEFAULT_THRESHOLDS, 'Relaxed image gates')
        for row in images['results']:
            require(row['status']=='pass', 'Affected image mismatch')
            cpp = self.checked(self.run/'affected-images-final/cpp'/row['suite']/(row['scene']+'.png'),row['cpp_image_sha256'])
            pas = self.checked(ROOT/'comparison/runs/external-complete/parameter-parity-batch/pascal'/(row['scene']+'.png'),row['pascal_image_sha256'])
            a,b=load_rgb(cpp),load_rgb(pas)
            actual=compute_metrics(a,b,int(DEFAULT_THRESHOLDS['diff_ratio_threshold']))
            require(all(evaluate(actual,DEFAULT_THRESHOLDS).values()), 'Recomputed image gate failure')
            require(bool(np.array_equal(a,b))==row['exact_rgb'], 'RGB does not match reported equality')
            if not row['exact_rgb']:
                require(np.count_nonzero(a!=b)==1 and actual.max_abs_delta==1,
                        'Difference is not exactly one channel at one LSB')
        require(sum(r['exact_rgb'] for r in images['results'])==38, 'Unexpected exact-image count')
        lsb=self.load('one_lsb_baseline_comparison.json')
        self.baseline_difference(lsb,next(r for r in images['results'] if not r['exact_rgb']))
        self.receipt('affected-images-final')
        new_image_counts = {}
        for rel,profile in [('hybrid-images-final','hybrid13'),('producer-images-final','producer18')]:
            s = self.artifacts(rel+'/summary.json')
            require(s['finished'] and s['profile']==profile, 'Wrong new image profile')
            require(len(s['results'])==len(PROFILES[profile]) and {r['id'] for r in s['results']}==PROFILES[profile], 'Wrong new image cases')
            require(s['thresholds']==DEFAULT_THRESHOLDS and s['identities'].get(str(renderer))==sha(renderer), 'Wrong new image candidate/gates')
            nonconstant=0
            for row in s['results']:
                self.checked(row['scene'],row['scene_sha256'])
                require(row['status']=='PASS' and row['exact_rgb'], 'New image failure')
                a,b=[load_rgb(self.run/rel/row['id']/(language+'.png')) for language in ['pascal','cpp']]
                require(np.array_equal(a,b), 'New image RGB mismatch')
                nonconstant += bool(np.any(a != a[0,0]))
            require(nonconstant==s['counts']['nonconstant'], 'Wrong image content claim')
            new_image_counts[profile] = s['counts']
            self.receipt(rel)

        visible_rel='producer-image-controls-source-directed'
        visible=self.artifacts(visible_rel+'/summary.json')
        visible_seal=self.load(visible_rel+'/qualification.json')
        self.checked(ROOT/visible_seal['run_summary'],visible_seal['run_summary_sha256'])
        self.checked(ROOT/visible_seal['immutable_numerical_qualification'],
                     visible_seal['immutable_numerical_qualification_sha256'])
        require(visible_seal['status']=='PASS' and visible_seal['candidate_sha256']==sha(renderer),
                'Wrong visible-control qualification')
        require(visible['finished'] and len(visible['results'])==2 and
                {r['id'] for r in visible['results']}=={'visible_ipm_0','visible_mip_0'},
                'Missing visible producer order')
        require(visible['thresholds']==DEFAULT_THRESHOLDS and
                visible['identities'].get(str(renderer))==sha(renderer),
                'Wrong visible-control candidate/gates')
        for row in visible['results']:
            directory=self.run/visible_rel/row['id']
            self.checked(ROOT/row['scene'],row['scene_sha256'])
            cpp,pas,previous=[load_rgb(directory/(tag+'.png')) for tag in ['cpp','pascal','before']]
            require(row['status']=='PASS' and row['exact_rgb'] and np.array_equal(cpp,pas),
                    'Visible producer RGB mismatch')
            require(np.any(cpp!=cpp[0,0]), 'Constant visible producer image')
            payload=(directory/'cpp.silight').read_bytes()
            magic,version,w,h,size,count=struct.unpack_from('<8s5I',payload)
            require(magic==b'SIL5DMP1' and version==1 and size==18 and count==w*h and
                    len(payload)==28+count*18,'Invalid visible-control geometry dump')
            records=np.frombuffer(payload,dtype='<u2',offset=28).reshape(-1,9)
            require(len(np.unique(records[:,4]))==row['unique_depths']>8 and
                    len(np.unique(records[:,:3],axis=0))==row['unique_normals']>8,
                    'Unsubstantiated visible geometry')
            old_metrics=compute_metrics(previous,pas,int(DEFAULT_THRESHOLDS['diff_ratio_threshold']))
            require(row['before_status']=='PASS' and all(evaluate(old_metrics,DEFAULT_THRESHOLDS).values()),
                    'Incorrect earlier-candidate acceptance claim')
            require(not np.array_equal(previous,pas),'No earlier-candidate pixel difference')
        new_image_counts['producer_visible2']=dict(passed=2,exact=2,nonconstant=2)

        points = self.load('aboxmod4d-points-final/summary.json')
        require(points['finished'] and points['cpp_sha256']==sha(probe), 'Wrong point candidate')
        validate_abox_cases(points)
        require(all(r['status']=='pass' and r['failed_lanes']==0 and r['max_scaled_error']==0 for r in points['results']), 'ABox point mismatch')
        for row in points['results']:
            self.checked(self.run/'aboxmod4d-points-final'/(row['case']+'.params'),row['parameter_sha256'])
        self.receipt('aboxmod4d-points-final')

        for name in ['ordinary-build-final','ordinary-ctest-final','python-tests-final']:
            self.receipt(name)
        xml=ET.parse(self.checked(self.run/'ordinary-ctest-final.xml')).getroot()
        tests=xml.findall('testcase')
        require(len(tests)==21 and xml.get('tests')=='21' and all(xml.get(k)=='0' for k in ['failures','skipped','disabled']), 'Incomplete CTest gate')
        require(all(t.get('status')=='run' and t.find('failure') is None for t in tests), 'CTest test did not pass')
        require({'manowar_hybrid_tests','manowar_sse_producer_tests','renderer_tests'} <= {t.get('name') for t in tests}, 'Missing core regression')
        self.hashes(self.load('tests-candidate-final/identity.json'), self.run/'tests-candidate-final')
        testlog=(self.run/'tests-candidate-final/LastTest.log').read_text()
        require('15437368 passed, 0 failed' in testlog and '1248 orbits, 41693 checks, 0 failures' in testlog and '1728 orbits, 50292 checks, 0 failures' in testlog, 'Missing assertion coverage')
        pylog=(self.run/'logs/python-tests-final.log').read_text()
        require(re.search(r'Ran 154 tests\b',pylog) and re.search(r'^OK$',pylog,re.M), 'Python suite incomplete')
        for name in ['test_windows_workspace','test_windows_path_hygiene','test_windows_helper_local_paths']:
            self.receipt(name)

        case_rows=[]
        for case in sorted(MANO):
            case_rows.append(dict(id=case, prior_categories=['point_unverified'], result='INDEPENDENT_LOADER_EXACT_AND_QUALIFIED_CALLER_PROFILES', generic_point_pass=False, evidence='manowar/qualification.json'))
        for case in sorted(ODD):
            case_rows.append(dict(id=case, prior_categories=['point_unverified','image_excluded'], result='ORIGINAL_CALLER_REPRODUCIBLE_CPP_STRICT_REJECTION', generic_point_pass=False, image_pass=False, evidence='odd-flip/summary.json'))
        for row in nonterm['cases']:
            categories=['image_excluded']
            if row['formula']=='TrifoxComplexAngles': categories.append('point_unverified')
            case_rows.append(dict(id=row['id'],prior_categories=categories,result=row['classification'],source_orbit_nontermination_proven=row['nontermination_proven'],image_pass=False,evidence='nontermination/final-revalidation.json'))
        for case in sorted(GEN):
            case_rows.append(dict(id=case,prior_categories=['selector_original_undefined'],result='ORIGINAL_X87_DOMAIN_PROOF_CPP_STRICT_REJECTION',generic_point_pass=False,evidence='gencwarp/summary.json'))
        require(len(case_rows)==len({r['id'] for r in case_rows})==43,'Wrong distinct gap ID inventory')
        categories={k:sum(k in r['prior_categories'] for r in case_rows) for k in ['point_unverified','image_excluded','selector_original_undefined']}
        require(categories==dict(point_unverified=19,image_excluded=28,selector_original_undefined=2),'Wrong overlapping original denominators')

        documentation = {p:sha(self.checked(ROOT/p)) for p in LIVE_DOCS}
        review = self.load('final-qualification-review.json')
        require({r['id'] for r in review['findings']}=={'R1','R2'},'Unexpected review finding set')
        negative = self.load('collector-negative-controls.json')
        require(negative['audit_tool_sha256']==sha(Path(__file__)) and
                negative['checker_sha256']==sha(self.checked(Path(__file__).with_name('check_reference_gap_collector_controls.py'))) and
                negative['finished'] and negative['positive_controls']==2 and
                len(negative['negative_controls'])==8 and
                all(r['rejected'] for r in negative['negative_controls']),
                'Collector identity rejection controls incomplete')
        self.hashes(negative['inputs_sha256'],ROOT)

        return dict(schema_version=1,created_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),
            status='PASS_SCOPED_REFERENCE_GAP_QUALIFICATION_WITH_EXPLICIT_LIMITS',
            scope='Four requested reference-gap workstreams and three resulting CPU compatibility fixes; Monte Carlo/JIT deferred',
            workstreams_completed=4,workstreams_total=4,full_pascal_parity_complete=False,stage_a_complete=False,overall_project_percentage=None,
            candidate=dict(renderer=str(renderer),renderer_sha256=sha(renderer),point_probe=str(probe),point_probe_sha256=sha(probe)),
            previous_full_corpus_qualification=dict(path=str(self.checked(OLD/'qualification.json')),sha256=sha(OLD/'qualification.json'),applies_to_previous_binary=True),
            production_sources=production,changed_from_full_corpus_baseline=sorted(changes),
            live_documentation_sha256=documentation,
            collector_review=dict(evidence='final-qualification-review.json',resolved_findings=['R1','R2'],negative_controls=8),
            fixes=mano['fixes'], tests=dict(ctest_suites=21,project_suites=20,zlib_suites=1,renderer_assertions=15437368,python_tests=154,python_modules=25,windows_standalone_checks=3),
            numerical_evidence=dict(caller_profiles=31,independent_orbits=2976,native_caller_checks=91985,original_prepared_buffers_exact=13,aboxmod4d_inputs_exact=14644,affected_corpus_images_pass=39,affected_corpus_images_exact=38,preexisting_one_channel_one_lsb_difference=lsb['case'],new_images=new_image_counts),
            reference_evidence=dict(odd_flip_processes=72,original_context_replays_exact=3584,source_orbit_nontermination_proofs=18,excessive_but_finite_orbits=6,gencwarp_experiments=2352,gencwarp_valid_controls=2304,current_strict_rejections=10),
            prior_overlapping_categories=categories,distinct_case_count=43,cases=case_rows,
            limitations=[
                'These 43 IDs have classified evidence, not 43 new numerical PASS results; prior full-corpus exclusions and generic point statuses are unchanged.',
                'The new candidate has scoped image coverage plus ordinary tests; the historical full-corpus numerical results belong to the prior binary.',
                'Manowar qualification covers the recorded FPC/SSE2 caller profiles. Arbitrary negative saved counts and other uncounted SSE-producing neighbours require additional caller qualification.',
                'odd-Flip references are reproducible for these original callers; C++ continues rejecting the four inputs. Arbitrary stack/address-dependent semantics remain unsupported.',
                '18 source/orbit invariants and 6 finite counter-wrap witnesses do not prove whole-image termination or nontermination for every pixel.',
                'Invalid GenCWarp selectors have no generic numeric contract. Diagnostic x87 seeds are forensic witnesses, not the application caller.',
                'ABoxMod4d x87 improvement is qualified for native MSVC x64. The portable path, GPU and Apple Silicon are not newly qualified.',
                'Numerical equality allows the stated 2e-12 tolerance and matching NaN/Inf classes; no NaN payload or signed-zero bitwise parity is asserted.',
                'The producer18 image matrix is spatially constant; its branch correctness is established by independent orbit and incoming-lane checks. Two additional visible controls cover both orders with exact RGB; their earlier-candidate images also pass the general image gates despite localized differences.'
            ], audit_tool_sha256=sha(Path(__file__)),verified_artifact_count=len(self.verified),verified_sha256=self.verified)

def main():
    ap=argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--run',type=Path,required=True)
    ap.add_argument('--output',type=Path,required=True)
    args=ap.parse_args()
    require(not args.output.exists(),'Refuse to overwrite a qualification')
    result=Audit(args.run).run_audit()
    args.output.parent.mkdir(parents=True,exist_ok=True)
    args.output.write_text(json.dumps(result,indent=2)+'\n')
    print(json.dumps({k:result[k] for k in ['status','distinct_case_count','tests','numerical_evidence','verified_artifact_count']},indent=2))

if __name__=='__main__':
    main()
