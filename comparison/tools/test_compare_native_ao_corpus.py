"""Negative controls for old/new comparison; fixtures are not render evidence."""
import copy
import unittest
from compare_native_ao_corpus import exact_pair


class CorpusPairTests(unittest.TestCase):
    def fixture(self):
        old = {'image': {'rgb_sha256': 'same'}, 'work': {'execution': {
            'execution_plan': 'general-v1|rng:quota=128,chunks=8|seed-geometry:version=1',
            'ao_certificate_version': 1, 'ao_algorithm': 4,
            'ao_certificate_levels': 3, 'ao_certificate_pixels': 360000,
            'ao_commands': 100, 'ao_dispatches': 100, 'rng_advances': 123}}}
        new = copy.deepcopy(old)
        new['work']['execution']['execution_plan'] += ',chunk-prefix=1,base-table-reuse=1'
        new['work']['execution']['ao_commands'] += 64
        new['work']['execution']['ao_dispatches'] += 64
        return old, new

    def test_exact_with_only_derived_physical_overhead(self):
        self.assertEqual(exact_pair(*self.fixture(), [400, 300], 6),
                         {'ao_chunk_preparation_commands': 64})

    def test_changed_rgb_rejected(self):
        old, new = self.fixture()
        new['image']['rgb_sha256'] = 'changed'
        with self.assertRaisesRegex(ValueError, 'RGB'):
            exact_pair(old, new, [400, 300], 6)

    def test_changed_logical_work_rejected(self):
        old, new = self.fixture()
        new['work']['execution']['rng_advances'] += 1
        with self.assertRaisesRegex(ValueError, 'logical work'):
            exact_pair(old, new, [400, 300], 6)

    def test_arbitrary_dispatch_change_rejected(self):
        old, new = self.fixture()
        new['work']['execution']['ao_dispatches'] += 1
        with self.assertRaisesRegex(ValueError, 'logical work'):
            exact_pair(old, new, [400, 300], 6)

    def test_invalid_normalization_policy_rejected(self):
        old, new = self.fixture()
        new['work']['execution']['execution_plan'] += ',base-table-reuse=2'
        with self.assertRaises(AssertionError):
            exact_pair(old, new, [400, 300], 6)


if __name__ == '__main__':
    unittest.main()
