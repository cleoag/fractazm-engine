"""Publication/resource metadata tests only; no renderer or qualification runs."""
import copy
import json
from pathlib import Path
import shutil
import tempfile
import unittest
from unittest.mock import patch

import prepare_native_scene_resources as resources
import report_gpu_acceptance as report
from qualify_native_metal_all import sha


class ResourcePublicationTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name).resolve()
        self.package = self.root / 'package'
        resources.prepare_profiles(self.package)
        metadata_path = Path('comparison/resources/phase1-selected/selection.json')
        metadata = json.loads((resources.ROOT / metadata_path).read_text())
        paths = [metadata_path, Path(metadata['contract']), Path('comparison/scene-queue.json')]
        paths += [Path(p['inventory']) for p in metadata['profiles'].values()]
        for relative in paths:
            target = self.root / relative;target.parent.mkdir(parents=True, exist_ok=True)
            shutil.copyfile(resources.ROOT / relative, target)
        self.binding = {'map_selection': self.ref(self.package / 'resources.json'),
                        'contract_sha256': metadata['contract_sha256'],
                        'scene_resources': json.loads((self.package / 'resources.json').read_text())['scene_resource_profiles'],
                        'source': {}, 'renderer': {}, 'library': {}}
        self.patch = patch.object(report, 'ROOT', self.root)
        self.patch.start();self.addCleanup(self.patch.stop)

    def ref(self, path):
        return {'path': str(path.relative_to(self.root)), 'sha256': sha(path)}

    def select_legacy(self, supplied=16, scenes=77, substitute=True, requested=True):
        data = {'installed_maps': supplied, 'scenes_with_available_maps': scenes,
                'missing_map_ids': [] if supplied == 16 else [1633],
                'selected_candidates': [{'number': 18, 'original_identity_claimed': False}],
                'substitutions': ([{'requested_id': 1633, 'selected_source_id': 3038,
                                    'status': 'USER_APPROVED_SUBSTITUTE', 'original_identity_claimed': False}]
                                  if substitute else [])}
        if requested:data['requested_maps'] = 16
        path = self.package / 'map-selection-v4.json';path.write_text(json.dumps(data))
        self.binding['map_selection'] = self.ref(path)
        return data

    def prepared_mutation(self, change):
        path = self.package / 'resources.json';data = json.loads(path.read_text());change(data)
        path.write_text(json.dumps(data));self.binding['map_selection'] = self.ref(path)

    def test_prepared_union_and_approved_identity_labels(self):
        summary = report.resource_summary(self.binding, 77)
        self.assertEqual(summary['status'], 'PINNED_PREPARED_PROFILES')
        self.assertEqual((summary['installed_maps'], summary['requested_maps'], summary['scenes_with_available_maps']), (16, 16, 77))
        self.assertEqual(summary['missing_map_ids'], [])
        self.assertFalse(summary['qualification'])
        self.assertFalse(summary['original_identity_claimed_for_selected_maps'])
        self.assertEqual(summary['substitutions'][0]['selected_source_id'], 3038)
        self.assertEqual(len(summary['selected_candidates']), 6)
        text = report.resource_description(summary)
        self.assertIn('16/16', text);self.assertIn('77/77', text)
        self.assertIn('а не подтверждёнными оригиналами', text)
        self.assertIn('1633 явно заменена картой 3038', text)

    def test_legacy_substitute_and_missing_map_formats(self):
        self.select_legacy()
        summary = report.resource_summary(self.binding, 77)
        self.assertEqual(summary['status'], 'PINNED_LEGACY_METADATA')
        self.assertIn('1633 явно заменена картой 3038', report.resource_description(summary))
        self.select_legacy(15, 76, False)
        summary = report.resource_summary(self.binding, 77)
        self.assertEqual(summary['installed_maps'], 15)
        self.assertEqual(summary['scenes_with_available_maps'], 76)
        self.assertEqual(summary['missing_map_ids'], [1633])
        self.assertIn('В выбранных профилях недоступны карты: 1633', report.resource_description(summary))
        self.assertNotIn('явно заменена', report.resource_description(summary))

    def test_legacy_unknown_denominator_not_fabricated(self):
        self.select_legacy(15, 76, False, requested=False)
        summary = report.resource_summary(self.binding, 77)
        self.assertIsNone(summary['requested_maps'])
        self.assertNotIn('15/16', report.resource_description(summary))

    def test_absent_and_unknown_metadata_are_explicitly_unverified(self):
        absent = copy.deepcopy(self.binding);absent.pop('map_selection')
        self.assertEqual(report.resource_summary(absent, 77)['status'], 'UNVERIFIED')
        path = self.package / 'unknown.json';path.write_text('{"schema":"unknown"}')
        unknown = dict(self.binding, map_selection=self.ref(path))
        summary = report.resource_summary(unknown, 77)
        self.assertIsNone(summary['installed_maps']);self.assertIsNone(summary['scenes_with_available_maps'])
        self.assertIn('не проверена', report.resource_description(summary))
        self.assertNotIn('15/16', report.resource_description(summary))

    def test_pinned_selection_and_resource_hash_changes_rejected(self):
        with patch.dict(self.binding['map_selection'], sha256='0' * 64):
            with self.assertRaises(ValueError):report.resource_summary(self.binding, 77)
        self.prepared_mutation(lambda d: d.update(selection_sha256='0' * 64))
        with self.assertRaises(ValueError):report.resource_summary(self.binding, 77)

    def test_mislabelled_profile_and_fabricated_availability_rejected(self):
        self.prepared_mutation(lambda d: d['profiles'].update(unknown=d['profiles']['available']))
        with self.assertRaisesRegex(ValueError, 'Unknown or missing'):report.resource_summary(self.binding, 77)
        self.prepared_mutation(lambda d: d['profiles'].pop('unknown'))
        self.prepared_mutation(lambda d: d.update(scenes_with_available_resources=76))
        with self.assertRaisesRegex(ValueError, 'availability count'):report.resource_summary(self.binding, 77)
        self.prepared_mutation(lambda d: d.update(scenes_with_available_resources=77))
        self.prepared_mutation(lambda d: d['profiles']['original']['files'].update(d['profiles']['available']['files']))
        with self.assertRaisesRegex(ValueError, 'differs from pinned'):report.resource_summary(self.binding, 77)

    def test_original_identity_and_contract_mislabeling_rejected(self):
        self.prepared_mutation(lambda d: d.update(original_identity_claimed_for_selected_maps=True))
        with self.assertRaisesRegex(ValueError, 'original selected-map identity'):
            report.resource_summary(self.binding, 77)
        self.prepared_mutation(lambda d: d.update(original_identity_claimed_for_selected_maps=False))
        with patch.dict(self.binding, contract_sha256='0' * 64):
            with self.assertRaisesRegex(ValueError, 'different acceptance contract'):
                report.resource_summary(self.binding, 77)
        legacy = self.select_legacy()
        legacy['substitutions'][0]['original_identity_claimed'] = True
        path = self.package / 'map-selection-v4.json';path.write_text(json.dumps(legacy))
        self.binding['map_selection'] = self.ref(path)
        with self.assertRaisesRegex(ValueError, 'mislabelled legacy substitute'):
            report.resource_summary(self.binding, 77)

    def test_bound_profile_controls_scene_availability(self):
        sid = next(k for k, v in self.binding['scene_resources'].items() if v == 'substitute1633')
        self.binding['scene_resources'][sid] = 'available'
        summary = report.resource_summary(self.binding, 77)
        self.assertEqual(summary['installed_maps'], 16)  # Exists in the separate prepared profile.
        self.assertEqual(summary['scenes_with_available_maps'], 76)
        self.assertEqual(summary['missing_map_ids'], [1633])
        self.assertEqual(summary['substitutions'], [])
        self.binding['scene_resources'][sid] = 'unknown'
        with self.assertRaises(ValueError):report.resource_summary(self.binding, 77)

    def publish(self, output_name, write_plan=True):
        (self.package / 'qualification.json').write_text(json.dumps({'status': 'OPEN', 'scene_outcomes': {},
            'qualified_gpu_scenes': 0, 'qualified_fp32_cpu_scenes': 0}))
        (self.package / 'evidence.json').write_text(json.dumps({'binding': self.binding, 'scenes': []}))
        plan = self.root / 'docs/NATIVE_ACCEPTANCE_PLAN.md';plan.parent.mkdir(exist_ok=True)
        plan.write_text('Manual context\n| Resource selection | Old | Conditions |\n| Full ordinary scenes | Old | Conditions |\n')
        before = plan.read_bytes();out = self.root / 'comparison/results' / output_name
        with patch.object(report.subprocess, 'check_output', return_value='synthetic-test-commit\n'):
            report.publish(self.package, output=out, write_plan=write_plan)
        return out, plan, before

    def test_publish_both_resource_schemas_html_json_and_plan(self):
        for mode in ('prepared', 'legacy'):
            if mode == 'legacy':self.select_legacy()
            out, plan, _ = self.publish(mode)
            data = json.loads((out / 'report.json').read_text())
            self.assertEqual(data['resources']['installed_maps'], 16)
            self.assertEqual(data['qualification']['qualified_gpu_scenes'], 0)
            self.assertIn('16/16', (out / 'index.html').read_text())
            self.assertIn('1633 явно заменена картой 3038', (out / 'index.html').read_text())
            self.assertIn('Done: 16/16 map IDs supplied, 77/77 scenes', plan.read_text())
            self.assertIn('Manual context', plan.read_text())

    def test_publish_unverified_and_no_plan_mode(self):
        self.binding.pop('map_selection')
        out, plan, _ = self.publish('unknown')
        self.assertIn('не проверена', (out / 'index.html').read_text())
        self.assertIn('UNVERIFIED: resource metadata', plan.read_text())
        self.assertNotIn('Шесть найденных', (out / 'index.html').read_text())
        out, plan, before = self.publish('no-plan', write_plan=False)
        self.assertEqual(plan.read_bytes(), before)


if __name__ == '__main__':
    unittest.main()
