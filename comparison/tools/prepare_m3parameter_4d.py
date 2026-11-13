#!/usr/bin/env python3
"""Reconstruct the pinned isolated 4D scheduling candidate, without archived objects."""
import argparse,shutil,subprocess
from pathlib import Path
from run_m3parameter_gpu import ROOT,read,write,sha,require,verify_inputs

def main():
 p=argparse.ArgumentParser();p.add_argument('--baseline',type=Path,default=ROOT/'comparison/runs/m3parameter-metal-performance-20261024/candidate04');p.add_argument('--output',type=Path,required=True);a=p.parse_args();base=a.baseline.resolve();out=a.output.resolve()
 verify_inputs(base);evidence=ROOT/'comparison/results/m3parameter-metal-stateful-20261024';checkpoint=read(evidence/'checkpoint.json');manifest=read(evidence/'source-manifest.json')
 require(sha(base/'source-manifest.json')==checkpoint['baseline_source_manifest_sha256'],'Wrong baseline');require(not out.exists(),'Fresh output required');require(shutil.disk_usage(out.parent).free>13*1024**3,'Need 13 GiB')
 require(sha(evidence/'implementation.patch')==checkpoint['patch_sha256'],'Patch changed');out.mkdir()
 for name in ['source','inputs','runtime','resources']:subprocess.run(['cp','-cR',str(base/name),str(out/name)],check=True)
 (out/'provenance').mkdir();shutil.copyfile(base/'provenance/capture.py',out/'provenance/capture.py')
 with (out/'provenance/patch.log').open('w') as log:subprocess.run(['patch','-p1','--batch','-i',str(evidence/'implementation.patch')],cwd=out/'source',stdout=log,stderr=subprocess.STDOUT,check=True)
 actual={str(f.relative_to(out/'source')):sha(f) for f in (out/'source').rglob('*') if f.is_file()};require(actual==manifest['source_files'],'Reconstructed source differs')
 shutil.copyfile(evidence/'source-manifest.json',out/'source-manifest.json');contract=read(base/'contract.json');contract['source_manifest_sha256']=sha(out/'source-manifest.json');write(out/'contract.json',contract)
 print('Verified source files:',len(actual))
if __name__=='__main__':main()
