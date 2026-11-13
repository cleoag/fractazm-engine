"""Narrow, replayed native-FP32 sensitivity reviews; never CPU/Pascal parity.

A committed review is necessary but insufficient: original-size sealed captures,
the exact camera intervention and the stated diagnostic bounds are replayed.
These bounds classify sensitivity; GPU repeats still require exact RGB/work.
"""
import statistics
import struct
from pathlib import Path

from PIL import Image,ImageChops,ImageStat
from qualify_native_metal_all import require,artifact,read_json,sha
from gpu_pipeline_proofs import run_event,output_bound,option
from run_m3parameter_gpu import validate_profile
from compare_silight5 import read_dump

REVIEW_REGISTRY='comparison/results/native-numeric-review-20261031/reviews.json'
PENDING_NAMES=('Dainbramage','Kiosk','Makin','Getting Loopy','ABoxSOff4d','Lenord - Between Dark and Light','PaMonk - Ringlets','Peacock in blue')


def pending(scene):return any(name in scene['path'] for name in PENDING_NAMES)


def sums(a,b):
    a=Image.open(a).convert('RGB');b=Image.open(b).convert('RGB')
    require(a.size==b.size,'Numeric image dimensions differ')
    absolute=list(map(int,ImageStat.Stat(ImageChops.difference(a,b)).sum))
    signed=[int(y-x) for x,y in zip(ImageStat.Stat(a).sum,ImageStat.Stat(b).sum)]
    return absolute,signed


def validate(base,review,scene,ctx,reference,first):
    from gpu_pipeline_frames import argv_shape,enabled
    r=read_json(artifact(base,review))
    require(r['schema']=='mb3d.native-camera-sensitivity-review.v1' and r['classification']=='camera_sensitivity',
            'Unsupported numerical review')
    require(r['scene_sha256']==scene['sha256'] and r['renderer_sha256']==ctx['renderer']['sha256'] and
            r['library_sha256']==ctx['library']['sha256'] and r['threads']==ctx['threads'] and
            r['runtime_files']==ctx['runtime_files'],'Stale numerical review identity/resources')
    capture=read_json(artifact(base,r['capture']))
    analysis=read_json(artifact(base,r['analysis']))
    require(analysis['capture_sha256']==r['capture']['sha256'],'Analysis belongs to different captures')
    require(capture['status']=='CAPTURE_COMPLETE' and set(capture['jobs'])=={'cpu-dump','cpu-shift24','gpu-dump'},
            'Incomplete numerical captures')
    source=ctx['inputs'][r['id']];original=source.read_bytes();shift=bytearray(original)
    for offset in (36,44,52):
        value=struct.unpack_from('<d',shift,offset)[0]
        struct.pack_into('<d',shift,offset,value+2**-24*max(abs(value),1))
    images={};surfaces={}
    for label,job in capture['jobs'].items():
        event_path=base/job['event'];e=run_event(base,{'path':job['event'],'sha256':sha(event_path)},ctx['renderer'])
        require(e['seconds']<=180,'Slow numerical diagnostic needs separate investigation')
        require(e['resource_files_before']==e['resource_files_after']==ctx['runtime_files'],'Numeric resources changed')
        require(Path(e['cwd']).resolve()==ctx['runtime'],'Numeric runtime changed')
        backend='metal' if label=='gpu-dump' else 'cpu';dump=label.endswith('dump')
        mandatory={'--backend','--precision','--width','--height','--threads','--output','--profile-json'}
        if backend=='metal':mandatory.add('--metal-library')
        if dump:mandatory.add('--silight5-dump')
        inp=Path(e['argv'][1]).resolve()
        require(inp.read_bytes()==(shift if label=='cpu-shift24' else original),'Changed numeric scene or camera intervention')
        require(any(artifact(base,x)==inp for x in e['inputs']),'Unbound numeric scene')
        argv_shape(e['argv'],mandatory,set(),inp)
        require(option(e['argv'],'--backend')==backend and option(e['argv'],'--precision')=='native' and
                [int(option(e['argv'],'--width')),int(option(e['argv'],'--height'))]==scene['settings']['size'] and
                int(option(e['argv'],'--threads'))==ctx['threads'],'Changed numeric settings')
        if backend=='metal':
            require(Path(option(e['argv'],'--metal-library')).resolve()==ctx['library_path'],'Wrong numeric library')
            require(any(artifact(base,x)==ctx['library_path'] and x['sha256']==ctx['library']['sha256'] for x in e['inputs']),
                    'Unbound numeric library')
        def output(flag):
            path=Path(option(e['argv'],flag)).resolve()
            return output_bound(base,e,{'path':str(path.relative_to(base)),'sha256':sha(path)})
        profile=read_json(output('--profile-json'));validate_profile(profile,backend,scene['settings']['size'])
        for stage,on in enabled(scene).items():
            require(profile['stages'][stage]['state']==('completed' if on else 'not_run'),'Numeric effect changed')
        images[label]=output('--output')
        if dump:surfaces[label]=output('--silight5-dump')
    require(sum(sums(images['cpu-dump'],artifact(base,reference['image']))[0])==0,'CPU review differs from retained reference')
    require(sum(sums(images['gpu-dump'],artifact(base,first['image']))[0])==0,'GPU review differs from qualified frame')
    gpu_abs,gpu_signed=sums(images['cpu-dump'],images['gpu-dump'])
    cpu_abs,_=sums(images['cpu-dump'],images['cpu-shift24'])
    require(sum(gpu_abs)<=2*sum(cpu_abs) and sum(cpu_abs)>0,'GPU difference exceeds the reviewed camera-sensitivity class')
    require(all(abs(s)*20<=a for a,s in zip(gpu_abs,gpu_signed)),'Directional RGB bias exceeds the narrow sensitivity class')
    cw,ch,cpu=read_dump(surfaces['cpu-dump']);gw,gh,gpu=read_dump(surfaces['gpu-dump'])
    require([cw,ch]==[gw,gh]==scene['settings']['size'] and len(cpu)==len(gpu)==cw*ch,'Numeric surface shape')
    require(all((a[4]>32767)==(b[4]>32767) for a,b in zip(cpu,gpu)),'Sensitivity review changed the hit mask')
    require(all((a[5]&1023)==(b[5]&1023) for a,b in zip(cpu,gpu)),'Sensitivity review changed march counts')
    common=[(a,b) for a,b in zip(cpu,gpu) if a[4]<=32767]
    require(common,'No foreground in sensitivity review')
    depth=[abs(((a[4]<<8)|((a[3]>>8)&255))-((b[4]<<8)|((b[3]>>8)&255))) for a,b in common]
    require(statistics.median(depth)<=256,'Median depth drift exceeds one coarse encoded unit')
    for axis in range(3):
        require(statistics.median(abs(a[axis]-b[axis]) for a,b in common)<=512,'Median normal drift exceeds the narrow sensitivity class')
    observed={'cpu_shift_absolute_rgb_sum':sum(cpu_abs),'gpu_cpu_absolute_rgb_sum':sum(gpu_abs),
              'gpu_cpu_signed_rgb_sums':gpu_signed,'hit_mask_disagreements':0,'march_count_disagreements':0,
              'median_packed_depth_difference':statistics.median(depth)}
    require(observed==r['observations'],'Numeric review observations changed')
    return observed


def approved(base,scene,ctx,reference,frames):
    registry=base/REVIEW_REGISTRY
    if not registry.exists():return None
    entries=read_json(registry)
    match=[]
    for entry in entries:
        if entry['scene_sha256']!=scene['sha256']:continue
        record=read_json(artifact(base,entry['review']))
        require(record['scene_sha256']==entry['scene_sha256'],'Wrong registry scene')
        if (record['renderer_sha256'],record['library_sha256'],record['threads'],record['runtime_files'])==(
                ctx['renderer']['sha256'],ctx['library']['sha256'],ctx['threads'],ctx['runtime_files']):match.append(entry)
    require(len(match)<=1,'Duplicate numerical approval')
    if not match:return None
    review=match[0]['review'];validate(base,review,scene,ctx,reference,frames[0]);return review
