#pragma once
#include "soft_log.h"

namespace mb3d_metal_exact {
struct LogInput { U64 a,b,c; U32 op,reserved; };
inline F64 evaluate_log_input(LogInput p) {
    return p.op ? native_log(F64::from_bits(p.a)) :
        fused_multiply_add(F64::from_bits(p.a),F64::from_bits(p.b),F64::from_bits(p.c));
}
#ifndef __METAL_VERSION__
} // namespace mb3d_metal_exact
#include <vector>
namespace mb3d_metal_exact {
inline std::vector<LogInput> log_inputs() {
    std::vector<LogInput> out;
    auto log=[&](U64 a){out.push_back({a,0,0,1,0});};
    auto fma=[&](U64 a,U64 b,U64 c){out.push_back({a,b,c,0,0});};
    const U64 edges[]={0,0x8000000000000000ULL,1,0x8000000000000001ULL,
        0x000fffffffffffffULL,0x0010000000000000ULL,0x3fefffffffffffffULL,
        0x3ff0000000000000ULL,0x3ff0000000000001ULL,0xbff0000000000000ULL,
        0x7fefffffffffffffULL,0xffefffffffffffffULL,0x7ff0000000000000ULL,
        0xfff0000000000000ULL,0x7ff8000000000000ULL,0x7ff0000000000001ULL};
    for(U64 a:edges) {log(a);for(U64 b:edges)for(U64 c:edges)fma(a,b,c);}
    for(U64 e=1;e<2047;++e) {
        for(int delta=-2;delta<=2;++delta) {
            log((e<<52)+delta);
            const U64 a=(e<<52)+delta;
            // Severe cancellation of the exact product against its rounded
            // value, in addition to random triples with large exponent gaps.
            const F64 b=F64::from_bits(0x3ff0000000000001ULL);
            const double product=F64::from_bits(a).to_double()*b.to_double();
            fma(a,b.bits,F64::from_double(-product).bits);
        }
    }
    for(U64 k=0;k<=128;++k) {
        const U64 midpoint=0x3ff0000000000000ULL+(k<<45);
        for(int delta=-8;delta<=8;++delta) {
            log(midpoint+delta);
            log(midpoint+0x100000000000ULL+delta);
            log(midpoint-0x10000000000000ULL+delta);
        }
    }
    for(U64 bit=1;bit<0x0010000000000000ULL;bit<<=1)
        for(int delta=-1;delta<=1;++delta)log(bit+delta);
    U64 seed=0x378b716ff091ea52ULL;
    auto next=[&](){seed^=seed<<13;seed^=seed>>7;seed^=seed<<17;return seed;};
    for(unsigned i=0;i<200000;++i) {
        U64 a=next(),b=next(),c=next();fma(a,b,c);log(a&0x7fffffffffffffffULL);
        if(i<10000) {
            a=(a&0x000fffffffffffffULL)|0x3ff0000000000000ULL;
            b=(b&0x000fffffffffffffULL)|0x3ff0000000000000ULL;
            const double p=F64::from_bits(a).to_double()*F64::from_bits(b).to_double();
            fma(a,b,F64::from_double(-p).bits);
        }
    }
    return out;
}
inline F64 native_log_expected(LogInput p) {
    const double a=F64::from_bits(p.a).to_double();
    const double b=F64::from_bits(p.b).to_double(),c=F64::from_bits(p.c).to_double();
    return F64::from_double(p.op?std::log(a):std::fma(a,b,c));
}
inline bool log_result_equal(F64 actual,F64 expected) {
    return isnan(expected)?isnan(actual):actual.bits==expected.bits;
}
#endif
} // namespace mb3d_metal_exact
