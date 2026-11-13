#pragma once
#include <cstdint>

// Native-only boundary inputs shared by the CPU oracle test and Metal driver.
// The existing exact_arithmetic kernel remains the device execution path.
namespace mb3d_metal_exact {
constexpr std::uint64_t normal_addition_boundary_pairs = 1910048;
constexpr std::uint64_t normal_addition_boundary_checks = 2 * normal_addition_boundary_pairs;
constexpr std::uint64_t normal_addition_boundary_swapped_checks = 1795360;
constexpr std::uint64_t normal_multiplication_boundary_checks = 1182464;
constexpr std::uint64_t normal_multiplication_halfway_checks = 224;

// emit(a,b,reversed) requests BOTH add and subtract. Covers every binary64 exponent,
// both signs, GRS alignment boundaries, cancellation to subnormal/zero, the
// fast-domain endpoints, infinity and both signalling/quiet NaN encodings.
template<class Emit>
std::uint64_t for_each_normal_addition_pair(Emit&& emit) {
    constexpr std::uint64_t mask = 0x000fffffffffffffULL;
    constexpr std::uint64_t fractions[]{0,1,2,0x0007ffffffffffffULL,
        0x0008000000000000ULL,0x000ffffffffffffeULL,mask};
    constexpr unsigned gaps[]{0,1,2,52,53,54,63,64,65};
    std::uint64_t count = 0;
    for (unsigned exponent = 0; exponent <= 2047; ++exponent)
        for (unsigned gap : gaps) {
            if (gap > exponent) continue;
            for (std::uint64_t fraction : fractions)
                for (unsigned pattern = 0; pattern < 2; ++pattern)
                    for (unsigned signs = 0; signs < 4; ++signs) {
                        const std::uint64_t a = (std::uint64_t(exponent) << 52) |
                            fraction | (std::uint64_t(signs & 1) << 63);
                        const std::uint64_t b = (std::uint64_t(exponent-gap) << 52) |
                            (pattern ? mask-fraction : fraction) |
                            (std::uint64_t(signs >> 1) << 63);
                        emit(a,b,false);
                        ++count;
                        if (gap > 0) {
                            emit(b,a,true);
                            ++count;
                        }
                    }
        }
    return count;
}

// emit(a,b,halfway) requests multiplication. Correlated exponents straddle the normal
// product fast path and IEEE underflow/overflow edges. Fractions include both
// neighbours of sqrt(2) to cross the product normalization boundary. Both
// operand orders exercise the distinct power-of-two shortcuts.
template<class Emit>
std::uint64_t for_each_normal_multiplication_pair(Emit&& emit) {
    constexpr std::uint64_t mask = 0x000fffffffffffffULL;
    constexpr std::uint64_t fractions[]{0,1,2,0x0006a09e667f3bccULL,
        0x0006a09e667f3bcdULL,0x0007ffffffffffffULL,
        0x0008000000000000ULL,0x000ffffffffffffeULL,mask};
    constexpr int result_exponents[]{0,1,2,2043,2044,2045,2046,2047};
    std::uint64_t count = 0;
    for (unsigned exponent = 0; exponent <= 2047; ++exponent)
        for (int result : result_exponents) {
            const int other = result + 1023 - int(exponent);
            if (other < 0 || other > 2047) continue;
            for (std::uint64_t fraction : fractions)
                for (unsigned pattern = 0; pattern < 2; ++pattern)
                    for (unsigned signs = 0; signs < 4; ++signs) {
                        const std::uint64_t a = (std::uint64_t(exponent) << 52) |
                            fraction | (std::uint64_t(signs & 1) << 63);
                        const std::uint64_t b = (std::uint64_t(other) << 52) |
                            (pattern ? mask-fraction : fraction) |
                            (std::uint64_t(signs >> 1) << 63);
                        emit(a,b,false);emit(b,a,false);
                        count += 2;
                    }
        }
    // Exact halfway products: retained even/odd mantissas, below/above two,
    // both signs/orders, and exponents across the fast-path result boundaries.
    constexpr unsigned halfway_exponents[]{1,2,1023,2043,2044,2045,2046};
    constexpr std::uint64_t halfway_fractions[]{1,3,0x0008000000000002ULL,
        0x0008000000000006ULL};
    for (unsigned exponent : halfway_exponents)
        for (std::uint64_t fraction : halfway_fractions)
            for (unsigned signs = 0; signs < 4; ++signs) {
                const std::uint64_t a = 0x3ff8000000000000ULL |
                    (std::uint64_t(signs & 1) << 63);
                const std::uint64_t b = (std::uint64_t(exponent) << 52) |
                    fraction | (std::uint64_t(signs >> 1) << 63);
                emit(a,b,true);emit(b,a,true);
                count += 2;
            }
    return count;
}
} // namespace mb3d_metal_exact
