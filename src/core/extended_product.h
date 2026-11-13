#pragma once
#include "core/extended_series_context.h"

#if defined(MB3D_SOFTWARE_EXT80) || defined(MB3D_HAVE_SOFT_EXT80)
namespace mb3d::numeric {

// Exact normal-product fast path for the extended series recurrences. Keep
// the complete 128-bit product until nearest-even rounding; there is no
// binary64 intermediate or changed arithmetic grouping. Boundary and alternate
// environment cases retain the original SoftFloat operation and its flags.
inline Extended80 extendedProduct(Extended80 a, Extended80 b,
                                  const ExtendedSeriesContext& context) {
    if (NativePrecisionEnabled()) return a * b;
#if !defined(__SIZEOF_INT128__)
    // MSVC uses the same SoftFloat operation; the ARM integer fast path is
    // optional and does not define a different arithmetic contract.
    return a * b;
#else
    const uint16_t signExponentA = a.signExponent();
    const uint16_t signExponentB = b.signExponent();
    const unsigned exponentA = signExponentA & 0x7fff;
    const unsigned exponentB = signExponentB & 0x7fff;
    constexpr uint64_t integerBit = UINT64_C(0x8000000000000000);
    if (exponentA == 0 || exponentA == 0x7fff ||
        exponentB == 0 || exponentB == 0x7fff ||
        !(a.significand() & b.significand() & integerBit) ||
        !context.nearestEven80())
        return a * b;

    using U = __uint128_t;
    U product = U(a.significand()) * U(b.significand());
    int exponent = int(exponentA) + int(exponentB) - 0x3ffe;
    if (!(uint64_t(product >> 64) & integerBit)) {
        product <<= 1;
        --exponent;
    }
    // Let SoftFloat handle both tininess policies and the overflow boundary.
    if (exponent <= 0 || exponent >= 0x7ffe) return a * b;
    uint64_t significand = uint64_t(product >> 64);
    const uint64_t remainder = uint64_t(product);
    const bool increment = remainder > integerBit ||
                           (remainder == integerBit && (significand & 1));
    if (increment && significand == UINT64_MAX) return a * b;
    if (remainder) context.raiseInexact();
    significand += increment;
    return Extended80::fromBits(
        uint16_t(((signExponentA ^ signExponentB) & 0x8000) | unsigned(exponent)),
        significand);
#endif
}

inline Extended80 extendedProduct(Extended80 a, Extended80 b) {
    const ExtendedSeriesContext context;
    return extendedProduct(a, b, context);
}

} // namespace mb3d::numeric
#endif
