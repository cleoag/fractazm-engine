"""Check the collector's identity gates using retained evidence and mutations.

No renderer is executed and no retained evidence is changed. The positive
controls use the real reports; each negative control changes one identity or
coverage relationship that a count-only collector would miss.
"""
import argparse
import copy
import json
from pathlib import Path

from audit_reference_gap_qualification import Audit, read, require, sha, validate_abox_cases
import audit_reference_gap_qualification as collector


def check(run):
    audit = Audit(run)
    points = read(run/'aboxmod4d-points-final/summary.json')
    lsb = read(run/'one_lsb_baseline_comparison.json')
    images = read(run/'affected-images-final/summary.json')
    current = next(r for r in images['results'] if not r['exact_rgb'])
    validate_abox_cases(points)
    audit.baseline_difference(lsb, current)
    results = []

    def reject(name, value, mutation, validate, expected):
        changed = copy.deepcopy(value)
        mutation(changed)
        try:
            validate(changed)
        except ValueError as exc:
            require(expected in str(exc), 'Rejected for unrelated reason: '+name+': '+str(exc))
            results.append(dict(id=name, rejected=True, reason=str(exc)))
        else:
            raise ValueError('Collector accepted mutated evidence: '+name)

    reject('abox_duplicate_case', points,
           lambda s:s['results'][1].update(case=s['results'][0]['case']),
           validate_abox_cases, 'Wrong ABox case identities')
    reject('abox_unknown_case', points,
           lambda s:s['results'][0].update(case='challenge_ABoxMod4d_unknown'),
           validate_abox_cases, 'Wrong ABox case identities')
    def redistribute(s):
        s['results'][0]['cases'] -= 1
        s['results'][1]['cases'] += 1
    reject('abox_same_total_wrong_distribution', points, redistribute,
           validate_abox_cases, 'Incomplete ABox inputs')
    reject('abox_missing_case', points, lambda s:s['results'].pop(),
           validate_abox_cases, 'Wrong ABox case identities')
    baseline = lambda s:audit.baseline_difference(s, current)
    reject('lsb_wrong_case', lsb, lambda s:s.update(case='unrelated'),
           baseline, 'Wrong one-LSB case identity')
    reject('lsb_unbound_old_result', lsb,
           lambda s:s['old_result'].update(scene='unrelated'),
           baseline, 'One-LSB receipt not bound to historical result')
    reject('lsb_wrong_current_image_same_hash', lsb,
           lambda s:s.update(new_image=s['old_image'],new_image_sha256=s['old_image_sha256']),
           baseline, 'One-LSB receipt names a different current image')
    reject('lsb_wrong_historical_image_same_hash', lsb,
           lambda s:s.update(old_image=s['new_image'],old_image_sha256=s['new_image_sha256']),
           baseline, 'One-LSB receipt names a different historical image')
    return dict(schema_version=1, finished=True, status='PASS',
                audit_tool_sha256=sha(Path(collector.__file__)),
                checker_sha256=sha(Path(__file__)), positive_controls=2,
                negative_controls=results,
                inputs_sha256={str(run/name):sha(run/name) for name in [
                    'aboxmod4d-points-final/summary.json',
                    'one_lsb_baseline_comparison.json',
                    'affected-images-final/summary.json']})


def main():
    ap=argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--run',type=Path,required=True)
    ap.add_argument('--output',type=Path,required=True)
    args=ap.parse_args()
    require(not args.output.exists(),'Refuse to overwrite collector controls')
    result=check(args.run.resolve())
    args.output.write_text(json.dumps(result,indent=2)+'\n')
    print('PASS: 2 positive controls; 8 identity/coverage mutations rejected')


if __name__=='__main__':
    main()
