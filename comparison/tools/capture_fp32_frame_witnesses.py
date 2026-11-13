#!/usr/bin/env python3
"""Isolated FP32 witness recorder; production build/source/runtime stay unchanged.

The diagnostic library observes finite radius operands and overflowing Integer
Power8 steps. All normal primary outputs must match an uninstrumented library
byte-for-byte before any recorded operands can support scene acceptance.
"""
import argparse,json,os,shlex,shutil,subprocess
from pathlib import Path
from native_gpu_coverage import sha256
from run_m3parameter_gpu import invoke
ROOT=Path(__file__).resolve().parents[2]

WITNESS=r'''
struct NativeRangeWitness {
    uint kind,slot,iteration,power;
    float input[4],j[4],output[4],z_mul;
    uint four,reserved[2];
};
'''
RADIUS=r'''
inline void native_radius_witness(thread const NativeResources& resources,
    thread const NativeFormulaState& s,bool four) {
    if(!resources.witness||resources.witness->kind)return;
    const float4 v(s.xyz[0],s.xyz[1],s.xyz[2],s.xyz[3]);
    if(!all(isfinite(v))||!isinf(native_radius(s,four)))return;
    thread auto& w=*resources.witness;w.kind=1;w.slot=s.current_slot;w.iteration=uint(s.iteration);w.four=uint(four);
    for(uint i=0;i<4;++i)w.input[i]=s.xyz[i];
}
'''
BEFORE=r'''
    const bool watch=resources.witness&&!resources.witness->kind&&spec.namespace_id==InternalFormula&&
        spec.id==0&&spec.internal_power==8&&abs(spec.negative_params[1])==1.f;
    float4 witness_input(0),witness_j(0);
    if(watch){witness_input=float4(s.xyz[0],s.xyz[1],s.xyz[2],s.xyz[3]);witness_j=float4(s.j[0],s.j[1],s.j[2],s.j[3]);}
'''
AFTER=r'''
    if(overflow&&watch&&all(isfinite(witness_input))&&all(isfinite(witness_j))&&
       max(max(abs(witness_input.x),abs(witness_input.y)),abs(witness_input.z))>=0x1p17f&&
       witness_input.x*witness_input.x+witness_input.y*witness_input.y>=1.f){
        thread auto& w=*resources.witness;w.kind=2;w.slot=slot;w.iteration=uint(s.iteration);w.power=8;w.z_mul=spec.negative_params[1];
        for(uint i=0;i<4;++i){w.input[i]=witness_input[i];w.j[i]=witness_j[i];w.output[i]=s.xyz[i];}
    }
'''
HOST_DUMP=r'''
    {
        const char* destination=std::getenv("NATIVE_RANGE_CAPTURE");
        gpu::require(destination&&*destination,"Missing diagnostic capture destination");
        gzFile dump=gzopen(destination,"wb1");gpu::require(dump!=nullptr,"Cannot create witness capture");
        auto write=[&](const void* ptr,uint64_t bytes){
            const auto* data=static_cast<const unsigned char*>(ptr);
            while(bytes){const unsigned chunk=unsigned(std::min<uint64_t>(bytes,1u<<20));
                if(gzwrite(dump,data,chunk)!=int(chunk)){gzclose(dump);throw std::runtime_error("Witness capture write failed");}
                data+=chunk;bytes-=chunk;}
        };
        const uint32_t header[]={0x4d425257u,1,scene.width,scene.height,10};write(header,sizeof(header));
        auto block=[&](const void* ptr,uint64_t bytes){write(&bytes,sizeof(bytes));write(ptr,bytes);};
        block(program.contents,program.length);block(&scene,sizeof(scene));
        block(records.contents,records.length);block(b.pixels.contents,b.pixels.length);block(b.surface.contents,b.surface.length);
        block(camera.rays.contents,camera.rays.length);block(normalRecords.contents,normalRecords.length);
        block(normalResults.contents,normalResults.length);block(insideRecords.contents,insideRecords.length);
        block(rangeWitness.contents,rangeWitness.length);
        gpu::require(gzclose(dump)==Z_OK,"Witness capture close failed");
    }
'''

def replace(text,old,new,count=1):
    assert text.count(old)==count,(old,text.count(old));return text.replace(old,new)


def instrument_native(native):
    f=native/'formula_abi.h';s=f.read_text();s=replace(s,'struct NativeResources {',WITNESS+'struct NativeResources {\n    thread NativeRangeWitness* witness=nullptr;')
    # Preserve aggregate initializer field order used throughout the library.
    s=s.replace('    thread NativeRangeWitness* witness=nullptr;','');s=replace(s,'    device const float* volume_map;','    device const float* volume_map;\n    thread NativeRangeWitness* witness;');f.write_text(s)
    f=native/'orbit_driver.metal.h';s=f.read_text()
    marker='inline bool native_step(thread NativeFormulaState& s,constant NativeFormulaProgram& p,'
    at=s.index(marker);s=s[:at]+RADIUS+s[at:]
    s=replace(s,'    bool ok=spec.namespace_id==InternalFormula?',BEFORE+'    bool ok=spec.namespace_id==InternalFormula?')
    needle='    overflow=!all(isfinite(float4(s.xyz[0],s.xyz[1],s.xyz[2],s.xyz[3])))||!isfinite(s.rout)||!isfinite(s.vary_scale);'
    s=replace(s,needle,needle+AFTER)
    needle='if(!isfinite(s.rout)&&!(radius_range&&radius_range->certified)){exit_pass=index;return 0.f;}'
    s=replace(s,needle,'if(!isfinite(s.rout)&&!(radius_range&&radius_range->certified)){native_radius_witness(resources,s,four);exit_pass=index;return 0.f;}');f.write_text(s)
    f=native/'camera_kernels.metal.h';s=f.read_text();start=s.index('kernel void native_camera_general_primary(');end=s.index('// Primary jitter probe',start);body=s[start:end]
    body=replace(body,'    uint local [[thread_position_in_grid]]) {','    device mb3d_native::NativeRangeWitness* witnesses [[buffer(5)]],\n    uint local [[thread_position_in_grid]]) {')
    body=replace(body,'    const uint index=launch.x+local;','    const uint index=launch.x+local;\n    mb3d_native::NativeRangeWitness witness{};')
    body=replace(body,'        resources.volume=&volume;resources.volume_map=volume_map;','        resources.volume=&volume;resources.volume_map=volume_map;resources.witness=&witness;')
    body=replace(body,'    inside_record.fault|=record.fault;','    witnesses[index]=witness;\n    inside_record.fault|=record.fault;');f.write_text(s[:start]+body+s[end:])


def instrument_host(h):
    h='#include <zlib.h>\n'+h
    needle='    id<MTLComputePipelineState> primary=primaryPersistent?nil:m.pipeline(@"native_camera_general_primary");'
    h=replace(h,needle,'    gpu::require(!primaryPersistent,"Witness recorder requires independent primary ownership");\n    auto rangeWitness=m.buffer(uint64_t(count)*80);std::memset(rangeWitness.contents,0,rangeWitness.length);\n'+needle)
    needle='\n            [e setBuffer:b.cols offset:0 atIndex:4];[e setBuffer:b.rows offset:0 atIndex:5];'
    # Only the independent pixel branch binds this exact indentation.
    h=replace(h,needle,needle+'\n            if(!probe)[e setBuffer:rangeWitness offset:0 atIndex:5];')
    needle='    if(primaryRangePixels*100>uint64_t(count)*fp32RangePixelPercent()){';h=replace(h,needle,HOST_DUMP+needle)
    return h


def main():
    p=argparse.ArgumentParser(description=__doc__)
    for key in ('build','output'):p.add_argument('--'+key,type=Path,required=True)
    p.add_argument('--scene',type=Path);p.add_argument('--runtime',type=Path)
    p.add_argument('--width',type=int);p.add_argument('--height',type=int)
    p.add_argument('--reuse',action='store_true')
    a=p.parse_args();b=a.build.resolve();out=a.output.resolve()
    if not a.reuse:
        assert not out.exists();out.mkdir(parents=True)
        assert shutil.disk_usage(out).free>=14*1024**3
        command=next(x for x in json.loads((b/'compile_commands.json').read_text()) if x['file'].endswith('/general_renderer.mm'))
        source=Path(command['file']);origin=source.parents[3];local=out/'source';native=local/'src/render/native_metal'
        shutil.copytree(origin/'src/render/native_metal',native)
        shutil.copytree(origin/'comparison/metal_exact',local/'comparison/metal_exact')
        originals={str(x.relative_to(origin)):sha256(x) for x in (origin/'src/render/native_metal').rglob('*') if x.is_file()}
        instrument_native(native)
        commands=[]
        def run(cmd,cwd,label):
            z=subprocess.run(cmd,cwd=cwd,text=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,timeout=180)
            (out/(label+'.log')).write_text(z.stdout);commands.append({'argv':cmd,'cwd':str(cwd),'exit_code':z.returncode});z.check_returncode()
        records=[json.loads(x.read_text()) for x in (b.parent/'provenance/records').glob('*.json')]
        air={};air_commands={}
        for d in records:
            argv=d.get('argv',[])
            if 'metal' not in argv or '-c' not in argv:continue
            old=Path(argv[argv.index('-o')+1])
            if old.suffix!='.air':continue
            if old.name in air:
                assert argv==air_commands[old.name];continue
            new=out/old.name;air[old.name]=new;air_commands[old.name]=argv
            cmd=[str(local/Path(x).relative_to(origin)) if str(x).startswith(str(origin/'src/render/native_metal')+'/') else x for x in argv]
            cmd[cmd.index('-o')+1]=str(new);cmd[cmd.index('-MF')+1]=str(new.with_suffix('.d'))
            cmd[4:4]=['-I',str(local),'-I',str(local/'src')]
            run(cmd,origin,old.stem)
        assert len(air)==6
        link=next(d['argv'] for d in records if 'metallib' in d.get('argv',[]) and d.get('output',{}).get('path')==str(b/'metal_native_scene.metallib'))
        cmd=[str(air[Path(x).name]) if Path(x).name in air else x for x in link];cmd[cmd.index('-o')+1]=str(out/'instrumented.metallib');run(cmd,origin,'metallib')
        h=instrument_host(source.read_text())
        diagnostic=out/'general_renderer.mm';diagnostic.write_text(h)
        cmd=shlex.split(command['command']);cmd[cmd.index('-o')+1]=str(out/'general_renderer.o');cmd[cmd.index('-c')+1]=str(diagnostic);cmd+=['-I'+str(source.parent)]
        assert not any(x in cmd for x in ('-MD','-MMD','-MF'));run(cmd,b,'host-compile')
        cmd=shlex.split((b/'CMakeFiles/mb3d_render.dir/link.txt').read_text())
        if '--kind' in cmd:cmd=cmd[cmd.index('--')+1:]
        cmd[cmd.index('-o')+1]=str(out/'recorder');cmd.insert(cmd.index('libmb3d_render_backend.a'),str(out/'general_renderer.o'));run(cmd,b,'host-link')
        receipt={'qualification':False,'production_build':str(b),'production_renderer_sha256':sha256(b/'mb3d_render'),
            'production_library_sha256':sha256(b/'metal_native_scene.metallib'),'recorder_sha256':sha256(out/'recorder'),
            'instrumented_library_sha256':sha256(out/'instrumented.metallib'),'original_native_sources':originals,
            'instrumented_sources':{str(x.relative_to(local)):sha256(x) for x in native.rglob('*') if x.is_file()},'commands':commands}
        (out/'recorder.json').write_text(json.dumps(receipt,indent=2)+'\n')
    if not a.scene:return
    assert a.runtime and bool(a.width)==bool(a.height)
    receipt=json.loads((out/'recorder.json').read_text());assert sha256(out/'recorder')==receipt['recorder_sha256']
    assert sha256(out/'instrumented.metallib')==receipt['instrumented_library_sha256']
    label=a.scene.stem+('-preview' if a.width else '-original');dest=out/label;assert a.reuse or not dest.exists()
    for mode,library in [('baseline',b/'metal_native_scene.metallib'),('witness',out/'instrumented.metallib')]:
        folder=dest/mode;argv=[out/'recorder',a.scene.resolve(),'--backend','metal','--precision','native','--threads','6',
            '--metal-library',library,'--output',folder/'image.png','--profile-json',folder/'profile.json']
        if a.width:argv+=['--width',str(a.width),'--height',str(a.height)]
        if (folder/'event.json').exists():
            e=json.loads((folder/'event.json').read_text())
            assert a.reuse and e['argv']==list(map(str,argv)) and not e['exit_code'] and not e['guard'] and e['seconds']<=180
            assert e['executable_sha256_before']==e['executable_sha256_after']==sha256(out/'recorder')
            assert all(sha256(ROOT/r['path'])==r['sha256'] for r in e['inputs']+e['outputs'])
            continue
        old=os.environ.get('NATIVE_RANGE_CAPTURE');os.environ['NATIVE_RANGE_CAPTURE']=str(folder/'capture.gz')
        try:event=invoke(argv,folder,a.runtime.resolve(),timeout=300,evidence_root=ROOT)
        finally:
            if old is None:os.environ.pop('NATIVE_RANGE_CAPTURE',None)
            else:os.environ['NATIVE_RANGE_CAPTURE']=old
        event['diagnostic_environment']={'NATIVE_RANGE_CAPTURE':str(folder/'capture.gz')};(folder/'event.json').write_text(json.dumps(event,indent=2)+'\n')
        assert not event['exit_code'] and not event['guard'] and event['seconds']<=180,event
        print(label,mode,'captured',flush=True)


if __name__=='__main__':main()
