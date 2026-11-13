#pragma once
#include "normal_abi.h"
#include "core/math3d.h"
#include <array>
#include <bit>
#include <cstring>

// Tiny diagnostic-only inputs never enter accepted scene buffers or timings.
// Included after Metal, require(), and the shared normal arithmetic.
static NSDictionary* normalGPUControls(
    Metal& m,id<MTLComputePipelineState> probes,id<MTLComputePipelineState> finish,
    const ExactScene& source,unsigned groupWidth) {
    constexpr uint32_t count=15;
    require(probes!=nil&&finish!=nil&&groupWidth>0&&
            groupWidth%probes.threadExecutionWidth==0&&
            groupWidth%finish.threadExecutionWidth==0&&
            groupWidth<=probes.maxTotalThreadsPerThreadgroup&&
            groupWidth<=finish.maxTotalThreadsPerThreadgroup,
            "invalid normal control pipeline or threadgroup");
    ExactScene scene=source;scene.width=5;scene.height=3;
    require(mb3d_metal_exact::valid_scene(scene),"normal control scene is not valid");
    auto submit=[&](id<MTLComputePipelineState> pipeline,uint32_t jobs,auto encode) {
        auto command=[m.queue commandBuffer];auto encoder=[command computeCommandEncoder];
        require(command!=nil&&encoder!=nil,"cannot encode normal GPU control");
        [encoder setComputePipelineState:pipeline];encode(encoder);
        [encoder dispatchThreads:MTLSizeMake(jobs,1,1)
            threadsPerThreadgroup:MTLSizeMake(groupWidth,1,1)];
        [encoder endEncoding];[command commit];[command waitUntilCompleted];
        require(command.status==MTLCommandBufferStatusCompleted,
                command.error?command.error.description.UTF8String:"normal GPU control failed");
    };
    auto abi=m.buffer(6*sizeof(uint32_t));
    std::memset(abi.contents,0xa5,abi.length);
    submit(m.pipeline(@"exact_normal_abi"),1,[&](id<MTLComputeCommandEncoder> e) {
        [e setBuffer:abi offset:0 atIndex:0];
    });
    const uint32_t expectedABI[]={sizeof(ExactNormalSamples),offsetof(ExactNormalSamples,distance),
        offsetof(ExactNormalSamples,orbit_trap),offsetof(ExactNormalSamples,reserved),6,sizeof(ExactPixel)};
    const auto* gpuABI=static_cast<const uint32_t*>(abi.contents);
    uint64_t abiFailures=0;
    NSMutableArray* abiValues=[NSMutableArray array];
    for(unsigned i=0;i<6;++i) {abiFailures+=gpuABI[i]!=expectedABI[i];[abiValues addObject:@(gpuABI[i])];}

    std::array<ExactPixel,count> input{},expected{};
    uint64_t active=0,inactive=0;
    for(unsigned i=0;i<count;++i) {
        auto& pixel=input[i];
        pixel.rm_result=i%3==0?0:1;
        pixel.start_inside=i%3==1?1:0;
        pixel.position[0]=mb3d_metal_exact::F64(int(i%5)-3).bits;
        pixel.position[1]=mb3d_metal_exact::F64::from_bits(0x3fc0000000000000ULL).bits;
        pixel.position[2]=mb3d_metal_exact::F64::from_bits(0xbfd0000000000000ULL).bits;
        pixel.depth=mb3d_metal_exact::F64(int(i)).bits;
        pixel.final_stop=scene.de_stop;
        pixel.fault=i>=9?4:0;
        pixel.reserved_tail=0x0102030405060708ULL+i;
        // Unrelated fields must survive finish byte-for-byte, including faults.
        for(unsigned axis=0;axis<3;++axis) {
            pixel.ray[axis]=0x3ff0000000000000ULL+i+axis;
            pixel.start[axis]=0xbff0000000000000ULL+i+axis;
        }
        expected[i]=pixel;
        if(pixel.rm_result&&!pixel.start_inside) {
            ++active;
            using namespace mb3d_metal_exact;
            const auto original=raw_normal_with_trap(
                {D(pixel.position[0]),D(pixel.position[1]),D(pixel.position[2])},
                D(pixel.depth),S(pixel.final_stop),scene);
            auto& out=expected[i];
            out.raw_normal[0]=original.normal.x.bits;
            out.raw_normal[1]=original.normal.y.bits;
            out.raw_normal[2]=original.normal.z.bits;
            out.orbit_trap=original.trap.bits;
            TVec3D normal{std::bit_cast<double>(out.raw_normal[0]),
                         std::bit_cast<double>(out.raw_normal[1]),
                         std::bit_cast<double>(out.raw_normal[2])};
            int16_t packed[3];
            mb3d::MakeWNormalsFromDVec(packed,normal);
            if(packed[0]==0&&packed[1]==0)packed[2]=packed[2]>0?32767:-32767;
            for(unsigned axis=0;axis<3;++axis)out.normal[axis]=packed[axis];
            if(!isfinite(original.trap)||!finite_vec(original.normal))out.fault|=4;
        } else ++inactive;
    }
    // One extra physical record is a canary. Overdispatch must not touch it.
    auto pixels=m.buffer((count+1)*sizeof(ExactPixel));
    auto samples=m.buffer((count+1)*sizeof(ExactNormalSamples));
    auto reset=[&] {
        std::memset(pixels.contents,0xa5,pixels.length);
        std::memcpy(pixels.contents,input.data(),sizeof(input));
        std::memset(samples.contents,0xa5,samples.length);
    };
    auto runProbes=[&](const ExactScene& descriptor) {
        submit(probes,count*6+6,[&](id<MTLComputeCommandEncoder> e) {
            [e setBuffer:pixels offset:0 atIndex:0];
            [e setBuffer:samples offset:0 atIndex:1];
            [e setBytes:&descriptor length:sizeof(descriptor) atIndex:2];
            [e setBytes:&count length:sizeof(count) atIndex:3];
        });
    };
    auto runFinish=[&](const ExactScene& descriptor) {
        submit(finish,count+1,[&](id<MTLComputeCommandEncoder> e) {
            [e setBuffer:pixels offset:0 atIndex:0];
            [e setBuffer:samples offset:0 atIndex:1];
            [e setBytes:&count length:sizeof(count) atIndex:2];
            [e setBytes:&descriptor length:sizeof(descriptor) atIndex:3];
        });
    };
    auto guardBad=[&] {
        uint64_t failures=0;
        const auto* pixelBytes=static_cast<const unsigned char*>(pixels.contents);
        const auto* sampleBytes=static_cast<const unsigned char*>(samples.contents);
        for(size_t i=count*sizeof(ExactPixel);i<pixels.length;++i)failures+=pixelBytes[i]!=0xa5;
        for(size_t i=count*sizeof(ExactNormalSamples);i<samples.length;++i)failures+=sampleBytes[i]!=0xa5;
        return failures;
    };
    reset();runProbes(scene);runFinish(scene);
    const auto* actual=static_cast<const ExactPixel*>(pixels.contents);
    const auto* observedSamples=static_cast<const ExactNormalSamples*>(samples.contents);
    const ExactNormalSamples zero{};
    uint64_t differences=0,inactiveSampleFailures=0;
    for(unsigned i=0;i<count;++i) {
        differences+=std::memcmp(&actual[i],&expected[i],sizeof(ExactPixel))!=0;
        if(!input[i].rm_result||input[i].start_inside)
            inactiveSampleFailures+=std::memcmp(&observedSamples[i],&zero,sizeof(zero))!=0;
    }
    uint64_t guardFailures=guardBad(),negativeFailures=0;
    ExactPixel invalid{};invalid.fault=1;
    for(unsigned control=0;control<8;++control) {
        auto bad=scene;
        if(control==0)++bad.width;
        if(control==1)--bad.width;
        if(control==2)++bad.height;
        if(control==3)--bad.height;
        if(control==4)bad.version=0;
        if(control==5)bad.formula_kind=0;
        if(control==6)bad.capability_id=0;
        if(control==7)bad.bulb.power=9;
        reset();runProbes(bad);runFinish(bad);
        for(unsigned i=0;i<count;++i) {
            negativeFailures+=std::memcmp(&observedSamples[i],&zero,sizeof(zero))!=0;
            negativeFailures+=std::memcmp(&actual[i],&invalid,sizeof(invalid))!=0;
        }
        guardFailures+=guardBad();
    }
    const bool syntheticPass=differences==0&&inactiveSampleFailures==0&&guardFailures==0;
    return @{
        @"pass":@(abiFailures==0&&negativeFailures==0&&syntheticPass),
        @"abi":@{@"checks":@6,@"failures":@(abiFailures),@"values":abiValues,@"pass":@(abiFailures==0)},
        @"negative_controls":@{@"cases":@8,@"checks":@(uint64_t(count)*8*2),
            @"failures":@(negativeFailures),@"pass":@(negativeFailures==0)},
        @"synthetic":@{@"pixels":@(count),@"active_pixels":@(active),@"inactive_pixels":@(inactive),
            @"different_pixels":@(differences),@"inactive_sample_failures":@(inactiveSampleFailures),
            @"guard_failures":@(guardFailures),@"pass":@(syntheticPass)},
        @"note":@"Synthetic normal contract controls are separate from accepted GPU frame buffers; original ordered probes and production CPU normal packing supply the expected fields."
    };
}
