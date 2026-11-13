#pragma once
#include "shadow_host.h"
#include "shadow_math.h"
#include "shadow_trace_math.h"
#include "ao_host.h"
#include "color_host.h"
#include "scene_host_internal.h"
#include "core/renderer.h"
#include <functional>

#include "metal_device.h"
using mb3d::metal_exact::gpu::dispatchStage;
template<class T> static std::vector<T> readBuffer(id<MTLBuffer> buffer) {
    std::vector<T> out(buffer.length/sizeof(T));std::memcpy(out.data(),buffer.contents,buffer.length);return out;
}
template<class T> static void saveBuffer(const std::filesystem::path& path,const std::vector<T>& data) {
    std::ofstream f(path,std::ios::binary);f.write(reinterpret_cast<const char*>(data.data()),data.size()*sizeof(T));require(bool(f),"cannot save stage buffer");
}
static NSDictionary* compareSurfaces(const std::vector<ExactSurface>& gpu,const std::vector<ExactSurface>& cpu) {
    require(gpu.size()==cpu.size(),"surface size mismatch");uint64_t mismatch=0,faults=0;NSMutableArray* examples=[NSMutableArray array];
    for(size_t i=0;i<gpu.size();++i) {
        auto& a=gpu[i];auto& b=cpu[i];faults+=a.fault!=0;
        bool bad=std::memcmp(&a,&b,sizeof(a))!=0;
        mismatch+=bad;if(bad&&examples.count<8)[examples addObject:@{@"pixel":@(i),@"gpu_depth":@(a.depth_packed),@"cpu_depth":@(b.depth_packed),@"gpu_shadow":@(a.shadow),@"cpu_shadow":@(b.shadow),@"gpu_ao":@(a.amb_shadow),@"cpu_ao":@(b.amb_shadow),@"gpu_si":@(a.si_gradient),@"cpu_si":@(b.si_gradient),@"gpu_trap":@(a.o_trap),@"cpu_trap":@(b.o_trap),@"fault":@(a.fault)}];
    }
    return @{@"pixels":@(gpu.size()),@"different_pixels":@(mismatch),@"fault_pixels":@(faults),@"gate":@"all 48 bytes exact (all production surface fields and zero reserved/fault)",@"examples":examples};
}
static NSDictionary* compareColors(const std::vector<ExactColorPixel>& gpu,const std::vector<uint8_t>& cpu) {
    require(cpu.size()==gpu.size()*3,"CPU RGB size mismatch");uint64_t pixels=0,channels=0,faults=0;unsigned maximum=0;NSMutableArray* examples=[NSMutableArray array];
    for(size_t i=0;i<gpu.size();++i) {
        bool bad=false;faults+=gpu[i].fault!=0||gpu[i].reserved0||gpu[i].reserved1;
        for(unsigned c=0;c<3;++c) {int diff=std::abs(int((gpu[i].rgb>>(c*8))&255)-int(cpu[i*3+c]));maximum=std::max(maximum,unsigned(diff));channels+=diff!=0;bad|=diff!=0;}
        pixels+=bad;if(bad&&examples.count<8)[examples addObject:@{@"pixel":@(i),@"gpu":@(gpu[i].rgb&0xffffff),@"cpu":@(uint32_t(cpu[i*3])|(uint32_t(cpu[i*3+1])<<8)|(uint32_t(cpu[i*3+2])<<16))}];
    }
    return @{@"pixels":@(gpu.size()),@"channels":@(cpu.size()),@"different_pixels":@(pixels),@"different_channels":@(channels),@"max_channel_error":@(maximum),@"fault_pixels":@(faults),@"gate":@"RGB8 exact, zero channel error",@"examples":examples};
}
static std::vector<uint8_t> colorBytes(const std::vector<ExactColorPixel>& color) {
    std::vector<uint8_t> result(color.size()*3);for(size_t i=0;i<color.size();++i)for(unsigned c=0;c<3;++c)result[i*3+c]=uint8_t(color[i].rgb>>(c*8));return result;
}
struct ShadingPipelines {
    id<MTLComputePipelineState> surface,shadow,prepare,preparedShadow,trace,resolve,init,sample,horizontal,vertical,color;
    explicit ShadingPipelines(Metal& m):surface(m.pipeline(@"exact_surface")),shadow(m.pipeline(@"exact_hard_shadow")),prepare(m.pipeline(@"exact_shadow_prepare")),preparedShadow(m.pipeline(@"exact_hard_shadow_prepared")),trace(m.pipeline(@"exact_shadow_trace")),resolve(m.pipeline(@"exact_hard_shadow_traced")),init(m.pipeline(@"exact_ao_init")),sample(m.pipeline(@"exact_ao_sample")),horizontal(m.pipeline(@"exact_ao_wavelet_h")),vertical(m.pipeline(@"exact_ao_wavelet_v")),color(m.pipeline(@"exact_color")){}
};
#include "shadow_dispatch_tuning.h"
#include "shadow_trace_gpu_controls.h"
static NSDictionary* shadingChecks(Metal& m,const Options& o,const mb3d::metal_exact::PreparedExactScene& prepared) {
    const mb3d::ScopedNumericPrecision precision(mb3d::PrecisionMode::Compatible);
    using namespace mb3d::metal_exact;
    const auto scene=prepared.device;const uint32_t count=scene.width*scene.height;
    double maxShadowCommandMs=0,maxPrepareCommandMs=0,maxTraceCommandMs=0,maxPrimaryCommandMs=0;
    const bool split=o.shadowMode!="legacy",parallel=o.shadowMode=="parallel";
    const auto packing=PrepareSurface(prepared);const auto shadow=PrepareShadow(prepared);
    const auto ao=PrepareExactAO(prepared,o.threads);const auto color=PrepareExactColor(prepared);
    ShadingPipelines p(m);
    NormalStages normals(m,o,count);
    require(o.shadowPrepThreads%p.prepare.threadExecutionWidth==0&&o.shadowPrepThreads<=p.prepare.maxTotalThreadsPerThreadgroup,"shadow prep threadgroup exceeds compiled pipeline limits or is not a SIMD multiple");
    require(o.shadowTraceThreads%p.trace.threadExecutionWidth==0&&o.shadowTraceThreads<=p.trace.maxTotalThreadsPerThreadgroup,"shadow trace group exceeds compiled pipeline limits or is not a SIMD multiple");
    auto abi=m.buffer(10*sizeof(uint32_t));m.run(m.pipeline(@"exact_shading_abi"),abi,nil,&scene,sizeof(scene),1);
    const uint32_t expectedABI[]={sizeof(ExactSurface),sizeof(ExactSurfaceParams),sizeof(ExactShadowParams),sizeof(ExactAOScene),sizeof(ExactAOLevel),sizeof(ExactColorScene),sizeof(ExactColorPixel),sizeof(SingleInput),sizeof(ExactShadowRay),sizeof(ExactShadowTrace)};
    require(std::memcmp(abi.contents,expectedABI,sizeof(expectedABI))==0,"shading ABI echo mismatch");
    auto cols=m.buffer(prepared.columns.size()*sizeof(ExactTrig),prepared.columns.data());auto rows=m.buffer(prepared.rows.size()*sizeof(ExactTrig),prepared.rows.data());
    auto pixels=m.buffer(count*sizeof(ExactPixel)),surface=m.buffer(count*sizeof(ExactSurface)),shaded=m.buffer(count*sizeof(ExactSurface));
    auto rays=m.buffer(size_t(count)*sizeof(ExactShadowRay));
    auto traces=m.buffer(size_t(count)*sizeof(ExactShadowTrace));
    auto depth=m.buffer(count*4),temporary=m.buffer(count*4),angles=m.buffer(size_t(count)*64);
    auto atan=m.buffer(ao.atan_terms.size()*8,ao.atan_terms.data());
    auto colorScene=m.buffer(sizeof(color.device),&color.device);
    auto colorCols=m.buffer(color.columns.size()*sizeof(ExactTrig),color.columns.data()),colorRows=m.buffer(color.rows.size()*sizeof(ExactTrig),color.rows.data());
    auto rgb=m.buffer(count*sizeof(ExactColorPixel));
    std::vector<ExactPixel> primary;std::vector<ExactSurface> packed,shadowed,occluded;std::vector<ExactColorPixel> colors;
    auto dispatchPrepare=[&](const ExactScene& s,const ExactShadowParams& sp,id<MTLBuffer> output) {
        double ms=dispatchStage(m,p.prepare,count,[&](id<MTLComputeCommandEncoder> e){
            [e setBuffer:pixels offset:0 atIndex:0];[e setBuffer:surface offset:0 atIndex:1];[e setBuffer:output offset:0 atIndex:2];[e setBytes:&s length:sizeof(s) atIndex:3];[e setBytes:&sp length:sizeof(sp) atIndex:4];[e setBytes:&count length:4 atIndex:5];},1,o.shadowPrepThreads);
        return ms;
    };
    auto dispatchTrace=[&] {
        return dispatchStage(m,p.trace,count,[&](id<MTLComputeCommandEncoder> e){
            [e setBuffer:rays offset:0 atIndex:0];[e setBuffer:traces offset:0 atIndex:1];[e setBytes:&scene length:sizeof(scene) atIndex:2];[e setBytes:&shadow length:sizeof(shadow) atIndex:3];[e setBytes:&count length:4 atIndex:4];},1,o.shadowTraceThreads);
    };
    auto run=[&](bool capture) {
        std::array<double,7> times{};times[0]=normals.run(pixels,cols,rows,scene,maxPrimaryCommandMs,capture);
        if(capture)primary=readBuffer<ExactPixel>(pixels);
        times[1]=dispatchStage(m,p.surface,count,[&](id<MTLComputeCommandEncoder> e){
            [e setBuffer:pixels offset:0 atIndex:0];[e setBuffer:surface offset:0 atIndex:1];[e setBytes:&scene length:sizeof(scene) atIndex:2];[e setBytes:&packing length:sizeof(packing) atIndex:3];[e setBytes:&count length:4 atIndex:4];});
        if(capture)packed=readBuffer<ExactSurface>(surface);
        if(split){times[2]=dispatchPrepare(scene,shadow,rays);maxPrepareCommandMs=std::max(maxPrepareCommandMs,times[2]);}
        if(parallel){times[3]=dispatchTrace();maxTraceCommandMs=std::max(maxTraceCommandMs,times[3]);}
        // Preserve the production row-local open-air shadow optimization.
        for(uint32_t y=0;y<scene.height;y+=o.shadowRows) {
            uint32_t launch[]={y,scene.width,scene.height};
            double commandMs;
            if(parallel)commandMs=dispatchStage(m,p.resolve,std::min(o.shadowRows,scene.height-y),[&](id<MTLComputeCommandEncoder> e){
                [e setBuffer:rays offset:0 atIndex:0];[e setBuffer:traces offset:0 atIndex:1];[e setBuffer:surface offset:0 atIndex:2];[e setBuffer:shaded offset:0 atIndex:3];[e setBytes:&scene length:sizeof(scene) atIndex:4];[e setBytes:&shadow length:sizeof(shadow) atIndex:5];[e setBytes:launch length:sizeof(launch) atIndex:6];});
            else commandMs=dispatchStage(m,split?p.preparedShadow:p.shadow,std::min(o.shadowRows,scene.height-y),[&](id<MTLComputeCommandEncoder> e){
                [e setBuffer:split?rays:pixels offset:0 atIndex:0];[e setBuffer:surface offset:0 atIndex:1];[e setBuffer:shaded offset:0 atIndex:2];[e setBytes:&scene length:sizeof(scene) atIndex:3];[e setBytes:&shadow length:sizeof(shadow) atIndex:4];[e setBytes:launch length:sizeof(launch) atIndex:5];});
            times[4]+=commandMs;maxShadowCommandMs=std::max(maxShadowCommandMs,commandMs);
        }
        if(capture)shadowed=readBuffer<ExactSurface>(shaded);
        times[5]=dispatchStage(m,p.init,count,[&](id<MTLComputeCommandEncoder> e){
            [e setBuffer:shaded offset:0 atIndex:0];[e setBuffer:depth offset:0 atIndex:1];[e setBuffer:angles offset:0 atIndex:2];[e setBytes:&ao.device length:sizeof(ao.device) atIndex:3];[e setBytes:&count length:4 atIndex:4];});
        for(const auto& level:ao.levels) {
            times[5]+=dispatchStage(m,p.sample,count,[&](id<MTLComputeCommandEncoder> e){
                [e setBuffer:shaded offset:0 atIndex:0];[e setBuffer:depth offset:0 atIndex:1];[e setBuffer:angles offset:0 atIndex:2];[e setBuffer:atan offset:0 atIndex:3];[e setBytes:&ao.device length:sizeof(ao.device) atIndex:4];[e setBytes:&level length:sizeof(level) atIndex:5];[e setBytes:&count length:4 atIndex:6];});
            if(level.index<ao.device.num_levels) {
                uint32_t step=1u<<level.index;
                for(unsigned dir=0;dir<2;++dir)times[5]+=dispatchStage(m,dir?p.vertical:p.horizontal,count,[&](id<MTLComputeCommandEncoder> e){
                    [e setBuffer:dir?temporary:depth offset:0 atIndex:0];[e setBuffer:dir?depth:temporary offset:0 atIndex:1];[e setBytes:&ao.device length:sizeof(ao.device) atIndex:2];[e setBytes:&step length:4 atIndex:3];[e setBytes:&count length:4 atIndex:4];});
            }
        }
        if(capture)occluded=readBuffer<ExactSurface>(shaded);
        uint32_t colorLaunch[]={0,scene.width,scene.height};times[6]=dispatchStage(m,p.color,scene.width,[&](id<MTLComputeCommandEncoder> e){
            [e setBuffer:colorScene offset:0 atIndex:0];[e setBuffer:colorCols offset:0 atIndex:1];[e setBuffer:colorRows offset:0 atIndex:2];[e setBuffer:shaded offset:0 atIndex:3];[e setBuffer:rgb offset:0 atIndex:4];[e setBytes:colorLaunch length:sizeof(colorLaunch) atIndex:5];},scene.height);
        if(capture)colors=readBuffer<ExactColorPixel>(rgb);
        return times;
    };
    auto wall=Clock::now();const auto cold=run(true);double firstWall=elapsed(wall);
    // Shared software preparation checks buffer arithmetic/ABI consistency.
    // Independent production CPU shadow and full Render remain separate oracles.
    // Legacy timing remains unchanged; its preparation diagnostic is untimed.
    if(!split)dispatchPrepare(scene,shadow,rays);
    const auto preparedRays=readBuffer<ExactShadowRay>(rays);
    if(!parallel)dispatchTrace();
    const auto gpuTraces=readBuffer<ExactShadowTrace>(traces);
    std::vector<ExactShadowRay> cpuRays(count);uint64_t rayDifferences=0,rayFaults=0,activeRays=0;
    for(uint32_t i=0;i<count;++i){
        cpuRays[i]=prepare_shadow_ray(scene,shadow,primary[i],packed[i],i/scene.width);
        rayDifferences+=std::memcmp(&preparedRays[i],&cpuRays[i],sizeof(ExactShadowRay))!=0;
        rayFaults+=preparedRays[i].fault!=0;activeRays+=preparedRays[i].active!=0;
    }
    auto preparationComparison=@{@"pixels":@(count),@"different_pixels":@(rayDifferences),@"fault_pixels":@(rayFaults),@"active_rays":@(activeRays),@"gate":@"all 96 bytes exact (prepared shadow rays, zero reserved/fault)",@"diagnostic_note":@"Shared software preparation on captured GPU primary/surface inputs; independent production CPU shadow and complete Render are separate gates."};
    saveBuffer(o.output/"gpu-shadow-rays.bin",preparedRays);saveBuffer(o.output/"cpu-shadow-rays.bin",cpuRays);
    std::vector<ExactShadowTrace> cpuTraces(count);uint64_t traceDifferences=0,traceFaults=0,activeTraces=0,terminalFaults=0;
    for(uint32_t i=0;i<count;++i){
        cpuTraces[i]=trace_shadow_ray(scene,shadow,preparedRays[i]);
        traceDifferences+=std::memcmp(&gpuTraces[i],&cpuTraces[i],sizeof(ExactShadowTrace))!=0;
        traceFaults+=gpuTraces[i].fault!=0;activeTraces+=gpuTraces[i].active!=0;terminalFaults+=gpuTraces[i].terminal_fault!=0;
    }
    auto traceComparison=@{@"pixels":@(count),@"different_pixels":@(traceDifferences),@"fault_pixels":@(traceFaults),@"active_rays":@(activeTraces),@"terminal_fault_pixels":@(terminalFaults),@"gate":@"all 64 bytes exact (independent shadow traces, zero record faults/reserved)",@"diagnostic_note":@"Shared software trace on captured GPU rays; independent production CPU shadow and complete Render remain separate gates."};
    saveBuffer(o.output/"gpu-shadow-traces.bin",gpuTraces);saveBuffer(o.output/"cpu-shadow-traces.bin",cpuTraces);
    std::vector<ExactSurface> preparedBaseline=shadowed;
    if(o.shadowMode!="prepared"){
        auto output=m.buffer(size_t(count)*sizeof(ExactSurface));std::memset(output.contents,0xa5,output.length);
        for(uint32_t y=0;y<scene.height;y+=o.shadowRows){uint32_t launch[]={y,scene.width,scene.height};
            dispatchStage(m,p.preparedShadow,std::min(o.shadowRows,scene.height-y),[&](id<MTLComputeCommandEncoder> e){
                [e setBuffer:rays offset:0 atIndex:0];[e setBuffer:surface offset:0 atIndex:1];[e setBuffer:output offset:0 atIndex:2];[e setBytes:&scene length:sizeof(scene) atIndex:3];[e setBytes:&shadow length:sizeof(shadow) atIndex:4];[e setBytes:launch length:sizeof(launch) atIndex:5];});
        }preparedBaseline=readBuffer<ExactSurface>(output);
    }
    auto preparedComparison=compareSurfaces(shadowed,preparedBaseline);
    saveBuffer(o.output/"gpu-prepared-shadow.bin",preparedBaseline);
    auto cpuPrimary=ReferencePrimary(prepared,o.threads);auto primaryComparison=compare(primary,cpuPrimary,scene);
    auto cpuPacked=ReferenceSurface(prepared,o.threads);auto packingComparison=compareSurfaces(packed,cpuPacked);
    auto cpuShadow=ReferenceShadow(prepared,cpuPacked);auto shadowComparison=compareSurfaces(shadowed,cpuShadow);
    auto cpuAO=ReferenceAO(prepared,cpuShadow,o.threads);auto aoComparison=compareSurfaces(occluded,cpuAO);
    auto cpuColor=ReferenceColor(prepared,cpuAO,o.threads);auto colorComparison=compareColors(colors,colorBytes(cpuColor));
    // Complete production pipeline independently starts from the pinned M3P.
    auto runCPU=[&] {
        mb3d::ScopedRenderContext binding(prepared.host->context);auto header=prepared.host->file.header;auto addon=prepared.host->file.addon;
        mb3d::RenderOptions opts;opts.precision=mb3d::PrecisionMode::Compatible;opts.thread_count=o.threads;opts.profile_timing=true;
        auto result=mb3d::Render(header,addon,opts);require(result.success,"production full CPU render failed: "+result.error);return result;
    };
    auto cpu=runCPU();auto finalComparison=compareColors(colors,cpu.image);
    std::vector<ExactSurface> fullSurface;for(const auto& s:cpu.silight)fullSurface.push_back(ExportSurface(s));
    auto fullSurfaceComparison=compareSurfaces(occluded,fullSurface);
    saveBuffer(o.output/"cpu-full-surface.bin",fullSurface);
    saveBuffer(o.output/"gpu-primary.bin",primary);saveBuffer(o.output/"cpu-primary.bin",cpuPrimary);
    for(auto item:{std::pair("packed",std::pair(&packed,&cpuPacked)),std::pair("shadow",std::pair(&shadowed,&cpuShadow)),std::pair("ao",std::pair(&occluded,&cpuAO))}) {
        saveBuffer(o.output/(std::string("gpu-")+item.first+".bin"),*item.second.first);saveBuffer(o.output/(std::string("cpu-")+item.first+".bin"),*item.second.second);
    }
    saveBuffer(o.output/"gpu-color.bin",colors);saveBuffer(o.output/"cpu-color.bin",cpuColor);
    std::vector<uint32_t> gpuRGB(count),cpuRGB(count);for(size_t i=0;i<count;++i){gpuRGB[i]=colors[i].rgb;cpuRGB[i]=cpu.image[i*3]|(uint32_t(cpu.image[i*3+1])<<8)|(uint32_t(cpu.image[i*3+2])<<16);}
    saveImage(o.output/"gpu-final.ppm",scene.width,scene.height,gpuRGB.data());saveImage(o.output/"cpu-final.ppm",scene.width,scene.height,cpuRGB.data());
    std::vector<double> gpuTimes,wallTimes,cpuTimes;std::array<std::vector<double>,7> stageTimes;bool repeatsExact=true,cpuRepeatsExact=true;
    for(unsigned r=0;r<o.repeats;++r) {
        wall=Clock::now();auto t=run(false);double total=0;for(unsigned k=0;k<7;++k){total+=t[k];stageTimes[k].push_back(t[k]);}
        auto repeated=readBuffer<ExactColorPixel>(rgb);wallTimes.push_back(elapsed(wall));gpuTimes.push_back(total);
        repeatsExact&=std::memcmp(repeated.data(),colors.data(),count*sizeof(ExactColorPixel))==0;
        repeatsExact&=std::memcmp(shaded.contents,occluded.data(),count*sizeof(ExactSurface))==0;
        repeatsExact&=std::memcmp(pixels.contents,primary.data(),count*sizeof(ExactPixel))==0;
        if(split)repeatsExact&=std::memcmp(rays.contents,preparedRays.data(),count*sizeof(ExactShadowRay))==0;
        if(parallel)repeatsExact&=std::memcmp(traces.contents,gpuTraces.data(),count*sizeof(ExactShadowTrace))==0;
    }
    // CPU timing runs do no diagnostics or reference probes, and use the same
    // resolution, scene settings and ordinary stages, with the stated workers.
    for(unsigned r=0;r<o.repeats;++r){wall=Clock::now();auto repeated=runCPU();cpuTimes.push_back(elapsed(wall));cpuRepeatsExact&=repeated.image==cpu.image;}
    std::vector<ExactPixel> primaryBaseline;
    auto primaryDispatch=primaryDispatchChecks(m,normals.selected(),scene,cols,rows,pixels,primary,o.primaryRows,o.primaryThreads,o.primarySweep,maxPrimaryCommandMs,primaryBaseline,o.primaryTileWidth,
        [&](id<MTLBuffer> output,const ExactScene& sc){normals.complete(output,sc);});
    saveBuffer(o.output/"gpu-baseline-primary.bin",primaryBaseline);
    auto normalReport=normals.report(scene,cols,rows,primary);
    NSDictionary* trials=nil;
    if(o.shadowSweep)trials=shadowDispatchTrials(m,o,scene,shadow,pixels,surface,rays,preparedRays,shadowed,p.prepare,p.preparedShadow);
    auto array=[](const std::vector<double>& x){NSMutableArray* a=[NSMutableArray array];for(auto v:x)[a addObject:@(v)];return a;};
    NSMutableDictionary* stages=[NSMutableDictionary dictionary];const char* names[]={"primary","surface_packing","shadow_preparation","shadow_traversal","hard_shadows","ambient_occlusion","final_color"};
    for(unsigned i=0;i<7;++i)stages[ns(names[i])]=@{@"first_gpu_ms":@(cold[i]),@"gpu_ms":array(stageTimes[i]),@"median_gpu_ms":@(median(stageTimes[i]))};
    uint64_t changedShadows=0,changedAO=0;for(size_t i=0;i<count;++i){changedShadows+=packed[i].shadow!=shadowed[i].shadow;changedAO+=shadowed[i].amb_shadow!=occluded[i].amb_shadow;}
    // Negative GPU controls never replace the saved successful outputs.
    uint64_t negativeChecks=0,negativeFailures=0;
    auto negPrimary=m.buffer(count*sizeof(ExactPixel)),negSurface=m.buffer(count*sizeof(ExactSurface)),negColor=m.buffer(count*sizeof(ExactColorPixel));
    auto checkAll=[&](id<MTLBuffer> buffer,size_t stride,size_t offset,unsigned mask){
        for(size_t i=0;i<count;++i){uint32_t f;std::memcpy(&f,static_cast<const char*>(buffer.contents)+i*stride+offset,4);++negativeChecks;negativeFailures+=(f&mask)==0;}
    };
    std::memset(negPrimary.contents,0,negPrimary.length);
    auto rejectPrimary=[&](const ExactScene& sc){runScene(m,normals.selected(),negPrimary,cols,rows,sc,o.primaryRows,o.primaryThreads,nullptr,o.primaryTileWidth);normals.complete(negPrimary,sc);};
    auto badScene=scene;badScene.version=scene.version+1;rejectPrimary(badScene);
    checkAll(negPrimary,sizeof(ExactPixel),offsetof(ExactPixel,fault),1);
    std::memset(negPrimary.contents,0,negPrimary.length);
    badScene=scene;badScene.max_steps=1;rejectPrimary(badScene);
    unsigned primaryExhausted=0;for(const auto& q:readBuffer<ExactPixel>(negPrimary))primaryExhausted+=(q.fault&2)!=0;
    ++negativeChecks;negativeFailures+=primaryExhausted==0;
    std::memset(negSurface.contents,0,negSurface.length);
    auto badPacking=packing;badPacking.version=packing.version+1;
    dispatchStage(m,p.surface,count,[&](id<MTLComputeCommandEncoder> e){
        [e setBuffer:pixels offset:0 atIndex:0];[e setBuffer:negSurface offset:0 atIndex:1];[e setBytes:&scene length:sizeof(scene) atIndex:2];[e setBytes:&badPacking length:sizeof(badPacking) atIndex:3];[e setBytes:&count length:4 atIndex:4];});
    checkAll(negSurface,sizeof(ExactSurface),offsetof(ExactSurface,fault),1);
    for(unsigned mode=0;mode<2;++mode) {
        std::memset(negSurface.contents,0,negSurface.length);
        auto badShadow=shadow;badShadow.enabled=1;badShadow.max_steps=mode;
        for(unsigned y=0;y<scene.height;y+=256) {uint32_t launch[]={y,scene.width,scene.height};
            dispatchStage(m,p.shadow,std::min(256u,scene.height-y),[&](id<MTLComputeCommandEncoder> e){
                [e setBuffer:pixels offset:0 atIndex:0];[e setBuffer:surface offset:0 atIndex:1];[e setBuffer:negSurface offset:0 atIndex:2];[e setBytes:&scene length:sizeof(scene) atIndex:3];[e setBytes:&badShadow length:sizeof(badShadow) atIndex:4];[e setBytes:launch length:sizeof(launch) atIndex:5];});
        }
        if(mode==0)checkAll(negSurface,sizeof(ExactSurface),offsetof(ExactSurface,fault),1);
        else {unsigned exhausted=0;for(const auto& q:readBuffer<ExactSurface>(negSurface))exhausted+=(q.fault&2)!=0;++negativeChecks;negativeFailures+=exhausted==0;}
    }
    auto negRays=m.buffer(size_t(count)*sizeof(ExactShadowRay));
    std::memset(negRays.contents,0,negRays.length);
    auto invalidShadow=shadow;invalidShadow.max_steps=0;dispatchPrepare(scene,invalidShadow,negRays);
    checkAll(negRays,sizeof(ExactShadowRay),offsetof(ExactShadowRay,fault),1);
    std::memset(negRays.contents,0,negRays.length);
    badScene=scene;++badScene.width;dispatchPrepare(badScene,shadow,negRays);
    checkAll(negRays,sizeof(ExactShadowRay),offsetof(ExactShadowRay,fault),1);
    auto rejectPrepared=[&](const ExactScene& sc,const ExactShadowParams& sp){
        std::memset(negSurface.contents,0,negSurface.length);
        for(uint32_t y=0;y<scene.height;y+=o.shadowRows){uint32_t launch[]={y,scene.width,scene.height};
            dispatchStage(m,p.preparedShadow,std::min(o.shadowRows,scene.height-y),[&](id<MTLComputeCommandEncoder> e){
                [e setBuffer:negRays offset:0 atIndex:0];[e setBuffer:surface offset:0 atIndex:1];[e setBuffer:negSurface offset:0 atIndex:2];[e setBytes:&sc length:sizeof(sc) atIndex:3];[e setBytes:&sp length:sizeof(sp) atIndex:4];[e setBytes:launch length:sizeof(launch) atIndex:5];});
        }
    };
    for(unsigned control=0;control<4;++control){
        auto invalidRays=preparedRays;auto sc=scene;
        if(control==1)++sc.width;
        for(auto& r:invalidRays){if(control==0)r.version=2;else if(control==2)r.z=0x7ff8000000000000ULL;else if(control==3)r.reserved[0]=1;}
        std::memcpy(negRays.contents,invalidRays.data(),negRays.length);
        rejectPrepared(sc,shadow);checkAll(negSurface,sizeof(ExactSurface),offsetof(ExactSurface,fault),1);
    }
    invalidShadow=shadow;invalidShadow.enabled=1;invalidShadow.max_steps=1;
    std::memset(negRays.contents,0,negRays.length);dispatchPrepare(scene,invalidShadow,negRays);
    rejectPrepared(scene,invalidShadow);
    unsigned preparedExhausted=0;for(const auto& q:readBuffer<ExactSurface>(negSurface))preparedExhausted+=(q.fault&2)!=0;
    ++negativeChecks;negativeFailures+=preparedExhausted==0;
    auto badAO=ao.device;badAO.version=2;std::memset(negSurface.contents,0,negSurface.length);
    dispatchStage(m,p.init,count,[&](id<MTLComputeCommandEncoder> e){
        [e setBuffer:negSurface offset:0 atIndex:0];[e setBuffer:depth offset:0 atIndex:1];[e setBuffer:angles offset:0 atIndex:2];[e setBytes:&badAO length:sizeof(badAO) atIndex:3];[e setBytes:&count length:4 atIndex:4];});
    checkAll(negSurface,sizeof(ExactSurface),offsetof(ExactSurface,fault),8);
    auto badLevel=ao.levels[0];badLevel.index=0;std::memset(negSurface.contents,0,negSurface.length);
    dispatchStage(m,p.sample,count,[&](id<MTLComputeCommandEncoder> e){
        [e setBuffer:negSurface offset:0 atIndex:0];[e setBuffer:depth offset:0 atIndex:1];[e setBuffer:angles offset:0 atIndex:2];[e setBuffer:atan offset:0 atIndex:3];[e setBytes:&ao.device length:sizeof(ao.device) atIndex:4];[e setBytes:&badLevel length:sizeof(badLevel) atIndex:5];[e setBytes:&count length:4 atIndex:6];});
    checkAll(negSurface,sizeof(ExactSurface),offsetof(ExactSurface,fault),8);
    for(unsigned mode=0;mode<2;++mode) {
        std::memset(negColor.contents,0,negColor.length);
        auto badColor=color.device;if(mode==0)badColor.version=2;else ++badColor.width;
        auto constants=m.buffer(sizeof(badColor),&badColor);uint32_t launch[]={0,scene.width,scene.height};
        dispatchStage(m,p.color,scene.width,[&](id<MTLComputeCommandEncoder> e){
            [e setBuffer:constants offset:0 atIndex:0];[e setBuffer:colorCols offset:0 atIndex:1];[e setBuffer:colorRows offset:0 atIndex:2];[e setBuffer:shaded offset:0 atIndex:3];[e setBuffer:negColor offset:0 atIndex:4];[e setBytes:launch length:sizeof(launch) atIndex:5];},scene.height);
        checkAll(negColor,sizeof(ExactColorPixel),offsetof(ExactColorPixel,fault),1);
    }
    auto formulaControls=formulaRejectionChecks(m,normals.selected(),scene,cols,rows,negPrimary,o,
        [&](id<MTLBuffer> buffer,const ExactScene& sc){normals.complete(buffer,sc);});
    auto traceControls=shadowTraceControls(m,p,scene,shadow,rays,traces,surface,negRays,o.shadowRows,o.shadowTraceThreads);
    NSDictionary* extraNegative=traceControls[@"negative"];NSDictionary* skipControls=traceControls[@"skip"];
    negativeChecks+=[extraNegative[@"checks"] unsignedLongLongValue];negativeFailures+=[extraNegative[@"failures"] unsignedLongLongValue];
    bool pass=[formulaControls[@"pass"] boolValue]&&[primaryDispatch[@"pass"] boolValue]&&traceDifferences==0&&traceFaults==0&&[skipControls[@"pass"] boolValue]&&(!trials||[trials[@"pass"] boolValue])&&rayDifferences==0&&rayFaults==0&&[primaryComparison[@"failures"] unsignedLongLongValue]==0&&[primaryComparison[@"different_numeric_bits"] unsignedLongLongValue]==0&&repeatsExact&&cpuRepeatsExact&&negativeFailures==0&&changedAO>0&&(!shadow.enabled||changedShadows>0);
    for(auto d:{preparedComparison,packingComparison,shadowComparison,aoComparison,colorComparison,finalComparison,fullSurfaceComparison})pass&=[d[@"different_pixels"] unsignedLongLongValue]==0&&[d[@"fault_pixels"] unsignedLongLongValue]==0;
    pass&=[normalReport[@"pass"] boolValue];
    auto pipelineLimits=[](id<MTLComputePipelineState> pipeline,unsigned threads,unsigned grid){return @{@"thread_execution_width":@(pipeline.threadExecutionWidth),@"max_total_threads_per_threadgroup":@(pipeline.maxTotalThreadsPerThreadgroup),@"threads_per_threadgroup":@(threads),@"grid_threads":@(grid)};};
    auto selectedShadow=parallel?p.resolve:(split?p.preparedShadow:p.shadow);
    NSMutableDictionary* result=[@{@"pass":@(pass),@"scene_sha256":ns(prepared.source_sha256),@"scene_id":ns(prepared.scene_id),@"capability_id":@(scene.capability_id),@"formula_kind":@(scene.formula_kind),@"formula_name":ns(prepared.formula_name),@"width":@(scene.width),@"height":@(scene.height),@"cpu_reference_threads":@(o.threads),@"hard_shadows_override":@(o.hardShadows),@"hard_shadows_enabled":@(shadow.enabled!=0),
        @"formula_rejections":formulaControls,@"negative_controls":@{@"pass":@(negativeFailures==0),@"checks":@(negativeChecks),@"failures":@(negativeFailures),@"cases":@26},@"primary":primaryComparison,@"primary_dispatch":primaryDispatch,@"shadow_traversal":traceComparison,@"prepared_shadow_equivalence":preparedComparison,@"trace_skip_controls":skipControls,@"shadow_preparation":preparationComparison,@"surface_packing":packingComparison,@"hard_shadows":shadowComparison,@"ambient_occlusion":aoComparison,@"color_stage":colorComparison,@"final_rgb":finalComparison,@"full_cpu_surface":fullSurfaceComparison,
        @"shadow_mode":ns(o.shadowMode),@"shadow_prep_threads_per_threadgroup":@(o.shadowPrepThreads),@"shadow_trace_threads_per_threadgroup":@(o.shadowTraceThreads),@"max_shadow_trace_command_ms":@(maxTraceCommandMs),@"shadow_rows_per_command":@(o.shadowRows),@"max_shadow_prepare_command_ms":@(maxPrepareCommandMs),@"pipeline_limits":@{@"shadow_prepare":pipelineLimits(p.prepare,o.shadowPrepThreads,count),@"shadow_trace":pipelineLimits(p.trace,o.shadowTraceThreads,count),@"shadow_march":pipelineLimits(selectedShadow,selectedShadow.threadExecutionWidth,std::min(o.shadowRows,scene.height))},@"max_hard_shadow_command_ms":@(maxShadowCommandMs),@"shadow_changed_pixels":@(changedShadows),@"ao_changed_pixels":@(changedAO),@"gpu_repeats_bit_exact":@(repeatsExact),@"cpu_repeats_bit_exact":@(cpuRepeatsExact),@"timing_stages":stages,
        @"first_encode_submit_wait_readback_ms":@(firstWall),@"gpu_timestamp_ms":array(gpuTimes),@"gpu_timestamp_median_ms":@(median(gpuTimes)),@"encode_submit_wait_readback_ms":array(wallTimes),@"encode_submit_wait_readback_median_ms":@(median(wallTimes)),@"cpu_full_render_ms":array(cpuTimes),@"cpu_full_render_median_ms":@(median(cpuTimes)),
        @"timing_note":@"Warm GPU pipeline/buffers versus warm production CPU full Render (including its parameter preparation), same settings and stated worker count. File encoding excluded; GPU command time and host submission/readback separately reported. Exact software arithmetic is not a throughput-optimized profile.",
        @"host_work":@"M3P parse/scene constants, O(width+height) scalar camera/paint sin/cos, fixed diffuse and full-domain int16 atan tables, allocation/dispatch/save; independent CPU validation separately",
        @"gpu_work":@"selected formula/DE/raymarch/refinement/normals, surface packing, optional hard shadows, production HiQ AO/wavelets and final production palette/light/fog/RGB",
        @"excluded":@"scenes/formulas outside the pinned catalog, soft shadows, reflections, volume, DOF, Metal device cancellation; not Pascal final-image qualification"} mutableCopy];
    if(trials)result[@"dispatch_trials"]=trials;
    result[@"normal_pipeline"]=normalReport;
    return result;
}
