"""Measure actual geometry and controlled formula influence in C++.
This is deliberately separate from Pascal parity. Multi-slot controls omit
the target. Sole-formula controls substitute Integer Power; these weaker
controls are labelled explicitly and cannot certify formula semantics.
"""
import argparse,concurrent.futures,hashlib,json,struct,subprocess
from pathlib import Path
from external_validation import decode,slots,write_json,digest
from compatibility_precision import bind_compatible, precision_evidence
ROOT=Path(__file__).resolve().parents[2]
def geometry(path):
 data=path.read_bytes()
 if data[:8]!=b'SIL5DMP1':raise ValueError('bad siLight signature')
 v,w,h,size,count=struct.unpack_from('<5I',data,8)
 if v!=1 or size!=18 or count!=w*h or len(data)!=28+count*size:raise ValueError('bad siLight layout')
 z=[struct.unpack_from('<H',data,28+i*size+8)[0] for i in range(count)]
 return z

def main():
 p=argparse.ArgumentParser();p.add_argument('--manifest',type=Path,required=True);p.add_argument('--cpp',type=Path,required=True);p.add_argument('--output',type=Path,required=True);p.add_argument('--jobs',type=int,default=4);a=p.parse_args()
 manifest=json.loads(a.manifest.read_text());a.output.mkdir(parents=True,exist_ok=True);exe=a.cpp.resolve();exe_hash=digest(exe.read_bytes())
 def render(scene,stem):
  dump=a.output/(stem+'.silight');png=a.output/(stem+'.png')
  if digest(exe.read_bytes())!=exe_hash:raise ValueError('C++ executable changed')
  cmd=[str(exe),str(scene.resolve()),'--output',str(png.resolve()),'--silight5-dump',str(dump.resolve()),'--threads','1','--deterministic']
  bind_compatible(cmd,png)
  r=subprocess.run(cmd,cwd=ROOT,capture_output=True,timeout=30)
  if r.returncode:raise ValueError(f'exit {r.returncode}: {(r.stdout+r.stderr).decode(errors="replace")[-600:]}')
  precision_evidence(png)
  return geometry(dump)
 def work(case):
  record={'case':case['id'],'scene_sha256':case['sha256'],'formulas':[]}
  try:
   if digest((ROOT/case['scene']).read_bytes())!=case['sha256']:raise ValueError('Scene changed since manifest')
   data=decode(ROOT/case['scene']);active=slots(data);z=render(ROOT/case['scene'],case['id']);record['geometry_pixels']=sum(v<32768 for v in z);record['pixels']=len(z)
   for target in case['target_formulas']:
    control=bytearray(data);targets=[f for f in active if f['number']>=20 and f['name'].casefold()==target.casefold()]
    entry={'name':target,'target_slots':[f['slot'] for f in targets]};record['formulas'].append(entry)
    if not targets:entry['status']='target-absent';continue
    kind='omit-target' if len(active)>len(targets) else 'substitute-builtin'
    for f in targets:
     off=848+f['slot']*188
     if kind=='omit-target':struct.pack_into('<i',control,off,0)
     else:
      struct.pack_into('<ii',control,off+4,0,2);control[off+12:off+44]=b'Integer Power'.ljust(32,b'\0');control[off+44:off+60]=bytes(16);struct.pack_into('<16d',control,off+60,2.,-1.,*([0.]*14))
    stem=case['id']+'_without_'+target;path=a.output/(stem+'.m3p');path.write_bytes(control)
    entry['control']=kind;entry['control_sha256']=digest(control)
    try:
     other=render(path,stem);entry['changed_depth_pixels']=sum(x!=y for x,y in zip(z,other));entry['changed_hit_pixels']=sum((x<32768)!=(y<32768) for x,y in zip(z,other));entry['status']='influential' if entry['changed_depth_pixels'] else 'no-measured-depth-influence'
    except (ValueError,subprocess.TimeoutExpired) as exc:entry['status']='control-error';entry['error']=str(exc)
  except (ValueError,OSError,subprocess.TimeoutExpired) as exc:record['error']=str(exc)
  return record
 report={'scope':__doc__,'cpp_sha256':exe_hash,'manifest_sha256':digest(a.manifest.read_bytes()),'results':[]}
 with concurrent.futures.ThreadPoolExecutor(max_workers=a.jobs) as pool:
  for i,result in enumerate(pool.map(work,manifest['cases']),1):
   report['results'].append(result)
   if i%25==0:print(f'{i}/{len(manifest["cases"])}',flush=True);write_json(a.output/'summary.json',report)
 from collections import Counter
 report['summary']={'scenes':len(report['results']),'errors':sum('error' in r for r in report['results']),'empty_geometry':sum(r.get('geometry_pixels')==0 for r in report['results']),'outcomes':dict(Counter(f['status'] for r in report['results'] for f in r['formulas']))}
 write_json(a.output/'summary.json',report);print(report['summary'])
if __name__=='__main__':main()
