"""Build an evidence-based work queue; frequency is unique scenes, not slot count.

This queue prioritizes investigation. A failing image containing a formula is
not proof that the formula caused the failure. Historical evidence stays labeled.
"""
import argparse
from collections import Counter, defaultdict
from pathlib import Path
from external_validation import digest, write_json
from report_external_coverage import latest_image_cases
import json


def image_category(result):
    status = result['status']
    if status in ('pass', 'fail'):
        return status
    error = str(result.get('error', '')).lower()
    if status == 'pascal_error' or error.startswith(('missing pascal reference', 'pascal retry:', 'pascal reference formula load mismatch:')):
        return 'reference_blocked'
    # Unknown execution errors require attribution, not a speculative code fix.
    return 'cpp_error' if status == 'cpp_error' else 'unattributed_error'


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--run', type=Path, required=True)
    ap.add_argument('--coverage', type=Path, required=True)
    ap.add_argument('--output', type=Path, required=True)
    a = ap.parse_args()
    evidence = {}
    def read(path):
        raw = path.read_bytes()
        evidence[str(path)] = digest(raw)
        return json.loads(raw)
    coverage = read(a.coverage)
    study = read(a.run/'native-study/study.json')
    manifest = read(a.run/'manifest.json')
    challenges = read(a.run/'challenges-v4/manifest.json')['cases']
    expected = {c['id']: c['sha256'] for c in challenges}
    names = {f['name'].casefold(): f['name'] for f in manifest['formulas']}
    if len(names) != len(manifest['formulas']):
        raise ValueError('Ambiguous case-insensitive registry names')
    usage = defaultdict(set)
    unknown = defaultdict(set)
    for scene in study['scenes']:
        for slot in scene['formula_slots']:
            name = slot.get('name', '').casefold()
            if not name or slot.get('iterations', 0) == 0:
                continue
            target = usage if name in names else unknown
            target[name].add(scene['sha256'])
    point_cases = {}
    # Explicit chronological overlays; do not treat an old IFS loader run as current.
    for phase in ['scene-points-current-v9', 'ifs-tess-points-v2',
                  'cantor-points-v1', 'amazing-surf2-points-v1',
                  'amazing-surf-priority-before-v1', 'amazing-surf-points-v1',
                  'heightmap-points-v1', 'aboxsmooth-points-v1','spherefold-points-v2','state-batch-expanded-v2','fold-rounding-points-v1','rootcause-points-v2','contract-batch-points-v1','contract-map-points-v1','shape-state-points-v1','orbit-shapes-three-points-v1','manowar-default-preconditions-v1','room-triplex-points-v1','fold-cut-points-v1','box-tetra-pine-points-v1','gnarly-family-points-v1','dodeca-wavy-points-v1','axis-wave-box-points-v1','map-icen-points-v1','map-icen-map-points-v1','iq-hex-points-v1','slice-coord-points-v1','angle-impact-after-v1','fold-kali-tube-points-v1','varyscale-pair-points-v1','amazingbox-sse2-points-v1','projection-reciprocal-points-v1','devil-citrus-points-v1','magnet-bicomplex-points-v2','angular-radius-points-v1','heightmap-family-planar-points-v1','heightmap-family-sphere-points-v1','branch-julia-points-v2','ifs-apply-points-v1','fold-order-points-v2','knots-weird-points-v1','fold-poincare-points-v1','last-image-points-v2','remaining-points-points-v1','scalar-map-points-after-v2','cyl-torus-points-v1','trifox-points-v1']:
        p = a.run/phase/'summary.json'
        if p.exists():
            for result in read(p)['results']:
                point_cases[result['case']] = result
    points = defaultdict(Counter)
    for result in point_cases.values():
        points[result['name'].casefold()][result['status']] += 1
    layouts = {r['name'].casefold(): r for r in
               read(a.run/'parameter-layouts-v1/summary.json')['results']}
    for phase in ('orbit-shapes-layouts-v1','manowar-layouts-v2','room-fold-layouts-v2','box-tetra-pine-layouts-v1','gnarly-family-layouts-v2','dodeca-wavy-layouts-v2','axis-wave-box-layouts-v1','map-icen-layouts-v2','iq-hex-layouts-v1','slice-coord-layouts-v1','angle-impact-layouts-v1','angle-hex-layout-v1','fold-kali-tube-layouts-v1','logsp-layouts-v2','varyscale-pair-layouts-v1','amazingbox-sse2-layouts-v1','projection-reciprocal-layouts-v1','devil-citrus-layouts-v1','magnet-bicomplex-layouts-v2','angular-radius-layouts-v1','heightmap-family-layouts-v1','branch-julia-layouts-v1','ifs-apply-layouts-v2','fold-order-layouts-v2','knots-weird-layouts-v1','fold-poincare-layouts-v1','last-image-layouts-v1','remaining-points-layouts-v1','scalar-map-layouts-v1','cyl-torus-layouts-v1','trifox-layouts-v2'):
        path=a.run/phase/'summary.json'
        if path.exists():
            for result in read(path)['results']:
                layouts[result['name'].casefold()]=result
    overlays = {}
    for phase in ['general-images-v1', 'ifs-tess-images-v4',
                  'priority-images-v1', 'priority-images-v2', 'amazing-surf-images-v2',
                  'heightmap-images-v1', 'aboxsmooth-images-v1','spherefold-images-v2','state-batch-images-v2','fold-rounding-images-v1','rootcause-images-v2','contract-batch-images-v1','jit-reference-audit-v1','shape-state-images-v1','shape-missing-reference-image-v1','orbit-shapes-three-images-v3','orbit-missing-images-v2','manowar-images-v1','room-triplex-images-v1','fold-cut-images-v1','room-fold-current-images-v1','crossbifold-missing-images-v1','remaining-images-radius-order-v1','box-tetra-pine-images-v1','gnarly-family-images-v2','gnarly-missing-image-v1','dodeca-wavy-images-v1','dodeca-wavy-missing-images-v1','axis-wave-box-images-v1','map-icen-images-v1','iq-hex-images-v1','iq-hex-missing-images-v1','slice-coord-images-v1','slice-coord-missing-images-v1','angle-impact-images-v1','angle-missing-images-v1','fold-kali-tube-images-v1','fold-kali-tube-missing-images-v1','fold-kali-tube-images-v2','fold-kali-tube-missing-images-v2','iteration-depth-impact-images-v2','varyscale-pair-images-v1','amazingbox-sse2-images-v1','projection-reciprocal-images-v1','devil-citrus-images-v1','magnet-bicomplex-images-v2','angular-radius-images-v1','heightmap-family-images-v1','branch-julia-images-v2','ifs-apply-images-v1','fold-order-images-v2','knots-weird-images-v1','fold-poincare-images-v1','last-image-images-v1','remaining-points-images-v1','priority-reference-recovery-v1','scalar-map-images-v2','cyl-torus-images-v1','priority-reference-recovery-v2']:
        path = a.run/phase/'summary.json'
        if path.exists():
            overlays[phase] = read(path)['results']
    # Completed recovery is separate from the retained failed/unfinished attempts.
    for phase in ('priority-reference-recovery-v4', 'trifox-images-v2'):
        path = a.run/phase/'summary.json'
        if path.exists():
            report = read(path)
            if report.get('finished'):
                overlays[phase] = report['results']
    rows = []
    for old in coverage['formulas']:
        name = old['formula']; key = name.casefold()
        identities = {c['id']: expected[c['id']] for c in challenges
                      if key in [n.casefold() for n in c['target_formulas']]}
        current = latest_image_cases({**old['images'], **overlays}, identities)
        image = Counter(image_category(r) for r in current.values())
        image['missing'] = len(identities) - len(current)
        p = points[key]
        has_error = bool(image['cpp_error'])
        has_failure = bool(image['fail'] or p['fail'])
        has_gap = (bool(p['unverified'] or p['error'] or image['reference_blocked']
                        or image['unattributed_error'] or image['missing']) or not p)
        # Raw unused bytes do not establish a rendering defect.
        layout = layouts.get(key, {}).get('status', 'missing')
        tier = (1 if has_error else 2 if has_failure else 3 if has_gap
                else 4 if layout != 'pass' else 5)
        rows.append(dict(formula=name, tier=tier,
                         downloaded_unique_scenes=len(usage[key]),
                         image_counts=dict(image), point_counts=dict(p),
                         parameter_layout=layout,
                         affected_cases=sum(image[s] for s in ['cpp_error', 'fail',
                             'reference_blocked', 'unattributed_error', 'missing'])+p['error']+p['fail'],
                         image_issues=[dict(case=k, category=image_category(v),
                             error=v.get('error'), evidence=v.get('evidence', v.get('reference_summary')))
                             for k,v in current.items() if v['status']!='pass'],
                         downloaded_scene_sha256=sorted(usage[key])))
    rows.sort(key=lambda r: (r['tier'], -r['downloaded_unique_scenes'],
                            -r['image_counts'].get('fail', 0), -r['affected_cases'], r['formula'].casefold()))
    common_checks = {}
    for phase in ['depth-codec-v1', 'priority-parameter-layouts-v1',
                  'rotation-original-source-v1', 'rotation-loader-impact-v1',
                  'priority-images-v2', 'priority-parameters-v1',
                  'iteration-step-after-v1', 'iteration-refinement-repro-v2']:
        path = a.run/phase/'summary.json'
        if path.exists():
            result = read(path)
            common_checks[phase] = {k: result[k] for k in
                ['rows', 'failed_rows', 'failed_lanes', 'exact', 'finished', 'counts',
                 'cpp_exact_matrices', 'patched_pascal_exact_matrices'] if k in result}
    report = dict(scope=__doc__, evidence_sha256=evidence,
                  criteria=['P0: measured common ordinary renderer/loader defect',
                            'P1: attributed C++ execution errors; P2: measured numerical/image failure',
                            'P3: missing reference, unattributed errors, unverified or missing tests',
                            'P4: raw loader differences without established output effect',
                            'P5: regression maintenance; no universal parity claim',
                            'Within tier: unique downloaded scenes, failing images, affected cases, stable name',
                            'Choose a demonstrated shared root cause before isolated symptoms'],
                  shared_first=['Investigate common ordinary renderer/loader cause before isolated formula edits',
                                'Classify independent parameter-buffer differences before formula edits'],
                  common_checks=common_checks,
                  shared_investigations=[
                      'Reference audit: original FSINCOS matches C++ in six matrices; patched Pascal differs in two. Preserve both reference identities before further loader fixes.',
                      'Map/pointer formula oracles: build semantic checks for currently unverified cases',
                      'Monte Carlo is temporarily deferred by user request; exclude it from active work and checks. Keep GUI inventory gaps separate from formula passes'],
                  scenes=len(study['scenes']), unique_scenes=len({s['sha256'] for s in study['scenes']}),
                  formulas=len(rows), complete=False, queue=rows,
                  unresolved_scene_names={k:len(v) for k,v in sorted(unknown.items())})
    write_json(a.output, report)
    for row in rows[:20]:
        print(row['tier'], row['formula'], row['downloaded_unique_scenes'],
              row['image_counts'], row['point_counts'], row['parameter_layout'])


if __name__ == '__main__':
    main()
