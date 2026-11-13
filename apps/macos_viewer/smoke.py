#!/usr/bin/env python3
"""Bounded prototype checks. No corpus qualification or MC/JIT discovery."""
import argparse, hashlib, json, pathlib, subprocess, time
p=argparse.ArgumentParser();p.add_argument('--app',type=pathlib.Path,required=True);p.add_argument('--output',type=pathlib.Path,required=True)
a=p.parse_args();a.output.mkdir(parents=True,exist_ok=True)
exe=a.app/'Contents/MacOS/MandelbulbViewer';res=a.app/'Contents/Resources'
results=[]
for name,scene,backend in [('metal',res/'demo.m3p','metal'),('orbit',a.output/'orbit.m3p','metal'),('fly',a.output/'fly.m3p','metal'),('pan_gpu',a.output/'pan.m3p','metal'),('pan_cpu',a.output/'pan.m3p','cpu'),('cpu',res/'demo.m3p','cpu'),('invalid',a.output/'missing.m3p','metal')]:
    image=a.output/f'{name}.png';meta=a.output/f'{name}.json'
    cmd=[str(exe),'--worker',str(scene),str(res/'runtime'),backend,'64',str(image),str(meta)]
    with (a.output/f'{name}.log').open('w') as log:
        completed=subprocess.run(cmd,stdout=log,stderr=subprocess.STDOUT,timeout=60)
    info=json.loads(meta.read_text())
    assert (completed.returncode==0)==(name!='invalid'),info
    if name!='invalid':
        assert info['status']=='completed' and info['width']==64 and info['height']==48,info
        assert info['backend']==backend,info
        info['png_sha256']=hashlib.sha256(image.read_bytes()).hexdigest()
    else: assert not image.exists()
    results.append(dict(name=name,command=cmd,**info))
assert len({r['png_sha256'] for r in results[:3]})==3
# Real process cancellation exercises the prototype's isolation boundary, not GPU cooperative control.
image=a.output/'cancel.png';meta=a.output/'cancel.json'
cmd=[str(exe),'--worker',str(res/'demo.m3p'),str(res/'runtime'),'cpu','1600',str(image),str(meta)]
with (a.output/'cancel.log').open('w') as log:
    job=subprocess.Popen(cmd,stdout=log,stderr=subprocess.STDOUT)
    time.sleep(.05)
    assert job.poll() is None, 'Cancellation fixture completed before cancellation'
    start=time.monotonic();job.terminate()
    try: job.wait(timeout=.25)
    except subprocess.TimeoutExpired: job.kill();job.wait(timeout=1)
    elapsed=time.monotonic()-start
assert job.returncode!=0 and not image.exists()
results.append(dict(name='process_cancel',command=cmd,status='cancelled',seconds=elapsed,returncode=job.returncode))
(a.output/'smoke.json').write_text(json.dumps(results,indent=2)+'\n')
print('8 worker cases passed (including GPU pan regression and process cancellation)')
