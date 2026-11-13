"""Synthetic tamper controls; no GPU, PNG-loader or arithmetic qualification."""
import copy
import hashlib
from pathlib import Path
import tempfile
import unittest
import struct
import zlib

import native_formula_resource_input as r
from gpu_pipeline_proofs import digest, semantic_input


class ResourceInput(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.root = Path(self.tmp.name).resolve()
        asset = self.root / 'runtime/M3Maps/17.png'
        asset.parent.mkdir(parents=True)
        def chunk(tag, payload):
            return struct.pack('>I', len(payload)) + tag + payload + struct.pack('>I', zlib.crc32(tag + payload))
        asset.write_bytes(b'\x89PNG\r\n\x1a\n' +
                          chunk(b'IHDR', struct.pack('>IIBBBBB', 1, 1, 8, 2, 0, 0, 0)) +
                          chunk(b'IDAT', zlib.compress(b'\x00\xff\x00\x00')) + chunk(b'IEND', b''))
        self.pixels = [0x437f0000, 0, 0] * 4
        self.payload = {'schema': r.SCHEMA, 'root': 'runtime', 'map_ids': [17],
                        'files': [{'path': 'runtime/M3Maps/17.png',
                                   'sha256': hashlib.sha256(asset.read_bytes()).hexdigest()}],
                        'descriptor_words': [[17, 1, 1, 3, 0, 12, 0, 0]],
                        'pixel_count': 12, 'pixel_words_sha256': r.pixel_hash(self.pixels),
                        'controls': 'active'}

    def tearDown(self):
        self.tmp.cleanup()

    def report(self):
        controls = []
        for name in r.CONTROLS:
            fault = 7 if name == 'missing' else 4 if name == 'nonfinite_pixel' else 9
            words = [0] * 59
            words[47], words[51] = fault, 1
            controls.append({'name': name, 'expected_fault': fault, 'repeats': 3,
                             'rows': [{'index': 0, 'result_words': words,
                                       'fault': fault, 'work': 1, 'completed': 0}]})
        return {'map_abi': [1, 32, 3, 1], 'resources': {
            'schema': 'mb3d.native-formula-resources.v1',
            'resource_root': str(self.root / 'runtime'),
            'loader': 'LoadLightMapNr via fresh Native RenderContext',
            'requested_map_ids': [17], 'descriptor_words': copy.deepcopy(self.payload['descriptor_words']),
            'pixel_words': self.pixels.copy(), 'map_count': 1, 'pixel_count': 12,
            'input_immutable': True, 'allocation_canaries': True, 'program_immutable': True,
            'control_mode': 'active', 'exact_control_repeats': True, 'controls': controls}}

    def test_bound_resource_input_and_actual_report(self):
        runtime, files = r.validate(self.payload, self.root)
        self.assertEqual(runtime, self.root / 'runtime')
        self.assertEqual(files, [runtime / 'M3Maps/17.png'])
        r.validate_report(self.report(), self.payload, self.root)

    def test_asset_change_cannot_reuse_pinned_resource_input(self):
        (self.root / self.payload['files'][0]['path']).write_bytes(b'changed')
        with self.assertRaises(ValueError):
            r.validate(self.payload, self.root)

    def test_competing_loader_candidate_rejected_even_with_same_png(self):
        (self.root / 'runtime/M3Maps/17.bmp').write_bytes(b'unpinned alternative')
        with self.assertRaisesRegex(ValueError, 'Unpinned competing'):
            r.validate(self.payload, self.root)

    def test_rehashed_but_invalid_png_cannot_fall_through_to_another_root(self):
        asset = self.root / self.payload['files'][0]['path']
        asset.write_bytes(b'not a PNG')
        self.payload['files'][0]['sha256'] = hashlib.sha256(asset.read_bytes()).hexdigest()
        with self.assertRaisesRegex(ValueError, 'not a PNG'):
            r.validate(self.payload, self.root)

    def test_invalid_resource_semantics(self):
        changes = [lambda x: x.update(root='../outside'),
                   lambda x: x.update(root=str(self.root)),
                   lambda x: x.update(map_ids=[True]),
                   lambda x: x.update(map_ids=[17, 17]),
                   lambda x: x.update(map_ids=[-1]),
                   lambda x: x.update(files=[]),
                   lambda x: x.update(controls='none'),
                   lambda x: x.update(pixel_count=True),
                   lambda x: x['descriptor_words'][0].__setitem__(4, 1),
                   lambda x: x['descriptor_words'][0].__setitem__(5, 11),
                   lambda x: x['descriptor_words'][0].__setitem__(7, 1)]
        for index, change in enumerate(changes):
            value = copy.deepcopy(self.payload); change(value)
            with self.subTest(index=index), self.assertRaises(ValueError):
                r.validate(value, self.root)

    def test_actual_upload_and_controls_cannot_be_forged_by_labels(self):
        changes = [lambda x: x.update(map_abi=[True, 32, 3, 1]),
                   lambda x: x['resources'].update(requested_map_ids=[18]),
                   lambda x: x['resources']['pixel_words'].__setitem__(0, 0),
                   lambda x: x['resources']['descriptor_words'][0].__setitem__(1, True),
                   lambda x: x['resources'].update(input_immutable=False),
                   lambda x: x['resources'].update(exact_control_repeats=1),
                   lambda x: x['resources'].update(control_mode='none'),
                   lambda x: x['resources']['controls'].pop(),
                   lambda x: x['resources']['controls'][0].update(name='missing'),
                   lambda x: x['resources']['controls'][0].update(repeats=2),
                   lambda x: x['resources']['controls'][0]['rows'][0].update(completed=1),
                   lambda x: x['resources']['controls'][0]['rows'][0]['result_words'].__setitem__(47, 0)]
        for index, change in enumerate(changes):
            value = self.report(); change(value)
            with self.subTest(index=index), self.assertRaises(ValueError):
                r.validate_report(value, self.payload, self.root)

    def test_resource_arguments_and_files_must_be_captured(self):
        event = {'argv': ['probe'] + r.arguments(self.payload, self.root),
                 'inputs': copy.deepcopy(self.payload['files'])}
        r.validate_arguments(event, self.payload, self.root)
        for change in (lambda x: x.update(inputs=[]),
                       lambda x: x['argv'].extend(['--map-id', '18']),
                       lambda x: x['argv'].__setitem__(-1, '18')):
            value = copy.deepcopy(event); change(value)
            with self.assertRaises(ValueError):
                r.validate_arguments(value, self.payload, self.root)

    def test_resource_bytes_belong_to_training_identity(self):
        case = {'identity': ['external', 10006, '_MapTranslate'], 'kind': 'point',
                'input_state_bits': {}, 'parameter_bits': [], 'resources': self.payload}
        baseline = digest(semantic_input(case))
        case['variant'] = 'renamed'
        self.assertEqual(digest(semantic_input(case)), baseline)
        case['resources'] = copy.deepcopy(self.payload)
        case['resources']['pixel_words_sha256'] = '0' * 64
        self.assertNotEqual(digest(semantic_input(case)), baseline)


if __name__ == '__main__':
    unittest.main()
