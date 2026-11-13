#pragma once
#include "formula_abi.h"
namespace mb3d_native {
// Native volumetric light (header bVolLightNr&7, DFogOnIt 65535). The host
// prepares the map geometry with PrepareVolLightMap; the GPU marches one ray
// per map cell (VLM_ComputeOneRay), then the primary march samples the map in
// DoDynFog sub-steps. Native arithmetic is FP32 throughout, so the map, the
// sub-step positions and the step count follow the CPU only within FP32 tolerance.
enum : U32 { kNativeVolumeLightVersion=1, kNativeVolumeFog=65535, kNativeVolumeMapSteps=10000,
    kNativeVolumeMaxSubsteps=1u<<20 };
struct alignas(16) NativeVolumeLight {
    U32 version,positional,side_count,cube_size; // IsPosLight; 6 or 1 faces of cube_size^2 cells
    I32 half_size,inside; U32 in_and_out,reserved;  // map march: bInsideRendering LongBool, bInAndOutside
    float stretch,size_factor,csize,hsize;          // StretchSide1, SizeFactor, CsizeS, HSizeS
    float min_distance,max_distance,cell_step,step_width; // MinDistance, maxDist, sgl, StepWidth
    float vl_mul,vl_step_mul,reserved_scalar[2];    // VLmul, VLstepmul
    float light[4];                                 // LightPos
    float rotation[3][4];                           // RotMatrix rows (directional lookup)
    float inverse[3][4];                            // invMs rows (directional map rays; row 2 = dirVec)
    float origin[4];                                // Ystart centered on the image, for the map's DE stop
    float forward[4];                               // Vgrads[2]
};
// One record per map cell: fault bits as in NativeGeometryRecord, FP32 range exits.
struct alignas(8) NativeVolumeCell { U32 fault,range_exits; };
#ifndef __METAL_VERSION__
static_assert(sizeof(NativeVolumeLight)==224);
static_assert(sizeof(NativeVolumeCell)==8);
#endif
}
