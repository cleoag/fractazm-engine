"""Build the library and run the small release check on native Apple Silicon."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import platform
import subprocess
import sys
import time
import xml.etree.ElementTree as ET
from native_evidence import source_identity
from release_smoke import check_disk, gpu_slot

ROOT=Path(__file__).resolve().parents[1]

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--scope',choices=['release','primitives','qualified','ordinary'],default='release')
    parser.add_argument('--cpu-only',action='store_true',help='Build without Metal and run only CPU smoke renders')
    parser.add_argument('--trace',choices=['ON','OFF'],default='ON')
    parser.add_argument('--lto',action='store_true',help='Enable compiler interprocedural optimization; numerical gates stay unchanged')
    parser.add_argument('--sanitizer',choices=['none','address','thread'],default='none')
    parser.add_argument('--jobs',type=int,default=4)
    parser.add_argument('--build-dir',type=Path)
    parser.add_argument('--output',type=Path,required=True)
    args=parser.parse_args()
    if platform.system()!='Darwin' or platform.machine()!='arm64':
        parser.error('Run with native Apple Silicon Python; Rosetta/Windows are excluded')
    translated=subprocess.run(['sysctl','-in','sysctl.proc_translated'],capture_output=True,text=True)
    if translated.stdout.strip()=='1': parser.error('Translated execution is excluded')
    if args.jobs<1: parser.error('--jobs must be positive')
    build=(args.build_dir or ROOT/(f'build-native-arm64-{args.scope}-{args.trace}-{args.sanitizer}'+('-lto' if args.lto else ''))).resolve()
    out=args.output.resolve();out.mkdir(parents=True,exist_ok=False)
    report={'status':'RUNNING','scope':args.scope,'trace':args.trace,'sanitizer':args.sanitizer,'lto':args.lto,
        'platform':platform.platform(),'machine':platform.machine(),'build_dir':str(build),
        **source_identity(),
        'commands':[],'binaries':{},'tests':[]}
    env={k:v for k,v in os.environ.items() if not k.upper().startswith('MB3D_')}
    if args.sanitizer=='address':
        # Apple's installed ASan runtime rejects detect_leaks=1 before main.
        # Retain address/UB checking; do not report LeakSanitizer coverage.
        env['ASAN_OPTIONS']='detect_leaks=0:halt_on_error=1'
        env['UBSAN_OPTIONS']='halt_on_error=1:print_stacktrace=1'
        report['leak_sanitizer']='unavailable in Apple runtime; disabled explicitly'
    def save(): (out/'bootstrap.json').write_text(json.dumps(report,indent=2)+'\n')
    def run(name,command,timeout=1800,lock_gpu=False):
        check_disk(out)
        start=time.monotonic()
        print(name,flush=True)
        with gpu_slot(lock_gpu), (out/(name+'.log')).open('wb') as log:
            result=subprocess.run(command,cwd=ROOT,env=env,stdout=log,stderr=subprocess.STDOUT,timeout=timeout)
        report['commands'].append({'name':name,'argv':command,'exit':result.returncode,'seconds':time.monotonic()-start})
        save()
        return result.returncode
    try:
        config=['cmake','-S',str(ROOT),'-B',str(build),'-DCMAKE_BUILD_TYPE=Release',
                '-DCMAKE_OSX_ARCHITECTURES=arm64','-DCMAKE_EXPORT_COMPILE_COMMANDS=ON',
                '-DMB3D_NATIVE_TESTS_ONLY='+('ON' if args.scope=='primitives' else 'OFF'),
                '-DMB3D_ENABLE_METAL_RENDERER='+('OFF' if args.cpu_only or args.scope=='primitives' else 'ON'),
                '-DMB3D_BUILD_DIAGNOSTICS=OFF',
                '-DMB3D_ENABLE_PARITY_TRACE='+args.trace,
                '-DCMAKE_INTERPROCEDURAL_OPTIMIZATION='+('ON' if args.lto else 'OFF')]
        sanitizer={'none':'','address':'-fsanitize=address,undefined -fno-omit-frame-pointer -g',
                   'thread':'-fsanitize=thread -fno-omit-frame-pointer -g'}[args.sanitizer]
        scale={'none':1,'address':2,'thread':10}[args.sanitizer]
        report['test_timeout_scale']=scale
        config+=['-DCMAKE_C_FLAGS='+sanitizer,'-DCMAKE_CXX_FLAGS='+sanitizer,
                 '-DMB3D_TEST_TIMEOUT_SCALE='+str(scale)]
        if run('configure',config): raise RuntimeError('configure failed')
        target,label={'release':('mb3d_release_checks','release'),
                      'primitives':('mb3d_native_tests','native_cpu'),
                      'qualified':('mb3d_arm64_qualified','arm64_qualified'),
                      'ordinary':('mb3d_ordinary','ordinary')}[args.scope]
        if run('build',['cmake','--build',str(build),'--target',target,'--parallel',str(args.jobs)]):
            raise RuntimeError('build failed')
        for binary in sorted(build.glob('mb3d_*')):
            if not binary.is_file() or not os.access(binary,os.X_OK): continue
            arch=subprocess.check_output(['lipo','-archs',str(binary)],text=True).strip()
            if arch!='arm64': raise RuntimeError('Non-native binary: '+str(binary))
            report['binaries'][binary.name]={'arch':arch,'sha256':hashlib.sha256(binary.read_bytes()).hexdigest()}
        junit=out/'ctest.xml'
        status=run('ctest',['ctest','--test-dir',str(build),'-L','^'+label+'$',
            '--output-on-failure','--timeout',str(180*scale),'--parallel','1',
            '--output-junit',str(junit)],lock_gpu=not args.cpu_only and args.scope!='primitives')
        if junit.exists():
            report['tests']=[{'name':t.attrib['name'],'status':t.attrib.get('status'),
                'seconds':t.attrib.get('time'),'failed':t.find('failure') is not None} for t in ET.parse(junit).iter('testcase')]
        if not report['tests']: raise RuntimeError('No tests selected')
        if status: raise RuntimeError('Selected tests failed')
        if args.scope=='release':
            status=run('smoke',[sys.executable,'-B',str(ROOT/'scripts/release_smoke.py'),
                '--build',str(build),'--output',str(out/'smoke'),
                '--backend','cpu' if args.cpu_only else 'all'],timeout=1200)
            report['smoke']=json.loads((out/'smoke/smoke.json').read_text())
        report['status']='FAIL' if status else ('PASS_RELEASE_CHECK' if args.scope=='release' else 'PASS_SCOPED')
        report['limits']='Release checks cover selected APIs and small real renders, not complete formula/Pascal qualification. Only this host was tested; no Windows, Monte Carlo or JIT execution.'
        save();print(out/'bootstrap.json');return status
    except Exception as error:
        report.update(status='ERROR',error=str(error));save();print(error,file=sys.stderr);return 1

if __name__=='__main__': raise SystemExit(main())
