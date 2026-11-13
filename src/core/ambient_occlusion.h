#ifndef MB3D_CORE_AMBIENT_OCCLUSION_H
#define MB3D_CORE_AMBIENT_OCCLUSION_H

#include "core/types.h"

namespace mb3d {

// ============================================================================
// Ambient Occlusion Calculation
// Ported from mb3d Pascal sources:
//   AmbShadowCalcThreadN.pas -- Type 0: 15-bit threshold AO
//   AmbHiQ.pas               -- Type 1: 24-bit threshold AO
//                             -- Type 2: 24-bit random AO
//   CalcAmbShadowDE.pas      -- Type 3: DE-based AO
// ============================================================================

// AO type (selected by bCalcAmbShadowAutomatic bits 3+4):
//   0 = 15-bit threshold AO   (AmbShadowCalcThreadN.pas)
//   1 = 24-bit threshold AO   (AmbHiQ.pas TAmbHiQCalcT0)
//   2 = 24-bit random AO      (AmbHiQ.pas TAmbHiQCalcR)
//   3 = DE-based AO            (CalcAmbShadowDE.pas)

// DE-based AO quality (bits 5+6 of bCalcAmbShadowAutomatic):
//   0 = 3 rays, 1 = 6 rays, 2 = 16 rays, 3 = 32 rays

// Result is stored in TsiLight5.AmbShadow (uint16).

void CalcAmbientOcclusion(
    TsiLight5* siLight,
    int width, int height,
    int slOffset,
    TMCTparameter* mcp,
    TLightVals* lvals,
    int aoType,          // 0-3 from bCalcAmbShadowAutomatic bits 3+4
    int aoQuality,       // from bits 5+6
    const TMandHeader10* header = nullptr, // needed for threshold-based AO params
    int logicalThreadCount = 1 // stochastic screen AO uses Pascal interleaved row lanes
);

// Single-position DE AO used by reflected/refracted rays.
// Mirrors Pascal CalcAmbShadowDEfor1pos from CalcAmbShadowDE.pas.
void CalcAmbientOcclusionForPoint(
    TMCTparameter* mcp,
    TIteration3Dext* it3Dext,
    TsiLight5* psi,
    int quality,
    int x = 0,
    int y = 0
);

// Row-range variant for multithreaded rendering.
// Processes rows [y_start, y_end).
// Caller must provide a per-thread copy of mcp (it will be mutated).
void CalcAmbientOcclusionRows(
    TsiLight5* siLight,
    int width, int height,
    int slOffset,
    TMCTparameter* mcp,
    TLightVals* lvals,
    int aoType,
    int aoQuality,
    const TMandHeader10* header,
    int y_start, int y_end
);

// Replays only the AO pass from an existing siLight5 buffer produced by the
// primary pass. Intended for parity-debug of late-stage AO without rerunning
// the full renderer.
void ReplayAmbientOcclusion(
    TsiLight5* siLight,
    int width, int height,
    int slOffset,
    TMCTparameter* mcp,
    TLightVals* lvals,
    int aoType,
    int aoQuality,
    const TMandHeader10* header,
    int threadCount = 1
);

void ReplayAmbientOcclusionPixel(
    TsiLight5* siLight,
    int width, int height,
    int slOffset,
    TMCTparameter* mcp,
    TLightVals* lvals,
    int aoType,
    int aoQuality,
    const TMandHeader10* header,
    int pixelX,
    int pixelY
);

// ============================================================================
// Wavelet smoothing passes (from AmbHiQ.pas)
// SmoothH/SmoothV are the horizontal and vertical 1D wavelet smoothing
// filters used by NextATlevelHiQ during AO calculation.
// These are already integrated into the AO pipeline but are also
// exposed here for standalone use.
// ============================================================================

// SmoothH -- horizontal wavelet smoothing pass.
// PIA: input/output array, SA: scratch copy of original data,
// ya: last valid index (count-1), Step: smoothing radius.
void SmoothH(uint32_t* PIA, const uint32_t* SA, int ya, int Step);

// SmoothV -- vertical wavelet smoothing pass.
// PIA: input/output array (strided), SA: scratch copy of column data,
// ye: last valid index (count-1), Step: smoothing radius,
// widBytes: stride in bytes between rows (4 * image_width for uint32 arrays).
void SmoothV(uint32_t* PIA, const uint32_t* SA, int ye, int Step, int widBytes);

// AmbSmooth -- post-processing smoothing pass for AO results.
// Performs the final summation of angular max arrays into per-pixel
// AmbShadow values. This is integrated into the AO pipeline's bSummUp
// phase but can also be called separately.
// Parameters match the Pascal TASCparameter fields:
//   siLight: per-pixel surface data
//   angMaxArr: 32-direction angular max array (width * height * 32 int16)
//   width, height: image dimensions
//   yStart, yEnd: row range [yStart, yEnd]
//   levelCount: total number of wavelet levels
//   threshold: AO threshold value
void AmbSmooth(
    TsiLight5* siLight,
    const int16_t* angMaxArr,
    int width, int height,
    int yStart, int yEnd,
    int levelCount,
    float threshold
);

} // namespace mb3d

#endif // MB3D_CORE_AMBIENT_OCCLUSION_H
