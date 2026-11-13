#pragma once
#include "general_geometry_abi.h"
#include "comparison/metal_exact/native_reflection_abi.h"
namespace mb3d_native {
enum : U32 { kGenericReflectionVersion=3 };
// Common activity remains independently inspectable. The second record binds
// every secondary/normal/AO/shadow DE call to the general formula work guards.
struct NativeGenericReflectionRecord {
    NativeReflectionRecord activity;
    NativeGeometryRecord eval;
};
// Deferred reflection shading. The camera-aware trace records each
// CalcPixelColorSvec evaluation of a pixel in trace order (primary hit,
// secondary hits, background) with the throughput on its radiance and the
// scattering added before it. A formula-independent pass shades and
// accumulates them; count zero keeps the incoming RGB.
// MB3D authors one to eight reflections. An opaque record holds the primary
// event and one per bounce. Version 2 adds the ray's CamPos and AbsViewVec for
// visible light shapes. Version 3 records transmission: SR_CalcRay's binary
// recursion (refracted subtree, then the Fresnel reflection) emits up to
// 2^(bounces+1)-1 events, so a record is this header followed by its
// capacity of events, and an event carries bScaleAmbDiffDown's light amount.
enum : U32 { kNativeReflectionShadeVersion=3, kNativeReflectionMaxBounces=8,
             kNativeReflectionShadeEvents=kNativeReflectionMaxBounces+1,
             kNativeReflectionTreeEvents=(2u<<kNativeReflectionMaxBounces)-1 };
struct alignas(16) NativeReflectionShadeEvent {
    ExactSurface surface;
    float view[3],world_view_y;  // unit view in the camera frame, world view Y
    float position[3],y;         // absolute hit point, depth-color position
    float zpos,fog_z;            // palette depth and dynamic-fog depth
    U32 suppress_fog,absorbs;    // CalcPixelColorSvecTrans; scattering present
    float scale_amount;          // bScaleAmbDiffDown's SRLightAmount, else zero
    U32 reserved[3];
    float weight[4];             // throughput on this radiance; w zero
    float absorbed[4];           // scattering color, w its amount
    float camera[4];             // CamPos: ray start relative to the scene center; w zero
    float absolute[4];           // AbsViewVec: unit ray direction; w zero
};
struct alignas(16) NativeReflectionShadeRecord {
    U32 version,pixel,count,capacity;
};
// Bytes of one record with its events; the trace and shade passes share it.
inline U32 native_reflection_shade_bytes(U32 capacity) {
    return U32(sizeof(NativeReflectionShadeRecord)+capacity*sizeof(NativeReflectionShadeEvent));
}
// Events a pixel can emit: one per bounce, or the full recursion tree.
inline U32 native_reflection_shade_capacity(U32 bounces,bool transmission) {
    return transmission?(2u<<bounces)-1:bounces+1;
}
#ifndef __METAL_VERSION__
static_assert(sizeof(NativeGenericReflectionRecord)==176);
static_assert(sizeof(NativeReflectionShadeEvent)==176);
static_assert(sizeof(NativeReflectionShadeRecord)==16);
#endif
}
