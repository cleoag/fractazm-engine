#pragma once
#include "soft_double.h"

namespace mb3d_metal_exact {
// A 128-bit accumulator holds the exact 106-bit product and 53-bit addend
// whenever cancellation is possible. Distant operands retain a sticky bit;
// rounding happens once, after addition. No native double or uint128 is used.
inline Wide fma_shift_right(Wide a, unsigned n) {
    if (!n) return a;
    if (n < 64) return {a.hi >> n,
        (a.hi << (64-n)) | (a.lo >> n) | ((a.lo << (64-n)) != 0)};
    if (n == 64) return {0, a.hi | (a.lo != 0)};
    return {0, shr_jam(a.hi,n-64) | (a.lo != 0)};
}
inline Wide fma_shift_left(Wide a, unsigned n) {
    if (!n) return a;
    if (n < 64) return {(a.hi << n) | (a.lo >> (64-n)), a.lo << n};
    return {a.lo << (n-64), 0};
}
inline F64 fused_multiply_add(F64 a, F64 b, F64 c) {
    const bool product_sign = signbit(a) != signbit(b);
    if (isnan(a) || isnan(b) || isnan(c)) return quiet_nan();
    if (isinf(a) || isinf(b)) {
        if (iszero(a) || iszero(b) || (isinf(c) && signbit(c)!=product_sign))
            return quiet_nan();
        return infinity(product_sign);
    }
    if (isinf(c)) return c;
    if (iszero(a) || iszero(b)) return zero(product_sign)+c;
    if (iszero(c)) return a*b;
    const Normalized na=unpack_finite_nonzero(a), nb=unpack_finite_nonzero(b),
        nc=unpack_finite_nonzero(c);
    Wide product=multiply_wide(na.sig,nb.sig);
    const bool upper=(product.hi & (U64(1)<<41))!=0;
    int pe=na.exponent+nb.exponent+int(upper), ce=nc.exponent;
    product=fma_shift_left(product,upper?21:22);
    Wide addend={nc.sig<<10,0};
    int exponent=pe;
    if (pe>=ce) addend=fma_shift_right(addend,unsigned(pe-ce));
    else {product=fma_shift_right(product,unsigned(ce-pe));exponent=ce;}
    Wide result;
    bool sign=product_sign;
    if (product_sign==signbit(c)) {
        result.lo=product.lo+addend.lo;
        result.hi=product.hi+addend.hi+(result.lo<product.lo);
    } else {
        if (product.hi<addend.hi || (product.hi==addend.hi && product.lo<addend.lo)) {
            Wide tmp=product;product=addend;addend=tmp;sign=signbit(c);
        }
        result.lo=product.lo-addend.lo;
        result.hi=product.hi-addend.hi-(product.lo<addend.lo);
    }
    if (!(result.hi|result.lo)) return zero(false);
    const int lead=result.hi ? 127-leading_zeroes(result.hi) : 63-leading_zeroes(result.lo);
    exponent+=lead-126;
    U64 sig;
    if (lead>=55) sig=wide_shr_jam(result,unsigned(lead-55));
    else sig=result.lo << (55-lead);
    return round_pack(sign,exponent,sig);
}
} // namespace mb3d_metal_exact
