#include "core/render_control.h"
#include "core/render_context.h"
#include "core/reflection.h"
#include "core/ray_marcher.h"
#include "core/math3d.h"
#include "core/packed_math.h"
#include "core/header_trafos.h"
#include "core/colorizer.h"
#include "core/shadow.h"
#include "core/ambient_occlusion.h"
#include "core/parity_trace.h"
#include "core/thread_pool.h"
#include <cmath>
#include <cstring>
#include <algorithm>
#include <cstdio>
#include <sstream>
#include <iomanip>
#include <thread>
#include <vector>
#include <fstream>
#include <stdexcept>

namespace mb3d {

float GetVolLightMapValue(const TVec3D& direction);



static bool DebugPrintSrPrimaryToStderr() {
    const char* spec = std::getenv("MB3D_DEBUG_PRINT_SR_PRIMARY");
    return spec && *spec && std::strcmp(spec, "0") != 0;
}

static bool DebugStopAfterSrPrimary() {
    const char* spec = std::getenv("MB3D_DEBUG_STOP_AFTER_SR_PRIMARY");
    return spec && *spec && std::strcmp(spec, "0") != 0;
}

static bool DebugPrintSrRecursionToStderr() {
    const char* spec = std::getenv("MB3D_DEBUG_PRINT_SR_RECURSION");
    return spec && *spec && std::strcmp(spec, "0") != 0;
}

static bool DebugStopAfterSrRecurseReady() {
    const char* spec = std::getenv("MB3D_DEBUG_STOP_AFTER_SR_RECURSE_READY");
    return spec && *spec && std::strcmp(spec, "0") != 0;
}

static bool DebugStopAfterSrRayExit() {
    const char* spec = std::getenv("MB3D_DEBUG_STOP_AFTER_SR_RAY_EXIT");
    return spec && *spec && std::strcmp(spec, "0") != 0;
}

static void SR_TraceStage(const char* stage, const std::string& payload) {
    if (!HasParityTraceContext() || !ShouldLogParityStage(stage)) return;
    AppendParityTrace(stage, payload);
}

// ============================================================================
// Constants -- use sr_ prefix for constants not in math3d.h
// ============================================================================

static constexpr float  sr_s025    = 0.25f;
static constexpr float  sr_s03     = 0.3f;
static constexpr float  sr_s059    = 0.59f;
static constexpr float  sr_s1em10  = 1e-10f;
static constexpr float  sr_s1em30  = 1e-30f;
static constexpr float  sr_s1em4   = 1e-4f;

// ============================================================================
// Helper: GetZPosFromSI
// ============================================================================

static double SR_GetZPosFromSI(int iz, double ZcMul, double Zcorr) {
    double v = (8388351.5 - static_cast<double>(iz)) / ZcMul + 1.0;
    return (v * v - 1.0) / Zcorr;
}

// ============================================================================
// Helper: DVecToSVec (inline, avoids calling the math3d version which
// has a different signature)
// ============================================================================

static void SR_DVecToSVec(TSVec& result, double x, double y, double z) {
    result[0] = static_cast<float>(x);
    result[1] = static_cast<float>(y);
    result[2] = static_cast<float>(z);
    result[3] = 0.0f;
}

// ============================================================================
// Original SVecPow calls Math.Power directly, including 0^0 and tiny bases.
// ============================================================================

float ReflectionAbsorptionPower(float base, float exponent) {
    return static_cast<float>(std::pow(static_cast<double>(base), static_cast<double>(exponent)));
}

float ReflectionAbsorptionExponent(double distance, float absorption) {
    return static_cast<float>(distance * static_cast<double>(absorption));
}

static void SR_SVecPow(TSVec& result, const TSVec& sv, float p) {
    for (int i = 0; i < 3; i++) {
        result[i] = ReflectionAbsorptionPower(sv[i], p);
    }
    result[3] = 0.0f;
}

// ============================================================================
// Helper: MaxOfSVec
// ============================================================================

static float SR_MaxOfSVec(const TSVec& sv) {
    return std::max(sv[0], std::max(sv[1], sv[2]));
}

// ============================================================================
// Helper: MinMaxClip15bit
// ============================================================================



// ============================================================================
// Helper: FlipVecs
// ============================================================================

static void SR_FlipVecs(void* a, void* b) {
    TVec3D tmp;
    std::memcpy(tmp, a, sizeof(TVec3D));
    std::memcpy(a, b, sizeof(TVec3D));
    std::memcpy(b, tmp, sizeof(TVec3D));
}

// ============================================================================
// SRCalcThread state -- all per-thread state from TSRCalcThread
// ============================================================================

struct SRCalcState {
    TsiLight5*       srPsiLight;
    float            TransDiConst;
    float            sAbsorption;
    LongBool         bCalcTrans;
    LongBool         bCalcTransR;
    LongBool         bTransFlipInside;
    LongBool         bOnlyDIFS;
    int              MaxReflections;
    float            SRLightAmount;
    float            LVsdiff;
    float            sLightScatteringMul;
    TSVec            tAmb;
    TVec3D           Normal;
    TIteration3Dext  Iteration3Dext;
    TPaintParameter  PaintParameter;
    TMCTparameter    MCTparas;
    TLightVals       LVals;
    alignas(16) TPaintLightVals PLV;
    alignas(16) TLValigned LValigned;
    int32_t          calcStop;
};

// ============================================================================
// minLengthToCutPlane
// ============================================================================

static void SR_minLengthToCutPlane(double& dLength, int& cutplane, double dLimit,
                                    double posX, double posY, double posZ,
                                    double vecX, double vecY, double vecZ,
                                    TMCTparameter& mcp) {
    cutplane = 0;
    if (mcp.iCutOptions == 0) return;
    if ((mcp.iCutOptions & 1) != 0 && std::fabs(vecX) > 1e-20) {
        double d = (mcp.dCOX - posX) / vecX;
        if (d > dLimit && d < dLength) { dLength = d; cutplane = 1; }
    }
    if ((mcp.iCutOptions & 2) != 0 && std::fabs(vecY) > 1e-20) {
        double d = (mcp.dCOY - posY) / vecY;
        if (d > dLimit && d < dLength) { dLength = d; cutplane = 2; }
    }
    if ((mcp.iCutOptions & 4) != 0 && std::fabs(vecZ) > 1e-20) {
        double d = (mcp.dCOZ - posZ) / vecZ;
        if (d > dLimit && d < dLength) { dLength = d; cutplane = 3; }
    }
}

// ============================================================================
// AddLight -- positional light contribution
// ============================================================================

static void SR_AddLight(TSVec& result, const TSVec& svDiff, SRCalcState& st) {
    ClearSVec(result);
    for (int i = 0; i < 6; i++) {
        if (st.LVals.iLightOption[i] != 0) continue;
        float stmp;
        if ((st.LVals.iLightPos[i] & 1) != 0) {
            TSVec sv, sv2, sv3;
            SubtractSVectors(sv, st.PLV.ObjPos, st.PLV.CamPos);
            SubtractSVectors(sv2, st.LVals.PLValigned->LN[i], st.PLV.CamPos);
            SubtractSVectors(sv3, st.LVals.PLValigned->LN[i], st.PLV.ObjPos);
            float stmp3 = st.LVals.sStepWidth * st.LVals.sStepWidth;
            float d2 = SqrLengthOfSVec(sv2) + stmp3;
            float d3 = SqrLengthOfSVec(sv3) + stmp3;
            stmp = DotOfSVectors(sv2, sv);
            if (stmp <= 0) {
                stmp = sr_s025 / d3 + 1.0f / d2;
            } else {
                float stmp2 = SqrLengthOfSVec(sv);
                if (stmp2 <= stmp) {
                    stmp = sr_s025 / d2 + 1.0f / d3;
                } else {
                    TSVec scaled, added, sub;
                    ScaleSVec(scaled, sv, stmp / stmp2);
                    AddSVec(added, st.PLV.CamPos, scaled);
                    SubtractSVectors(sub, st.LVals.PLValigned->LN[i], added);
                    stmp = 1.0f / (SqrLengthOfSVec(sub) + stmp3)
                         + sr_s025 / std::max(d3, d2);
                }
            }
        } else {
            stmp = 1.0f;
        }
        AddSVecWeights(&result, &st.LVals.PLValigned->sLCols[i], stmp);
    }
    TSVec tmp;
    MultiplySVectors(tmp, svDiff, result);
    std::memcpy(result, tmp, sizeof(TSVec));
}

// ============================================================================
// CalcLightZPos
// ============================================================================

static void SR_CalcLightZPos(SRCalcState& st) {
    for (int i = 0; i < 6; i++) {
        if ((st.LVals.iLightPos[i] & 1) == 0) continue;
        double dTmp;
        if (std::fabs(st.PLV.AbsViewVec[0]) > s05f)
            dTmp = (st.LVals.PLValigned->LN[i][0] - st.PLV.CamPos[0]) / st.PLV.AbsViewVec[0];
        else if (std::fabs(st.PLV.AbsViewVec[1]) > s05f)
            dTmp = (st.LVals.PLValigned->LN[i][1] - st.PLV.CamPos[1]) / st.PLV.AbsViewVec[1];
        else
            dTmp = (st.LVals.PLValigned->LN[i][2] - st.PLV.CamPos[2]) / st.PLV.AbsViewVec[2];
        st.LVals.sPosLightZpos[i] = Max0S(static_cast<float>(dTmp));
        dTmp = 8388352.0 - st.MCTparas.ZcMul *
               (std::sqrt(st.LVals.sPosLightZpos[i] / st.LVals.sStepWidth * st.MCTparas.Zcorr + 1.0) - 1.0);
        st.LVals.sPosLP[i] = Min0MaxCS(static_cast<float>(dTmp / 256.0), 32767.0f);
    }
}

// ============================================================================
// CalcOpenAir
// ============================================================================

bool ReflectionFpcNotInsideXor(bool initialInside, bool currentInside) {
    const int32_t notInitial = initialInside ? 0 : 1;
    const int32_t currentLongBool = currentInside ? -1 : 0;
    return (notInitial ^ currentLongBool) != 0;
}

static void SR_CalcOpenAir(TsiLight5& siLight, TLightSD& SDsvecs, TSVec& tAbsorb,
                            LongBool bCalcT, float zz, SRCalcState& st) {
    if ((st.LVals.bFarFog || st.LVals.bCalcPixColSqr) &&
        (std::fabs(st.LVals.sDepth) > 1e-10f)) {
        double dTmp = 1.0 - Max0S(1.0f + (static_cast<int>(st.srPsiLight->Zpos) - 28000) * st.LVals.sDepth);
        if (dTmp > 0) { dTmp = dTmp * dTmp; if (st.LVals.bFarFog && st.LVals.bCalcPixColSqr) dTmp *= dTmp; }
        double dTmp2 = 1.0 - Max0S(1.0f - 28000.0f * st.LVals.sDepth);
        if (dTmp2 > 0) { dTmp2 = dTmp2 * dTmp2; if (st.LVals.bFarFog && st.LVals.bCalcPixColSqr) dTmp2 *= dTmp2; }
        dTmp = dTmp2 - dTmp;
        if (dTmp > 0) { dTmp = std::sqrt(dTmp); if (st.LVals.bFarFog && st.LVals.bCalcPixColSqr) dTmp = std::sqrt(dTmp); }
        siLight.Zpos = static_cast<uint16_t>(std::round(
            MinMaxCS(32768.0f, static_cast<float>(60768.0 - dTmp / st.LVals.sDepth), 65535.0f)));
    } else {
        siLight.Zpos = 32768;
    }

    st.PLV.zPos = static_cast<float>(st.PaintParameter.BackDist + st.MCTparas.sZZstmitDif);
    st.LVals.ZposDynFog = st.PLV.zPos;
    st.LVals.bDivOptions = 0;

    TSVec tmpAmb;
    if (bCalcT) {
        if (!ReflectionFpcNotInsideXor(st.bTransFlipInside != 0,
                                       st.MCTparas.bInsideRendering != 0)) {
            // inside: apply absorption
            TSVec tmpAbsorp;
            SR_SVecPow(tmpAbsorp, SDsvecs[1], zz * st.sAbsorption);
            float sTmp = (1.0f - YofSVec(tmpAbsorp)) * st.sLightScatteringMul;
            MultiplySVectorsV(reinterpret_cast<TSVec*>(&tAbsorb), &tmpAbsorp);
            TSVec lightContrib;
            SR_AddLight(lightContrib, SDsvecs[1], st);
            for (int c = 0; c < 3; c++)
                st.tAmb[c] += lightContrib[c] * tAbsorb[c] * sTmp;
            st.LVals.bDivOptions = 1;
        }
        TSVec transp;
        CalcPixelColorSvecTrans(&tmpAmb, SDsvecs, &siLight, &st.LVals, &st.PLV, transp);
    } else {
        TSVec transp;
        CalcPixelColorSvec(&tmpAmb, SDsvecs, &siLight, &st.LVals, &st.PLV, transp);
    }
    for (int c = 0; c < 3; c++)
        st.tAmb[c] += tAbsorb[c] * tmpAmb[c];
}

// ============================================================================
// DoDynFog
// ============================================================================

bool DirectionalVolumeLightMapContains(const TVolumeLightMap& vlm, const TVec3D& worldPos) {
    if (!vlm.Initialized || vlm.CubeSize <= 0 || vlm.CubeSides[0].empty()) return false;
    TSVec v = {
        static_cast<float>(worldPos[0] - vlm.LightPos[0]),
        static_cast<float>(worldPos[1] - vlm.LightPos[1]),
        static_cast<float>(worldPos[2] - vlm.LightPos[2]), 0.0f
    };
    RotateSVectorS(&v, &vlm.RotMatrix);
    // MB3DMaps.VolLightMapPosSSE adds the center and clamps in Single
    // BEFORE cvtss2si. Round(v*stretch)+HalfSize differs at half ties when
    // HalfSize is odd, even if Round itself uses nearest-even.
    const float sx = std::max(0.0f, std::min(vlm.CsizeS, v[0] * vlm.StretchSide1 + vlm.HSizeS));
    const float sy = std::max(0.0f, std::min(vlm.CsizeS, v[1] * vlm.StretchSide1 + vlm.HSizeS));
    const int x = static_cast<int>(std::nearbyint(sx));
    const int y = static_cast<int>(std::nearbyint(sy));
    return vlm.CubeSides[0][x + y * vlm.CubeSize] > v[2];
}

static bool SR_VolLightMapPos(const TVec3D& worldPos) {
    return DirectionalVolumeLightMapContains(CurrentRenderContext().volume_light_map, worldPos);
}

static void SR_DoDynFog(double& actDE, float& StepCount, float RSFmul,
                         float LastStepWidth, const TIteration3Dext& it3d, SRCalcState& st) {
    actDE = std::max(static_cast<double>(s011f),
                     (actDE - st.MCTparas.msDEsub * st.MCTparas.msDEstop) *
                     st.MCTparas.sZstepDiv * RSFmul);
    float s1 = std::max(st.MCTparas.msDEstop, 0.4f) * st.MCTparas.mctMH04ZSD;
    if (s1 < actDE) actDE = s1;

    TVec3D currentPos = { it3d.C1, it3d.C2, it3d.C3 };
    VolLightDoDynFogSubstep(currentPos, st.MCTparas.mVgradsFOV,
                            LastStepWidth,
                            st.MCTparas.mZZ,
                            st.MCTparas.mctDEstopFactor,
                            st.MCTparas.VLstepmul,
                            st.MCTparas.VLmul,
                            st.MCTparas.StepWidth,
                            StepCount);
}

// Volume-light fog sub-step accumulation, shared between the primary
// ray marcher (ray_marcher.cpp) and the reflection/transmission path
// (SR_DoDynFog above). Mirrors Pascal CalcThread.pas/CalcSR.pas
// DoDynFog, else-branch (DFogOnIt == 65535).
void VolLightDoDynFogSubstep(
    const TVec3D& currentPos,
    const TVec3D& marchVec,
    float LastStepWidth,
    double mZZ,
    float mctDEstopFactor,
    float VLstepmul,
    float VLmul,
    double stepWidth,
    float& stepCount)
{
    if (!CurrentRenderContext().volume_light_map.Initialized) return;
    const double d2 = stepWidth * stepWidth;
    TVec3D v;
    v[0] = currentPos[0];
    v[1] = currentPos[1];
    v[2] = currentPos[2];
    // Pascal: mCopyAddVecWeight(@v, @Iteration3Dext.C1, @mVgradsFOV, -LastStepWidth)
    AddPackedVecWeight(v, marchVec, -static_cast<double>(LastStepWidth));

    float s1 = (1.0f + static_cast<float>(mZZ) * mctDEstopFactor) * VLstepmul;
    float stLocal = LastStepWidth;
    do {
        RenderWork();
        if (s1 > stLocal) s1 = stLocal;
        AddPackedVecWeight(v, marchVec, static_cast<double>(s1));
        stLocal -= s1;
        if (CurrentRenderContext().volume_light_map.IsPosLight) {
            TVec3D vs = {
                v[0] - CurrentRenderContext().volume_light_map.LightPos[0],
                v[1] - CurrentRenderContext().volume_light_map.LightPos[1],
                v[2] - CurrentRenderContext().volume_light_map.LightPos[2]
            };
            double d1 = VolumeLightDirectionSquaredLength(vs);
            float vl = GetVolLightMapValue(vs);
            if (static_cast<double>(vl) * static_cast<double>(vl) > d1) {
                // The product is Single, but d1+d2 and the accumulation are
                // Double in CalcThread/CalcSR before the final Single store.
                stepCount = static_cast<float>(static_cast<double>(stepCount) +
                    static_cast<double>(VLmul * s1) / (d1 + d2));
            }
        } else if (SR_VolLightMapPos(v)) {
            stepCount += VLmul * s1;
        }
    } while (stLocal >= s001f);
}


// ============================================================================
// CalcRay -- recursive ray marching for reflection + transmission
// ============================================================================

static void SR_CalcRay(double ZZ, const TVec3D& SRVec, TSVec tAbsorb_in,
                        TLightSD SDsvecs_in, int Rit, SRCalcState& st) {
    RenderPoll();
    // Local copies (Pascal passed these by value)
    TSVec tAbsorb;
    TLightSD SDsvecs;
    std::memcpy(tAbsorb, tAbsorb_in, sizeof(TSVec));
    std::memcpy(SDsvecs, SDsvecs_in, sizeof(TLightSD));

    int itmp;
    LongBool DElimited, OpenAir, bCalcT;
    float RSFmul, sTmp, StepCount, LastZZ;
    TsiLight5 siLight;
    double RLastStepWidth, dTmp, dStep, ZZplus, ZZ2, RLastDE, MaxL, ZZtmp, SpecMulT;
    TVec3D NewVec, tmpLoc, tmpNorm;
    TSVec tmpAmb, tmpSpec, tmpAbsorb;
    int RMStepCount;
    TLightSD tmpSDsvecs;

    std::memset(&siLight, 0, sizeof(TsiLight5));
    auto& mcp = st.MCTparas;
    auto& it3d = st.Iteration3Dext;

    // Build Xmit vector for position calculations
    TVec3D xmitVec = { mcp.Xmit, mcp.Ymit, mcp.Zmit };

    // Get absolute normal vector
    TMatrix3 normalMatrix;
    std::memcpy(normalMatrix, mcp.Vgrads, sizeof(normalMatrix));
    RotateVectorReverse(st.Normal, normalMatrix);

    // Determine transmission
    if (!st.bCalcTrans) {
        st.bCalcTransR = 0;
    } else if (!st.bOnlyDIFS) {
        st.bCalcTransR = 1;
    } else {
        mcp.CalcDE(&it3d, &mcp);
        st.bCalcTransR = (mcp.DEoptionResult == 20) ? 1 : 0;
    }
    bCalcT = st.bCalcTransR;
    if (HasParityTraceContext() && ShouldLogParityStage("sr_ray_enter")) {
        std::ostringstream payload;
        payload.setf(std::ios::fixed, std::ios::floatfield);
        payload << std::setprecision(12)
                << " Rit=" << Rit
                << " bCalcT=" << (bCalcT ? 1 : 0)
                << " ZZ=" << ZZ
                << " SRVecX=" << SRVec[0]
                << " SRVecY=" << SRVec[1]
                << " SRVecZ=" << SRVec[2]
                << " AbsorbX=" << tAbsorb[0]
                << " AbsorbY=" << tAbsorb[1]
                << " AbsorbZ=" << tAbsorb[2]
                << " SD0X=" << SDsvecs[0][0]
                << " SD0Y=" << SDsvecs[0][1]
                << " SD0Z=" << SDsvecs[0][2]
                << " SD0W=" << SDsvecs[0][3]
                << " SD1X=" << SDsvecs[1][0]
                << " SD1Y=" << SDsvecs[1][1]
                << " SD1Z=" << SDsvecs[1][2]
                << " SD1W=" << SDsvecs[1][3]
                << " tAmbX=" << st.tAmb[0]
                << " tAmbY=" << st.tAmb[1]
                << " tAmbZ=" << st.tAmb[2];
        AppendParityTrace("sr_ray_enter", payload.str());
        if (DebugPrintSrRecursionToStderr()) {
            std::fprintf(stderr, "SR_RAY_ENTER %s\n", payload.str().c_str());
            std::fflush(stderr);
        }
    }

lab1:
    if (st.calcStop ||
        (std::fabs(tAbsorb[0]) * sr_s03 + std::fabs(tAbsorb[1]) * sr_s059 +
         std::fabs(tAbsorb[2]) * s011f < sr_s1em4))
        return;

lab2:
    if (bCalcT) {
        // Transmission: Snell's law refraction
        sTmp = SR_MaxOfSVec(SDsvecs[0]);
        if (st.bTransFlipInside ^ mcp.bInsideRendering)
            dStep = st.TransDiConst;
        else
            dStep = 1.0 / st.TransDiConst;

        dTmp = LengthOfVec(SRVec);
        MaxL = LengthOfVec(st.Normal);
        SpecMulT = -DotVec3D(SRVec, st.Normal) / (dTmp * MaxL);
        ZZ2 = 1.0 - dStep * dStep * (1.0 - SpecMulT * SpecMulT);

        if (ZZ2 <= 0.0) {
            // Total internal reflection
            ScaleSVectorV(reinterpret_cast<TSVec*>(SDsvecs[0]),
                          1.0f - SDsvecs[0][3] + SDsvecs[0][3] * SDsvecs[0][3] / (sTmp + 0.01f));
            bCalcT = 0;
            goto lab2;
        }

        // Save location + normals
        tmpLoc[0] = it3d.C1; tmpLoc[1] = it3d.C2; tmpLoc[2] = it3d.C3;
        std::memcpy(tmpNorm, st.Normal, sizeof(TVec3D));
        std::memcpy(tmpAbsorb, tAbsorb, sizeof(TSVec));
        std::memcpy(tmpSDsvecs, SDsvecs, sizeof(TLightSD));
        ZZtmp = ZZ;

        // Refracted direction
        {
            TVec3D scaledN;
            double factor = dTmp / (MaxL * dStep) * (std::sqrt(ZZ2) - dStep * SpecMulT);
            ScaleVec3D(scaledN, st.Normal, factor);
            SubtractVec3D(NewVec, SRVec, scaledN);
            double newLen = LengthOfVec(NewVec);
            if (newLen > 1e-20) ScaleVec3D_inplace(NewVec, dTmp / newLen);
        }

        // Fresnel reflective amount
        dTmp = std::fabs(DotVec3D(NewVec, st.Normal)) / (dTmp * MaxL);
        SpecMulT = std::fabs(SpecMulT);
        if (std::fabs(dTmp) > 1e-16) {
            double r1 = (SpecMulT - dStep * dTmp) / (SpecMulT + dStep * dTmp);
            double r2 = (dTmp - dStep * SpecMulT) / (dTmp + dStep * SpecMulT);
            SpecMulT = (r1 * r1 + r2 * r2) * s05f;
        }
        ScaleSVectorV(reinterpret_cast<TSVec*>(tAbsorb),
                       static_cast<float>(1.0 - SpecMulT * sTmp));
        mcp.bInsideRendering = !mcp.bInsideRendering;
        mcp.bCalcInside = !mcp.bCalcInside;
    } else {
        // Reflection: R = D - 2*(D.N)*N / |N|^2
        dTmp = SqrLengthOfVec(st.Normal);
        if (dTmp < static_cast<double>(sr_s1em10)) return;
        TVec3D scaledN;
        ScaleVec3D(scaledN, st.Normal, 2.0 * DotVec3D(st.Normal, SRVec) / dTmp);
        SubtractVec3D(NewVec, SRVec, scaledN);
    }

    // Projection
    TVec3D marchDirection;
    std::memcpy(marchDirection, mcp.mVgradsFOV, sizeof(marchDirection));
    ZZplus = DotVec3D(marchDirection, NewVec) /
             std::sqrt(SqrLengthOfVec(marchDirection) * SqrLengthOfVec(NewVec) + 1e-100);

    MaxL = mcp.Zend;
    itmp = 0;
    if (mcp.iCutOptions != 0)
        SR_minLengthToCutPlane(MaxL, itmp, 1.0,
                               it3d.C1, it3d.C2, it3d.C3,
                               NewVec[0], NewVec[1], NewVec[2], mcp);

    LastZZ = static_cast<float>(std::fabs(ZZ));

    // Step forward to avoid self-intersection. Keep this initial bias in ZZ2:
    // Pascal accumulates the later reflected-ray path length on top of it.
    ZZ2 = MinCD(1.0, static_cast<double>(mcp.DEstop) * sr_s025);
    ZZ += ZZ2 * ZZplus;
    AddPackedVecWeight(&it3d.C1, NewVec, ZZ2);
    mcp.msDEstop = mcp.DEstop * (1.0f + static_cast<float>(std::fabs(ZZ)) * mcp.mctDEstopFactor);

    // CamPos for light
    SR_DVecToSVec(st.PLV.CamPos,
                  it3d.C1 - mcp.Xmit, it3d.C2 - mcp.Ymit, it3d.C3 - mcp.Zmit);

    OpenAir = 1;
    StepCount = 0;
    DElimited = 1;

    if (st.srPsiLight->SIgradient > 32767) {
        ZZ2 = mcp.Zend;
    } else {
        OpenAir = 0;
        RSFmul = 1.0f;
        dTmp = mcp.CalcDE(&it3d, &mcp);
        RLastStepWidth = dTmp * mcp.sZstepDiv;
        RMStepCount = 0;

        if (dTmp > static_cast<double>(sr_s1em10)) {
            do {
                RenderPoll();
                RMStepCount++;
                if (RMStepCount > 100000) { OpenAir = 1; break; }
                RLastDE = dTmp;
                if (HasParityTraceContext() && ShouldLogParityStage("sr_step_begin")) {
                    std::ostringstream payload;
                    payload.setf(std::ios::fixed, std::ios::floatfield);
                    payload << std::setprecision(12)
                            << " Rit=" << Rit
                            << " ZZ=" << ZZ
                            << " ZZ2=" << ZZ2
                            << " dTmp=" << dTmp
                            << " RLastStepWidth=" << RLastStepWidth
                            << " RLastDE=" << RLastDE
                            << " msDEstop=" << mcp.msDEstop
                            << " ItResultI=" << it3d.ItResultI
                            << " RSFmul=" << RSFmul;
                    AppendParityTrace("sr_step_begin", payload.str());
                }

                if (mcp.DFogOnIt == 65535) {
                    dStep = dTmp;
                    mcp.mZZ = std::fabs(ZZ);
                    SR_DoDynFog(dStep, StepCount, RSFmul, static_cast<float>(RLastStepWidth), it3d, st);
                } else {
                    dStep = std::max(static_cast<double>(s011f),
                                     (dTmp - mcp.msDEsub * mcp.msDEstop) * mcp.sZstepDiv * RSFmul);
                    sTmp = std::max(mcp.msDEstop, 0.4f) * mcp.mctMH04ZSD;
                    if (sTmp < dStep) {
                        if (mcp.DFogOnIt == 0) StepCount += sTmp / static_cast<float>(dStep);
                        else if (it3d.ItResultI == mcp.DFogOnIt) StepCount += sTmp / static_cast<float>(dStep);
                        dStep = sTmp;
                    } else {
                        if (mcp.DFogOnIt == 0) StepCount += 1.0f;
                        else if (it3d.ItResultI == mcp.DFogOnIt) StepCount += 1.0f;
                    }
                }
                if (HasParityTraceContext() && ShouldLogParityStage("sr_step_after_dynfog")) {
                    std::ostringstream payload;
                    payload.setf(std::ios::fixed, std::ios::floatfield);
                    payload << std::setprecision(12)
                            << " Rit=" << Rit
                            << " dStep=" << dStep
                            << " StepCount=" << StepCount
                            << " RSFmul=" << RSFmul
                            << " msDEstop=" << mcp.msDEstop;
                    AppendParityTrace("sr_step_after_dynfog", payload.str());
                }

                RLastStepWidth = dStep;
                ZZ2 += dStep;

                if (ZZ2 > MaxL) {
                    if (mcp.DFogOnIt == 65535) {
                        double adj = MaxL - ZZ2 + RLastStepWidth;
                        AddPackedVecWeight(&it3d.C1, NewVec, adj);
                        float adjf = static_cast<float>(adj);
                        SR_DoDynFog(RLastDE, StepCount, RSFmul, adjf, it3d, st);
                    }
                    OpenAir = 1;
                    break;
                }

                AddPackedVecWeight(&it3d.C1, NewVec, dStep);
                ZZ += dStep * ZZplus;
                mcp.msDEstop = mcp.DEstop * (1.0f + static_cast<float>(std::fabs(ZZ)) * mcp.mctDEstopFactor);

                dTmp = mcp.CalcDE(&it3d, &mcp);
                if (HasParityTraceContext() && ShouldLogParityStage("sr_step_after_de")) {
                    std::ostringstream payload;
                    payload.setf(std::ios::fixed, std::ios::floatfield);
                    payload << std::setprecision(12)
                            << " Rit=" << Rit
                            << " ZZ=" << ZZ
                            << " ZZ2=" << ZZ2
                            << " C1=" << it3d.C1
                            << " C2=" << it3d.C2
                            << " C3=" << it3d.C3
                            << " dTmp=" << dTmp
                            << " Rout=" << it3d.Rout
                            << " ItResultI=" << it3d.ItResultI
                            << " RSFmul=" << RSFmul;
                    AppendParityTrace("sr_step_after_de", payload.str());
                }
                if (std::isnan(dTmp) || std::isinf(dTmp)) { OpenAir = 1; break; }
                if (it3d.ItResultI >= mcp.MaxItsResult) { DElimited = 0; break; }
                if (dTmp < mcp.msDEstop && it3d.ItResultI >= mcp.iMinIt) break;

                // Overstep correction
                if (dTmp > RLastDE + RLastStepWidth) dTmp = RLastDE + RLastStepWidth;
                if (RLastDE > dTmp + static_cast<double>(sr_s1em10)) {
                    dStep = RLastStepWidth / (RLastDE - dTmp);
                    RSFmul = (dStep < 1.0) ? std::max(s05f, static_cast<float>(dStep)) : 1.0f;
                } else {
                    RSFmul = 1.0f;
                }
            } while (!st.calcStop);
        }
        if (st.calcStop) return;
    }

    // View direction for reflected ray
    SR_DVecToSVec(st.PLV.AbsViewVec, NewVec[0], NewVec[1], NewVec[2]);
    NormalizeSVec_inplace(st.PLV.AbsViewVec);
    std::memcpy(st.PLV.ViewVec, st.PLV.AbsViewVec, sizeof(TSVec));
    RotateSVectorReverseS(&st.PLV.ViewVec, &st.PaintParameter.m);
    SR_CalcLightZPos(st);

    st.LVals.bScaleAmbDiffDown = 0;
    st.LVals.ZposDynFog = static_cast<float>(ZZ2 * mcp.StepWidth + mcp.sZZstmitDif);

    // ypos/xpos from view direction
    {
        const TSVec& tmpPSV = st.LVals.bAmbRelObj ? st.PLV.AbsViewVec : st.PLV.ViewVec;
        st.PLV.yPos = static_cast<float>(ArcSinSafe(tmpPSV[1]) / mcp.FOVy + s05f);
        if (st.LVals.bBackBMP) {
            double dTmpBg = mcp.FOVy / std::max(0.1, PiM2 - mcp.FOVy);
            if (st.PLV.yPos > 1.0f) {
                st.PLV.yPos = 1.0f + static_cast<float>(dTmpBg) * (1.0f - st.PLV.yPos);
                if (st.PLV.yPos < 0.75f) st.PLV.yPos = 1.5f - st.PLV.yPos;
            } else if (st.PLV.yPos < 0.0f) {
                st.PLV.yPos = std::fabs(st.PLV.yPos) * static_cast<float>(dTmpBg);
                if (st.PLV.yPos > sr_s025) st.PLV.yPos = s05f - st.PLV.yPos;
            }
        }
        Clamp01Svar(st.PLV.yPos);

        dStep = static_cast<double>(mcp.iMandHeight) / (mcp.FOVy * mcp.iMandWidth);
        st.PLV.xPos = static_cast<float>(std::atan2(tmpPSV[0], tmpPSV[2]) * dStep + s05f);
        if (st.LVals.bBackBMP) {
            double d2 = 1.0 / (dStep * std::max(0.1, PiM2 - 1.0 / dStep));
            if (st.PLV.xPos > 1.0f) st.PLV.xPos = 1.0f + static_cast<float>(d2) * (1.0f - st.PLV.xPos);
            else if (st.PLV.xPos < 0.0f) st.PLV.xPos = std::fabs(st.PLV.xPos) * static_cast<float>(d2);
        }
        Clamp01Svar(st.PLV.xPos);

        if (!st.LVals.bAmbRelObj) {
            sTmp = st.PLV.yPos;
            if (st.LVals.iDfunc == 1) sTmp *= sTmp;
            else if (st.LVals.iDfunc != 0) sTmp = std::sqrt(sTmp);
            LinInterpolate2SVecs(st.PLV.PreDepthCol,
                                 st.LVals.PLValigned->sDepthCol2,
                                 st.LVals.PLValigned->sDepthCol, sTmp);
        }
    }

    // Encode shadow/vlight
    if (mcp.DFogOnIt == 65535) {
        siLight.Shadow = static_cast<uint16_t>((siLight.Shadow & 0xFC00u) |
                                               static_cast<uint16_t>(RMcalcVLight(StepCount) & 0x03FF));
    } else {
        if (mcp.bInsideRendering) StepCount *= 200.0f * mcp.DEstop / mcp.iMandWidth;
        siLight.Shadow = static_cast<uint16_t>((siLight.Shadow & 0xFC00u) |
            static_cast<uint16_t>(std::lround(Min0MaxCS((StepCount - 1.0f) + 1.0f, 1023.0f)) & 0x03FF));
    }

    if (!OpenAir) {
        // Surface hit: binary search refinement
        if (mcp.iDEAddSteps != 0) {
            if (DElimited) {
                itmp = mcp.iDEAddSteps;
                dStep = RLastStepWidth * -0.5;
                dTmp = mcp.CalcDE(&it3d, &mcp);
                while (std::fabs(dTmp - mcp.msDEstop) > 0.001) {
                    RenderPoll();
                    ZZ2 += dStep;
                    ZZ += dStep * ZZplus;
                    AddPackedVecWeight(&it3d.C1, NewVec, dStep);
                    mcp.msDEstop = mcp.DEstop * (1.0f + static_cast<float>(std::fabs(ZZ)) * mcp.mctDEstopFactor);
                    itmp--;
                    if (itmp <= 0) break;
                    dTmp = mcp.CalcDE(&it3d, &mcp);
                    dStep = (dTmp < mcp.msDEstop) ? std::fabs(dStep) * -0.55 : std::fabs(dStep) * 0.55;
                }
            } else {
                dStep = ZZ2;
                SR_FlipVecs(NewVec, mcp.mVgradsFOV);
                RMdoBinSearchIt(&mcp, &ZZ2);
                ZZ += (ZZ2 - dStep) * ZZplus;
                SR_FlipVecs(NewVec, mcp.mVgradsFOV);
            }
        }
        mcp.mZZ = std::fabs(ZZ);

        // Normals
        mcp.mPsiLight = &siLight;
        if (mcp.NormalsOnDE)
            RMCalculateNormals(&mcp, RSFmul);
        else
            RMCalculateNormalsOnSmoothIt(&mcp, RSFmul);

        MakeDVecFromNormals(&siLight, st.Normal);
        if (HasParityTraceContext() && ShouldLogParityStage("sr_hit_after_normals")) {
            std::ostringstream payload;
            payload.setf(std::ios::fixed, std::ios::floatfield);
            payload << std::setprecision(12)
                    << " Rit=" << Rit
                    << " DElimited=" << (DElimited ? 1 : 0)
                    << " ZZ=" << ZZ
                    << " ZZ2=" << ZZ2
                    << " LastZZ=" << LastZZ
                    << " RSFmul=" << RSFmul
                    << " NormalX=" << st.Normal[0]
                    << " NormalY=" << st.Normal[1]
                    << " NormalZ=" << st.Normal[2]
                    << " PackedNormalX=" << siLight.NormalX
                    << " PackedNormalY=" << siLight.NormalY
                    << " PackedNormalZ=" << siLight.NormalZ
                    << " ItResultI=" << it3d.ItResultI
                    << " Rout=" << it3d.Rout
                    << " OTrapRaw=" << it3d.OTrap;
            AppendParityTrace("sr_hit_after_normals", payload.str());
        }

        // SIgradient
        if (DElimited)
            RSFmul = 32767.0f - (RSFmul + mcp.dColPlus +
                     mcp.mctColVarDEstopMul * std::log(mcp.msDEstop * static_cast<float>(mcp.StepWidth))) * mcp.mctsM;
        else
            RSFmul = 32767.0f - RSFmul * mcp.mctsM;
        MinMaxClip15bit(RSFmul, siLight.SIgradient);
        if (HasParityTraceContext() && ShouldLogParityStage("sr_hit_after_sig")) {
            std::ostringstream payload;
            payload.setf(std::ios::fixed, std::ios::floatfield);
            payload << std::setprecision(12)
                    << " Rit=" << Rit
                    << " DElimited=" << (DElimited ? 1 : 0)
                    << " RSFmul=" << RSFmul
                    << " SIgradient=" << siLight.SIgradient
                    << " msDEstop=" << mcp.msDEstop
                    << " StepWidth=" << mcp.StepWidth
                    << " dColPlus=" << mcp.dColPlus
                    << " mctColVarDEstopMul=" << mcp.mctColVarDEstopMul
                    << " mctsM=" << mcp.mctsM;
            AppendParityTrace("sr_hit_after_sig", payload.str());
        }

        if (mcp.ColorOnIt != 0) RMdoColorOnIt(&mcp);
        RMdoColor(&mcp);
        if (HasParityTraceContext() && ShouldLogParityStage("sr_hit_after_color")) {
            std::ostringstream payload;
            payload.setf(std::ios::fixed, std::ios::floatfield);
            payload << std::setprecision(12)
                    << " Rit=" << Rit
                    << " ColorOnIt=" << mcp.ColorOnIt
                    << " ColorOption=" << static_cast<int>(mcp.ColorOption)
                    << " SIgradient=" << siLight.SIgradient
                    << " OTrap=" << siLight.OTrap
                    << " OTrapRaw=" << it3d.OTrap
                    << " Rout=" << it3d.Rout
                    << " Rold=" << it3d.Rold
                    << " X=" << it3d.x
                    << " Y=" << it3d.y
                    << " Z=" << it3d.z;
            AppendParityTrace("sr_hit_after_color", payload.str());
        }

        // Z position encoding
        if (st.LVals.bFarFog || st.LVals.bCalcPixColSqr) {
            double lz = static_cast<double>(LastZZ);
            dStep = std::sqrt((ZZ2 + lz) * (ZZ2 + lz) - lz + sr_s1em30);
        } else {
            dStep = ZZ2;
        }
        CalcZposAndRough(&siLight, &mcp, dStep);
        if (HasParityTraceContext() && ShouldLogParityStage("sr_hit_after_zpos")) {
            std::ostringstream payload;
            payload.setf(std::ios::fixed, std::ios::floatfield);
            payload << std::setprecision(12)
                    << " Rit=" << Rit
                    << " dStep=" << dStep
                    << " Zpos=" << siLight.Zpos
                    << " RoughZposFine=" << siLight.RoughZposFine
                    << " SIgradient=" << siLight.SIgradient
                    << " OTrap=" << siLight.OTrap;
            AppendParityTrace("sr_hit_after_zpos", payload.str());
        }
        RSFmul = mcp.msDEstop;

        // Hard shadow on reflected/refracted surface
        if ((mcp.calcHardShadow & 1) != 0) {
            mcp.mZZ = std::fabs(ZZ);
            CalcHardShadowForPoint(&mcp, &it3d, &siLight, &st.LVals, mcp.iMandHeight);
        }

        // PLV positions
        st.PLV.zPos = static_cast<float>(std::fabs(ZZ) * mcp.StepWidth + mcp.sZZstmitDif);
        SR_DVecToSVec(st.PLV.ObjPos,
                      it3d.C1 - mcp.Xmit, it3d.C2 - mcp.Ymit, it3d.C3 - mcp.Zmit);

        // Ambient shadow on reflected surface
        if (mcp.bCalcAmbShadow) {
            int aoQual = (Rit <= 1) ? 2 : ((Rit == 2) ? 1 : 0);
            mcp.mZZ = std::fabs(ZZ);
            mcp.AOdither = 0;
            CalcAmbientOcclusionForPoint(&mcp, &it3d, &siLight, aoQual, 0, 0);
            siLight.AmbShadow = static_cast<uint16_t>((siLight.AmbShadow * 8) / (8 + Rit));
        } else {
            siLight.AmbShadow = 5000;
        }

        mcp.msDEstop = RSFmul;
        st.LVals.bDivOptions = 0;
        st.LVals.bScaleAmbDiffDown = st.bCalcTransR && (Rit < st.MaxReflections);

        // Absorption
        if (bCalcT)
            ScaleSVectorV(reinterpret_cast<TSVec*>(tAbsorb), SDsvecs[0][3]);
        else
            MultiplySVectorsV(reinterpret_cast<TSVec*>(tAbsorb), reinterpret_cast<const TSVec*>(SDsvecs[0]));

        if (st.bTransFlipInside ^ mcp.bInsideRendering) {
            // Inside material: absorption
            TSVec tmpAbsorp;
            SR_SVecPow(tmpAbsorp, SDsvecs[1], ReflectionAbsorptionExponent(MinCD(ZZ2, MaxL), st.sAbsorption));
            std::memcpy(tmpSpec, tAbsorb, sizeof(TSVec));
            sTmp = (1.0f - YofSVec(tmpAbsorp)) * st.sLightScatteringMul;
            MultiplySVectorsV(reinterpret_cast<TSVec*>(tAbsorb), &tmpAbsorp);
            TSVec lightContrib;
            SR_AddLight(lightContrib, SDsvecs[1], st);
            TSVec interpAbsorb;
            LinInterpolate2SVecs(interpAbsorb, tAbsorb, tmpSpec, s05f);
            for (int c = 0; c < 3; c++)
                st.tAmb[c] += lightContrib[c] * interpAbsorb[c] * sTmp;
            st.LVals.bDivOptions = 1;
        }

        // Pixel coloring
        TSVec transp;
        CalcPixelColorSvecTrans(&tmpAmb, SDsvecs, &siLight, &st.LVals, &st.PLV, transp);
        for (int c = 0; c < 3; c++)
            st.tAmb[c] += tAbsorb[c] * tmpAmb[c];

        if ((!bCalcT) || ReflectionFpcNotInsideXor(st.bTransFlipInside != 0,
                                                  mcp.bInsideRendering != 0))
            MultiplySVectorsV(reinterpret_cast<TSVec*>(tAbsorb), &transp);

        // Recurse for next reflection
        if (Rit < st.MaxReflections) {
            if (HasParityTraceContext() && ShouldLogParityStage("sr_recurse_ready")) {
                std::ostringstream payload;
                payload.setf(std::ios::fixed, std::ios::floatfield);
                payload << std::setprecision(12)
                        << " Rit=" << Rit
                        << " NextRit=" << (Rit + 1)
                        << " OpenAir=" << (OpenAir ? 1 : 0)
                        << " bCalcT=" << (bCalcT ? 1 : 0)
                        << " ZZ=" << ZZ
                        << " ZZ2=" << ZZ2
                        << " NewVecX=" << NewVec[0]
                        << " NewVecY=" << NewVec[1]
                        << " NewVecZ=" << NewVec[2]
                        << " NormalX=" << st.Normal[0]
                        << " NormalY=" << st.Normal[1]
                        << " NormalZ=" << st.Normal[2]
                        << " ObjPosX=" << st.PLV.ObjPos[0]
                        << " ObjPosY=" << st.PLV.ObjPos[1]
                        << " ObjPosZ=" << st.PLV.ObjPos[2]
                        << " Shadow=" << siLight.Shadow
                        << " AmbShadow=" << siLight.AmbShadow
                        << " SIgradient=" << siLight.SIgradient
                        << " OTrap=" << siLight.OTrap
                        << " AbsorbX=" << tAbsorb[0]
                        << " AbsorbY=" << tAbsorb[1]
                        << " AbsorbZ=" << tAbsorb[2]
                        << " SD0X=" << SDsvecs[0][0]
                        << " SD0Y=" << SDsvecs[0][1]
                        << " SD0Z=" << SDsvecs[0][2]
                        << " SD0W=" << SDsvecs[0][3]
                        << " SD1X=" << SDsvecs[1][0]
                        << " SD1Y=" << SDsvecs[1][1]
                        << " SD1Z=" << SDsvecs[1][2]
                        << " SD1W=" << SDsvecs[1][3]
                        << " tAmbX=" << st.tAmb[0]
                        << " tAmbY=" << st.tAmb[1]
                        << " tAmbZ=" << st.tAmb[2];
                AppendParityTrace("sr_recurse_ready", payload.str());
                if (DebugPrintSrRecursionToStderr()) {
                    std::fprintf(stderr, "SR_RECURSE_READY %s\n", payload.str().c_str());
                    std::fflush(stderr);
                }
                if (DebugStopAfterSrRecurseReady()) {
                    st.calcStop = 1;
                    return;
                }
            }
            ScaleSVectorV(reinterpret_cast<TSVec*>(tAbsorb), st.SRLightAmount);
            SR_CalcRay(ZZ, NewVec, tAbsorb, SDsvecs, Rit + 1, st);
        }
    } else {
        // Open air (background)
        siLight.Zpos = 32768;
        if (bCalcT)
            ScaleSVectorV(reinterpret_cast<TSVec*>(tAbsorb), SDsvecs[0][3]);
        else
            MultiplySVectorsV(reinterpret_cast<TSVec*>(tAbsorb), reinterpret_cast<const TSVec*>(SDsvecs[0]));
        SR_CalcOpenAir(siLight, SDsvecs, tAbsorb, bCalcT, static_cast<float>(MinCD(ZZ2, MaxL)), st);
    }

    if (HasParityTraceContext() && ShouldLogParityStage("sr_ray_exit")) {
        std::ostringstream payload;
        payload.setf(std::ios::fixed, std::ios::floatfield);
        payload << std::setprecision(12)
                << " Rit=" << Rit
                << " OpenAir=" << (OpenAir ? 1 : 0)
                << " bCalcT=" << (bCalcT ? 1 : 0)
                << " ZZ=" << ZZ
                << " ZZ2=" << ZZ2
                << " Zpos=" << siLight.Zpos
                << " Shadow=" << siLight.Shadow
                << " AmbShadow=" << siLight.AmbShadow
                << " SIgradient=" << siLight.SIgradient
                << " OTrap=" << siLight.OTrap
                << " tAmbX=" << st.tAmb[0]
                << " tAmbY=" << st.tAmb[1]
                << " tAmbZ=" << st.tAmb[2]
                << " AbsorbX=" << tAbsorb[0]
                << " AbsorbY=" << tAbsorb[1]
                << " AbsorbZ=" << tAbsorb[2];
        AppendParityTrace("sr_ray_exit", payload.str());
        if (DebugPrintSrRecursionToStderr()) {
            std::fprintf(stderr, "SR_RAY_EXIT %s\n", payload.str().c_str());
            std::fflush(stderr);
        }
        if (DebugStopAfterSrRayExit()) {
            st.calcStop = 1;
            return;
        }
    }

    // After transmission, do the reflection component
    if (bCalcT) {
        it3d.C1 = tmpLoc[0]; it3d.C2 = tmpLoc[1]; it3d.C3 = tmpLoc[2];
        std::memcpy(st.Normal, tmpNorm, sizeof(TVec3D));
        ZZ = ZZtmp;
        std::memcpy(SDsvecs, tmpSDsvecs, sizeof(TLightSD));
        float fresnelScale = static_cast<float>(SpecMulT + (1.0 - SpecMulT) * (1.0 - SDsvecs[0][3]));
        for (int c = 0; c < 4; c++) tAbsorb[c] = tmpAbsorb[c] * fresnelScale;
        mcp.bInsideRendering = !mcp.bInsideRendering;
        mcp.bCalcInside = !mcp.bCalcInside;
        bCalcT = 0;
        goto lab1;
    }
}

// Forward declaration
static void SR_ProcessRows(SRCalcState& st, TsiLight5* siLight, uint8_t* imageBuffer,
                           int width, int height, TMCTparameter* mcp,
                           LongBool bInsideTmp, int y_start, int y_end,
                           int x_start, int x_end);

// ============================================================================
// CalcReflections -- main entry point
// ============================================================================

void CalcReflections(
    TsiLight5* siLight,
    uint8_t* imageBuffer,
    int width, int height,
    int slOffset,
    TMCTparameter* mcp,
    TLightVals* lvals,
    const TMandHeader10* header,
    const THeaderCustomAddon* addon
) {
    RenderPoll();
    SRCalcState st;
    std::memset(&st, 0, sizeof(SRCalcState));

    st.MCTparas = *mcp;
    st.LVals = *lvals;
    st.calcStop = 0;

    // Aligned light storage copy
    TLValigned localAligned;
    if (lvals->PLValigned)
        std::memcpy(&localAligned, lvals->PLValigned, sizeof(TLValigned));
    else
        std::memset(&localAligned, 0, sizeof(TLValigned));
    st.LValigned = localAligned;
    st.LVals.PLValigned = &st.LValigned;

    // Iteration state
    std::memset(&st.Iteration3Dext, 0, sizeof(TIteration3Dext));
    st.MCTparas.pIt3Dext = &st.Iteration3Dext;
    IniIt3D(&st.MCTparas, &st.Iteration3Dext);

    LongBool bInsideTmp = st.MCTparas.bInsideRendering;
    st.bTransFlipInside = bInsideTmp;

    // Reflection parameters -- read from header
    st.SRLightAmount = lvals->SRLightAmount;
    st.MaxReflections = header->SRreflectioncount;
    if (st.MaxReflections < 1) st.MaxReflections = 1;

    // Refraction index: check bOptions2 bit pattern
    // (PTHeaderCustomAddon(Header.PCFAddon).bOptions2 and 6) = 4 => sRI = 1
    // else sRI = header->sTRIndex
    float sRI = ((addon != nullptr) && ((addon->bOptions2 & 6) == 4))
        ? 1.0f
        : header->sTRIndex;
    if (sRI <= 0.0f) sRI = 1.5f;
    st.TransDiConst = sRI;

    // Transmission (transparency) enabled: bit 1 of bCalcSRautomatic
    st.bCalcTrans = (header->bCalcSRautomatic & 2) ? 1 : 0;
    st.sAbsorption = ReflectionAbsorptionExponent(header->dStepWidth, header->sTransmissionAbsorption);
    // Only DIFS shapes get transparency: bit 2 of bCalcSRautomatic
    st.bOnlyDIFS = (header->bCalcSRautomatic & 4) ? 1 : 0;
    st.sLightScatteringMul = header->sTRscattering / 330.0f;
    st.LVsdiff = lvals->sDiff;
    st.LVals.sObjLightDecreaser = std::max(s05f, 1.0f - std::sqrt(st.SRLightAmount) * 0.17f);

    // Paint parameters
    auto& pp = st.PaintParameter;
    pp.ppWidth = header->Width;
    pp.ppHeight = header->Height;
    pp.ppYinc = 1;
    pp.PLVals = &st.LVals;
    pp.pVgrads = &st.MCTparas.Vgrads;
    pp.StepWidth = st.MCTparas.StepWidth;
    pp.sFOVy = static_cast<float>(st.MCTparas.FOVy);
    pp.ppPlanarOptic = st.MCTparas.MCTCameraOptic;

    double d = MinCD(1.5, MaxCD(0.01, static_cast<double>(pp.sFOVy) * s05f));
    pp.ppPlOpticZ = static_cast<float>(std::cos(d) * d / std::sin(d));
    pp.Zcorr = st.MCTparas.Zcorr;
    pp.ZcMul = st.MCTparas.ZcMul;
    pp.ZZstmitDif = st.MCTparas.sZZstmitDif;
    pp.ppXOff = st.MCTparas.FOVXoff / std::max(1, st.MCTparas.iMandWidth);

    NormaliseMatrixToS(pp.m, 1.0f, &st.MCTparas.Vgrads);

    // BackDist
    {
        if (std::fabs(pp.ZcMul) > 1e-20) {
            double ratio = 8388352.0 / pp.ZcMul;
            pp.BackDist = (std::pow(ratio + 1.0, 2.0) - 1.0) * pp.StepWidth / pp.Zcorr;
        } else {
            pp.BackDist = 1e10;
        }
    }

    st.PLV.PSmatrix = &pp.m;
    st.PLV.iPlanarOptic = pp.ppPlanarOptic;
    st.PLV.PlOpticZ = pp.ppPlOpticZ;
    st.PLV.xOff = pp.ppXOff;
    st.PLV.sFOVy = pp.sFOVy;

    // Volume light map is precomputed by the caller on the main thread
    // (see renderer.cpp Render() / Pascal CalcSR.pas:107). We intentionally
    // do not compute it here, because CalcReflections may be invoked from a
    // worker context and the map mutates a shared global.

    // Process pixels
    SR_ProcessRows(st, siLight, imageBuffer, width, height, mcp, bInsideTmp, 0, height, 0, width);
}

// Row-range variant for multithreaded rendering
void CalcReflectionsRows(
    TsiLight5* siLight,
    uint8_t* imageBuffer,
    int width, int height,
    int slOffset,
    TMCTparameter* mcp,
    TLightVals* lvals,
    const TMandHeader10* header,
    const THeaderCustomAddon* addon,
    int y_start, int y_end,
    int x_start, int x_end
) {
    RenderPoll();
    SRCalcState st;
    std::memset(&st, 0, sizeof(SRCalcState));

    st.MCTparas = *mcp;
    st.LVals = *lvals;
    st.calcStop = 0;

    TLValigned localAligned;
    if (lvals->PLValigned)
        std::memcpy(&localAligned, lvals->PLValigned, sizeof(TLValigned));
    else
        std::memset(&localAligned, 0, sizeof(TLValigned));
    st.LValigned = localAligned;
    st.LVals.PLValigned = &st.LValigned;

    std::memset(&st.Iteration3Dext, 0, sizeof(TIteration3Dext));
    st.MCTparas.pIt3Dext = &st.Iteration3Dext;
    IniIt3D(&st.MCTparas, &st.Iteration3Dext);

    LongBool bInsideTmp = st.MCTparas.bInsideRendering;
    st.bTransFlipInside = bInsideTmp;

    st.SRLightAmount = lvals->SRLightAmount;
    st.MaxReflections = header->SRreflectioncount;
    if (st.MaxReflections < 1) st.MaxReflections = 1;

    float sRI = ((addon != nullptr) && ((addon->bOptions2 & 6) == 4))
        ? 1.0f
        : header->sTRIndex;
    if (sRI <= 0.0f) sRI = 1.5f;
    st.TransDiConst = sRI;

    st.bCalcTrans = (header->bCalcSRautomatic & 2) ? 1 : 0;
    st.sAbsorption = ReflectionAbsorptionExponent(header->dStepWidth, header->sTransmissionAbsorption);
    st.bOnlyDIFS = (header->bCalcSRautomatic & 4) ? 1 : 0;
    st.sLightScatteringMul = header->sTRscattering / 330.0f;
    st.LVsdiff = lvals->sDiff;
    st.LVals.sObjLightDecreaser = std::max(s05f, 1.0f - std::sqrt(st.SRLightAmount) * 0.17f);

    auto& pp = st.PaintParameter;
    pp.ppWidth = header->Width;
    pp.ppHeight = header->Height;
    pp.ppYinc = 1;
    pp.PLVals = &st.LVals;
    pp.pVgrads = &st.MCTparas.Vgrads;
    pp.StepWidth = st.MCTparas.StepWidth;
    pp.sFOVy = static_cast<float>(st.MCTparas.FOVy);
    pp.ppPlanarOptic = st.MCTparas.MCTCameraOptic;

    double d = MinCD(1.5, MaxCD(0.01, static_cast<double>(pp.sFOVy) * s05f));
    pp.ppPlOpticZ = static_cast<float>(std::cos(d) * d / std::sin(d));
    pp.Zcorr = st.MCTparas.Zcorr;
    pp.ZcMul = st.MCTparas.ZcMul;
    pp.ZZstmitDif = st.MCTparas.sZZstmitDif;
    pp.ppXOff = st.MCTparas.FOVXoff / std::max(1, st.MCTparas.iMandWidth);

    NormaliseMatrixToS(pp.m, 1.0f, &st.MCTparas.Vgrads);

    {
        if (std::fabs(pp.ZcMul) > 1e-20) {
            double ratio = 8388352.0 / pp.ZcMul;
            pp.BackDist = (std::pow(ratio + 1.0, 2.0) - 1.0) * pp.StepWidth / pp.Zcorr;
        } else {
            pp.BackDist = 1e10;
        }
    }

    st.PLV.PSmatrix = &pp.m;
    st.PLV.iPlanarOptic = pp.ppPlanarOptic;
    st.PLV.PlOpticZ = pp.ppPlOpticZ;
    st.PLV.xOff = pp.ppXOff;
    st.PLV.sFOVy = pp.sFOVy;

    // The volumetric light map MUST be precomputed on the main thread before
    // this row worker runs (see renderer.cpp Render() and Pascal CalcSR.pas:107).
    // Calling CalcVolLightMap here races across worker threads: memset over a
    // struct containing std::vector<float> + concurrent resize corrupts the heap.
    // We simply trust that CurrentRenderContext().volume_light_map is already populated when vol-light
    // is enabled for the scene.

    const int clampedXStart = std::max(0, x_start);
    const int clampedXEnd = (x_end < 0) ? width : std::min(width, x_end);
    if (clampedXStart >= clampedXEnd) {
        return;
    }

    SR_ProcessRows(st, siLight, imageBuffer, width, height, mcp, bInsideTmp,
                   y_start, y_end, clampedXStart, clampedXEnd);
}

// Static helper: processes rows [y_start, y_end)
static void SR_ProcessRows(SRCalcState& st, TsiLight5* siLight, uint8_t* imageBuffer,
                           int width, int height, TMCTparameter* mcp,
                           LongBool bInsideTmp, int y_start, int y_end,
                           int x_start, int x_end) {
    auto& pp = st.PaintParameter;

    for (int y = y_start; y < y_end; y++) {
        RenderPoll();
        const int screenY = y + mcp->CalcRect.Top;
        st.MCTparas.CAFY = (static_cast<double>(screenY) / mcp->iMandHeight - s05f) * mcp->FOVy;

        for (int x = x_start; x < x_end; x++) {
            RenderWork();
            ScopedParityTracePixel tracePixel(x, y);
            int idx = y * width + x;
            st.srPsiLight = &siLight[idx];

            if (st.srPsiLight->Zpos >= 32768) continue;

            // In-and-outside modes
            if (st.MCTparas.bInAndOutside && (st.srPsiLight->OTrap & 0x8000) != 0) {
                st.MCTparas.bInsideRendering = 0;
                st.MCTparas.bCalcInside = 0;
            } else {
                st.MCTparas.bInsideRendering = bInsideTmp;
                st.MCTparas.bCalcInside = bInsideTmp;
            }

            IniIt3D(&st.MCTparas, &st.Iteration3Dext);
            const int screenX = x + mcp->CalcRect.Left;
            RMCalculateVgradsFOV(&st.MCTparas, screenX + 1);
            RMCalculateStartPos(&st.MCTparas, screenX, screenY);

            // Light setup
            SR_DVecToSVec(st.PLV.CamPos,
                          st.Iteration3Dext.C1 - st.MCTparas.Xmit,
                          st.Iteration3Dext.C2 - st.MCTparas.Ymit,
                          st.Iteration3Dext.C3 - st.MCTparas.Zmit);
            SR_DVecToSVec(st.PLV.AbsViewVec,
                          st.MCTparas.mVgradsFOV[0],
                          st.MCTparas.mVgradsFOV[1],
                          st.MCTparas.mVgradsFOV[2]);
            NormalizeSVec_inplace(st.PLV.AbsViewVec);
            std::memcpy(st.PLV.ViewVec, st.PLV.AbsViewVec, sizeof(TSVec));
            RotateSVectorReverseS(&st.PLV.ViewVec, &pp.m);
            st.PLV.yPos = static_cast<float>(screenY) / mcp->iMandHeight;
            st.PLV.xPos = static_cast<float>(screenX + 1) / mcp->iMandWidth;
            PreCalcDepthCol(st.LVals.iDfunc, &st.PLV, st.LVals.PLValigned);
            SR_CalcLightZPos(st);

            st.Iteration3Dext.CalcSIT = 0;

            // Decode ZZ
            uint32_t packed = 0;
            std::memcpy(&packed, &st.srPsiLight->RoughZposFine, 4);
            packed >>= 8;
            st.MCTparas.mZZ = SR_GetZPosFromSI(static_cast<int>(packed),
                                                 st.MCTparas.ZcMul, st.MCTparas.Zcorr);
            AddPackedVecWeight(&st.Iteration3Dext.C1,
                          st.MCTparas.mVgradsFOV, st.MCTparas.mZZ);
            st.MCTparas.msDEstop = st.MCTparas.DEstop *
                (1.0f + static_cast<float>(st.MCTparas.mZZ) * st.MCTparas.mctDEstopFactor);

            // Surface refinement
            if (st.srPsiLight->SIgradient < 32768) {
                double dTmp = st.MCTparas.CalcDE(&st.Iteration3Dext, &st.MCTparas);
                if (dTmp < st.MCTparas.msDEstop * s05f) continue;
                LongBool DElimited = (st.Iteration3Dext.ItResultI < st.MCTparas.MaxItsResult) ||
                                     (dTmp <= st.MCTparas.msDEstop);
                if (DElimited) {
                    double dT1 = st.MCTparas.mZZ -
                        SR_GetZPosFromSI(static_cast<int>(packed) - 1,
                                         st.MCTparas.ZcMul, st.MCTparas.Zcorr);
                    RMdoBinSearch(&st.MCTparas, dTmp, dT1);
                } else {
                    RMdoBinSearchIt(&st.MCTparas, &st.MCTparas.mZZ);
                }
            } else {
                // Cut plane
                double dT1 = st.MCTparas.Zend;
                int itmpCut = 0;
                if (st.MCTparas.iCutOptions != 0)
                    SR_minLengthToCutPlane(dT1, itmpCut, -2.0,
                                           st.Iteration3Dext.C1, st.Iteration3Dext.C2, st.Iteration3Dext.C3,
                                           st.MCTparas.mVgradsFOV[0], st.MCTparas.mVgradsFOV[1],
                                           st.MCTparas.mVgradsFOV[2], st.MCTparas);
                if (std::fabs(dT1) > 1.0) continue;
                st.MCTparas.mZZ += dT1;
                AddPackedVecWeight(&st.Iteration3Dext.C1, st.MCTparas.mVgradsFOV, dT1);
                st.MCTparas.msDEstop = st.MCTparas.DEstop *
                    (1.0f + static_cast<float>(st.MCTparas.mZZ) * st.MCTparas.mctDEstopFactor);
            }

            // Get normal
            MakeDVecFromNormals(st.srPsiLight, st.Normal);

            // PLV positions
            st.PLV.zPos = static_cast<float>(st.MCTparas.mZZ * st.MCTparas.StepWidth +
                                              st.MCTparas.sZZstmitDif);
            SR_DVecToSVec(st.PLV.ObjPos,
                          st.Iteration3Dext.C1 - st.MCTparas.Xmit,
                          st.Iteration3Dext.C2 - st.MCTparas.Ymit,
                          st.Iteration3Dext.C3 - st.MCTparas.Zmit);
            st.LVals.ZposDynFog = st.PLV.zPos;

            st.bCalcTransR = st.bCalcTrans && ((!st.bOnlyDIFS) || (st.MCTparas.DEoptionResult == 20));
            st.LVals.bScaleAmbDiffDown = st.bCalcTransR ? 1 : 0;

            const bool tracePrimaryPixel =
                HasParityTraceContext() &&
                x == GetParityTracePixelX() &&
                y == GetParityTracePixelY();

            if (tracePrimaryPixel && ShouldLogParityStage("sr_primary_inputs")) {
                std::ostringstream payload;
                payload.setf(std::ios::fixed, std::ios::floatfield);
                payload << std::setprecision(12)
                        << " xPos=" << st.PLV.xPos
                        << " yPos=" << st.PLV.yPos
                        << " zPos=" << st.PLV.zPos
                        << " ObjPosX=" << st.PLV.ObjPos[0]
                        << " ObjPosY=" << st.PLV.ObjPos[1]
                        << " ObjPosZ=" << st.PLV.ObjPos[2]
                        << " ViewVecX=" << st.PLV.ViewVec[0]
                        << " ViewVecY=" << st.PLV.ViewVec[1]
                        << " ViewVecZ=" << st.PLV.ViewVec[2]
                        << " AbsViewVecX=" << st.PLV.AbsViewVec[0]
                        << " AbsViewVecY=" << st.PLV.AbsViewVec[1]
                        << " AbsViewVecZ=" << st.PLV.AbsViewVec[2]
                        << " PreDepthColX=" << st.PLV.PreDepthCol[0]
                        << " PreDepthColY=" << st.PLV.PreDepthCol[1]
                        << " PreDepthColZ=" << st.PLV.PreDepthCol[2]
                        << " Shadow=" << st.srPsiLight->Shadow
                        << " AmbShadow=" << st.srPsiLight->AmbShadow
                        << " Zpos=" << st.srPsiLight->Zpos
                        << " RoughZposFine=" << st.srPsiLight->RoughZposFine
                        << " SIgradient=" << st.srPsiLight->SIgradient
                        << " OTrap=" << st.srPsiLight->OTrap
                        << " bAmbRelObj=" << (st.LVals.bAmbRelObj ? 1 : 0)
                        << " bUseSmallBGpicForAmb=" << (st.LVals.bUseSmallBGpicForAmb ? 1 : 0)
                        << " bCalcPixColSqr=" << (st.LVals.bCalcPixColSqr ? 1 : 0)
                        << " bBackBMP=" << (st.LVals.bBackBMP ? 1 : 0)
                        << " bScaleAmbDiffDown=" << (st.LVals.bScaleAmbDiffDown ? 1 : 0)
                        << " bCalcTransR=" << (st.bCalcTransR ? 1 : 0)
                        << " SRLightAmount=" << st.SRLightAmount;
                AppendParityTrace("sr_primary_inputs", payload.str());
                if (DebugPrintSrPrimaryToStderr()) {
                    std::fprintf(stderr, "SR_PRIMARY_INPUTS %s\n", payload.str().c_str());
                    std::fflush(stderr);
                }
            }

            // Initial surface color
            TLightSD SDsv;
            TSVec transpResult;
            PushPaintColorsInterpTraceSuppression();
            if (tracePrimaryPixel) PushPaintSvecTraceScope();
            CalcPixelColorSvec(&st.tAmb, SDsv, st.srPsiLight, &st.LVals, &st.PLV, transpResult);
            if (tracePrimaryPixel) PopPaintSvecTraceScope();
            PopPaintColorsInterpTraceSuppression();
            ScaleSVectorV(&transpResult, st.SRLightAmount);
            st.tAmb[3] = 0.0f;
            st.tAmb[3] = 0;

            TSVec sv;
            std::memcpy(sv, transpResult, sizeof(TSVec));

            if (tracePrimaryPixel && ShouldLogParityStage("sr_primary")) {
                std::ostringstream payload;
                payload.setf(std::ios::fixed, std::ios::floatfield);
                payload << std::setprecision(12)
                        << " tAmbX=" << st.tAmb[0]
                        << " tAmbY=" << st.tAmb[1]
                        << " tAmbZ=" << st.tAmb[2]
                        << " svX=" << sv[0]
                        << " svY=" << sv[1]
                        << " svZ=" << sv[2]
                        << " SD0X=" << SDsv[0][0]
                        << " SD0Y=" << SDsv[0][1]
                        << " SD0Z=" << SDsv[0][2]
                        << " SD0W=" << SDsv[0][3]
                        << " SD1X=" << SDsv[1][0]
                        << " SD1Y=" << SDsv[1][1]
                        << " SD1Z=" << SDsv[1][2]
                        << " SD1W=" << SDsv[1][3]
                        << " bCalcTransR=" << (st.bCalcTransR ? 1 : 0)
                        << " SRLightAmount=" << st.SRLightAmount;
                AppendParityTrace("sr_primary", payload.str());
                if (DebugPrintSrPrimaryToStderr()) {
                    std::fprintf(stderr, "SR_PRIMARY %s\n", payload.str().c_str());
                    std::fflush(stderr);
                }
                if (DebugStopAfterSrPrimary()) {
                    st.calcStop = 1;
                    return;
                }
            }

            // Recursive reflection + transmission
            if (tracePrimaryPixel) PushPaintSvecTraceScope();
            TVec3D reflectionDirection;
            std::memcpy(reflectionDirection, st.MCTparas.mVgradsFOV, sizeof(reflectionDirection));
            SR_CalcRay(st.MCTparas.mZZ, reflectionDirection, sv, SDsv, 1, st);
            if (tracePrimaryPixel) PopPaintSvecTraceScope();

            // Clamp
            TSVec finalColor;
            mMinMaxSVec(finalColor, 0.0f, 1.0f, st.tAmb);

            // Sqrt gamma
            if (st.LVals.bCalcPixColSqr) {
                TSVec tmp;
                mSqrtSVec(tmp, finalColor);
                std::memcpy(finalColor, tmp, sizeof(TSVec));
            }

            // Additional gamma
            if (st.LVals.iGammaH != 0) {
                TSVec ga;
                if (st.LVals.iGammaH > 0) {
                    mSqrtSVec(ga, finalColor);
                } else {
                    ga[0] = finalColor[0] * finalColor[0];
                    ga[1] = finalColor[1] * finalColor[1];
                    ga[2] = finalColor[2] * finalColor[2];
                    ga[3] = 0;
                }
                for (int c = 0; c < 3; c++)
                    finalColor[c] += (ga[c] - finalColor[c]) * st.LVals.sGamma;
            }

            // Write RGB
            int r = std::max(0, std::min(255, static_cast<int>(std::round(finalColor[0] * s255f))));
            int g = std::max(0, std::min(255, static_cast<int>(std::round(finalColor[1] * s255f))));
            int b = std::max(0, std::min(255, static_cast<int>(std::round(finalColor[2] * s255f))));
            imageBuffer[idx * 3 + 0] = static_cast<uint8_t>(r);
            imageBuffer[idx * 3 + 1] = static_cast<uint8_t>(g);
            imageBuffer[idx * 3 + 2] = static_cast<uint8_t>(b);

            ClearSVec(st.tAmb);
        }
    }
}

// ============================================================================
// Volume Light Map
// Ported from mb3d/maps/MB3DMaps.pas:
//   CalcVolLightMap, MakeVolumicLightMapThreads,
//   TCalcVolumetricLightMapThread.Execute
// ============================================================================

// Helper: Make a 3D vector from components
static void MakeVecD(TVec3D& result, double x, double y, double z) {
    result[0] = x; result[1] = y; result[2] = z;
}

// Helper: Invert a 3x3 single-precision matrix (transpose for orthonormal)
static void InvertSMatrix(TSMatrix3& m) {
    // For an orthonormal rotation matrix, inverse == transpose
    float tmp;
    tmp = m[0][1]; m[0][1] = m[1][0]; m[1][0] = tmp;
    tmp = m[0][2]; m[0][2] = m[2][0]; m[2][0] = tmp;
    tmp = m[1][2]; m[1][2] = m[2][1]; m[2][1] = tmp;
}

// Helper: Make orthogonal vectors from a direction vector
static void MakeOrthoVecs(const TVec3D& dir, TSMatrix3& m) {
    // Literal Math3D.MakeOrthoVecs basis; arbitrary alternative bases change
    // which finite grid cells are sampled by the directional shadow map.
    TVec3D n = {dir[0], dir[1], dir[2]};
    NormalizeVec3D_inplace(n);
    if (std::fabs(n[0]) > 0.1) {
        double d = 1.0 / std::sqrt(n[0]*n[0] + n[2]*n[2]);
        m[0][0] = static_cast<float>(n[2] * d);
        m[0][1] = 0;
        m[0][2] = static_cast<float>(-n[0] * d);
    } else {
        double d = 1.0 / std::sqrt(n[1]*n[1] + n[2]*n[2]);
        m[0][0] = 0;
        m[0][1] = static_cast<float>(-n[2] * d);
        m[0][2] = static_cast<float>(n[1] * d);
    }
    m[1][0] = static_cast<float>(m[0][1]*n[2] - m[0][2]*n[1]);
    m[1][1] = static_cast<float>(m[0][2]*n[0] - m[0][0]*n[2]);
    m[1][2] = static_cast<float>(m[0][0]*n[1] - m[0][1]*n[0]);
    m[2][0] = static_cast<float>(dir[0]);
    m[2][1] = static_cast<float>(dir[1]);
    m[2][2] = static_cast<float>(dir[2]);
    m[0][3] = m[1][3] = m[2][3] = 0;
}

// Helper: Add SVec to Vec3D in place
static void AddSVecToVec3D(const TSVec& sv, TVec3D& dv) {
    dv[0] += sv[0];
    dv[1] += sv[1];
    dv[2] += sv[2];
}

// Helper: Scale single row of TSMatrix3, return as SVec
static void ScaleSMatrixRow(TSVec& result, const TSMatrix3& m, int row, float scale) {
    result[0] = m[row][0] * scale;
    result[1] = m[row][1] * scale;
    result[2] = m[row][2] * scale;
    result[3] = 0.0f;
}

// Change sign of SVec
static void SVectorChangeSign(TSVec& sv) {
    sv[0] = -sv[0];
    sv[1] = -sv[1];
    sv[2] = -sv[2];
}

// RotateVectorS: rotate a double vec by single-precision matrix
static void RotateVectorS(TVec3D& v, const TSMatrix3& m) {
    double x = v[0], y = v[1], z = v[2];
    v[0] = x * m[0][0] + y * m[1][0] + z * m[2][0];
    v[1] = x * m[0][1] + y * m[1][1] + z * m[2][1];
    v[2] = x * m[0][2] + y * m[1][2] + z * m[2][2];
}

static void VLM_ComputeOneRay(
    TVolumeLightMap& vlm,
    TMCTparameter& mcp,
    TIteration3Dext& it3d,
    int side, int x, int y, int HS,
    float sgl, float maxDist,
    const TVec3D& dirVec, const TSMatrix3& invMs)
{
    RenderPoll();
    TVec3D vec;
    float dist;

    if (vlm.IsPosLight) {
        switch (side) {
            case 0: MakeVecD(vec, HS, x, y); break;
            case 1: MakeVecD(vec, -HS, -x, -y); break;
            case 2: MakeVecD(vec, x, HS, y); break;
            case 3: MakeVecD(vec, -x, -HS, -y); break;
            case 4: MakeVecD(vec, x, y, HS); break;
            case 5: MakeVecD(vec, -x, -y, -HS); break;
            default: MakeVecD(vec, HS, x, y); break;
        }
        if (vlm.Rotate) RotateVectorS(vec, vlm.RotMatrix);
        NormalizeVec3D_inplace(vec);
        it3d.C1 = vlm.LightPos[0]; it3d.C2 = vlm.LightPos[1]; it3d.C3 = vlm.LightPos[2];
        AddPackedVecWeight(&it3d.C1, vec, static_cast<double>(vlm.MinDistance));
        dist = vlm.MinDistance / static_cast<float>(mcp.StepWidth);
    } else {
        // Directional light: offset from center
        it3d.C1 = vlm.LightPos[0] + dirVec[0] * vlm.MinDistance;
        it3d.C2 = vlm.LightPos[1] + dirVec[1] * vlm.MinDistance;
        it3d.C3 = vlm.LightPos[2] + dirVec[2] * vlm.MinDistance;

        TSVec rowScaled;
        ScaleSMatrixRow(rowScaled, invMs, 0, x * sgl);
        AddSVecToVec3D(rowScaled, *(TVec3D*)&it3d.C1);
        ScaleSMatrixRow(rowScaled, invMs, 1, y * sgl);
        AddSVecToVec3D(rowScaled, *(TVec3D*)&it3d.C1);

        std::memcpy(vec, dirVec, sizeof(TVec3D));
        dist = 0;
    }

    // March ray to find distance through scene
    float RStepFactorDiff = 1.0f;
    TVec3D position, origin;
    std::memcpy(position, &it3d.C1, sizeof(position));
    std::memcpy(origin, mcp.Ystart, sizeof(origin));
    double zz = DistanceOf2Vecs(position, origin) / mcp.StepWidth;
    mcp.msDEstop = mcp.DEstop * (1.0f + static_cast<float>(zz) * mcp.mctDEstopFactor);
    TVec3D forward;
    std::memcpy(forward, mcp.Vgrads[2], sizeof(forward));
    double zzmul = DotOfVectorsNormalize(forward, vec);

    double ds = mcp.CalcDE(&it3d, &mcp);
    if (mcp.bInAndOutside && ds < mcp.msDEstop) {
        mcp.bCalcInside = !mcp.bCalcInside;
        mcp.bInsideRendering = !mcp.bInsideRendering;
        ds = mcp.CalcDE(&it3d, &mcp);
    }

    float totalDist = 0;
    for (int iter = 0; iter < 10000; iter++) {
        RenderPoll();
        double RLastDE = ds;
        float RLastStepWidth = MinCS(
            MaxCS(s011f, static_cast<float>((ds - mcp.msDEsub * mcp.msDEstop) * mcp.sZstepDiv * RStepFactorDiff)),
            MaxCS(mcp.msDEstop, 0.4f) * mcp.mctMH04ZSD);

        totalDist += RLastStepWidth;
        AddPackedVecWeight(&it3d.C1, vec, static_cast<double>(RLastStepWidth) * mcp.StepWidth);
        zz += RLastStepWidth * zzmul;
        mcp.msDEstop = mcp.DEstop * (1.0f + static_cast<float>(std::fabs(zz)) * mcp.mctDEstopFactor);
        ds = mcp.CalcDE(&it3d, &mcp);

        if (it3d.ItResultI >= mcp.MaxItsResult || ds <= mcp.msDEstop) break;
        if (ds > RLastDE + RLastStepWidth) ds = RLastDE + RLastStepWidth;
        if (RLastDE > ds + 1e-30) {
            float st2 = RLastStepWidth / static_cast<float>(RLastDE - ds);
            RStepFactorDiff = (st2 < 1.0f) ? MaxCS(s05f, st2) : 1.0f;
        } else {
            RStepFactorDiff = 1.0f;
        }
        if (totalDist >= maxDist) {
            totalDist = maxDist * 1000.0f;
            break;
        }
    }

    // Store result
    int pixelIdx = (y + HS) * vlm.CubeSize + (x + HS);
    if (pixelIdx >= 0 && pixelIdx < static_cast<int>(vlm.CubeSides[side].size())) {
        vlm.CubeSides[side][pixelIdx] = static_cast<float>(totalDist * mcp.StepWidth + vlm.MinDistance);
    }
}

void ResetVolLightMap() {
    auto& vlm = CurrentRenderContext().volume_light_map;
    for (int i = 0; i < 6; ++i) vlm.CubeSides[i].clear();
    std::memset(vlm.LightPos, 0, sizeof(TVec3D));
    std::memset(vlm.RotMatrix, 0, sizeof(TSMatrix3));
    vlm.StretchSide1 = 0.0f;
    vlm.MinDistance = 0.0f;
    vlm.SizeFactor = 0.0f;
    vlm.CubeSize = 0;
    vlm.HalfSize = 0;
    vlm.SideCount = 0;
    vlm.HeightS2to5 = 0;
    vlm.CsizeS = 0.0f;
    vlm.HSizeS = 0.0f;
    vlm.IsPosLight = false;
    vlm.Rotate = false;
    vlm.Initialized = false;
    CurrentRenderContext().volume_light_map_calculated = false;
}

bool PrepareVolLightMap(
    const TMandHeader10* header,
    TLightVals* lvals,
    const TMCTparameter* source,
    TVolumeLightMap& vlm,
    TVolumeLightMapSetup& setup)
{
    int vlNr = header->bVolLightNr & 7;
    if (vlNr == 0) return false;
    vlNr = std::min(5, vlNr - 1);

    // Check if the light is enabled
    if ((header->Light.Lights[vlNr].Loption & 3) != 0) return false;

    // Pascal prepares a private parameter record and centers Ystart for
    // the map's distance-dependent DE threshold; preserve the primary camera.
    setup.params = *source;
    TMCTparameter* mcp = &setup.params;
    if (header->bPlanarOptic != 2) {
        AddPackedVecWeight(mcp->Ystart, mcp->Vgrads[0], mcp->iMandWidth * 0.5);
        AddPackedVecWeight(mcp->Ystart, mcp->Vgrads[1], mcp->iMandHeight * 0.5);
    }

    // NOTE: TVolumeLightMap contains std::vector<float> CubeSides[6], so we
    // MUST NOT memset the whole struct to zero (that would clobber the vector
    // control blocks and cause UB). Reset every field explicitly instead.
    for (int i = 0; i < 6; ++i) vlm.CubeSides[i].clear();
    std::memset(vlm.LightPos, 0, sizeof(TVec3D));
    std::memset(vlm.RotMatrix, 0, sizeof(TSMatrix3));
    vlm.StretchSide1 = 0.0f;
    vlm.MinDistance = 0.0f;
    vlm.SizeFactor = 0.0f;
    vlm.CubeSize = 0;
    vlm.HalfSize = 0;
    vlm.SideCount = 0;
    vlm.HeightS2to5 = 0;
    vlm.CsizeS = 0.0f;
    vlm.HSizeS = 0.0f;
    vlm.IsPosLight = false;
    vlm.Rotate = false;
    vlm.Initialized = false;
    float s = static_cast<float>(std::max(mcp->iMandWidth, mcp->iMandHeight));
    float sft = static_cast<float>(1.0 + 0.2 * ((header->bVolLightNr >> 4) - 2));

    vlm.IsPosLight = (header->Light.Lights[vlNr].Loption & 4) != 0;

    if (vlm.IsPosLight) {
        vlm.MinDistance = 0;
        vlm.Rotate = false;
        std::memcpy(vlm.LightPos, lvals->lvMidPos, sizeof(TVec3D));
        AddSVecToVec3D(lvals->PLValigned->LN[lvals->SortTab[vlNr]], vlm.LightPos);
        vlm.SideCount = 6;
        vlm.CubeSize = (static_cast<int>(std::lrint(s * sft * 0.25f)) | 1);
        vlm.StretchSide1 = 1.0f;
    } else {
        double dTmp = MinCD(mcp->Zend, s * 16.0);
        float sm = static_cast<float>(MaxCD(dTmp, static_cast<double>(s)) / s);
        s = static_cast<float>(s * std::pow(sm, 0.36));
        dTmp *= 0.5;
        vlm.Rotate = true;
        vlm.SideCount = 1;
        vlm.CubeSize = (static_cast<int>(std::lrint(s * sft * 0.625f)) | 1);

        // Compute LightPos at center of scene frustum
        std::memcpy(vlm.LightPos, lvals->lvMidPos, sizeof(TVec3D));
        AddPackedVecWeight(vlm.LightPos, mcp->Vgrads[2],
                      mcp->sZZstmitDif / mcp->StepWidth + dTmp);

        // Compute rotation matrix from light direction
        TSVec sv;
        std::memcpy(sv, lvals->PLValigned->LN[lvals->SortTab[vlNr]], sizeof(TSVec));
        const double vx = sv[0], vy = sv[1], vz = sv[2];
        for (int c = 0; c < 3; ++c)
            sv[c] = static_cast<float>(vx * mcp->Vgrads[0][c] + vy * mcp->Vgrads[1][c] + vz * mcp->Vgrads[2][c]);
        SVectorChangeSign(sv);
        NormalizeSVec_inplace(sv);

        TVec3D dv;
        SVecToDVec(dv, sv);
        MakeOrthoVecs(dv, vlm.RotMatrix);
        std::memcpy(vlm.RotMatrix[2], &sv, 3 * sizeof(float));
        InvertSMatrix(vlm.RotMatrix);

        dTmp = mcp->StepWidth * dTmp * 2.0 * std::pow(std::max(1.0f, sft), 0.25);
        vlm.StretchSide1 = vlm.CubeSize / static_cast<float>(dTmp * 1.75);
        vlm.MinDistance = static_cast<float>(-dTmp * mcp->sHSmaxLengthMultiplier);
    }

    vlm.HalfSize = vlm.CubeSize / 2;
    vlm.SizeFactor = (vlm.CubeSize - 1) * 0.5f;
    vlm.CsizeS = static_cast<float>(vlm.CubeSize - 1);
    vlm.HSizeS = static_cast<float>(vlm.HalfSize);
    vlm.HeightS2to5 = vlm.CubeSize;

    setup.sgl = 1.0f / vlm.StretchSide1;
    std::memset(setup.dirVec, 0, sizeof(TVec3D));
    std::memset(setup.invMs, 0, sizeof(TSMatrix3));
    if (vlm.IsPosLight) {
        setup.maxDist = static_cast<float>(mcp->sHSmaxLengthMultiplier * MaxCD(vlm.CubeSize * 3.0, mcp->Zend * 0.5));
    } else {
        std::memcpy(setup.invMs, vlm.RotMatrix, sizeof(TSMatrix3));
        InvertSMatrix(setup.invMs);
        setup.dirVec[0] = setup.invMs[2][0]; setup.dirVec[1] = setup.invMs[2][1]; setup.dirVec[2] = setup.invMs[2][2];
        setup.maxDist = static_cast<float>(std::fabs(vlm.MinDistance) * 2.0 / mcp->StepWidth);
    }
    return true;
}

void CalcVolLightMap(
    const TMandHeader10* header,
    TLightVals* lvals,
    TMCTparameter* source,
    int numThreads)
{
    RenderPoll();
    int vlNr = header->bVolLightNr & 7;
    if (vlNr == 0) return;
    vlNr = std::min(5, vlNr - 1);

    // Check if the light is enabled
    if ((header->Light.Lights[vlNr].Loption & 3) != 0) return;

    // Already computed?
    if (CurrentRenderContext().volume_light_map_calculated) return;

    auto& vlm = CurrentRenderContext().volume_light_map;
    TVolumeLightMapSetup setup;
    if (!PrepareVolLightMap(header, lvals, source, vlm, setup)) return;
    TMCTparameter* mcp = &setup.params;

    // Allocate cube sides
    for (int i = vlm.SideCount; i < 6; i++) vlm.CubeSides[i].clear();
    for (int i = 0; i < vlm.SideCount; i++) {
        RenderPoll();
        vlm.CubeSides[i].resize(vlm.CubeSize * vlm.CubeSize, 0.0f);
    }

    // Compute rays. Pascal MB3DMaps.pas:345 spawns up to
    // Min(UpDown3.Position, VolumeLightMap.CubeSize) threads, and each worker
    // walks a stride'd slice of y rows (Pascal MB3DMaps.pas:396,469). We
    // mirror that topology: each thread owns a local TMCTparameter +
    // TIteration3Dext copy (mcp.CalcDE / mMandFunction only touch per-thread
    // state, so the pattern is race-free), each ray writes a unique pixel
    // index in vlm.CubeSides, and no synchronization is required inside the
    // inner ray loop.
    //
    // Before this parallelization, scenes with an expensive per-formula-step
    // CalcDE (e.g. Bulbox zoom=70 where every ray marches ~200 iterations of
    // a hybrid-blend formula at ~5us each under the Rout3_x87_pascw precision
    // fence) pushed the serial map-compute past 13 minutes. Pascal runs the
    // same work multi-threaded at CalcVolLightMap time; without this
    // parallelism Pascal-vs-C++ sweeps saw bulbox time out at 180s while
    // Pascal returned in ~5s (scenes_phase3_vollight/phase3_vl_bulbox_medium).

    LongBool bInsideTmp = mcp->bInsideRendering;
    int HS = vlm.HalfSize;
    float sgl = setup.sgl;
    float maxDist = setup.maxDist;
    const TVec3D& dirVec = setup.dirVec;
    const TSMatrix3& invMs = setup.invMs;

    // Pick a thread count. Pascal caps at Min(UpDown3, CubeSize); match that
    // bound (a thread per row gives no benefit and wastes coordination
    // overhead). Default to hardware_concurrency() when the caller passes 0.
    int threadCount = numThreads;
    if (threadCount <= 0) {
        threadCount = static_cast<int>(std::thread::hardware_concurrency());
        if (threadCount <= 0) threadCount = 4;
    }
    threadCount = std::max(1, std::min(threadCount, vlm.CubeSize));

    auto worker = [&](int tid) {
        // Each thread needs its own TMCTparameter copy + TIteration3Dext:
        // the inner CalcDE path mutates pIt3Dext fields, msDEstop,
        // bCalcInside, and several scratch slots. Pascal gives each thread
        // its own TMCTparameter via the thread's with-block copy; we mirror
        // that here. Copying the whole struct is cheap relative to the ray
        // loop cost (millions of CalcDE calls per ray).
        TMCTparameter localMcp = *mcp;
        TIteration3Dext localIt3d;
        std::memset(&localIt3d, 0, sizeof(TIteration3Dext));
        localMcp.pIt3Dext = &localIt3d;
        localMcp.iThreadId = tid + 1;
        localMcp.iThreadCount = threadCount;
        IniIt3D(&localMcp, &localIt3d);

        for (int side = 0; side < vlm.SideCount; side++) {
            // Stride'd row partition mirroring Pascal MB3DMaps.pas:396,469:
            //   y := Integer(iThreadID) - HS - 1;
            //   ...; Inc(y, iThreadCount);
            // Pascal's iThreadID is 1-based, so first y = tid (0-based) - HS.
            for (int y = -HS + tid; y <= HS; y += threadCount) {
                RenderPoll();
                for (int x = -HS; x <= HS; x++) {
                    RenderPoll();
                    localMcp.bCalcInside = bInsideTmp;
                    localMcp.bInsideRendering = bInsideTmp;

                    VLM_ComputeOneRay(vlm, localMcp, localIt3d,
                                      side, x, y, HS,
                                      sgl, maxDist, dirVec, invMs);
                }
            }
        }
    };

    RunParallelWorkers(threadCount, worker);

    vlm.Initialized = true;
    CurrentRenderContext().volume_light_map_calculated = true;
    if (const char* path = std::getenv("MB3D_VLM_DUMP")) {
        // Diagnostic layout: int32 size/sides, float stretch/minDistance,
        // double lightPos[3], float matrix[3][4], then float cells per side.
        std::ofstream dump(path, std::ios::binary);
        dump.write(reinterpret_cast<const char*>(&vlm.CubeSize), 4);
        dump.write(reinterpret_cast<const char*>(&vlm.SideCount), 4);
        dump.write(reinterpret_cast<const char*>(&vlm.StretchSide1), 4);
        dump.write(reinterpret_cast<const char*>(&vlm.MinDistance), 4);
        dump.write(reinterpret_cast<const char*>(vlm.LightPos), 24);
        dump.write(reinterpret_cast<const char*>(vlm.RotMatrix), 48);
        for (int side = 0; side < vlm.SideCount; ++side)
            dump.write(reinterpret_cast<const char*>(vlm.CubeSides[side].data()), vlm.CubeSides[side].size() * sizeof(float));
        if (!dump) throw std::runtime_error("cannot write volumetric map dump");
    }
}

double VolumeLightDirectionSquaredLength(const TVec3D& direction) {
    // SubtractVectors2s stores Single components; the x87 SqrLengthOfSVec
    // return is then assigned to the caller's Double d1 without a Single store.
    const double x=static_cast<float>(direction[0]);
    const double y=static_cast<float>(direction[1]);
    const double z=static_cast<float>(direction[2]);
    return (x*x+y*y)+z*z;
}

float SamplePositionalVolumeLightMap(const TVolumeLightMap& vlm, const TVec3D& direction) {
    if (!vlm.Initialized || vlm.SideCount != 6 || vlm.CubeSize <= 0) return 1e10f;
    // Both ordinary callers use SubtractVectors2s before invoking the SSE
    // sampler. Dominant-axis selection, division and products use Single.
    const float v[3] = {static_cast<float>(direction[0]), static_cast<float>(direction[1]),
                        static_cast<float>(direction[2])};
    const float ax=std::fabs(v[0]), ay=std::fabs(v[1]), az=std::fabs(v[2]);
    const int axis = ax>=ay ? (ax>=az ? 0 : 2) : (ay>=az ? 1 : 2);
    if (!std::isfinite(ax) || !std::isfinite(ay) || !std::isfinite(az) || v[axis]==0)
        throw std::invalid_argument("positional volume lookup requires a finite nonzero direction; original SSE indexes outside the map otherwise");
    const int face=axis*2+(v[axis]<0 ? 1 : 0);
    const float scale=vlm.SizeFactor/v[axis]; // signed component on negative faces too
    const float sx=v[axis==0 ? 1 : 0]*scale;
    const float sy=v[axis==2 ? 1 : 2]*scale;
    if (!std::isfinite(sx) || !std::isfinite(sy))
        throw std::invalid_argument("positional volume projection overflow; original SSE indexes outside the map");
    const int x=static_cast<int>(std::nearbyint(sx))+vlm.HalfSize;
    const int y=static_cast<int>(std::nearbyint(sy))+vlm.HalfSize;
    // With a correctly constructed cube, dominant-axis projection is in range.
    if (x<0 || y<0 || x>=vlm.CubeSize || y>=vlm.CubeSize)
        throw std::invalid_argument("positional volume projection lies outside its cube");
    if (vlm.CubeSides[face].empty()) return 1e10f;
    return vlm.CubeSides[face][y*vlm.CubeSize+x];
}

float GetVolLightMapValue(const TVec3D& direction) {
    if (!CurrentRenderContext().volume_light_map.Initialized) return 1e10f;

    auto& vlm = CurrentRenderContext().volume_light_map;
    int HS = vlm.HalfSize;
    if (HS <= 0) return 1e10f;

    // For directional light (SideCount == 1), project onto the single face
    if (vlm.SideCount == 1 && !vlm.CubeSides[0].empty()) {
        // Transform direction by rotation matrix
        TVec3D vd;
        std::memcpy(vd, direction, sizeof(TVec3D));
        RotateVectorS(vd, vlm.RotMatrix);

        // Project onto face
        float sf = vlm.SizeFactor;
        if (std::fabs(vd[2]) < 1e-30) return 1e10f;
        float scale = sf / static_cast<float>(vd[2]);
        int xi = static_cast<int>(std::round(vd[0] * scale)) + HS;
        int yi = static_cast<int>(std::round(vd[1] * scale)) + HS;

        xi = std::max(0, std::min(vlm.CubeSize - 1, xi));
        yi = std::max(0, std::min(vlm.CubeSize - 1, yi));

        return vlm.CubeSides[0][yi * vlm.CubeSize + xi];
    }

    if (vlm.SideCount == 6) return SamplePositionalVolumeLightMap(vlm, direction);

    return 1e10f;
}

} // namespace mb3d
