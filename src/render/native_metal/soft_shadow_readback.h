#pragma once
#include "soft_shadow_abi.h"
#include "comparison/metal_exact/shadow_abi.h"
#include <cstring>
namespace mb3d::native_metal {
inline bool ValidSoftShadowRecord(const mb3d_native::NativeSoftShadowRecord& r,
    const ExactSurface& before,const ExactSurface& after,unsigned max_steps,bool after_ao=false) {
    if(r.version!=mb3d_native::kNativeSoftShadowVersion||r.reserved||r.intensity>63||
       r.partial!=unsigned(r.intensity>0&&r.intensity<63)||r.intensity!=(after.shadow>>10)||
       ((before.shadow^after.shadow)&0x3ffu))return false;
    auto a=before,b=after;a.shadow=b.shadow=0;
    if(after_ao)a.amb_shadow=b.amb_shadow=0;
    if(std::memcmp(&a,&b,sizeof(a)))return false;
    const auto& t=r.total;
    const bool active=(before.depth_packed>>16)<32768&&before.si_gradient<32768;
    return t.active==unsigned(active)&&!t.skipped&&t.backface+t.rays<=1&&t.steps<=max_steps&&
        t.shadow_de_calls==t.steps+t.rays&&t.hits+t.misses==t.rays&&
        (t.rays?t.steps>0:(!t.steps&&!t.shadow_de_calls))&&
        (!t.backface||!r.intensity)&&
        (t.rays||t.backface||r.intensity==63)&&
        (t.active||(r.intensity==63&&!t.rays&&!t.backface&&!t.eval.de_calls));
}
}
