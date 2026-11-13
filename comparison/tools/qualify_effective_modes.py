"""Add immutable separate live postprocess-state captures to effective modes."""
import json
from pathlib import Path
import struct
from prepare_effective_modes import ROOT, OUT, EXE, EXE_SHA, capture, sha, rel
from external_validation import slots, write_json
from compare_silight5 import read_dump, FIELD_NAMES
from image_diff import load_rgb

def main():
    output=OUT/'live-qualification.json'
    if output.exists():raise ValueError('Existing live qualification must be preserved')
    if sha(EXE.read_bytes())!=EXE_SHA:raise ValueError('Reference identity')
    manifest=json.loads((OUT/'manifest.json').read_text());selected={c['id']:c for c in manifest['cases']}
    report=dict(schema_version=1,scope='separate live postprocess state, no C++ acceptance',pascal_sha256=EXE_SHA,manifest_sha256=sha((OUT/'manifest.json').read_bytes()),results=[],comparisons=[],finished=False)
    names=['effective_control','effective_normals_depth','effective_shadow_hard','effective_shadow_soft']
    source=selected['effective_shadow_hard'];data=bytearray((ROOT/source['scene']).read_bytes());data[133]=6
    path=OUT/'scenes'/'effective_shadow_gateoff.m3p';path.write_bytes(data)
    gateoff=dict(source,id=path.stem,scene=rel(path),sha256=sha(data),control='effective_shadow_hard',purpose='single-bit shadow auto-off intervention')
    selected[gateoff['id']]=gateoff;names.append(gateoff['id']);report['additional_scene']=gateoff
    for name in names:
        row=capture(OUT,selected[name],EXE,live=True);report['results'].append(row);print(name,row['status'],flush=True)
    for a,b,field in [('effective_control','effective_normals_depth','normal'),('effective_shadow_gateoff','effective_shadow_hard','shadow'),('effective_shadow_hard','effective_shadow_soft','shadow')]:
        pa=OUT/'live'/(a+'-live.png');pb=OUT/'live'/(b+'-live.png')
        _,_,ra=read_dump(pa.with_suffix('.silight'));_,_,rb=read_dump(pb.with_suffix('.silight'))
        changes={f:sum(x[i]!=y[i] for x,y in zip(ra,rb)) for i,f in enumerate(FIELD_NAMES)}
        report['comparisons'].append(dict(control=a,enabled=b,target=field,changed_silight_fields=changes,changed_rgb_pixels=int((load_rgb(pa)!=load_rgb(pb)).any(axis=2).sum()),
            target_state_changed=any(v for k,v in changes.items() if k.startswith(field))))
    for name in names[:-1]:
        same=bool((load_rgb(OUT/'live'/(name+'-live.png'))==load_rgb(OUT/'pascal'/(name+'.png'))).all())
        report['results'][names.index(name)]['exact_rgb_live_dump_invariance']=same
    report['finished']=True;report['qualification']='target state changed with isolated enable/mode controls' if all(c['target_state_changed'] for c in report['comparisons']) else 'some target state did not change; see comparisons'
    write_json(output,report);print(report['qualification'],flush=True)

if __name__=='__main__':main()
