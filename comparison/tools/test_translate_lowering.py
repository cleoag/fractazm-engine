"""Fail-closed original-source lowering controls; arithmetic tested separately."""
import dataclasses
from pathlib import Path
import sys
import tempfile
import unittest
from unittest.mock import patch

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'scripts'))
import gen_native_metal_formulas as g


class TranslateLoweringTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.entry = next(e for e in g.registry() if e.id == 8065)
        expanded = g.expand_local(g.FORMULAS / cls.entry.source, set(), {})
        selected = g.select_arm(expanded, '/usr/bin/clang')
        values, cls.traits = g.remove_metadata(g.tokens(selected), cls.entry.symbol)
        cls.values = g.lower_context(g.lower(values))

    def test_actual_original_both_entrypoints_use_the_same_exact_add(self):
        out = g.lower_translate_source_add(self.values, self.entry, self.traits)
        self.assertEqual(out.count('native_translate_add'), 6)
        for old in ('p.x+X_add', 'p.y+Y_add', 'p.z+Z_add',
                    'p.x+params[0]', 'p.y+params[1]', 'p.z+params[2]'):
            pattern = g.tokens(old)
            self.assertFalse(any(out[i:i+len(pattern)] == pattern for i in range(len(out))))

    def test_changed_descriptor_or_traits_refuse(self):
        for changes in ({'de_option': 2}, {'param_count': 4}, {'adds_c': 0},
                        {'parameter_types': (1, 0, 0)}, {'symbol': 'WRONG'},
                        {'source': 'Ads/other.inc'}):
            with self.subTest(changes=changes), self.assertRaisesRegex(ValueError, 'descriptor'):
                g.lower_translate_source_add(self.values, dataclasses.replace(self.entry, **changes), self.traits)
        with self.assertRaisesRegex(ValueError, 'descriptor'):
            g.lower_translate_source_add(self.values, self.entry, 8)

    def test_original_code_change_refuses_without_editing_immutable_tree(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory); p = root / 'mb3d/M3Formulas/_Translate.m3f'
            p.parent.mkdir(parents=True); p.write_bytes(b'changed original source')
            with patch.object(g, 'ROOT', root), self.assertRaisesRegex(ValueError, 'CODE'):
                g.lower_translate_source_add(self.values, self.entry, self.traits)

    def test_missing_duplicate_or_reassociated_operation_refuses(self):
        original = 'return FormulaVec3(p.x+X_add,p.y+Y_add,p.z+Z_add);'
        candidates = [g.replace_tokens(self.values, original, 'return p;'),
                      self.values + g.tokens(original),
                      g.replace_tokens(self.values, 'p.x+X_add', 'X_add+p.x')]
        for values in candidates:
            with self.subTest(tokens=len(values)), self.assertRaisesRegex(ValueError, 'source contract'):
                g.lower_translate_source_add(values, self.entry, self.traits)

    def test_unrelated_formula_is_untouched(self):
        other = dataclasses.replace(self.entry, id=6003)
        self.assertIs(g.lower_translate_source_add(self.values, other, self.traits), self.values)


if __name__ == '__main__':
    unittest.main()
