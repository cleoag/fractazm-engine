#!/usr/bin/env python3
"""Check raw replay trace ownership; this is not scene qualification."""
from pathlib import Path
import argparse,collections,json,struct,zlib

def validate_trace(path,cases):
 assert 1<=cases<=8192,'invalid case count'
 data=zlib.decompress(Path(path).read_bytes());coverage=collections.Counter()
 assert len(data)==cases*128*512,'trace size mismatch'
 for case in range(cases):
  rows=[]
  for t in range(128):
   r=struct.unpack_from('<128I',data,(case*128+t)*512)
   if not r[0]:break
   rows.append(r)
  assert rows and len(rows)%2==0 and len(rows)<128
  entry=rows[0][55:102]
  for i in range(0,len(rows),2):
   before,after=rows[i:i+2];assert before[0]==201 and after[0]==202
   assert before[55:102]==entry==after[55:102]
   assert before[1]==after[1] and before[106]<=after[106]
   coverage['orbit_calls']+=1;coverage['work_units']+=after[106]-before[106]
   if after[3]!=0xffffffff and not after[102]:
    assert i+2<len(rows);nxt=rows[i+2]
    assert nxt[1]==after[3]<before[1]
    assert nxt[8:55]==entry
    assert nxt[102:110]==after[102:110]
    assert nxt[6]==after[6]+(not after[4])
    coverage['replays']+=1;coverage['inert_replays']+=bool(after[4])
    coverage['replays_with_retained_work']+=after[106]>0
   else:assert i+2==len(rows)
  coverage['canonical_boundaries']+=len(rows)
  coverage['max_boundaries_per_case']=max(coverage['max_boundaries_per_case'],len(rows))
 return dict(coverage)

if __name__=='__main__':
 ap=argparse.ArgumentParser(description=__doc__);ap.add_argument('trace',type=Path);ap.add_argument('--cases',type=int,required=True);args=ap.parse_args()
 print(json.dumps(validate_trace(args.trace,args.cases),indent=2))
