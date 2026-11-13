#pragma once
#include "normal_abi.h"
#include "normal_gpu_checks.h"

// Host scheduling only: positions, six probes and normal packing stay on GPU.
// Every pass consumes the allocation-derived count, independent of scene ABI.
struct NormalStages {
    Metal& metal;
    const Options& options;
    id<MTLComputePipelineState> original, geometry, probes, finish;
    id<MTLBuffer> samples;
    uint32_t count;
    std::array<double,3> first{};
    std::array<std::vector<double>,3> timings;

    NormalStages(Metal& m,const Options& o,uint32_t n)
      :metal(m),options(o),original(m.pipeline(@"exact_primary")),
       geometry(m.pipeline(@"exact_primary_geometry")),
       probes(m.pipeline(@"exact_normal_probes")),
       finish(m.pipeline(@"exact_normal_finish")),
       samples(m.buffer(size_t(n)*sizeof(ExactNormalSamples))),count(n) {
        for(auto pipeline:{probes,finish})
            require(pipeline.threadExecutionWidth>0&&
                    o.normalThreads%pipeline.threadExecutionWidth==0&&
                    o.normalThreads<=pipeline.maxTotalThreadsPerThreadgroup,
                    "normal threadgroup must be a legal SIMD-width multiple");
    }
    bool parallel() const {return options.normalMode=="parallel";}
    id<MTLComputePipelineState> selected() const {return parallel()?geometry:original;}
    double dispatch(id<MTLComputePipelineState> pipeline,id<MTLBuffer> pixels,
                    const ExactScene& scene,bool probe) {
        require(pixels.length==size_t(count)*sizeof(ExactPixel),
                "normal pixel allocation mismatch");
        auto c=[metal.queue commandBuffer];auto e=[c computeCommandEncoder];
        require(c&&e,"cannot encode normal stage");
        [e setComputePipelineState:pipeline];
        [e setBuffer:pixels offset:0 atIndex:0];
        [e setBuffer:samples offset:0 atIndex:1];
        if(probe) {
            [e setBytes:&scene length:sizeof(scene) atIndex:2];
            [e setBytes:&count length:sizeof(count) atIndex:3];
        } else {
            [e setBytes:&count length:sizeof(count) atIndex:2];
            [e setBytes:&scene length:sizeof(scene) atIndex:3];
        }
        [e dispatchThreads:MTLSizeMake(probe?count*6:count,1,1)
           threadsPerThreadgroup:MTLSizeMake(options.normalThreads,1,1)];
        [e endEncoding];[c commit];[c waitUntilCompleted];
        require(c.status==MTLCommandBufferStatusCompleted,
                c.error?c.error.description.UTF8String:"GPU normal stage failed");
        double ms=(c.GPUEndTime-c.GPUStartTime)*1000;
        require(c.GPUStartTime>0&&std::isfinite(ms)&&ms>0,
                "GPU normal timestamp unavailable");
        return ms;
    }
    void complete(id<MTLBuffer> pixels,const ExactScene& scene) {
        if(parallel()) {
            dispatch(probes,pixels,scene,true);
            dispatch(finish,pixels,scene,false);
        }
    }
    double run(id<MTLBuffer> pixels,id<MTLBuffer> cols,id<MTLBuffer> rows,
               const ExactScene& scene,double& maximum,bool cold) {
        std::array<double,3> t{};
        t[0]=runScene(metal,selected(),pixels,cols,rows,scene,options.primaryRows,
                      options.primaryThreads,&maximum,options.primaryTileWidth);
        if(parallel()) {
            t[1]=dispatch(probes,pixels,scene,true);
            t[2]=dispatch(finish,pixels,scene,false);
        }
        if(cold) first=t;
        else for(unsigned i=0;i<3;++i)timings[i].push_back(t[i]);
        return t[0]+t[1]+t[2];
    }
    NSDictionary* report(const ExactScene& scene,id<MTLBuffer> cols,id<MTLBuffer> rows,
                         const std::vector<ExactPixel>& expected) {
        auto baseline=metal.buffer(size_t(count)*sizeof(ExactPixel));
        std::memset(baseline.contents,0xa5,baseline.length);
        runScene(metal,original,baseline,cols,rows,scene,options.primaryRows,
                 options.primaryThreads,nullptr,options.primaryTileWidth);
        uint64_t differences=0,faults=0;
        checkPrimaryDispatchRecords(baseline,expected,differences,faults);
        const auto* actual=static_cast<const ExactPixel*>(baseline.contents);
        for(size_t i=0;i<expected.size();++i)
            faults+=expected[i].fault!=0&&actual[i].fault==0;
        std::ofstream file(options.output/"gpu-inline-primary.bin",std::ios::binary);
        file.write(static_cast<const char*>(baseline.contents),baseline.length);
        require(bool(file),"cannot save inline normal baseline");
        auto controls=normalGPUControls(metal,probes,finish,scene,options.normalThreads);
        auto limits=[](id<MTLComputePipelineState> p) {
            return @ {@"thread_execution_width":@(p.threadExecutionWidth),
                @"max_total_threads_per_threadgroup":@(p.maxTotalThreadsPerThreadgroup)};
        };
        NSMutableDictionary *cold=[NSMutableDictionary dictionary],
            *medians=[NSMutableDictionary dictionary],*runs=[NSMutableDictionary dictionary];
        const char* names[]{"geometry","probes","finish"};
        for(unsigned i=0;i<3;++i) {
            require(timings[i].size()==options.repeats,"incomplete normal timings");
            NSMutableArray* values=[NSMutableArray array];
            for(double value:timings[i])[values addObject:@(value)];
            cold[ns(names[i])]=@(first[i]);medians[ns(names[i])]=@(median(timings[i]));
            runs[ns(names[i])]=values;
        }
        return @ {@"pass":@(differences==0&&faults==0&&[controls[@"pass"] boolValue]),
            @"mode":ns(options.normalMode),@"pixels":@(count),
            @"scratch_record_bytes":@(sizeof(ExactNormalSamples)),
            @"probe_jobs":@(parallel()?uint64_t(count)*6:0),
            @"threads_per_threadgroup":@(options.normalThreads),
            @"baseline":@ {@"pixels":@(count),@"different_pixels":@(differences),
                @"fault_pixels":@(faults),
                @"gate":@"all 176 bytes exact (inline GPU normal baseline)"},
            @"gpu_ms":runs,@"median_gpu_ms":medians,@"first_gpu_ms":cold,
            @"pipeline_limits":@ {@"probes":limits(probes),@"finish":limits(finish)},
            @"controls":controls};
    }
};
