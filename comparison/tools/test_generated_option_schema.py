"""Regressions for selectors previously overwritten by loader defaults."""
import struct
import unittest

from external_validation import (ROOT, prepare_external_options,
                                 saved_option_types, slots, parse_m3f_options)
from generate_external_challenges import set_slot


class GeneratedOptionSchemaTests(unittest.TestCase):
    def scene(self, name, saved_count, values):
        data = bytearray(1976)
        struct.pack_into('<iii', data, 848, 1, 20, saved_count)
        data[860:892] = name.encode().ljust(32, b'\0')
        struct.pack_into('<16d', data, 908, *(values + [0.] * (16-len(values))))
        return data

    def test_last_selector_survives_full_schema(self):
        for name, count, full, selector in [('Amazing Surf 2', 5, 7, 6),
                                            ('koch_surf', 12, 14, 13)]:
            with self.subTest(name=name):
                data = self.scene(name, count, [42.] * 16)
                slot = prepare_external_options(data, slots(data)[0])
                self.assertEqual(slot['option_count'], full)
                metadata = parse_m3f_options(ROOT / 'mb3d/M3Formulas' / (name+'.m3f'))
                self.assertEqual(slot['options'][selector], metadata['option_defaults'][selector])
                struct.pack_into('<d', data, 908+selector*8, 15.)
                before = bytes(data)
                prepare_external_options(data, slots(data)[0])
                self.assertEqual(bytes(data), before, 'explicit final selector must survive another preparation')

    def test_set_slot_counts_saved_values(self):
        metadata = dict(option_types=[0, 6, 2], option_defaults=[2., 0., 0., 0., 3.])
        data = bytearray(1976)
        set_slot(data, 0, 'test', metadata)
        self.assertEqual(slots(data)[0]['option_count'], 5)
        self.assertEqual(list(data[892:897]), [0, 6, 6, 6, 2])
        self.assertEqual(struct.unpack_from('<d', data, 908+4*8)[0], 3.)

    def test_cylinder_legacy_fields_move_before_explicit_mutation(self):
        data = self.scene('cylinderIFS', 8, [0., 1., 2., 3., 4., 5., 6., 17.])
        slot = prepare_external_options(data, slots(data)[0])
        self.assertEqual(slot['options'][6:9], [6., 0., 17.])
        self.assertGreaterEqual(slot['option_count'], 10)

    def test_sphere_heightmap_migration_is_not_applied_twice(self):
        data = self.scene('SphereHeightMap', 14, [10., 0., 6., 8., 0., 2., 4., .5,
                                                     13., -27., 41., 2., -.125, -2.])
        slot = prepare_external_options(data, slots(data)[0])
        self.assertEqual(slot['options'][2:4], [3., 2.])
        self.assertEqual(slot['options'][8:12], [1., 13., -27., 41.])
        before = bytes(data)
        prepare_external_options(data, slots(data)[0])
        self.assertEqual(bytes(data), before)

    def test_compound_overflow_is_rejected_before_writing(self):
        with self.assertRaises(ValueError):
            saved_option_types(dict(option_types=[12]*3, option_defaults=[0.]*18))


if __name__ == '__main__':
    unittest.main()
