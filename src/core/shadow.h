#ifndef MB3D_CORE_SHADOW_H
#define MB3D_CORE_SHADOW_H

#include "core/types.h"

namespace mb3d {

// ============================================================================
// Hard Shadow Calculation
// Ported from mb3d/CalcHardShadow.pas
// ============================================================================

// Calculate hard shadows for all pixels.
// Updates the Shadow field (bits 10-15) of each TsiLight5 entry.
//
// siLight:   per-pixel surface data buffer
// width:     image width in pixels
// height:    image height in pixels
// slOffset:  row stride in TsiLight5 units (typically == width)
// mcp:       ray march parameters (provides DE functions, camera, etc.)
// lvals:     light values (provides light directions, positions, etc.)
void CalcHardShadows(
    TsiLight5* siLight,
    int width, int height,
    int slOffset,
    TMCTparameter* mcp,
    TLightVals* lvals
);

// Row-range variant for multithreaded rendering.
// Processes rows [y_start, y_end).
// Caller must provide a per-thread copy of mcp (it will be mutated).
void CalcHardShadowsRows(
    TsiLight5* siLight,
    int width, int height,
    int slOffset,
    TMCTparameter* mcp,
    TLightVals* lvals,
    int y_start, int y_end
);

// Single-point hard shadow used by reflected/refracted rays.
// Mirrors Pascal CalcHS/CalcHSsoft from Calc.pas.
void CalcHardShadowForPoint(
    TMCTparameter* mcp,
    TIteration3Dext* it3Dext,
    TsiLight5* psi,
    TLightVals* lvals,
    int y
);

// Replays the hard-shadow pass for one row prefix [0..target_x] on row target_y
// using an existing siLight5 buffer produced by the primary pass.
void ReplayHardShadowsPrefix(
    TsiLight5* siLight,
    int width, int height,
    int slOffset,
    TMCTparameter* mcp,
    TLightVals* lvals,
    int target_x,
    int target_y
);

} // namespace mb3d

#endif // MB3D_CORE_SHADOW_H
