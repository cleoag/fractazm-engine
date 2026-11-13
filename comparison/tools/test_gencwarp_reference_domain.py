"""Negative controls for the exact original-CODE selector-domain proof."""
import unittest

from check_gencwarp_reference_domain import ASSET, prove_domain


class DomainProofTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.code=bytes.fromhex(ASSET.read_text().split('[CODE]')[1].split('[END]')[0])

    def test_complete_original_domain(self):
        proofs=prove_domain(self.code)
        self.assertEqual([p['parameter_offset'] for p in proofs],[-64,-68])
        for proof in proofs:
            self.assertEqual(proof['compared_values'],list(range(24)))
            self.assertEqual(proof['x87_instructions_before_fpatan_on_this_path'],0)

    def test_wrong_selector_source_is_rejected(self):
        code=bytearray(self.code);code[0x6e]=0xbc
        with self.assertRaisesRegex(ValueError,'selector operand'): prove_domain(bytes(code))

    def test_missing_selector_is_rejected(self):
        code=bytearray(self.code);code[0x6f]=24
        with self.assertRaisesRegex(ValueError,'exactly selectors'): prove_domain(bytes(code))

    def test_default_operand_load_is_not_mistaken_for_original(self):
        # NOP on the all-not-equal route becomes FLD1: the proof must fail.
        code=bytearray(self.code);code[0x90:0x92]=bytes.fromhex('d9e8')
        with self.assertRaises(ValueError): prove_domain(bytes(code))

    def test_missing_fpatan_is_rejected(self):
        code=bytearray(self.code);code[0x390:0x392]=bytes.fromhex('d9f1')
        with self.assertRaisesRegex(ValueError,'FPATAN'): prove_domain(bytes(code))

    def test_wrong_replayed_instruction_is_rejected(self):
        code=bytearray(self.code);code[0x394]=0xc4
        with self.assertRaisesRegex(ValueError,'trace site'): prove_domain(bytes(code))


if __name__=='__main__': unittest.main()
