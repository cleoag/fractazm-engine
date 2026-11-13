"""Replay must preserve failures/pending work and reject tampered evidence."""
import json
import unittest
from replay_parity import selected_case_ids


class CaseSelectionTests(unittest.TestCase):
    def test_exact_subset(self):
        self.assertEqual(selected_case_ids(['b'], [{'id':'a'},{'id':'b'}]), {'b'})

    def test_bad_selection_is_not_silently_skipped(self):
        for bad in ([], ['unknown'], ['a','a'], [1], {'a':True}):
            with self.subTest(bad=bad), self.assertRaises(ValueError):
                selected_case_ids(bad, [{'id':'a'}])
from pathlib import Path
import shutil
import tempfile
from unittest.mock import patch
from PIL import Image
import replay_parity
from external_validation import digest, write_json


class ReplayIntegrityTests(unittest.TestCase):
    def setUp(self):
        temp = tempfile.TemporaryDirectory()
        self.addCleanup(temp.cleanup)
        self.root = Path(temp.name)
        self.exe = self.root / 'renderer.exe'
        self.exe.write_bytes(b'frozen-renderer')
        self.scene = self.root / 'first.m3p'
        self.scene.write_bytes(b'frozen-scene')
        self.manifest = self.root / 'manifest.json'
        self.cases = [dict(id=name, scene=str(self.scene), sha256=digest(self.scene.read_bytes()), target_formulas=[name])
                      for name in ('first', 'second')]
        write_json(self.manifest, dict(cases=self.cases))
        self.reference = self.root / 'reference' / 'summary.json'
        self.png = self.reference.parent / 'pascal' / 'first.png'
        self.png.parent.mkdir(parents=True)
        Image.new('RGB', (16, 16), (80, 120, 200)).save(self.png)
        self.old = dict(manifest_sha256=digest(self.manifest.read_bytes()), batches=[],
                        provenance={}, pascal_sha256='frozen-reference', finished=False,
                        results=[dict(scene='first', scene_sha256=self.cases[0]['sha256'],
                                      pascal_image_sha256=digest(self.png.read_bytes()), status='fail')])
        write_json(self.reference, self.old)
        self.output = self.root / 'replay'

    def run_replay(self, prefix=True, resume=False, formulas=(), reference_argument=None, checkpoint_every=1):
        argv = ['replay', '--reference', str(reference_argument or self.reference), '--cpp-exe', str(self.exe),
                '--manifest', str(self.manifest), '--output', str(self.output),
                '--checkpoint-every', str(checkpoint_every)]
        if prefix: argv += ['--snapshot-prefix']
        if resume: argv += ['--resume']
        for formula in formulas: argv += ['--formula', formula]
        def render(exe, scene, target, *args):
            target.parent.mkdir(parents=True, exist_ok=True)
            shutil.copyfile(self.png, target)
            Path(str(target)+'.precision.json').write_text(json.dumps(dict(
                schema='mb3d.cpu-stage-profile.v1',precision='compatible',status='complete',
                exit_code=0,render_status='completed')))
            return 0, 1, ''
        with patch('sys.argv', argv), patch.object(replay_parity, 'source_provenance', return_value={}), \
                patch.object(replay_parity, 'run_render', side_effect=render) as run:
            result = replay_parity.main()
        return result, run.call_count, json.loads((self.output / 'summary.json').read_text())

    def test_prefix_keeps_pending_and_freezes_exact_reference(self):
        result, count, report = self.run_replay()
        self.assertEqual((result, count), (1, 1))
        self.assertEqual(report['pending_reference_cases'], ['second'])
        self.assertFalse(report['reference_finished'])
        self.assertEqual(report['counts'], {'pass': 1})
        self.assertEqual((self.output / 'reference-checkpoint.json').read_bytes(), self.reference.read_bytes())

    def test_unfinished_reference_requires_explicit_prefix(self):
        with self.assertRaisesRegex(ValueError, 'Unfinished batch'):
            self.run_replay(prefix=False)

    def test_reordered_reference_rejected(self):
        self.old['results'][0]['scene'] = 'second'
        write_json(self.reference, self.old)
        with self.assertRaisesRegex(ValueError, 'prefix differs'):
            self.run_replay()

    def test_changed_image_is_error_before_render(self):
        Image.new('RGB', (16, 16), (0, 0, 0)).save(self.png)
        result, count, report = self.run_replay()
        self.assertEqual((result, count), (1, 0))
        self.assertEqual(report['counts'], {'error': 1})
        self.assertIn('image hash changed', report['results'][0]['error'])

    def test_changed_scene_is_error_before_render(self):
        self.scene.write_bytes(b'changed-scene')
        result, count, report = self.run_replay()
        self.assertEqual((result, count), (1, 0))
        self.assertIn('Scene changed', report['results'][0]['error'])

    def test_reference_error_without_image_is_retained(self):
        self.png.unlink()
        self.old['results'][0].update(status='error', error='Pascal timeout')
        write_json(self.reference, self.old)
        result, count, report = self.run_replay()
        self.assertEqual((result, count), (1, 0))
        self.assertEqual(len(report['results']), 1)
        self.assertIn('Pascal timeout', report['results'][0]['error'])

    def test_resume_uses_frozen_prefix_when_live_reference_advances(self):
        self.run_replay()
        self.old['results'].append(dict(self.old['results'][0], scene='second'))
        write_json(self.reference, self.old)
        result, count, report = self.run_replay(resume=True)
        self.assertEqual((result, count), (1, 0))
        self.assertEqual(report['pending_reference_cases'], ['second'])

    def test_resume_only_renders_uncommitted_cases(self):
        self.old['results'].append(dict(self.old['results'][0], scene='second'))
        shutil.copyfile(self.png, self.png.with_name('second.png'))
        write_json(self.reference, self.old)
        _, _, report = self.run_replay()
        report['results'] = report['results'][:1]
        report['counts'] = {'pass': 1}
        report['finished'] = False
        write_json(self.output/'summary.json', report)
        result, count, report = self.run_replay(resume=True)
        self.assertEqual((result, count), (0, 1))
        self.assertEqual([r['scene'] for r in report['results']], ['first', 'second'])
        self.assertEqual(report['counts'], {'pass': 2})

    def test_resume_rejects_binary_change(self):
        self.run_replay()
        self.exe.write_bytes(b'changed')
        with self.assertRaisesRegex(ValueError, 'executable changed'):
            self.run_replay(resume=True)

    def test_resume_rejects_changed_cpp_image(self):
        _, _, report = self.run_replay()
        path = self.output/'cpp'/report['results'][0]['suite']/'first.png'
        path.write_bytes(b'changed')
        with self.assertRaisesRegex(ValueError, 'cpp image changed'):
            self.run_replay(resume=True)

    def test_resume_rejects_changed_frozen_reference(self):
        self.run_replay()
        write_json(self.output/'reference-checkpoint.json', dict(self.old, finished=True))
        with self.assertRaisesRegex(ValueError, 'checkpoint changed'):
            self.run_replay(resume=True)

    def test_selection_excludes_unrequested_cases_but_records_scope(self):
        result, count, report = self.run_replay(formulas=['first'])
        self.assertEqual((result, count), (0, 1))
        self.assertEqual(report['selected_case_count'], 1)
        self.assertEqual(report['unselected_case_count'], 1)
        self.assertEqual(report['pending_reference_cases'], [])
        self.assertIn('not full-manifest', report['scope'])

    def test_selection_preserves_missing_reference_cases(self):
        result, count, report = self.run_replay(formulas=['second'])
        self.assertEqual((result, count), (1, 0))
        self.assertEqual(report['pending_reference_cases'], ['second'])

    def test_unknown_selection_cannot_pass_as_empty_suite(self):
        with self.assertRaisesRegex(ValueError, 'Unknown requested formulas'):
            self.run_replay(formulas=['missing'])

    def test_selected_replay_cannot_resume_as_full_prefix(self):
        self.run_replay(formulas=['first'])
        with self.assertRaisesRegex(ValueError, 'cannot resume as a full prefix'):
            self.run_replay(resume=True)

    def test_resume_accepts_one_nested_batch_summary(self):
        directory = self.root/'nested-reference'
        self.reference = directory/'one'/'summary.json'
        self.png = directory/'one'/'pascal'/'first.png'
        self.png.parent.mkdir(parents=True)
        Image.new('RGB', (16,16), (80,120,200)).save(self.png)
        write_json(self.reference, self.old)
        self.run_replay(reference_argument=directory)
        result, count, report = self.run_replay(resume=True, reference_argument=directory)
        self.assertEqual((result, count), (1, 0))
        self.assertEqual(report['pending_reference_cases'], ['second'])

    def test_batched_and_immediate_checkpoints_have_same_measurements(self):
        self.old['results'].append(dict(self.old['results'][0], scene='second'))
        shutil.copyfile(self.png, self.png.with_name('second.png'))
        write_json(self.reference,self.old)
        _, _, immediate = self.run_replay()
        self.output = self.root/'batched'
        _, _, batched = self.run_replay(checkpoint_every=32)
        self.assertEqual(immediate['results'],batched['results'])
        self.assertEqual(immediate['counts'],batched['counts'])
        self.assertTrue(batched['finished'])
        self.assertEqual(len(batched['provenance_records']),1)
        self.assertNotIn('reference_provenance',batched['results'][0])

    def test_resume_migrates_legacy_inline_manifests(self):
        _, _, report = self.run_replay()
        for item in report['results']:
            item['reference_provenance'] = report['provenance_records'][item.pop('reference_provenance_sha256')]
        del report['provenance_records']
        write_json(self.output/'summary.json',report)
        _, count, migrated = self.run_replay(resume=True,checkpoint_every=32)
        self.assertEqual(count,0)
        self.assertEqual(len(migrated['provenance_records']),1)

    def test_resume_rejects_modified_compact_manifest(self):
        _, _, report = self.run_replay()
        key = report['results'][0]['reference_provenance_sha256']
        report['provenance_records'][key] = {'tampered':True}
        write_json(self.output/'summary.json',report)
        with self.assertRaisesRegex(ValueError,'Changed provenance'):
            self.run_replay(resume=True)

    def test_resume_rejects_missing_compact_manifest(self):
        _, _, report = self.run_replay()
        report['provenance_records'] = {}
        write_json(self.output/'summary.json',report)
        with self.assertRaisesRegex(ValueError,'Missing provenance'):
            self.run_replay(resume=True)

    def test_resume_rejects_replaced_manifest_with_valid_hash(self):
        _, _, report = self.run_replay()
        from external_validation import compact_image_evidence
        records = {}
        item = compact_image_evidence({'reference_provenance':{'replacement':True}},records)
        report['provenance_records'] = records
        report['results'][0].update(item)
        write_json(self.output/'summary.json',report)
        with self.assertRaisesRegex(ValueError,'reference provenance changed'):
            self.run_replay(resume=True)


if __name__ == '__main__':
    unittest.main()
