#pragma once
#include "scene_abi.h"
struct alignas(16) SingleInput { ExactU32 a, b, op, reserved; };
struct alignas(16) SingleOutput {
    ExactU32 bits;
    ExactI32 integer_rtz, integer_rne;
    ExactU32 fault;
    ExactU64 double_bits, reserved;
};
#ifndef __METAL_VERSION__
static_assert(sizeof(SingleInput)==16);
static_assert(sizeof(SingleOutput)==32);
static_assert(offsetof(SingleOutput,double_bits)==16);
#endif
