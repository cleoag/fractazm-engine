#pragma once
#ifdef MB3D_METAL_NATIVE_FLOAT
#include "native_arithmetic.h"
#else
#include "soft_extended.h"

namespace mb3d_metal_exact {

inline E80 extended_ln2() {
    return E80::from_bits(0x3ffe, 0xb17217f7d1cf79acULL);
}

// Same finite reduction, ascending series and binary80 stores as the shared
// CPU numerical-DE helper. Other libm callers retain their existing profile.
// Nonfinite values follow E80's established canonical-NaN contract.
inline E80 extended_log2(E80 value) {
    const E80 zero(0), one(1), two(2);
    if (isnan(value)) return ext_nan();
    if (isinf(value)) return signbit(value) ? ext_nan() : value;
    if (iszero(value)) return ext_inf(true);
    if (!(value > zero)) return ext_nan();

    const Normalized normal = unpack_finite_nonzero(value);
    int exponent = normal.exponent;
    E80 mantissa = E80::from_bits(0x3fff, normal.sig);
    if (mantissa > E80(F64::from_bits(0x3ff8000000000000ULL))) {
        mantissa = mantissa / two;
        ++exponent;
    }
    const E80 t = (mantissa - one) / (mantissa + one);
    const E80 square = t * t;
    E80 term = t, sum = t;
    for (int k = 1; k < 32; ++k) {
        term = term * square;
        const E80 next = sum + term / E80(2 * k + 1);
        if (next == sum) break;
        sum = next;
    }
    return E80(exponent) + (two * sum) / extended_ln2();
}

struct ExtendedNumericalDE {
    F64 raw;
    F64 denominator;
};

inline ExtendedNumericalDE extended_numerical_de(F64 radius, F64 dxSquared,
                                               F64 dySquared, F64 dzSquared,
                                               F64 scale, F64 offset) {
    const F64 denominator = sqrt((dzSquared + dySquared) + dxSquared) + offset;
    const E80 logarithm = extended_ln2() * extended_log2(E80(radius));
    const E80 numerator = (logarithm * E80(radius)) * E80(scale);
    return {(numerator / E80(denominator)).to_f64(), denominator};
}

} // namespace mb3d_metal_exact

#endif // MB3D_METAL_NATIVE_FLOAT
