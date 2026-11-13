#pragma once
#include "native_inside_abi.h"
#include "native_inside_reflection_abi.h"
#include "core/types.h"
namespace mb3d::native_metal {
mb3d_inside::Params PrepareNativeInside(const TMCTparameter&);
// Transmission (bCalcSRautomatic bit 2) refracts at the header index, or one
// when the addon's bOptions2 selects it; only from an outside camera.
mb3d_inside::ReflectionConfig PrepareNativeInsideReflection(const TMCTparameter&,const TMandHeader10&,const THeaderCustomAddon&);
}
