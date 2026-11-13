#include "core/numerical_de.h"
#include "core/numeric_precision.h"

#include <bit>
#include <cmath>
#include <cstddef>
#include <cstdint>

#if defined(MB3D_SOFTWARE_NUMERICAL_DE)
extern "C" {
#if defined(_MSC_VER)
#define THREAD_LOCAL __declspec(thread)
#else
#define THREAD_LOCAL __thread
#endif
#include "softfloat.h"
#undef THREAD_LOCAL
}

namespace mb3d {
namespace {

// This translation unit deliberately does not use MB3D_SOFTWARE_EXT80: enabling
// that profile globally on Windows would change unrelated formula kernels.
class ArithmeticScope {
    uint_fast8_t rounding_ = softfloat_roundingMode;
    uint_fast8_t precision_ = extF80_roundingPrecision;
    uint_fast8_t flags_ = softfloat_exceptionFlags;
public:
    ArithmeticScope() {
        softfloat_roundingMode = softfloat_round_near_even;
        extF80_roundingPrecision = 80;
        softfloat_exceptionFlags = 0;
    }
    ~ArithmeticScope() {
        softfloat_roundingMode = rounding_;
        extF80_roundingPrecision = precision_;
        softfloat_exceptionFlags = flags_;
    }
    ArithmeticScope(const ArithmeticScope&) = delete;
    ArithmeticScope& operator=(const ArithmeticScope&) = delete;
};

class Extended {
    extFloat80_t value_;
    explicit Extended(extFloat80_t value) : value_(value) {}
public:
    explicit Extended(double value)
        : value_(f64_to_extF80({std::bit_cast<uint64_t>(value)})) {}
    static Extended bits(uint16_t exponent, uint64_t significand) {
        extFloat80_t value{};
        value.signExp = exponent;
        value.signif = significand;
        return Extended(value);
    }
    uint16_t exponent() const { return value_.signExp; }
    uint64_t significand() const { return value_.signif; }
    explicit operator double() const {
        return std::bit_cast<double>(extF80_to_f64(value_).v);
    }
    friend Extended operator+(Extended a, Extended b) {
        return Extended(extF80_add(a.value_, b.value_));
    }
    friend Extended operator-(Extended a, Extended b) {
        return Extended(extF80_sub(a.value_, b.value_));
    }
    friend Extended operator*(Extended a, Extended b) {
        return Extended(extF80_mul(a.value_, b.value_));
    }
    friend Extended operator/(Extended a, Extended b) {
        return Extended(extF80_div(a.value_, b.value_));
    }
    friend bool operator==(Extended a, Extended b) {
        return extF80_eq(a.value_, b.value_);
    }
    friend bool operator>(Extended a, Extended b) {
        return extF80_lt_quiet(b.value_, a.value_);
    }
};

Extended logarithmTwo() {
    return Extended::bits(0x3ffe, UINT64_C(0xb17217f7d1cf79ac));
}

// Same reduction, ascending series and stopping rule as extendedLog2. The
// private SoftFloat operators also work with MSVC, without native int128.
Extended logarithmBaseTwo(Extended value) {
    const Extended zero(0), one(1), two(2);
    const unsigned exponentBits = value.exponent() & 0x7fff;
    if (exponentBits == 0x7fff) {
        if (value.significand() != UINT64_C(0x8000000000000000))
            return value + zero; // quiet the original NaN, retaining its payload
        if (value.exponent() & 0x8000)
            return Extended::bits(0xffff, UINT64_C(0xc000000000000000));
        return value;
    }
    if (value == zero)
        return Extended::bits(0xffff, UINT64_C(0x8000000000000000));
    if (!(value > zero))
        return Extended::bits(0xffff, UINT64_C(0xc000000000000000));

    int exponent = int(exponentBits) - 16383;
    uint64_t significand = value.significand();
    if (exponentBits == 0) {
        const int shift = std::countl_zero(significand);
        significand <<= shift;
        exponent = 1 - 16383 - shift;
    }
    Extended mantissa = Extended::bits(0x3fff, significand);
    if (mantissa > Extended(1.5)) {
        mantissa = mantissa / two;
        ++exponent;
    }
    const Extended t = (mantissa - one) / (mantissa + one);
    const Extended square = t * t;
    Extended term = t, sum = t;
    for (int k = 1; k < 32; ++k) {
        term = term * square;
        const Extended next = sum + term / Extended(2 * k + 1);
        if (next == sum) break;
        sum = next;
    }
    return Extended(exponent) + (two * sum) / logarithmTwo();
}

bool isNaNBits(uint64_t value) {
    return (value & UINT64_C(0x7fffffffffffffff)) > UINT64_C(0x7ff0000000000000);
}
double quietNaN(uint64_t value) {
    return std::bit_cast<double>(value | UINT64_C(0x0008000000000000));
}
double indefinite() {
    return std::bit_cast<double>(UINT64_C(0xfff8000000000000));
}

// Original ADDSD/SQRTSD propagate an input NaN, but produce Intel's negative
// indefinite for invalid arithmetic without a NaN operand. ARM's default
// invalid NaN has a different sign, which is observable in the frozen corpus.
double addDouble(double a, double b) {
    const auto ab = std::bit_cast<uint64_t>(a);
    const auto bb = std::bit_cast<uint64_t>(b);
    if (isNaNBits(ab)) return quietNaN(ab);
    if (isNaNBits(bb)) return quietNaN(bb);
    const double result = a + b;
    return std::isnan(result) ? indefinite() : result;
}
double squareRootDouble(double value) {
    const auto bits = std::bit_cast<uint64_t>(value);
    if (isNaNBits(bits)) return quietNaN(bits);
    const double result = std::sqrt(value);
    return std::isnan(result) ? indefinite() : result;
}

} // namespace

double NumericalDistanceEstimate(double radius, double dxSquared,
                                 double dySquared, double dzSquared,
                                 double scale, double offset,
                                 double* denominatorOut) {
    if (NativePrecisionEnabled()) {
        const double denominator = std::sqrt((dzSquared + dySquared) + dxSquared) + offset;
        if (denominatorOut) *denominatorOut = denominator;
        return radius * std::log(radius) * scale / denominator;
    }
    const ArithmeticScope scope;
    const double sum = addDouble(addDouble(dzSquared, dySquared), dxSquared);
    const double denominator = addDouble(squareRootDouble(sum), offset);
    if (denominatorOut) *denominatorOut = denominator;

    const Extended logarithm = logarithmTwo() * logarithmBaseTwo(Extended(radius));
    const Extended numerator = (logarithm * Extended(radius)) * Extended(scale);
    return double(numerator / Extended(denominator));
}

} // namespace mb3d
#else
namespace mb3d {
double NumericalDistanceEstimate(double radius, double dxSquared,
                                 double dySquared, double dzSquared,
                                 double scale, double offset,
                                 double* denominatorOut) {
    const double denominator = std::sqrt((dzSquared + dySquared) + dxSquared) + offset;
    if (denominatorOut) *denominatorOut = denominator;
    return radius * std::log(radius) * scale / denominator;
}
} // namespace mb3d
#endif
