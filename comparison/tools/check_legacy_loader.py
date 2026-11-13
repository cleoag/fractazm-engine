"""Capture actual Pascal LoadParameter output before headless render overrides.

Raw differences are retained; this diagnostic does not turn unclassified bytes
into a parity pass. zlib is tested only as a C++ extension of the raw format.
"""
import argparse
from collections import Counter
import hashlib
import json
import os
from pathlib import Path
import struct
import subprocess
import zlib

from external_validation import ROOT, decode


def sha(b): return hashlib.sha256(b).hexdigest()


def old_light(version=0):
    b=bytearray(272)
    struct.pack_into('<11i',b,0,25,20,50,7,120,40,50,50,30,20,20)
    struct.pack_into('<III9I',b,44,version<<20,0x321234,0x204080,
                     0x7090B0,0xE8C040,0x183878,0xCC7744,0xA0C0E0,0x447722,
                     0x3488CC,0x906030,0x8899AA)
    for i in range(4):struct.pack_into('<BBIii',b,92+14*i,int(i!=0),3,0xD0B090,1000*i,-2000*i)
    for i in range(10):struct.pack_into('<HII',b,148+10*i,i*3500,0x1188CC+i*123,0xDDAA88-i*123)
    for i in range(4):struct.pack_into('<HI',b,248+6*i,i*10922,0x334455+i*31)
    return b


def old_addon():
    b=bytearray(1016)
    struct.pack_into('<ii6i6i',b,0,1,0,1,0,0,0,0,0,0,-1,-1,-1,-1,-1)
    for i in range(6):struct.pack_into('<16d',b,248+128*i,2.,1.,*[0.]*14)
    return b


def legacy(base,mid,fn=0,lightver=0,reverse=False,cache=False,oldha=False):
    if mid>=20:
        header=bytearray(base[:840]);struct.pack_into('<i',header,0,mid)
    else:
        header=bytearray(704)
        header[:432]=base[:432]
        struct.pack_into('<i',header,0,mid)
        struct.pack_into('<i',header,16,fn)
        struct.pack_into('<3d',header,60,0.1,-0.2,0.3)
        struct.pack_into('<d',header,100,8.)
        struct.pack_into('<2d',header,116,0.,0.)
        struct.pack_into('<3f',header,164,.125,-.25,.375)
        struct.pack_into('<2f',header,181,1.,2.)
        struct.pack_into('<2d',header,215,-1.,.5)
        header[231:318]=bytes(87)
        struct.pack_into('<3i',header,234,1,0,0)
        struct.pack_into('<3d',header,246,8.,8.,8.)
        struct.pack_into('<3d',header,270,-1.,-1.,-1.)
        struct.pack_into('<3d',header,294,.5,.5,.5)
        header[327:344]=bytes(17)
        header[370:432]=bytes(62)
        if mid>=17:header[246:318]=base[246:318]
        header[432:704]=old_light(lightver)
        if reverse:struct.pack_into('<2d',header,20,4.,-4.)
        if mid<7:header=header[:108]+header[432:524]+header[108:180]
        if mid<4:header=header[:200]
        elif mid==7:header=header[:580]
    if mid<16:return header
    addon=old_addon() if oldha else bytearray(base[840:1976])
    if mid<20 and not oldha:
        # Old integer-power formula ID 0 remains ID0 during v20 migration.
        struct.pack_into('<d',addon,8+60,2.)
    if cache:
        w,h=struct.unpack_from('<2i',header,4)
        pixels=bytes(w*h*(12 if mid<=17 else 16 if mid==18 else 18))
    else:pixels=b''
    return header+pixels+addon


def generate(out):
    base=decode(ROOT/'comparison/scenes_phase2/phase2_intpow2_small.m3p')
    struct.pack_into('<2i',base,4,8,6)
    cases=[]
    def add(name,b):
        path=out/'scenes'/(name+'.m3p');path.parent.mkdir(exist_ok=True,parents=True);path.write_bytes(b)
        cases.append(dict(id=name,path=str(path),sha256=sha(b)))
    for mid in range(45):add(f'version_{mid:02d}',legacy(base,mid))
    for fn in range(1,16):
        # Historical external IDs need a real named file; cover built-in branches.
        if fn in (14,15):continue
        add(f'v06_formula_{fn:02d}',legacy(base,6,fn=fn))
    for mid in (0,4):add(f'v{mid:02d}_reverse',legacy(base,mid,reverse=True))
    for ver in (1,2):add(f'v19_light{ver}',legacy(base,19,lightver=ver))
    # The old hybrid IDs and parameter storage precede the camera-matrix layout.
    for mid in (8,9,15):
        for group in range(3):
            b=legacy(base,mid,fn=9)
            struct.pack_into('<3i',b,234,1,2,3)
            b[231:234]=bytes((6,7,7) if group==2 and mid<10 else range(group*3,group*3+3))
            struct.pack_into('<3d',b,246,2.,3.,4.)
            b[328:344]=b'_SinePow2'+bytes(8)
            add(f'v{mid:02d}_hybrid_{group}',b)
    for bits in (0,1,3):
        add(f'v05_formula10_opt{bits}',legacy(base,5,fn=10|(bits<<18)))
    for name in ('_SinePow2','Riemann','Fuzzy'):
        b=legacy(base,15,fn=11)
        b[328:344]=name.encode()+bytes(16-len(name))
        add(f'v15_named_{name}',b)
    for ver in range(8):
        b=legacy(base,44)
        tb=struct.unpack_from('<I',b,476)[0]
        struct.pack_into('<I',b,476,(tb&~(7<<20))|(ver<<20))
        add(f'light_version{ver}',b)
    for ver in range(9):
        b=legacy(base,44);b[432+84]=ver
        add(f'light_extended{ver}',b)
    b=legacy(base,19,lightver=0)
    struct.pack_into('<i',b,432+16,1);struct.pack_into('<i',b,432+28,1)
    struct.pack_into('<I',b,432+56,0x224400);struct.pack_into('<I',b,432+64,0x224401)
    add('light_half_tie',b)
    for fine in (0,1):
        b=legacy(base,19,lightver=2)
        struct.pack_into('<i',b,432+32,-50);struct.pack_into('<i',b,432+36,-70)
        struct.pack_into('<I',b,432+44,(2<<20)|(fine<<16))
        add(f'light_negative_position_fine{fine}',b)
    for mid in (16,17,18,19,20,34,35,44):
        add(f'v{mid}_pixel_cache',legacy(base,mid,cache=True))
        add(f'v{mid}_old_addon',legacy(base,mid,oldha=True))
    (out/'manifest.json').write_text(json.dumps(cases,indent=2))
    return cases


def main():
    ap=argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--output',type=Path,required=True)
    ap.add_argument('--cpp',type=Path,required=True)
    ap.add_argument('--pascal',type=Path,required=True)
    ap.add_argument('--limit',type=int)
    ap.add_argument('--reuse-reference',action='store_true')
    args=ap.parse_args();out=args.output.resolve();out.mkdir(parents=True,exist_ok=True)
    previous=json.loads((out/'summary.json').read_text()) if (out/'summary.json').exists() else {}
    previous_rows={r['id']:r for r in previous.get('results',[])}
    cases=generate(out);rows=[]
    report=dict(cpp_sha256=sha(args.cpp.read_bytes()),pascal_sha256=sha(args.pascal.read_bytes()),
                scope='load-only diagnostic; raw differences unqualified',results=rows,finished=False)
    for c in cases[:args.limit]:
        row=dict(c);name=c['id'];p=out/(name+'.pascal.bin');q=out/(name+'.cpp.bin')
        old=previous_rows.get(name,{})
        reuse=(args.reuse_reference and p.exists() and old.get('sha256')==c['sha256']
               and previous.get('pascal_sha256')==report['pascal_sha256']
               and old.get('pascal_sha256')==sha(p.read_bytes()))
        if not reuse:
            env=dict(os.environ);env['MB3D_HEADLESS_LOADER_DUMP']=str(p)
            if p.exists():p.unlink()
            try:
                r=subprocess.run([str(args.pascal.resolve()),'--render',c['path'],'--output',str(out/'unused.png'),'--threads','1'],cwd=args.pascal.resolve().parent,env=env,capture_output=True,timeout=12)
                (out/(name+'.pascal.log')).write_bytes(r.stdout+b'\n'+r.stderr)
                row['pascal_exit']=r.returncode
            except subprocess.TimeoutExpired:row['pascal_exit']='TIMEOUT'
        r=subprocess.run([str(args.cpp.resolve()),c['path'],str(q)],cwd=ROOT,capture_output=True,timeout=10)
        row['cpp_exit']=r.returncode
        if p.exists() and q.exists():
            a,b=p.read_bytes(),q.read_bytes();row.update(pascal_sha256=sha(a),cpp_sha256=sha(b),pascal_size=len(a),cpp_size=len(b),diff_offsets=[i for i,(x,y) in enumerate(zip(a,b)) if x!=y])
            row['raw_status']='EXACT' if a==b else 'DIFFERENT'
            z=out/(name+'.zlib');z.write_bytes(zlib.compress(Path(c['path']).read_bytes()))
            t=out/(name+'.zlib.bin');rz=subprocess.run([str(args.cpp.resolve()),str(z),str(t)],cwd=ROOT,capture_output=True,timeout=10)
            row['zlib_extension_status']='PASS' if rz.returncode==0 and t.read_bytes()==b else 'FAIL'
        else:row['raw_status']='ERROR'
        rows.append(row);print(name,row['raw_status'],len(row.get('diff_offsets',[])),flush=True)
        report['counts']=dict(Counter(x['raw_status'] for x in rows));(out/'summary.json').write_text(json.dumps(report,indent=2))
    report['finished']=len(rows)==len(cases);(out/'summary.json').write_text(json.dumps(report,indent=2))


if __name__=='__main__':main()
