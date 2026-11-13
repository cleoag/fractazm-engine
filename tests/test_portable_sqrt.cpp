#include "portable_math.h"

#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <limits>

#ifndef PM_FORCE_PORTABLE_SQRT
#error This standalone target must compile portable_math.c with PM_FORCE_PORTABLE_SQRT.
#endif

static_assert(sizeof(double) == sizeof(uint64_t) &&
              std::numeric_limits<double>::is_iec559 &&
              std::numeric_limits<double>::digits == 53,
              "These fixtures require IEEE-754 binary64");

static double from_bits(uint64_t bits) {
    double value;
    std::memcpy(&value, &bits, sizeof(value));
    return value;
}

static uint64_t to_bits(double value) {
    uint64_t bits;
    std::memcpy(&bits, &value, sizeof(bits));
    return bits;
}

int main() {
    struct Fixture { uint64_t input; uint64_t expected; };
    // Independent constants: exact powers of two and 200-digit Decimal
    // square roots rounded to binary64, not a second call to host sqrt.
    // Includes both zeros, subnormal boundaries, extreme exponents, values
    // adjacent to one, ordinary irrational roots, and positive infinity.
    const Fixture fixtures[] = {
        {UINT64_C(0x0000000000000000), UINT64_C(0x0000000000000000)},
        {UINT64_C(0x8000000000000000), UINT64_C(0x8000000000000000)},
        {UINT64_C(0x0000000000000001), UINT64_C(0x1e60000000000000)},
        {UINT64_C(0x0000000000000002), UINT64_C(0x1e66a09e667f3bcd)},
        {UINT64_C(0x000fffffffffffff), UINT64_C(0x1fffffffffffffff)},
        {UINT64_C(0x0010000000000000), UINT64_C(0x2000000000000000)},
        {UINT64_C(0x0170000000000000), UINT64_C(0x20b0000000000000)},
        {UINT64_C(0x7e70000000000000), UINT64_C(0x5f30000000000000)},
        {UINT64_C(0x7fefffffffffffff), UINT64_C(0x5fefffffffffffff)},
        {UINT64_C(0x3fd0000000000000), UINT64_C(0x3fe0000000000000)},
        {UINT64_C(0x3fefffffffffffff), UINT64_C(0x3fefffffffffffff)},
        {UINT64_C(0x3ff0000000000000), UINT64_C(0x3ff0000000000000)},
        {UINT64_C(0x3ff0000000000001), UINT64_C(0x3ff0000000000000)},
        {UINT64_C(0x4000000000000000), UINT64_C(0x3ff6a09e667f3bcd)},
        {UINT64_C(0x4008000000000000), UINT64_C(0x3ffbb67ae8584caa)},
        {UINT64_C(0x4010000000000000), UINT64_C(0x4000000000000000)},
        {UINT64_C(0x4022000000000000), UINT64_C(0x4008000000000000)},
        {UINT64_C(0x7ff0000000000000), UINT64_C(0x7ff0000000000000)},
    };
    const uint64_t nan_inputs[] = {
        UINT64_C(0xbff0000000000000), // -1
        UINT64_C(0x8000000000000001), // negative smallest subnormal
        UINT64_C(0xffefffffffffffff), // negative largest finite
        UINT64_C(0xfff0000000000000), // -infinity
        UINT64_C(0x7ff8000000001234), // positive quiet NaN
        UINT64_C(0xfff8000000005678), // negative quiet NaN
    };

    int checks = 0;
    int failures = 0;
    for (const auto& fixture : fixtures) {
        const double input = from_bits(fixture.input);
        double out;
        pm_sqrt_out(input, &out);
        const uint64_t actual[] = {
            to_bits(pm_sqrt(input)), to_bits(out), pm_sqrt_bits(fixture.input)
        };
        for (int api = 0; api < 3; ++api) {
            ++checks;
            if (actual[api] != fixture.expected) {
                ++failures;
                std::printf("FAIL sqrt API %d input %016llx: %016llx != %016llx\n",
                            api, static_cast<unsigned long long>(fixture.input),
                            static_cast<unsigned long long>(actual[api]),
                            static_cast<unsigned long long>(fixture.expected));
            }
        }
    }
    for (uint64_t bits : nan_inputs) {
        const double input = from_bits(bits);
        double out;
        pm_sqrt_out(input, &out);
        const double actual[] = {pm_sqrt(input), out, from_bits(pm_sqrt_bits(bits))};
        for (int api = 0; api < 3; ++api) {
            ++checks;
            // NaN payload/sign and FP exception flags are separate contracts.
            if (!std::isnan(actual[api])) {
                ++failures;
                std::printf("FAIL sqrt API %d input %016llx: expected NaN\n",
                            api, static_cast<unsigned long long>(bits));
            }
        }
    }
    std::printf("Portable sqrt fallback: %d/%d checks passed\n", checks - failures, checks);
    return failures ? 1 : 0;
}
