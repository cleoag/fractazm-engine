#pragma once
#ifdef MB3D_METAL_NATIVE_FLOAT
#include "native_arithmetic.h"
#else

// Integer-backed IEEE binary64, round-to-nearest, ties-to-even. A hardware
// binary32 reciprocal/sqrt and residual correction supply only integer hints; exact
// integer verification and restoring steps determine every output bit
// independently of those approximations and the host floating environment.
// All arithmetic NaNs are canonical quiet NaNs; payload/sign and exception
// flags are deliberately outside this value-only profile. Gradual underflow,
// signed zero and infinities are retained. This is not x87 extended arithmetic.
// See arithmetic_provenance.json for scope, source attribution and verification.
#ifdef __METAL_VERSION__
#include <metal_stdlib>
#define MB3D_EXACT_THREAD thread
#else
#include <cmath>
#include <cstdint>
#include <cstring>
#define MB3D_EXACT_THREAD
#endif

namespace mb3d_metal_exact {
#ifdef __METAL_VERSION__
using U64 = unsigned long;
#else
using U64 = unsigned long long;
#endif
using U32 = unsigned int;

inline U64 shr_jam(U64 value, unsigned distance) {
    if (!distance) return value;
    if (distance >= 64) return value != 0;
    return (value >> distance) | ((value << (64 - distance)) != 0);
}
inline int leading_zeroes(U64 value) {
#ifdef __METAL_VERSION__
    return int(metal::clz(value));
#else
    return value ? __builtin_clzll(value) : 64;
#endif
}
struct Wide { U64 hi, lo; };
// Four 32x32 partial products, with explicit carries; no uint128 extension.
inline Wide multiply_wide(U64 a, U64 b) {
    U64 a0 = U32(a), a1 = a >> 32, b0 = U32(b), b1 = b >> 32;
    U64 p00 = a0 * b0, p01 = a0 * b1, p10 = a1 * b0, p11 = a1 * b1;
    U64 middle = (p00 >> 32) + U32(p01) + U32(p10);
    return {p11 + (p01 >> 32) + (p10 >> 32) + (middle >> 32),
            (middle << 32) | U32(p00)};
}
inline U64 wide_shr_jam(Wide v, unsigned distance) {
    if (!distance) return v.lo;
    if (distance < 64)
        return (v.hi << (64 - distance)) | (v.lo >> distance) |
               ((v.lo << (64 - distance)) != 0);
    if (distance == 64) return v.hi | (v.lo != 0);
    return shr_jam(v.hi, distance - 64) | (v.lo != 0);
}

struct F64 {
    U64 bits;
    F64() : bits(0) {}
    explicit F64(int value);
    explicit F64(unsigned value);
    static F64 from_bits(U64 value) { F64 r; r.bits = value; return r; }
    static F64 from_i32(int value);
    static F64 from_u32(unsigned value);
    static F64 from_f32_bits(U32 value);
    U64 to_bits() const { return bits; }
    U32 to_f32_bits() const;
    int to_i32_rtz() const;
#ifndef __METAL_VERSION__
    static F64 from_double(double value) {
        U64 result; std::memcpy(&result, &value, sizeof(result)); return from_bits(result);
    }
    double to_double() const {
        double result; std::memcpy(&result, &bits, sizeof(result)); return result;
    }
#endif
};
inline bool signbit(F64 x) { return (x.bits >> 63) != 0; }
inline bool isnan(F64 x) { return (x.bits & 0x7fffffffffffffffULL) > 0x7ff0000000000000ULL; }
inline bool isinf(F64 x) { return (x.bits & 0x7fffffffffffffffULL) == 0x7ff0000000000000ULL; }
inline bool isfinite(F64 x) { return (x.bits & 0x7ff0000000000000ULL) != 0x7ff0000000000000ULL; }
inline bool iszero(F64 x) { return (x.bits & 0x7fffffffffffffffULL) == 0; }
inline F64 quiet_nan() { return F64::from_bits(0x7ff8000000000000ULL); }
inline F64 infinity(bool sign) { return F64::from_bits((U64(sign) << 63) | 0x7ff0000000000000ULL); }
inline F64 zero(bool sign) { return F64::from_bits(U64(sign) << 63); }
inline F64 abs(F64 x) { return F64::from_bits(x.bits & 0x7fffffffffffffffULL); }
inline F64 operator-(F64 x) { return F64::from_bits(x.bits ^ 0x8000000000000000ULL); }
inline bool operator==(F64 a, F64 b) {
    return !isnan(a) && !isnan(b) && (a.bits == b.bits || (iszero(a) && iszero(b)));
}
inline bool operator!=(F64 a, F64 b) { return !(a == b); }
inline bool operator<(F64 a, F64 b) {
    if (isnan(a) || isnan(b) || (iszero(a) && iszero(b))) return false;
    if (signbit(a) != signbit(b)) return signbit(a);
    return signbit(a) ? a.bits > b.bits : a.bits < b.bits;
}
inline bool operator>(F64 a, F64 b) { return b < a; }
inline bool operator<=(F64 a, F64 b) { return (a < b) || (a == b); }
inline bool operator>=(F64 a, F64 b) { return (b < a) || (a == b); }
// Deliberately follows the conditional form, including unordered/tie selection.
inline F64 min(F64 a, F64 b) { return b < a ? b : a; }
inline F64 max(F64 a, F64 b) { return a < b ? b : a; }

struct Normalized { U64 sig; int exponent; };
inline Normalized unpack_finite_nonzero(F64 a) {
    U64 sig = a.bits & 0x000fffffffffffffULL;
    int exponent = int((a.bits >> 52) & 2047);
    if (exponent) return {sig | 0x0010000000000000ULL, exponent - 1023};
    int shift = leading_zeroes(sig) - 11;
    return {sig << shift, -1022 - shift};
}
// sig has its leading bit at 55 (or smaller for zero/subnormal), followed by
// guard/round/sticky bits. exponent is the unbiased exponent of that leading bit.
inline F64 round_pack(bool sign, int exponent, U64 sig) {
    if (exponent < -1022) {
        sig = shr_jam(sig, unsigned(-1022 - exponent));
        exponent = -1022;
    }
    U64 rounded = (sig >> 3) + ((sig & 7) > 4 || ((sig & 7) == 4 && ((sig >> 3) & 1)));
    if (rounded >= 0x0020000000000000ULL) { rounded >>= 1; ++exponent; }
    if (exponent > 1023) return infinity(sign);
    U64 biased = rounded >= 0x0010000000000000ULL ? U64(exponent + 1023) : 0;
    return F64::from_bits((U64(sign) << 63) | (biased << 52) | (rounded & 0x000fffffffffffffULL));
}
inline F64 F64::from_u32(unsigned value) {
    if (!value) return zero(false);
    int exponent = 63 - leading_zeroes(value);
    return round_pack(false, exponent, U64(value) << (55 - exponent));
}
inline F64 F64::from_i32(int value) {
    U32 mag = value < 0 ? U32(0) - U32(value) : U32(value);
    F64 result = from_u32(mag);
    return value < 0 ? -result : result;
}
inline F64::F64(int value) : bits(from_i32(value).bits) {}
inline F64::F64(unsigned value) : bits(from_u32(value).bits) {}
inline F64 F64::from_f32_bits(U32 value) {
    bool sign = (value >> 31) != 0;
    int exponent = int((value >> 23) & 255);
    U64 sig = value & 0x007fffffU;
    if (exponent == 255) return sig ? quiet_nan() : infinity(sign);
    if (!exponent && !sig) return zero(sign);
    if (exponent) return F64::from_bits((U64(sign) << 63) | (U64(exponent - 127 + 1023) << 52) | (sig << 29));
    int shift = leading_zeroes(sig) - 40;
    return F64::from_bits((U64(sign) << 63) | (U64(-126 - shift + 1023) << 52) |
                          ((sig << (shift + 29)) & 0x000fffffffffffffULL));
}
inline U32 F64::to_f32_bits() const {
    bool sign = signbit(*this);
    U32 sign_bits = U32(sign) << 31;
    if (isnan(*this)) return 0x7fc00000U;
    if (isinf(*this)) return sign_bits | 0x7f800000U;
    if (iszero(*this)) return sign_bits;
    Normalized n = unpack_finite_nonzero(*this);
    U64 sig = shr_jam(n.sig, 26); // 24 significant + 3 rounding bits.
    if (n.exponent < -126) { sig = shr_jam(sig, unsigned(-126 - n.exponent)); n.exponent = -126; }
    U64 rounded = (sig >> 3) + ((sig & 7) > 4 || ((sig & 7) == 4 && ((sig >> 3) & 1)));
    if (rounded >= 0x01000000ULL) { rounded >>= 1; ++n.exponent; }
    if (n.exponent > 127) return sign_bits | 0x7f800000U;
    U32 biased = rounded >= 0x00800000ULL ? U32(n.exponent + 127) : 0;
    return sign_bits | (biased << 23) | U32(rounded & 0x007fffffULL);
}
inline int F64::to_i32_rtz() const {
    if (!isfinite(*this)) return (-2147483647 - 1);
    if (iszero(*this)) return 0;
    Normalized n = unpack_finite_nonzero(*this);
    if (n.exponent < 0) return 0;
    if (n.exponent > 31) return (-2147483647 - 1);
    U64 mag = n.sig >> (52 - n.exponent);
    if (mag > (signbit(*this) ? 0x80000000ULL : 0x7fffffffULL)) return (-2147483647 - 1);
    return signbit(*this) ? (mag == 0x80000000ULL ? (-2147483647 - 1) : -int(mag)) : int(mag);
}
inline F64 add_general(F64 a, F64 b) {
    if (isnan(a) || isnan(b)) return quiet_nan();
    if (isinf(a)) return isinf(b) && signbit(a) != signbit(b) ? quiet_nan() : a;
    if (isinf(b)) return b;
    if (iszero(a)) return iszero(b) ? zero(signbit(a) && signbit(b)) : b;
    if (iszero(b)) return a;
    if ((a.bits & 0x7fffffffffffffffULL) < (b.bits & 0x7fffffffffffffffULL)) { F64 tmp = a; a = b; b = tmp; }
    Normalized na = unpack_finite_nonzero(a), nb = unpack_finite_nonzero(b);
    U64 sa = na.sig << 3, sb = shr_jam(nb.sig << 3, unsigned(na.exponent - nb.exponent));
    U64 sig;
    if (signbit(a) == signbit(b)) {
        sig = sa + sb;
        if (sig & 0x0100000000000000ULL) { sig = shr_jam(sig, 1); ++na.exponent; }
    } else {
        sig = sa - sb;
        if (!sig) return zero(false);
        int shift = leading_zeroes(sig) - 8;
        sig <<= shift; na.exponent -= shift;
    }
    return round_pack(signbit(a), na.exponent, sig);
}
// Normal operands with enough exponent headroom admit exact packing without
// the general subnormal/overflow checks. All other inputs retain the IEEE path.
inline F64 operator+(F64 a, F64 b) {
    const U64 ma = a.bits & 0x7fffffffffffffffULL;
    const U64 mb = b.bits & 0x7fffffffffffffffULL;
    unsigned ea = unsigned(ma >> 52), eb = unsigned(mb >> 52);
    if (ea - 1u < 2046u && eb - 1u < 2046u) {
        if (ma < mb) { F64 tmp = a; a = b; b = tmp; unsigned te = ea; ea = eb; eb = te; }
        // Adjacent binades can cancel at most 53 exponent bits, so ea >= 54
        // leaves a nonzero result normal. At ea <= 2045, even the sum of
        // two maximal significands cannot exceed the largest finite value.
        if (ea >= 54u && ea <= 2045u) {
            const U64 sa = ((a.bits & 0x000fffffffffffffULL) | 0x0010000000000000ULL) << 3;
            const U64 sb = shr_jam(((b.bits & 0x000fffffffffffffULL) | 0x0010000000000000ULL) << 3, ea - eb);
            U64 sig;
            if (((a.bits ^ b.bits) >> 63) == 0) {
                sig = sa + sb;
                if (sig & 0x0100000000000000ULL) { sig = shr_jam(sig, 1); ++ea; }
            } else {
                sig = sa - sb;
                if (!sig) return zero(false);
                const unsigned shift = unsigned(leading_zeroes(sig) - 8);
                sig <<= shift; ea -= shift;
            }
            // Three GRS bits: +3+retained_lsb implements nearest, ties-to-even.
            const U64 rounded = (sig + 3 + ((sig >> 3) & 1)) >> 3;
            // Including the hidden bit in addition also handles a rounding carry.
            return F64::from_bits((a.bits & 0x8000000000000000ULL) |
                                  ((U64(ea - 1) << 52) + rounded));
        }
    }
    return add_general(a, b);
}
inline F64 operator-(F64 a, F64 b) { return a + (-b); }
inline F64 multiply_general(F64 a, F64 b) {
    bool sign = signbit(a) != signbit(b);
    if (isnan(a) || isnan(b)) return quiet_nan();
    if (isinf(a) || isinf(b)) return iszero(a) || iszero(b) ? quiet_nan() : infinity(sign);
    if (iszero(a) || iszero(b)) return zero(sign);
    Normalized na = unpack_finite_nonzero(a), nb = unpack_finite_nonzero(b);
    // A finite power-of-two factor changes only the exponent. Keep round_pack
    // for gradual-underflow rounding and overflow, including subnormal powers.
    // The special-value checks above retain NaN canonicalization and zero sign.
    if (nb.sig == 0x0010000000000000ULL)
        return round_pack(sign, na.exponent + nb.exponent, na.sig << 3);
    if (na.sig == 0x0010000000000000ULL)
        return round_pack(sign, na.exponent + nb.exponent, nb.sig << 3);
    Wide product = multiply_wide(na.sig, nb.sig);
    bool high = (product.hi & (1ULL << 41)) != 0;
    return round_pack(sign, na.exponent + nb.exponent + int(high), wide_shr_jam(product, high ? 50 : 49));
}
// For two normal operands, biased product exponent 1..2044 leaves room
// for both normalization and rounding carries without underflow or overflow.
inline F64 operator*(F64 a, F64 b) {
    const unsigned ea = unsigned((a.bits >> 52) & 2047);
    const unsigned eb = unsigned((b.bits >> 52) & 2047);
    const unsigned exponent = ea + eb - 1023u;
    if (ea - 1u < 2046u && eb - 1u < 2046u && exponent - 1u < 2044u) {
        const U64 sign = (a.bits ^ b.bits) & 0x8000000000000000ULL;
        const U64 fa = a.bits & 0x000fffffffffffffULL;
        const U64 fb = b.bits & 0x000fffffffffffffULL;
        if (!fb) return F64::from_bits(sign | (U64(exponent) << 52) | fa);
        if (!fa) return F64::from_bits(sign | (U64(exponent) << 52) | fb);
        const Wide product = multiply_wide(fa | 0x0010000000000000ULL,
                                           fb | 0x0010000000000000ULL);
        const bool high = (product.hi & (1ULL << 41)) != 0;
        const U64 sig = wide_shr_jam(product, high ? 50 : 49);
        const U64 rounded = (sig + 3 + ((sig >> 3) & 1)) >> 3;
        return F64::from_bits(sign | ((U64(exponent + unsigned(high) - 1) << 52) + rounded));
    }
    return multiply_general(a, b);
}
inline F64 divide_restoring(F64 a, F64 b) {
    bool sign = signbit(a) != signbit(b);
    if (isnan(a) || isnan(b)) return quiet_nan();
    if (isinf(a)) return isinf(b) ? quiet_nan() : infinity(sign);
    if (isinf(b)) return zero(sign);
    if (iszero(b)) return iszero(a) ? quiet_nan() : infinity(sign);
    if (iszero(a)) return zero(sign);
    Normalized na = unpack_finite_nonzero(a), nb = unpack_finite_nonzero(b);
    int exponent = na.exponent - nb.exponent;
    // An exact power-of-two denominator only changes the exponent. Keep the
    // normal packer: a subnormal quotient still needs guard/sticky rounding.
    // This includes subnormal powers after unpacking and preserves the sign.
    if (nb.sig == 0x0010000000000000ULL)
        return round_pack(sign, exponent, na.sig << 3);
    if (na.sig < nb.sig) { na.sig <<= 1; --exponent; }
    U64 remainder = na.sig - nb.sig, quotient = 1ULL << 55;
    for (int bit = 54; bit >= 0; --bit) {
        remainder <<= 1;
        if (remainder >= nb.sig) { remainder -= nb.sig; quotient |= 1ULL << bit; }
    }
    return round_pack(sign, exponent, quotient | (remainder != 0));
}
// A verified radix digit of floor(numerator * 2^shift / denominator).
// Inputs cover the first 19-bit digit and the two continuation digits. An
// arbitrary hint is accepted only after the exact product/remainder bracket;
// failed verification always leaves a zero record for the caller's fallback.
struct DivisionDigit { U64 quotient, remainder; bool valid; };
inline DivisionDigit division_digit(U64 numerator, U64 denominator,
                                    unsigned shift, U64 hint) {
    if (denominator < (1ULL << 52) || denominator >= (1ULL << 53) ||
        numerator >= (denominator << 1) || (shift != 18 && shift != 19) ||
        hint >= (1ULL << 20)) return {0, 0, false};
    const Wide target = {numerator >> (64 - shift), numerator << shift};
    // The hint is a small digit: two 32x32 products replace a general 64x64.
    const U64 low = U64(U32(denominator)) * U32(hint);
    const U64 high = (denominator >> 32) * U32(hint);
    Wide product = {(high >> 32), low + (high << 32)};
    product.hi += product.lo < low;
    U64 quotient = hint;
    for (unsigned correction = 0; correction <= 2; ++correction) {
        const bool above = product.hi > target.hi ||
            (product.hi == target.hi && product.lo > target.lo);
        if (!above) {
            const U64 remainder = target.lo - product.lo;
            const U64 upper = target.hi - product.hi - (target.lo < product.lo);
            if (!upper && remainder < denominator)
                return {quotient, remainder, true};
        }
        if (correction == 2) break;
        if (above) {
            // product > target implies quotient != 0.
            const U64 previous = product.lo;
            product.lo -= denominator; product.hi -= previous < denominator;
            --quotient;
        } else {
            const U64 previous = product.lo;
            product.lo += denominator; product.hi += product.lo < previous;
            ++quotient;
        }
    }
    return {0, 0, false};
}
inline U64 division_hint(float estimate) {
    // Reject NaN, infinity, negative and out-of-domain values before casting.
    return estimate >= 0.0f && estimate < 1048576.0f ? U64(estimate) : 1ULL << 20;
}
inline F64 divide_normalized_with_reciprocal(F64 a, F64 b, Normalized na,
                                            Normalized nb, float reciprocal) {
    int exponent = na.exponent - nb.exponent;
    if (na.sig < nb.sig) { na.sig <<= 1; --exponent; }
    // A single approximate reciprocal proposes 18+19+18 quotient bits.
    // Each accepted digit preserves N = q*D + r, 0 <= r < D. Therefore the
    // concatenation is the same 56-bit quotient as all 55 restoring steps,
    // and the last remainder supplies exactly the same sticky bit.
    const DivisionDigit first = division_digit(na.sig, nb.sig, 18,
        division_hint((float(na.sig) * reciprocal) * 0.5f));
    if (!first.valid) return divide_restoring(a, b);
    const DivisionDigit middle = division_digit(first.remainder, nb.sig, 19,
        division_hint(float(first.remainder) * reciprocal));
    if (!middle.valid) return divide_restoring(a, b);
    const DivisionDigit last = division_digit(middle.remainder, nb.sig, 18,
        division_hint((float(middle.remainder) * reciprocal) * 0.5f));
    if (!last.valid) return divide_restoring(a, b);
    const U64 quotient = (first.quotient << 37) | (middle.quotient << 18) | last.quotient;
    return round_pack(signbit(a) != signbit(b), exponent, quotient | (last.remainder != 0));
}
// Diagnostic entry intentionally accepts unrelated or exceptional seeds.
inline F64 divide_with_reciprocal_hint(F64 a, F64 b, float reciprocal) {
    if (!isfinite(a) || !isfinite(b) || iszero(a) || iszero(b))
        return divide_restoring(a, b);
    const Normalized na = unpack_finite_nonzero(a), nb = unpack_finite_nonzero(b);
    if (nb.sig == 0x0010000000000000ULL)
        return round_pack(signbit(a) != signbit(b), na.exponent - nb.exponent, na.sig << 3);
    return divide_normalized_with_reciprocal(a, b, na, nb, reciprocal);
}
#ifdef __METAL_VERSION__
// Optional specialization keeps the original algorithm available in the same
// offline library for controlled A/B measurements; undefined selects verified.
constant bool exactDivisionBaseline [[function_constant(4)]];
#endif
inline F64 operator/(F64 a, F64 b) {
#ifdef __METAL_VERSION__
    if (is_function_constant_defined(exactDivisionBaseline) && exactDivisionBaseline)
        return divide_restoring(a, b);
#endif
    if (!isfinite(a) || !isfinite(b) || iszero(a) || iszero(b))
        return divide_restoring(a, b);
    const Normalized na = unpack_finite_nonzero(a), nb = unpack_finite_nonzero(b);
    if (nb.sig == 0x0010000000000000ULL)
        return round_pack(signbit(a) != signbit(b), na.exponent - nb.exponent, na.sig << 3);
    const float reciprocal = 524288.0f / float(nb.sig);
    return divide_normalized_with_reciprocal(a, b, na, nb, reciprocal);
}
struct SqrtPrefix {
    U64 root, remainder;
    bool valid;
};
// The first 24 restoring digits consume a high limb below 2^48. An estimate
// is useful only after this exact bracket proves the integer square root.
// Bounded correction is an optimization; a rejected hint retains the original
// restoring computation. The bound also keeps every square below 2^49.
inline SqrtPrefix sqrt_high_prefix(U64 high, U64 hint) {
    if (high >= (1ULL << 48) || hint > (1ULL << 24))
        return {0, 0, false};
    U64 root = hint;
    for (unsigned correction = 0; correction < 4; ++correction) {
        U64 square = root * root;
        if (square > high) --root;
        else if (high - square >= (root << 1) + 1) ++root;
        else return {root, high - square, true};
    }
    U64 square = root * root;
    if (square <= high && high - square < (root << 1) + 1)
        return {root, high - square, true};
    return {0, 0, false};
}
// n is the normalized finite, positive input. Completing the low 32 digits
// from a verified prefix is identical to completing the original 56-digit
// restoring loop: high = root^2 + remainder, remainder < 2*root + 1.
inline F64 sqrt_normalized_with_hint(Normalized n, U64 hint) {
    if (n.exponent % 2 != 0) { n.sig <<= 1; --n.exponent; }
    Wide radicand = {n.sig >> 6, n.sig << 58};
    SqrtPrefix prefix = sqrt_high_prefix(radicand.hi, hint);
    U64 root = prefix.root, remainder = prefix.remainder;
    if (!prefix.valid) {
        // Original high-limb portion of the restoring loop (pairs 55..32).
        for (int pair = 23; pair >= 0; --pair) {
            U64 digit = (radicand.hi >> unsigned(pair * 2)) & 3;
            remainder = (remainder << 2) | digit;
            root <<= 1;
            U64 trial = (root << 1) | 1;
            if (remainder >= trial) { remainder -= trial; ++root; }
        }
    }
    for (int pair = 31; pair >= 0; --pair) {
        U64 digit = (radicand.lo >> unsigned(pair * 2)) & 3;
        remainder = (remainder << 2) | digit;
        root <<= 1;
        U64 trial = (root << 1) | 1;
        if (remainder >= trial) { remainder -= trial; ++root; }
    }
    return round_pack(false, n.exponent / 2, root | (remainder != 0));
}
// Diagnostic entry for adversarial estimates. It has the same value contract
// as sqrt even when the hint is wholly unrelated to the input.
inline F64 sqrt_with_hint(F64 a, U64 hint) {
    if (isnan(a)) return quiet_nan();
    if (iszero(a)) return a;
    if (signbit(a)) return quiet_nan();
    if (isinf(a)) return a;
    return sqrt_normalized_with_hint(unpack_finite_nonzero(a), hint);
}
// A 32-bit root consumes the high 64 bits of the normalized radicand. Every
// candidate square fits U64. The upper bracket is tested as H-q^2 < 2*q+1,
// so its mathematical endpoint 2^64 never needs an overflowing square.
inline SqrtPrefix sqrt_wide_prefix(U64 high, U64 hint) {
    constexpr U64 rootLimit = 1ULL << 32, rootMax = rootLimit - 1;
    if (hint >= rootLimit) return {0, 0, false};
    U64 root = hint, square = root * root;
    if (square <= high && high - square < (root << 1) + 1)
        return {root, high - square, true};
    if (!root) return {0, 0, false};

    // One approximate residual correction restores the low integer bits lost
    // by the binary32 sqrt. Apply that correction to the INTEGER candidate;
    // adding it to float(root) would round away those same bits again.
    const bool subtract = square > high;
    const U64 magnitude = subtract ? square - high : high - square;
    const float estimate = float(magnitude) / (2.0f * float(root));
    if (!(estimate >= 0.0f && estimate <= 1024.0f)) return {0, 0, false};
    const U64 correction = U64(estimate);
    if (subtract) {
        if (correction > root) return {0, 0, false};
        root -= correction;
    } else {
        // A rounded residual can suggest the one-past-root endpoint near
        // UINT64_MAX. Clamp only this untrusted proposal; the exact bracket
        // below still decides whether the resulting prefix is admissible.
        const U64 room = rootMax - root;
        root += correction < room ? correction : room;
    }
    // Neither the sqrt estimate nor its refinement is trusted. At most four
    // single-unit corrections are allowed before reverting to the old path.
    for (unsigned adjustment = 0; adjustment <= 4; ++adjustment) {
        square = root * root;
        if (square <= high && high - square < (root << 1) + 1)
            return {root, high - square, true};
        if (adjustment == 4) break;
        if (square > high) --root; // A zero square cannot exceed high.
        else ++root; // At rootMax the bracket above accepts every H>=q^2.
    }
    return {0, 0, false};
}
inline F64 sqrt_normalized_with_wide_hint(Normalized n, U64 hint) {
    Normalized even = n;
    if (even.exponent % 2 != 0) { even.sig <<= 1; --even.exponent; }
    // R = even.sig * 2^58. Its high 64 bits are even.sig * 2^10, and
    // the remaining 48 bits are zero. even.sig is strictly below 2^54.
    const SqrtPrefix prefix = sqrt_wide_prefix(even.sig << 10, hint);
    if (!prefix.valid) {
        const U64 high = even.sig >> 6;
#ifdef __METAL_VERSION__
        const float estimate = metal::precise::sqrt(float(high));
#else
        const float estimate = std::sqrt(float(high));
#endif
        const U64 fallback = estimate >= 0.0f && estimate <= 16777216.0f
            ? U64(estimate) : (1ULL << 24) + 1;
        return sqrt_normalized_with_hint(n, fallback);
    }
    U64 root = prefix.root, remainder = prefix.remainder;
#ifdef __METAL_VERSION__
    // Keeping this exact recurrence compact is faster on the measured Apple
    // GPU than automatic/full unrolling; its operation order is unchanged.
#pragma clang loop unroll(disable)
#endif
    for (int pair = 23; pair >= 0; --pair) {
        remainder <<= 2;
        root <<= 1;
        const U64 trial = (root << 1) | 1;
        if (remainder >= trial) { remainder -= trial; ++root; }
    }
    return round_pack(false, even.exponent / 2, root | (remainder != 0));
}
// Independent diagnostic entry: arbitrary seeds must preserve the complete
// sqrt value even when refinement fails or the seed cannot be squared safely.
inline F64 sqrt_with_wide_hint(F64 a, U64 hint) {
    if (isnan(a)) return quiet_nan();
    if (iszero(a)) return a;
    if (signbit(a)) return quiet_nan();
    if (isinf(a)) return a;
    return sqrt_normalized_with_wide_hint(unpack_finite_nonzero(a), hint);
}
inline F64 sqrt(F64 a) {
    if (isnan(a)) return quiet_nan();
    if (iszero(a)) return a;
    if (signbit(a)) return quiet_nan();
    if (isinf(a)) return a;
    Normalized n = unpack_finite_nonzero(a);
    const U64 wideHigh = n.sig << (n.exponent % 2 != 0 ? 11 : 10);
#ifdef __METAL_VERSION__
    const float estimate = metal::precise::sqrt(float(wideHigh));
#else
    const float estimate = std::sqrt(float(wideHigh));
#endif
    // Test the float before conversion: NaN, infinity and out-of-range values
    // must never reach a float-to-integer cast. In particular, the rounded
    // endpoint 2^32 must not be squared in U64.
    const U64 hint = estimate >= 0.0f && estimate < 4294967296.0f
        ? U64(estimate) : 1ULL << 32;
    return sqrt_normalized_with_wide_hint(n, hint);
}
inline MB3D_EXACT_THREAD F64& operator+=(MB3D_EXACT_THREAD F64& a, F64 b) { a = a + b; return a; }
inline MB3D_EXACT_THREAD F64& operator-=(MB3D_EXACT_THREAD F64& a, F64 b) { a = a - b; return a; }
inline MB3D_EXACT_THREAD F64& operator*=(MB3D_EXACT_THREAD F64& a, F64 b) { a = a * b; return a; }
inline MB3D_EXACT_THREAD F64& operator/=(MB3D_EXACT_THREAD F64& a, F64 b) { a = a / b; return a; }
} // namespace mb3d_metal_exact
#undef MB3D_EXACT_THREAD

#endif // MB3D_METAL_NATIVE_FLOAT
