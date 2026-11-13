#pragma once
#ifdef MB3D_METAL_NATIVE_FLOAT
#include "native_arithmetic.h"
#else
#include "soft_double.h"

// Value-only x87 extended storage profile: 64-bit significand, 15-bit exponent,
// nearest-even; canonical quiet NaN and no floating exception state. Operands
// must have canonical encodings (no x87 unsupported/pseudo-denormal encodings).
namespace mb3d_metal_exact {
inline Wide wide_shift_right_jam(Wide a, unsigned n) {
    if (!n) return a;
    if (n < 64) return {a.hi >> n, (a.hi << (64-n)) | (a.lo >> n) | ((a.lo << (64-n)) != 0)};
    if (n == 64) return {0, a.hi | (a.lo != 0)};
    if (n < 128) return {0, shr_jam(a.hi, n-64) | (a.lo != 0)};
    return {0, (a.hi | a.lo) != 0};
}
inline Wide wide_shift_left(Wide a, unsigned n) {
    if (!n) return a;
    if (n < 64) return {(a.hi << n) | (a.lo >> (64-n)), a.lo << n};
    return {a.lo << (n-64), 0};
}
inline bool wide_less(Wide a, Wide b) { return a.hi < b.hi || (a.hi == b.hi && a.lo < b.lo); }
inline Wide wide_sub(Wide a, Wide b) { return {a.hi - b.hi - (a.lo < b.lo), a.lo - b.lo}; }

struct E80 {
    U64 significand;
    U32 sign_exponent;
    E80() : significand(0), sign_exponent(0) {}
    explicit E80(F64 value);
    explicit E80(int value) : E80(F64(value)) {}
    static E80 from_bits(U32 sign_exp, U64 sig) { E80 r; r.significand = sig; r.sign_exponent = sign_exp; return r; }
    F64 to_f64() const;
    U32 to_f32_bits() const;
    int to_i32_rtz() const;
    int to_i32_rne() const;
};
inline bool signbit(E80 a) { return (a.sign_exponent & 0x8000) != 0; }
inline bool isnan(E80 a) { return (a.sign_exponent & 0x7fff) == 0x7fff && (a.significand & 0x7fffffffffffffffULL) != 0; }
inline bool isinf(E80 a) { return (a.sign_exponent & 0x7fff) == 0x7fff && a.significand == 0x8000000000000000ULL; }
inline bool isfinite(E80 a) { return (a.sign_exponent & 0x7fff) != 0x7fff; }
inline bool iszero(E80 a) { return a.significand == 0 && (a.sign_exponent & 0x7fff) == 0; }
inline E80 ext_nan() { return E80::from_bits(0x7fff, 0xc000000000000000ULL); }
inline E80 ext_inf(bool sign) { return E80::from_bits((U32(sign) << 15) | 0x7fff, 0x8000000000000000ULL); }
inline E80 ext_zero(bool sign) { return E80::from_bits(U32(sign) << 15, 0); }
inline E80 operator-(E80 a) { return E80::from_bits(a.sign_exponent ^ 0x8000, a.significand); }
inline E80 abs(E80 a) { return E80::from_bits(a.sign_exponent & 0x7fff, a.significand); }
inline bool operator==(E80 a, E80 b) {
    return !isnan(a) && !isnan(b) && ((a.sign_exponent == b.sign_exponent && a.significand == b.significand) || (iszero(a) && iszero(b)));
}
inline bool operator!=(E80 a, E80 b) { return !(a == b); }
inline bool operator<(E80 a, E80 b) {
    if (isnan(a) || isnan(b) || (iszero(a) && iszero(b))) return false;
    if (signbit(a) != signbit(b)) return signbit(a);
    bool less = a.sign_exponent < b.sign_exponent || (a.sign_exponent == b.sign_exponent && a.significand < b.significand);
    return signbit(a) ? !less && a != b : less;
}
inline bool operator>(E80 a, E80 b) { return b < a; }
inline bool operator<=(E80 a, E80 b) { return a < b || a == b; }
inline bool operator>=(E80 a, E80 b) { return b <= a; }
inline Normalized unpack_finite_nonzero(E80 a) {
    int exponent = int(a.sign_exponent & 0x7fff);
    if (exponent) return {a.significand, exponent - 16383};
    int shift = leading_zeroes(a.significand);
    return {a.significand << shift, -16382 - shift};
}
inline E80 round_pack_ext(bool sign, int exponent, Wide sig) {
    if (exponent < -16382) { sig = wide_shift_right_jam(sig, unsigned(-16382 - exponent)); exponent = -16382; }
    bool increment = sig.lo > 0x8000000000000000ULL || (sig.lo == 0x8000000000000000ULL && (sig.hi & 1));
    U64 rounded = sig.hi + increment;
    if (increment && !rounded) { rounded = 0x8000000000000000ULL; ++exponent; }
    if (exponent > 16383) return ext_inf(sign);
    U32 biased = (rounded >> 63) ? U32(exponent + 16383) : 0;
    return E80::from_bits((U32(sign) << 15) | biased, rounded);
}
inline E80::E80(F64 value) : significand(0), sign_exponent(0) {
    if (isnan(value)) { significand = 0xc000000000000000ULL; sign_exponent = 0x7fff; }
    else if (isinf(value)) { significand = 0x8000000000000000ULL; sign_exponent = (U32(signbit(value)) << 15) | 0x7fff; }
    else if (iszero(value)) { sign_exponent = U32(signbit(value)) << 15; }
    else { Normalized n = unpack_finite_nonzero(value); significand = n.sig << 11; sign_exponent = (U32(signbit(value)) << 15) | U32(n.exponent + 16383); }
}
inline F64 E80::to_f64() const {
    if (isnan(*this)) return quiet_nan();
    if (isinf(*this)) return infinity(signbit(*this));
    if (iszero(*this)) return zero(signbit(*this));
    Normalized n = unpack_finite_nonzero(*this);
    return round_pack(signbit(*this), n.exponent, shr_jam(n.sig, 8));
}
inline U32 E80::to_f32_bits() const {
    U32 sign = U32(signbit(*this)) << 31;
    if (isnan(*this)) return 0x7fc00000U;
    if (isinf(*this)) return sign | 0x7f800000U;
    if (iszero(*this)) return sign;
    Normalized n = unpack_finite_nonzero(*this);
    U64 sig = shr_jam(n.sig, 37);
    if (n.exponent < -126) { sig = shr_jam(sig, unsigned(-126 - n.exponent)); n.exponent = -126; }
    U64 rounded = (sig >> 3) + ((sig & 7) > 4 || ((sig & 7) == 4 && ((sig >> 3) & 1)));
    if (rounded >= 0x01000000ULL) { rounded >>= 1; ++n.exponent; }
    if (n.exponent > 127) return sign | 0x7f800000U;
    U32 biased = rounded >= 0x00800000ULL ? U32(n.exponent + 127) : 0;
    return sign | (biased << 23) | U32(rounded & 0x007fffffULL);
}
inline int ext_integer(E80 a, bool nearest) {
    if (!isfinite(a)) return (-2147483647 - 1);
    if (iszero(a)) return 0;
    Normalized n = unpack_finite_nonzero(a);
    if (n.exponent > 31) return (-2147483647 - 1);
    U64 mag = 0;
    if (n.exponent >= 0) {
        unsigned shift = unsigned(63 - n.exponent);
        mag = n.sig >> shift;
        U64 rem = n.sig & ((1ULL << shift) - 1), midpoint = 1ULL << (shift - 1);
        if (nearest && (rem > midpoint || (rem == midpoint && (mag & 1)))) ++mag;
    } else if (nearest && n.exponent == -1 && n.sig > 0x8000000000000000ULL) mag = 1;
    if (mag > (signbit(a) ? 0x80000000ULL : 0x7fffffffULL)) return (-2147483647 - 1);
    return signbit(a) ? (mag == 0x80000000ULL ? (-2147483647 - 1) : -int(mag)) : int(mag);
}
inline int E80::to_i32_rtz() const { return ext_integer(*this, false); }
inline int E80::to_i32_rne() const { return ext_integer(*this, true); }
inline E80 operator+(E80 a, E80 b) {
    if (isnan(a) || isnan(b)) return ext_nan();
    if (isinf(a)) return isinf(b) && signbit(a) != signbit(b) ? ext_nan() : a;
    if (isinf(b)) return b;
    if (iszero(a)) return iszero(b) ? ext_zero(signbit(a) && signbit(b)) : b;
    if (iszero(b)) return a;
    if (abs(a) < abs(b)) { E80 tmp = a; a = b; b = tmp; }
    Normalized na = unpack_finite_nonzero(a), nb = unpack_finite_nonzero(b);
    Wide sb = wide_shift_right_jam({nb.sig,0}, unsigned(na.exponent - nb.exponent)), sig;
    if (signbit(a) == signbit(b)) {
        sig = {na.sig + sb.hi, sb.lo};
        if (sig.hi < na.sig) { sig = {0x8000000000000000ULL | (sig.hi >> 1), (sig.hi << 63) | (sig.lo >> 1) | ((sig.lo & 1) != 0)}; ++na.exponent; }
    } else {
        sig = wide_sub({na.sig,0}, sb);
        if (!(sig.hi | sig.lo)) return ext_zero(false);
        int shift = sig.hi ? leading_zeroes(sig.hi) : 64 + leading_zeroes(sig.lo);
        sig = wide_shift_left(sig, unsigned(shift)); na.exponent -= shift;
    }
    return round_pack_ext(signbit(a), na.exponent, sig);
}
inline E80 operator-(E80 a, E80 b) { return a + (-b); }
inline E80 operator*(E80 a, E80 b) {
    bool sign = signbit(a) != signbit(b);
    if (isnan(a) || isnan(b)) return ext_nan();
    if (isinf(a) || isinf(b)) return iszero(a) || iszero(b) ? ext_nan() : ext_inf(sign);
    if (iszero(a) || iszero(b)) return ext_zero(sign);
    Normalized na = unpack_finite_nonzero(a), nb = unpack_finite_nonzero(b);
    Wide sig = multiply_wide(na.sig, nb.sig);
    bool high = (sig.hi >> 63) != 0;
    if (!high) sig = wide_shift_left(sig,1);
    return round_pack_ext(sign, na.exponent + nb.exponent + int(high), sig);
}
inline E80 operator/(E80 a, E80 b) {
    bool sign = signbit(a) != signbit(b);
    if (isnan(a) || isnan(b)) return ext_nan();
    if (isinf(a)) return isinf(b) ? ext_nan() : ext_inf(sign);
    if (isinf(b)) return ext_zero(sign);
    if (iszero(b)) return iszero(a) ? ext_nan() : ext_inf(sign);
    if (iszero(a)) return ext_zero(sign);
    Normalized na = unpack_finite_nonzero(a), nb = unpack_finite_nonzero(b);
    int exponent = na.exponent - nb.exponent;
    U64 remainder;
    if (na.sig < nb.sig) { remainder = (na.sig << 1) - nb.sig; --exponent; }
    else remainder = na.sig - nb.sig;
    U64 quotient = 0x8000000000000000ULL;
    for (int bit = 62; bit >= 0; --bit) {
        bool carry = (remainder >> 63) != 0; remainder <<= 1;
        if (carry || remainder >= nb.sig) { remainder -= nb.sig; quotient |= 1ULL << bit; }
    }
    bool carry = (remainder >> 63) != 0; remainder <<= 1;
    U64 extra = 0;
    if (carry || remainder >= nb.sig) { remainder -= nb.sig; extra = 0x8000000000000000ULL; }
    extra |= remainder != 0;
    return round_pack_ext(sign, exponent, {quotient, extra});
}
inline E80 sqrt(E80 a) {
    if (isnan(a)) return ext_nan();
    if (iszero(a)) return a;
    if (signbit(a)) return ext_nan();
    if (isinf(a)) return a;
    Normalized n = unpack_finite_nonzero(a);
    bool odd = n.exponent % 2 != 0;
    Wide radicand = odd ? Wide{n.sig,0} : Wide{n.sig >> 1,n.sig << 63};
    if (odd) --n.exponent;
    U64 root = 0; Wide remainder = {0,0};
    for (int pair = 63; pair >= 0; --pair) {
        unsigned bit = unsigned(pair*2);
        U64 digit = bit >= 64 ? (radicand.hi >> (bit-64)) & 3 : (radicand.lo >> bit) & 3;
        remainder = wide_shift_left(remainder,2); remainder.lo |= digit;
        root <<= 1;
        Wide trial = {root >> 63, (root << 1) | 1};
        if (!wide_less(remainder,trial)) { remainder = wide_sub(remainder,trial); ++root; }
    }
    // Integer radicand cannot lie at (root + 1/2)^2. remainder > root is
    // therefore exactly the round-up predicate, with no tie approximation.
    bool increment = remainder.hi != 0 || remainder.lo > root;
    return round_pack_ext(false, n.exponent/2, {root, increment ? 0x8000000000000001ULL : 0});
}
} // namespace mb3d_metal_exact

#endif // MB3D_METAL_NATIVE_FLOAT
