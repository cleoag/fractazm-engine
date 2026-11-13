#include "core/render_control.h"
#include "core/ambient_occlusion.h"
#include "core/ambient_direction_table.h"
#include "core/ray_marcher.h"
#include "core/math3d.h"
#include "core/packed_math.h"
#include "core/header_trafos.h"
#include "core/colorizer.h"
#include "core/parity_trace.h"
#include "core/pas_lcg.h"
#include "core/thread_pool.h"
#include <cmath>
#include <cstring>
#include <iomanip>
#include "core/sse_estimates.h"  // SSE2: _mm_rsqrt_ss, _mm_rcp_ss for HiQ AO Random/RT0 parity
#include <algorithm>
#include <cstdio>
#include <sstream>
#include <vector>
#include <cstdlib>
#include <thread>
#include <limits>
#include <mutex>
#include <stdexcept>

namespace mb3d {

static bool DebugPrintAoHiqStderr() {
    const char* spec = std::getenv("MB3D_DEBUG_PRINT_AOHIQ_STDERR");
    return spec && *spec && std::strcmp(spec, "0") != 0;
}

static bool DebugPrintAoPointStderr() {
    const char* spec = std::getenv("MB3D_DEBUG_PRINT_AO_POINT_STDERR");
    return spec && *spec && std::strcmp(spec, "0") != 0;
}

static bool DebugPrintAoPointRaysStderr() {
    const char* spec = std::getenv("MB3D_DEBUG_PRINT_AO_POINT_RAYS_STDERR");
    return spec && *spec && std::strcmp(spec, "0") != 0;
}

static bool DebugResetAoPointCarryPerRay() {
    const char* spec = std::getenv("MB3D_DEBUG_RESET_AO_POINT_CARRY_PER_RAY");
    return spec && *spec && std::strcmp(spec, "0") != 0;
}

static bool DebugSwapAoPointRotationOrder() {
    const char* spec = std::getenv("MB3D_DEBUG_SWAP_AO_POINT_ROT_ORDER");
    return spec && *spec && std::strcmp(spec, "0") != 0;
}

// ============================================================================
// Constants
// ============================================================================

// s05f moved to math3d.h
static constexpr float  s002f   = 0.02f;
static constexpr float  s03f    = 0.3f;
static constexpr float  s1d3f   = 1.0f / 3.0f;
static constexpr double d00002441406 = 0.0002441406;

static inline int PascalRoundToInt(double value) {
    return static_cast<int>(std::nearbyint(value));
}

static inline float CalcAOHiQSumPasLike(const int16_t* psi32) {
    float stSum = 0.0f;
    for (int i = 0; i < 32; ++i) {
        if (psi32[i] > -32768) {
            const double term =
                std::atan(static_cast<double>(psi32[i]) * d00002441406);
            stSum = static_cast<float>(static_cast<double>(stSum) + term);
        }
    }
    return stSum;
}

// Pascal creates one fresh worker per logical row lane at each wavelet level.
// Execute rows serially here, preserving each worker's independent RNG history.
static std::vector<uint32_t> InitAOLogicalLaneSeeds(int threadCount, int height) {
    std::vector<uint32_t> seeds(std::clamp(threadCount, 1, height));
    for (size_t lane = 0; lane < seeds.size(); ++lane)
        seeds[lane] = PasLcgInitFromThread(static_cast<int>(lane) + 1);
    return seeds;
}

static inline float CalcAOScaledSamplePasLike(int atValue, int zpValue, float scale) {
    return static_cast<float>(
        static_cast<double>(atValue - zpValue) * static_cast<double>(scale));
}

static inline int CalcAOHiQItPasLike(float st, float sit, float sZRT) {
    return PascalRoundToInt(static_cast<double>(st) * static_cast<double>(sit) /
                            (static_cast<double>(sZRT) +
                             std::fabs(static_cast<double>(st))));
}

static inline int CalcAOHiQT0ItPasLike(float st, float sit, float sZRT) {
    return PascalRoundToInt(static_cast<double>(st) * static_cast<double>(sit) /
                            (static_cast<double>(st) * static_cast<double>(st) +
                             static_cast<double>(sZRT)));
}

static void ResetThresholdAoBackgroundQuartetLanes(TsiLight5* siLight, int width, int height) {
    if (!siLight || width < 4 || height <= 0) return;

    // Pascal TAmbShadowCalc / TAmbShadowCalcT0 process image-space threshold AO in
    // fixed 4-pixel quartets. When any lane in a quartet is a surface pixel, the
    // background lanes in the same quartet still flow through the scalar/vector path
    // and end up clamped to zero AmbShadow instead of preserving the pre-AO 5000.
    // The scalar C++ port computes surface pixels one-by-one, so we need an explicit
    // quartet cleanup pass to keep replay/render semantics aligned with Pascal.
    for (int y = 0; y < height; ++y) {
        RenderPoll();
        TsiLight5* row = siLight + static_cast<size_t>(y) * width;
        for (int x = 0; x + 3 < width; x += 4) {
            RenderWork(4);
            bool quartetHasSurface = false;
            for (int lane = 0; lane < 4; ++lane) {
                if (row[x + lane].Zpos < 32768) {
                    quartetHasSurface = true;
                    break;
                }
            }
            if (!quartetHasSurface) continue;
            for (int lane = 0; lane < 4; ++lane) {
                if (row[x + lane].Zpos >= 32768) {
                    row[x + lane].AmbShadow = 0;
                }
            }
        }
    }
}

// MakeSVecFromNormals and NormaliseSVectorVar are in colorizer.h/cpp

// ============================================================================
// Helper: BuildRotMatrixS -- build single-precision 3x3 rotation matrix
// From Math3D.pas: BuildRotMatrixS(xa, ya, za, @RotW)
// Builds rotation matrix from Euler angles (single-precision columns)
// TSMatrix3 is float[3][4] -- 3 rows x 4 cols
// ============================================================================

static void BuildRotMatrixS(float xa, float ya, float za, TSMatrix3& m) {
    float sinX, cosX, sinY, cosY, sinZ, cosZ;
    sinX = std::sin(xa); cosX = std::cos(xa);
    sinY = std::sin(ya); cosY = std::cos(ya);
    sinZ = std::sin(za); cosZ = std::cos(za);

    // Row 0
    m[0][0] = cosY * cosZ;
    m[0][1] = -cosY * sinZ;
    m[0][2] = sinY;
    m[0][3] = 0.0f;
    // Row 1
    m[1][0] = sinX * sinY * cosZ + cosX * sinZ;
    m[1][1] = cosX * cosZ - sinX * sinY * sinZ;
    m[1][2] = -sinX * cosY;
    m[1][3] = 0.0f;
    // Row 2
    m[2][0] = sinX * sinZ - cosX * sinY * cosZ;
    m[2][1] = cosX * sinY * sinZ + sinX * cosZ;
    m[2][2] = cosX * cosY;
    m[2][3] = 0.0f;
}

// ============================================================================
// Helper: ScaleSVector -- scale TSVec by scalar, return new TSVec
// ============================================================================

static void ScaleSVector(TSVec& result, const float* src, float s) {
    result[0] = src[0] * s;
    result[1] = src[1] * s;
    result[2] = src[2] * s;
    result[3] = 0.0f;
}

// ============================================================================
// Helper: NormaliseMatrixToS -- normalize TMatrix3 to step width, return TSMatrix3
// Each row of VGrads is scaled so its length = StepWidth
// ============================================================================

static void NormaliseMatrixToS(float stepWidth, const TMatrix3& vgrads, TSMatrix3& result) {
    for (int i = 0; i < 3; i++) {
        double len = std::sqrt(vgrads[i][0]*vgrads[i][0] + vgrads[i][1]*vgrads[i][1] + vgrads[i][2]*vgrads[i][2]);
        double scale = (len > 1e-30) ? (stepWidth / len) : stepWidth;
        result[i][0] = static_cast<float>(vgrads[i][0] * scale);
        result[i][1] = static_cast<float>(vgrads[i][1] * scale);
        result[i][2] = static_cast<float>(vgrads[i][2] * scale);
        result[i][3] = 0.0f;
    }
}

// ============================================================================
// Helper: MakeRotQuatFromSNormals -- create quaternion from surface normal
// Rotation from (0,0,-1) to the given normal direction
// ============================================================================

static void MakeRotQuatFromSNormals(const TSVec& sv, TQuaternion& q) {
    // Ported directly from Math3D.pas MakeRotQuatFromSNormals.
    double sinA, cosA;
    SinCosD(ArcCosSafe(-static_cast<double>(sv[2])) * 0.5, sinA, cosA);
    double n = std::sqrt(static_cast<double>(sv[1]) * static_cast<double>(sv[1]) +
                         static_cast<double>(sv[0]) * static_cast<double>(sv[0]));
    if (n < 1e-25) {
        q[0] = 0.0;
        q[1] = 0.0;
    } else {
        sinA /= n;
        q[0] = -static_cast<double>(sv[1]) * sinA;
        q[1] =  static_cast<double>(sv[0]) * sinA;
    }
    q[2] = 0.0;
    q[3] = cosA;
}

// ============================================================================
// Helper: CreateSMatrixFromQuat -- quaternion to TSMatrix3
// ============================================================================

static void CreateSMatrixFromQuat(TSMatrix3& m, const TQuaternion& q) {
    double xx = q[0]*q[0], yy = q[1]*q[1], zz = q[2]*q[2];
    double xy = q[0]*q[1], xz = q[0]*q[2], yz = q[1]*q[2];
    double wx = q[3]*q[0], wy = q[3]*q[1], wz = q[3]*q[2];

    m[0][0] = static_cast<float>(1.0 - 2.0*(yy + zz));
    m[0][1] = static_cast<float>(2.0*(xy + wz));
    m[0][2] = static_cast<float>(2.0*(xz - wy));
    m[0][3] = 0.0f;

    m[1][0] = static_cast<float>(2.0*(xy - wz));
    m[1][1] = static_cast<float>(1.0 - 2.0*(xx + zz));
    m[1][2] = static_cast<float>(2.0*(yz + wx));
    m[1][3] = 0.0f;

    m[2][0] = static_cast<float>(2.0*(xz + wy));
    m[2][1] = static_cast<float>(2.0*(yz - wx));
    m[2][2] = static_cast<float>(1.0 - 2.0*(xx + yy));
    m[2][3] = 0.0f;
}

// ============================================================================
// Helper: RotateSVectorReverseS -- rotate TSVec by transpose of TSMatrix3
// ============================================================================

static void RotateSVectorReverseS(TSVec& v, const TSMatrix3& m) {
    float x = v[0], y = v[1], z = v[2];
    v[0] = x * m[0][0] + y * m[0][1] + z * m[0][2];
    v[1] = x * m[1][0] + y * m[1][1] + z * m[1][2];
    v[2] = x * m[2][0] + y * m[2][1] + z * m[2][2];
}

// ============================================================================
// Helper: RotateSVectorS -- rotate TSVec by TSMatrix3 (forward)
// ============================================================================

static void RotateSVectorS(TSVec& v, const TSMatrix3& m) {
    float x = v[0], y = v[1], z = v[2];
    v[0] = x * m[0][0] + y * m[1][0] + z * m[2][0];
    v[1] = x * m[0][1] + y * m[1][1] + z * m[2][1];
    v[2] = x * m[0][2] + y * m[1][2] + z * m[2][2];
}

// DotOfSVectors is in colorizer.h/cpp

// ============================================================================
// Helper: CopyAddSVecWeight
// dest = src + svec * weight
// ============================================================================

static void CopyAddSVecWeight(TVec3D& dest, const TVec3D& src, const TSVec& svec, double w) {
    dest[0] = src[0] + static_cast<double>(svec[0]) * w;
    dest[1] = src[1] + static_cast<double>(svec[1]) * w;
    dest[2] = src[2] + static_cast<double>(svec[2]) * w;
}

// Max0S is inline in math3d.h

// ============================================================================
// Helper: MinLengthToCutPlane
// Ported from CalcAmbShadowDE.pas
// ============================================================================

static double MinLengthToCutPlane(TMCTparameter* mcp, const TSVec& vec, const TVec3D& vPos) {
    double result = 1e31;
    if ((mcp->iCutOptions & 1) != 0 && std::fabs(vec[0]) > 1e-20) {
        double dTmp = (mcp->dCOX - vPos[0]) / static_cast<double>(vec[0]);
        if (dTmp > 0 && dTmp < result) result = dTmp;
    }
    if ((mcp->iCutOptions & 2) != 0 && std::fabs(vec[1]) > 1e-20) {
        double dTmp = (mcp->dCOY - vPos[1]) / static_cast<double>(vec[1]);
        if (dTmp > 0 && dTmp < result) result = dTmp;
    }
    if ((mcp->iCutOptions & 4) != 0 && std::fabs(vec[2]) > 1e-20) {
        double dTmp = (mcp->dCOZ - vPos[2]) / static_cast<double>(vec[2]);
        if (dTmp > 0 && dTmp < result) result = dTmp;
    }
    return result;
}

// ============================================================================
// Helper: AtCutPlane2
// ============================================================================

static bool AtCutPlane2(TMCTparameter* mcp, const TVec3D& vPos, double plusD) {
    if ((mcp->iCutOptions & 1) != 0) {
        if ((std::fabs(vPos[0] - mcp->dCOX) - plusD) < 0) return true;
    }
    if ((mcp->iCutOptions & 2) != 0) {
        if ((std::fabs(vPos[1] - mcp->dCOY) - plusD) < 0) return true;
    }
    if ((mcp->iCutOptions & 4) != 0) {
        if ((std::fabs(vPos[2] - mcp->dCOZ) - plusD) < 0) return true;
    }
    return false;
}

// ============================================================================
// Helper: mCopyVec
// ============================================================================

static inline void mCopyVec(TVec3D& dest, const TVec3D& src) {
    dest[0] = src[0];
    dest[1] = src[1];
    dest[2] = src[2];
}

// ============================================================================
// NOTE: The former GetRand()/AOThreadSeed() helpers were removed; Pascal's
// Random()/seed initialiser are now replaced by the shared mb3d::PasLcg API
// (see core/pas_lcg.h and patches/phase2-T07/T12). Per-sample LCG advances
// that were NOT swapped (shift+mask jitter after the initial seed) continue
// to use the Pascal-native `seed = 214013*seed + 2531011` recurrence inline.
// ============================================================================

void CalcAmbientOcclusionForPoint(
    TMCTparameter* mcp,
    TIteration3Dext* it3Dext,
    TsiLight5* psi,
    int quality,
    int x,
    int y)
{
    RenderPoll();
    if (!mcp || !it3Dext || !psi) return;

    psi->AmbShadow = 0;
    if (psi->Zpos >= 32768 || psi->SIgradient >= 32768) return;

    const float origMsDEstop = mcp->msDEstop;
    const LongBool origIsIFS = mcp->bIsIFS;
    TVec3D IC;
    mCopyVec(IC, reinterpret_cast<TVec3D&>(it3Dext->C1));

    it3Dext->CalcSIT = 0;

    TSMatrix3 RotV;
    TMatrix3 packedGrads;
    std::memcpy(packedGrads, mcp->Vgrads, sizeof(packedGrads));
    NormaliseMatrixToS(static_cast<float>(mcp->StepWidth), packedGrads, RotV);
    float sMaxD = mcp->DEAOmaxL * s05f *
                  std::sqrt(static_cast<float>(mcp->iMandHeight * mcp->iMandHeight +
                                               mcp->iMandWidth * mcp->iMandWidth));
    float sDT1mul = static_cast<float>(mcp->StepWidth) * s05f;
    mcp->bIsIFS = 0;

    double dither1;
    double dither2;
    if (mcp->AOdither > 0) {
        dither1 = static_cast<double>(y % (mcp->AOdither + 1)) * 0.5 / mcp->AOdither;
        dither2 = static_cast<double>(x % (mcp->AOdither + 1)) * 0.5 / mcp->AOdither;
    } else {
        dither1 = 0.25;
        dither2 = 0.0;
    }

    int rayCount = 0;
    TSVec rotM[33];
    TSMatrix3 RotW;
    float ABR;
    float DStepMul;
    float CorrectionWeight;
    double dMinADif;

    if (quality == 0) {
        rayCount = 3;
        ABR = 60.0f * static_cast<float>(Pid180);
        for (int i = 0; i < 3; i++) {
            if (mcp->AOdither > 0) {
                BuildRotMatrixS(0.0f,
                                static_cast<float>((dither1 + 0.5) * 50.0 * Pid180),
                                static_cast<float>((i + dither2) * PiM2) * s1d3f,
                                RotW);
            } else {
                BuildRotMatrixS(0.0f, 0.5f * ABR, i * static_cast<float>(PiM2) * s1d3f, RotW);
            }
            ScaleSVector(rotM[i], RotW[2], -1.0f);
        }
        DStepMul = 1.8f;
        dMinADif = -1.0;
        CorrectionWeight = s03f;
    } else {
        ABR = static_cast<float>(Pi) * s05f / (quality + 0.9f);
        if (dither1 < 0.1) {
            rayCount = 0;
        } else {
            rayCount = 1;
            BuildRotMatrixS(0.0f, 0.0f, 0.0f, RotW);
            ScaleSVector(rotM[0], RotW[2], -1.0f);
        }
        for (int row = 1; row <= quality; row++) {
            int count = std::lround(std::sin(dither1) * PiM2 / ABR);
            for (int col = 0; col < count; col++) {
                BuildRotMatrixS(0.0f,
                                ABR * static_cast<float>(row + dither1 - 0.25),
                                static_cast<float>((col + dither2) * PiM2 / count),
                                RotW);
                ScaleSVector(rotM[rayCount], RotW[2], -1.0f);
                rayCount++;
            }
        }
        DStepMul = 1.0f + std::sin(ABR);
        dMinADif = std::cos(ABR * 1.2f);
        CorrectionWeight = (quality == 1) ? 0.2f : 0.1666f;
    }

    if (mcp->bInsideRendering && !mcp->bIsIFS) {
        DStepMul *= s05f;
    }

    mcp->msDEstop = MinCS(mcp->DEstop * (1.0f + static_cast<float>(std::fabs(mcp->mZZ)) * mcp->mctDEstopFactor), 1e6f);
    float StepAO = mcp->msDEstop / mcp->DEstop;
    double MaxDist = static_cast<double>(sMaxD) * std::sqrt(StepAO);
    if (DebugPrintAoPointStderr() && HasParityTraceContext()) {
        std::fprintf(stderr,
                     "AO_POINT_BEGIN quality=%d mZZ=%.12f msDEstop=%.12f StepAO=%.12f MaxDist=%.12f Zpos=%u SIgradient=%u\n",
                     quality, mcp->mZZ, static_cast<double>(mcp->msDEstop), static_cast<double>(StepAO),
                     MaxDist, static_cast<unsigned>(psi->Zpos), static_cast<unsigned>(psi->SIgradient));
        std::fflush(stderr);
    }

    TSVec svNormal;
    MakeSVecFromNormalsD(svNormal, psi);
    NormaliseSVectorVar(svNormal);
    TQuaternion quat;
    MakeRotQuatFromSNormals(svNormal, quat);
    CreateSMatrixFromQuat(RotW, quat);

    if (mcp->bVaryDEstop) {
        if (!mcp->bInsideRendering || mcp->bIsIFS) {
            mcp->msDEstop = mcp->msDEstop / (DStepMul * DStepMul);
        }
    } else {
        mcp->msDEstop = mcp->DEstop / (DStepMul * DStepMul);
    }

    float DEmul = (mcp->bInsideRendering && !mcp->bIsIFS) ? 1.0f : std::sqrt(rayCount * s05f);
    float ABRcalc = 1.2f / static_cast<float>(ArcSinSafe(1.0 / DEmul));
    float MDd10 = 0.1f / (static_cast<float>(MaxDist) * DEmul);

    float minRA[33];
    float sAdd[33];
    TSVec finalRayDir[33]{};
    double lastDT1[33]{};
    double lastDT2[33]{};
    int exitKind[33]{};
    // Mirror of Pascal CalcAmbShadowDE.pas (T07): per-thread deterministic LCG
    // replacing the old Random-seeded init.
    uint32_t seed = mb3d::PasLcgInitFromThread(std::max(1, static_cast<int>(mcp->iThreadId)));

    for (int rayIdx = 0; rayIdx < rayCount; rayIdx++) {
        RenderPoll();
        if (DebugResetAoPointCarryPerRay()) {
            it3Dext->CalcSIT = 0;
            it3Dext->ItResultI = 0;
            it3Dext->SmoothItD = 0.0;
            it3Dext->Rout = 0.0;
            it3Dext->OTrap = 0.0;
        }
        TSVec SVec;
        std::memcpy(SVec, rotM[rayIdx], sizeof(TSVec));
        if (DebugSwapAoPointRotationOrder()) {
            RotateSVectorS(SVec, RotV);
            RotateSVectorReverseS(SVec, RotW);
        } else {
            RotateSVectorReverseS(SVec, RotW);
            RotateSVectorS(SVec, RotV);
        }
        std::memcpy(finalRayDir[rayIdx], SVec, sizeof(TSVec));

        double dT1 = StepAO * DStepMul;
        float sTmp = 1.0f;
        bool bEnd = false;
        bool bFirstStep = mcp->bMCTFirstStepRandom != 0;

        do {
            if (bFirstStep) {
                bFirstStep = false;
                // Pascal CalcAmbShadowDE.pas (T07): dT1 := dT1 * (PasLcgU01(lcgState) * 1.5 + s05)
                dT1 = dT1 * (mb3d::PasLcgU01(seed) * 1.5 + 0.5);
            } else if (dT1 > MaxDist) {
                dT1 = MaxDist;
                bEnd = true;
                exitKind[rayIdx] = 4;
            }

            CopyAddSVecWeight(reinterpret_cast<TVec3D&>(it3Dext->C1), IC, SVec, dT1);
            if (mcp->iCutOptions > 0 &&
                AtCutPlane2(mcp, reinterpret_cast<TVec3D&>(it3Dext->C1), dT1 * sDT1mul)) {
                lastDT1[rayIdx] = dT1;
                exitKind[rayIdx] = 1;
                break;
            }

            double dT2 = mcp->CalcDE(it3Dext, mcp);
            lastDT1[rayIdx] = dT1;
            lastDT2[rayIdx] = dT2;
            if (mcp->bInsideRendering && !mcp->bIsIFS) {
                if (dT2 < mcp->msDEstop) {
                    float ratio = static_cast<float>(dT1 / MaxDist);
                    sTmp = ratio * ratio;
                    sTmp = sTmp * sTmp;
                    exitKind[rayIdx] = 2;
                    break;
                }
                dT1 = dT1 + dT1 * DStepMul * 0.1 + mcp->msDEstop;
            } else {
                sTmp = MinCS(sTmp, static_cast<float>((dT2 - mcp->msDEstop + dT1 * MDd10) / dT1));
                if (sTmp < s002f) {
                    exitKind[rayIdx] = 3;
                    break;
                }
                dT1 = dT1 + MaxCD(dT2, dT1 * DStepMul);
            }
        } while (!bEnd);

        minRA[rayIdx] = Max0S(sTmp) * DEmul;
    }

    double dAmount = 0.0;
    if (mcp->bInsideRendering && !mcp->bIsIFS) {
        for (int i = 0; i < rayCount; i++) {
            dAmount += minRA[i];
        }
    } else {
        for (int i = 0; i < rayCount; i++) {
            sAdd[i] = 0.0f;
            if (minRA[i] < 1.0f) {
                float maxAdd = 1.0f - minRA[i];
                for (int j = 0; j < rayCount; j++) {
                    RenderWork();
                    if (j == i) continue;
                    double dot = DotOfSVectors(rotM[i], rotM[j]);
                    if (dot > dMinADif) {
                        float overlap = minRA[j] - static_cast<float>(ArcCosSafe(dot)) * ABRcalc + 1.0f;
                        if (overlap > 0.0f) {
                            sAdd[i] += MinCS(maxAdd, overlap) * CorrectionWeight;
                        }
                    }
                }
            }
        }
        for (int i = 0; i < rayCount; i++) {
            dAmount += MinCS(1.0f, sAdd[i] + minRA[i]);
        }
    }

    if (DebugPrintAoPointRaysStderr() && HasParityTraceContext()) {
        for (int i = 0; i < rayCount; ++i) {
            const double contrib = mcp->bInsideRendering && !mcp->bIsIFS
                ? static_cast<double>(minRA[i])
                : static_cast<double>(MinCS(1.0f, sAdd[i] + minRA[i]));
            std::fprintf(stderr,
                         "AO_RAY idx=%d dirX=%.12f dirY=%.12f dirZ=%.12f exit=%d lastDT1=%.12f lastDT2=%.12f minRA=%.12f sAdd=%.12f contrib=%.12f\n",
                         i,
                         static_cast<double>(finalRayDir[i][0]),
                         static_cast<double>(finalRayDir[i][1]),
                         static_cast<double>(finalRayDir[i][2]),
                         exitKind[i], lastDT1[i], lastDT2[i], static_cast<double>(minRA[i]),
                         static_cast<double>(sAdd[i]), contrib);
        }
        std::fflush(stderr);
    }

    if (rayCount > 0) {
        psi->AmbShadow = static_cast<uint16_t>(std::max(0, static_cast<int>(
            std::lround(16383.0 * (1.0 - dAmount / rayCount)))));
    }
    if (DebugPrintAoPointStderr() && HasParityTraceContext()) {
        std::fprintf(stderr,
                     "AO_POINT_FINISH quality=%d RayCount=%d dAmount=%.12f AmbShadow=%u\n",
                     quality, rayCount, dAmount, static_cast<unsigned>(psi->AmbShadow));
        std::fflush(stderr);
    }
    mCopyVec(reinterpret_cast<TVec3D&>(it3Dext->C1), IC);
    mcp->bIsIFS = origIsIFS;
    mcp->msDEstop = origMsDEstop;
}

// ============================================================================
// fastIntArcTan2 -- integer angle 0..31 from y, x
// Ported from AmbShadowCalcThreadN.pas
// ============================================================================

static int fastIntArcTan2(int y, int x) {
    if (x == 0)
        return (y >= 0 ? 1 : 0) * 16 + 8;
    if (y == 0)
        return (x >= 0 ? 1 : 0) * 16;
    if (y < 0) {
        if (x < 0) {
            if (x >= y) return 7 - (x * 4) / y;
            else return (y * 4) / x;
        } else {
            if (-y < x) return 15 + (y * 4) / x;
            else return 8 - (x * 4) / y;
        }
    } else {
        if (x >= 0) {
            if (x > y) return 16 + (y * 4) / x;
            else return 23 - (x * 4) / y;
        } else {
            if (y < -x) return 31 + (y * 4) / x;
            else return 24 - (x * 4) / y;
        }
    }
}


// ############################################################################
// TYPE 0: 15-bit threshold AO
// Ported from AmbShadowCalcThreadN.pas TAmbShadowCalc.Execute
//
// This is a screen-space AO that builds multi-level wavelet Z-maps
// and samples 32 angular directions to find occlusion angles.
// ############################################################################

// ----------------------------------------------------------------------------
// BuildATlevels -- build wavelet decomposition of Z buffer
// Ported from AmbShadowCalcThreadN.pas BuildATlevels
// Returns number of levels. PATlevel[1..result] are filled with Z data.
// ----------------------------------------------------------------------------

static int BuildATlevels(
    TsiLight5* psiLight, int MWidth, int MHeight,
    std::vector<std::vector<uint16_t>>& patLevels,
    float& corrMul, int& zSub)
{
    int numLevels = 1;
    int x = MWidth / 16;
    do {
        numLevels++;
        x >>= 1;
    } while (numLevels < 8 && x >= 4);

    // Allocate levels (1-indexed, so patLevels[0] unused; [1]..[numLevels+1] used)
    patLevels.resize(numLevels + 2);
    for (int i = 1; i <= numLevels + 1; i++) {
        RenderPoll();
        patLevels[i].resize(MWidth * MHeight, 0);
    }

    // Find Z range
    int za = 32767, zp = 0;
    for (int i = 0; i < MWidth * MHeight; i++) {
        RenderWork();
        uint16_t z = psiLight[i].Zpos;
        if (z < 32768) {
            if (z > zp) zp = z;
            if (z < za) za = z;
        }
    }
    if (zp < za) {
        zp = 32768;
        za = 0;
    } else {
        zp++;
    }
    float s1 = 128.0f / static_cast<float>(zp - za);
    int zaShifted = za << 8;
    zSub = zaShifted;
    corrMul = s1;

    // Build level 1: encode Z values
    for (int i = 0; i < MWidth * MHeight; i++) {
        RenderWork();
        if (psiLight[i].Zpos < 32768) {
            // Reconstruct packed Z: RoughZposFine(2 bytes) + Zpos(2 bytes)
            uint32_t packedZ;
            std::memcpy(&packedZ, &psiLight[i].RoughZposFine, 4);
            packedZ >>= 8;
            int val = PascalRoundToInt(static_cast<float>(static_cast<int>(packedZ) - zaShifted) * s1);
            patLevels[1][i] = static_cast<uint16_t>(std::max(0, std::min(65535, val)));
        } else {
            patLevels[1][i] = 0;
        }
    }

    // Median 3x3 filter to remove single-pixel noise
    for (int y = 1; y < MHeight - 1; y++) {
        RenderPoll();
        for (int xi = 1; xi < MWidth - 1; xi++) {
            RenderWork();
            int idx = y * MWidth + xi;
            uint16_t zpVal = patLevels[1][idx];
            int zaVal = 0;
            // Check 8 neighbours
            int offsets[] = {
                -MWidth - 1, -MWidth, -MWidth + 1,
                -1, 1,
                MWidth - 1, MWidth, MWidth + 1
            };
            bool allLess = true;
            for (int n = 0; n < 8; n++) {
                uint16_t nv = patLevels[1][idx + offsets[n]];
                if (nv >= zpVal) {
                    allLess = false;
                    break;
                }
                if (nv > zaVal) zaVal = nv;
            }
            if (allLess && zaVal > 0) {
                patLevels[1][idx] = static_cast<uint16_t>(zaVal + 1);
            }
        }
    }

    // Build subsequent levels by smoothing.
    // Match Pascal BuildATlevels: separate first/middle/last edge handling
    // and exact unsigned-word averaging semantics.
    int iStep = 1;
    for (int ATl = 2; ATl <= numLevels; ATl++) {
        RenderPoll();
        int iStep2 = iStep * 2;
        std::copy(patLevels[ATl - 1].begin(), patLevels[ATl - 1].end(), patLevels[ATl].begin());

        // Smooth horizontal into the Pascal temp buffer PATlevel[ATl + 1].
        // The subsequent vertical pass reads from that horizontally smoothed temp
        // image and writes the final level into PATlevel[ATl].
        for (int y = 0; y < MHeight; y++) {
            RenderPoll();
            int rowStart = y * MWidth;
            const auto* src = &patLevels[ATl - 1][rowStart];
            auto* dst = &patLevels[ATl + 1][rowStart];
            if (MWidth <= 0) continue;

            const uint32_t zp = static_cast<uint32_t>(src[0]) + 1u;

            const int firstCount = std::min(iStep, MWidth);
            for (int xi = 0; xi < firstCount; ++xi) {
                RenderWork();
                const uint32_t vC = static_cast<uint32_t>(src[xi]) + 1u;
                const uint32_t vR = static_cast<uint32_t>(src[std::min(MWidth - 1, xi + iStep)]);
                dst[xi] = static_cast<uint16_t>((vC + ((zp + vR) >> 1)) >> 1);
            }

            const int middleStart = iStep;
            const int middleEnd = std::max(iStep, MWidth - iStep);
            for (int xi = middleStart; xi < middleEnd; ++xi) {
                RenderWork();
                const uint32_t vC = static_cast<uint32_t>(src[xi]) + 1u;
                const uint32_t vL = static_cast<uint32_t>(src[xi - iStep]);
                const uint32_t vR = static_cast<uint32_t>(src[xi + iStep]);
                dst[xi] = static_cast<uint16_t>((vC + ((vL + vR + 1u) >> 1)) >> 1);
            }

            // The original right horizontal border omits the +1 used at the left.
            const uint32_t zpLast = static_cast<uint32_t>(src[MWidth - 1]);
            for (int xi = std::max(iStep, MWidth - iStep); xi < MWidth; ++xi) {
                RenderWork();
                const uint32_t vC = static_cast<uint32_t>(src[xi]) + 1u;
                const uint32_t vL = static_cast<uint32_t>(src[std::max(0, xi - iStep)]);
                dst[xi] = static_cast<uint16_t>((vC + ((vL + zpLast) >> 1)) >> 1);
            }
        }

        // Smooth vertical from PATlevel[ATl + 1] into PATlevel[ATl].
        for (int xi = 0; xi < MWidth; xi++) {
            RenderPoll();
            const uint32_t zp = static_cast<uint32_t>(patLevels[ATl + 1][xi]) + 1u;

            const int firstCount = std::min(iStep, MHeight);
            for (int y = 0; y < firstCount; ++y) {
                RenderWork();
                const uint32_t vC = static_cast<uint32_t>(patLevels[ATl + 1][y * MWidth + xi]) + 1u;
                const uint32_t vB = static_cast<uint32_t>(
                    patLevels[ATl + 1][std::min(MHeight - 1, y + iStep) * MWidth + xi]);
                if (xi < (MWidth & ~3)) {
                    patLevels[ATl][y * MWidth + xi] =
                        static_cast<uint16_t>((vC + ((zp + vB) >> 1)) >> 1);
                } else {
                    // Pascal's scalar trailing columns read a Word at byte +1
                    // from the lower sample. Preserve this defined packed-byte
                    // behavior independently of host alignment/endianness.
                    const size_t lower = static_cast<size_t>(y + iStep) * MWidth + xi;
                    if (lower + 1 >= patLevels[ATl + 1].size())
                        throw std::invalid_argument("Threshold AO scalar border exceeds the original wavelet buffer");
                    const uint32_t next = patLevels[ATl + 1][lower + 1];
                    const uint32_t unaligned = (vB >> 8) | ((next & 255u) << 8);
                    patLevels[ATl][y * MWidth + xi] =
                        static_cast<uint16_t>(((vC - 1u) + ((zp + unaligned) >> 1)) >> 1);
                }
            }

            const int middleStart = iStep;
            const int middleEnd = std::max(iStep, MHeight - iStep);
            for (int y = middleStart; y < middleEnd; ++y) {
                RenderWork();
                const uint32_t vC = static_cast<uint32_t>(patLevels[ATl + 1][y * MWidth + xi]) + 1u;
                const uint32_t vT = static_cast<uint32_t>(patLevels[ATl + 1][(y - iStep) * MWidth + xi]);
                const uint32_t vB = static_cast<uint32_t>(patLevels[ATl + 1][(y + iStep) * MWidth + xi]);
                patLevels[ATl][y * MWidth + xi] =
                    static_cast<uint16_t>((vC + ((vT + vB + 1u) >> 1)) >> 1);
            }

            const uint32_t zpLast = static_cast<uint32_t>(patLevels[ATl + 1][(MHeight - 1) * MWidth + xi]) + 1u;
            for (int y = std::max(iStep, MHeight - iStep); y < MHeight; ++y) {
                RenderWork();
                const uint32_t vC = static_cast<uint32_t>(patLevels[ATl + 1][y * MWidth + xi]) + 1u;
                const uint32_t vT = static_cast<uint32_t>(
                    patLevels[ATl + 1][std::max(0, y - iStep) * MWidth + xi]);
                patLevels[ATl][y * MWidth + xi] =
                    static_cast<uint16_t>((vC + ((vT + zpLast) >> 1)) >> 1);
            }
        }

        iStep *= 2;
    }

    return numLevels;
}

// ----------------------------------------------------------------------------
// CalcAO_Type0 -- 15-bit threshold SSAO
// Ported from AmbShadowCalcThreadN.pas TAmbShadowCalc.Execute
// ----------------------------------------------------------------------------

static void CalcAO_Type0(
    TsiLight5* siLight, int width, int height,
    const TMandHeader10* header)
{
    if (!header) return;

    // Build wavelet levels
    float corrMul = 1.0f;
    int zSub = 0;
    std::vector<std::vector<uint16_t>> patLevels;
    int numLevels = BuildATlevels(siLight, width, height, patLevels, corrMul, zSub);
    if (numLevels < 1) return;

    float threshold = header->sAmbShadowThreshold;
    if (threshold < 0.01f) threshold = 0.01f;

    float st2 = (static_cast<float>(std::pow(256.0 / 1.0 + 1.0, 2.0) - 1.0) * 0.001f) / (corrMul * 256.0f);
    // Pascal path:
    //   aZScaleFactor := (Sqr(256 / ZcMul + 1) - 1) / Zcorr
    //   st2 := aZScaleFactor / (aCorrMul * 256)
    double Zcorr_local, ZcMul_local, ZZstmitDif_local;
    TMandHeader10 headerCopy = *header;
    CalcPPZvals(headerCopy, Zcorr_local, ZcMul_local, ZZstmitDif_local);
    const double zScaleInner = 256.0 / ZcMul_local + 1.0;
    float aZScaleFactor = static_cast<float>(((zScaleInner * zScaleInner) - 1.0) / Zcorr_local);
    st2 = aZScaleFactor / (corrMul * 256.0f);

    float sZRT = threshold / st2;
    const double levelsSqrtSqrt = std::sqrt(std::sqrt(static_cast<double>(numLevels)));
    float sMul = static_cast<float>(1.35 * 32767.0 /
                 (Pi * 32.0 *
                  std::pow(std::atan(threshold * levelsSqrtSqrt), 0.8)));

    // Process each pixel
    for (int y = 0; y < height; y++) {
        RenderPoll();
        for (int x = 0; x < width; x++) {
            RenderWork();
            int pixIdx = y * width + x;
            TsiLight5* psi = &siLight[pixIdx];
            const bool tracePixel = HasParityTraceContext() &&
                                    x == GetParityTracePixelX() &&
                                    y == GetParityTracePixelY();

            if (psi->Zpos >= 32768) continue; // background

            // Get encoded Z value for this pixel
            uint32_t packedZ;
            std::memcpy(&packedZ, &psi->RoughZposFine, 4);
            packedZ >>= 8;
            int zp0 = PascalRoundToInt(static_cast<float>(static_cast<int>(packedZ) - zSub) * corrMul);
            if (tracePixel) {
                std::ostringstream payload;
                payload << std::fixed << std::setprecision(12)
                        << "numLevels=" << numLevels
                        << " corrMul=" << corrMul
                        << " zSub=" << zSub
                        << " threshold=" << threshold
                        << " st2=" << st2
                        << " sZRT=" << sZRT
                        << " sMul=" << sMul
                        << " zp0=" << zp0;
                AppendParityTrace("ao_type0_begin", payload.str());
            }

            // Angular max array: 32 directions + 4 wrap = 36
            float angMaxArr[36];
            for (int i = 0; i < 36; i++) angMaxArr[i] = -1e10f;

            int iMaxRad = 0;
            int iMinRad = 0;

            for (int ATl = 1; ATl <= numLevels; ATl++) {
                RenderPoll();
                int iStep = 1 << (ATl - 1);
                float RM = static_cast<float>(std::sqrt(static_cast<double>(iStep)) * 5.0);
                iMaxRad += 4 * iStep;
                int iMaxRadS = iMaxRad * iMaxRad;
                int iMinRadS = iMinRad * iMinRad;
                bool multiC = (PascalRoundToInt(RM / (iMinRad + 1)) > 0);

                float sZRTLev = static_cast<float>(sZRT * std::sqrt(std::sqrt(
                    static_cast<double>(numLevels) / ATl)));
                // Pascal selects SSE by the start of the fixed four-pixel group.
                const int quartetX = x & ~3;
                const bool vectorLevel = quartetX < width - 3 &&
                    quartetX >= iMaxRad && quartetX < width - iMaxRad - 3;

                int ya = -iMaxRad;
                int ya2 = std::numeric_limits<int>::max();
                if (ya + y < 0) {
                    ya2 = ya;
                    while (ya2 + y < 0) ya2 += iStep;
                    if (ya2 + y >= height) ya2 = height - y - 1;
                    ya = -y;
                    if (ya == ya2) ya2 = std::numeric_limits<int>::max();
                }
                int ye = (y + iMaxRad >= height) ? (height - y - 1) : iMaxRad;

                for (int y2 = ya; y2 <= ye; ) {
                    RenderPoll();
                    if (y2 > ya2) {
                        y2 = ya2;
                        ya2 = std::numeric_limits<int>::max();
                    }

                    int xa = -iMaxRad;
                    int xa2 = std::numeric_limits<int>::max();
                    if (xa + x < 0) {
                        xa2 = xa;
                        while (xa2 + x < 0) xa2 += iStep;
                        if (xa2 + x >= width) xa2 = width - x - 1;
                        xa = -x;
                        if (xa == xa2) xa2 = std::numeric_limits<int>::max();
                    }
                    int xe = (x + iMaxRad >= width) ? (width - x - 1) : iMaxRad;

                    for (int x2 = xa, xa2t = xa2; x2 <= xe; ) {
                        RenderWork();
                        if (x2 > xa2t) {
                            x2 = xa2t;
                            xa2t = std::numeric_limits<int>::max();
                        }
                        int RadS = y2 * y2 + x2 * x2;
                        if (RadS > iMinRadS && RadS <= iMaxRadS) {
                            int iAngC = fastIntArcTan2(y2, x2);
                            float R1d = vectorLevel
                                ? numeric::reciprocalSqrtEstimate(static_cast<float>(RadS))
                                : static_cast<float>(1.0 / std::sqrt(static_cast<double>(RadS)));
                            const uint16_t pat = patLevels[ATl][static_cast<size_t>((y + y2) * width + (x + x2))];
                            float stRaw = static_cast<float>(static_cast<int>(pat) - zp0) * R1d;
                            float st = stRaw;
                            if (st > sZRTLev) st = sZRTLev;
                            int iC = 0;
                            int startAng = iAngC;
                            if (multiC) {
                                iC = PascalRoundToInt(RM * R1d);
                                startAng = (iAngC - (iC >> 1)) & 31;
                            }
                            if (tracePixel) {
                                std::ostringstream payload;
                                payload << std::fixed << std::setprecision(12)
                                        << "ATl=" << ATl
                                        << " sample_x2=" << x2
                                        << " sample_y2=" << y2
                                        << " RadS=" << RadS
                                        << " pat=" << pat
                                        << " zp0=" << zp0
                                        << " iAngC=" << iAngC
                                        << " multiC=" << (multiC ? 1 : 0)
                                        << " iC=" << iC
                                        << " startAng=" << startAng
                                        << " endAng=" << ((startAng + iC) & 31)
                                        << " stRaw=" << stRaw
                                        << " stClamped=" << st;
                                AppendParityTrace("ao_type0_sample", payload.str());
                            }

                            if (multiC) {
                                for (int ii = 0; ii <= iC; ii++) {
                                    int idx = (startAng + ii) & 31;
                                    // Use idx < 32, but AngMaxArr has 36 slots for wrap
                                    if (angMaxArr[idx] < st) angMaxArr[idx] = st;
                                }
                            } else {
                                if (angMaxArr[iAngC] < st) angMaxArr[iAngC] = st;
                            }
                        }
                        int xt = x2;
                        x2 += iStep;
                        if (x2 > xe && xt < xe) x2 = xe;
                    }
                    int prevY = y2;
                    y2 += iStep;
                    if (y2 > ye && prevY < ye) y2 = ye;
                }

                if (tracePixel) {
                    std::ostringstream payload;
                    payload << std::fixed << std::setprecision(12)
                            << "ATl=" << ATl
                            << " iStep=" << iStep
                            << " iMaxRad=" << iMaxRad
                            << " iMinRad=" << iMinRad
                            << " sZRTLev=" << sZRTLev;
                    for (int i = 0; i < 32; ++i) {
                        payload << " p" << i << "=" << angMaxArr[i];
                    }
                    AppendParityTrace("ao_type0_level", payload.str());
                }

                iMinRad = iMaxRad;
            }

            // Wrap angles 32..35 back
            for (int i = 0; i < 4; i++) {
                if (angMaxArr[i + 32] > angMaxArr[i])
                    angMaxArr[i] = angMaxArr[i + 32];
            }

            // Sum up: ArcTan of max angles * scale
            float stSum = 0.0f;
            for (int i = 0; i < 32; i++) {
                if (angMaxArr[i] > -1e9f)
                    stSum = static_cast<float>(static_cast<double>(stSum) +
                        std::atan(static_cast<double>(angMaxArr[i] * st2)));
            }
            int ambVal = std::max(0, std::min(16383, PascalRoundToInt(stSum * sMul)));
            psi->AmbShadow = static_cast<uint16_t>(ambVal);
            if (tracePixel) {
                std::ostringstream payload;
                payload << std::fixed << std::setprecision(12)
                        << "stSum=" << stSum
                        << " AmbShadow=" << psi->AmbShadow;
                for (int i = 0; i < 32; ++i) {
                    payload << " p" << i << "=" << angMaxArr[i];
                }
                AppendParityTrace("ao_type0_finish", payload.str());
            }
        }
    }

    ResetThresholdAoBackgroundQuartetLanes(siLight, width, height);
}

// ----------------------------------------------------------------------------
// CalcAO_Type0_T0 -- 15-bit threshold SSAO, T0 variant
// Ported from AmbShadowCalcThreadN.pas TAmbShadowCalcT0.Execute
// ----------------------------------------------------------------------------

static void CalcAO_Type0_T0(
    TsiLight5* siLight, int width, int height,
    const TMandHeader10* header)
{
    if (!header) return;

    float corrMul = 1.0f;
    int zSub = 0;
    std::vector<std::vector<uint16_t>> patLevels;
    int numLevels = BuildATlevels(siLight, width, height, patLevels, corrMul, zSub);
    if (numLevels < 1) return;

    float threshold = header->sAmbShadowThreshold;
    if (threshold < 0.01f) threshold = 0.01f;

    double Zcorr_local, ZcMul_local, ZZstmitDif_local;
    TMandHeader10 headerCopy = *header;
    CalcPPZvals(headerCopy, Zcorr_local, ZcMul_local, ZZstmitDif_local);
    float aZScaleFactor = static_cast<float>((std::pow(256.0 / ZcMul_local + 1.0, 2.0) - 1.0) / Zcorr_local);
    float st2 = aZScaleFactor / (corrMul * 256.0f);

    float sZRT = std::max(0.01f, threshold * 2.0f / st2);
    const double levelsSqrtSqrt = std::sqrt(std::sqrt(static_cast<double>(numLevels)));
    float sMul = static_cast<float>(1.35 * 32767.0 /
                 (Pi * 32.0 *
                  std::pow(std::atan(threshold * 0.8 * levelsSqrtSqrt), 0.9)));

    for (int y = 0; y < height; y++) {
        RenderPoll();
        for (int x = 0; x < width; x++) {
            RenderWork();
            int pixIdx = y * width + x;
            TsiLight5* psi = &siLight[pixIdx];
            if (psi->Zpos >= 32768) continue;

            uint32_t packedZ;
            std::memcpy(&packedZ, &psi->RoughZposFine, 4);
            packedZ >>= 8;
            int zp0 = PascalRoundToInt(static_cast<float>(static_cast<int>(packedZ) - zSub) * corrMul);

            float angMaxArr[36];
            for (float& v : angMaxArr) v = -1e10f;

            int iMaxRad = 0;
            int iMinRad = 0;

            for (int ATl = 1; ATl <= numLevels; ATl++) {
                RenderPoll();
                int iStep = 1 << (ATl - 1);
                float RM = static_cast<float>(std::sqrt(static_cast<double>(iStep)) * 5.0);
                iMaxRad += 4 * iStep;
                int iMaxRadS = iMaxRad * iMaxRad;
                int iMinRadS = iMinRad * iMinRad;
                bool multiC = (PascalRoundToInt(RM / (iMinRad + 1)) > 0);

                float sZRTLev = static_cast<float>(sZRT * std::sqrt(std::sqrt(
                    static_cast<double>(numLevels) / ATl)));
                // Pascal selects SSE by the start of the fixed four-pixel group.
                const int quartetX = x & ~3;
                const bool vectorLevel = quartetX < width - 3 &&
                    quartetX >= iMaxRad && quartetX < width - iMaxRad - 3;

                int ya = -iMaxRad;
                int ya2 = std::numeric_limits<int>::max();
                if (ya + y < 0) {
                    ya2 = ya;
                    while (ya2 + y < 0) ya2 += iStep;
                    if (ya2 + y >= height) ya2 = height - y - 1;
                    ya = -y;
                    if (ya == ya2) ya2 = std::numeric_limits<int>::max();
                }
                int ye = (y + iMaxRad >= height) ? (height - y - 1) : iMaxRad;

                for (int y2 = ya; y2 <= ye; ) {
                    RenderPoll();
                    if (y2 > ya2) {
                        y2 = ya2;
                        ya2 = std::numeric_limits<int>::max();
                    }

                    int xa = -iMaxRad;
                    int xa2 = std::numeric_limits<int>::max();
                    if (xa + x < 0) {
                        xa2 = xa;
                        while (xa2 + x < 0) xa2 += iStep;
                        if (xa2 + x >= width) xa2 = width - x - 1;
                        xa = -x;
                        if (xa == xa2) xa2 = std::numeric_limits<int>::max();
                    }
                    int xe = (x + iMaxRad >= width) ? (width - x - 1) : iMaxRad;

                    for (int x2 = xa, xa2t = xa2; x2 <= xe; ) {
                        RenderWork();
                        if (x2 > xa2t) {
                            x2 = xa2t;
                            xa2t = std::numeric_limits<int>::max();
                        }
                        int RadS = y2 * y2 + x2 * x2;
                        if (RadS > iMinRadS && RadS <= iMaxRadS) {
                            int iAngC = fastIntArcTan2(y2, x2);
                            float R1d = vectorLevel
                                ? numeric::reciprocalSqrtEstimate(static_cast<float>(RadS))
                                : static_cast<float>(1.0 / std::sqrt(static_cast<double>(RadS)));
                            float st = static_cast<float>(patLevels[ATl][static_cast<size_t>((y + y2) * width + (x + x2))] - zp0) * R1d;
                            if (vectorLevel || st < sZRTLev) {
                                if (vectorLevel) {
                                    st = std::min(st, sZRTLev);
                                    const float invThreshold = numeric::reciprocalEstimate(sZRTLev);
                                    const float t = invThreshold * st;
                                    const float cube = (t * t) * t;
                                    st = st - cube * st;
                                } else {
                                    const float t = st / sZRTLev;
                                    st = st * (1.0f - t * t * t);
                                }
                                if (multiC) {
                                    int iC = PascalRoundToInt(RM * R1d);
                                    int startAng = (iAngC - (iC >> 1)) & 31;
                                    for (int ii = 0; ii <= iC; ii++) {
                                        int idx = (startAng + ii) & 31;
                                        if (angMaxArr[idx] < st) angMaxArr[idx] = st;
                                    }
                                } else {
                                    if (angMaxArr[iAngC] < st) angMaxArr[iAngC] = st;
                                }
                            }
                        }

                        int prevX = x2;
                        x2 += iStep;
                        if (x2 > xe && prevX < xe) x2 = xe;
                    }

                    int prevY = y2;
                    y2 += iStep;
                    if (y2 > ye && prevY < ye) y2 = ye;
                }

                iMinRad = iMaxRad;
            }

            for (int i = 0; i < 4; i++) {
                if (angMaxArr[i + 32] > angMaxArr[i]) angMaxArr[i] = angMaxArr[i + 32];
            }

            float stSum = 0.0f;
            for (int i = 0; i < 32; i++) {
                if (angMaxArr[i] > -1e9f) stSum = static_cast<float>(static_cast<double>(stSum) +
                        std::atan(static_cast<double>(angMaxArr[i] * st2)));
            }
            int ambVal = std::max(0, std::min(16383, PascalRoundToInt(stSum * sMul)));
            psi->AmbShadow = static_cast<uint16_t>(ambVal);
        }
    }

    ResetThresholdAoBackgroundQuartetLanes(siLight, width, height);
}


// ############################################################################
// TYPE 1 & 2: 24-bit SSAO (threshold and random variants)
// Ported from AmbHiQ.pas
// ############################################################################

// Direction step table: 32 directions around a circle
// Initialized like IniATdirSteps
static const detail::AmbientDirections& EnsureATdirSteps() {
    static detail::AmbientDirectionTables tables;
    return tables.get(CurrentPrecisionMode());
}

// ----------------------------------------------------------------------------
// FirstATlevelHiQ -- init Z buffer for 24-bit SSAO
// Ported from AmbHiQ.pas
// ----------------------------------------------------------------------------

static void FirstATlevelHiQ(std::vector<uint32_t>& atLevel, TsiLight5* psiLight, int len) {
    for (int i = 0; i < len; i++) {
        RenderWork();
        if (psiLight[i].Zpos < 32768) {
            uint32_t packedZ;
            std::memcpy(&packedZ, &psiLight[i].RoughZposFine, 4);
            atLevel[i] = (packedZ & 0xFFFFFF00u) >> 1;
        } else {
            atLevel[i] = 0;
        }
    }
}

// ----------------------------------------------------------------------------
// NextATlevelHiQ -- smooth Z buffer for next wavelet level
// Ported from AmbHiQ.pas
// ----------------------------------------------------------------------------

static void NextATlevelHiQ(std::vector<uint32_t>& atLevel, int wid, int hei, int step) {
    std::vector<uint32_t> SA(std::max(wid, hei));

    // Smooth horizontal
    for (int y = 0; y < hei; y++) {
        RenderPoll();
        int rowStart = y * wid;
        std::copy(atLevel.begin() + rowStart, atLevel.begin() + rowStart + wid, SA.begin());
        for (int xi = 0; xi < wid; xi++) {
            RenderWork();
            int a = std::max(0, xi - step);
            int b = std::min(wid - 1, xi + step);
            atLevel[rowStart + xi] = (atLevel[rowStart + xi] + (SA[a] + SA[b]) / 2) / 2;
        }
    }

    // Smooth vertical
    for (int xi = 0; xi < wid; xi++) {
        RenderPoll();
        for (int y = 0; y < hei; y++) {
            RenderWork();
            SA[y] = atLevel[y * wid + xi];
        }
        for (int y = 0; y < hei; y++) {
            RenderWork();
            int a = std::max(0, y - step);
            int b = std::min(hei - 1, y + step);
            atLevel[y * wid + xi] = (SA[y] + (SA[a] + SA[b]) / 2) / 2;
        }
    }
}

// ----------------------------------------------------------------------------
// CalcAO_HiQ -- 24-bit HiQ AO
// Ported from AmbHiQ.pas TAmbHiQCalc.Execute (scalar path)
// Uses st*sit / (sZRT + abs(st)) formula.
// ----------------------------------------------------------------------------

static void CalcAO_HiQ(
    TsiLight5* siLight, int width, int height,
    const TMandHeader10* header)
{
    const auto& ATdirSteps=EnsureATdirSteps();
    if (HasParityTraceContext()) {
        std::ostringstream payload;
        payload << "width=" << width
                << " height=" << height
                << " traceX=" << GetParityTracePixelX()
                << " traceY=" << GetParityTracePixelY();
        AppendParityTrace("ao_hiq_enter", payload.str());
    }

    double Zcorr_local, ZcMul_local, ZZstmitDif_local;
    TMandHeader10 headerCopy = *header;
    CalcPPZvals(headerCopy, Zcorr_local, ZcMul_local, ZZstmitDif_local);

    float aZScaleFactor = static_cast<float>((std::pow(256.0 / ZcMul_local + 1.0, 2.0) - 1.0) / Zcorr_local);
    float threshold = header->sAmbShadowThreshold;
    if (threshold < 0.01f) threshold = 0.01f;
    float borderMirror = MinCS(header->bSSAO24BorderMirrorSize * 0.01f, 0.9f);

    int ymin = PascalRoundToInt(std::sqrt(static_cast<float>(width * width + height * height)) * 0.5f);
    int numLevels = 1;
    int tmpX = 5;
    do {
        numLevels++;
        tmpX <<= 1;
    } while (numLevels < 15 && tmpX <= ymin);

    std::vector<uint32_t> atLevel(width * height);
    std::vector<int16_t> angMaxArr(static_cast<size_t>(width) * height * 32, -32768);

    int MH2 = 2 * (height - 1);
    int MW2 = 2 * (width - 1);
    int WLo = PascalRoundToInt(static_cast<float>(width) * borderMirror);
    int WHi = width - 1 - WLo;
    int HLo = PascalRoundToInt(static_cast<float>(height) * borderMirror);
    int HHi = height - 1 - HLo;

    FirstATlevelHiQ(atLevel, siLight, width * height);

    for (int currentLevel = 1; currentLevel <= numLevels; currentLevel++) {
        RenderPoll();
        bool bSummUp = (currentLevel == numLevels);

        float sit = aZScaleFactor / 22000.0f * 4096.0f;
        const double levelFactor =
            std::sqrt(std::sqrt(static_cast<double>(numLevels) / static_cast<double>(currentLevel)));
        float sZRT = static_cast<float>(
            static_cast<double>(threshold) * 0.7 * 4096.0 / static_cast<double>(sit) * levelFactor);
        const double maxAngle =
            std::atan(static_cast<double>(threshold) * 0.6 *
                      std::sqrt(std::sqrt(static_cast<double>(numLevels))));
        float sMul = static_cast<float>(
            1.5 * 32767.0 / (static_cast<double>(Pi) * 32.0 * std::pow(maxAngle, 0.9)));

        int iStep = 1 << (currentLevel - 1);
        float sMinRad = (iStep < 2) ? 1.0f : 3.25f * iStep;
        int stepCount = (iStep < 2) ? 5 : 3;
        int iMaxRad = PascalRoundToInt(static_cast<double>(sMinRad) +
                                       static_cast<double>(iStep * stepCount) + 0.1);
        float RMA[6];
        for (int i = 0; i < stepCount; i++) {
            RMA[i] = static_cast<float>(
                1.0 / (static_cast<double>(sMinRad) +
                       static_cast<double>(i * iStep) + 0.1));
        }
        sit = sit * sZRT;

        for (int y = 0; y < height; y++) {
            RenderPoll();
            for (int x = 0; x < width;) {
                RenderPoll();
                int pixIdx = y * width + x;
                TsiLight5* psi = &siLight[pixIdx];

                if (!bSummUp && x >= iMaxRad && x < width - iMaxRad - 3) {
                    RenderWork(4);
                    uint32_t raw0, raw1, raw2, raw3;
                    std::memcpy(&raw0, &siLight[pixIdx + 0].RoughZposFine, 4);
                    std::memcpy(&raw1, &siLight[pixIdx + 1].RoughZposFine, 4);
                    std::memcpy(&raw2, &siLight[pixIdx + 2].RoughZposFine, 4);
                    std::memcpy(&raw3, &siLight[pixIdx + 3].RoughZposFine, 4);
                    const bool allBackground =
                        ((raw0 & raw1 & raw2 & raw3 & 0x80000000u) != 0);
                    if (!allBackground) {
                        const int zp[4] = {
                            static_cast<int>((raw0 & 0xFFFFFF00u) >> 1),
                            static_cast<int>((raw1 & 0xFFFFFF00u) >> 1),
                            static_cast<int>((raw2 & 0xFFFFFF00u) >> 1),
                            static_cast<int>((raw3 & 0xFFFFFF00u) >> 1),
                        };
                        const bool traceQuartet =
                            HasParityTraceContext() &&
                            y == GetParityTracePixelY() &&
                            GetParityTracePixelX() >= x &&
                            GetParityTracePixelX() < x + 4;
                        const int traceLane = traceQuartet ? (GetParityTracePixelX() - x) : -1;
                        int16_t* psi2 = &angMaxArr[static_cast<size_t>(pixIdx) * 32];
                        for (int iDir = 0; iDir < 32; ++iDir) {
                            float sx = x + ATdirSteps[iDir][0] * sMinRad;
                            float sy = y + ATdirSteps[iDir][1] * sMinRad;
                            for (int sc = 0; sc < stepCount; ++sc) {
                                RenderWork(4);
                                const int x2 = PascalRoundToInt(sx);
                                int y2 = PascalRoundToInt(sy);
                                if (y2 < 0) {
                                    y2 = -y2;
                                    if (y2 >= HLo) break;
                                } else if (y2 >= height) {
                                    y2 = MH2 - y2;
                                    if (y2 < HHi) break;
                                }

                                int16_t* lanePsm = psi2 + iDir;
                                const int base = y2 * width + x2;
                                for (int lane = 0; lane < 4; ++lane) {
                                    float st = CalcAOScaledSamplePasLike(
                                        static_cast<int>(atLevel[base + lane]), zp[lane], RMA[sc]);
                                    int it = CalcAOHiQItPasLike(st, sit, sZRT);
                                    if (traceQuartet && lane == traceLane && iDir == 1 &&
                                        DebugPrintAoHiqStderr()) {
                                        std::fprintf(stderr,
                                                     "AOHIQ_QUARTET currentLevel=%d lane=%d iDir=%d sc=%d x2=%d y2=%d at=%u zp=%d rma=%.12f st=%.12f prev=%d it=%d\n",
                                                     currentLevel, lane, iDir, sc, x2 + lane, y2,
                                                     atLevel[base + lane], zp[lane],
                                                     static_cast<double>(RMA[sc]), st,
                                                     static_cast<int>(*lanePsm), it);
                                    }
                                    if (*lanePsm < it) {
                                        *lanePsm = static_cast<int16_t>(std::min(32767, it));
                                    }
                                    lanePsm += 32;
                                }

                                sx += ATdirSteps[iDir][0] * iStep;
                                sy += ATdirSteps[iDir][1] * iStep;
                            }
                        }
                    }
                    x += 4;
                    continue;
                }

                RenderWork();
                uint32_t rawCheck;
                std::memcpy(&rawCheck, &psi->RoughZposFine, 4);
                if ((rawCheck & 0x80000000u) != 0) {
                    ++x;
                    continue;
                }

                int zp0 = static_cast<int>((rawCheck & 0xFFFFFF00u) >> 1);
                int16_t* psm = &angMaxArr[static_cast<size_t>(pixIdx) * 32];
                int16_t* psi2 = psm;
                const bool tracePixel = HasParityTraceContext() &&
                                        x == GetParityTracePixelX() &&
                                        y == GetParityTracePixelY();
                if (tracePixel) {
                    std::ostringstream payload;
                    payload << std::fixed << std::setprecision(12)
                            << "currentLevel=" << currentLevel
                            << " bSummUp=" << (bSummUp ? 1 : 0)
                            << " zp0=" << zp0
                            << " sit=" << sit
                            << " sZRT=" << sZRT
                            << " sMul=" << sMul
                            << " iStep=" << iStep
                            << " sMinRad=" << sMinRad
                            << " stepCount=" << stepCount
                            << " WLo=" << WLo
                            << " WHi=" << WHi
                            << " HLo=" << HLo
                            << " HHi=" << HHi;
                    AppendParityTrace("ao_hiq_begin", payload.str());
                }

                for (int iDir = 0; iDir < 32; iDir++) {
                    float sx = x + ATdirSteps[iDir][0] * sMinRad;
                    float sy = y + ATdirSteps[iDir][1] * sMinRad;

                    for (int sc = 0; sc < stepCount; sc++) {
                        RenderWork();
                        int x2 = PascalRoundToInt(sx);
                        int y2 = PascalRoundToInt(sy);

                        if (x2 < 0) {
                            x2 = -x2;
                            if (x2 >= WLo) break;
                        } else if (x2 >= width) {
                            x2 = MW2 - x2;
                            if (x2 < WHi) break;
                        }
                        if (y2 < 0) {
                            y2 = -y2;
                            if (y2 >= HLo) break;
                        } else if (y2 >= height) {
                            y2 = MH2 - y2;
                            if (y2 < HHi) break;
                        }

                        const uint32_t atValue = atLevel[y2 * width + x2];
                        float st = CalcAOScaledSamplePasLike(
                            static_cast<int>(atValue), zp0, RMA[sc]);
                        int it = CalcAOHiQItPasLike(st, sit, sZRT);
                        if (tracePixel && iDir == 1 && DebugPrintAoHiqStderr()) {
                            std::fprintf(stderr,
                                         "AOHIQ_SAMPLE currentLevel=%d iDir=%d sc=%d x2=%d y2=%d at=%u zp0=%d rma=%.12f st=%.12f prev=%d it=%d\n",
                                         currentLevel, iDir, sc, x2, y2, atValue, zp0,
                                         static_cast<double>(RMA[sc]), st,
                                         static_cast<int>(psm[iDir]), it);
                        }
                        if (psm[iDir] < it) {
                            psm[iDir] = static_cast<int16_t>(std::min(32767, it));
                        }

                        sx += ATdirSteps[iDir][0] * iStep;
                        sy += ATdirSteps[iDir][1] * iStep;
                    }
                }

                if (bSummUp) {
                    float stSum = CalcAOHiQSumPasLike(psi2);
                    if (tracePixel) {
                        std::ostringstream payload;
                        payload << std::fixed << std::setprecision(12)
                                << "currentLevel=" << currentLevel
                                << " stSum=" << stSum;
                        for (int i = 0; i < 32; i++) {
                            payload << " p" << i << "=" << psm[i];
                        }
                        AppendParityTrace("ao_hiq_sum", payload.str());
                        if (DebugPrintAoHiqStderr()) {
                            std::fprintf(stderr,
                                         "AOHIQ_SUM currentLevel=%d stSum=%.12f sMul=%.12f\n",
                                         currentLevel, static_cast<double>(stSum), static_cast<double>(sMul));
                        }
                    }
                    psi->AmbShadow = static_cast<uint16_t>(
                        PascalRoundToInt(std::clamp(stSum * sMul, 0.0f, 16383.0f)));
                    if (tracePixel) {
                        std::ostringstream payload;
                        payload << std::fixed << std::setprecision(12)
                                << "currentLevel=" << currentLevel
                                << " AmbShadow=" << psi->AmbShadow;
                        AppendParityTrace("ao_hiq_finish", payload.str());
                        if (DebugPrintAoHiqStderr()) {
                            std::fprintf(stderr,
                                         "AOHIQ_FINISH currentLevel=%d AmbShadow=%u\n",
                                         currentLevel, static_cast<unsigned>(psi->AmbShadow));
                        }
                    }
                }
                ++x;
            }
        }

        if (currentLevel < numLevels) {
            NextATlevelHiQ(atLevel, width, height, 1 << currentLevel);
        }
    }
}

// ----------------------------------------------------------------------------
// CalcAO_HiQ_Threshold -- 24-bit threshold AO (ATi=3)
// Ported from AmbHiQ.pas TAmbHiQCalcT0.Execute (scalar path)
// Uses st*sit / (st*st + sZRT) formula (smoother falloff)
// ----------------------------------------------------------------------------

static void CalcAO_HiQ_Threshold(
    TsiLight5* siLight, int width, int height,
    const TMandHeader10* header)
{
    const auto& ATdirSteps=EnsureATdirSteps();
    if (HasParityTraceContext()) {
        std::ostringstream payload;
        payload << "width=" << width
                << " height=" << height
                << " traceX=" << GetParityTracePixelX()
                << " traceY=" << GetParityTracePixelY();
        AppendParityTrace("ao_hiq_t0_enter", payload.str());
    }

    double Zcorr_local, ZcMul_local, ZZstmitDif_local;
    TMandHeader10 headerCopy = *header;
    CalcPPZvals(headerCopy, Zcorr_local, ZcMul_local, ZZstmitDif_local);

    float aZScaleFactor = static_cast<float>((std::pow(256.0 / ZcMul_local + 1.0, 2.0) - 1.0) / Zcorr_local);
    float threshold = header->sAmbShadowThreshold;
    if (threshold < 0.01f) threshold = 0.01f;
    float borderMirror = MinCS(header->bSSAO24BorderMirrorSize * 0.01f, 0.9f);

    int ymin = PascalRoundToInt(std::sqrt(static_cast<float>(width * width + height * height)) * 0.5f);
    int numLevels = 1;
    int tmpX = 5;
    do {
        numLevels++;
        tmpX <<= 1;
    } while (numLevels < 15 && tmpX <= ymin);

    // Allocate Z-level buffer
    std::vector<uint32_t> atLevel(width * height);
    // Angular max arrays: per-pixel, 32 directions, int16 values
    std::vector<int16_t> angMaxArr(static_cast<size_t>(width) * height * 32, -32768);

    int MH2 = 2 * (height - 1);
    int MW2 = 2 * (width - 1);
    int WLo = PascalRoundToInt(static_cast<float>(width) * borderMirror);
    int WHi = width - 1 - WLo;
    int HLo = PascalRoundToInt(static_cast<float>(height) * borderMirror);
    int HHi = height - 1 - HLo;

    FirstATlevelHiQ(atLevel, siLight, width * height);

    for (int currentLevel = 1; currentLevel <= numLevels; currentLevel++) {
        RenderPoll();
        bool bSummUp = (currentLevel == numLevels);

        float sit = aZScaleFactor / 22000.0f * 4096.0f;
        float sZRT = threshold * 4096.0f / sit * std::sqrt(std::sqrt(static_cast<float>(numLevels) / static_cast<float>(currentLevel)));
        sZRT = sZRT * sZRT; // Sqr(sZRT) -- T0 variant uses squared threshold
        float sMul = 1.5f * 32767.0f / (static_cast<float>(Pi) * 32.0f * std::atan(threshold * 0.6f * std::sqrt(std::sqrt(static_cast<float>(numLevels)))));

        int iStep = 1 << (currentLevel - 1);
        float sMinRad = (iStep < 2) ? 1.0f : 3.25f * iStep;
        int stepCount = (iStep < 2) ? 5 : 3;
        float RMA[6];
        for (int i = 0; i < stepCount; i++)
            RMA[i] = 1.0f / (sMinRad + i * iStep + 0.1f);
        sit = sit * sZRT;

        for (int y = 0; y < height; y++) {
            RenderPoll();
            for (int x = 0; x < width; x++) {
                RenderWork();
                int pixIdx = y * width + x;
                TsiLight5* psi = &siLight[pixIdx];
                const bool tracePixel = HasParityTraceContext() &&
                                        x == GetParityTracePixelX() &&
                                        y == GetParityTracePixelY();

                // Check background
                uint32_t rawCheck;
                std::memcpy(&rawCheck, &psi->RoughZposFine, 4);
                if ((rawCheck & 0x80000000u) != 0) continue; // background (Zpos bit 15)

                // Get Z position for this pixel
                int zp0 = static_cast<int>((rawCheck & 0xFFFFFF00u) >> 1);
                int16_t* psm = &angMaxArr[static_cast<size_t>(pixIdx) * 32];
                if (tracePixel) {
                    std::ostringstream payload;
                    payload << std::fixed << std::setprecision(12)
                            << "currentLevel=" << currentLevel
                            << " bSummUp=" << (bSummUp ? 1 : 0)
                            << " zp0=" << zp0
                            << " sit=" << sit
                            << " sZRT=" << sZRT
                            << " sMul=" << sMul
                            << " iStep=" << iStep
                            << " sMinRad=" << sMinRad
                            << " stepCount=" << stepCount
                            << " WLo=" << WLo
                            << " WHi=" << WHi
                            << " HLo=" << HLo
                            << " HHi=" << HHi;
                    AppendParityTrace("ao_hiq_t0_begin", payload.str());
                }

                for (int iDir = 0; iDir < 32; iDir++) {
                    float sx = x + ATdirSteps[iDir][0] * sMinRad;
                    float sy = y + ATdirSteps[iDir][1] * sMinRad;

                    for (int sc = 0; sc < stepCount; sc++) {
                        RenderWork();
                        int x2 = PascalRoundToInt(sx);
                        int y2 = PascalRoundToInt(sy);

                        // Border reflection
                        if (x2 < 0) {
                            x2 = -x2;
                            if (x2 >= WLo) break;
                        } else if (x2 >= width) {
                            x2 = MW2 - x2;
                            if (x2 < WHi) break;
                        }
                        if (y2 < 0) {
                            y2 = -y2;
                            if (y2 >= HLo) break;
                        } else if (y2 >= height) {
                            y2 = MH2 - y2;
                            if (y2 < HHi) break;
                        }

                        float st = static_cast<float>(static_cast<int>(atLevel[y2 * width + x2]) - zp0) * RMA[sc];
                        int it = PascalRoundToInt(st * sit / (st * st + sZRT));
                        if (psm[iDir] < it) {
                            psm[iDir] = static_cast<int16_t>(std::min(32767, it));
                        }

                        sx += ATdirSteps[iDir][0] * iStep;
                        sy += ATdirSteps[iDir][1] * iStep;
                    }
                }

                if (bSummUp) {
                    float stSum = CalcAOHiQSumPasLike(psm);
                    if (tracePixel) {
                        std::ostringstream payload;
                        payload << std::fixed << std::setprecision(12)
                                << "currentLevel=" << currentLevel
                                << " stSum=" << stSum;
                        for (int i = 0; i < 32; i++) {
                            payload << " p" << i << "=" << psm[i];
                        }
                        AppendParityTrace("ao_hiq_t0_sum", payload.str());
                    }
                    psi->AmbShadow = static_cast<uint16_t>(
                        PascalRoundToInt(std::clamp(stSum * sMul, 0.0f, 16383.0f)));
                    if (tracePixel) {
                        std::ostringstream payload;
                        payload << std::fixed << std::setprecision(12)
                                << "currentLevel=" << currentLevel
                                << " AmbShadow=" << psi->AmbShadow;
                        AppendParityTrace("ao_hiq_t0_finish", payload.str());
                    }
                }
            }
        }

        // Advance to next level
        if (currentLevel < numLevels) {
            NextATlevelHiQ(atLevel, width, height, 1 << currentLevel);
        }
    }
}

// ----------------------------------------------------------------------------
// CalcAO_HiQ_Random -- 24-bit random AO (Type 2)
// Ported from AmbHiQ.pas TAmbHiQCalcR.Execute (non-SSE path)
// Uses random jittered sampling directions
// ----------------------------------------------------------------------------

static void CalcAO_HiQ_Random(
    TsiLight5* siLight, int width, int height,
    const TMandHeader10* header,
    int logicalThreadCount = 1)
{
    const auto& ATdirSteps=EnsureATdirSteps();

    double Zcorr_local, ZcMul_local, ZZstmitDif_local;
    TMandHeader10 headerCopy = *header;
    CalcPPZvals(headerCopy, Zcorr_local, ZcMul_local, ZZstmitDif_local);

    float aZScaleFactor = static_cast<float>((std::pow(256.0 / ZcMul_local + 1.0, 2.0) - 1.0) / Zcorr_local);
    float threshold = header->sAmbShadowThreshold;
    if (threshold < 0.01f) threshold = 0.01f;
    float borderMirror = MinCS(header->bSSAO24BorderMirrorSize * 0.01f, 0.9f);
    int rcount = std::max(1, static_cast<int>(header->SSAORcount));

    int ymin = PascalRoundToInt(std::sqrt(static_cast<float>(width * width + height * height)) * 0.5f);
    int numLevels = 1;
    int tmpX = 5;
    do {
        numLevels++;
        tmpX <<= 1;
    } while (numLevels < 15 && tmpX <= ymin);

    std::vector<uint32_t> atLevel(width * height);
    std::vector<int16_t> angMaxArr(static_cast<size_t>(width) * height * 32, -32768);

    int MH2 = 2 * (height - 1);
    int MW2 = 2 * (width - 1);
    int WLo = PascalRoundToInt(static_cast<float>(width) * borderMirror);
    int WHi = width - 1 - WLo;
    int HLo = PascalRoundToInt(static_cast<float>(height) * borderMirror);
    int HHi = height - 1 - HLo;

    FirstATlevelHiQ(atLevel, siLight, width * height);

    for (int currentLevel = 1; currentLevel <= numLevels; currentLevel++) {
        RenderPoll();
        bool bSummUp = (currentLevel == numLevels);
        auto laneSeeds = InitAOLogicalLaneSeeds(logicalThreadCount, height);

        float sit = static_cast<float>(static_cast<double>(aZScaleFactor) / 22000.0 * 4096.0);
        float sZRT = static_cast<float>(static_cast<double>(threshold) * 0.7 * 4096.0 / sit *
                     std::sqrt(std::sqrt(static_cast<double>(numLevels) / currentLevel)));
        float sMul = static_cast<float>(1.5 * 32767.0 /
                     (Pi * 32.0 * std::pow(std::atan(static_cast<double>(threshold) * 0.65 *
                     std::sqrt(std::sqrt(static_cast<double>(numLevels)))), 0.9)) / rcount);
        int imin = 16383 / rcount;

        int iStep = 1 << (currentLevel - 1);
        float sMinRad = (iStep < 2) ? 1.0f : 3.25f * iStep;
        int stepCount = (iStep < 2) ? 5 : 3;
        int iand = iStep - 1;
        float ssub = iand * 0.5f;
        float sstep_val = static_cast<float>(iStep);

        // Pascal TAmbHiQCalcR has NO sit*=sZRT pre-multiply, NO 4-pixel fast-path,
        // and uses linear denominator (sZRT + |st|) in the it formula.
        // T12c: removed all three port artifacts that came from copy-pasting
        // the Threshold path.

        for (int y = 0; y < height; y++) {
            RenderPoll();
            uint32_t& seed = laneSeeds[static_cast<size_t>(y) % laneSeeds.size()];
            for (int x = 0; x < width; x++) {
                RenderWork();
                int pixIdx = y * width + x;
                TsiLight5* psi = &siLight[pixIdx];

                uint32_t rawCheck;
                std::memcpy(&rawCheck, &psi->RoughZposFine, 4);
                if ((rawCheck & 0x80000000u) != 0) {
                    continue;
                }

                int zp0 = static_cast<int>((rawCheck & 0xFFFFFF00u) >> 1);
                int16_t* psm = &angMaxArr[static_cast<size_t>(pixIdx) * 32];
                ParityEnterPixel(x, y);
                const bool tracePixel = ParityActive();
                if (tracePixel) {
                    ParityBeginStage("ao_hiq_r_begin", x, y);
                    ParityAddInt("currentLevel", currentLevel);
                    ParityAddBoolNeg1("bSummUp", bSummUp);
                    ParityAddInt("zp0", zp0);
                    ParityAddDouble12("sit", sit);
                    ParityAddDouble12("sZRT", sZRT);
                    ParityAddDouble12("sMul", sMul);
                    ParityAddInt("iStep", iStep);
                    ParityAddDouble12("sMinRad", sMinRad);
                    ParityAddInt("stepCount", stepCount);
                    ParityAddInt("iand", iand);
                    ParityAddInt("WLo", WLo);
                    ParityAddInt("WHi", WHi);
                    ParityAddInt("HLo", HLo);
                    ParityAddInt("HHi", HHi);
                    ParityEndStage();
                }

                for (int iDir = 0; iDir < 32; iDir++) {
                    // Pascal SSE advances PS while its destination index counts down.
                    const int angleIndex = 31 - iDir;
                    float sx = ATdirSteps[iDir][0] * sMinRad - ssub;
                    float sy = ATdirSteps[iDir][1] * sMinRad - ssub;

                    for (int sc = 0; sc < stepCount; sc++) {
                        RenderWork();
                        seed = 214013 * seed + 2531011;
                        // Pascal asm: x2 uses (seed>>10)&iand, y2 uses (seed>>16)&iand
                        // (after shr eax,10 the second shr eax,6 makes it >>16 cumulative).
                        int x2 = PascalRoundToInt(sx) + ((seed >> 10) & iand);
                        int y2 = PascalRoundToInt(sy) + ((seed >> 16) & iand);
                        float distSqr = static_cast<float>(x2 * x2 + y2 * y2);
                        if (tracePixel && iDir == 0) {
                            ParityBeginStage("ao_hiq_r_dir0_step", x, y);
                            ParityAddInt("currentLevel", currentLevel);
                            ParityAddInt("iDir", iDir);
                            ParityAddInt("sc", sc);
                            {
                                char buf[32];
                                std::snprintf(buf, sizeof(buf), "0x%016llX",
                                              static_cast<unsigned long long>(seed));
                                ParityAddString("seed", buf);
                            }
                            ParityAddDouble12("sx", sx);
                            ParityAddDouble12("sy", sy);
                            ParityAddInt("x2pre", x2);
                            ParityAddInt("y2pre", y2);
                            ParityAddDouble12("distSqr", distSqr);
                            ParityAddInt("prev", psm[angleIndex]);
                            ParityEndStage();
                        }
                        if (distSqr == 0.0f) {
                            sx += ATdirSteps[iDir][0] * sstep_val;
                            sy += ATdirSteps[iDir][1] * sstep_val;
                            continue;
                        }

                        x2 += x;
                        y2 += y;

                        // Border reflection
                        if (x2 < 0) {
                            x2 = -x2;
                            if (x2 >= WLo) break;
                        } else if (x2 >= width) {
                            x2 = MW2 - x2;
                            if (x2 < WHi) break;
                        }
                        if (y2 < 0) {
                            y2 = -y2;
                            if (y2 >= HLo) break;
                        } else if (y2 >= height) {
                            y2 = MH2 - y2;
                            if (y2 < HHi) break;
                        }

                        // Pascal asm flow (TAmbHiQCalcR):
                        //   xmm0 = (PATL - zp); CVTSI2SS to float
                        //   xmm2 = 1/sqrt(distSqr)  (RSQRTSS approx)
                        //   xmm0 = xmm0 * xmm2 = st
                        //   xmm3 = xmm0  (st)
                        //   xmm0 = |xmm0| = |st|
                        //   xmm3 = xmm3 * sit       = st * sit
                        //   xmm0 = xmm0 + sZRT      = sZRT + |st|
                        //   xmm3 = xmm3 * sZRT      = (st * sit) * sZRT
                        //   xmm0 = 1/xmm0  (RCPSS approx)
                        //   xmm3 = xmm3 * xmm0      = it
                        // Order matters: Pascal does (st*sit)*sZRT, not (st*sZRT)*sit.
                        float invSqrt = numeric::reciprocalSqrtEstimate(distSqr);
                        float st = static_cast<float>(static_cast<int>(atLevel[y2 * width + x2]) - zp0) * invSqrt;
                        float num1 = st * sit;
                        float invDenom = numeric::reciprocalEstimate(sZRT + std::fabs(st));
                        float num2 = num1 * sZRT;
                        int it = PascalRoundToInt(std::clamp(num2 * invDenom, -32768.0f, 32767.0f));
                        if (tracePixel && iDir == 0) {
                            ParityBeginStage("ao_hiq_r_dir0_eval", x, y);
                            ParityAddInt("currentLevel", currentLevel);
                            ParityAddInt("iDir", iDir);
                            ParityAddInt("sc", sc);
                            ParityAddInt("x2", x2);
                            ParityAddInt("y2", y2);
                            ParityAddInt("at", static_cast<int>(atLevel[y2 * width + x2]));
                            ParityAddInt("zp", zp0);
                            ParityAddDouble12("st", st);
                            ParityAddInt("it", it);
                            ParityEndStage();
                        }
                        if (psm[angleIndex] < it) {
                            psm[angleIndex] = static_cast<int16_t>(std::min(32767, it));
                        }

                        sx += ATdirSteps[iDir][0] * sstep_val;
                        sy += ATdirSteps[iDir][1] * sstep_val;
                    }
                }

                if (bSummUp) {
                    float stSum = CalcAOHiQSumPasLike(psm);
                    if (tracePixel) {
                        ParityBeginStage("ao_hiq_r_sum", x, y);
                        ParityAddInt("currentLevel", currentLevel);
                        ParityAddDouble12("stSum", stSum);
                        char nm[8];
                        for (int i = 0; i < 32; i++) {
                            std::snprintf(nm, sizeof(nm), "p%d", i);
                            ParityAddInt(nm, psm[31 - i]);
                        }
                        ParityEndStage();
                    }
                    long addVal = std::max(0L, std::min(static_cast<long>(imin), static_cast<long>(PascalRoundToInt(stSum * sMul))));
                    psi->AmbShadow = static_cast<uint16_t>(std::min(16383, static_cast<int>(psi->AmbShadow) + static_cast<int>(addVal)));
                    if (tracePixel) {
                        ParityBeginStage("ao_hiq_r_finish", x, y);
                        ParityAddInt("currentLevel", currentLevel);
                        ParityAddInt("AmbShadow", psi->AmbShadow);
                        ParityEndStage();
                    }
                }
                ParityLeavePixel();
            }
        }

        if (currentLevel < numLevels) {
            NextATlevelHiQ(atLevel, width, height, 1 << currentLevel);
        }
    }
}

// ----------------------------------------------------------------------------
// CalcAO_HiQ_RandomThreshold -- 24-bit random threshold AO (ATi=5)
// Ported from AmbHiQ.pas TAmbHiQCalcRT0.Execute (non-SSE path)
// ----------------------------------------------------------------------------

static void CalcAO_HiQ_RandomThreshold(
    TsiLight5* siLight, int width, int height,
    const TMandHeader10* header,
    int logicalThreadCount = 1)
{
    const auto& ATdirSteps=EnsureATdirSteps();

    double Zcorr_local, ZcMul_local, ZZstmitDif_local;
    TMandHeader10 headerCopy = *header;
    CalcPPZvals(headerCopy, Zcorr_local, ZcMul_local, ZZstmitDif_local);

    float aZScaleFactor = static_cast<float>((std::pow(256.0 / ZcMul_local + 1.0, 2.0) - 1.0) / Zcorr_local);
    float threshold = header->sAmbShadowThreshold;
    if (threshold < 0.01f) threshold = 0.01f;
    float borderMirror = MinCS(header->bSSAO24BorderMirrorSize * 0.01f, 0.9f);
    int rcount = std::max(1, static_cast<int>(header->SSAORcount));

    int ymin = PascalRoundToInt(std::sqrt(static_cast<float>(width * width + height * height)) * 0.5f);
    int numLevels = 1;
    int tmpX = 5;
    do {
        numLevels++;
        tmpX <<= 1;
    } while (numLevels < 15 && tmpX <= ymin);

    std::vector<uint32_t> atLevel(width * height);
    std::vector<int16_t> angMaxArr(static_cast<size_t>(width) * height * 32, -32768);

    int MH2 = 2 * (height - 1);
    int MW2 = 2 * (width - 1);
    int WLo = PascalRoundToInt(static_cast<float>(width) * borderMirror);
    int WHi = width - 1 - WLo;
    int HLo = PascalRoundToInt(static_cast<float>(height) * borderMirror);
    int HHi = height - 1 - HLo;

    FirstATlevelHiQ(atLevel, siLight, width * height);

    for (int currentLevel = 1; currentLevel <= numLevels; currentLevel++) {
        RenderPoll();
        bool bSummUp = (currentLevel == numLevels);
        auto laneSeeds = InitAOLogicalLaneSeeds(logicalThreadCount, height);

        float sit = static_cast<float>(static_cast<double>(aZScaleFactor) / 22000.0 * 4096.0);
        // Pascal evaluates the Single-only prefix before the Double sqrt operand.
        const float zrtPrefix = threshold * 4096.0f / sit;
        const double zrt = static_cast<double>(zrtPrefix) *
                           std::sqrt(std::sqrt(static_cast<double>(numLevels) / currentLevel));
        float sZRT = static_cast<float>(zrt * zrt);
        float sMul = static_cast<float>(1.5 * 32767.0 /
                     (Pi * 32.0 * std::atan(static_cast<double>(threshold) * 0.64 *
                     std::sqrt(std::sqrt(static_cast<double>(numLevels))))) / rcount);
        int imin = 16383 / rcount;

        int iStep = 1 << (currentLevel - 1);
        float sMinRad = (iStep < 2) ? 1.0f : 3.25f * iStep;
        int stepCount = (iStep < 2) ? 5 : 3;
        int iand = iStep * 2 - 1;
        float ssub = iand * 0.5f;
        float sstepVal = static_cast<float>(iStep);

        // Pascal TAmbHiQCalcRT0 has NO sit*=sZRT pre-multiply, and uses
        // quadratic denominator (st*st + sZRT) in the it formula. T12c:
        // formulas were swapped between Random and RandomThreshold paths
        // during port — both halves now match their Pascal originals.

        for (int y = 0; y < height; y++) {
            RenderPoll();
            uint32_t& seed = laneSeeds[static_cast<size_t>(y) % laneSeeds.size()];
            for (int x = 0; x < width; x++) {
                RenderWork();
                int pixIdx = y * width + x;
                TsiLight5* psi = &siLight[pixIdx];

                uint32_t rawCheck;
                std::memcpy(&rawCheck, &psi->RoughZposFine, 4);
                if ((rawCheck & 0x80000000u) != 0) continue;

                int zp0 = static_cast<int>((rawCheck & 0xFFFFFF00u) >> 1);
                int16_t* psm = &angMaxArr[static_cast<size_t>(pixIdx) * 32];

                for (int iDir = 0; iDir < 32; iDir++) {
                    // Pascal SSE advances PS while its destination index counts down.
                    const int angleIndex = 31 - iDir;
                    float sx = ATdirSteps[iDir][0] * sMinRad - ssub;
                    float sy = ATdirSteps[iDir][1] * sMinRad - ssub;

                    for (int sc = 0; sc < stepCount; sc++) {
                        RenderWork();
                        seed = 214013 * seed + 2531011;
                        // Pascal asm: x2 uses (seed>>10)&iand, y2 uses (seed>>16)&iand.
                        int x2 = PascalRoundToInt(sx) + ((seed >> 10) & iand);
                        int y2 = PascalRoundToInt(sy) + ((seed >> 16) & iand);
                        float distSqr = static_cast<float>(x2 * x2 + y2 * y2);
                        if (distSqr == 0.0f) {
                            sx += ATdirSteps[iDir][0] * sstepVal;
                            sy += ATdirSteps[iDir][1] * sstepVal;
                            continue;
                        }

                        x2 += x;
                        y2 += y;

                        if (x2 < 0) {
                            x2 = -x2;
                            if (x2 >= WLo) break;
                        } else if (x2 >= width) {
                            x2 = MW2 - x2;
                            if (x2 < WHi) break;
                        }
                        if (y2 < 0) {
                            y2 = -y2;
                            if (y2 >= HLo) break;
                        } else if (y2 >= height) {
                            y2 = MH2 - y2;
                            if (y2 < HHi) break;
                        }

                        // Pascal asm flow (TAmbHiQCalcRT0):
                        //   xmm2 = 1/sqrt(distSqr)  (RSQRTSS)
                        //   xmm0 = (PATL-zp) * xmm2 = st
                        //   xmm3 = xmm0  (st)
                        //   xmm0 = xmm0 * xmm0 = st*st
                        //   xmm3 = xmm3 * sit  = st*sit
                        //   xmm0 = xmm0 + sZRT = st*st + sZRT
                        //   xmm3 = xmm3 * sZRT = (st*sit)*sZRT
                        //   xmm0 = 1/xmm0      (RCPSS)
                        //   xmm3 = xmm3 * xmm0 = it
                        float invSqrt = numeric::reciprocalSqrtEstimate(distSqr);
                        float st = static_cast<float>(static_cast<int>(atLevel[y2 * width + x2]) - zp0) * invSqrt;
                        float num1 = st * sit;
                        float invDenom = numeric::reciprocalEstimate(st * st + sZRT);
                        float num2 = num1 * sZRT;
                        int it = PascalRoundToInt(std::clamp(num2 * invDenom, -32768.0f, 32767.0f));
                        if (psm[angleIndex] < it) {
                            psm[angleIndex] = static_cast<int16_t>(std::min(32767, it));
                        }

                        sx += ATdirSteps[iDir][0] * sstepVal;
                        sy += ATdirSteps[iDir][1] * sstepVal;
                    }
                }

                if (bSummUp) {
                    float stSum = CalcAOHiQSumPasLike(psm);
                    ParityEnterPixel(x, y);
                    if (ParityActive()) {
                        ParityBeginStage("ao_hiq_rt0_sum", x, y);
                        ParityAddDouble12("stSum", stSum);
                        ParityAddDouble12("sMul", sMul);
                        ParityAddDouble12("sit", sit);
                        ParityAddDouble12("sZRT", sZRT);
                        char name[8];
                        for (int i = 0; i < 32; ++i) {
                            std::snprintf(name, sizeof(name), "p%d", i);
                            ParityAddInt(name, psm[i]);
                        }
                        ParityEndStage();
                    }
                    ParityLeavePixel();
                    long addVal = std::max(0L, std::min(static_cast<long>(imin), static_cast<long>(PascalRoundToInt(stSum * sMul))));
                    psi->AmbShadow = static_cast<uint16_t>(std::min(16383, static_cast<int>(psi->AmbShadow) + static_cast<int>(addVal)));
                }
            }
        }

        if (currentLevel < numLevels) {
            NextATlevelHiQ(atLevel, width, height, 1 << currentLevel);
        }
    }
}


// ############################################################################
// Pano HiQ AO kernels
// Ported from AmbHiQ.pas TAmbHiQCalcPano, TAmbHiQCalcT0Pano,
//   TAmbHiQCalcRPano, TAmbHiQCalcRT0Pano
// Key difference from non-pano: X wraps horizontally, Y mirrors at poles
// ############################################################################

// ----------------------------------------------------------------------------
// CalcAO_HiQ_Pano -- pano variant of CalcAO_HiQ (ATi=6 after +4 remap)
// From TAmbHiQCalcPano.Execute: X wrap, Y pole mirror,
// formula: st*sit / (sZRT + abs(st))
// ----------------------------------------------------------------------------
static void CalcAO_HiQ_Pano(
    TsiLight5* siLight, int width, int height,
    const TMandHeader10* header)
{
    const auto& ATdirSteps=EnsureATdirSteps();

    double Zcorr_local, ZcMul_local, ZZstmitDif_local;
    TMandHeader10 headerCopy = *header;
    CalcPPZvals(headerCopy, Zcorr_local, ZcMul_local, ZZstmitDif_local);

    float aZScaleFactor = static_cast<float>((std::pow(256.0 / ZcMul_local + 1.0, 2.0) - 1.0) / Zcorr_local);
    float threshold = header->sAmbShadowThreshold;
    if (threshold < 0.01f) threshold = 0.01f;
    float borderMirror = MinCS(header->bSSAO24BorderMirrorSize * 0.01f, 0.9f);

    int ymin = PascalRoundToInt(std::sqrt(static_cast<float>(width * width + height * height)) * 0.5f);
    int numLevels = 1;
    int tmpX = 5;
    do {
        numLevels++;
        tmpX <<= 1;
    } while (numLevels < 15 && tmpX <= ymin);

    std::vector<uint32_t> atLevel(width * height);
    std::vector<int16_t> angMaxArr(static_cast<size_t>(width) * height * 32, -32768);

    int MH2 = 2 * (height - 1);
    int HLo = PascalRoundToInt(static_cast<float>(height) * borderMirror);
    int HHi = height - 1 - HLo;

    FirstATlevelHiQ(atLevel, siLight, width * height);

    for (int currentLevel = 1; currentLevel <= numLevels; currentLevel++) {
        RenderPoll();
        bool bSummUp = (currentLevel == numLevels);

        float sit = aZScaleFactor / 22000.0f * 4096.0f;
        float sZRT = threshold * 0.7f * 4096.0f / sit *
                     std::sqrt(std::sqrt(static_cast<float>(numLevels) / static_cast<float>(currentLevel)));
        float sMul = 1.5f * 32767.0f /
                     (static_cast<float>(Pi) * 32.0f *
                      std::pow(std::atan(threshold * 0.6f * std::sqrt(std::sqrt(static_cast<float>(numLevels)))), 0.9f));

        int iStep = 1 << (currentLevel - 1);
        float sMinRad = (iStep < 2) ? 1.0f : 3.25f * iStep;
        int stepCount = (iStep < 2) ? 5 : 3;
        int iMaxRad = PascalRoundToInt(static_cast<double>(sMinRad) +
                                       static_cast<double>(iStep * stepCount) + 0.1);
        float RMA[6];
        for (int i = 0; i < stepCount; i++)
            RMA[i] = static_cast<float>(
                1.0 / (static_cast<double>(sMinRad) +
                       static_cast<double>(i * iStep) + 0.1));
        sit = sit * sZRT;

        for (int y = 0; y < height; y++) {
            RenderPoll();
            for (int x = 0; x < width;) {
                RenderPoll();
                int pixIdx = y * width + x;
                TsiLight5* psi = &siLight[pixIdx];

                if (!bSummUp && x >= iMaxRad && x < width - iMaxRad - 3) {
                    RenderWork(4);
                    uint32_t raw0, raw1, raw2, raw3;
                    std::memcpy(&raw0, &siLight[pixIdx + 0].RoughZposFine, 4);
                    std::memcpy(&raw1, &siLight[pixIdx + 1].RoughZposFine, 4);
                    std::memcpy(&raw2, &siLight[pixIdx + 2].RoughZposFine, 4);
                    std::memcpy(&raw3, &siLight[pixIdx + 3].RoughZposFine, 4);
                    const bool allBackground =
                        ((raw0 & raw1 & raw2 & raw3 & 0x80000000u) != 0);
                    if (!allBackground) {
                        const int zp[4] = {
                            static_cast<int>((raw0 & 0xFFFFFF00u) >> 1),
                            static_cast<int>((raw1 & 0xFFFFFF00u) >> 1),
                            static_cast<int>((raw2 & 0xFFFFFF00u) >> 1),
                            static_cast<int>((raw3 & 0xFFFFFF00u) >> 1),
                        };
                        int16_t* psi2 = &angMaxArr[static_cast<size_t>(pixIdx) * 32];
                        for (int iDir = 0; iDir < 32; ++iDir) {
                            float sx = x + ATdirSteps[iDir][0] * sMinRad;
                            float sy = y + ATdirSteps[iDir][1] * sMinRad;
                            for (int sc = 0; sc < stepCount; ++sc) {
                                RenderWork(4);
                                const int x2 = PascalRoundToInt(sx);
                                int y2 = PascalRoundToInt(sy);
                                if (y2 < 0) {
                                    y2 = -y2;
                                    if (y2 >= HLo) break;
                                } else if (y2 >= height) {
                                    y2 = MH2 - y2;
                                    if (y2 < HHi) break;
                                }

                                int16_t* lanePsm = psi2 + iDir;
                                const int base = y2 * width + x2;
                                for (int lane = 0; lane < 4; ++lane) {
                                    float st = CalcAOScaledSamplePasLike(
                                        static_cast<int>(atLevel[base + lane]), zp[lane], RMA[sc]);
                                    int it = CalcAOHiQItPasLike(st, sit, sZRT);
                                    if (*lanePsm < it) {
                                        *lanePsm = static_cast<int16_t>(std::min(32767, it));
                                    }
                                    lanePsm += 32;
                                }

                                sx += ATdirSteps[iDir][0] * iStep;
                                sy += ATdirSteps[iDir][1] * iStep;
                            }
                        }
                    }
                    x += 4;
                    continue;
                }

                RenderWork();
                uint32_t rawCheck;
                std::memcpy(&rawCheck, &psi->RoughZposFine, 4);
                if ((rawCheck & 0x80000000u) != 0) {
                    ++x;
                    continue;
                }

                int zp0 = static_cast<int>((rawCheck & 0xFFFFFF00u) >> 1);
                int16_t* psm = &angMaxArr[static_cast<size_t>(pixIdx) * 32];
                int16_t* psi2 = psm;

                for (int iDir = 0; iDir < 32; iDir++) {
                    float sx = x + ATdirSteps[iDir][0] * sMinRad;
                    float sy = y + ATdirSteps[iDir][1] * sMinRad;

                    for (int sc = 0; sc < stepCount; sc++) {
                        RenderWork();
                        int x2 = PascalRoundToInt(sx);
                        int y2 = PascalRoundToInt(sy);

                        // Pano X: horizontal wrap
                        if (x2 < 0) {
                            x2 = x2 + width;
                            if (x2 < 0) break;
                        } else if (x2 >= width) {
                            x2 = x2 - width;
                            if (x2 >= width) break;
                        }
                        // Pano Y: pole mirror
                        if (y2 < 0) {
                            y2 = -y2;
                            if (y2 >= HLo) break;
                        } else if (y2 >= height) {
                            y2 = MH2 - y2;
                            if (y2 < HHi) break;
                        }

                        float st = CalcAOScaledSamplePasLike(
                            static_cast<int>(atLevel[y2 * width + x2]), zp0, RMA[sc]);
                        int it = CalcAOHiQItPasLike(st, sit, sZRT);
                        if (psm[iDir] < it)
                            psm[iDir] = static_cast<int16_t>(std::min(32767, it));

                        sx += ATdirSteps[iDir][0] * iStep;
                        sy += ATdirSteps[iDir][1] * iStep;
                    }
                }

                if (bSummUp) {
                    float stSum = CalcAOHiQSumPasLike(psi2);
                    psi->AmbShadow = static_cast<uint16_t>(
                        PascalRoundToInt(std::clamp(stSum * sMul, 0.0f, 16383.0f)));
                }
                ++x;
            }
        }

        if (currentLevel < numLevels)
            NextATlevelHiQ(atLevel, width, height, 1 << currentLevel);
    }
}

// ----------------------------------------------------------------------------
// CalcAO_HiQ_T0_Pano -- pano variant of CalcAO_HiQ_Threshold (ATi=7)
// From TAmbHiQCalcT0Pano.Execute: X wrap, Y pole mirror,
// formula: st*sit / (st*st + sZRT) -- T0 squared denominator
// ----------------------------------------------------------------------------
static void CalcAO_HiQ_T0_Pano(
    TsiLight5* siLight, int width, int height,
    const TMandHeader10* header)
{
    const auto& ATdirSteps=EnsureATdirSteps();

    double Zcorr_local, ZcMul_local, ZZstmitDif_local;
    TMandHeader10 headerCopy = *header;
    CalcPPZvals(headerCopy, Zcorr_local, ZcMul_local, ZZstmitDif_local);

    float aZScaleFactor = static_cast<float>((std::pow(256.0 / ZcMul_local + 1.0, 2.0) - 1.0) / Zcorr_local);
    float threshold = header->sAmbShadowThreshold;
    if (threshold < 0.01f) threshold = 0.01f;
    float borderMirror = MinCS(header->bSSAO24BorderMirrorSize * 0.01f, 0.9f);

    int ymin = PascalRoundToInt(std::sqrt(static_cast<float>(width * width + height * height)) * 0.5f);
    int numLevels = 1;
    int tmpX = 5;
    do {
        numLevels++;
        tmpX <<= 1;
    } while (numLevels < 15 && tmpX <= ymin);

    std::vector<uint32_t> atLevel(width * height);
    std::vector<int16_t> angMaxArr(static_cast<size_t>(width) * height * 32, -32768);

    int MH2 = 2 * (height - 1);
    int HLo = PascalRoundToInt(static_cast<float>(height) * borderMirror);
    int HHi = height - 1 - HLo;

    FirstATlevelHiQ(atLevel, siLight, width * height);

    for (int currentLevel = 1; currentLevel <= numLevels; currentLevel++) {
        RenderPoll();
        bool bSummUp = (currentLevel == numLevels);

        float sit = aZScaleFactor / 22000.0f * 4096.0f;
        // T0: no 0.7 multiplier, sZRT is squared
        float sZRT = threshold * 4096.0f / sit *
                     std::sqrt(std::sqrt(static_cast<float>(numLevels) / static_cast<float>(currentLevel)));
        sZRT = sZRT * sZRT;
        // T0: sMul uses ArcTan directly (not Power(..., 0.9))
        float sMul = 1.5f * 32767.0f /
                     (static_cast<float>(Pi) * 32.0f *
                      std::atan(threshold * 0.6f * std::sqrt(std::sqrt(static_cast<float>(numLevels)))));

        int iStep = 1 << (currentLevel - 1);
        float sMinRad = (iStep < 2) ? 1.0f : 3.25f * iStep;
        int stepCount = (iStep < 2) ? 5 : 3;
        float RMA[6];
        for (int i = 0; i < stepCount; i++)
            RMA[i] = 1.0f / (sMinRad + i * iStep + 0.1f);
        sit = sit * sZRT;

        for (int y = 0; y < height; y++) {
            RenderPoll();
            for (int x = 0; x < width; x++) {
                RenderWork();
                int pixIdx = y * width + x;
                TsiLight5* psi = &siLight[pixIdx];

                uint32_t rawCheck;
                std::memcpy(&rawCheck, &psi->RoughZposFine, 4);
                if ((rawCheck & 0x80000000u) != 0) continue;

                int zp0 = static_cast<int>((rawCheck & 0xFFFFFF00u) >> 1);
                int16_t* psm = &angMaxArr[static_cast<size_t>(pixIdx) * 32];

                for (int iDir = 0; iDir < 32; iDir++) {
                    float sx = x + ATdirSteps[iDir][0] * sMinRad;
                    float sy = y + ATdirSteps[iDir][1] * sMinRad;

                    for (int sc = 0; sc < stepCount; sc++) {
                        RenderWork();
                        int x2 = PascalRoundToInt(sx);
                        int y2 = PascalRoundToInt(sy);

                        // Pano X: horizontal wrap
                        if (x2 < 0) {
                            x2 = x2 + width;
                            if (x2 < 0) break;
                        } else if (x2 >= width) {
                            x2 = x2 - width;
                            if (x2 >= width) break;
                        }
                        // Pano Y: pole mirror
                        if (y2 < 0) {
                            y2 = -y2;
                            if (y2 >= HLo) break;
                        } else if (y2 >= height) {
                            y2 = MH2 - y2;
                            if (y2 < HHi) break;
                        }

                        {
                            float st = static_cast<float>(static_cast<int>(atLevel[y2 * width + x2]) - zp0) * RMA[sc];
                            // T0 formula: st*sit / (st*st + sZRT)
                            int it = PascalRoundToInt(st * sit / (st * st + sZRT));
                            if (psm[iDir] < it)
                                psm[iDir] = static_cast<int16_t>(std::min(32767, it));
                        }

                        sx += ATdirSteps[iDir][0] * iStep;
                        sy += ATdirSteps[iDir][1] * iStep;
                    }
                    pano_t0_break1:;
                }

                if (bSummUp) {
                    float stSum = CalcAOHiQSumPasLike(psm);
                    psi->AmbShadow = static_cast<uint16_t>(
                        PascalRoundToInt(std::clamp(stSum * sMul, 0.0f, 16383.0f)));
                }
            }
        }

        if (currentLevel < numLevels)
            NextATlevelHiQ(atLevel, width, height, 1 << currentLevel);
    }
}

// ----------------------------------------------------------------------------
// CalcAO_HiQ_Random_Pano -- pano variant of CalcAO_HiQ_Random (ATi=8)
// From TAmbHiQCalcRPano.Execute: X wrap, Y pole mirror,
// random jitter, formula: st*sZRT*sit / (sZRT + abs(st))
// ----------------------------------------------------------------------------
static void CalcAO_HiQ_Random_Pano(
    TsiLight5* siLight, int width, int height,
    const TMandHeader10* header,
    int logicalThreadCount = 1)
{
    const auto& ATdirSteps=EnsureATdirSteps();

    double Zcorr_local, ZcMul_local, ZZstmitDif_local;
    TMandHeader10 headerCopy = *header;
    CalcPPZvals(headerCopy, Zcorr_local, ZcMul_local, ZZstmitDif_local);

    float aZScaleFactor = static_cast<float>((std::pow(256.0 / ZcMul_local + 1.0, 2.0) - 1.0) / Zcorr_local);
    float threshold = header->sAmbShadowThreshold;
    if (threshold < 0.01f) threshold = 0.01f;
    float borderMirror = MinCS(header->bSSAO24BorderMirrorSize * 0.01f, 0.9f);
    int rcount = std::max(1, static_cast<int>(header->SSAORcount));

    int ymin = PascalRoundToInt(std::sqrt(static_cast<float>(width * width + height * height)) * 0.5f);
    int numLevels = 1;
    int tmpX = 5;
    do {
        numLevels++;
        tmpX <<= 1;
    } while (numLevels < 15 && tmpX <= ymin);

    std::vector<uint32_t> atLevel(width * height);
    std::vector<int16_t> angMaxArr(static_cast<size_t>(width) * height * 32, -32768);

    int MH2 = 2 * (height - 1);
    int HLo = PascalRoundToInt(static_cast<float>(height) * borderMirror);
    int HHi = height - 1 - HLo;

    FirstATlevelHiQ(atLevel, siLight, width * height);

    for (int currentLevel = 1; currentLevel <= numLevels; currentLevel++) {
        RenderPoll();
        bool bSummUp = (currentLevel == numLevels);
        auto laneSeeds = InitAOLogicalLaneSeeds(logicalThreadCount, height);

        float sit = static_cast<float>(static_cast<double>(aZScaleFactor) / 22000.0 * 4096.0);
        float sZRT = static_cast<float>(static_cast<double>(threshold) * 0.7 * 4096.0 / sit *
                     std::sqrt(std::sqrt(static_cast<double>(numLevels) / currentLevel)));
        float sMul = static_cast<float>(1.5 * 32767.0 /
                     (Pi * 32.0 *
                      std::pow(std::atan(static_cast<double>(threshold) * 0.65 * std::sqrt(std::sqrt(static_cast<double>(numLevels)))), 0.9)) / rcount);
        int imin = 16383 / rcount;

        int iStep = 1 << (currentLevel - 1);
        float sMinRad = (iStep < 2) ? 1.0f : 3.25f * iStep;
        int stepCount = (iStep < 2) ? 5 : 3;
        int iand = iStep - 1;
        float ssub = iand * 0.5f;
        float sstep_val = static_cast<float>(iStep);

        // Random kernels apply sZRT once in the sample numerator below.

        for (int y = 0; y < height; y++) {
            RenderPoll();
            uint32_t& seed = laneSeeds[static_cast<size_t>(y) % laneSeeds.size()];
            for (int x = 0; x < width; x++) {
                RenderWork();
                int pixIdx = y * width + x;
                TsiLight5* psi = &siLight[pixIdx];

                uint32_t rawCheck;
                std::memcpy(&rawCheck, &psi->RoughZposFine, 4);
                if ((rawCheck & 0x80000000u) != 0) continue;

                int zp0 = static_cast<int>((rawCheck & 0xFFFFFF00u) >> 1);
                int16_t* psm = &angMaxArr[static_cast<size_t>(pixIdx) * 32];

                for (int iDir = 0; iDir < 32; iDir++) {
                    float sx = ATdirSteps[iDir][0] * sMinRad - ssub;
                    float sy = ATdirSteps[iDir][1] * sMinRad - ssub;

                    for (int sc = 0; sc < stepCount; sc++) {
                        RenderWork();
                        seed = 214013 * seed + 2531011;
                        // Pascal asm: x2 uses (seed>>10)&iand, y2 uses (seed>>16)&iand
                        // (after shr eax,10 the second shr eax,6 makes it >>16 cumulative).
                        int x2 = PascalRoundToInt(sx) + ((seed >> 10) & iand);
                        int y2 = PascalRoundToInt(sy) + ((seed >> 16) & iand);
                        float distSqr = static_cast<float>(x2 * x2 + y2 * y2);
                        if (distSqr == 0.0f) {
                            sx += ATdirSteps[iDir][0] * sstep_val;
                            sy += ATdirSteps[iDir][1] * sstep_val;
                            continue;
                        }

                        x2 += x;
                        y2 += y;

                        // Pano X: horizontal wrap
                        if (x2 < 0) {
                            x2 = x2 + width;
                            if (x2 < 0) break;
                        } else if (x2 >= width) {
                            x2 = x2 - width;
                            if (x2 >= width) break;
                        }
                        // Pano Y: pole mirror
                        if (y2 < 0) {
                            y2 = -y2;
                            if (y2 >= HLo) break;
                        } else if (y2 >= height) {
                            y2 = MH2 - y2;
                            if (y2 < HHi) break;
                        }

                        {
                            // Match the original SSE sample: RSQRTSS, two
                            // ordered multiplies, RCPSS, then signed saturation.
                            float invSqrt = numeric::reciprocalSqrtEstimate(distSqr);
                            float st = static_cast<float>(static_cast<int>(atLevel[y2 * width + x2]) - zp0) * invSqrt;
                            float num1 = st * sit;
                            float invDenom = numeric::reciprocalEstimate(sZRT + std::fabs(st));
                            float num2 = num1 * sZRT;
                            int it = PascalRoundToInt(std::clamp(num2 * invDenom, -32768.0f, 32767.0f));
                            // PS advances while Pascal's SSE iDir counts down.
                            int angleIndex = 31 - iDir;
                            if (psm[angleIndex] < it)
                                psm[angleIndex] = static_cast<int16_t>(it);
                        }

                        sx += ATdirSteps[iDir][0] * sstep_val;
                        sy += ATdirSteps[iDir][1] * sstep_val;
                    }
                }

                if (bSummUp) {
                    float stSum = CalcAOHiQSumPasLike(psm);
                    long addVal = std::max(0L, std::min(static_cast<long>(imin), static_cast<long>(PascalRoundToInt(stSum * sMul))));
                    psi->AmbShadow = static_cast<uint16_t>(std::min(16383, static_cast<int>(psi->AmbShadow) + static_cast<int>(addVal)));
                }
            }
        }

        if (currentLevel < numLevels)
            NextATlevelHiQ(atLevel, width, height, 1 << currentLevel);
    }
}

// ----------------------------------------------------------------------------
// CalcAO_HiQ_RandomT0_Pano -- pano variant of CalcAO_HiQ_RandomThreshold (ATi=9)
// From TAmbHiQCalcRT0Pano.Execute: X wrap, Y pole mirror,
// random jitter, T0 formula: st*sit*sZRT / (st*st + sZRT)
// iand = iStep*2 - 1 (wider jitter than R variant)
// ----------------------------------------------------------------------------
static void CalcAO_HiQ_RandomT0_Pano(
    TsiLight5* siLight, int width, int height,
    const TMandHeader10* header,
    int logicalThreadCount = 1)
{
    const auto& ATdirSteps=EnsureATdirSteps();

    double Zcorr_local, ZcMul_local, ZZstmitDif_local;
    TMandHeader10 headerCopy = *header;
    CalcPPZvals(headerCopy, Zcorr_local, ZcMul_local, ZZstmitDif_local);

    float aZScaleFactor = static_cast<float>((std::pow(256.0 / ZcMul_local + 1.0, 2.0) - 1.0) / Zcorr_local);
    float threshold = header->sAmbShadowThreshold;
    if (threshold < 0.01f) threshold = 0.01f;
    float borderMirror = MinCS(header->bSSAO24BorderMirrorSize * 0.01f, 0.9f);
    int rcount = std::max(1, static_cast<int>(header->SSAORcount));

    int ymin = PascalRoundToInt(std::sqrt(static_cast<float>(width * width + height * height)) * 0.5f);
    int numLevels = 1;
    int tmpX = 5;
    do {
        numLevels++;
        tmpX <<= 1;
    } while (numLevels < 15 && tmpX <= ymin);

    std::vector<uint32_t> atLevel(width * height);
    std::vector<int16_t> angMaxArr(static_cast<size_t>(width) * height * 32, -32768);

    int MH2 = 2 * (height - 1);
    int HLo = PascalRoundToInt(static_cast<float>(height) * borderMirror);
    int HHi = height - 1 - HLo;

    FirstATlevelHiQ(atLevel, siLight, width * height);

    for (int currentLevel = 1; currentLevel <= numLevels; currentLevel++) {
        RenderPoll();
        bool bSummUp = (currentLevel == numLevels);
        auto laneSeeds = InitAOLogicalLaneSeeds(logicalThreadCount, height);

        float sit = static_cast<float>(static_cast<double>(aZScaleFactor) / 22000.0 * 4096.0);
        // RT0: sZRT is squared, no 0.7 multiplier
        const float zrtPrefix = threshold * 4096.0f / sit;
        const double zrt = static_cast<double>(zrtPrefix) *
                     std::sqrt(std::sqrt(static_cast<double>(numLevels) / currentLevel));
        float sZRT = static_cast<float>(zrt * zrt);
        // RT0: sMul uses 0.64 and ArcTan (not Power)
        float sMul = static_cast<float>(1.5 * 32767.0 /
                     (Pi * 32.0 *
                      std::atan(static_cast<double>(threshold) * 0.64 * std::sqrt(std::sqrt(static_cast<double>(numLevels))))) / rcount);
        float smin = static_cast<float>(16383 / rcount);

        int iStep = 1 << (currentLevel - 1);
        float sMinRad = (iStep < 2) ? 1.0f : 3.25f * iStep;
        int stepCount = (iStep < 2) ? 5 : 3;
        int iand = iStep * 2 - 1;   // RT0: wider jitter
        float ssub = iand * 0.5f;
        float sstepVal = static_cast<float>(iStep);

        // Random kernels apply sZRT once in the sample numerator below.

        for (int y = 0; y < height; y++) {
            RenderPoll();
            uint32_t& seed = laneSeeds[static_cast<size_t>(y) % laneSeeds.size()];
            for (int x = 0; x < width; x++) {
                RenderWork();
                int pixIdx = y * width + x;
                TsiLight5* psi = &siLight[pixIdx];

                uint32_t rawCheck;
                std::memcpy(&rawCheck, &psi->RoughZposFine, 4);
                if ((rawCheck & 0x80000000u) != 0) continue;

                int zp0 = static_cast<int>((rawCheck & 0xFFFFFF00u) >> 1);
                int16_t* psm = &angMaxArr[static_cast<size_t>(pixIdx) * 32];

                for (int iDir = 0; iDir < 32; iDir++) {
                    float sx = ATdirSteps[iDir][0] * sMinRad - ssub;
                    float sy = ATdirSteps[iDir][1] * sMinRad - ssub;

                    for (int sc = 0; sc < stepCount; sc++) {
                        RenderWork();
                        seed = 214013 * seed + 2531011;
                        // Pascal asm: x2 uses (seed>>10)&iand, y2 uses (seed>>16)&iand.
                        int x2 = PascalRoundToInt(sx) + ((seed >> 10) & iand);
                        int y2 = PascalRoundToInt(sy) + ((seed >> 16) & iand);
                        float distSqr = static_cast<float>(x2 * x2 + y2 * y2);
                        if (distSqr == 0.0f) {
                            sx += ATdirSteps[iDir][0] * sstepVal;
                            sy += ATdirSteps[iDir][1] * sstepVal;
                            continue;
                        }

                        x2 += x;
                        y2 += y;

                        // Pano X: horizontal wrap
                        if (x2 < 0) {
                            x2 = x2 + width;
                            if (x2 < 0) break;
                        } else if (x2 >= width) {
                            x2 = x2 - width;
                            if (x2 >= width) break;
                        }
                        // Pano Y: pole mirror
                        if (y2 < 0) {
                            y2 = -y2;
                            if (y2 >= HLo) break;
                        } else if (y2 >= height) {
                            y2 = MH2 - y2;
                            if (y2 < HHi) break;
                        }

                        {
                            // Match the original SSE sample: RSQRTSS, two
                            // ordered multiplies, RCPSS, then signed saturation.
                            float invSqrt = numeric::reciprocalSqrtEstimate(distSqr);
                            float st = static_cast<float>(static_cast<int>(atLevel[y2 * width + x2]) - zp0) * invSqrt;
                            float num1 = st * sit;
                            float invDenom = numeric::reciprocalEstimate(st * st + sZRT);
                            float num2 = num1 * sZRT;
                            int it = PascalRoundToInt(std::clamp(num2 * invDenom, -32768.0f, 32767.0f));
                            // PS advances while Pascal's SSE iDir counts down.
                            int angleIndex = 31 - iDir;
                            if (psm[angleIndex] < it)
                                psm[angleIndex] = static_cast<int16_t>(it);
                        }

                        sx += ATdirSteps[iDir][0] * sstepVal;
                        sy += ATdirSteps[iDir][1] * sstepVal;
                    }
                }

                if (bSummUp) {
                    float stSum = CalcAOHiQSumPasLike(psm);
                    long addVal = std::max(0L, std::min(static_cast<long>(std::lround(smin)),
                                                        static_cast<long>(PascalRoundToInt(stSum * sMul))));
                    if (addVal < 0) addVal = 0;
                    psi->AmbShadow = static_cast<uint16_t>(std::min(16383, static_cast<int>(psi->AmbShadow) + static_cast<int>(addVal)));
                }
            }
        }

        if (currentLevel < numLevels)
            NextATlevelHiQ(atLevel, width, height, 1 << currentLevel);
    }
}


// ############################################################################
// TAmbVideo stochastic SSAO
// Ported from AmbHiQ.pas TAmbVideo.Execute
// Single wavelet level, 32-direction stochastic sampling with LCG seed.
// Uses MSVC-style LCG (214013 * seed + 2531011) for per-sample jitter.
// Boundary: pano-style X wrap + Y pole mirror.
// ############################################################################

static void CalcAO_Video(
    TsiLight5* siLight, int width, int height,
    const TMandHeader10* header,
    int threadId = 1)
{
    const auto& ATdirSteps=EnsureATdirSteps();

    double Zcorr_local, ZcMul_local, ZZstmitDif_local;
    TMandHeader10 headerCopy = *header;
    CalcPPZvals(headerCopy, Zcorr_local, ZcMul_local, ZZstmitDif_local);

    float aZScaleFactor = static_cast<float>((std::pow(256.0 / ZcMul_local + 1.0, 2.0) - 1.0) / Zcorr_local);
    float borderMirror = MinCS(header->bSSAO24BorderMirrorSize * 0.01f, 0.9f);

    int MH2 = 2 * (height - 1);
    int MW2 = 2 * (width - 1);
    int WLo = PascalRoundToInt(static_cast<float>(width) * borderMirror);
    int WHi = width - 1 - WLo;
    int HLo = PascalRoundToInt(static_cast<float>(height) * borderMirror);
    int HHi = height - 1 - HLo;

    // sit = aZScaleFactor / 22000.0 * 4096 * 0.00001
    float sit = aZScaleFactor / 22000.0f * 4096.0f * 0.00001f;

    // Pascal (post-T12): seed := PasLcgInitFromThread(ASCpar.aThreadId)
    uint32_t seed = mb3d::PasLcgInitFromThread(threadId);

    int stepCount = PascalRoundToInt(std::sqrt(static_cast<float>(width * width + height * height)) * 0.1f) + 1;
    float sMul = 16383.0f / (32.0f * stepCount);

    // psi2 base pointer for reading neighbor Z values
    // In Pascal: psi2 := PWord(Integer(ASCpar.aPsiLight) + 6)
    // This points to RoughZposFine of the first pixel.

    for (int y = 0; y < height; y++) {
        RenderPoll();
        for (int x = 0; x < width; x++) {
            RenderWork();
            int pixIdx = y * width + x;
            TsiLight5* psi = &siLight[pixIdx];

            uint32_t rawCheck;
            std::memcpy(&rawCheck, &psi->RoughZposFine, 4);
            if ((rawCheck & 0x80000000u) != 0) continue;

            // MakeSVecFromNormals -- extract surface normal from pixel
            TSVec sv;
            MakeSVecFromNormalsD(sv, psi);

            int zp = static_cast<int>((rawCheck & 0xFFFFFF00u) >> 1);
            int tA = 0;

            for (int iDir = 0; iDir < 32; iDir++) {
                float sx = ATdirSteps[iDir][0] * 2.5f;
                float sy = ATdirSteps[iDir][1] * 2.0f;

                for (int sc = 0; sc < stepCount; sc++) {
                    RenderWork();
                    seed = 214013 * seed + 2531011;
                    int x2 = PascalRoundToInt(sx - 1.5f) + ((seed >> 16) & 3);
                    int y2 = PascalRoundToInt(sy - 1.5f) + ((seed >> 10) & 3);
                    float distSqr = static_cast<float>(x2 * x2 + y2 * y2);
                    if (distSqr == 0.0f) {
                        sx += ATdirSteps[iDir][0] * 2.0f;
                        sy += ATdirSteps[iDir][1] * 2.0f;
                        continue;
                    }

                    x2 += x;
                    y2 += y;

                    // Pano-style X wrap (same boundary as TAmbVideo in Pascal)
                    if (x2 < 0) {
                        x2 = x2 + width;
                        if (x2 < 0) goto video_next_dir;
                    } else if (x2 >= width) {
                        x2 = x2 - width;
                        if (x2 >= width) goto video_next_dir;
                    }
                    // Y: pole mirror
                    if (y2 < 0) {
                        y2 = -y2;
                        if (y2 >= HLo) goto video_next_dir;
                    } else if (y2 >= height) {
                        y2 = MH2 - y2;
                        if (y2 < HHi) goto video_next_dir;
                    }

                    {
                        // Read neighbor Z from siLight buffer directly
                        // Pascal: st2 := (((PCardinal(Integer(psi2) + (y2 * MWidth + x2) * 18)^ and $FFFFFF00) shr 1) - zp) * sit
                        int neighborIdx = y2 * width + x2;
                        uint32_t neighborRaw;
                        std::memcpy(&neighborRaw, &siLight[neighborIdx].RoughZposFine, 4);
                        float st2 = static_cast<float>(static_cast<int>((neighborRaw & 0xFFFFFF00u) >> 1) - zp) * sit;

                        // if st2 * sv[2] + sx * sv[0] + sy * sv[1] < 0 then Inc(tA)
                        if (st2 * sv[2] + sx * sv[0] + sy * sv[1] < 0.0f)
                            tA++;
                    }

                    sx += ATdirSteps[iDir][0] * 2.0f;
                    sy += ATdirSteps[iDir][1] * 2.0f;
                }
                video_next_dir:;
            }

            psi->AmbShadow = static_cast<uint16_t>(PascalRoundToInt(tA * sMul));
        }
    }
}


// ############################################################################
// TYPE 3: DE-based AO
// Ported from CalcAmbShadowDE.pas TCalcAmbShadowDEThreadGeneral.Execute
//
// Casts rays in hemisphere above surface normal, uses distance estimation
// to determine ambient occlusion based on closest approach.
// Quality levels: 0=3 rays, 1=6 rays, 2=16 rays, 3=32 rays
// ############################################################################

static void CalcAO_DE(
    TsiLight5* siLight, int width, int height,
    int slOffset,
    TMCTparameter* mcp,
    int quality,    // 0..3
    int dither,     // AODEdithering: 0, 1, 2
    bool firstStepRandom,  // bit 8 of bCalcAmbShadowAutomatic
    int y_start = 0, int y_end = -1,
    int x_start = 0, int x_end = -1)
{
    if (y_end < 0) y_end = height;
    if (x_end < 0) x_end = width;
    TIteration3Dext it3Dext;
    std::memset(&it3Dext, 0, sizeof(TIteration3Dext));
    mcp->pIt3Dext = &it3Dext;
    IniIt3D(mcp, &it3Dext);

    float origDEstop = mcp->DEstop;
    mcp->msDEstop = mcp->DEstop;
    it3Dext.CalcSIT = 0;
    mcp->iDEAddSteps = 5;
    LongBool bInsideTmp = mcp->bInsideRendering;
    LongBool origIsIFS = mcp->bIsIFS;
    mcp->bIsIFS = 0; // test, too dark with inside rendering+ifs

    // Pascal CalcAmbShadowDE.pas (post-T07): seed := PasLcgInitFromThread(iThreadId)
    uint32_t seed = mb3d::PasLcgInitFromThread(std::max(1, static_cast<int>(mcp->iThreadId)));

    // Build rotation matrix for stepwidth-scaled VGrads
    TSMatrix3 RotV;
    TMatrix3 packedGrads;
    std::memcpy(packedGrads, mcp->Vgrads, sizeof(packedGrads));
    NormaliseMatrixToS(static_cast<float>(mcp->StepWidth), packedGrads, RotV);
    float sDT1mul = static_cast<float>(mcp->StepWidth) * 0.5f;

    // Build ray vectors for the hemisphere
    int rayCount = 0;
    TSVec rotM[33];
    int rowCount[5] = {0};
    float ABR, DStepMul, CorrectionWeight;
    double dMinADif;

            if (quality == 0) {
                ABR = 60.0f * static_cast<float>(Pid180);
                for (int i = 0; i < 3; i++) {
                    TSMatrix3 RotW;
                    BuildRotMatrixS(0, 0.5f * ABR, i * static_cast<float>(PiM2) * s1d3f, RotW);
                    ScaleSVector(rotM[rayCount], RotW[2], -1.0f);
                    rayCount++;
                }
                DStepMul = 1.8f;
                dMinADif = -1.0;
                CorrectionWeight = 0.3f;
            } else {
                TSMatrix3 RotW;
                BuildRotMatrixS(0, 0, 0, RotW);
                ScaleSVector(rotM[0], RotW[2], -1.0f);
                rayCount = 1;
                ABR = static_cast<float>(Pi) * 0.5f / (quality + 0.9f);
                for (int row = 1; row <= quality; row++) {
                    float rowAngle = row * ABR;
                    int count = std::lround(std::sin(rowAngle) * Pi * 2.0 / ABR);
                    rowCount[row] = count;
                    for (int i = 0; i < count; i++) {
                        BuildRotMatrixS(0, rowAngle, i * 2.0f * static_cast<float>(Pi) / count, RotW);
                        ScaleSVector(rotM[rayCount], RotW[2], -1.0f);
                        rayCount++;
                    }
                }
                DStepMul = 1.0f + std::sin(ABR);
                dMinADif = std::cos(ABR * 1.2f);
                CorrectionWeight = (quality == 1) ? 0.2f : 0.1666f;
            }

            float sMaxD = mcp->DEAOmaxL * 0.5f * std::sqrt(static_cast<float>(
                mcp->iMandHeight * mcp->iMandHeight + mcp->iMandWidth * mcp->iMandWidth));

    for (int y = y_start; y < y_end; y++) {
        RenderPoll();
        const int screenY = y + mcp->CalcRect.Top;
        mcp->CAFY = (static_cast<double>(screenY) / mcp->iMandHeight - 0.5) * mcp->FOVy;

        for (int x = x_start; x < x_end; x++) {
            RenderWork();
            TsiLight5* psi = &siLight[y * slOffset + x];
            const bool tracePixel = HasParityTraceContext() &&
                                    x == GetParityTracePixelX() &&
                                    y == GetParityTracePixelY();
            psi->AmbShadow = 0;

            if (psi->Zpos >= 32768 || psi->SIgradient >= 32768) continue;

            const int screenX = x + mcp->CalcRect.Left;
            RMCalculateVgradsFOV(mcp, screenX + 1);
            RMCalculateStartPos(mcp, screenX, screenY);
            it3Dext.CalcSIT = 0;

            // Reconstruct Z position
            uint32_t packedZ;
            std::memcpy(&packedZ, &psi->RoughZposFine, 4);
            packedZ >>= 8;
            double ZZ = (std::pow((8388351.5 - static_cast<double>(packedZ)) / mcp->ZcMul + 1.0, 2.0) - 1.0) / mcp->Zcorr;
            AddPackedVecWeight(&it3Dext.C1, mcp->mVgradsFOV, ZZ);
            mcp->msDEstop = origDEstop * (1.0f + static_cast<float>(ZZ) * mcp->mctDEstopFactor);

            if (mcp->bInAndOutside) {
                mcp->bInsideRendering = ((psi->OTrap & 0x8000) == 0) ? 1 : 0;
            } else {
                mcp->bInsideRendering = bInsideTmp;
            }
            mcp->bCalcInside = mcp->bInsideRendering;

            // Calculate DE at surface
            double dTmp = mcp->CalcDE(&it3Dext, mcp);
            if (dTmp > mcp->msDEstop * 2.0) dTmp = mcp->msDEstop * 2.0;

            bool DElimited = (it3Dext.ItResultI < mcp->MaxItsResult) || (dTmp < mcp->msDEstop);
            if (tracePixel) {
                std::ostringstream payload;
                payload << "ZZ=" << ZZ
                        << " msDEstop=" << mcp->msDEstop
                        << " dTmp=" << dTmp
                        << " ItResultI=" << it3Dext.ItResultI
                        << " DElimited=" << (DElimited ? -1 : 0)
                        << " bInsideRendering=" << ((mcp->bInsideRendering != 0) ? -1 : 0)
                        << " bCalcInside=" << ((mcp->bCalcInside != 0) ? -1 : 0);
                AppendParityTrace("ao_de_enter", payload.str());
            }
            if (DElimited) {
                // Binary search to find exact surface
                uint32_t packedZ2;
                std::memcpy(&packedZ2, &psi->RoughZposFine, 4);
                packedZ2 >>= 8;
                double dT1 = (std::pow((8388351.9 - static_cast<double>(packedZ2)) / mcp->ZcMul + 1.0, 2.0) - 1.0) / mcp->Zcorr - ZZ;
                int itmp = 5;
                while (itmp > 0 && std::fabs(dTmp - mcp->msDEstop) > 0.004) {
                    RenderPoll();
                    ZZ += dT1;
                    AddPackedVecWeight(&it3Dext.C1, mcp->mVgradsFOV, dT1);
                    mcp->msDEstop = origDEstop * (1.0f + static_cast<float>(ZZ) * mcp->mctDEstopFactor);
                    itmp--;
                    if (itmp > 0) {
                        dTmp = mcp->CalcDE(&it3Dext, mcp);
                        if (dTmp < mcp->msDEstop) dT1 = std::fabs(dT1) * -0.55;
                        else dT1 = std::fabs(dT1) * 0.55;
                    }
                }
            } else {
                RMdoBinSearchIt(mcp, &ZZ);
                mcp->msDEstop = origDEstop * (1.0f + static_cast<float>(std::fabs(ZZ)) * mcp->mctDEstopFactor);
            }

            mcp->msDEstop = MinCS(mcp->msDEstop, 1e6f);

            TVec3D IC;
            mCopyVec(IC, reinterpret_cast<TVec3D&>(it3Dext.C1));
            float StepAO = mcp->msDEstop / origDEstop;
            double MaxDist = sMaxD * std::sqrt(StepAO);

            if (mcp->bVaryDEstop) {
                if (!mcp->bInsideRendering || mcp->bIsIFS)
                    mcp->msDEstop = mcp->msDEstop / (DStepMul * DStepMul);
            } else {
                mcp->msDEstop = origDEstop / (DStepMul * DStepMul);
            }

            // Build local ray vectors with dithering
            int localRayCount = rayCount;
            TSVec localRotM[33];
            std::memcpy(localRotM, rotM, sizeof(TSVec) * rayCount);

            if (dither > 0) {
                float dT1_d = static_cast<float>(screenY % (dither + 1)) * 0.5f / dither;
                float dT2_d = static_cast<float>(screenX % (dither + 1)) * 0.5f / dither;
                if (quality == 0) {
                    for (int i = 0; i < 3; i++) {
                        TSMatrix3 RotW;
                        BuildRotMatrixS(0, (dT1_d + 0.5f) * 50.0f * static_cast<float>(Pid180),
                                       (i + dT2_d) * static_cast<float>(PiM2) * s1d3f, RotW);
                        ScaleSVector(localRotM[i], RotW[2], -1.0f);
                    }
                } else {
                    if (dT1_d > 0.1f) localRayCount = 1; else localRayCount = 0;
                    float ABR_d = static_cast<float>(Pi) * 0.5f / (quality + 0.9f);
                    TSMatrix3 RotW;
                    BuildRotMatrixS(0, 0, 0, RotW);
                    ScaleSVector(localRotM[0], RotW[2], -1.0f);
                    for (int row = 1; row <= quality; row++) {
                        int count = rowCount[row];
                        for (int i = 0; i < count; i++) {
                            BuildRotMatrixS(0, ABR_d * (row + dT1_d - 0.25f),
                                           (i + dT2_d) * static_cast<float>(PiM2) / count, RotW);
                            ScaleSVector(localRotM[localRayCount], RotW[2], -1.0f);
                            localRayCount++;
                        }
                    }
                }
            }

            float DEmul = std::sqrt(localRayCount * 0.5f);
            float ABR_calc = 1.2f / static_cast<float>(ArcSinSafe(1.0 / DEmul));
            float MDd10 = 0.1f / (static_cast<float>(MaxDist) * DEmul);

            if (mcp->bInsideRendering && !mcp->bIsIFS) DEmul = 1.0f;

            if (tracePixel) {
                std::ostringstream payload;
                payload << "ZZ=" << ZZ
                        << " msDEstop=" << mcp->msDEstop
                        << " StepAO=" << StepAO
                        << " MaxDist=" << MaxDist
                        << " quality=" << quality
                        << " dither=" << dither
                        << " RayCount=" << localRayCount
                        << " DStepMul=" << DStepMul
                        << " DEmul=" << DEmul
                        << " ABRcalc=" << ABR_calc
                        << " MDd10=" << MDd10;
                AppendParityTrace("ao_de_surface_refine", payload.str());
            }

            // Build rotation from normal to world space
            TSVec svNormal;
            MakeSVecFromNormalsD(svNormal, psi);
            NormaliseSVectorVar(svNormal);
            TQuaternion quat;
            MakeRotQuatFromSNormals(svNormal, quat);
            TSMatrix3 RotW;
            CreateSMatrixFromQuat(RotW, quat);

            float minRA[33];
            float sAdd[33];

            for (int rayIdx = 0; rayIdx < localRayCount; rayIdx++) {
                RenderPoll();
                TSVec SVec;
                std::memcpy(SVec, localRotM[rayIdx], sizeof(TSVec));
                TSVec localVec;
                std::memcpy(localVec, SVec, sizeof(TSVec));
                RotateSVectorReverseS(SVec, RotW);  // rotate to normal space
                TSVec normalVec;
                std::memcpy(normalVec, SVec, sizeof(TSVec));
                RotateSVectorS(SVec, RotV);          // rotate to world space
                if (tracePixel) {
                    std::ostringstream payload;
                    payload << "idx=" << rayIdx
                            << " localX=" << localVec[0]
                            << " localY=" << localVec[1]
                            << " localZ=" << localVec[2]
                            << " normalX=" << normalVec[0]
                            << " normalY=" << normalVec[1]
                            << " normalZ=" << normalVec[2]
                            << " worldX=" << SVec[0]
                            << " worldY=" << SVec[1]
                            << " worldZ=" << SVec[2];
                    AppendParityTrace("ao_de_ray_vec", payload.str());
                }

                double dT1 = StepAO * DStepMul;
                float sTmp = 1.0f;
                bool bEnd = false;
                bool bFirstStep = firstStepRandom;

                do {
                    if (bFirstStep) {
                        bFirstStep = false;
                        // Pascal CalcAmbShadowDE.pas (T07): dT1 := dT1 * (PasLcgU01(lcgState) * 1.5 + s05)
                        dT1 = dT1 * (mb3d::PasLcgU01(seed) * 1.5 + 0.5);
                    } else if (dT1 > MaxDist) {
                        dT1 = MaxDist;
                        bEnd = true;
                    }

                    CopyAddSVecWeight(reinterpret_cast<TVec3D&>(it3Dext.C1), IC, SVec, dT1);

                    if (mcp->iCutOptions > 0 && AtCutPlane2(mcp, reinterpret_cast<TVec3D&>(it3Dext.C1), dT1 * sDT1mul))
                        break;

                    double dT2 = mcp->CalcDE(&it3Dext, mcp);

                    if (mcp->bInsideRendering && !mcp->bIsIFS) {
                        if (dT2 < mcp->msDEstop) {
                            float ratio = static_cast<float>(dT1 / MaxDist);
                            sTmp = ratio * ratio * ratio * ratio; // Sqr(Sqr(x))
                            break;
                        }
                        dT1 = dT1 + dT1 * DStepMul * 0.1 + mcp->msDEstop;
                    } else {
                        sTmp = MinCS(sTmp, static_cast<float>((dT2 - mcp->msDEstop + dT1 * MDd10) / dT1));
                        if (sTmp < s002f) break;
                        dT1 = dT1 + std::max(dT2, dT1 * static_cast<double>(DStepMul));
                    }
                } while (!bEnd);

                minRA[rayIdx] = Max0S(sTmp) * DEmul;
                if (tracePixel) {
                    std::ostringstream payload;
                    payload << "idx=" << rayIdx
                            << " minRA=" << minRA[rayIdx]
                            << " sTmp=" << sTmp
                            << " endDist=" << dT1;
                    AppendParityTrace("ao_de_ray_final", payload.str());
                }
            }

            // Compute final AO value
            double dAmount = 0.0;

            if (mcp->bInsideRendering && !mcp->bIsIFS) {
                for (int i = 0; i < localRayCount; i++)
                    dAmount += minRA[i];
            } else {
                // Apply correction: for each ray, add contribution from neighbors
                for (int i = 0; i < localRayCount; i++) {
                    sAdd[i] = 0.0f;
                    if (minRA[i] < 1.0f) {
                        float maxAdd = 1.0f - minRA[i];
                        for (int j = 0; j < localRayCount; j++) {
                            RenderWork();
                            if (j == i) continue;
                            float dot = DotOfSVectors(localRotM[i], localRotM[j]);
                            if (dot > dMinADif) {
                                float overlap = minRA[j] - static_cast<float>(ArcCosSafe(dot)) * ABR_calc + 1.0f;
                                if (overlap > 0)
                                    sAdd[i] += MinCS(maxAdd, overlap) * CorrectionWeight;
                            }
                        }
                    }
                    if (tracePixel) {
                        std::ostringstream payload;
                        payload << "idx=" << i
                                << " minRA=" << minRA[i]
                                << " sAdd=" << sAdd[i];
                        AppendParityTrace("ao_de_correction", payload.str());
                    }
                }
                for (int i = 0; i < localRayCount; i++)
                    dAmount += MinCS(1.0f, sAdd[i] + minRA[i]);
            }

            int ambVal = std::max(0, static_cast<int>(std::lround(16383.0 * (1.0 - dAmount / localRayCount))));
            psi->AmbShadow = static_cast<uint16_t>(ambVal);
            if (tracePixel) {
                std::ostringstream payload;
                payload << "dAmount=" << dAmount
                        << " RayCount=" << localRayCount
                        << " AmbShadow=" << psi->AmbShadow;
                AppendParityTrace("ao_de_finish", payload.str());
            }
        }
    }

    // Restore
    mcp->bInsideRendering = bInsideTmp;
    mcp->bCalcInside = bInsideTmp;
    mcp->bIsIFS = origIsIFS;
    mcp->msDEstop = origDEstop;
    mcp->DEstop = origDEstop;
}


// ############################################################################
// CalcAmbientOcclusion -- main dispatcher
// ############################################################################

void CalcAmbientOcclusion(
    TsiLight5* siLight,
    int width, int height,
    int slOffset,
    TMCTparameter* mcp,
    TLightVals* lvals,
    int aoType,
    int aoQuality,
    const TMandHeader10* header,
    int logicalThreadCount)
{
    RenderPoll();
    if (!siLight || width <= 0 || height <= 0) return;

    // DE-based AO is dispatched separately (bits 2-3 == 3, i.e., (byte & 12) == 12)
    // It does not go through the HiQ ATi dispatch path.
    if (header && (header->bCalcAmbShadowAutomatic & 12) == 12 && mcp) {
        int dither = header->AODEdithering;
        bool fsr = (header->bCalcAmbShadowAutomatic & 128) != 0;
        CalcAO_DE(siLight, width, height, slOffset, mcp, aoQuality, dither, fsr);
        return;
    }

    // ATi dispatch: matches Pascal CalcAmbShadowTHiQ (AmbHiQ.pas lines 149-209)
    int ATi = header ? ((header->bCalcAmbShadowAutomatic >> 1) & 7) : 0;
    bool isPano = header && (header->bPlanarOptic == 2);

    // Remap ATi=6 -> TAmbVideo (case 10), else if pano: ATi += 4 for pano variants
    if (ATi == 6) {
        ATi = 10;
    } else if (isPano && ATi >= 2 && ATi <= 5) {
        ATi += 4;
    }

    if (HasParityTraceContext()) {
        std::ostringstream payload;
        payload << "ATi=" << ATi << " isPano=" << (isPano ? 1 : 0);
        AppendParityTrace("ao_dispatch", payload.str());
    }

    switch (ATi) {
        case 0:
            // CalcAmbShadowT, bT0calc = False -> TAmbShadowCalc.Execute
            CalcAO_Type0(siLight, width, height, header);
            break;

        case 1:
            // CalcAmbShadowT, bT0calc = True -> TAmbShadowCalcT0.Execute
            CalcAO_Type0_T0(siLight, width, height, header);
            break;

        case 2:
            // ATi=2: HiQ AO (AmbHiQ.pas TAmbHiQCalc)
            CalcAO_HiQ(siLight, width, height, header);
            break;

        case 3:
            // ATi=3: HiQ threshold AO (AmbHiQ.pas TAmbHiQCalcT0)
            CalcAO_HiQ_Threshold(siLight, width, height, header);
            break;

        case 4:
            // ATi=4: HiQ random AO (AmbHiQ.pas TAmbHiQCalcR)
            CalcAO_HiQ_Random(siLight, width, height, header, logicalThreadCount);
            break;

        case 5:
            // ATi=5: HiQ random threshold AO (AmbHiQ.pas TAmbHiQCalcRT0)
            CalcAO_HiQ_RandomThreshold(siLight, width, height, header, logicalThreadCount);
            break;

        case 6:
            // ATi=6: Pano HiQ standard (AmbHiQ.pas TAmbHiQCalcPano)
            CalcAO_HiQ_Pano(siLight, width, height, header);
            break;

        case 7:
            // ATi=7: Pano HiQ T0 threshold (AmbHiQ.pas TAmbHiQCalcT0Pano)
            CalcAO_HiQ_T0_Pano(siLight, width, height, header);
            break;

        case 8:
            // ATi=8: Pano HiQ random (AmbHiQ.pas TAmbHiQCalcRPano)
            CalcAO_HiQ_Random_Pano(siLight, width, height, header, logicalThreadCount);
            break;

        case 9:
            // ATi=9: Pano HiQ random T0 (AmbHiQ.pas TAmbHiQCalcRT0Pano)
            CalcAO_HiQ_RandomT0_Pano(siLight, width, height, header, logicalThreadCount);
            break;

        case 10:
            // ATi=10: TAmbVideo stochastic SSAO (AmbHiQ.pas TAmbVideo)
            CalcAO_Video(siLight, width, height, header);
            break;

        default:
            break;
    }
}

void CalcAmbientOcclusionRows(
    TsiLight5* siLight,
    int width, int height,
    int slOffset,
    TMCTparameter* mcp,
    TLightVals* lvals,
    int aoType,
    int aoQuality,
    const TMandHeader10* header,
    int y_start, int y_end)
{
    RenderPoll();
    if (!siLight || width <= 0 || height <= 0) return;
    if (y_start >= y_end) return;

    bool isDeAO = header && (header->bCalcAmbShadowAutomatic & 12) == 12;

    // Only DE-based AO supports row-range parallelism.
    // Other modes are image-space operations that need the full buffer.
    if (isDeAO && mcp) {
        int dither = 0;
        bool fsr = false;
        if (header) {
            dither = header->AODEdithering;
            fsr = (header->bCalcAmbShadowAutomatic & 128) != 0;
        }
        CalcAO_DE(siLight, width, height, slOffset, mcp, aoQuality, dither, fsr, y_start, y_end);
    } else {
        // For types 0-2, only run if this is the full range (y_start==0, y_end==height).
        // The caller should not split these types.
        if (y_start == 0 && y_end == height) {
            CalcAmbientOcclusion(siLight, width, height, slOffset, mcp, lvals, aoType, aoQuality, header);
        }
    }
}

void ReplayAmbientOcclusion(
    TsiLight5* siLight,
    int width, int height,
    int slOffset,
    TMCTparameter* mcp,
    TLightVals* lvals,
    int aoType,
    int aoQuality,
    const TMandHeader10* header,
    int threadCount)
{
    RenderPoll();
    if (!siLight || width <= 0 || height <= 0 || !header) return;

    const bool isDeAO = (header->bCalcAmbShadowAutomatic & 12) == 12;
    const bool hiqAo = (header->bCalcAmbShadowAutomatic & 12) == 8;
    const int aoIterations = (hiqAo && header->SSAORcount > 1) ? static_cast<int>(header->SSAORcount) : 1;
    threadCount = std::max(1, threadCount);

    if (hiqAo) {
        for (int i = 0; i < width * height; ++i) {
            RenderWork();
            siLight[i].AmbShadow = 0;
        }
    }

    for (int aoIter = 0; aoIter < aoIterations; ++aoIter) {
        RenderPoll();
        if (isDeAO && mcp && threadCount > 1) {
            RunParallelWorkers(threadCount, [=](int t) {
                const int yStart = (height * t) / threadCount;
                const int yEnd = (height * (t + 1)) / threadCount;
                    TMCTparameter localMct = *mcp;
                    TLightVals localLV;
                    std::memcpy(&localLV, lvals, sizeof(TLightVals));
                    TLValigned localAligned;
                    if (lvals && lvals->PLValigned) {
                        localAligned = *lvals->PLValigned;
                        localLV.PLValigned = &localAligned;
                    } else {
                        std::memset(&localAligned, 0, sizeof(TLValigned));
                        localLV.PLValigned = &localAligned;
                    }
                    CalcAmbientOcclusionRows(siLight, width, height, slOffset,
                                             &localMct, &localLV, aoType, aoQuality,
                                             header, yStart, yEnd);
            });
        } else {
            CalcAmbientOcclusion(siLight, width, height, slOffset, mcp, lvals, aoType, aoQuality, header, threadCount);
        }
    }
}

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
    int pixelY)
{
    RenderPoll();
    if (!siLight || !header) return;
    if (pixelX < 0 || pixelX >= width || pixelY < 0 || pixelY >= height) return;

    const int aoMode = (header->bCalcAmbShadowAutomatic >> 1) & 7;
    if (HasParityTraceContext()) {
        const TsiLight5& psi = siLight[pixelY * width + pixelX];
        std::ostringstream payload;
        payload << "aoMode=" << aoMode
                << " aoType=" << aoType
                << " aoQuality=" << aoQuality
                << " width=" << width
                << " height=" << height
                << " pixelX=" << pixelX
                << " pixelY=" << pixelY
                << " NormalX=" << psi.NormalX
                << " NormalY=" << psi.NormalY
                << " NormalZ=" << psi.NormalZ
                << " RoughZposFine=" << psi.RoughZposFine
                << " Zpos=" << psi.Zpos
                << " Shadow=" << psi.Shadow
                << " AmbShadow=" << psi.AmbShadow
                << " SIgradient=" << psi.SIgradient
                << " OTrap=" << psi.OTrap;
        AppendParityTrace("ao_replay_begin", payload.str());
    }
    if (aoMode == 6 && mcp) {
        int dither = header->AODEdithering;
        bool fsr = (header->bCalcAmbShadowAutomatic & 128) != 0;
        TMCTparameter localMct = *mcp;
        localMct.iThreadId = 1;
        CalcAO_DE(siLight, width, height, slOffset, &localMct, aoQuality, dither, fsr,
                  pixelY, pixelY + 1, pixelX, pixelX + 1);
        return;
    }

    std::vector<TsiLight5> scratch(static_cast<size_t>(width) * static_cast<size_t>(height));
    std::memcpy(scratch.data(), siLight, scratch.size() * sizeof(TsiLight5));
    ReplayAmbientOcclusion(scratch.data(), width, height, slOffset, mcp, lvals, aoType, aoQuality, header, 1);
    siLight[pixelY * width + pixelX].AmbShadow = scratch[static_cast<size_t>(pixelY) * width + pixelX].AmbShadow;
}

// ============================================================================
// SmoothH -- horizontal wavelet smoothing pass
// Ported from AmbHiQ.pas: procedure SmoothH
// Pascal pseudocode (from commented-out Pascal):
//   for x := 0 to ya do
//     a := max(0, x - Step);
//     b := min(ya, x + Step);
//     PIA[x] := (PIA[x] + (SA[a] + PIA[b]) shr 1) shr 1;
// ============================================================================

void SmoothH(uint32_t* PIA, const uint32_t* SA, int ya, int Step) {
    for (int x = 0; x <= ya; x++) {
        RenderWork();
        int a = std::max(0, x - Step);
        int b = std::min(ya, x + Step);
        PIA[x] = (PIA[x] + (SA[a] + PIA[b]) / 2) / 2;
    }
}

// ============================================================================
// SmoothV -- vertical wavelet smoothing pass
// Ported from AmbHiQ.pas: procedure SmoothV
// Pascal pseudocode:
//   ya := 0;
//   for y := 0 to ye do
//     a := max(0, y - Step);
//     b := min(ye, y + Step);
//     PIA[ya] := (SA[y] + (SA[a] + SA[b]) shr 1) shr 1;
//     Inc(ya, wid);
// widBytes is in bytes = 4 * image_width for uint32 arrays
// ============================================================================

void SmoothV(uint32_t* PIA, const uint32_t* SA, int ye, int Step, int widBytes) {
    int strideU32 = widBytes / 4; // Convert byte stride to uint32 stride
    int outIdx = 0;
    for (int y = 0; y <= ye; y++) {
        RenderWork();
        int a = std::max(0, y - Step);
        int b = std::min(ye, y + Step);
        PIA[outIdx] = (SA[y] + (SA[a] + SA[b]) / 2) / 2;
        outIdx += strideU32;
    }
}

// ============================================================================
// AmbSmooth -- post-processing smoothing pass for AO results
// Ported from AmbHiQ.pas: procedure AmbSmooth
//
// The Pascal AmbSmooth has empty loop bodies for the horizontal and vertical
// smoothing passes (they were planned but never implemented). The only active
// part is the final summation pass (when aCurrentLevel == aATlevelCount),
// which converts the 32-direction angular max arrays into per-pixel AmbShadow
// values using ArcTan accumulation.
// ============================================================================

void AmbSmooth(
    TsiLight5* siLight,
    const int16_t* angMaxArr,
    int width, int height,
    int yStart, int yEnd,
    int levelCount,
    float threshold)
{
    RenderPoll();
    float sMul = 1.5f * 32767.0f /
        (static_cast<float>(Pi) * 32.0f *
         std::pow(std::atan(threshold * 0.7f *
                  std::pow(std::sqrt(std::sqrt(static_cast<float>(levelCount))), 1.0f)),
                  0.9f));

    for (int y = yStart; y <= yEnd; y++) {
        RenderPoll();
        int apos = y * width;
        for (int x = 0; x < width; x++) {
            RenderWork();
            int pixelIdx = y * width + x;
            const int16_t* psi = &angMaxArr[(apos + x) * 32];

            float st = 0.0f;
            for (int dir = 0; dir < 32; dir++) {
                if (psi[dir] > -32768) {
                    st += std::atan(static_cast<float>(psi[dir]) * static_cast<float>(d00002441406));
                }
            }

            uint16_t val = static_cast<uint16_t>(
                std::max(0L, std::min(16383L, std::lround(st * sMul))));

            // Write to AmbShadow field (offset 6 words = 12 bytes from start)
            siLight[pixelIdx].AmbShadow = val;
        }
    }
}

} // namespace mb3d
