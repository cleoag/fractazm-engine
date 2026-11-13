#pragma once

// _Translate's source add, including finite binary32 subnormals. The only
// arithmetic operation is one hardware float add. Integer operations encode
// exact power-of-two scale boundaries; they do not implement a soft adder.
#ifdef __METAL_VERSION__
#include <metal_stdlib>
#else
#include <bit>
#include <cstdint>
#include <cstring>
#endif

namespace mb3d_native {
#ifdef __METAL_VERSION__
using TranslateWord = uint;
// as_type is the MSL built-in macro, not a metal:: namespace member.
inline TranslateWord translate_add_bits(float x) { return as_type<uint>(x); }
inline float translate_add_float(TranslateWord x) { return as_type<float>(x); }
inline unsigned translate_add_clz(TranslateWord x) { return metal::clz(x); }
#else
using TranslateWord = std::uint32_t;
inline TranslateWord translate_add_bits(float x) {
    TranslateWord word; std::memcpy(&word, &x, sizeof(word)); return word;
}
inline float translate_add_float(TranslateWord x) {
    float value; std::memcpy(&value, &x, sizeof(value)); return value;
}
inline unsigned translate_add_clz(TranslateWord x) { return std::countl_zero(x); }
#endif

// Precondition: abs(x) < 2^-100. Every nonzero encoded result is normal.
inline TranslateWord translate_add_scale24(TranslateWord word) {
    const TranslateWord magnitude = word & 0x7fffffffu;
    if (!magnitude) return word;
    if (magnitude & 0x7f800000u) return word + (24u << 23);
    const unsigned leading = 31u - translate_add_clz(magnitude);
    return (word & 0x80000000u) | ((leading + 2u) << 23) |
        ((magnitude << (23u - leading)) & 0x7fffffu);
}

// The small-operand sum is a multiple of 2^-149 before scaling. If its
// unscaled result is subnormal it has at most23 significant bits, so the
// hardware scaled add was exact and this right shift discards only zeroes.
inline TranslateWord translate_add_unscale24(TranslateWord word) {
    const TranslateWord magnitude = word & 0x7fffffffu;
    if (!magnitude) return word;
    const unsigned exponent = magnitude >> 23;
    if (exponent > 24u) return word - (24u << 23);
    return (word & 0x80000000u) |
        ((0x800000u | (word & 0x7fffffu)) >> (25u - exponent));
}

inline float native_translate_add(float a, float b) {
    const TranslateWord aw = translate_add_bits(a), bw = translate_add_bits(b);
    const TranslateWord am = aw & 0x7fffffffu, bm = bw & 0x7fffffffu;
    // This also retains the original hardware expression for INF/NaN and
    // overflow. Above this threshold a subnormal is below half either adjacent
    // ULP, and cancellation of normal operands cannot yield a subnormal.
    if (am >= 0x0d800000u || bm >= 0x0d800000u) return a + b;
    const float scaled_a = translate_add_float(translate_add_scale24(aw));
    const float scaled_b = translate_add_float(translate_add_scale24(bw));
    const float sum = scaled_a + scaled_b;
    return translate_add_float(translate_add_unscale24(translate_add_bits(sum)));
}
} // namespace mb3d_native
