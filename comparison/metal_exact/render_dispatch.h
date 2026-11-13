#pragma once
// Shared render-only kernel sequence used by the benchmark and main renderer.
// Host constants/tables are prepared independently; no CPU pixels enter here.
#include "metal_device.h"
#include "normal_abi.h"
#include "shadow_trace_abi.h"
#include "shadow_host.h"
#include "ao_host.h"
#include "color_host.h"
namespace mb3d::metal_exact::gpu {
struct RenderBenchmarkPipelines {
    id<MTLComputePipelineState> primary=nil,probes=nil,finish=nil,surface=nil;
    id<MTLComputePipelineState> prepare=nil,trace=nil,shadow=nil;
    id<MTLComputePipelineState> init=nil,sample=nil,horizontal=nil,vertical=nil,color=nil;
    NSMutableArray* names=[NSMutableArray array];

    template<class Options>
    RenderBenchmarkPipelines(Metal& m,const Options& o,bool shadows,bool ambient=true) {
        auto create=[&](NSString* name) { [names addObject:name];return m.pipeline(name); };
        primary=create(o.normalMode=="parallel"?@"exact_primary_geometry":@"exact_primary");
        if(o.normalMode=="parallel") {
            probes=create(@"exact_normal_probes");finish=create(@"exact_normal_finish");
            for(auto p:{probes,finish})
                require(p.threadExecutionWidth>0&&o.normalThreads%p.threadExecutionWidth==0&&
                        o.normalThreads<=p.maxTotalThreadsPerThreadgroup,"invalid benchmark normal group");
        }
        surface=create(@"exact_surface");
        if(shadows) {
            if(o.shadowMode!="legacy") {
                prepare=create(@"exact_shadow_prepare");
                require(prepare.threadExecutionWidth>0&&o.shadowPrepThreads%prepare.threadExecutionWidth==0&&
                        o.shadowPrepThreads<=prepare.maxTotalThreadsPerThreadgroup,"invalid benchmark shadow preparation group");
            }
            if(o.shadowMode=="parallel") {
                trace=create(@"exact_shadow_trace");
                require(trace.threadExecutionWidth>0&&o.shadowTraceThreads%trace.threadExecutionWidth==0&&
                        o.shadowTraceThreads<=trace.maxTotalThreadsPerThreadgroup,"invalid benchmark shadow trace group");
            }
            shadow=create(o.shadowMode=="parallel"?@"exact_hard_shadow_traced":
                          o.shadowMode=="prepared"?@"exact_hard_shadow_prepared":@"exact_hard_shadow");
        }
        if(ambient) {
            init=create(@"exact_ao_init");sample=create(@"exact_ao_sample");
            horizontal=create(@"exact_ao_wavelet_h");vertical=create(@"exact_ao_wavelet_v");
        }
        color=create(@"exact_color");
    }
};

struct RenderStageBuffers {
    id<MTLBuffer> cols=nil;
    id<MTLBuffer> rows=nil;
    id<MTLBuffer> pixels=nil;
    id<MTLBuffer> surface=nil;
    id<MTLBuffer> shaded=nil;
    id<MTLBuffer> normalSamples=nil;
    id<MTLBuffer> rays=nil;
    id<MTLBuffer> traces=nil;
    id<MTLBuffer> depth=nil;
    id<MTLBuffer> temporary=nil;
    id<MTLBuffer> angles=nil;
    id<MTLBuffer> atan=nil;
    id<MTLBuffer> colorScene=nil;
    id<MTLBuffer> colorCols=nil;
    id<MTLBuffer> colorRows=nil;
    id<MTLBuffer> rgb=nil;

};
struct RenderStageMetrics {
    std::array<double,8> gpu{},wall{};
    std::array<double,3> normalGPU{};
};
template<class Options>
RenderStageMetrics RunRenderStages(Metal& m,const Options& o,const ExactScene& scene,
                                  const ExactSurfaceParams& packing,const ExactShadowParams& shadow,
                                  const PreparedExactAO& ao,const RenderBenchmarkPipelines& p,
                                  const RenderStageBuffers& buffers,
                                  double& maxPrimaryCommandMs,double& maxShadowCommandMs,
                                  const std::function<void(unsigned,bool)>& observe={},bool ambient=true,
                                  const std::function<void()>& afterPrimary={}) {
    const uint32_t count=scene.width*scene.height;
    const bool shadows=shadow.enabled!=0;
    const bool split=shadows&&o.shadowMode!="legacy",parallel=shadows&&o.shadowMode=="parallel";
    const bool parallelNormals=o.normalMode=="parallel";
    std::array<double,8> gpu{},wall{};
    std::array<double,3> normalGPU{};
    auto cols=buffers.cols;
    auto rows=buffers.rows;
    auto pixels=buffers.pixels;
    auto surface=buffers.surface;
    auto shaded=buffers.shaded;
    auto normalSamples=buffers.normalSamples;
    auto rays=buffers.rays;
    auto traces=buffers.traces;
    auto depth=buffers.depth;
    auto temporary=buffers.temporary;
    auto angles=buffers.angles;
    auto atan=buffers.atan;
    auto colorScene=buffers.colorScene;
    auto colorCols=buffers.colorCols;
    auto colorRows=buffers.colorRows;
    auto rgb=buffers.rgb;
        if(observe)observe(0,true);
        auto stageStart=Clock::now();
        normalGPU[0]=runScene(m,p.primary,pixels,cols,rows,scene,o.primaryRows,
                             o.primaryThreads,&maxPrimaryCommandMs,o.primaryTileWidth);
        if(parallelNormals) {
            normalGPU[1]=dispatchStage(m,p.probes,count*6,[&](id<MTLComputeCommandEncoder> e) {
                [e setBuffer:pixels offset:0 atIndex:0];[e setBuffer:normalSamples offset:0 atIndex:1];
                [e setBytes:&scene length:sizeof(scene) atIndex:2];[e setBytes:&count length:4 atIndex:3];
            },1,o.normalThreads);
            normalGPU[2]=dispatchStage(m,p.finish,count,[&](id<MTLComputeCommandEncoder> e) {
                [e setBuffer:pixels offset:0 atIndex:0];[e setBuffer:normalSamples offset:0 atIndex:1];
                [e setBytes:&count length:4 atIndex:2];[e setBytes:&scene length:sizeof(scene) atIndex:3];
            },1,o.normalThreads);
        }
        gpu[0]=normalGPU[0]+normalGPU[1]+normalGPU[2];wall[0]=elapsed(stageStart);
        if(afterPrimary)afterPrimary();
        if(observe)observe(0,false);
        if(observe)observe(1,true);
        stageStart=Clock::now();
        gpu[1]=dispatchStage(m,p.surface,count,[&](id<MTLComputeCommandEncoder> e) {
            [e setBuffer:pixels offset:0 atIndex:0];[e setBuffer:surface offset:0 atIndex:1];
            [e setBytes:&scene length:sizeof(scene) atIndex:2];[e setBytes:&packing length:sizeof(packing) atIndex:3];
            [e setBytes:&count length:4 atIndex:4];
        });wall[1]=elapsed(stageStart);
        if(observe)observe(1,false);
        if(split) {
            if(observe)observe(2,true);
            stageStart=Clock::now();
            gpu[2]=dispatchStage(m,p.prepare,count,[&](id<MTLComputeCommandEncoder> e) {
                [e setBuffer:pixels offset:0 atIndex:0];[e setBuffer:surface offset:0 atIndex:1];
                [e setBuffer:rays offset:0 atIndex:2];[e setBytes:&scene length:sizeof(scene) atIndex:3];
                [e setBytes:&shadow length:sizeof(shadow) atIndex:4];[e setBytes:&count length:4 atIndex:5];
            },1,o.shadowPrepThreads);wall[2]=elapsed(stageStart);
            if(observe)observe(2,false);
        }
        if(parallel) {
            if(observe)observe(3,true);
            stageStart=Clock::now();
            gpu[3]=dispatchStage(m,p.trace,count,[&](id<MTLComputeCommandEncoder> e) {
                [e setBuffer:rays offset:0 atIndex:0];[e setBuffer:traces offset:0 atIndex:1];
                [e setBytes:&scene length:sizeof(scene) atIndex:2];[e setBytes:&shadow length:sizeof(shadow) atIndex:3];
                [e setBytes:&count length:4 atIndex:4];
            },1,o.shadowTraceThreads);wall[3]=elapsed(stageStart);
            if(observe)observe(3,false);
        }
        if(shadows) {
            if(observe)observe(4,true);
            stageStart=Clock::now();
            for(uint32_t y=0;y<scene.height;y+=o.shadowRows) {
                uint32_t launch[]={y,scene.width,scene.height};double commandMs;
                if(parallel) commandMs=dispatchStage(m,p.shadow,std::min(o.shadowRows,scene.height-y),[&](id<MTLComputeCommandEncoder> e) {
                    [e setBuffer:rays offset:0 atIndex:0];[e setBuffer:traces offset:0 atIndex:1];
                    [e setBuffer:surface offset:0 atIndex:2];[e setBuffer:shaded offset:0 atIndex:3];
                    [e setBytes:&scene length:sizeof(scene) atIndex:4];[e setBytes:&shadow length:sizeof(shadow) atIndex:5];
                    [e setBytes:launch length:sizeof(launch) atIndex:6];
                });
                else commandMs=dispatchStage(m,p.shadow,std::min(o.shadowRows,scene.height-y),[&](id<MTLComputeCommandEncoder> e) {
                    [e setBuffer:split?rays:pixels offset:0 atIndex:0];[e setBuffer:surface offset:0 atIndex:1];
                    [e setBuffer:shaded offset:0 atIndex:2];[e setBytes:&scene length:sizeof(scene) atIndex:3];
                    [e setBytes:&shadow length:sizeof(shadow) atIndex:4];[e setBytes:launch length:sizeof(launch) atIndex:5];
                });
                gpu[4]+=commandMs;maxShadowCommandMs=std::max(maxShadowCommandMs,commandMs);
            }
            wall[4]=elapsed(stageStart);
            if(observe)observe(4,false);
        } else {
            // Disabled shadows only require a surface copy. Do not create or
            // dispatch a formula/shadow kernel to perform this transfer.
            if(observe)observe(7,true);
            stageStart=Clock::now();auto command=[m.queue commandBuffer];auto copy=[command blitCommandEncoder];
            require(command&&copy,"cannot encode benchmark surface copy");
            [copy copyFromBuffer:surface sourceOffset:0 toBuffer:shaded destinationOffset:0 size:surface.length];
            [copy endEncoding];[command commit];[command waitUntilCompleted];
            require(command.status==MTLCommandBufferStatusCompleted,"benchmark surface copy failed");
            gpu[7]=(command.GPUEndTime-command.GPUStartTime)*1000;
            require(command.GPUStartTime>0&&std::isfinite(gpu[7])&&gpu[7]>=0,"surface copy timestamp missing");
            wall[7]=elapsed(stageStart);
            if(observe)observe(7,false);
        }
        if(ambient) {
        if(observe)observe(5,true);
        stageStart=Clock::now();
        gpu[5]=dispatchStage(m,p.init,count,[&](id<MTLComputeCommandEncoder> e) {
            [e setBuffer:shaded offset:0 atIndex:0];[e setBuffer:depth offset:0 atIndex:1];
            [e setBuffer:angles offset:0 atIndex:2];[e setBytes:&ao.device length:sizeof(ao.device) atIndex:3];
            [e setBytes:&count length:4 atIndex:4];
        });
        for(const auto& level:ao.levels) {
            gpu[5]+=dispatchStage(m,p.sample,count,[&](id<MTLComputeCommandEncoder> e) {
                [e setBuffer:shaded offset:0 atIndex:0];[e setBuffer:depth offset:0 atIndex:1];
                [e setBuffer:angles offset:0 atIndex:2];[e setBuffer:atan offset:0 atIndex:3];
                [e setBytes:&ao.device length:sizeof(ao.device) atIndex:4];[e setBytes:&level length:sizeof(level) atIndex:5];
                [e setBytes:&count length:4 atIndex:6];
            });
            if(level.index<ao.device.num_levels) {
                uint32_t step=1u<<level.index;
                for(unsigned dir=0;dir<2;++dir) gpu[5]+=dispatchStage(m,dir?p.vertical:p.horizontal,count,[&](id<MTLComputeCommandEncoder> e) {
                    [e setBuffer:dir?temporary:depth offset:0 atIndex:0];[e setBuffer:dir?depth:temporary offset:0 atIndex:1];
                    [e setBytes:&ao.device length:sizeof(ao.device) atIndex:2];[e setBytes:&step length:4 atIndex:3];
                    [e setBytes:&count length:4 atIndex:4];
                });
            }
        }
        wall[5]=elapsed(stageStart);
        if(observe)observe(5,false);
        }
        if(observe)observe(6,true);
        stageStart=Clock::now();
        uint32_t colorLaunch[]={0,scene.width,scene.height};
        gpu[6]=dispatchStage(m,p.color,scene.width,[&](id<MTLComputeCommandEncoder> e) {
            [e setBuffer:colorScene offset:0 atIndex:0];[e setBuffer:colorCols offset:0 atIndex:1];
            [e setBuffer:colorRows offset:0 atIndex:2];[e setBuffer:shaded offset:0 atIndex:3];
            [e setBuffer:rgb offset:0 atIndex:4];[e setBytes:colorLaunch length:sizeof(colorLaunch) atIndex:5];
        },scene.height);wall[6]=elapsed(stageStart);
        if(observe)observe(6,false);
    return {gpu,wall,normalGPU};
}
} // namespace mb3d::metal_exact::gpu
