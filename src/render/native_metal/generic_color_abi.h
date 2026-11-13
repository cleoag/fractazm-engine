#pragma once
#include "formula_abi.h"
namespace mb3d_native {
// Scene-only interior palette, separate from the immutable compatible and
// three-formula color ABI. Values are FP32 bit encodings, including RGBA.
// Version 4 adds the original depth-gradient function (0 linear, 1 square,
// 2/3 square root); version 3 libraries must be rejected.
// Version 5 is a separate opt-in for completed Native soft shadows: light
// hs_mask=0xffffffff consumes the six-bit intensity. Version 4 stays hard-only.
// Versions 6/7 retain hard/soft policies and add explicit object-relative
// material coordinates. Older library markers must fail before dispatch.
// Flags: cyclic, no interpolation, far fog, orbit-trap palette,
// nonnegative dynamic fog, blended dynamic fog, object-relative ambient/depth.
struct alignas(16) NativeDirectionalLight {
    U32 enabled,power,function,hs_mask;
    U32 hs_calculated,hs_enabled,reserved[2];
    U32 direction[4],color[4];
};
struct alignas(16) NativeInteriorColorParams {
    U32 version,flags,reserved[2];
    U32 palette_start,palette_mul,reserved_scalar[2];
    I32 positions[4];
    U32 divisors[4],colors[4][4];
    I32 gamma_mode; U32 gamma,squared,depth_function;
    NativeDirectionalLight lights[6];
    U32 object_y[4]; // FP32 paint-matrix column Y; fourth word reserved zero.
};
// Flag 128: at least one enabled light has position options (iLightPos):
// bit 0 positional, bits 1..3 a visible light shape. The separately bound
// geometry gives the paint frame (CalcObjPos inputs) and each light's options;
// a kernel without it must reject the flag.
enum : U32 { kNativeLightGeometryVersion=1, kNativeLightGeometryFlag=128 };
// Flag 256: iExModes selects CalcTotalLight2 (luminance-preserving bounce)
// instead of CalcTotalLight1. Camera color kernels only.
enum : U32 { kNativeTotalLight2Flag=256 };
// Flag 1024: volumetric light (bVolLight). The fog step count in the low ten
// Shadow bits is RMcalcVLight-packed; ConvertVLight decodes it. Camera color only.
enum : U32 { kNativeVolumeLightFlag=1024 };
struct alignas(16) NativePointLight {
    U32 options,max_length2,depth_position,z_position; // iLightPos; FP32 sLmaxL, sPosLP, sPosLightZpos
};
struct alignas(16) NativeLightGeometry {
    U32 version,zz_mid_difference,reserved[2]; // FP32 TLightVals.sZZstmitDif
    U32 start[4],y_add[4],x_add[4]; // GetStartSPosAndAddVecs, FP32
    U32 matrix[3][4];               // normalized paint matrix rows, FP32
    NativePointLight lights[6];
};
// Flag 512: material maps loaded by CompleteLightValsSetup — a diffuse color
// map (iColOnOT 2..9) and/or light-map lights (iLightOption 2). The separately
// bound descriptor and FP32 raw samples ((width+1)*(height+1)*3 per map, the
// loader's seam layout) carry them; a kernel without both must reject the
// flag. Wrap-3D modes (iColOnOT 6..9) also need the light geometry (flag 128)
// for CalcObjPos and the paint matrix. Camera color kernels only.
// Version 2 adds the loaded background map (bBackBMP), sphere-sampled along
// AbsViewVec for primary and reflected misses (background bit 1). Bit 4 is
// bAddBGlight; bit 8 (bUseSmallBGpicForAmb) samples the same map, which
// BGsmallLM shares, around the paint-rotated normal for the ambient light.
// Both need the light geometry (flag 128) for the paint matrix. Bit 16 is
// bUseSmallBGpicForAmb without a loaded map: the ambient gradient follows the
// paint-rotated normal instead (geometry too, no map).
enum : U32 { kNativeMaterialMapVersion=2, kNativeMaterialMapFlag=512 };
enum : U32 { kNativeBackgroundMap=1, kNativeBackgroundAddLight=4, kNativeBackgroundAmbient=8,
             kNativeBackgroundRotatedAmbient=16 };
struct alignas(16) NativeMaterialMap {
    U32 width,height,offset,word_map;          // LMWidth/LMHeight, first sample, iMapType==1
    U32 intensity,x_factor,y_factor,reserved;  // FP32 sIntensity, sLMXfactor, sLMYfactor
    U32 average[4],average_squared[4];         // FP32 LMavrgCol, LMavrgColSqr
    U32 rotation[3][4];                        // FP32 PicRotMatrix rows
};
struct alignas(16) NativeMaterialMaps {
    U32 version,diffuse_mode,yc_combine,light_mask;  // iColOnOT (0: no map), bYCcomb, bit i: light-map light
    U32 offset_x,offset_y,rotation_sin,rotation_cos; // FP32 DCLMapOffX/Y, DCLMapRotSin/Cos
    U32 scale,mid[3];                                // FP32 lvMapScale, lvMidPos
    U32 sample_count,background,reserved[2];         // background: kNativeBackground* bits
    NativeMaterialMap diffuse,lights[6],background_map;
};
// Reflection material (camera-aware reflection kernels, buffer 19; map samples
// at buffer 24): CalcPixelColorSvec for every enabled light, positional
// lights, squared color, CalcTotalLight2, material maps, soft-shadow intensity
// and visible light shapes. SR_CalcRay's ObjPos is the hit point minus the
// scene center (Xmit, Ymit, Zmit). The geometry and maps copy the camera color
// descriptors and are read only with interior flags 128 and 512. Version 2
// adds SR_CalcLightZPos's constants for a position light's per-ray depth.
enum : U32 { kNativeReflectionMaterialVersion=2 };
struct alignas(16) NativeReflectionMaterial {
    U32 version,step_width,z_c_mul,z_corr; // FP32 sStepWidth, ZcMul, Zcorr
    U32 mid[4];                     // FP32 Xmit, Ymit, Zmit; fourth word zero
    NativeLightGeometry geometry;
    NativeMaterialMaps maps;
};
#ifndef __METAL_VERSION__
static_assert(sizeof(NativeInteriorColorParams)==544);
static_assert(sizeof(NativeLightGeometry)==208);
static_assert(sizeof(NativeMaterialMap)==112);
static_assert(sizeof(NativeMaterialMaps)==960);
static_assert(sizeof(NativeReflectionMaterial)==1200);
#endif
} // namespace mb3d_native
