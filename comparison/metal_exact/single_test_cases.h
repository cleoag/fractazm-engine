#pragma once
// Test inputs only. Native binary32, not the device implementation, supplies all
// expected arithmetic values. Reused by the native and actual GPU checks.
#include "single_abi.h"
#include <bit>
#include <cmath>
#include <cstdint>
#include <vector>

namespace mb3d_single_test {
inline std::vector<SingleInput> cases(unsigned random_pairs) {
    constexpr uint32_t edge[] = {0,0x80000000u,1,2,3,0x007ffffeu,0x007fffffu,
        0x00800000u,0x00800001u,0x3effffffu,0x3f000000u,0x3f000001u,
        0x3f7fffffu,0x3f800000u,0x3f800001u,0x3fffffffu,0x40000000u,
        0x7f7fffffu,0x7f800000u,0xff800000u,0x7fc00000u,0x7f800001u,
        0xffa12345u,0x33800000u,0x33800001u,0x337fffffu,0x4effffffu,0xcf000000u};
    std::vector<SingleInput> out;
    auto pair = [&](uint32_t a,uint32_t b) {
        for (unsigned op = 0; op < 12; ++op) out.push_back({a,b,op,0});
    };
    for (uint32_t a : edge) for (uint32_t b : edge) { pair(a,b); pair(a^0x80000000u,b); }
    uint64_t state = 0xf34c019e572a6bd8ULL;
    auto random = [&]() { state^=state>>12;state^=state<<25;state^=state>>27;return uint32_t(state*2685821657736338717ULL); };
    for (unsigned i = 0; i < random_pairs; ++i) pair(random(),random());
    for (unsigned exponent = 1; exponent < 255; ++exponent) {
        for (unsigned i = 0; i < 8; ++i) {
            uint32_t a = (exponent<<23)|(random()&0x007fffffu);
            pair(a,a+(random()&7u));
            pair(a,(a+(random()&7u))^0x80000000u);
            pair(a,((exponent>24 ? exponent-24 : 0)<<23)|(random()&0x007fffffu));
        }
    }
    // Every subnormal power-of-two divisor and all finite normal exponents.
    // These force gradual-underflow ties and values adjacent to exponent limits.
    for (int exponent = -149; exponent <= 127; ++exponent) {
        uint32_t divisor = exponent >= -126 ? uint32_t(exponent+127)<<23 : 1u<<(exponent+149);
        for (uint32_t numerator : edge)
            for (unsigned signs = 0; signs < 4; ++signs)
                out.push_back({numerator|((signs&1u)<<31),divisor|((signs>>1)<<31),3,0});
    }
    return out;
}

inline float native_value(const SingleInput input) {
    volatile float a = std::bit_cast<float>(input.a), b = std::bit_cast<float>(input.b);
    switch (input.op) {
        case 0:return a+b; case 1:return a-b; case 2:return a*b; case 3:return a/b;
        case 4:return std::sqrt(a); case 5:return -a; case 6:return std::fabs(a);
        case 7:return b<a ? b : a; case 8:return a<b ? b : a;
        case 9:return a<b ? 1.0f : 0.0f; case 10:return a==b ? 1.0f : 0.0f;
        case 11:return a<=b ? 1.0f : 0.0f;
        default:return std::bit_cast<float>(0x7fc00000u);
    }
}
inline int32_t native_integer(float value,bool nearest) {
    const double rounded = nearest ? std::nearbyint(double(value)) : std::trunc(double(value));
    return std::isfinite(rounded) && rounded>=-2147483648.0 && rounded<=2147483647.0 ?
        static_cast<int32_t>(rounded) : (-2147483647-1);
}
inline unsigned failure_count(SingleOutput actual,float expected) {
    const bool expected_nan = std::isnan(expected);
    const bool bits_good = expected_nan ? (actual.bits&0x7fffffffu)>0x7f800000u :
                          actual.bits==std::bit_cast<uint32_t>(expected);
    const bool double_good = expected_nan ? (actual.double_bits&0x7fffffffffffffffULL)>0x7ff0000000000000ULL :
        actual.double_bits==std::bit_cast<uint64_t>(double(expected));
    return unsigned(!bits_good)+unsigned(!double_good)+unsigned(actual.integer_rtz!=native_integer(expected,false))+
        unsigned(actual.integer_rne!=native_integer(expected,true))+unsigned(actual.fault!=0 || actual.reserved!=0);
}
} // namespace mb3d_single_test
