#!/usr/bin/env python3
"""Link the prototype against an explicitly selected, existing engine build.
Compiles current viewer adapters over pinned engine archives; never mutates that build.
"""
import argparse
import hashlib
import io
import json
import pathlib
import plistlib
import shutil
import shlex
import subprocess
import tarfile

ROOT = pathlib.Path(__file__).resolve().parents[2]
LIBS = ['mb3d_render_backend', 'mb3d_native_formula_host', 'mb3d_io',
        'mb3d_core', 'mb3d_runtime', 'mb3d_softfloat']
def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()
def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--engine-build', type=pathlib.Path, required=True)
    p.add_argument('--engine-source', required=True, help='Exact source commit of selected engine')
    p.add_argument('--output', type=pathlib.Path, default=ROOT/'build/macos-viewer')
    a = p.parse_args()
    engine, out = a.engine_build.resolve(), a.output.resolve()
    if out == engine or engine in out.parents or out in engine.parents:
        p.error('Output and engine build must be separate')
    if shutil.disk_usage(out.parent if out.parent.exists() else ROOT).free < 13*1024**3:
        p.error('Need 13 GiB free before packaging')
    commit = subprocess.check_output(['git','rev-parse',f'{a.engine_source}^{{commit}}'],cwd=ROOT,text=True).strip()
    paths = [engine/f'lib{x}.a' for x in LIBS] + [engine/'metal_native_scene.metallib']
    for path in paths:
        if not path.is_file(): p.error(f'Missing selected engine artifact: {path}')
    out.mkdir(parents=True,exist_ok=True)
    headers = out/'engine-headers'
    if headers.exists(): shutil.rmtree(headers)
    headers.mkdir()
    # Header snapshot belongs to the selected engine, not the potentially newer checkout.
    archive=subprocess.check_output(['git','archive',commit,'src/core','src/io','src/render','include'],cwd=ROOT)
    with tarfile.open(fileobj=io.BytesIO(archive)) as tar:
        for member in tar:
            if member.isfile() and member.name.endswith('.h'):
                dest=headers/member.name;dest.parent.mkdir(parents=True,exist_ok=True)
                dest.write_bytes(tar.extractfile(member).read())
    # These narrow host adapters intentionally supersede the archived members.
    # CPU execution and production shader bytes stay pinned. Shared host preparation
    # is rebuilt explicitly into a separate archive below, before the old archive.
    for name in ['render/navigation_camera.h','render/backend.h','render/metal_renderer.h','render/native_metal/navigation_preview.h','render/native_metal/general_navigation.h','render/native_metal/general_scene_host.h','render/native_metal/formula_host.h','render/native_metal/restricted_primary_admission.h']:
        dest=headers/'src'/name;dest.parent.mkdir(parents=True,exist_ok=True)
        shutil.copy2(ROOT/'src'/name,dest)
    app=out/'Mandelbulb Viewer.app'; mac=app/'Contents/MacOS'; resources=app/'Contents/Resources'
    mac.mkdir(parents=True,exist_ok=True);resources.mkdir(parents=True,exist_ok=True)
    (resources/'runtime/M3Maps').mkdir(parents=True,exist_ok=True)
    shutil.copy2(paths[-1], resources/paths[-1].name)
    shutil.copy2(ROOT/'comparison/scenes/test_01_intpow2.m3p',resources/'demo.m3p')
    with (app/'Contents/Info.plist').open('wb') as f:
        plistlib.dump(dict(CFBundleExecutable='MandelbulbViewer',CFBundleIdentifier='org.mb3d.viewer.prototype',
            CFBundleName='Mandelbulb Viewer',CFBundlePackageType='APPL',CFBundleVersion='6',
            CFBundleShortVersionString='0.6-candidate',NSHighResolutionCapable=True),f)
    command=['xcrun','clang++','-std=c++20','-O2','-DMB3D_ENABLE_METAL_RENDERER','-arch','arm64','-fobjc-arc','-fno-fast-math','-ffp-contract=off',
        '-I'+str(headers),'-I'+str(ROOT),'-I'+str(headers/'src'),'-I'+str(headers/'include'),'-MMD']
    if 'MB3D_ENABLE_PARITY_TRACE:BOOL=ON' in (engine/'CMakeCache.txt').read_text():
        command+=['-DMB3D_ENABLE_PARITY_TRACE']
    compile_command=command+['-c',str(ROOT/'apps/macos_viewer/main.mm'),'-o',str(out/'viewer.o')]
    adapters=[ROOT/'src/render/backend.cpp',ROOT/'src/render/metal_renderer.mm',ROOT/'src/render/native_metal/navigation_preview.mm',ROOT/'src/render/native_metal/general_navigation.mm']
    adapter_commands=[command+['-c',str(src),'-o',str(out/(src.stem+'.o'))] for src in adapters]
    preparation=[ROOT/'src/render/native_metal/general_scene_host.cpp', ROOT/'src/render/native_metal/formula_host.cpp']
    preparation_commands=[command+['-c',str(src),'-o',str(out/(src.stem+'.o'))] for src in preparation]
    preparation_archive=out/'libmb3d_p1_preparation.a'
    archive_command=['xcrun','ar','rcs',str(preparation_archive),*[str(out/(src.stem+'.o')) for src in preparation]]
    shader_commands=[['xcrun','-sdk','macosx','metal','-std=metal3.0','-fno-fast-math','-MMD','-MF',str(out/'navigation_preview.d'),'-I',str(ROOT),'-c',str(ROOT/'src/render/native_metal/navigation_preview.metal'),'-o',str(out/'navigation_preview.air')],
        ['xcrun','-sdk','macosx','metallib',str(out/'navigation_preview.air'),'-o',str(resources/'navigation_preview.metallib')]]
    shader_commands += [
        ['xcrun','-sdk','macosx','metal','-std=metal3.1','-fno-fast-math','-ffp-contract=fast','-MMD','-MF',str(out/'general_navigation.d'),'-I',str(headers),'-I',str(ROOT),'-c',str(ROOT/'src/render/native_metal/general_navigation.metal'),'-o',str(out/'general_navigation.air')],
        ['xcrun','-sdk','macosx','metallib',str(out/'general_navigation.air'),'-o',str(resources/'general_navigation.metallib')]]
    link_command=['xcrun' ,'clang++','-arch','arm64',str(out/'viewer.o'),*[str(out/(src.stem+'.o')) for src in adapters],str(preparation_archive),*[str(x) for x in paths[:-1]],
        '-framework','Cocoa','-framework','QuartzCore','-framework','Metal','-framework','UniformTypeIdentifiers','-lz','-o',str(mac/'MandelbulbViewer')]
    with (out/'build.log').open('w') as log:
        for cmd in [compile_command,*adapter_commands,*preparation_commands,archive_command,*shader_commands,link_command]:
            log.write(json.dumps(cmd)+'\n');log.flush()
            subprocess.run(cmd,check=True,stdout=log,stderr=subprocess.STDOUT)
    subprocess.run(['codesign','--force','--sign','-',str(app)],check=True)
    launcher=out/'Launch Experimental Preview.command'
    launcher.write_text('#!/bin/zsh\nset -e\ntask_viewer_dir=${0:A:h}\nexport MB3D_VIEWER_GENERAL_PREVIEW=1\nexec "$task_viewer_dir/Mandelbulb Viewer.app/Contents/MacOS/MandelbulbViewer"\n')
    launcher.chmod(0o755)

    dependencies={}
    for dep in out.glob('*.d'):
        for item in shlex.split(dep.read_text().replace(chr(92)+chr(10),' ').split(':',1)[1]):
            path=pathlib.Path(item)
            if path.is_file(): dependencies[str(path.resolve())]=sha(path)
    record=dict(dependencies=dependencies,objects={x.name:sha(x) for x in out.glob('*.o')},engine_source=commit,viewer_base_source=subprocess.check_output(['git','rev-parse','HEAD'],cwd=ROOT,text=True).strip(),
        engine_build=str(engine),engine_artifacts={str(x):sha(x) for x in paths},
        commands=[compile_command,*adapter_commands,*preparation_commands,archive_command,*shader_commands,link_command],
        adapter_sources={str(x.relative_to(ROOT)):sha(x) for x in adapters+preparation},
        preparation_archive_sha256=sha(preparation_archive),
        general_preview_shader_sha256=sha(resources/"general_navigation.metallib"),
        shader_objects={x.name:sha(x) for x in out.glob("*.air")},
        preview_shader_sha256=sha(resources/"navigation_preview.metallib"),object_sha256=sha(out/'viewer.o'),executable_sha256=sha(mac/'MandelbulbViewer'),
        viewer_sources={str(x.relative_to(ROOT)):sha(x) for x in (ROOT/'apps/macos_viewer').glob('*') if x.is_file()},
        interface_headers={str(x.relative_to(headers)):sha(x) for x in headers.rglob('*.h')},
        note='New prototype host identity. Does not inherit engine scene qualification. Engine provenance remains in its selected build record.')
    (out/'build-record.json').write_text(json.dumps(record,indent=2)+'\n')
    print(app)
if __name__=='__main__': main()
