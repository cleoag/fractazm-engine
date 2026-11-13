#!/usr/bin/env python3
"""Independent accounting checks for the isolated ordered-normal probe."""
import argparse,collections,json,struct
from pathlib import Path
OUTPUT_WORDS=99
ROW_WORDS=92
TRACE_WORDS=2+14*ROW_WORDS

def require(value,why):
 if not value:raise ValueError(why)

def validate_case(out,trace):
 require(len(out)==OUTPUT_WORDS and len(trace)==TRACE_WORDS,'record size')
 count,overflow=trace[:2];require(not overflow and count<=14 and count%2==0,'trace bound')
 require(not any(trace[2+count*ROW_WORDS:]),'inactive trace storage')
 lane=out[16:63];g=out[63:91];n=out[91:99]
 require(out[11]==(g[1]|n[1]),'terminal fault')
 require(n[5]==out[7] and out[7] in (0,1),'completed accounting')
 require(n[2]+n[4]==n[3]==g[5] and n[4]==g[3],'normal/DE accounting')
 total=collections.Counter(canonical_boundaries=count,child_calls=count//2,normal_samples=n[3],formula_work=g[4],completed_normals=out[7],range_exits=g[18])
 if not count:
  require(not out[7] and not n[3] and out[3]==0,'guarded parent publication');return total
 previous=None;first=None
 for k in range(count//2):
  before=trace[2+2*k*ROW_WORDS:2+(2*k+1)*ROW_WORDS];after=trace[2+(2*k+1)*ROW_WORDS:2+(2*k+2)*ROW_WORDS]
  require(before[0]==1 and after[0]==2,'phase order')
  require(before[1:7]==after[1:7] and before[1] in (0,1),'child input identity')
  require(before[2]==(1 if k==0 else before[1]),'smoothing selection')
  require(before[90:92]==(0,0),'uninitialized return')
  bl,al=before[7:54],after[7:54];bg,ag=before[54:82],after[54:82];bn,an=before[82:90],after[82:90]
  if previous is not None:require(before[7:90]==previous[7:90],'carried child state')
  if bg[1] or bn[1]:
   require(before[7:90]==after[7:90] and after[90:92]==(0,0),'guarded child mutation')
  else:
   require(an[3]==bn[3]+1 and ag[5]==bg[5]+1,'sample accounted once')
   which=2 if before[1] else 4
   require(an[which]==bn[which]+1 and an[6-which]==bn[6-which],'DE/direct accounting')
   require(ag[3]==bg[3]+int(not before[1]) and ag[4]>=bg[4],'DE/work progress')
  require(an[5]==bn[5]==0,'premature normal completion')
  if first is None:first=after
  previous=after
 require(out[3]==first[91],'first smoothing result')
 require(lane[4:7]==first[3:6],'center restored')
 require(lane[:4]+lane[7:]==previous[7:11]+previous[14:54],'last lane preserved')
 require(g[2:8]==previous[56:62] and n[2:5]==previous[84:87],'final work preserved')
 require(g[18:24]==previous[72:78],'final range/carried/smoothing counters preserved')
 require(out[5]==lane[28] and out[4]==0 and out[6]==0,'trap/disabled roughness')
 require(0<=count//2-n[3]<=1,'guarded minus call bound')
 if out[7]:require(count==14 and not out[11] and any(out[8:11]),'complete normal coverage')
 return total

def validate_directory(path,cases=1024):
 require(1<=cases<=8192,'case bound');path=Path(path)
 output=(path/'control-first.bin').read_bytes();trace=(path/'observed-first.bin').read_bytes()
 require(len(output)==cases*OUTPUT_WORDS*4 and len(trace)==cases*TRACE_WORDS*4,'buffer size')
 require(output==(path/'candidate-first.bin').read_bytes(),'final state equality')
 require(trace==(path/'witness-first.bin').read_bytes(),'canonical equality')
 total=collections.Counter()
 for i in range(cases):
  try:total.update(validate_case(struct.unpack_from('<99I',output,i*396),struct.unpack_from('<'+str(TRACE_WORDS)+'I',trace,i*TRACE_WORDS*4)))
  except ValueError as e:raise ValueError(f'case {i}: {e}') from e
 return dict(total)
if __name__=='__main__':
 ap=argparse.ArgumentParser(description=__doc__);ap.add_argument('data',type=Path);ap.add_argument('--cases',type=int,default=1024);a=ap.parse_args();print(json.dumps(validate_directory(a.data,a.cases),indent=2))
