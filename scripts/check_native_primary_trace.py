#!/usr/bin/env python3
"""Validate canonical primary caller snapshots, not renderer acceptance."""
import argparse,json,struct
from pathlib import Path
CAPACITY=128;SNAPSHOT_WORDS=244;TRACE_WORDS=4+CAPACITY*SNAPSHOT_WORDS

def need(ok,message):
 if not ok:raise ValueError(message)

def validate(data,cases):
 need(cases>0,'positive case count required');need(len(data)==cases*TRACE_WORDS*4,'trace byte length')
 counts={str(i):0 for i in range(1,7)};boundaries=0
 for case in range(cases):
  words=struct.unpack_from('<'+str(TRACE_WORDS)+'I',data,case*TRACE_WORDS*4);count,overflow,r0,r1=words[:4]
  need(count<=CAPACITY and count%2==0 and not overflow and not r0 and not r1,f'case {case}: trace header')
  need(not any(words[4+count*SNAPSHOT_WORDS:]),f'case {case}: unused trace storage')
  for index in range(0,count,2):
   start=4+index*SNAPSHOT_WORDS;a=words[start:start+SNAPSHOT_WORDS];b=words[start+SNAPSHOT_WORDS:start+2*SNAPSHOT_WORDS]
   site=a[1];need(a[0]==b[0]==1 and 1<=site<=6 and b[1]==site and a[2]==1 and b[2]==2,f'case {case}: boundary identity')
   mask=1|(2 if site!=1 else 0)|(4 if site in [2,3,4,5]else 0)|(8 if site==2 else 0)|(16 if site in [4,5]else 0)
   need(a[3]==b[3]==mask,f'case {case}: live mask');counts[str(site)]+=1;boundaries+=2
   for value in [a,b]:
    need(not any(value[k] for k in [31,38,39,242,243]),f'case {case}: reserved snapshot fields')
    if not mask&2:need(not any(value[k]for k in [27,32,33,34]),f'case {case}: initial-query inactive fields')
    if not mask&4:need(not any(value[k]for k in [28,29,35,36]),f'case {case}: inactive march fields')
    if not mask&8:need(value[30]==0,f'case {case}: inactive backup step')
    if not mask&16:need(value[37]==0,f'case {case}: inactive contact flag')
    if mask&4:need(value[36]<=1,f'case {case}: ended flag')
    if mask&16:need(value[37]<=1,f'case {case}: contact flag')
    if site>3 or value[2]==1:need(not any(value[183:242]),f'case {case}: inactive child result')
   need(a[4:10]==b[4:10],f'case {case}: immutable ray/start')
   if site in [1,2,3,6]:need(a[4:40]==b[4:40],f'case {case}: child changed caller locals')
   need(b[134]>=a[134] and b[135]>=a[135] and b[137]>=a[137],f'case {case}: decreasing work counters')
 return {'cases':cases,'boundaries':boundaries,'calls_by_site':counts}

def compare(reference,candidate,cases):
 a=validate(reference,cases);b=validate(candidate,cases);need(reference==candidate,'canonical trace differs');need(a==b,'trace counts differ');return a

def main():
 ap=argparse.ArgumentParser(description=__doc__);ap.add_argument('reference',type=Path);ap.add_argument('candidate',type=Path,nargs='?');ap.add_argument('--cases',type=int,required=True);a=ap.parse_args()
 try:
  source=a.reference.read_bytes();result=compare(source,a.candidate.read_bytes(),a.cases) if a.candidate else validate(source,a.cases)
  print(json.dumps(result));return 0
 except (OSError,ValueError)as e:print(str(e));return 1
if __name__=='__main__':raise SystemExit(main())
