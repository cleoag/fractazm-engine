#!/usr/bin/env python3
"""Build an isolated host-only fault recorder against a frozen build.

No production file/build is edited. The selected offline library is recorded
without modification; it may be a separately instrumented diagnostic. This is a
failure diagnostic, never clean-build provenance or scene qualification.
"""
import argparse
import json
from pathlib import Path
import shlex
import subprocess
from native_gpu_coverage import sha256
from run_m3parameter_gpu import invoke
ROOT = Path(__file__).resolve().parents[2]


def main():
    p = argparse.ArgumentParser(description=__doc__)
    for name in ('build','scene','runtime','output'): p.add_argument('--'+name,type=Path,required=True)
    p.add_argument('--library', type=Path)
    a = p.parse_args(); b = a.build.resolve(); out = a.output.resolve()
    library = a.library.resolve() if a.library else b/'metal_native_scene.metallib'
    assert not out.exists(); out.mkdir(parents=True)
    command = next(x for x in json.loads((b/'compile_commands.json').read_text()) if x['file'].endswith('/general_renderer.mm'))
    source = Path(command['file']); original = source.read_text(); path = out/'primary-fault.bin'
    needle = '        if(g.formula_fault.code){++out.execution.faults;'
    assert original.count(needle)==1
    injected = '''        if(g.formula_fault.code){
            std::ofstream dump(PATH,std::ios::binary);
            const uint32_t identity[]={0x4d423352u,1,index,scene.width,scene.height,
                sizeof(mb3d_native::NativeFormulaProgram),sizeof(scene),sizeof(ExactPixel),sizeof(g),sizeof(mb3d_camera::Ray)};
            dump.write(reinterpret_cast<const char*>(identity),sizeof(identity));
            dump.write(static_cast<const char*>(program.contents),sizeof(mb3d_native::NativeFormulaProgram));
            dump.write(reinterpret_cast<const char*>(&scene),sizeof(scene));
            dump.write(reinterpret_cast<const char*>(static_cast<const ExactPixel*>(b.pixels.contents)+index),sizeof(ExactPixel));
            dump.write(reinterpret_cast<const char*>(&g),sizeof(g));
            dump.write(reinterpret_cast<const char*>(static_cast<const mb3d_camera::Ray*>(camera.rays.contents)+index),sizeof(mb3d_camera::Ray));
            gpu::require(bool(dump),"Cannot capture primary failure");
        }
'''.replace('PATH',json.dumps(str(path)))
    diagnostic = out/'general_renderer.mm'; diagnostic.write_text(original.replace(needle,injected+needle))
    compile_cmd = shlex.split(command['command']); compile_cmd[compile_cmd.index('-o')+1]=str(out/'general_renderer.o')
    compile_cmd[compile_cmd.index('-c')+1]=str(diagnostic); compile_cmd+=['-I'+str(source.parent)]
    # compile_commands normally excludes dependency side effects; reject an
    # unexpected command rather than modifying the frozen build's depfiles.
    assert not any(x in compile_cmd for x in ('-MF','-MD','-MMD','-MT','-MQ'))
    link = shlex.split((b/'CMakeFiles/mb3d_render.dir/link.txt').read_text())
    if '--kind' in link: link=link[link.index('--')+1:]
    link[link.index('-o')+1]=str(out/'mb3d_render'); link.insert(link.index('libmb3d_render_backend.a'),str(out/'general_renderer.o'))
    inputs={str(source):sha256(source)}
    for x in link[1:]:
        f=(b/x).resolve()
        if f.is_file(): inputs[str(f)]=sha256(f)
    for label,cmd in [('compile',compile_cmd),('link',link)]:
        result=subprocess.run(cmd,cwd=b,text=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,timeout=90)
        (out/(label+'.json')).write_text(json.dumps({'argv':cmd,'cwd':str(b),'exit_code':result.returncode},indent=2)+'\n')
        (out/(label+'.log')).write_text(result.stdout); result.check_returncode()
    frame=out/'frame'; cmd=[out/'mb3d_render',a.scene.resolve(),'--backend','metal','--precision','native',
        '--threads','6','--metal-library',library,'--output',frame/'image.png','--profile-json',frame/'profile.json']
    event=invoke(cmd,frame,a.runtime.resolve(),timeout=60,evidence_root=ROOT)
    assert all(sha256(Path(f))==h for f,h in inputs.items())
    assert event['exit_code']==3 and path.is_file() and not (frame/'image.png').exists()
    (out/'receipt.json').write_text(json.dumps({'qualification':False,'kind':'isolated host failure capture',
        'inputs':inputs,'original_host_source_sha256':sha256(source),'diagnostic_source_sha256':sha256(diagnostic),
        'library_sha256':sha256(library),'capture_sha256':sha256(path)},indent=2)+'\n')
    print(path)


if __name__ == '__main__': main()
