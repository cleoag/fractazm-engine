"""Seal explicitly supplied ordinary supplemental evidence; never run a renderer.

The pinned historical reports define corpus identities, not current success.
Unknown/missing schemas, artifacts, statuses and candidate identities fail closed.
Only guarded_cpp_finite may reuse an old report, and only for the identical exe.
"""
from __future__ import annotations

import argparse
from collections import Counter
import hashlib
import json
import math
from pathlib import Path

from PIL import Image

from image_diff import DEFAULT_THRESHOLDS
from audit_stage_a_release import SELECTOR_UNVERIFIED, allowed_unverified

ROOT = Path(__file__).resolve().parents[2]
POLICY = Path(__file__).with_name('audit_final_supplementary_policy.json')
FIELDS = {'normal_x', 'normal_y', 'normal_z', 'rough_zpos_fine', 'zpos',
          'shadow', 'amb_shadow', 'si_gradient', 'o_trap'}
MANOWAR_SUFFIXES = ('early', 'julia', 'p0_alternate', 'p0_zero', 'p1_alternate',
                   'p1_zero', 'p2_alternate', 'p2_zero', 'p3_alternate',
                   'p3_zero', 'p4_alternate', 'p5_alternate', 'p6_alternate')


def require(value, message):
    if not value:
        raise ValueError(message)


def digest(path):
    with Path(path).open('rb') as stream:
        return hashlib.file_digest(stream, 'sha256').hexdigest()


def check_hash(path, expected):
    require(isinstance(expected, str) and len(expected) == 64,
            f'Missing SHA-256 for {path}')
    actual = digest(path)
    require(actual == expected, f'Changed artifact: {path}')
    return actual


def check_stdin_hash(path, expected):
    # check_integer_boundaries hashes the logical LF stdin, but write_text
    # stores CRLF on Windows. Normalize only that translation; the Audit.file
    # caller separately seals the exact on-disk bytes.
    data = Path(path).read_bytes().replace(b'\r\n', b'\n')
    require(hashlib.sha256(data).hexdigest() == expected,
            f'Changed stdin artifact: {path}')


def check_candidate(actual, expected, label):
    require(isinstance(actual, str) and actual == expected,
            f'{label}: wrong or missing candidate hash')


def index_rows(rows, fields, label):
    require(isinstance(rows, list), label + ': missing row array')
    indexed = {}
    for row in rows:
        require(isinstance(row, dict), label + ': non-object row')
        key = tuple(row.get(k) for k in fields)
        require(all(v is not None for v in key) and key not in indexed,
                label + ': missing or duplicate row identity ' + repr(key))
        indexed[key] = row
    return indexed


def same_cases(rows, expected, fields, label):
    actual = index_rows(rows, fields, label)
    prior = index_rows(expected, fields, label + ' corpus')
    require(actual.keys() == prior.keys(), label + ': missing/unexpected cases')
    for key, row in actual.items():
        for field in ('scene_sha256', 'source_sha256', 'sha256'):
            if field in prior[key]:
                require(row.get(field) == prior[key][field],
                        label + ': changed input identity ' + repr(key))
    return actual


def check_statuses(rows, allowed, label):
    for row in rows:
        require(row.get('status') in allowed,
                label + ': nonpassing or missing status ' + repr(row.get('case', row.get('id'))))


def check_selector_status(row):
    if row.get('case') in SELECTOR_UNVERIFIED:
        require(row.get('status') == 'unverified' and allowed_unverified('selector_points', row),
                'Known original-undefined selector must remain an explicit rejection/unverified')
    else:
        require(row.get('status') == 'pass', 'Selector numerical failure/error')
        require(row.get('failed_lanes') == 0 and row.get('examples') == [],
                'Selector has failed lanes or counterexamples')
        require(all(n == 0 for n in row.get('lane_failures', [1])), 'Selector lane failure')
        require(float(row['max_scaled_error']) <= 2e-12, 'Selector tolerance exceeded')
    native = row.get('native_selector_check', {})
    require('expected' in native and native.get('loaded') == native['expected'],
            'Effective native selector does not match requested selector')


def check_metrics(metrics):
    require(isinstance(metrics, dict), 'Missing image metrics')
    for key in ('max_abs_delta', 'mean_abs_delta', 'p99_abs_delta', 'diff_ratio'):
        value = float(metrics[key])
        require(math.isfinite(value) and 0 <= value <= DEFAULT_THRESHOLDS[key],
                'Image gate failed: ' + key)
    require(metrics.get('diff_ratio_threshold') == 2, 'Changed image difference definition')
    require(float(metrics['psnr']) >= 40 and float(metrics['ssim']) >= .98,
            'Image PSNR/SSIM gate failed')


def check_state(row):
    changed = row.get('changed_fields')
    if changed is None and isinstance(row.get('fields'), dict):
        changed = {k: v.get('changed') for k, v in row['fields'].items()}
    require(isinstance(changed, dict) and set(changed) == FIELDS and
            all(v == 0 for v in changed.values()), 'Required exact nine-field packed state did not match')
    require(row.get('state_status', 'PASS') in ('PASS', 'pass'), 'State status failed')


def rgb(path):
    with Image.open(path) as image:
        return image.size, image.convert('RGB').tobytes()


class Audit:
    def __init__(self, manifest):
        require(manifest.get('schema_version') == 1, 'Unknown manifest schema')
        self.manifest = manifest
        self.new_root = self.path(manifest['new_root'])
        self.files = {}
        self.historical = []
        self.reports = {}
        self.identities = manifest['identities']
        require(set(self.identities) == {'renderer', 'point_probe', 'orbit_probe',
                'scene_loader_probe', 'm3i_probe', 'guarded_pascal'}, 'Missing/unexpected binary roles')
        for item in self.identities.values():
            require(item.get('sha256'), 'Binary identity must include SHA-256')
            self.file(item['path'], item['sha256'])
        self.policy = json.loads(POLICY.read_bytes())
        self.file(POLICY)
        require(set(manifest['reports']) == set(self.policy['reports']) | {'manowar_caller'},
                'Missing/unexpected supplemental report roles')

    @staticmethod
    def path(value):
        require(isinstance(value, (str, Path)), 'Required path not supplied')
        p = Path(value)
        return (p if p.is_absolute() else ROOT / p).resolve()

    def file(self, path, expected=None, current=False):
        p = self.path(path)
        if current:
            require(p.is_relative_to(self.new_root), f'Current output outside new run: {p}')
        require(p.is_file(), f'Missing artifact: {p}')
        actual = check_hash(p, expected) if expected is not None else digest(p)
        key = str(p)
        require(key not in self.files or self.files[key] == actual, f'Artifact changed during audit: {p}')
        self.files[key] = actual
        return p

    def read(self, item, current=False):
        require(isinstance(item, dict) and item.get('sha256'), 'Report identity must include SHA-256')
        p = self.file(item['path'], item['sha256'], current)
        return p, json.loads(p.read_bytes())

    def binary(self, report, key, role='renderer'):
        check_candidate(report.get(key), self.identities[role]['sha256'], key + '/' + role)

    def command(self, command, role='renderer'):
        require(isinstance(command, list) and command, 'Missing execution command')
        require(self.path(command[0]) == self.path(self.identities[role]['path']), 'Wrong executed binary path')

    @staticmethod
    def arg(command, option):
        require(option in command, 'Missing command option ' + option)
        return command[command.index(option) + 1]

    def linked_hashes(self, value, artifact_root=None):
        """Verify explicit path/hash pairs and hash maps; do not guess bare hashes."""
        if isinstance(value, list):
            for child in value:
                self.linked_hashes(child, artifact_root)
        elif isinstance(value, dict):
            if isinstance(value.get('path'), str) and value.get('sha256'):
                self.file(value['path'], value['sha256'])
            for name in ('identities', 'artifact_sha256', 'input_sha256', 'libraries'):
                mapping = value.get(name)
                if isinstance(mapping, dict):
                    for path, expected in mapping.items():
                        if isinstance(expected, str) and len(expected) == 64:
                            if name == 'artifact_sha256' and not Path(path).is_absolute():
                                require(artifact_root is not None, 'Relative artifact map needs report directory')
                                target = (artifact_root / path).resolve()
                                require(target.is_relative_to(artifact_root.resolve()), 'Artifact escapes report directory')
                                self.file(target, expected)
                            else:
                                self.file(path, expected)
            for key, expected in value.items():
                if key.endswith('_sha256') and isinstance(expected, str):
                    stem = key[:-7]
                    path = value.get(stem)
                    if isinstance(path, str) and (self.path(path).is_file()):
                        self.file(path, expected)
            for child in value.values():
                if isinstance(child, (dict, list)):
                    self.linked_hashes(child, artifact_root)

    def output(self, row, report_path, role='renderer'):
        if 'command' in row:
            command = row['command']; self.command(command, role)
            require(row.get('returncode') == 0, 'Image command failed')
            option = '--output' if '--output' in command else '-o'
            p = self.arg(command, option)
            expected = row.get('image_sha256', row.get('cpp_image_sha256', row.get('output_sha256')))
            require(expected, 'Missing rendered image hash')
            self.file(p, expected, True)
            for flag, names in [('--silight5-dump', ('dump_sha256', 'depth_sha256', 'cpp_state_sha256')),
                                ('--parity-trace', ('trace_sha256',))]:
                if flag in command:
                    expected = next((row[k] for k in names if k in row), None)
                    require(expected or flag == '--parity-trace', 'Missing state output hash')
                    self.file(self.arg(command, flag), expected, True)
        else:
            p = report_path.parent / 'cpp' / row['suite'] / (row['scene'] + '.png')
            self.file(p, row['cpp_image_sha256'], True)
            ref = self.file(row['reference_summary'], row['reference_summary_sha256'])
            self.file(ref.parent / 'pascal' / (row['scene'] + '.png'), row['pascal_image_sha256'])

    def images(self, path, report, expected, exact=False, state=False):
        require(report.get('finished', True) is True, 'Unfinished image report')
        require(report.get('thresholds') == DEFAULT_THRESHOLDS, 'Changed/missing image thresholds')
        self.binary(report, 'cpp_sha256' if 'cpp_sha256' in report else 'binary_sha256')
        rows = report['results']
        fields = ('scene', 'suite') if rows and 'scene' in rows[0] else ('id',)
        same_cases(rows, expected['results'], fields, str(path))
        check_statuses(rows, {'pass', 'PASS'}, str(path))
        for row in rows:
            check_metrics(row.get('diff', row.get('metrics')))
            if 'checks' in row:
                require(set(row['checks']) == {'max_abs_delta','mean_abs_delta','p99_abs_delta','diff_ratio','psnr','ssim'}
                        and all(v is True for v in row['checks'].values()), 'Image check failure/missing gate')
            if exact: require(row.get('exact_rgb') is True, 'Required exact RGB did not match')
            if state:
                check_state(row)
            self.output(row, path)
        if 'counts' in report:
            require(report['counts'] == dict(Counter(r['status'] for r in rows)), 'Image aggregate/rows mismatch')
        return len(rows)

    def replay_children(self, path, report, prior, finite=False):
        self.binary(report, 'cpp_sha256')
        require(report.get('finished') is True, 'Unfinished combined replay')
        key = 'matrices' if finite else 'jobs'
        require(len(report[key]) == len(prior[key]), 'Combined replay child count mismatch')
        total = 0
        for i, child in enumerate(report[key]):
            previous = prior[key][i]
            require(child.get('returncode') == 0, 'Child replay failed')
            name = 'report' if finite else 'summary'
            child_path = self.file(child[name], child.get('summary_sha256'), True)
            c = json.loads(child_path.read_bytes())
            expected_hash = (self.policy['reports']['finite_matrices']['children'][previous['name']]
                             if finite else previous['summary_sha256'])
            previous_path = self.file(previous[name], expected_hash)
            old = json.loads(previous_path.read_bytes())
            if finite and child['name'] == 'interpolation2':
                # This adapter creates its manifest inside the output directory;
                # replay_parity derives suite from that directory's name.
                # Scene names and hashes still have to match the pinned corpus.
                require({r['suite'] for r in old['results']} == {previous_path.parent.parent.name} and
                        {r['suite'] for r in c['results']} == {child_path.parent.parent.name},
                        'Unexpected interpolation manifest-derived suite')
                old = dict(old, results=[dict(r, suite=child_path.parent.parent.name) for r in old['results']])
            total += self.images(child_path, c, old)
            if finite:
                require(child['name'] == previous['name'] and child['expected'] == previous['expected'], 'Changed finite matrix')
                require(len(c['results']) == child['expected'], 'Finite child missing cases')
            else:
                projection = ('scene','scene_sha256','status','exact_rgb','cpp_ms','error')
                require([{k:r.get(k) for k in projection} for r in c['results']] == child['results'], 'Combined/child rows disagree')
        require(report['counts'] == {'pass':total}, 'Combined aggregate mismatch')
        return total

    def diagnostics(self, path, report, prior, role):
        self.binary(report, 'binary_sha256', role)
        require(report.get('finished') is True, 'Diagnostics unfinished')
        same_cases(report['runs'], prior['runs'], ('case','threads','mode'), 'diagnostics runs')
        same_cases(report['comparisons'], prior['comparisons'], ('case','threads','mode'), 'diagnostics comparisons')
        require(len(report['runs']) == 24 and len(report['comparisons']) == 18, 'Wrong diagnostic denominator')
        require(all(r.get('exact_rgb') is True for r in report['comparisons']), 'Diagnostic instrumentation/repeat changed RGB')
        pixels = {}
        for row in report['runs']:
            self.command(row['command'],role); require(row.get('returncode') == 0,'Diagnostic run failed')
            p = self.file(self.arg(row['command'],'--output'), current=True)
            value = rgb(p); require(hashlib.sha256(value[1]).hexdigest() == row['rgb_sha256'], 'Diagnostic RGB hash mismatch')
            pixels[(row['case'],row['threads'],row['mode'])] = value
            for flag in ('--parity-trace','--diag-trace','--ray-setup-dump'):
                if flag in row['command']: self.file(self.arg(row['command'],flag), current=True)
        for row in report['comparisons']:
            require(pixels[(row['case'],row['threads'],'plain')] == pixels[(row['case'],row['threads'],row['mode'])], 'Diagnostic pixels differ')
        return 24

    def guarded(self, path, report, prior, application):
        require(report.get('finished') is True and report.get('success') is True, 'Guard report unfinished/failed')
        if application:
            self.binary(report['binary_sha256'], 'cpp')
            self.binary(report['binary_sha256'], 'pascal', 'guarded_pascal')
        else:
            self.binary(report,'binary_sha256','guarded_pascal')
        rows = report['results']; same_cases(rows,prior['results'],('case','engine','threads'),'guard cases')
        require(report.get('passed') == len(rows) == report.get('checks'), 'Guard aggregate mismatch')
        configs = report['cases'] + report.get('controls', [])
        prior_configs = prior['cases'] + prior.get('controls', [])
        same_cases(configs, prior_configs, ('id',), 'guard inputs')
        previous = {r['id']: r for r in prior_configs}
        for case in configs:
            self.file(case['scene'], case['sha256'])
            for key in ('expect', 'exit_codes', 'diagnostics', 'required_diagnostics', 'reference_sha256'):
                require(case.get(key) == previous[case['id']].get(key), 'Changed guard expectation ' + key)
        cases = {r['id']:r for r in report['cases'] + report.get('controls',[])}
        for row in rows:
            require(row.get('contract_pass') is True, 'Guard hidden behavioral failure')
            case = cases[row['case']]; role = 'renderer' if row['engine']=='cpp' else 'guarded_pascal'
            self.command(row['command'],role)
            output = self.path(self.arg(row['command'],'--output'))
            label = f"{row['engine']}_t{row['threads']}_{row['case']}"
            logs = []
            for stream in ('stdout','stderr'):
                p=self.file(path.parent/(label+'.'+stream+'.txt'),row[stream+'_sha256'],True)
                logs.append(p.read_text(encoding='utf-8',errors='replace'))
            if case['expect']=='rejected':
                require(row['returncode'] in case['exit_codes'][row['engine']] and not output.exists(), 'Guard rejection returned success/partial image')
                text='\n'.join(logs)
                require(any(s in text for s in case['diagnostics'][row['engine']]) and
                        all(s in text for s in case.get('required_diagnostics',{}).get(row['engine'],[])), 'Wrong guard rejection reason')
                require(row.get('parity_status') in ('unverified_original_nontermination','unverified_original_excessive_work'), 'Rejection promoted to numerical parity')
            else:
                require(row.get('exact_original_rgb') is True and row['returncode']==0,'Finite guard control failed')
                self.file(output,row['image_sha256'],True)
                ref=self.file(case['reference_image'],case['reference_sha256'])
                require(rgb(output)==rgb(ref),'Finite guard pixels changed')
        return len(rows)

    def validate(self, role, path, report, prior):
        if role == 'selector_points':
            require(report.get('finished') is True and report.get('selected_cases')==591 and report.get('native_selectors_verified')==591,'Incomplete selectors')
            same_cases(report['results'],prior['results'],('case',),'selector corpus')
            require(report['identities'].get(str(self.path(self.identities['point_probe']['path'])))==self.identities['point_probe']['sha256'],'Wrong selector probe')
            for row in report['results']: check_selector_status(row)
            require(report['counts']=={'pass':589,'unverified':2},'Wrong selector statuses')
            for job in report['jobs']:
                child=self.file(self.path(self.arg(job['command'],'--output'))/'summary.json',job['summary_sha256'],True)
                data=json.loads(child.read_bytes());self.binary(data,'cpp_sha256','point_probe')
                require(data.get('finished') is True,'Unfinished selector child')
                for row in data['results']:
                    matching=next(r for r in report['results'] if r['case']==row['case'])
                    require(row['status']==matching['status'] and row.get('parameter_sha256')==matching.get('parameter_sha256'),'Selector child mismatch')
                    if row['status']=='pass': self.file(child.parent/(row['case']+'.params'),row['parameter_sha256'],True)
            return 591
        if role == 'integer_boundaries':
            self.binary(report,'cpp_sha256','point_probe');require(report.get('finished') is True,'Integers unfinished')
            rows=report['results'];same_cases(rows,prior['results'],('case',),'integer corpus');check_statuses(rows,{'pass'},role)
            require(report['relative_tolerance']==2e-12 and report['counts']=={'pass':166},'Integer policy/count changed')
            for r in rows:
                require(float(r['max_scaled_error'])<=2e-12 and r['examples']==[],'Integer numerical failure')
                require(r['native_parameter_checks'] and all(c['expected']==c['loaded'] for c in r['native_parameter_checks']),'Integer effective input mismatch')
                for suffix,key in (('.params','parameter_sha256'),('.m3p','scene_sha256')):
                    self.file(path.parent/(r['case']+suffix),r[key],True)
                stdin_path = self.file(path.parent/(r['case']+'.input'),current=True)
                check_stdin_hash(stdin_path,r['inputs_sha256'])
                self.file(path.parent/(r['case']+'.cpp.json'),current=True);self.file(path.parent/(r['case']+'.original.json'),current=True)
            return len(rows)
        if role in ('selector287_images','finite_matrices'):
            return self.replay_children(path,report,prior,role=='finite_matrices')
        if role in ('selector31_images','volume_images','transmission_images','transmission_zero','transmission_controls','dof_images','live_controls'):
            n=self.images(path,report,prior,exact=role=='volume_images',state=role=='live_controls')
            if role=='dof_images':require(all(r.get('exact_depth') is True for r in report['results']),'DOF depth changed')
            return n
        if role in ('ao_lanes','ao_pano_lanes','ao_threshold','tiles'):
            self.binary(report,'cpp_sha256')
            n=self.images(path,report,prior,exact=role!='tiles',state=True)
            same_cases(report['repeats'],prior['repeats'],('id','kind'),'AO/tile repeats')
            require(len(report['repeats'])==(22 if role=='tiles' else 16),'Wrong repeat denominator')
            check_statuses(report['repeats'], {'PASS'}, 'AO/tile repeat')
            plain = {r['id']: r for r in report['results']}
            for row in report['repeats']:
                require(row.get('exact_rgb') is True, 'Repeat/trace RGB mismatch')
                check_state(row)
                self.output(row, path)
                control = plain[row['id']]
                require(rgb(self.arg(row['command'], '--output')) ==
                        rgb(self.arg(control['command'], '--output')), 'Repeat/trace pixels changed')
                require(self.path(self.arg(row['command'], '--silight5-dump')).read_bytes() ==
                        self.path(self.arg(control['command'], '--silight5-dump')).read_bytes(),
                        'Repeat/trace packed state changed')
            require(report['repeat_counts'] == {'PASS': len(report['repeats'])}, 'Repeat aggregate mismatch')
            return n
        if role.startswith('diagnostics_'):
            return self.diagnostics(path,report,prior,'renderer' if role.endswith('cpp') else 'guarded_pascal')
        if role in ('hang_app','guarded_pascal_finite'):
            return self.guarded(path,report,prior,role=='hang_app')
        if role=='guarded_cpp_finite':
            self.binary(report,'cpp_sha256');require(report.get('finished') is True,'Finite report unfinished')
            rows=report['results'];same_cases(rows,prior['results'],('case',),'guarded finite corpus');check_statuses(rows,{'pass'},role)
            for r in rows:
                require(r.get('exact_png') is True and r.get('exact_rgb') is True and r.get('pascal_original_status')=='pass','Guarded finite mismatch')
                a=self.file(r['new_image'],r['new_image_sha256']);b=self.file(r['v20_image'],r['v20_image_sha256'])
                require(digest(a)==digest(b),'Retained finite PNG changed')
            self.historical.append({'role':role,'report':str(path),'qualification':'Same executable SHA, retained execution; not a fresh replay'})
            return len(rows)
        if role=='manowar_caller':
            require(report.get('finished') is True and report.get('profile')=='fpc-i386-sse2-single-slot-doHybridPas-v1','Wrong caller profile')
            rows=report['results'];check_statuses(rows,{'pass'},role)
            indexed=index_rows(rows,('case',),role)
            require(set(indexed)=={('challenge_ManowarBulb_'+s,) for s in MANOWAR_SUFFIXES},'Wrong Manowar cases')
            require(sum(r['rows'] for r in rows)==9360 and sum(r['exact_rows'] for r in rows)==9360,'Manowar rows not exact')
            _, layouts = self.read(self.policy['manowar_layouts'])
            _, corpus = self.read(self.policy['manowar_manifest'])
            layout_cases = {r['case'] for r in layouts['results']}
            expected = [{'case': c['id'], 'scene_sha256': c['sha256']} for c in corpus['cases']
                        if c['target_formulas'] == ['ManowarBulb']]
            same_cases(rows, expected, ('case',), 'Manowar input corpus')
            for r in rows:
                require(r['failed_lanes']==0 and r['examples']==[] and
                        r['loaded_prerequisites']=='verified by production GetMCTparasFromHeader', 'Manowar failure')
                transfer = r['independent_loader_transfer']
                if r['case'] in layout_cases:
                    require(isinstance(transfer, dict) and transfer.get('status')=='pass', 'Manowar independent layout transfer failed')
                else:
                    require(transfer is None, 'Unexpected independent layout claim')
                self.command(r['commands']['cpp_plain'],'orbit_probe');self.command(r['commands']['cpp_trace'],'orbit_probe')
            require(report.get('artifact_sha256') and report.get('identities'),'Missing caller artifacts')
            return 13
        if role=='legacy_loader':
            rows=report['results'];same_cases(rows,prior['results'],('id',),role)
            require(report['camera_absolute_tolerance']==2e-12 and report['original_undefined_case_count']==25,'Legacy policy changed')
            check_statuses(rows,{'EXACT','QUALIFIED_WITH_LIMITATIONS'},role)
            require(all(r['unexplained_offsets']==[] and all(c['absolute_error']<=2e-12 for c in r['camera_differences']) for r in rows),'Unexplained legacy fields')
            raw=[]
            for item in report['inputs']:
                p,d=self.read(item,True);self.binary(d,'cpp_sha256','scene_loader_probe');require(d.get('finished') is True,'Legacy unfinished')
                for r in d['results']:
                    require(r['cpp_exit']==0 and r['raw_status'] in ('EXACT','DIFFERENT') and r['zlib_extension_status']=='PASS','Legacy raw/extension failure')
                    self.file(r['cpp_dump'],r['cpp_dump_sha256'],True);self.file(r['reference'],r['reference_sha256']);self.file(r['scene'],r['scene_sha256'])
                    raw.append(r)
            same_cases(raw,rows,('id',),'Legacy raw/classified rows')
            from qualify_legacy_loader import classify
            for r in raw:
                classified=classify(self.path(r['reference']).read_bytes(),self.path(r['cpp_dump']).read_bytes(),next(v['source_mand_id'] for v in rows if v['id']==r['id']))
                expected=next(v for v in rows if v['id']==r['id'])
                require(all(expected[k]==v for k,v in classified.items()),'Legacy classification differs from actual records')
            require(report['counts']==dict(Counter(r['status'] for r in rows)),'Legacy aggregate mismatch')
            return len(rows)
        if role=='file_readback':
            self.binary(report,'cpp_sha256','scene_loader_probe');require(report.get('finished') is True,'Readback unfinished')
            rows=report['results'];same_cases(rows,prior['results'],('id',),role)
            require(report['counts']=={'SELECTOR_MATCH':12,'MATCHED_ORIGINAL_REJECTION':3},'Readback status denominator')
            for r in rows:
                log=self.file(path.parent/(r['id']+'.cpp.log'),r['diagnostic_sha256'],True)
                target=path.parent/(r['id']+'.cpp.bin')
                if r['status']=='MATCHED_ORIGINAL_REJECTION':
                    require(r['cpp_exit']==1 and r['original_exit']!=0 and r.get('numerical_parity_pass') is False and not target.exists() and 'Invalid file camera optic' in log.read_text(),'Incorrect readback rejection')
                else:
                    require(r['status']=='SELECTOR_MATCH' and r['cpp_exit']==0 and r['expected']==r['actual'],'Readback selector mismatch')
                    data=self.file(target,r['cpp_dump_sha256'],True).read_bytes()
                    actual={k:int.from_bytes(data[int(k[:-4]):int(k[:-4])+2],'little') if k.endswith('_u16') else data[int(k)] for k in r['expected']}
                    require(actual==r['expected'],'Readback bytes do not contain claimed selector')
            return len(rows)
        if role=='m3i_codec':
            check_candidate(report['cpp']['exe_sha256'],self.identities['m3i_probe']['sha256'],'M3I probe')
            require(report['cpp'].get('compile_parser',False) is False,'M3I must link frozen production libraries')
            rows=report['cases'];same_cases(rows,prior['cases'],('id',),role)
            require(report['counts']=={'EXACT':24,'READ_STATUS_MATCH':6},'Wrong M3I status denominator')
            for r in rows:
                if r['status']=='EXACT':
                    require(r['different_records']==0 and r['field_counts']=={} and r['examples']==[] and r['image_status']=='EXACT','M3I hidden state/image mismatch')
                    a=self.file(path.parent/(r['id']+'.cpp.silight'),r['cpp_silight_sha256'],True)
                    b=self.file(path.parent/(r['id']+'.pascal.silight'),r['pascal_silight_sha256'],True)
                    require(a.read_bytes()==b.read_bytes(),'M3I state differs')
                    if r['cpp_has_image']:
                        a=self.file(path.parent/(r['id']+'.cpp.image'),current=True);b=self.file(path.parent/(r['id']+'.pascal.image'),current=True)
                        require(a.read_bytes()==b.read_bytes(),'M3I image differs')
                else:
                    expected=next(v for v in prior['cases'] if v['id']==r['id'])
                    require(r['status']=='READ_STATUS_MATCH' and r['cpp_exit']==expected['cpp_exit'] and r['pascal_exit']==expected['pascal_exit'],'M3I read failure mismatch')
                self.file(path.parent/(r['id']+'.m3i'),r['scene_sha256'],True)
            _, linkage = self.read(self.manifest['m3i_linkage'],True)
            require(linkage['exit_code'] == 0 and linkage['build_exit_code'] == 0 and
                    linkage['no_parser_recompile'] is True and linkage['no_main_or_core_recompile'] is True and
                    linkage['case_set_matches_original_v3'] is True and linkage['counts'] == report['counts'],
                    'M3I frozen linkage/qualification failed')
            check_candidate(linkage['cpp']['exe_sha256'], self.identities['m3i_probe']['sha256'], 'M3I linkage')
            require(len(linkage['libraries']) == 3 and linkage['artifact_sha256'], 'Missing M3I library/artifact identities')
            for library in linkage['libraries']:
                keys = ('before_sha256', 'frozen_before_sha256', 'after_sha256',
                        'frozen_after_sha256', 'after_qualification_sha256')
                require(library['exact_frozen_before_after'] is True and
                        len({library[k] for k in keys}) == 1, 'M3I linked library changed')
                self.file(library['frozen_path'], library['after_qualification_sha256'], True)
                require(self.path(library['link_path']) == self.path(library['frozen_path']), 'M3I did not link frozen library')
            self.linked_hashes(linkage, path.parent)
            return len(rows)
        if role=='input_validation':
            self.binary(report,'cpp_sha256');require(report.get('finished') is True and report.get('success') is True,'Input validation incomplete')
            rows=report['results'];same_cases(rows,prior['results'],('case','threads'),role)
            same_cases(report['cases'], prior['cases'], ('id',), 'input-validation inputs')
            cases={r['id']:r for r in report['cases']}
            old_cases={r['id']:r for r in prior['cases']}
            for case in cases.values():
                self.file(case['scene'], case['sha256'])
                require(case['expected_diagnostic'] == old_cases[case['id']]['expected_diagnostic'], 'Changed expected CLI diagnostic')
            for r in rows:
                self.command(r['command']);p=self.path(self.arg(r['command'],'-o'))
                stem=p.stem
                logs=[]
                for stream in ('stdout','stderr'):
                    log=self.file(path.parent/(stem+'.'+stream+'.txt'),r[stream+'_sha256'],True);logs.append(log.read_text(encoding='utf-8',errors='replace'))
                expected=cases[r['case']]['expected_diagnostic']
                if expected:
                    require(r['status']=='rejected' and r.get('contract_verified') is True and r.get('parity_status')=='unverified' and r['returncode']==3 and not p.exists() and expected in '\n'.join(logs),'Incorrect CLI rejection')
                else:
                    require(r['status']=='rendered' and r['returncode']==0,'CLI finite control failed');self.file(p,r['output_sha256'],True)
            require(report['api']['status']=='not_run','API evidence belongs to separate current CTest gate')
            return len(rows)
        raise ValueError('No validator for role '+role)

    def run(self):
        for role,item in self.manifest['reports'].items():
            reuse=role=='guarded_cpp_finite'
            path,report=self.read(item,not reuse)
            prior={}
            if role in self.policy['reports']:
                baseline=self.policy['reports'][role];_,prior=self.read(baseline)
            measured=self.validate(role,path,report,prior)
            require(measured==(13 if role=='manowar_caller' else self.policy['reports'][role]['expected_rows']),role+': wrong denominator')
            self.linked_hashes(report, path.parent)
            self.reports[role]={'path':str(path),'sha256':self.files[str(path)],'validated_rows':measured,'status':'pass','fresh_execution':not reuse}
        for item in self.manifest.get('historical_support',[]):
            p=self.file(item['path'],item['sha256']);self.historical.append({'path':str(p),'sha256':self.files[str(p)],'qualification':'Historical support only'})
        # Recheck all consumed evidence at the end; the output is a seal, not a
        # claim based on files that changed while they were being inspected.
        for p,expected in self.files.items():check_hash(p,expected)
        return {'schema_version':1,'scope':'Finite ordinary supplemental gates only; MC/JIT/GPU excluded',
                'status':'pass','full_pascal_parity_complete':False,'reports':self.reports,
                'identities':self.identities,'historical_support':self.historical,
                'artifacts':self.files,'tool_sha256':digest(__file__),
                'limitations':['Original undefined/rejected inputs remain unverified, not numerical PASS.',
                               'No primary-corpus gate or overall completion claim is implied.']}


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--manifest',type=Path,required=True)
    parser.add_argument('--output',type=Path,required=True)
    args=parser.parse_args()
    require(not args.output.exists(),'Refusing to overwrite a prior seal')
    report=Audit(json.loads(args.manifest.read_bytes())).run()
    report['manifest']={'path':str(args.manifest.resolve()),'sha256':digest(args.manifest)}
    args.output.write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8')
    print(json.dumps({'status':report['status'],'reports':len(report['reports']),'artifacts':len(report['artifacts'])}))


if __name__=='__main__':main()
