#pragma once
#include "generic_shadow_abi.h"
#include "comparison/metal_exact/shadow_abi.h"
#include <cstring>
namespace mb3d::native_metal {
inline bool ValidMultiShadowRecord(const mb3d_native::NativeMultiShadowRecord& r,
    const ExactSurface& before,const ExactSurface& after,unsigned mask,unsigned max_steps,bool after_ao=false,
    unsigned position_mask=0) {
    if(!mask||mask>63||!max_steps||max_steps>65536)return false;
    auto a=before,b=after;
    if((a.shadow^b.shadow)&~(mask<<10))return false;
    a.shadow=b.shadow=0;
    // The final renderer readback follows the separately validated AO pass.
    if(after_ao)a.amb_shadow=b.amb_shadow=0;
    if(std::memcmp(&a,&b,sizeof(a)))return false;
    uint64_t backface=0,rays=0,hits=0,misses=0,skipped=0,steps=0,calls=0;
    for(unsigned i=0;i<6;++i){const auto& w=r.lights[i];
        if(!(mask&(1u<<i))){const mb3d_native::NativeShadowLightRecord zero{};
            if(std::memcmp(&w,&zero,sizeof(w)))return false;}
        if(w.beyond>1||w.backface>1||w.rays>1||w.hits>1||w.misses>1||w.skipped>w.rays||
           w.hits+w.misses!=w.rays||w.backface+w.rays+w.beyond>1||w.steps>max_steps||
           w.shadow_de_calls!=w.steps+w.rays||(!w.rays&&(w.steps||w.shadow_de_calls))||
           (w.skipped&&!w.misses)||(!r.total.active&&(w.backface||w.rays||w.beyond))||
           (w.beyond&&!(position_mask&(1u<<i))))return false;
        if((mask&(1u<<i))&&bool(after.shadow&(1u<<(10+i)))!=bool(w.backface||w.hits||w.beyond))return false;
        backface+=w.backface;rays+=w.rays;hits+=w.hits;misses+=w.misses;
        skipped+=w.skipped;steps+=w.steps;calls+=w.shadow_de_calls;
    }
    const auto& t=r.total;
    return t.backface==backface&&t.rays==rays&&t.hits==hits&&t.misses==misses&&
        t.skipped==skipped&&t.steps==steps&&t.shadow_de_calls==calls;
}
}
