#pragma once
#include "scene_abi.h"

// Native-only opaque secondary rays. Every floating field is an IEEE binary32
// word prepared once per scene; no host pointers or per-pixel CPU results.
struct alignas(16) NativeReflectionAO {
    ExactU32 ray_count, step_multiplier, de_multiplier, correction_weight;
    ExactU32 max_distance_base, minimum_overlap_dot, overlap_angle_scale, reserved;
    ExactU32 directions[8][4];
};
struct alignas(16) NativeReflectionParams {
    ExactU32 version, width, height, max_bounces;
    ExactU32 max_steps, vary_de_stop, ao_enabled, shadows_enabled;
    ExactU32 amount, step_width, fov_y, z_mid_difference;
    // first_step_random: CalcAmbientOcclusionForPoint's first-step jitter
    // (header iOptions bit 0); only general camera reflections admit it.
    ExactU32 z_corr, z_c_mul, back_distance, first_step_random;
    // Three stored CPU matrix columns, four floats per column. The inverse
    // view rotation takes their dot products; AO then rotates into world space.
    ExactU32 view_matrix[3][4], ao_matrix[3][4];
    // Bounce 1 uses quality 2, bounce 2 quality 1, every later bounce quality 0.
    NativeReflectionAO ao[3];
};

// Fault bits: 1 invalid ABI/input, 2 bounded work exhausted, 4 nonfinite math,
// 8 unsupported numerical branch. A fault fails the whole GPU render.
struct alignas(16) NativeReflectionRecord {
    ExactU32 fault, active, rays, hits;
    ExactU32 misses, bounces, changed_pixels, primary_skipped;
    ExactU32 ao_rays, ao_steps, shadow_rays, shadow_steps;
    ExactU32 ray_steps, nonfinite, reserved0, reserved1;
};
#ifndef __METAL_VERSION__
static_assert(sizeof(NativeReflectionAO)==160);
static_assert(sizeof(NativeReflectionParams)==640);
static_assert(sizeof(NativeReflectionRecord)==64);
static_assert(offsetof(NativeReflectionParams, ao)==160);
#endif
