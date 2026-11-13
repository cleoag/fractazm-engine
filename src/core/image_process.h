#pragma once
#include <cstdint>
#include <vector>

namespace mb3d {

struct FinalRGBSurface {
    std::vector<uint8_t> pixels;
    int width = 0;
    int height = 0;
};

// Anti-aliasing downscale filter, ported from Pascal ImageProcess.pas doAA().
// When imageScale is 2 or 3, applies edge-aware lowpass + optional sharpening
// and downsamples from (fullWidth x fullHeight) to (fullWidth/imageScale x fullHeight/imageScale).
// Returns new buffer (caller owns via delete[]). Returns nullptr if no AA needed.
// channels must be 3 (RGB).
// sharpen: 0-3 (default 0 = pure lowpass, no sharpening).
uint8_t* SdoAA(const uint8_t* fullBuffer, int fullWidth, int fullHeight,
               int channels, int imageScale, int sharpen,
               int* outWidth, int* outHeight);

// Apply Pascal-like final save semantics for RGB output:
// 1. downscale according to ImageScale (1..10)
// 2. crop tiling border after scaling
// Returns false on invalid input; out is cleared on failure.
bool BuildFinalRGBSurface(const uint8_t* fullBuffer,
                          int fullWidth,
                          int fullHeight,
                          int imageScale,
                          int sharpen,
                          int tilingCrop,
                          FinalRGBSurface& out);

} // namespace mb3d
