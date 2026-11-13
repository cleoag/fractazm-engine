"""Regressions for layouts observed in the independent Pascal buffer dump."""
import math
from types import SimpleNamespace
from capture_formula_regressions import literal
import struct
import sys
import tempfile
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[2] / 'scripts'))
from gen_bulk_formula_table import parse_m3f_options, append_constant_offsets, get_adds_c_flag


class ExternalConstantLayoutTests(unittest.TestCase):
    def test_fixture_literal_preserves_negative_zero(self):
        self.assertEqual(literal(-0.0), '-0.0')
        self.assertEqual(math.copysign(1.0, float(literal(-0.0))), -1.0)

    def test_native_formula_c_addition_survives_regeneration(self):
        for name, de in [('Aexion1', 4), ('ATetraVS', 0), ('_SphereFolding1', -1), ('_MapTranslate', -1), ('KaliLinCombSSE2sm', 0), ('_helispiral', -1), ('MengerKochV2', 2)]:
            entry = SimpleNamespace(de_option=de, formula_name=name,
                                    formula_id_name='', adds_c_internally=False)
            self.assertEqual(get_adds_c_flag(entry), 1, name)

    def test_invalid_float_constant_keeps_zero_slot(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / 'probe.m3f'
            path.write_text('[CONSTANTS]\nDouble = 1e-30;\nSingle = invalid\nDouble = 0.25\n[CODE]\nC3\n[END]\n')
            self.assertEqual(parse_m3f_options(str(path))['const_entries'],
                             [(0, 0), (1, 0), (0, 0x3fd0000000000000)])

    def test_ifs_tess_matches_pascal_constant_slots(self):
        root = Path(__file__).resolve().parents[2]
        data = parse_m3f_options(str(root / 'mb3d/M3Formulas/_IFS_tess.m3f'))
        self.assertEqual(data['const_entries'], [
            (0, 0x4000000000000000), (0, 0x3fe6a09e6665983e),
            (0, 0x3f91df46aa91ed06), (0, 0x407f400000000000)])
        # Integer = 50 is ignored by Pascal; Double 500 occupies PVar+24.
        self.assertEqual(append_constant_offsets([], data['const_entries']),
                         [(0, 0), (8, 0), (16, 0), (24, 0)])

    def test_unsupported_named_integer_does_not_shift_later_constants(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / 'probe.m3f'
            path.write_text('[CONSTANTS]\n.Integer Count = 50\n'
                            '.Double Clip = 500\n.Single Half = 0.5\n'
                            'Int64 = $8000000000000000\n[CODE]\nC3\n[END]\n')
            self.assertEqual(parse_m3f_options(str(path))['const_entries'], [
                (0, struct.unpack('<Q', struct.pack('<d', 500.))[0]),
                (1, 0x3f000000), (3, 0x8000000000000000)])


if __name__ == '__main__':
    unittest.main()
