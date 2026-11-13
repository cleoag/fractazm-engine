"""Reproduce finite valid fractal+dIFS mix controls, retaining independent refs."""
import argparse
import json
import struct
from collections import Counter
from pathlib import Path

from external_validation import decode, slots, write_json
from prepare_effective_modes import ROOT, EXE, EXE_SHA, capture
from prepare_stage_a_modes import rel, sha
from image_diff import DEFAULT_THRESHOLDS

def main():
    ap=argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--output',type=Path,default=ROOT/'comparison/results/stage-a-20261018/de-mix-controls-v1')
    ap.add_argument('--render-reference',action='store_true')
    args=ap.parse_args();out=args.output.resolve()
    if out.exists():raise ValueError('Use a fresh output directory')
    out.mkdir(parents=True);(out/'scenes').mkdir()
    first=ROOT/'comparison/results/stage-a-20261018/effective-modes-v1/scenes/effective_comb5.m3p'
    second=ROOT/'comparison/scenes/extall_dIFS_SphereIFS.m3p'
    base=decode(first);sphere=decode(second)[848:1036]
    cases=[]
    for power in (0,2):
        for color in (0,1,2):
            data=bytearray(base);data[1036:1224]=sphere
            struct.pack_into('<ii',data,1036,1,20)
            # A centered unit sphere on the final orbit, without scale/add.
            for index,value in {1:1,4:0,8:0}.items():struct.pack_into('<d',data,1036+60+8*index,value)
            struct.pack_into('<f',data,104,power);data[422]=color
            path=out/'scenes'/f'mix_power{power}_color{color}.m3p';path.write_bytes(data)
            cases.append(dict(id=path.stem,scene=rel(path),sha256=sha(data),slots=slots(data),target_formulas=['SphereIFS'],
                control=None,purpose='valid integer-power + centered SphereIFS mix',saved_mix_power=power,
                expected_effective_mix_power=.01 if power==0 else power,color=color))
    manifest=dict(schema_version=1,scope='ordinary mix finite controls; no exhaustive proof',sources={rel(p):sha(p.read_bytes()) for p in (first,second)},cases=cases)
    write_json(out/'manifest.json',manifest)
    if not args.render_reference:return
    if sha(EXE.read_bytes())!=EXE_SHA:raise ValueError('Original executable identity changed')
    report=dict(schema_version=1,finished=False,batches=[],pascal_sha256=EXE_SHA,provenance=dict(pascal_exe=rel(EXE),pascal_sha256=EXE_SHA,tool_sha256=sha(Path(__file__).read_bytes())),
                manifest_sha256=sha((out/'manifest.json').read_bytes()),thresholds=DEFAULT_THRESHOLDS,results=[])
    for case in cases:
        row=capture(out,case,EXE);row['scene_sha256']=case['sha256'];report['results'].append(row)
        report['counts']=dict(Counter(x['status'] for x in report['results']));write_json(out/'summary.json',report)
        print(case['id'],row['status'],row.get('hit_pixels'),flush=True)
    report['finished']=True;write_json(out/'summary.json',report)

if __name__=='__main__':main()
