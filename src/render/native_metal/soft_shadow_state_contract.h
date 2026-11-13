#pragma once
#include "formula_abi.h"
#ifdef __METAL_VERSION__
#define MB3D_SHADOW_CONSTANT constant
#else
#define MB3D_SHADOW_CONSTANT const
#endif
namespace mb3d_native {
// Shared host/device identity guard for the separately audited soft-shadow
// entry. This does not derive permission from the primary dispatch policy.
inline bool SoftShadowPixelChain(MB3D_SHADOW_CONSTANT NativeFormulaProgram& p) {
    if(p.version!=kFormulaAbiVersion||p.slot_count!=6||p.driver!=Hybrid3DAnalytic||p.de_option!=2||
       p.start_slot!=0||p.repeat_slot!=1||p.end_slot!=5||p.inside||p.combination_smooth!=0.f||
       p.formula_type||p.mix_color||p.mix_power!=0.f)return false;
    const U32 ids[]={3676,4,10010,7076,10171,1509};
    const U32 counts[]={0x80000001u,1,0x80000001u,0x80000001u,8,102};
    const I32 options[]={-1,11,-1,-1,2,11};
    const U32 parameters[]={7,0,15,7,22,15};
    for(unsigned i=0;i<6;++i){MB3D_SHADOW_CONSTANT auto& s=p.slots[i];
        if(s.version!=kFormulaAbiVersion||s.enabled!=1||s.id!=ids[i]||
           s.namespace_id!=(i==1?InternalFormula:ExternalFormula)||
           s.de_option!=options[i]||s.param_count!=parameters[i]||s.traits||s.difs_style||
           s.wrapper_flags!=(i==1?2u:1u)||s.reserved[0]||s.reserved[1]||
           U32(p.hybrid_counts[i])!=counts[i])return false;
    }
    return true;
}
}
#undef MB3D_SHADOW_CONSTANT
