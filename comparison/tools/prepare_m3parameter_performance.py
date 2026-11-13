#!/usr/bin/env python3
"""Reproduce the isolated performance candidate from the pinned package1 source."""
import argparse, shutil, subprocess
from pathlib import Path
from run_m3parameter_gpu import ROOT, read, write, sha, require, verify_inputs

def main():
    p=argparse.ArgumentParser()
    p.add_argument('--baseline',type=Path,default=ROOT/'comparison/runs/m3parameter-package1-20261024/candidate02')
    p.add_argument('--output',type=Path,required=True)
    a=p.parse_args();base=a.baseline.resolve();out=a.output.resolve()
    verify_inputs(base)
    evidence=ROOT/'comparison/results/m3parameter-metal-performance-20261024'
    checkpoint=read(evidence/'checkpoint.json')
    require(sha(base/'source-manifest.json')==checkpoint['baseline_source_manifest_sha256'],'Wrong baseline')
    require(not out.exists(),'Fresh output required')
    require(shutil.disk_usage(out.parent).free>13*1024**3,'Need 13 GiB before preparation')
    out.mkdir()
    for name in ['source','inputs','runtime','resources']:shutil.copytree(base/name,out/name)
    (out/'provenance').mkdir();shutil.copyfile(base/'provenance/capture.py',out/'provenance/capture.py')
    require(sha(evidence/'implementation.patch')==checkpoint['patch_sha256'],'Patch changed')
    with (out/'provenance/patch.log').open('w') as log:
        subprocess.run(['patch','-p1','--batch','-i',str(evidence/'implementation.patch')],cwd=out/'source',stdout=log,stderr=subprocess.STDOUT,check=True)
    for row in read(evidence/'changed-files.json'):
        require(sha(out/'source'/row['path'])==row['candidate_sha256'],'Patched source mismatch')
    manifest=read(base/'source-manifest.json')
    manifest['parent_manifest_sha256']=sha(base/'source-manifest.json')
    manifest['selection']='Package1 plus audited pixel independence and bounded larger independent dispatch'
    manifest['source_files']={str(f.relative_to(out/'source')):sha(f) for f in (out/'source').rglob('*') if f.is_file()}
    write(out/'source-manifest.json',manifest)
    contract=read(base/'contract.json');contract['source_manifest_sha256']=sha(out/'source-manifest.json');write(out/'contract.json',contract)
    print('Performance source ready:',out)

if __name__=='__main__':main()
