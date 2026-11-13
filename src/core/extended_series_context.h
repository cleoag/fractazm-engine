#pragma once
#include "core/extended80.h"

#if defined(MB3D_SOFTWARE_EXT80) || defined(MB3D_HAVE_SOFT_EXT80)
namespace mb3d::numeric {

// Stack-local snapshot for a pure arithmetic series. Construct inside the
// caller's ExtendedScope, and discard before changing the rounding mode or
// precision, leaving that scope, invoking callbacks, or changing threads.
// The flags reference stays live: operations accumulate the real thread-local
// flags, including flags raised by a fallback SoftFloat operation. This avoids
// looking up the same TLS addresses for every term; no global cache is used.
class ExtendedSeriesContext {
    const bool nearestEven80_ = !NativePrecisionEnabled() &&
                                softfloat_roundingMode == softfloat_round_near_even &&
                                extF80_roundingPrecision == 80;
    uint_fast8_t* flags_ = NativePrecisionEnabled() ? nullptr : &softfloat_exceptionFlags;
public:
    ExtendedSeriesContext() = default;
    ExtendedSeriesContext(const ExtendedSeriesContext&) = delete;
    ExtendedSeriesContext& operator=(const ExtendedSeriesContext&) = delete;
    bool nearestEven80() const { return nearestEven80_; }
    void raiseInexact() const { if (flags_) *flags_ |= softfloat_flag_inexact; }
};

} // namespace mb3d::numeric
#endif
