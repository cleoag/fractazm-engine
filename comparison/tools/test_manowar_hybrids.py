"""Negative controls for caller evidence; no render processes are launched."""
import unittest
from check_manowar_hybrids import incoming_mismatches,validate_rows,consumed_parameter_differences
class CallerEvidence(unittest.TestCase):
 def base(self):return [0.]*14+[1.,0.,.25,.5,.75,1.]
 def test_zero_weight_consumed_lane_fails(self):
  p=self.base();c=p[:];c[16]+=1
  self.assertEqual(incoming_mismatches(c,p,['ManowarBulb','CommQuat']),['manowar_incoming_lanes'])
 def test_identical_lanes_pass(self):
  p=self.base();self.assertEqual(incoming_mismatches(p,p,['ManowarBulb','CommQuat']),[])
 def test_slot_order_fails(self):
  p=self.base();c=p[:];c[15]=1;self.assertEqual(incoming_mismatches(c,p,['ManowarBulb','CommQuat']),['slot_order'])
 def test_truncated_trace_rejected(self):
  with self.assertRaises(ValueError):validate_rows({'pas_trace':[self.base()[:-1]]},1)
 def test_overlong_plain_rejected(self):
  with self.assertRaises(ValueError):validate_rows({'cpp_plain':[[0.]*16]},1)
 def test_wrong_row_count_rejected(self):
  with self.assertRaises(ValueError):validate_rows({'pas_repeat':[]},1)
 def test_valid_rows(self):
  validate_rows({'cpp_plain':[[0.]*15],'pas_trace':[self.base()]},1)
 def test_positive_epsilon_is_consumed(self):
  p=bytes(512);c=bytearray(1024);c[512]=1
  self.assertEqual(consumed_parameter_differences('ABoxMod4d',p,c),([0],[0]))
 def test_unused_positive_constant_is_retained_raw(self):
  p=bytes(512);c=bytearray(1024);c[512]=1
  self.assertEqual(consumed_parameter_differences('ManowarBulb',p,c),([0],[]))
if __name__=='__main__':unittest.main()
