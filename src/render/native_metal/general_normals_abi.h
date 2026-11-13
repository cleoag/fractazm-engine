#pragma once
#include "formula_abi.h"
namespace mb3d_native {
enum : U32 { kGeneralNormalVersion=1, kGeneralNormalMaxSamples=161,
    kGeneralNormalSampleBudget=1u<<18, kGeneralNormalOrbitBudget=1u<<18 };
inline U32 GeneralNormalSamples(U32 normals_on_de,U32 smoothing) {
    return smoothing==8 ? 125u+(normals_on_de?0u:4u)+4u*smoothing :
        7u+(smoothing&&(!normals_on_de||smoothing<8)?4u:0u)+4u*smoothing;
}
struct NativeNormalParams {
    U32 version,normals_on_de,smoothing,inside;
    U32 reserved[4];
};
struct NativeNormalRecord {
    U32 version,fault,direct_orbit_calls,sample_calls;
    U32 de_calls,completed_evaluations,reserved[2];
};
struct NativeNormalResult {
    float raw_normal[3],smooth;
    float roughness,orbit_trap;
    U32 roughness_byte,completed;
    I32 packed[3];U32 fault;
    U32 reserved[4];
};
#ifndef __METAL_VERSION__
static_assert(sizeof(NativeNormalParams)==32);
static_assert(sizeof(NativeNormalRecord)==32);
static_assert(sizeof(NativeNormalResult)==64);
#endif
}
