#include "core/extended_small_division.h"
#include <array>
#include <cstdio>
#include <limits>

namespace {
using mb3d::numeric::Extended80;
using mb3d::numeric::extendedDivideSmall;

struct Differential {
    uint64_t checks = 0;
    uint64_t failures = 0;

    void check(uint16_t signExponent, uint64_t significand, uint32_t divisor,
               uint_fast8_t mode = softfloat_round_near_even,
               uint_fast8_t precision = 80,
               uint_fast8_t tininess = softfloat_tininess_afterRounding,
               uint_fast8_t initialFlags = 0) {
        const extFloat80_t input{significand, signExponent};
        softfloat_roundingMode = mode;
        extF80_roundingPrecision = precision;
        softfloat_detectTininess = tininess;
        // Every uint32_t is exactly representable as both binary64 and ext80.
        const auto denominator = f64_to_extF80({std::bit_cast<uint64_t>(static_cast<double>(divisor))});
        softfloat_exceptionFlags = initialFlags;
        const auto expected = extF80_div(input, denominator);
        const auto expectedFlags = softfloat_exceptionFlags;
        const mb3d::numeric::ExtendedSeriesContext context;
        for (const bool cached : {false, true}) {
            softfloat_exceptionFlags = initialFlags;
            const auto inputValue = Extended80::fromBits(signExponent, significand);
            const auto actual = cached ? extendedDivideSmall(inputValue, divisor, context)
                                       : extendedDivideSmall(inputValue, divisor);
            ++checks;
            if (actual.signExponent() != expected.signExp ||
                actual.significand() != expected.signif ||
                softfloat_exceptionFlags != expectedFlags ||
                softfloat_roundingMode != mode || extF80_roundingPrecision != precision ||
                softfloat_detectTininess != tininess) {
                if (failures++ < 12)
                    std::fprintf(stderr,
                        "FAIL: %04x:%016llx / %u mode=%u precision=%u tiny=%u cached=%d "
                        "flags=%u => %04x:%016llx flags=%u; expected %04x:%016llx flags=%u\n",
                        unsigned(signExponent), static_cast<unsigned long long>(significand),
                        divisor, unsigned(mode), unsigned(precision), unsigned(tininess), cached,
                        unsigned(initialFlags), unsigned(actual.signExponent()),
                        static_cast<unsigned long long>(actual.significand()),
                        unsigned(softfloat_exceptionFlags), unsigned(expected.signExp),
                        static_cast<unsigned long long>(expected.signif), unsigned(expectedFlags));
            }
        }
    }
};

uint64_t nextRandom(uint64_t& state) {
    // Fixed integer seed and recurrence make failures reproducible on any host.
    state ^= state << 13;
    state ^= state >> 7;
    state ^= state << 17;
    return state;
}
} // namespace

int main() {
    mb3d::numeric::ExtendedScope scope;
    const auto savedTininess = softfloat_detectTininess;
    Differential test;
    constexpr uint64_t integerBit = UINT64_C(0x8000000000000000);
    constexpr uint64_t maximum = std::numeric_limits<uint64_t>::max();

    // Exercise every supported denominator, both signs, boundaries of the
    // normalized significand and every power-of-two transition in the divisor.
    constexpr std::array<uint64_t, 12> significands{
        integerBit, integerBit + 1, integerBit + 2, integerBit + 3,
        UINT64_C(0xaaaaaaaaaaaaaaaa), UINT64_C(0xaaaaaaaaaaaaaaab),
        UINT64_C(0xbfffffffffffffff), UINT64_C(0xc000000000000000),
        UINT64_C(0xc000000000000001), maximum - 2, maximum - 1, maximum};
    constexpr std::array<uint16_t, 6> normalExponents{
        11, 12, 0x3f00, 0x3fff, 0x4000, 0x7ffe};
    uint64_t random = UINT64_C(0x7a3b561a4df29c87);
    for (uint32_t divisor = 1; divisor <= 1024; ++divisor) {
        for (const auto exponent : normalExponents)
            for (const auto significand : significands)
                for (const uint16_t sign : {uint16_t(0), uint16_t(0x8000)})
                    test.check(exponent | sign, significand, divisor,
                               softfloat_round_near_even, 80,
                               softfloat_tininess_afterRounding,
                               softfloat_flag_invalid | softfloat_flag_underflow);
        for (unsigned i = 0; i < 128; ++i) {
            const auto significand = nextRandom(random) | integerBit;
            const uint16_t exponent = 11 + nextRandom(random) % (0x7fff - 11);
            test.check(exponent | ((i & 1) ? 0x8000 : 0), significand, divisor);
        }
    }

    // Subnormals, underflow boundaries, zeros, infinities, signaling/quiet NaNs
    // and noncanonical encodings go through the unchanged upstream operation.
    // Alternate rounding/precision modes and sticky flags must also be kept.
    constexpr std::array<uint16_t, 16> boundaryExponents{
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 0x3fff, 0x7ffe, 0x7fff};
    constexpr std::array<uint64_t, 10> boundarySignificands{
        0, 1, 2, integerBit - 1, integerBit, integerBit + 1,
        UINT64_C(0xa000000000000001), UINT64_C(0xc000000000000000),
        maximum - 1, maximum};
    constexpr std::array<uint32_t, 18> boundaryDivisors{
        0, 1, 2, 3, 4, 5, 7, 8, 31, 63, 79, 255, 930, 1023, 1024, 1025,
        65536, std::numeric_limits<uint32_t>::max()};
    constexpr std::array<uint_fast8_t, 6> modes{
        softfloat_round_near_even, softfloat_round_minMag, softfloat_round_min,
        softfloat_round_max, softfloat_round_near_maxMag, softfloat_round_odd};
    for (const auto exponent : boundaryExponents)
        for (const auto significand : boundarySignificands)
            for (const auto divisor : boundaryDivisors)
                for (const auto mode : modes)
                    for (const uint_fast8_t precision : {32, 64, 80})
                        for (const uint_fast8_t tininess : {
                                 softfloat_tininess_beforeRounding,
                                 softfloat_tininess_afterRounding})
                            for (const uint16_t sign : {uint16_t(0), uint16_t(0x8000)})
                                test.check(exponent | sign, significand, divisor,
                                           mode, precision, tininess,
                                           (divisor & 1) ? softfloat_flag_infinite : 0);

    // Reuse one context across changing operands and sticky flags, including
    // fallback operations, as the production recurrences do. Flags are live
    // state, not a value captured when the context is constructed.
    softfloat_roundingMode = softfloat_round_near_even;
    extF80_roundingPrecision = 80;
    softfloat_exceptionFlags = 0;
    const mb3d::numeric::ExtendedSeriesContext context;
    for (const auto exponent : boundaryExponents)
        for (const auto significand : boundarySignificands)
            for (const auto divisor : boundaryDivisors) {
                const auto before = softfloat_exceptionFlags;
                const extFloat80_t input{significand, exponent};
                const auto denominator = f64_to_extF80({std::bit_cast<uint64_t>(static_cast<double>(divisor))});
                const auto expected = extF80_div(input, denominator);
                const auto expectedFlags = softfloat_exceptionFlags;
                softfloat_exceptionFlags = before;
                const auto actual = extendedDivideSmall(
                    Extended80::fromBits(exponent, significand), divisor, context);
                ++test.checks;
                if (actual.signExponent() != expected.signExp ||
                    actual.significand() != expected.signif ||
                    softfloat_exceptionFlags != expectedFlags) {
                    if (test.failures++ < 12) std::fprintf(stderr, "FAIL: reused division context\n");
                }
            }
    softfloat_detectTininess = savedTininess;
    std::printf("Extended small integer division: %llu differential cases, %llu failures\n",
                static_cast<unsigned long long>(test.checks),
                static_cast<unsigned long long>(test.failures));
    return test.failures ? 1 : 0;
}
