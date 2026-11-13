#!/usr/bin/env python3
"""Inventory P1-prepared P2 programs without dispatching GPU or formula work.

Requires an explicit retained P1 build and its ordinary-scene request inventory.
These archived objects are diagnostic inputs, not bootstrap dependencies.
"""
import argparse,base64,hashlib,json,shlex,subprocess
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
def sha(p):return hashlib.sha256(Path(p).read_bytes()).hexdigest()
def main():
 ap=argparse.ArgumentParser(description=__doc__);ap.add_argument('--inventory',type=Path,required=True);ap.add_argument('--p1-build',type=Path,required=True);ap.add_argument('--output',type=Path,required=True);a=ap.parse_args()
 out=a.output.resolve();out.mkdir(parents=True,exist_ok=False)
 p1=json.loads(a.p1_build.read_text());inventory=json.loads(a.inventory.read_text())['ordinary'];queue=json.loads((ROOT/'comparison/scene-queue.json').read_text())['rows']
 if len(inventory)!=77 or {x['id']for x in inventory}!={x['id']for x in queue}:raise ValueError('ordinary inventory differs')
 # Locate the selected P1 preparation archive from its captured link command.
 archive=next(Path(x) for x in p1['commands'][-1] if str(x).endswith('/libmb3d_p1_preparation.a'));headers=archive.parent/'engine-headers'
 if sha(archive)!=p1['preparation_archive_sha256']:raise ValueError('P1 preparation archive changed')
 libraries=[x for x in p1['engine_artifacts'] if x.endswith('.a')]
 if any(sha(x)!=p1['engine_artifacts'][x] for x in libraries):raise ValueError('P1 engine archive changed')
 source=ROOT/'tests/p2_prepare_scene_programs.cpp'
 command=['xcrun','clang++','-std=c++20','-O2','-arch','arm64','-fno-fast-math','-ffp-contract=off','-DMB3D_ENABLE_PARITY_TRACE','-I'+str(headers),'-I'+str(ROOT),'-I'+str(headers/'src'),'-I'+str(headers/'include'),'-MMD','-MF',str(out/'prepare.d'),str(source),str(archive),*libraries,'-framework','Cocoa','-framework','Metal','-lz','-o',str(out/'prepare')]
 r={'scope':'read-only prepared-program inventory; no numerical dispatch or acceptance','source_commit':subprocess.check_output(['git','rev-parse','HEAD'],cwd=ROOT,text=True).strip(),'command':command,'p1_build_sha256':sha(a.p1_build),'inventory_sha256':sha(a.inventory),'archive_sha256':sha(archive),'libraries':{x:sha(x) for x in libraries},'status':'BUILDING','rows':[]}
 def save():(out/'receipt.json').write_text(json.dumps(r,indent=2)+'\n')
 save();v=subprocess.run(command,capture_output=True,text=True,timeout=120);(out/'compile.log').write_text(v.stdout+v.stderr);r['compile_exit']=v.returncode;save();v.check_returncode()
 deps=shlex.split((out/'prepare.d').read_text().replace('\\\n',' ').split(':',1)[1]);r['dependencies']={str(Path(x).resolve()):sha(x)for x in deps};r['dependencies'][str(Path(__file__).resolve())]=sha(__file__);r['executable_sha256']=sha(out/'prepare')
 # Every project header is bound to the selected P1 dependency record, including
 # engine-headers aliases. A new diagnostic source is the only code addition.
 for name,digest in r['dependencies'].items():
  path=Path(name)
  if path==source or path==Path(__file__).resolve():continue
  if not path.is_relative_to(ROOT):continue
  canonical=str(ROOT/path.relative_to(headers)) if path.is_relative_to(headers) else name
  if p1['dependencies'].get(name,p1['dependencies'].get(canonical))!=digest:raise ValueError('Unbound P1 header '+name)
 for row in inventory:
  if sha(ROOT/row['scene'])!=row['sha256']:raise ValueError('scene changed')
  request=json.loads(Path(row['request']).read_text());directory=out/row['id'];directory.mkdir()
  for name in ['header','addon']:(directory/(name+'.bin')).write_bytes(base64.b64decode(request[name],validate=True))
  for fast in [0,1]:
   dest=directory/('fastgeometry' if fast else 'points');cmd=[str(out/'prepare'),str(directory/'header.bin'),str(directory/'addon.bin'),request['root'],str(dest),str(fast)]
   result=subprocess.run(cmd,capture_output=True,text=True,timeout=15)
   if result.returncode:raise RuntimeError(row['scene']+': '+result.stderr)
   item={'id':row['id'],'scene':row['scene'],'scene_sha256':row['sha256'],'request_sha256':sha(row['request']),'profile':dest.name,'argv':cmd,**json.loads(result.stdout),'files':{x.name:sha(x)for x in dest.glob('*.bin')}};r['rows'].append(item);save()
 if any(sha(x)!=digest for x,digest in r['dependencies'].items()):raise ValueError('Source changed during inventory')
 if sha(archive)!=r['archive_sha256']or any(sha(x)!=digest for x,digest in r['libraries'].items()):raise ValueError('Frozen archive changed')
 r['status']='PASS';r['driver_counts']={str(d):sum(x['driver']==d for x in r['rows'])for d in range(10)};save();print(json.dumps({'status':r['status'],'profiles':len(r['rows']),'drivers':r['driver_counts']}))
if __name__=='__main__':main()
