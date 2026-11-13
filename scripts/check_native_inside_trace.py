#!/usr/bin/env python3
"""Validate inside-owner snapshots and retry/post-smooth transitions."""
import argparse,json,struct
from pathlib import Path
CAPACITY=8;SNAPSHOT_WORDS=260;TRACE_WORDS=4+CAPACITY*SNAPSHOT_WORDS

def need(ok,message):
 if not ok:raise ValueError(message)

def validate(data,cases):
 need(cases>0,'positive case count required');need(len(data)==cases*TRACE_WORDS*4,'inside trace byte length')
 counts={'attempts':0,'post_smooth':0,'retries':0};boundaries=0
 for case in range(cases):
  w=struct.unpack_from('<'+str(TRACE_WORDS)+'I',data,case*TRACE_WORDS*4);count,overflow,r0,r1=w[:4]
  need(count<=CAPACITY and count%2==0 and not overflow and not r0 and not r1,f'case {case}: trace header');need(not any(w[4+count*SNAPSHOT_WORDS:]),f'case {case}: unused storage')
  pairs=[]
  for index in range(0,count,2):
   k=4+index*SNAPSHOT_WORDS;a=w[k:k+SNAPSHOT_WORDS];b=w[k+SNAPSHOT_WORDS:k+2*SNAPSHOT_WORDS];site=a[1]
   need(a[0]==b[0]==1 and site in [1,2]and b[1]==site and a[2]==1 and b[2]==2 and a[3]==b[3]==site,f'case {case}: boundary identity');pairs.append((a,b));boundaries+=2
   for v in [a,b]:
    need(not any(v[j]for j in [7,11,198,258,259]),f'case {case}: reserved fields')
    need(v[4]in [0,1,0xffffffff]and v[5]==v[4]and v[6]<=1,f'case {case}: signed retry state')
    need(v[12]==1 and v[13]in [0,1,0xffffffff]and v[14]<=1 and v[15]>0 and v[16]<=1 and not any(v[17:20]),f'case {case}: admitted policy')
    need(v[191]==1 and not v[192]and 1<=v[193]<=2 and v[194]<=1 and v[197]<=1,f'case {case}: pre-final activity')
    if site==1 or v[2]==1:need(not any(v[199:258]),f'case {case}: inactive child result')
   need(a[4:48]==b[4:48]and a[191:199]==b[191:199],f'case {case}: child changed owner controls')
   for offset in [141,142,143,144,145]:need(b[offset]>=a[offset],f'case {case}: decreasing geometry work')
   if site==1:
    counts['attempts']+=1;need(a[8]<2 and a[193]==a[8]+1 and a[194]==a[8]and a[6]==a[8],f'case {case}: attempt index')
    need(not any(a[44:48]),f'case {case}: inactive smooth point')
   else:
    counts['post_smooth']+=1;need(a[8]==0 and a[4]!=0 and a[81]!=0 and a[82]!=0 and not a[86]and not a[140]and a[197]==1,f'case {case}: smooth precondition')
    need(a[48:92]==b[48:92]and a[155]==b[155]and a[163:167]==b[163:167],f'case {case}: captured hit payload changed')
    need(a[47]==a[87],f'case {case}: smooth stop differs from pixel')
    for axis in range(3):
     value=struct.unpack('<d',struct.pack('<2I',*a[60+2*axis:62+2*axis]))[0]
     try:bits=struct.unpack('<I',struct.pack('<f',value))[0]
     except OverflowError:raise ValueError(f'case {case}: invalid smooth point')
     need(bits==a[44+axis],f'case {case}: smooth point differs from pixel')
  sites=[a[1]for a,b in pairs];need(sites in [[],[1],[1,1],[1,2],[1,1,2]],f'case {case}: owner call order')
  if pairs:
   first=pairs[0][0];need(first[8]==0 and first[4]==first[13]and not any(first[48:92]),f'case {case}: first attempt initialization')
   for previous,(a,b)in zip(pairs,pairs[1:]):
    last=previous[1];need(a[9:44]==last[9:44]and a[48:191]==last[48:191],f'case {case}: state lost between owner calls')
    if a[1]==1:
     counts['retries']+=1;need(last[82]!=0 and not last[86]and not last[140]and last[14]==1 and not last[6],f'case {case}: retry precondition')
     need(a[4]==(0 if last[13]else 0xffffffff)and a[6]==1 and a[8]==1,f'case {case}: raw retry flip')
    else:need(a[4:8]==last[4:8]and a[193:195]==last[193:195]and a[195]==a[4]and a[196]==a[82],f'case {case}: post-smooth owner finalization')
 return {'cases':cases,'boundaries':boundaries,**counts}

def compare(reference,candidate,cases):
 result=validate(reference,cases);validate(candidate,cases);need(reference==candidate,'inside canonical trace differs');return result

def main():
 ap=argparse.ArgumentParser(description=__doc__);ap.add_argument('reference',type=Path);ap.add_argument('candidate',type=Path,nargs='?');ap.add_argument('--cases',type=int,required=True);a=ap.parse_args()
 try:
  source=a.reference.read_bytes();result=compare(source,a.candidate.read_bytes(),a.cases)if a.candidate else validate(source,a.cases);print(json.dumps(result));return 0
 except(OSError,ValueError)as e:print(str(e));return 1
if __name__=='__main__':raise SystemExit(main())
