#include "core/sse_estimates.h"
#include <cfenv>
#include <cstdio>
#include <limits>

// Independent hardware capture, pinned upstream revision (data fingerprints,
// no upstream implementation copied):
// https://gist.github.com/AndrewBelt/eea2759c9e99a7b900ccba417726bd4b
// revision afa37381f1f884206d1190f21cd258fa2b3be409, soft_rcp_rsqrt.cpp.
// FNV-1a over little-endian uint32 results in ascending input-bucket order.
static uint64_t fingerprint(bool rsqrt, uint32_t exponent, unsigned buckets) {
    uint64_t h = 14695981039346656037ULL;
    for (unsigned i = 0; i < buckets; ++i) {
        const float x = std::bit_cast<float>((exponent << 23) | (i * (8388608 / buckets)));
        const uint32_t bits = std::bit_cast<uint32_t>(rsqrt
            ? mb3d::numeric::softwareReciprocalSqrtEstimate(x)
            : mb3d::numeric::softwareReciprocalEstimate(x));
        for (unsigned j = 0; j < 4; ++j) h = (h ^ ((bits >> (8*j)) & 255)) * 1099511628211ULL;
    }
    return h;
}
int main() {
    using namespace mb3d::numeric;
    int checks = 0, failures = 0;
    auto check = [&](bool pass, const char* what) {
        ++checks;
        if (!pass) { ++failures; std::fprintf(stderr, "FAIL: %s\n", what); }
    };
    // Hash constants are independent of the midpoint implementation.
    check(fingerprint(false,127,2048) == 0x15a527651af59360ULL, "RCPSS normalized hardware capture");
    check(fingerprint(true,127,1024) == 0x6e09fc05d2e99350ULL, "RSQRTSS even exponent hardware capture");
    check(fingerprint(true,128,1024) == 0xf0bbe6ad5cf6a131ULL, "RSQRTSS odd exponent hardware capture");
    struct Special { uint32_t input, rcp, rsqrt; };
    for (const auto& v : {
        Special{0x00000000,0x7f800000,0x7f800000}, {0x80000000,0xff800000,0xff800000},
        {0x00000001,0x7f800000,0x7f800000}, {0x807fffff,0xff800000,0xff800000},
        {0x7f800000,0x00000000,0x00000000}, {0xff800000,0x80000000,0xffc00000},
        {0x7f800001,0x7fc00001,0x7fc00001}, {0xffc12345,0xffc12345,0xffc12345},
        {0x3f800000,0x3f7ff000,0x3f7ff000}, {0xbf800000,0xbf7ff000,0xffc00000}}) {
        check(std::bit_cast<uint32_t>(softwareReciprocalEstimate(std::bit_cast<float>(v.input))) == v.rcp, "RCPSS special value");
        check(std::bit_cast<uint32_t>(softwareReciprocalSqrtEstimate(std::bit_cast<float>(v.input))) == v.rsqrt, "RSQRTSS special value");
    }
    const int saved = std::fegetround();
    for (const int rounding : {FE_TONEAREST, FE_DOWNWARD, FE_UPWARD, FE_TOWARDZERO}) {
        std::fesetround(rounding);
        check(fingerprint(false,127,2048) == 0x15a527651af59360ULL, "RCPSS independent of rounding mode");
        // Boundaries of every bucket and every normal exponent. Low mantissa
        // bits cannot influence an estimate; results have 12 significant bits.
        for (unsigned e = 1; e < 255; ++e) {
            for (unsigned i = 0; i < 2048; ++i) {
                const uint32_t input = (e<<23) | (i<<12);
                auto r = [&](uint32_t b) { return std::bit_cast<uint32_t>(softwareReciprocalEstimate(std::bit_cast<float>(b))); };
                check(r(input) == r(input|4095), "RCPSS bucket endpoints");
                check(r(input|0x80000000u) == (r(input)|0x80000000u), "RCPSS signed magnitude");
                if (!(i&1)) {
                    auto s = [&](uint32_t b) { return std::bit_cast<uint32_t>(softwareReciprocalSqrtEstimate(std::bit_cast<float>(b))); };
                    check(s(input) == s(input|8191), "RSQRTSS bucket endpoints");
                }
            }
        }
    }
    std::fesetround(saved);
    std::printf("SSE estimates: %d checks, %d failures\n", checks, failures);
    return failures ? 1 : 0;
}
