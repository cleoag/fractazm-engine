"""Negative controls for release-report acceptance, without launching renders."""
import unittest
import xml.etree.ElementTree as ET

from audit_stage_a_release import (
    MANOWAR_REASON, ORDINARY_TESTS, POINT_UNVERIFIED, SELECTOR_UNVERIFIED,
    primary_failures, validate_case_identities, validate_ordinary_tests,
)


def report(names):
    root = ET.Element('testsuites')
    suite = ET.SubElement(root, 'testsuite')
    for name in names:
        ET.SubElement(suite, 'testcase', name=name)
    return root


class ReleaseGateTests(unittest.TestCase):
    def test_baseline(self):
        self.assertEqual(validate_ordinary_tests(report(ORDINARY_TESTS)), sorted(ORDINARY_TESTS))

    def test_declared_extra_suite(self):
        names = ORDINARY_TESTS | {'legacy_loader_tests'}
        self.assertEqual(validate_ordinary_tests(report(names), ['legacy_loader_tests']), sorted(names))

    def test_same_count_wrong_suite(self):
        names = (ORDINARY_TESTS - {'renderer_tests'}) | {'halton_tests'}
        with self.assertRaises(ValueError):
            validate_ordinary_tests(report(names))

    def test_duplicate_or_missing(self):
        for names in (list(ORDINARY_TESTS) + ['renderer_tests'], ORDINARY_TESTS - {'renderer_tests'}):
            with self.subTest(names=names), self.assertRaises(ValueError):
                validate_ordinary_tests(report(names))

    def test_failed_or_skipped_suite(self):
        for tag in ('failure', 'error', 'skipped'):
            root = report(ORDINARY_TESTS)
            ET.SubElement(root.find('.//testcase'), tag)
            with self.subTest(tag=tag), self.assertRaises(ValueError):
                validate_ordinary_tests(root)

    def test_invalid_extra_declaration(self):
        for extra in (['halton_tests'], ['monte_carlo_tests'], ['renderer_tests'], ['', 'x'], ['x', 'x']):
            with self.subTest(extra=extra), self.assertRaises(ValueError):
                validate_ordinary_tests(report(ORDINARY_TESTS), extra)

    def test_unknown_unverified_fails_on_every_surface(self):
        surfaces = [(s, [dict(case='c', status='unverified')])
                    for s in ('points', 'selector_points', 'images', 'ordinary343')]
        self.assertEqual([r['surface'] for r in primary_failures(surfaces)],
                         ['points', 'selector_points', 'images', 'ordinary343'])

    def test_recorded_unverified_policy(self):
        self.assertEqual(len(POINT_UNVERIFIED), 19)
        self.assertEqual(len(SELECTOR_UNVERIFIED), 2)
        for surface, policy in [('points', POINT_UNVERIFIED), ('selector_points', SELECTOR_UNVERIFIED)]:
            for case, (name, reason) in policy.items():
                row = dict(case=case, name=name, status='unverified', reason=reason)
                if name != 'ManowarBulb':
                    row.update(reason=reason+' slot 1 original unsupported input',
                               input_validation='rejected', cpp_exit_code=12)
                with self.subTest(surface=surface, case=case):
                    self.assertEqual(primary_failures([(surface, [row])]), [])
                    # A known ID cannot hide a new failure under another reason.
                    for change in ({'reason': 'Formula work budget exhausted'},
                                   {'name': 'other'}, {'status': 'error'},
                                   {'status': 'fail'}):
                        self.assertEqual(len(primary_failures([(surface, [dict(row, **change)])])), 1)

    def test_known_rejection_requires_the_declared_exit_contract(self):
        row = dict(case='challenge_TrifoxComplexAngles_p0_zero', name='TrifoxComplexAngles',
                   status='unverified', reason='FORMULA_INPUT_REJECTED[nonterminating_zero_power] slot 1',
                   input_validation='rejected', cpp_exit_code=12)
        for change in ({'cpp_exit_code': 13}, {'input_validation': None}, {'reason': ''}):
            with self.subTest(change=change):
                self.assertEqual(len(primary_failures([('points', [dict(row, **change)])])), 1)

    def test_primary_and_selector_exceptions_cannot_swap(self):
        row = dict(case='challenge_ManowarBulb_early', name='ManowarBulb',
                   status='unverified', reason=MANOWAR_REASON)
        self.assertEqual(len(primary_failures([('selector_points', [row])])), 1)

    def test_new_case_of_known_formula_is_not_an_exception(self):
        row = dict(case='new_Manowar_case', name='ManowarBulb',
                   status='unverified', reason=MANOWAR_REASON)
        self.assertEqual(len(primary_failures([('points', [row])])), 1)

    def test_unknown_and_missing_status_fail_closed(self):
        rows = [dict(case='unknown', status='timeout'), dict(case='missing')]
        self.assertEqual(len(primary_failures([('points', rows)])), 2)

    def test_pass_is_accepted(self):
        self.assertEqual(primary_failures([('images', [dict(scene='s', status='pass')])]), [])


class CaseIdentityTests(unittest.TestCase):
    def test_exact_set_may_be_reordered(self):
        expected = {('a',): 'hash-a', ('b',): 'hash-b'}
        rows = [dict(case='b', scene_sha256='hash-b'), dict(case='a', scene_sha256='hash-a')]
        self.assertEqual(set(validate_case_identities(rows, expected, ('case',), 'points')), set(expected))

    def test_duplicate_does_not_replace_missing_case(self):
        expected = {('a',): 'hash-a', ('b',): 'hash-b'}
        rows = [dict(case='a', scene_sha256='hash-a')] * 2
        with self.assertRaisesRegex(ValueError, 'duplicate'):
            validate_case_identities(rows, expected, ('case',), 'selector points')

    def test_same_count_wrong_case_is_rejected(self):
        expected = {('a',): 'hash-a', ('b',): 'hash-b'}
        rows = [dict(case='a', scene_sha256='hash-a'), dict(case='other', scene_sha256='hash-b')]
        with self.assertRaisesRegex(ValueError, 'missing/unexpected'):
            validate_case_identities(rows, expected, ('case',), 'points')

    def test_missing_extra_and_changed_scene_are_rejected(self):
        expected = {('a',): 'hash-a'}
        for rows in ([], [dict(case='a', scene_sha256='wrong')],
                     [dict(case='a')], [dict(scene_sha256='hash-a')],
                     [dict(case='a', scene_sha256='hash-a'), dict(case='b', scene_sha256='hash-b')]):
            with self.subTest(rows=rows), self.assertRaises(ValueError):
                validate_case_identities(rows, expected, ('case',), 'excluded images')

    def test_ordinary_case_identity_includes_suite(self):
        expected = {('suite1', 'shared'): 'hash1', ('suite2', 'shared'): 'hash2'}
        rows = [dict(suite='suite1', scene='shared', scene_sha256='hash1'),
                dict(suite='suite2', scene='shared', scene_sha256='hash2')]
        self.assertEqual(len(validate_case_identities(rows, expected, ('suite', 'scene'), 'ordinary343')), 2)
        rows[1]['suite'] = 'suite1'
        with self.assertRaisesRegex(ValueError, 'duplicate'):
            validate_case_identities(rows, expected, ('suite', 'scene'), 'ordinary343')


if __name__ == '__main__':
    unittest.main()
