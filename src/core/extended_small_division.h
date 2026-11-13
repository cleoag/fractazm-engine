#pragma once
#include "core/extended_series_context.h"

#if defined(MB3D_SOFTWARE_EXT80) || defined(MB3D_HAVE_SOFT_EXT80)
namespace mb3d::numeric {

// The series recurrences divide by small, exact positive integers. For a
// normalized result, obtain the same 64-bit significand and remainder as a
// full extended division using two native integer divisions. Exceptional and
// reduced-precision cases keep the upstream SoftFloat implementation.
inline Extended80 extendedDivideSmall(Extended80 value, uint32_t divisor,
                                      const ExtendedSeriesContext& context) {
    if (NativePrecisionEnabled()) return value / Extended80(static_cast<double>(divisor));
    const uint16_t signExponent = value.signExponent();
    const unsigned exponent = signExponent & 0x7fff;
    const uint64_t significand = value.significand();
    if (divisor == 0 || divisor > 1024 || exponent == 0 || exponent == 0x7fff ||
        (significand >> 63) == 0 ||
        !context.nearestEven80())
        return value / Extended80(static_cast<double>(divisor));

    const uint64_t quotient = significand / divisor;
    const unsigned shift = std::countl_zero(quotient); // 0..10, quotient != 0
    if (exponent <= shift)
        return value / Extended80(static_cast<double>(divisor));

    // significand = quotient * divisor + remainder. Left-shifting the
    // quotient normalizes it; the remainder contributes at most ten more
    // integer bits and an exact rounding residual. No 128-bit divide is used.
    const uint32_t remainder = static_cast<uint32_t>(significand % divisor);
    const uint32_t scaledRemainder = remainder << shift;
    uint64_t result = (quotient << shift) + scaledRemainder / divisor;
    const uint32_t residual = scaledRemainder % divisor;
    unsigned resultExponent = exponent - shift;
    if (residual != 0) {
        context.raiseInexact();
        const uint32_t twiceResidual = 2 * residual;
        if (twiceResidual > divisor ||
            (twiceResidual == divisor && (result & 1))) {
            if (++result == 0) {
                result = UINT64_C(0x8000000000000000);
                ++resultExponent;
            }
        }
    }
    return Extended80::fromBits(
        static_cast<uint16_t>((signExponent & 0x8000) | resultExponent), result);
}

inline Extended80 extendedDivideSmall(Extended80 value, uint32_t divisor) {
    const ExtendedSeriesContext context;
    return extendedDivideSmall(value, divisor, context);
}

} // namespace mb3d::numeric
#endif
