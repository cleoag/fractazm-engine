"""Replay actual original GUI file-selector captures on a frozen loader.

Selector equality and original rejection are separate outcomes. This does not
certify a direct-header numerical branch or turn a failed original into PASS.
"""
import argparse
from collections import Counter
import hashlib
import json
from pathlib import Path
import subprocess

ROOT=Path(__file__).resolve().parents[2]
def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()

def main():
    ap=argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--hybrid-reference',type=Path,required=True)
    ap.add_argument('--reserved-reference',type=Path,required=True)
    ap.add_argument('--cpp',type=Path,required=True)
    ap.add_argument('--output',type=Path,required=True)
    a=ap.parse_args();out=a.output.resolve();out.mkdir(parents=True,exist_ok=False);rows=[];identities=[]
    for path,hybrid in [(a.hybrid_reference,True),(a.reserved_reference,False)]:
        report=json.loads(path.read_bytes())
        identities.append(dict(path=str(path.resolve()),sha256=sha(path),pascal_sha256=report['pascal_sha256']))
        for r in report['results']:
            if hybrid and not r['after_makeheader']:continue
            name=r['id'];scene=Path(r['command'][r['command'].index('--render')+1])
            if sha(scene)!=r['scene_sha256']:raise ValueError('Changed original input: '+name)
            target=out/(name+'.cpp.bin')
            proc=subprocess.run([str(a.cpp.resolve()),str(scene),str(target)],cwd=ROOT,capture_output=True,timeout=10)
            log=out/(name+'.cpp.log');log.write_bytes(proc.stdout+b'\n'+proc.stderr)
            row=dict(id=name,scene=str(scene),scene_sha256=sha(scene),original_exit=r['exit_code'],cpp_exit=proc.returncode,
                diagnostic_sha256=sha(log))
            if r['exit_code']!=0:
                row.update(status='MATCHED_ORIGINAL_REJECTION' if proc.returncode==1 and not target.exists()
                    and b'Invalid file camera optic' in proc.stderr else 'FAIL',numerical_parity_pass=False)
            else:
                ref=path.parent/(name+'.bin')
                if sha(ref)!=r['dump_sha256']:raise ValueError('Changed original dump: '+name)
                expected={'841':r['effective_mode'],'845':r['hyb1']} if hybrid else r['effective']
                if proc.returncode or not target.exists():
                    row.update(status='FAIL',reason='C++ unexpectedly rejected')
                else:
                    data=target.read_bytes();original=ref.read_bytes()
                    actual={key:data[int(key)] for key in expected}
                    if hybrid:
                        expected['846_u16']=r['hyb2'];actual['846_u16']=int.from_bytes(data[846:848],'little')
                    row.update(status='SELECTOR_MATCH' if actual==expected else 'FAIL',expected=expected,actual=actual,
                        reference_sha256=sha(ref),cpp_dump_sha256=sha(target),raw_record_equal=data==original,
                        raw_difference_offsets=[i for i,(x,y) in enumerate(zip(original,data)) if x!=y])
            rows.append(row)
    report=dict(scope=__doc__,cpp=str(a.cpp.resolve()),cpp_sha256=sha(a.cpp),tool_sha256=sha(Path(__file__)),
        independent_references=identities,counts=dict(Counter(r['status'] for r in rows)),results=rows,finished=True)
    (out/'summary.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report['counts']))
    return int(any(r['status']=='FAIL' for r in rows))

if __name__=='__main__':raise SystemExit(main())
