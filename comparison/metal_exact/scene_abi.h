#pragma once

#ifdef __METAL_VERSION__
#include <metal_stdlib>
using ExactU32 = metal::uint;
using ExactI32 = int;
using ExactU64 = metal::ulong;
#else
#include <cstddef>
#include <cstdint>
using ExactU32 = std::uint32_t;
using ExactI32 = std::int32_t;
using ExactU64 = std::uint64_t;
#endif

// Integer encodings retain every binary64/Single bit across the device ABI.
// Prepared constants are per scene, never pointer-bearing runtime records.
struct ExactBulbParams {
    ExactU64 bailout, gradient_bailout, de_offset, de_offset006;
    ExactU32 smooth_hln, smooth_lnrstop, power, reserved;
};
struct alignas(16) ExactScene {
    ExactU32 version, width, height, max_steps;
    ExactU32 max_iterations, slot_count, min_iterations, refinement_steps;
    ExactU64 params[13];
    ExactU64 vgrads[9];
    ExactU64 y_start[3];
    ExactU64 max_ray_length;
    ExactU32 de_stop, de_stop_factor, z_step_div, step_limiter;
    // fog_iteration: Native general DFogOnIt (0 counts every step, N only steps
    // whose last DE ran N iterations). Every other profile keeps it zero.
    ExactU32 de_scale, step_width_103, de_sub, fog_iteration;
    // Version 2: explicit uniform formula and pinned capability selection.
    // color_on_it: Native general RMdoColorOnIt iteration maximum (ColorOnIt-1,
    // zero when it has no effect) and its Single RStop. Exact profiles keep both zero.
    ExactU32 formula_kind, capability_id, color_on_it, color_radius_stop;
    ExactBulbParams bulb;
};

// One table entry per column or row. Host performs scene preparation's exact
// scalar trigonometry; GPU builds/normalizes/rotates every camera ray itself.
struct alignas(16) ExactTrig {
    ExactU64 sin_value, cos_value;
    // Version 4 primary-camera operands retain the FSINCOS extended stores.
    // Color tables use only the binary64 fields above and zero these words.
    ExactU64 sin_significand, cos_significand;
    ExactU32 sin_sign_exponent, cos_sign_exponent, reserved[2];
};

struct alignas(16) ExactPixel {
    ExactU64 ray[3], start[3], position[3];
    ExactU64 depth, axial_depth, final_de;
    ExactU64 raw_normal[3];
    ExactI32 normal[3];
    ExactU32 rm_result, start_inside, step_count, smooth_iterations;
    ExactU32 final_iterations, fault, final_stop;
    ExactU64 orbit_trap, reserved_tail;
};
// position is the RayMarch ActPos: it remains the initial position on misses.
// final_de is the pre-refinement DE of a marched hit, otherwise canonical zero;
// CPU production's stale previous-pixel finalDE on misses is not a distance.
// final_iterations is sampled before normals; smooth_iterations is the Single
// NN returned by production normals. Normal fields are zero for misses/inside.

#ifndef __METAL_VERSION__
static_assert(sizeof(ExactScene) == 336);
static_assert(offsetof(ExactScene, params) == 32);
static_assert(offsetof(ExactScene, max_ray_length) == 232);
static_assert(offsetof(ExactScene, formula_kind) == 272);
static_assert(sizeof(ExactBulbParams) == 48);
static_assert(offsetof(ExactScene, bulb) == 288);
static_assert(offsetof(ExactPixel, orbit_trap) == 160);
static_assert(sizeof(ExactTrig) == 48);
static_assert(offsetof(ExactTrig, sin_significand) == 16);
static_assert(offsetof(ExactTrig, sin_sign_exponent) == 32);
static_assert(sizeof(ExactPixel) == 176);
static_assert(offsetof(ExactPixel, normal) == 120);
#endif
