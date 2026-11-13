#pragma once
#include "shadow_trace_abi.h"
#include "soft_double.h"
#include <algorithm>
#include <array>
#include <cstdint>
#include <cstring>

// Diagnostics only. Copies and fabricated inputs below never enter the accepted
// render, saved successful buffers or complete-render timing measurements.
static NSDictionary* shadowTraceControls(
    Metal& m, ShadingPipelines& p, const ExactScene& scene,
    const ExactShadowParams& shadow, id<MTLBuffer> rays, id<MTLBuffer> traces,
    id<MTLBuffer> surface, id<MTLBuffer> enabledBudgetRays,
    unsigned rows, unsigned traceThreads) {
    using mb3d_metal_exact::F64;
    const uint32_t count=scene.width*scene.height;
    require(scene.width>0&&scene.height>0&&scene.width<=4096&&scene.height<=4096,
            "invalid shadow trace control dimensions");
    require(rows>0&&rows<=4096&&traceThreads>0&&p.trace.threadExecutionWidth>0&&
            traceThreads%p.trace.threadExecutionWidth==0&&
            traceThreads<=p.trace.maxTotalThreadsPerThreadgroup,
            "invalid shadow trace control dispatch");
    require(rays.length==size_t(count)*sizeof(ExactShadowRay)&&
            enabledBudgetRays.length==rays.length&&
            traces.length==size_t(count)*sizeof(ExactShadowTrace)&&
            surface.length==size_t(count)*sizeof(ExactSurface),
            "invalid shadow trace control input buffers");
    auto diagnosticRays=m.buffer(rays.length);
    auto diagnosticTraces=m.buffer(traces.length);
    auto diagnosticSurface=m.buffer(surface.length);
    uint64_t negativeChecks=0,negativeFailures=0;

    auto dispatchTrace=[&](const ExactScene& sc,const ExactShadowParams& sp) {
        std::memset(diagnosticTraces.contents,0,diagnosticTraces.length);
        dispatchStage(m,p.trace,count,[&](id<MTLComputeCommandEncoder> e){
            [e setBuffer:diagnosticRays offset:0 atIndex:0];
            [e setBuffer:diagnosticTraces offset:0 atIndex:1];
            [e setBytes:&sc length:sizeof(sc) atIndex:2];
            [e setBytes:&sp length:sizeof(sp) atIndex:3];
            [e setBytes:&count length:sizeof(count) atIndex:4];
        },1,traceThreads);
    };
    auto dispatchResolve=[&](const ExactScene& sc,const ExactShadowParams& sp,
                              id<MTLBuffer> sourceRays,id<MTLBuffer> sourceTraces,
                              id<MTLBuffer> sourceSurface,id<MTLBuffer> output,
                              uint32_t width,uint32_t height,unsigned batch,
                              unsigned char sentinel=0) {
        std::memset(output.contents,sentinel,output.length);
        for(uint32_t y=0;y<height;y+=batch){
            const uint32_t launch[]{y,width,height};
            dispatchStage(m,p.resolve,std::min(batch,height-y),[&](id<MTLComputeCommandEncoder> e){
                [e setBuffer:sourceRays offset:0 atIndex:0];
                [e setBuffer:sourceTraces offset:0 atIndex:1];
                [e setBuffer:sourceSurface offset:0 atIndex:2];
                [e setBuffer:output offset:0 atIndex:3];
                [e setBytes:&sc length:sizeof(sc) atIndex:4];
                [e setBytes:&sp length:sizeof(sp) atIndex:5];
                [e setBytes:launch length:sizeof(launch) atIndex:6];
            });
        }
    };
    // Invalid dimensions/parameters and two invalid ray fields must reject
    // every record, including inactive background rays.
    for(unsigned control=0;control<4;++control){
        std::memcpy(diagnosticRays.contents,rays.contents,rays.length);
        auto sc=scene;auto sp=shadow;
        if(control==0)++sc.width;
        if(control==1)sp.max_steps=0;
        auto data=static_cast<ExactShadowRay*>(diagnosticRays.contents);
        for(uint32_t i=0;i<count;++i){
            if(control==2)data[i].version=2;
            if(control==3)data[i].z=0x7ff8000000000000ULL;
        }
        dispatchTrace(sc,sp);
        for(const auto& value:readBuffer<ExactShadowTrace>(diagnosticTraces)){
            ++negativeChecks;negativeFailures+=(value.fault&1)==0;
        }
    }

    // The caller prepares this separate diagnostic input on GPU with shadows
    // enabled, even when the accepted scene disables the shadow pass.
    std::memcpy(diagnosticRays.contents,enabledBudgetRays.contents,rays.length);
    auto exhausted=shadow;exhausted.enabled=1;exhausted.max_steps=1;
    dispatchTrace(scene,exhausted);
    uint64_t exhaustedRays=0;
    for(const auto& value:readBuffer<ExactShadowTrace>(diagnosticTraces))
        exhaustedRays+=(value.terminal_fault&2)!=0;
    ++negativeChecks;negativeFailures+=exhaustedRays==0;

    for(unsigned control=0;control<5;++control){
        std::memcpy(diagnosticTraces.contents,traces.contents,traces.length);
        auto sc=scene;
        if(control==4)++sc.width;
        auto data=static_cast<ExactShadowTrace*>(diagnosticTraces.contents);
        for(uint32_t i=0;i<count;++i){
            if(control==0)data[i].version=2;
            if(control==1)data[i].reserved[0]=1;
            if(control==2)data[i].min_remaining=0x7ff8000000000000ULL;
            if(control==3)data[i].final_threshold=2;
        }
        dispatchResolve(sc,shadow,rays,diagnosticTraces,surface,diagnosticSurface,
                        scene.width,scene.height,rows);
        for(const auto& value:readBuffer<ExactSurface>(diagnosticSurface)){
            ++negativeChecks;negativeFailures+=(value.fault&1)==0;
        }
    }
    require(negativeChecks==uint64_t(9)*count+1,"shadow trace control denominator mismatch");

    // Pixel zero seeds reachable row history; pixel one then observes an exact
    // subtraction of one. These hand-authored fixtures distinguish loop-entry
    // minimum from terminal remaining, and strict greater-than from equality.
    auto fixtureScene=scene;fixtureScene.width=2;fixtureScene.height=1;
    auto fixtureShadow=shadow;fixtureShadow.enabled=1;
    std::array<ExactShadowRay,2> fixtureRays{};
    for(auto& value:fixtureRays){
        value.version=1;value.active=1;value.length=F64(10).bits;
        value.threshold=F64(100).bits;value.initial_stop=0x3f800000U;
    }
    const std::array<ExactSurface,2> fixtureSurface{};
    auto fixtureRayBuffer=m.buffer(sizeof(fixtureRays),fixtureRays.data());
    auto fixtureTraceBuffer=m.buffer(2*sizeof(ExactShadowTrace));
    auto fixtureSurfaceBuffer=m.buffer(sizeof(fixtureSurface),fixtureSurface.data());
    auto fixtureOutput=m.buffer(sizeof(fixtureSurface));
    const unsigned seeds[]{7,6,7,6,5,6};
    const unsigned terminalFaults[]{2,2,4,4,0,0};
    uint64_t skipChecks=0,skipFailures=0;
    for(unsigned control=0;control<6;++control){
        std::array<ExactShadowTrace,2> fixtureTraces{};
        for(auto& value:fixtureTraces){
            value.version=1;value.active=1;value.has_loop=1;
            value.min_remaining=F64(5).bits;
        }
        fixtureTraces[0].final_remaining=F64(-1).bits;
        fixtureTraces[0].last_found=F64(seeds[control]).bits;
        fixtureTraces[0].writes_last_found=1;fixtureTraces[0].final_threshold=3;
        fixtureTraces[1].terminal_fault=terminalFaults[control];
        fixtureTraces[1].final_remaining=F64(terminalFaults[control]?0:3).bits;
        std::memcpy(fixtureTraceBuffer.contents,fixtureTraces.data(),sizeof(fixtureTraces));
        // Expected clear surfaces are all zero, so poison these outputs to
        // catch a missing write as well as an incorrect state transition.
        dispatchResolve(fixtureScene,fixtureShadow,fixtureRayBuffer,fixtureTraceBuffer,
                        fixtureSurfaceBuffer,fixtureOutput,2,1,1,0xa5);
        std::array<ExactSurface,2> expected{};
        if(control==1)expected[1].fault=2;
        if(control==3)expected[1].fault=4;
        if(control>=4)expected[1].shadow=1024;
        const auto actual=readBuffer<ExactSurface>(fixtureOutput);
        for(unsigned i=0;i<2;++i){
            ++skipChecks;
            skipFailures+=std::memcmp(&actual[i],&expected[i],sizeof(ExactSurface))!=0;
        }
    }
    return @{
        @"negative":@{@"cases":@10,@"checks":@(negativeChecks),
                       @"failures":@(negativeFailures),@"pass":@(negativeFailures==0)},
        @"skip":@{@"cases":@6,@"checks":@(skipChecks),
                   @"failures":@(skipFailures),@"pass":@(skipFailures==0)}
    };
}
