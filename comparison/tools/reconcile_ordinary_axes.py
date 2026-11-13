"""Reconcile the finite75-axis ordinary inventory with retained checks and reviewed equivalences.

Read-only with respect to source, tests and historical evidence; does not execute a renderer.
The output accounts for the declared finite classes, not all Pascal source or real parameters.
"""
import argparse
from collections import Counter
import hashlib
import json
from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[2]
R = 'comparison/results/stage-a-20261018/'
U = 'comparison/runs/stage-a-20261018/'

EVIDENCE = {
    'native': [U+'candidate-v20/ctest-full-output.log', U+'candidate-v20/build-identity.json'],
    'loader': [R+'legacy-loader-evidence-v20.json', R+'legacy-loader-qualification-v20.json'],
    'm3i': [R+'m3i-source-qualification-v20.json'],
    'direct': [R+'direct-header-contract-v20.json', U+'file-readback-candidate-v20/summary.json'],
    'integer': [R+'integer-domain-ledger-v6.json', 'tests/test_integer_options.cpp'],
    'const': ['tests/test_renderer.cpp', 'tests/test_formula_iteration_guard.cpp', R+'loader-used-bytes-v9.json'],
    'finite': [U+'finite-matrices-candidate-v20/summary.json'],
    'lighting': [R+'lighting-modes-v1/summary.json', R+'lighting-modes-v1/manifest.json', U+'finite-matrices-candidate-v20/summary.json'],
    'maps': [R+'map-evidence-v20.json'],
    'volume': [R+'volume-evidence-v20.json'],
    'couplings': [R+'ordinary-couplings-v20.json', R+'targeted-couplings-checkpoint-v20.json'],
    'live': [U+'live-controls-candidate-v20/summary.json', R+'effective-modes-v1/live-qualification.json'],
    'ao': [R+'ao-tile-checkpoint-v20.json'],
    'guards': ['tests/test_formula_iteration_guard.cpp', 'tests/test_formula_validation.cpp', 'tests/test_render_failures.cpp'],
    'diagnostics': [U+'diagnostics-candidate-v20/summary.json'],
    'formula': [R+'final-candidate-v19/formula-status.json', U+'final-external-points-v20/summary.json',
                U+'final-selector-points-v20/summary.json', U+'integer-boundaries-v20/summary.json',
                R+'integer-domain-ledger-v6.json', 'tests/test_renderer.cpp'],
}

# The loader's tiled-cache calculation was moved to this reviewed helper after
# the original inventory. Keep both anchors rather than silently dropping it.
ANCHOR_UPDATES = {
    ('tiling', 'cpp'): ('GetLegacyCacheTileDimensions(',
                       'v20 centralizes the original clamped cache dimensions for M3P/M3I.')
}

# These source-reviewed domains extend the original scene-only inventory.
# Detailed local constraints and independent witnesses live in the linked
# direct-header report; none authorizes blanket enum rejection or normalization.
DIRECT_CLASSES = {
    'hybrid-mode': ['hybrid-low2-mode3'],
    'inside': ['inside-low2-mode3'],
    'optics': ['camera-full-byte'],
    'volume-light': ['volume-low3-mode7'],
    'visible-light-shape': ['visible-light-shapes10-12-14'],
    'dof-dispatch': ['dof-direct-four-passes'],
    'stereo': ['stereo-all-byte-values'],
    'de-combination': ['decomb-selector-clamp'],
    'color-option': ['color-selector-clamp'],
}

# Explicit review decisions. Every old axis must have exactly one entry.
# "check+equivalence" means the described local equivalence, never a blanket alias.
DECISIONS = {}


def add(ids, basis, evidence, reason, limitation='Finite witnesses and the stated source class; arbitrary numeric inputs/interactions are not certified.'):
    for name in ids.split():
        if name in DECISIONS:
            raise ValueError('Duplicate review decision '+name)
        DECISIONS[name] = dict(basis=basis, evidence_groups=evidence.split(), justification=reason, limitation=limitation)


add('file-container', 'check+equivalence', 'loader m3i',
    'Raw original records are independently compared. Zlib is a C++ container extension:121 decompressions feed the same checked record loader; no original compressed-file algorithm is claimed.')
add('header-layout cached-pixel-layout addon-layout', 'check', 'loader m3i',
    '121 independent loaded records span MandId0..44 and source size/migration boundaries;30 M3I cases cover12/16/18-byte records plus32768 angle records. Version markers/inactive bytes are qualified separately.',
    'Original MandId<7 iMaxIts is uninitialized in25 controls; deterministic port policy is not strict parity. Qualified camera tolerance is not exact storage equality.')
add('tiling', 'check', 'ao native', '21 independent tile controls, packed state,22 repeats and native tile dimensions/stride/replay tests; unsupported original postprocessor-capacity combinations have explicit tested rejection.')
add('slot-identity', 'check', 'formula loader guards native', '457 ordinary registered groups, loader identities/aliases including legacy Flip migration, unsupported-name errors and typed tails are checked separately from kernel outputs.')
add('slot-position', 'check+equivalence', 'const guards native', 'Slots0..5 use the same indexed initializer;72 original layout rows plus first/middle/end and second-group guard scenarios qualify distinct routing boundaries.')
add('integer-round negative-buffer', 'check', 'integer const native', 'Original FPC308 conversion cases and924 stores, FOLDING selection, negative-buffer capacity/compound cursor fixtures are permanent tests; pointer relocation bytes are not compared as numeric constants.')
add('option-type preprocess-boundary', 'check+equivalence', 'const integer native', 'The23 typed storage cases and saved/runtime cursor mappings are explicit. Original loader buffer fixtures, angle matrices and per-formula used-byte comparisons supply independent values; unused/pointer storage is separately qualified.')
add('hybrid-mode', 'check+equivalence', 'direct finite native', 'File mode3 normalizes to0; direct mode3 retains split layout.72 independent layout rows plus existing alternating/interpolated/DE-combination images distinguish the domains.')
add('de-combination mix-color', 'check+equivalence', 'finite guards direct native', 'Existing effective modes cover six DE operations and six valid mixed controls cover three color branches/first-group kinds. Original-invalid second-group mix configurations are rejected; high selector values retain local FormulaType clamp, not a global alias.')
add('repeat-control interpolation-weight', 'check', 'guards finite native', 'Original interpolation setup fixture and307 native guard scenarios cover stored Single weights, fallback, endpoints/nonfinite rejection, slot skips/stops/wrap and independently reproduced legacy disabled-start exit.',
    'Original22 nonterminating conditional scenes remain excluded reference cases; finite replacements and explicit guard behavior do not relabel them as parity PASS.')
add('julia orbit-driver formula-write-contract', 'check', 'formula guards native', 'Existing independent original instruction fixtures, typed state fields, caller profiles and hybrid scene checks cover the named3D/4D/IFS/interpolation/group drivers and C/W/radius/DE/trap/persistent/conditional state classes.',
    '13 generic Manowar caller cases, four odd-Flip caller-memory cases and two Trifox zero-power cases remain original limitations. Per-formula finite evidence is not proof of every state history.')
add('inside', 'check+equivalence', 'finite couplings direct native', 'Ordinary outside/inside/both have independent transmission controls. Direct raw3 uses the same inside/both Boolean setup asraw2 but retains sceneRI; exact Pascal/C++ setup and refraction condition are recorded. File raw3 becomesoutside0.')
add('disable-analytic-de normals-mode vary-de-stop color-option color-on-it', 'check+equivalence', 'finite live native', 'Qualified effective-mode scenes cover requested/forced drivers and color reruns; shared setup clamps color>=5 to5. Post-depth normals have separate live output evidence; header presence alone is not used as stage proof.')
add('termination', 'check', 'guards formula native', 'Permanent original DE/refinement/packed boundary fixtures and caller guards cover escape/maxIt/minimum/step/refinement/unordered paths; failures do not publish successful partial images.',
    'The three known huge-work formula counts are classified repeated-loop bodies with StageB bounded-work/cancellation obligations, not numerically executed or silently clamped.')
add('optics stereo', 'check+equivalence', 'direct native', 'Original file readback rejects full optics bytes>2 and resets stereo. Direct camera masks/FOV equality checks are explicitly distinct; all256 stereo bytes×3 configurations are independently verified. No blanket GUI rejection is added to internal Render(header).')
add('slice-plane slice-driver cut-mask', 'check+equivalence', 'finite native', 'Existing qualified modes include slice orbit/DE-combination and all cut masks0..7; slice raw values except1/3 normalize to2. Cut combinations remain distinct nearest-plane cases.')
add('hard-shadow-stage completed-shadow-state', 'check', 'finite live native', 'Separate live gateoff/hard/soft controls verify actual Shadow changes; permanent paint replay tests preserve completed state and avoid forcing completion when the pass is disabled.')
add('selected-shadow-light', 'check+equivalence', 'lighting native', 'Independent lighting_shadow0..5 use one identical source moved across six slots and retain exact RGB. Pascal/CPP sorted index mapping and0x400<<slot packing preserve original identity; historical old inventory omitted these later controls.')
add('ao-kernel ao-quality de-ao-quality de-ao-dither de-ao-first-random', 'check+equivalence', 'finite ao native', 'Effective ordinary AO scenes cover actual reachable dispatch/quality/dither/random branches; v20 adds24 exact images/state and threshold SIMD/odd-width witnesses. Raw DE routing is resolved before derived ATi; unreachable video path is not credited.')
add('random-ao-repeat', 'check', 'ao native', 'Original repeat1/2×logical1/2×ordinary/panorama witnesses and48 fixed-schedule repeat/trace comparisons verify lane lifetime and repeated accumulation.')
add('light-kind light-position diffuse-function', 'check+equivalence', 'lighting maps native', 'Later lighting interventions cover directional/positional, linear/squared diffuse0..3; real maps prove map-enabled routing. Disabled raw3 and map-number0 source-normalize to disabled1.')
add('specular-exponent', 'check+equivalence', 'lighting native', 'The masked integer exponent is2<<(raw&7) through one FastIntPow algorithm. Qualified low/high interventions plus existing arithmetic fixtures bound the numeric parameter class; no eight distinct algorithms are invented.')
add('visible-light-shape', 'check+equivalence', 'lighting direct native', 'Original ordinary shape images and1152 scalar fixtures cover0/2/4/6/8/10/12/14. Actual MakeHeader retains10/12/14. Their default drawing branch shares compositing, but positional softshadow splits10(70/radius) from12/14(40/radius).')
add('color-arithmetic gamma-class color-source gradient-cycling gradient-interpolation total-light-mode', 'check+equivalence', 'finite lighting native', 'Existing finite scene controls cover linear/squared, gamma below/identity/above, smooth/trap, wrap and nearest/interpolation. All nonzero total-light values share the same checked alternative function; arithmetic branches remain distinct.')
add('diffuse-map-projection background-projection resource-contract formula-map-callback', 'check+equivalence', 'maps native', 'Source-linked map report includes real BMP/P2 assets, preprocessing32rows,2520 sampled channels and map-dependent controlled images. Ordinary IniIt3D fixes sphere/XY spline callbacks; unused helper alternatives are not ordinary selectable algorithms.')
add('fog-driver fog-composite far-fog', 'check+equivalence', 'finite lighting volume native', 'Existing distance/iteration/volume controls and near/far lighting interventions supply actual image influence; raw composite3 normalizes1. Volume overrides iteration-fog setup by source.')
add('volume-light', 'check+equivalence', 'lighting direct volume native', 'Original volume1..6 directional permutations and positional1/6 controls retain exact RGB; direct low7 clamps to the same light5 aslow6 with the same upper nibble. File39→38 is independently measured.')
add('volume-light-position volume-map-face', 'check+equivalence', 'volume native', 'Eight original volume images and6068 numeric checks cover signed cube faces, axis/tie/seam/rounding and directional transforms. Shared sampler/fog routing links primary and secondary consumers without claiming every material/recursion combination.')
add('secondary-mode bounce-boundary secondary-medium', 'check+equivalence', 'couplings native', '13 qualified transmission images distinguish medium/RI/absorption/control boundaries; independent63 Power/product and24 FPC predicate checks preserve original branch arithmetic. Existing secondary/replay tests cover final-depth state.')
add('dof-dispatch dof-depth-region', 'check+equivalence', 'couplings direct native', '13 original DOF controls cover ordinary1/2/3 passes and depth regions; direct4 passes repeats the same original default-aperture body and is checked by192 exact-focus/canary cases. It is not an alias of3 passes.')
add('surface-class normal-source post-depth-normals', 'check+equivalence', 'couplings live native', 'Independent8769 depth/15704 normal samples, current named packing tests and live post-depth on/off buffers qualify shared encoding plus producer state; primary-captured buffers are not substituted for final normals.')
add('export-kind error-class', 'check', 'native guards loader m3i', 'Current native tests exercise BMP/PNG/depth8/16/32/normal exports, successful C-API buffers and bounded errors/recovery. File byte identity is not claimed for independent encoder choices.')
add('ordinary-schedule random-state-owner', 'check+equivalence', 'ao diagnostics native', 'Current logical1/16 plain/repeat/diagnostic/trace controls plus AO logical1/2 lanes and exact repeated tile controls qualify fixed schedules. Different logical-worker counts are separate original profiles.',
    'Independent concurrent jobs, ownership, cancellation and schedule-independent profiles are StageB work, not StageA fixed-schedule parity.')
add('trace-stage replay-state', 'check', 'diagnostics live couplings native', 'Current representative plain/repeat/all-trace checks and permanent stage-window/replay tests preserve output and packed handoffs for primary,AO,paint/reflection and late passes.')


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--output', type=Path, required=True)
    args = ap.parse_args()
    inputs = {}
    def link(p):
        path = ROOT/p
        digest = hashlib.sha256(path.read_bytes()).hexdigest()
        inputs[p] = digest
        return {'path':p, 'sha256':digest}
    source_path = R+'ordinary-source-audit-v2.json'
    original = json.loads((ROOT/source_path).read_bytes())
    ids = {row['id'] for row in original['axes']}
    if ids != set(DECISIONS) or len(original['axes']) !=75:
        raise ValueError({'missing':sorted(ids-set(DECISIONS)), 'extra':sorted(set(DECISIONS)-ids)})
    groups = {name:[link(path) for path in paths] for name,paths in EVIDENCE.items()}
    direct = json.loads((ROOT/(R+'direct-header-contract-v20.json')).read_bytes())
    if not direct.get('contract_tests_all_passed'):
        raise ValueError('Reserved contract tests not qualified on selected candidate')
    direct_by_id = {item['id']: item for item in direct['classes']}
    missing_direct = {item for items in DIRECT_CLASSES.values() for item in items} - set(direct_by_id)
    if missing_direct:
        raise ValueError('Direct-header evidence class names changed: '+str(sorted(missing_direct)))
    built = json.loads((ROOT/direct['candidate']['build_identity']['path']).read_bytes())
    source_hashes = {(ROOT/item['path']).resolve():item['sha256'] for item in built['sources']}
    test_text = (ROOT/(U+'candidate-v20/ctest-full-output.log')).read_text(encoding='utf-8')
    native_tests = {}
    for block in re.split(r'(?m)^\d+/\d+ Testing: ',test_text)[1:]:
        name = block.splitlines()[0].strip()
        native_tests[name] = 'pass' if '\nTest Passed.\n' in block else 'not_passed'
    if len(native_tests) != 18 or set(native_tests.values()) != {'pass'}:
        raise ValueError('Expected the frozen 17 project tests plus zlib example to pass')
    rows = []
    for old in original['axes']:
        decision = DECISIONS[old['id']]
        source = []
        for side in ('pascal','cpp'):
            item = old[side]
            current = link(item['path'])
            if side == 'cpp' and source_hashes.get((ROOT/item['path']).resolve()) != current['sha256']:
                raise ValueError('Source differs from frozen candidate: '+item['path'])
            lines = (ROOT/item['path']).read_text(encoding='latin1').splitlines()
            anchor, update = ANCHOR_UPDATES.get((old['id'],side),(item['anchor'],None))
            matches = [i+1 for i,line in enumerate(lines) if anchor in line]
            if not matches:
                raise ValueError('Source anchor changed: '+item['path']+' '+anchor)
            source.append(dict(current,anchor=anchor,lines=matches,language=side,
                               inventory_anchor=item['anchor'],reviewed_update=update))
        rows.append(dict(id=old['id'], family=old['family'], inventoried_classes=old['classes'],
                         gate=old['gate'], a1='accounted_finite_domain',
                         a2='recorded_finite_check_and_equivalence' if '+' in decision['basis'] else 'recorded_finite_check',
                         decision=decision, source=source,
                         old_missing_scene_values=old['not_observed_in_qualified_scene_inventory'],
                         old_missing_list_is_current_backlog=False,
                         direct_header_domain_extensions=[direct_by_id[item] for item in DIRECT_CLASSES.get(old['id'],[])],
                         outstanding_actionable_implementation_defect=None))
    report = dict(schema_version=1, scope='Reconciliation of the existing75 declared ordinary axes, not an automatic proof of every Pascal statement',
                  generator=link('comparison/tools/reconcile_ordinary_axes.py'),
                  source_inventory=link(source_path), selected_candidate=direct['candidate'],
                  deferred=['Monte Carlo','JIT'], axes=rows, evidence_groups=groups,
                  current_native_execution=dict(tests=native_tests,project_tests=17,dependency_examples=1),
                  totals=dict(axes=len(rows), families=len({r['family'] for r in rows}),
                              a1=dict(Counter(r['a1'] for r in rows)), a2=dict(Counter(r['a2'] for r in rows)),
                              extended_direct_header_axes=len(DIRECT_CLASSES),
                              unresolved_classification_rows=0, newly_confirmed_implementation_defects=0),
                  remaining_actions=[dict(gate='A5', action='Finish and seal current frozen v20 ordinary343/external primary replays; bind supplementary evidence to its own binary identities.', status='separate_primary_gate'),
                                     dict(gate='A1/A2 reporting', action='Use this reconciliation and direct-header report instead of the superseded missing-scene list; preserve old evidence as historical.', status='delivered')],
                  retained_limitations=[
                      'A finite check/class is not strict proof of every continuous input, arbitrary hybrid or caller state.',
                      'Historical originals and the v19 formula grouping preserve their own identities; v20 numerical results have their own replay evidence. A linked source/evidence classification is not necessarily an exact image comparison.',
                      'Manowar caller context, odd Flip caller memory, Trifox zero powers, original conditional nontermination and old uninitialized loader fields remain explicit original limitations.',
                      'Classifications use the current declared75-axis inventory; no claim that a static parser enumerated every possible source branch.',
                      'Huge-work bounds/cancellation, ownership, arbitrary-job parallelism, device layout and GPU/Apple execution remain StageB/C.'],
                  stage_a_complete=False, strict_full_parity_complete=False, input_sha256=inputs)
    args.output.parent.mkdir(parents=True,exist_ok=True)
    args.output.write_text(json.dumps(report,indent=2)+'\n')
    print(json.dumps(report['totals']))


if __name__=='__main__':main()
