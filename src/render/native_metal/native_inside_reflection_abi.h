#pragma once
#include "native_inside_abi.h"
namespace mb3d_inside {
// Reflection-only buffers29/30. Keep the existing480-byte reflection ABI and
// the primary32-byte policy unchanged. No pointers or CPU-produced pixels.
// transmission (bCalcSRautomatic bit 2) refracts at TransDiConst, refraction;
// both stay zero for opaque reflections.
struct ReflectionConfig {
    U32 version,is_ifs;
    float absorption,scattering;
    U32 transmission;float refraction;
    U32 reserved[2];
};
struct ReflectionRecord {
    U32 version,fault; I32 selected_inside;
    U32 inside_ao_rays,absorption_bounces,initial_primary,cut_ao_skips,cut_shadow_limits;
};
struct AOPolicy {
    float step_multiplier,de_multiplier,stop,max_distance,initial_travel,md;
    U32 inside_non_ifs,fault;
};
struct AOStep { float amount,next; U32 ended,fault; };
struct InitialReflection { float distance; U32 skip,open_air,fault; };
struct AbsorptionChannel { float factor,throughput,scatter; U32 fault; };
// Independently specified primitive operations, not a render protocol.
enum : U32 { SelectPolicy=1,SecondaryShadow=2,PrepareAO=3,InsideAOStep=4,
    AbsorbChannel=5,InitialBranch=6,AOCutPlane=7 };
struct ReflectionProbeInput { Params policy; ReflectionConfig config; U32 words[8]; float values[16]; };
struct ReflectionProbeOutput { U32 words[16]; float values[16]; };
#ifndef __METAL_VERSION__
static_assert(sizeof(ReflectionConfig)==32);
static_assert(sizeof(ReflectionRecord)==32);
static_assert(sizeof(AOPolicy)==32);
static_assert(sizeof(ReflectionProbeInput)==160);
static_assert(sizeof(ReflectionProbeOutput)==128);
#endif
}
