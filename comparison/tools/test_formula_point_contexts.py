import hashlib
from pathlib import Path
import unittest
from formula_point_contexts import SCALAR_MAP_ASSETS, scalar_map_context


class ScalarMapContextTests(unittest.TestCase):
    def test_permission_is_bound_to_original_asset_bytes(self):
        assets = Path(__file__).resolve().parents[2] / 'mb3d/M3Formulas'
        for name in SCALAR_MAP_ASSETS:
            raw = (assets / (name + '.m3f')).read_bytes()
            self.assertTrue(scalar_map_context(name, hashlib.sha256(raw).hexdigest()))
            self.assertFalse(scalar_map_context(name, hashlib.sha256(raw + b'changed').hexdigest()))

    def test_exact_lf_and_crlf_variants_are_accepted(self):
        assets = Path(__file__).resolve().parents[2] / 'mb3d/M3Formulas'
        for name, crlf_digest in SCALAR_MAP_ASSETS.items():
            raw = (assets / (name + '.m3f')).read_bytes()
            # Fixture construction only: production receives an exact raw hash.
            lf = raw.replace(b'\r\n', b'\n')
            self.assertNotIn(b'\r', lf)
            crlf = lf.replace(b'\n', b'\r\n')
            self.assertIn(raw, (lf, crlf))
            self.assertEqual(hashlib.sha256(crlf).hexdigest(), crlf_digest)
            for variant in (lf, crlf):
                with self.subTest(name=name, crlf=variant == crlf):
                    self.assertTrue(scalar_map_context(name, hashlib.sha256(variant).hexdigest()))
                    self.assertFalse(scalar_map_context(name, hashlib.sha256(variant + b'changed').hexdigest()))

    def test_mixed_and_cr_only_endings_remain_excluded(self):
        assets = Path(__file__).resolve().parents[2] / 'mb3d/M3Formulas'
        for name in SCALAR_MAP_ASSETS:
            lf = (assets / (name + '.m3f')).read_bytes().replace(b'\r\n', b'\n')
            self.assertGreater(lf.count(b'\n'), 1)
            for invalid in (lf.replace(b'\n', b'\r\n', 1), lf.replace(b'\n', b'\r')):
                self.assertFalse(scalar_map_context(name, hashlib.sha256(invalid).hexdigest()))

    def test_hashes_are_bound_to_their_exact_formula_name(self):
        assets = Path(__file__).resolve().parents[2] / 'mb3d/M3Formulas'
        for name in SCALAR_MAP_ASSETS:
            lf = (assets / (name + '.m3f')).read_bytes().replace(b'\r\n', b'\n')
            for variant in (lf, lf.replace(b'\n', b'\r\n')):
                digest = hashlib.sha256(variant).hexdigest()
                for wrong_name in (*SCALAR_MAP_ASSETS, 'unknown', name.lower(), ''):
                    if wrong_name != name:
                        self.assertFalse(scalar_map_context(wrong_name, digest))

    def test_real_maps_and_unknown_names_remain_excluded(self):
        for name in ('HeightMapIFS', 'CylinderHeightMap', '_MapTranslate', 'TgladTetraExtra', ''):
            self.assertFalse(scalar_map_context(name, SCALAR_MAP_ASSETS['TgladTetra']))
        self.assertFalse(scalar_map_context('unknown', None))


if __name__ == '__main__':
    unittest.main()
