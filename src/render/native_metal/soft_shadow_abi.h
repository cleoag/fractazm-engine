#pragma once
#include "generic_shadow_abi.h"
namespace mb3d_native {
// Version 3 carries calcHSsoft's ZRSmul numerator (80 directional; 70 or 40
// for a position light by iLightPos&6) and admits position soft lights and
// the reflection pass's CalcHSsoftPoint. Version 2 libraries must fail.
enum : U32 { kNativeSoftShadowVersion=3, kNativeSoftColorVersion=5 };
struct NativeSoftShadowConfig { U32 version,radius,light,numerator; };
struct NativeSoftShadowRecord {
    NativeShadowRecord total;
    U32 version,intensity,partial,reserved;
};
#ifndef __METAL_VERSION__
static_assert(sizeof(NativeSoftShadowConfig)==16);
static_assert(sizeof(NativeSoftShadowRecord)==176);
#endif
}
