#!/usr/bin/env python3
"""Capture authored-RNG numeric diagnostics without altering the scene corpus."""
import argparse
import json
import struct
import sys
from pathlib import Path


def main():
    p=argparse.ArgumentParser(description=__doc__)
    for name in ('repository','package','contract','output'):
        p.add_argument('--'+name,type=Path,required=True)
    p.add_argument('--ranks',required=True)
    p.add_argument('--cpu-only',action='store_true')
    p.add_argument('--resume',action='store_true')
    a=p.parse_args();root=a.repository.resolve();package=a.package.resolve();output=a.output.resolve()
    sys.path.insert(0,str(root/'comparison/tools'))
    from qualify_native_metal_all import read_json,require,sha
    from qualify_gpu_parallel_pipeline import scope
    from gpu_pipeline_binding import core_context,for_scene
    from run_m3parameter_gpu import invoke,validate_profile
    from native_gpu_coverage import image_difference
    from prepare_gpu_parallel_bundle import write
    c,formulas,scenes=scope(a.contract.resolve())
    binding=read_json(package/'evidence.json')['binding']
    context=core_context(root,binding,c,formulas,scenes,root)
    ranks=set(map(int,a.ranks.split(',')))
    for entry in read_json(root/'comparison/scene-queue.json')['rows']:
        if entry['rank'] not in ranks:continue
        sid=entry['id'];scene=scenes[sid];ctx=for_scene(context,sid)
        source=ctx['inputs'][sid];size=scene['settings']['size'];dest=output/sid
        require(sha(source)==scene['sha256'],'Changed original scene')
        original=source.read_bytes();data=bytearray(original);changes=[]
        for offset in (36,44,52):
            before=struct.unpack_from('<d',data,offset)[0];after=before+2**-24*max(abs(before),1)
            require(before!=after and abs(after)<1e300,'Invalid camera shift')
            struct.pack_into('<d',data,offset,after);changes.append({'offset':offset,'before':before,'after':after})
        require(data[:36]==original[:36] and data[60:]==original[60:],'Changed non-camera bytes')
        dest.mkdir(parents=True,exist_ok=True);shifted=dest/'shift24.m3p'
        if shifted.exists():require(shifted.read_bytes()==data,'Changed saved shifted scene')
        else:shifted.write_bytes(data)
        result={'rank':entry['rank'],'scene':scene['path'],'size':size,'threads':ctx['threads'],
                'qualification':False,'rng':'authored','camera_changes':changes,'jobs':{}}
        jobs=[('cpu-dump','cpu',source,True),('cpu-shift24','cpu',shifted,False)]
        if not a.cpu_only:jobs.append(('gpu-dump','metal',source,True))
        try:
            for label,backend,inp,dump in jobs:
                out=dest/label
                argv=[ctx['renderer']['path'],str(inp),'--backend',backend,'--precision','native',
                      '--width',str(size[0]),'--height',str(size[1]),'--threads',str(ctx['threads']),
                      '--output',str(out/'image.png'),'--profile-json',str(out/'profile.json')]
                if backend=='metal':argv+=['--metal-library',str(ctx['library_path'])]
                if dump:argv+=['--silight5-dump',str(out/'surface.bin')]
                if (out/'event.json').exists():
                    require(a.resume,'Existing diagnostic needs --resume');event=read_json(out/'event.json')
                    require(event['argv']==argv,'Changed diagnostic arguments')
                    require(event['executable_sha256_before']==event['executable_sha256_after']==ctx['renderer']['sha256'],'Changed diagnostic host')
                    require(all(sha(root/r['path'])==r['sha256'] for r in event['inputs']+event['outputs']),'Changed diagnostic artifact')
                else:
                    require(not out.exists(),'Inspect incomplete diagnostic')
                    print('START NUMERIC',entry['rank'],label,flush=True)
                    event=invoke(argv,out,ctx['runtime'],timeout=300,evidence_root=root)
                require(event['exit_code']==0 and event['guard'] is None and event['seconds']<=180,'Diagnostic failed or exceeded threshold')
                profile=read_json(out/'profile.json');validate_profile(profile,backend,size)
                row={'seconds':event['seconds'],'event':str((out/'event.json').relative_to(root))}
                reference=package/'scene-runs'/sid/('reference-cpu' if backend=='cpu' else 'repeat-0')/'image.png'
                if backend=='cpu' and not reference.is_file():
                    case=read_json(package/'scene-runs'/sid/'case.json')
                    reference=root/case['cpu_reference']['image']['path']
                row['difference']=image_difference(reference,out/'image.png')
                if label.endswith('dump'):require(row['difference']['pixels']==0,'Dump capture changed the original image')
                result['jobs'][label]=row;write(dest/'capture.json',result)
                print('DONE NUMERIC',entry['rank'],label,round(event['seconds'],3),row['difference'],flush=True)
            result['status']='CPU_DIAGNOSTICS_COMPLETE' if a.cpu_only else 'CAPTURE_COMPLETE'
        except (ValueError,KeyError,TypeError,OSError) as error:
            result['status']='OPEN';result['error']=str(error)
            print('OPEN NUMERIC',entry['rank'],str(error),flush=True)
        write(dest/'capture.json',result)


if __name__=='__main__':main()
