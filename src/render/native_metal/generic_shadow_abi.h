#pragma once
#include "general_geometry_abi.h"
namespace mb3d_native {
enum : U32 { kGenericShadowVersion=3 };
struct NativeShadowRowState {float last_found,last_z;I32 threshold;U32 reserved;};
// Camera multi-light pass: one ordered cache per original (unsorted) light.
// All caches share the previous accepted surface depth, as in calcHShard.
struct NativeMultiShadowRowState {
    float last_found[6],last_z;
    I32 threshold[6]; U32 reserved;
};
// One record per pixel. Backface rejection is separate from a traced ray hit;
// skipped counts the source's ordered open-air early exit, never missing work.
struct NativeShadowRecord {
    U32 version,fault,active,backface;
    U32 rays,hits,misses,skipped;
    U32 steps,changed,shadow_de_calls,reserved;
    NativeGeometryRecord eval;
};
// WholeRow single-light shadows rebuild the row shortcut instead of tracing
// the row in order. A fresh-cache pass traces every pixel and records what
// the ordered cache transition needs; a per-row scan replays the cache and
// marks the pixels whose ordered entry state can skip, which are traced again
// from that state. z, threshold and margin are the pixel's shortcut depth,
// scale and per-pixel cache reduction, as the pixel kernel rounded them;
// last_reach is the remaining extent at the last loop check (the smallest);
// threshold_out/last_found are the cache after a fresh-entry trace.
enum : U32 { kShadowNoCache=0, kShadowCounterOnly=1, kShadowTraced=2 };
enum : U32 { kShadowBackface=1, kShadowRayLoop=2, kShadowHit=4, kShadowRecompute=8, kShadowFault=16 };
struct NativeShadowSpeculation {
    float z,threshold,margin,last_reach,last_found;
    U32 kind,flags; I32 threshold_out;
};
// The multi-light form keeps one fresh-entry cache result per original light;
// z, threshold and margin are shared, as is the row's last depth.
struct NativeMultiShadowSpeculation {
    float z,threshold,margin; U32 kind,flags;
    float last_reach[6],last_found[6];
    I32 threshold_out[6]; U32 light_flags[6];
    U32 reserved;
};
// beyond: a position light past its squared reach, shadowed without a march.
struct NativeShadowLightRecord {
    U32 backface,rays,hits,misses,skipped,steps,shadow_de_calls,beyond;
};
struct NativeMultiShadowRecord {
    NativeShadowRecord total;
    NativeShadowLightRecord lights[6];
};
#ifndef __METAL_VERSION__
static_assert(sizeof(NativeShadowRecord)==160);
static_assert(sizeof(NativeShadowRowState)==16);
static_assert(sizeof(NativeShadowSpeculation)==32);
static_assert(sizeof(NativeMultiShadowSpeculation)==120);
static_assert(sizeof(NativeMultiShadowRowState)==56);
static_assert(sizeof(NativeMultiShadowRecord)==352);
#endif
} // namespace mb3d_native
