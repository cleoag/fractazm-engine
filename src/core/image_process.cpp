#include "core/render_control.h"
#include "core/image_process.h"
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <memory>
#include <vector>

namespace mb3d {

// Filter coefficients from Pascal ImageProcess.pas doAA()
static const float cLP2[17] = {
    129.447f, 80.065f, -1.5584f, -24.16f,
    0.793f, 12.267f, -0.1375f, -7.096f, -0.2744f, 4.324f, 0.472f, -2.69f, -0.526f, 1.68f,
    0.5f, -1.0f, -0.4f
};

static const float cLP3[25] = {
    86.299f, 70.497f, 33.722f, -1.0383f, -16.482f,
    -11.906f, 0.529f, 8.0127f, 6.317f, -0.091f, -4.458f, -3.818f, -0.183f, 2.585f, 2.446f,
    0.3147f, -1.512f, -1.609f, -0.35f, 0.875f, 1.068f, 0.332f, -0.493f, -0.708f, -0.28f
};

// Sharpen filter coefficients
static const float cSP2[17] = {
    0.74581218f, 0.307845f, -0.135913f, -0.12739f,
    0.03f, 0.07615f, 0.00947f, -0.0345f, -0.0174f, 0.0131f, 0.0135f, -0.00189f, -0.0084f, -0.00164f,
    0.00383f, 0.00252f, -0.00142f
};

static const float cSP3[25] = {
    0.508389f, 0.338648f, 0.07211f, -0.08756f, -0.1117f,
    -0.04961f, 0.0207f, 0.05232f, 0.0394f, -0.0184f, -0.0229f, -0.0114f, 0.0035f, 0.0111f, 0.0092f,
    0.0021f, -0.0039f, -0.0055f, -0.003f, 0.0005f, 0.0026f, 0.0024f, 0.0007f, 0.0f, 0.0f
};

// Port of Pascal doAA from ImageProcess.pas lines 370-525.
// Pascal operates on 4-byte/pixel bitmaps (BGRA); our C++ buffers are 3 bytes/pixel (RGB).
// fullBuf: full-resolution RGB buffer (Hwidth*Scale * Hheight*Scale * 3)
// outBuf: downsampled RGB buffer (Hwidth * Hheight * 3)
// Hwidth, Hheight: output (half) dimensions
// Scale: 2 or 3
// Sharpen: 0-3
static void doAA(const uint8_t* fullBuf, uint8_t* outBuf,
                 int Hwidth, int Hheight, int Scale, int Sharpen) {
    int n = Scale * 8;
    int filterLen = (Scale == 2) ? 17 : 25;

    // Build combined lowpass + sharpen filter
    float aLP[25];
    float sSh;
    if (Scale == 2)
        sSh = Sharpen * 200.0f / 3.0f;
    else
        sSh = Sharpen * 180.0f / 3.0f;

    if (Scale == 2) {
        for (int i = 0; i < 17; i++)
            aLP[i] = cLP2[i] + sSh * cSP2[i];
    } else {
        for (int i = 0; i < 25; i++)
            aLP[i] = cLP3[i] + sSh * cSP3[i];
    }

    int fullStride = Hwidth * Scale * 3; // bytes per row in full-res buffer
    int halfStride = Hwidth * 3;         // bytes per row in intermediate/output buffer

    // Intermediate buffer: Hwidth * 3 bytes wide, Hheight * Scale rows tall
    std::vector<uint8_t> Buf(Hwidth * Hheight * 3 * Scale, 0);

    // --- Horizontal lowpass pass ---
    // Pascal: for y := 0 to Hheight * Scale - 1 do
    //   PB1 := PByte(StartSLfullBMP + OffsetFullBMP * y)  -- points to start of row y in full BMP
    //   PBh := @Buf[y * Hwidth * 3]                        -- output row in intermediate
    //   for x := 1 to Hwidth do                             -- 1-based pixel index
    //     for c := 0 to 2 do                                -- R,G,B
    //       ... filter along horizontal axis with stride 4 (Pascal BGRA) -> stride 3 (our RGB)
    //       PB1 advances by 1 per channel, then by (4*Scale-3) per pixel
    // In Pascal, sub-pixel stride is 4 bytes (32-bit BMP). In C++, stride is 3 bytes (RGB).
    // So Pascal's +4/-4 per sub-pixel becomes +3/-3 in C++.

    for (int y = 0; y < Hheight * Scale; y++) {
        RenderPoll();
        const uint8_t* rowFull = fullBuf + y * fullStride;
        uint8_t* rowBuf = &Buf[y * halfStride];

        // PB1 starts at beginning of row, pointing at first channel of first pixel
        // In Pascal, PB1 advances through channels then skips to next pixel
        // x is 1-based in Pascal (1..Hwidth)
        const uint8_t* PB1 = rowFull;

        for (int x = 1; x <= Hwidth; x++) {
            RenderWork();
            for (int c = 0; c < 3; c++) {
                // Edge threshold detection
                // Pascal: iThr := Abs(PB1^ - PByte(Integer(PB1) + 4)^)
                // In Pascal +4 means next sub-pixel (4 bytes/pixel BMP). In C++ it's +3.
                int iThr = std::abs((int)PB1[0] - (int)PB1[3]); // next sub-pixel, same channel

                if (x < Hwidth) {
                    // Pascal: +8 means 2 sub-pixels ahead -> C++ +6
                    int x2 = std::abs((int)PB1[0] - (int)PB1[6]);
                    if (x2 > iThr) iThr = x2;
                    if (Scale == 3 && x < Hwidth - 1) {
                        // Pascal: +12 -> C++ +9
                        int x2b = std::abs((int)PB1[0] - (int)PB1[9]);
                        if (x2b > iThr) iThr = x2b;
                    }
                }
                if (x > 1) {
                    // Pascal: -4 -> C++ -3
                    int x2 = std::abs((int)PB1[0] - (int)PB1[-3]);
                    if (x2 > iThr) iThr = x2;
                    // Pascal: -8 -> C++ -6
                    x2 = std::abs((int)PB1[0] - (int)PB1[-6]);
                    if (x2 > iThr) iThr = x2;
                    if (Scale == 3 && x > 2) {
                        // Pascal: -12 -> C++ -9
                        int x2b = std::abs((int)PB1[0] - (int)PB1[-9]);
                        if (x2b > iThr) iThr = x2b;
                    }
                }
                iThr *= 10;

                float sVal = 0.0f;
                float sWei = 0.0f;
                const uint8_t* PB2 = PB1;

                // Forward pass: x2 := 0 to Min(n, (Hwidth - x) * Scale)
                int fwdLimit = std::min(n, (Hwidth - x) * Scale);
                for (int x2 = 0; x2 <= fwdLimit; x2++) {
                    if (std::abs((int)PB2[0] - (int)PB1[0]) > iThr) break;
                    sVal += aLP[x2] * (float)(int)PB2[0];
                    sWei += aLP[x2];
                    PB2 += 3; // Pascal: Inc(PB2, 4) -> C++ +3
                }

                // Backward pass: x2 := 1 to Min(n, (x - 1) * Scale)
                PB2 = PB1;
                int bwdLimit = std::min(n, (x - 1) * Scale);
                for (int x2 = 1; x2 <= bwdLimit; x2++) {
                    PB2 -= 3; // Pascal: Dec(PB2, 4) -> C++ -3
                    if (std::abs((int)PB2[0] - (int)PB1[0]) > iThr) break;
                    sVal += aLP[x2] * (float)(int)PB2[0];
                    sWei += aLP[x2];
                }

                int outIdx = (x - 1) * 3 + c;
                if (sWei > 0.0f) {
                    int v = (int)std::round(sVal / sWei);
                    if (v < 0) v = 0;
                    if (v > 255) v = 255;
                    rowBuf[outIdx] = (uint8_t)v;
                }

                PB1++; // next channel (Pascal: Inc(PB1))
            }
            // Pascal: Inc(PB1, 4 * Scale - 3) -> skip to next pixel group
            // In Pascal, after processing 3 channels of a pixel (advancing PB1 by 3),
            // it skips by (4*Scale - 3) to reach the start of the next output pixel.
            // In our 3-byte format: after 3 channels we've advanced 3 bytes within
            // the current Scale-pixel group. We need to skip to the next group.
            // Each group is Scale pixels * 3 bytes = 3*Scale bytes.
            // We've consumed 3 bytes, so skip (3*Scale - 3).
            PB1 += (3 * Scale - 3);
        }
    }

    // --- Vertical lowpass pass ---
    // Pascal: for x := 0 to Hwidth - 1 do
    //   PB1 := @Buf[x * 3]                                    -- column x in intermediate
    //   PBh := PByte(StartSLhalfBMP + x * 4)                  -- column x in output BMP (4 bytes/pixel)
    //   for y := 1 to Hheight do
    //     for c := 0 to 2 do
    //       ... filter along vertical axis with stride 3*Hwidth (intermediate row stride)
    //       PB1 advances by 1 per channel, then by (Hwidth*3*Scale - 3) per pixel row
    //       PBh advances by 1 per channel, then by (OffsetHalfBMP - 3)
    // In Pascal, vertical sub-pixel stride in Buf is 3*Hwidth (one row of intermediate).
    // Output stride: Pascal uses OffsetHalfBMP (4-byte BMP row stride). C++ uses 3*Hwidth.

    int bufRowStride = halfStride; // 3 * Hwidth — one row in intermediate Buf
    int outRowStride = halfStride; // 3 * Hwidth — one row in output (also RGB)

    for (int x = 0; x < Hwidth; x++) {
        RenderPoll();
        const uint8_t* PB1base = &Buf[x * 3];
        uint8_t* PBhBase = outBuf + x * 3;

        const uint8_t* PB1 = PB1base;

        for (int y = 1; y <= Hheight; y++) {
            RenderWork();
            for (int c = 0; c < 3; c++) {
                // Edge threshold: compare with next sub-pixel row
                // Pascal: +3*Hwidth means next row in intermediate
                int iThr = std::abs((int)PB1[0] - (int)PB1[bufRowStride]);

                if (y < Hheight) {
                    // Pascal: +6*Hwidth -> 2 rows ahead
                    int x2 = std::abs((int)PB1[0] - (int)PB1[2 * bufRowStride]);
                    if (x2 > iThr) iThr = x2;
                    if (Scale == 3 && y < Hheight - 1) {
                        // Pascal: +9*Hwidth -> 3 rows ahead
                        int x2b = std::abs((int)PB1[0] - (int)PB1[3 * bufRowStride]);
                        if (x2b > iThr) iThr = x2b;
                    }
                }
                if (y > 1) {
                    // Pascal: -3*Hwidth -> previous row
                    int x2 = std::abs((int)PB1[0] - (int)PB1[-bufRowStride]);
                    if (x2 > iThr) iThr = x2;
                    // Pascal: -6*Hwidth -> 2 rows back
                    x2 = std::abs((int)PB1[0] - (int)PB1[-2 * bufRowStride]);
                    if (x2 > iThr) iThr = x2;
                    if (Scale == 3 && y > 2) {
                        // Pascal: -9*Hwidth -> 3 rows back
                        int x2b = std::abs((int)PB1[0] - (int)PB1[-3 * bufRowStride]);
                        if (x2b > iThr) iThr = x2b;
                    }
                }
                iThr *= 10;

                float sVal = 0.0f;
                float sWei = 0.0f;
                const uint8_t* PB2 = PB1;

                // Forward (downward): x2 := 0 to Min(n, (Hheight - y) * Scale)
                int fwdLimit = std::min(n, (Hheight - y) * Scale);
                for (int x2 = 0; x2 <= fwdLimit; x2++) {
                    if (std::abs((int)PB2[0] - (int)PB1[0]) > iThr) break;
                    sVal += aLP[x2] * (float)(int)PB2[0];
                    sWei += aLP[x2];
                    PB2 += bufRowStride; // next row in intermediate
                }

                // Backward (upward): x2 := 1 to Min(n, (y - 1) * Scale)
                PB2 = PB1;
                int bwdLimit = std::min(n, (y - 1) * Scale);
                for (int x2 = 1; x2 <= bwdLimit; x2++) {
                    PB2 -= bufRowStride;
                    if (std::abs((int)PB2[0] - (int)PB1[0]) > iThr) break;
                    sVal += aLP[x2] * (float)(int)PB2[0];
                    sWei += aLP[x2];
                }

                int outOffset = (y - 1) * outRowStride + c;
                if (sWei > 0.0f) {
                    int v = (int)std::round(sVal / sWei);
                    if (v < 0) v = 0;
                    if (v > 255) v = 255;
                    PBhBase[outOffset] = (uint8_t)v;
                }

                PB1++; // next channel
            }
            // Pascal: Inc(PB1, Hwidth * 3 * Scale - 3)
            // After processing 3 channels (advancing by 3), skip to next output row position.
            // In intermediate Buf, each output pixel group spans Scale rows.
            // So advance by Scale rows minus the 3 bytes already consumed.
            PB1 += (bufRowStride * Scale - 3);
        }
    }
}

uint8_t* SdoAA(const uint8_t* fullBuffer, int fullWidth, int fullHeight,
               int channels, int imageScale, int sharpen,
               int* outWidth, int* outHeight) {
    RenderPoll();
    if (channels != 3) return nullptr;
    if (imageScale < 2 || imageScale > 3) return nullptr;

    int Hwidth = fullWidth / imageScale;
    int Hheight = fullHeight / imageScale;
    if (Hwidth < 1 || Hheight < 1) return nullptr;

    *outWidth = Hwidth;
    *outHeight = Hheight;

    std::unique_ptr<uint8_t[]> result(new uint8_t[Hwidth * Hheight * 3]);
    std::memset(result.get(), 0, Hwidth * Hheight * 3);

    doAA(fullBuffer, result.get(), Hwidth, Hheight, imageScale, sharpen);

    return result.release();
}

static bool DownscaleRGBBoxAverage(const uint8_t* fullBuffer,
                                   int fullWidth,
                                   int fullHeight,
                                   int imageScale,
                                   FinalRGBSurface& out) {
    if (!fullBuffer || fullWidth <= 0 || fullHeight <= 0 || imageScale < 1) {
        out = {};
        return false;
    }

    const int outWidth = fullWidth / imageScale;
    const int outHeight = fullHeight / imageScale;
    if (outWidth < 1 || outHeight < 1) {
        out = {};
        return false;
    }

    out.width = outWidth;
    out.height = outHeight;
    out.pixels.assign(static_cast<size_t>(outWidth) * outHeight * 3, 0);

    if (imageScale == 1) {
        RenderWork(static_cast<uint64_t>(fullWidth) * fullHeight);
        std::memcpy(out.pixels.data(), fullBuffer, static_cast<size_t>(fullWidth) * fullHeight * 3);
        return true;
    }

    const int divisor = imageScale * imageScale;
    const int roundBias = divisor / 2;
    for (int y = 0; y < outHeight; ++y) {
        RenderPoll();
        for (int x = 0; x < outWidth; ++x) {
            RenderWork();
            int sums[3] = {0, 0, 0};
            const int srcY = y * imageScale;
            const int srcX = x * imageScale;
            for (int yy = 0; yy < imageScale; ++yy) {
                RenderPoll();
                const uint8_t* srcRow = fullBuffer + static_cast<size_t>(srcY + yy) * fullWidth * 3;
                for (int xx = 0; xx < imageScale; ++xx) {
                    RenderWork();
                    const uint8_t* src = srcRow + static_cast<size_t>(srcX + xx) * 3;
                    sums[0] += src[0];
                    sums[1] += src[1];
                    sums[2] += src[2];
                }
            }

            uint8_t* dst = out.pixels.data() + (static_cast<size_t>(y) * outWidth + x) * 3;
            dst[0] = static_cast<uint8_t>((sums[0] + roundBias) / divisor);
            dst[1] = static_cast<uint8_t>((sums[1] + roundBias) / divisor);
            dst[2] = static_cast<uint8_t>((sums[2] + roundBias) / divisor);
        }
    }
    return true;
}

static bool CropRGBSurface(const FinalRGBSurface& input, int tilingCrop, FinalRGBSurface& out) {
    if (input.width <= 0 || input.height <= 0 ||
        input.pixels.size() != static_cast<size_t>(input.width) * input.height * 3) {
        out = {};
        return false;
    }

    out = input;
    if (tilingCrop <= 0) return true;
    if (input.width <= 2 * tilingCrop || input.height <= 2 * tilingCrop) return true;

    out.width = input.width - 2 * tilingCrop;
    out.height = input.height - 2 * tilingCrop;
    out.pixels.assign(static_cast<size_t>(out.width) * out.height * 3, 0);

    for (int row = 0; row < out.height; ++row) {
        RenderWork(static_cast<uint64_t>(out.width));
        const uint8_t* src = input.pixels.data() +
                             (static_cast<size_t>(row + tilingCrop) * input.width + tilingCrop) * 3;
        uint8_t* dst = out.pixels.data() + static_cast<size_t>(row) * out.width * 3;
        std::memcpy(dst, src, static_cast<size_t>(out.width) * 3);
    }
    return true;
}

bool BuildFinalRGBSurface(const uint8_t* fullBuffer,
                          int fullWidth,
                          int fullHeight,
                          int imageScale,
                          int sharpen,
                          int tilingCrop,
                          FinalRGBSurface& out) {
    RenderPoll();
    out = {};
    if (!fullBuffer || fullWidth <= 0 || fullHeight <= 0) return false;

    FinalRGBSurface scaled;
    const int normalizedScale = std::max(1, std::min(10, imageScale));
    if (normalizedScale >= 2 && normalizedScale <= 3) {
        int aaWidth = 0;
        int aaHeight = 0;
        std::unique_ptr<uint8_t[]> aaFiltered(SdoAA(fullBuffer, fullWidth, fullHeight, 3,
                                    normalizedScale, sharpen, &aaWidth, &aaHeight));
        if (aaFiltered) {
            scaled.width = aaWidth;
            scaled.height = aaHeight;
            scaled.pixels.assign(aaFiltered.get(),
                                 aaFiltered.get() + static_cast<size_t>(aaWidth) * aaHeight * 3);
        } else if (!DownscaleRGBBoxAverage(fullBuffer, fullWidth, fullHeight, normalizedScale, scaled)) {
            return false;
        }
    } else {
        if (!DownscaleRGBBoxAverage(fullBuffer, fullWidth, fullHeight, normalizedScale, scaled)) {
            return false;
        }
    }

    int scaledCrop = tilingCrop;
    if (normalizedScale >= 2) scaledCrop /= normalizedScale;
    return CropRGBSurface(scaled, scaledCrop, out);
}

} // namespace mb3d
