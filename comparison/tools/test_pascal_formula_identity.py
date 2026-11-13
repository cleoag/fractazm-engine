import unittest
from pathlib import Path
from pascal_formula_identity import validate_formula_load, validate_reference_formula_load


class FormulaIdentityTests(unittest.TestCase):
    scene = r'C:\scenes\jit.m3p'
    slots = [dict(slot=0, number=20, iterations=1, name='JITTrigExample')]

    def test_rejects_disabled_formula_even_when_another_formula_renders(self):
        log = ('[MB3D] Loading parameters: ' + self.scene + '\n'
               '[MB3D]   Formula[0]: iFnr=-1 iItCount=0 name=\n'
               '[MB3D]   Formula[1]: iFnr=0 iItCount=1 name=Integer Power\n'
               '[MB3D] Saved: output.png\n')
        with self.assertRaisesRegex(ValueError, 'load mismatch'):
            validate_formula_load(log, self.scene, self.slots)

    def test_saved_external_id_is_normalized_only_after_successful_load(self):
        slots = [dict(self.slots[0], number=25)]
        start = '[MB3D] Loading parameters: ' + self.scene + '\n'
        with self.assertRaisesRegex(ValueError, 'load mismatch'):
            validate_formula_load(start + 'Formula[0]: iFnr=-1 iItCount=0 name=\n', self.scene, slots)
        result = validate_formula_load(start + 'Formula[0]: iFnr=20 iItCount=1 name=JITTrigExample\n', self.scene, slots)
        self.assertEqual(result['external_slots'], [0])

    def test_batch_scene_isolation_and_last_load(self):
        good = '[MB3D]   Formula[0]: iFnr=20 iItCount=1 name=JITTrigExample\n'
        log = '[MB3D] Loading parameters: c:/scenes/jit.m3p\n' + good
        log += '[MB3D] Loading parameters: C:/scenes/other.m3p\n'
        log += '[MB3D]   Formula[0]: iFnr=-1 iItCount=0 name=\n'
        self.assertTrue(validate_formula_load(log, self.scene, self.slots)['verified'])
        log += '[MB3D] Loading parameters: ' + self.scene + '\n'
        with self.assertRaisesRegex(ValueError, 'load mismatch'):
            validate_formula_load(log, self.scene, self.slots)

    def test_wrong_name_and_missing_log_are_not_evidence(self):
        for log in ['', '[MB3D] Loading parameters: ' + self.scene + '\n'
                    '[MB3D] Formula[0]: iFnr=20 iItCount=1 name=Other']:
            with self.assertRaises(ValueError):
                validate_formula_load(log, self.scene, self.slots)

    def test_invalid_reference_stays_invalid_and_absent_history_stays_unknown(self):
        with self.assertRaisesRegex(ValueError, 'load mismatch'):
            validate_reference_formula_load(Path('.'),
                {'error': 'Pascal reference formula load mismatch: disabled'}, self.scene, self.slots)
        self.assertIsNone(validate_reference_formula_load(Path('.'),
            {'retry_ms': 12, 'batch': 0}, self.scene, self.slots))
