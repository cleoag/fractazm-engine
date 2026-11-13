#pragma once

// Compatible arithmetic preserves the x87 64-bit significand and exponent
// range up to 16384, including hosts whose long double is binary64. Native
// arithmetic keeps the same translated operations with hardware binary64.
// Neither profile executes x87 machine code.
#if defined(MB3D_SOFTWARE_EXT80) || defined(MB3D_HAVE_SOFT_EXT80)
#include "core/numeric_precision.h"
#include <bit>
#include <cstdint>
#include <cstddef>
#include <cmath>
#include <limits>
extern "C" {
// Match the C library's TLS ABI. C++ thread_local can request a dynamic-
// initialization wrapper that upstream's C _Thread_local does not emit.
#if defined(_MSC_VER)
#define THREAD_LOCAL __declspec(thread)
#else
#define THREAD_LOCAL __thread
#endif
#include "softfloat.h"
#undef THREAD_LOCAL
}

namespace mb3d::numeric {
static_assert(offsetof(extFloat80_t, signif) == 0 && offsetof(extFloat80_t, signExp) == 8,
              "SoftFloat C and C++ must agree on the little-endian ext80 layout");
class ExtendedScope {
    bool active_ = !NativePrecisionEnabled();
    uint_fast8_t rounding_ = 0, precision_ = 0, flags_ = 0;
public:
    ExtendedScope() {
        if (!active_) return;
        rounding_ = softfloat_roundingMode;
        precision_ = extF80_roundingPrecision;
        flags_ = softfloat_exceptionFlags;
        softfloat_roundingMode = softfloat_round_near_even;
        extF80_roundingPrecision = 80;
        softfloat_exceptionFlags = 0;
    }
    ~ExtendedScope() {
        if (!active_) return;
        softfloat_roundingMode = rounding_;
        extF80_roundingPrecision = precision_;
        softfloat_exceptionFlags = flags_;
    }
    ExtendedScope(const ExtendedScope&) = delete;
    ExtendedScope& operator=(const ExtendedScope&) = delete;
};

class Extended80 {
    union Storage {
        extFloat80_t extended;
        double native;
        explicit Storage(extFloat80_t value) : extended(value) {}
        explicit Storage(double value) : native(value) {}
    } value_;
    bool native_;
    explicit Extended80(extFloat80_t value) : value_(value), native_(false) {}
    Extended80(double value, bool) : value_(value), native_(true) {}

    // Exact storage conversions, with no SoftFloat calls or state changes.
    // Objects may outlive the profile scope in which they were constructed.
    static extFloat80_t encodeNative(double value) {
        const uint64_t bits = std::bit_cast<uint64_t>(value);
        const uint16_t sign = uint16_t((bits >> 48) & 0x8000);
        const unsigned exponent = unsigned((bits >> 52) & 0x7ff);
        const uint64_t fraction = bits & UINT64_C(0x000fffffffffffff);
        extFloat80_t out{};
        if (exponent == 0x7ff) {
            out.signExp = sign | 0x7fff;
            out.signif = UINT64_C(0x8000000000000000) | (fraction << 11);
            if (fraction) out.signif |= UINT64_C(0x4000000000000000);
        } else if (exponent) {
            out.signExp = uint16_t(sign | (exponent + 16383 - 1023));
            out.signif = (fraction | UINT64_C(0x0010000000000000)) << 11;
        } else if (fraction) {
            const int shift = std::countl_zero(fraction);
            out.signExp = uint16_t(sign | (16383 - 1011 - shift));
            out.signif = fraction << shift;
        } else out.signExp = sign;
        return out;
    }
    static double decodeNative(extFloat80_t value) {
        const uint64_t sign = uint64_t(value.signExp & 0x8000) << 48;
        const unsigned exponent = value.signExp & 0x7fff;
        uint64_t significand = value.signif;
        if (exponent == 0x7fff) {
            if (significand == UINT64_C(0x8000000000000000))
                return std::bit_cast<double>(sign | UINT64_C(0x7ff0000000000000));
            return std::bit_cast<double>(sign | UINT64_C(0x7ff8000000000000) |
                ((significand >> 11) & UINT64_C(0x000fffffffffffff)));
        }
        if (!significand) return std::bit_cast<double>(sign);
        int binaryExponent = int(exponent ? exponent : 1) - 16383;
        const int normalize = std::countl_zero(significand);
        significand <<= normalize;
        binaryExponent -= normalize;
        if (binaryExponent > 1023)
            return std::bit_cast<double>(sign | UINT64_C(0x7ff0000000000000));
        const bool subnormal = binaryExponent < -1022;
        const unsigned shift = unsigned(11 + (subnormal ? -1022 - binaryExponent : 0));
        uint64_t rounded = 0;
        if (shift < 64) {
            rounded = significand >> shift;
            const uint64_t remainder = significand & ((UINT64_C(1) << shift) - 1);
            const uint64_t half = UINT64_C(1) << (shift - 1);
            rounded += remainder > half || (remainder == half && (rounded & 1));
        } else if (shift == 64) {
            rounded = significand > UINT64_C(0x8000000000000000);
        }
        if (subnormal) return std::bit_cast<double>(sign | rounded);
        if (rounded == UINT64_C(0x0020000000000000)) {
            rounded >>= 1;
            ++binaryExponent;
        }
        if (binaryExponent > 1023)
            return std::bit_cast<double>(sign | UINT64_C(0x7ff0000000000000));
        return std::bit_cast<double>(sign | (uint64_t(binaryExponent + 1023) << 52) |
            (rounded & UINT64_C(0x000fffffffffffff)));
    }
    extFloat80_t extendedValue() const {
        return native_ ? encodeNative(value_.native) : value_.extended;
    }
    double nativeValue() const {
        return native_ ? value_.native : decodeNative(value_.extended);
    }
    static Extended80 nativeResult(double value) { return Extended80(value, true); }
    static int32_t nativeInteger(double value, bool nearest) {
        value = nearest ? std::nearbyint(value) : std::trunc(value);
        if (!(value >= -2147483648.0 && value < 2147483648.0)) return INT32_MIN;
        return static_cast<int32_t>(value);
    }
public:
    Extended80() : Extended80(0.0) {}
    Extended80(double value) : value_(value), native_(NativePrecisionEnabled()) {
        if (!native_) value_.extended = f64_to_extF80({std::bit_cast<uint64_t>(value)});
    }
    static Extended80 fromBits(uint16_t exponent, uint64_t significand) {
        extFloat80_t v{}; v.signExp = exponent; v.signif = significand;
        return NativePrecisionEnabled() ? nativeResult(decodeNative(v)) : Extended80(v);
    }
    uint16_t signExponent() const { return extendedValue().signExp; }
    uint64_t significand() const { return extendedValue().signif; }
    explicit operator double() const {
        return native_ || NativePrecisionEnabled() ? nativeValue()
            : std::bit_cast<double>(extF80_to_f64(value_.extended).v);
    }
    explicit operator float() const {
        return NativePrecisionEnabled() ? static_cast<float>(nativeValue())
            : std::bit_cast<float>(extF80_to_f32(extendedValue()).v);
    }
    explicit operator int32_t() const {
        return NativePrecisionEnabled() ? nativeInteger(nativeValue(), false)
            : extF80_to_i32(extendedValue(), softfloat_round_minMag, false);
    }
    int32_t integer() const {
        return NativePrecisionEnabled() ? nativeInteger(nativeValue(), true)
            : extF80_to_i32(extendedValue(), softfloat_round_near_even, false);
    }
    friend Extended80 operator+(Extended80 a, Extended80 b) {
        return NativePrecisionEnabled() ? nativeResult(a.nativeValue() + b.nativeValue())
            : Extended80(extF80_add(a.extendedValue(), b.extendedValue()));
    }
    friend Extended80 operator-(Extended80 a, Extended80 b) {
        return NativePrecisionEnabled() ? nativeResult(a.nativeValue() - b.nativeValue())
            : Extended80(extF80_sub(a.extendedValue(), b.extendedValue()));
    }
    friend Extended80 operator*(Extended80 a, Extended80 b) {
        return NativePrecisionEnabled() ? nativeResult(a.nativeValue() * b.nativeValue())
            : Extended80(extF80_mul(a.extendedValue(), b.extendedValue()));
    }
    friend Extended80 operator/(Extended80 a, Extended80 b) {
        return NativePrecisionEnabled() ? nativeResult(a.nativeValue() / b.nativeValue())
            : Extended80(extF80_div(a.extendedValue(), b.extendedValue()));
    }
    Extended80 operator-() const {
        if (NativePrecisionEnabled()) return nativeResult(-nativeValue());
        auto v = extendedValue(); v.signExp ^= 0x8000; return Extended80(v);
    }
    Extended80& operator+=(Extended80 b) { return *this = *this + b; }
    Extended80& operator-=(Extended80 b) { return *this = *this - b; }
    Extended80& operator*=(Extended80 b) { return *this = *this * b; }
    Extended80& operator/=(Extended80 b) { return *this = *this / b; }
    friend bool operator<(Extended80 a, Extended80 b) {
        return NativePrecisionEnabled() ? a.nativeValue() < b.nativeValue()
            : extF80_lt_quiet(a.extendedValue(), b.extendedValue());
    }
    friend bool operator==(Extended80 a, Extended80 b) {
        return NativePrecisionEnabled() ? a.nativeValue() == b.nativeValue()
            : extF80_eq(a.extendedValue(), b.extendedValue());
    }
    friend bool operator>(Extended80 a, Extended80 b) { return b < a; }
    friend bool operator<=(Extended80 a, Extended80 b) { return a < b || a == b; }
    friend bool operator>=(Extended80 a, Extended80 b) { return b <= a; }
    friend bool operator!=(Extended80 a, Extended80 b) { return !(a == b); }
    friend Extended80 sqrt(Extended80 a) {
        return NativePrecisionEnabled() ? nativeResult(std::sqrt(a.nativeValue()))
            : Extended80(extF80_sqrt(a.extendedValue()));
    }
    friend Extended80 abs(Extended80 a) {
        if (NativePrecisionEnabled()) return nativeResult(std::fabs(a.nativeValue()));
        auto v = a.extendedValue(); v.signExp &= 0x7fff; return Extended80(v);
    }
    friend Extended80 round(Extended80 a) {
        return NativePrecisionEnabled() ? nativeResult(std::nearbyint(a.nativeValue()))
            : Extended80(extF80_roundToInt(a.extendedValue(), softfloat_round_near_even, false));
    }
    friend Extended80 trunc(Extended80 a) {
        return NativePrecisionEnabled() ? nativeResult(std::trunc(a.nativeValue()))
            : Extended80(extF80_roundToInt(a.extendedValue(), softfloat_round_minMag, false));
    }
};
}
#endif

// Selected formula translations share their operation order on every host.
// Only Apple ARM64 substitutes software extended precision.
#include <cmath>
namespace mb3d::numeric {
#ifdef MB3D_SOFTWARE_EXT80
using X87Real = Extended80;
using X87Scope = ExtendedScope;
inline X87Real x87Abs(X87Real x) { return abs(x); }
inline X87Real x87Sqrt(X87Real x) { return sqrt(x); }
inline X87Real x87Round(X87Real x) { return round(x); }
inline X87Real x87Trunc(X87Real x) { return trunc(x); }
inline X87Real x87Pi() { return Extended80::fromBits(0x4000, 0xc90fdaa22168c235ULL); }
#else
using X87Real = long double;
struct X87Scope {};
inline X87Real x87Abs(X87Real x) { return std::fabs(x); }
inline X87Real x87Sqrt(X87Real x) { return std::sqrt(x); }
inline X87Real x87Round(X87Real x) { return std::nearbyint(x); }
inline X87Real x87Trunc(X87Real x) { return std::trunc(x); }
inline X87Real x87Pi() { return 3.141592653589793238462643383279502884L; }
#endif
}
