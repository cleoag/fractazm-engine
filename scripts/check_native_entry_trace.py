#!/usr/bin/env python3
"""Independently validate geometry query accounting and caller ownership."""
from pathlib import Path
import argparse,collections,json,struct,zlib
CALL_LIMIT=1<<24
WORK_LIMIT=1<<28

def validate_case(inp,out,rows,maximum):
 assert len(inp)==153 and len(out)==141
 initial=inp[125:153];final=out[113:141];entry=inp[71:118]
 assert out[106:113]==(0,)*7 # Rebound observations must not leak to caller counters.
 count=collections.Counter()
 if initial[1] or initial[3]>=CALL_LIMIT or initial[4]>=WORK_LIMIT:
  assert not rows and out[:59]==(0,)*59 and out[59:106]==entry
  expected=list(initial)
  if not initial[1]:expected[1]=2;expected[8]=5 # FormulaWorkExhausted (ABI checked by tests).
  assert final==tuple(expected)
  count['guarded_queries']=1
  return count
 assert len(rows)==2
 before,after=rows;assert before[0]==1 and after[0]==2
 for row in rows:
  assert row[1:4]==(int(bool(inp[116])),inp[107] or maximum,inp[124])
  assert row[148]==inp[121]
  assert row[142:146]==row[134:138] and row[146:148]==row[132:134]
 assert before[8:55]==entry and before[55:114]==(0,)*59 and before[149]==0
 expected=list(initial);expected[3]+=1
 assert before[114:142]==tuple(expected)
 assert after[8:55]==out[59:106] and after[55:114]==out[:59]
 expected[18:24]=final[18:24]
 assert after[114:142]==tuple(expected)
 expected[4]+=out[51]
 if out[47] or not out[56]:
  expected[8:16]=out[47:55]
  expected[1]=2 if out[47]==5 else 4 if out[47]==4 else 8
 elif expected[4]>WORK_LIMIT:
  expected[1]=2;expected[8:16]=out[47:55];expected[8]=5
 else:expected[17]+=int(after[149]!=0)
 assert final==tuple(expected)
 count['child_queries']=1;count['child_work']=out[51];count['canonical_boundaries']=2
 count['failed_queries' if final[1] else 'successful_queries']=1
 count['maximum_defaults']=inp[107]==0
 count['outer_work_exhaustions']=not out[47] and bool(out[56]) and final[1]==2
 return count

def validate_directory(path,cases):
 assert 1<=cases<=8192
 path=Path(path);read=lambda name:zlib.decompress((path/(name+'.bin.zlib')).read_bytes())
 data,inputs,outputs,programs=map(read,('canonical','input','original','slots'))
 assert len(data)==cases*65536 and len(inputs)==cases*612 and len(outputs)==cases*564
 assert len(programs)%(cases*2)==0;stride=len(programs)//(cases*2)
 total=collections.Counter()
 for case in range(cases):
  inp=struct.unpack_from('<153I',inputs,case*612);out=struct.unpack_from('<141I',outputs,case*564)
  rows=[]
  for step in range(64):
   row=struct.unpack_from('<256I',data,(case*64+step)*1024)
   if not row[0]:break
   rows.append(row)
  maximum=struct.unpack_from('<I',programs,case*2*stride+7*4)[0]
  try:total.update(validate_case(inp,out,rows,maximum))
  except AssertionError as e:raise AssertionError(f'case {case}: geometry ownership/accounting')from e
 return dict(total)
if __name__=='__main__':
 ap=argparse.ArgumentParser(description=__doc__);ap.add_argument('data',type=Path);ap.add_argument('--cases',type=int,required=True);a=ap.parse_args();print(json.dumps(validate_directory(a.data,a.cases),indent=2))
