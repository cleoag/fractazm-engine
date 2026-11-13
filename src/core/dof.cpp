#include "core/render_control.h"
#include "core/dof.h"
#include "core/header_trafos.h"
#include "core/math3d.h"
#include <cmath>
#include <cstring>
#include <cstdio>
#include <algorithm>
#include <vector>

namespace mb3d {

// ============================================================================
// Internal helpers
// ============================================================================

// Sign function: returns +1 for positive, -1 for negative, 0 for zero
static inline float Sign(float x) {
    if (x > 0.0f) return 1.0f;
    if (x < 0.0f) return -1.0f;
    return 0.0f;
}

// Max0S is inline in math3d.h

// ============================================================================
// ColToSVecNoScale: Extract BGRA Cardinal -> TSVec (no /255 scaling)
// In the C++ port, image is RGB 3-byte layout, so we provide helpers
// for both uint32_t BGRA (used by Pascal) and uint8_t* RGB
// ============================================================================

static inline void ColToSVecNoScale(float* result, uint32_t c) {
    result[0] = static_cast<float>(c & 0xFF);
    result[1] = static_cast<float>((c >> 8) & 0xFF);
    result[2] = static_cast<float>((c >> 16) & 0xFF);
}

// RGB 3-byte version: reads 3 bytes and stores as float[3]
static inline void RGBToSVec(float* result, const uint8_t* rgb) {
    result[0] = static_cast<float>(rgb[0]);
    result[1] = static_cast<float>(rgb[1]);
    result[2] = static_cast<float>(rgb[2]);
}

// ============================================================================
// SVecToColNoScale: TSVec -> clamped uint8_t RGB
// ============================================================================

static inline void SVecToRGB(uint8_t* rgb, const float* sv) {
    auto clampByte = [](float v) -> uint8_t {
        if (v < 0.0f) return 0;
        if (v > 255.0f) return 255;
        return static_cast<uint8_t>(v + 0.5f);
    };
    rgb[0] = clampByte(sv[0]);
    rgb[1] = clampByte(sv[1]);
    rgb[2] = clampByte(sv[2]);
}

// ============================================================================
// AddSVecWeightS: dest[0..2] += src[0..2] * w
// ============================================================================

static inline void AddSVecWeightS(float* dest, const float* src, float w) {
    dest[0] += src[0] * w;
    dest[1] += src[1] * w;
    dest[2] += src[2] * w;
}

// ============================================================================
// ScaleSVectorV: sv[0..2] *= s (in place)
// ============================================================================

static inline void ScaleSVecV(float* sv, float s) {
    sv[0] *= s;
    sv[1] *= s;
    sv[2] *= s;
}

// ============================================================================
// QuickSort for TSortItem by iZ (ascending -- back-to-front for sorted DOF)
// Direct port from DOF.pas QuickSortInt
// ============================================================================

static void QuickSortImpl(int L, int R, TSortItem* list) {
    int LPos = L - 1;
    int RPos = R;
    int ListR = list[R].iZ;
    TSortItem Tmp;

    for (;;) {
        RenderPoll();
        do { RenderWork(); LPos++; } while (list[LPos].iZ < ListR);
        do { RenderWork(); RPos--; } while (RPos > LPos && list[RPos].iZ > ListR);
        if (LPos >= RPos) break;
        Tmp = list[LPos]; list[LPos] = list[RPos]; list[RPos] = Tmp;
    }
    Tmp = list[LPos]; list[LPos] = list[R]; list[R] = Tmp;
    if (LPos - 1 > L) QuickSortImpl(L, LPos - 1, list);
    if (R > LPos + 1) QuickSortImpl(LPos + 1, R, list);
}

void QuickSortInt(int count, TSortItem* list) {
    if (count <= 1) return;
    QuickSortImpl(0, count - 1, list);
}

// ============================================================================
// doDOF -- Forward scatter DOF (non-sorted mode)
// Direct port from DOF.pas procedure doDOF (line 110-252)
//
// For each source pixel, scatter its color into a blur buffer weighted
// by the circle-of-confusion radius at that pixel's depth.
// Pixels farther from the focus plane get larger scatter radius.
// Pixels in front of the focus point have their contribution downweighted
// relative to closer pixels (to avoid foreground leaking onto background).
// ============================================================================

static void doDOF(
    uint8_t* imageBuffer,       // RGB 3-byte layout
    const TsiLight5* siLight,
    int width, int height,
    int slOffset,
    const TMandHeader10* header,
    int pass)
{
    double Zcorr, ZcMul, ZZstmitDif;
    // Use a local copy of header for CalcPPZvals (it may modify dStepWidth)
    TMandHeader10 hdrCopy = *header;
    CalcPPZvals(hdrCopy, Zcorr, ZcMul, ZZstmitDif);

    float sDOFaperture = MinMaxCS(0.0001f, header->sDOFaperture, 2.0f);
    float sDOFclipR = MinMaxCS(0.1f, header->sDOFclipR, 1000.0f);

    int lp = (header->bCalcDOFtype >> 1) & 3;
    float Ap;
    switch (lp) {
        case 0:
            Ap = sDOFaperture * 0.5f;
            break;
        case 1:
            Ap = sDOFaperture * (0.3f + pass * 0.55f) * 0.5f;
            break;
        default:
            Ap = sDOFaperture / 12.0f * static_cast<float>(1 << pass);
            break;
    }

    float DEstopFactor = static_cast<float>(MaxCD(0.0, header->dFOVy * Pid180)) / height;
    ZcMul = 1.0 / ZcMul;
    Zcorr = 1.0 / Zcorr;

    float sw_f = MinCS(header->sDOFZsharp, header->sDOFZsharp2) * width;
    float sMs_f = MaxCS(header->sDOFZsharp, header->sDOFZsharp2) * width;
    float w_f = (1.0f + sw_f * DEstopFactor) / (1.0f + sMs_f * DEstopFactor);
    float Zsharp = (w_f * sMs_f + sw_f) / (1.0f + w_f);
    float Rsub = std::fabs((sw_f - Zsharp) / (1.0f + sw_f * DEstopFactor) * Ap);

    int totalPixels = width * height;

    // Allocate buffers
    std::vector<float> RBuf(totalPixels);         // blur radius per pixel
    std::vector<float> TranspBuf(totalPixels, 0.0f); // accumulated weight
    std::vector<float> ColBuf(totalPixels * 3, 0.0f); // accumulated color (RGB)

    // Calculate blur radius for each pixel
    const TsiLight5* SL2 = siLight;
    float sMaxR = 0.0f;
    // Pascal uses Sqr(x) = x*x (exact mul); std::pow(x, 2.0) goes through
    // exp(2*log(x)) which drifts by 1+ ULP. Force x*x for bit-exact parity.
    double sMs_bg_base = 8388353.0 * ZcMul + 1.0;
    float sMs_bg = static_cast<float>((sMs_bg_base * sMs_bg_base - 1.0) * Zcorr);

    for (int i = 0; i < totalPixels; i++) {
        RenderWork();
        float w;
        if (SL2->Zpos > 32767) {
            w = sMs_bg;
        } else {
            // Extract 24-bit Z from RoughZposFine + Zpos
            uint32_t rawZ;
            std::memcpy(&rawZ, &SL2->RoughZposFine, sizeof(int32_t));
            rawZ >>= 8;
            double base = (8388352.0 - rawZ) * ZcMul + 1.0;
            w = static_cast<float>((base * base - 1.0) * Zcorr);
        }

        float r = (w - Zsharp) / (1.0f + w * DEstopFactor) * Ap;
        r = Max0S(std::fabs(r) - Rsub) * Sign(r);
        if (std::fabs(r) > sDOFclipR) r = sDOFclipR * Sign(r);
        if (std::fabs(r) > sMaxR) sMaxR = std::fabs(r);
        RBuf[i] = r;
        SL2++;
    }

    float SM = 1.0f / (1.0f + sMaxR);

    // Scatter pass: for each pixel, spread its color to nearby pixels
    float* pR = RBuf.data();
    float* pT = TranspBuf.data();
    float* pC = ColBuf.data();

    for (int y = 0; y < height; y++) {
        RenderPoll();
        const uint8_t* rowImg = imageBuffer + y * slOffset * 3;

        for (int x = 0; x < width; x++) {
            RenderWork();
            float radius = *pR;
            float sqrRadius = (std::fabs(radius) + 1.0f);
            sqrRadius *= sqrRadius;

            float rgbSV[3];
            RGBToSVec(rgbSV, rowImg + x * 3);

            float sw_pixel = 0.5f / sqrRadius;
            int r_int = static_cast<int>(std::lrint(std::fabs(radius) + 0.5f));

            // Clamp scatter bounds
            // Pascal's source column is 1..Width; this loop is 0..width-1.
            int xa = (r_int > x) ? -x : -r_int;
            int xe = (r_int + x >= width) ? (width - x - 1) : r_int;
            int ya = (r_int > y) ? -y : -r_int;
            int ye = (r_int + y >= height) ? (height - y - 1) : r_int;

            int lineSkip = width - xe + xa - 1;

            // Index into center of scatter region
            int baseIdx = (y + ya) * width + (x + xa);
            float* pT_local = &TranspBuf[baseIdx];
            float* pC_local = &ColBuf[baseIdx * 3];
            float* pR_local = &RBuf[baseIdx];

            for (int yy = ya; yy <= ye; yy++) {
                RenderPoll();
                for (int xx = xa; xx <= xe; xx++) {
                    RenderWork();
                    float wt = sqrRadius - static_cast<float>(xx * xx) - static_cast<float>(yy * yy);
                    if (wt > 0.0f) {
                        if (wt > 1.0f) wt = 1.0f;
                        // Down-weight if scatter target is closer to camera than source
                        if (*pR_local >= radius)
                            wt = wt * sw_pixel;
                        else
                            wt = wt * sw_pixel * std::fabs(*pR_local) * SM;

                        *pT_local += wt;
                        AddSVecWeightS(pC_local, rgbSV, wt);
                    }
                    pT_local++;
                    pR_local++;
                    pC_local += 3;
                }
                pT_local += lineSkip;
                pR_local += lineSkip;
                pC_local += lineSkip * 3;
            }

            pR++;
            pT++;
            pC += 3;
        }
    }

    // Normalize and write back
    pT = TranspBuf.data();
    pC = ColBuf.data();
    for (int y = 0; y < height; y++) {
        RenderPoll();
        uint8_t* rowImg = imageBuffer + y * slOffset * 3;
        for (int x = 0; x < width; x++) {
            RenderWork();
            float invW = (*pT < 1e-10f) ? 1e10f : (1.0f / *pT);
            float out[3];
            out[0] = pC[0] * invW;
            out[1] = pC[1] * invW;
            out[2] = pC[2] * invW;
            SVecToRGB(rowImg + x * 3, out);
            pT++;
            pC += 3;
        }
    }
}

// ============================================================================
// doDOFsort -- Sorted (back-to-front) DOF mode
// Direct port from DOF.pas procedure doDOFsort (line 313-507)
//
// Sorts all pixels by depth (back to front), then scatters each pixel's
// color into a buffer. Front pixels can occlude back pixels via a
// transparency accumulation scheme. Produces more physically correct
// bokeh than the unsorted mode.
// ============================================================================

static void doDOFsort(
    uint8_t* imageBuffer,       // RGB 3-byte layout
    const TsiLight5* siLight,
    int width, int height,
    int slOffset,
    const TMandHeader10* header,
    int pass)
{
    double Zcorr, ZcMul, ZZstmitDif;
    TMandHeader10 hdrCopy = *header;
    CalcPPZvals(hdrCopy, Zcorr, ZcMul, ZZstmitDif);

    float sDOFaperture = MinMaxCS(0.0001f, header->sDOFaperture, 2.0f);
    float sDOFclipR = MinMaxCS(0.1f, header->sDOFclipR, 1000.0f);

    int lp_mode = (header->bCalcDOFtype >> 1) & 3;
    float Ap;
    switch (lp_mode) {
        case 0:
            Ap = sDOFaperture * 0.5f;
            break;
        case 1:
            Ap = sDOFaperture * (0.25f + pass * 0.58f) * 0.5f;
            break;
        default:
            Ap = sDOFaperture / 22.0f * static_cast<float>(std::pow(3.0, pass));
            break;
    }

    float DEstopFactor = static_cast<float>(MaxCD(0.0, header->dFOVy * Pid180)) / height;
    ZcMul = 1.0 / ZcMul;
    Zcorr = 1.0 / Zcorr;

    float dw = MinCS(header->sDOFZsharp, header->sDOFZsharp2) * width;
    float st = MaxCS(header->sDOFZsharp, header->sDOFZsharp2) * width;
    float w_f = (1.0f + dw * DEstopFactor) / (1.0f + st * DEstopFactor);
    float Zsharp = (w_f * st + dw) / (1.0f + w_f);
    float Rsub = std::fabs((dw - Zsharp) / (1.0f + dw * DEstopFactor) * Ap);

    int totalPixels = width * height;

    // Allocate buffers
    std::vector<TSortItem> sortList(totalPixels);
    std::vector<float> TranspBuf(totalPixels, 0.0f);
    std::vector<float> ColBuf(totalPixels * 3 + 1, 0.0f);

    // Find nearest and farthest pixels for clipping maxR
    const TsiLight5* SL2 = siLight;
    int nearestZ = 32768;
    int farestZ = 0;
    for (int i = 0; i < totalPixels; i++) {
        RenderWork();
        if (SL2->Zpos < 32768) {
            if (SL2->Zpos < nearestZ) nearestZ = SL2->Zpos;
            if (SL2->Zpos > farestZ) farestZ = SL2->Zpos;
        }
        SL2++;
    }

    // Calculate maxR from nearest/farthest
    double w_near_base = (32768 - nearestZ) * 256.0 * ZcMul + 1.0;
    float w_near = static_cast<float>((w_near_base * w_near_base - 1.0) * Zcorr);
    float sRadius_near = std::fabs((w_near - Zsharp) * Ap / (1.0f + w_near * DEstopFactor));

    double w_far_base = (32768 - farestZ) * 256.0 * ZcMul + 1.0;
    float w_far = static_cast<float>((w_far_base * w_far_base - 1.0) * Zcorr);
    float maxR = MinCS(sDOFclipR, MaxCS(sRadius_near,
                 std::fabs((w_far - Zsharp) / (1.0f + w_far * DEstopFactor) * Ap)) * 1.5f);

    // Build sort list and clear color buffer
    SL2 = siLight;
    float* pC = ColBuf.data();
    TSortItem* pSI = sortList.data();
    for (int y = 0; y < height; y++) {
        RenderPoll();
        for (int x = 0; x < width; x++) {
            RenderWork();
            uint32_t rawZ;
            std::memcpy(&rawZ, &SL2->RoughZposFine, sizeof(int32_t));
            pSI->iZ = rawZ >> 8;
            if (pSI->iZ > 8388352) pSI->iZ = -1;
            pSI->wX = static_cast<uint16_t>(x);
            pSI->wY = static_cast<uint16_t>(y);
            pC[0] = 0.0f; pC[1] = 0.0f; pC[2] = 0.0f;
            pC += 3;
            SL2++;
            pSI++;
        }
    }

    // Sort back-to-front (ascending iZ -> farthest first since lower iZ = farther)
    QuickSortInt(totalPixels, sortList.data());

    // Process each pixel in sorted order
    pSI = sortList.data();
    for (int i = 0; i < totalPixels; i++) {
        RenderWork();
        int x = pSI->wX;
        int y = pSI->wY;
        int pixelIdx = x + y * width;

        // Get color of this pixel from the image
        const uint8_t* pixelRGB = imageBuffer + (y * slOffset + x) * 3;
        float sRGB[3];
        RGBToSVec(sRGB, pixelRGB);

        // Compute blur radius from depth
        double w_val_base = (8388352.0 - pSI->iZ) * ZcMul + 1.0;
        float w_val = static_cast<float>((w_val_base * w_val_base - 1.0) * Zcorr);
        float sRadius = (w_val - Zsharp) * Ap / (1.0f + w_val * DEstopFactor);
        bool bDescale = (sRadius < 0.0f);
        sRadius = Min0MaxCS(std::fabs(sRadius) - Rsub, maxR);

        float sMs = sRadius + 1.0f;
        int ims = static_cast<int>(sMs * sMs);
        float sw_pixel = 1.0f / (2.5f * sRadius * sRadius + 1.0f);

        bool bBG = (pSI->iZ == -1);
        int r_int = static_cast<int>(std::lrint(std::fabs(sRadius) + 0.5f));

        // Clamp scatter bounds
        int xa = (r_int > x) ? -x : -r_int;
        int xe = (r_int + x >= width) ? (width - x - 1) : r_int;
        int ya = (r_int > y) ? -y : -r_int;
        int ye = (r_int + y >= height) ? (height - y - 1) : r_int;

        int lineSkip = width - xe + xa - 1;

        // Index into scatter region
        int ii = pixelIdx + ya * width + xa;
        float* pC_local = &ColBuf[ii * 3];
        float* pST = &TranspBuf[ii];

        for (int yy = ya; yy <= ye; yy++) {
            RenderPoll();
            int iys = yy * yy;
            for (int xx = xa; xx <= xe; xx++) {
                RenderWork();
                int distSq = xx * xx + iys;
                if (distSq <= ims) {
                    float w_wt = sMs - std::sqrt(static_cast<float>(distSq));
                    float dw_t = 0.0f;
                    if (bDescale) dw_t = 1.0f - (0.8f * w_wt / sMs) * (0.8f * w_wt / sMs);
                    if (w_wt > 1.0f) w_wt = sw_pixel;
                    else w_wt = w_wt * sw_pixel;

                    if (bDescale && (*pST > dw_t)) {
                        float scale = dw_t / *pST;
                        ScaleSVecV(pC_local, scale);
                        *pST *= scale;
                    }

                    if ((!bBG) && ((w_wt + *pST) > 1.01f)) {
                        float st_scale = (1.01f - w_wt) / *pST;
                        pC_local[0] = pC_local[0] * st_scale + sRGB[0] * w_wt;
                        pC_local[1] = pC_local[1] * st_scale + sRGB[1] * w_wt;
                        pC_local[2] = pC_local[2] * st_scale + sRGB[2] * w_wt;
                        *pST = 1.01f;
                    } else {
                        pC_local[0] += sRGB[0] * w_wt;
                        pC_local[1] += sRGB[1] * w_wt;
                        pC_local[2] += sRGB[2] * w_wt;
                        *pST += w_wt;
                    }
                } else {
                    // No contribution -- just advance pointers below
                }
                pC_local += 3;
                pST++;
            }
            pST += lineSkip;
            pC_local += lineSkip * 3;
        }

        pSI++;
    }

    // Normalize and write back
    float* pST = TranspBuf.data();
    pC = ColBuf.data();
    for (int y = 0; y < height; y++) {
        RenderPoll();
        uint8_t* rowImg = imageBuffer + y * slOffset * 3;
        for (int x = 0; x < width; x++) {
            RenderWork();
            float invW = (*pST < 1e-10f) ? 1e10f : (1.0f / *pST);
            float out[3];
            out[0] = pC[0] * invW;
            out[1] = pC[1] * invW;
            out[2] = pC[2] * invW;
            SVecToRGB(rowImg + x * 3, out);
            pST++;
            pC += 3;
        }
    }
}

// ============================================================================
// CalcDepthOfField -- Main entry point
//
// Determines the number of passes and mode from bCalcDOFtype:
//   Bits 1-2 (shifted right 1, masked &3): pass-count mode
//     0 -> 1 pass
//     1 -> 2 passes
//     2 -> 3 passes; 3 -> 4 passes (Mand.pas: for pass := 0 to mode)
//   Bit 3: mode selector
//     1 -> doDOF
//     0 -> doDOFsort
// ============================================================================

void CalcDepthOfField(
    uint8_t* imageBuffer,
    const TsiLight5* siLight,
    int width, int height,
    int slOffset,
    TMCTparameter* /*mcp*/,
    const TMandHeader10* header)
{
    if (!imageBuffer || !siLight || width <= 0 || height <= 0) return;
    if ((header->bCalcDOFtype & 1) == 0) return;

    // Determine number of passes from bits 1-2
    int passMode = (header->bCalcDOFtype >> 1) & 3;
    int numPasses = passMode + 1;

    bool usePlain = ((header->bCalcDOFtype >> 3) & 1) != 0;

    for (int pass = 0; pass < numPasses; pass++) {
        RenderPoll();
        if (usePlain) {
            doDOF(imageBuffer, siLight, width, height, slOffset, header, pass);
        } else {
            doDOFsort(imageBuffer, siLight, width, height, slOffset, header, pass);
        }
    }
}

} // namespace mb3d
