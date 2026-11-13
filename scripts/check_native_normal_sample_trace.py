#!/usr/bin/env python3
"""Validate independent normal-sample counters, ownership and child returns."""
from pathlib import Path
import argparse,collections,json,math,struct,zlib
BUDGET=1<<18
WORK_LIMIT=1<<28

def finite(word):return math.isfinite(struct.unpack('<f',struct.pack('<I',word))[0])
def positive(word):return finite(word) and struct.unpack('<f',struct.pack('<I',word))[0]>0

def validate_case(inp,out,rows,program):
 assert len(inp)==161 and len(out)==92
 initial_g=inp[125:153];initial_n=inp[153:161];entry=inp[71:118];direct=bool(inp[119])
 final_g=out[56:84];final_n=out[84:92];lane=out[2:49]
 assert out[49:56]==(0,)*7
 g=list(initial_g);n=list(initial_n);count=collections.Counter()
 if not rows:
  assert out[:2]==(0,0) and lane==entry
  if g[1] or n[1]:pass
  elif n[3]>=BUDGET or g[5]>=BUDGET or g[4]>=WORK_LIMIT:g[1]|=2;n[1]|=2
  elif not all(map(finite,inp[:3])) or not positive(inp[120]):g[1]|=4;n[1]|=4
  else:
   assert direct and n[2]>=BUDGET
   n[3]+=1;g[5]+=1;g[1]|=2;n[1]|=2
  assert final_g==tuple(g) and final_n==tuple(n)
  count['guarded_samples']=1;return count
 assert len(rows)==2
 before,after=rows;assert (before[0],after[0])==((2,3) if direct else (1,4))
 n[3]+=1;g[5]+=1
 n[2 if direct else 4]+=1
 assert before[55:83]==tuple(g) and before[83:91]==tuple(n) and after[83:91]==tuple(n)
 assert before[91:150]==(0,)*59
 assert after[8:55]==lane
 if direct:
  expected_lane=list(entry);expected_lane[4:7]=inp[:3];expected_lane[36]=program[7];expected_lane[26]=program[16];expected_lane[45]=1
  assert before[8:55]==tuple(expected_lane)
  assert before[150:158]==(0,0,0,0,0,program[11],0,0) and after[91:150]==(0,)*59
  g[18:24]=final_g[18:24];assert after[55:83]==tuple(g)
  fault=after[150:158];g[4]+=fault[4]
  if fault[0]:
   g[8:16]=fault;code=2 if fault[0]==5 else 4 if fault[0]==4 else 8;g[1]|=code;n[1]|=code
  elif g[4]>WORK_LIMIT:
   g[8:16]=fault;g[8]=5;g[1]|=2;n[1]|=2
  elif not all(finite(lane[i]) for i in (32,24,28)):
   if not g[8]:g[8:12]=[4,0,lane[46],lane[33]]
   g[1]|=4;n[1]|=4
  assert final_g==tuple(g) and final_n==tuple(n)
  assert out[:2]==((0,0) if g[1] else (lane[32],lane[32]))
  count['direct_orbit_samples']=1;count['child_work']=fault[4]
 else:
  assert before[8:55]==entry and before[150:158]==after[150:158]==(0,)*8
  child=after[91:150];assert out[:2]==(child[55],child[32]);assert final_g==after[55:83]
  n[1]|=final_g[1];assert final_n==tuple(n)
  assert final_g[4]==initial_g[4]+child[51] and final_g[5]==g[5]
  assert final_g[3]==initial_g[3]+int(initial_g[3]<(1<<24))
  assert lane==(child[:47] if child[56] else entry)
  count['distance_samples']=1;count['child_work']=child[51]
 count['canonical_boundaries']=2;count['failed_samples' if final_g[1] or final_n[1] else 'successful_samples']=1
 return count

def validate_directory(path,cases):
 assert 1<=cases<=8192
 path=Path(path);read=lambda name:zlib.decompress((path/(name+'.bin.zlib')).read_bytes())
 data,inputs,outputs,programs=map(read,('canonical','input','original','slots'))
 admission=read('admission');assert len(admission)==cases*4
 assert len(data)==cases*65536 and len(inputs)==cases*644 and len(outputs)==cases*368
 assert len(programs)%(cases*2)==0;stride=len(programs)//(cases*2)
 total=collections.Counter()
 for case in range(cases):
  inp=struct.unpack_from('<161I',inputs,case*644);out=struct.unpack_from('<92I',outputs,case*368)
  rows=[]
  for step in range(64):
   row=struct.unpack_from('<256I',data,(case*64+step)*1024)
   if not row[0]:break
   rows.append(row)
  program=struct.unpack_from('<24I',programs,case*2*stride)
  if inp[119]:assert struct.unpack_from('<I',admission,case*4)[0]==1
  try:total.update(validate_case(inp,out,rows,program))
  except AssertionError as e:raise AssertionError(f'case {case}: normal sample ownership/accounting')from e
 return dict(total)
if __name__=='__main__':
 ap=argparse.ArgumentParser(description=__doc__);ap.add_argument('data',type=Path);ap.add_argument('--cases',type=int,required=True);a=ap.parse_args();print(json.dumps(validate_directory(a.data,a.cases),indent=2))
