"""Guard the exact reviewed Surf sequence and its narrow contraction scope."""
import dataclasses
from pathlib import Path
import sys
import tempfile
import unittest
from unittest.mock import patch

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'scripts'))
import gen_native_metal_formulas as g


class SurfSourceOrder(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.entry = next(e for e in g.registry() if e.id == 1509)
        path = ROOT / 'comparison/results/formula-campaign-004/group01/8065/t009/original-generated/formula_1509.metal.h'
        original = path.read_text()
        cls.text = original[original.index('constant constexpr'):original.rindex('}}')]

    def test_only_function_local_pragma_changes_reviewed_operations(self):
        out = g.emit_surf_source_contract(self.text, self.entry, 0)
        self.assertEqual(out.count('#pragma clang fp contract(off)'), 1)
        self.assertEqual(out.replace('\n#pragma clang fp contract(off)', ''), self.text)
        self.assertLess(out.index('execute_formula <'), out.index('#pragma'))
        self.assertLess(out.index('#pragma'), out.index('const numeric'))

    def test_reassociation_or_operation_change_refuses(self):
        for text in (self.text.replace('fy * fy + fx * fx', 'fx * fx + fy * fy'),
                     self.text.replace('x * matrix [ 0 ]', 'x + matrix [ 0 ]'), self.text + '\n'):
            with self.subTest(text=text[:20]), self.assertRaisesRegex(ValueError, 'sequence'):
                g.emit_surf_source_contract(text, self.entry, 0)

    def test_descriptor_traits_and_original_code_refuse(self):
        for changes in ({'param_count': 18}, {'de_option': 2}, {'adds_c': 0}, {'source': 'other.inc'}):
            with self.subTest(changes=changes), self.assertRaisesRegex(ValueError, 'descriptor'):
                g.emit_surf_source_contract(self.text, dataclasses.replace(self.entry, **changes), 0)
        with self.assertRaisesRegex(ValueError, 'descriptor'):
            g.emit_surf_source_contract(self.text, self.entry, 8)
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory); path = root / 'mb3d/M3Formulas/Amazing Surf.m3f'
            path.parent.mkdir(parents=True); path.write_text('changed source')
            with patch.object(g, 'ROOT', root), self.assertRaisesRegex(ValueError, 'CODE'):
                g.emit_surf_source_contract(self.text, self.entry, 0)

    def test_other_formula_text_is_unchanged(self):
        self.assertIs(g.emit_surf_source_contract(self.text, dataclasses.replace(self.entry, id=7039), 0), self.text)


if __name__ == '__main__':
    unittest.main()
