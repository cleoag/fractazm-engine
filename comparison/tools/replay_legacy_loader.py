"""Replay immutable full Pascal loader dumps on a selected C++ probe.

Raw record differences are diagnostic, never automatically numerical PASS.
The independent references and original scenes are verified by their stored
hashes before use. The zlib check is a C++ extension, not Pascal compatibility.
"""
import argparse
from collections import Counter
from concurrent.futures import ThreadPoolExecutor
import hashlib
import json
from pathlib import Path
import subprocess
import zlib

ROOT = Path(__file__).resolve().parents[2]

def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()

def main():
    ap=argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--reference',type=Path,required=True)
    ap.add_argument('--cpp',type=Path,required=True)
    ap.add_argument('--output',type=Path,required=True)
    args=ap.parse_args()
    reference=args.reference.resolve();cpp=args.cpp.resolve();out=args.output.resolve()
    previous=json.loads(reference.read_bytes())
    if not previous.get('finished'):
        raise ValueError('Incomplete original reference collection')
    for row in previous['results']:
        scene=Path(row['path']);original=reference.parent/(row['id']+'.pascal.bin')
        if sha(scene)!=row['sha256'] or sha(original)!=row['pascal_sha256']:
            raise ValueError('Original input/reference changed: '+row['id'])
    out.mkdir(parents=True,exist_ok=False)
    def run(row):
        name=row['id'];scene=Path(row['path']);original=reference.parent/(name+'.pascal.bin')
        target=out/(name+'.cpp.bin')
        proc=subprocess.run([str(cpp),str(scene),str(target)],cwd=ROOT,capture_output=True,timeout=10)
        (out/(name+'.log')).write_bytes(proc.stdout+b'\n'+proc.stderr)
        result=dict(id=name,scene=str(scene),scene_sha256=row['sha256'],reference=str(original),
            reference_sha256=row['pascal_sha256'],cpp_exit=proc.returncode,cpp_dump=str(target))
        if proc.returncode or not target.exists():
            return dict(result,raw_status='ERROR')
        a,b=original.read_bytes(),target.read_bytes()
        zipped=out/(name+'.zlib');zipped.write_bytes(zlib.compress(scene.read_bytes()))
        ztarget=out/(name+'.zlib.bin')
        zp=subprocess.run([str(cpp),str(zipped),str(ztarget)],cwd=ROOT,capture_output=True,timeout=10)
        return dict(result,raw_status='EXACT' if a==b else 'DIFFERENT',
            reference_size=len(a),cpp_size=len(b),cpp_dump_sha256=sha(target),
            diff_offsets=[i for i,(x,y) in enumerate(zip(a,b)) if x!=y],
            zlib_extension_status='PASS' if zp.returncode==0 and ztarget.read_bytes()==b else 'FAIL')
    with ThreadPoolExecutor(max_workers=4) as pool:
        rows=list(pool.map(run,previous['results']))
    report=dict(scope=__doc__,cpp=str(cpp),cpp_sha256=sha(cpp),
        original_pascal_sha256=previous['pascal_sha256'],reference_report=str(reference),
        reference_report_sha256=sha(reference),tool_sha256=sha(Path(__file__)),
        results=rows,counts=dict(Counter(r['raw_status'] for r in rows)),finished=True)
    (out/'summary.json').write_text(json.dumps(report,indent=2)+'\n')
    print(json.dumps(report['counts']))
    return int(any(r['raw_status']=='ERROR' or r.get('zlib_extension_status')!='PASS' for r in rows))

if __name__=='__main__':raise SystemExit(main())
