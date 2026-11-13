"""A strict unsupported-input rejection is not a numerical result."""
import subprocess
import unittest
from unittest.mock import patch

import check_formula_scene_points as checker


class FormulaInputRejectionTests(unittest.TestCase):
    def test_strict_rejection_preserves_diagnostic_without_parsing_numeric_output(self):
        diagnostic = ('FORMULA_INPUT_REJECTED[caller_dependent_flip] slot 1 '
                      'gnarlyIFS: original caller-stack dependency.\n')
        result = subprocess.CompletedProcess(['probe'], 12, 'not numeric', diagnostic)
        with patch.object(checker.subprocess, 'run', return_value=result):
            with self.assertRaises(checker.FormulaInputRejected) as caught:
                checker.execute(['probe'], 'input')
        self.assertEqual(str(caught.exception), diagnostic.strip())

    def test_both_zero_power_diagnostics_are_retained(self):
        diagnostic = ('FORMULA_INPUT_REJECTED[nonterminating_zero_power] Integer power R\n'
                      'FORMULA_INPUT_REJECTED[nonterminating_zero_power] Integer power Angle\n')
        result = subprocess.CompletedProcess(['probe'], 12, '', diagnostic)
        with patch.object(checker.subprocess, 'run', return_value=result):
            with self.assertRaises(checker.FormulaInputRejected) as caught:
                checker.execute(['probe'], 'input')
        self.assertIn('Integer power R', str(caught.exception))
        self.assertIn('Integer power Angle', str(caught.exception))

    def test_unrelated_failure_is_not_relabelled_as_unsupported_input(self):
        for code, diagnostic in ((12, 'unrelated failure'),
                                  (1, 'FORMULA_INPUT_REJECTED[unknown_external_formula] x')):
            with self.subTest(code=code, diagnostic=diagnostic):
                result = subprocess.CompletedProcess(['probe'], code, '', diagnostic)
                with patch.object(checker.subprocess, 'run', return_value=result):
                    with self.assertRaises(RuntimeError) as caught:
                        checker.execute(['probe'], 'input')
                self.assertNotIsInstance(caught.exception, checker.FormulaInputRejected)

    def test_successful_numeric_output_is_unchanged(self):
        result = subprocess.CompletedProcess(['probe'], 0, '1 2 3 4\n5 6 7 8\n', '')
        with patch.object(checker.subprocess, 'run', return_value=result):
            self.assertEqual(checker.execute(['probe'], 'input', lanes=4),
                             [[1, 2, 3, 4], [5, 6, 7, 8]])


if __name__ == '__main__':
    unittest.main()
