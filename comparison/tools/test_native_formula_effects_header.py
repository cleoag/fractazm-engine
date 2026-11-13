"""Fixtures for native_formula_effects_header and a staleness check of the committed header.

No compiler or device; the staleness case reads only checked-in sources.
"""
import hashlib, json, re, tempfile, unittest
from pathlib import Path
from unittest.mock import patch
import native_formula_effects as fx
import native_formula_effects_header as hx

GENERATED = fx.ROOT / hx.OUTPUT
MANIFEST = GENERATED.with_name(GENERATED.name.removesuffix(".h") + hx.MANIFEST_SUFFIX)


def entry(ident, consumed, written, imprecise=(), internal=False, self_only=(), **meta):
    base = {"id": ident, "generated": f"g/formula_{ident}.h", "generated_sha256": f"{ident:064x}",
            "effects": {"status": "ANALYZED", "opaque": False, "consumed": consumed, "written": written,
                        "imprecise": list(imprecise), "self_only": list(self_only)}}
    if internal:
        return {**base, "namespace": "internal"}
    return {**base, "de_option": -1, "param_count": 3, "traits": 32, "wrapper_flags": 1, **meta}


class Header(unittest.TestCase):
    def test_masks_follow_state_word_order_and_imprecise_lanes_count_both_ways(self):
        effects = {"wrapper_sha256": "0" * 64, "internal_formulas": [entry(2, ["j[3]"], ["xyz[0]"], internal=True)],
                   "formulas": [entry(7004, ["xyz[0]", "rout"], ["xyz[1]"], imprecise=["dfree2"])]}
        rows = {(r["namespace"], r["id"]): r for r in hx.entries(effects)}
        ext, internal = rows[(1, 7004)], rows[(0, 2)]
        self.assertEqual(ext["reads"], (1 << 0) | (1 << 24) | (1 << fx.LANES.index("dfree2")))
        self.assertEqual(ext["writes"], (1 << 1) | (1 << fx.LANES.index("dfree2")))
        self.assertEqual((ext["exact"], ext["de_option"], ext["param_count"], ext["traits"]), (True, -1, 3, 32))
        self.assertEqual((internal["exact"], internal["reads"], internal["writes"]), (False, 1 << 11, 1))
        text = hx.render(list(rows.values()), "a" * 64, "b" * 64)
        self.assertIn("{1,7004,true,-1,3,32,1,", text)
        self.assertIn("{0,2,false,0,0,0,0,0x800ull,0x1ull,0x0ull},", text)

    def test_contained_lanes_exclude_imprecise_ones(self):
        lanes = ["derivative[0]", "derivative[1]"]
        effects = {"wrapper_sha256": "0" * 64, "internal_formulas": [],
                   "formulas": [entry(8896, ["xyz[0]", *lanes], ["xyz[0]", *lanes], imprecise=["derivative[1]"],
                                      self_only=lanes)]}
        row = hx.entries(effects)[0]
        self.assertEqual(row["contained"], 1 << fx.LANES.index("derivative[0]"))
        self.assertIn(f",0x{1 << 16:x}ull}},", hx.render([row], "a" * 64, "b" * 64))

    def test_unanalyzed_entries_are_left_unknown(self):
        bad = entry(9, ["xyz[0]"], [])
        bad["effects"]["opaque"] = True
        effects = {"wrapper_sha256": "0" * 64, "internal_formulas": [], "formulas": [bad]}
        self.assertEqual(hx.entries(effects), [])

    def test_result_contract_is_mandatory_and_bound_in_source_manifest(self):
        effects = {"wrapper_sha256": "0"*64, "math_sha256": "2"*64, "translate_add_sha256": "3"*64,
                   "internal_formulas": [], "formulas": []}
        with self.assertRaisesRegex(ValueError, 'result contract identity'):
            hx.sources(effects)
        effects['result_contract_sha256'] = "1"*64
        self.assertIn((str(hx.RESULT_CONTRACT), "1"*64), hx.sources(effects))
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory); path = root / hx.RESULT_CONTRACT
            path.parent.mkdir(parents=True); path.write_text('original')
            pair = [(str(hx.RESULT_CONTRACT), hx.sha256(path))]
            self.assertEqual(hx.stale(root, pair), [])
            path.write_text('changed helper')
            self.assertEqual(hx.stale(root, pair), [str(hx.RESULT_CONTRACT)])

    def test_math_identity_is_mandatory_valid_and_bound_in_manifest(self):
        effects = {'wrapper_sha256': '0'*64, 'result_contract_sha256': '1'*64,
                   'translate_add_sha256': '3'*64, 'internal_formulas': [], 'formulas': []}
        with self.assertRaisesRegex(ValueError, 'math identity'):
            hx.sources(effects)
        for invalid in (None, 0, True, [], b'a'*64, '', 'a'*63, 'a'*65,
                        'A'*64, 'g'*64, 'a'*63+'\n'):
            with self.subTest(invalid=invalid), self.assertRaisesRegex(ValueError, 'math identity'):
                hx.sources({**effects, 'math_sha256': invalid})
        pairs = hx.sources({**effects, 'math_sha256': 'ab'*32})
        self.assertIn((str(hx.MATH), 'ab'*32), pairs)
        self.assertIn((str(hx.WRAPPER), '0'*64), pairs)
        self.assertIn((str(hx.RESULT_CONTRACT), '1'*64), pairs)
        self.assertIn(str(hx.MATH)+'\t'+'ab'*32+'\n', hx.manifest(pairs))

    def test_math_only_change_makes_the_analysis_stale(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            for relative in (hx.WRAPPER, hx.MATH, hx.TRANSLATE_ADD, hx.RESULT_CONTRACT):
                path = root/relative
                path.parent.mkdir(parents=True, exist_ok=True)
                path.write_text('synthetic fixture: '+str(relative))
            effects = {'wrapper_sha256': hx.sha256(root/hx.WRAPPER),
                       'result_contract_sha256': hx.sha256(root/hx.RESULT_CONTRACT),
                       'math_sha256': hx.sha256(root/hx.MATH),
                       'translate_add_sha256': hx.sha256(root/hx.TRANSLATE_ADD),
                       'internal_formulas': [], 'formulas': []}
            pairs = hx.sources(effects)
            self.assertEqual(hx.stale(root, pairs), [])
            (root/hx.MATH).write_text('changed synthetic math helper only')
            self.assertEqual(hx.stale(root, pairs), [str(hx.MATH)])

    def test_translate_add_identity_is_mandatory_valid_and_bound_in_manifest(self):
        effects = {'wrapper_sha256': '0'*64, 'result_contract_sha256': '1'*64,
                   'math_sha256': '2'*64, 'internal_formulas': [], 'formulas': []}
        with self.assertRaisesRegex(ValueError, 'translate add helper identity'):
            hx.sources(effects)
        for invalid in (None, 0, True, [], b'a'*64, '', 'a'*63, 'a'*65,
                        'A'*64, 'g'*64, 'a'*63+'\n'):
            with self.subTest(invalid=invalid), self.assertRaisesRegex(ValueError, 'translate add helper identity'):
                hx.sources({**effects, 'translate_add_sha256': invalid})
        pairs = hx.sources({**effects, 'translate_add_sha256': 'ab'*32})
        self.assertIn((str(hx.TRANSLATE_ADD), 'ab'*32), pairs)
        self.assertIn(str(hx.TRANSLATE_ADD)+'\t'+'ab'*32+'\n', hx.manifest(pairs))

    def test_missing_or_stale_translate_add_cannot_replace_current_header(self):
        for change in ('missing_identity', 'mutated_helper', 'missing_helper'):
            with self.subTest(change=change), tempfile.TemporaryDirectory() as directory:
                root = Path(directory)
                for relative in (hx.WRAPPER, hx.MATH, hx.TRANSLATE_ADD, hx.RESULT_CONTRACT):
                    path = root/relative
                    path.parent.mkdir(parents=True, exist_ok=True)
                    path.write_text('synthetic fixture: '+str(relative))
                effects = {'schema': fx.SCHEMA, 'lanes': list(fx.LANES),
                           'wrapper_sha256': hx.sha256(root/hx.WRAPPER),
                           'result_contract_sha256': hx.sha256(root/hx.RESULT_CONTRACT),
                           'math_sha256': hx.sha256(root/hx.MATH),
                           'translate_add_sha256': hx.sha256(root/hx.TRANSLATE_ADD),
                           'internal_formulas': [], 'formulas': []}
                pairs = hx.sources(effects)
                self.assertEqual(hx.stale(root, pairs), [])
                if change == 'missing_identity':
                    del effects['translate_add_sha256']
                elif change == 'mutated_helper':
                    (root/hx.TRANSLATE_ADD).write_text('changed synthetic translate add only')
                else:
                    (root/hx.TRANSLATE_ADD).unlink()
                if change != 'missing_identity':
                    self.assertEqual(hx.stale(root, pairs), [str(hx.TRANSLATE_ADD)])
                source = root/'effects.json'
                source.write_text(json.dumps(effects))
                output, manifest = root/'current.h', root/'current.sources.tsv'
                output.write_bytes(b'preserved current header')
                manifest.write_bytes(b'preserved current manifest')
                exception = ValueError if change == 'missing_identity' else SystemExit
                pattern = 'translate add helper identity' if change == 'missing_identity' else 'Effects are stale'
                with patch.object(hx, 'ROOT', root), \
                     patch('sys.argv', ['effects-header', '--effects', str(source), '--output', str(output)]), \
                     self.assertRaisesRegex(exception, pattern):
                    hx.main()
                self.assertEqual(output.read_bytes(), b'preserved current header')
                self.assertEqual(manifest.read_bytes(), b'preserved current manifest')

    def test_stale_sources_are_named(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            (root / "a.h").write_text("x")
            pairs = [("a.h", hashlib.sha256(b"x").hexdigest()), ("b.h", "0" * 64)]
            self.assertEqual(hx.stale(root, pairs), ["b.h"])
            (root / "a.h").write_text("y")
            self.assertEqual(hx.stale(root, pairs), ["a.h", "b.h"])


@unittest.skipUnless(GENERATED.is_file() and MANIFEST.is_file(), "generated header absent")
class Committed(unittest.TestCase):
    def test_header_matches_manifest_and_checkout_sources(self):
        manifest = MANIFEST.read_text()
        recorded = re.search(r'kSourcesSha256\[\]="([0-9a-f]{64})"', GENERATED.read_text()).group(1)
        self.assertEqual(hashlib.sha256(manifest.encode()).hexdigest(), recorded)
        pairs = [tuple(line.split("\t")) for line in manifest.splitlines()]
        self.assertIn(str(hx.MATH), {path for path, _ in pairs}, "shared math must be pinned; regenerate effects/header")
        self.assertIn(str(hx.TRANSLATE_ADD), {path for path, _ in pairs},
                      "translate add helper must be pinned; regenerate effects/header")
        self.assertEqual(hx.stale(fx.ROOT, pairs), [], "regenerate effects and the header after source changes")


if __name__ == "__main__":
    unittest.main()
