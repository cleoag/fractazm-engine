#!/usr/bin/env python3
"""Bounded A/B of logical-lane dispatch; every frame retains raw RGB/profile.

Fresh processes with warm OS/driver caches are NOT same-process warm renders.
This diagnostic never awards the full parallel-pipeline contract.
"""
import argparse
import statistics
from pathlib import Path
from run_m3parameter_gpu import read, write, require, verify_inputs, render_one, invoke, validate_profile, rgb

def worker_frame(package, scene, folder, size, threads):
    if threads == 6:
        return render_one(package, scene, 'metal', folder, size, 300)
    argv=[package/'build/mb3d_render',package/scene['frozen'],'--backend','metal',
          '--precision','native','--width',size[0],'--height',size[1],'--threads',threads,
          '--deterministic','--metal-library',package/'build/metal_native_scene.metallib',
          '--output',folder/'image.png','--profile-json',folder/'profile.json']
    event=invoke(argv,folder,package/'runtime',300)
    require(event['exit_code']==0 and not event['guard'],'Held-out frame failed')
    profile=read(folder/'profile.json')
    work=validate_profile(profile,'metal',size); im,_=rgb(folder/'image.png',size)
    s=scene['settings']
    for stage,on in {'ao':s['ao_kind']!='disabled','shadows':s['hard_shadows_enabled'],
                     'reflections':s['reflection_enabled'],'dof':s['dof_enabled'],
                     'normals':s['zbuffer_normals'],'volume':bool(s['volume_light'])}.items():
        require(profile['stages'][stage]['state']==('completed' if on else 'not_run'),'Effect changed')
    return {'status':'PREVIEW_COMPLETE','event':event,'profile':str(folder/'profile.json'),
            'image':im,'work':work}

def run(baseline, candidate, output, ids, size, threads=6):
    require(not output.exists(), 'Fresh output required')
    output.mkdir(parents=True)
    contracts = [verify_inputs(p)[0] for p in (baseline, candidate)]
    scenes = [{s['id']: s for s in c['scenes']} for c in contracts]
    rows = []
    for sid in ids:
        require(scenes[0][sid] == scenes[1][sid], 'Scene inputs changed')
        scene = scenes[0][sid]
        dimensions = size or scene['settings']['size']
        row = {'id': sid, 'scene': scene['path'], 'size': dimensions,
               'original_size': dimensions == scene['settings']['size'],
               'threads': threads, 'baseline': [], 'candidate': []}
        for repeat in range(3):
            for name, package in ([('baseline', baseline), ('candidate', candidate)]
                                  if repeat % 2 == 0 else [('candidate', candidate), ('baseline', baseline)]):
                print('START', sid, name, repeat, dimensions, flush=True)
                frame = worker_frame(package, scene, output/sid/name/str(repeat), dimensions, threads)
                row[name].append(frame)
                write(output/'partial.json', rows + [row])
                require(frame['status'] == 'PREVIEW_COMPLETE', 'Frame failed: ' + str(frame))
                reference = row['baseline'][0]
                require(frame['image'] == reference['image'] and frame['work'] == reference['work'],
                        'Exact old/new RGB or logical work mismatch')
                print('PASS', name, read(frame['profile'])['render_total_seconds'], flush=True)
        for name in ('baseline', 'candidate'):
            row[name+'_seconds'] = [read(f['profile'])['render_total_seconds'] for f in row[name]]
        row['speedup'] = statistics.median(row['baseline_seconds'])/statistics.median(row['candidate_seconds'])
        row['status'] = 'PASS_EXACT_RGB_WORK'
        rows.append(row)
        write(output/'result.json', {'scope': 'Fresh-process diagnostic; not full contract qualification', 'rows': rows})
        print('SPEEDUP', sid, row['speedup'], flush=True)
    verify_inputs(baseline); verify_inputs(candidate)

if __name__ == '__main__':
    p = argparse.ArgumentParser(description=__doc__)
    for name in ('baseline','candidate','output'): p.add_argument('--'+name, type=Path, required=True)
    p.add_argument('--scene', action='append', required=True)
    p.add_argument('--size', nargs=2, type=int)
    p.add_argument('--threads', type=int, choices=(1,2,6), default=6)
    a = p.parse_args()
    run(a.baseline.resolve(), a.candidate.resolve(), a.output.resolve(), a.scene, a.size, a.threads)
