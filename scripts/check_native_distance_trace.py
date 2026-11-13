#!/usr/bin/env python3
"""Validate query trace ownership and coverage; no scene qualification claim."""
from pathlib import Path
import argparse,collections,json,struct,zlib

def validate_directory(path,cases):
 assert 1<=cases<=8192,'invalid case count'
 path=Path(path);data=zlib.decompress((path/'canonical.bin.zlib').read_bytes())
 inputs=zlib.decompress((path/'input.bin.zlib').read_bytes());outputs=zlib.decompress((path/'original.bin.zlib').read_bytes())
 assert len(data)==cases*64*1024 and len(inputs)==cases*496 and len(outputs)==cases*452
 coverage=collections.Counter()
 for case in range(cases):
  inp=struct.unpack_from('<124I',inputs,case*496);out=struct.unpack_from('<113I',outputs,case*452)
  rows=[]
  for t in range(64):
   r=struct.unpack_from('<256I',data,(case*64+t)*1024)
   if not r[0]:break
   rows.append(r)
  assert len(rows)%2==0 and len(rows)<64
  previous=None
  for i in range(0,len(rows),2):
   before,after=rows[i:i+2];site=before[0]
   assert site in [1,2,3,4] and after[0]==site+100
   assert before[55:102]==inp[71:118]==after[55:102]
   assert before[102]==0 and before[106]<=after[106] and before[107]==after[107]
   assert before[1]==after[1]
   if previous:
    assert previous[102]==0 and before[106]==previous[106]
   else:assert site==1 and before[106]==0
   coverage['orbit_calls']+=1;coverage['work_units']+=after[106]-before[106]
   if site==2:
    axis=before[130];assert axis<3 and before[135]==1
    assert before[12+axis]==before[121] and before[120]==inp[axis]
    coverage['gradient_samples']+=1
    coverage['sample_faults']+=bool(after[102])
    coverage['certified_samples']+=bool(after[2])
   elif site==3:
    assert before[1]==0 and before[125]==0
    coverage['zero_pass_retries']+=1
   elif site==4:
    assert before[1]==0xffffffff and before[44]==before[126]
    coverage['previous_iteration_retries']+=1
   previous=after
  assert out[51]==(previous[106] if previous else 0)
  if out[56]:
   assert out[47]==0 and out[59:106]==out[:47]
   coverage['completed_queries']+=1
  else:
   assert out[47] and out[59:106]==inp[71:118]
   coverage['failed_queries']+=1
  if previous and previous[0]==102 and previous[102]:
   axis=previous[130];assert out[4+axis]==previous[120]
   coverage['fault_coordinate_restores']+=1
  mask=inp[123]
  if not mask&1:assert out[106:110]==(0,0,0,0)
  if not mask&2:assert out[110]==0
  if not mask&4:assert out[111]==0
  if not mask&8:assert out[112]==0
  coverage['reported_range_exits']+=out[111];coverage['reported_minimum_clamps']+=out[112]
  coverage['canonical_boundaries']+=len(rows)
  coverage['max_boundaries_per_case']=max(coverage['max_boundaries_per_case'],len(rows))
 return dict(coverage)

if __name__=='__main__':
 ap=argparse.ArgumentParser(description=__doc__);ap.add_argument('data',type=Path);ap.add_argument('--cases',type=int,required=True);args=ap.parse_args()
 print(json.dumps(validate_directory(args.data,args.cases),indent=2))
