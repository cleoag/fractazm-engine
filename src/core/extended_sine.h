#pragma once

#include "core/extended_transcendentals.h"

#if defined(MB3D_SOFTWARE_EXT80) || defined(MB3D_HAVE_SOFT_EXT80)
namespace mb3d::numeric {

// Scalar FSIN for the selected software-extended kernels. Keep the existing
// extendedSinCos reduction and recurrence order, but evaluate only the series
// selected by the quadrant. No binary64 intermediate or reciprocal is used.
// The caller owns ExtendedScope (nearest-even, 64 significand bits).
inline Extended80 extendedSin(Extended80 x) {
    using E = Extended80;
    if (NativePrecisionEnabled()) return E(std::sin(double(x)));
    const bool negative = (x.signExponent() & 0x8000) != 0;
    const int exponent = (x.signExponent() & 0x7fff) - 16383;
    if (exponent == 16384) return extendedNaN();
    // Unlike FSINCOS's empty-stack sine result, scalar FSIN leaves an
    // out-of-range finite operand unchanged.
    if (exponent >= 63) return x;

    E r = abs(x);
    unsigned quadrant = 0;
    if (exponent >= -4) {
        const auto reduced=reduceExtendedTrig(x,exponent);
        r=reduced.residual;quadrant=reduced.quadrant;
    }

    const E square = -(r * r);
    const bool cosine = (quadrant & 1) != 0;
    E term = cosine ? E(1) : r;
    E sum = term;
    const ExtendedSeriesContext arithmetic;
    const bool canStop = arithmetic.nearestEven80();
    unsigned unchanged = 0;
    for (int k = 1; k < 16; ++k) {
        const int divisor = cosine ? (2 * k - 1) * (2 * k)
                                   : (2 * k) * (2 * k + 1);
        term = extendedDivideSmall(extendedProduct(term, square, arithmetic), uint32_t(divisor), arithmetic);
        const E next = sum + term;
        const bool same = next.signExponent() == sum.signExponent() &&
                          next.significand() == sum.significand();
        sum = next;
        // The reduced |r| <= pi/4, so |square| < 1. Both rounded operations
        // in each recurrence make its magnitude decrease monotonically; the
        // terms alternate signs (including signed underflowed zero). Once
        // two consecutive additions leave the complete sum bits unchanged,
        // each sign has failed to cross its rounding boundary. Every later
        // term of that sign is smaller and therefore cannot change the sum.
        // Two, not one, unchanged additions also cover the asymmetric ulps
        // above/below exact powers of two and the signed-zero transition.
        // This argument uses nearest-even/80 precision. Other environments
        // retain all 15 terms. ExtendedScope restores the caller's flags;
        // flags from discarded, numerically inactive tail work are private.
        unchanged = same ? unchanged + 1 : 0;
        if (canStop && unchanged == 2) break;
    }
    if (quadrant & 2) sum = -sum;
    return negative ? -sum : sum;
}

} // namespace mb3d::numeric
#endif
