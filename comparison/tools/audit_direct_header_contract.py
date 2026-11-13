"""Collect source-linked reserved-selector contracts; never certify renders from raw bytes."""
import argparse
import hashlib
import json
from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[2]
BASE = Path('comparison/results/stage-a-20261018/direct-header-contract-v1')


def linked(path, anchor=None):
    p = ROOT / path
    value = {'path': str(path).replace('\\', '/'),
             'sha256': hashlib.sha256(p.read_bytes()).hexdigest()}
    if anchor:
        lines = p.read_text(encoding='latin1').splitlines()
        matches = [i + 1 for i, line in enumerate(lines) if anchor in line]
        if not matches:
            raise ValueError(f'Missing source anchor {path}: {anchor}')
        value.update(anchor=anchor, lines=matches)
    return value


def sources(*entries):
    return [linked(path, anchor) for path, anchor in entries]


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--output', type=Path, required=True)
    ap.add_argument('--test-log', type=Path, action='append', default=[])
    ap.add_argument('--candidate', type=Path,
                    help='Frozen candidate directory with build-identity.json; verify its render and contract-test binaries.')
    ap.add_argument('--ordinary-summary', type=Path,
                    help='Extract the three retained globalIFS controls from the final343 run; never re-render.')
    args = ap.parse_args()
    classes = [
        dict(id='hybrid-low2-mode3', field='addon.bOptions1',
             file_contract='Only low value 3 becomes 0; upper six bits remain unchanged.',
             direct_contract='Low value 3 retains original split layout, end1=start2-1. It is not mode0.',
             caveat='The full byte is also compared to 2 in IsSingleF/DE classification: upper bits are not globally equivalent.',
             evidence=['file-readback-repeat/summary.json', 'hybrid-layout/identity.json'],
             test='tests/test_formula_iteration_guard.cpp:testDirectHybridLayout; 72 independent integer-layout rows',
             source=sources(('mb3d/CustomFormulas.pas', 'if (pHCA.bOptions1 and 3) = 0'),
                            ('mb3d/HeaderTrafos.pas', 'CheckHybridOptions(PCFA);'),
                            ('src/core/header_trafos.cpp', 'if ((PCFA->bOptions1 & 3) == 0)'),
                            ('src/io/m3p_parser.cpp', '(result.addon.bOptions1 & 3u) == 3u'))),
        dict(id='inside-low2-mode3', field='addon.bOptions2 bits1..2',
             file_contract='Raw masked value6 becomes0, retaining unrelated bits; original readback observed.',
             direct_contract='Raw6 enables inside and both flags, but uses the scene refraction index. Raw4 uses RI=1.',
             caveat='Raw6 is therefore not globally equivalent to raw4, despite equal primary flags.',
             evidence=['reserved-readback/summary.json'], test='Existing inside/transmission witnesses cover admitted GUI modes; direct reserved distinction is source-qualified.',
             source=sources(('mb3d/HeaderTrafos.pas', 'Result.bInsideRendering :='),
                            ('mb3d/CalcSR.pas', 'bOptions2 and 6) = 4'),
                            ('src/core/header_trafos.cpp', 'Result.bInsideRendering ='),
                            ('src/core/reflection.cpp', 'bOptions2 & 6) == 4'))),
        dict(id='camera-full-byte', field='header.bPlanarOptic',
             file_contract='Only full bytes0,1,2 are accepted. Original rejects3,6,255 during radio-control readback; C++ returns explicit error.',
             direct_contract='All bytes retain original mixed semantics: MCT=min(2,byte&3); panoramic FOV/setup requires exact byte2; paint consumes low2.',
             caveat='Do not alias3 to2 or6 to2, and do not apply file-only range validation to Render(header).',
             evidence=['reserved-readback/summary.json'], test='Parser positive0/1/2 and rejection controls; direct mixed contract is source-qualified.',
             source=sources(('mb3d/HeaderTrafos.pas', 'Result.MCTCameraOptic :='),
                            ('mb3d/HeaderTrafos.pas', 'if bPlanarOptic = 2 then Result.FOVXmul'),
                            ('src/core/header_trafos.cpp', 'Result.MCTCameraOptic ='),
                            ('src/io/m3p_parser.cpp', 'if (result.header.bPlanarOptic > 2)'))),
        dict(id='volume-low3-mode7', field='header.bVolLightNr',
             file_contract='Low7 becomes6; other bits retained. Original39(0x27) read back as38(0x26).',
             direct_contract='Low7 andlow6 select light5, enable volume and share map routing, for identical upper nibble and all other input.',
             caveat='The upper nibble controls map resolution/step scale and is not equivalent across values.',
             evidence=['reserved-readback/summary.json'], test='Parser low0..7 controls plus retained ordinary volume image/sampler evidence.',
             source=sources(('mb3d/HeaderTrafos.pas', 'n := Min(5, Max(0, (bVolLightNr and 7) - 1))'),
                            ('mb3d/maps/MB3DMaps.pas', 'Header.bVolLightNr and 7'),
                            ('src/core/header_trafos.cpp', 'int volN = std::min(5'),
                            ('src/io/m3p_parser.cpp', '(result.header.bVolLightNr & 7u) == 7u'))),
        dict(id='visible-light-shapes10-12-14', field='Light.Loption/LFunction decoded shape',
             file_contract='Original MakeHeader preserves all three raw encodings. GUI combo item range does not imply unreachable file inputs.',
             direct_contract='10,12,14 share the default shape kernel and ordinary attenuating compositing/sorting class for equal other inputs.',
             caveat='Not equivalent to0(disabled) or8(special compositing/radius). Positional soft shadows additionally split byshape&6:10 shares70/radius with2,12/14 share40/radius with4/6. No global three-shape alias is claimed. Pure kernel witnesses are not whole-render certification.',
             evidence=['reserved-readback/summary.json', 'visible-math/identity.json'],
             test='tests/test_renderer.cpp:test_visible_light_original_math;1152 original rows including432 reserved-shape rows',
             source=sources(('mb3d/PaintThread.pas', 'procedure CalcPosLightShape'),
                            ('src/core/colorizer.cpp', 'void CalcPosLightShape('),
                            ('mb3d/CalcHardShadow.pas', '(PLVals.iLightPos[iHSnr] and 6) = 2'),
                            ('src/core/shadow.cpp', '(lvals->iLightPos[iHSnr] & 6) == 2'))),
        dict(id='dof-direct-four-passes', field='header.bCalcDOFtype',
             file_contract='GUI pass radio accepts0..2; original rejection for3 is retained in earlier mode evidence.',
             direct_contract='Bit0 enables; bits1..2 select1..4 passes; bit3 selects plain; bits4..7 are unconsumed by DOF.',
             caveat='Mode2/3 use the same original default aperture branch; pass3 is an additional loop iteration with2^pass(plain) or3^pass(sorted). New four-pass test proves zero-radius identity and allocation boundaries, not arbitrary defocus radii.',
             evidence=[], test='tests/test_dof.cpp:192 zero-radius cases (48 new four-pass cases), plain/sorted,size/stride combinations',
             source=sources(('mb3d/PostProcessForm.pas', 'z := (Mand3DForm.MHeader.bCalcDOFtype shr 1) and 3'),
                            ('mb3d/DOF.pas', 'Ap := sDOFaperture /'),
                            ('src/core/dof.cpp', 'int passMode = (header->bCalcDOFtype >> 1) & 3;'))),
        dict(id='stereo-all-byte-values', field='header.bStereoMode',
             file_contract='File migration resets stereo to0, as in original.',
             direct_contract='Only1,3,4 have nonzero camera transforms.0,2,5..255 use the original default branch.',
             caveat='768 independent rows verify offset/camera setup, not all downstream image interactions.',
             evidence=['camera/identity.json'], test='tests/test_camera_tile_setup.cpp:all256bytes×3configurations, plus existing24 tile setup fixtures',
             source=sources(('mb3d/HeaderTrafos.pas', 'case Header.bStereoMode of'),
                            ('src/core/header_trafos.cpp', 'switch (Header->bStereoMode)'))),
        dict(id='decomb-selector-clamp', field='addon.bOptions3',
             file_contract='Existing DE-combination setup and validation retained.',
             direct_contract='FormulaType clamps values5..255 to6, but the exact5 original dIFS classification remains separately significant.',
             caveat='This is a local FormulaType equivalence, not permission to rewrite the stored selector.', evidence=[],
             test='Existing DE combination mix/validation and iteration-guard fixtures.',
             source=sources(('mb3d/HeaderTrafos.pas', 'Result.FormulaType := Min(6'),
                            ('mb3d/HeaderTrafos.pas', '(pHCA.bOptions3 = 5)'),
                            ('src/core/header_trafos.cpp', 'Result.FormulaType = std::min(6'),
                            ('src/core/header_trafos.cpp', '(pHCA->bOptions3 == 5)'))),
        dict(id='color-selector-clamp', field='header.byColor2Option',
             file_contract='No new migration required.',
             direct_contract='MCT ColorOption maps full bytes5..255 to5, as original.',
             caveat='Equivalence is for this consumed color selector; unrelated option bits/state are held fixed.', evidence=[],
             test='Existing ordinary color-mode scene matrix and source-equivalence class.',
             source=sources(('mb3d/HeaderTrafos.pas', 'Result.ColorOption := Min(5'),
                            ('src/core/header_trafos.cpp', 'Result.ColorOption  = std::min'))),
    ]
    evidence_paths = sorted({str(BASE / e) for c in classes for e in c['evidence']})
    report = dict(schema_version=1, scope='Remaining ordinary reserved direct-header selector audit; not a replacement for the75-axis ordinary inventory',
                  deferred=['Monte Carlo', 'JIT'], stage_a_closed=False,
                  boundaries=dict(public_C_API='File-only mb3d_render_file follows loader migrations/errors.',
                                  internal_CPP='Render(header,addon) and GetMCT retain raw original caller semantics; no blanket GUI-enum rejection.'),
                  status='Source/evidence classification complete for these9 reserved-selector obligations; execution is candidate-specific.',
                  classes=classes, class_count=len(classes),
                  evidence=[linked(p) for p in evidence_paths],
                  test_sources=[linked(p) for p in ('tests/test_formula_iteration_guard.cpp','tests/test_camera_tile_setup.cpp','tests/test_renderer.cpp','tests/test_dof.cpp')],
                  fixture_sources=[linked(p) for p in ('tests/fixtures/hybrid_layout.h','tests/fixtures/direct_header_camera_tile_setup.h','tests/fixtures/visible_light_math_direct.h')],
                  execution_logs=[linked(p.resolve().relative_to(ROOT)) for p in args.test_log],
                  limitations=['No finite fixture is a proof for every continuous parameter or hybrid combination.',
                               'Source-qualified local equivalence is distinguished from measured original numerical parity.',
                               'High-work loop bounds/cancellation belong to Stage B; this audit does not execute huge counts.',
                               'Undefined/nonterminating original cases and final frozen-candidate gates remain separately open.',
                               'Prior GUI-unreachable claims for visible shapes10/12/14 are superseded by actual MakeHeader readback.'],
                  source_inventory=linked('comparison/results/stage-a-20261018/ordinary-source-audit-v2.json'))
    if args.candidate:
        candidate = args.candidate.resolve()
        identity = candidate/'build-identity.json'
        built = json.loads(identity.read_bytes())
        wanted = {'mb3d_render.exe', 'mb3d_tests_formula_iteration_guard.exe',
                  'mb3d_tests_camera_tile_setup.exe', 'mb3d_tests_renderer.exe', 'mb3d_tests_dof.exe'}
        verified = []
        for binary in built['binaries']:
            path = Path(binary['path'])
            if path.name in wanted:
                current = linked(path)
                if current['sha256'] != binary['sha256']:
                    raise ValueError('Frozen candidate binary changed: '+str(path))
                verified.append(current)
        if {Path(r['path']).name for r in verified} != wanted:
            raise ValueError('Frozen candidate is missing contract-test identities')
        report['candidate'] = dict(build_identity=linked(identity.relative_to(ROOT)), binaries_verified=verified)
        statuses = {}
        for path in args.test_log:
            text = path.read_text(encoding='utf-8', errors='replace')
            for block in re.split(r'(?m)^\d+/\d+ Testing: ', text)[1:]:
                name = block.splitlines()[0].strip()
                if name in {'formula_iteration_guard_tests','camera_tile_setup_tests','renderer_tests','dof_tests'}:
                    statuses[name] = 'pass' if '\nTest Passed.\n' in block else 'not_passed'
        report['contract_test_execution'] = statuses
        report['contract_tests_all_passed'] = len(statuses) == 4 and set(statuses.values()) == {'pass'}
    reference = Path('comparison/runs/20261013T132245Z/scenes_phase3_ifs/20261013T133437Z/summary.json')
    original = json.loads((ROOT/reference).read_bytes())
    retained = [r for r in original['results'] if '_globalifs_' in r['scene']]
    if len(retained) != 3:
        raise ValueError('Expected exactly the three historical globalIFS controls')
    controls = []
    for row in retained:
        scene = Path('comparison/scenes_phase3_ifs')/(row['scene']+'.m3p')
        item = dict(id=row['scene'], scene=linked(scene),
                    original_image=linked(reference.parent/'pascal'/(row['scene']+'.png')),
                    historical_status=row['status'], final_status='pending_final343')
        if item['scene']['sha256'] != row['scene_sha256']:
            raise ValueError('Historical globalIFS scene changed: '+row['scene'])
        controls.append(item)
    report['retained_globalifs'] = dict(reference=linked(reference), cases=controls,
                                       execution='Reuse final343 result; no duplicate rendering.')
    if args.ordinary_summary:
        current = json.loads(args.ordinary_summary.read_bytes())
        if args.candidate:
            render = next(row for row in verified if Path(row['path']).name == 'mb3d_render.exe')
            if current['cpp_sha256'] != render['sha256']:
                raise ValueError('Ordinary replay does not belong to selected frozen renderer')
        lookup = {r['scene']: r for r in current['results']}
        report['retained_globalifs'].update(summary=linked(args.ordinary_summary.resolve().relative_to(ROOT)),
                                            cpp_sha256=current['cpp_sha256'], finished=current['finished'])
        for control in controls:
            r = lookup.get(control['id'])
            if r:
                if r['scene_sha256'] != control['scene']['sha256']:
                    raise ValueError('Current globalIFS scene mismatch')
                control.update(final_status=r['status'], final_result=r)
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(report, indent=2)+'\n')
    print(json.dumps({'classes':len(classes),'output':str(args.output),'execution_logs':len(args.test_log)}))


if __name__ == '__main__':
    main()
