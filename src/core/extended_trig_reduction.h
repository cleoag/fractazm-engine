#pragma once
#include "core/extended80.h"

#if defined(MB3D_SOFTWARE_EXT80) || defined(MB3D_HAVE_SOFT_EXT80)
namespace mb3d::numeric {

struct ExtendedTrigReduction { Extended80 residual; unsigned quadrant; };

// Integer division by Intel's internal Pi = floor(pi*2^66)/2^66. The
// remainder needs at most 69 bits before subtraction, so two 64-bit limbs
// preserve the same reduction on MSVC and ARM without a compiler int128 type.
inline ExtendedTrigReduction reduceExtendedTrig(Extended80 x, int exponent) {
    struct Wide { uint64_t high, low; };
    constexpr Wide pi{0xc, UINT64_C(0x90fdaa22168c234c)};
    constexpr Wide halfPi{pi.high >> 1, (pi.high << 63) | (pi.low >> 1)};
    const auto less=[](Wide a, Wide b) {
        return a.high < b.high || (a.high == b.high && a.low < b.low);
    };
    const auto subtract=[](Wide a, Wide b) {
        return Wide{a.high - b.high - (a.low < b.low), a.low - b.low};
    };
    Wide remainder{0,x.significand()};
    unsigned quadrant=0;
    for(int i=0;i<exponent+4;++i) {
        remainder={(remainder.high<<1)|(remainder.low>>63),remainder.low<<1};
        quadrant=(quadrant<<1)&3;
        if(!less(remainder,pi)) {remainder=subtract(remainder,pi);quadrant|=1;}
    }
    const bool reflect=less(halfPi,remainder);
    if(reflect) {remainder=subtract(pi,remainder);quadrant=(quadrant+1)&3;}
    Extended80 reduced(0);
    if(remainder.high || remainder.low) {
        const int bits=remainder.high ? 128-std::countl_zero(remainder.high)
                                      : 64-std::countl_zero(remainder.low);
        const int shift=bits-64;
        uint64_t significand;
        int resultExponent=bits-1-67;
        if(shift<=0) significand=remainder.low<<(-shift);
        else {
            significand=(remainder.high<<(64-shift))|(remainder.low>>shift);
            const uint64_t tail=remainder.low&((UINT64_C(1)<<shift)-1);
            const uint64_t half=UINT64_C(1)<<(shift-1);
            if(tail>half || (tail==half && (significand&1))) {
                if(++significand==0) {significand=UINT64_C(0x8000000000000000);++resultExponent;}
            }
        }
        reduced=Extended80::fromBits(uint16_t(16383+resultExponent),significand);
    }
    return {reflect ? -reduced : reduced,quadrant};
}

} // namespace mb3d::numeric
#endif
