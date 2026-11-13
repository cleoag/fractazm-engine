#!/usr/bin/env python3
"""Verify the captured normal-gradient fix and three original CPU-fallback frames.

Does not award full-scope or whole-frame mathematical-overflow qualification.
An existing first frame may be reused only after validating its recorded process
arguments, input/output hashes, binary/library identities and frame guard.
"""
import argparse
import json
from pathlib import Path
import struct
from native_gpu_coverage import rgb_sha256, sha256
from run_m3parameter_gpu import invoke
ROOT=Path(__file__).resolve().parents[2]


def main():
    p=argparse.ArgumentParser(description=__doc__)
    for name in ('build','runtime','output'):p.add_argument('--'+name,type=Path,required=True)
    p.add_argument('--first-frame',type=Path)
    a=p.parse_args();b=a.build.resolve();runtime=a.runtime.resolve();out=a.output.resolve()
    assert not out.exists();out.mkdir(parents=True)
    fixture=json.loads((ROOT/'comparison/fixtures/native_gradient_normal_case.json').read_text())
    scene=ROOT/fixture['scene'];assert sha256(scene)==fixture['scene_sha256']
    identities={str(x):sha256(x) for x in (b/'mb3d_render',b/'metal_native_scene.metallib',b/'mb3d_native_formula_probe')}
    resources={str(x.relative_to(runtime)):sha256(x) for x in runtime.rglob('*') if x.is_file()}
    for key in ('program','state'):
        w=fixture[key+'_words'];(out/(key+'.bin')).write_bytes(struct.pack('<'+str(len(w))+'I',*w))
    probe=out/'point';e=invoke([b/'mb3d_native_formula_probe','--program',out/'program.bin','--states',out/'state.bin',
        '--mode','distance','--repeats','3','--library',b/'metal_native_scene.metallib','--output',probe/'probe.json'],probe,ROOT,timeout=60,evidence_root=ROOT)
    assert e['exit_code']==0 and not e['guard']
    d=json.loads((probe/'probe.json').read_text());row=d['rows'][0]
    assert d['exact_state_repeats'] and d['allocation_canaries'] and d['input_immutable'] and d['invalid_descriptor_controls']==7
    assert not row['fault'] and row['result_words'][33]==fixture['expected_iteration']
    assert row['result_words'][55]==fixture['expected_distance_word'] and row['result_words'][56]==1

    def frame(folder,backend='metal',reuse=False):
        cmd=[b/'mb3d_render',scene,'--backend',backend,'--precision','native','--threads','6',
             '--output',folder/'image.png','--profile-json',folder/'profile.json']
        if backend=='metal':cmd+=['--metal-library',b/'metal_native_scene.metallib']
        event=json.loads((folder/'event.json').read_text()) if reuse else invoke(cmd,folder,runtime,timeout=300,evidence_root=ROOT)
        assert event['exit_code']==0 and not event['guard'] and not event['frame_warning_emitted'] and event['owned_process_reaped']
        assert event['executable_sha256_before']==event['executable_sha256_after']==identities[str(b/'mb3d_render')]
        assert Path(event['cwd']).resolve()==runtime
        argv=event['argv'];assert len(argv)==len(cmd) and Path(argv[0]).resolve()==b/'mb3d_render' and Path(argv[1]).resolve()==scene
        assert len(set(argv[2::2]))==len(argv[2::2])
        assert dict(zip(argv[2::2],argv[3::2]))==dict(zip(map(str,cmd[2::2]),map(str,cmd[3::2])))
        for x in event['inputs']+event['outputs']:
            f=(ROOT/x['path']).resolve();assert f.is_relative_to(ROOT) and sha256(f)==x['sha256']
        d=json.loads((folder/'profile.json').read_text());m=d['metal_execution']
        assert d['status']=='complete' and not d['error'] and d['precision']=='native'
        assert [d['width'],d['height']]==fixture['original_size'] and d['backend_requested']==backend and d['backend']=='cpu'
        assert d['backend_fallback']==(backend=='metal') and not m['faults'] and not m['nonfinite']
        if backend=='metal':
            assert d['backend_fallback_code']=='fp32_range' and m['fp32_range_gate_stage']=='primary'
            assert m['fp32_range_gate_units']==480000 and m['fp32_range_gate_percent']==1
            assert 4800<m['fp32_range_gate_exceeded']<=480000
        return {'folder':str(folder),'rgb_sha256':rgb_sha256(folder/'image.png'),'seconds':d['render_total_seconds'],
                'selection_seconds':d['backend_selection_seconds'],'range_pixels':m['fp32_range_gate_exceeded']}
    result={'qualification':False,'point_exact_repeats':3,'identities':identities,'resources':resources,'frames':[],'status':'OPEN'}
    def save(): (out/'report.json').write_text(json.dumps(result,indent=2)+'\n')
    for i in range(3):
        existing=a.first_frame.resolve() if i==0 and a.first_frame else None
        result['frames'].append(frame(existing or out/('repeat-'+str(i)),reuse=existing is not None));save()
        print('FRAME',i+1,'PASS',flush=True)
    result['direct_cpu']=frame(out/'direct-cpu','cpu')
    assert len({x['rgb_sha256'] for x in result['frames']+[result['direct_cpu']]})==1
    assert len({x['range_pixels'] for x in result['frames']})==1
    assert all(sha256(Path(x))==h for x,h in identities.items())
    assert resources=={str(x.relative_to(runtime)):sha256(x) for x in runtime.rglob('*') if x.is_file()}
    result['status']='PASS_FIXED_NORMAL_REGRESSION';save();print(result['status'])


if __name__=='__main__':main()
