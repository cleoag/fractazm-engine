#pragma once
#include "core/extended80.h"
#include "core/extended_small_division.h"
#include "core/extended_product.h"
#include "core/extended_trig_reduction.h"
#include <limits>

#if defined(MB3D_SOFTWARE_EXT80) || defined(MB3D_HAVE_SOFT_EXT80)
namespace mb3d::numeric {
inline Extended80 extendedNaN() { return Extended80::fromBits(0xffff,0xc000000000000000ULL); }
inline Extended80 extendedLn2() { return Extended80::fromBits(0x3ffe,0xb17217f7d1cf79acULL); }
inline Extended80 extendedLog2(Extended80 x) {
    using E=Extended80;
    if (NativePrecisionEnabled()) return E(std::log2(double(x)));
    const E zero=E::fromBits(0,0), one=E::fromBits(0x3fff,0x8000000000000000ULL);
    const E two=E::fromBits(0x4000,0x8000000000000000ULL);
    const E oneAndHalf=E::fromBits(0x3fff,0xc000000000000000ULL);
    if(!(x>zero) || (x.signExponent()&0x7fff)==0x7fff) return extendedNaN();
    const ExtendedSeriesContext arithmetic;
    int exponent=(x.signExponent()&0x7fff)-16383;
    uint64_t sig=x.significand();
    if((x.signExponent()&0x7fff)==0) {
        const int shift=std::countl_zero(sig);
        sig<<=shift; exponent=1-16383-shift;
    }
    E m=E::fromBits(0x3fff,sig);
    if(m>oneAndHalf) {m=extendedDivideSmall(m,2,arithmetic);++exponent;}
    const E t=(m-one)/(m+one),square=t*t;
    E term=t,sum=t;
    for(int k=1;k<32;++k) {
        term=extendedProduct(term,square,arithmetic);
        const E next=sum+extendedDivideSmall(term,2*k+1,arithmetic);
        if(next==sum) break;
        sum=next;
    }
    return E(exponent)+(two*sum)/extendedLn2();
}
inline Extended80 extendedExp2(Extended80 x, bool nearest=true) {
    using E=Extended80;
    if (NativePrecisionEnabled()) return E(std::exp2(double(x)));
    if((x.signExponent()&0x7fff)==0x7fff) return extendedNaN();
    if(x>=E(16384)) return E(std::numeric_limits<double>::infinity());
    if(x<E(-16446)) return E(0);
    const E integer=nearest ? round(x) : trunc(x);
    const int exponent=static_cast<int32_t>(integer);
    const E t=(x-integer)*extendedLn2();
    E term=t,sum=t;
    for(int k=2;k<32;++k) {
        term=term*t/E(k);
        const E next=sum+term;
        if(next==sum) break;
        sum=next;
    }
    E value=E(1)+sum; // F2XM1 followed by FLD1/FADDP
    // Split extreme scaling so both powers of two are normal ext80 values.
    const int first=exponent>16000 ? 16000 : (exponent< -16000 ? -16000 : exponent);
    value*=E::fromBits(static_cast<uint16_t>(16383+first),0x8000000000000000ULL);
    return value*E::fromBits(static_cast<uint16_t>(16383+exponent-first),0x8000000000000000ULL);
}
inline Extended80 extendedLogPower(Extended80 base, Extended80 power) {
    if (NativePrecisionEnabled()) return Extended80(std::pow(double(base), double(power)));
    // ABox's FLDLN2/FYL2X, exponent multiply, FLDL2E sequence.
    const auto natural=extendedLn2()*extendedLog2(base);
    const auto log2e=Extended80::fromBits(0x3fff,0xb8aa3b295c17f0bcULL);
    return extendedExp2((natural*power)*log2e);
}

// Extended arithmetic for selected translated kernels, not an x87 instruction
// emulator. atan reduction and ascending series: DLMF 4.24.3 and 4.45(i).
// The caller owns ExtendedScope. No intermediate binary64 store is introduced.
inline Extended80 extendedAtan2(Extended80 y, Extended80 x) {
    using E = Extended80;
    if (NativePrecisionEnabled()) return E(std::atan2(double(y), double(x)));
    if (x!=x || y!=y) return x+y;
    const bool negativeY=(y.signExponent()&0x8000)!=0;
    const bool negativeX=(x.signExponent()&0x8000)!=0;
    const ExtendedSeriesContext arithmetic;
    const E zero=E::fromBits(0,0), one=E::fromBits(0x3fff,0x8000000000000000ULL);
    const E half=E::fromBits(0x3ffe,0x8000000000000000ULL);
    const E pi=E::fromBits(0x4000,0xc90fdaa22168c235ULL), infinity(std::numeric_limits<double>::infinity());
    E ax=abs(x), ay=abs(y), angle;
    if (ay==zero) angle=negativeX ? pi : zero;
    else if (ax==zero) angle=extendedDivideSmall(pi,2,arithmetic);
    else if (ay==infinity || ax==infinity) {
        angle=ay==ax ? extendedDivideSmall(pi,4,arithmetic)
            : (ay==infinity ? extendedDivideSmall(pi,2,arithmetic) : zero);
        if (negativeX) angle=pi-angle;
    } else {
        const bool invert=ay>ax;
        E t=invert ? ax/ay : ay/ax;
        const bool shift=t>half;
        if (shift) t=(t-one)/(t+one);
        // |t| <= 1/2: the first omitted term after 40 is < 6e-27.
        const E square=-(t*t);
        E term=t, sum=t;
        for(int k=1;k<40;++k) {
            term=extendedProduct(term,square,arithmetic);
            const E next=sum+extendedDivideSmall(term,2*k+1,arithmetic);
            if(next==sum) break;
            sum=next;
        }
        angle=shift ? extendedDivideSmall(pi,4,arithmetic)+sum : sum;
        if(invert) angle=extendedDivideSmall(pi,2,arithmetic)-angle;
        if(negativeX) angle=pi-angle;
    }
    return negativeY ? -angle : angle;
}

// A wider evaluation is needed where a later large inverse power magnifies
// a one-ulp angle error (IQ-bulb old). Only the transcendental is evaluated
// wider; its input and returned result, and the caller's arithmetic, are ext80.
// SoftFloat binary128 is native integer arithmetic, already in the ARM library.
inline Extended80 extendedAtan2Wide(Extended80 y,Extended80 x) {
    using E=Extended80;
    if (NativePrecisionEnabled()) return E(std::atan2(double(y), double(x)));
    if(x==E(0) || y==E(0) || (x.signExponent()&0x7fff)==0x7fff ||
       (y.signExponent()&0x7fff)==0x7fff) return extendedAtan2(y,x);
    const auto wide=[](E v) {
        extFloat80_t raw{}; raw.signExp=v.signExponent(); raw.signif=v.significand();
        return extF80_to_f128(raw);
    };
    const auto integer=[](int v) { return i32_to_f128(v); };
    const auto negate=[](float128_t v) { v.v[1]^=UINT64_C(0x8000000000000000); return v; };
    const float128_t pi={{UINT64_C(0x8469898cc51701b8),UINT64_C(0x4000921fb54442d1)}};
    const bool inverted=abs(y)>abs(x);
    auto t=inverted ? f128_div(wide(abs(x)),wide(abs(y))) : f128_div(wide(abs(y)),wide(abs(x)));
    const bool shifted=f128_lt(integer(1),f128_mul(t,integer(2)));
    if(shifted) t=f128_div(f128_sub(t,integer(1)),f128_add(t,integer(1)));
    const auto square=negate(f128_mul(t,t));
    auto term=t,sum=t;
    // |t| <= 1/2: 64 terms leave < 5e-41, below binary128 precision.
    for(int k=1;k<64;++k) {
        term=f128_mul(term,square);
        const auto next=f128_add(sum,f128_div(term,integer(2*k+1)));
        if(f128_eq(next,sum)) break;
        sum=next;
    }
    auto angle=shifted ? f128_add(f128_div(pi,integer(4)),sum) : sum;
    if(inverted) angle=f128_sub(f128_div(pi,integer(2)),angle);
    if(x.signExponent()&0x8000) angle=f128_sub(pi,angle);
    if(y.signExponent()&0x8000) angle=negate(angle);
    const auto result=f128_to_extF80(angle);
    return E::fromBits(result.signExp,result.signif);
}

struct ExtendedSinCos { Extended80 sin,cos; };
inline ExtendedSinCos extendedSinCos(Extended80 x) {
    using E=Extended80;
    if (NativePrecisionEnabled()) {
        const double value = double(x);
        return {E(std::sin(value)), E(std::cos(value))};
    }
    const bool negative=(x.signExponent()&0x8000)!=0;
    const int exponent=(x.signExponent()&0x7fff)-16383;
    if(exponent==16384) return {extendedNaN(),extendedNaN()};
    // FSINCOS leaves its operand on the stack at |x| >= 2^63. These
    // selected kernels then store it as cos and pop an empty stack as sin.
    if(exponent>=63) return {extendedNaN(),x};
    E r=abs(x);
    unsigned quadrant=0;
    if(exponent>=-4) {
        const auto reduced=reduceExtendedTrig(x,exponent);
        r=reduced.residual;quadrant=reduced.quadrant;
    }
    const E square=-(r*r);
    E st=r,ct(1),s=r,c(1);
    for(int k=1;k<16;++k) {
        st=st*square/E((2*k)*(2*k+1));
        ct=ct*square/E((2*k-1)*(2*k));
        s+=st;c+=ct;
    }
    E sn,cs;
    switch(quadrant) {
    case 0:sn=s;cs=c;break;
    case 1:sn=c;cs=-s;break;
    case 2:sn=-s;cs=-c;break;
    default:sn=-c;cs=s;break;
    }
    return {negative ? -sn : sn,cs};
}
}
#endif
