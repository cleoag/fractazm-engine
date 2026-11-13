#!/usr/bin/env python3
"""Real full-scene RNG benchmark/CLI bridge; does not award scene qualification.

Two unchanged ordinary scenes exercise primary jitter and DE-AO jitter. The
legacy parity mode remains a separate control. Every frame has a 180 s guard;
no failed or slow benchmark is followed by additional repeats.
"""
import argparse
import json
from pathlib import Path
import struct
from native_gpu_coverage import sha256, rgb_sha256, image_difference
from run_m3parameter_gpu import invoke, validate_profile
import gpu_pipeline_frames as gate

ROOT=Path(__file__).resolve().parents[2]


def main():
    p=argparse.ArgumentParser(description=__doc__)
    for key in ('build','resources','output'):p.add_argument('--'+key,type=Path,required=True)
    p.add_argument('--resume',action='store_true')
    a=p.parse_args();b=a.build.resolve();resources=a.resources.resolve();runtime=resources/'runtime';out=a.output.resolve()
    assert a.resume or not out.exists();out.mkdir(parents=True,exist_ok=a.resume)
    def ref(path):return {'path':str(path.relative_to(ROOT)),'sha256':sha256(path)}
    def binary(path):return {'path':str(path),'sha256':sha256(path)}
    def run(argv,folder,timeout):
        if a.resume and (folder/'event.json').exists():
            event=json.loads((folder/'event.json').read_text())
        else:
            assert not folder.exists(), 'Partial job needs inspection: '+str(folder)
            event=invoke(argv,folder,runtime,timeout=timeout,evidence_root=ROOT)
        assert event['argv']==list(map(str,argv)) and Path(event['cwd']).resolve()==runtime
        assert event['executable_sha256_before']==event['executable_sha256_after']==sha256(Path(argv[0]))
        assert event['owned_process_reaped'] and event['exit_code']==0 and not event['guard']
        for item in event['inputs']+event['outputs']:
            path=(ROOT/item['path']).resolve();assert path.is_relative_to(ROOT) and sha256(path)==item['sha256']
        return event

    identities={str(b/n):sha256(b/n) for n in ('mb3d_render','mb3d_backend_render_benchmark','metal_native_scene.metallib')}
    runtime_files={str(x.relative_to(runtime)):sha256(x) for x in runtime.rglob('*') if x.is_file()}
    inventory={x['path']:x for x in json.loads((ROOT/'comparison/results/m3parameter-gpu-priority-20261024/inventory.json').read_text())['scenes']}
    manifest=json.loads((resources/'resources.json').read_text())
    scenes=[x for x in json.loads((ROOT/'comparison/scene-queue.json').read_text())['rows'] if x['rank'] in (26,46)]
    assert len(scenes)==2 and not {x['scene'] for x in scenes}&set(manifest['blocked_scenes'])
    report={'status':'OPEN','qualification':False,'identities':identities,'resources':runtime_files,'cases':[]}
    def save():(out/'report.json').write_text(json.dumps(report,indent=2)+'\n')
    for row in scenes:
        source=ROOT/row['scene'];scene=dict(inventory[row['scene']],id=row['id']);size=scene['settings']['size']
        assert sha256(source)==scene['sha256']
        raw=source.read_bytes();assert struct.unpack_from('<i',raw)[0]==44
        flags={'primary_first_step_jitter':bool(struct.unpack_from('<H',raw,16)[0]&1),'ao_first_step_jitter':bool(raw[149]&128)}
        assert flags['primary_first_step_jitter'] and (row['rank']!=46 or flags['ao_first_step_jitter'])
        ctx={'renderer':binary(b/'mb3d_render'),'benchmarks':{'candidate':binary(b/'mb3d_backend_render_benchmark')},
             'libraries':{'candidate':b/'metal_native_scene.metallib'},'library_path':b/'metal_native_scene.metallib',
             'runtime':runtime,'threads':6,'inputs':{scene['id']:source}}
        for backend in ('metal','cpu'):
            previous=None
            for mode in ('scene','parity'):
                folder=out/str(row['rank'])/backend/mode;frames=folder/'frames'
                argv=[b/'mb3d_backend_render_benchmark','--scene',source,'--output',frames,'--resource-root',runtime,
                      '--backend',backend,'--precision','native','--width',str(size[0]),'--height',str(size[1]),
                      '--threads','6','--warmups','0','--repeats','3','--rng-mode',mode,'--frame-timeout-seconds','180']
                if backend=='metal':argv+=['--metal-library',b/'metal_native_scene.metallib']
                event=run(argv,folder,740)
                assert event['exit_code']==0 and not event['guard']
                d=json.loads((frames/'report.json').read_text());samples=[d['cold'],*d['samples']]
                assert d['status']=='COMPLETE' and d['execution']['rng_mode']==mode and d['execution']['deterministic']==(mode=='parity')
                assert d['effective_rng']==(flags if mode=='scene' else dict.fromkeys(flags,False))
                assert d['loaded_features']['iOptions']==struct.unpack_from('<H',raw,16)[0] and d['loaded_features']['bCalcAmbShadowAutomatic']==raw[149]
                assert len(samples)==4 and all(s['exact_rgb_matches_cold'] and s['deadline']['completed'] and
                    s['deadline']['limit_seconds']==180 and s['deadline']['seconds']<=180 and
                    s['backend']['effective']==backend and not s['backend']['fallback'] for s in samples)
                hashes=[rgb_sha256(frames/s['image']) for s in samples];assert len(set(hashes))==1
                cli=folder/'cli';cmd=[b/'mb3d_render',source,'--backend',backend,'--precision','native','--threads','6',
                    '--width',str(size[0]),'--height',str(size[1]),'--output',cli/'image.png','--profile-json',cli/'profile.json']
                if backend=='metal':cmd+=['--metal-library',b/'metal_native_scene.metallib']
                if mode=='parity':cmd+=['--deterministic']
                e=run(cmd,cli,180);assert e['exit_code']==0 and not e['guard']
                profile=json.loads((cli/'profile.json').read_text());validate_profile(profile,backend,size)
                assert rgb_sha256(cli/'image.png')==hashes[0], 'Benchmark differs from main CLI'
                if backend=='metal' and mode=='scene':
                    f=gate.frame(ROOT,{'event':ref(cli/'event.json'),'profile':ref(cli/'profile.json'),'image':ref(cli/'image.png')},scene,ctx)
                    bench=gate.benchmark(ROOT,{'event':ref(folder/'event.json'),'report':ref(frames/'report.json')},scene,ctx,'candidate')
                    assert bench[2]==(f['image'],f['work']), 'Real benchmark/CLI logical work mismatch'
                result={'rank':row['rank'],'scene':row['scene'],'backend':backend,'mode':mode,'rgb_sha256':hashes[0],
                        'exact_repeats':4,'main_cli_exact':True,'effective_rng':d['effective_rng'],
                        'frame_seconds':[s['deadline']['seconds'] for s in samples],
                        'archive_states':[s['backend']['execution'].get('pipeline_archive') for s in samples],
                        'event':ref(folder/'event.json'),'cli_event':ref(cli/'event.json')}
                if mode=='scene':previous=frames/samples[0]['image']
                else:
                    result['scene_mode_difference']=image_difference(previous,frames/samples[0]['image'])
                    assert result['scene_mode_difference']['pixels']>0,'Jitter control had no visible effect'
                report['cases'].append(result);save();print(row['rank'],backend,mode,'PASS',flush=True)
    assert all(sha256(Path(x))==h for x,h in identities.items())
    assert runtime_files=={str(x.relative_to(runtime)):sha256(x) for x in runtime.rglob('*') if x.is_file()}
    report['status']='PASS_SCENE_RNG_BENCHMARK_BRIDGE';save();print(report['status'])


if __name__=='__main__':main()
