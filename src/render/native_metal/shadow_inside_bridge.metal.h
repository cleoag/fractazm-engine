#pragma once
#include "native_inside_math.h"
namespace mb3d_native {
// Insert at generic_shadow_pixel's first DE call, AFTER its foreground and
// SIgradient-high-bit rejection. Subsequent refine/march calls use this lane.
// The first DE is intentionally not skipped for an outside-marked surface:
// CPU shadow.cpp:326-356 performs it before switching to outside and redoing DE.
inline NativeOrbitResult general_shadow_inside_start(float3 point,float stop,
    constant NativeFormulaProgram& program,constant mb3d_inside::Params& policy,
    uint orbit_trap,thread NativeResources& resources,thread NativeFormulaState& lane,
    thread NativeGeometryRecord& record) {
    NativeOrbitResult value{};
    if(!mb3d_inside::valid(policy)||program.inside!=as_type<uint>(policy.initial_inside)||orbit_trap>65535u){record.fault=1;return value;}
    lane.inside=as_type<uint>(policy.initial_inside);
    value=general_de(point,stop,program,resources,lane,record);
    if(!record.fault&&policy.in_and_out&&(orbit_trap&0x8000u)){
        lane.inside=0;value=general_de(point,stop,program,resources,lane,record);
    }
    return value;
}
}
