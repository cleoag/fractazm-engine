#!/usr/bin/env python3
"""Reject corrupted copies of a real qualified bundle, without editing evidence."""
import argparse,copy,json
from pathlib import Path
from qualify_native_metal_all import sha,require
from gpu_pipeline_fp32 import scene_frames
ROOT=Path(__file__).resolve().parents[2]


def main():
    p=argparse.ArgumentParser(description=__doc__)
    for key in ('qualification','build','runtime','output'):p.add_argument('--'+key,type=Path,required=True)
    a=p.parse_args();b=a.build.resolve();runtime=a.runtime.resolve()
    report=json.loads((a.qualification/'report.json').read_text());require(report['status']=='PASS_FP32_CPU_SCENES','Missing successful real qualification')
    require(report['runtime_files']=={str(x.relative_to(runtime)):sha(x) for x in runtime.rglob('*') if x.is_file()},'Changed resources')
    record=report['scenes'][0]['evidence'];require(sha(ROOT/record['path'])==record['sha256'],'Changed real evidence')
    payload=json.loads((ROOT/record['path']).read_text())
    ctx={'renderer':{'path':str(b/'mb3d_render'),'sha256':sha(b/'mb3d_render')},'library':{'sha256':sha(b/'metal_native_scene.metallib')},
         'inputs':{payload['scene']['id']:ROOT/payload['scene']['path']},'threads':6,'runtime':runtime,'runtime_files':report['runtime_files']}
    scene_frames(ROOT,payload['row'],payload['scene'],ctx,payload['effects'])
    checks=[]
    def check(label,change):
        value=copy.deepcopy(payload);context=copy.deepcopy(ctx);change(value,context)
        try:scene_frames(ROOT,value['row'],value['scene'],context,value['effects'])
        except (ValueError,KeyError,TypeError,OSError) as e:checks.append({'case':label,'status':'REJECTED','reason':str(e)})
        else:raise AssertionError('Accepted corrupted evidence: '+label)
    check('two_repeats',lambda v,c:v['row']['frames'].pop())
    check('copied_repeat',lambda v,c:v['row']['frames'].__setitem__(1,v['row']['frames'][0]))
    check('missing_effect',lambda v,c:v['effects']['controls'].pop())
    check('point_only_proof',lambda v,c:v['row'].__setitem__('fp32_overflow_proof',{'point_probe':'PASS'}))
    check('wrong_final_host',lambda v,c:c['renderer'].__setitem__('sha256','0'*64))
    check('wrong_final_library',lambda v,c:c['library'].__setitem__('sha256','0'*64))
    check('wrong_resource_closure',lambda v,c:v['effects']['runtime_files'].__setitem__('M3Maps/24.jpg','0'*64))
    check('wrong_scene',lambda v,c:v['effects'].__setitem__('scene_sha256','0'*64))
    check('unbound_image',lambda v,c:v['row']['frames'][0]['image'].__setitem__('sha256','0'*64))
    check('fallback_used_as_direct_cpu_control',lambda v,c:v['row'].__setitem__('direct_cpu',v['row']['frames'][0]))
    check('recorder_summary_without_capture',lambda v,c:v['row']['fp32_overflow_proof']['witness'].pop('capture'))
    check('unbound_capture',lambda v,c:v['row']['fp32_overflow_proof']['witness'].__setitem__('capture',v['row']['fp32_overflow_proof']['baseline']['capture']))
    a.output.write_text(json.dumps({'status':'PASS','scene':payload['scene']['id'],'count':len(checks),'cases':checks},indent=2)+'\n')
    print('PASS',len(checks),'real-bundle corruption controls')


if __name__=='__main__':main()
