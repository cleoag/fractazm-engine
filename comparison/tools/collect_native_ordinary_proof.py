#!/usr/bin/env python3
"""Capture actual ordinary CTest inventory, immutable inputs and JUnit.

Run in an exclusive GPU slot on a frozen build. This collector does not modify
an acceptance manifest or grant complete build/pipeline qualification.
"""
import argparse,json,shutil
from pathlib import Path
from gpu_pipeline_proofs import ORDINARY,cpu_ordinary,ordinary_input_files,sha
from run_m3parameter_gpu import invoke,require,write


def collect(root,build,out,ctest):
    root=root.resolve();build=build.resolve();out=out.resolve();ctest=ctest.resolve()
    require(build.is_dir() and build.is_relative_to(root),'Build must be frozen inside the repository archive')
    require(out.is_relative_to(root) and not out.exists(),'A new repository output directory is required')
    require(ctest.is_file(),'CTest executable missing')
    out.mkdir(parents=True)
    # Keep the external runner as an immutable artifact, not a project build product.
    tool=out/'tools'/'ctest';tool.parent.mkdir();shutil.copy2(ctest,tool)
    require(sha(tool)==sha(ctest),'CTest copy changed')
    def ref(p):return {'path':str(p.relative_to(root)),'sha256':sha(p)}
    def run(argv,folder,timeout,inputs=(),exclusive_gpu=False):
        event=invoke(argv,folder,root,timeout,evidence_root=root,input_files=inputs,exclusive_gpu=exclusive_gpu)
        require(event['exit_code']==0 and event['guard'] is None,'CTest failed: '+str(folder))
        return ref(folder/'event.json')
    listing=out/'listing'
    le=run([tool,'--show-only=json-v1','-L','ordinary','--test-dir',build],listing,60)
    data=json.loads((listing/'process.log').read_text());commands={t['name']:t['command'] for t in data['tests']}
    require(len(data['tests'])==len(ORDINARY) and set(commands)==ORDINARY,'Current ordinary inventory differs')
    inputs=ordinary_input_files(commands,root,build)
    require(any(p.is_relative_to(build) for p in inputs),'No project test binaries observed')
    suite=out/'suite';junit=suite/'ordinary.xml'
    ev=run([tool,'--test-dir',build,'-L','ordinary','--parallel','1','--timeout','180',
            '--output-on-failure','--output-junit',junit],suite,1200,inputs,exclusive_gpu=True)
    proof={'listing_event':le,'listing':ref(listing/'process.log'),'event':ev,'junit':ref(junit)}
    ctx={'tools':{'cpu_ordinary':{'path':str(tool),'sha256':sha(tool)}},'ordinary_commands':commands,'build_dir':build}
    cpu_ordinary(root,proof,ctx)
    result={'schema':'mb3d.native-ordinary-capture.v1','status':'PASS_SCOPED','suite_count':len(commands),
            'build_dir':str(build),'renderer':ref(build/'mb3d_render'),'library':ref(build/'metal_native_scene.metallib'),
            'ctest_original':{'path':str(ctest),'sha256':sha(ctest)},'tool':ref(tool),
            'ordinary_commands':commands,'proof':proof,'qualification_credit':'ordinary proof only; global binding/build closure remain required'}
    write(out/'capture.json',result)
    return result


def main():
    p=argparse.ArgumentParser(description=__doc__)
    for k in ('repository','build','output'):p.add_argument('--'+k,type=Path,required=True)
    p.add_argument('--ctest',type=Path,default=Path(shutil.which('ctest') or '/missing/ctest'))
    a=p.parse_args();r=collect(a.repository,a.build,a.output,a.ctest)
    print(r['status'],r['suite_count'],'ordinary suites')

if __name__=='__main__':main()
