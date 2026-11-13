"""Consolidate six finite ordinary coupling evidence families without rendering.

This is an evidence index, not a Stage A2 certificate. Retained results belong to
their recorded binaries; a newer candidate does not inherit their acceptance.
"""
from __future__ import annotations

import argparse
from collections import Counter
import hashlib
import json
from pathlib import Path
import re
import xml.etree.ElementTree as ET

ROOT = Path(__file__).resolve().parents[2]
BASE = ROOT / 'comparison/results/stage-a-20261018'
RUNS = ROOT / 'comparison/runs/stage-a-20261018'


class Evidence:
    def __init__(self):
        self.inputs = {}

    def path(self, path):
        path = path.resolve()
        try:
            return path.relative_to(ROOT).as_posix()
        except ValueError:
            return str(path)

    def raw(self, path):
        path = path.resolve()
        value = path.read_bytes()
        self.inputs[self.path(path)] = hashlib.sha256(value).hexdigest()
        return value

    def read(self, path):
        return json.loads(self.raw(path))

    def link(self, path):
        self.raw(path)
        return {'path': self.path(path), 'sha256': self.inputs[self.path(path)]}

    def anchor(self, path, text):
        lines = self.raw(ROOT/path).decode('utf-8', errors='replace').splitlines()
        matches = [n+1 for n, line in enumerate(lines) if text in line]
        if not matches:
            raise ValueError(f'Missing source anchor {path}: {text}')
        return dict(path=path, lines=matches, anchor=text)

    def summary(self, path, expected, latest_hash):
        data = self.read(path)
        rows = data['results']
        if len(rows) != expected or data.get('finished', True) is not True:
            raise ValueError(f'Incomplete/changed finite matrix: {path}')
        def counts(items):
            return dict(Counter(str(row['status']).lower() for row in items))
        binary = data.get('binary_sha256', data.get('cpp_sha256'))
        if not binary:
            raise ValueError(f'Missing binary identity: {path}')
        measured_state = [row for row in rows if 'state_status' in row]
        result = dict(report=self.link(path), cases=len(rows), counts=counts(rows),
            exact_rgb=sum(row.get('exact_rgb', False) for row in rows),
            binary_sha256=binary, matches_selected_candidate=binary == latest_hash,
            thresholds=data.get('thresholds'),
            state_cases=len(measured_state), state_counts=counts([
                dict(status=row['state_status']) for row in measured_state]),
            exact_state=sum(row.get('exact_state', False) for row in measured_state),
            exact_depth_cases=sum('exact_depth' in row for row in rows),
            exact_depth=sum(row.get('exact_depth', False) for row in rows),
            failed_cases=[row.get('id', row.get('scene')) for row in rows
                          if str(row['status']).lower() != 'pass'],
            state_failed_cases=[row.get('id', row.get('scene')) for row in measured_state
                                if str(row['state_status']).lower() != 'pass'])
        if 'repeats' in data:
            result['repeat_trace'] = dict(cases=len(data['repeats']),
                counts=counts(data['repeats']),
                exact_rgb=sum(row.get('exact_rgb', False) for row in data['repeats']),
                exact_state=sum(row.get('exact_state', False) for row in data['repeats']))
        if any('depth_regions' in row for row in rows):
            region_counts = Counter()
            for row in rows:
                region_counts.update(row.get('depth_regions', {}).get('counts', {}))
            result['depth_region_observations'] = dict(counts=dict(region_counts),
                denominator_note='Sum over images, including repeated source pixels; not unique pixels or instrumented kernel branch counts.')
        if any('original_witness' in row for row in rows):
            result['tir_first_body_witnesses'] = [
                dict(case=row['id'], original=row['original_witness']['first_body_tir_count'],
                     cpp=row['witness']['first_body_tir_count']) for row in rows]
        return result


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--candidate', type=Path, default=RUNS/'candidate-v17-a2')
    ap.add_argument('--renderer-log', type=Path)
    ap.add_argument('--ordinary-test-xml', type=Path,
                    help='Completed CTest JUnit report for the selected candidate; records math3d and zbuffer-normal acceptance separately.')
    ap.add_argument('--live-controls-summary', type=Path,
                    help='Independent live normals on/off and hard/soft-shadow control replay (5 cases)')
    ap.add_argument('--source-audit', type=Path, default=BASE/'ordinary-source-audit-v2.json')
    ap.add_argument('--transmission-summary', type=Path,
                    default=BASE/'transmission-couplings-v1/candidate-v17-a2/summary.json')
    ap.add_argument('--zero-power-summary', type=Path,
                    default=BASE/'transmission-zero-power-v1/candidate-v17-a2/summary.json')
    ap.add_argument('--transmission-controls-summary', type=Path,
                    default=BASE/'transmission-controls-v1/candidate-v17-a2/summary.json')
    ap.add_argument('--dof-summary', type=Path,
                    default=BASE/'dof-couplings-v1/candidate-v17-a2/summary.json')
    ap.add_argument('--volume-evidence', type=Path, default=BASE/'volume-evidence-v1.json')
    ap.add_argument('--ao-summary', type=Path, action='append',
                    help='Repeat for ordinary and panorama 8-case AO matrices; defaults retain v17 evidence.')
    ap.add_argument('--ao-threshold-summary', type=Path,
                    help='Independent 8-case raw0/1 threshold/SIMD boundary matrix')
    ap.add_argument('--direct-contract', type=Path,
                    help='Candidate-qualified direct/file selector contract; keeps source equivalence separate from execution')
    ap.add_argument('--file-readback-summary', type=Path,
                    help='Selected-candidate replay of original GUI file-selector readbacks and rejections')
    ap.add_argument('--output', type=Path, default=BASE/'ordinary-couplings-v1.json')
    args = ap.parse_args()
    e = Evidence()
    binary_hash = hashlib.sha256(e.raw(args.candidate/'mb3d_render.exe')).hexdigest()
    test_hash = hashlib.sha256(e.raw(args.candidate/'mb3d_tests_renderer.exe')).hexdigest()
    build = e.read(args.candidate/'build-identity.json')
    for group in ('sources','binaries'):
        build[group]=[dict(Path=str((ROOT/row.get('Path',row.get('path'))).resolve()),
            Hash=row.get('Hash',row.get('sha256'))) for row in build[group]]
    build_hashes = {row['Hash'].lower() for row in build['binaries']}
    if not {binary_hash, test_hash}.issubset(build_hashes):
        raise ValueError('Frozen build identity does not match selected executables')
    source_drift = []
    for row in build['sources']:
        path = Path(row['Path'])
        if path.exists() and hashlib.sha256(e.raw(path)).hexdigest() != row['Hash'].lower():
            source_drift.append(e.path(path))
    log_path = args.renderer_log or args.candidate/'renderer-tests.log'
    log = e.raw(log_path).decode('utf-8', errors='replace')
    totals = re.findall(r'Results: (\d+) passed, (\d+) failed', log)
    if len(totals) != 1:
        raise ValueError('Require a completed renderer test log')
    markers = dict((name, status.lower()) for status, name in
                   re.findall(r'\[ (PASS|FAIL) \] (\w+)', log))
    def named(*names):
        return [dict(name=name, status=markers.get(name, 'not_observed'),
                     log=e.path(log_path), binary_sha256=test_hash) for name in names]
    ordinary_tests = []
    if args.ordinary_test_xml:
        xml = ET.fromstring(e.raw(args.ordinary_test_xml))
        for name, executable in [('math3d_tests', 'mb3d_tests.exe'),
                                 ('zbuffer_normals_tests', 'mb3d_tests_zbuffer_normals.exe')]:
            matches = [row for row in xml.iter('testcase') if row.get('name') == name]
            if len(matches) != 1:
                raise ValueError('Missing/ambiguous ordinary test: '+name)
            row = matches[0]
            exe_hash = hashlib.sha256(e.raw(args.candidate/executable)).hexdigest()
            if exe_hash not in build_hashes:
                raise ValueError('Ordinary test binary does not match frozen build: '+name)
            output = row.findtext('system-out', '')
            ordinary_tests.append(dict(name=name,
                status='pass' if row.get('status')=='run' and
                    all(row.find(tag) is None for tag in ('failure','error','skipped')) else 'not_pass',
                report=e.link(args.ordinary_test_xml), binary_sha256=exe_hash,
                output=output.strip(),
                interpretation='Suite acceptance on the selected frozen build; assertion totals are not attributed to one packing helper.'))
    source_audit = e.read(args.source_audit)
    def axes(*families):
        return [dict(id=row['id'], classes=row['classes'], pascal=row['pascal'],
                     cpp=row['cpp'], equivalence=row['equivalence'])
                for row in source_audit['axes'] if row['family'] in families]
    tx = e.summary(args.transmission_summary, 8, binary_hash)
    tx_zero = e.summary(args.zero_power_summary, 3, binary_hash)
    tx_controls = e.summary(args.transmission_controls_summary, 2, binary_hash)
    dof = e.summary(args.dof_summary, 13, binary_hash)
    volume = e.read(args.volume_evidence)
    live = e.summary(args.live_controls_summary, 5, binary_hash) if args.live_controls_summary else None
    live_ok = (live and live['matches_selected_candidate'] and not live['failed_cases']
               and live['state_cases']==5 and live['exact_state']==5 and not live['state_failed_cases'])
    ao_paths = args.ao_summary or [RUNS/'ao-wavelet-lanes-candidate-v17-a2/summary.json',
                                  RUNS/'ao-wavelet-pano-lanes-candidate-v17-a2/summary.json']
    ao = [e.summary(path, 8, binary_hash) for path in ao_paths]
    if args.ao_threshold_summary:
        ao.append(e.summary(args.ao_threshold_summary, 8, binary_hash))
    ao_remaining = []
    if any(row['failed_cases'] or row['state_failed_cases'] for row in ao):
        ao_remaining.append('Resolve and replay every recorded RGB or packed-state failure.')
    if not args.ao_threshold_summary:
        ao_remaining.append('AO0/1 SSE quartet and odd-width boundary witnesses are not selected in this report; add their final frozen-candidate summary.')
    if any(not row['matches_selected_candidate'] for row in ao):
        ao_remaining.append('Some AO evidence belongs to another binary; final candidate acceptance needs its own replay.')
    historical = []
    for filename, expected in [('depth-codec-v1.json', 8769), ('packed-normals-v1.json', 15704)]:
        path = ROOT/'comparison/results/external-complete-20261013'/filename
        data = e.read(path)
        if data['rows'] != expected:
            raise ValueError('Changed historical finite denominator')
        historical.append(dict(report=e.link(path), rows=data['rows'],
            failed_rows=data['failed_rows'], exact=data['exact'], binaries=data['binaries'],
            applicability='Historical standalone numeric probe; not a rerun on the selected candidate.'))

    reserved_contract = None
    if args.direct_contract:
        contract = e.read(args.direct_contract)
        verified = contract.get('candidate', {}).get('binaries_verified', [])
        for row in verified:
            if hashlib.sha256(e.raw(ROOT/row['path'])).hexdigest() != row['sha256']:
                raise ValueError('Direct contract binary changed: '+row['path'])
        if binary_hash not in {row['sha256'] for row in verified}:
            raise ValueError('Direct contract belongs to another candidate')
        if len(contract['classes']) != 9 or not contract.get('contract_tests_all_passed'):
            raise ValueError('Incomplete direct selector contract execution')
        for group in ('evidence', 'test_sources', 'fixture_sources', 'execution_logs'):
            for row in contract.get(group, []):
                if hashlib.sha256(e.raw(ROOT/row['path'])).hexdigest() != row['sha256']:
                    raise ValueError('Direct contract input changed: '+row['path'])
        reserved_contract = dict(report=e.link(args.direct_contract),
            classes=contract['classes'], boundaries=contract['boundaries'],
            tests=contract['contract_test_execution'], limitations=contract['limitations'],
            matches_selected_candidate=True)
    file_readback = None
    if args.file_readback_summary:
        data = e.read(args.file_readback_summary)
        if data['cpp_sha256'] not in build_hashes:
            raise ValueError('File readback loader is not in selected frozen build')
        if hashlib.sha256(e.raw(Path(data['cpp']))).hexdigest() != data['cpp_sha256']:
            raise ValueError('File readback loader changed')
        counts = dict(Counter(row['status'] for row in data['results']))
        if counts != {'SELECTOR_MATCH': 12, 'MATCHED_ORIGINAL_REJECTION': 3}:
            raise ValueError('Incomplete/failed file selector readback matrix')
        file_readback = dict(report=e.link(args.file_readback_summary),
            loader_sha256=data['cpp_sha256'], matches_selected_candidate=True,
            cases=len(data['results']), counts=counts,
            interpretation='Selector matches and original GUI rejection matches are separate outcomes; neither certifies all bytes or direct numerical parity.')

    couplings = [
        dict(id='transmission-medium-depth-absorption', axes=axes('reflection'),
             evidence=[tx, tx_zero, tx_controls], tests=named('test_transmission_fpc_predicate',
                 'test_transmission_power_boundaries', 'test_reflection_primary_trace_matches_pascal_hotspot'),
             accounted_classes=['bounce header 0/1/3 with retained actual recursion depth',
                 'non-TIR/TIR first-body transition', 'absorption 0/.8/4',
                 'cut-plane plus absorption', 'zero red/RGB diffuse channels',
                 'active-file reflection amount/count/absorption/refraction/scatter control clamps'],
             equivalence='One secondary-ray implementation handles recursive depths; the first-body TIR transition is checked separately from later Fresnel reflection. No equivalence of arbitrary depths/materials is claimed.',
             limitations=['Legacy trace establishes control-path witnesses only; it is not schema-certified numerical parity.',
                 'FPC i386 LongBool not/XOR fixture certifies that caller profile; it does not define every compiler Boolean ABI.',
                 'Passing eight earlier images does not cover the independent zero-channel loader boundary.'],
             remaining=(['Zero-channel/zero-absorption failures require new-candidate replay after the confirmed GUI loader clamp fix.'] if tx_zero['failed_cases'] else []) +
                 (['Active-file low/high SR control failures require replay after the common five-control GUI readback normalization fix.'] if tx_controls['failed_cases'] else [])),
        dict(id='dof-driver-depth-order', axes=axes('dof'), evidence=[dof],
             accounted_classes=['disabled control', 'sorted and plain pass counts 1/2/3',
                 'foreground/focus/background/no-hit depth regions', 'radius clipping and image-edge candidates'],
             equivalence='Dispatch classes share their named driver, while pass counts and sorted/plain accumulation order remain separate. Depth-region counts are source-derived observations, not exact dynamic branch counters.',
             limitations=['Finite aperture/focus/clip values do not prove every real-valued threshold.',
                 'Raw pass selector3 is rejected by the Pascal radio caller and is not an alias of selector2.'],
             remaining=[]),
        dict(id='volume-primary-secondary-face-rounding', axes=axes('volume'),
             evidence=[dict(report=e.link(args.volume_evidence), numerical=volume['numerical'],
                 images=volume['images'], matches_selected_candidate=volume['images']['binary_sha256']==binary_hash)],
             tests=named('test_volume_sampling_original', 'test_directional_volumetric_map_matches_pascal'),
             accounted_classes=['six signed cube faces', 'dominant-axis ties', 'nearest-even texel ties',
                 'Single input conversion', 'directional identity/mixed rotation and clamps',
                 'odd/even half-size', 'shared primary/secondary fog substep'],
             equivalence='Primary and secondary C++ consumers call the same fog substep and pure samplers; immutable Pascal callers use the corresponding same formulas. This proves shared routing, not all recursion/volume/material combinations.',
             limitations=volume['limitations'], remaining=[]),
        dict(id='ao-wavelet-repeat-logical-lane', axes=axes('ao','workers-rng'), evidence=ao,
             accounted_classes=['raw4/5', 'repeat1/2', 'logical threads1/2',
                 'ordinary/panorama', 'plain/repeat/trace within each logical schedule'] +
                 (['raw0/1 threshold, SIMD quartet and odd-width boundary witnesses'] if args.ao_threshold_summary else []),
             equivalence='Physical execution of a fixed logical schedule must preserve each lane RNG lifetime; different logical thread counts are separate reference profiles.',
             limitations=['RGB PASS is independent of exact packed AmbShadow acceptance.',
                 'Raw4/5 images alone do not establish AO0/1 quartet and odd-width boundary behavior.',
                 'Current source changes do not retroactively alter retained candidate results.'],
             remaining=ao_remaining),
        dict(id='packed-output-depth-normal-tags', axes=axes('packed-output'),
             evidence=historical, ordinary_tests=ordinary_tests,
             live_postprocess_evidence=live,
             tests=named('test_depth_codec_original_assembly',
                 'test_unordered_ifs_minimum_and_inside_flags', 'test_normalmap_export',
                 'test_zbuffer_export'),
             source=[e.anchor('tests/test_math3d.cpp','static void test_pascal_packing_regressions()'),
                 e.anchor('src/core/math3d.cpp','void MakeWNormalsFromDVec('),
                 e.anchor('src/core/ray_marcher.cpp','void CalcZposAndRough(')],
             accounted_classes=['depth/roughness combined record packing', 'normal coordinate signs and axes',
                 'half-integer/saturation/nonfinite packing boundaries', 'inside high-bit flags'],
             equivalence='Packing helpers are shared across their callers; independent packed-field and image witnesses remain necessary for producer state. Historical probe rows certify their own binaries only.',
             limitations=['15,704 normal vectors and 8,769 depth rows are finite historical samples.',
                 'Historical numerical probes and selected-candidate suite acceptance have separate denominators.'],
             remaining=([] if len(ordinary_tests)==2 and all(row['status']=='pass' for row in ordinary_tests)
                        else ['Include math3d packing and zbuffer-normal tests in final candidate acceptance.']) +
                 ([] if live_ok else ['Applicable live post-depth-normal image/state witnesses remain a separate integration gate.'])),
        dict(id='reserved-selector-caller-domain',
             source=[e.anchor('mb3d/LightAdjust.pas','function TLightAdjustForm.VisFuncToIndex'),
                 e.anchor('mb3d/LightAdjust.pas','const Ctab: array[0..3] of Integer = (0, 3, 1, 2);'),
                 e.anchor('mb3d/LightAdjust.lfm','object ComboBox4: TComboBox'),
                 e.anchor('mb3d/LightAdjust.lfm','object ComboBox5: TComboBox'),
                 e.anchor('mb3d/Mand.pas','else bVolLightNr := Min(6, Max(1, StrToIntTrim(Edit16.Text)))'),
                 e.anchor('src/core/header_trafos.cpp','int volN = std::min(5, std::max(0, (Header.bVolLightNr & 7) - 1));')],
             accounted_classes=[dict(raw_visible_shape=10, gui_index=7),
                 dict(raw_visible_shape=12, gui_index=5),dict(raw_visible_shape=14, gui_index=6),
                 dict(raw_volume_light=7, gui_normalized_light=6)],
             equivalence='Visible-light GUI ComboBox4/5 lists only indices0..4. Raw shapes10/12/14 map to out-of-list indices7/5/6; they are not certified numerical aliases of shape0. The file GUI volume caller clamps raw7 to light6.',
             limitations=['Source domain accounting does not prove arbitrary invalid-header roundtrip or direct-header parity.',
                 'No image test is credited merely because a reserved raw byte was written.'],
             remaining=['Direct-header reserved values require an explicit contract and independent witness if they are included in the supported input domain.'])]
    if reserved_contract:
        couplings[-1] = dict(id='reserved-selector-caller-domain',
            direct_file_contract=reserved_contract, file_readback_evidence=file_readback,
            accounted_classes=[row['id'] for row in reserved_contract['classes']],
            equivalence='Actual original GUI readback and direct kernel callers are distinct contracts. Visible shapes10/12/14 survive file readback; local kernel equivalence does not imply identical soft-shadow behavior. Hybrid mode3 is preserved for direct callers and normalized only by the file loader.',
            limitations=reserved_contract['limitations'] + [
                'Source-qualified direct inside/camera distinctions are not numerical image certificates.',
                'Four-pass DOF witnesses establish zero-radius identity and allocation boundaries only.'],
            remaining=[] if file_readback else [
                'Select the frozen-candidate file readback/rejection replay for the loader boundary.'])
    report = dict(schema_version=1, scope='Six ordinary finite coupling evidence families; Monte Carlo/JIT excluded',
        stage_a2_closed=False, completion_percentage=None,
        selected_candidate=dict(path=e.path(args.candidate), render_sha256=binary_hash,
            renderer_tests_sha256=test_hash, renderer_test_log=e.link(log_path),
            renderer_test_counts=dict(passed=int(totals[0][0]), failed=int(totals[0][1])),
            source_changed_since_build=source_drift),
        source_inventory=e.link(args.source_audit), couplings=couplings,
        interpretation=['Six entries are an explicitly requested coupling index, not an exhaustive count of all Stage A obligations.',
            'No overall percentage is derived by adding scenes, numeric rows, source classes or assertions.',
            'Every image report retains its binary identity; matches_selected_candidate=false means historical evidence.',
            'Static equivalence is limited to the stated shared routing or caller domain.',
            'Final343, active external suite and all applicable tests on one frozen candidate remain separate gates.'],
        input_sha256=e.inputs)
    e.raw(Path(__file__))
    for path, expected in list(e.inputs.items()):
        full = Path(path) if Path(path).is_absolute() else ROOT/path
        if hashlib.sha256(full.read_bytes()).hexdigest() != expected:
            raise ValueError('Input changed during audit: '+path)
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(report, indent=2, ensure_ascii=False)+'\n', encoding='utf-8')
    print(json.dumps(dict(couplings=len(couplings), stage_a2_closed=False,
        renderer=report['selected_candidate']['renderer_test_counts'],
        transmission=tx['counts'], zero_power=tx_zero['counts'],
        transmission_controls=tx_controls['counts'], dof=dof['counts'],
        ao_state=[row['state_counts'] for row in ao])))


if __name__ == '__main__':
    main()
