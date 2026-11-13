"""Prove a rebuilt archive differs only in two ar member dates, by exact old hash."""
from pathlib import Path
import argparse,json,hashlib

def run(p,qualified,out):
 old=json.loads(qualified.read_text());lib=p/'build/libmb3d_render_backend.a';wanted=next(x['sha256'] for x in old['products'] if x['path']=='build/libmb3d_render_backend.a')
 rs=[json.loads((p/x['path']).read_text()) for x in old['records']];objects=[r for r in rs if r.get('output',{}).get('path','').endswith('general_renderer.mm.o')];ars=[r for r in rs if r.get('output',{}).get('path')==str(lib)]
 base=bytearray(lib.read_bytes());assert base[:8]==b'!<arch>\n';i=8;fields={}
 while i<len(base):
  h=base[i:i+60];name=h[:16].decode().strip();size=int(h[48:58]);body=base[i+60:i+60+size]
  if name.startswith('#1/'):name=body[:int(name[3:])].decode().strip('\0')
  if name in ['__.SYMDEF','general_renderer.mm.o']:fields[name]=i+16
  i+=60+size+size%2
 objtime=objects[0]['output']['mtime_ns']//10**9;times=set()
 for a in ars:
  for k in ['start_time_ns','end_time_ns']:
   t=a[k]//10**9;times.update(range(t-10,t+11)) # Darwin symbol table dates may be in the near future.
 match=None
 for t in sorted(times):
  b=bytearray(base);changes=[]
  for name,val in [('__.SYMDEF',t),('general_renderer.mm.o',objtime)]:
   off=fields[name];before=bytes(b[off:off+12]);after=str(val).ljust(12).encode();b[off:off+12]=after;changes.append({'member':name,'offset':off,'current_date':before.decode(),'qualified_date':after.decode()})
  if hashlib.sha256(b).hexdigest()==wanted:match=changes;break
 assert match,'Not a timestamp-only archive change'
 result={'status':'PASS_TIMESTAMP_ONLY_ARCHIVE_CHANGE','archive':str(lib),'qualified_sha256':wanted,'current_sha256':hashlib.sha256(base).hexdigest(),'reconstructed_original_sha256':hashlib.sha256(b).hexdigest(),'only_changed_fields':match,'all_member_payloads_unchanged':True,'qualified_executables_objects_and_AIR_unchanged':all(hashlib.sha256((p/x['path']).read_bytes()).hexdigest()==x['sha256'] for x in old['products'] if x['path']!='build/libmb3d_render_backend.a')}
 assert result['qualified_executables_objects_and_AIR_unchanged'];out.write_text(json.dumps(result,indent=2)+'\n');print('Archive date-only proof PASS')
if __name__=='__main__':
 a=argparse.ArgumentParser();a.add_argument('--package',type=Path,required=True);a.add_argument('--qualified',type=Path,required=True);a.add_argument('--output',type=Path,required=True);v=a.parse_args();run(v.package.resolve(),v.qualified.resolve(),v.output.resolve())
