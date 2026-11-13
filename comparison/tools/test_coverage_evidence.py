import json
import unittest
from report_external_coverage import compact_image_evidence, latest_image_cases
from external_validation import digest


class CoverageEvidenceTests(unittest.TestCase):
    def test_challenge_totals_exclude_downloaded_and_extra_hybrids(self):
        result = latest_image_cases({'phase': [
            {'scene': 'challenge', 'scene_sha256': 'a', 'status': 'pass'},
            {'scene': 'downloaded', 'scene_sha256': 'b', 'status': 'pass'},
            {'scene': 'hybrid', 'scene_sha256': 'c', 'status': 'pass'}]}, {'challenge': 'a'})
        self.assertEqual(list(result), ['challenge'])

    def test_latest_failure_supersedes_only_the_same_scene(self):
        phases = {'old': [{'scene': 'early', 'scene_sha256': 'a', 'status': 'pass'}],
                  'new': [{'scene': 'early', 'scene_sha256': 'a', 'status': 'fail'},
                          {'scene': 'once', 'scene_sha256': 'b', 'status': 'pass'}]}
        result = latest_image_cases(phases, {'early': 'a', 'once': 'b'})
        self.assertEqual(result['early']['status'], 'fail')
        self.assertEqual(result['once']['status'], 'pass')

    def test_changed_scene_cannot_count_as_a_pass(self):
        result = latest_image_cases({'new': [
            {'scene': 'one', 'scene_sha256': 'changed', 'status': 'pass'}]}, {'one': 'original'})
        self.assertEqual(result['one']['status'], 'error')

    def test_interns_full_manifest_without_changing_measurements(self):
        manifest = {'sources': {'a.cpp': 'abc'}, 'binary': 'def'}
        source = {'scene': 'one', 'status': 'fail', 'diff': {'p99': 17},
                  'reference_provenance': manifest}
        records = {}
        first = compact_image_evidence(source, records)
        second = compact_image_evidence({**source, 'scene': 'two'}, records)
        key = first['reference_provenance_sha256']
        self.assertEqual(second['reference_provenance_sha256'], key)
        self.assertEqual(len(records), 1)
        self.assertEqual(records[key], manifest)
        self.assertEqual(key, digest(json.dumps(manifest, sort_keys=True,
                         separators=(',', ':')).encode('utf-8')))
        self.assertEqual(first['status'], 'fail')
        self.assertEqual(first['diff'], source['diff'])
        self.assertIn('reference_provenance', source)

    def test_distinct_manifests_remain_distinct(self):
        records = {}
        for value in ['old', 'new']:
            compact_image_evidence({'reference_provenance': {'binary': value}}, records)
        self.assertEqual(len(records), 2)

    def test_missing_manifest_does_not_invent_provenance(self):
        records = {}
        self.assertEqual(compact_image_evidence({'status': 'error'}, records), {'status': 'error'})
        self.assertEqual(records, {})

    def test_compact_result_requires_retained_manifest(self):
        with self.assertRaisesRegex(ValueError, 'Missing provenance'):
            compact_image_evidence({'status': 'pass', 'reference_provenance_sha256': 'missing'}, {})

    def test_inline_manifest_cannot_replace_conflicting_reference(self):
        with self.assertRaisesRegex(ValueError, 'Inline provenance differs'):
            compact_image_evidence({'reference_provenance': {'binary': 'one'},
                                    'reference_provenance_sha256': 'another'}, {})


if __name__ == '__main__':
    unittest.main()
