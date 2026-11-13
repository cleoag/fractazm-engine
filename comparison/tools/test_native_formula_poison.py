"""Corpus and comparison fixtures for native_formula_poison.

These check the comparison rules only; they run no device code.
"""
import unittest
import native_formula_effects as fx
import native_formula_poison as px

W = px.STATE_WORDS


def row(inputs, outputs, tail=None):
    return {"input_words": list(inputs), "result_words": list(outputs) + list(tail or [0] * (px.RESULT_WORDS - W))}


def effects(consumed, written):
    return {"consumed": list(consumed), "written": list(written)}


class Corpus(unittest.TestCase):
    def test_variants_poison_only_requested_lanes_with_distinct_patterns(self):
        base = list(range(W))
        rows = px.variants(base, ["rout", "tmp"], 7)
        self.assertEqual(len(rows), 3)
        self.assertEqual(rows[0], base)
        for k in (1, 2):
            changed = {fx.LANES[i] for i in range(W) if rows[k][i] != base[i]}
            self.assertEqual(changed, {"rout", "tmp"})
        self.assertNotEqual(rows[1][px.LANE["rout"]], rows[2][px.LANE["rout"]])
        self.assertEqual(px.variants(base, ["rout"], 7), px.variants(base, ["rout"], 7))

    def test_program_header_and_slots(self):
        words = [0] * 2048
        words[1], words[6], words[8] = 1, 0xFFFFFFFF, 1
        words[56 + 332 + 1], words[56 + 332 + 2], words[56 + 332 + 9] = 1, 7098, 1
        info = px.program_info(words)
        self.assertEqual((info["driver"], info["de_option"], info["julia"]), (1, -1, 1))
        self.assertEqual(info["slots"][1], {"namespace": 1, "id": 7098, "enabled": 1})
        self.assertEqual(px.driver_variant(info), ("Hybrid3DAnalytic", True))
        words[6] = 32
        self.assertEqual(px.driver_variant(px.program_info(words))[0], "Hybrid3DAnalytic/xyz-derivative")

    def test_orbit_entry_lanes_exclude_query_and_live_inputs(self):
        lanes = set(px.orbit_entry_lanes("Hybrid3DAnalytic", False))
        self.assertTrue({"xyz[0]", "rout", "vary_scale", "first"} <= lanes)
        self.assertFalse(lanes & (fx.QUERY_INIT | fx.LIVE_INPUTS | {"current_slot", "rold"}))
        self.assertNotIn("vary_scale", px.orbit_entry_lanes("Hybrid4DAnalytic", False))
        self.assertNotIn("tmp", px.orbit_entry_lanes("HybridDifs", False))
        self.assertNotIn("xyz[0]", px.orbit_entry_lanes("HybridDifsNoInitialization", False))


class StepComparison(unittest.TestCase):
    def group(self, consumed, written, mutate):
        base = [0x3F800000 + i for i in range(W)]
        rows = px.variants(base, [n for n in fx.LANES if n not in consumed and n not in px.STEP_READS], 3)
        out = []
        for k, inputs in enumerate(rows):
            outputs = list(inputs)
            mutate(k, inputs, outputs)
            out.append(row(inputs, outputs))
        return out

    def test_passes_when_written_lanes_depend_only_on_consumed_lanes(self):
        def step(k, i, o):
            o[px.LANE["xyz[0]"]] = i[px.LANE["xyz[1]"]] + 1
            o[px.LANE["current_slot"]] = 2
        rows = self.group({"xyz[1]"}, {"xyz[0]"}, step)
        self.assertEqual(px.compare_step(rows, effects(["xyz[1]"], ["xyz[0]"]), 1), [])

    def test_hidden_read_of_poisoned_lane_fails(self):
        def step(k, i, o):
            o[px.LANE["xyz[0]"]] = i[px.LANE["dfree1"]] & 0xFFFF
        rows = self.group(set(), {"xyz[0]"}, step)
        failures = px.compare_step(rows, effects([], ["xyz[0]"]), 1)
        self.assertEqual({f["lane"] for f in failures}, {"xyz[0]"})

    def test_hidden_write_fails(self):
        def step(k, i, o):
            o[px.LANE["tmp"]] = 5
        rows = self.group(set(), set(), step)
        self.assertEqual({f["lane"] for f in px.compare_step(rows, effects([], []), 1)}, {"tmp"})

    def test_static_write_that_keeps_poisoned_input_is_accepted(self):
        # A conditional write that did not fire must still carry the variant's own word.
        def step(k, i, o):
            pass
        rows = self.group(set(), {"sse[0]"}, step)
        self.assertEqual(px.compare_step(rows, effects([], ["sse[0]"]), 1), [])

    def test_fault_or_work_difference_fails(self):
        rows = self.group(set(), set(), lambda k, i, o: None)
        rows[2]["result_words"][W + 1] = 9
        failures = px.compare_step(rows, effects([], []), 1)
        self.assertEqual([f["field"] for f in failures], ["fault/distance/completed"])


class Control(unittest.TestCase):
    def test_nudge_is_finite_and_different(self):
        import struct
        for word in (0, 0x3F800000, 0xBF800000, 0x7FC00000, 0x7F800000, 5):
            value = struct.unpack("<f", struct.pack("<I", px.nudge(word)))[0]
            self.assertTrue(value == value and abs(value) < float("inf"))
            self.assertNotEqual(px.nudge(word), word)

    def test_observed_lanes_ignore_own_pass_through(self):
        base_in = [0x3F800000] * W
        base = row(base_in, base_in)
        lanes = ["rout", "tmp"]
        rows = []
        for lane, inputs in zip([l for l in lanes for _ in (0, 1)], px.lane_variants(base_in, lanes)):
            outputs = list(inputs)  # every lane passes through
            if lane == "rout":
                outputs[px.LANE["xyz[0]"]] = inputs[px.LANE["rout"]]
            rows.append(row(inputs, outputs))
        self.assertEqual(px.observed_lanes(base, rows, lanes), {"rout"})


    def test_contained_lane_may_change_only_its_own_word(self):
        base_in = [0x3F800000] * W
        base = row(base_in, base_in)
        lanes = ["derivative[0]", "dfree1"]
        rows = []
        for lane, inputs in zip([l for l in lanes for _ in (0, 1)], px.lane_variants(base_in, lanes)):
            outputs = list(base_in)
            outputs[px.LANE[lane]] = inputs[px.LANE[lane]] ^ 1  # rescaled in place
            if lane == "dfree1":
                outputs[px.LANE["xyz[0]"]] = 7
            rows.append(row(inputs, outputs))
        self.assertEqual(px.contained_violations(base, rows, lanes, {"derivative[0]"}), [])
        found = px.contained_violations(base, rows, lanes, {"derivative[0]", "dfree1"})
        self.assertEqual([(v["lane"], v["variant"], v["changed"]) for v in found],
                         [("dfree1", "nudge", ["xyz[0]"]), ("dfree1", "nan", ["xyz[0]"])])


class OrbitComparison(unittest.TestCase):
    def test_any_record_difference_fails(self):
        base = row([0] * W, [1] * W)
        rows = [base, row([0] * W, [1] * W), row([0] * W, [1] * (W - 1) + [2])]
        failures = px.compare_orbit(rows, 1)
        self.assertEqual(failures, [{"point": 0, "variant": 2, "differences": [fx.LANES[-1]]}])


if __name__ == "__main__":
    unittest.main()
