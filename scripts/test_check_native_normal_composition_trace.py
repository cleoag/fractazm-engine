#!/usr/bin/env python3
import unittest
from check_native_normal_composition_trace import validate_case,TRACE_WORDS

def fixture():
 lane=[0]*47;lane[28]=0x40000000;g=[0]*28;n=[1,0,0,0,0,0,0,0];trace=[14,0]
 for k in range(7):
  point=[k+1,k+2,k+3];before=[1,1,1,*point,0x3c23d70a,*lane,*g,*n,0,0]
  lane[4:7]=point;lane[32]=0x3f800000;g[4]+=1;g[5]+=1;n[2]+=1;n[3]+=1
  after=[2,1,1,*point,0x3c23d70a,*lane,*g,*n,0x3f800000,0x3f800000];trace+=before+after
 lane[4:7]=[1,2,3];n[5]=1
 result=[0,0,0,0x3f800000,0,0x40000000,0,1,0,0,0xffff8001,0,0,0,0,0]
 return result+lane+g+n,trace
class NormalCompositionTraceTests(unittest.TestCase):
 def test_complete_chain(self):
  out,trace=fixture();result=validate_case(tuple(out),tuple(trace));self.assertEqual(result['normal_samples'],7);self.assertEqual(result['canonical_boundaries'],14)
 def reject(self,mutate):
  out,trace=fixture();mutate(out,trace)
  with self.assertRaises(ValueError):validate_case(tuple(out),tuple(trace))
 def test_duplicate_accounting(self):self.reject(lambda o,t:t.__setitem__(2+92+85,2))
 def test_changed_point(self):self.reject(lambda o,t:t.__setitem__(2+92+3,999))
 def test_lost_carried_state(self):self.reject(lambda o,t:t.__setitem__(2+184+7,999))
 def test_wrong_first_smoothing(self):self.reject(lambda o,t:t.__setitem__(2+2,0))
 def test_missing_center_restore(self):self.reject(lambda o,t:o.__setitem__(16+4,999))
 def test_overflow(self):self.reject(lambda o,t:t.__setitem__(1,1))
 def test_fault_cannot_complete(self):self.reject(lambda o,t:o.__setitem__(11,4))
 def test_range_exits_survive_parent_completion(self):
  out,trace=fixture();out[63+18]=3;trace[2+13*92+54+18]=3
  self.assertEqual(validate_case(tuple(out),tuple(trace))['range_exits'],3)
 def test_parent_cannot_drop_range_counter(self):
  self.reject(lambda o,t:t.__setitem__(2+13*92+54+18,3))
 def test_guarded_parent(self):
  out=[0]*99;out[11]=out[64]=out[92]=1
  self.assertEqual(validate_case(tuple(out),(0,)*TRACE_WORDS)['child_calls'],0)
 def test_guarded_parent_cannot_publish(self):
  out=[0]*99;out[7]=out[96]=1
  with self.assertRaises(ValueError):validate_case(tuple(out),(0,)*TRACE_WORDS)
if __name__=='__main__':unittest.main()
