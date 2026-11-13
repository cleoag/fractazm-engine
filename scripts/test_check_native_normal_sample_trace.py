#!/usr/bin/env python3
"""Synthetic validator controls, never scene/renderer qualification."""
import unittest,struct,tempfile,zlib
from pathlib import Path
from check_native_normal_sample_trace import validate_case,validate_directory

def fixture(direct):
 inp=[0]*161;inp[107]=3;inp[116]=1;inp[119]=int(direct);inp[120]=0x3f800000
 g=[0]*28;g[0]=6;g[3]=2;g[4]=7;inp[125:153]=g;n=[1]+[0]*7;inp[153:161]=n
 p=[0]*24;p[7]=3;p[11]=64;p[16]=0x40800000
 g[5]+=1;n[3]+=1;n[2 if direct else 4]+=1
 first=[0]*256;first[0]=2 if direct else 1;first[8:55]=inp[71:118];first[55:83]=g;first[83:91]=n
 last=first.copy();last[0]=3 if direct else 4
 lane=list(inp[71:118]);lane[24]=0x3f800000;lane[28]=0x3f800000;lane[32]=0x40000000
 if direct:
  first[8+26]=p[16];lane[26]=p[16];first[155]=64;last[155]=64;last[154]=5
  g[4]+=5;value=lane[32]
 else:
  child=lane+[0]*12;child[51]=5;child[55]=0x40400000;child[56]=1;last[91:150]=child
  g[3]+=1;g[4]+=5;last[55:83]=g;value=child[55]
 last[8:55]=lane
 out=[value,lane[32]]+lane+[0]*7+g+n
 return inp,out,[first,last],p

def check(data):
 inp,out,rows,p=data;return validate_case(tuple(inp),tuple(out),[tuple(r) for r in rows],tuple(p))
class NormalSampleTraceTests(unittest.TestCase):
 def test_direct(self):self.assertEqual(check(fixture(True))['direct_orbit_samples'],1)
 def test_de(self):self.assertEqual(check(fixture(False))['distance_samples'],1)
 def test_direct_restoration(self):
  data=fixture(True);data[2][0][34]=0
  with self.assertRaises(AssertionError):check(data)
 def test_double_work(self):
  for direct in (False,True):
   data=fixture(direct);data[1][60]+=5
   with self.assertRaises(AssertionError):check(data)
 def test_counter_leak(self):
  data=fixture(True);data[1][49]=1
  with self.assertRaises(AssertionError):check(data)
 def test_child_reorder(self):
  data=fixture(False);data[2].reverse()
  with self.assertRaises(AssertionError):check(data)
 def test_direct_value(self):
  data=fixture(True);data[1][0]+=1
  with self.assertRaises(AssertionError):check(data)
 def admission_fixture(self,path):
  inp,out,rows,program=fixture(True)
  records={'input':inp,'original':out,'canonical':sum(rows,[])+[0]*(62*256),'slots':program*2,'admission':[1]}
  for name,words in records.items():
   (path/(name+'.bin.zlib')).write_bytes(zlib.compress(struct.pack('<'+str(len(words))+'I',*words)))
 def test_directory_requires_direct_admission(self):
  with tempfile.TemporaryDirectory() as tmp:
   path=Path(tmp);self.admission_fixture(path)
   self.assertEqual(validate_directory(path,1)['direct_orbit_samples'],1)
   (path/'admission.bin.zlib').write_bytes(zlib.compress(struct.pack('<I',0)))
   with self.assertRaises(AssertionError):validate_directory(path,1)
 def test_missing_admission_is_not_evidence(self):
  with tempfile.TemporaryDirectory() as tmp:
   path=Path(tmp);self.admission_fixture(path);(path/'admission.bin.zlib').unlink()
   with self.assertRaises(FileNotFoundError):validate_directory(path,1)
 def test_truncated_admission_is_rejected(self):
  with tempfile.TemporaryDirectory() as tmp:
   path=Path(tmp);self.admission_fixture(path);(path/'admission.bin.zlib').write_bytes(zlib.compress(b''))
   with self.assertRaises(AssertionError):validate_directory(path,1)
 def test_initial_fault(self):
  inp=[0]*161;inp[126]=4;out=[0]*56+inp[125:153]+inp[153:161]
  self.assertEqual(check((inp,out,[],[0]*24))['guarded_samples'],1)
if __name__=='__main__':unittest.main()
