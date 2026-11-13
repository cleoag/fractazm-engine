#!/usr/bin/env python3
"""Check independently owned DE calls and retained combined work/state."""
from pathlib import Path
import argparse,collections,json,struct,zlib

def validate_case(inp,out,rows):
 assert len(inp)==125 and len(out)==113 and len(rows) in (2,4)
 counts=collections.Counter();kind=rows[0][224];assert 0<=kind<=6
 assert rows[0][0]==(2 if kind else 1)
 entry=inp[71:118];first=None;total=0
 for index in range(0,len(rows),2):
  before,after=rows[index:index+2];site=before[0]
  assert site in (1,2,3) and after[0]==site+100
  assert before[224]==after[224]==kind
  assert before[1:4]==after[1:4] and before[1]==inp[116]
  assert before[7]==after[7]==inp[121]
  assert before[102:161]==(0,)*59
  child=after[102:161];failed=bool(child[47] or not child[56])
  total+=child[51];counts['child_queries']+=1;counts['child_work']+=child[51]
  counts['failed_child_queries' if failed else 'completed_child_queries']+=1
  assert before[55:102]==entry
  assert after[8:55]==(before[8:55] if failed else child[:47])
  assert after[55:102]==(after[8:55] if site==1 else entry)
  if site in (1,2):
   assert before[8:55]==entry and before[2]==inp[107]
   assert before[3]==(1 if kind else 0)
   assert before[161:220]==after[161:220]==(0,)*59
   first=child
   if failed:assert len(rows)==2
  else:
   assert index==2 and first is not None and first[56] and first[47]==0
   assert before[8:55]==first[:47]
   assert before[161:220]==after[161:220]==first
   assert before[3]==(2 if kind==3 else 1)
   counts['second_queries']+=1;counts['forced_inside_queries']+=kind==3
  for j in (4,5,6,220,221,222,223):assert before[j]<=after[j]
  if index:
   previous=rows[index-1]
   for j in (4,5,6,220,221,222,223):assert before[j]==previous[j]
 assert out[51]==total
 assert out[59:106]==(out[:47] if out[56] else entry)
 assert bool(out[56])!=bool(out[47])
 counts['completed_combinations' if out[56] else 'failed_combinations']+=1
 counts['two_query_fault_work_preserved']+=bool(len(rows)==4 and rows[-1][149])
 assert out[106:110]==rows[-1][220:224]
 assert out[110:113]==(rows[-1][5],rows[-1][4],rows[-1][6])
 mask=inp[123]
 if not mask&1:assert out[106:110]==(0,)*4
 if not mask&2:assert out[110]==0
 if not mask&4:assert out[111]==0
 if not mask&8:assert out[112]==0
 counts['canonical_boundaries']=len(rows)
 return counts

def validate_directory(path,cases):
 assert 1<=cases<=8192
 path=Path(path);read=lambda name:zlib.decompress((path/(name+'.bin.zlib')).read_bytes())
 data,inputs,outputs,programs=read('canonical'),read('input'),read('original'),read('slots')
 witness=read('first-witness');assert len(witness)==cases*452
 assert len(programs)%(cases*2)==0;program_bytes=len(programs)//(cases*2);assert program_bytes>=56*4
 assert len(data)==cases*64*1024 and len(inputs)==cases*500 and len(outputs)==cases*452
 total=collections.Counter()
 for case in range(cases):
  inp=struct.unpack_from('<125I',inputs,case*500);out=struct.unpack_from('<113I',outputs,case*452)
  rows=[]
  for step in range(64):
   row=struct.unpack_from('<256I',data,(case*64+step)*1024)
   if not row[0]:break
   rows.append(row)
  try:
   total.update(validate_case(inp,out,rows))
   first=struct.unpack_from('<113I',witness,case*452)
   assert rows[1][102:149]==first[:47] and rows[1][153]==first[51]
   assert rows[1][220:224]==first[106:110]
   assert (rows[1][5],rows[1][4],rows[1][6])==first[110:113]
   total['frozen_first_query_witnesses']+=1
   if len(rows)==4:
    option,maximum=struct.unpack_from('<ii',programs,(case*2+1)*program_bytes+6*4)
    raised=bool(inp[124] and option!=20)
    assert rows[2][2]==(maximum+raised)&0xffffffff
    total['temporary_second_maximum_increments']+=raised
  except AssertionError as e:raise AssertionError(f'case {case}: combined ownership invariant')from e
 return dict(total)
if __name__=='__main__':
 ap=argparse.ArgumentParser(description=__doc__);ap.add_argument('data',type=Path);ap.add_argument('--cases',type=int,required=True);a=ap.parse_args()
 print(json.dumps(validate_directory(a.data,a.cases),indent=2))
