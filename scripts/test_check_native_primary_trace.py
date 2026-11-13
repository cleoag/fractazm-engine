#!/usr/bin/env python3
import struct,unittest
from check_native_primary_trace import validate,compare,TRACE_WORDS,SNAPSHOT_WORDS

def fixture(site=1):
 words=[0]*TRACE_WORDS;words[0]=2;mask=1|(2 if site!=1 else 0)|(4 if site in [2,3,4,5]else 0)|(8 if site==2 else 0)|(16 if site in [4,5]else 0)
 for phase in [1,2]:
  i=4+(phase-1)*SNAPSHOT_WORDS;words[i:i+4]=[1,site,phase,mask]
  words[i+134]=phase;words[i+135]=phase*3;words[i+137]=phase
 return words

def blob(words):return struct.pack('<'+str(len(words))+'I',*words)
class PrimaryTraceValidation(unittest.TestCase):
 def test_all_six_sites_and_empty_case(self):
  for site in range(1,7):self.assertEqual(validate(blob(fixture(site)),1)['calls_by_site'][str(site)],1)
  self.assertEqual(validate(bytes(TRACE_WORDS*4),1)['boundaries'],0)
 def test_bad_size_and_case_count(self):
  for data,count in [(b'',1),(bytes(TRACE_WORDS*4),0),(bytes(TRACE_WORDS*4-1),1)]:
   with self.assertRaises(ValueError):validate(data,count)
 def test_header_faults(self):
  for index,value in [(0,129),(0,1),(1,1),(2,1),(3,1)]:
   w=fixture();w[index]=value
   with self.assertRaises(ValueError):validate(blob(w),1)
 def test_bad_pair_identity_or_mask(self):
  for offset,value in [(0,2),(1,7),(2,2),(3,31)]:
   w=fixture();w[4+offset]=value
   with self.assertRaises(ValueError):validate(blob(w),1)
 def test_inactive_fields_rejected(self):
  for offset in [27,28,29,30,31,32,33,34,35,36,37,38,39,183,242,243]:
   w=fixture();w[4+offset]=1
   with self.assertRaises(ValueError):validate(blob(w),1)
 def test_inactive_child_after_non_de(self):
  w=fixture(4);w[4+SNAPSHOT_WORDS+183]=1
  with self.assertRaises(ValueError):validate(blob(w),1)
 def test_live_flags_are_typed(self):
  for site,offset in [(2,36),(4,37)]:
   w=fixture(site);w[4+offset]=2
   with self.assertRaises(ValueError):validate(blob(w),1)
 def test_de_and_normal_preserve_parent_locals(self):
  for site in [1,2,3,6]:
   w=fixture(site);w[4+SNAPSHOT_WORDS+10]=1
   with self.assertRaises(ValueError):validate(blob(w),1)
 def test_refinement_can_change_point_but_not_camera(self):
  for site in [4,5]:
   w=fixture(site);w[4+SNAPSHOT_WORDS+10]=1;validate(blob(w),1)
   w[4+SNAPSHOT_WORDS+4]=1
   with self.assertRaises(ValueError):validate(blob(w),1)
 def test_work_cannot_decrease(self):
  for offset in [134,135,137]:
   w=fixture();w[4+SNAPSHOT_WORDS+offset]=0
   with self.assertRaises(ValueError):validate(blob(w),1)
 def test_unused_trace_storage_must_be_zero(self):
  w=fixture();w[-1]=1
  with self.assertRaises(ValueError):validate(blob(w),1)
 def test_comparison_is_bit_exact(self):
  a=blob(fixture(4));w=fixture(4);w[4+10]=0x80000000;b=blob(w);validate(b,1)
  compare(a,a,1)
  with self.assertRaises(ValueError):compare(a,b,1)
if __name__=='__main__':unittest.main()
