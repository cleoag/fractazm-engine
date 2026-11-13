#!/usr/bin/env python3
"""Freeze the promoted main checkout and retained phase-one resources for a fresh build."""
import argparse,json,hashlib,shutil,subprocess
from qualify_native_metal_all import source_snapshot
from pathlib import Path
ROOT=Path(__file__).resolve().parents[2]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 parser=argparse.ArgumentParser();parser.add_argument('--output',type=Path,required=True);a=parser.parse_args();out=a.output.resolve()
 base=ROOT/'comparison/runs/m3parameter-makin-effects-20261024/candidate04'
 assert not out.exists(),'Use a fresh output directory'
 assert shutil.disk_usage(out.parent).free>=12.375*1024**3,'Need 12 GiB reserve plus build headroom'
 manifest=json.loads((base/'source-manifest.json').read_text());out.mkdir()
 for name in ['inputs','runtime','resources']:subprocess.run(['cp','-cR',str(base/name),str(out/name)],check=True)
 # Include new headers/tests/tools; inheriting only the archived list silently
 # omitted dependency closure for newly introduced implementation files.
 manifest['source_files'].update(source_snapshot(ROOT))
 for rel in manifest['source_files']:
  p=ROOT/rel;q=out/'source'/rel;q.parent.mkdir(parents=True,exist_ok=True);shutil.copyfile(p,q);manifest['source_files'][rel]=sha(p)
 (out/'provenance').mkdir();capture=(base/'provenance/capture.py').read_text().replace("os.environ['MB3D_CLEAN_CAPTURE_CONTEXT']","os.environ.get('MB3D_CLEAN_CAPTURE_CONTEXT', str(Path(__file__).with_name('context.json')))")
 # A depfile named by -MF is an output, even when incremental compilation
 # replaces an existing file. Do not classify it as an immutable input.
 capture=capture.replace("    for value in argv[1:]:\n", "    dependency_output=(cwd/argv[argv.index('-MF')+1]).resolve() if '-MF' in argv else None\n    for value in argv[1:]:\n").replace("p!=out:", "p!=out and p!=dependency_output:")
 (out/'provenance/capture.py').write_text(capture)
 manifest.update(selection='Current promoted main checkout with pinned phase-one resources',promotion='main checkout, new verification required',capture_sha256=sha(out/'provenance/capture.py'))
 (out/'source-manifest.json').write_text(json.dumps(manifest,indent=2)+'\n')
 contract=json.loads((base/'contract.json').read_text());contract['source_manifest_sha256']=sha(out/'source-manifest.json');(out/'contract.json').write_text(json.dumps(contract,indent=2)+'\n')
 print('Frozen main source files:',len(manifest['source_files']))
if __name__=='__main__':main()
