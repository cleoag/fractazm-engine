#pragma once
#include "native_camera.metal.h"
namespace mb3d_camera {
// Separate from the primary maximum/halfspace operation. CPU reference:
// reflection.cpp SR_minLengthToCutPlane. High sign bits are intentionally
// irrelevant here; strict interval tests keep X before Y before Z on ties.
struct SecondaryCut {float depth;uint axis,fault,reserved;};
static_assert(sizeof(SecondaryCut)==16);
inline SecondaryCut secondary_cut_distance(constant Params& p,float3 origin,
    float3 direction,float initial_length,float minimum_length) {
    SecondaryCut out{initial_length,0,0,0};
    if(!valid_camera(p)||!all(isfinite(origin))||!all(isfinite(direction))||
       !isfinite(initial_length)||initial_length<=0||!isfinite(minimum_length)) {
        out.fault=1;return out;
    }
    for(uint axis=0;axis<3;++axis) {
        if(!(p.cut_options&(1u<<axis))||abs(direction[axis])<=1.e-20f)continue;
        const float distance=(p.cut[axis]-origin[axis])/direction[axis];
        if(!isfinite(distance)){out.fault=4;return out;}
        if(distance>minimum_length&&distance<out.depth){out.depth=distance;out.axis=axis+1;}
    }
    return out;
}
}
