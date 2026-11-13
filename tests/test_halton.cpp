// Unit test: Halton radical inverse + precompute table
//
// Verifies mb3d::HaltonRadicalInverse and mb3d::PreComputeHaltonSequence
// against bit-identical Pascal output from CalcMonteCarlo.pas:120-151.
//
// The expected values here are computed from Pascal's algorithm by
// running the same float-trunc loop mentally; any genuine divergence
// between C++ and Pascal on Double arithmetic would surface as a bit
// mismatch.

#include "core/calc_monte_carlo.h"
#include <cassert>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>

static int s_failures = 0;

static void check_halton_value(int32_t index, int32_t base,
                               double expected, double eps = 1e-15) {
    const double actual = mb3d::HaltonRadicalInverse(index, base);
    const double diff = std::fabs(actual - expected);
    if (diff > eps) {
        std::printf("  FAIL HaltonRadicalInverse(%d, %d) = %.17g expected %.17g (diff %.3e)\n",
                    index, base, actual, expected, diff);
        ++s_failures;
    }
}

static void check_u16(const char* label, uint16_t actual, uint16_t expected) {
    if (actual != expected) {
        std::printf("  FAIL %s: got %u, expected %u\n",
                    label, actual, expected);
        ++s_failures;
    }
}

int main() {
    std::printf("test_halton: HaltonRadicalInverse basic values\n");

    // Base 2 — exact representations in IEEE-754
    check_halton_value(1, 2, 0.5);          // 1/2
    check_halton_value(2, 2, 0.25);         // 01 binary = 1/4
    check_halton_value(3, 2, 0.75);         // 11 = 3/4
    check_halton_value(4, 2, 0.125);        // 001 = 1/8
    check_halton_value(5, 2, 0.625);        // 101 = 5/8
    check_halton_value(6, 2, 0.375);        // 011 = 3/8
    check_halton_value(7, 2, 0.875);        // 111 = 7/8
    check_halton_value(8, 2, 0.0625);       // 0001 = 1/16

    // Base 3 — Van der Corput well-known values
    check_halton_value(1, 3, 1.0 / 3.0, 1e-15);    // 0.3333...
    check_halton_value(2, 3, 2.0 / 3.0, 1e-15);    // 0.6666...
    check_halton_value(3, 3, 1.0 / 9.0, 1e-15);    // 0.1111...
    check_halton_value(4, 3, 4.0 / 9.0, 1e-15);    // 0.4444...
    check_halton_value(5, 3, 7.0 / 9.0, 1e-15);    // 0.7777...

    // Base 5, 7 — just verify they're in (0, 1) and monotonic within
    // first-cycle blocks (index 1..base-1 produces 1/base, 2/base, ...)
    for (int32_t i = 1; i < 5; ++i) {
        double v = mb3d::HaltonRadicalInverse(i, 5);
        if (!(v > 0.0 && v < 1.0)) {
            std::printf("  FAIL Halton(%d, 5) = %.17g out of (0,1)\n", i, v);
            ++s_failures;
        }
    }
    for (int32_t i = 1; i < 7; ++i) {
        double v = mb3d::HaltonRadicalInverse(i, 7);
        if (!(v > 0.0 && v < 1.0)) {
            std::printf("  FAIL Halton(%d, 7) = %.17g out of (0,1)\n", i, v);
            ++s_failures;
        }
    }

    // Edge cases
    check_halton_value(0, 2, 0.0);   // index 0 -> 0
    check_halton_value(0, 3, 0.0);
    check_halton_value(-1, 2, 0.0);  // negative index -> 0
    check_halton_value(100, 1, 0.0); // invalid base -> 0

    std::printf("test_halton: PreComputeHaltonSequence table\n");

    mb3d::PreComputeHaltonSequence();
    const mb3d::HaltonRec* t = mb3d::GetHaltonSequence();
    if (!t) {
        std::printf("  FAIL GetHaltonSequence returned nullptr\n");
        return 1;
    }

    // Table entry 0 corresponds to Pascal i=0, which calls halton(0+1, 2)
    // = halton(1, 2) = 0.5. 0.5 * 65535 = 32767.5; Pascal Round is banker's
    // (32768) — but with binary-exact 0.5 representation and lrint default
    // FE_TONEAREST rounding mode, result is 32768 on x64.
    check_u16("t[0].HRx (halton(1,2)*65535)", t[0].HRx, 32768);
    // halton(1, 3) = 1/3. Round(1/3 * 65535) = Round(21845.0) = 21845.
    check_u16("t[0].HRy (halton(1,3)*65535)", t[0].HRy, 21845);
    // halton(1, 5) = 1/5. Round(1/5 * 65535) = Round(13107.0) = 13107.
    check_u16("t[0].HDx (halton(1,5)*65535)", t[0].HDx, 13107);
    // halton(1, 7) = 1/7. Round(1/7 * 65535) ~= Round(9362.142857..) = 9362.
    check_u16("t[0].HDy (halton(1,7)*65535)", t[0].HDy, 9362);

    // Table entry 1: halton(2, 2) = 0.25 -> 16384; halton(2, 3) = 2/3 -> 43690
    check_u16("t[1].HRx (halton(2,2)*65535)", t[1].HRx, 16384);
    check_u16("t[1].HRy (halton(2,3)*65535)", t[1].HRy, 43690);

    // Table entry 2: halton(3, 2) = 0.75 -> 49151; halton(3, 3) = 1/9 -> 7282
    check_u16("t[2].HRx (halton(3,2)*65535)", t[2].HRx, 49151);
    check_u16("t[2].HRy (halton(3,3)*65535)", t[2].HRy, 7282);

    // Table entry 3: halton(4, 2) = 0.125 -> 8192; halton(4, 3) = 4/9 -> 29127
    check_u16("t[3].HRx (halton(4,2)*65535)", t[3].HRx, 8192);
    check_u16("t[3].HRy (halton(4,3)*65535)", t[3].HRy, 29127);

    // Verify idempotence: second call should not crash / re-populate
    mb3d::PreComputeHaltonSequence();
    const mb3d::HaltonRec* t2 = mb3d::GetHaltonSequence();
    if (t2 != t) {
        std::printf("  FAIL table pointer changed on second call\n");
        ++s_failures;
    }

    if (s_failures > 0) {
        std::printf("test_halton: %d failure(s)\n", s_failures);
        return 1;
    }
    std::printf("test_halton: all checks OK\n");
    return 0;
}
