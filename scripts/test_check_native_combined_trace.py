#!/usr/bin/env python3
"""Mutation checks for the combined ownership validator; synthetic, not GPU proof."""
import unittest
from check_native_combined_trace import validate_case

def fixture():
 inp=[0]*125;inp[107]=3
 first=[0]*59;first[36]=3;first[0]=11;first[51]=5;first[56]=1
 second=[0]*59;second[36]=3;second[0]=22;second[51]=7;second[56]=1
 rows=[]
 for site,child in [(2,first),(3,second)]:
  before=[0]*256;before[0]=site;before[2]=3;before[3]=1;before[224]=1;before[55:102]=inp[71:118];before[8:55]=inp[71:118]
  if site==3:before[8:55]=first[:47];before[161:220]=first
  after=before.copy();after[0]+=100;after[8:55]=child[:47];after[102:161]=child
  rows.extend([before,after])
 out=[0]*113;out[:59]=second;out[51]=12;out[59:106]=second[:47]
 return inp,out,rows

def check(values):
 inp,out,rows=values
 return validate_case(tuple(inp),tuple(out),[tuple(row)for row in rows])
class OwnershipMutations(unittest.TestCase):
 def test_valid_two_queries(self):self.assertEqual(check(fixture())['child_work'],12)
 def test_reject_lost_first_work(self):
  v=fixture();v[1][51]=7
  with self.assertRaises(AssertionError):check(v)
 def test_reject_changed_first_result_during_second(self):
  v=fixture();v[2][3][161]=99
  with self.assertRaises(AssertionError):check(v)
 def test_reject_early_caller_publication(self):
  v=fixture();v[2][1][55]=11
  with self.assertRaises(AssertionError):check(v)
 def test_reject_second_after_first_fault(self):
  v=fixture();v[2][1][149]=1;v[2][1][158]=0
  with self.assertRaises(AssertionError):check(v)
 def test_reject_changed_live_counter_between_calls(self):
  v=fixture();v[2][2][4]=1;v[2][3][4]=1;v[1][111]=1
  with self.assertRaises(AssertionError):check(v)
if __name__=='__main__':unittest.main()
