"""Join fixtures for native_formula_effects_report; no device or compiler."""
import unittest
import native_formula_effects as fx
import native_formula_effects_report as rx


def formula(ident, consumed, written, driver="Hybrid3D"):
    effects = {"status": "ANALYZED", "opaque": False, "consumed": consumed, "written": written, "imprecise": []}
    classes = {rx.key(d, j): fx.classify(effects, d, j) for d, j in fx.DRIVERS}
    return {"id": ident, "name": f"F{ident}", "family": "3D", "effects": effects, "classes": classes}


def scene(name, slots, driver="Hybrid3D", status="COMPLETE", orbit="PASS"):
    return {"scene": f"comparison/scenes/{name}.m3p", "status": status, "driver": driver, "julia": False,
            "slots": slots, "orbit": {"status": orbit}, "error": "map is unavailable"}


class Join(unittest.TestCase):
    def test_classes_counted_by_scene_driver_and_device_outcome(self):
        base = ["xyz[0]", "xyz[1]", "xyz[2]", "xyz[3]", "rout", "vary_scale"]
        effects = {"formulas": [formula(1, base, ["xyz[0]"]), formula(2, base + ["sse[0]"], ["sse[0]"]),
                                formula(3, base, [])]}
        poison = {"results": [
            scene("a", [{"slot": 0, "id": 1, "status": "PASS", "control": {"observed": base, "unobserved": []}}]),
            scene("b", [{"slot": 0, "id": 2, "status": "PASS",
                         "control": {"observed": base, "unobserved": ["sse[0]"]}}], driver="Hybrid3DAnalytic"),
            scene("c", [], status="PREPARATION_FAILED")]}
        rows, counts, skipped = rx.build(effects, poison)
        self.assertEqual(counts, {"Hybrid3D": {"A": {"device": 1}}, "Hybrid3DAnalytic": {"C": {"device": 1}}})
        self.assertEqual(skipped, {"c": "map is unavailable"})
        by_id = {r["id"]: r for r in rows}
        self.assertEqual(by_id[3]["probes"], [])
        self.assertEqual(by_id[3]["untested"], "no extall scene")
        probe = by_id[2]["probes"][0]
        self.assertEqual((probe["class"], probe["carried"], probe["retained_observed"]), ("C", ["sse[0]"], []))
        self.assertEqual(probe["consumed_unobserved"], ["sse[0]"])

    def test_untested_reason_names_failed_or_substituted_scene(self):
        skipped, loaded = {"extall_3Da_Amazing_Surf_2": "FP32 range"}, {"extall_Ads__FlipXY": [7004]}
        self.assertEqual(rx.untested_reason("Amazing Surf 2", skipped, loaded), "extall_3Da_Amazing_Surf_2: FP32 range")
        self.assertEqual(rx.untested_reason("_FlipXY", skipped, loaded),
                         "extall_Ads__FlipXY: scene loads formula IDs [7004]")

    def test_orbit_failure_is_not_device_evidence(self):
        effects = {"formulas": [formula(1, ["xyz[0]"], ["xyz[0]"])]}
        poison = {"results": [scene("a", [{"slot": 0, "id": 1, "status": "PASS", "control": {}}], orbit="FAIL")]}
        self.assertEqual(rx.build(effects, poison)[1], {"Hybrid3D": {"A": {"step:PASS/orbit:FAIL": 1}}})


if __name__ == "__main__":
    unittest.main()
