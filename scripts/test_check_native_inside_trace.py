#!/usr/bin/env python3
import struct,unittest
from check_native_inside_trace import validate,compare,TRACE_WORDS,SNAPSHOT_WORDS

def snapshot(site,phase):
 v=[0]*SNAPSHOT_WORDS;v[:4]=[1,site,phase,site];v[12]=1;v[15]=1;v[191]=1;v[193]=1;return v

def fixture(kind='one'):
 a=snapshot(1,1);b=snapshot(1,2);b[142]=1;b[143]=3;pairs=[(a,b)]
 if kind in ['retry','smooth']:
  for v in [a,b]:v[14]=1 if kind=='retry'else 0;v[4]=v[5]=v[13]=0 if kind=='retry'else 1
  b[81]=1;b[82]=1
 if kind=='retry':
  c=b.copy();c[2]=1;c[4]=c[5]=0xffffffff;c[6]=1;c[8]=1;c[193]=2;c[194]=1;d=c.copy();d[2]=2;d[142]+=1;d[143]+=3;pairs.append((c,d))
 if kind=='smooth':
  c=b.copy();c[1]=c[3]=2;c[2]=1;c[195]=1;c[196]=1;c[197]=1;d=c.copy();d[2]=2;d[142]+=1;d[143]+=3;pairs.append((c,d))
 w=[0]*TRACE_WORDS;w[0]=len(pairs)*2
 for i,(a,b)in enumerate(pairs):w[4+2*i*SNAPSHOT_WORDS:4+(2*i+2)*SNAPSHOT_WORDS]=a+b
 return w

def blob(w):return struct.pack('<'+str(len(w))+'I',*w)
class InsideTraceValidation(unittest.TestCase):
 def test_valid_paths_and_empty(self):
  for kind in ['one','retry','smooth']:validate(blob(fixture(kind)),1)
  self.assertEqual(validate(bytes(TRACE_WORDS*4),1)['boundaries'],0)
 def test_length_and_header(self):
  for data,count in [(b'',1),(bytes(TRACE_WORDS*4),0)]:
   with self.assertRaises(ValueError):validate(data,count)
  for index,value in [(0,9),(0,1),(1,1),(2,1)]:
   w=fixture();w[index]=value
   with self.assertRaises(ValueError):validate(blob(w),1)
 def test_pair_identity_and_reserved(self):
  for offset,value in [(0,2),(1,3),(2,2),(3,2),(7,1),(11,1),(198,1),(258,1),(259,1)]:
   w=fixture();w[4+offset]=value
   with self.assertRaises(ValueError):validate(blob(w),1)
 def test_invalid_signed_word(self):
  w=fixture();w[8]=2
  with self.assertRaises(ValueError):validate(blob(w),1)
 def test_inactive_fields(self):
  for offset in [44,45,46,47,199]:
   w=fixture();w[4+offset]=1
   with self.assertRaises(ValueError):validate(blob(w),1)
 def test_camera_cannot_change(self):
  w=fixture();w[4+SNAPSHOT_WORDS+20]=1
  with self.assertRaises(ValueError):validate(blob(w),1)
 def test_work_cannot_decrease(self):
  w=fixture();w[4+143]=4
  with self.assertRaises(ValueError):validate(blob(w),1)
 def test_retry_flip_is_raw_signed_word(self):
  w=fixture('retry')
  for i in [2,3]:w[4+i*SNAPSHOT_WORDS+4]=w[4+i*SNAPSHOT_WORDS+5]=1
  with self.assertRaises(ValueError):validate(blob(w),1)
 def test_state_cannot_be_reset_on_retry(self):
  w=fixture('retry');w[4+2*SNAPSHOT_WORDS+92]=1;w[4+3*SNAPSHOT_WORDS+92]=1
  with self.assertRaises(ValueError):validate(blob(w),1)
 def test_post_smooth_preserves_captured_payload(self):
  for offset in [48,155,163]:
   w=fixture('smooth');w[4+3*SNAPSHOT_WORDS+offset]=1
   with self.assertRaises(ValueError):validate(blob(w),1)
 def test_smooth_point_and_stop_bound_to_pixel(self):
  for offset in [44,47]:
   w=fixture('smooth');w[4+2*SNAPSHOT_WORDS+offset]=w[4+3*SNAPSHOT_WORDS+offset]=1
   with self.assertRaises(ValueError):validate(blob(w),1)
 def test_unused_storage_and_exact_comparison(self):
  a=blob(fixture());compare(a,a,1);w=fixture();w[-1]=1
  with self.assertRaises(ValueError):validate(blob(w),1)
  w=fixture();w[4+SNAPSHOT_WORDS+92]=0x80000000;validate(blob(w),1)
  with self.assertRaises(ValueError):compare(a,blob(w),1)
if __name__=='__main__':unittest.main()
