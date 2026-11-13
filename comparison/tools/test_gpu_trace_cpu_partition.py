"""Synthetic rejection tests only; no fixture counts as render qualification."""
import copy
import tempfile
import unittest
from pathlib import Path
from test_gpu_pipeline_proofs import Bundle
import gpu_pipeline_proofs as proof


class TracePartitionTests(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.bundle = Bundle(Path(self.tmp.name).resolve())

    def tearDown(self):
        self.tmp.cleanup()

    def fixture(self):
        b = self.bundle
        data = b.trace()
        b.ctx['scenes']['cpu-case'] = dict(b.scene, id='cpu-case')
        result = {'status': 'QUALIFIED_FP32_CPU', 'independently_proven_pixels': 2,
                  'range_pixels': 3, 'size': [2, 2], 'image': {'rgb_sha256': 'fixture'}}
        b.ctx['qualified_fp32_cpu_scenes'] = {'cpu-case': result}
        data['cpu_fallback_scenes'] = [{'id': 'cpu-case', 'qualification': 'QUALIFIED_FP32_CPU',
                                      'result_sha256': proof.digest(result)}]
        return data

    def validate(self, data):
        proof.no_cpu_pixel_work(self.bundle.root, data, self.bundle.ctx)

    def test_separate_replayed_cpu_outcome(self):
        self.validate(self.fixture())

    def test_gpu_trace_uses_its_selected_resource_directory(self):
        data = self.fixture()
        ctx = self.bundle.ctx
        selected = ctx['runtime']
        default = self.bundle.root / 'different-default-resources'
        default.mkdir()
        ctx['runtime'] = default
        ctx['resource_profiles'] = {'selected': {'runtime': selected,
            'runtime_files': {}, 'candidate_map_ids': []}}
        ctx['scene_resources'] = {'s': 'selected'}
        ctx['scene_required_maps'] = {'s': []}
        self.validate(data)

    def test_missing_cpu_report_rejected(self):
        data = self.fixture()
        data['cpu_fallback_scenes'] = []
        with self.assertRaises(ValueError):
            self.validate(data)

    def test_unproved_cpu_claim_rejected(self):
        data = self.fixture()
        self.bundle.ctx['qualified_fp32_cpu_scenes'] = {}
        with self.assertRaises(ValueError):
            self.validate(data)

    def test_cpu_mixed_into_gpu_rows_rejected(self):
        data = self.fixture()
        data['scenes'].append(dict(data['scenes'][0], id='cpu-case'))
        with self.assertRaises(ValueError):
            self.validate(data)

    def test_stale_exception_proof_rejected(self):
        data = self.fixture()
        self.bundle.ctx['qualified_fp32_cpu_scenes']['cpu-case']['range_pixels'] += 1
        with self.assertRaises(ValueError):
            self.validate(data)

    def test_unqualified_context_rejected(self):
        data = self.fixture()
        self.bundle.ctx['qualified_fp32_cpu_scenes']['cpu-case']['status'] = 'FP32_RANGE'
        with self.assertRaises(ValueError):
            self.validate(data)

    def test_missing_gpu_trace_rejected(self):
        data = self.fixture()
        data['scenes'] = []
        with self.assertRaises(ValueError):
            self.validate(data)

    def test_duplicate_exception_rejected(self):
        data = self.fixture()
        data['cpu_fallback_scenes'].append(copy.deepcopy(data['cpu_fallback_scenes'][0]))
        with self.assertRaises(ValueError):
            self.validate(data)


if __name__ == '__main__':
    unittest.main()
