"""Reject vacuous or inconsistent evidence when qualifying diagnostic views."""
import json
import struct
import sys
import tempfile
import unittest
from pathlib import Path
from unittest.mock import patch
from PIL import Image
import qualify_formula_views as views


class FormulaViewTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        (self.root/'scene.m3p').write_bytes(b'scene')
        (self.root/'case_without_F.m3p').write_bytes(b'control')
        self.depth = [*range(11), 32768]
        self.dump('case', self.depth)
        self.dump('case_without_F', [32768]*12)
        image = Image.new('RGB', (4, 3))
        image.putdata([(i*20, i, 0) for i in range(12)])
        image.save(self.root/'case.png')
        case = dict(id='case', mode=1, scene='scene.m3p', target_formulas=['F'],
                    sha256=views.digest(b'scene'))
        self.manifest = self.root/'manifest.json'
        self.manifest.write_text(json.dumps(dict(cases=[case])))
        self.formula = dict(name='F', control='omit-target',
                            control_sha256=views.digest(b'control'), changed_depth_pixels=11)
        self.evidence = dict(manifest_sha256=views.digest(self.manifest.read_bytes()),
            cpp_sha256='renderer', results=[dict(case='case', scene_sha256=case['sha256'],
            geometry_pixels=11, pixels=12, formulas=[self.formula])])

    def dump(self, name, depth):
        data = bytearray(b'SIL5DMP1'+struct.pack('<5I', 1, 4, 3, 18, 12)+bytes(12*18))
        for i, value in enumerate(depth):
            struct.pack_into('<H', data, 28+i*18+8, value)
        (self.root/(name+'.silight')).write_bytes(data)

    def run_check(self):
        evidence = self.root/'influence.json'
        evidence.write_text(json.dumps(self.evidence))
        output = self.root/'result.json'
        argv = ['qualify', '--manifest', str(self.manifest), '--influence', str(evidence),
                '--output', str(output)]
        with patch.object(views, 'ROOT', self.root), patch.object(sys, 'argv', argv):
            views.main()
        return json.loads(output.read_bytes())

    def test_varied_geometry_and_omission_are_useful(self):
        report = self.run_check()
        self.assertEqual(report['summary'], {'useful': 1})
        self.assertEqual(report['useful_modes'], [1])
        self.assertFalse(report['pascal_parity'])

    def test_constant_image_is_not_useful(self):
        Image.new('RGB', (4, 3)).save(self.root/'case.png')
        self.assertEqual(self.run_check()['summary'], {'not-qualified': 1})

    def test_no_geometry_is_not_useful(self):
        self.dump('case', [32768]*12)
        self.formula['changed_depth_pixels'] = 0
        self.assertEqual(self.run_check()['summary'], {'not-qualified': 1})

    def test_substitution_cannot_replace_omission(self):
        self.formula['control'] = 'substitute-builtin'
        self.assertEqual(self.run_check()['summary'], {'error': 1})

    def test_changed_control_is_an_error(self):
        self.dump('case_without_F', self.depth)
        self.assertEqual(self.run_check()['summary'], {'error': 1})

    def test_changed_scene_is_an_error(self):
        (self.root/'scene.m3p').write_bytes(b'other scene')
        self.assertEqual(self.run_check()['summary'], {'error': 1})


if __name__ == '__main__':
    unittest.main()
