#!/usr/bin/env python3
"""Synthetic validator mutations; these are not renderer evidence."""
import unittest
from check_native_entry_trace import validate_case,CALL_LIMIT,WORK_LIMIT

def fixture(work=7):
 inp=[0]*153;inp[107]=3;inp[116]=1;inp[124]=1
 geometry=[0]*28;geometry[0]=6;geometry[3]=2;geometry[4]=work;geometry[17]=3;geometry[18:24]=[4,5,6,7,8,9];inp[125:153]=geometry
 child=list(range(47))+[0]*12;child[51]=5;child[56]=1
 out=child+child[:47]+[0]*7+geometry
 out[116]+=1;out[117]+=5;out[130]+=1
 out[131:137]=[5,6,7,8,9,10]
 rows=[]
 for phase in (1,2):
  row=[0]*256;row[:4]=[phase,1,3,1];row[8:55]=inp[71:118];row[114:142]=geometry;row[117]+=1
  if phase==2:
   row[8:55]=child[:47];row[55:114]=child;row[132:138]=out[131:137];row[149]=2
  row[142:146]=row[134:138];row[146:148]=row[132:134];rows.append(row)
 return inp,out,rows

def check(data):
 inp,out,rows=data
 return validate_case(tuple(inp),tuple(out),[tuple(x) for x in rows],3)
class EntryTraceTests(unittest.TestCase):
 def test_success(self):self.assertEqual(check(fixture())['child_work'],5)
 def test_double_work_rejected(self):
  data=fixture();data[1][117]+=5
  with self.assertRaises(AssertionError):check(data)
 def test_clamps_count_queries_not_parts(self):
  data=fixture();data[1][130]+=1
  with self.assertRaises(AssertionError):check(data)
 def test_caller_counter_leak_rejected(self):
  data=fixture();data[1][106]=1
  with self.assertRaises(AssertionError):check(data)
 def test_work_added_before_child_return_rejected(self):
  data=fixture();data[2][1][118]+=5
  with self.assertRaises(AssertionError):check(data)
 def test_initial_fault_preserved(self):
  inp=[0]*153;inp[126]=4;inp[129]=9
  out=[0]*113+inp[125:153]
  self.assertEqual(check((inp,out,[]))['guarded_queries'],1)
  out[117]+=1
  with self.assertRaises(AssertionError):check((inp,out,[]))
 def test_budget_guard(self):
  for index,limit in ((128,CALL_LIMIT),(129,WORK_LIMIT)):
   inp=[0]*153;inp[index]=limit;out=[0]*113+inp[125:153];out[114]=2;out[121]=5
   self.assertEqual(check((inp,out,[]))['guarded_queries'],1)
 def test_post_work_exhaustion(self):
  inp,out,rows=fixture(WORK_LIMIT-1);out[114]=2;out[121:129]=out[47:55];out[121]=5;out[130]-=1
  self.assertEqual(check((inp,out,rows))['outer_work_exhaustions'],1)
 def test_maximum_default(self):
  data=fixture();data[0][107]=0;data[2][0][44]=0
  self.assertEqual(check(data)['maximum_defaults'],1)
if __name__=='__main__':unittest.main()
