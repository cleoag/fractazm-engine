#pragma once
#include "formula_abi.h"
#include "comparison/metal_exact/shadow_abi.h"

namespace mb3d_native {
enum : U32 { kNativeSurfaceVersion=3, kNativeHitColorVersion=1 };
// Native-only descriptor. Compatible ExactSurfaceParams remains version 2.
// color_multiplier belongs to SIgradient; orbit_color_multiplier is mctColorMul.
struct alignas(16) NativeSurfaceParams {
    U32 version, color_option;
    float orbit_color_multiplier;
    U32 reserved;
    ExactU64 z_corr, z_c_mul, color_plus, color_multiplier;
    ExactU64 color_stop_multiplier, step_width;
};
// Snapshot of the carried state at RMdoColor's boundary. Rout/Rold may come
// from different ordered samples: numerical DE restores only Rout. Never
// reconstruct this pair by evaluating the hit again or reset the formula lane.
struct NativeHitColorPayload {
    U32 version;
    float rout, rold;
    U32 reserved;
};
#ifndef __METAL_VERSION__
static_assert(sizeof(NativeSurfaceParams)==64);
static_assert(sizeof(NativeHitColorPayload)==16);
static_assert(offsetof(NativeSurfaceParams,z_corr)==16);
#endif
}
