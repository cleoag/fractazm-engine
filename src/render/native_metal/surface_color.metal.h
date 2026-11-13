#pragma once
#include "surface_color_abi.h"
namespace mb3d_native {
inline bool valid_surface_color_descriptor(constant NativeSurfaceParams& p,uint driver,int de_option) {
    return p.version==kNativeSurfaceVersion&&!p.reserved&&p.color_option<=1u&&
        metal::isfinite(p.orbit_color_multiplier)&&(p.color_option||p.orbit_color_multiplier==0.f)&&
        (!p.color_option||(de_option!=20&&driver<HybridDifs));
}
inline NativeHitColorPayload capture_hit_color(thread const NativeFormulaState& lane) {
    return {kNativeHitColorVersion,lane.rout,lane.rold,0u};
}
inline uint general_orbit_color(NativeHitColorPayload payload,float multiplier,thread uint& fault) {
    if(payload.version!=kNativeHitColorVersion||payload.reserved){fault|=1u;return 0u;}
    if(!metal::isfinite(payload.rout)||!metal::isfinite(payload.rold)||!metal::isfinite(multiplier)){
        fault|=4u;return 0u;
    }
    // Preserve the source expression and the Single-to-15-bit boundary.
    // Finite inputs can produce log(0), log(negative), or overflow. The CPU
    // codec saturates +Inf/NaN high and -Inf low; these are not raw payload faults.
    const float scalar=metal::log(payload.rout/(payload.rold+1.f))*multiplier;
    if(metal::isnan(scalar)||scalar>=32767.f)return 32767u;
    if(scalar<=0.f)return 0u;
    return uint(metal::rint(scalar));
}
}
