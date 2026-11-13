#include "core/extended_product.h"
#include <array>
#include <cstdio>
#include <limits>

namespace {
using mb3d::numeric::Extended80;
using mb3d::numeric::extendedProduct;

struct Differential {
    uint64_t checks = 0;
    uint64_t failures = 0;

    void check(uint16_t exponentA, uint64_t significandA,
               uint16_t exponentB, uint64_t significandB,
               uint_fast8_t mode = softfloat_round_near_even,
               uint_fast8_t precision = 80,
               uint_fast8_t tininess = softfloat_tininess_afterRounding,
               uint_fast8_t initialFlags = 0) {
        const extFloat80_t a{significandA, exponentA}, b{significandB, exponentB};
        softfloat_roundingMode = mode;
        extF80_roundingPrecision = precision;
        softfloat_detectTininess = tininess;
        softfloat_exceptionFlags = initialFlags;
        const auto expected = extF80_mul(a, b);
        const auto expectedFlags = softfloat_exceptionFlags;
        const mb3d::numeric::ExtendedSeriesContext context;
        for (const bool cached : {false, true}) {
            softfloat_exceptionFlags = initialFlags;
            const auto valueA = Extended80::fromBits(exponentA, significandA);
            const auto valueB = Extended80::fromBits(exponentB, significandB);
            const auto actual = cached ? extendedProduct(valueA, valueB, context)
                                       : extendedProduct(valueA, valueB);
            ++checks;
            if (actual.signExponent() != expected.signExp ||
                actual.significand() != expected.signif ||
                softfloat_exceptionFlags != expectedFlags ||
                softfloat_roundingMode != mode || extF80_roundingPrecision != precision ||
                softfloat_detectTininess != tininess) {
                if (failures++ < 12)
                    std::fprintf(stderr,
                        "FAIL: %04x:%016llx * %04x:%016llx mode=%u precision=%u tiny=%u cached=%d "
                        "flags=%u => %04x:%016llx flags=%u; expected %04x:%016llx flags=%u\n",
                        unsigned(exponentA), static_cast<unsigned long long>(significandA),
                        unsigned(exponentB), static_cast<unsigned long long>(significandB),
                        unsigned(mode), unsigned(precision), unsigned(tininess), cached,
                        unsigned(initialFlags), unsigned(actual.signExponent()),
                        static_cast<unsigned long long>(actual.significand()),
                        unsigned(softfloat_exceptionFlags), unsigned(expected.signExp),
                        static_cast<unsigned long long>(expected.signif), unsigned(expectedFlags));
            }
        }
    }
};

uint64_t nextRandom(uint64_t& state) {
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
    uint64_t random = UINT64_C(0xa194e730b6c2538f);

    // Every normal exponent in both operands, random full-width significands,
    // all four signs, and products crossing underflow/overflow boundaries.
    // Mirrored exponents separately keep extreme inputs' products ordinary.
    for (unsigned exponent = 1; exponent < 0x7fff; ++exponent) {
        for (unsigned selector = 0; selector < 4; ++selector) {
            const unsigned otherExponent = selector == 0 ? 1 : selector == 1 ? 0x7ffe :
                                           selector == 2 ? 0x3fff : 0x7fff - exponent;
            const uint64_t significandA = nextRandom(random) | integerBit;
            const uint64_t significandB = nextRandom(random) | integerBit;
            for (unsigned signs = 0; signs < 4; ++signs)
                test.check(uint16_t(exponent | ((signs & 1) ? 0x8000 : 0)), significandA,
                           uint16_t(otherExponent | ((signs & 2) ? 0x8000 : 0)), significandB,
                           softfloat_round_near_even, 80, softfloat_tininess_afterRounding,
                           softfloat_flag_invalid | softfloat_flag_underflow);
        }
    }

    // Exact halfway residuals in both normalization branches: an odd low
    // significand multiplied by 1.5 in the first branch, or 1.5 multiplied by
    // a significand congruent to 2 mod 4 in the second. Neighbours straddle
    // the halfway boundary and alternate retained-significand parity.
    constexpr uint64_t oneAndHalf = UINT64_C(0xc000000000000000);
    for (unsigned low = 1; low < 512; low += 2) {
        for (int neighbour = -1; neighbour <= 1; ++neighbour) {
            const uint64_t factor = oneAndHalf + neighbour;
            test.check(0x3fff, integerBit + low, 0x3fff, factor);
            test.check(0xbfff, factor, 0x3fff, integerBit + low);
        }
    }
    for (unsigned low = 2; low < 1024; low += 4)
        for (int neighbour = -1; neighbour <= 1; ++neighbour)
            test.check(0x3fff, oneAndHalf, 0x3fff, oneAndHalf + low + neighbour);

    // This product rounds through a significand carry into the next binade.
    test.check(0x3fff, integerBit + 1, 0x3fff, maximum - 1);
    test.check(0x7ffe, integerBit + 1, 0x3fff, maximum - 1);

    // Zero/subnormal/noncanonical/NaN/Inf encodings, exponent limits,
    // precision reduction, every rounding mode and both tininess policies.
    constexpr std::array<uint16_t, 10> boundaryExponents{
        0, 1, 2, 3, 0x3ffe, 0x3fff, 0x4000, 0x7ffd, 0x7ffe, 0x7fff};
    constexpr std::array<uint64_t, 12> boundarySignificands{
        0, 1, integerBit - 1, integerBit, integerBit + 1, integerBit + 3,
        oneAndHalf - 1, oneAndHalf, oneAndHalf + 1, maximum - 2, maximum - 1, maximum};
    constexpr std::array<uint_fast8_t, 6> modes{
        softfloat_round_near_even, softfloat_round_minMag, softfloat_round_min,
        softfloat_round_max, softfloat_round_near_maxMag, softfloat_round_odd};
    for (const auto exponentA : boundaryExponents)
        for (const auto exponentB : boundaryExponents)
            for (const auto significandA : boundarySignificands)
                for (const auto significandB : boundarySignificands)
                    for (const auto mode : modes)
                        for (const uint_fast8_t precision : {32, 64, 80})
                            for (const uint_fast8_t tininess : {
                                     softfloat_tininess_beforeRounding,
                                     softfloat_tininess_afterRounding})
                                for (unsigned signs = 0; signs < 4; ++signs)
                                    test.check(exponentA | ((signs & 1) ? 0x8000 : 0),
                                               significandA,
                                               exponentB | ((signs & 2) ? 0x8000 : 0),
                                               significandB, mode, precision, tininess,
                                               (signs & 1) ? softfloat_flag_infinite : 0);

    // One cached context spans normal and fallback products while their
    // exception flags accumulate; it must refer to the live TLS flag byte.
    softfloat_roundingMode = softfloat_round_near_even;
    extF80_roundingPrecision = 80;
    softfloat_exceptionFlags = 0;
    const mb3d::numeric::ExtendedSeriesContext context;
    for (const auto exponentA : boundaryExponents)
        for (const auto exponentB : boundaryExponents)
            for (const auto significandA : boundarySignificands)
                for (const auto significandB : boundarySignificands) {
                    const auto before = softfloat_exceptionFlags;
                    const extFloat80_t a{significandA, exponentA}, b{significandB, exponentB};
                    const auto expected = extF80_mul(a, b);
                    const auto expectedFlags = softfloat_exceptionFlags;
                    softfloat_exceptionFlags = before;
                    const auto actual = extendedProduct(
                        Extended80::fromBits(exponentA, significandA),
                        Extended80::fromBits(exponentB, significandB), context);
                    ++test.checks;
                    if (actual.signExponent() != expected.signExp ||
                        actual.significand() != expected.signif ||
                        softfloat_exceptionFlags != expectedFlags) {
                        if (test.failures++ < 12) std::fprintf(stderr, "FAIL: reused product context\n");
                    }
                }
    softfloat_detectTininess = savedTininess;
    std::printf("Extended normal product: %llu differential cases, %llu failures\n",
                static_cast<unsigned long long>(test.checks),
                static_cast<unsigned long long>(test.failures));
    return test.failures ? 1 : 0;
}
