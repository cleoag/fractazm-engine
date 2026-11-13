#!/usr/bin/env python3
"""Replay sealed numeric captures; report evidence without granting qualification."""
import argparse
import hashlib
import json
import sys
from pathlib import Path

import numpy as np
from PIL import Image


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def statistics(values):
    values=np.asarray(values,dtype=np.float64)
    if not values.size:return {'count':0}
    return {'count':values.size,'signed_mean':float(values.mean()),
            'mean_absolute':float(np.abs(values).mean()),
            'absolute_quantiles':dict(zip(('p50','p90','p99','max'),map(float,np.quantile(np.abs(values),(.5,.9,.99,1)))))}


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--repository',type=Path,required=True)
    p.add_argument('--capture',type=Path,required=True)
    p.add_argument('--output',type=Path,required=True)
    a=p.parse_args();root=a.repository.resolve();capture=a.capture.resolve()
    sys.path.insert(0,str(root/'comparison/tools'))
    from compare_silight5 import read_dump,compare_pair,FIELD_NAMES
    record=json.loads(capture.read_text())
    assert record['status']=='CAPTURE_COMPLETE','Complete CPU/GPU captures required'
    folders={};identities={}
    for label,job in record['jobs'].items():
        path=root/job['event'];event=json.loads(path.read_text())
        assert event['exit_code']==0 and event['guard'] is None and event['seconds']<=180
        assert event['executable_sha256_before']==event['executable_sha256_after']
        assert '--deterministic' not in event['argv']
        for artifact in event['inputs']+event['outputs']:
            assert digest(root/artifact['path'])==artifact['sha256'],artifact['path']
        folders[label]=path.parent
        identities[label]={'event':job['event'],'event_sha256':digest(path),
                           'renderer_sha256':event['executable_sha256_before']}
    assert set(folders)=={'cpu-dump','cpu-shift24','gpu-dump'}
    cpu=folders['cpu-dump'];gpu=folders['gpu-dump'];shift=folders['cpu-shift24']
    images=[np.asarray(Image.open(d/'image.png').convert('RGB'),dtype=np.int32) for d in (cpu,gpu,shift)]
    c,g,s=images
    assert c.shape==g.shape==s.shape and list(c.shape[:2][::-1])==record['size']
    delta=g-c;shift_delta=s-c;h,w=c.shape[:2]
    regions={}
    for name,y,x in [('all',slice(None),slice(None)),('top_left',slice(0,h//2),slice(0,w//2)),
                     ('top_right',slice(0,h//2),slice(w//2,None)),
                     ('bottom_left',slice(h//2,None),slice(0,w//2)),
                     ('bottom_right',slice(h//2,None),slice(w//2,None))]:
        regions[name]={channel:statistics(delta[y,x,k]) for k,channel in enumerate('RGB')}
    cw,ch,cr=read_dump(cpu/'surface.bin');gw,gh,gr=read_dump(gpu/'surface.bin')
    assert (cw,ch)==(gw,gh)==(w,h) and len(cr)==len(gr)==w*h
    cr=np.asarray(cr,dtype=np.int64);gr=np.asarray(gr,dtype=np.int64)
    c_hit=cr[:,4]<=32767;g_hit=gr[:,4]<=32767;both=c_hit&g_hit
    cz=(cr[:,4]<<8)|((cr[:,3]>>8)&255);gz=(gr[:,4]<<8)|((gr[:,3]>>8)&255)
    fields={name:statistics(gr[both,k]-cr[both,k]) for k,name in enumerate(FIELD_NAMES) if k!=3}
    fields['packed_z']=statistics(gz[both]-cz[both])
    fields['roughness']=statistics((gr[both,3]&255)-(cr[both,3]&255))
    fields['march_count']=statistics((gr[both,5]&1023)-(cr[both,5]&1023))
    cn=cr[both,:3].astype(float);gn=gr[both,:3].astype(float)
    norm=np.linalg.norm(cn,axis=1)*np.linalg.norm(gn,axis=1);valid=norm>0
    angles=np.degrees(np.arccos(np.clip((cn[valid]*gn[valid]).sum(axis=1)/norm[valid],-1,1)))
    result={'schema':'mb3d.native-numeric-observation.v1','qualification':False,'status':'REVIEW_REQUIRED',
            'analyzer_sha256':digest(Path(__file__)),
            'rank':record['rank'],'scene':record['scene'],'size':record['size'],'threads':record['threads'],
            'capture_sha256':digest(capture),'identities':identities,
            'cpu_shift_rgb':statistics(shift_delta),'gpu_cpu_rgb':statistics(delta),
            'signed_rgb_regions':regions,'surface_comparison':compare_pair(cpu/'surface.bin',gpu/'surface.bin',set()),
            'cpu_hits':int(c_hit.sum()),'gpu_hits':int(g_hit.sum()),'hit_mask_disagreements':int((c_hit!=g_hit).sum()),
            'surface_field_differences_on_common_hits':fields,'normal_angle_degrees':statistics(angles)}
    a.output.parent.mkdir(parents=True,exist_ok=True)
    a.output.write_text(json.dumps(result,indent=2)+'\n')
    print(json.dumps({k:result[k] for k in ('rank','status','cpu_shift_rgb','gpu_cpu_rgb','hit_mask_disagreements','normal_angle_degrees')}))


if __name__=='__main__':main()
