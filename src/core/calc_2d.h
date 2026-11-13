#ifndef MB3D_CORE_CALC_2D_H
#define MB3D_CORE_CALC_2D_H

#include "core/types.h"

namespace mb3d {

// ============================================================================
// 2D Slice Rendering
// Ported from mb3d/CalcThread2D.pas -- T2DcalcThread.Execute
//
// When bCalc3D = 0 in the header, instead of 3D ray marching, the renderer
// computes a 2D slice through the fractal. Each pixel evaluates the fractal
// formula at the corresponding 3D position (determined by camera orientation
// and cut plane), then encodes the iteration result into the TsiLight5 buffer.
//
// The result is a 2D cross-section visualization rather than a 3D rendering.
// ============================================================================

// Calc2DSlice -- evaluate fractals on a 2D grid, writing per-pixel
// iteration data into the siLight buffer.
//
// Parameters:
//   siLight   -- per-pixel surface data buffer (width * height elements)
//   width     -- image width
//   height    -- image height
//   slOffset  -- row stride in bytes of siLight (typically width * sizeof(TsiLight5))
//   mcp       -- ray march / iteration parameters (will be mutated)
void Calc2DSlice(
    TsiLight5* siLight,
    int width, int height,
    int slOffset,
    TMCTparameter* mcp
);

// Row-range variant for multithreaded rendering.
// Processes rows [y_start, y_end).
void Calc2DSliceRows(
    TsiLight5* siLight,
    int width, int height,
    int slOffset,
    TMCTparameter* mcp,
    int y_start, int y_end
);

} // namespace mb3d

#endif // MB3D_CORE_CALC_2D_H
