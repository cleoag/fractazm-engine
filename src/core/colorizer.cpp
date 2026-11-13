#include "core/render_control.h"
#include "core/render_context.h"
#include "core/colorizer.h"
#include "core/math3d.h"
#include "core/header_trafos.h"
#include "core/lightmap.h"
#include "core/parity_trace.h"
#include <cmath>
#include <cstring>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <mutex>

namespace mb3d {

// ============================================================================
// Constants
// ============================================================================

// s011f, s05f, s255f, s001f moved to math3d.h
static constexpr float s1d255 = 1.0f / 255.0f;
static constexpr float s1d63  = 1.0f / 63.0f;
static constexpr float s1d16383 = 1.0f / 16383.0f;
static constexpr float s1em30 = 1e-30f;
static constexpr float s03f   = 0.3f;
static constexpr float Pi1d   = static_cast<float>(1.0 / 3.14159265358979323846);

static float g_diffCosTabSmall[8][128];
static std::once_flag g_diffCosTabsInit;
thread_local int g_paintColorsInterpTraceSuppressionDepth = 0;
thread_local int g_paintSvecTraceScopeDepth = 0;

static inline int PascalRoundToInt(float value) {
    return static_cast<int>(std::lrint(std::min(1e9f, std::max(-1e9f, value))));
}

static void BuildRotMatrixSLocal(float xa, float ya, float za, TSMatrix3& m) {
    TMatrix3 d;
    BuildRotMatrix(xa, ya, za, d);
    for (int r = 0; r < 3; ++r) {
        for (int c = 0; c < 3; ++c) {
            m[r][c] = static_cast<float>(d[r][c]);
        }
        m[r][3] = 0.0f;
    }
}

static void MultiplySMatrix3Local(TSMatrix3& left, const TSMatrix3& right) {
    TSMatrix3 result{};
    for (int r = 0; r < 3; ++r) {
        for (int c = 0; c < 3; ++c) {
            float sum = 0.0f;
            for (int k = 0; k < 3; ++k) {
                sum += left[r][k] * right[k][c];
            }
            result[r][c] = sum;
        }
        result[r][3] = 0.0f;
    }
    std::memcpy(left, result, sizeof(TSMatrix3));
}

static inline float Clamp0DLocal(double value) {
    return static_cast<float>(std::max(0.0, value));
}

void PushPaintColorsInterpTraceSuppression() {
    ++g_paintColorsInterpTraceSuppressionDepth;
}

void PopPaintColorsInterpTraceSuppression() {
    if (g_paintColorsInterpTraceSuppressionDepth > 0) {
        --g_paintColorsInterpTraceSuppressionDepth;
    }
}

void PushPaintSvecTraceScope() {
    ++g_paintSvecTraceScopeDepth;
}

void PopPaintSvecTraceScope() {
    if (g_paintSvecTraceScopeDepth > 0) {
        --g_paintSvecTraceScopeDepth;
    }
}

bool IsPaintSvecTraceScopeEnabled() {
    return g_paintSvecTraceScopeDepth > 0;
}

static void BuildDiffCosTabs() {
    for (int i = 0; i < 128; ++i) {
        // d := 1 - (i - 2) / 60;
        double d = 1.0 - (static_cast<double>(i) - 2.0) / 60.0;
        if (d > 0.15) g_diffCosTabSmall[0][i] = static_cast<float>((d - 0.08) * 1.0869565);
        else if (d <= 0.0) g_diffCosTabSmall[0][i] = 0.0f;
        else g_diffCosTabSmall[0][i] = static_cast<float>(std::pow(d, std::max(1.0, (0.505 - d) * 3.8)));
        g_diffCosTabSmall[1][i] = static_cast<float>(std::max(0.0, d));
        g_diffCosTabSmall[1][i] *= g_diffCosTabSmall[1][i];
        g_diffCosTabSmall[2][i] = static_cast<float>(d * 0.5 + 0.5);
        g_diffCosTabSmall[3][i] = g_diffCosTabSmall[2][i] * g_diffCosTabSmall[2][i];
    }

    for (int k = 0; k < 4; ++k) {
        float tmpTab[128];
        for (int j = 0; j < 128; ++j) {
            tmpTab[j] = std::sqrt(std::max(0.0f, g_diffCosTabSmall[k][j]));
        }
        for (int j = 0; j < 128; ++j) {
            double e = 0.0;
            for (int i = 0; i <= 60; ++i) {
                int l = std::abs(j + i - 30);
                if (l < 128) e += tmpTab[l];
            }
            // DiffCosTabNsmall[k + 4][j] := Sqr(e * 0.011 + Sqr(e * 0.007));
            double e007 = e * 0.007;
            double inner = e * 0.011 + e007 * e007;
            g_diffCosTabSmall[k + 4][j] = static_cast<float>(inner * inner);
        }
    }
}

static void EnsureDiffCosTabs() {
    std::call_once(g_diffCosTabsInit, BuildDiffCosTabs);
}

float FastIntPow(float base, int exp) {
    if (base < 0.0f) return 0.0f;
    float result = base;
    int i = exp >> 1;
    while (i > 0) {
        result *= result;
        i >>= 1;
    }
    return result;
}

static int GetDiffMapNrLocal(const TLightingParas9& lightPars) {
    return static_cast<int>(lightPars.bColorMap |
           (static_cast<int>(lightPars.Lights[1].AdditionalByteEx) << 8));
}

// ============================================================================
// SVec helpers
// ============================================================================

void MakeSVecFromNormalsD(TSVec& result, const TsiLight5* psi) {
    // Math3D.MakeSVecFromNormalsD multiplies by this Double constant in x87.
    constexpr double scale = 3.0518509476e-5;
    result[0] = static_cast<float>(psi->NormalX * scale);
    result[1] = static_cast<float>(psi->NormalY * scale);
    result[2] = static_cast<float>(psi->NormalZ * scale);
    result[3] = 0;
}

float DotOf2VecNormalize(const TSVec& norm, const TSVec& light, const TSVec& view) {
    float d2 = 2.0f * (norm[0]*view[0] + norm[1]*view[1] + norm[2]*view[2]);
    return light[0]*(view[0] - norm[0]*d2) +
           light[1]*(view[1] - norm[1]*d2) +
           light[2]*(view[2] - norm[2]*d2);
}

float GetCosTabVal(int funcType, float dot, float rough) {
    EnsureDiffCosTabs();
    funcType = std::clamp(funcType, 0, 3);
    float t = 62.0f - 60.0f * dot;
    int ip = static_cast<int>(t) - 1;
    if (ip < 0) {
        ip = 0;
        t = 0.0f;
    } else if (ip > 124) {
        ip = 124;
        t = 1.0f;
    } else {
        t -= std::floor(t);
    }

    TSVec w;
    MakeSplineCoeff(t, w);
    // Match Pascal's unpcklps/unpckhps + addps reduction: (0+2)+(1+3).
    float products[4], roughProducts[4];
    for (int i = 0; i < 4; ++i) {
        products[i] = g_diffCosTabSmall[funcType][ip + i] * w[i];
        roughProducts[i] = g_diffCosTabSmall[funcType + 4][ip + i] * w[i];
    }
    float base = (products[0] + products[2]) + (products[1] + products[3]);
    float roughVal = (roughProducts[0] + roughProducts[2]) + (roughProducts[1] + roughProducts[3]);
    return base + rough * (roughVal - base);
}

float GetCosTabValSqr(int funcType, float dot, float rough) {
    EnsureDiffCosTabs();
    funcType = std::clamp(funcType, 0, 3);
    float t = 62.0f - 60.0f * dot;
    int ip = static_cast<int>(t) - 1;
    if (ip < 0) {
        ip = 0;
        t = 0.0f;
    } else if (ip > 124) {
        ip = 124;
        t = 1.0f;
    } else {
        t -= std::floor(t);
    }

    TSVec w;
    MakeSplineCoeff(t, w);
    // Match Pascal's unpcklps/unpckhps + addps reduction: (0+2)+(1+3).
    float products[4], roughProducts[4];
    for (int i = 0; i < 4; ++i) {
        products[i] = g_diffCosTabSmall[funcType][ip + i] * w[i];
        roughProducts[i] = g_diffCosTabSmall[funcType + 4][ip + i] * w[i];
    }
    float base = (products[0] + products[2]) + (products[1] + products[3]);
    float roughVal = (roughProducts[0] + roughProducts[2]) + (roughProducts[1] + roughProducts[3]);
    float result = base + rough * (roughVal - base);
    return result * result;
}

float FastPow(float base, int exp) {
    // Math3D.FastPow is the original rational highlight approximation,
    // not exponentiation. Its x87 expression rounds once to Single.
    const double x = base;
    const double y = exp;
    return static_cast<float>(x / (y - x * y + x));
}

void LoadPackedTSVec(const void* src, TSVec& out) {
    std::memcpy(out, src, sizeof(TSVec));
}

void StorePackedTSVec(void* dst, const TSVec& value) {
    std::memcpy(dst, value, sizeof(TSVec));
}

void AddSVecWeights(TSVec* dest, const TSVec* src, float w) {
    TSVec dstVec{};
    TSVec srcVec{};
    LoadPackedTSVec(dest, dstVec);
    LoadPackedTSVec(src, srcVec);
    dstVec[0] += srcVec[0] * w;
    dstVec[1] += srcVec[1] * w;
    dstVec[2] += srcVec[2] * w;
    StorePackedTSVec(dest, dstVec);
}

void AddSVectors_inplace(TSVec* dest, const TSVec& src) {
    TSVec dstVec{};
    LoadPackedTSVec(dest, dstVec);
    dstVec[0] += src[0];
    dstVec[1] += src[1];
    dstVec[2] += src[2];
    StorePackedTSVec(dest, dstVec);
}

void MultiplySVectors(TSVec& result, const TSVec& a, const TSVec& b) {
    result[0] = a[0] * b[0];
    result[1] = a[1] * b[1];
    result[2] = a[2] * b[2];
    result[3] = 0;
}

void MultiplySVectorsV(TSVec* dest, const TSVec* src) {
    TSVec dstVec{};
    TSVec srcVec{};
    LoadPackedTSVec(dest, dstVec);
    LoadPackedTSVec(src, srcVec);
    dstVec[0] *= srcVec[0];
    dstVec[1] *= srcVec[1];
    dstVec[2] *= srcVec[2];
    StorePackedTSVec(dest, dstVec);
}

void ScaleSVectorV(TSVec* sv, float s) {
    TSVec vec{};
    LoadPackedTSVec(sv, vec);
    vec[0] *= s;
    vec[1] *= s;
    vec[2] *= s;
    StorePackedTSVec(sv, vec);
}

void LinInterpolate2SVecs(TSVec& result, const TSVec& a, const TSVec& b, float t) {
    // Pascal's SSE implementation uses subps, mulps, addps in this order.
    result[0] = b[0] + (a[0] - b[0]) * t;
    result[1] = b[1] + (a[1] - b[1]) * t;
    result[2] = b[2] + (a[2] - b[2]) * t;
    result[3] = b[3] + (a[3] - b[3]) * t;
}

void Add2SVecsWeight(TSVec& result, const TSVec& a, const TSVec& b, float wa, float wb) {
    result[0] = a[0] * wa + b[0] * wb;
    result[1] = a[1] * wa + b[1] * wb;
    result[2] = a[2] * wa + b[2] * wb;
    result[3] = 0;
}

void LinInterpolate2SVecsPacked(TSVec& result, const void* a, const void* b, float t) {
    TSVec av{};
    TSVec bv{};
    LoadPackedTSVec(a, av);
    LoadPackedTSVec(b, bv);
    LinInterpolate2SVecs(result, av, bv, t);
}

void Add2SVecsWeightPacked(TSVec& result, const void* a, const void* b, float wa, float wb) {
    TSVec av{};
    TSVec bv{};
    LoadPackedTSVec(a, av);
    LoadPackedTSVec(b, bv);
    Add2SVecsWeight(result, av, bv, wa, wb);
}

void Add2SVecsWeight2(TSVec& result, const TSVec& a, const TSVec& b, float wb) {
    result[0] = a[0] + b[0] * wb;
    result[1] = a[1] + b[1] * wb;
    result[2] = a[2] + b[2] * wb;
    result[3] = 0;
}

void Add3SVectors(TSVec& result, const TSVec& a, const TSVec& b, const TSVec& c) {
    result[0] = a[0] + b[0] + c[0];
    result[1] = a[1] + b[1] + c[1];
    result[2] = a[2] + b[2] + c[2];
    result[3] = 0;
}

void SubtractSVectors(TSVec& result, const TSVec& a, const TSVec& b) {
    result[0] = a[0] - b[0];
    result[1] = a[1] - b[1];
    result[2] = a[2] - b[2];
    result[3] = 0;
}

void mMinMaxSVec(TSVec& result, float mn, float mx, const TSVec& sv) {
    result[0] = std::min(mx, std::max(mn, sv[0]));
    result[1] = std::min(mx, std::max(mn, sv[1]));
    result[2] = std::min(mx, std::max(mn, sv[2]));
    result[3] = 0;
}

void mSqrtSVec(TSVec& result, const TSVec& sv) {
    result[0] = std::sqrt(std::max(0.0f, sv[0]));
    result[1] = std::sqrt(std::max(0.0f, sv[1]));
    result[2] = std::sqrt(std::max(0.0f, sv[2]));
    result[3] = 0;
}

void SqrSV255(TSVec& result, const TSVec& sv) {
    result[0] = sv[0] * sv[0] * s1d255;
    result[1] = sv[1] * sv[1] * s1d255;
    result[2] = sv[2] * sv[2] * s1d255;
    result[3] = 0;
}

// Max0S moved to math3d.h (inline)

float DotOfSVectors(const TSVec& a, const TSVec& b) {
    // Pascal's x87 helper narrows once, after summing the three products.
    return static_cast<float>(static_cast<double>(a[0])*b[0] +
                              static_cast<double>(a[1])*b[1] +
                              static_cast<double>(a[2])*b[2]);
}

// SqrLengthOfSVec defined in math3d.cpp, use that one via header

void NormaliseSVectorVar(TSVec& sv) {
    float len = std::sqrt(sv[0]*sv[0] + sv[1]*sv[1] + sv[2]*sv[2] + 1e-30f);
    float inv = 1.0f / len;
    sv[0] *= inv;
    sv[1] *= inv;
    sv[2] *= inv;
}

float SqrDistSV(const TSVec& diff, const TSVec& dir) {
    // Match Pascal: |diff × dir|² via cross product (MinDistSV + SqrLengthOfSVec)
    // Pascal: SqrLengthOfSVec(MinDistSV(sv1, SubtractSVectors(@sv1, sv2)))
    // equivalent to |sv1 × sv2|²
    float cx = diff[1]*dir[2] - diff[2]*dir[1];
    float cy = diff[2]*dir[0] - diff[0]*dir[2];
    float cz = diff[0]*dir[1] - diff[1]*dir[0];
    return cx*cx + cy*cy + cz*cz;
}

void RotateSVectorS(TSVec* sv, const TSMatrix3* m) {
    float x = (*sv)[0], y = (*sv)[1], z = (*sv)[2];
    (*sv)[0] = x * (*m)[0][0] + y * (*m)[1][0] + z * (*m)[2][0];
    (*sv)[1] = x * (*m)[0][1] + y * (*m)[1][1] + z * (*m)[2][1];
    (*sv)[2] = x * (*m)[0][2] + y * (*m)[1][2] + z * (*m)[2][2];
}

void RotateSVectorReverseS(TSVec* sv, const TSMatrix3* m) {
    // Pascal's x87 path stores Single only after each complete dot product.
    double x = (*sv)[0], y = (*sv)[1], z = (*sv)[2];
    (*sv)[0] = static_cast<float>(x * (*m)[0][0] + y * (*m)[0][1] + z * (*m)[0][2]);
    (*sv)[1] = static_cast<float>(x * (*m)[1][0] + y * (*m)[1][1] + z * (*m)[1][2]);
    (*sv)[2] = static_cast<float>(x * (*m)[2][0] + y * (*m)[2][1] + z * (*m)[2][2]);
}

float YofSVec_col(const TSVec& sv) {
    return sv[0] * 0.3f + sv[1] * 0.59f + sv[2] * 0.11f;
}

void Clamp01Svar(float& s) {
    if (s < 0) s = 0;
    if (s > 1) s = 1;
}

void AddSVecS(TSVec& result, const TSVec& sv, float s) {
    result[0] = sv[0] + s;
    result[1] = sv[1] + s;
    result[2] = sv[2] + s;
    result[3] = 0;
}

void SVecPow(TSVec& result, const TSVec& sv, float p) {
    result[0] = std::pow(std::max(0.0f, sv[0]), p);
    result[1] = std::pow(std::max(0.0f, sv[1]), p);
    result[2] = std::pow(std::max(0.0f, sv[2]), p);
    result[3] = 0;
}

void AbsSVecVar(TSVec& sv) {
    sv[0] = std::fabs(sv[0]);
    sv[1] = std::fabs(sv[1]);
    sv[2] = std::fabs(sv[2]);
}

void NormaliseMatrixToS(TSMatrix3& result, float n, const TMatrix3* m) {
    TMatrix3 matrix{};
    std::memcpy(&matrix, static_cast<const void*>(m), sizeof(matrix));
    for (int r = 0; r < 3; r++) {
        double len = std::sqrt(matrix[r][0]*matrix[r][0] + matrix[r][1]*matrix[r][1] + matrix[r][2]*matrix[r][2]);
        if (len < 1e-30) len = 1e-30;
        double scale = n / len;
        for (int c = 0; c < 3; c++) {
            result[r][c] = static_cast<float>(matrix[r][c] * scale);
        }
        result[r][3] = 0;
    }
}

void ScaleSVectorD(TSVec& result, const TSMatrix3& row, float scale) {
    // Takes a row of TSMatrix3 (which is float[4]) and scales it
    result[0] = row[0][0] * scale; // This is actually called with a row pointer
    result[1] = row[0][1] * scale;
    result[2] = row[0][2] * scale;
    result[3] = 0;
}

void BuildViewVectorFOV(float CY, float CX, TSVec& result) {
    float sinY = std::sin(CY);
    float cosY = std::cos(CY);
    float sinX = std::sin(CX);
    float cosX = std::cos(CX);
    result[0] = -sinX;
    result[1] = sinY;
    result[2] = cosX * cosY;
    float lenSq = result[0] * result[0] +
                  result[1] * result[1] +
                  result[2] * result[2];
    if (lenSq > s1em30) {
        float invLen = 1.0f / std::sqrt(lenSq);
        result[0] *= invLen;
        result[1] *= invLen;
        result[2] *= invLen;
    }
    result[3] = 0;
}

void BuildViewVectorSphereFOV(float CY, float CX, TSVec& result) {
    float sinY = std::sin(CY);
    float cosY = std::cos(CY);
    float sinX = std::sin(CX);
    float cosX = std::cos(CX);
    result[0] = -sinX * cosY;
    result[1] = sinY;
    result[2] = cosX * cosY;
    result[3] = 0;
}

static void BuildViewVectorSphereFOVDouble(double CY, double CX, TSVec& result) {
    double sinY = std::sin(CY);
    double cosY = std::cos(CY);
    double sinX = std::sin(CX);
    double cosX = std::cos(CX);
    result[0] = static_cast<float>(-sinX * cosY);
    result[1] = static_cast<float>(sinY);
    result[2] = static_cast<float>(cosX * cosY);
    result[3] = 0;
}

// ============================================================================
// Ambient shadow calculation
// ============================================================================

void calcAmbshadow(float& dAmbS, float sAmplitude, const TsiLight5* psi) {
    float shadow;
    if (psi->AmbShadow >= 16383)
        shadow = 1.0f;
    else
        shadow = psi->AmbShadow * s1d16383;

    if (sAmplitude > 1.0f) {
        dAmbS = 1.0f - shadow;
        dAmbS = dAmbS + (sAmplitude - 1.0f) * (dAmbS * dAmbS - dAmbS);
    } else {
        dAmbS = 1.0f - sAmplitude * shadow;
    }
}

// ============================================================================
// ConvertVLight
// ============================================================================

int ConvertVLight(int Win) {
    int v = Win & 0x3FF;
    int shift = (v >> 7) & 7;
    return (v & 0x7F) << shift;
}

// ============================================================================
// CalcPosLightShape
// ============================================================================

void CalcPosLightShape(float& flux, float& transp, int LightNr, TLightVals* PLVals) {
    float tmpR = transp;
    float RtmpR = 1.0f / tmpR;
    bool PosLight = (PLVals->iLightPos[LightNr] & 1) != 0;

    switch (PLVals->iLightPos[LightNr] & 14) {
        case 4: // old light option with more transparency
            transp = flux * RtmpR;
            transp *= transp;
            transp *= transp;
            flux = static_cast<float>(7e-3 * (tmpR - flux) * (RtmpR * RtmpR));
            break;
        case 2: // vislight3
            transp = 1.0f;
            flux = 1.0f - std::sqrt(flux * RtmpR);
            if (PLVals->bCalcPixColSqr)
                flux = static_cast<float>(5e-2 * FastPow(flux, 80) * RtmpR);
            else
                flux = static_cast<float>(5e-2 * FastPow(flux, 50) * RtmpR);
            if (PosLight) flux *= 1.5f;
            break;
        case 6: { // vislight2
            flux = static_cast<float>(flux * 1.05 * RtmpR);
            if (flux < 1.0f) {
                if (PosLight)
                    transp = Max0S(static_cast<float>(flux - 0.95)) * 10.0f;
                else
                    transp = 0.7f;
                flux = static_cast<float>(4e-3 * std::sqrt(1.05 - flux * flux) * RtmpR);
                if (!PosLight) flux = static_cast<float>(flux * 0.7);
            } else if (PosLight) {
                transp = static_cast<float>((flux - 0.95) * 10.0);
                flux = static_cast<float>(17.8e-3 * (1.05 - flux) * RtmpR);
            } else {
                transp = static_cast<float>((flux - 1.0f) * 6.0f + 0.7);
                flux = static_cast<float>(12.5e-3 * (1.05 - flux) * RtmpR);
            }
            break;
        }
        default: {
            transp = 1.0f;
            float tmpR2 = PLVals->bCalcPixColSqr ? 40.0f : 25.0f;
            float ratio = flux * RtmpR;
            double denom = (ratio + 0.0004) * tmpR2;
            flux = static_cast<float>((1.0f - ratio) * 0.1 / (denom * denom) * RtmpR);
            break;
        }
    }
}

// ============================================================================
// CalcColorsInside
// ============================================================================

void CalcColorsInside(TSVec* iDif, TSVec* iSpe, const TsiLight5* psi, TLightVals* PLVals) {
    int ir = PascalRoundToInt(((psi->SIgradient - PLVals->sCiStart) * PLVals->sCimul + (*iDif)[0]) * 16384.0f);

    if (PLVals->bColCycling) {
        ir = ir & 32767;
    } else {
        if (ir < 0) {
            std::memcpy(iDif, PLVals->PLValigned->ColInt[0], sizeof(TSVec));
            goto set_spe;
        } else if (ir > PLVals->IColPos[3]) {
            std::memcpy(iDif, PLVals->PLValigned->ColInt[3], sizeof(TSVec));
            goto set_spe;
        }
    }

    {
        int iL2 = 1;
        while (iL2 < 4 && PLVals->IColPos[iL2] < ir) iL2++;
        if (PLVals->bNoColIpol) {
            std::memcpy(iDif, PLVals->PLValigned->ColInt[iL2 - 1], sizeof(TSVec));
        } else {
            int iL1 = iL2 - 1;
            iL2 = iL2 & 3;
            float stmp = (ir - PLVals->IColPos[iL1]) * PLVals->sICDiv[iL1];
            TSVec colInt2{};
            TSVec colInt1{};
            LoadPackedTSVec(PLVals->PLValigned->ColInt[iL2], colInt2);
            LoadPackedTSVec(PLVals->PLValigned->ColInt[iL1], colInt1);
            LinInterpolate2SVecs(*iDif, colInt2, colInt1, stmp);
        }
    }

set_spe:
    (*iSpe)[0] = (*iDif)[3];
    (*iSpe)[1] = (*iDif)[3];
    (*iSpe)[2] = (*iDif)[3];
    (*iSpe)[3] = (*iDif)[3];
    (*iDif)[3] = 0;
}

// ============================================================================
// CalcColors
// ============================================================================

void CalcColors(TSVec* iDif, TSVec* iSpe, const TsiLight5* psi, TLightVals* PLVals) {
    int ir;
    if ((PLVals->iColOnOT & 1) == 0)
        ir = psi->SIgradient;
    else
        ir = psi->OTrap & 0x7FFF;

    float raw = ((ir - PLVals->sCStart) * PLVals->sCmul + (*iDif)[0]) * 16384.0f;
    raw = MinMaxCS(-1.0e9f, raw, 1.0e9f);
    ir = PascalRoundToInt(raw);

    int iL2 = 5;
    if (PLVals->bColCycling) {
        ir = ir & 32767;
    } else {
        if (ir < 0) {
            std::memcpy(iSpe, PLVals->PLValigned->ColSpe[0], sizeof(TSVec));
            std::memcpy(iDif, PLVals->PLValigned->ColDif[0], sizeof(TSVec));
            return;
        } else if (ir >= PLVals->ColPos[9]) {
            std::memcpy(iSpe, PLVals->PLValigned->ColSpe[9], sizeof(TSVec));
            std::memcpy(iDif, PLVals->PLValigned->ColDif[9], sizeof(TSVec));
            return;
        }
    }

    if (PLVals->ColPos[iL2] < ir) {
        do { iL2++; } while (iL2 < 10 && PLVals->ColPos[iL2] < ir);
    } else {
        while (iL2 > 1 && PLVals->ColPos[iL2 - 1] >= ir) iL2--;
    }

    if (PLVals->bNoColIpol) {
        if ((g_paintColorsInterpTraceSuppressionDepth == 0) &&
            HasParityTraceContext() && ShouldLogParityStage("paint_colors_interp")) {
            std::ostringstream payload;
            payload.setf(std::ios::fixed, std::ios::floatfield);
            payload << std::setprecision(12)
                    << " mode=direct"
                    << " iColOnOT=" << PLVals->iColOnOT
                    << " sourceVal=" << ((PLVals->iColOnOT & 1) == 0 ? psi->SIgradient : (psi->OTrap & 0x7FFF))
                    << " sCStart=" << PLVals->sCStart
                    << " sCmul=" << PLVals->sCmul
                    << " iDif0=" << (*iDif)[0]
                    << " ir=" << ir
                    << " iL1=" << (iL2 - 1)
                    << " iL2=" << (iL2 - 1)
                    << " stmp=0.000000000000"
                    << " Dif1X=" << PLVals->PLValigned->ColDif[iL2 - 1][0]
                    << " Dif1Y=" << PLVals->PLValigned->ColDif[iL2 - 1][1]
                    << " Dif1Z=" << PLVals->PLValigned->ColDif[iL2 - 1][2]
                    << " Dif1W=" << PLVals->PLValigned->ColDif[iL2 - 1][3]
                    << " Dif2X=" << PLVals->PLValigned->ColDif[iL2 - 1][0]
                    << " Dif2Y=" << PLVals->PLValigned->ColDif[iL2 - 1][1]
                    << " Dif2Z=" << PLVals->PLValigned->ColDif[iL2 - 1][2]
                    << " Dif2W=" << PLVals->PLValigned->ColDif[iL2 - 1][3]
                    << " Spe1X=" << PLVals->PLValigned->ColSpe[iL2 - 1][0]
                    << " Spe1Y=" << PLVals->PLValigned->ColSpe[iL2 - 1][1]
                    << " Spe1Z=" << PLVals->PLValigned->ColSpe[iL2 - 1][2]
                    << " Spe1W=" << PLVals->PLValigned->ColSpe[iL2 - 1][3]
                    << " Spe2X=" << PLVals->PLValigned->ColSpe[iL2 - 1][0]
                    << " Spe2Y=" << PLVals->PLValigned->ColSpe[iL2 - 1][1]
                    << " Spe2Z=" << PLVals->PLValigned->ColSpe[iL2 - 1][2]
                    << " Spe2W=" << PLVals->PLValigned->ColSpe[iL2 - 1][3];
            AppendParityTrace("paint_colors_interp", payload.str());
        }
        std::memcpy(iSpe, PLVals->PLValigned->ColSpe[iL2 - 1], sizeof(TSVec));
        std::memcpy(iDif, PLVals->PLValigned->ColDif[iL2 - 1], sizeof(TSVec));
    } else {
        int iL1 = iL2 - 1;
        if (iL2 > 9) iL2 = 0;
        float stmp = (ir - PLVals->ColPos[iL1]) * PLVals->sCDiv[iL1];
        if ((g_paintColorsInterpTraceSuppressionDepth == 0) &&
            HasParityTraceContext() && ShouldLogParityStage("paint_colors_interp")) {
            std::ostringstream payload;
            payload.setf(std::ios::fixed, std::ios::floatfield);
            payload << std::setprecision(12)
                    << " mode=interp"
                    << " iColOnOT=" << PLVals->iColOnOT
                    << " sourceVal=" << ((PLVals->iColOnOT & 1) == 0 ? psi->SIgradient : (psi->OTrap & 0x7FFF))
                    << " sCStart=" << PLVals->sCStart
                    << " sCmul=" << PLVals->sCmul
                    << " iDif0=" << (*iDif)[0]
                    << " ir=" << ir
                    << " iL1=" << iL1
                    << " iL2=" << iL2
                    << " stmp=" << stmp
                    << " Dif1X=" << PLVals->PLValigned->ColDif[iL1][0]
                    << " Dif1Y=" << PLVals->PLValigned->ColDif[iL1][1]
                    << " Dif1Z=" << PLVals->PLValigned->ColDif[iL1][2]
                    << " Dif1W=" << PLVals->PLValigned->ColDif[iL1][3]
                    << " Dif2X=" << PLVals->PLValigned->ColDif[iL2][0]
                    << " Dif2Y=" << PLVals->PLValigned->ColDif[iL2][1]
                    << " Dif2Z=" << PLVals->PLValigned->ColDif[iL2][2]
                    << " Dif2W=" << PLVals->PLValigned->ColDif[iL2][3]
                    << " Spe1X=" << PLVals->PLValigned->ColSpe[iL1][0]
                    << " Spe1Y=" << PLVals->PLValigned->ColSpe[iL1][1]
                    << " Spe1Z=" << PLVals->PLValigned->ColSpe[iL1][2]
                    << " Spe1W=" << PLVals->PLValigned->ColSpe[iL1][3]
                    << " Spe2X=" << PLVals->PLValigned->ColSpe[iL2][0]
                    << " Spe2Y=" << PLVals->PLValigned->ColSpe[iL2][1]
                    << " Spe2Z=" << PLVals->PLValigned->ColSpe[iL2][2]
                    << " Spe2W=" << PLVals->PLValigned->ColSpe[iL2][3];
            AppendParityTrace("paint_colors_interp", payload.str());
        }
        TSVec colSpe2{};
        TSVec colSpe1{};
        TSVec colDif2{};
        TSVec colDif1{};
        LoadPackedTSVec(PLVals->PLValigned->ColSpe[iL2], colSpe2);
        LoadPackedTSVec(PLVals->PLValigned->ColSpe[iL1], colSpe1);
        LoadPackedTSVec(PLVals->PLValigned->ColDif[iL2], colDif2);
        LoadPackedTSVec(PLVals->PLValigned->ColDif[iL1], colDif1);
        LinInterpolate2SVecs(*iSpe, colSpe2, colSpe1, stmp);
        LinInterpolate2SVecs(*iDif, colDif2, colDif1, stmp);
    }
}

// ============================================================================
// CalcTotalLight1 / CalcTotalLight2
// ============================================================================

void CalcTotalLight1(TLightLSDAIs* LiLSDAI, TLightVals* PLVals, float* pAmbsh) {
    float stmp = PLVals->bCalcPixColSqr ? s011f : s03f;

    // LiLSDAI[3] = LiLSDAI[4] + LiLSDAI[0] * LiLSDAI[3] * (pAmbsh * stmp)
    TSVec tmp1;
    MultiplySVectors(tmp1, (*LiLSDAI)[0], (*LiLSDAI)[3]);
    Add2SVecsWeight2((*LiLSDAI)[3], (*LiLSDAI)[4], tmp1, (*pAmbsh) * stmp);

    stmp = Max0S(1.0f - (*pAmbsh));
    float stmp2 = stmp * stmp;
    stmp2 = stmp2 * stmp2; // amount of 2nd reflection

    // LiLSDAI[4] = LiLSDAI[1] + LiLSDAI[3] * LiLSDAI[2] * (LiLSDAI[2] * stmp2 * sIndLightReflect + (stmp - stmp2))
    TSVec scaled;
    ScaleSVec(scaled, (*LiLSDAI)[2], stmp2 * PLVals->sIndLightReflect);
    AddSVecS(scaled, scaled, stmp - stmp2);

    TSVec mul1;
    MultiplySVectors(mul1, (*LiLSDAI)[2], scaled);
    TSVec mul2;
    MultiplySVectors(mul2, (*LiLSDAI)[3], mul1);
    AddSVec((*LiLSDAI)[4], (*LiLSDAI)[1], mul2);
}

void CalcTotalLight2(TLightLSDAIs* LiLSDAI, TLightVals* PLVals, float* pAmbsh) {
    float stmp = Max0S(1.0f - (*pAmbsh));
    stmp = stmp * stmp * PLVals->sIndLightReflect;

    float stmp2 = YofSVec_col((*LiLSDAI)[1]) + 4.0f;

    // LiLSDAI[4] = LiLSDAI[1] + LiLSDAI[1]*LiLSDAI[2] * stmp
    TSVec mul1;
    MultiplySVectors(mul1, (*LiLSDAI)[1], (*LiLSDAI)[2]);
    Add2SVecsWeight2((*LiLSDAI)[4], (*LiLSDAI)[1], mul1, stmp);

    float newY = Max0S(YofSVec_col((*LiLSDAI)[4])) + 3.0f;
    float scale = (stmp2 + (stmp2 + 128.0f) * (stmp + s011f) * 0.04f) / newY;
    ScaleSVectorV(&(*LiLSDAI)[4], scale);
}

// ============================================================================
// PreCalcDepthCol
// ============================================================================

void PreCalcDepthCol(int Dfunc, TPaintLightVals* PLV, TLValigned* PLValigned) {
    float s;
    if (Dfunc == 1)
        s = PLV->yPos * PLV->yPos;
    else if (Dfunc == 0)
        s = PLV->yPos;
    else
        s = std::sqrt(std::max(0.0f, PLV->yPos));
    TSVec preDepthCol{};
    TSVec depthCol2{};
    TSVec depthCol{};
    std::memcpy(depthCol2, PLValigned->sDepthCol2, sizeof(TSVec));
    std::memcpy(depthCol, PLValigned->sDepthCol, sizeof(TSVec));
    LinInterpolate2SVecs(preDepthCol, depthCol2, depthCol, s);
    std::memcpy(PLV->PreDepthCol, preDepthCol, sizeof(TSVec));
}

// ============================================================================
// CalcPixelColor2 -- Standard per-pixel coloring
// Full port from PaintThread.pas CalcPixelColor2
// ============================================================================

void CalcPixelColor2(uint32_t* SL, const TsiLight5* PsiLight,
                     TLightVals* PLVals, TPaintLightVals* PLV) {
    RenderWork();
    bool bSubAmbSh, noHS, softHS;
    int ir, iL1;
    float dRough, dTmp2, dAmbSh, dFog, dTmp = 0.0f, dTmp3 = 0.0f;
    TLightLSDAIs LiLSDAI;
    TSVec sLiSpe, DepC;
    auto tracePaintState = [&](const char* stage, float a0, float a1, float a2,
                               float b0, float b1, float b2,
                               float c0, float c1, float c2,
                               float d0, float d1, float d2,
                               float amb, float fog, float tmp, float tmp3) {
        if (!HasParityTraceContext() || !ShouldLogParityStage(stage)) return;
        std::ostringstream payload;
        payload.setf(std::ios::fixed, std::ios::floatfield);
        payload << std::setprecision(12)
                << " amb=" << amb
                << " fog=" << fog
                << " tmp=" << tmp
                << " tmp3=" << tmp3
                << " V0X=" << a0
                << " V0Y=" << a1
                << " V0Z=" << a2
                << " V1X=" << b0
                << " V1Y=" << b1
                << " V1Z=" << b2
                << " V3X=" << c0
                << " V3Y=" << c1
                << " V3Z=" << c2
                << " V4X=" << d0
                << " V4Y=" << d1
                << " V4Z=" << d2;
        AppendParityTrace(stage, payload.str());
    };
    auto tracePaintColors = [&](const char* stage, const TSVec& dif, const TSVec& spe,
                                bool interior, float zColorBase) {
        if (!HasParityTraceContext() || !ShouldLogParityStage(stage)) return;
        std::ostringstream payload;
        payload.setf(std::ios::fixed, std::ios::floatfield);
        payload << std::setprecision(12)
                << " interior=" << (interior ? 1 : 0)
                << " zColorBase=" << zColorBase
                << " DifX=" << dif[0]
                << " DifY=" << dif[1]
                << " DifZ=" << dif[2]
                << " SpeX=" << spe[0]
                << " SpeY=" << spe[1]
                << " SpeZ=" << spe[2];
        AppendParityTrace(stage, payload.str());
    };
    auto tracePaintAmbshadow = [&](const char* stage) {
        if (!HasParityTraceContext() || !ShouldLogParityStage(stage)) return;
        std::ostringstream payload;
        payload.setf(std::ios::fixed, std::ios::floatfield);
        payload << std::setprecision(12)
                << " amb=" << dAmbSh
                << " fog=" << dFog
                << " rough=" << dRough
                << " Zpos=" << PsiLight->Zpos
                << " Shadow=" << PsiLight->Shadow
                << " AmbShadow=" << PsiLight->AmbShadow
                << " SIgradient=" << PsiLight->SIgradient
                << " OTrap=" << PsiLight->OTrap
                << " sAmbShad=" << PLVals->sAmbShad
                << " sDiffuseShadowing=" << PLVals->sDiffuseShadowing;
        AppendParityTrace(stage, payload.str());
    };

    // Depth color
    if (PLVals->bAmbRelObj) {
        float arcVal = PLV->AbsViewVec[1];
        if (arcVal > 1.0f) arcVal = 1.0f;
        if (arcVal < -1.0f) arcVal = -1.0f;
        LinInterpolate2SVecsPacked(DepC, PLVals->PLValigned->sDepthCol2, PLVals->PLValigned->sDepthCol,
                                   std::asin(arcVal) * Pi1d + s05f);
    } else {
        std::memcpy(DepC, PLV->PreDepthCol, sizeof(TSVec));
    }

    if (PsiLight->Zpos < 32768) { // Object, not background
        dRough = (PsiLight->RoughZposFine & 0xFF) * PLVals->sRoughnessFactor;
        calcAmbshadow(dAmbSh, PLVals->sAmbShad, PsiLight);
        dFog = PLVals->sDiffuseShadowing * (dAmbSh - 1.0f) + 1.0f;
        tracePaintAmbshadow("paint_after_ambshadow");

        MakeSVecFromNormalsD(LiLSDAI[0], PsiLight);
        ClearSVec(LiLSDAI[1]);
        ClearSVec(sLiSpe);
        ClearSVec(LiLSDAI[3]);

        ir = 0;
        while (ir < 6) {
            if (PLVals->iLightOption[ir] == 0) { // light on
                bSubAmbSh = (PLVals->iHScalced[ir] ^ PLVals->iHSenabled[ir]) != 0;
                softHS = PLVals->iHSmask[ir] == -1;
                noHS = softHS || ((PsiLight->Shadow & PLVals->iHSmask[ir]) == 0) || (PLVals->iHScalced[ir] == 0);

                std::memcpy(LiLSDAI[5], PLVals->PLValigned->LN[ir], sizeof(TSVec));

                if ((PLVals->iLightPos[ir] & 1) != 0) { // positional light
                    SubtractSVectors(LiLSDAI[5], LiLSDAI[5], PLV->ObjPos);
                    dTmp2 = SqrLengthOfSVec(LiLSDAI[5]);
                    if (dTmp2 > PLVals->sLmaxL[ir]) { ir++; continue; }
                    float invLen = 1.0f / std::sqrt(dTmp2);
                    ScaleSVectorV(&LiLSDAI[5], invLen);
                    RotateSVectorReverseS(&LiLSDAI[5], PLV->PSmatrix);
                    dTmp2 = 1.0f / (dTmp2 + s1em30);
                } else {
                    dTmp2 = 1.0f;
                }

                float dotNL = DotOfSVectors(LiLSDAI[0], LiLSDAI[5]);
                dTmp = GetCosTabVal(PLVals->iLightFuncDiff[ir], dotNL, dRough) * dTmp2;
                if (ir == 0 && HasParityTraceContext() && ShouldLogParityStage("paint_light0")) {
                    std::ostringstream payload;
                    payload.setf(std::ios::fixed, std::ios::floatfield);
                    payload << std::setprecision(12)
                            << " dot=" << dotNL
                            << " dTmp=" << dTmp
                            << " dTmp2=" << dTmp2
                            << " rough=" << dRough
                            << " noHS=" << (noHS ? 1 : 0)
                            << " softHS=" << (softHS ? 1 : 0)
                            << " bSubAmbSh=" << (bSubAmbSh ? 1 : 0);
                    AppendParityTrace("paint_light0", payload.str());
                }
                AddSVecWeights(&LiLSDAI[3], &PLVals->PLValigned->sLCols[ir], dTmp);

                if (noHS) {
                    if (bSubAmbSh) dTmp *= dAmbSh; else dTmp *= dFog;
                    if (softHS) dTmp *= (PsiLight->Shadow >> 10) * s1d63;
                    AddSVecWeights(&LiLSDAI[1], &PLVals->PLValigned->sLCols[ir], dTmp);
                }

                // Specular calc
                dTmp = DotOf2VecNormalize(LiLSDAI[0], LiLSDAI[5], PLV->ViewVec);
                if (dTmp > 0) {
                    dTmp2 = (dTmp2 + std::min(1.0f, dRough * 2.0f) * (1.0f / PLVals->iLightPowFunc[ir] - dTmp2)) * PLVals->sSpec;
                    if (dTmp2 > 0) {
                        AddSVecWeights(&LiLSDAI[3], &PLVals->PLValigned->sLCols[ir], dTmp2 / PLVals->iLightPowFunc[ir]);
                        if (noHS) {
                            dTmp2 *= FastIntPow(dTmp, PLVals->iLightPowFunc[ir]);
                            if (bSubAmbSh) dTmp2 *= dAmbSh; else dTmp2 *= dFog;
                            if (softHS) dTmp2 *= (PsiLight->Shadow >> 10) * s1d63;
                            AddSVecWeights(&sLiSpe, &PLVals->PLValigned->sLCols[ir], dTmp2);
                        }
                    }
                }
            } else if (PLVals->iLightOption[ir] == 2 && PLVals->LLightMaps[ir] != nullptr) {
                TSVec lmPix, lmScaled;
                GetLightMapPixelSphere(lmPix, LiLSDAI[0], &PLVals->LLightMaps[ir]->PicRotMatrix,
                                       *PLVals->LLightMaps[ir], PLVals->bCalcPixColSqr);
                ScaleSVec(lmScaled, lmPix, 255.0f);
                LinInterpolate2SVecs(LiLSDAI[4], PLVals->LLightMaps[ir]->LMavrgCol, lmScaled, dRough);
                AddSVecWeights(&LiLSDAI[3], &LiLSDAI[4], 1.0f);
                AddSVecWeights(&LiLSDAI[1], &LiLSDAI[4], dAmbSh);
            }
            ir++;
        }

        bSubAmbSh = PsiLight->SIgradient > 32767; // interior

        LiLSDAI[2][0] = PLVals->sColZmul * PLV->zPos;
        float zColorBase = LiLSDAI[2][0];
        if (bSubAmbSh)
            CalcColorsInside(&LiLSDAI[2], &LiLSDAI[5], PsiLight, PLVals);
        else
            CalcColors(&LiLSDAI[2], &LiLSDAI[5], PsiLight, PLVals);
        tracePaintColors("paint_after_colors", LiLSDAI[2], LiLSDAI[5], bSubAmbSh, zColorBase);

        if (PLVals->iColOnOT > 1 && PLVals->DiffColLightMap != nullptr) {
            if (PLVals->bYCcomb) {
                std::memcpy(LiLSDAI[4], LiLSDAI[2], sizeof(TSVec));
            }
            if (PLVals->iColOnOT > 3) {
                if (PLVals->iColOnOT > 5) {
                    GetDiffMapWrap3D(LiLSDAI[2], PLVals, &LiLSDAI[0], PLV);
                } else {
                    GetLightMapPixelSphere(LiLSDAI[2], LiLSDAI[0], &PLVals->DiffColLightMap->PicRotMatrix,
                                           *PLVals->DiffColLightMap, 0);
                }
                TSVec avgScaled;
                ScaleSVec(avgScaled, PLVals->DiffColLightMap->LMavrgCol, dRough * s1d255);
                Add2SVecsWeight(LiLSDAI[2], LiLSDAI[2], avgScaled, 1.0f - dRough, 1.0f);
            } else {
                dTmp = (PsiLight->OTrap & 0x7FFF) * 3.05186851e-5f - 0.5f;
                dTmp2 = PsiLight->SIgradient * 3.05186851e-5f - 0.5f;
                GetLightMapPixel(
                    LiLSDAI[2],
                    std::fmod((PLVals->DCLMapRotCos * dTmp + PLVals->DCLMapRotSin * dTmp2 +
                               PLVals->DCLMapOffX) * PLVals->lvMapScale + 1.0f, 1.0f),
                    std::fmod((PLVals->DCLMapRotCos * dTmp2 - PLVals->DCLMapRotSin * dTmp +
                               PLVals->DCLMapOffY) * PLVals->lvMapScale + 1.0f, 1.0f),
                    PLVals->DiffColLightMap, 0, 1);
            }
            if (PLVals->bYCcomb) {
                float denom = s001f + YofSVec_col(LiLSDAI[4]);
                ScaleSVec(LiLSDAI[2], LiLSDAI[4], YofSVec_col(LiLSDAI[2]) / denom);
            }
        }

        if (PLVals->bUseSmallBGpicForAmb) {
            RotateSVectorS(&LiLSDAI[0], PLV->PSmatrix);
            if (PLVals->BGsmallLM != nullptr) {
                GetLightMapPixelSphere(LiLSDAI[4], LiLSDAI[0], &PLVals->BGsmallLM->PicRotMatrix,
                                       *PLVals->BGsmallLM, 0);
                ScaleSVectorV(&LiLSDAI[4], 255.0f);
                dTmp = Max0S(1.0f - Max0S(1.0f - 28000.0f * PLVals->sDepth));
                if (PLVals->bFarFog) dTmp = dTmp * dTmp;
                if (PLVals->bAddBGlight) {
                    AddSVecWeights(&LiLSDAI[4], &DepC, dTmp);
                } else {
                    LinInterpolate2SVecs(LiLSDAI[4], LiLSDAI[4], DepC, 1.0f - dTmp);
                }
                ScaleSVectorV(&LiLSDAI[4], dAmbSh);
            } else {
                if (PLVals->bAmbRelObj) {
                    LiLSDAI[0][1] = LiLSDAI[0][0] * (*PLV->PSmatrix)[0][1] +
                                    LiLSDAI[0][1] * (*PLV->PSmatrix)[1][1] +
                                    LiLSDAI[0][2] * (*PLV->PSmatrix)[2][1];
                }
                dTmp2 = (LiLSDAI[0][1] * s05f + s05f) * dAmbSh;
                dTmp = dAmbSh - dTmp2;
                Add2SVecsWeightPacked(LiLSDAI[4], PLVals->PLValigned->sAmbCol, PLVals->PLValigned->sAmbCol2, dTmp, dTmp2);
            }
        } else {
            if (PLVals->bAmbRelObj) {
                LiLSDAI[0][1] = LiLSDAI[0][0] * (*PLV->PSmatrix)[0][1] +
                                LiLSDAI[0][1] * (*PLV->PSmatrix)[1][1] +
                                LiLSDAI[0][2] * (*PLV->PSmatrix)[2][1];
            }
            dTmp2 = (LiLSDAI[0][1] * s05f + s05f) * dAmbSh;
            dTmp = dAmbSh - dTmp2;
            Add2SVecsWeightPacked(LiLSDAI[4], PLVals->PLValigned->sAmbCol, PLVals->PLValigned->sAmbCol2, dTmp, dTmp2);
        }

        MultiplySVectorsV(&LiLSDAI[4], &LiLSDAI[2]); // amb light * diffuse color

        // Diff object color scaled
        ScaleSVec(LiLSDAI[0], LiLSDAI[2], PLVals->sDiff);

        // Total light = ambient + diffuse*lightcol + specular
        TSVec diffLight, specLight;
        MultiplySVectors(diffLight, LiLSDAI[0], LiLSDAI[1]);
        MultiplySVectors(specLight, LiLSDAI[5], sLiSpe);
        Add3SVectors(LiLSDAI[1], LiLSDAI[4], diffLight, specLight);
        tracePaintState("paint_before_total",
                        LiLSDAI[0][0], LiLSDAI[0][1], LiLSDAI[0][2],
                        LiLSDAI[1][0], LiLSDAI[1][1], LiLSDAI[1][2],
                        LiLSDAI[3][0], LiLSDAI[3][1], LiLSDAI[3][2],
                        LiLSDAI[4][0], LiLSDAI[4][1], LiLSDAI[4][2],
                        dAmbSh, dFog, dTmp, dTmp3);

        if (PLVals->iExModes == 0)
            CalcTotalLight1(&LiLSDAI, PLVals, &dAmbSh);
        else
            CalcTotalLight2(&LiLSDAI, PLVals, &dAmbSh);
        tracePaintState("paint_after_object",
                        LiLSDAI[0][0], LiLSDAI[0][1], LiLSDAI[0][2],
                        LiLSDAI[1][0], LiLSDAI[1][1], LiLSDAI[1][2],
                        LiLSDAI[3][0], LiLSDAI[3][1], LiLSDAI[3][2],
                        LiLSDAI[4][0], LiLSDAI[4][1], LiLSDAI[4][2],
                        dAmbSh, dFog, dTmp, dTmp3);

        dTmp = Max0S((static_cast<int>(PsiLight->Zpos) - 28000) * PLVals->sDepth + 1.0f);
    } else {
        // Background
        if (PLVals->bBackBMP && PLVals->BGLightMap != nullptr) {
            if (PLVals->bDirectImageCoord) {
                GetLightMapPixel(LiLSDAI[4], PLV->xPos, PLV->yPos, PLVals->BGLightMap, 0, 0);
                ScaleSVectorV(&LiLSDAI[4], 255.0f);
            } else {
                GetLightMapPixelSphere(LiLSDAI[4], PLV->AbsViewVec, &PLVals->BGLightMap->PicRotMatrix,
                                       *PLVals->BGLightMap, 0);
                ScaleSVectorV(&LiLSDAI[4], 255.0f);
            }
        } else {
            std::memcpy(LiLSDAI[4], DepC, sizeof(TSVec));
        }
        dTmp = Max0S(1.0f - 28000.0f * PLVals->sDepth);
    }

    // Pascal: only apply depth-fog remap when bFarFog is enabled
    if (PLVals->bFarFog && dTmp < 1.0f) {
        float t = 1.0f - dTmp;
        dTmp = 1.0f - t * t;
    }

    if ((PsiLight->Zpos < 32768) || (!PLVals->bBackBMP) || (!PLVals->bAddBGlight)) {
        ScaleSVectorV(&LiLSDAI[4], dTmp);
    }

    // Dynamic fog
    if (PLVals->bVolLight)
        ir = ConvertVLight(PsiLight->Shadow);
    else
        ir = PsiLight->Shadow & 0x3FF;

    dFog = (ir - PLVals->sShad - PLVals->sShadZmul * PLV->zPos) * PLVals->sShadGr;
    if ((PLVals->bDFogOptions & 2) != 0) dFog = Max0S(dFog);
    dTmp3 = std::min(1.0f, ir * PLVals->sDynFogMul) * dFog;

    // Add depth fog color
    Add2SVecsWeight2(LiLSDAI[4], LiLSDAI[4], DepC, Max0S(1.0f - dTmp));

    if ((PLVals->bDFogOptions & 1) != 0) {
        Clamp01Svar(dFog);
        Clamp01Svar(dTmp3);
        ScaleSVectorV(&LiLSDAI[4], 1.0f - dFog);
    }

    // Add dynamic fog colors
    TSVec fogMix{};
    TSVec dynFogCol{};
    TSVec dynFogCol2{};
    LoadPackedTSVec(PLVals->PLValigned->sDynFogCol, dynFogCol);
    LoadPackedTSVec(PLVals->PLValigned->sDynFogCol2, dynFogCol2);
    Add2SVecsWeight(fogMix, dynFogCol, dynFogCol2, dFog - dTmp3, dTmp3);
    AddSVectors_inplace(&LiLSDAI[4], fogMix);
    tracePaintState("paint_after_fog",
                    LiLSDAI[0][0], LiLSDAI[0][1], LiLSDAI[0][2],
                    LiLSDAI[1][0], LiLSDAI[1][1], LiLSDAI[1][2],
                    LiLSDAI[3][0], LiLSDAI[3][1], LiLSDAI[3][2],
                    LiLSDAI[4][0], LiLSDAI[4][1], LiLSDAI[4][2],
                    dAmbSh, dFog, dTmp, dTmp3);

    // Visible light sources
    iL1 = PsiLight->Zpos;
    if (iL1 < 32768) iL1 = 32768 - iL1;
    dRough = 1.0f / iL1;
    for (ir = 0; ir < 6; ir++) {
        if ((PLVals->iLightPos[ir] & 14) != 0) {
            noHS = (PLVals->iLightPos[ir] & 1) != 0;
            if (noHS) { // poslight
                TSVec camDiff;
                SubtractSVectors(camDiff, PLV->CamPos, PLVals->PLValigned->LN[ir]);
                dTmp3 = SqrDistSV(camDiff, PLV->AbsViewVec);
            } else { // global light
                if (PsiLight->Zpos < 32768) continue;
                bSubAmbSh = false;
                dTmp3 = Max0S(1.0f - DotOfSVectors(PLVals->PLValigned->LN[ir], PLV->ViewVec));
            }

            LiLSDAI[2][0] = PLVals->sLmaxL[ir] * 1e-8f;
            if (dTmp3 < LiLSDAI[2][0]) {
                if (noHS) { // poslight
                    TSVec lightToCam;
                    SubtractSVectors(lightToCam, PLVals->PLValigned->LN[ir], PLV->CamPos);
                    if (DotOfSVectors(lightToCam, PLV->AbsViewVec) < 0) continue;
                    if (PsiLight->Zpos > 32767) {
                        bSubAmbSh = false;
                    } else {
                        LiLSDAI[2][1] = -std::sqrt(LiLSDAI[2][0] - dTmp3);
                        if (std::fabs(PLV->AbsViewVec[0]) > s05f)
                            bSubAmbSh = LiLSDAI[2][1] > (PLV->ObjPos[0] - PLVals->PLValigned->LN[ir][0]) / PLV->AbsViewVec[0];
                        else if (std::fabs(PLV->AbsViewVec[1]) > s05f)
                            bSubAmbSh = LiLSDAI[2][1] > (PLV->ObjPos[1] - PLVals->PLValigned->LN[ir][1]) / PLV->AbsViewVec[1];
                        else
                            bSubAmbSh = LiLSDAI[2][1] > (PLV->ObjPos[2] - PLVals->PLValigned->LN[ir][2]) / PLV->AbsViewVec[2];
                    }
                }
                if (bSubAmbSh) continue;

                CalcPosLightShape(dTmp3, LiLSDAI[2][0], ir, PLVals);

                // Depth fog at light position
                LiLSDAI[2][2] = 1.0f - Max0S(1.0f + (PLVals->sPosLP[ir] - 28000.0f) * PLVals->sDepth);
                if (PLVals->bFarFog && LiLSDAI[2][2] > 0)
                    LiLSDAI[2][2] = LiLSDAI[2][2] * LiLSDAI[2][2];
                dTmp3 *= (1.0f - LiLSDAI[2][2] * 0.9f);

                int posOpt = PLVals->iLightPos[ir] & 14;
                if (posOpt == 2 || posOpt == 8) {
                    // vislight3: don't decrease light behind
                    Add2SVecsWeight2(LiLSDAI[4], LiLSDAI[4], PLVals->PLValigned->sLCols[ir], dTmp3);
                } else {
                    LiLSDAI[2][2] = Max0S(LiLSDAI[2][2]);
                    int iL1v;
                    if (PLVals->bVolLight)
                        iL1v = ConvertVLight(PsiLight->Shadow);
                    else
                        iL1v = PsiLight->Shadow & 0x3FF;

                    dFog = (iL1v * (32768.0f - PLVals->sPosLP[ir]) * dRough - PLVals->sShad -
                           PLVals->sShadZmul * (PLVals->sPosLightZpos[ir] + PLVals->sZZstmitDif)) *
                           PLVals->sShadGr * (1.0f - LiLSDAI[2][0]) * (1.0f - LiLSDAI[2][2]);
                    dAmbSh = std::min(1.0f, iL1v * (32768.0f - PLVals->sPosLP[ir]) * dRough * PLVals->sDynFogMul) * dFog;

                    TSVec t1, t2, t3;
                    Add2SVecsWeight(t1, LiLSDAI[4], DepC, LiLSDAI[2][0], LiLSDAI[2][2] * (1.0f - LiLSDAI[2][0]));
                    Add2SVecsWeightPacked(t2, PLVals->PLValigned->sDynFogCol, PLVals->PLValigned->sDynFogCol2, dFog - dAmbSh, dAmbSh);
                    ScaleSVec(t3, PLVals->PLValigned->sLCols[ir], dTmp3);
                    Add3SVectors(LiLSDAI[4], t1, t2, t3);
                }
            }
        }
    }
    tracePaintState("paint_after_vislights",
                    LiLSDAI[0][0], LiLSDAI[0][1], LiLSDAI[0][2],
                    LiLSDAI[1][0], LiLSDAI[1][1], LiLSDAI[1][2],
                    LiLSDAI[3][0], LiLSDAI[3][1], LiLSDAI[3][2],
                    LiLSDAI[4][0], LiLSDAI[4][1], LiLSDAI[4][2],
                    dAmbSh, dFog, dTmp, dTmp3);

    // Clamp and gamma
    LiLSDAI[4][3] = 0;
    mMinMaxSVec(LiLSDAI[0], 0, 255, LiLSDAI[4]);

    if (PLVals->iGammaH != 0) {
        if (PLVals->iGammaH > 0) {
            TSVec scaledForGamma;
            ScaleSVec(scaledForGamma, LiLSDAI[0], 255.0f);
            mSqrtSVec(LiLSDAI[5], scaledForGamma);
        } else {
            SqrSV255(LiLSDAI[5], LiLSDAI[0]);
        }
        LinInterpolate2SVecs(LiLSDAI[0], LiLSDAI[5], LiLSDAI[0], PLVals->sGamma);
    }

    // Write pixel (BGRA format for compatibility, but we store RGB)
    // Use lrint (banker's rounding) to match Pascal Round()
    int r = std::max(0, std::min(255, static_cast<int>(std::lrint(LiLSDAI[0][0]))));
    int g = std::max(0, std::min(255, static_cast<int>(std::lrint(LiLSDAI[0][1]))));
    int b = std::max(0, std::min(255, static_cast<int>(std::lrint(LiLSDAI[0][2]))));
    *SL = (r << 16) | (g << 8) | b;
}

// ============================================================================
// CalcPixelColorSqr -- Squared color variant
// ============================================================================

void CalcPixelColorSqr(uint32_t* SL, const TsiLight5* PsiLight,
                       TLightVals* PLVals, TPaintLightVals* PLV) {
    RenderWork();
    bool bSubAmbSh, noHS, softHS;
    int ir, iL1;
    float dRough, dTmp2, dAmbSh, dFog, dTmp = 0.0f, dTmp3 = 0.0f;
    TLightLSDAIs LiLSDAI;
    TSVec sLiSpe, DepC;

    if (PLVals->bAmbRelObj) {
        float arcVal = std::max(-1.0f, std::min(1.0f, PLV->AbsViewVec[1]));
        LinInterpolate2SVecsPacked(DepC, PLVals->PLValigned->sDepthCol2, PLVals->PLValigned->sDepthCol,
                                   std::asin(arcVal) * Pi1d + s05f);
    } else {
        std::memcpy(DepC, PLV->PreDepthCol, sizeof(TSVec));
    }

    if (PsiLight->Zpos < 32768) {
        calcAmbshadow(dAmbSh, PLVals->sAmbShad, PsiLight);
        dAmbSh = (dAmbSh * dAmbSh + dAmbSh) * s05f;
        dFog = PLVals->sDiffuseShadowing * (dAmbSh - 1.0f) + 1.0f;
        dRough = (PsiLight->RoughZposFine & 0xFF) * PLVals->sRoughnessFactor;

        MakeSVecFromNormalsD(LiLSDAI[0], PsiLight);
        ClearSVec(LiLSDAI[1]);
        ClearSVec(sLiSpe);
        ClearSVec(LiLSDAI[3]);

        ir = 0;
        while (ir < 6) {
            if (PLVals->iLightOption[ir] == 0) {
                bSubAmbSh = (PLVals->iHScalced[ir] ^ PLVals->iHSenabled[ir]) != 0;
                softHS = PLVals->iHSmask[ir] == -1;
                noHS = softHS || ((PsiLight->Shadow & PLVals->iHSmask[ir]) == 0) || (PLVals->iHScalced[ir] == 0);

                std::memcpy(LiLSDAI[5], PLVals->PLValigned->LN[ir], sizeof(TSVec));
                dTmp2 = 1.0f;

                if ((PLVals->iLightPos[ir] & 1) != 0) {
                    SubtractSVectors(LiLSDAI[5], LiLSDAI[5], PLV->ObjPos);
                    dTmp2 = SqrLengthOfSVec(LiLSDAI[5]);
                    if (dTmp2 > PLVals->sLmaxL[ir]) { ir++; continue; }
                    NormaliseSVectorVar(LiLSDAI[5]);
                    RotateSVectorReverseS(&LiLSDAI[5], PLV->PSmatrix);
                    dTmp2 = 1.0f / (dTmp2 + s1em30);
                }

                dTmp = GetCosTabValSqr(PLVals->iLightFuncDiff[ir], DotOfSVectors(LiLSDAI[0], LiLSDAI[5]), dRough) * dTmp2;
                AddSVecWeights(&LiLSDAI[3], &PLVals->PLValigned->sLCols[ir], dTmp);

                if (noHS) {
                    if (bSubAmbSh) dTmp *= dAmbSh; else dTmp *= dFog;
                    if (softHS) dTmp *= (PsiLight->Shadow >> 10) * s1d63;
                    AddSVecWeights(&LiLSDAI[1], &PLVals->PLValigned->sLCols[ir], dTmp);
                }

                dTmp = DotOf2VecNormalize(LiLSDAI[0], LiLSDAI[5], PLV->ViewVec);
                if (dTmp > 0) {
                    dTmp2 = (dTmp2 + std::min(1.0f, dRough * 2.0f) * (1.0f / PLVals->iLightPowFunc[ir] - dTmp2)) * PLVals->sSpec;
                    if (dTmp2 > 0) {
                        AddSVecWeights(&LiLSDAI[3], &PLVals->PLValigned->sLCols[ir], dTmp2 / PLVals->iLightPowFunc[ir]);
                        if (noHS) {
                            dTmp2 *= FastIntPow(dTmp, PLVals->iLightPowFunc[ir]);
                            if (bSubAmbSh) dTmp2 *= dAmbSh; else dTmp2 *= dFog;
                            if (softHS) dTmp2 *= (PsiLight->Shadow >> 10) * s1d63;
                            AddSVecWeights(&sLiSpe, &PLVals->PLValigned->sLCols[ir], dTmp2);
                        }
                    }
                }
            } else if (PLVals->iLightOption[ir] == 2 && PLVals->LLightMaps[ir] != nullptr) {
                TSVec lmPix, lmScaled;
                GetLightMapPixelSphere(lmPix, LiLSDAI[0], &PLVals->LLightMaps[ir]->PicRotMatrix,
                                       *PLVals->LLightMaps[ir], 1);
                ScaleSVec(lmScaled, lmPix, 255.0f);
                LinInterpolate2SVecs(LiLSDAI[4], PLVals->LLightMaps[ir]->LMavrgColSqr, lmScaled, dRough);
                AddSVecWeights(&LiLSDAI[3], &LiLSDAI[4], 1.0f);
                AddSVecWeights(&LiLSDAI[1], &LiLSDAI[4], dAmbSh);
            }
            ir++;
        }

        bSubAmbSh = PsiLight->SIgradient > 32767;
        LiLSDAI[2][0] = PLVals->sColZmul * PLV->zPos;
        if (bSubAmbSh)
            CalcColorsInside(&LiLSDAI[2], &LiLSDAI[5], PsiLight, PLVals);
        else
            CalcColors(&LiLSDAI[2], &LiLSDAI[5], PsiLight, PLVals);

        // DiffMap section
        if (PLVals->iColOnOT > 1 && PLVals->DiffColLightMap != nullptr) {
            if (PLVals->bYCcomb) {
                std::memcpy(LiLSDAI[4], LiLSDAI[2], sizeof(TSVec));
            }
            if (PLVals->iColOnOT > 3) {
                if (PLVals->iColOnOT > 5) {
                    GetDiffMapWrap3D(LiLSDAI[2], PLVals, &LiLSDAI[0], PLV);
                } else {
                    GetLightMapPixelSphere(LiLSDAI[2], LiLSDAI[0], &PLVals->DiffColLightMap->PicRotMatrix,
                                           *PLVals->DiffColLightMap, 1);
                }
                TSVec avgScaled;
                ScaleSVec(avgScaled, PLVals->DiffColLightMap->LMavrgCol, dRough * s1d255);
                Add2SVecsWeight(LiLSDAI[2], LiLSDAI[2], avgScaled, 1.0f - dRough, 1.0f);
            } else {
                dTmp = (PsiLight->OTrap & 0x7FFF) * 3.05186851e-5f - 0.5f;
                dTmp2 = PsiLight->SIgradient * 3.05186851e-5f - 0.5f;
                GetLightMapPixel(
                    LiLSDAI[2],
                    std::fmod((PLVals->DCLMapRotCos * dTmp + PLVals->DCLMapRotSin * dTmp2 +
                               PLVals->DCLMapOffX) * PLVals->lvMapScale + 1.0f, 1.0f),
                    std::fmod((PLVals->DCLMapRotCos * dTmp2 - PLVals->DCLMapRotSin * dTmp +
                               PLVals->DCLMapOffY) * PLVals->lvMapScale + 1.0f, 1.0f),
                    PLVals->DiffColLightMap, 1, 1);
            }
            if (PLVals->bYCcomb) {
                float denom = s001f + YofSVec_col(LiLSDAI[4]);
                ScaleSVec(LiLSDAI[2], LiLSDAI[4], YofSVec_col(LiLSDAI[2]) / denom);
            }
        }

        if (PLVals->bUseSmallBGpicForAmb) {
            RotateSVectorS(&LiLSDAI[0], PLV->PSmatrix);
            if (PLVals->BGsmallLM != nullptr) {
                GetLightMapPixelSphere(LiLSDAI[4], LiLSDAI[0], &PLVals->BGsmallLM->PicRotMatrix,
                                       *PLVals->BGsmallLM, 1);
                ScaleSVectorV(&LiLSDAI[4], 255.0f);
                dTmp = Max0S(1.0f - Max0S(1.0f - 28000.0f * PLVals->sDepth));
                if (PLVals->bFarFog) dTmp = dTmp * dTmp;
                if (PLVals->bAddBGlight) {
                    AddSVecWeights(&LiLSDAI[4], &DepC, dTmp);
                } else {
                    LinInterpolate2SVecs(LiLSDAI[4], LiLSDAI[4], DepC, 1.0f - dTmp);
                }
                ScaleSVectorV(&LiLSDAI[4], dAmbSh);
            } else {
                if (PLVals->bAmbRelObj)
                    LiLSDAI[0][1] = LiLSDAI[0][0] * (*PLV->PSmatrix)[0][1] + LiLSDAI[0][1] * (*PLV->PSmatrix)[1][1] + LiLSDAI[0][2] * (*PLV->PSmatrix)[2][1];
                dTmp2 = (LiLSDAI[0][1] * s05f + s05f) * dAmbSh;
                dTmp = dAmbSh - dTmp2;
                Add2SVecsWeightPacked(LiLSDAI[4], PLVals->PLValigned->sAmbCol, PLVals->PLValigned->sAmbCol2, dTmp, dTmp2);
            }
        } else {
            if (PLVals->bAmbRelObj)
                LiLSDAI[0][1] = LiLSDAI[0][0] * (*PLV->PSmatrix)[0][1] + LiLSDAI[0][1] * (*PLV->PSmatrix)[1][1] + LiLSDAI[0][2] * (*PLV->PSmatrix)[2][1];
            dTmp2 = (LiLSDAI[0][1] * s05f + s05f) * dAmbSh;
            dTmp = dAmbSh - dTmp2;
            Add2SVecsWeightPacked(LiLSDAI[4], PLVals->PLValigned->sAmbCol, PLVals->PLValigned->sAmbCol2, dTmp, dTmp2);
        }
        MultiplySVectorsV(&LiLSDAI[4], &LiLSDAI[2]);

        ScaleSVec(LiLSDAI[0], LiLSDAI[2], PLVals->sDiff);
        TSVec diffLight, specLight;
        MultiplySVectors(diffLight, LiLSDAI[0], LiLSDAI[1]);
        MultiplySVectors(specLight, LiLSDAI[5], sLiSpe);
        Add3SVectors(LiLSDAI[1], LiLSDAI[4], diffLight, specLight);

        if (PLVals->iExModes == 0)
            CalcTotalLight1(&LiLSDAI, PLVals, &dAmbSh);
        else
            CalcTotalLight2(&LiLSDAI, PLVals, &dAmbSh);

        dTmp = Max0S((static_cast<int>(PsiLight->Zpos) - 28000) * PLVals->sDepth + 1.0f);
    } else {
        if (PLVals->bBackBMP && PLVals->BGLightMap != nullptr) {
            if (PLVals->bDirectImageCoord) {
                GetLightMapPixel(LiLSDAI[4], PLV->xPos, PLV->yPos, PLVals->BGLightMap, 1, 0);
                ScaleSVectorV(&LiLSDAI[4], 255.0f);
            } else {
                GetLightMapPixelSphere(LiLSDAI[4], PLV->AbsViewVec, &PLVals->BGLightMap->PicRotMatrix,
                                       *PLVals->BGLightMap, 1);
                ScaleSVectorV(&LiLSDAI[4], 255.0f);
            }
        } else {
            std::memcpy(LiLSDAI[4], DepC, sizeof(TSVec));
        }
        dTmp = Max0S(1.0f - 28000.0f * PLVals->sDepth);
    }

    // Depth fog (squared variant)
    if (dTmp < 1.0f) {
        dTmp = (1.0f - dTmp) * (1.0f - dTmp);
        if (PLVals->bFarFog) dTmp = dTmp * dTmp;
        dTmp = 1.0f - dTmp;
    }

    if ((PsiLight->Zpos < 32768) || (!PLVals->bBackBMP) || (!PLVals->bAddBGlight))
        ScaleSVectorV(&LiLSDAI[4], dTmp);

    if (PLVals->bVolLight) ir = ConvertVLight(PsiLight->Shadow); else ir = PsiLight->Shadow & 0x3FF;
    dFog = (ir - PLVals->sShad - PLVals->sShadZmul * PLV->zPos) * PLVals->sShadGr;
    if ((PLVals->bDFogOptions & 2) != 0) dFog = Max0S(dFog);
    dTmp3 = std::min(1.0f, ir * PLVals->sDynFogMul) * dFog;
    Add2SVecsWeight2(LiLSDAI[4], LiLSDAI[4], DepC, Max0S(1.0f - dTmp));
    if ((PLVals->bDFogOptions & 1) != 0) {
        Clamp01Svar(dFog);
        Clamp01Svar(dTmp3);
        ScaleSVectorV(&LiLSDAI[4], 1.0f - dFog);
    }
    TSVec fogMix;
    Add2SVecsWeightPacked(fogMix, PLVals->PLValigned->sDynFogCol, PLVals->PLValigned->sDynFogCol2, dFog - dTmp3, dTmp3);
    AddSVectors_inplace(&LiLSDAI[4], fogMix);

    // Visible light sources (same pattern as CalcPixelColor2)
    iL1 = PsiLight->Zpos;
    if (iL1 < 32768) iL1 = 32768 - iL1;
    dRough = 1.0f / iL1;
    for (ir = 0; ir < 6; ir++) {
        if ((PLVals->iLightPos[ir] & 14) != 0) {
            noHS = (PLVals->iLightPos[ir] & 1) != 0;
            if (noHS) {
                TSVec camDiff;
                SubtractSVectors(camDiff, PLV->CamPos, PLVals->PLValigned->LN[ir]);
                dTmp3 = SqrDistSV(camDiff, PLV->AbsViewVec);
            } else {
                if (PsiLight->Zpos < 32768) continue;
                bSubAmbSh = false;
                dTmp3 = Max0S(1.0f - DotOfSVectors(PLVals->PLValigned->LN[ir], PLV->ViewVec));
            }
            LiLSDAI[2][0] = PLVals->sLmaxL[ir] * 1e-8f;
            if (dTmp3 < LiLSDAI[2][0]) {
                if (noHS) {
                    TSVec lightToCam;
                    SubtractSVectors(lightToCam, PLVals->PLValigned->LN[ir], PLV->CamPos);
                    if (DotOfSVectors(lightToCam, PLV->AbsViewVec) < 0) continue;
                    if (PsiLight->Zpos > 32767) { bSubAmbSh = false; }
                    else {
                        LiLSDAI[2][1] = -std::sqrt(LiLSDAI[2][0] - dTmp3);
                        if (std::fabs(PLV->AbsViewVec[0]) > s05f)
                            bSubAmbSh = LiLSDAI[2][1] > (PLV->ObjPos[0] - PLVals->PLValigned->LN[ir][0]) / PLV->AbsViewVec[0];
                        else if (std::fabs(PLV->AbsViewVec[1]) > s05f)
                            bSubAmbSh = LiLSDAI[2][1] > (PLV->ObjPos[1] - PLVals->PLValigned->LN[ir][1]) / PLV->AbsViewVec[1];
                        else
                            bSubAmbSh = LiLSDAI[2][1] > (PLV->ObjPos[2] - PLVals->PLValigned->LN[ir][2]) / PLV->AbsViewVec[2];
                    }
                }
                if (bSubAmbSh) continue;
                CalcPosLightShape(dTmp3, LiLSDAI[2][0], ir, PLVals);
                LiLSDAI[2][2] = 1.0f - Max0S(1.0f + (PLVals->sPosLP[ir] - 28000.0f) * PLVals->sDepth);
                if (LiLSDAI[2][2] > 0) {
                    LiLSDAI[2][2] *= LiLSDAI[2][2];
                    if (PLVals->bFarFog) LiLSDAI[2][2] *= LiLSDAI[2][2];
                }
                dTmp3 *= (1.0f - LiLSDAI[2][2] * 0.9f);
                int posOpt = PLVals->iLightPos[ir] & 14;
                if (posOpt == 2 || posOpt == 8) {
                    Add2SVecsWeight2(LiLSDAI[4], LiLSDAI[4], PLVals->PLValigned->sLCols[ir], dTmp3);
                } else {
                    LiLSDAI[2][2] = Max0S(LiLSDAI[2][2]);
                    int iL1v = PLVals->bVolLight ? ConvertVLight(PsiLight->Shadow) : (PsiLight->Shadow & 0x3FF);
                    dFog = (iL1v * (32768.0f - PLVals->sPosLP[ir]) * dRough - PLVals->sShad -
                           PLVals->sShadZmul * (PLVals->sPosLightZpos[ir] + PLVals->sZZstmitDif)) *
                           PLVals->sShadGr * (1.0f - LiLSDAI[2][0]) * (1.0f - LiLSDAI[2][2]);
                    dAmbSh = std::min(1.0f, iL1v * (32768.0f - PLVals->sPosLP[ir]) * dRough * PLVals->sDynFogMul) * dFog;
                    TSVec t1, t2, t3;
                    Add2SVecsWeight(t1, LiLSDAI[4], DepC, LiLSDAI[2][0], LiLSDAI[2][2] * (1.0f - LiLSDAI[2][0]));
                    Add2SVecsWeightPacked(t2, PLVals->PLValigned->sDynFogCol, PLVals->PLValigned->sDynFogCol2, dFog - dAmbSh, dAmbSh);
                    ScaleSVec(t3, PLVals->PLValigned->sLCols[ir], dTmp3);
                    Add3SVectors(LiLSDAI[4], t1, t2, t3);
                }
            }
        }
    }

    // Gamma and output (squared variant uses sqrt)
    LiLSDAI[4][3] = 0;
    TSVec scaled255;
    ScaleSVec(scaled255, LiLSDAI[4], s255f);
    mMinMaxSVec(scaled255, 0, 65025, scaled255);
    mSqrtSVec(LiLSDAI[0], scaled255);

    if (PLVals->iGammaH != 0) {
        if (PLVals->iGammaH > 0) {
            TSVec scaledForGamma;
            ScaleSVec(scaledForGamma, LiLSDAI[0], 255.0f);
            mSqrtSVec(LiLSDAI[5], scaledForGamma);
        } else {
            SqrSV255(LiLSDAI[5], LiLSDAI[0]);
        }
        LinInterpolate2SVecs(LiLSDAI[0], LiLSDAI[5], LiLSDAI[0], PLVals->sGamma);
    }

    // Use lrint (banker's rounding) to match Pascal Round()
    int r = std::max(0, std::min(255, static_cast<int>(std::lrint(LiLSDAI[0][0]))));
    int g = std::max(0, std::min(255, static_cast<int>(std::lrint(LiLSDAI[0][1]))));
    int b = std::max(0, std::min(255, static_cast<int>(std::lrint(LiLSDAI[0][2]))));
    *SL = (r << 16) | (g << 8) | b;
}

// ============================================================================
// CalcPixelColorSvec -- SVec-based coloring (for reflections)
// ============================================================================

void CalcPixelColorSvec(TSVec* SVcol, TLightSD& InSD, const TsiLight5* PsiLight,
                        TLightVals* PLVals, TPaintLightVals* PLV, TSVec& ResultTransp) {
    RenderWork();
    // Ported from PaintThread.pas CalcPixelColorSvec -- outputs float SVec for reflections
    bool bSubAmbSh, noHS, softHS;
    int ir, iL1;
    float dRough, dTmp2, dAmbSh = 0.0f, dFog = 0.0f, dTmp = 0.0f, dTmp3 = 0.0f;
    TLightLSDAIs LiLSDAI{};
    TSVec sLiSpe, DepC;

    auto traceSvecState = [&](const char* stage, float a0, float a1, float a2,
                              float b0, float b1, float b2,
                              float c0, float c1, float c2,
                              float d0, float d1, float d2,
                              float amb, float fog, float tmp, float tmp3) {
        if (!(HasParityTraceContext() && IsPaintSvecTraceScopeEnabled() && ShouldLogParityStage(stage))) return;
        std::ostringstream payload;
        payload.setf(std::ios::fixed, std::ios::floatfield);
        payload << std::setprecision(12)
                << " amb=" << amb
                << " fog=" << fog
                << " tmp=" << tmp
                << " tmp3=" << tmp3
                << " V0X=" << a0
                << " V0Y=" << a1
                << " V0Z=" << a2
                << " V1X=" << b0
                << " V1Y=" << b1
                << " V1Z=" << b2
                << " V3X=" << c0
                << " V3Y=" << c1
                << " V3Z=" << c2
                << " V4X=" << d0
                << " V4Y=" << d1
                << " V4Z=" << d2;
        AppendParityTrace(stage, payload.str());
    };

    auto traceSvecColors = [&](const char* stage, const TSVec& dif, const TSVec& spe,
                               bool interior, float zColorBase) {
        if (!(HasParityTraceContext() && IsPaintSvecTraceScopeEnabled() && ShouldLogParityStage(stage))) return;
        std::ostringstream payload;
        payload.setf(std::ios::fixed, std::ios::floatfield);
        payload << std::setprecision(12)
                << " interior=" << (interior ? 1 : 0)
                << " zColorBase=" << zColorBase
                << " DifX=" << dif[0]
                << " DifY=" << dif[1]
                << " DifZ=" << dif[2]
                << " SpeX=" << spe[0]
                << " SpeY=" << spe[1]
                << " SpeZ=" << spe[2];
        AppendParityTrace(stage, payload.str());
    };

    if (HasParityTraceContext() && IsPaintSvecTraceScopeEnabled() && ShouldLogParityStage("paint_svec_pre")) {
        std::ostringstream payload;
        payload.setf(std::ios::fixed, std::ios::floatfield);
        payload << std::setprecision(12)
                << " Zpos=" << PsiLight->Zpos
                << " RoughZposFine=" << PsiLight->RoughZposFine
                << " Shadow=" << PsiLight->Shadow
                << " AmbShadow=" << PsiLight->AmbShadow
                << " SIgradient=" << PsiLight->SIgradient
                << " OTrap=" << PsiLight->OTrap
                << " xPos=" << PLV->xPos
                << " yPos=" << PLV->yPos
                << " zPos=" << PLV->zPos
                << " ViewVecX=" << PLV->ViewVec[0]
                << " ViewVecY=" << PLV->ViewVec[1]
                << " ViewVecZ=" << PLV->ViewVec[2]
                << " AbsViewVecX=" << PLV->AbsViewVec[0]
                << " AbsViewVecY=" << PLV->AbsViewVec[1]
                << " AbsViewVecZ=" << PLV->AbsViewVec[2]
                << " CamPosX=" << PLV->CamPos[0]
                << " CamPosY=" << PLV->CamPos[1]
                << " CamPosZ=" << PLV->CamPos[2]
                << " ObjPosX=" << PLV->ObjPos[0]
                << " ObjPosY=" << PLV->ObjPos[1]
                << " ObjPosZ=" << PLV->ObjPos[2]
                << " PreDepthColX=" << PLV->PreDepthCol[0]
                << " PreDepthColY=" << PLV->PreDepthCol[1]
                << " PreDepthColZ=" << PLV->PreDepthCol[2]
                << " sAmbShad=" << PLVals->sAmbShad
                << " sDiffuseShadowing=" << PLVals->sDiffuseShadowing
                << " sIndLightReflect=" << PLVals->sIndLightReflect
                << " sDiff=" << PLVals->sDiff
                << " sSpec=" << PLVals->sSpec
                << " sDepth=" << PLVals->sDepth
                << " sDynFogMul=" << PLVals->sDynFogMul
                << " sShad=" << PLVals->sShad
                << " sShadZmul=" << PLVals->sShadZmul
                << " sShadGr=" << PLVals->sShadGr
                << " iExModes=" << PLVals->iExModes
                << " iGammaH=" << PLVals->iGammaH
                << " bUseSmallBGpicForAmb=" << (PLVals->bUseSmallBGpicForAmb ? 1 : 0)
                << " bBackBMP=" << (PLVals->bBackBMP ? 1 : 0)
                << " bAddBGlight=" << (PLVals->bAddBGlight ? 1 : 0)
                << " bDirectImageCoord=" << (PLVals->bDirectImageCoord ? 1 : 0)
                << " bFarFog=" << (PLVals->bFarFog ? 1 : 0)
                << " bVolLight=" << (PLVals->bVolLight ? 1 : 0)
                << " bAmbRelObj=" << (PLVals->bAmbRelObj ? 1 : 0);
        AppendParityTrace("paint_svec_pre", payload.str());
    }

    if (PLVals->bAmbRelObj) {
        float arcVal = std::max(-1.0f, std::min(1.0f, PLV->AbsViewVec[1]));
        LinInterpolate2SVecsPacked(DepC, PLVals->PLValigned->sDepthCol2, PLVals->PLValigned->sDepthCol,
                                   std::asin(arcVal) * Pi1d + s05f);
    } else {
        std::memcpy(DepC, PLV->PreDepthCol, sizeof(TSVec));
    }

    // Initialize result transparency
    ResultTransp[0] = 1; ResultTransp[1] = 1; ResultTransp[2] = 1; ResultTransp[3] = 0;

    if (PsiLight->Zpos < 32768) {
        calcAmbshadow(dAmbSh, PLVals->sAmbShad, PsiLight);
        if (PLVals->bCalcPixColSqr) dAmbSh = dAmbSh * dAmbSh;
        dFog = PLVals->sDiffuseShadowing * (dAmbSh - 1.0f) + 1.0f;
        dRough = (PsiLight->RoughZposFine & 0xFF) * PLVals->sRoughnessFactor;
        if (HasParityTraceContext() && IsPaintSvecTraceScopeEnabled() && ShouldLogParityStage("paint_svec_after_ambshadow")) {
            std::ostringstream payload;
            payload.setf(std::ios::fixed, std::ios::floatfield);
            payload << std::setprecision(12)
                    << " amb=" << dAmbSh
                    << " fog=" << dFog
                    << " rough=" << dRough
                    << " Zpos=" << PsiLight->Zpos
                    << " Shadow=" << PsiLight->Shadow
                    << " AmbShadow=" << PsiLight->AmbShadow
                    << " SIgradient=" << PsiLight->SIgradient
                    << " OTrap=" << PsiLight->OTrap
                    << " sAmbShad=" << PLVals->sAmbShad
                    << " sDiffuseShadowing=" << PLVals->sDiffuseShadowing;
            AppendParityTrace("paint_svec_after_ambshadow", payload.str());
        }

        MakeSVecFromNormalsD(LiLSDAI[0], PsiLight);
        ClearSVec(LiLSDAI[1]);
        ClearSVec(sLiSpe);
        ClearSVec(LiLSDAI[3]);

        ir = 0;
        while (ir < 6) {
            if (PLVals->iLightOption[ir] == 0) {
                bSubAmbSh = (PLVals->iHScalced[ir] ^ PLVals->iHSenabled[ir]) != 0;
                softHS = PLVals->iHSmask[ir] == -1;
                noHS = softHS || ((PsiLight->Shadow & PLVals->iHSmask[ir]) == 0) || (PLVals->iHScalced[ir] == 0);
                std::memcpy(LiLSDAI[5], PLVals->PLValigned->LN[ir], sizeof(TSVec));
                dTmp2 = 1.0f;
                if ((PLVals->iLightPos[ir] & 1) != 0) {
                    SubtractSVectors(LiLSDAI[5], LiLSDAI[5], PLV->ObjPos);
                    dTmp2 = SqrLengthOfSVec(LiLSDAI[5]);
                    if (dTmp2 > PLVals->sLmaxL[ir]) { ir++; continue; }
                    NormaliseSVectorVar(LiLSDAI[5]);
                    RotateSVectorReverseS(&LiLSDAI[5], PLV->PSmatrix);
                    dTmp2 = 1.0f / (dTmp2 + s1em30);
                }
                if (PLVals->bCalcPixColSqr)
                    dTmp = GetCosTabValSqr(PLVals->iLightFuncDiff[ir], DotOfSVectors(LiLSDAI[0], LiLSDAI[5]), dRough) * dTmp2;
                else
                    dTmp = GetCosTabVal(PLVals->iLightFuncDiff[ir], DotOfSVectors(LiLSDAI[0], LiLSDAI[5]), dRough) * dTmp2;
                if (HasParityTraceContext() && ShouldLogParityStage("paint_light_input")) {
                    std::ostringstream payload;
                    payload << std::fixed << std::setprecision(12)
                            << " ir=" << ir << " dot=" << DotOfSVectors(LiLSDAI[0], LiLSDAI[5])
                            << " weight=" << dTmp
                            << " colX=" << PLVals->PLValigned->sLCols[ir][0]
                            << " colY=" << PLVals->PLValigned->sLCols[ir][1]
                            << " colZ=" << PLVals->PLValigned->sLCols[ir][2]
                            << " pow=" << PLVals->iLightPowFunc[ir];
                    AppendParityTrace("paint_light_input", payload.str());
                }
                AddSVecWeights(&LiLSDAI[3], &PLVals->PLValigned->sLCols[ir], dTmp);
                if (noHS) {
                    if (bSubAmbSh) dTmp *= dAmbSh; else dTmp *= dFog;
                    if (softHS) dTmp *= (PsiLight->Shadow >> 10) * s1d63;
                    AddSVecWeights(&LiLSDAI[1], &PLVals->PLValigned->sLCols[ir], dTmp);
                }
                dTmp = DotOf2VecNormalize(LiLSDAI[0], LiLSDAI[5], PLV->ViewVec);
                if (dTmp > 0) {
                    dTmp2 = (dTmp2 + std::min(1.0f, dRough * 2.0f) * (1.0f / PLVals->iLightPowFunc[ir] - dTmp2)) * PLVals->sSpec;
                    if (dTmp2 > 0) {
                        AddSVecWeights(&LiLSDAI[3], &PLVals->PLValigned->sLCols[ir], dTmp2 / PLVals->iLightPowFunc[ir]);
                        if (noHS) {
                            dTmp2 *= FastIntPow(dTmp, PLVals->iLightPowFunc[ir]);
                            if (bSubAmbSh) dTmp2 *= dAmbSh; else dTmp2 *= dFog;
                            if (softHS) dTmp2 *= (PsiLight->Shadow >> 10) * s1d63;
                            AddSVecWeights(&sLiSpe, &PLVals->PLValigned->sLCols[ir], dTmp2);
                        }
                    }
                }
            } else if (PLVals->iLightOption[ir] == 2 && PLVals->LLightMaps[ir] != nullptr) {
                // Lightmap lights
                TSVec lmPix, lmScaled;
                GetLightMapPixelSphere(lmPix, LiLSDAI[0], &PLVals->LLightMaps[ir]->PicRotMatrix,
                                       *PLVals->LLightMaps[ir], PLVals->bCalcPixColSqr);
                ScaleSVec(lmScaled, lmPix, 255.0f);
                if (PLVals->bCalcPixColSqr)
                    LinInterpolate2SVecs(LiLSDAI[4], PLVals->LLightMaps[ir]->LMavrgColSqr, lmScaled, dRough);
                else
                    LinInterpolate2SVecs(LiLSDAI[4], PLVals->LLightMaps[ir]->LMavrgCol, lmScaled, dRough);
                AddSVecWeights(&LiLSDAI[3], &LiLSDAI[4], 1.0f);
                AddSVecWeights(&LiLSDAI[1], &LiLSDAI[4], dAmbSh);
            }
            ir++;
        }

        bSubAmbSh = PsiLight->SIgradient > 32767;
        LiLSDAI[2][0] = PLVals->sColZmul * PLV->zPos;
        float zColorBase = LiLSDAI[2][0];
        if (bSubAmbSh)
            CalcColorsInside(&LiLSDAI[2], &LiLSDAI[5], PsiLight, PLVals);
        else
            CalcColors(&LiLSDAI[2], &LiLSDAI[5], PsiLight, PLVals);
        traceSvecColors("paint_svec_after_colors", LiLSDAI[2], LiLSDAI[5], bSubAmbSh, zColorBase);

        // DiffMap section
        if (PLVals->iColOnOT > 1 && PLVals->DiffColLightMap != nullptr) {
            if (PLVals->bYCcomb) {
                std::memcpy(LiLSDAI[4], LiLSDAI[2], sizeof(TSVec));
            }
            if (PLVals->iColOnOT > 3) {
                if (PLVals->iColOnOT > 5) {
                    GetDiffMapWrap3D(LiLSDAI[2], PLVals, &LiLSDAI[0], PLV);
                } else {
                    GetLightMapPixelSphere(LiLSDAI[2], LiLSDAI[0], &PLVals->DiffColLightMap->PicRotMatrix,
                                           *PLVals->DiffColLightMap, PLVals->bCalcPixColSqr);
                }
                TSVec avgScaled;
                ScaleSVec(avgScaled, PLVals->DiffColLightMap->LMavrgCol, dRough * s1d255);
                Add2SVecsWeight(LiLSDAI[2], LiLSDAI[2], avgScaled, 1.0f - dRough, 1.0f);
            } else {
                dTmp = (PsiLight->OTrap & 0x7FFF) * 3.05186851e-5f - 0.5f;
                dTmp2 = PsiLight->SIgradient * 3.05186851e-5f - 0.5f;
                GetLightMapPixel(
                    LiLSDAI[2],
                    std::fmod((PLVals->DCLMapRotCos * dTmp + PLVals->DCLMapRotSin * dTmp2 +
                               PLVals->DCLMapOffX) * PLVals->lvMapScale + 1.0f, 1.0f),
                    std::fmod((PLVals->DCLMapRotCos * dTmp2 - PLVals->DCLMapRotSin * dTmp +
                               PLVals->DCLMapOffY) * PLVals->lvMapScale + 1.0f, 1.0f),
                    PLVals->DiffColLightMap, PLVals->bCalcPixColSqr, 1);
            }
            if (PLVals->bYCcomb) {
                float denom = s001f + YofSVec_col(LiLSDAI[4]);
                ScaleSVec(LiLSDAI[2], LiLSDAI[4], YofSVec_col(LiLSDAI[2]) / denom);
            }
        }

        InSD[0][0] = LiLSDAI[5][0]; InSD[0][1] = LiLSDAI[5][1]; InSD[0][2] = LiLSDAI[5][2]; InSD[0][3] = LiLSDAI[5][3];
        InSD[1][0] = LiLSDAI[2][0]; InSD[1][1] = LiLSDAI[2][1]; InSD[1][2] = LiLSDAI[2][2]; InSD[1][3] = LiLSDAI[2][3];

        float dTmp3_local = PLVals->sDiff;
        if (PLVals->bScaleAmbDiffDown) {
            dTmp = 1.0f - InSD[0][3] * PLVals->SRLightAmount;
            dAmbSh *= dTmp;
            dTmp3_local = PLVals->sDiff * dTmp;
        }

        // Ambient light calculation
        if (PLVals->bUseSmallBGpicForAmb) {
            RotateSVectorS(&LiLSDAI[0], PLV->PSmatrix);
            if (PLVals->BGsmallLM != nullptr) {
                GetLightMapPixelSphere(LiLSDAI[4], LiLSDAI[0], &PLVals->BGsmallLM->PicRotMatrix,
                                       *PLVals->BGsmallLM, PLVals->bCalcPixColSqr);
                ScaleSVectorV(&LiLSDAI[4], 255.0f);
                dTmp = Max0S(1.0f - Max0S(1.0f - 28000.0f * PLVals->sDepth));
                if (PLVals->bFarFog) dTmp = dTmp * dTmp;
                if (PLVals->bAddBGlight) {
                    AddSVecWeights(&LiLSDAI[4], &DepC, dTmp);
                } else {
                    LinInterpolate2SVecs(LiLSDAI[4], LiLSDAI[4], DepC, 1.0f - dTmp);
                }
                ScaleSVectorV(&LiLSDAI[4], dAmbSh);
            } else {
                if (PLVals->bAmbRelObj)
                    LiLSDAI[0][1] = LiLSDAI[0][0] * (*PLV->PSmatrix)[0][1] + LiLSDAI[0][1] * (*PLV->PSmatrix)[1][1] + LiLSDAI[0][2] * (*PLV->PSmatrix)[2][1];
                dTmp2 = (LiLSDAI[0][1] * s05f + s05f) * dAmbSh;
                dTmp = dAmbSh - dTmp2;
                Add2SVecsWeightPacked(LiLSDAI[4], PLVals->PLValigned->sAmbCol, PLVals->PLValigned->sAmbCol2, dTmp, dTmp2);
            }
        } else {
            if (PLVals->bAmbRelObj)
                LiLSDAI[0][1] = LiLSDAI[0][0] * (*PLV->PSmatrix)[0][1] + LiLSDAI[0][1] * (*PLV->PSmatrix)[1][1] + LiLSDAI[0][2] * (*PLV->PSmatrix)[2][1];
            dTmp2 = (LiLSDAI[0][1] * s05f + s05f) * dAmbSh;
            dTmp = dAmbSh - dTmp2;
            Add2SVecsWeightPacked(LiLSDAI[4], PLVals->PLValigned->sAmbCol, PLVals->PLValigned->sAmbCol2, dTmp, dTmp2);
        }
        MultiplySVectorsV(&LiLSDAI[4], &LiLSDAI[2]);

        ScaleSVec(LiLSDAI[0], LiLSDAI[2], dTmp3_local);
        TSVec dL, sL;
        MultiplySVectors(dL, LiLSDAI[0], LiLSDAI[1]);
        MultiplySVectors(sL, LiLSDAI[5], sLiSpe);
        Add3SVectors(LiLSDAI[1], LiLSDAI[4], dL, sL);
        traceSvecState("paint_svec_before_total",
                       LiLSDAI[0][0], LiLSDAI[0][1], LiLSDAI[0][2],
                       LiLSDAI[1][0], LiLSDAI[1][1], LiLSDAI[1][2],
                       LiLSDAI[3][0], LiLSDAI[3][1], LiLSDAI[3][2],
                       LiLSDAI[4][0], LiLSDAI[4][1], LiLSDAI[4][2],
                       dAmbSh, dFog, dTmp, dTmp3);

        if (PLVals->iExModes == 0)
            CalcTotalLight1(&LiLSDAI, PLVals, &dAmbSh);
        else
            CalcTotalLight2(&LiLSDAI, PLVals, &dAmbSh);
        traceSvecState("paint_svec_after_object",
                       LiLSDAI[0][0], LiLSDAI[0][1], LiLSDAI[0][2],
                       LiLSDAI[1][0], LiLSDAI[1][1], LiLSDAI[1][2],
                       LiLSDAI[3][0], LiLSDAI[3][1], LiLSDAI[3][2],
                       LiLSDAI[4][0], LiLSDAI[4][1], LiLSDAI[4][2],
                       dAmbSh, dFog, dTmp, dTmp3);

        dTmp = Max0S(1.0f + (static_cast<int>(PsiLight->Zpos) - 28000) * PLVals->sDepth);
    } else {
        // Background
        if (PLVals->bBackBMP && PLVals->BGLightMap != nullptr) {
            if (PLVals->bDirectImageCoord) {
                GetLightMapPixel(LiLSDAI[4], PLV->xPos, PLV->yPos, PLVals->BGLightMap, PLVals->bCalcPixColSqr, 0);
                ScaleSVectorV(&LiLSDAI[4], 255.0f);
            } else {
                GetLightMapPixelSphere(LiLSDAI[4], PLV->AbsViewVec, &PLVals->BGLightMap->PicRotMatrix,
                                       *PLVals->BGLightMap, PLVals->bCalcPixColSqr);
                ScaleSVectorV(&LiLSDAI[4], 255.0f);
            }
        } else {
            std::memcpy(LiLSDAI[4], DepC, sizeof(TSVec));
        }
        dTmp = Max0S(1.0f - (60768.0f - static_cast<int>(PsiLight->Zpos)) * PLVals->sDepth);
    }

    if (dTmp < 1.0f) {
        dTmp = 1.0f - dTmp;
        if (PLVals->bCalcPixColSqr) dTmp = dTmp * dTmp;
        if (PLVals->bFarFog) dTmp = dTmp * dTmp;
        dTmp = 1.0f - dTmp;
    }

    if ((PsiLight->Zpos < 32768) || (!PLVals->bBackBMP) || (!PLVals->bAddBGlight))
        ScaleSVectorV(&LiLSDAI[4], dTmp);

    if (PLVals->bVolLight) ir = ConvertVLight(PsiLight->Shadow); else ir = PsiLight->Shadow & 0x3FF;
    dFog = (ir - PLVals->sShad - PLVals->sShadZmul * PLVals->ZposDynFog) * PLVals->sShadGr;
    if ((PLVals->bDFogOptions & 2) != 0) dFog = Max0S(dFog);
    dTmp3 = std::min(1.0f, ir * PLVals->sDynFogMul) * dFog;
    Add2SVecsWeight2(LiLSDAI[4], LiLSDAI[4], DepC, Max0S(1.0f - dTmp));
    if ((PLVals->bDFogOptions & 1) != 0) {
        Clamp01Svar(dFog);
        Clamp01Svar(dTmp3);
        ScaleSVectorV(&LiLSDAI[4], 1.0f - dFog);
        ScaleSVectorV(&ResultTransp, 1.0f - dFog);
    }
    TSVec fogMix;
    Add2SVecsWeightPacked(fogMix, PLVals->PLValigned->sDynFogCol, PLVals->PLValigned->sDynFogCol2, dFog - dTmp3, dTmp3);
    AddSVectors_inplace(&LiLSDAI[4], fogMix);
    ScaleSVectorV(&ResultTransp, std::min(1.0f, dTmp));
    traceSvecState("paint_svec_after_fog",
                   LiLSDAI[0][0], LiLSDAI[0][1], LiLSDAI[0][2],
                   LiLSDAI[1][0], LiLSDAI[1][1], LiLSDAI[1][2],
                   LiLSDAI[3][0], LiLSDAI[3][1], LiLSDAI[3][2],
                   LiLSDAI[4][0], LiLSDAI[4][1], LiLSDAI[4][2],
                   dAmbSh, dFog, dTmp, dTmp3);

    // Visible light sources
    iL1 = PsiLight->Zpos;
    if (iL1 < 32768) iL1 = 32768 - iL1;
    dRough = 1.0f / iL1;

    for (ir = 0; ir < 6; ir++) {
        if ((PLVals->iLightPos[ir] & 14) != 0) {
            noHS = (PLVals->iLightPos[ir] & 1) != 0;
            if (noHS) {
                // Visible positional light
                TSVec camDiff;
                SubtractSVectors(camDiff, PLV->CamPos, PLVals->PLValigned->LN[ir]);
                dTmp3 = SqrDistSV(camDiff, PLV->AbsViewVec);
            } else {
                // Visible global light
                if (PsiLight->Zpos < 32768) continue;
                bSubAmbSh = false;
                dTmp3 = Max0S(1.0f - DotOfSVectors(PLVals->PLValigned->LN[ir], PLV->ViewVec));
            }

            LiLSDAI[2][0] = PLVals->sLmaxL[ir] * 1e-8f;
            if (dTmp3 < LiLSDAI[2][0]) {
                if (noHS) {
                    // Positional light: check if light is behind viewer
                    TSVec lightToCam;
                    SubtractSVectors(lightToCam, PLVals->PLValigned->LN[ir], PLV->CamPos);
                    if (DotOfSVectors(lightToCam, PLV->AbsViewVec) < 0) continue;
                    if (PsiLight->Zpos > 32767) {
                        bSubAmbSh = false;
                    } else {
                        LiLSDAI[2][1] = -std::sqrt(LiLSDAI[2][0] - dTmp3);
                        if (std::fabs(PLV->AbsViewVec[0]) > s05f)
                            bSubAmbSh = LiLSDAI[2][1] > (PLV->ObjPos[0] - PLVals->PLValigned->LN[ir][0]) / PLV->AbsViewVec[0];
                        else if (std::fabs(PLV->AbsViewVec[1]) > s05f)
                            bSubAmbSh = LiLSDAI[2][1] > (PLV->ObjPos[1] - PLVals->PLValigned->LN[ir][1]) / PLV->AbsViewVec[1];
                        else
                            bSubAmbSh = LiLSDAI[2][1] > (PLV->ObjPos[2] - PLVals->PLValigned->LN[ir][2]) / PLV->AbsViewVec[2];
                    }
                }
                if (bSubAmbSh) continue;

                CalcPosLightShape(dTmp3, LiLSDAI[2][0], ir, PLVals);

                LiLSDAI[2][2] = 1.0f - Max0S(1.0f + (PLVals->sPosLP[ir] - 28000.0f) * PLVals->sDepth);
                if (LiLSDAI[2][2] > 0) {
                    if (PLVals->bCalcPixColSqr) LiLSDAI[2][2] = LiLSDAI[2][2] * LiLSDAI[2][2];
                    if (PLVals->bFarFog) LiLSDAI[2][2] = LiLSDAI[2][2] * LiLSDAI[2][2];
                }
                dTmp3 *= (1.0f - LiLSDAI[2][2] * 0.9f);

                int posOpt = PLVals->iLightPos[ir] & 14;
                if (posOpt == 2 || posOpt == 8) {
                    // vislight3: don't decrease light behind, just descale vislight
                    Add2SVecsWeight2(LiLSDAI[4], LiLSDAI[4], PLVals->PLValigned->sLCols[ir], dTmp3);
                } else {
                    LiLSDAI[2][2] = Max0S(LiLSDAI[2][2]);
                    int iL1v;
                    if (PLVals->bVolLight) iL1v = ConvertVLight(PsiLight->Shadow); else iL1v = PsiLight->Shadow & 0x3FF;
                    dFog = (iL1v * (32768.0f - PLVals->sPosLP[ir]) * dRough - PLVals->sShad -
                           PLVals->sShadZmul * (PLVals->sPosLightZpos[ir] + PLVals->sZZstmitDif)) *
                           PLVals->sShadGr * (1.0f - LiLSDAI[2][0]) * (1.0f - LiLSDAI[2][2]);
                    dAmbSh = std::min(1.0f, iL1v * (32768.0f - PLVals->sPosLP[ir]) * dRough * PLVals->sDynFogMul) * dFog;
                    TSVec t1, t2, t3;
                    Add2SVecsWeight(t1, LiLSDAI[4], DepC, LiLSDAI[2][0], LiLSDAI[2][2] * (1.0f - LiLSDAI[2][0]));
                    Add2SVecsWeightPacked(t2, PLVals->PLValigned->sDynFogCol, PLVals->PLValigned->sDynFogCol2, dFog - dAmbSh, dAmbSh);
                    ScaleSVec(t3, PLVals->PLValigned->sLCols[ir], dTmp3);
                    Add3SVectors(LiLSDAI[4], t1, t2, t3);
                }
                ScaleSVectorV(&ResultTransp, std::min(1.0f, LiLSDAI[2][0])); // descale with hidden by visposlights
            }
        }
    }
    traceSvecState("paint_svec_after_vislights",
                   LiLSDAI[0][0], LiLSDAI[0][1], LiLSDAI[0][2],
                   LiLSDAI[1][0], LiLSDAI[1][1], LiLSDAI[1][2],
                   LiLSDAI[3][0], LiLSDAI[3][1], LiLSDAI[3][2],
                   LiLSDAI[4][0], LiLSDAI[4][1], LiLSDAI[4][2],
                   dAmbSh, dFog, dTmp, dTmp3);

    LiLSDAI[4][3] = 0;
    // Pascal: SVcol^ := mMinMaxSVec(0, 8, ScaleSVector(LiLSDAI[4], s1d255));
    // Scale FIRST (÷255), THEN clamp to [0, 8]
    TSVec scaledTmp;
    ScaleSVec(scaledTmp, LiLSDAI[4], s1d255);
    mMinMaxSVec(*SVcol, 0, 8, scaledTmp);
    if (HasParityTraceContext() && IsPaintSvecTraceScopeEnabled() && ShouldLogParityStage("paint_svec")) {
        std::ostringstream payload;
        payload.setf(std::ios::fixed, std::ios::floatfield);
        payload << std::setprecision(12)
                << " SVcolX=" << (*SVcol)[0]
                << " SVcolY=" << (*SVcol)[1]
                << " SVcolZ=" << (*SVcol)[2]
                << " ResultX=" << ResultTransp[0]
                << " ResultY=" << ResultTransp[1]
                << " ResultZ=" << ResultTransp[2]
                << " SD0X=" << InSD[0][0]
                << " SD0Y=" << InSD[0][1]
                << " SD0Z=" << InSD[0][2]
                << " SD0W=" << InSD[0][3]
                << " SD1X=" << InSD[1][0]
                << " SD1Y=" << InSD[1][1]
                << " SD1Z=" << InSD[1][2]
                << " SD1W=" << InSD[1][3];
        AppendParityTrace("paint_svec", payload.str());
    }
}

// ============================================================================
// CalcPixelColorSvecTrans
// ============================================================================

void CalcPixelColorSvecTrans(TSVec* SVcol, TLightSD& InSD, const TsiLight5* PsiLight,
                             TLightVals* PLVals, TPaintLightVals* PLV, TSVec& ResultTransp) {
    RenderWork();
    // Ported from PaintThread.pas CalcPixelColorSvecTrans (line 1251)
    // Transparent/transmission coloring for reflected rays.
    // Key differences from CalcPixelColorSvec:
    //   - Absorption color tracking (sAbsorpCoeff) via bDivOptions
    //   - Inside/outside rendering differentiation (bUseSmallBGpicForAmb)
    //   - bDivOptions flag controls depth fog application
    //   - bScaleAmbDiffDown for inside transmission
    //   - DiffOut tracks diffuse color separately for output
    bool bSubAmbSh, noHS, softHS;
    int ir, iL1;
    float dRough, dTmp2, dAmbSh, dFog, dTmp = 0.0f, dTmp3 = 0.0f;
    TLightLSDAIs LiLSDAI;
    TSVec DepC, sLiSpe, DiffOut;

    if (PLVals->bAmbRelObj) {
        float arcVal = std::max(-1.0f, std::min(1.0f, PLV->AbsViewVec[1]));
        LinInterpolate2SVecsPacked(DepC, PLVals->PLValigned->sDepthCol2, PLVals->PLValigned->sDepthCol,
                                   std::asin(arcVal) * Pi1d + s05f);
    } else {
        std::memcpy(DepC, PLV->PreDepthCol, sizeof(TSVec));
    }

    // Initialize result transparency = (1,1,1,0)
    ResultTransp[0] = 1; ResultTransp[1] = 1; ResultTransp[2] = 1; ResultTransp[3] = 0;

    if (PsiLight->Zpos < 32768) {
        // Object hit (not background)
        calcAmbshadow(dAmbSh, PLVals->sAmbShad, PsiLight);
        if (PLVals->bCalcPixColSqr) dAmbSh = dAmbSh * dAmbSh;
        dFog = PLVals->sDiffuseShadowing * (dAmbSh - 1.0f) + 1.0f;
        dRough = (PsiLight->RoughZposFine & 0xFF) * PLVals->sRoughnessFactor;

        MakeSVecFromNormalsD(LiLSDAI[0], PsiLight);
        ClearSVec(LiLSDAI[1]);
        ClearSVec(sLiSpe);
        ClearSVec(LiLSDAI[3]);

        ir = 0;
        while (ir < 6) {
            if (PLVals->iLightOption[ir] == 0) {
                // Light on
                bSubAmbSh = (PLVals->iHScalced[ir] ^ PLVals->iHSenabled[ir]) != 0;
                softHS = PLVals->iHSmask[ir] == -1;
                noHS = softHS || ((PsiLight->Shadow & PLVals->iHSmask[ir]) == 0) || (PLVals->iHScalced[ir] == 0);
                std::memcpy(LiLSDAI[5], PLVals->PLValigned->LN[ir], sizeof(TSVec));
                dTmp2 = 1.0f;
                if ((PLVals->iLightPos[ir] & 1) != 0) {
                    // Positional light
                    SubtractSVectors(LiLSDAI[5], LiLSDAI[5], PLV->ObjPos);
                    dTmp2 = SqrLengthOfSVec(LiLSDAI[5]);
                    if (dTmp2 > PLVals->sLmaxL[ir]) { ir++; continue; }
                    NormaliseSVectorVar(LiLSDAI[5]);
                    RotateSVectorReverseS(&LiLSDAI[5], PLV->PSmatrix);
                    dTmp2 = 1.0f / (dTmp2 + s1em30);
                }
                if (PLVals->bCalcPixColSqr)
                    dTmp = GetCosTabValSqr(PLVals->iLightFuncDiff[ir], DotOfSVectors(LiLSDAI[0], LiLSDAI[5]), dRough) * dTmp2;
                else
                    dTmp = GetCosTabVal(PLVals->iLightFuncDiff[ir], DotOfSVectors(LiLSDAI[0], LiLSDAI[5]), dRough) * dTmp2;
                AddSVecWeights(&LiLSDAI[3], &PLVals->PLValigned->sLCols[ir], dTmp);
                if (noHS) {
                    if (bSubAmbSh) dTmp *= dAmbSh; else dTmp *= dFog;
                    if (softHS) dTmp *= (PsiLight->Shadow >> 10) * s1d63;
                    AddSVecWeights(&LiLSDAI[1], &PLVals->PLValigned->sLCols[ir], dTmp);
                }
                // Specular calc
                dTmp = DotOf2VecNormalize(LiLSDAI[0], LiLSDAI[5], PLV->ViewVec);
                if (dTmp > 0) {
                    dTmp2 = (dTmp2 + std::min(1.0f, dRough * 2.0f) * (1.0f / PLVals->iLightPowFunc[ir] - dTmp2)) * PLVals->sSpec;
                    if (dTmp2 > 0) {
                        AddSVecWeights(&LiLSDAI[3], &PLVals->PLValigned->sLCols[ir], dTmp2 / PLVals->iLightPowFunc[ir]);
                        if (noHS) {
                            dTmp2 *= FastIntPow(dTmp, PLVals->iLightPowFunc[ir]);
                            if (bSubAmbSh) dTmp2 *= dAmbSh; else dTmp2 *= dFog;
                            if (softHS) dTmp2 *= (PsiLight->Shadow >> 10) * s1d63;
                            AddSVecWeights(&sLiSpe, &PLVals->PLValigned->sLCols[ir], dTmp2);
                        }
                    }
                }
            } else if (PLVals->iLightOption[ir] == 2 && PLVals->LLightMaps[ir] != nullptr) {
                TSVec lmPix, lmScaled;
                GetLightMapPixelSphere(lmPix, LiLSDAI[0], &PLVals->LLightMaps[ir]->PicRotMatrix,
                                       *PLVals->LLightMaps[ir], PLVals->bCalcPixColSqr);
                ScaleSVec(lmScaled, lmPix, 255.0f);
                const TSVec& lmAverage = PLVals->bCalcPixColSqr
                    ? PLVals->LLightMaps[ir]->LMavrgColSqr
                    : PLVals->LLightMaps[ir]->LMavrgCol;
                LinInterpolate2SVecs(LiLSDAI[4],
                    lmAverage,
                    lmScaled, dRough);
                AddSVecWeights(&LiLSDAI[3], &LiLSDAI[4], 1.0f);
                AddSVecWeights(&LiLSDAI[1], &LiLSDAI[4], dAmbSh);
            }
            ir++;
        }

        bSubAmbSh = PsiLight->SIgradient > 32767; // bIsInterior

        LiLSDAI[2][0] = PLVals->sColZmul * PLV->zPos;
        if (bSubAmbSh)
            CalcColorsInside(&LiLSDAI[2], &LiLSDAI[5], PsiLight, PLVals);
        else
            CalcColors(&LiLSDAI[2], &LiLSDAI[5], PsiLight, PLVals);

        if (PLVals->iColOnOT > 1 && PLVals->DiffColLightMap != nullptr) {
            if (PLVals->bYCcomb) {
                std::memcpy(LiLSDAI[4], LiLSDAI[2], sizeof(TSVec));
            }
            if (PLVals->iColOnOT > 3) {
                if (PLVals->iColOnOT > 5) {
                    GetDiffMapWrap3D(LiLSDAI[2], PLVals, &LiLSDAI[0], PLV);
                } else {
                    GetLightMapPixelSphere(LiLSDAI[2], LiLSDAI[0], &PLVals->DiffColLightMap->PicRotMatrix,
                                           *PLVals->DiffColLightMap, PLVals->bCalcPixColSqr);
                }
                TSVec avgScaled;
                ScaleSVec(avgScaled, PLVals->DiffColLightMap->LMavrgCol, dRough * s1d255);
                Add2SVecsWeight(LiLSDAI[2], LiLSDAI[2], avgScaled, 1.0f - dRough, 1.0f);
            } else {
                dTmp = (PsiLight->OTrap & 0x7FFF) * 3.05186851e-5f - 0.5f;
                dTmp2 = PsiLight->SIgradient * 3.05186851e-5f - 0.5f;
                GetLightMapPixel(
                    LiLSDAI[2],
                    std::fmod((PLVals->DCLMapRotCos * dTmp + PLVals->DCLMapRotSin * dTmp2 +
                               PLVals->DCLMapOffX) * PLVals->lvMapScale + 1.0f, 1.0f),
                    std::fmod((PLVals->DCLMapRotCos * dTmp2 - PLVals->DCLMapRotSin * dTmp +
                               PLVals->DCLMapOffY) * PLVals->lvMapScale + 1.0f, 1.0f),
                    PLVals->DiffColLightMap, PLVals->bCalcPixColSqr, 1);
            }
            if (PLVals->bYCcomb) {
                float denom = s001f + YofSVec_col(LiLSDAI[4]);
                ScaleSVec(LiLSDAI[2], LiLSDAI[4], YofSVec_col(LiLSDAI[2]) / denom);
            }
        }

        // Save DiffOut = object diffuse color for output
        std::memcpy(DiffOut, LiLSDAI[2], sizeof(TSVec));
        // InSD[0] = specular/reflection color
        std::memcpy(InSD[0], LiLSDAI[5], sizeof(TSVec));

        // bScaleAmbDiffDown: for inside transmission, reduce ambient+diffuse
        if (PLVals->bScaleAmbDiffDown) {
            dTmp = 1.0f - InSD[0][3] * PLVals->SRLightAmount;
            dAmbSh *= dTmp;
            dTmp3 = PLVals->sDiff * dTmp;
        } else {
            dTmp3 = PLVals->sDiff;
        }

        // Ambient light calculation
        if (PLVals->bUseSmallBGpicForAmb) {
            RotateSVectorS(&LiLSDAI[0], PLV->PSmatrix);
            if (PLVals->BGsmallLM != nullptr) {
                GetLightMapPixelSphere(LiLSDAI[4], LiLSDAI[0], &PLVals->BGsmallLM->PicRotMatrix,
                                       *PLVals->BGsmallLM, PLVals->bCalcPixColSqr);
                ScaleSVectorV(&LiLSDAI[4], 255.0f);
                dTmp = Max0S(1.0f - Max0S(1.0f - 28000.0f * PLVals->sDepth));
                if (PLVals->bFarFog) dTmp *= dTmp;
                if (PLVals->bAddBGlight) {
                    AddSVecWeights(&LiLSDAI[4], &DepC, dTmp);
                } else {
                    LinInterpolate2SVecs(LiLSDAI[4], LiLSDAI[4], DepC, 1.0f - dTmp);
                }
                ScaleSVectorV(&LiLSDAI[4], dAmbSh);
            } else {
                if (PLVals->bAmbRelObj)
                    LiLSDAI[0][1] = LiLSDAI[0][0] * (*PLV->PSmatrix)[0][1] + LiLSDAI[0][1] * (*PLV->PSmatrix)[1][1] + LiLSDAI[0][2] * (*PLV->PSmatrix)[2][1];
                dTmp2 = (LiLSDAI[0][1] * s05f + s05f) * dAmbSh;
                dTmp = dAmbSh - dTmp2;
                Add2SVecsWeightPacked(LiLSDAI[4], PLVals->PLValigned->sAmbCol, PLVals->PLValigned->sAmbCol2, dTmp, dTmp2);
            }
        } else {
            if (PLVals->bAmbRelObj)
                LiLSDAI[0][1] = LiLSDAI[0][0] * (*PLV->PSmatrix)[0][1] + LiLSDAI[0][1] * (*PLV->PSmatrix)[1][1] + LiLSDAI[0][2] * (*PLV->PSmatrix)[2][1];
            dTmp2 = (LiLSDAI[0][1] * s05f + s05f) * dAmbSh;
            dTmp = dAmbSh - dTmp2;
            Add2SVecsWeightPacked(LiLSDAI[4], PLVals->PLValigned->sAmbCol, PLVals->PLValigned->sAmbCol2, dTmp, dTmp2);
        }
        MultiplySVectorsV(&LiLSDAI[4], &LiLSDAI[2]); // amb light * diffuse color

        // Diff object color scaled
        ScaleSVec(LiLSDAI[0], LiLSDAI[2], dTmp3);

        // Total light = ambient + diffuse*lightcol + specular
        TSVec dL, sL;
        MultiplySVectors(dL, LiLSDAI[0], LiLSDAI[1]);
        MultiplySVectors(sL, LiLSDAI[5], sLiSpe);
        Add3SVectors(LiLSDAI[1], LiLSDAI[4], dL, sL);

        if (PLVals->iExModes == 0)
            CalcTotalLight1(&LiLSDAI, PLVals, &dAmbSh);
        else
            CalcTotalLight2(&LiLSDAI, PLVals, &dAmbSh);

        dTmp = Max0S(1.0f + (static_cast<int>(PsiLight->Zpos) - 28000) * PLVals->sDepth);
    } else {
        // Background
        if (PLVals->bBackBMP && PLVals->BGLightMap != nullptr) {
            if (PLVals->bDirectImageCoord) {
                GetLightMapPixel(LiLSDAI[4], PLV->xPos, PLV->yPos, PLVals->BGLightMap, PLVals->bCalcPixColSqr, 0);
                ScaleSVectorV(&LiLSDAI[4], 255.0f);
            } else {
                GetLightMapPixelSphere(LiLSDAI[4], PLV->AbsViewVec, &PLVals->BGLightMap->PicRotMatrix,
                                       *PLVals->BGLightMap, PLVals->bCalcPixColSqr);
                ScaleSVectorV(&LiLSDAI[4], 255.0f);
            }
        } else {
            std::memcpy(LiLSDAI[4], DepC, sizeof(TSVec));
        }
        dTmp = Max0S(1.0f - (60768.0f - static_cast<int>(PsiLight->Zpos)) * PLVals->sDepth);
    }

    // ---- Depth fog and dynamic fog: only when NOT inside transparent material ----
    if (PLVals->bDivOptions == 0) {
        // Standard depth fog (not inside transparent material)
        if (dTmp < 1.0f) {
            dTmp = 1.0f - dTmp;
            if (PLVals->bCalcPixColSqr) dTmp = dTmp * dTmp;
            if (PLVals->bFarFog) dTmp = dTmp * dTmp;
            dTmp = 1.0f - dTmp;
        }

        if ((PsiLight->Zpos < 32768) || (!PLVals->bBackBMP) || (!PLVals->bAddBGlight))
            ScaleSVectorV(&LiLSDAI[4], dTmp);

        if (PLVals->bVolLight) ir = ConvertVLight(PsiLight->Shadow); else ir = PsiLight->Shadow & 0x3FF;
        dFog = (ir - PLVals->sShad - PLVals->sShadZmul * PLVals->ZposDynFog) * PLVals->sShadGr;
        if ((PLVals->bDFogOptions & 2) != 0) dFog = Max0S(dFog);
        dTmp3 = std::min(1.0f, ir * PLVals->sDynFogMul) * dFog;
        Add2SVecsWeight2(LiLSDAI[4], LiLSDAI[4], DepC, Max0S(1.0f - dTmp));
        if ((PLVals->bDFogOptions & 1) != 0) {
            Clamp01Svar(dFog);
            Clamp01Svar(dTmp3);
            ScaleSVectorV(&LiLSDAI[4], 1.0f - dFog);
            ScaleSVectorV(&ResultTransp, 1.0f - dFog);
        }
        TSVec fogMix;
        Add2SVecsWeightPacked(fogMix, PLVals->PLValigned->sDynFogCol, PLVals->PLValigned->sDynFogCol2, dFog - dTmp3, dTmp3);
        AddSVectors_inplace(&LiLSDAI[4], fogMix);
        ScaleSVectorV(&ResultTransp, std::min(1.0f, dTmp));
    }

    // Visible light sources
    iL1 = PsiLight->Zpos;
    if (iL1 < 32768) iL1 = 32768 - iL1;
    dRough = 1.0f / iL1;

    for (ir = 0; ir < 6; ir++) {
        if ((PLVals->iLightPos[ir] & 14) != 0) {
            noHS = (PLVals->iLightPos[ir] & 1) != 0;
            if (noHS) {
                // Visible positional light
                TSVec camDiff;
                SubtractSVectors(camDiff, PLV->CamPos, PLVals->PLValigned->LN[ir]);
                dTmp3 = SqrDistSV(camDiff, PLV->AbsViewVec);
            } else {
                // Visible global light
                if (PsiLight->Zpos < 32768) continue;
                bSubAmbSh = false;
                dTmp3 = Max0S(1.0f - DotOfSVectors(PLVals->PLValigned->LN[ir], PLV->ViewVec));
            }

            LiLSDAI[2][0] = PLVals->sLmaxL[ir] * 1e-8f;
            if (dTmp3 < LiLSDAI[2][0]) {
                if (noHS) {
                    // Positional light: check if light is behind viewer
                    TSVec lightToCam;
                    SubtractSVectors(lightToCam, PLVals->PLValigned->LN[ir], PLV->CamPos);
                    if (DotOfSVectors(lightToCam, PLV->AbsViewVec) < 0) continue;
                    if (PsiLight->Zpos > 32767) {
                        bSubAmbSh = false;
                    } else {
                        LiLSDAI[2][1] = -std::sqrt(LiLSDAI[2][0] - dTmp3);
                        if (std::fabs(PLV->AbsViewVec[0]) > s05f)
                            bSubAmbSh = LiLSDAI[2][1] > (PLV->ObjPos[0] - PLVals->PLValigned->LN[ir][0]) / PLV->AbsViewVec[0];
                        else if (std::fabs(PLV->AbsViewVec[1]) > s05f)
                            bSubAmbSh = LiLSDAI[2][1] > (PLV->ObjPos[1] - PLVals->PLValigned->LN[ir][1]) / PLV->AbsViewVec[1];
                        else
                            bSubAmbSh = LiLSDAI[2][1] > (PLV->ObjPos[2] - PLVals->PLValigned->LN[ir][2]) / PLV->AbsViewVec[2];
                    }
                }
                if (bSubAmbSh) continue;

                CalcPosLightShape(dTmp3, LiLSDAI[2][0], ir, PLVals);

                if (PLVals->bDivOptions != 0) {
                    // Inside transparent material: downscale light by power of raylength
                    // using diffuse color of raystart as absorption
                    dTmp = 1.0f - Max0S(1.0f + (PLVals->sPosLP[ir] - 28000.0f) * PLVals->sDepth);
                    TSVec absorpPow;
                    SVecPow(absorpPow, InSD[1], dTmp * PLVals->sAbsorpCoeff);
                    TSVec scaledLight;
                    ScaleSVec(scaledLight, PLVals->PLValigned->sLCols[ir], dTmp3);
                    TSVec absorbedLight;
                    MultiplySVectors(absorbedLight, absorpPow, scaledLight);
                    AddSVectors_inplace(&LiLSDAI[4], absorbedLight);
                } else {
                    // Standard visible light handling (not inside transparent material)
                    LiLSDAI[2][2] = 1.0f - Max0S(1.0f + (PLVals->sPosLP[ir] - 28000.0f) * PLVals->sDepth);
                    if (LiLSDAI[2][2] > 0) {
                        if (PLVals->bCalcPixColSqr) LiLSDAI[2][2] = LiLSDAI[2][2] * LiLSDAI[2][2];
                        if (PLVals->bFarFog) LiLSDAI[2][2] = LiLSDAI[2][2] * LiLSDAI[2][2];
                    }
                    dTmp3 *= (1.0f - LiLSDAI[2][2] * 0.9f);

                    int posOpt = PLVals->iLightPos[ir] & 14;
                    if (posOpt == 2 || posOpt == 8) {
                        // vislight3: don't decrease light behind, just descale vislight
                        Add2SVecsWeight2(LiLSDAI[4], LiLSDAI[4], PLVals->PLValigned->sLCols[ir], dTmp3);
                    } else {
                        LiLSDAI[2][2] = Max0S(LiLSDAI[2][2]);
                        int iL1v;
                        if (PLVals->bVolLight) iL1v = ConvertVLight(PsiLight->Shadow); else iL1v = PsiLight->Shadow & 0x3FF;
                        dFog = (iL1v * (32768.0f - PLVals->sPosLP[ir]) * dRough - PLVals->sShad -
                               PLVals->sShadZmul * (PLVals->sPosLightZpos[ir] + PLVals->sZZstmitDif)) *
                               PLVals->sShadGr * (1.0f - LiLSDAI[2][0]) * (1.0f - LiLSDAI[2][2]);
                        dAmbSh = std::min(1.0f, iL1v * (32768.0f - PLVals->sPosLP[ir]) * dRough * PLVals->sDynFogMul) * dFog;
                        TSVec t1, t2, t3;
                        Add2SVecsWeight(t1, LiLSDAI[4], DepC, LiLSDAI[2][0], LiLSDAI[2][2] * (1.0f - LiLSDAI[2][0]));
                        Add2SVecsWeightPacked(t2, PLVals->PLValigned->sDynFogCol, PLVals->PLValigned->sDynFogCol2, dFog - dAmbSh, dAmbSh);
                        ScaleSVec(t3, PLVals->PLValigned->sLCols[ir], dTmp3);
                        Add3SVectors(LiLSDAI[4], t1, t2, t3);
                    }
                }
                ScaleSVectorV(&ResultTransp, std::min(1.0f, LiLSDAI[2][0])); // descale with hidden by visposlights
            }
        }
    }

    LiLSDAI[4][3] = 0;
    if (HasParityTraceContext() && IsPaintSvecTraceScopeEnabled() &&
        ShouldLogParityStage("paint_svec_trans_state")) {
        std::ostringstream payload;
        payload.setf(std::ios::fixed, std::ios::floatfield);
        payload << std::setprecision(12)
                << " dAmbSh=" << dAmbSh
                << " dFog=" << dFog
                << " dTmp=" << dTmp
                << " dTmp2=" << dTmp2
                << " dTmp3=" << dTmp3
                << " dRough=" << dRough
                << " DiffOutX=" << DiffOut[0]
                << " DiffOutY=" << DiffOut[1]
                << " DiffOutZ=" << DiffOut[2]
                << " Li4X=" << LiLSDAI[4][0]
                << " Li4Y=" << LiLSDAI[4][1]
                << " Li4Z=" << LiLSDAI[4][2]
                << " Li4W=" << LiLSDAI[4][3]
                << " Li2X=" << LiLSDAI[2][0]
                << " Li2Y=" << LiLSDAI[2][1]
                << " Li2Z=" << LiLSDAI[2][2]
                << " Li2W=" << LiLSDAI[2][3]
                << " SpeX=" << sLiSpe[0]
                << " SpeY=" << sLiSpe[1]
                << " SpeZ=" << sLiSpe[2]
                << " SpeW=" << sLiSpe[3];
        AppendParityTrace("paint_svec_trans_state", payload.str());
    }
    // Pascal: SVcol^ := mMinMaxSVec(0, 8, ScaleSVector(LiLSDAI[4], s1d255));
    TSVec scaledTmpT;
    ScaleSVec(scaledTmpT, LiLSDAI[4], s1d255);
    mMinMaxSVec(*SVcol, 0, 8, scaledTmpT);
    // Output diffuse color for absorption tracking
    std::memcpy(InSD[1], DiffOut, sizeof(TSVec));
    if (HasParityTraceContext() && IsPaintSvecTraceScopeEnabled() && ShouldLogParityStage("paint_svec_trans")) {
        std::ostringstream payload;
        payload.setf(std::ios::fixed, std::ios::floatfield);
        payload << std::setprecision(12)
                << " SVcolX=" << (*SVcol)[0]
                << " SVcolY=" << (*SVcol)[1]
                << " SVcolZ=" << (*SVcol)[2]
                << " ResultX=" << ResultTransp[0]
                << " ResultY=" << ResultTransp[1]
                << " ResultZ=" << ResultTransp[2]
                << " SD0X=" << InSD[0][0]
                << " SD0Y=" << InSD[0][1]
                << " SD0Z=" << InSD[0][2]
                << " SD0W=" << InSD[0][3]
                << " SD1X=" << InSD[1][0]
                << " SD1Y=" << InSD[1][1]
                << " SD1Z=" << InSD[1][2]
                << " SD1W=" << InSD[1][3];
        AppendParityTrace("paint_svec_trans", payload.str());
    }
}

// ============================================================================
// View/position helpers for paint loop
// ============================================================================

void CalcViewVec(TPaintLightVals* PLV, float sAspect) {
    // FPC/SSE2 evaluates these Single operands with subss/mulss, then
    // widens the result to the Double arguments of BuildViewVectorFOV.
    // Widening before multiplication changes the reflected ray by one ULP.
    double CX = static_cast<double>((PLV->xOff - PLV->xPos) * PLV->sFOVy * sAspect);
    double CY = static_cast<double>((PLV->yPos - 0.5f) * PLV->sFOVy);

    if (PLV->iPlanarOptic == 1) {
        PLV->ViewVec[0] = static_cast<float>(-CX);
        PLV->ViewVec[1] = static_cast<float>(CY);
        PLV->ViewVec[2] = PLV->PlOpticZ;
        PLV->ViewVec[3] = 0;
        NormaliseSVectorVar(PLV->ViewVec);
    } else if (PLV->iPlanarOptic == 2) {
        BuildViewVectorSphereFOVDouble(CY, CX, PLV->ViewVec);
    } else {
        BuildViewVectorFOVDouble(CY, CX, PLV->ViewVec);
    }
}

void GetStartSPosAndAddVecs(TPaintLightVals& PLV, TPaintParameter& PP,
                            TSVec& StartPos, TSVec& SPosYadd, TSVec& SPosXadd) {
    if (PP.ppPlanarOptic == 2) {
        ClearSVec(StartPos);
    } else {
        StartPos[0] = static_cast<float>(-0.5 * static_cast<double>(PP.ppWidth) * PP.StepWidth);
        StartPos[1] = static_cast<float>(-0.5 * static_cast<double>(PP.ppHeight) * PP.StepWidth);
    }
    StartPos[2] = static_cast<float>(PP.ZZstmitDif);
    StartPos[3] = 0;

    NormaliseMatrixToS(PP.m, 1.0f, PP.pVgrads);
    PLV.PSmatrix = &PP.m;
    PLV.iPlanarOptic = PP.ppPlanarOptic;
    PLV.PlOpticZ = PP.ppPlOpticZ;
    PLV.xOff = PP.ppXOff;
    PLV.sFOVy = PP.sFOVy;

    RotateSVectorS(&StartPos, &PP.m);

    // SPosYadd = m[1] * StepWidth
    SPosYadd[0] = PP.m[1][0] * static_cast<float>(PP.StepWidth);
    SPosYadd[1] = PP.m[1][1] * static_cast<float>(PP.StepWidth);
    SPosYadd[2] = PP.m[1][2] * static_cast<float>(PP.StepWidth);
    SPosYadd[3] = 0;

    SPosXadd[0] = PP.m[0][0] * static_cast<float>(PP.StepWidth);
    SPosXadd[1] = PP.m[0][1] * static_cast<float>(PP.StepWidth);
    SPosXadd[2] = PP.m[0][2] * static_cast<float>(PP.StepWidth);
    SPosXadd[3] = 0;

    float zmul_f = static_cast<float>(PP.ZcMul);
    if (std::fabs(PP.ZcMul) > 1e-20) {
        double ratio = 8388352.0 / PP.ZcMul;
        PP.BackDist = (std::pow(ratio + 1.0, 2.0) - 1.0) * PP.StepWidth / PP.Zcorr;
    } else {
        PP.BackDist = 1e10;
    }
}

void CalcObjPos(TPaintLightVals& PLV, TPaintParameter& PP,
                const TsiLight5* PsiLight, const TSVec& SPosX) {
    double z1;
    if (PsiLight->Zpos > 32767) {
        z1 = PP.BackDist;
    } else {
        // Decode Z position from packed data
        uint32_t packed = 0;
        std::memcpy(&packed, &PsiLight->RoughZposFine, 4);
        packed >>= 8;
        double zval = (8388352.0 - static_cast<double>(packed)) / PP.ZcMul + 1.0;
        z1 = (zval * zval - 1.0) * PP.StepWidth / PP.Zcorr;
    }

    std::memcpy(PLV.AbsViewVec, PLV.ViewVec, sizeof(TSVec));
    RotateSVectorS(&PLV.AbsViewVec, &PP.m);

    std::memcpy(PLV.CamPos, SPosX, sizeof(TSVec));
    PLV.zPos = static_cast<float>(z1 + PP.ZZstmitDif);
    PLV.ObjPos[0] = SPosX[0] + static_cast<float>(z1) * PLV.AbsViewVec[0];
    PLV.ObjPos[1] = SPosX[1] + static_cast<float>(z1) * PLV.AbsViewVec[1];
    PLV.ObjPos[2] = SPosX[2] + static_cast<float>(z1) * PLV.AbsViewVec[2];
    PLV.ObjPos[3] = 0;
}

static double DistanceFromViewVecToLightPosLocal(const TSVec& viewVec,
                                                const TSVec& lightPos,
                                                const TSVec& camPos) {
    TSVec diff;
    diff[0] = lightPos[0] - camPos[0];
    diff[1] = lightPos[1] - camPos[1];
    diff[2] = lightPos[2] - camPos[2];
    diff[3] = 0;
    // Preserve the Single stores in HeaderTrafos.DistanceFromViewVecToLightPos.
    TSVec cross{};
    cross[0] = diff[1] * (diff[2] - viewVec[2]) - diff[2] * (diff[1] - viewVec[1]);
    cross[1] = diff[2] * (diff[0] - viewVec[0]) - diff[0] * (diff[2] - viewVec[2]);
    cross[2] = diff[0] * (diff[1] - viewVec[1]) - diff[1] * (diff[0] - viewVec[0]);
    return std::sqrt(static_cast<double>(cross[0] * cross[0] + cross[1] * cross[1] + cross[2] * cross[2]));
}

static void CalcXYZposForLightLocal(TMandHeader10* Header, const TSVec& lightPos,
                                    float& x, float& y, float& z, bool bPos) {
    TPaintLightVals plv{};
    TPaintParameter pp{};
    TSVec startPos, sPosXadd, sPosYadd, sPosX;
    TMatrix3 paintVGrads;
    TMatrix3 mt;

    NormaliseMatrixTo(paintVGrads, Header->dStepWidth, Header->hVGrads);

    pp.ppWidth = Header->Width;
    pp.ppHeight = Header->Height;
    pp.ppYinc = 1;
    pp.pVgrads = &paintVGrads;
    pp.sFOVy = (Header->bPlanarOptic == 2) ? static_cast<float>(Pi) : static_cast<float>(Header->dFOVy * Pid180);
    pp.ppXOff = CalcXoff(Header);
    CalcPPZvals(*Header, pp.Zcorr, pp.ZcMul, pp.ZZstmitDif);
    pp.StepWidth = Header->dStepWidth;
    pp.ppPlanarOptic = Header->bPlanarOptic & 3;
    double d = std::min(1.5, std::max(static_cast<double>(0.01f), static_cast<double>(pp.sFOVy) * 0.5));
    pp.ppPlOpticZ = static_cast<float>(std::cos(d) * d / std::sin(d));
    GetStartSPosAndAddVecs(plv, pp, startPos, sPosYadd, sPosXadd);

    float aspect = static_cast<float>(pp.ppWidth) / std::max(1, pp.ppHeight);
    double dmul = static_cast<double>(0.001f) / pp.StepWidth;
    float px = pp.ppWidth * 0.5f;
    float py = pp.ppHeight * 0.5f;

    auto makePLVForXY = [&](double mx, double my) {
        for (int axis = 0; axis < 3; ++axis) {
            // Pascal calls AddSVecWeight twice, storing Single after each call.
            sPosX[axis] = static_cast<float>(startPos[axis] + sPosYadd[axis] * my);
            sPosX[axis] = static_cast<float>(sPosX[axis] + sPosXadd[axis] * mx);
        }
        sPosX[3] = 0;
        plv.yPos = static_cast<float>(my / std::max(1, pp.ppHeight));
        plv.xPos = static_cast<float>(mx / std::max(1, pp.ppWidth));
        CalcViewVec(&plv, aspect);
        std::memcpy(plv.AbsViewVec, plv.ViewVec, sizeof(TSVec));
        RotateSVectorS(&plv.AbsViewVec, &pp.m);
    };

    for (int i = 0; i < 21; i++) {
        makePLVForXY(px, py);
        double dist;
        double dx;
        double dy;
        if (bPos) {
            dist = DistanceFromViewVecToLightPosLocal(plv.AbsViewVec, lightPos, sPosX) * dmul;
            if (dist < 1e-4) break;
            makePLVForXY(px + 0.01f, py);
            dx = dist - DistanceFromViewVecToLightPosLocal(plv.AbsViewVec, lightPos, sPosX) * dmul;
            makePLVForXY(px, py + 0.01f);
            dy = dist - DistanceFromViewVecToLightPosLocal(plv.AbsViewVec, lightPos, sPosX) * dmul;
        } else {
            dist = ArcCosSafe(DotOfSVectors(plv.ViewVec, lightPos));
            if (dist < 1e-4) break;
            makePLVForXY(px + 0.01f, py);
            dx = dist - ArcCosSafe(DotOfSVectors(plv.ViewVec, lightPos));
            makePLVForXY(px, py + 0.01f);
            dy = dist - ArcCosSafe(DotOfSVectors(plv.ViewVec, lightPos));
        }
        double dd = std::sqrt(dx * dx + dy * dy) + 1e-16;
        if (dd * 2 > dist) break;
        double mul = bPos ? 4e-3 : 5e-3;
        px = static_cast<float>(px + (dx > 0 ? 1 : dx < 0 ? -1 : 0) * dist * mul / dd);
        py = static_cast<float>(py + (dy > 0 ? 1 : dy < 0 ? -1 : 0) * dist * mul / dd);
    }

    x = static_cast<float>(px / std::max(1, pp.ppWidth));
    y = static_cast<float>(py / std::max(1, pp.ppHeight));

    // Pascal uses the last sampled ray here, including the final y probe.
    if (std::fabs(plv.AbsViewVec[0]) > 0.5f) {
        z = (lightPos[0] - sPosX[0]) / plv.AbsViewVec[0];
    } else if (std::fabs(plv.AbsViewVec[1]) > 0.5f) {
        z = (lightPos[1] - sPosX[1]) / plv.AbsViewVec[1];
    } else {
        z = (lightPos[2] - sPosX[2]) / plv.AbsViewVec[2];
    }

    NormaliseMatrixTo(mt, 1.0, Header->hVGrads);
    TVec3D dv{
        lightPos[0] - mt[2][0] * (Header->dZstart - Header->dZmid),
        lightPos[1] - mt[2][1] * (Header->dZstart - Header->dZmid),
        lightPos[2] - mt[2][2] * (Header->dZstart - Header->dZmid)
    };
    double planeZ = dv[0] * mt[2][0] + dv[1] * mt[2][1] + dv[2] * mt[2][2];
    z = Max0S(static_cast<float>(std::fabs(planeZ) > std::fabs(z) ? planeZ : z));
}

static void SortLightsPascalLike(TLightVals* Lvals) {
    static constexpr int stab[4] = {0, 2, 1, 3};
    struct SortItem {
        int key;
        uint8_t src;
    };
    auto quickSortItems = [&](auto&& self, SortItem* items, int left, int right) -> void {
        int lpos = left - 1;
        int rpos = right;
        int pivot = items[right].key;
        for (;;) {
            do { ++lpos; } while (items[lpos].key < pivot);
            do { --rpos; } while ((rpos > lpos) && (items[rpos].key > pivot));
            if (lpos >= rpos) break;
            std::swap(items[lpos], items[rpos]);
        }
        std::swap(items[lpos], items[right]);
        if (lpos - 1 > left) self(self, items, left, lpos - 1);
        if (right > lpos + 1) self(self, items, lpos + 1, right);
    };

    SortItem order[6];
    for (int i = 0; i < 6; i++) {
        if ((Lvals->iLightPos[i] & 6) != 0) {
            order[i].key = static_cast<int>(std::lrint(Lvals->sPosLP[i]));
        } else {
            order[i].key = 40000 + stab[Lvals->iLightOption[i] & 3];
        }
        order[i].src = static_cast<uint8_t>(i);
    }
    quickSortItems(quickSortItems, order, 0, 5);

    int32_t tmpLightOption[6], tmpHSenabled[6], tmpHScalced[6], tmpHSmask[6];
    int32_t tmpLightPos[6], tmpLightAbs[6], tmpLightPowFunc[6], tmpLightFuncDiff[6];
    float tmpSLmaxL[6], tmpPosZ[6], tmpPosX[6], tmpPosY[6], tmpPosLP[6];
    TPLightMap tmpMaps[6];
    TSVec tmpLN[6], tmpSLCols[6];

    for (int i = 0; i < 6; i++) {
        tmpLightOption[i] = Lvals->iLightOption[i];
        tmpHSenabled[i] = Lvals->iHSenabled[i];
        tmpHScalced[i] = Lvals->iHScalced[i];
        tmpHSmask[i] = Lvals->iHSmask[i];
        tmpLightPos[i] = Lvals->iLightPos[i];
        tmpLightAbs[i] = Lvals->iLightAbs[i];
        tmpLightPowFunc[i] = Lvals->iLightPowFunc[i];
        tmpLightFuncDiff[i] = Lvals->iLightFuncDiff[i];
        tmpSLmaxL[i] = Lvals->sLmaxL[i];
        tmpPosZ[i] = Lvals->sPosLightZpos[i];
        tmpPosX[i] = Lvals->sPosLightXpos[i];
        tmpPosY[i] = Lvals->sPosLightYpos[i];
        tmpPosLP[i] = Lvals->sPosLP[i];
        tmpMaps[i] = Lvals->LLightMaps[i];
        std::memcpy(tmpLN[i], Lvals->PLValigned->LN[i], sizeof(TSVec));
        std::memcpy(tmpSLCols[i], Lvals->PLValigned->sLCols[i], sizeof(TSVec));
    }

    for (int i = 0; i < 6; i++) {
        int src = order[i].src;
        Lvals->SortTab[src] = static_cast<uint8_t>(i);
        Lvals->iLightOption[i] = tmpLightOption[src];
        Lvals->iHSenabled[i] = tmpHSenabled[src];
        Lvals->iHScalced[i] = tmpHScalced[src];
        Lvals->iHSmask[i] = tmpHSmask[src];
        Lvals->iLightPos[i] = tmpLightPos[src];
        Lvals->iLightAbs[i] = tmpLightAbs[src];
        Lvals->iLightPowFunc[i] = tmpLightPowFunc[src];
        Lvals->iLightFuncDiff[i] = tmpLightFuncDiff[src];
        Lvals->sLmaxL[i] = tmpSLmaxL[src];
        Lvals->sPosLightZpos[i] = tmpPosZ[src];
        Lvals->sPosLightXpos[i] = tmpPosX[src];
        Lvals->sPosLightYpos[i] = tmpPosY[src];
        Lvals->sPosLP[i] = tmpPosLP[src];
        Lvals->LLightMaps[i] = tmpMaps[src];
        std::memcpy(Lvals->PLValigned->LN[i], tmpLN[src], sizeof(TSVec));
        std::memcpy(Lvals->PLValigned->sLCols[i], tmpSLCols[src], sizeof(TSVec));
    }
    Lvals->SortTab[6] = 6;
    Lvals->SortTab[7] = 7;
}

// ============================================================================
// CompleteLightValsSetup
// ============================================================================

void CompleteLightValsSetup(TMandHeader10* Header, TPLightVals Lvals) {
    auto& HLight = Header->Light;
    const bool calcPixColSqr = (HLight.AdditionalOptions & 1) != 0;
    const float sLightScale = calcPixColSqr ? 1.5f : 1.0f;
    TSMatrix3 objectMatrix;
    NormaliseMatrixToS(objectMatrix, 1.0f, &Header->hVGrads);
    int reverseSort[6] = {0};
    for (int raw = 0; raw < 6; ++raw) {
        reverseSort[Lvals->SortTab[raw]] = raw;
    }

    float ambMul = (HLight.TBpos[5] & 0xFFF) / 90.0f;
    if (calcPixColSqr) {
        Lvals->PLValigned->sDynFogCol2[0] = HLight.DynFogCol2[0] * HLight.DynFogCol2[0] * s1d255;
        Lvals->PLValigned->sDynFogCol2[1] = HLight.DynFogCol2[1] * HLight.DynFogCol2[1] * s1d255;
        Lvals->PLValigned->sDynFogCol2[2] = HLight.DynFogCol2[2] * HLight.DynFogCol2[2] * s1d255;
        Lvals->PLValigned->sDepthCol2[0] = HLight.DepthCol2[0] * HLight.DepthCol2[0] * s1d255;
        Lvals->PLValigned->sDepthCol2[1] = HLight.DepthCol2[1] * HLight.DepthCol2[1] * s1d255;
        Lvals->PLValigned->sDepthCol2[2] = HLight.DepthCol2[2] * HLight.DepthCol2[2] * s1d255;
        ambMul = (ambMul * ambMul + ambMul) * 0.5f * sLightScale;
        Lvals->PLValigned->sAmbCol2[0] = HLight.AmbCol2[0] * HLight.AmbCol2[0] * s1d255 * ambMul;
        Lvals->PLValigned->sAmbCol2[1] = HLight.AmbCol2[1] * HLight.AmbCol2[1] * s1d255 * ambMul;
        Lvals->PLValigned->sAmbCol2[2] = HLight.AmbCol2[2] * HLight.AmbCol2[2] * s1d255 * ambMul;
    } else {
        Lvals->PLValigned->sDynFogCol2[0] = static_cast<float>(HLight.DynFogCol2[0]);
        Lvals->PLValigned->sDynFogCol2[1] = static_cast<float>(HLight.DynFogCol2[1]);
        Lvals->PLValigned->sDynFogCol2[2] = static_cast<float>(HLight.DynFogCol2[2]);
        Lvals->PLValigned->sDepthCol2[0] = static_cast<float>(HLight.DepthCol2[0]);
        Lvals->PLValigned->sDepthCol2[1] = static_cast<float>(HLight.DepthCol2[1]);
        Lvals->PLValigned->sDepthCol2[2] = static_cast<float>(HLight.DepthCol2[2]);
        Lvals->PLValigned->sAmbCol2[0] = static_cast<float>(HLight.AmbCol2[0]) * ambMul;
        Lvals->PLValigned->sAmbCol2[1] = static_cast<float>(HLight.AmbCol2[1]) * ambMul;
        Lvals->PLValigned->sAmbCol2[2] = static_cast<float>(HLight.AmbCol2[2]) * ambMul;
    }
    Lvals->PLValigned->sDynFogCol2[3] = 0;
    Lvals->PLValigned->sDepthCol2[3] = 0;
    Lvals->PLValigned->sAmbCol2[3] = 0;

    // DivOptions and other flags
    Lvals->bDivOptions = HLight.AdditionalOptions >> 4;
    // I10: bAddBGlight from AdditionalOptions bit3
    Lvals->bAddBGlight = (HLight.AdditionalOptions & 8) != 0;
    // I9: bDFogOptions from Lights[0].FreeByte
    Lvals->bDFogOptions = HLight.Lights[0].FreeByte & 3;
    if (Lvals->bDFogOptions == 3) Lvals->bDFogOptions = 1;
    // C10: bAmbRelObj correct bit (bit 29)
    Lvals->bAmbRelObj = (HLight.TBoptions & 0x20000000) != 0;
    Lvals->iColOnOT = (HLight.TBoptions >> 17) & 1;
    Lvals->bNoColIpol = (HLight.Lights[3].FreeByte & 1) != 0;
    Lvals->bYCcomb = (HLight.AdditionalOptions & 4) != 0;
    Lvals->bBackBMP = HLight.BGbmp[0] != 0;
    Lvals->bUseSmallBGpicForAmb = (HLight.AdditionalOptions & 32) != 0;
    Lvals->bDirectImageCoord = (static_cast<uint32_t>(HLight.TBoptions) & 0x8000u) != 0;
    Lvals->bScaleAmbDiffDown = 0;
    Lvals->iExModes = HLight.Lights[2].FreeByte;
    // I7: sDiffuseShadowing from header
    Lvals->sDiffuseShadowing = static_cast<float>(HLight.Lights[3].AdditionalByteEx * (1.0 / 256.0));
    // I8: sIndLightReflect computed from header.
    // Pascal stores TBpos[3..11]; this C++ array is remapped to 0..8,
    // so Pascal TBpos[11] corresponds to C++ TBpos[8].
    {
        int8_t tb8hi = static_cast<int8_t>((HLight.TBpos[8] >> 8) & 0xFF);
        double val = (tb8hi + 53) * 0.022;
        Lvals->sIndLightReflect = static_cast<float>(val * val);
        if (calcPixColSqr) {
            Lvals->sIndLightReflect *= 0.5f;
        }
    }
    Lvals->sObjLightDecreaser = 0;
    // NI3: Read from header
    Lvals->SRLightAmount = std::min(Header->SRamount, 100.0f);
    Lvals->ZposDynFog = 0;
    Lvals->sAbsorpCoeff = Header->sTransmissionAbsorption;

    // Shadow Z multiply
    if (HLight.TBpos[4] > 0) {
        Lvals->sShadZ = static_cast<float>(HLight.TBpos[4]) * 0.001f;
    }

    // DFunc for depth color interpolation
    int dfunc = (HLight.TBoptions >> 30) & 3;
    Lvals->iDfunc = dfunc;

    // C9: bCalcPixColSqr from AdditionalOptions bit0
    Lvals->bCalcPixColSqr = (HLight.AdditionalOptions & 1) != 0;
    Lvals->lvCalcPixelColor = Lvals->bCalcPixColSqr ?
        reinterpret_cast<void*>(CalcPixelColorSqr) :
        reinterpret_cast<void*>(CalcPixelColor2);

    // Position lights
    for (int i = 0; i < 6; i++) {
        Lvals->sPosLP[i] = 0;
        Lvals->sPosLightZpos[i] = 0;
        Lvals->sPosLightXpos[i] = 0;
        Lvals->sPosLightYpos[i] = 0;
        int rawIdx = reverseSort[i];
        if (Lvals->iLightOption[i] == 2) {
            Lvals->LLightMaps[i] = &CurrentRenderContext().lighting_maps.lights[i];
            if (!LoadLightMapNr(HLight.Lights[rawIdx].LightMapNr, Lvals->LLightMaps[i])) {
                Lvals->iLightOption[i] = 1;
                Lvals->LLightMaps[i] = nullptr;
            } else {
                BuildRotMatrixSLocal(
                    static_cast<float>(Pi - HLight.Lights[rawIdx].LXpos[0] * Pi * 0.0078125),
                    static_cast<float>(HLight.Lights[rawIdx].LYpos[0] * Pi * 0.0078125),
                    static_cast<float>(HLight.Lights[rawIdx].LZpos[0] * Pi * 0.0078125),
                    Lvals->LLightMaps[i]->PicRotMatrix);
                if (Lvals->iLightAbs[i] > 0) {
                    TSMatrix3 tmpMatrix;
                    std::memcpy(tmpMatrix, objectMatrix, sizeof(TSMatrix3));
                    MultiplySMatrix3Local(tmpMatrix, Lvals->LLightMaps[i]->PicRotMatrix);
                    std::memcpy(Lvals->LLightMaps[i]->PicRotMatrix, tmpMatrix, sizeof(TSMatrix3));
                }
                Lvals->LLightMaps[i]->sIntensity =
                    ShortFloatToSingle(*reinterpret_cast<const ShortFloat*>(&HLight.Lights[rawIdx].Lamp));
            }
        }
    }
    CalcPosLightsRelPos(Header, Lvals);
    Lvals->BGLightMap = nullptr;
    Lvals->BGsmallLM = nullptr;
    Lvals->DiffColLightMap = nullptr;

    // Init DiffColLightMap PicRotMatrix to identity (Pascal: BuildRotMatrixS(0,0,0,...) at line 1270)
    TSMatrix3 picRotMatrix{};
    BuildRotMatrixSLocal(0.0f, 0.0f, 0.0f, picRotMatrix);
    std::memcpy(CurrentRenderContext().lighting_maps.diffuse.PicRotMatrix, picRotMatrix, sizeof(picRotMatrix));

    int diffMapNr = GetDiffMapNrLocal(HLight);
    if (diffMapNr != 0) {
        Lvals->DiffColLightMap = &CurrentRenderContext().lighting_maps.diffuse;
        if (LoadLightMapNr(diffMapNr, Lvals->DiffColLightMap)) {
            Lvals->DCLMapOffX = (((HLight.TBpos[4] >> 12) & 0xFF) + 256) * (1.0f / 256.0f);  // Pascal TBpos[7]
            Lvals->DCLMapOffY = (((HLight.TBpos[4] >> 20) & 0xFF) + 256) * (1.0f / 256.0f);  // Pascal TBpos[7]
            Lvals->iColOnOT |= (((HLight.Lights[1].FreeByte & 3) + 1) << 1);
            if (Lvals->iColOnOT > 3) {
                Lvals->lvMapScale = std::pow(1.2f, HLight.Lights[2].AdditionalByteEx - 30.0f);
                BuildRotMatrixSLocal(
                    static_cast<float>(Pi - ((HLight.TBpos[4] >> 12) & 0xFF) * Pi * 0.0078125),  // Pascal TBpos[7]
                    static_cast<float>(((HLight.TBpos[4] >> 20) & 0xFF) * Pi * 0.0078125),        // Pascal TBpos[7]
                    static_cast<float>(((HLight.TBpos[5] >> 20) & 0xFF) * Pi * 0.0078125),        // Pascal TBpos[8]
                    Lvals->DiffColLightMap->PicRotMatrix);
                TSMatrix3 tmpMatrix;
                std::memcpy(tmpMatrix, objectMatrix, sizeof(TSMatrix3));
                MultiplySMatrix3Local(tmpMatrix, Lvals->DiffColLightMap->PicRotMatrix);
                std::memcpy(Lvals->DiffColLightMap->PicRotMatrix, tmpMatrix, sizeof(TSMatrix3));
            } else {
                float rot = static_cast<float>((((HLight.TBpos[5] >> 20) & 0xFF) - 128) * PiM2 / 256.0);  // Pascal TBpos[8]
                Lvals->DCLMapRotSin = std::sin(rot);
                Lvals->DCLMapRotCos = std::cos(rot);
                Lvals->lvMapScale = std::pow(1.05f, HLight.Lights[2].AdditionalByteEx - 30.0f);
            }
        } else {
            Lvals->DiffColLightMap = nullptr;
            Lvals->iColOnOT &= 1;
        }
    }

    if (Lvals->bBackBMP) {
        char bgName[25] = {};
        std::memcpy(bgName, HLight.BGbmp, 24);
        Lvals->BGLightMap = &CurrentRenderContext().lighting_maps.background;
        if (LoadBackgroundMapFile(bgName, Lvals->BGLightMap) && Lvals->BGLightMap->LMWidth > 0) {
            PrepareLightMap(Lvals->BGLightMap, (HLight.TBoptions & 0x80000) != 0,
                            (HLight.AdditionalOptions & 0x80) != 0,
                            (HLight.AdditionalOptions & 16) != 0);
            Lvals->BGsmallLM = &CurrentRenderContext().lighting_maps.small_background;
            std::memcpy(Lvals->BGsmallLM, Lvals->BGLightMap, sizeof(TLightMap));
            BuildRotMatrixSLocal(
                static_cast<float>(Pi - HLight.PicOffsetX * Pi * 0.0078125),
                static_cast<float>(HLight.PicOffsetY * Pi * 0.0078125),
                static_cast<float>(HLight.PicOffsetZ * Pi * 0.0078125),
                Lvals->BGLightMap->PicRotMatrix);
            std::memcpy(Lvals->BGsmallLM->PicRotMatrix, Lvals->BGLightMap->PicRotMatrix, sizeof(TSMatrix3));
            float intensity = std::pow(1.04f, HLight.Lights[4].AdditionalByteEx - 40.0f);
            Lvals->BGLightMap->sIntensity = intensity;
            Lvals->BGsmallLM->sIntensity = intensity;
        } else {
            Lvals->bBackBMP = 0;
            Lvals->BGLightMap = nullptr;
            Lvals->BGsmallLM = nullptr;
        }
    }

    for (int i = 0; i < 6; i++) {
        // The shape8 radius multiplier was already applied by
        // MakeLightValsFromHeaderLight before light sorting.
        if ((Lvals->iLightPos[i] & 14) != 0) {
            CalcXYZposForLightLocal(Header, Lvals->PLValigned->LN[i],
                                    Lvals->sPosLightXpos[i],
                                    Lvals->sPosLightYpos[i],
                                    Lvals->sPosLightZpos[i],
                                    (Lvals->iLightPos[i] & 1) != 0);
            if ((Lvals->iLightPos[i] & 1) == 0) {
                Lvals->sPosLP[i] = 0;
                Lvals->sPosLightZpos[i] = static_cast<float>(Header->dZend - Header->dZstart);
            } else {
                double zcorr = 0;
                double zcMul = 0;
                double zzst = 0;
                CalcPPZvals(*Header, zcorr, zcMul, zzst);
                double dTmp = 8388352.0 - zcMul *
                              (std::sqrt(Lvals->sPosLightZpos[i] / std::max(1e-30f, Lvals->sStepWidth) * zcorr + 1.0) - 1.0);
                Lvals->sPosLP[i] = Min0MaxCS(static_cast<float>(dTmp / 256.0), 32767.0f);
            }
        }
    }
}

} // namespace mb3d
