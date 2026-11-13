#include "soft_double.h"
#include "normal_arithmetic_vectors.h"
#include <algorithm>
#include <array>
#include <bit>
#include <cmath>
#include <cstdio>
#include <limits>
#include <vector>
#include <cstdint>
using namespace mb3d_metal_exact;
namespace {
std::uint64_t checks = 0, failures = 0;
std::uint64_t sqrt_boundary_checks = 0, sqrt_prefix_checks = 0, sqrt_hint_checks = 0;
std::uint64_t sqrt_wide_boundary_checks = 0, sqrt_wide_prefix_checks = 0, sqrt_wide_hint_checks = 0;
std::uint64_t multiply_boundary_checks = 0;
std::uint64_t division_boundary_checks = 0, division_hint_checks = 0, division_digit_checks = 0;
std::uint64_t normal_add_checks = 0, normal_subtract_checks = 0, normal_swapped_checks = 0;
std::uint64_t normal_multiply_checks = 0, normal_halfway_checks = 0;
std::uint64_t state = 0xf34c019e572a6bd8ULL;
std::uint64_t next() { state ^= state >> 12; state ^= state << 25; state ^= state >> 27; return state * 2685821657736338717ULL; }
void check(const char* op, F64 actual, double expected, U64 a, U64 b = 0) {
    ++checks;
    bool ok = std::isnan(expected) ? isnan(actual) && actual.bits == 0x7ff8000000000000ULL : actual.bits == std::bit_cast<U64>(expected);
    if (!ok && failures++ < 20) std::fprintf(stderr, "%s %016llx %016llx got %016llx expected %016llx\n", op, a, b, actual.bits, std::bit_cast<U64>(expected));
}
void pair(U64 ab, U64 bb) {
    F64 a = F64::from_bits(ab), b = F64::from_bits(bb);
    volatile double da = a.to_double(), db = b.to_double();
    check("add", a + b, da + db, ab, bb); check("sub", a - b, da - db, ab, bb);
    check("mul", a * b, da * db, ab, bb); check("div", a / b, da / db, ab, bb);
    check("sqrt", sqrt(a), std::sqrt(da), ab);
    const bool expected[] = { da == db, da != db, da < db, da <= db, da > db, da >= db };
    const bool actual[] = { a == b, a != b, a < b, a <= b, a > b, a >= b };
    for (unsigned k = 0; k < 6; ++k) { ++checks; if (expected[k] != actual[k]) ++failures; }
    volatile float f = static_cast<float>(da);
    U32 fb = std::bit_cast<U32>(static_cast<float>(f));
    ++checks;
    if (std::isnan(f) ? a.to_f32_bits() != 0x7fc00000U : a.to_f32_bits() != fb) {
        if (failures++ < 20) std::fprintf(stderr, "to_f32 %016llx got %08x expected %08x\n", ab, a.to_f32_bits(), fb);
    }
    U32 input32 = U32(bb);
    volatile float f32 = std::bit_cast<float>(input32);
    check("from_f32", F64::from_f32_bits(input32), static_cast<double>(f32), input32);
    check("from_i32", F64::from_i32(std::bit_cast<int>(U32(ab))), double(std::bit_cast<int>(U32(ab))), ab);
    check("from_u32", F64::from_u32(U32(ab)), double(U32(ab)), ab);
    int expected_int = std::isfinite(da) && std::trunc(da) >= -2147483648.0 && std::trunc(da) <= 2147483647.0 ? int(da) : (-2147483647 - 1);
    ++checks; if (a.to_i32_rtz() != expected_int) ++failures;
}

void focused_normal_arithmetic_checks() {
    const auto addition_pairs = for_each_normal_addition_pair(
        [](std::uint64_t ab, std::uint64_t bb, bool reversed) {
            const F64 a = F64::from_bits(ab), b = F64::from_bits(bb);
            volatile double da = a.to_double(), db = b.to_double();
            check("normal_add_boundary", a + b, da + db, ab, bb); ++normal_add_checks;
            check("normal_subtract_boundary", a - b, da - db, ab, bb); ++normal_subtract_checks;
            if (reversed) normal_swapped_checks += 2;
        });
    const auto multiplication_pairs = for_each_normal_multiplication_pair(
        [](std::uint64_t ab, std::uint64_t bb, bool halfway) {
            const F64 a = F64::from_bits(ab), b = F64::from_bits(bb);
            volatile double da = a.to_double(), db = b.to_double();
            check("normal_multiply_boundary", a * b, da * db, ab, bb); ++normal_multiply_checks;
            if (halfway) ++normal_halfway_checks;
        });
    if (addition_pairs != normal_addition_boundary_pairs ||
        normal_add_checks != normal_addition_boundary_pairs ||
        normal_subtract_checks != normal_addition_boundary_pairs ||
        normal_swapped_checks != normal_addition_boundary_swapped_checks ||
        multiplication_pairs != normal_multiplication_boundary_checks ||
        normal_multiply_checks != normal_multiplication_boundary_checks ||
        normal_halfway_checks != normal_multiplication_halfway_checks) {
        ++failures;
        std::fprintf(stderr, "normal arithmetic boundary matrix denominator changed\n");
    }
}

void division_value(U64 ab, U64 bb) {
    const F64 a = F64::from_bits(ab), b = F64::from_bits(bb);
    volatile double da = a.to_double(), db = b.to_double();
    const double expected = da / db;
    check("division_boundary", a / b, expected, ab, bb);
    check("division_restoring", divide_restoring(a, b), expected, ab, bb);
    division_boundary_checks += 2;
    U32 actualSeed = 0;
    if (isfinite(b) && !iszero(b))
        actualSeed = std::bit_cast<U32>(524288.0f / float(unpack_finite_nonzero(b).sig));
    const U32 hints[]{0, 0x80000000U, 0x7f800000U, 0xff800000U, 0x7fc00001U,
        0x3f800000U, 0xbf800000U, 1, 0x7f7fffffU, actualSeed,
        actualSeed ? actualSeed - 1 : 0, actualSeed + 1};
    for (U32 hint : hints) {
        check("division_reciprocal_hint", divide_with_reciprocal_hint(a, b, std::bit_cast<float>(hint)),
              expected, ab, bb);
        ++division_hint_checks;
    }
}
void division_digit_case(U64 numerator, U64 denominator, unsigned shift) {
    using U128 = unsigned __int128;
    const bool domain = denominator >= (1ULL << 52) && denominator < (1ULL << 53) &&
        numerator < denominator * 2 && (shift == 18 || shift == 19);
    const U128 target = U128(numerator) << (shift < 64 ? shift : 0);
    const U64 quotient = domain ? U64(target / denominator) : 0;
    const U64 remainder = domain ? U64(target % denominator) : 0;
    const U64 hints[]{0, 1, quotient, quotient - 1, quotient + 1, quotient - 2,
        quotient + 2, quotient - 3, quotient + 3, (1ULL << 20) - 1, 1ULL << 20, ~U64(0)};
    for (U64 hint : hints) {
        const DivisionDigit actual = division_digit(numerator, denominator, shift, hint);
        const U64 distance = hint > quotient ? hint - quotient : quotient - hint;
        const bool accept = domain && hint < (1ULL << 20) && distance <= 2;
        const bool correct = actual.valid == accept && (accept ?
            actual.quotient == quotient && actual.remainder == remainder &&
            U128(actual.quotient) * denominator + actual.remainder == target &&
            actual.remainder < denominator : actual.quotient == 0 && actual.remainder == 0);
        ++checks; ++division_digit_checks;
        if (!correct && failures++ < 20)
            std::fprintf(stderr, "division_digit n=%016llx d=%016llx shift=%u hint=%016llx got=(%016llx,%016llx,%u) expected=(%016llx,%016llx,%u)\n",
                         numerator, denominator, shift, hint, actual.quotient, actual.remainder,
                         unsigned(actual.valid), quotient, remainder, unsigned(accept));
    }
}
void focused_division_checks() {
    // Every exponent/sign, including exceptional encodings, and divisors with
    // all significant bits set or near a power of two. Correlated exponents
    // add quotient transitions at underflow, normality and overflow.
    const U64 fractions[]{0, 1, 2, 0x0007ffffffffffffULL, 0x0008000000000000ULL,
        0x000ffffffffffffeULL, 0x000fffffffffffffULL};
    for (unsigned exponent = 0; exponent <= 2047; ++exponent)
        for (U64 fraction : fractions)
            for (U64 sign : {0ULL, 0x8000000000000000ULL}) {
                const U64 a = (U64(exponent) << 52) | fraction | sign;
                division_value(a, 0x3ff0000000000001ULL);
                division_value(a, 0xbfffffffffffffffULL);
            }
    for (int resultExponent : {-1075, -1074, -1023, -1022, 1023, 1024})
        for (unsigned numeratorExponent = 1; numeratorExponent < 2047; numeratorExponent += 17) {
            const int divisorExponent = int(numeratorExponent) - resultExponent;
            if (divisorExponent <= 0 || divisorExponent >= 2047) continue;
            for (U64 fraction : fractions)
                division_value((U64(numeratorExponent) << 52) | fraction,
                               (U64(divisorExponent) << 52) | (fraction ^ 0x000fffffffffffffULL));
        }
    for (unsigned i = 0; i < 2048; ++i) division_value(next(), next());
    const U64 denominators[]{0, 1, (1ULL << 52) - 1, 1ULL << 52, (1ULL << 52) + 1,
        (1ULL << 52) + 2, (3ULL << 51) - 1, 3ULL << 51, (1ULL << 53) - 2,
        (1ULL << 53) - 1, 1ULL << 53, ~U64(0)};
    for (U64 denominator : denominators)
        for (U64 numerator : {0ULL, 1ULL, denominator - 1, denominator,
                              denominator + 1, denominator * 2 - 1, denominator * 2, ~U64(0)})
            for (unsigned shift : {0U, 17U, 18U, 19U, 20U, 64U})
                division_digit_case(numerator, denominator, shift);
    for (unsigned i = 0; i < 4096; ++i) {
        const U64 denominator = (1ULL << 52) | (next() & ((1ULL << 52) - 1));
        const U64 numerator = next() % (denominator * 2);
        division_digit_case(numerator, denominator, 18);
        division_digit_case(numerator, denominator, 19);
    }
}

U64 power_two_bits(int exponent) {
    return exponent >= -1022 ? U64(exponent + 1023) << 52 : 1ULL << (exponent + 1074);
}

void multiply_boundary(U64 magnitude, U64 power) {
    for (unsigned signs = 0; signs < 4; ++signs) {
        U64 ab = magnitude | (U64(signs & 1) << 63), bb = power | (U64(signs >> 1) << 63);
        F64 a = F64::from_bits(ab), b = F64::from_bits(bb);
        volatile double da = a.to_double(), db = b.to_double();
        check("power_two_mul_boundary_right", a * b, da * db, ab, bb);
        check("power_two_mul_boundary_left", b * a, db * da, bb, ab);
        multiply_boundary_checks += 2;
    }
}

void focused_power_two_multiply_checks() {
    // Correlate the other operand's exponent with every power-of-two factor.
    // Product boundaries include half the smallest subnormal, the normal
    // transition and overflow. Native multiplication is the independent oracle.
    const U64 fractions[]{0, 1, 2, 3, 0x0007ffffffffffffULL,
        0x0008000000000000ULL, 0x0008000000000001ULL,
        0x000ffffffffffffeULL, 0x000fffffffffffffULL};
    for (int factor = -1074; factor <= 1023; ++factor) {
        U64 power = power_two_bits(factor);
        for (int product_exponent : {-1075, -1074, -1023, -1022, 1023, 1024}) {
            int operand_exponent = product_exponent - factor;
            if (operand_exponent < -1022 || operand_exponent > 1023) continue;
            for (U64 fraction : fractions)
                multiply_boundary((U64(operand_exponent + 1023) << 52) | fraction, power);
        }
    }
    // All 53 possible subnormal right shifts: exact halfway inputs and their
    // adjacent binary64 values. A second center changes the retained integer's
    // parity wherever both centers fit the same normalized binade.
    for (unsigned shift = 1; shift <= 53; ++shift) {
        constexpr U64 base = 1ULL << 52, limit = 1ULL << 53;
        const U64 first = ((base >> shift) << shift) | (1ULL << (shift - 1));
        for (unsigned parity = 0; parity < 2; ++parity) {
            U64 center = first + (U64(parity) << shift);
            if (center < base || center >= limit) continue;
            const U64 bits = 0x0010000000000000ULL + center - base;
            for (int offset = -1; offset <= 1; ++offset) {
                U64 adjacent = offset < 0 ? bits - U64(-offset) : bits + U64(offset);
                multiply_boundary(adjacent, power_two_bits(-int(shift)));
            }
        }
    }
    // Literal result bits independently pin representative ties, normal
    // crossover and overflow; all signs and operand orders remain explicit.
    const U64 exact[][3] = {
        {1, 0x3fe0000000000000ULL, 0},
        {3, 0x3fe0000000000000ULL, 2},
        {5, 0x3fe0000000000000ULL, 2},
        {7, 0x3fe0000000000000ULL, 4},
        {0x000fffffffffffffULL, 0x4000000000000000ULL, 0x001ffffffffffffeULL},
        {0x0010000000000001ULL, 0x3fe0000000000000ULL, 0x0008000000000000ULL},
        {0x0010000000000003ULL, 0x3fe0000000000000ULL, 0x0008000000000002ULL},
        {0x001fffffffffffffULL, 0x3fe0000000000000ULL, 0x0010000000000000ULL},
        {0x7fdfffffffffffffULL, 0x4000000000000000ULL, 0x7fefffffffffffffULL},
        {0x7fefffffffffffffULL, 0x4000000000000000ULL, 0x7ff0000000000000ULL}
    };
    for (const auto& row : exact) for (unsigned signs = 0; signs < 4; ++signs) {
        const U64 ab = row[0] | (U64(signs & 1) << 63), bb = row[1] | (U64(signs >> 1) << 63);
        const U64 expected = row[2] | (U64((signs & 1) ^ (signs >> 1)) << 63);
        F64 a = F64::from_bits(ab), b = F64::from_bits(bb);
        const double value = F64::from_bits(expected).to_double();
        check("power_two_mul_literal_right", a * b, value, ab, bb);
        check("power_two_mul_literal_left", b * a, value, bb, ab);
        multiply_boundary_checks += 2;
    }
}

U64 wide_high_for_value(F64 a) {
    if (!isfinite(a) || iszero(a) || signbit(a)) return 0;
    const Normalized n = unpack_finite_nonzero(a);
    return n.sig << (n.exponent % 2 != 0 ? 11 : 10);
}

U64 wide_hardware_hint(U64 high) {
    const float estimate = std::sqrt(float(high));
    return estimate >= 0.0f && estimate < 4294967296.0f
        ? U64(estimate) : 1ULL << 32;
}

void sqrt_wide_boundary(U64 bits) {
    F64 a = F64::from_bits(bits);
    volatile double value = a.to_double();
    check("sqrt_wide_boundary", sqrt_with_wide_hint(a, wide_hardware_hint(wide_high_for_value(a))),
          std::sqrt(value), bits);
    ++sqrt_wide_boundary_checks;
}

void sqrt_boundary(U64 bits) {
    F64 a = F64::from_bits(bits);
    volatile double value = a.to_double();
    check("sqrt_boundary", sqrt(a), std::sqrt(value), bits);
    ++sqrt_boundary_checks;
    // Exercise the diagnostic entry alongside production sqrt using every
    // exponent, subnormal and rounding vector.
    sqrt_wide_boundary(bits);
}

// Independent integer oracle for the high-limb prefix. Binary search does not
// use floating estimates, restoring-square-root code or production helpers.
U64 prefix_root_oracle(U64 high) {
    U64 lo = 0, hi = 1ULL << 24;
    while (lo + 1 < hi) {
        U64 mid = lo + (hi - lo) / 2;
        if (mid * mid <= high) lo = mid;
        else hi = mid;
    }
    return lo;
}

std::vector<U64> prefix_hints(U64 root) {
    std::vector<U64> hints{0, 1, root, (1ULL << 24) - 1,
                           1ULL << 24, (1ULL << 24) + 1, ~U64(0)};
    for (U64 distance = 1; distance <= 5; ++distance) {
        if (root >= distance) hints.push_back(root - distance);
        hints.push_back(root + distance);
    }
    std::sort(hints.begin(), hints.end());
    hints.erase(std::unique(hints.begin(), hints.end()), hints.end());
    return hints;
}

void sqrt_prefix(U64 high) {
    constexpr U64 high_limit = 1ULL << 48, hint_limit = 1ULL << 24;
    const bool in_domain = high < high_limit;
    const U64 root = in_domain ? prefix_root_oracle(high) : 0;
    const auto hints = prefix_hints(root);
    for (U64 hint : hints) {
        const auto prefix = sqrt_high_prefix(high, hint);
        const U64 distance = hint < root ? root - hint : hint - root;
        const bool expected_valid = in_domain && hint <= hint_limit && distance <= 4;
        const bool bracket = !prefix.valid ||
            (prefix.root == root && prefix.root * prefix.root <= high &&
             (prefix.root + 1) * (prefix.root + 1) > high &&
             prefix.remainder == high - prefix.root * prefix.root);
        ++checks; ++sqrt_prefix_checks;
        if ((!bracket || prefix.valid != expected_valid) && failures++ < 20)
            std::fprintf(stderr, "sqrt_prefix high=%016llx hint=%016llx root=%016llx remainder=%016llx valid=%u expected_root=%016llx valid=%u\n",
                         high, hint, prefix.root, prefix.remainder, unsigned(prefix.valid),
                         root, unsigned(expected_valid));
    }
    // The prefix domain used by binary64 sqrt is [2^46,2^48). Construct actual
    // inputs on either side of exponent parity, retaining low-significand
    // suffixes. An odd exponent doubles the original significand, so that
    // normalized radicand's least-significant bit must remain zero.
    if (high < (1ULL << 46) || !in_domain) return;
    const bool odd_exponent = high >= (1ULL << 47);
    for (U64 suffix : {0ULL, 1ULL, 2ULL, 31ULL, 32ULL, 62ULL, 63ULL}) {
        if (odd_exponent && (suffix & 1)) continue;
        const U64 sig = ((high << 6) | suffix) >> unsigned(odd_exponent);
        const U64 bits = (U64(odd_exponent ? 1024 : 1023) << 52) |
                         (sig & 0x000fffffffffffffULL);
        F64 a = F64::from_bits(bits);
        volatile double value = a.to_double();
        const double expected = std::sqrt(value);
        sqrt_boundary(bits);
        for (U64 hint : hints) {
            check("sqrt_hint", sqrt_with_hint(a, hint), expected, bits, hint);
            ++sqrt_hint_checks;
        }
    }
}

void focused_sqrt_checks() {
    const U64 fractions[]{0, 1, 2, 3, 0x0003ffffffffffffULL,
        0x0004000000000000ULL, 0x0007ffffffffffffULL,
        0x0008000000000000ULL, 0x0008000000000001ULL,
        0x000ffffffffffffdULL, 0x000ffffffffffffeULL, 0x000fffffffffffffULL};
    // Every exponent field, including subnormal and exceptional encodings.
    // Both signs exercise canonical NaNs and preservation of negative zero.
    for (unsigned exponent = 0; exponent <= 2047; ++exponent)
        for (U64 fraction : fractions)
            for (U64 sign : {0ULL, 0x8000000000000000ULL})
                sqrt_boundary((U64(exponent) << 52) | fraction | sign);

    // All possible subnormal leading-bit positions and nearby representable
    // values stress normalization before the prefix is extracted.
    for (unsigned bit = 0; bit < 52; ++bit) {
        const U64 center = 1ULL << bit;
        for (int offset = -2; offset <= 2; ++offset) {
            if (offset < 0 && center < U64(-offset)) continue;
            const U64 value = offset < 0 ? center - U64(-offset) : center + U64(offset);
            sqrt_boundary(value);
            sqrt_boundary(value | 0x8000000000000000ULL);
        }
    }

    // Inputs neighboring squares of representable numbers exercise final
    // output-rounding transitions. The independently rounded hardware product
    // locates the neighborhood; hardware sqrt supplies each exact-bit oracle.
    constexpr double significands[]{1.0, 1.0000000298023223876953125,
                                    1.25, 1.5, 1.75, 1.9999999701976776123046875};
    for (int exponent = -537; exponent <= 511; ++exponent)
        for (double significand : significands) {
            volatile double root = std::ldexp(significand, exponent);
            volatile double square = root * root;
            const U64 center = std::bit_cast<U64>(double(square));
            for (int offset = -2; offset <= 2; ++offset) {
                if (offset < 0 && center < U64(-offset)) continue;
                const U64 bits = offset < 0 ? center - U64(-offset) : center + U64(offset);
                if (bits < 0x7ff0000000000000ULL) sqrt_boundary(bits);
            }
        }

    // Prefix transitions require seeds above and below the correct floor.
    // Include invalid high limbs and seed values to verify bounded rejection.
    const U64 high_edges[]{0, 1, 2, 3, (1ULL << 46) - 1, 1ULL << 46,
        (1ULL << 46) + 1, (1ULL << 47) - 1, 1ULL << 47,
        (1ULL << 47) + 1, (1ULL << 48) - 1, 1ULL << 48,
        (1ULL << 48) + 1, ~U64(0)};
    for (U64 high : high_edges) sqrt_prefix(high);
    const U64 root_edges[]{0, 1, 2, 3, (1ULL << 23) - 1, 1ULL << 23,
        (1ULL << 23) + 1, (1ULL << 24) - 2, (1ULL << 24) - 1, 1ULL << 24};
    auto square_neighbors = [](U64 root) {
        const U64 center = root * root;
        for (int offset = -2; offset <= 2; ++offset) {
            if (offset < 0 && center < U64(-offset)) continue;
            sqrt_prefix(offset < 0 ? center - U64(-offset) : center + U64(offset));
        }
    };
    for (U64 root : root_edges) square_neighbors(root);
    for (unsigned i = 0; i < 512; ++i)
        square_neighbors((1ULL << 23) + (next() & ((1ULL << 23) - 1)));

    const U64 special[]{0, 0x8000000000000000ULL, 1, 0x8000000000000001ULL,
        0x0010000000000000ULL, 0x3ff0000000000000ULL, 0xbff0000000000000ULL,
        0x7fefffffffffffffULL, 0x7ff0000000000000ULL, 0xfff0000000000000ULL,
        0x7ff0000000000001ULL, 0x7ff8000000001234ULL, 0xfff923456789abcdULL};
    for (U64 bits : special) {
        F64 a = F64::from_bits(bits);
        volatile double value = a.to_double();
        for (U64 hint : {0ULL, 1ULL, 1ULL << 24, (1ULL << 24) + 1, ~U64(0)}) {
            check("sqrt_special_hint", sqrt_with_hint(a, hint), std::sqrt(value), bits, hint);
            ++sqrt_hint_checks;
        }
    }
}

U64 wide_prefix_root_oracle(U64 high) {
    // Native unsigned-128 arithmetic keeps the mathematical 2^64 endpoint
    // representable and is independent of the production residual bracket.
    using U128 = unsigned __int128;
    U64 lo = 0, hi = 1ULL << 32;
    while (lo + 1 < hi) {
        const U64 mid = lo + (hi - lo) / 2;
        if (U128(mid) * mid <= high) lo = mid;
        else hi = mid;
    }
    return lo;
}

std::vector<U64> wide_prefix_hints(U64 high) {
    const U64 root = wide_prefix_root_oracle(high);
    std::vector<U64> hints{0, 1, root, wide_hardware_hint(high), (1ULL << 32) - 1,
                          1ULL << 32, (1ULL << 32) + 1, ~U64(0)};
    for (U64 distance : {1ULL, 2ULL, 3ULL, 4ULL, 5ULL, 127ULL, 128ULL,
                         255ULL, 256ULL, 511ULL, 512ULL, 1023ULL, 1024ULL,
                         1025ULL, 2048ULL}) {
        if (root >= distance) hints.push_back(root - distance);
        hints.push_back(root + distance);
    }
    std::sort(hints.begin(), hints.end());
    hints.erase(std::unique(hints.begin(), hints.end()), hints.end());
    return hints;
}

void sqrt_wide_prefix_case(U64 high) {
    using U128 = unsigned __int128;
    constexpr U64 limit = 1ULL << 32;
    const U64 root = wide_prefix_root_oracle(high);
    const U64 hardwareHint = wide_hardware_hint(high);
    for (U64 hint : wide_prefix_hints(high)) {
        const SqrtPrefix actual = sqrt_wide_prefix(high, hint);
        const U64 distance = hint > root ? hint - root : root - hint;
        const bool mustAccept = hint == root || (hint == hardwareHint && hint < limit) ||
            (root >= (1ULL << 31) && hint < limit && distance <= 256);
        bool mustReject = hint >= limit || (!hint && high);
        if (hint && hint < limit) {
            const U64 square = hint * hint;
            const U64 magnitude = square > high ? square - high : high - square;
            // Far beyond the correction guard, with a factor-of-two margin
            // that cannot depend on binary32 rounding at the bound itself.
            mustReject |= magnitude > 4096 * hint;
        }
        const bool exact = actual.valid
            ? actual.root == root && U128(actual.root) * actual.root <= high &&
              U128(actual.root + 1) * (actual.root + 1) > high &&
              actual.remainder == high - actual.root * actual.root
            : actual.root == 0 && actual.remainder == 0;
        ++checks; ++sqrt_wide_prefix_checks;
        if ((!exact || (mustAccept && !actual.valid) || (mustReject && actual.valid)) &&
            failures++ < 20)
            std::fprintf(stderr, "sqrt_wide_prefix high=%016llx hint=%016llx got=(%016llx,%016llx,%u) root=%016llx must_accept=%u must_reject=%u\n",
                         high, hint, actual.root, actual.remainder, unsigned(actual.valid),
                         root, unsigned(mustAccept), unsigned(mustReject));
    }
}

void sqrt_wide_hint_case(U64 bits) {
    F64 a = F64::from_bits(bits);
    volatile double value = a.to_double();
    const double expected = std::sqrt(value);
    for (U64 hint : wide_prefix_hints(wide_high_for_value(a))) {
        check("sqrt_wide_hint", sqrt_with_wide_hint(a, hint), expected, bits, hint);
        ++sqrt_wide_hint_checks;
    }
}

void focused_wide_sqrt_checks() {
    const U64 highEdges[]{0, 1, 2, 3, 4, 15, 16, 17,
        (1ULL << 62) - 1, 1ULL << 62, (1ULL << 62) + 1,
        (1ULL << 63) - 1, 1ULL << 63, (1ULL << 63) + 1,
        ~U64(0) - 2048, ~U64(0) - 1024, ~U64(0) - 1, ~U64(0)};
    for (U64 high : highEdges) sqrt_wide_prefix_case(high);
    const U64 roots[]{0, 1, 2, 3, 4, 31, (1ULL << 16) - 1,
        1ULL << 16, (1ULL << 16) + 1, (1ULL << 24) - 1,
        1ULL << 24, (1ULL << 24) + 1, (1ULL << 31) - 1,
        1ULL << 31, (1ULL << 31) + 1, (1ULL << 32) - 2,
        (1ULL << 32) - 1, 1ULL << 32};
    auto squareNeighbors = [](U64 root) {
        using U128 = unsigned __int128;
        const U128 center = U128(root) * root;
        for (int offset = -2; offset <= 2; ++offset) {
            if (offset < 0 && center < U64(-offset)) continue;
            const U128 adjacent = offset < 0 ? center - U64(-offset) : center + U64(offset);
            if (adjacent <= ~U64(0)) sqrt_wide_prefix_case(U64(adjacent));
        }
        // Real normalized inputs have H divisible by 1024, or by 2048
        // after odd-exponent normalization. Neighbors of the mapped input
        // exercise both the prefix and the final rounding transitions.
        if (center >= (U128(1) << 62) && center <= ~U64(0)) {
            const U64 high = U64(center);
            const bool odd = high >= (1ULL << 63);
            const U64 sig = high >> (odd ? 11 : 10);
            const U64 bits = (U64(odd ? 1024 : 1023) << 52) |
                             (sig & 0x000fffffffffffffULL);
            for (int offset = -1; offset <= 1; ++offset)
                sqrt_wide_hint_case(offset < 0 ? bits - U64(-offset) : bits + U64(offset));
        }
    };
    for (U64 root : roots) squareNeighbors(root);
    for (unsigned i = 0; i < 512; ++i) squareNeighbors(next() & 0xffffffffULL);

    // Include prefixes away from perfect squares and complete binary64
    // values independently of the structured exponent/rounding matrix.
    for (unsigned i = 0; i < 512; ++i) sqrt_wide_prefix_case(next());
    for (unsigned i = 0; i < 50000; ++i) sqrt_wide_boundary(next());
    const U64 special[]{0, 0x8000000000000000ULL, 1, 0x8000000000000001ULL,
        0x000fffffffffffffULL, 0x0010000000000000ULL, 0x0010000000000001ULL,
        0x3ff0000000000000ULL, 0xbff0000000000000ULL, 0x4000000000000000ULL,
        0x7fefffffffffffffULL, 0x7ff0000000000000ULL, 0xfff0000000000000ULL,
        0x7ff0000000000001ULL, 0x7ff8000000001234ULL, 0xfff923456789abcdULL};
    for (U64 bits : special) sqrt_wide_hint_case(bits);
}

}
int main() {
    static_assert(sizeof(F64) == 8 && sizeof(U64) == 8 && sizeof(U32) == 4);
    const U64 edges[] = {0, 0x8000000000000000ULL, 1, 2, 3, 0x000fffffffffffffULL, 0x0010000000000000ULL,
        0x0010000000000001ULL, 0x3fefffffffffffffULL, 0x3ff0000000000000ULL, 0x3ff0000000000001ULL,
        0x4000000000000000ULL, 0x4000000000000001ULL, 0x7fefffffffffffffULL, 0x7ff0000000000000ULL,
        0xfff0000000000000ULL, 0x7ff8000000000000ULL, 0x7ff0000000000001ULL, 0xfff923456789abcdULL,
        0x3ca0000000000000ULL, 0x3ca0000000000001ULL, 0x3c9fffffffffffffULL};
    for (U64 a : edges) for (U64 b : edges) { pair(a,b); pair(a ^ 0x8000000000000000ULL,b); }
    for (unsigned i = 0; i < 250000; ++i) pair(next(),next());
    // Correlated operands stress cancellation, halfway ties and normalization.
    for (unsigned exponent = 1; exponent < 2047; ++exponent) {
        for (unsigned i = 0; i < 16; ++i) {
            U64 a = (U64(exponent) << 52) | (next() & 0x000fffffffffffffULL);
            pair(a, a + (next() & 7)); pair(a, (a + (next() & 7)) ^ 0x8000000000000000ULL);
            pair(a, (U64(exponent > 53 ? exponent - 53 : 0) << 52) | (next() & 0x000fffffffffffffULL));
        }
    }
    // Every representable power-of-two divisor, including all 52 subnormal
    // powers, both signs, zero/NaN/Inf and numerator neighbors at boundaries.
    // Native division remains independent of the exponent-only implementation.
    const U64 numerator_edges[] = {0,1,2,3,4,5,0x000ffffffffffffeULL,
        0x000fffffffffffffULL,0x0010000000000000ULL,0x0010000000000001ULL,
        0x0010000000000002ULL,0x3fdfffffffffffffULL,0x3fe0000000000000ULL,
        0x3fe0000000000001ULL,0x3fefffffffffffffULL,0x3ff0000000000000ULL,
        0x3ff0000000000001ULL,0x3ff0000000000002ULL,0x3fffffffffffffffULL,
        0x4000000000000000ULL,0x4000000000000001ULL,0x7feffffffffffffeULL,
        0x7fefffffffffffffULL,0x7ff0000000000000ULL,0x7ff8000000000001ULL,
        0x7ff0000000000001ULL};
    std::uint64_t power_two_checks = 0, power_two_multiply_left_checks = 0, power_two_multiply_right_checks = 0;
    for (int exponent = -1074; exponent <= 1023; ++exponent) {
        U64 divisor = exponent >= -1022 ? U64(exponent + 1023) << 52 : 1ULL << (exponent + 1074);
        for (U64 numerator : numerator_edges) for (unsigned signs = 0; signs < 4; ++signs) {
            U64 ab = numerator | (U64(signs & 1) << 63), bb = divisor | (U64(signs >> 1) << 63);
            F64 a = F64::from_bits(ab), b = F64::from_bits(bb);
            volatile double da = a.to_double(), db = b.to_double();
            check("power_two_div", a / b, da / db, ab, bb); ++power_two_checks;
            check("power_two_mul_right", a * b, da * db, ab, bb); ++power_two_multiply_right_checks;
            check("power_two_mul_left", b * a, db * da, bb, ab); ++power_two_multiply_left_checks;
        }
    }
    const auto divisionState = state;
    focused_division_checks();
    state = divisionState;
    focused_power_two_multiply_checks();
    focused_sqrt_checks();
    focused_wide_sqrt_checks();
    focused_normal_arithmetic_checks();
    std::printf("{\"schema_version\":1,\"profile\":\"integer-ieee-binary64-rne-value-only-v1\",\"checks\":%llu,\"power_two_division_checks\":%llu,\"power_two_multiplication_checks\":%llu,\"power_two_multiplication_left_checks\":%llu,\"power_two_multiplication_right_checks\":%llu,\"power_two_multiplication_boundary_checks\":%llu,\"division_boundary_checks\":%llu,\"division_hint_checks\":%llu,\"division_digit_checks\":%llu,\"sqrt_boundary_checks\":%llu,\"sqrt_prefix_checks\":%llu,\"sqrt_hint_checks\":%llu,\"sqrt_wide_boundary_checks\":%llu,\"sqrt_wide_prefix_checks\":%llu,\"sqrt_wide_hint_checks\":%llu,\"normal_addition_boundary_checks\":%llu,\"normal_addition_boundary_add_checks\":%llu,\"normal_addition_boundary_subtract_checks\":%llu,\"normal_addition_boundary_swapped_checks\":%llu,\"normal_multiplication_boundary_checks\":%llu,\"normal_multiplication_halfway_checks\":%llu,\"failures\":%llu}\n",
                (unsigned long long)checks, (unsigned long long)power_two_checks,
                (unsigned long long)(power_two_multiply_left_checks + power_two_multiply_right_checks),
                (unsigned long long)power_two_multiply_left_checks, (unsigned long long)power_two_multiply_right_checks,
                (unsigned long long)multiply_boundary_checks,
                (unsigned long long)division_boundary_checks, (unsigned long long)division_hint_checks,
                (unsigned long long)division_digit_checks, (unsigned long long)sqrt_boundary_checks, (unsigned long long)sqrt_prefix_checks,
                (unsigned long long)sqrt_hint_checks, (unsigned long long)sqrt_wide_boundary_checks,
                (unsigned long long)sqrt_wide_prefix_checks, (unsigned long long)sqrt_wide_hint_checks,
                (unsigned long long)(normal_add_checks + normal_subtract_checks),
                (unsigned long long)normal_add_checks, (unsigned long long)normal_subtract_checks,
                (unsigned long long)normal_swapped_checks, (unsigned long long)normal_multiply_checks,
                (unsigned long long)normal_halfway_checks,
                (unsigned long long)failures);
    return failures ? 1 : 0;
}
