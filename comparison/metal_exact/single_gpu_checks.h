#pragma once
#include "single_test_cases.h"
// Include after the existing native Metal helper. CPU hardware binary32 is an
// independent oracle; no expected values come from soft_single.h.
static NSDictionary* singleArithmeticChecks(Metal& metal) {
    const auto input=mb3d_single_test::cases(20000);
    uint32_t count=uint32_t(input.size());
    auto src=metal.buffer(input.size()*sizeof(SingleInput),input.data());
    auto dst=metal.buffer(input.size()*sizeof(SingleOutput));
    std::memset(dst.contents,0xa5,input.size()*sizeof(SingleOutput));
    double gpuMs=metal.run(metal.pipeline(@"exact_single"),src,dst,&count,sizeof(count),count);
    const auto* actual=static_cast<const SingleOutput*>(dst.contents);
    uint64_t checks=0,failures=0,nanClasses=0,perOp[12]{},badOp[12]{};
    NSMutableArray* examples=[NSMutableArray array];
    for(size_t index=0;index<input.size();++index) {
        const auto p=input[index];const float expected=mb3d_single_test::native_value(p);
        const unsigned bad=mb3d_single_test::failure_count(actual[index],expected);
        checks+=5;perOp[p.op]+=5;failures+=bad;badOp[p.op]+=bad;nanClasses+=std::isnan(expected);
        if(bad&&examples.count<12)[examples addObject:@{@"index":@(index),@"operation":@(p.op),
            @"input_a":@(p.a),@"input_b":@(p.b),@"actual_bits":@(actual[index].bits),
            @"expected_bits":@(std::bit_cast<uint32_t>(expected)),@"failed_checks":@(bad)}];
    }
    NSMutableArray* operations=[NSMutableArray array];
    for(unsigned op=0;op<12;++op)[operations addObject:@{@"operation":@(op),@"checks":@(perOp[op]),@"failures":@(badOp[op])}];
    return @{@"rows":@(input.size()),@"checks":@(checks),@"failures":@(failures),@"nan_class_rows":@(nanClasses),
        @"profile":@"binary32 nearest-even with gradual underflow and signed zero; NaN class only",
        @"oracle":@"native Apple ARM64 hardware binary32; independent arithmetic and float stores, contraction disabled",
        @"operations":operations,@"gpu_timestamp_ms":@(gpuMs),@"failure_examples":examples};
}
