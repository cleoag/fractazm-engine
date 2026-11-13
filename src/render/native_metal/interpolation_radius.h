#pragma once
#include "translate_add.metal.h"
#ifndef __METAL_VERSION__
#include <cmath>
#endif
namespace mb3d_native {
// The Pascal interpolation expressions have separate, left-associated adds.
// Keep their projection distinct from the generic contracted radius helper.
__attribute__((noinline)) inline float native_interpolation_radius(
    float x,float y,float z,float w,bool four) {
#pragma clang fp contract(off)
    const float xyz=(x*x+y*y)+z*z;
    return four?xyz+w*w:xyz;
}
// Original interpolation computes sqrt(radius + 1e-40). The RN32 epsilon
// (0x116c2) is subnormal; feeding it directly to Metal arithmetic flushes it
// to zero and turns the valid zero-point normalization into 0/0. Scaling both
// operands by 2^24 keeps the add and sqrt normal. Binary rounding commutes
// with this exact power-of-two change of exponent; the final root is normal.
// Above 2^-100 the positive epsilon is below half an ulp of radius.
inline float native_interpolation_denominator(float radius) {
#pragma clang fp contract(off)
#ifdef __METAL_VERSION__
    using metal::sqrt;
#else
    using std::sqrt;
#endif
    const TranslateWord word=translate_add_bits(radius);
    if((word&0x7fffffffu)>=0x0d800000u)return sqrt(radius+1e-40f);
    const float scaled_radius=translate_add_float(translate_add_scale24(word));
    const float scaled_epsilon=translate_add_float(translate_add_scale24(0x000116c2u));
    return sqrt(scaled_radius+scaled_epsilon)*0x1p-12f;
}
} // namespace mb3d_native
