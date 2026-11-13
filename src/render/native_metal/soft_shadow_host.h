#pragma once
#include "soft_shadow_abi.h"
#include "comparison/metal_exact/scene_host_internal.h"
#include "comparison/metal_exact/shadow_host.h"
#include <bit>
#include <cmath>
#include <stdexcept>
namespace mb3d::native_metal {
inline mb3d_native::NativeSoftShadowConfig PrepareGeneralSoftShadow(
    const metal_exact::PreparedExactScene& scene,const ExactShadowParams& shadow) {
    if(!scene.host||scene.precision!=PrecisionMode::Native||
       scene.device.formula_kind!=mb3d_native::kGeneralFormulaKind||
       scene.device.capability_id!=mb3d_native::kGeneralCapability||
       !(scene.host->file.header.bCalc1HSsoft&1)||!shadow.enabled||
       !shadow.light_mask||shadow.light_mask>63)
        throw std::invalid_argument("Unsupported Native soft-shadow configuration");
    const float radius=scene.host->mct.SoftShadowRadius;
    if(!std::isfinite(radius)||radius<=0||!std::isfinite(80.f/radius))
        throw std::invalid_argument("Invalid Native soft-shadow radius");
    unsigned light=0;for(unsigned i=0;i<6;++i)if(shadow.light_mask&(1u<<i))light=i;
    const unsigned numerator=metal_exact::NativeSoftShadowNumerator(scene);
    if((numerator!=80u)!=bool((shadow.position_mask>>light)&1u))
        throw std::invalid_argument("Inconsistent Native soft-shadow light position");
    return {mb3d_native::kNativeSoftShadowVersion,std::bit_cast<uint32_t>(radius),light,numerator};
}
}
