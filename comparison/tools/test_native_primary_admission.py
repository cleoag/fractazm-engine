"""Profile normalization and status fixtures for native_primary_admission; no device runs."""
import tempfile
import unittest
from pathlib import Path
from PIL import Image
import native_primary_admission as ax


def frame(reason, rgb="a", **execution):
    return {"event": {"exit_code": 0, "rgb_sha256": rgb, "seconds": 1.0, "over_diagnostic_threshold": False},
            "profile": {"backend": "metal", "backend_fallback": False, "width": 4, "height": 3,
                        "metal_execution": {"engine": "general_formula", "primary_state_reason": reason, **execution}}}


def fault(reason, error="Native primary formula failed at pixel 7 code=4"):
    return {"event": {"exit_code": 3, "seconds": 1.0, "over_diagnostic_threshold": False},
            "profile": {"backend": "metal", "backend_fallback": False, "error": error,
                        "metal_execution": {"engine": "general_formula", "primary_state_reason": reason}}}


class Fields(unittest.TestCase):
    def test_timings_and_output_path_are_not_work_fields(self):
        profile = {"output": "/a/image.png", "render_total_seconds": 1.5, "clock": "steady_clock wall seconds",
                   "stages": {"primary": {"seconds": 0.2, "status": "completed"}}, "stages_seconds": {"ao": 1.0},
                   "metal_execution": {"primary_state_reason": 5, "ao_gpu_seconds": 2.0, "primary_dispatches": 3}}
        self.assertEqual(ax.work_fields(profile), {
            "clock": "steady_clock wall seconds", "stages.primary.status": "completed",
            "metal_execution.primary_state_reason": 5, "metal_execution.primary_dispatches": 3})

    def test_pipeline_archive_counts_are_not_work_fields(self):
        profile = {"metal_execution": {"pipeline_archive": [20, 1], "primary_dispatches": 3}}
        self.assertEqual(ax.work_fields(profile), {"metal_execution.primary_dispatches": 3})

    def test_rgb_difference_counts_pixels_and_largest_channel(self):
        with tempfile.TemporaryDirectory() as directory:
            a, b = Path(directory) / "a.png", Path(directory) / "b.png"
            Image.new("RGB", (3, 2), (10, 20, 30)).save(a)
            image = Image.new("RGB", (3, 2), (10, 20, 30))
            image.putpixel((0, 0), (11, 20, 30))
            image.putpixel((2, 1), (10, 20, 27))
            image.save(b)
            self.assertEqual(ax.rgb_difference(a, b), {"pixels": 2, "max": 3})

    def test_differences_include_missing_keys(self):
        self.assertEqual(ax.differences({"a": 1, "b": 2}, {"a": 1, "c": 3}), ["b", "c"])

    def test_only_general_metal_runs_are_usable(self):
        good = frame(5)
        self.assertTrue(ax.usable(good["event"], good["profile"]))
        self.assertFalse(ax.usable(good["event"], {**good["profile"], "backend_fallback": True}))
        self.assertFalse(ax.usable(good["event"], {**good["profile"], "metal_execution": {"engine": "integer_power"}}))
        self.assertFalse(ax.usable({"exit_code": 0}, good["profile"]))


class Status(unittest.TestCase):
    def test_admission_may_change_only_primary_scheduling(self):
        runs = {"baseline": frame(3, primary_dispatches=985, primary_formula_reads=0),
                "candidate-1": frame(5, primary_dispatches=12, primary_formula_reads=9),
                "candidate-2": frame(5, primary_dispatches=12, primary_formula_reads=9)}
        self.assertEqual(ax.evaluate(1, "s", runs)["status"], "ADMITTED_MATCH")
        runs["candidate-2"] = frame(5, "b", primary_dispatches=12, primary_formula_reads=9)
        self.assertEqual(ax.evaluate(1, "s", runs)["status"], "DIFFERENT")

    def test_unchanged_ownership_may_change_only_contract_records(self):
        runs = {"baseline": frame(2, primary_formula_reads=0), "candidate-1": frame(2, primary_formula_reads=9)}
        self.assertEqual(ax.evaluate(1, "s", runs)["status"], "UNCHANGED_MATCH")
        runs = {"baseline": frame(2, primary_dispatches=1), "candidate-1": frame(2, primary_dispatches=2)}
        result = ax.evaluate(1, "s", runs)
        self.assertEqual((result["status"], result["checks"]["undeclared_fields"]),
                         ("DIFFERENT", ["metal_execution.primary_dispatches"]))

    def test_conditional_admission_may_change_shadow_rows(self):
        runs = {"baseline": frame(3, shadow_logical_tasks=24, difs_conditional_pixels=False),
                "candidate-1": frame(5, shadow_logical_tasks=120, difs_conditional_pixels=True)}
        self.assertEqual(ax.evaluate(1, "s", runs)["status"], "ADMITTED_MATCH")
        runs["candidate-1"] = frame(3, shadow_logical_tasks=120)
        self.assertEqual(ax.evaluate(1, "s", runs)["status"], "DIFFERENT")

    def test_carried_rerun_may_record_only_the_discarded_attempt(self):
        runs = {"baseline": frame(3, primary_driver_may_write=1),
                "candidate-1": frame(3, primary_driver_may_write=7, difs_rerun_carried_reads=9,
                                     difs_rerun_carried_pixels=2)}
        self.assertEqual(ax.evaluate(1, "s", runs)["status"], "RERUN_MATCH")
        runs["candidate-1"] = frame(3, difs_rerun_carried_pixels=2)
        self.assertEqual(ax.evaluate(1, "s", runs)["status"], "DIFFERENT")

    def test_fresh_lane_control_marks_a_kernel_difference(self):
        def runs(control):
            return {"baseline": frame(3, primary_dispatches=985), "control": control,
                    "candidate-1": frame(5, "b", primary_dispatches=12),
                    "candidate-2": frame(5, "b", primary_dispatches=12)}
        control = frame(3, primary_dispatches=985)
        control["profile"]["metal_library"] = "fresh.metallib"
        self.assertEqual(ax.evaluate(1, "s", runs(control))["status"], "FP32_KERNEL_DIFFERENCE")
        self.assertEqual(ax.evaluate(1, "s", runs(frame(3, "c", primary_dispatches=985)))["status"], "DIFFERENT")
        self.assertEqual(ax.evaluate(1, "s", runs(frame(3, primary_dispatches=984)))["status"], "DIFFERENT")
        unadmitted = runs(frame(3, primary_dispatches=985))
        unadmitted["candidate-1"] = unadmitted["candidate-2"] = frame(3, "b", primary_dispatches=985)
        self.assertEqual(ax.evaluate(1, "s", unadmitted)["status"], "DIFFERENT")
        repeats = runs(frame(3, primary_dispatches=985))
        repeats["candidate-2"] = frame(5, "d", primary_dispatches=12)
        self.assertEqual(ax.evaluate(1, "s", repeats)["status"], "DIFFERENT")
        without = runs(control)
        del without["control"]
        self.assertEqual(ax.evaluate(1, "s", without)["status"], "DIFFERENT")

    def test_identical_gpu_faults_match(self):
        runs = {"baseline": fault(3), "candidate-1": fault(5), "candidate-2": fault(5)}
        self.assertEqual(ax.evaluate(1, "s", runs)["status"], "FAULTS_MATCH")
        runs["candidate-2"] = fault(5, "Native primary formula failed at pixel 9 code=4")
        self.assertEqual(ax.evaluate(1, "s", runs)["status"], "DIFFERENT_FAULTS")

    def test_admitted_pixels_may_report_an_earlier_fault_pixel(self):
        late = "Native primary formula failed at pixel 1343 code=4 formula=0"
        early = "Native primary formula failed at pixel 1030 code=4 formula=0"
        runs = {"baseline": fault(3, late), "candidate-1": fault(5, early), "candidate-2": fault(5, early)}
        self.assertEqual(ax.evaluate(1, "s", runs)["status"], "FAULT_ORDER_ONLY")
        runs = {"baseline": fault(3, early), "candidate-1": fault(5, late), "candidate-2": fault(5, late)}
        self.assertEqual(ax.evaluate(1, "s", runs)["status"], "DIFFERENT_FAULTS")
        runs = {"baseline": fault(3, late), "candidate-1": fault(5, early.replace("code=4", "code=2")),
                "candidate-2": fault(5, early.replace("code=4", "code=2"))}
        self.assertEqual(ax.evaluate(1, "s", runs)["status"], "DIFFERENT_FAULTS")

    def test_baseline_timeout_keeps_candidate_repeats(self):
        timed = {"event": {"timed_out": True, "seconds": 300.0, "over_diagnostic_threshold": True}, "profile": {}}
        runs = {"baseline": timed, "candidate-1": frame(5), "candidate-2": frame(5)}
        result = ax.evaluate(1, "s", runs)
        self.assertEqual((result["status"], result["checks"]), ("BASELINE_TIMED_OUT", {"candidate_repeats_exact": True}))
        runs["candidate-2"] = timed
        self.assertEqual(ax.evaluate(1, "s", runs)["status"], "TIMED_OUT")


if __name__ == "__main__":
    unittest.main()
