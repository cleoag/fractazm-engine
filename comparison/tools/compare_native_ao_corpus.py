#!/usr/bin/env python3
"""Replay full old/new GPU frames and controls; normalize only AO preparation.

This is ownership-change evidence, not a new scene or performance qualification.
Incomplete candidate coverage remains OPEN. No files in either input bundle
are changed and no rendering is performed.
"""
import argparse
import sys
from pathlib import Path

from native_ao_chunk_evidence import normalize


def exact_pair(old, new, size, threads):
    work, physical = normalize(new['work'], size, threads)
    if old['image'] != new['image']:
        raise ValueError('Old/new RGB differs')
    if old['work'] != work:
        raise ValueError('Old/new logical work differs')
    return physical


def main():
    p = argparse.ArgumentParser(description=__doc__)
    for key in ('repository', 'baseline', 'candidate', 'contract', 'output'):
        p.add_argument('--' + key, type=Path, required=True)
    a = p.parse_args()
    root = a.repository.resolve()
    sys.path.insert(0, str(root / 'comparison/tools'))
    from qualify_gpu_parallel_pipeline import scope
    from gpu_pipeline_binding import core_context, for_scene
    from gpu_pipeline_frames import frame, scene_frames
    from qualify_native_metal_all import artifact, require, unique
    from run_m3parameter_gpu import read, sha, write
    from prepare_gpu_parallel_bundle import ref
    c, formulas, scenes = scope(a.contract.resolve())
    bundles = [read(path / 'evidence.json') for path in (a.baseline, a.candidate)]
    contexts = [core_context(root, ev['binding'], c, formulas, scenes, root) for ev in bundles]
    require(contexts[0]['threads'] == contexts[1]['threads'], 'Different thread count')
    rows = [unique(ev['scenes'], lambda row: row['id'], 'scene') for ev in bundles]
    expected = {sid for sid, row in rows[0].items() if row['qualification'] == 'QUALIFIED_GPU'}
    require(expected, 'Empty baseline GPU inventory')
    report = {'qualification': False, 'status': 'OPEN', 'expected': len(expected),
              'checker': ref(Path(__file__)), 'normalizer': ref(Path(normalize.__code__.co_filename)),
              'bindings': [ev['binding'] for ev in bundles], 'rows': [], 'missing': [],
              'timing_claim': 'None; these are independently collected acceptance frames'}
    for sid in sorted(expected):
        if sid not in rows[1] or rows[1][sid]['qualification'] != 'QUALIFIED_GPU':
            report['missing'].append(sid)
            continue
        scene = scenes[sid]
        ctxs = [for_scene(ctx, sid) for ctx in contexts]
        require(ctxs[0]['runtime_files'] == ctxs[1]['runtime_files'], 'Different resources')
        proofs = [read(artifact(root, row[sid]['effect_observations'])) for row in rows]
        ids = [scene_frames(root, row[sid], scene, ctx, proof)
               for row, ctx, proof in zip(rows, ctxs, proofs)]
        require(not ids[0] & ids[1], 'Old/new process reuse')
        full = [[frame(root, job, scene, ctx) for job in row[sid]['frames']]
                for row, ctx in zip(rows, ctxs)]
        physical = [exact_pair(full[0][0], new, scene['settings']['size'], ctxs[0]['threads'])
                    for new in full[1]]
        controls = [unique(proof['controls'], lambda row: row['stage'], 'control') for proof in proofs]
        require(set(controls[0]) == set(controls[1]), 'Different effect controls')
        control_physical = {}
        for stage in controls[0]:
            pair = [frame(root, control[stage]['off'], scene, ctx, stage)
                    for control, ctx in zip(controls, ctxs)]
            control_physical[stage] = exact_pair(*pair, scene['settings']['size'], ctxs[0]['threads'])
        report['rows'].append({'id': sid, 'scene': scene['path'], 'status': 'PASS_EXACT_RGB_WORK',
                               'frames': [row[sid]['frames'] for row in rows],
                               'effects': [row[sid]['effect_observations'] for row in rows],
                               'physical_preparation': physical, 'control_preparation': control_physical})
        print('EXACT', len(report['rows']), '/', len(expected), scene['path'], flush=True)
    report['checked'] = len(report['rows'])
    report['status'] = 'PASS_SCOPED_OLD_NEW' if not report['missing'] else 'OPEN'
    # The live candidate collector can append cases, but already captured inputs
    # and both selected implementations must remain immutable during replay.
    for ctx in contexts:
        require(sha(ctx['library_path']) == ctx['library']['sha256'], 'Library changed during replay')
    for item in (report['checker'], report['normalizer']):
        artifact(root, item)
    write(a.output, report)
    print(report['status'], report['checked'], '/', report['expected'], flush=True)
    return 0 if not report['missing'] else 2


if __name__ == '__main__':
    raise SystemExit(main())
