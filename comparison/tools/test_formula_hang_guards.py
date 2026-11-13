"""Fail-closed evidence checks for the modified-reference hang audit."""
import subprocess
import tempfile
import unittest
from pathlib import Path
from unittest.mock import patch

from PIL import Image

import check_formula_hang_guards as checker
from check_guarded_pascal_controls import validate_cases


class HangGuardEvidenceTests(unittest.TestCase):
    def test_finite_controls_require_complete_distinct_inventory(self):
        cases = [dict(id=str(i)) for i in range(65)]
        validate_cases(cases)
        for invalid in ([], cases[:-1], cases[:-1] + [cases[0]]):
            with self.subTest(count=len(invalid)):
                with self.assertRaisesRegex(ValueError, "65 distinct"):
                    validate_cases(invalid)

    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        scene = self.root / "scene.m3p"
        scene.write_bytes(b"fixture")
        self.case = dict(id="case", scene=str(scene), sha256=checker.sha(scene),
                         expect="rejected", diagnostics={"cpp": ["work budget exhausted"]},
                         exit_codes={"cpp": [3]})

    def invoke(self, code, message, image=False):
        def execute(*args, **kwargs):
            if image:
                Image.new("RGB", (2, 2)).save(self.root / "cpp_t1_case.png")
            return subprocess.CompletedProcess(args[0], code, "", message)
        with patch.object(checker.subprocess, "run", side_effect=execute):
            return checker.run(Path("candidate.exe"), "cpp", self.case, 1, self.root, 1)

    def test_only_explicit_clean_failure_is_accepted(self):
        for code, message in [(0, "work budget exhausted"), (3, "other error"),
                              (-1, "work budget exhausted")]:
            with self.subTest(code=code, message=message):
                self.assertFalse(self.invoke(code, message)["contract_pass"])
        row = self.invoke(3, "work budget exhausted")
        self.assertTrue(row["contract_pass"])
        self.assertEqual(row["parity_status"], "unverified_original_nontermination")

    def test_partial_image_is_a_failure(self):
        self.assertFalse(self.invoke(3, "work budget exhausted", image=True)["contract_pass"])

    def test_timeout_is_a_failure(self):
        with patch.object(checker.subprocess, "run", side_effect=subprocess.TimeoutExpired("render", 1)):
            row = checker.run(Path("candidate.exe"), "cpp", self.case, 1, self.root, 1)
        self.assertFalse(row["contract_pass"])
        self.assertEqual(row["error"], "timeout")

    def test_changed_scene_is_not_run(self):
        Path(self.case["scene"]).write_bytes(b"changed")
        with patch.object(checker.subprocess, "run") as execute:
            with self.assertRaisesRegex(ValueError, "Recorded input changed"):
                checker.run(Path("candidate.exe"), "cpp", self.case, 1, self.root, 1)
            execute.assert_not_called()

    def test_successful_control_requires_matching_pixels(self):
        ref = self.root / "reference.png"
        Image.new("RGB", (2, 2), "white").save(ref)
        self.case.update(expect="rendered", reference_image=str(ref))
        self.assertFalse(self.invoke(0, "", image=True)["contract_pass"])
        (self.root / "cpp_t1_case.png").unlink()
        Image.new("RGB", (2, 2)).save(ref)
        self.assertTrue(self.invoke(0, "", image=True)["contract_pass"])


if __name__ == "__main__":
    unittest.main()
