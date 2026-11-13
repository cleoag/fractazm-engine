"""Re-render C++ against immutable, hash-matched Pascal image evidence.

Retains every reference case, including reference errors. Never silently skips
changed or missing scenes. The Pascal source/binary identity comes from the
original run, not a newly built executable.
"""
import argparse
from collections import Counter
from dataclasses import asdict
import json
from pathlib import Path
from pascal_formula_identity import validate_reference_formula_load
from external_validation import ROOT, digest, write_json, compact_image_evidence, compact_replay_report
from image_diff import DEFAULT_THRESHOLDS, compute_metrics, evaluate, load_rgb
from sweep import run_render
from compatibility_precision import precision_evidence, verify_precision_evidence
from validate_monte_carlo import source_provenance


def selected_case_ids(requested, cases):
    """Reject typos and duplicate IDs rather than silently reducing coverage."""
    if not isinstance(requested, list) or not requested or any(not isinstance(v, str) for v in requested):
        raise ValueError('Case selection must be a nonempty JSON list of scene IDs')
    if len(set(requested)) != len(requested):
        raise ValueError('Duplicate selected case IDs')
    unknown = set(requested) - {c['id'] for c in cases}
    if unknown:
        raise ValueError('Unknown selected case IDs: ' + ', '.join(sorted(unknown)))
    return set(requested)


def validate_resume(report, exe, reference, checkpoint, cases, output):
    """Validate the frozen batch prefix, including failed results, before reuse."""
    if report.get('requested_formulas') or report.get('requested_cases'):
        raise ValueError('Selected-formula replay cannot resume as a full prefix')
    if report.get('cpp_sha256') != digest(exe.read_bytes()):
        raise ValueError('Resume C++ executable changed')
    if report.get('cpp_precision_requested') != 'compatible':
        raise ValueError('Resume requires explicit compatible precision evidence')
    if report.get('reference') != str(reference.resolve()) or report.get('thresholds') != DEFAULT_THRESHOLDS:
        raise ValueError('Resume reference or thresholds changed')
    raw = checkpoint.read_bytes()
    old = json.loads(raw)
    if 'batches' not in old:
        raise ValueError('Resume requires a frozen batch checkpoint')
    compact_replay_report(report)
    results = report['results']
    if len(results) > len(old['results']):
        raise ValueError('Resume exceeds frozen reference')
    by_name = {c['id']: c for c in cases}
    summaries = [reference] if reference.is_file() else sorted(reference.rglob('summary.json'))
    if len(summaries) != 1:
        raise ValueError('Resume requires exactly one reference summary')
    reference_summary = summaries[0]
    for item, original in zip(results, old['results']):
        name = item['scene']
        if report['provenance_records'].get(item.get('reference_provenance_sha256')) != old['provenance']:
            raise ValueError('Resume reference provenance changed')
        if name != original['scene'] or item.get('scene_sha256') != original.get('scene_sha256'):
            raise ValueError('Resume scene order or hash changed')
        if item.get('reference_summary_sha256') != digest(raw):
            raise ValueError('Resume reference checkpoint changed')
        scene = ROOT / by_name[name]['scene']
        if digest(scene.read_bytes()) != by_name[name]['sha256']:
            raise ValueError('Resume scene bytes changed')
        # Errors remain errors. Successful measurements must retain both images.
        if item['status'] in ('pass', 'fail'):
            verify_precision_evidence(output/'cpp'/item['suite']/(name+'.png'), item)
            for language, path in [
                ('pascal', reference_summary.parent/'pascal'/(name+'.png')),
                ('cpp', output/'cpp'/item['suite']/(name+'.png')),
            ]:
                if not item.get(language+'_image_sha256') or digest(path.read_bytes()) != item[language+'_image_sha256']:
                    raise ValueError('Resume '+language+' image changed')
    return len(results)


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--reference', type=Path, required=True)
    ap.add_argument('--cpp-exe', type=Path, required=True)
    ap.add_argument('--output', type=Path, required=True)
    ap.add_argument('--checkpoint-every', type=int, default=1, help='Commit every N cases; an interrupted suffix is rendered again after verified resume')
    ap.add_argument('--timeout-ms', type=int, default=240000)
    ap.add_argument('--manifest', type=Path, help='Required for batch-run reference summaries')
    ap.add_argument('--snapshot-prefix', action='store_true', help='Replay only the committed prefix of an unfinished batch; pending cases remain explicit')
    ap.add_argument('--resume', action='store_true', help='Continue a batch replay against its original frozen reference checkpoint')
    ap.add_argument('--formula', action='append', default=[], help='Select a named formula from one batch manifest; repeat for multiple formulas. Does not certify the full manifest.')
    ap.add_argument('--case-list', type=Path, help='JSON list of exact scene IDs from one batch manifest; cannot combine with --formula or --resume.')
    args = ap.parse_args()
    if args.checkpoint_every < 1: ap.error('--checkpoint-every must be positive')
    args.output.mkdir(parents=True, exist_ok=args.resume)
    exe = args.cpp_exe.resolve()
    report = {'provenance': source_provenance(), 'cpp_sha256': digest(exe.read_bytes()),
              'cpp_precision_requested': 'compatible',
              'reference': str(args.reference.resolve()), 'thresholds': DEFAULT_THRESHOLDS,
              'results': [], 'finished': False, 'provenance_records': {}}
    summaries = [args.reference] if args.reference.is_file() else sorted(args.reference.rglob('summary.json'))
    if not summaries: raise ValueError('No reference summaries found')
    if args.formula and args.case_list:
        raise ValueError('Choose formula selection or case selection')
    if (args.formula or args.case_list) and (args.resume or len(summaries) != 1 or not args.manifest):
        raise ValueError('Formula selection requires one batch and its manifest, without resume')
    resume_count = 0
    if args.resume:
        if len(summaries) != 1 or not args.manifest:
            raise ValueError('Resume requires one batch summary and its manifest')
        report = json.loads((args.output/'summary.json').read_bytes())
        resume_count = validate_resume(report, exe, args.reference, args.output/'reference-checkpoint.json',
                                       json.loads(args.manifest.read_bytes())['cases'], args.output)
    report['finished'] = False
    report['checkpoint_every'] = args.checkpoint_every
    report.setdefault('tool_history', []).append({'sha256':digest(Path(__file__).read_bytes()),'resume_from':resume_count,'checkpoint_every':args.checkpoint_every})
    for summary in summaries:
        raw = (args.output/'reference-checkpoint.json').read_bytes() if args.resume else summary.read_bytes()
        old = json.loads(raw)
        batch = 'manifest_sha256' in old and 'batches' in old
        if (args.formula or args.case_list) and not batch:
            raise ValueError('Formula selection requires a batch reference')
        selected_ids = None
        if batch:
            if not args.manifest or digest(args.manifest.read_bytes()) != old['manifest_sha256']:
                raise ValueError('Batch manifest missing or changed')
            cases = json.loads(args.manifest.read_bytes())['cases']
            if len(old['results']) > len(cases): raise ValueError('Batch exceeds manifest')
            for c,r in zip(cases,old['results']):
                if c['id']!=r['scene'] or c['sha256']!=r['scene_sha256']:
                    raise ValueError('Batch prefix differs from manifest')
            if not old.get('finished') and not args.snapshot_prefix:
                raise ValueError('Unfinished batch requires --snapshot-prefix')
            by_name={c['id']:c for c in cases}
            report['pending_reference_cases']=[c['id'] for c in cases[len(old['results']):]]
            if args.case_list:
                selection_bytes = args.case_list.read_bytes()
                selected_ids = selected_case_ids(json.loads(selection_bytes), cases)
                report['requested_cases'] = sorted(selected_ids)
                report['case_list_sha256'] = digest(selection_bytes)
                report['scope'] = 'selected-cases; not full-manifest coverage'
                report['selected_case_count'] = len(selected_ids)
                report['unselected_case_count'] = len(cases) - len(selected_ids)
                report['pending_reference_cases'] = [name for name in report['pending_reference_cases'] if name in selected_ids]
            if args.formula:
                requested = set(args.formula)
                available = {name for c in cases for name in c.get('target_formulas', [])}
                if requested - available:
                    raise ValueError('Unknown requested formulas: ' + ', '.join(sorted(requested - available)))
                selected_ids = {c['id'] for c in cases if requested.intersection(c.get('target_formulas', []))}
                report['requested_formulas'] = sorted(requested)
                report['scope'] = 'selected-formulas; not full-manifest coverage'
                report['selected_case_count'] = len(selected_ids)
                report['unselected_case_count'] = len(cases) - len(selected_ids)
                report['pending_reference_cases'] = [name for name in report['pending_reference_cases'] if name in selected_ids]
            report['reference_finished']=old.get('finished',False)
            if not args.resume:
                (args.output/'reference-checkpoint.json').write_bytes(raw)
        else:
            if not old.get('ended'): raise ValueError('Reference run is unfinished')
            scene_dir = Path(old['provenance']['scene_dir'])
        for case in old['results'][resume_count:]:
            name = case['scene']
            if selected_ids is not None and name not in selected_ids:
                continue
            item = {'scene': name, 'suite': summary.parent.parent.name,
                    'reference_summary': str(summary.resolve()), 'reference_summary_sha256': digest(raw),
                    'reference_provenance': old['provenance'], 'scene_sha256': case.get('scene_sha256')}
            if batch: item['reference_pascal_sha256']=old['pascal_sha256']
            try:
                scene = ROOT / by_name[name]['scene'] if batch else scene_dir / (name + '.m3p')
                if digest(scene.read_bytes()) != case.get('scene_sha256'):
                    raise ValueError('Scene changed or missing original hash')
                if batch:
                    item['pascal_formula_load'] = validate_reference_formula_load(
                        summary.parent, case, scene, by_name[name].get('slots'))
                reference = summary.parent / 'pascal' / (name + '.png')
                if not reference.exists(): raise ValueError('Missing Pascal reference: ' + str(case.get('error')))
                recorded=case.get('pascal_image_sha256')
                if batch and not recorded: raise ValueError('Batch case lacks committed Pascal image hash: '+str(case.get('error')))
                if recorded and digest(reference.read_bytes())!=recorded:
                    raise ValueError('Pascal reference image hash changed')
                target = args.output / 'cpp' / item['suite'] / (name + '.png')
                if digest(exe.read_bytes())!=report['cpp_sha256']:raise ValueError('C++ executable changed during replay')
                code, elapsed, log = run_render(exe, scene, target, 1 if batch else old['threads'], args.timeout_ms, [])
                item['cpp_ms'] = elapsed
                if code: raise RuntimeError(log)
                item.update(precision_evidence(target))
                a, b = load_rgb(reference), load_rgb(target)
                metrics = compute_metrics(a, b, DEFAULT_THRESHOLDS['diff_ratio_threshold'])
                checks = evaluate(metrics, DEFAULT_THRESHOLDS)
                item.update(status='pass' if all(checks.values()) else 'fail', diff=asdict(metrics),
                            exact_rgb=metrics.max_abs_delta == 0, checks=checks,
                            pascal_image_sha256=digest(reference.read_bytes()), cpp_image_sha256=digest(target.read_bytes()))
            except (OSError, ValueError, RuntimeError) as exc:
                item.update(status='error', error=str(exc))
            item = compact_image_evidence(item,report['provenance_records'])
            report['results'].append(item)
            report['counts'] = dict(Counter(r['status'] for r in report['results']))
            if len(report['results']) % args.checkpoint_every == 0:
                write_json(args.output / 'summary.json', report)
            print(len(report['results']), name, item['status'], flush=True)
    report['finished'] = True
    write_json(args.output / 'summary.json', report)
    return int(bool(report.get('pending_reference_cases')) or not report['results'] or any(r['status'] != 'pass' for r in report['results']))

if __name__ == '__main__': raise SystemExit(main())
