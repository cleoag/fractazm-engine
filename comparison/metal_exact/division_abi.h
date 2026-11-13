#pragma once
#include "scene_abi.h"

struct DivisionSeedInput {
    ExactU64 a, b, numerator, denominator, hint;
    ExactU32 shift, reciprocal_bits;
};
struct DivisionSeedOutput {
    ExactU64 bits, restoring_bits, hinted_bits, quotient, remainder;
    ExactU32 valid, reserved;
};
#ifndef __METAL_VERSION__
static_assert(sizeof(DivisionSeedInput) == 48 && sizeof(DivisionSeedOutput) == 48);
#endif
