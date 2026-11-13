#pragma once
#include "log_fixture.h"
static NSDictionary* nativeLogChecks(Metal& metal) {
    const auto input=mb3d_metal_exact::log_inputs();
    const uint32_t count=uint32_t(input.size());
    auto src=metal.buffer(input.size()*sizeof(mb3d_metal_exact::LogInput),input.data());
    auto dst=metal.buffer(input.size()*sizeof(uint64_t));
    std::memset(dst.contents,0xa5,dst.length);
    const double ms=metal.run(metal.pipeline(@"exact_native_log"),src,dst,&count,sizeof(count),count);
    const auto* output=static_cast<const uint64_t*>(dst.contents);
    uint64_t failures=0,logs=0,fmas=0;
    NSMutableArray* examples=[NSMutableArray array];
    for(size_t i=0;i<input.size();++i) {
        input[i].op?++logs:++fmas;
        const auto expected=mb3d_metal_exact::native_log_expected(input[i]);
        if(!mb3d_metal_exact::log_result_equal(mb3d_metal_exact::F64::from_bits(output[i]),expected)) {
            ++failures;
            if(examples.count<12)[examples addObject:@{@"index":@(i),@"operation":@(input[i].op),
                @"actual_bits":@(output[i]),@"expected_bits":@(expected.bits)}];
        }
    }
    return @{@"checks":@(count),@"log_checks":@(logs),@"fma_checks":@(fmas),
        @"failures":@(failures),@"gpu_timestamp_ms":@(ms),@"failure_examples":examples,
        @"oracle":@"native Apple ARM64 std::log and std::fma; every non-NaN bit exact",
        @"profile":@"Apple ARM64 libm log compatibility; fused binary64 nearest-even"};
}
