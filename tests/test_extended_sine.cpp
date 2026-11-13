#include "core/extended_sine.h"

#include <cstdint>
#include <cstdio>
#include <initializer_list>

namespace {
using E = mb3d::numeric::Extended80;

bool sameBits(E a, E b) {
    return a.signExponent() == b.signExponent() && a.significand() == b.significand();
}

// The pre-optimization gnarlyIFS scalar FSIN path. The unchanged two-series
// implementation is the differential oracle, including its reduction and
// deliberate scalar/paired out-of-range distinction.
E previousScalarSine(E angle) {
    const auto exponent = angle.signExponent() & 0x7fff;
    return exponent >= 16383 + 63 && exponent < 0x7fff
        ? angle : mb3d::numeric::extendedSinCos(angle).sin;
}
}

int main() {
    using namespace mb3d::numeric;
    ExtendedScope scope;
    uint64_t checks = 0, failures = 0;
    auto check = [&](E x) {
        const E expected = previousScalarSine(x), actual = extendedSin(x);
        ++checks;
        if (!sameBits(actual, expected)) {
            if (failures < 8) {
                std::fprintf(stderr,
                    "sine mismatch input=%04x:%016llx expected=%04x:%016llx actual=%04x:%016llx\n",
                    unsigned(x.signExponent()), static_cast<unsigned long long>(x.significand()),
                    unsigned(expected.signExponent()), static_cast<unsigned long long>(expected.significand()),
                    unsigned(actual.signExponent()), static_cast<unsigned long long>(actual.significand()));
            }
            ++failures;
        }
    };
    auto bothSigns = [&](uint16_t exponent, uint64_t significand) {
        check(E::fromBits(exponent, significand));
        check(E::fromBits(exponent | 0x8000, significand));
    };

    // Signed zeros, minimum/maximum subnormal, infinity and quiet/signaling
    // NaN payloads. These compare the complete ext80 representation.
    for (uint64_t sig : {UINT64_C(0), UINT64_C(1), UINT64_C(2),
                         UINT64_C(0x7fffffffffffffff)}) bothSigns(0, sig);
    for (uint64_t sig : {UINT64_C(0x8000000000000000), UINT64_C(0x8000000000000001),
                         UINT64_C(0xc000000000000000), UINT64_C(0xffffffffffffffff)})
        bothSigns(0x7fff, sig);

    // Every normal exponent, including the reduction threshold, the scalar
    // 2^63 boundary, extreme finite values, and underflowed series terms.
    for (unsigned exponent = 1; exponent < 0x7fff; ++exponent) {
        bothSigns(uint16_t(exponent), UINT64_C(0x8000000000000000));
        bothSigns(uint16_t(exponent), UINT64_C(0xffffffffffffffff));
    }

    // Quadrant/reflection boundaries and immediate ext80 neighbours.
    for (int multiple = -128; multiple <= 128; ++multiple) {
        const E center = x87Pi() * E(multiple) / E(4);
        const uint64_t sig = center.significand();
        for (int offset = -4; offset <= 4; ++offset) {
            if (offset < 0 && sig < uint64_t(-offset)) continue;
            const uint64_t neighbour = sig + offset;
            if ((center.signExponent() & 0x7fff) != 0 &&
                !(neighbour & UINT64_C(0x8000000000000000))) continue;
            check(E::fromBits(center.signExponent(), neighbour));
        }
    }

    // Deterministic dense samples in the finite reduction range, with no
    // dependence on a host random generator or platform libm.
    uint64_t random = UINT64_C(0x9e3779b97f4a7c15);
    auto next = [&] {
        random ^= random >> 12; random ^= random << 25; random ^= random >> 27;
        return random * UINT64_C(0x2545f4914f6cdd1d);
    };
    for (int i = 0; i < 32768; ++i) {
        const uint16_t exponent = uint16_t(16383 - 80 + next() % 144);
        bothSigns(exponent, next() | UINT64_C(0x8000000000000000));
    }
    // Early termination is restricted to the formula's nearest-even/80
    // environment. Exercise the fixed-length fallback in other supported
    // SoftFloat precision and rounding modes, using the same bitwise oracle.
    for (uint_fast8_t precision : {uint_fast8_t(32), uint_fast8_t(64), uint_fast8_t(80)}) {
        extF80_roundingPrecision = precision;
        for (uint_fast8_t mode : {uint_fast8_t(softfloat_round_near_even),
                                 uint_fast8_t(softfloat_round_minMag),
                                 uint_fast8_t(softfloat_round_min),
                                 uint_fast8_t(softfloat_round_max),
                                 uint_fast8_t(softfloat_round_near_maxMag)}) {
            softfloat_roundingMode = mode;
            for (int i = 0; i < 256; ++i)
                bothSigns(uint16_t(16383 - 20 + next() % 30),
                          next() | UINT64_C(0x8000000000000000));
        }
    }
    softfloat_roundingMode = softfloat_round_near_even;
    extF80_roundingPrecision = 80;
    std::printf("Extended scalar sine: %llu bit-exact checks, %llu failures\n",
                static_cast<unsigned long long>(checks), static_cast<unsigned long long>(failures));
    return failures ? 1 : 0;
}
