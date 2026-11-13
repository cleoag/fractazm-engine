import json
from pathlib import Path
import sys
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from trace_diff import parse_line, schema_validate, diff_traces, ulp_distance


class TraceValidationTests(unittest.TestCase):
    schema = json.loads((Path(__file__).resolve().parents[1] / 'parity_trace_schema.json').read_text())

    def line(self, payload, stage='normals_raw'):
        return parse_line(f'stage={stage} x=2 y=3 {payload}', 1, 'test')

    def test_duplicate_keys_rejected(self):
        for payload in ('nx=1 nx=2', 'x=4', 'stage=other'):
            with self.assertRaises(ValueError):
                self.line(payload)

    def test_missing_fields_wrong_type_and_forged_hash_rejected(self):
        for payload in ('nx=1', 'nx=1 ny=two nz=3', 'nx=1 ny=2 nz=3'):
            line = self.line(payload + ' checksum=0x0000000000000000')
            self.assertTrue(schema_validate([line], self.schema, 'test'))

    def test_real_emitter_hash_and_order(self):
        # Independently computed FNV-1a64 of b'nx=1\nny=2\nnz=3'.
        value = 14695981039346656037
        for byte in b'nx=1\nny=2\nnz=3':
            value = ((value ^ byte) * 1099511628211) % (2**64)
        suffix = f'checksum=0x{value:016X}'
        self.assertEqual(schema_validate([self.line('nx=1 ny=2 nz=3 ' + suffix)], self.schema, 'test'), [])
        self.assertTrue(schema_validate([self.line('ny=2 nx=1 nz=3 ' + suffix)], self.schema, 'test'))

    def test_tolerance_cannot_accept_checkpoint_or_missing_integer(self):
        for p, c in [('checksum=0x1234', 'checksum=0x1234'),
                     ('currentLevel=1 AmbShadow=5', 'currentLevel=1 AmbShadow=6'),
                     ('currentLevel=1 AmbShadow=5', 'currentLevel=1')]:
            index, _ = diff_traces([self.line(p)], [self.line(c)], 'tolerance', 0, 4)
            self.assertEqual(index, 0)

    def test_signed_ulp_zero_and_nonfinite(self):
        self.assertEqual(ulp_distance('0x8000000000000000', '0x0000000000000000'), 0)
        self.assertEqual(ulp_distance('0x8000000000000001', '0x0000000000000001'), 2)
        self.assertEqual(ulp_distance('0xBFF0000000000001', '0xBFF0000000000000'), 1)
        self.assertIsNone(ulp_distance('0x7FEFFFFFFFFFFFFF', '0x7FF0000000000000'))
        self.assertIsNone(ulp_distance('0x7FF8000000000000', '0x7FF8000000000001'))


if __name__ == '__main__':
    unittest.main()
