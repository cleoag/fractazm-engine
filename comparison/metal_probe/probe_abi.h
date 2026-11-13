#pragma once
// Experimental device ABI v1. No production records, pointers or native bools.
#ifdef __METAL_VERSION__
using ProbeUInt = uint;
#else
#include <cstdint>
#include <cstddef>
using ProbeUInt = std::uint32_t;
#endif

struct alignas(16) ProbeScene {
    ProbeUInt version, width, height, max_steps;
    ProbeUInt shadow_steps, ao_samples, iterations, reserved;
    float epsilon, max_distance, step_scale, fov_scale;
};
struct alignas(16) ProbePixel {
    float red, green, blue, depth;
    float nx, ny, nz, ao;
    ProbeUInt hit, steps, de_calls, fault;
};
struct alignas(16) ArithmeticInput {
    ProbeUInt operation, reserved0, reserved1, reserved2;
    float a, b, c, reserved3;
};
struct alignas(16) FormulaInput {
    float params[16];
    float x, y, z, w;
};
struct alignas(16) FormulaOutput {
    float x, y, z, w;
    float distance, vary_scale, trap, reserved;
};
#ifndef __METAL_VERSION__
static_assert(sizeof(ProbeScene)==48 && alignof(ProbeScene)==16);
static_assert(offsetof(ProbeScene,epsilon)==32);
static_assert(sizeof(ProbePixel)==48 && offsetof(ProbePixel,hit)==32);
static_assert(sizeof(ArithmeticInput)==32 && offsetof(ArithmeticInput,a)==16);
static_assert(sizeof(FormulaInput)==80 && offsetof(FormulaInput,x)==64);
static_assert(sizeof(FormulaOutput)==32 && offsetof(FormulaOutput,distance)==16);
#endif
