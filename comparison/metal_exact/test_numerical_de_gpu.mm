#import <Foundation/Foundation.h>
#import <Metal/Metal.h>
#include "numerical_de_abi.h"
#include <algorithm>
#include <array>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <stdexcept>
#include <string>
#include <vector>
#include <sys/sysctl.h>

namespace {
#include "tests/fixtures/radius_de_reference.h"
using namespace mb3d_metal_exact;
static_assert(sizeof(radiusDEReferences) / sizeof(radiusDEReferences[0]) == 2240);

void require(bool success, const char* message) {
    if (!success) throw std::runtime_error(message);
}
bool nan_bits(uint64_t bits) {
    return (bits & UINT64_C(0x7fffffffffffffff)) > UINT64_C(0x7ff0000000000000);
}
bool equal(uint64_t actual, uint64_t expected) {
    // E80/F64 intentionally canonicalize NaNs. This checks that exact
    // canonical encoding, and exact bits for every other value.
    return actual == (nan_bits(expected) ? UINT64_C(0x7ff8000000000000) : expected);
}
} // namespace

int main(int argc, char** argv) {
#if !defined(__APPLE__) || !defined(__aarch64__)
    std::fprintf(stderr,"Numerical DE GPU test requires native Apple ARM64.\n");
    return 2;
#else
    @autoreleasepool {
        try {
            require(argc == 2,"usage: mb3d_tests_metal_numerical_de_gpu <offline.metallib>");
            int translated = 0;
            size_t translatedSize = sizeof(translated);
            (void)sysctlbyname("sysctl.proc_translated",&translated,&translatedSize,nullptr,0);
            require(translated == 0,"GPU test host must be native ARM64");
            std::vector<NumericalDEInput> inputs;
            std::vector<NumericalDEOutput> expected;
            unsigned expectedRawNaNs = 0, expectedDenominatorNaNs = 0, finitePositive = 0;
            for (const auto& row : radiusDEReferences) {
                NumericalDEInput input{};
                for (unsigned j=0;j<6;++j) input.values[j]=row.input[j];
                inputs.push_back(input);
                expected.push_back({row.expected[1],row.expected[2]});
                expectedRawNaNs += nan_bits(row.expected[1]);
                expectedDenominatorNaNs += nan_bits(row.expected[2]);
                const auto positiveFinite=[](uint64_t bits) {
                    return bits > 0 && bits < UINT64_C(0x7ff0000000000000);
                };
                if (positiveFinite(row.input[0]) && positiveFinite(row.expected[2]) &&
                    (row.expected[1]&UINT64_C(0x7fffffffffffffff)) < UINT64_C(0x7ff0000000000000))
                    ++finitePositive;
            }
            require(finitePositive == 617,"frozen finite-positive corpus changed");
            const uint64_t diagnosed[][8] = {
                {0x40F399CF13949B90ULL,0x3FE868151D9DBA71ULL,0x403F1D819BD2E4FFULL,
                 0x405435C55B0AC97AULL,0x3F70624DE0000000ULL,0x3F2F751060000000ULL,
                 0x4075597F4CFD8B6EULL,0x40253BF71D986422ULL},
                {0x40804822751856FBULL,0x3FCED566132F1AA3ULL,0x3FC3EE74CFFC130CULL,
                 0x3FFCB7268AACEE4BULL,0x3F70624DE0000000ULL,0x3F2F751060000000ULL,
                 0x40219C9619109BD8ULL,0x3FF7B054C6FFE053ULL}
            };
            for (const auto& row:diagnosed) {
                NumericalDEInput input{};
                for (unsigned j=0;j<6;++j) input.values[j]=row[j];
                inputs.push_back(input);expected.push_back({row[6],row[7]});
            }

            id<MTLDevice> device=MTLCreateSystemDefaultDevice();
            require(device!=nil,"Metal device unavailable");
            NSError* error=nil;
            NSString* path=[NSString stringWithUTF8String:argv[1]];
            id<MTLLibrary> library=[device newLibraryWithURL:[NSURL fileURLWithPath:path] error:&error];
            require(library!=nil,error?error.localizedDescription.UTF8String:"offline Metal library unavailable");
            id<MTLFunction> function=[library newFunctionWithName:@"exact_numerical_de"];
            require(function!=nil,"exact_numerical_de kernel unavailable");
            if (function.functionConstantsDictionary.count) {
                MTLFunctionConstantValues* constants=[MTLFunctionConstantValues new];
                for (MTLFunctionConstant* constant in function.functionConstantsDictionary.allValues) {
                    require(constant.index==4&&constant.type==MTLDataTypeBool,"unexpected function constant");
                    bool restoring=false;
                    [constants setConstantValue:&restoring type:MTLDataTypeBool atIndex:4];
                }
                function=[library newFunctionWithName:@"exact_numerical_de" constantValues:constants error:&error];
                require(function!=nil,error?error.localizedDescription.UTF8String:"kernel specialization failed");
            }
            id<MTLComputePipelineState> pipeline=[device newComputePipelineStateWithFunction:function error:&error];
            require(pipeline!=nil,error?error.localizedDescription.UTF8String:"pipeline creation failed");
            id<MTLCommandQueue> queue=[device newCommandQueue];
            id<MTLBuffer> src=[device newBufferWithBytes:inputs.data() length:inputs.size()*sizeof(inputs[0]) options:MTLResourceStorageModeShared];
            id<MTLBuffer> dst=[device newBufferWithLength:expected.size()*sizeof(expected[0]) options:MTLResourceStorageModeShared];
            require(queue!=nil&&src!=nil&&dst!=nil,"GPU resource allocation failed");
            const uint32_t count=uint32_t(inputs.size());
            unsigned failures=0,repeatFailures=0;
            std::vector<NumericalDEOutput> first;
            for (unsigned repeat=0;repeat<2;++repeat) {
                std::memset(dst.contents,0xa5,dst.length);
                id<MTLCommandBuffer> command=[queue commandBuffer];
                id<MTLComputeCommandEncoder> encoder=[command computeCommandEncoder];
                require(command!=nil&&encoder!=nil,"GPU encoder unavailable");
                [encoder setComputePipelineState:pipeline];
                [encoder setBuffer:src offset:0 atIndex:0];
                [encoder setBuffer:dst offset:0 atIndex:1];
                [encoder setBytes:&count length:sizeof(count) atIndex:2];
                const NSUInteger width=std::min(pipeline.threadExecutionWidth,pipeline.maxTotalThreadsPerThreadgroup);
                [encoder dispatchThreads:MTLSizeMake(count,1,1) threadsPerThreadgroup:MTLSizeMake(width,1,1)];
                [encoder endEncoding];[command commit];[command waitUntilCompleted];
                require(command.status==MTLCommandBufferStatusCompleted,
                        command.error?command.error.localizedDescription.UTF8String:"GPU dispatch failed");
                const auto* output=static_cast<const NumericalDEOutput*>(dst.contents);
                for (unsigned i=0;i<count;++i) {
                    const uint64_t actual[]={output[i].raw,output[i].denominator};
                    const uint64_t wanted[]={expected[i].raw,expected[i].denominator};
                    for (unsigned lane=0;lane<2;++lane) if (!equal(actual[lane],wanted[lane])) {
                        if (failures++<12) std::fprintf(stderr,"FAIL repeat%u row%u lane%u actual=%016llX expected=%016llX\n",
                            repeat,i,lane,static_cast<unsigned long long>(actual[lane]),static_cast<unsigned long long>(wanted[lane]));
                    }
                    if (repeat && std::memcmp(&output[i],&first[i],sizeof(output[i]))) ++repeatFailures;
                }
                if (!repeat) first.assign(output,output+count);
            }
            NSDictionary* report=@{@"schema":@"mb3d.metal-numerical-de-fixtures.v1",@"status":failures||repeatFailures?@"FAIL":@"PASS",
                @"device":device.name,@"native_arm64":@YES,@"runtime_shader_compilation":@NO,
                @"frozen_rows":@2240,@"finite_positive_rows":@(finitePositive),@"diagnosed_rows":@2,
                @"repeats":@2,@"checks":@(count*2*2),@"failures":@(failures),@"repeat_failures":@(repeatFailures),
                @"expected_raw_nans":@(expectedRawNaNs),@"expected_denominator_nans":@(expectedDenominatorNaNs),
                @"comparison":@"Every non-NaN bit exact; expected NaNs require canonical 0x7ff8000000000000; repeats require all bits exact"};
            NSData* json=[NSJSONSerialization dataWithJSONObject:report options:NSJSONWritingPrettyPrinted|NSJSONWritingSortedKeys error:&error];
            require(json!=nil,"JSON serialization failed");
            std::fwrite(json.bytes,1,json.length,stdout);std::putchar('\n');
            return failures||repeatFailures?1:0;
        } catch(const std::exception& error) {
            std::fprintf(stderr,"Numerical DE GPU test failed: %s\n",error.what());
            return 1;
        }
    }
#endif
}
