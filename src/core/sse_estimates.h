#pragma once
#include <array>
#include <bit>
#include <cstdint>
#if defined(__SSE__) || defined(_M_X64)
#include <xmmintrin.h>
#endif

namespace mb3d::numeric {
// Intel's legacy SSE estimate profile, qualified against independent published
// hardware captures (test_sse_estimates.cpp). SSE permits implementation-specific
// approximations: this is a named compatibility profile, not an IEEE operation.
// Generate bucket midpoints using integers, without double rounding or a host
// floating-point environment dependency. Only the small seed tables are stored.
constexpr uint32_t reciprocalSeed(unsigned i) {
    const uint64_t n = 8192ULL * 4096, d = 4097 + 2 * i;
    return static_cast<uint32_t>((n + d / 2) / d);
}
constexpr uint32_t reciprocalSqrtSeed(unsigned i, bool odd) {
    const uint64_t n = 8192ULL * 8192 * 2048;
    const uint64_t d = (2049 + 2 * i) * (odd ? 2 : 1);
    uint64_t lo = 4096, hi = 8192;
    while (lo + 1 < hi) {
        const uint64_t mid = (lo + hi) / 2;
        if (mid * mid * d <= n) lo = mid; else hi = mid;
    }
    const uint64_t midpoint = d * (2 * lo + 1) * (2 * lo + 1);
    return static_cast<uint32_t>(lo + (4 * n > midpoint || (4 * n == midpoint && (lo & 1))));
}
inline constexpr auto reciprocalSeeds = [] {
    std::array<uint16_t, 2048> a{};
    for (unsigned i = 0; i < a.size(); ++i) a[i] = reciprocalSeed(i);
    return a;
}();
inline constexpr auto reciprocalSqrtSeeds = [] {
    std::array<std::array<uint16_t, 1024>, 2> a{};
    for (unsigned p = 0; p < 2; ++p)
        for (unsigned i = 0; i < a[p].size(); ++i) a[p][i] = reciprocalSqrtSeed(i, p != 0);
    return a;
}();
inline float softwareReciprocalEstimate(float x) {
    const uint32_t bits = std::bit_cast<uint32_t>(x), sign = bits & 0x80000000u;
    const unsigned exp = (bits >> 23) & 255, frac = bits & 0x7fffff;
    uint32_t result;
    if (exp == 255) result = frac ? bits | 0x00400000u : sign;
    else if (exp == 0) result = sign | 0x7f800000u;
    else if (exp >= 253) result = sign; // Underflow is flushed by RCPSS.
    else result = sign | ((253 - exp) << 23) | ((reciprocalSeeds[frac >> 12] - 4096) << 11);
    return std::bit_cast<float>(result);
}
inline float softwareReciprocalSqrtEstimate(float x) {
    const uint32_t bits = std::bit_cast<uint32_t>(x), sign = bits & 0x80000000u;
    const unsigned exp = (bits >> 23) & 255, frac = bits & 0x7fffff;
    uint32_t result;
    if (exp == 255 && frac) result = bits | 0x00400000u;
    else if (exp == 0) result = sign | 0x7f800000u;
    else if (sign) result = 0xffc00000u;
    else if (exp == 255) result = 0;
    else {
        const int unbiased = static_cast<int>(exp) - 127;
        const int odd = (unbiased % 2 + 2) % 2;
        result = ((126 - (unbiased - odd) / 2) << 23)
               | ((reciprocalSqrtSeeds[odd][frac >> 13] - 4096) << 11);
    }
    return std::bit_cast<float>(result);
}
inline float reciprocalEstimate(float x) {
#if defined(__SSE__) || defined(_M_X64)
    return _mm_cvtss_f32(_mm_rcp_ss(_mm_set_ss(x)));
#else
    return softwareReciprocalEstimate(x);
#endif
}
inline float reciprocalSqrtEstimate(float x) {
#if defined(__SSE__) || defined(_M_X64)
    return _mm_cvtss_f32(_mm_rsqrt_ss(_mm_set_ss(x)));
#else
    return softwareReciprocalSqrtEstimate(x);
#endif
}
}
