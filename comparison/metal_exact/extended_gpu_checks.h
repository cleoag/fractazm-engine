#pragma once
#include "extended_abi.h"
#include "src/core/extended80.h"

// Include in the native ObjC++ driver after its Metal helper. The independent
// oracle is the pinned upstream SoftFloat used by the native CPU renderer.
static NSDictionary* extendedArithmeticChecks(Metal& m) {
    using namespace mb3d_metal_exact;
    using mb3d::numeric::Extended80;
    mb3d::numeric::ExtendedScope scope;
    const E80 edges[]={E80::from_bits(0,0),E80::from_bits(0x8000,0),
      E80::from_bits(0,1),E80::from_bits(0,2),E80::from_bits(0,0x7fffffffffffffffULL),
      E80::from_bits(1,0x8000000000000000ULL),E80::from_bits(1,0x8000000000000001ULL),
      E80::from_bits(0x3fff,0x8000000000000000ULL),E80::from_bits(0x3fff,0x8000000000000001ULL),
      E80::from_bits(0x7ffe,0xffffffffffffffffULL),ext_inf(false),ext_inf(true),ext_nan(),
      E80::from_bits(0x3ffe,0xffffffffffffffffULL)};
    std::vector<ExtendedInput> input;
    auto append=[&](E80 a,E80 b,U64 c){for(U32 op=0;op<7;++op)input.push_back({a.significand,b.significand,c,a.sign_exponent,b.sign_exponent,op,0});};
    for(E80 a:edges)for(E80 b:edges){append(a,b,0x3ff0000000000000ULL);append(-a,b,0xbfe0000000000000ULL);}
    U64 state=0x6857c9be30d21fa4ULL;
    auto random=[&](){state^=state>>12;state^=state<<25;state^=state>>27;return state*2685821657736338717ULL;};
    for(unsigned i=0;i<10000;++i){
      U32 ae=U32(random()%0x7fff),be=U32(random()%0x7fff);U64 as=random(),bs=random();
      as=ae?as|0x8000000000000000ULL:as&0x7fffffffffffffffULL;
      bs=be?bs|0x8000000000000000ULL:bs&0x7fffffffffffffffULL;
      append(E80::from_bits(ae|(U32(random()&1)<<15),as),E80::from_bits(be|(U32(random()&1)<<15),bs),random());
    }
    // Correlated finite scene-scale cases exercise cancellation and tiny steps.
    for(unsigned i=0;i<10000;++i){
      U64 a=(U64(1000+random()%48)<<52)|(random()&0x000fffffffffffffULL);
      U64 b=(U64(980+random()%64)<<52)|(random()&0x000fffffffffffffULL);
      append(E80(F64::from_bits(a)),E80(F64::from_bits(b)),a^0x8000000000000000ULL);
    }
    uint32_t count=uint32_t(input.size());auto src=m.buffer(input.size()*sizeof(input[0]),input.data());
    auto dst=m.buffer(input.size()*sizeof(ExtendedOutput));std::memset(dst.contents,0xa5,input.size()*sizeof(ExtendedOutput));
    double gpuMs=m.run(m.pipeline(@"exact_extended"),src,dst,&count,sizeof(count),count);
    auto actual=static_cast<const ExtendedOutput*>(dst.contents);
    uint64_t failures=0,checks=0,nanClasses=0,opChecks[7]={},opFailures[7]={};NSMutableArray* examples=[NSMutableArray array];
    for(size_t i=0;i<input.size();++i){
      const auto& p=input[i];Extended80 a=Extended80::fromBits(p.a_exp,p.a_sig),b=Extended80::fromBits(p.b_exp,p.b_sig),value;
      double c=std::bit_cast<double>(p.c_f64);
      switch(p.op){case 0:value=a+b;break;case 1:value=a-b;break;case 2:value=a*b;break;case 3:value=a/b;break;
        case 4:value=sqrt(a);break;
        case 5:value=Extended80(static_cast<double>(a*b+Extended80(c)));break;
        case 6:value=Extended80(double(static_cast<float>((a*b+Extended80(1.0))*Extended80(c))));break;
      }
      bool nan=(value.signExponent()&0x7fff)==0x7fff&&(value.significand()&0x7fffffffffffffffULL);
      const auto& z=actual[i];bool raw=nan?((z.sign_exponent&0x7fff)==0x7fff&&(z.significand&0x7fffffffffffffffULL)):
        (z.sign_exponent==value.signExponent()&&z.significand==value.significand());
      double d=static_cast<double>(value);float f=static_cast<float>(value);
      bool doubleGood=std::isnan(d)?((z.double_bits&0x7fffffffffffffffULL)>0x7ff0000000000000ULL):z.double_bits==std::bit_cast<U64>(d);
      bool singleGood=std::isnan(f)?((z.single_bits&0x7fffffffU)>0x7f800000U):z.single_bits==std::bit_cast<U32>(f);
      bool nearestGood=z.integer_rne==value.integer(),truncateGood=z.integer_rtz==static_cast<int32_t>(value);
      unsigned bad=unsigned(!raw)+unsigned(!doubleGood)+unsigned(!singleGood)+unsigned(!nearestGood)+unsigned(!truncateGood);
      checks+=5;opChecks[p.op]+=5;failures+=bad;opFailures[p.op]+=bad;nanClasses+=nan;
      if(bad&&examples.count<12)[examples addObject:@{@"index":@(i),@"op":@(p.op),@"failed_values":@(bad),
        @"actual_exponent":@(z.sign_exponent),@"expected_exponent":@(value.signExponent()),
        @"actual_significand":@(z.significand),@"expected_significand":@(value.significand())}];
    }
    NSMutableArray* perOp=[NSMutableArray array];for(unsigned op=0;op<7;++op)[perOp addObject:@{@"operation":@(op),@"checks":@(opChecks[op]),@"failures":@(opFailures[op])}];
    return @{@"rows":@(input.size()),@"checks":@(checks),@"failures":@(failures),@"nan_class_rows":@(nanClasses),
      @"profile":@"canonical x87 extended values; 64-bit significand, nearest-even, gradual underflow; no NaN payload/flags or unsupported encodings",
      @"oracle":@"pinned upstream Berkeley SoftFloat 3e native ARM64 integer implementation",
      @"operations":perOp,@"gpu_timestamp_ms":@(gpuMs),@"failure_examples":examples};
}
