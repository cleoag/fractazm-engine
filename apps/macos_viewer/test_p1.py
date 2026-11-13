#!/usr/bin/env python3
"""Bounded P1 checks on frozen apps. Each frame has a 180 s guard; no qualification.
Run through run_m3parameter_gpu.invoke(exclusive_gpu=True), never unfiltered discovery.
"""
import base64
import struct
import subprocess
import argparse
import hashlib
import json
import math
import os
from pathlib import Path
import statistics
import time
from PIL import Image
from check_scenes import Service, save

p=argparse.ArgumentParser(description=__doc__)
p.add_argument('--package',type=Path,required=True)
p.add_argument('--baseline',type=Path,required=True)
p.add_argument('--suite',choices=('smoke','corpus','latency','full'),default='smoke')
a=p.parse_args(); package=a.package.resolve(); baseline=a.baseline.resolve()
out=package/('p1-'+a.suite);out.mkdir(exist_ok=False)
os.environ['MB3D_VIEWER_GENERAL_PREVIEW']='1'
exe=lambda x:x/'final/Mandelbulb Viewer.app/Contents/MacOS/MandelbulbViewer'
new,old,control=(Service(exe(x),out,name) for x,name in [(package,'new'),(baseline,'old'),(package,'control')])
rows=json.loads((package/'inventory.json').read_text())['ordinary']
records=[]; summaries=[]
def run(s,q,label):
    dest=out/label;dest.mkdir()
    r,rgb=s.run(q,label,dest,time.monotonic()+180)
    assert r['status'] not in ('transport_failed','guarded'),r
    if s is not old:
        assert r['protocol']==1 and r['request_id']==s.request_serial,r
        for k in ('scene_revision','camera_revision','quality_revision','epoch'):assert r[k]==q[k],(k,r)
        assert r['presentation']=='unpresented' and r.get('present_seconds') is None,r
    records.append(r)
    with (out/'requests.jsonl').open('a') as f:f.write(json.dumps(r)+'\n')
    return r,rgb

def exact(a,b):
    assert a['status']==b['status'],(a,b)
    keys=('rgb_sha256','formula_work','samples','covered','width','height','phases','engine') if a['status']=='completed' else ('error',)
    assert all(a.get(k)==b.get(k) for k in keys),{k:(a.get(k),b.get(k)) for k in keys}

def request(row):
    q=json.loads((package/'requests'/(row['id']+'.json')).read_text());q.update(edge=64,phase=0)
    return q
try:
    chosen=rows if a.suite in ('corpus','latency') else [r for r in rows if r['rank'] in ((3,37) if a.suite=='full' else (3,12,17,56))]
    for row in chosen:
        q=request(row);rank=row['rank'];print('P1',a.suite,rank,flush=True)
        if a.suite=='latency':
            warm,_=run(new,q,f'{rank}-preflight')
            samples=[];first=warm if warm['status']=='completed' and warm['engine'].startswith('general_navigation') else None;failures=[];general_count=0
            for i in range(20):
                moved=json.loads((package/'moved-requests/p1-series'/f'{row["id"]}-{i:02}.json').read_text());moved.update(edge=64,phase=0)
                r,_=run(new,moved,f'{rank}-motion-{i}')
                fresh,_=run(control,dict(moved,disable_reuse=True),f'{rank}-fresh-motion-{i}');exact(r,fresh)
                if r['status']=='completed' and r['engine'].startswith('general_navigation'):
                    first=first or r
                    assert r['scene_preparations']==first['scene_preparations'],r
                    assert r['resource_uploads']==first['resource_uploads'],r
                    general_count+=1
                if r['status']=='completed':
                    r['publish_duration_seconds']=r['publish_seconds']-r['service_begin_seconds']
                    samples.append(r)
                else:failures.append(dict(input=i,status=r['status'],error=r.get('error')));first=None
            metrics={}
            for key in ('prepare_seconds','general_gpu_seconds','publish_duration_seconds','seconds','wall_seconds'):
                values=sorted(r[key] for r in samples if key not in ('prepare_seconds','general_gpu_seconds') or r['engine'].startswith('general_navigation'))
                metrics[key]=dict(n=len(values),p50=statistics.median(values) if values else None,p95=values[math.ceil(.95*len(values))-1] if values else None,max=max(values) if values else None)
            summaries.append(dict(rank=rank,inputs=20,successful=len(samples),successful_general=general_count,failures=failures,metrics=metrics,presentation=dict(n=0,unpresented=20,p50=None,p95=None,max=None),cold_compile_seconds=warm.get('compile_seconds')))
            save(out/'summary.json',summaries);continue
        modes={}
        for mode in (('full','noao') if a.suite=='full' else ('points','fastgeometry')):
            q['mode']=mode
            reference,_=run(old,q,f'{rank}-{mode}-old')
            current,rgb=run(new,q,f'{rank}-{mode}-new');exact(reference,current);modes[mode]=current
            fresh,_=run(control,dict(q,disable_reuse=True,disable_telemetry=True),f'{rank}-{mode}-fresh');exact(current,fresh)
            for i in range(2):
                repeat,_=run(new,q,f'{rank}-{mode}-repeat-{i}');exact(current,repeat)
            if rgb:Image.frombytes('RGB',(current['width'],current['height']),rgb).save(out/f'{rank}-{mode}.png')
            if a.suite=='smoke' and current['status']=='completed':
                moved=json.loads((package/'moved-requests'/(row['id']+'-orbit.json')).read_text());moved.update(edge=64,mode=mode)
                r,_=run(new,moved,f'{rank}-{mode}-move');f,_=run(control,dict(moved,disable_reuse=True),f'{rank}-{mode}-move-fresh');exact(r,f)
                if r['status']=='completed' and r['engine'].startswith('general_navigation'):
                    assert r['scene_preparations']==current['scene_preparations'] and r['resource_uploads']==current['resource_uploads'],r
                reloaded,_=run(new,dict(q,scene_revision=2),f'{rank}-{mode}-reload');exact(current,reloaded)
                for key,value in [('edge',96),('threads',2),('root',q['root']+'/.')]:
                    changed=dict(q,**{key:value})
                    r,_=run(new,changed,f'{rank}-{mode}-{key}');f,_=run(control,dict(changed,disable_reuse=True),f'{rank}-{mode}-{key}-fresh');exact(r,f)
        if a.suite=='full':
            assert modes['full']['status']=='completed' and modes['full']['ao_active'],modes
            assert modes['noao']['status']=='completed' and not modes['noao']['ao_active'],modes
            assert modes['full']['rgb_sha256']!=modes['noao']['rgb_sha256'],modes
            cpu= dict(q,backend='cpu',mode='full');before,_=run(old,cpu,f'{rank}-cpu-old');after,_=run(new,cpu,f'{rank}-cpu-new');exact(before,after)
        summaries.append(dict(rank=rank,old_new_exact=True,three_repeats=True))
        save(out/'summary.json',summaries)
    if a.suite=='smoke':
        q=request(chosen[0])
        # Use raw Service so deliberate protocol rejection is not mistaken for a missing envelope.
        for i,(key,value) in enumerate([('protocol',999),('protocol',1.5),('protocol',True),('scene_revision',-1),('camera_revision',1.5),('input_seconds',2),('threads',{}),('threads',2.5),('threads',True),('disable_reuse',{}),('disable_telemetry',[]),('diagnostic_timeout',{})]):
            bad=dict(q,**{key:value});dest=out/f'bad-{i}';dest.mkdir()
            r,rgb=new.run(bad,f'bad-{i}',dest,time.monotonic()+180)
            assert r['status']=='failed' and not rgb and 'protocol' in r['error'],r
            r['rejected_envelope']={k:bad.get(k) for k in ('protocol','request_id','scene_revision','camera_revision','input_seconds','submit_seconds')}
            records.append(r)
            with (out/'requests.jsonl').open('a') as f:f.write(json.dumps(r)+'\n')
        recovered,_=run(new,q,'recovery');assert recovered['status']=='completed'
        resource_root=out/'mutable-resources';(resource_root/'M3Maps').mkdir(parents=True)
        mapfile=resource_root/'M3Maps/1.bmp'
        Image.new('RGB',(8,8),(20,40,60)).save(mapfile)
        mapped=json.loads(subprocess.check_output([str(exe(package)),'--request','comparison/scenes/extall_Ads__MapTranslate.m3p',str(resource_root),'metal','64','fastgeometry'],text=True,timeout=10))
        data=bytearray(base64.b64decode(mapped['addon']));struct.pack_into('<d',data,8+60,1.)
        mapped['addon']=base64.b64encode(data).decode()
        before,before_rgb=run(new,mapped,'map-before');assert before['status']=='completed',before
        camera_inputs=out/'mapped-camera-requests';camera_inputs.mkdir();save(camera_inputs/'mapped.json',mapped)
        camera_outputs=out/'mapped-camera-motions'
        subprocess.run([str(package/'test_camera_snapshots'),str(camera_inputs),str(camera_outputs),'--single-fixture'],check=True,timeout=30)
        for i in range(20):
            moved=json.loads((camera_outputs/'p1-series'/f'mapped-{i:02}.json').read_text());moved.update(edge=64,mode='fastgeometry')
            r,_=run(new,moved,f'mapped-motion-{i}');f,_=run(control,dict(moved,disable_reuse=True),f'mapped-fresh-motion-{i}');exact(r,f)
            assert r['scene_preparations']==before['scene_preparations'] and r['resource_uploads']==before['resource_uploads']
            assert r['resource_identity']==before['resource_identity']
        altered=bytearray(base64.b64decode(mapped['addon']));struct.pack_into('<d',altered,8+60+8,.001)
        changed=dict(mapped,addon=base64.b64encode(altered).decode())
        r,_=run(new,changed,'formula-parameter-change');f,_=run(control,dict(changed,disable_reuse=True),'formula-parameter-fresh');exact(r,f)
        assert r['scene_preparations']==before['scene_preparations']+1
        restored,_=run(new,mapped,'restored-original-parameters');exact(before,restored)
        assert restored['resource_identity']==before['resource_identity']
        old_bytes=mapfile.read_bytes()
        Image.new('RGB',(8,8),(80,100,120)).save(mapfile)
        held,_=run(new,mapped,'map-held');exact(before,held)
        assert held['resource_identity']==before['resource_identity']
        loaded,_=run(new,dict(mapped,scene_revision=2),'map-reloaded');assert loaded['status']=='completed',loaded
        assert loaded['resource_identity']!=before['resource_identity']
        assert loaded['resource_uploads']==held['resource_uploads']+1
        isolated,_=run(control,mapped,'map-other-session');assert isolated['resource_identity']==loaded['resource_identity']
        mapfile.unlink()
        retained,_=run(new,dict(mapped,scene_revision=2),'map-retained-after-delete');exact(loaded,retained)
        failed,_=run(new,dict(mapped,scene_revision=3),'map-missing-reload');assert failed['status']=='failed' and 'unavailable' in failed['error']
        mapfile.write_bytes(old_bytes)
        recovered,_=run(new,dict(mapped,scene_revision=4),'map-recovery');exact(before,recovered)
        assert hashlib.sha256(before_rgb).hexdigest()==before['rgb_sha256']

    save(out/'result.json',dict(status='PASS',suite=a.suite,requests=len(records),scenes=len(chosen),qualification=False,summary=summaries))
    print('PASS',len(records),'requests',flush=True)
finally:
    for s in (new,old,control):s.close()
