"""State-construction, chain-union and comparison fixtures for native_formula_chains.

No device runs; the policy helper case compiles the production headers only.
"""
import shutil, struct, tempfile, unittest
from pathlib import Path
import native_formula_effects as fx
import native_formula_chains as cx

W = len(fx.LANES)


def words(driver=1, de_option=2, julia=0, slots=()):
    w = [0] * cx.PROGRAM_WORDS
    w[0], w[1], w[2], w[6], w[7], w[8] = 1, driver, 6, de_option & 0xFFFFFFFF, 60, julia
    w[16], w[22] = cx.word(16.0), cx.word(4.0)
    w[24:28] = [cx.word(v) for v in (0.5, -0.25, 0.125, 2.0)]
    w[50:56] = [1, 2, 0, 0, 0, 0]
    for n, (ns, ident, traits) in enumerate(slots):
        b = cx.SLOT_BASE + n * cx.SLOT_WORDS
        w[b], w[b + 1], w[b + 2], w[b + 5], w[b + 9] = 1, ns, ident, traits, 1
    return w


def entry(ident, consumed, written, **meta):
    effects = {"status": "ANALYZED", "opaque": False, "consumed": consumed, "written": written, "imprecise": []}
    return {"id": ident, "name": f"F{ident}", "effects": effects, "de_option": 0, "param_count": 0, "traits": 0,
            "wrapper_flags": 0, "difs_style": 0, **meta}


class State(unittest.TestCase):
    def test_initial_state_matches_native_initial_state(self):
        info = cx.program(words(julia=1))
        s = cx.initial_state(info, (0.1, 0.2, 0.3), 0.001)
        self.assertEqual(s[cx.LANE["c[1]"]], cx.word(0.2))
        self.assertEqual(s[cx.LANE["rstop_d"]], cx.word(cx.f32(0.001) * 4.0))
        self.assertEqual(s[cx.LANE["rold"]], s[cx.LANE["rstop_d"]])
        self.assertEqual(s[cx.LANE["rstop"]], cx.word(16.0))
        self.assertEqual((s[cx.LANE["j[3]"]], s[cx.LANE["original_j[3]"]]), (cx.word(2.0), cx.word(2.0)))
        self.assertEqual(s[cx.LANE["derivative[0]"]], cx.word(1.0))
        self.assertEqual((s[cx.LANE["hybrid_counts[1]"]], s[cx.LANE["julia"]], s[cx.LANE["max_iterations"]]), (2, 1, 60))
        for lane in ("xyz[0]", "xyz[3]", "sse[0]", "vary_scale", "tmp", "dfree2", "current_slot"):
            self.assertEqual(s[cx.LANE[lane]], 0, lane)

    def test_query_changes_only_point_and_stop(self):
        info = cx.program(words())
        lane = list(range(W))
        q = cx.query(lane, (1.0, 2.0, 3.0), 0.5, info)
        changed = {fx.LANES[i] for i in range(W) if q[i] != lane[i]}
        self.assertEqual(changed, {"c[0]", "c[1]", "c[2]", "rstop_d"})

    def test_scouts_and_segments_are_deterministic_fp32(self):
        self.assertEqual(cx.scouts(7, 16), cx.scouts(7, 16))
        a = cx.sequences([[0.0, 0.0, 0.0], [1.0, 1.0, 1.0]], [[1.0, 2.0, 3.0], [1.0, 1.0, 1.0]], 5)
        self.assertEqual((len(a), len(a[0]), a[0][0], a[0][-1], a[0][2]), (2, 5, [0.0, 0.0, 0.0], [1.0, 2.0, 3.0], [0.5, 1.0, 1.5]))
        self.assertTrue(all(cx.f32(v) == v for p in cx.scouts(3, 32) for v in p))

    def test_anchors_skip_faults_and_prefer_deep_orbits(self):
        def row(iteration, fault=0, completed=1):
            r = [0] * 59
            r[cx.LANE["iteration"]], r[47], r[56] = iteration, fault, completed
            return r
        points = [[float(i), 0.0, 0.0] for i in range(8)]
        rows = [row(1), row(50, fault=4, completed=0), row(40), row(2), row(3), row(39), row(1), row(1)]
        chosen = cx.anchors(points, rows, 2, 1)
        self.assertEqual(sorted(p[0] for p in chosen), [2.0, 5.0])
        self.assertEqual(cx.anchors(points, [row(9, fault=4)] * 8, 2, 1), [])


class Chain(unittest.TestCase):
    base = ["xyz[0]", "xyz[1]", "xyz[2]", "xyz[3]", "rout", "vary_scale"]

    def test_member_write_of_retained_lane_carries(self):
        index = {(1, 10): entry(10, self.base + ["sse[0]"], ["xyz[0]"]), (1, 11): entry(11, self.base, ["sse[0]"]),
                 (0, 4): entry(4, ["xyz[0]", "j[0]"], ["xyz[0]"])}
        alone = cx.chain(cx.program(words(slots=[(1, 10, 0), (0, 4, 0)])), index)
        self.assertEqual(alone["class"], "B")
        mixed = cx.chain(cx.program(words(slots=[(1, 10, 0), (1, 11, 0), (0, 4, 0)])), index)
        self.assertEqual((mixed["class"], mixed["carried"]), ("C", ["sse[0]"]))
        self.assertEqual([m["namespace"] for m in mixed["members"]], ["external", "external", "internal"])

    def test_history_lanes_exclude_query_driver_live_and_retained(self):
        index = {(1, 10): entry(10, self.base + ["sse[0]"], ["xyz[0]"])}
        result = cx.chain(cx.program(words(slots=[(1, 10, 0)])), index)
        lanes = set(cx.history_lanes(result, result["driver"], result["julia"]))
        self.assertNotIn("sse[0]", lanes)          # retained read stays at its initial value
        self.assertIn("sse[1]", lanes)
        self.assertTrue({"tmp", "dfree2", "derivative[2]"} <= lanes)
        self.assertFalse(lanes & (cx.QUERY_SETS | fx.LIVE_INPUTS))
        self.assertTrue({"xyz[0]", "rout", "vary_scale"} <= lanes)  # orbit entry must overwrite these

    def test_adversarial_accepts_own_poison_pass_through_only(self):
        def r(inputs, outputs):
            return {"input_words": inputs, "result_words": outputs}
        base = [0] * 59
        nan, rnd = list(base), list(base)
        nan[cx.LANE["tmp"]], rnd[cx.LANE["tmp"]] = 7, 9
        rows = [r(base[:W], base), r(nan[:W], nan), r(rnd[:W], rnd)]
        self.assertEqual(cx.adversarial_failures(rows, 1), [])
        rnd_out = list(rnd); rnd_out[55] = 3
        rows[2] = r(rnd[:W], rnd_out)
        self.assertEqual(cx.adversarial_failures(rows, 1), [{"point": 0, "variant": 2, "words": ["distance"]}])

    def test_carried_variant_sets_only_driver_lanes(self):
        s = [0] * W
        v = cx.carried_variant(s, ["dfree2", "tmp"])
        self.assertEqual({fx.LANES[i] for i in range(W) if v[i] != s[i]}, {"dfree2", "tmp"})
        self.assertEqual(cx.carried_variant(s, ["dfree2"])[cx.LANE["tmp"]], 0)

    def test_metadata_mismatch_is_unknown(self):
        index = {(1, 10): entry(10, self.base, ["xyz[0]"])}
        result = cx.chain(cx.program(words(slots=[(1, 10, 576)])), index)
        self.assertEqual(result["class"], "U")
        self.assertEqual(result["metadata_mismatch"][0]["program"][2], 576)

    def test_compare_names_state_and_record_words(self):
        a, b = [0] * 59, [0] * 59
        b[cx.LANE["tmp"]], b[55] = 1, 2
        self.assertEqual(cx.compare(a, b), ["tmp", "distance"])


class HostPersistence(unittest.TestCase):
    def test_internal_only_3d_chains_are_legacy_pixels(self):
        internal = words(driver=0, de_option=0, slots=[(0, 2, 0), (0, 0, 0)])
        self.assertFalse(cx.legacy_persistent(internal, False))
        self.assertEqual(cx.policy_label({"independent_primary_pixels": False, "legacy_formula_persistent": False,
                                          "production_chain": 0, "contract_independent": True}), "legacy-pixels")
        self.assertTrue(cx.legacy_persistent(words(driver=0, de_option=0, slots=[(0, 0, 0), (1, 7004, 32)]), False))
        self.assertFalse(cx.legacy_persistent(words(driver=1, slots=[(1, 10186, 0)]), True))

    def test_independent_contract_replaces_only_the_legacy_order(self):
        base = {"independent_primary_pixels": False, "legacy_formula_persistent": True, "production_chain": 0}
        self.assertEqual(cx.policy_label({**base, "contract_independent": True}), "state-pixels")
        self.assertEqual(cx.policy_label({**base, "contract_independent": False}), "ordered")
        self.assertEqual(cx.policy_label({**base, "production_chain": 2, "contract_independent": True}), "contract2")

    def test_driver_and_derivative_mode_force_persistence(self):
        for driver in (2, 3, 6, 7, 8, 9):
            self.assertTrue(cx.legacy_persistent(words(driver=driver, slots=[(0, 4, 0)]), False), driver)
        self.assertTrue(cx.legacy_persistent(words(driver=1, de_option=32 | 2, slots=[(0, 4, 0)]), False))
        self.assertFalse(cx.legacy_persistent(words(driver=1, de_option=16 | 2, slots=[(0, 4, 0)]), False))

    def test_mirrored_rule_matches_formula_host_source(self):
        source = "".join((cx.ROOT / "src/render/native_metal/formula_host.cpp").read_text().split())
        for line in cx.HOST_PERSISTENCE_RULE:
            self.assertIn("".join(line.split()), source)


@unittest.skipUnless(shutil.which("xcrun"), "toolchain unavailable")
class Policy(unittest.TestCase):
    def test_policy_helper_runs_production_headers(self):
        import subprocess, json
        with tempfile.TemporaryDirectory() as directory:
            out = Path(directory)
            binary = cx.build_policy(out)
            program = words(driver=1, de_option=2, slots=[(1, 10186, 0)])
            b = cx.SLOT_BASE
            program[3:6] = [0, 0, 0]
            program[b + 4], program[b + 3], program[b + 6] = 19, 2, 1
            program[50:56] = [1, 0, 0, 0, 0, 0]
            (out / "p.bin").write_bytes(struct.pack("<2048I", *program))
            result = json.loads(subprocess.run([str(binary), str(out / "p.bin")], capture_output=True, text=True).stdout)
        # The closed Amazing Surf 2 singleton in IndependentPrimaryPixels.
        self.assertTrue(result["independent_primary_pixels"])


if __name__ == "__main__":
    unittest.main()
