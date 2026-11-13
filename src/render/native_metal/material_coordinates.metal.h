#pragma once
// Shared coordinate semantics for primary and secondary materials. Directional
// lighting still receives the original local normal/view. Only ambient and
// object-relative background/depth use the object frame.
#include "generic_color_abi.h"
#include "comparison/metal_exact/color_math.h"
namespace mb3d_native {
using namespace mb3d_metal_exact;
inline F32 native_material_object_y(ColorVec local,constant NativeInteriorColorParams& p) {
    return (local.x*color_s(p.object_y[0])+local.y*color_s(p.object_y[1]))+
        local.z*color_s(p.object_y[2]);
}
inline F32 native_material_ambient_y(ColorVec normal,constant NativeInteriorColorParams& p) {
    return (p.flags&64u)?native_material_object_y(normal,p):normal.y;
}
inline F32 native_material_object_depth(F32 world_y) {
    const float y=metal::clamp(world_y.value,-1.f,1.f);
    return F32::from_native(metal::asin(y)*(1.f/M_PI_F)+.5f);
}
inline F32 native_material_depth(F32 row,ColorVec local_view,constant NativeInteriorColorParams& p) {
    if(p.flags&64u)return native_material_object_depth(native_material_object_y(local_view,p));
    if(p.depth_function==1)return row*row;
    if(p.depth_function==0)return row;
    return sqrt(color_max0(row));
}
}
