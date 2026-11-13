"""Synthetic admission/aggregation tests: no renderer, Metal or VM runs."""
import copy
import contextlib
import io
import json
from pathlib import Path
import tempfile
import unittest
from unittest import mock

from PIL import Image
import benchmark_backend_reflections as bench


class BackendReflectionBenchmarkTests(unittest.TestCase):
    def setUp(self):
        temporary = tempfile.TemporaryDirectory()
        self.addCleanup(temporary.cleanup)
        self.root = Path(temporary.name)
        self.settings = dict(width=2, height=2, threads=2, warmups=1, repeats=3)
        self.scene, self.library, self.runtime = (self.root / n for n in ('scene.m3p', 'native.metallib', 'runtime'))
        self.scene.write_bytes(b'opaque scene')
        self.library.write_bytes(b'native FP32 library')
        self.runtime.mkdir()
        self.directories = {b: self.root / b for b in bench.BACKENDS}
        for path in self.directories.values():
            path.mkdir()

    def selection(self):
        return json.loads((bench.ROOT / 'comparison/benchmark_scenes/effects-v1.json').read_text())

    def test_selection_schema_types_and_duplicate_control_ids_rejected(self):
        for mutation in ('schema', 'count', 'rows', 'row', 'duplicate', 'control_duplicate'):
            value = self.selection()
            if mutation == 'schema':
                value['schema'] = 'wrong'
            elif mutation == 'count':
                value['expected_count'] = True
            elif mutation == 'rows':
                value['scenes'] = {}
            elif mutation == 'row':
                value['scenes'][0] = None
            elif mutation == 'duplicate':
                value['scenes'][1]['id'] = value['scenes'][0]['id']
            else:
                row = next(s for s in value['scenes'] if s['id'] == 'integerpower8-reflections')
                row['effect_control']['id'] = bench.ORDINARY[0]
            with self.subTest(mutation=mutation), self.assertRaises(ValueError):
                bench.selected_inputs(value)
        self.assertEqual(len(bench.selected_inputs(self.selection())[3]), 5)

    def test_malicious_control_id_rejected_before_output_or_outside_write(self):
        output, gallery = self.root / 'new-output', self.root / 'new-gallery'
        victim = self.root / 'victim.m3p'
        victim.write_bytes(b'outside file must survive')
        for bad_id in ('../../victim', '/absolute/path', r'..\victim', '', None, ['bad']):
            selection = self.selection()
            row = next(s for s in selection['scenes'] if s['id'] == 'integerpower8-reflections')
            row['effect_control']['id'] = bad_id
            manifest = self.root / 'selection.json'
            manifest.write_text(json.dumps(selection))
            args = []
            for key, value in (('driver', self.root / 'unused-driver'), ('renderer', self.root / 'unused-renderer'),
                               ('native-library', self.library), ('prior-gallery', self.root),
                               ('prior-reflection-gallery', self.root), ('output', output), ('gallery', gallery),
                               ('selection', manifest)):
                args += ['--' + key, str(value)]
            error = io.StringIO()
            with mock.patch.object(bench, 'quiet_host'), mock.patch.object(bench.common, 'check_disk', return_value=100 * 1024**3), \
                 mock.patch.object(bench, 'binary_identity') as binary, contextlib.redirect_stderr(error):
                self.assertEqual(bench.main(args), 1)
                binary.assert_not_called()
            self.assertIn('Unsafe scene ID', error.getvalue())
            self.assertFalse(output.exists())
            self.assertFalse(gallery.exists())
            self.assertEqual(victim.read_bytes(), b'outside file must survive')

    def test_input_destination_rejects_traversal_and_symlink_escape(self):
        output = self.root / 'destination'
        output.mkdir()
        outside = self.root / 'outside'
        outside.mkdir()
        with self.assertRaises(ValueError):
            bench.input_destination(output, '../../victim')
        (output / 'inputs').symlink_to(outside, target_is_directory=True)
        with self.assertRaises(ValueError):
            bench.input_destination(output, 'safe-id')

    def activity(self, enabled):
        result = dict(enabled=enabled, max_command_seconds=.001 if enabled else 0,
                      **{k: 0 for k in bench.COUNTS})
        if enabled:
            result.update(active_pixels=2, rays=3, hits=2, misses=1, changed_pixels=1, max_bounces=2,
                          ray_steps=15, primary_skipped=0, ao_rays=6, ao_steps=8, shadow_rays=3, shadow_steps=5)
        return result

    def report(self, backend, reflected=True):
        frames = []
        for phase, index in [('cold', 0), ('warmup', 0), *[('warm', i) for i in range(3)]]:
            name = 'cold.png' if phase == 'cold' else f'{phase}-{index:03d}.png'
            Image.frombytes('RGB', (2, 2), bytes(range(12))).save(self.directories[backend] / name)
            stages = {k: {'state': 'not_run', 'seconds': 0} for k in bench.STAGES}
            for k in (*bench.REQUIRED, 'final_surface', 'save', *(("load",) if phase == 'cold' else ()),
                      *(("reflections",) if reflected else ())):
                stages[k] = dict(state='completed', seconds=.001)
            frames.append(dict(phase=phase, index=index, image=name, state='completed', error='', width=2, height=2,
                threads=0 if backend == 'metal' else 2, precision='native', exact_rgb_matches_cold=True,
                backend=dict(requested=backend, effective=backend, fallback=False, fallback_code='', fallback_reason='',
                             device='Apple synthetic' if backend == 'metal' else '',
                             library=str(self.library) if backend == 'metal' else '', selection_seconds=.001,
                             reflections=self.activity(backend == 'metal' and reflected)),
                load_ms=1 if phase == 'cold' else 0, preparation_ms=1, render_ms=10, readback_ms=1,
                render_and_readback_ms=11, png_encode_write_ms=2, end_to_end_ms=14.2 if phase == 'cold' else 13,
                cold_total_ms=12.1, cold_end_to_end_ms=14.2, render_profile_total_ms=10, stages=stages))
        features = dict(load_success=True, effective_bCalc3D=1, MCoptions=0, bCalcSRautomatic=1 if reflected else 0,
                        SRamount=.800000011920929, SRreflectioncount=2, bCalculateHardShadow=7,
                        bCalcAmbShadowAutomatic=21, bNormalsOnDE=1,
                        formulas=[dict(slot=i, id=i, name='internal', iterations=1) for i in range(6)])
        return dict(schema=bench.DRIVER_SCHEMA, backend=backend, status='COMPLETE', architecture='arm64', error='',
                    settings=dict(self.settings), scene=str(self.scene), resource_root=str(self.runtime),
                    execution=dict(deterministic=True, profile_timing=True, work_control='none', precision='native'),
                    loaded_features=features, setup_ms=dict(load=1, ordinary_input_check=.1), cold=frames[0],
                    warmups=frames[1:2], samples=frames[2:])

    def validate(self, report, backend='metal', reflected=True):
        return bench.validate_report(report, self.directories[backend], backend, self.library,
                                     self.settings, self.scene, self.runtime, reflected)

    def test_both_backends_and_disabled_control_admitted(self):
        for backend in bench.BACKENDS:
            for reflected in (False, True):
                with self.subTest(backend=backend, reflected=reflected):
                    rgb, images = self.validate(self.report(backend, reflected), backend, reflected)
                    self.assertEqual(len(rgb), 12)
                    self.assertEqual(len(images), 5)

    def test_schema_precision_backend_and_fallback_rejected(self):
        for mutation in ('schema', 'status', 'backend', 'architecture', 'precision', 'fallback', 'effective', 'library'):
            report = self.report('metal')
            if mutation in ('schema', 'status', 'backend', 'architecture'):
                report[mutation] = 'wrong'
            elif mutation == 'precision':
                report['execution']['precision'] = 'compatible'
            else:
                report['cold']['backend'][mutation] = True if mutation == 'fallback' else 'wrong'
            with self.subTest(mutation=mutation), self.assertRaises(ValueError):
                self.validate(report)

    def test_frame_precision_workers_and_inputs_rejected(self):
        for mutation in ('precision', 'threads', 'scene', 'resource_root', 'width', 'phase', 'index', 'image'):
            report = self.report('metal')
            if mutation in ('scene', 'resource_root'):
                report[mutation] = 'wrong'
            else:
                report['samples'][0][mutation] = -1
            with self.subTest(mutation=mutation), self.assertRaises(ValueError):
                self.validate(report)

    def cli_profile(self, backend):
        raw = self.report(backend)
        frame = raw['cold']
        binding = frame['backend']
        return dict(schema='mb3d.render-stage-profile.v1' if backend == 'metal' else 'mb3d.cpu-stage-profile.v1',
                    status='complete', render_status='completed', exit_code=0, error='', precision='native',
                    numeric_profile='native_metal_fp32' if backend == 'metal' else 'native_cpu_fp64',
                    backend_requested=backend, backend=backend, backend_fallback=False,
                    backend_fallback_code='', backend_fallback_reason='', metal_device=binding['device'],
                    metal_library=binding['library'], backend_selection_seconds=binding['selection_seconds'],
                    metal_reflections=binding['reflections'], width=2, height=2,
                    workers=frame['threads'], threads_requested=2, stages=frame['stages'])

    def test_cli_control_requires_native_real_backend_and_activity(self):
        for backend in bench.BACKENDS:
            raw = self.cli_profile(backend)
            bench.validate_cli_profile(raw, backend, self.library, True, self.settings)
            raw['backend_fallback'] = True
            with self.assertRaises(ValueError):
                bench.validate_cli_profile(raw, backend, self.library, True, self.settings)
        raw = self.cli_profile('metal')
        raw['backend_requested'] = 'auto'
        bench.validate_cli_profile(raw, 'metal', self.library, True, self.settings, 'auto')
        raw['metal_reflections']['faults'] = 1
        with self.assertRaises(ValueError):
            bench.validate_cli_profile(raw, 'metal', self.library, True, self.settings, 'auto')

    def test_required_stages_missing_or_interrupted_rejected(self):
        for stage in (*bench.REQUIRED, 'reflections', 'final_surface', 'save'):
            for mutation in ('missing', 'not_run', 'interrupted', 'nan'):
                report = self.report('metal')
                if mutation == 'missing':
                    del report['samples'][0]['stages'][stage]
                elif mutation == 'nan':
                    report['samples'][0]['stages'][stage]['seconds'] = float('nan')
                else:
                    report['samples'][0]['stages'][stage]['state'] = mutation
                with self.subTest(stage=stage, mutation=mutation), self.assertRaises(ValueError):
                    self.validate(report)

    def test_faults_and_missing_secondary_work_rejected(self):
        for mutation in ('faults', 'rays', 'changed_pixels', 'hits', 'max_bounces', 'ao_rays', 'shadow_steps', 'ray_steps'):
            report = self.report('metal')
            report['cold']['backend']['reflections'][mutation] = 1 if mutation == 'faults' else 0
            with self.subTest(mutation=mutation), self.assertRaises(ValueError):
                self.validate(report)

    def test_repeat_activity_changes_rejected_but_clock_changes_allowed(self):
        report = self.report('metal')
        report['samples'][0]['backend']['reflections']['max_command_seconds'] = .002
        self.validate(report)
        report['samples'][0]['backend']['reflections']['ray_steps'] += 1
        with self.assertRaises(ValueError):
            self.validate(report)

    def test_missing_cold_warmup_and_repeat_rejected(self):
        for mutation in ('cold', 'warmups', 'samples'):
            report = self.report('cpu')
            report[mutation] = None if mutation == 'cold' else []
            with self.subTest(mutation=mutation), self.assertRaises(ValueError):
                self.validate(report, 'cpu')

    def test_timing_must_include_full_api_and_png_and_load(self):
        for mutation in ('render_ms', 'png_encode_write_ms', 'cold_total_ms', 'cold_end_to_end_ms', 'load_ms'):
            report = self.report('cpu')
            report['cold'][mutation] = 0
            with self.subTest(mutation=mutation), self.assertRaises(ValueError):
                self.validate(report, 'cpu')
        report = self.report('cpu')
        report['cold']['cold_end_to_end_ms'] += .05
        report['cold']['end_to_end_ms'] += .05
        self.validate(report, 'cpu')  # real interval clocks may include a small save-start gap

    def test_repeated_rgb_bytes_checked_independently_of_claim(self):
        report = self.report('metal')
        Image.frombytes('RGB', (2, 2), bytes(reversed(range(12)))).save(self.directories['metal'] / 'warm-001.png')
        with self.assertRaises(ValueError):
            self.validate(report)

    def test_cross_backend_differences_allowed_but_process_repeats_exact(self):
        references = {}
        bench.admit_reference(references, 'cpu', b'abc')
        bench.admit_reference(references, 'metal', b'def')
        bench.admit_reference(references, 'metal', b'def')
        with self.assertRaises(ValueError):
            bench.admit_reference(references, 'metal', b'deg')
        self.assertGreater(bench.common.rgb_difference(references['cpu'], references['metal'])['different_pixels'], 0)

    def test_warm_summary_is_median_of_process_medians(self):
        processes = []
        # Unequal sample lengths deliberately distinguish aggregation formulas.
        for values in ([1, 2, 3], [10, 20, 30, 40, 50], [100, 200, 300]):
            report = self.report('cpu')
            report['samples'] = [copy.deepcopy(report['samples'][0]) for _ in values]
            for frame, value in zip(report['samples'], values):
                for key in bench.TIMINGS:
                    frame[key] = value
            processes.append(dict(report_data=report, process=dict(wall_seconds=1, cpu_seconds=2, peak_rss_bytes=100)))
        result = bench.summarize_processes(processes)
        self.assertEqual(result['warm']['render_ms']['values'], [2, 30, 200])
        self.assertEqual(result['warm']['render_ms']['median'], 30)
        self.assertEqual(bench.backend_order(0), ('cpu', 'metal'))
        self.assertEqual(bench.backend_order(1), ('metal', 'cpu'))

    def test_deferred_and_wrong_effect_scene_rejected(self):
        for mutation in ('MCoptions', 'bCalcSRautomatic', 'bCalculateHardShadow', 'SRreflectioncount', 'formula'):
            report = self.report('metal')
            if mutation == 'formula':
                report['loaded_features']['formulas'][0]['name'] = 'JIT deferred'
            else:
                report['loaded_features'][mutation] = 3 if mutation in ('MCoptions', 'bCalcSRautomatic') else 0
            with self.subTest(mutation=mutation), self.assertRaises(ValueError):
                self.validate(report)

    def test_disabled_reflections_cannot_hide_work(self):
        report = self.report('metal', False)
        report['cold']['backend']['reflections']['rays'] = 1
        with self.assertRaises(ValueError):
            self.validate(report, reflected=False)

    def test_retention_manifest_detects_changed_missing_unlisted_files(self):
        path = self.root / 'retained'
        path.mkdir()
        (path / 'frame.png').write_bytes(b'frozen png')
        record = dict(file='frame.png', sha256=bench.sha(path / 'frame.png'), bytes=(path / 'frame.png').stat().st_size)
        (path / 'manifest.json').write_text(json.dumps(dict(files=[record])))
        self.assertEqual(bench.verify_manifest(path), 1)
        (path / 'frame.png').write_bytes(b'changed png')
        with self.assertRaises(ValueError):
            bench.verify_manifest(path)
        (path / 'frame.png').write_bytes(b'frozen png')
        (path / 'unlisted.txt').write_text('hidden')
        with self.assertRaises(ValueError):
            bench.verify_manifest(path)
        (path / 'unlisted.txt').unlink(); (path / 'frame.png').unlink()
        with self.assertRaises(ValueError):
            bench.verify_manifest(path)

    def test_nested_source_manifest_is_retained_and_verified(self):
        path = self.root / 'nested-manifests'
        nested = path / 'evidence/source/tests/manifest.json'
        nested.parent.mkdir(parents=True)
        nested.write_text('{"fixture":"manifest"}')
        record = dict(file=str(nested.relative_to(path)), sha256=bench.sha(nested), bytes=nested.stat().st_size)
        (path / 'manifest.json').write_text(json.dumps(dict(files=[record])))
        self.assertEqual(bench.verify_manifest(path), 1)

    def test_export_rejects_admitted_image_tamper_before_new_manifest(self):
        output = self.root / 'evidence-output'
        output.mkdir()
        image = output / 'admitted.png'
        Image.frombytes('RGB', (2, 2), bytes(range(12))).save(image)
        identity = bench.image_details(image, 2, 2)
        report = dict(source_identity=dict(files_sha256={}), identities={}, build_evidence=[], scene_inputs=[],
                      controls=[], backends={}, prior_report_files=[],
                      prior_references=[dict(frozen_image='admitted.png', **identity)])
        Image.frombytes('RGB', (2, 2), bytes(reversed(range(12)))).save(image)
        with mock.patch.object(bench.common, 'check_disk', return_value=100 * 1024**3), self.assertRaises(ValueError):
            bench.export_gallery(report, output, self.root / 'tampered-gallery')

    def test_gallery_rehashes_source_and_keeps_all_frames(self):
        output, gallery = self.root / 'output', self.root / 'gallery'
        source = output / 'evidence/source/src/example.cpp'
        source.parent.mkdir(parents=True)
        source.write_text('measured source')
        (output / 'cold.png').write_bytes(b'cold output')
        report = dict(source_identity=dict(files_sha256={'src/example.cpp': bench.sha(source)}),
                      identities={}, build_evidence=[], scene_inputs=[], controls=[], backends={},
                      prior_references=[], prior_report_files=[])
        with mock.patch.object(bench.common, 'check_disk', return_value=100 * 1024**3):
            bench.export_gallery(report, output, gallery)
        self.assertEqual(bench.verify_manifest(gallery), 2)
        self.assertEqual((gallery / 'cold.png').read_bytes(), b'cold output')
        source.write_text('modified source')
        with mock.patch.object(bench.common, 'check_disk', return_value=100 * 1024**3), self.assertRaises(ValueError):
            bench.export_gallery(report, output, self.root / 'gallery-bad')


if __name__ == '__main__':
    unittest.main()
