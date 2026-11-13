// Optional native Metal feasibility experiment. It does not select a renderer
// backend. Compare the real CPU final-surface path (scales 4..10) byte for byte.
#import <Foundation/Foundation.h>
#import <Metal/Metal.h>
#include "core/image_process.h"
#include <algorithm>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <vector>

static NSString* source = @R"MSL(
#include <metal_stdlib>
using namespace metal;
kernel void box_average(device const uchar* input [[buffer(0)]],
                        device uchar* output [[buffer(1)]],
                        constant uint4& shape [[buffer(2)]],
                        uint2 pos [[thread_position_in_grid]]) {
    const uint width=shape.x, height=shape.y, scale=shape.z, crop=shape.w;
    const uint ow=width/scale, oh=height/scale;
    if (pos.x>=ow-2*crop || pos.y>=oh-2*crop) return;
    uint3 sum(0);
    for (uint y=0;y<scale;++y)
        for (uint x=0;x<scale;++x) {
            uint at=(((pos.y+crop)*scale+y)*width+(pos.x+crop)*scale+x)*3;
            sum+=uint3(input[at],input[at+1],input[at+2]);
        }
    uint div=scale*scale;
    uint3 rgb=(sum+div/2)/div;
    uint at=(pos.y*(ow-2*crop)+pos.x)*3;
    output[at]=uchar(rgb.x); output[at+1]=uchar(rgb.y); output[at+2]=uchar(rgb.z);
}
)MSL";

using Clock = std::chrono::steady_clock;
static double millis(Clock::time_point start) {
    return std::chrono::duration<double,std::milli>(Clock::now()-start).count();
}
static double median(std::vector<double> samples) {
    std::sort(samples.begin(),samples.end()); return samples[samples.size()/2];
}
int main() {
    @autoreleasepool {
        id<MTLDevice> device=MTLCreateSystemDefaultDevice();
        if (!device) { std::fprintf(stderr,"No Metal device\n"); return 2; }
        NSError* error=nil;
        MTLCompileOptions* options=[MTLCompileOptions new];
        if (@available(macOS 15.0, *)) options.mathMode=MTLMathModeSafe;
        id<MTLLibrary> library=[device newLibraryWithSource:source options:options error:&error];
        if (!library) { std::fprintf(stderr,"%s\n",error.description.UTF8String); return 2; }
        id<MTLComputePipelineState> pipeline=[device newComputePipelineStateWithFunction:
            [library newFunctionWithName:@"box_average"] error:&error];
        if (!pipeline) { std::fprintf(stderr,"%s\n",error.description.UTF8String); return 2; }
        id<MTLCommandQueue> queue=[device newCommandQueue];
        if (!queue) return 2;
        // Query this installed compiler rather than inferring FP64 from GPU name.
        error=nil;
        id<MTLLibrary> fp64=[device newLibraryWithSource:
            @"#include <metal_stdlib>\nusing namespace metal;\nkernel void fp64(device double* x [[buffer(0)]], uint i [[thread_position_in_grid]]) { x[i]=x[i]*2.0; }"
            options:options error:&error];
        if (!fp64) std::fprintf(stderr,"FP64 compiler diagnostic: %s\n",error.description.UTF8String);
        NSMutableArray* cases=[NSMutableArray array];
        unsigned failures=0;
        uint64_t checkedBytes=0;
        for (unsigned scale=4;scale<=10;++scale) {
            for (unsigned pattern=0;pattern<4;++pattern) {
                // Odd dimensions also verify discarded right/bottom remainders.
                unsigned width=127*scale+3, height=79*scale+1;
                unsigned crop=pattern==3 ? 2 : 0;
                std::vector<uint8_t> input(size_t(width)*height*3);
                uint32_t seed=0x12345678;
                for (size_t i=0;i<input.size();++i) {
                    seed=1664525*seed+1013904223;
                    input[i]=pattern==0 ? 255 : pattern==1 ? uint8_t((i/3)%width) :
                             pattern==2 ? uint8_t(((i/3)%width>width/2)*255) : uint8_t(seed>>24);
                }
                mb3d::FinalRGBSurface cpu;
                if (!mb3d::BuildFinalRGBSurface(input.data(),width,height,scale,0,crop*scale,cpu)) return 2;
                id<MTLBuffer> src=[device newBufferWithBytes:input.data() length:input.size() options:MTLResourceStorageModeShared];
                id<MTLBuffer> dst=[device newBufferWithLength:cpu.pixels.size() options:MTLResourceStorageModeShared];
                if (!src || !dst) return 2;
                const uint32_t shape[]={width,height,scale,crop};
                auto dispatch=[&] {
                    id<MTLCommandBuffer> command=[queue commandBuffer];
                    id<MTLComputeCommandEncoder> encoder=[command computeCommandEncoder];
                    [encoder setComputePipelineState:pipeline];
                    [encoder setBuffer:src offset:0 atIndex:0]; [encoder setBuffer:dst offset:0 atIndex:1];
                    [encoder setBytes:shape length:sizeof(shape) atIndex:2];
                    const NSUInteger tw=pipeline.threadExecutionWidth;
                    const NSUInteger th=std::min<NSUInteger>(8,pipeline.maxTotalThreadsPerThreadgroup/tw);
                    [encoder dispatchThreads:MTLSizeMake(cpu.width,cpu.height,1) threadsPerThreadgroup:MTLSizeMake(tw,th,1)];
                    [encoder endEncoding]; [command commit]; [command waitUntilCompleted];
                    if (command.status!=MTLCommandBufferStatusCompleted) {
                        std::fprintf(stderr,"%s\n",command.error.description.UTF8String); return false;
                    }
                    return true;
                };
                if (!dispatch()) return 2;
                const bool equal=std::memcmp(cpu.pixels.data(),dst.contents,cpu.pixels.size())==0;
                if (!equal) ++failures;
                checkedBytes+=cpu.pixels.size();
                std::vector<double> cpuTimes,gpuTimes,endToEndTimes;
                for (int repeat=0;repeat<7;++repeat) {
                    auto start=Clock::now();
                    mb3d::BuildFinalRGBSurface(input.data(),width,height,scale,0,crop*scale,cpu);
                    cpuTimes.push_back(millis(start));
                    start=Clock::now();
                    if (!dispatch()) return 2;
                    gpuTimes.push_back(millis(start));
                    start=Clock::now();
                    src=[device newBufferWithBytes:input.data() length:input.size() options:MTLResourceStorageModeShared];
                    dst=[device newBufferWithLength:cpu.pixels.size() options:MTLResourceStorageModeShared];
                    if (!src || !dst || !dispatch()) return 2;
                    std::vector<uint8_t> copied(cpu.pixels.size());
                    std::memcpy(copied.data(),dst.contents,copied.size());
                    endToEndTimes.push_back(millis(start));
                    if (copied!=cpu.pixels) ++failures;
                }
                [cases addObject:@{@"scale":@(scale),@"pattern":@(pattern),@"crop":@(crop),
                    @"input_width":@(width),@"input_height":@(height),@"bytes":@(cpu.pixels.size()),
                    @"exact_cpu_match":@(equal),@"cpu_median_ms":@(median(cpuTimes)),
                    @"gpu_warm_dispatch_wait_median_ms":@(median(gpuTimes)),
                    @"gpu_allocation_copy_dispatch_wait_median_ms":@(median(endToEndTimes))}];
            }
        }
        NSDictionary* report=@{@"scope":@"Integer final RGB downscale/crop only; no formula or ray tracing acceleration",
            @"device":device.name,@"unified_memory":@(device.hasUnifiedMemory),
            @"fp64_compiles":@(fp64!=nil),@"checked_bytes":@(checkedBytes),@"failures":@(failures),
            @"timing_contract":@"7 repeats after warmup; CPU includes production allocation/copy; GPU reports both preallocated encode/submit/wait and allocation/input+output copy/dispatch/wait. Both exclude shader compilation. Crop is in output pixels. Not end-to-end render speedup.",
            @"cases":cases};
        NSData* json=[NSJSONSerialization dataWithJSONObject:report options:NSJSONWritingPrettyPrinted error:&error];
        if (!json) return 2;
        std::fwrite(json.bytes,1,json.length,stdout); std::putchar('\n');
        return failures ? 1 : 0;
    }
}
