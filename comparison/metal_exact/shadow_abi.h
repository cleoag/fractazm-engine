#pragma once
#include "scene_abi.h"

// A pointer-free representation of every production TsiLight5 field. The two
// packed depth words remain one integer so no device struct packing is assumed.
struct alignas(16) ExactSurface {
    ExactI32 normal[3];
    ExactU32 depth_packed;
    ExactU32 si_gradient, o_trap, shadow, amb_shadow;
    ExactU32 fault, reserved[3];
};

struct alignas(16) ExactSurfaceParams {
    ExactU32 version, reserved[3];
    ExactU64 z_corr, z_c_mul, color_plus, color_multiplier;
    ExactU64 color_stop_multiplier, step_width;
};

struct alignas(16) ExactShadowParams {
    ExactU32 version, enabled, light_mask, max_steps;
    ExactU64 z_corr, z_c_mul, fov_y, length_multiplier;
    ExactU64 light_vectors[18];
    ExactU32 max_iterations_result, position_mask, reserved[2];
    // Native general only (the exact path requires position_mask 0):
    // positional lights (iLightPos&1) march toward LN+Xmit (binary64), skip
    // beyond the squared reach sLmaxL*sHSmaxLengthMultiplier (Single) and
    // scale their direction by StepWidth (binary64), as calcHShard does.
    ExactU64 light_positions[18];
    ExactU32 light_reach[6];
    ExactU64 step_width;
};

// Independent per-pixel preparation for the admitted single global light.
// Every floating value is an existing binary64 or Single encoding, with no new
// narrowing at the pass boundary. The ordered row pass still owns all mutable
// open-air threshold history.
struct alignas(16) ExactShadowRay {
    ExactU32 version, active, backface, fault;
    ExactU64 position[3];
    ExactU64 z, length, threshold, zmul, initial_de;
    ExactU32 initial_stop, initial_iterations, reserved[2];
};
#ifndef __METAL_VERSION__
static_assert(sizeof(ExactSurface) == 48);
static_assert(offsetof(ExactSurface, depth_packed) == 12);
static_assert(offsetof(ExactSurface, shadow) == 24);
static_assert(offsetof(ExactSurface, fault) == 32);
static_assert(sizeof(ExactSurfaceParams) == 64);
static_assert(sizeof(ExactShadowParams) == 384);
static_assert(offsetof(ExactShadowParams, light_positions) == 208);
static_assert(offsetof(ExactShadowParams, step_width) == 376);
static_assert(sizeof(ExactShadowRay) == 96);
static_assert(offsetof(ExactShadowRay, position) == 16);
static_assert(offsetof(ExactShadowRay, z) == 40);
static_assert(offsetof(ExactShadowRay, initial_stop) == 80);
#endif
