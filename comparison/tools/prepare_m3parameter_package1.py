#!/usr/bin/env python3
"""Reproduce package1's isolated source from the pinned package0 candidate."""
import argparse,shutil,subprocess
from pathlib import Path
from run_m3parameter_gpu import ROOT,read,write,sha,require,verify_inputs

def main():
 p=argparse.ArgumentParser();p.add_argument('--baseline',type=Path,default=ROOT/'comparison/runs/m3parameter-package0-20261024/candidate02');p.add_argument('--output',type=Path,required=True);a=p.parse_args()
 base=a.baseline.resolve();out=a.output.resolve();verify_inputs(base)
 require(sha(base/'source-manifest.json')=='2024dadd98db72ca91ba1487a8aaaee1964aa57bb9bb0549a92283ba7c5543a5','Wrong package0 source')
 require(not out.exists(),'Fresh output required');require(shutil.disk_usage(out.parent).free>13*1024**3,'Need 13 GiB before source copy/build')
 changes=ROOT/'comparison/results/m3parameter-package1-20261024';files=read(changes/'changed-files.json')
 out.mkdir()
 for name in ['source','inputs','runtime','resources']:shutil.copytree(base/name,out/name)
 (out/'provenance').mkdir();shutil.copyfile(base/'provenance/capture.py',out/'provenance/capture.py')
 for f in files:
  path=out/'source'/f['path'];require(path.resolve().is_relative_to((out/'source').resolve()),'Escaping patch path')
  require(sha(path)==f['baseline_sha256'] if path.exists() else f['baseline_sha256'] is None,'Patch baseline mismatch')
 with (out/'provenance/patch.log').open('w') as log:
  subprocess.run(['patch','-p1','--batch','-i',str(changes/'implementation.patch')],cwd=out/'source',stdout=log,stderr=subprocess.STDOUT,check=True)
 for f in files:require(sha(out/'source'/f['path'])==f['candidate_sha256'],'Patched source mismatch')
 sm=read(base/'source-manifest.json');sm['parent_manifest_sha256']=sha(base/'source-manifest.json');sm['selection']='Package0 candidate02 + package1 Native directional light/color extension';sm['implementation_patch_sha256']=sha(changes/'implementation.patch');sm['source_files']={str(f.relative_to(out/'source')):sha(f) for f in (out/'source').rglob('*') if f.is_file()};write(out/'source-manifest.json',sm)
 c=read(base/'contract.json');c['source_manifest_sha256']=sha(out/'source-manifest.json');write(out/'contract.json',c)
 print('Package1 source ready:',out)
if __name__=='__main__':main()
