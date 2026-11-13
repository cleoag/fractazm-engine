#!/usr/bin/env python3
"""No-cache build launcher. Record one actual invocation and its file closure."""
from pathlib import Path
import argparse,hashlib,json,os,shlex,shutil,subprocess,sys,time,uuid

def sha(path):
    h=hashlib.sha256()
    with open(path,'rb') as stream:
        for block in iter(lambda:stream.read(1024*1024),b''):h.update(block)
    return h.hexdigest()

def identity(path):
    path=Path(path).resolve();s=path.stat()
    return {'path':str(path),'sha256':sha(path),'size':s.st_size,'device':s.st_dev,
            'inode':s.st_ino,'mtime_ns':s.st_mtime_ns,'ctime_ns':s.st_ctime_ns}

def dep_paths(path,cwd):
    # A compiler depfile's first logical rule lists its complete dependency set;
    # any following -MP phony rules are redundant. Do not execute Make syntax.
    text=Path(path).read_text().replace('\\\n','')
    line=next((line for line in text.splitlines() if line.strip()),'')
    split=next((i for i,c in enumerate(line) if c==':' and (i==0 or line[i-1]!='\\')),None)
    if split is None:raise ValueError('Missing dependency target separator: '+str(path))
    tokens=shlex.split(line[split+1:],comments=False,posix=True)
    return sorted({str((Path(cwd)/token.replace('$$','$')).resolve()) for token in tokens})

def expanded_args(args,cwd):
    result=[]
    for value in args:
        if value.startswith('@'):
            response=(Path(cwd)/value[1:]).resolve()
            result.extend(shlex.split(response.read_text(),comments=False,posix=True))
        else:result.append(value)
    return result

def output_path(argv,cwd):
    if '-o' in argv:return (Path(cwd)/argv[argv.index('-o')+1]).resolve()
    tool=Path(argv[0]).name
    if tool in ('ar','llvm-ar') and len(argv)>2:return (Path(cwd)/argv[2]).resolve()
    if tool in ('ranlib','llvm-ranlib') and len(argv)>1:return (Path(cwd)/argv[-1]).resolve()
    return None

def dependency_path(argv,output,cwd):
    if '-MF' in argv:return (Path(cwd)/argv[argv.index('-MF')+1]).resolve()
    if output and output.suffix in ('.o','.obj'):
        candidate=Path(str(output)+'.d')
        if candidate.is_file():return candidate
    return None

def classification(path,context):
    path=Path(path).resolve();build=Path(context['build']);root=Path(context['root'])
    if path.is_relative_to(build):
        return 'downloaded_dependency_source' if '/_deps/' in str(path) and '-src/' in str(path) else 'build_generated'
    if path.is_relative_to(root):return 'repository'
    return 'sdk_or_system'

def main():
    parser=argparse.ArgumentParser();parser.add_argument('--kind',required=True,choices=['compile','link','custom'])
    parser.add_argument('command',nargs=argparse.REMAINDER);a=parser.parse_args()
    argv=a.command[1:] if a.command[:1]==['--'] else a.command
    if not argv:raise ValueError('Missing captured command')
    context=json.loads(Path(os.environ.get('MB3D_CLEAN_CAPTURE_CONTEXT', str(Path(__file__).with_name('context.json')))).read_text())
    cwd=Path.cwd();original_argv=list(argv);argv=expanded_args(argv,cwd);record_dir=Path(context['records'])
    token=f'{time.time_ns()}-{os.getpid()}-{uuid.uuid4().hex}'
    record={'schema':'mb3d.actual-build-command.v1','kind':a.kind,'argv':original_argv,'expanded_argv':argv,'cwd':str(cwd),
            'pid':os.getpid(),'start_time_ns':time.time_ns(),'started_monotonic':time.monotonic(),
            'phase':context['phase'],
            'context_sha256':sha(os.environ.get('MB3D_CLEAN_CAPTURE_CONTEXT', str(Path(__file__).with_name('context.json'))))}
    executable=shutil.which(argv[0]) if not Path(argv[0]).is_absolute() else argv[0]
    if executable and Path(executable).is_file():record['tool']=identity(executable)
    out=output_path(argv,cwd);record['output_existed_before']=bool(out and out.exists())
    inputs=[]
    for value in original_argv:
        if value.startswith('@'):
            p=(cwd/value[1:]).resolve()
            inputs.append({**identity(p),'classification':classification(p,context)})
    dependency_output=(cwd/argv[argv.index('-MF')+1]).resolve() if '-MF' in argv else None
    for value in argv[1:]:
        p=(cwd/value).resolve()
        if not value.startswith('-') and p.is_file() and p!=out and p!=dependency_output:
            inputs.append({**identity(p),'classification':classification(p,context)})
    record['explicit_inputs_before']=inputs
    record_dir.mkdir(parents=True,exist_ok=True)
    start=record_dir/(token+'.started.json');start.write_text(json.dumps(record,indent=2)+'\n')
    result=subprocess.run(argv)
    record['exit_code']=result.returncode;record['end_time_ns']=time.time_ns()
    record['elapsed_seconds']=time.monotonic()-record.pop('started_monotonic')
    errors=[]
    for item in inputs:
        if identity(item['path'])!={k:v for k,v in item.items() if k!='classification'}:
            errors.append('Explicit input changed during invocation: '+item['path'])
    if out and out.is_file():record['output']=identity(out)
    dep=dependency_path(argv,out,cwd)
    if dep and dep.is_file():
        record['depfile']=identity(dep)
        try:record['dependencies']=[{**identity(p),'classification':classification(p,context)} for p in dep_paths(dep,cwd)]
        except Exception as error:errors.append('Dependency capture: '+str(error))
    elif out and out.suffix in ('.o','.obj','.air'):errors.append('Compiled output has no dependency file')
    record['capture_errors']=errors
    (record_dir/(token+'.json')).write_text(json.dumps(record,indent=2)+'\n')
    # Keep .started receipt to prove an interrupted command never silently vanishes.
    return result.returncode if result.returncode else (91 if errors else 0)

if __name__=='__main__':
    try:sys.exit(main())
    except Exception as error:print('clean capture failed:',error,file=sys.stderr);sys.exit(92)
