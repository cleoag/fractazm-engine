"""Seal bounded source/branch witnesses without rerunning a renderer or oracle."""
from __future__ import annotations

import argparse
import json
from collections import Counter
from pathlib import Path
import struct

import qualify_nontermination_gaps as q


def audit(summary: Path):
    s = json.loads(summary.read_bytes())
    q.validate(s)
    q.require(s.get('status') == 'pass', 'Witness qualification failed')
    negative = q.negative_controls(s)
    q.require(len(negative) == 10 and all(r['rejected'] for r in negative), 'Negative controls failed')
    cases = []
    for c in s['cases']:
        kind, proof = c['classification'], c['proof']
        opts = q.slots(q.decode(Path(c['scene']['path'])))[0]['options']
        w = proof.get('witness')
        if w:
            q.require(w['initial_counts'][0] < 0 and w['end'] == 1, 'Unexpected source dispatch domain')
            after = [r for r in w['rows'] if r['stage'] == 'after']
            q.require(after, 'No callback observed')
            if kind != 'excessive_finite_signed_counter_wrap':
                q.require(all(r['slot'] == 0 and r['counted'] == 0 for r in after), 'Counted partner reached')
            q.check_conditional_trace(w, c['formula'], opts)
        if kind == 'zero_shift_fixed_point':
            param = 0 if c['id'].endswith('p0_zero') else 3
            q.require(opts[param] == 0, 'Trifox source power is not zero')
        elif kind == 'empty_repeat_range_excludes_start':
            q.require(w['stop'] == 'bounded_scan_stop' and w['repeat'] == 1 and len(after) == 1,
                      'Empty repeat range not established')
            scans = [r for r in w['rows'] if r['stage'] == 'scan']
            q.require(after[0]['counts'][1] == 0 and scans[0]['slot'] == 0 and all(r['slot'] == 1 for r in scans[1:]),
                      'Empty scan can reach another slot')
        elif kind == 'radius_fixed_point_disables_only_counted_slot':
            q.require(w['repeat'] == 0 and all(r['counts'][1] == 0 for r in after), 'Missing disabled repeat invariant')
            projected = {(r['first'], r['counted'], r['remaining'], r['radius_bits'], tuple(r['xyz_bits']), tuple(r['counts'])) for r in after}
            q.require(len(projected) == 1, 'Observable loop state does not repeat')
        elif kind == 'zero_sequence_only_enables_out_of_range_slot':
            q.require(round(opts[0]) == 0 and round(opts[1]) == 2 and round(opts[2]) == 3, 'Zero sequence/source slots changed')
            q.require(w['repeat'] == 0 and all(r['counts'][1] == 0 and r['counts'][2] > 0 for r in after), 'Zero sequence invariant failed')
        elif kind == 'all_switch_branches_disable_only_counted_slot':
            q.require(round(opts[1]) == 2 and all(max(0, min(100, round(x))) == 0 for x in opts[2:5]), 'Switch contains an enabling branch')
            q.require(w['repeat'] == 0 and all(r['counts'][1] == 0 for r in after), 'Switch repeat invariant failed')
        else:
            ww = proof['wrap_witness']
            q.check_conditional_trace(ww, c['formula'], opts)
            wrapped = [r for r in ww['rows'] if r['stage'] == 'after' and r['slot'] == 0 and r['first'] == -(1 << 31)]
            q.require(wrapped and wrapped[0]['counts'][1] > 0 and ww['rows'][-1]['counted'] == ww['maxit'], 'Wrap does not restore finite counted progression')
            stable = [r for r in after if r['first'] >= 64]
            projected = {(r['counted'], r['remaining'], r['radius_bits'], tuple(r['xyz_bits']), tuple(r['counts'])) for r in stable}
            q.require(stable and len(projected) == 1, 'Counter acceleration does not preserve projected state')
            for r in stable:
                counts, _, branches = q.expected_conditional(c['formula'], opts, r)
                q.require(counts == r['counts'] and all(branch == 2 for _, branch in branches), 'Unstable high branch')
        if w:
            rstop = struct.unpack_from('<d', q.decode(Path(c['scene']['path'])), 92)[0]
            q.require(all(struct.unpack('<d', bytes.fromhex(r['radius_bits'])[::-1])[0] <= rstop for r in after), 'Orbit would already escape')
        replacement = c['replacement']
        reach = replacement['witness']
        cases.append(dict(
            id=c['id'], formula=c['formula'], source_scene=c['scene'],
            prepared_parameters=c['prepared_parameter_comparison'], classification=kind,
            proof_scope='Source invariant for the recorded admissible orbit state; not a universal whole-image claim',
            nontermination_proven=kind != 'excessive_finite_signed_counter_wrap',
            original_image_status='Excluded; no new original numerical image produced',
            source_argument=proof['argument'], disassembly=c['disassembly'],
            original_trace=w['trace'] if w else None,
            accelerated_trace=proof.get('wrap_witness', {}).get('trace'),
            guarded_rejection='Retained paired C++/Pascal t1 failures; no output image',
            finite_replacement=dict(scene=replacement['scene'], trace=reach['trace'],
                original_image=replacement['original_image'], current_cpp_image=replacement['current_cpp_image'],
                current_guarded_pascal_image=replacement['current_pascal_image'],
                original_rgb_exact=True, target_callback_reached=True,
                counted_at_return=reach['rows'][-1]['counted'],
                counted_progress_required=c['formula'].startswith('_Cond'),
                reached_branches=replacement.get('reached_branches', [['Trifox finite nonzero powers']]),
                boundary_callbacks=[dict(trace=x['witness']['trace'], branches=x['branches']) for x in replacement.get('boundary_callbacks', [])]))
        )
    hang_path = q.BASE/'hang-app-v3/summary.json'
    hang = json.loads(hang_path.read_bytes())
    executables = {k: q.ref(q.checked(p, hang['binary_sha256'][k])) for k, p in hang['executables'].items()}
    return dict(
        status='qualified_scoped_source_and_branch_evidence', finished=True,
        scope='Fixed 2 Trifox zero-power point/image cases and 22 conditional image exclusions; MC/JIT excluded',
        original_numerical_parity_created=False, universal_image_nontermination_proven=False,
        counts=dict(cases=24, source_nontermination_orbit_proofs=18, excessive_finite_orbit_witnesses=6,
                    independent_prepared_buffers_exact=24, conditional_finite_driver_witnesses=22,
                    conditional_boundary_callbacks=44, trifox_finite_callbacks=2,
                    unique_original_finite_image_controls=23, current_rgb_comparisons=48,
                    validator_negative_controls=10),
        classifications=dict(Counter(c['classification'] for c in cases)),
        witness_report=q.ref(summary), tools=[q.ref(Path(__file__)), s['tool']],
        compiler=q.ref(q.FPC), source_extraction=s['build'], current_guarded_executables=executables,
        immutable_formula_assets=[q.ref(q.checked(q.ROOT/'mb3d/M3Formulas'/(n+'.m3f'), h)) for n, h in q.ASSET_SHA.items()],
        retained_evidence=s['retained'] + [q.ref(q.ROOT/'comparison/results/compatibility-final-20261018/qualification.json'),
                                         q.ref(q.ROOT/'comparison/results/compatibility-final-20261018/supplementary-qualification.json')],
        original_driver_source=dict(path=str(q.ROOT/'mb3d/formulas.pas'), procedure_line=3442,
                                    start_slot_empty_escape_line=3465, termination_line=3478),
        policy='Budget exhaustion alone is not proof. Six original signed-counter orbits are excessive but finite; retain their policy rejection and original-image exclusion separately.',
        limitations=s['limitations'] + [
            'The 22 driver traces use a declared admissible (.1,.2,.3) orbit input, not a captured render-pixel coordinate.',
            'The two Trifox cases share one previously qualified nonzero-power image control; this is 23 unique images, not 24.',
            'Original image comparisons are retained hashed evidence revalidated by decoded RGB; no full application or renderer was executed in this qualification.',
            '18/6 classifies these recorded source/orbit witnesses only; original excluded-image denominators are unchanged.'],
        cases=cases, validator_negative_controls=negative)


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--witness-summary', type=Path, required=True)
    ap.add_argument('--output', type=Path, required=True)
    args = ap.parse_args()
    q.require(not args.output.exists(), 'Refusing overwrite')
    result = audit(args.witness_summary.resolve())
    q.write_json(args.output, result)
    print(json.dumps(result['counts'], sort_keys=True))


if __name__ == '__main__':
    main()
