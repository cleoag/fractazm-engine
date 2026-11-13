#!/usr/bin/env python3
"""Bounded old/new range-policy regression; never awards scene qualification.

Original scenes keep effects/settings. CPU fallback, GPU frames and missing
resources are distinct. New policy observations cannot prove the mathematical
cause of a range exit; separate full-frame attribution remains required.
"""
import argparse
import json
from pathlib import Path
from native_gpu_coverage import rgb_sha256, sha256
from run_m3parameter_gpu import invoke, work_record
ROOT = Path(__file__).resolve().parents[2]


def work(profile):
    return {k: v for k, v in work_record(profile['metal_execution']).items()
            if not k.startswith('fp32_range_gate_') and k not in
            ('pipeline_archive', 'pipeline_archive_hits', 'pipeline_archive_misses')}


def main():
    p = argparse.ArgumentParser(description=__doc__)
    for name in ('baseline', 'candidate', 'runtime', 'output'):
        p.add_argument('--' + name, required=True, type=Path)
    p.add_argument('--original', action='store_true')
    p.add_argument('--ranks', nargs='+', type=int, default=[25, 26, 30, 73])
    a = p.parse_args()
    assert set(a.ranks) <= {25, 26, 30, 73} and not a.output.exists()
    a.output = a.output.resolve(); a.output.mkdir(parents=True)
    builds = {k: getattr(a, k).resolve() for k in ('baseline', 'candidate')}
    identities = {str(b / name): sha256(b / name) for b in builds.values()
                  for name in ('mb3d_render', 'metal_native_scene.metallib')}
    rows = json.loads((ROOT / 'comparison/scene-queue.json').read_text())['rows']
    inventory = {r['path']: r for r in json.loads((ROOT / 'comparison/results/m3parameter-gpu-priority-20261024/inventory.json').read_text())['scenes']}
    runtime = a.runtime.resolve()
    resources = {str(f.relative_to(runtime)): sha256(f) for f in runtime.rglob('*') if f.is_file()}
    report = {'qualification': False, 'original_size': a.original, 'identities': identities,
              'resources': resources, 'results': []}

    def render(row, label, build, backend='metal'):
        folder = a.output / str(row['rank']) / label
        scene = ROOT / row['scene']
        assert sha256(scene) == inventory[row['scene']]['sha256']
        cmd = [build / 'mb3d_render', scene, '--backend', backend, '--precision', 'native',
               '--threads', '6', '--output', folder / 'image.png', '--profile-json', folder / 'profile.json']
        if backend == 'metal': cmd += ['--metal-library', build / 'metal_native_scene.metallib']
        if not a.original: cmd += ['--width', '160', '--height', '120']
        event = invoke(cmd, folder, runtime, timeout=300, evidence_root=ROOT)
        if event['exit_code'] or event['guard'] or event['frame_warning_emitted']:
            raise ValueError('Failed/slow frame; no further repeats: ' + str(folder))
        d = json.loads((folder / 'profile.json').read_text())
        assert d['status'] == 'complete' and d['precision'] == 'native' and d['backend_requested'] == backend
        assert [d['width'], d['height']] == (inventory[row['scene']]['settings']['size'] if a.original else [160, 120])
        expected = 'metal' if backend == 'metal' and row['rank'] == 26 else 'cpu'
        assert d['backend'] == expected
        assert d['backend_fallback'] == (backend == 'metal' and expected == 'cpu')
        if d['backend_fallback']: assert d['backend_fallback_code'] == 'fp32_range'
        assert not d['metal_execution']['faults'] and not d['metal_execution']['nonfinite']
        if backend == 'metal' and build == builds['candidate']:
            e = d['metal_execution']
            assert e['fp32_range_gate_stage'] in ('primary', 'geometry')
            assert e['fp32_range_gate_units'] == d['width'] * d['height'] and e['fp32_range_gate_percent'] == 1
            assert 0 <= e['fp32_range_gate_exceeded'] <= e['fp32_range_gate_units']
            assert (e['fp32_range_gate_exceeded'] * 100 > e['fp32_range_gate_units']) == d['backend_fallback']
        return folder, d

    for row in rows:
        if row['rank'] not in a.ranks: continue
        result = {'rank': row['rank'], 'scene': row['scene'], 'status': 'OPEN'}
        report['results'].append(result)
        try:
            repeats = [render(row, 'repeat-' + str(i), builds['candidate']) for i in range(3)]
            rgb = [rgb_sha256(f / 'image.png') for f, _ in repeats]
            assert len(set(rgb)) == 1 and all(work(d) == work(repeats[0][1]) for _, d in repeats)
            old, old_profile = render(row, 'baseline', builds['baseline'])
            assert rgb_sha256(old / 'image.png') == rgb[0] and work(old_profile) == work(repeats[0][1])
            direct_cpu_exact = None
            if repeats[0][1]['backend_fallback']:
                cpu, _ = render(row, 'direct-cpu', builds['candidate'], 'cpu')
                direct_cpu_exact = rgb_sha256(cpu / 'image.png') == rgb[0]
                assert direct_cpu_exact
            e = repeats[0][1]['metal_execution']
            result.update(status='PASS_ORIGINAL_REGRESSION' if a.original else 'PASS_PREVIEW_ONLY',
                          exact_repeats=3, old_new_rgb_work_exact=True, direct_cpu_rgb_exact=direct_cpu_exact,
                          effective_backend=repeats[0][1]['backend'], rgb_sha256=rgb[0],
                          policy={k: v for k, v in e.items() if k.startswith('fp32_range_gate_')},
                          renderer_seconds=[d['render_total_seconds'] for _, d in repeats],
                          selection_seconds=[d['backend_selection_seconds'] for _, d in repeats],
                          archive_states=[d['metal_execution']['pipeline_archive'] for _, d in repeats])
        except (AssertionError, OSError, ValueError, KeyError) as error:
            result['error'] = repr(error)
        (a.output / 'report.json').write_text(json.dumps(report, indent=2) + '\n')
        print(json.dumps(result), flush=True)
    assert all(sha256(Path(f)) == v for f, v in identities.items())
    assert resources == {str(f.relative_to(runtime)): sha256(f) for f in runtime.rglob('*') if f.is_file()}
    return int(any(r['status'] == 'OPEN' for r in report['results']))


if __name__ == '__main__':
    raise SystemExit(main())
