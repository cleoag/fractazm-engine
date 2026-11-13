#ifndef MB3D_CORE_DOF_H
#define MB3D_CORE_DOF_H

#include "core/types.h"
#include <cstdint>

namespace mb3d {

// ============================================================================
// Depth of Field Calculation
// Ported from mb3d/DOF.pas
//
// Post-processing pass that applies bokeh blur based on z-buffer depth.
// Supports:
//   - Circle of confusion from z-buffer distance to focus plane (sDOFZsharp)
//   - Optional second focus point (sDOFZsharp2) with interpolated focus zone
//   - Aperture-based blur kernel (sDOFaperture)
//   - Multiple pass modes via bCalcDOFtype bits 1-2 (1, 2, or 3 passes)
//   - Clip radius (sDOFclipR)
//   - Forward (scatter) mode: doDOF
//   - Sorted back-to-front mode: doDOFsort (bit 3 clear)
// ============================================================================

// Sort item for depth-sorted DOF
struct TSortItem {
    int32_t  iZ;
    uint16_t wX;
    uint16_t wY;
};

// Quicksort for TSortItem array by iZ ascending
void QuickSortInt(int count, TSortItem* list);

// Main entry point: dispatches to doDOF or doDOFsort based on bCalcDOFtype
// Runs the required number of passes internally.
void CalcDepthOfField(
    uint8_t* imageBuffer,        // RGB image to blur (3 bytes per pixel)
    const TsiLight5* siLight,    // per-pixel surface data
    int width, int height,
    int slOffset,                // RGB row stride in pixels; depth is tightly packed
    TMCTparameter* mcp,
    const TMandHeader10* header
);

} // namespace mb3d

#endif // MB3D_CORE_DOF_H
