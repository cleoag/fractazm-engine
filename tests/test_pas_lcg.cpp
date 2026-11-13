// Unit tests for the C++ PasLcg helper (src/core/pas_lcg.h).
// Mirrors PasLcg.pas (patches/PasLcg.pas) byte for byte.
// Contract: src/core/pas_lcg.h and patches/PasLcg.pas.
#include "core/pas_lcg.h"

#include <cstdint>
#include <cstdio>
#include <cstring>

static int g_passed = 0;
static int g_failed = 0;

#define CHECK(cond, msg) do { \
    if (cond) { g_passed++; } \
    else { g_failed++; std::printf("FAIL: %s (line %d)\n", msg, __LINE__); } \
} while(0)

#define CHECK_EQ_U32(a, b, msg) do { \
    uint32_t _a = (a), _b = (b); \
    if (_a == _b) { g_passed++; } \
    else { g_failed++; std::printf("FAIL: %s: got 0x%08X, expected 0x%08X (line %d)\n", \
        msg, _a, _b, __LINE__); } \
} while(0)

#define CHECK_EQ_DOUBLE_BITS(a, b, msg) do { \
    double _a = (a), _b = (b); \
    uint64_t _ab, _bb; \
    static_assert(sizeof(double) == sizeof(uint64_t), "double must be 64 bits"); \
    std::memcpy(&_ab, &_a, sizeof(_ab)); \
    std::memcpy(&_bb, &_b, sizeof(_bb)); \
    if (_ab == _bb) { g_passed++; } \
    else { g_failed++; std::printf("FAIL: %s: got %.17g (0x%016llX), expected %.17g (0x%016llX) (line %d)\n", \
        msg, _a, (unsigned long long)_ab, _b, (unsigned long long)_bb, __LINE__); } \
} while(0)

// Case 1: PasLcgInitFromThread(0) == 1 * 0x324594A1 + 0x24563487
static void test_init_from_thread_zero() {
    uint32_t expected = 0x324594A1u + 0x24563487u;
    CHECK_EQ_U32(mb3d::PasLcgInitFromThread(0), expected,
                 "PasLcgInitFromThread(0)");
}

// Case 2: PasLcgInitFromThread(1) == 2 * 0x324594A1 + 0x24563487
static void test_init_from_thread_one() {
    uint32_t expected = 2u * 0x324594A1u + 0x24563487u;
    CHECK_EQ_U32(mb3d::PasLcgInitFromThread(1), expected,
                 "PasLcgInitFromThread(1)");
}

// Case 3: PasLcgNext advances state — state=0 -> 2531011 -> 2531011*214013+2531011 (wrap)
static void test_next_advances_from_zero() {
    uint32_t state = 0u;
    mb3d::PasLcgNext(state);
    CHECK_EQ_U32(state, 2531011u, "PasLcgNext first advance from 0");
    uint32_t expected2 = 2531011u * 214013u + 2531011u; // uint32_t wrap
    mb3d::PasLcgNext(state);
    CHECK_EQ_U32(state, expected2, "PasLcgNext second advance");
}

// Case 4: PasLcgNext returns the NEW state, not the old one
static void test_next_returns_new_state() {
    uint32_t state = 0u;
    uint32_t returned = mb3d::PasLcgNext(state);
    CHECK_EQ_U32(returned, 2531011u, "PasLcgNext returns new state (not old)");
    CHECK_EQ_U32(returned, state, "PasLcgNext return value matches state");
}

// Case 5: PasLcgU01 advances first then samples — state=0 gives 9886/16777216
static void test_u01_advance_then_sample() {
    uint32_t state = 0u;
    double got = mb3d::PasLcgU01(state);
    // After advance, state == 2531011, then (2531011 >> 8) == 9886.
    // 9886.0 / 16777216.0 is an exact IEEE-754 Double division.
    double expected = static_cast<double>(9886u) / 16777216.0;
    CHECK_EQ_DOUBLE_BITS(got, expected, "PasLcgU01 state=0 first output");
    // And state must have advanced
    CHECK_EQ_U32(state, 2531011u, "PasLcgU01 advanced state");
}

// Case 6: PasLcgU01 output is always in [0.0, 1.0) — 1024 draws from thread 0
static void test_u01_range_1024_draws() {
    uint32_t state = mb3d::PasLcgInitFromThread(0);
    int inRange = 0;
    for (int i = 0; i < 1024; ++i) {
        double v = mb3d::PasLcgU01(state);
        if (v >= 0.0 && v < 1.0) {
            ++inRange;
        } else {
            std::printf("FAIL: PasLcgU01 out of range at i=%d: v=%.17g\n", i, v);
        }
    }
    CHECK(inRange == 1024, "PasLcgU01 1024 draws all in [0.0, 1.0)");
}

int main() {
    std::printf("Running PasLcg tests...\n\n");

    test_init_from_thread_zero();
    test_init_from_thread_one();
    test_next_advances_from_zero();
    test_next_returns_new_state();
    test_u01_advance_then_sample();
    test_u01_range_1024_draws();

    std::printf("\n========================================\n");
    std::printf("Results: %d passed, %d failed\n", g_passed, g_failed);
    std::printf("========================================\n");

    return g_failed > 0 ? 1 : 0;
}
