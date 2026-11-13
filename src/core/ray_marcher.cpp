#include "core/render_control.h"
#include "core/ray_marcher.h"
#include "core/math3d.h"
#include "core/lightmap.h"
#include "core/formulas.h"
#include "core/header_trafos.h"
#include "core/renderer.h"
#include "core/reflection.h"
#include "core/parity_trace.h"
#include "core/pas_lcg.h"
#include "core/extended80.h"
#include "core/numerical_de.h"
#include "core/numerical_camera.h"
#include <cmath>
#include <cstring>
#include <algorithm>
#include <cstdlib>
#include <sstream>
#include <iomanip>
#include <limits>

namespace mb3d {

#if defined(_MSC_VER) && defined(_M_X64)
extern "C" void DeltaSq_x87(double* out, const double* a, const double* b);
extern "C" void CalcRawDe_x87(double* out_raw, double* out_denom, const double* params);
extern "C" void CalcRawDe_x87_extcw(double* out_raw, double* out_denom, const double* params);
extern "C" void CalcRawDe_x87_pascw(double* out_raw, double* out_denom, const double* params);
extern "C" void DynFogStep_x87(float* out, const void* params);
extern "C" void BuildViewVectorDFOV_x87(double* out_xyz, const double* in);
extern "C" void BuildViewVectorDFOV_x87_passthru(double* out_xyz, const double* in);
extern "C" void BuildRotateViewVectorDFOV_x87_passthru(double* out_xyz, const double* in, const double* matrix9);
extern "C" void BuildRotateViewVectorDFOV_x87_pascw(double* out_xyz, const double* in, const double* matrix9);
extern "C" void RotateVectorReverse_x87(double* vec_io, const double* matrix9);
extern "C" void RotateVectorReverse_x87_extcw(double* vec_io, const double* matrix9);
extern "C" void AccumulateIterationDepth_sse2(double* depth, double step);
extern "C" void OffsetIterationDepth_x87(double* depth);
extern "C" void FinalizeIterationBoundaryStep_x87(double* out, const double* inputs);
extern "C" void EncodeDepthRough_x87(uint32_t* out, const double* in, int smooth);
#endif

struct DynFogStepX87Params {
    double actDE;
    float msDEsub;
    float msDEstop;
    float sZstepDiv;
    float RSFmul;
    float s011;
};

static inline bool DebugEnvOnce(const char* name) {
    const char* spec = std::getenv(name);
    return spec && *spec;
}

static inline bool DebugRotateX87ExtCWEnabled() {
    static const bool value = DebugEnvOnce("MB3D_DEBUG_ROTATE_X87_EXTCW");
    return value;
}

static inline bool DebugBuildViewX87PassthruEnabled() {
    static const bool value = DebugEnvOnce("MB3D_DEBUG_BUILD_VIEW_X87_PASSTHRU");
    return value;
}

static inline bool DebugBuildRotateCombinedX87PassthruEnabled() {
    static const bool value = DebugEnvOnce("MB3D_DEBUG_BUILD_ROTATE_COMBINED_X87_PASSTHRU");
    return value;
}

static inline double DeltaSqPasLike(double lhs, double rhs) {
    // FPC -CfSSE2 rounds the subtraction before squaring it.
    const double delta = lhs - rhs;
    return delta * delta;
}

static inline double CalcRawDePasLike(double bufRout, double dt, double wt, double rst,
                                      double dDEscale, double offset006, double* denomOut) {
    return NumericalDistanceEstimate(bufRout, dt, wt, rst, dDEscale, offset006, denomOut);
}

static inline double DynFogStepPasLike(double actDE, float msDEsub, float msDEstop,
                                       float sZstepDiv, float RSFmul, float s011) {
#if defined(_MSC_VER) && defined(_M_X64)
    float out = 0.0f;
    DynFogStepX87Params params{actDE, msDEsub, msDEstop, sZstepDiv, RSFmul, s011};
    DynFogStep_x87(&out, &params);
    return static_cast<double>(out);
#else
    const float expr = static_cast<float>((actDE - static_cast<double>(msDEsub) *
                                           static_cast<double>(msDEstop)) *
                                          static_cast<double>(sZstepDiv) *
                                          static_cast<double>(RSFmul));
    return static_cast<double>(MaxCS(s011, expr));
#endif
}

static bool ApplyDebugRefineEntryOverride(PMCTparameter pMCTparas,
                                          TPIteration3Dext it3d,
                                          double& dTmp,
                                          double& RLastStepWidth) {
    if (!HasParityTraceContext()) {
        return false;
    }
    const char* spec = std::getenv("MB3D_DEBUG_REFINE_ENTRY_OVERRIDE");
    if (!spec || !*spec) {
        return false;
    }

    int px = 0;
    int py = 0;
    double mzz = 0.0;
    double de = 0.0;
    double lastStep = 0.0;
    double c1 = 0.0;
    double c2 = 0.0;
    double c3 = 0.0;
    double msDEstop = 0.0;
    if (std::sscanf(spec, "%d,%d,%lf,%lf,%lf,%lf,%lf,%lf,%lf",
                    &px, &py, &mzz, &de, &lastStep, &c1, &c2, &c3, &msDEstop) != 9) {
        return false;
    }
    if (px != GetParityTracePixelX() || py != GetParityTracePixelY()) {
        return false;
    }

    pMCTparas->mZZ = mzz;
    dTmp = de;
    RLastStepWidth = lastStep;
    it3d->C1 = c1;
    it3d->C2 = c2;
    it3d->C3 = c3;
    pMCTparas->msDEstop = static_cast<float>(msDEstop);

    std::ostringstream payload;
    payload.setf(std::ios::fixed, std::ios::floatfield);
    payload << std::setprecision(12)
            << "mZZ=" << pMCTparas->mZZ
            << " dTmp=" << dTmp
            << " RLastStepWidth=" << RLastStepWidth
            << " C1=" << it3d->C1
            << " C2=" << it3d->C2
            << " C3=" << it3d->C3
            << " msDEstop=" << pMCTparas->msDEstop;
    AppendParityTrace("refine_entry_override", payload.str());
    return true;
}

static bool ApplyDebugStepCalcdeEntryOverride(PMCTparameter pMCTparas,
                                              TPIteration3Dext it3d,
                                              int traceStep) {
    if (!HasParityTraceContext()) {
        return false;
    }
    const char* spec = std::getenv("MB3D_DEBUG_STEP_CALCDE_ENTRY_OVERRIDE");
    if (!spec || !*spec) {
        return false;
    }

    int px = 0;
    int py = 0;
    int step = -1;
    double mzz = 0.0;
    double c1 = 0.0;
    double c2 = 0.0;
    double c3 = 0.0;
    double msDEstop = 0.0;
    int itResultI = 0;
    double smoothItD = 0.0;
    double rout = 0.0;
    double otrap = 0.0;
    int calcSIT = 0;
    const int parsed = std::sscanf(spec, "%d,%d,%d,%lf,%lf,%lf,%lf,%lf,%d,%lf,%lf,%lf,%d",
                                   &px, &py, &step, &mzz, &c1, &c2, &c3, &msDEstop,
                                   &itResultI, &smoothItD, &rout, &otrap, &calcSIT);
    if (parsed < 8) {
        return false;
    }
    if (px != GetParityTracePixelX() || py != GetParityTracePixelY() || step != traceStep) {
        return false;
    }

    pMCTparas->mZZ = mzz;
    it3d->C1 = c1;
    it3d->C2 = c2;
    it3d->C3 = c3;
    pMCTparas->msDEstop = static_cast<float>(msDEstop);
    if (parsed >= 9) {
        it3d->ItResultI = itResultI;
    }
    if (parsed >= 10) {
        it3d->SmoothItD = smoothItD;
    }
    if (parsed >= 11) {
        it3d->Rout = rout;
    }
    if (parsed >= 12) {
        it3d->OTrap = otrap;
    }
    if (parsed >= 13) {
        it3d->CalcSIT = calcSIT != 0;
    }

    std::ostringstream payload;
    payload.setf(std::ios::fixed, std::ios::floatfield);
    payload << std::setprecision(12)
            << "step_index=" << traceStep
            << " mZZ=" << pMCTparas->mZZ
            << " C1=" << it3d->C1
            << " C2=" << it3d->C2
            << " C3=" << it3d->C3
            << " msDEstop=" << pMCTparas->msDEstop
            << " ItResultI=" << it3d->ItResultI
            << " SmoothItD=" << it3d->SmoothItD
            << " Rout=" << it3d->Rout
            << " OTrap=" << it3d->OTrap
            << " CalcSIT=" << (it3d->CalcSIT ? -1 : 0);
    AppendParityTrace("step_calcde_entry_override", payload.str());
    return true;
}

static bool ApplyDebugStepDeResultOverride(double& dTmp, int traceStep) {
    if (!HasParityTraceContext()) {
        return false;
    }
    const char* spec = std::getenv("MB3D_DEBUG_STEP_DE_RESULT_OVERRIDE");
    if (!spec || !*spec) {
        return false;
    }

    int px = 0;
    int py = 0;
    int step = -1;
    double overrideDe = 0.0;
    if (std::sscanf(spec, "%d,%d,%d,%lf", &px, &py, &step, &overrideDe) != 4) {
        return false;
    }
    if (px != GetParityTracePixelX() || py != GetParityTracePixelY() || step != traceStep) {
        return false;
    }

    dTmp = overrideDe;
    std::ostringstream payload;
    payload.setf(std::ios::fixed, std::ios::floatfield);
    payload << std::setprecision(12)
            << "step_index=" << traceStep
            << " dTmp=" << dTmp;
    AppendParityTrace("step_de_result_override", payload.str());
    return true;
}

static bool ApplyDebugStepAfterDeStateOverride(PMCTparameter pMCTparas,
                                               TPIteration3Dext it3d,
                                               double& dTmp,
                                               float& RSFmul,
                                               int traceStep) {
    if (!HasParityTraceContext()) {
        return false;
    }
    const char* spec = std::getenv("MB3D_DEBUG_STEP_AFTER_DE_STATE_OVERRIDE");
    if (!spec || !*spec) {
        return false;
    }

    int px = 0;
    int py = 0;
    int step = -1;
    double mzz = 0.0;
    double c1 = 0.0;
    double c2 = 0.0;
    double c3 = 0.0;
    double de = 0.0;
    double rout = 0.0;
    int itResultI = 0;
    double smoothItD = 0.0;
    double otrap = 0.0;
    double rsf = 1.0;
    const int parsed = std::sscanf(spec, "%d,%d,%d,%lf,%lf,%lf,%lf,%lf,%lf,%d,%lf,%lf,%lf",
                                   &px, &py, &step, &mzz, &c1, &c2, &c3, &de, &rout,
                                   &itResultI, &smoothItD, &otrap, &rsf);
    if (parsed < 9) {
        return false;
    }
    if (px != GetParityTracePixelX() || py != GetParityTracePixelY() || step != traceStep) {
        return false;
    }

    pMCTparas->mZZ = mzz;
    it3d->C1 = c1;
    it3d->C2 = c2;
    it3d->C3 = c3;
    dTmp = de;
    it3d->Rout = rout;
    if (parsed >= 10) {
        it3d->ItResultI = itResultI;
    }
    if (parsed >= 11) {
        it3d->SmoothItD = smoothItD;
    }
    if (parsed >= 12) {
        it3d->OTrap = otrap;
    }
    if (parsed >= 13) {
        RSFmul = static_cast<float>(rsf);
    }

    std::ostringstream payload;
    payload.setf(std::ios::fixed, std::ios::floatfield);
    payload << std::setprecision(12)
            << "step_index=" << traceStep
            << " mZZ=" << pMCTparas->mZZ
            << " C1=" << it3d->C1
            << " C2=" << it3d->C2
            << " C3=" << it3d->C3
            << " dTmp=" << dTmp
            << " Rout=" << it3d->Rout
            << " ItResultI=" << it3d->ItResultI
            << " SmoothItD=" << it3d->SmoothItD
            << " OTrap=" << it3d->OTrap
            << " RSFmul=" << RSFmul;
    AppendParityTrace("step_after_de_state_override", payload.str());
    return true;
}

static inline void MaterializeDouble(double& value) {
    volatile double spill = value;
    value = spill;
}

static inline void MaterializeFloat(float& value) {
    volatile float spill = value;
    value = spill;
}

// Env-var lookups are evaluated once per process. Debug flags do not change
// at runtime, and calling std::getenv() from the ray-march hot path (millions
// of invocations per render) dominated profile on built-in scenes.
static inline bool CheckEnvBoolOnce(const char* name) {
    const char* spec = std::getenv(name);
    return spec && *spec;
}

static inline bool DebugCalcDeSpillEnabled() {
    static const bool value = CheckEnvBoolOnce("MB3D_DEBUG_CALCDE_SPILL");
    return value;
}

static inline bool DebugThreadMsDeStopEnabled() {
    static const bool value = CheckEnvBoolOnce("MB3D_DEBUG_THREAD_MSDESTOP");
    return value;
}

static inline bool DebugDynFogNoFloatCastEnabled() {
    static const bool value = CheckEnvBoolOnce("MB3D_DEBUG_DYNFOG_NO_FLOAT_CAST");
    return value;
}

static inline bool DebugDynFogX87StepEnabled() {
    static const bool value = CheckEnvBoolOnce("MB3D_DEBUG_DYNFOG_X87_STEP");
    return value;
}

static inline bool DebugCalcPasRaymarchDynFogEnabled() {
    static const bool value = CheckEnvBoolOnce("MB3D_DEBUG_CALC_PAS_RAYMARCH_DYNFOG");
    return value;
}

static inline bool DebugCalcPasInsideMsDeStopEnabled() {
    static const bool value = CheckEnvBoolOnce("MB3D_DEBUG_CALC_PAS_INSIDE_MSDESTOP");
    return value;
}

static bool ApplyDebugFpBarrier(PMCTparameter pMCTparas,
                                TPIteration3Dext it3d,
                                double* dTmp,
                                double* lastStepWidth,
                                float* RSFmul,
                                int traceStep,
                                const char* stageName) {
    // Read env spec once — identical reasoning to the Debug*Enabled caches above.
    static const char* const spec_cache = std::getenv("MB3D_DEBUG_FP_BARRIER");
    const char* spec = spec_cache;
    if (!spec || !*spec || !stageName) {
        return false;
    }

    int px = 0;
    int py = 0;
    int step = -1;
    char stage[32] = {};
    if (std::sscanf(spec, "%d,%d,%d,%31s", &px, &py, &step, stage) != 4) {
        return false;
    }
    if (px != GetParityTracePixelX() || py != GetParityTracePixelY() || step != traceStep) {
        return false;
    }
    if (std::strcmp(stage, stageName) != 0) {
        return false;
    }

    MaterializeDouble(pMCTparas->mZZ);
    MaterializeFloat(pMCTparas->msDEstop);
    MaterializeDouble(it3d->C1);
    MaterializeDouble(it3d->C2);
    MaterializeDouble(it3d->C3);
    MaterializeDouble(it3d->Rout);
    MaterializeFloat(it3d->SmoothItD);
    MaterializeDouble(it3d->OTrap);
    if (dTmp) {
        MaterializeDouble(*dTmp);
    }
    if (lastStepWidth) {
        MaterializeDouble(*lastStepWidth);
    }
    if (RSFmul) {
        MaterializeFloat(*RSFmul);
    }

    if (HasParityTraceContext()) {
        std::ostringstream payload;
        payload.setf(std::ios::fixed, std::ios::floatfield);
        payload << std::setprecision(12)
                << "step_index=" << traceStep
                << " stage=" << stageName
                << " mZZ=" << pMCTparas->mZZ
                << " C1=" << it3d->C1
                << " C2=" << it3d->C2
                << " C3=" << it3d->C3
                << " Rout=" << it3d->Rout
                << " SmoothItD=" << it3d->SmoothItD
                << " OTrap=" << it3d->OTrap
                << " msDEstop=" << pMCTparas->msDEstop;
        if (dTmp) {
            payload << " dTmp=" << *dTmp;
        }
        if (lastStepWidth) {
            payload << " RLastStepWidth=" << *lastStepWidth;
        }
        if (RSFmul) {
            payload << " RSFmul=" << *RSFmul;
        }
        AppendParityTrace("fp_barrier", payload.str());
    }
    return true;
}

static constexpr double kPascalSeedMul31 = 1.0 / 2147483647.0;

static inline int32_t AdvancePascalSeed(int32_t seed) {
    const uint32_t next = static_cast<uint32_t>(seed) * 214013u + 2531011u;
    return static_cast<int32_t>(next);
}

static inline double PascalFirstStepScale(int32_t seed) {
    const uint32_t useed = static_cast<uint32_t>(seed);
    return static_cast<double>(useed & 0x7FFFFFFFu) * kPascalSeedMul31;
}

static inline double PascalCalcRayFirstStepScale(int32_t seed) {
    const uint32_t useed = static_cast<uint32_t>(seed);
    return static_cast<double>((useed >> 16) & 0x7FFFu) * 0.000030517578125;
}

static inline std::string DoubleBitsHex(double value) {
    std::uint64_t bits = 0;
    std::memcpy(&bits, &value, sizeof(bits));
    std::ostringstream ss;
    ss << "0x" << std::uppercase << std::hex << std::setw(16) << std::setfill('0') << bits;
    return ss.str();
}

static inline void AppendPrimaryTrace(TPRaymarchRec rec, const char* stage, int stepIndex,
                                      const std::string& payload) {
    if (!HasParityTraceContext() && !rec->TraceLines) return;
    AppendParityTrace(stage, payload, stepIndex);
    if (!rec->TraceLines) return;
    std::ostringstream ss;
    ss.setf(std::ios::fixed, std::ios::floatfield);
    ss << "stage=" << stage
       << " x=" << rec->TracePixelX
       << " y=" << rec->TracePixelY;
    (void)stepIndex;
    if (!payload.empty()) {
        ss << " " << payload;
    }
    rec->TraceLines->push_back(ss.str());
}

static inline bool PrimaryTraceActive(TPRaymarchRec rec) {
    return HasParityTraceContext() || rec->TraceLines != nullptr;
}

static inline void LoadPackedVec3D(const void* src, TVec3D& out) {
    std::memcpy(&out, src, sizeof(TVec3D));
}

static inline void LoadIt3DPos(const TPIteration3Dext it, TVec3D& out) {
    std::memcpy(&out, &it->C1, sizeof(TVec3D));
}

static inline void StoreIt3DPos(TPIteration3Dext it, const TVec3D& value) {
    std::memcpy(&it->C1, value, sizeof(TVec3D));
}

static inline void AddVecWeightToIt3DPos(TPIteration3Dext it, const void* packedAdd, double w) {
    TVec3D pos{};
    TVec3D add{};
    LoadIt3DPos(it, pos);
    LoadPackedVec3D(packedAdd, add);
    mAddVecWeight(pos, add, w);
    StoreIt3DPos(it, pos);
}

static inline void CopyAddVecWeightToIt3DPos(TPIteration3Dext it, const TVec3D& base, const void* packedAdd, double w) {
    TVec3D dest{};
    TVec3D add{};
    LoadPackedVec3D(packedAdd, add);
    mCopyAddVecWeight(dest, base, add, w);
    StoreIt3DPos(it, dest);
}

// ============================================================================
// Constants
// ============================================================================

// s011f, s05f, s255f moved to math3d.h
static constexpr double d1em200 = 1e-200;
static constexpr double d1em40  = 1e-40;
static constexpr float  s025    = 0.25f;
static constexpr float  s006    = 0.06f;
static constexpr float  s055    = 0.55f;
static constexpr float  s0001   = 0.001f;
static constexpr float  s3      = 3.0f;
static constexpr float  s7      = 7.0f;
static constexpr float  s005    = 0.05f;
static constexpr double d1p0    = 1.0;
static constexpr double d005    = 0.05;
static constexpr double d7      = 7.0;

// ============================================================================
// RMcalcVLight -- volume light step count encoding
// ============================================================================

int RMcalcVLight(float StepCount) {
    int i = (StepCount > 16383.0f) ? 16383 : static_cast<int>(std::round(StepCount));
    if (i <= 0) return 0;
    
    // Find highest set bit index (equivalent to BSR instruction)
    int ecx = 0;
    uint32_t val = static_cast<uint32_t>(i);
#if defined(_MSC_VER)
    unsigned long index;
    if (_BitScanReverse(&index, val)) ecx = static_cast<int>(index);
#else
    ecx = 31 - __builtin_clz(val);
#endif

    if (ecx <= 6) return i;

    int shift = ecx - 6;
    int eax = i >> shift;
    // Pascal asm (Calc.pas:64 RMcalcVLight): `shl ecx, 7` runs AFTER
    // `sub ecx, 6`, so the shifted value is (highest_bit - 6) << 7, not
    // highest_bit << 7. Decoder ConvertVLight reads bits 7-9 as the shift
    // count; using ecx here (before decrement) made encoded values decode
    // at 1/64 of their true step count and wiped the vol-light contribution.
    int exp = shift << 7;
    return eax | exp;
}

// ============================================================================
// BuildViewVectorDFOV -- perspective camera ray
// ============================================================================

void BuildViewVectorDFOV(double CAFY, double CAFX, TVec3D& result) {
#if defined(_MSC_VER) && defined(_M_X64)
    if (DebugBuildViewX87PassthruEnabled()) {
        double in[2] = {CAFY, CAFX};
        BuildViewVectorDFOV_x87_passthru(result, in);
        return;
    }
#endif
    NumericalBuildViewVectorDFOV(CAFY, CAFX, result);
}

// ============================================================================
// BuildViewVectorDSphereFOV -- spherical panoramic camera ray
// ============================================================================

void BuildViewVectorDSphereFOV(double CAFY, double CAFX, TVec3D& result) {
    NumericalBuildViewVectorDSphereFOV(CAFY, CAFX, result);
}

// ============================================================================
// RMCalculateStartPos -- calculate start position for a pixel
// ============================================================================

void RMCalculateStartPos(PMCTparameter pMCTparas, int ix, int iy) {
    if (pMCTparas->MCTCameraOptic == 2) {
        // Panoramic: start from Ystart directly
        pMCTparas->pIt3Dext->C1 = pMCTparas->Ystart[0];
        pMCTparas->pIt3Dext->C2 = pMCTparas->Ystart[1];
        pMCTparas->pIt3Dext->C3 = pMCTparas->Ystart[2];
    } else {
        // Standard perspective/planar
        double xx = static_cast<double>(ix);
        double yy = static_cast<double>(iy);
        // Match Pascal RMCalculateStartPos ordering: (v0*ix + v1*iy) + Ystart.
        double c1 = pMCTparas->Vgrads[0][0] * xx;
        c1 += pMCTparas->Vgrads[1][0] * yy;
        c1 += pMCTparas->Ystart[0];
        pMCTparas->pIt3Dext->C1 = c1;

        double c2 = pMCTparas->Vgrads[0][1] * xx;
        c2 += pMCTparas->Vgrads[1][1] * yy;
        c2 += pMCTparas->Ystart[1];
        pMCTparas->pIt3Dext->C2 = c2;

        double c3 = pMCTparas->Vgrads[0][2] * xx;
        c3 += pMCTparas->Vgrads[1][2] * yy;
        c3 += pMCTparas->Ystart[2];
        pMCTparas->pIt3Dext->C3 = c3;
    }
}

// ============================================================================
// RMCalculateVgradsFOV -- calculate view direction for a pixel column
// ============================================================================

void RMCalculateVgradsFOV(PMCTparameter pMCTparas, int ix) {
    const double fovxOff = static_cast<double>(pMCTparas->FOVXoff);
    const double fovxMul = static_cast<double>(pMCTparas->FOVXmul);
    pMCTparas->CAFX = (fovxOff - static_cast<double>(ix)) * fovxMul;
    TVec3D vgradsFOV{};

    if (pMCTparas->MCTCameraOptic == 1) {
        // Planar optic
        vgradsFOV[0] = -pMCTparas->CAFX;
        vgradsFOV[1] = pMCTparas->CAFY;
        vgradsFOV[2] = pMCTparas->mctPlOpticZ;
        NormalizeVec3D_inplace(vgradsFOV);
    } else if (pMCTparas->MCTCameraOptic == 2) {
        // Spherical panoramic
        BuildViewVectorDSphereFOV(pMCTparas->CAFY, pMCTparas->CAFX, vgradsFOV);
    } else {
        // Standard perspective
        if (DebugBuildRotateCombinedX87PassthruEnabled()) {
#if defined(_MSC_VER) && defined(_M_X64)
            double in[2] = {pMCTparas->CAFY, pMCTparas->CAFX};
            BuildRotateViewVectorDFOV_x87_passthru(vgradsFOV, in, &pMCTparas->Vgrads[0][0]);
#else
            BuildViewVectorDFOV(pMCTparas->CAFY, pMCTparas->CAFX, vgradsFOV);
            TMatrix3 vgrads{};
            std::memcpy(&vgrads, pMCTparas->Vgrads, sizeof(vgrads));
            RotateVectorReverse(vgradsFOV, vgrads);
#endif
            std::memcpy(pMCTparas->mVgradsFOV, vgradsFOV, sizeof(vgradsFOV));
            return;
        }
        NumericalBuildRotateViewVectorDFOV(pMCTparas->CAFY, pMCTparas->CAFX,
                                           &pMCTparas->Vgrads[0][0], vgradsFOV);
        std::memcpy(pMCTparas->mVgradsFOV, vgradsFOV, sizeof(vgradsFOV));
        return;
    }
#if defined(_MSC_VER) && defined(_M_X64)
    if (DebugRotateX87ExtCWEnabled()) {
        RotateVectorReverse_x87_extcw(vgradsFOV, &pMCTparas->Vgrads[0][0]);
    } else {
        NumericalRotateVectorReverse(vgradsFOV, &pMCTparas->Vgrads[0][0]);
    }
#else
    NumericalRotateVectorReverse(vgradsFOV, &pMCTparas->Vgrads[0][0]);
#endif
    std::memcpy(pMCTparas->mVgradsFOV, vgradsFOV, sizeof(vgradsFOV));
}

static void TraceDEGradient(double radius, double dx2, double dy2, double dz2,
                            double scale, double offset, double raw) {
    if (!ParityActive()) return;
    ParityBeginCurrentStage("calcde_gradient");
    ParityAddDouble17("bufRout", radius);
    ParityAddDouble17("dx2", dx2);
    ParityAddDouble17("dy2", dy2);
    ParityAddDouble17("dz2", dz2);
    ParityAddDouble17("sqrt_sum", std::sqrt(dz2 + dy2 + dx2));
    ParityAddDouble17("scale", scale);
    ParityAddDouble17("offset", offset);
    ParityAddDouble17("raw_de", raw);
    ParityEndStage();
}

// ============================================================================
// CalcDEanalytic -- analytic distance estimation
// ============================================================================

double CalcDEanalytic(TPIteration3Dext It3Dex, void* mctp) {
    RenderWork();
    PMCTparameter mct = static_cast<PMCTparameter>(mctp);
    if (HasParityTraceContext()) {
        int calcSitPas = It3Dex->CalcSIT ? -1 : 0;
        std::ostringstream payload;
        payload.setf(std::ios::fixed, std::ios::floatfield);
        payload << std::setprecision(12)
                << "C1=" << It3Dex->C1
                << " C2=" << It3Dex->C2
                << " C3=" << It3Dex->C3
                << " RStop=" << It3Dex->RStop
                << " maxIt=" << It3Dex->maxIt
                << " CalcSIT=" << calcSitPas
                << " bInsideRendering=" << (mct->bInsideRendering ? -1 : 0)
                << " dDEscale=" << mct->dDEscale
                << " mctDEoffset=" << mct->mctDEoffset
                << " mctDEoffset006=" << mct->mctDEoffset006;
        AppendParityTrace("calcde_enter", payload.str());
    }
    if (ParityActive()) {
        ParityBeginCurrentStage("calcde_enter");
        ParityAddDouble12("C1", It3Dex->C1);
        ParityAddDouble12("C2", It3Dex->C2);
        ParityAddDouble12("C3", It3Dex->C3);
        ParityAddDouble12("RStop", It3Dex->RStop);
        ParityAddInt("maxIt", It3Dex->maxIt);
        ParityAddInt("DEoption", It3Dex->DEoption);
        ParityAddBoolNeg1("bInsideRendering", mct->bInsideRendering != 0);
        ParityAddDouble12("dDEscale", mct->dDEscale);
        ParityAddDouble12("mctDEoffset", mct->mctDEoffset);
        ParityEndStage();
    }

    if (It3Dex->DEoption == 20) { // dIFS
        It3Dex->Rold = mct->msDEstop * mct->sStepWm103;
        It3Dex->RStopD = It3Dex->Rold;
        It3Dex->bIsInsideRender = mct->bInsideRendering;
    }

    double result = mct->mMandFunctionDE(reinterpret_cast<TPIteration3D>(&It3Dex->C1)) * mct->dDEscale;
    if (ParityActive()) {
        ParityBeginCurrentStage("calcde_after_orbit");
        ParityAddInt("ItResultI", It3Dex->ItResultI);
        ParityAddDouble12("Rout", It3Dex->Rout);
        ParityAddDouble12("OTrap", It3Dex->OTrap);
        ParityAddDouble12("SmoothItD", It3Dex->CalcSIT ? It3Dex->SmoothItD : std::numeric_limits<double>::quiet_NaN());
        ParityAddBoolNeg1("bCalcInside", mct->bCalcInside != 0);
        ParityAddBoolNeg1("CalcSIT", It3Dex->CalcSIT != 0);
        ParityEndStage();
    }
    mct->MaxItsResult = It3Dex->maxIt;

    if (It3Dex->DEoption == 20) { // dIFS
        mct->MaxItsResult++;
    } else {
        // Standard DE clamping: Min(result, msDEstop * 0.25)
        double minDE = mct->msDEstop * 0.25;
        if (result < minDE) result = minDE;
    }

    mct->DEoptionResult = It3Dex->DEoption;

    if (mct->bCalcInside) {
        if (mct->DEoptionResult == 20) {
            // Inside dIFS: 2*msDEstop - result
            result = static_cast<double>(mct->msDEstop) * 2.0 - result;
        } else {
            // Inside Standard: 4*msDEstop - 3*result
            result = static_cast<double>(mct->msDEstop) * 4.0 - result * 3.0;
            if (result >= mct->msDEstop) {
                if (It3Dex->ItResultI > 0) It3Dex->ItResultI--;
            }
        }
    }

    if (HasParityTraceContext()) {
        std::ostringstream payload;
        payload.setf(std::ios::fixed, std::ios::floatfield);
        const double traceSqrtSum = std::numeric_limits<double>::quiet_NaN();
        payload << std::setprecision(12)
                << "bufRout=" << It3Dex->Rout
                << " bufRout_bits=" << DoubleBitsHex(It3Dex->Rout)
                << " sqrt_sum=" << traceSqrtSum
                << " sqrt_sum_bits=" << DoubleBitsHex(traceSqrtSum)
                << " raw_de=" << result
                << " raw_de_bits=" << DoubleBitsHex(result)
                << " final_de=" << result
                << " final_de_bits=" << DoubleBitsHex(result)
                << " ItResultI=" << It3Dex->ItResultI
                << " DEoptionResult=" << mct->DEoptionResult;
        AppendParityTrace("calcde_result", payload.str());
    }
    if (ParityActive()) {
        ParityBeginCurrentStage("calcde_result");
        ParityAddDouble12("bufRout", It3Dex->Rout);


        ParityAddDouble12("final_de", result);
        ParityAddInt("ItResultI", It3Dex->ItResultI);
        ParityAddInt("DEoptionResult", mct->DEoptionResult);
        ParityEndStage();
    }

    return result;
}

// ============================================================================
// CalcDEnoADE -- fallback numerical DE (no analytic DE)
// ============================================================================

#if defined(_MSC_VER)
#pragma float_control(precise, on, push)
#pragma fp_contract(off)
#endif
double CalcDEnoADE(TPIteration3Dext It3Dex, void* mctp) {
    RenderWork();
    PMCTparameter mct = static_cast<PMCTparameter>(mctp);
    if (HasParityTraceContext()) {
        int calcSitPas = It3Dex->CalcSIT ? -1 : 0;
        std::ostringstream payload;
        payload.setf(std::ios::fixed, std::ios::floatfield);
        payload << std::setprecision(12)
                << "C1=" << It3Dex->C1
                << " C2=" << It3Dex->C2
                << " C3=" << It3Dex->C3
                << " RStop=" << It3Dex->RStop
                << " maxIt=" << It3Dex->maxIt
                << " CalcSIT=" << calcSitPas
                << " bInsideRendering=" << (mct->bInsideRendering ? -1 : 0)
                << " dDEscale=" << mct->dDEscale
                << " mctDEoffset=" << mct->mctDEoffset
                << " mctDEoffset006=" << mct->mctDEoffset006;
        AppendParityTrace("calcde_enter", payload.str());
    }
    if (ParityActive()) {
        ParityBeginCurrentStage("calcde_enter");
        ParityAddDouble12("C1", It3Dex->C1);
        ParityAddDouble12("C2", It3Dex->C2);
        ParityAddDouble12("C3", It3Dex->C3);
        ParityAddDouble12("RStop", It3Dex->RStop);
        ParityAddInt("maxIt", It3Dex->maxIt);
        ParityAddInt("DEoption", It3Dex->DEoption);
        ParityAddBoolNeg1("bInsideRendering", mct->bInsideRendering != 0);
        ParityAddDouble12("dDEscale", mct->dDEscale);
        ParityAddDouble12("mctDEoffset", mct->mctDEoffset);
        ParityEndStage();
    }

    if (HasParityTraceContext()) {
        std::ostringstream payload;
        payload.setf(std::ios::fixed, std::ios::floatfield);
        payload << std::setprecision(12)
                << "ItResultI=" << It3Dex->ItResultI
                << " maxIt=" << It3Dex->maxIt
                << " CalcSIT=" << (It3Dex->CalcSIT ? -1 : 0)
                << " bInsideRendering=" << (mct->bInsideRendering ? -1 : 0)
                << " bCalcInside=" << (mct->bCalcInside ? -1 : 0)
                << " Rout=" << It3Dex->Rout
                << " OTrap=" << It3Dex->OTrap
                << " SmoothItD=" << It3Dex->SmoothItD;
        AppendParityTrace("calcde_before_orbit", payload.str());
    }
    mct->mMandFunction(reinterpret_cast<TPIteration3D>(&It3Dex->C1));
    if (ParityActive()) {
        ParityBeginCurrentStage("calcde_after_orbit");
        ParityAddInt("ItResultI", It3Dex->ItResultI);
        ParityAddDouble12("Rout", It3Dex->Rout);
        ParityAddDouble12("OTrap", It3Dex->OTrap);
        ParityAddDouble12("SmoothItD", It3Dex->CalcSIT ? It3Dex->SmoothItD : std::numeric_limits<double>::quiet_NaN());
        ParityAddBoolNeg1("bCalcInside", mct->bCalcInside != 0);
        ParityAddBoolNeg1("CalcSIT", It3Dex->CalcSIT != 0);
        ParityEndStage();
    }
    if (DebugCalcDeSpillEnabled()) {
        MaterializeDouble(It3Dex->C1);
        MaterializeDouble(It3Dex->C2);
        MaterializeDouble(It3Dex->C3);
        MaterializeDouble(It3Dex->Rout);
        MaterializeDouble(It3Dex->OTrap);
        MaterializeFloat(It3Dex->SmoothItD);
    }
    if (HasParityTraceContext()) {
        std::ostringstream payload;
        payload.setf(std::ios::fixed, std::ios::floatfield);
        payload << std::setprecision(12)
                << "ItResultI=" << It3Dex->ItResultI
                << " maxIt=" << It3Dex->maxIt
                << " CalcSIT=" << (It3Dex->CalcSIT ? -1 : 0)
                << " bInsideRendering=" << (mct->bInsideRendering ? -1 : 0)
                << " bCalcInside=" << (mct->bCalcInside ? -1 : 0)
                << " Rout=" << It3Dex->Rout
                << " OTrap=" << It3Dex->OTrap
                << " SmoothItD=" << It3Dex->SmoothItD;
        AppendParityTrace("calcde_after_orbit", payload.str());
    }

    double result;
    double traceSqrtSum = 0.0;
    double traceRawDE = 0.0;
    if (mct->bInsideRendering && It3Dex->ItResultI == It3Dex->maxIt) {
        result = 0.0;
        traceRawDE = 0.0;
    } else if (It3Dex->Rout < d1em200) {
        result = 0.0;
        traceRawDE = 0.0;
    } else {
        // Save state
        int bufCalcSIT = *reinterpret_cast<int32_t*>(&It3Dex->CalcSIT);
        int bufMaxIt = It3Dex->maxIt;
        double bufRout = It3Dex->Rout;

        It3Dex->CalcSIT = 0;
        It3Dex->maxIt = It3Dex->ItResultI;
        It3Dex->RStop = static_cast<float>(mct->Rstop3D);

        // 3-point numerical gradient
        double bufD = It3Dex->C1;
        It3Dex->C1 = It3Dex->C1 + mct->mctDEoffset;
        mct->mMandFunction(reinterpret_cast<TPIteration3D>(&It3Dex->C1));
        double dt = DeltaSqPasLike(bufRout, It3Dex->Rout);
        if (DebugCalcDeSpillEnabled()) {
            MaterializeDouble(It3Dex->C1);
            MaterializeDouble(It3Dex->C2);
            MaterializeDouble(It3Dex->C3);
            MaterializeDouble(It3Dex->Rout);
            MaterializeDouble(dt);
        }
        if (HasParityTraceContext()) {
            std::ostringstream payload;
            payload.setf(std::ios::fixed, std::ios::floatfield);
            payload << std::setprecision(17)
                    << "C1=" << It3Dex->C1
                    << " C2=" << It3Dex->C2
                    << " C3=" << It3Dex->C3
                    << " Rout=" << It3Dex->Rout
                    << " delta=" << (bufRout - It3Dex->Rout)
                    << " dt=" << dt;
            AppendParityTrace("calcde_sample_x", payload.str());
        }
        It3Dex->C1 = bufD;

        bufD = It3Dex->C2;
        It3Dex->C2 = It3Dex->C2 + mct->mctDEoffset;
        mct->mMandFunction(reinterpret_cast<TPIteration3D>(&It3Dex->C1));
        double wt = DeltaSqPasLike(bufRout, It3Dex->Rout);
        if (DebugCalcDeSpillEnabled()) {
            MaterializeDouble(It3Dex->C1);
            MaterializeDouble(It3Dex->C2);
            MaterializeDouble(It3Dex->C3);
            MaterializeDouble(It3Dex->Rout);
            MaterializeDouble(wt);
        }
        if (HasParityTraceContext()) {
            std::ostringstream payload;
            payload.setf(std::ios::fixed, std::ios::floatfield);
            payload << std::setprecision(17)
                    << "C1=" << It3Dex->C1
                    << " C2=" << It3Dex->C2
                    << " C3=" << It3Dex->C3
                    << " Rout=" << It3Dex->Rout
                    << " delta=" << (bufRout - It3Dex->Rout)
                    << " wt=" << wt;
            AppendParityTrace("calcde_sample_y", payload.str());
        }
        It3Dex->C2 = bufD;

        bufD = It3Dex->C3;
        It3Dex->C3 = It3Dex->C3 + mct->mctDEoffset;
        mct->mMandFunction(reinterpret_cast<TPIteration3D>(&It3Dex->C1));
        double Rst = DeltaSqPasLike(bufRout, It3Dex->Rout);
        if (DebugCalcDeSpillEnabled()) {
            MaterializeDouble(It3Dex->C1);
            MaterializeDouble(It3Dex->C2);
            MaterializeDouble(It3Dex->C3);
            MaterializeDouble(It3Dex->Rout);
            MaterializeDouble(Rst);
        }
        if (HasParityTraceContext()) {
            std::ostringstream payload;
            payload.setf(std::ios::fixed, std::ios::floatfield);
            payload << std::setprecision(17)
                    << "C1=" << It3Dex->C1
                    << " C2=" << It3Dex->C2
                    << " C3=" << It3Dex->C3
                    << " Rout=" << It3Dex->Rout
                    << " delta=" << (bufRout - It3Dex->Rout)
                    << " Rst=" << Rst;
            AppendParityTrace("calcde_sample_z", payload.str());
        }
        It3Dex->C3 = bufD;

        double sqrtSum = 0.0;
        double rawDE = CalcRawDePasLike(bufRout, dt, wt, Rst,
                                        mct->dDEscale, mct->mctDEoffset006, &sqrtSum);
        if (DebugCalcDeSpillEnabled()) {
            MaterializeDouble(dt);
            MaterializeDouble(wt);
            MaterializeDouble(Rst);
            MaterializeDouble(sqrtSum);
            MaterializeDouble(rawDE);
        }
        TraceDEGradient(bufRout, dt, wt, Rst, mct->dDEscale, mct->mctDEoffset006, rawDE);
        result = rawDE;
        traceSqrtSum = sqrtSum;
        traceRawDE = rawDE;

        It3Dex->Rout = bufRout;
        It3Dex->ItResultI = It3Dex->maxIt;
        It3Dex->maxIt = bufMaxIt;
        *reinterpret_cast<int32_t*>(&It3Dex->CalcSIT) = bufCalcSIT;
        It3Dex->RStop = mct->dRStop;
    }

    mct->MaxItsResult = It3Dex->maxIt;
    double minDE = mct->msDEstop * 0.25;
    if (result < minDE) result = minDE;

    mct->DEoptionResult = It3Dex->DEoption;

    if (mct->bCalcInside) {
        result = mct->msDEstop * 4.0 - result * 3.0;
        if (result >= mct->msDEstop) It3Dex->ItResultI--;
    }

    if (HasParityTraceContext()) {
        std::ostringstream payload;
        payload.setf(std::ios::fixed, std::ios::floatfield);
        payload << std::setprecision(17)
                << "bufRout=" << It3Dex->Rout
                << " bufRout_bits=" << DoubleBitsHex(It3Dex->Rout)
                << " sqrt_sum=" << traceSqrtSum
                << " sqrt_sum_bits=" << DoubleBitsHex(traceSqrtSum)
                << " raw_de=" << traceRawDE
                << " raw_de_bits=" << DoubleBitsHex(traceRawDE)
                << " final_de=" << result
                << " final_de_bits=" << DoubleBitsHex(result)
                << " ItResultI=" << It3Dex->ItResultI
                << " DEoptionResult=" << mct->DEoptionResult;
        AppendParityTrace("calcde_result", payload.str());
    }
    if (ParityActive()) {
        ParityBeginCurrentStage("calcde_result");
        ParityAddDouble12("bufRout", It3Dex->Rout);


        ParityAddDouble12("final_de", result);
        ParityAddInt("ItResultI", It3Dex->ItResultI);
        ParityAddInt("DEoptionResult", mct->DEoptionResult);
        ParityEndStage();
    }

    return result;
}
#if defined(_MSC_VER)
#pragma float_control(pop)
#endif

// ============================================================================
// PrepareF1DEcomb -- prepare for second formula group in DEcomb mode
// Ported from Calc.pas lines 393-414
//
// Saves first formula group state into DEcombRec, then loads second group.
// ============================================================================

static void PrepareF1DEcomb(TBufDEcomb* DEcombRec, PMCTparameter mct) {
    // Copy repeat/end settings for second hybrid part
    *reinterpret_cast<uint32_t*>(&mct->pIt3Dext->iRepeatFrom) =
        *reinterpret_cast<uint32_t*>(&mct->RepeatFrom2);
    mct->pIt3Dext->EndTo = mct->iEnd2;

    // Save first group function pointers
    DEcombRec->bufMCTMandFunction = mct->mMandFunction;
    DEcombRec->bufMCTMandFunctionDE = mct->mMandFunctionDE;

    // Load second group function pointers
    mct->mMandFunction = mct->mMandFunction2;
    mct->mMandFunctionDE = mct->mMandFunctionDE2;

    // Save iteration results from first group
    DEcombRec->bufIt3DItResultI = mct->pIt3Dext->ItResultI;
    DEcombRec->bufIt3DOtrap = mct->pIt3Dext->OTrap;
    DEcombRec->bufIt3DSmoothItD = mct->pIt3Dext->SmoothItD;

    // Save and swap DE scale
    DEcombRec->bufMCTdDEscale = mct->dDEscale;
    DEcombRec->bufMCTmaxItsResult = mct->MaxItsResult;
    mct->dDEscale = mct->dDEscale2;

    // Swap IsCustomDE
    mct->IsCustomDE = mct->IsCustomDE2;
    mct->bCalcInside = 0;  // disable inside for second part

    // Set max iterations for second part
    mct->pIt3Dext->maxIt = mct->iMaxitF2;
    mct->pIt3Dext->DEoption = mct->DEoption2;
}

// ============================================================================
// RestoreF1DEcomb -- restore first formula group after DEcomb
// Ported from Calc.pas lines 377-391
// ============================================================================

static void RestoreF1DEcomb(TBufDEcomb* DEcombRec, PMCTparameter mct) {
    mct->pIt3Dext->maxIt = mct->iMaxIt;
    mct->pIt3Dext->DEoption = mct->DEoption;

    *reinterpret_cast<uint32_t*>(&mct->pIt3Dext->iRepeatFrom) =
        *reinterpret_cast<uint32_t*>(&mct->RepeatFrom1);
    mct->pIt3Dext->EndTo = mct->wEndTo;

    mct->bCalcInside = mct->bInsideRendering;
    mct->IsCustomDE = mct->IsCustomDE1;
    mct->dDEscale = DEcombRec->bufMCTdDEscale;
    mct->mMandFunction = DEcombRec->bufMCTMandFunction;
    mct->mMandFunctionDE = DEcombRec->bufMCTMandFunctionDE;
}

// ============================================================================
// CalcDEfull -- full DE calculation with 4-point gradient or analytic
// Ported from Calc.pas lines 528-698
//
// Handles all DE modes:
//   FormulaType 0: single formula group (standard)
//   FormulaType 1: DEcomb min (union)
//   FormulaType 2: DEcomb max (intersection)
//   FormulaType 3: DEcomb max-inverse (one formula inverted)
//   FormulaType 4: DEcomb smooth linear blend
//   FormulaType 5: DEcomb smooth quadratic blend
//   FormulaType 6+: DEcomb mix (first common fractal, then dIFS)
// ============================================================================

double CalcDEfull(TPIteration3Dext It3Dex, void* mctp) {
    RenderWork();
    PMCTparameter mct = static_cast<PMCTparameter>(mctp);
    if (ParityActive()) {
        ParityBeginCurrentStage("calcde_enter");
        ParityAddDouble12("C1", It3Dex->C1);
        ParityAddDouble12("C2", It3Dex->C2);
        ParityAddDouble12("C3", It3Dex->C3);
        ParityAddDouble12("RStop", It3Dex->RStop);
        ParityAddInt("maxIt", It3Dex->maxIt);
        ParityAddInt("DEoption", It3Dex->DEoption);
        ParityAddBoolNeg1("bInsideRendering", mct->bInsideRendering != 0);
        ParityAddDouble12("dDEscale", mct->dDEscale);
        ParityAddDouble12("mctDEoffset", mct->mctDEoffset);
        ParityEndStage();
    }
    double DS025 = mct->msDEstop * 0.25;
    double result;
    double wt, dt, Rst;

    if (mct->IsCustomDE) {
        // Analytic DE path
        if (It3Dex->DEoption == 20) { // dIFS
            It3Dex->Rold = mct->msDEstop * mct->sStepWm103;
            It3Dex->bIsInsideRender = mct->bInsideRendering;
            It3Dex->RStopD = mct->msDEstop * mct->sStepWm103;
        }
        result = mct->mMandFunctionDE(reinterpret_cast<TPIteration3D>(&It3Dex->C1)) * mct->dDEscale;
        wt = std::fabs(It3Dex->w);
        dt = std::fabs(It3Dex->Deriv1);
    } else {
        // 3D gradient (numerical DE)
        if (HasParityTraceContext()) {
            std::ostringstream payload;
            payload.setf(std::ios::fixed, std::ios::floatfield);
            payload << std::setprecision(17)
                    << "ItResultI=" << It3Dex->ItResultI
                    << " maxIt=" << It3Dex->maxIt
                    << " CalcSIT=" << (It3Dex->CalcSIT ? -1 : 0)
                    << " bInsideRendering=" << (mct->bInsideRendering ? -1 : 0)
                    << " bCalcInside=" << (mct->bCalcInside ? -1 : 0)
                    << " Rout=" << It3Dex->Rout
                    << " OTrap=" << It3Dex->OTrap
                    << " SmoothItD=" << It3Dex->SmoothItD;
            AppendParityTrace("calcde_before_orbit", payload.str());
        }
        mct->mMandFunction(reinterpret_cast<TPIteration3D>(&It3Dex->C1));
        if (HasParityTraceContext()) {
            std::ostringstream payload;
            payload.setf(std::ios::fixed, std::ios::floatfield);
            payload << std::setprecision(12)
                    << "ItResultI=" << It3Dex->ItResultI
                    << " maxIt=" << It3Dex->maxIt
                    << " CalcSIT=" << (It3Dex->CalcSIT ? -1 : 0)
                    << " bInsideRendering=" << (mct->bInsideRendering ? -1 : 0)
                    << " bCalcInside=" << (mct->bCalcInside ? -1 : 0)
                    << " Rout=" << It3Dex->Rout
                    << " OTrap=" << It3Dex->OTrap
                    << " SmoothItD=" << It3Dex->SmoothItD;
            AppendParityTrace("calcde_after_orbit", payload.str());
        }
        if (ParityActive()) {
            ParityBeginCurrentStage("calcde_after_orbit");
            ParityAddInt("ItResultI", It3Dex->ItResultI);
            ParityAddDouble12("Rout", It3Dex->Rout);
            ParityAddDouble12("OTrap", It3Dex->OTrap);
            ParityAddDouble12("SmoothItD", It3Dex->CalcSIT ? It3Dex->SmoothItD : std::numeric_limits<double>::quiet_NaN());
            ParityAddBoolNeg1("bCalcInside", mct->bCalcInside != 0);
            ParityAddBoolNeg1("CalcSIT", It3Dex->CalcSIT != 0);
            ParityEndStage();
        }
        if (mct->bInsideRendering && It3Dex->ItResultI == It3Dex->maxIt) {
            result = 0.0;
            goto afterDE;
        }
        if (It3Dex->Rout < d1em200) {
            result = 0.0;
            goto afterDE;
        }

        {
            LongBool bufCalcSIT = *reinterpret_cast<LongBool*>(&It3Dex->CalcSIT);
            int bufMaxIt = It3Dex->maxIt;
            double bufRout = It3Dex->Rout;

            It3Dex->CalcSIT = 0;
            It3Dex->maxIt = It3Dex->ItResultI;
            It3Dex->RStop = static_cast<float>(mct->Rstop3D);

            double bufC1 = It3Dex->C1;
            It3Dex->C1 = It3Dex->C1 + mct->mctDEoffset;
            mct->mMandFunction(reinterpret_cast<TPIteration3D>(&It3Dex->C1));
            dt = (bufRout - It3Dex->Rout);
            dt = dt * dt;
            It3Dex->C1 = bufC1;

            double bufC2 = It3Dex->C2;
            It3Dex->C2 = It3Dex->C2 + mct->mctDEoffset;
            mct->mMandFunction(reinterpret_cast<TPIteration3D>(&It3Dex->C1));
            wt = (bufRout - It3Dex->Rout);
            wt = wt * wt;
            It3Dex->C2 = bufC2;

            double bufC3 = It3Dex->C3;
            It3Dex->C3 = It3Dex->C3 + mct->mctDEoffset;
            mct->mMandFunction(reinterpret_cast<TPIteration3D>(&It3Dex->C1));
            Rst = (bufRout - It3Dex->Rout);
            Rst = Rst * Rst;
            It3Dex->C3 = bufC3;

            result = bufRout * std::log(bufRout) * mct->dDEscale /
                     (std::sqrt(Rst + wt + dt) + mct->mctDEoffset006);
            TraceDEGradient(bufRout, dt, wt, Rst, mct->dDEscale, mct->mctDEoffset006, result);

            It3Dex->Rout = bufRout;
            It3Dex->ItResultI = It3Dex->maxIt;
            It3Dex->maxIt = bufMaxIt;
            *reinterpret_cast<LongBool*>(&It3Dex->CalcSIT) = bufCalcSIT;
            It3Dex->RStop = mct->dRStop;
        }
    }

afterDE:
    mct->MaxItsResult = mct->iMaxIt;
    if (It3Dex->DEoption == 20) {
        mct->MaxItsResult++;
    } else {
        if (result < DS025) result = DS025;
    }
    mct->DEoptionResult = It3Dex->DEoption;

    // DEcomb handling (FormulaType > 0)
    // Ported from Calc.pas lines 612-688
    if (mct->FormulaType > 0) {
        TBufDEcomb DEcombRec;
        PrepareF1DEcomb(&DEcombRec, mct);

        // FormulaType 3 inverts the inside flag for second formula
        if (mct->FormulaType == 3) {
            mct->bCalcInside = !mct->bCalcInside;
        }

        // Calculate DE for second formula group
        double Rst2;
        if (mct->IsCustomDE) {
            Rst2 = CalcDEanalytic(It3Dex, mctp);
        } else {
            Rst2 = CalcDEnoADE(It3Dex, mctp);
        }

        switch (mct->FormulaType) {
            case 1:
                // Min DE combination (union)
                if (Rst2 < result) {
                    result = Rst2;
                } else {
                    // Restore first group's iteration results
                    It3Dex->ItResultI = DEcombRec.bufIt3DItResultI;
                    It3Dex->SmoothItD = DEcombRec.bufIt3DSmoothItD;
                    It3Dex->OTrap = DEcombRec.bufIt3DOtrap;
                    mct->MaxItsResult = DEcombRec.bufMCTmaxItsResult;
                    mct->DEoptionResult = mct->DEoption;
                }
                break;

            case 2:
            case 3:
                // Max DE combination (intersection) / MaxInv
                if (Rst2 > result) {
                    result = Rst2;
                } else {
                    It3Dex->ItResultI = DEcombRec.bufIt3DItResultI;
                    It3Dex->SmoothItD = DEcombRec.bufIt3DSmoothItD;
                    It3Dex->OTrap = DEcombRec.bufIt3DOtrap;
                    mct->MaxItsResult = DEcombRec.bufMCTmaxItsResult;
                    mct->DEoptionResult = mct->DEoption;
                }
                break;

            case 4:
            case 5: {
                // Smooth blend combinations
                if (Rst2 >= result) {
                    It3Dex->ItResultI = DEcombRec.bufIt3DItResultI;
                    mct->MaxItsResult = DEcombRec.bufMCTmaxItsResult;
                    mct->DEoptionResult = mct->DEoption;
                }

                if (mct->FormulaType == 4) {
                    // Linear smooth blend
                    double cR = Clamp0D(mct->sDEcombSmooth - result);
                    double cS = Clamp0D(mct->sDEcombSmooth - Rst2);
                    result = std::min(Rst2 - cR, result - cS);
                } else {
                    // Quadratic smooth blend
                    double RD1 = Clamp0D(mct->sDEcombSmooth - result - mct->msDEstop);
                    double ddt = Clamp0D(mct->sDEcombSmooth - Rst2 - mct->msDEstop);
                    result = std::min(
                        Rst2 - RD1 * (mct->sDEcombSmooth - ddt) / mct->sDEcombSmooth,
                        result - ddt * (mct->sDEcombSmooth - RD1) / mct->sDEcombSmooth);
                }

                // Weighted average for smooth iterations and OTrap
                double absRst2 = std::fabs(Rst2);
                double absResult = std::fabs(result);
                double invSum = 1.0 / (absResult + absRst2 + 1e-10);
                It3Dex->SmoothItD = static_cast<float>(
                    (It3Dex->SmoothItD * absResult + DEcombRec.bufIt3DSmoothItD * absRst2) * invSum);
                It3Dex->OTrap =
                    (It3Dex->OTrap * absResult + DEcombRec.bufIt3DOtrap * absRst2) * invSum;
                break;
            }

            default: {
                // FormulaType 6+: Mix mode (first common fractal, then dIFS)
                // Color handling based on DEmixCol option
                switch (mct->DEmixCol) {
                    case 0:
                        It3Dex->SmoothItD = It3Dex->SmoothItD + DEcombRec.bufIt3DSmoothItD;
                        It3Dex->OTrap = (It3Dex->OTrap + DEcombRec.bufIt3DOtrap) * 0.5;
                        break;
                    case 1:
                        It3Dex->SmoothItD = DEcombRec.bufIt3DSmoothItD;
                        It3Dex->OTrap = DEcombRec.bufIt3DOtrap;
                        break;
                    // case 2: color of second formula, already set
                }

                // DE calculation based on DEoption of first formula
                switch (mct->DEoption) {
                    case 5:
                    case 6:
                        result = Rst2 / dt;
                        break;
                    case 2:
                    case 11:
                        result = Rst2 / wt;
                        break;
                    default:
                        result = Rst2 * std::pow(std::fabs(mct->FmixPow),
                                                  -1.0 - DEcombRec.bufIt3DItResultI);
                        break;
                }
                break;
            }
        }

        RestoreF1DEcomb(&DEcombRec, mct);
    }

    // Inside rendering
    if (mct->bCalcInside) {
        if (mct->DEoptionResult == 20) {
            result = mct->msDEstop * 2.0 - result;
        } else {
            result = mct->msDEstop * 4.0 - result * 3.0;
            if (result >= mct->msDEstop) It3Dex->ItResultI--;
        }
    }
    if (ParityActive()) {
        ParityBeginCurrentStage("calcde_result");
        ParityAddDouble12("bufRout", It3Dex->Rout);


        ParityAddDouble12("final_de", result);
        ParityAddInt("ItResultI", It3Dex->ItResultI);
        ParityAddInt("DEoptionResult", mct->DEoptionResult);
        ParityEndStage();
    }

    return result;
}

// ============================================================================
// RMCalcRoughness -- surface roughness from normal deviation
// ============================================================================

void RMCalcRoughness(TPVec3D N, float& sRough, double* dt2, double* dsG) {
    double nLenSq = (*N)[0]*(*N)[0] + (*N)[1]*(*N)[1] + (*N)[2]*(*N)[2] + d1em40;
    double val = (*dsG) * d7 * (*dt2) * (*dt2) + d1em40;
    val = val / nLenSq;
    if (val < 0) val = 0;
    val = std::sqrt(val) - d005;
    if (val < 0) val = 0;
    if (val > d1p0) val = d1p0;
    sRough = static_cast<float>(val);
}

// ============================================================================
// MakeWNormalsFromDVec -- convert double-precision normal to 16-bit packed
// ============================================================================



// ============================================================================
// CreateXYVecsFromNormals -- make orthogonal vectors from normal
// ============================================================================

static void CreateXYVecsFromNormals(TPVec3D N, TPVec3D Vx, TPVec3D Vy) {
    double d = (*N)[1] * (*N)[1] + (*N)[0] * (*N)[0];
    if (d < 1e-50) {
        (*Vx)[0] = 1.0;
        (*Vx)[1] = 0.0;
        (*Vx)[2] = 0.0;
        (*Vy)[0] = 0.0;
        (*Vy)[1] = 1.0;
        (*Vy)[2] = 0.0;
        return;
    }

    const double denom = std::sqrt(d + (*N)[2] * (*N)[2] + 1e-100);
    double sinA = 0.0;
    double cosA = 1.0;
    SinCosD(ArcCosSafe(-(*N)[2] / denom) * 0.5, sinA, cosA);
    sinA /= std::sqrt(d);

    const double qx = -(*N)[1] * sinA;
    const double qy = (*N)[0] * sinA;

    (*Vx)[0] = 1.0 - 2.0 * qy * qy;
    (*Vx)[1] = 2.0 * qx * qy;
    (*Vx)[2] = 2.0 * qy * cosA;

    (*Vy)[0] = (*Vx)[1];
    (*Vy)[1] = 1.0 - 2.0 * qx * qx;
    (*Vy)[2] = -2.0 * qx * cosA;
}

// ============================================================================
// RMCalculateNormals -- calculate surface normals using DE gradients
// ============================================================================

void RMCalculateNormals(PMCTparameter pMCTparas, float& NN) {
    auto It3Dex = pMCTparas->pIt3Dext;

    It3Dex->CalcSIT = 1;
    double Noffset = MinCS(1.0f, pMCTparas->DEstop) * (1.0 + pMCTparas->mZZ * pMCTparas->mctDEstopFactor) * 0.15;

    TVec3D CT1;
    CT1[0] = It3Dex->C1; CT1[1] = It3Dex->C2; CT1[2] = It3Dex->C3;

    double dNN = pMCTparas->CalcDE(It3Dex, pMCTparas);
    NN = It3Dex->SmoothItD;
    It3Dex->CalcSIT = 0;

    TVec3D N;
    if (pMCTparas->iSmNormals == 8) {
        // High quality 5x5x5 normals
        ClearDVec(N);
        double StepSNorm = Noffset * 1.3333;
        for (int itmp = -2; itmp <= 2; itmp++) {
            for (int iy = -2; iy <= 2; iy++) {
                for (int ix = -2; ix <= 2; ix++) {
                    if ((itmp | iy | ix) != 0) {
                        CopyAddVecWeightToIt3DPos(It3Dex, CT1, &pMCTparas->Vgrads[2], itmp * StepSNorm);
                        AddVecWeightToIt3DPos(It3Dex, &pMCTparas->Vgrads[1], iy * StepSNorm);
                        AddVecWeightToIt3DPos(It3Dex, &pMCTparas->Vgrads[0], ix * StepSNorm);
                        double dT2 = pMCTparas->CalcDE(It3Dex, pMCTparas);
                        if (itmp != 0) N[2] += dT2 / itmp;
                        if (iy != 0) N[1] += dT2 / iy;
                        if (ix != 0) N[0] += dT2 / ix;
                    }
                }
            }
        }
        ScaleVec3D_inplace(N, 0.0075);
    } else {
        // Standard 6-point central difference normals
        // Z gradient
        CopyAddVecWeightToIt3DPos(It3Dex, CT1, &pMCTparas->Vgrads[2], Noffset);
        double zPlus = pMCTparas->CalcDE(It3Dex, pMCTparas);
        N[2] = zPlus;
        AddVecWeightToIt3DPos(It3Dex, &pMCTparas->Vgrads[2], -2.0 * Noffset);
        double zMinus = pMCTparas->CalcDE(It3Dex, pMCTparas);
        N[2] = (N[2] - zMinus) * 0.5;

        // X gradient
        CopyAddVecWeightToIt3DPos(It3Dex, CT1, &pMCTparas->Vgrads[0], Noffset);
        double xPlus = pMCTparas->CalcDE(It3Dex, pMCTparas);
        N[0] = xPlus;
        AddVecWeightToIt3DPos(It3Dex, &pMCTparas->Vgrads[0], -2.0 * Noffset);
        double xMinus = pMCTparas->CalcDE(It3Dex, pMCTparas);
        N[0] = (N[0] - xMinus) * 0.5;

        // Y gradient
        CopyAddVecWeightToIt3DPos(It3Dex, CT1, &pMCTparas->Vgrads[1], Noffset);
        double yPlus = pMCTparas->CalcDE(It3Dex, pMCTparas);
        N[1] = yPlus;
        AddVecWeightToIt3DPos(It3Dex, &pMCTparas->Vgrads[1], -2.0 * Noffset);
        double yMinus = pMCTparas->CalcDE(It3Dex, pMCTparas);
        N[1] = (N[1] - yMinus) * 0.5;

        if (HasParityTraceContext()) {
            std::ostringstream payload;
            payload.setf(std::ios::fixed, std::ios::floatfield);
            payload << std::setprecision(12)
                    << "Noffset=" << Noffset
                    << " z_plus=" << zPlus
                    << " z_minus=" << zMinus
                    << " x_plus=" << xPlus
                    << " x_minus=" << xMinus
                    << " y_plus=" << yPlus
                    << " y_minus=" << yMinus
                    << " grad_x=" << N[0]
                    << " grad_y=" << N[1]
                    << " grad_z=" << N[2];
            AppendParityTrace("normals_samples", payload.str());
        }
    }

    // Normal smoothing with roughness estimation
    if (pMCTparas->iSmNormals > 0) {
        Noffset *= 2.0;
        int SmoothN = pMCTparas->iSmNormals;

        if (pMCTparas->iSmNormals < 8) {
            CopyAddVecWeightToIt3DPos(It3Dex, CT1, &pMCTparas->Vgrads[0], -Noffset);
            dNN += pMCTparas->CalcDE(It3Dex, pMCTparas);
            AddVecWeightToIt3DPos(It3Dex, &pMCTparas->Vgrads[0], 2.0 * Noffset);
            dNN += pMCTparas->CalcDE(It3Dex, pMCTparas);
            CopyAddVecWeightToIt3DPos(It3Dex, CT1, &pMCTparas->Vgrads[1], -Noffset);
            dNN += pMCTparas->CalcDE(It3Dex, pMCTparas);
            AddVecWeightToIt3DPos(It3Dex, &pMCTparas->Vgrads[1], 2.0 * Noffset);
            dNN = (dNN + pMCTparas->CalcDE(It3Dex, pMCTparas)) * 0.2;
        }

        double StepSNorm = Noffset * 3.0 / (SmoothN + 0.5);
        double dM = SmoothN * 2.0;

        TVec3D Vx, Vy;
        CreateXYVecsFromNormals(&N, &Vx, &Vy);
        RotateVectorReverse(Vx, pMCTparas->Vgrads);
        RotateVectorReverse(Vy, pMCTparas->Vgrads);

        // X direction smoothing
        CopyAddVecWeightToIt3DPos(It3Dex, CT1, Vx, -SmoothN * StepSNorm);
        double NN1 = 0, dS = 0;
        for (int itmp = -SmoothN; itmp <= SmoothN; itmp++) {
            if (itmp != 0) {
                double dT2 = (pMCTparas->CalcDE(It3Dex, pMCTparas) - dNN) / itmp;
                NN1 += dT2;
                dS += dT2 * dT2;
            }
            AddVecWeightToIt3DPos(It3Dex, Vx, StepSNorm);
        }
        double dSG = dS * dM - NN1 * NN1;

        // Y direction smoothing
        CopyAddVecWeightToIt3DPos(It3Dex, CT1, Vy, -SmoothN * StepSNorm);
        double NN2 = 0;
        dS = 0;
        for (int itmp = -SmoothN; itmp <= SmoothN; itmp++) {
            if (itmp != 0) {
                double dT2 = (pMCTparas->CalcDE(It3Dex, pMCTparas) - dNN) / itmp;
                NN2 += dT2;
                dS += dT2 * dT2;
            }
            AddVecWeightToIt3DPos(It3Dex, Vy, StepSNorm);
        }
        double dT2 = Noffset * 0.5 / (dM * StepSNorm);
        dSG += dS * dM - NN2 * NN2;
        RMCalcRoughness(&N, pMCTparas->sRoughness, &dT2, &dSG);

        if (pMCTparas->iSmNormals < 8) {
            N[0] += NN1 * dT2;
            N[1] += NN2 * dT2;
        }
    }

    // Restore position
    It3Dex->C1 = CT1[0]; It3Dex->C2 = CT1[1]; It3Dex->C3 = CT1[2];

    // Convert normal to packed 16-bit format
    if (pMCTparas->mPsiLight) {
        TLNormals normals;
        MakeWNormalsFromDVec(normals, N);
        pMCTparas->mPsiLight->NormalX = normals[0];
        pMCTparas->mPsiLight->NormalY = normals[1];
        pMCTparas->mPsiLight->NormalZ = normals[2];

        if (HasParityTraceContext()) {
            std::ostringstream payload;
            payload.setf(std::ios::fixed, std::ios::floatfield);
            payload << std::setprecision(12)
                    << "raw_nx=" << N[0]
                    << " raw_ny=" << N[1]
                    << " raw_nz=" << N[2]
                    << " packed_x=" << normals[0]
                    << " packed_y=" << normals[1]
                    << " packed_z=" << normals[2];
            AppendParityTrace("normals_raw", payload.str());
        }
        if (ParityActive()) {
            ParityBeginCurrentStage("normals_raw");
            ParityAddInt("nx", normals[0]);
            ParityAddInt("ny", normals[1]);
            ParityAddInt("nz", normals[2]);
            ParityEndStage();
        }

        // Ensure nonzero normal
        if (*reinterpret_cast<int32_t*>(&pMCTparas->mPsiLight->NormalX) == 0) {
            if (pMCTparas->mPsiLight->NormalZ > 0) {
                pMCTparas->mPsiLight->NormalZ = 32767;
            } else {
                pMCTparas->mPsiLight->NormalZ = -32767;
            }
        }
    }
}

// ============================================================================
// RMCalculateNormalsOnSmoothIt -- normals based on smooth iteration gradient
// ============================================================================

void RMCalculateNormalsOnSmoothIt(PMCTparameter pMCTparas, float& NN) {
    auto It3Dex = pMCTparas->pIt3Dext;

    double Noffset = MinCS(1.0f, pMCTparas->DEstop) * (1.0 + pMCTparas->mZZ * pMCTparas->mctDEstopFactor) * 0.15;
    It3Dex->CalcSIT = 1;

    TVec3D CT1;
    CT1[0] = It3Dex->C1; CT1[1] = It3Dex->C2; CT1[2] = It3Dex->C3;

    pMCTparas->mMandFunction(reinterpret_cast<TPIteration3D>(&It3Dex->C1));
    NN = It3Dex->SmoothItD;

    TVec3D N;
    if (pMCTparas->iSmNormals == 8) {
        ClearDVec(N);
        double StepSNorm = Noffset * 1.3333;
        for (int itmp = -2; itmp <= 2; itmp++) {
            for (int iy = -2; iy <= 2; iy++) {
                for (int ix = -2; ix <= 2; ix++) {
                    if ((itmp | iy | ix) != 0) {
                        CopyAddVecWeightToIt3DPos(It3Dex, CT1, &pMCTparas->Vgrads[2], itmp * StepSNorm);
                        AddVecWeightToIt3DPos(It3Dex, &pMCTparas->Vgrads[1], iy * StepSNorm);
                        AddVecWeightToIt3DPos(It3Dex, &pMCTparas->Vgrads[0], ix * StepSNorm);
                        pMCTparas->mMandFunction(reinterpret_cast<TPIteration3D>(&It3Dex->C1));
                        double dT2 = It3Dex->SmoothItD;
                        if (itmp != 0) N[2] -= dT2 / itmp;
                        if (iy != 0) N[1] -= dT2 / iy;
                        if (ix != 0) N[0] -= dT2 / ix;
                    }
                }
            }
        }
        ScaleVec3D_inplace(N, 0.0075);
    } else {
        // Z gradient
        CopyAddVecWeightToIt3DPos(It3Dex, CT1, &pMCTparas->Vgrads[2], Noffset);
        pMCTparas->mMandFunction(reinterpret_cast<TPIteration3D>(&It3Dex->C1));
        N[2] = It3Dex->SmoothItD;
        AddVecWeightToIt3DPos(It3Dex, &pMCTparas->Vgrads[2], -2.0 * Noffset);
        pMCTparas->mMandFunction(reinterpret_cast<TPIteration3D>(&It3Dex->C1));
        N[2] = (It3Dex->SmoothItD - N[2]) * 0.5;

        // X gradient
        CopyAddVecWeightToIt3DPos(It3Dex, CT1, &pMCTparas->Vgrads[0], Noffset);
        pMCTparas->mMandFunction(reinterpret_cast<TPIteration3D>(&It3Dex->C1));
        N[0] = It3Dex->SmoothItD;
        AddVecWeightToIt3DPos(It3Dex, &pMCTparas->Vgrads[0], -2.0 * Noffset);
        pMCTparas->mMandFunction(reinterpret_cast<TPIteration3D>(&It3Dex->C1));
        N[0] = (It3Dex->SmoothItD - N[0]) * 0.5;

        // Y gradient
        CopyAddVecWeightToIt3DPos(It3Dex, CT1, &pMCTparas->Vgrads[1], Noffset);
        pMCTparas->mMandFunction(reinterpret_cast<TPIteration3D>(&It3Dex->C1));
        N[1] = It3Dex->SmoothItD;
        AddVecWeightToIt3DPos(It3Dex, &pMCTparas->Vgrads[1], -2.0 * Noffset);
        pMCTparas->mMandFunction(reinterpret_cast<TPIteration3D>(&It3Dex->C1));
        N[1] = (It3Dex->SmoothItD - N[1]) * 0.5;
    }

    // Normal smoothing
    if (pMCTparas->iSmNormals > 0) {
        Noffset *= 2.0;
        double dNN_smooth = NN;
        int SmoothN = pMCTparas->iSmNormals;

        // Smooth midpoint (always, including iSmNormals==8)
        CopyAddVecWeightToIt3DPos(It3Dex, CT1, &pMCTparas->Vgrads[0], -Noffset);
        pMCTparas->mMandFunction(reinterpret_cast<TPIteration3D>(&It3Dex->C1));
        float nn_acc = NN + It3Dex->SmoothItD;
        AddVecWeightToIt3DPos(It3Dex, &pMCTparas->Vgrads[0], 2.0 * Noffset);
        pMCTparas->mMandFunction(reinterpret_cast<TPIteration3D>(&It3Dex->C1));
        nn_acc += It3Dex->SmoothItD;
        CopyAddVecWeightToIt3DPos(It3Dex, CT1, &pMCTparas->Vgrads[1], -Noffset);
        pMCTparas->mMandFunction(reinterpret_cast<TPIteration3D>(&It3Dex->C1));
        nn_acc += It3Dex->SmoothItD;
        AddVecWeightToIt3DPos(It3Dex, &pMCTparas->Vgrads[1], 2.0 * Noffset);
        pMCTparas->mMandFunction(reinterpret_cast<TPIteration3D>(&It3Dex->C1));
        dNN_smooth = (nn_acc + It3Dex->SmoothItD) * 0.2;

        double StepSNorm = Noffset * 3.0 / (SmoothN + 0.5);
        double dM = SmoothN * 2.0;

        TVec3D Vx, Vy;
        CreateXYVecsFromNormals(&N, &Vx, &Vy);
        RotateVectorReverse(Vx, pMCTparas->Vgrads);
        RotateVectorReverse(Vy, pMCTparas->Vgrads);

        CopyAddVecWeightToIt3DPos(It3Dex, CT1, Vx, -SmoothN * StepSNorm);
        double NN1 = 0, dS = 0;
        for (int itmp = -SmoothN; itmp <= SmoothN; itmp++) {
            if (itmp != 0) {
                pMCTparas->mMandFunction(reinterpret_cast<TPIteration3D>(&It3Dex->C1));
                double dT2 = (dNN_smooth - It3Dex->SmoothItD) / itmp;
                NN1 += dT2;
                dS += dT2 * dT2;
            }
            AddVecWeightToIt3DPos(It3Dex, Vx, StepSNorm);
        }
        double dSG = dS * dM - NN1 * NN1;

        CopyAddVecWeightToIt3DPos(It3Dex, CT1, Vy, -SmoothN * StepSNorm);
        double NN2 = 0;
        dS = 0;
        for (int itmp = -SmoothN; itmp <= SmoothN; itmp++) {
            if (itmp != 0) {
                pMCTparas->mMandFunction(reinterpret_cast<TPIteration3D>(&It3Dex->C1));
                double dT2 = (dNN_smooth - It3Dex->SmoothItD) / itmp;
                NN2 += dT2;
                dS += dT2 * dT2;
            }
            AddVecWeightToIt3DPos(It3Dex, Vy, StepSNorm);
        }
        double dT2 = Noffset * 0.5 / (dM * StepSNorm);
        dSG += dS * dM - NN2 * NN2;
        RMCalcRoughness(&N, pMCTparas->sRoughness, &dT2, &dSG);
        if (pMCTparas->iSmNormals < 8) {
            N[0] += NN1 * dT2;
            N[1] += NN2 * dT2;
        }
    }

    It3Dex->C1 = CT1[0]; It3Dex->C2 = CT1[1]; It3Dex->C3 = CT1[2];

    if (pMCTparas->bInsideRendering) {
        ScaleVec3D_inplace(N, -1.0);
    }

    if (pMCTparas->mPsiLight) {
        TLNormals normals;
        MakeWNormalsFromDVec(normals, N);
        pMCTparas->mPsiLight->NormalX = normals[0];
        pMCTparas->mPsiLight->NormalY = normals[1];
        pMCTparas->mPsiLight->NormalZ = normals[2];
        if (*reinterpret_cast<int32_t*>(&pMCTparas->mPsiLight->NormalX) == 0) {
            if (pMCTparas->mPsiLight->NormalZ > 0) pMCTparas->mPsiLight->NormalZ = 32767;
            else pMCTparas->mPsiLight->NormalZ = -32767;
        }
    }
}

// ============================================================================
// RMmaxLengthToCutPlane -- distance to cutting planes
// ============================================================================

void RMmaxLengthToCutPlane(PMCTparameter pMCTparas, double& dLength, int& itmp, TPPos3D vPos) {
    dLength = 0;
    if (pMCTparas->iCutOptions == 0) return;

    auto& vfov = pMCTparas->mVgradsFOV;

    if ((pMCTparas->iCutOptions & 1) != 0 && std::fabs(vfov[0]) > 1e-20) {
        // Check sign bits for direction
        bool signVFOV = (vfov[0] < 0);
        bool signCut  = ((pMCTparas->iCutOptions << 3) & 0x80) != 0;
        if (signVFOV == signCut) {
            dLength = 1e20;
        } else {
            double dTmp = (pMCTparas->dCOX - (*vPos)[0]) / vfov[0];
            if (dTmp > dLength) {
                dLength = dTmp;
                itmp = 1;
            }
        }
    }
    if ((pMCTparas->iCutOptions & 2) != 0 && std::fabs(vfov[1]) > 1e-20) {
        bool signVFOV = (vfov[1] < 0);
        bool signCut  = ((pMCTparas->iCutOptions << 2) & 0x80) != 0;
        if (signVFOV == signCut) {
            dLength = 1e20;
        } else {
            double dTmp = (pMCTparas->dCOY - (*vPos)[1]) / vfov[1];
            if (dTmp > dLength) {
                dLength = dTmp;
                itmp = 2;
            }
        }
    }
    if ((pMCTparas->iCutOptions & 4) != 0 && std::fabs(vfov[2]) > 1e-20) {
        bool signVFOV = (vfov[2] < 0);
        bool signCut  = ((pMCTparas->iCutOptions << 1) & 0x80) != 0;
        if (signVFOV == signCut) {
            dLength = 1e20;
        } else {
            double dTmp = (pMCTparas->dCOZ - (*vPos)[2]) / vfov[2];
            if (dTmp > dLength) {
                dLength = dTmp;
                itmp = 3;
            }
        }
    }
}

// ============================================================================
// RMdoBinSearch -- binary search refinement after DE hit
// ============================================================================

void RMdoBinSearch(PMCTparameter pMCTparas, double& DE, double& RLastStepWidth) {
    RenderPoll();
    auto It3Dex = pMCTparas->pIt3Dext;

    int itmp = pMCTparas->iDEAddSteps;
    double dT1 = RLastStepWidth * -0.5;

    while (std::fabs(DE - pMCTparas->msDEstop) > 0.001 && itmp > 0) {
        RenderPoll();
        TVec3D pos, direction;
        LoadIt3DPos(It3Dex, pos);
        LoadPackedVec3D(pMCTparas->mVgradsFOV, direction);
        double depth = pMCTparas->mZZ; // packed member cannot bind to double&
        pMCTparas->msDEstop = RefineRayStep(pos, depth, direction,
            dT1, pMCTparas->mctDEstopFactor, pMCTparas->DEstop);
        pMCTparas->mZZ = depth;
        StoreIt3DPos(It3Dex, pos);
        itmp--;
        if (itmp <= 0) break;
        DE = pMCTparas->CalcDE(It3Dex, pMCTparas);

        double absDT1 = std::fabs(dT1);
        if (DE < pMCTparas->msDEstop) {
            dT1 = absDT1 * -s055;
        } else {
            dT1 = absDT1 * s055;
        }
    }
}

// ============================================================================
// RMdoBinSearchIt -- binary search on iteration count boundary
// ============================================================================

double CalcIterationBoundaryStep(float YP, float LastSI, float smooth, double dmul) {
    const float R = LastSI - smooth;
    double step;
    if (std::fabs(R) < 1e-30) {
        step = (LastSI < smooth ? 1.0 : 0.0) - 0.5;
    } else {
        step = (YP - smooth) / (R * (R < 0 ? 500.0f : 1000.0f));
    }
#if defined(MB3D_SOFTWARE_EXT80)
    // Invalid arithmetic without a NaN operand produces Intel's negative
    // indefinite. ARM's default NaN has the opposite sign, inspected later by
    // the original depth codec. Incoming NaN payloads/signs remain untouched.
    if (std::isnan(step) && !std::isnan(YP) && !std::isnan(LastSI) && !std::isnan(smooth))
        step = -std::numeric_limits<double>::quiet_NaN();
#endif
    if (step > 4) step = std::sqrt(step) * 2;
    else if (step < -9) step = std::sqrt(-step) * -3;
#if defined(_MSC_VER) && defined(_M_X64)
    // FPC rounds the product to Double, then adds the untyped 0.0005
    // literal in Extended. Rounding that literal to Double changes the step.
    const double inputs[] = {step, dmul};
    FinalizeIterationBoundaryStep_x87(&step, inputs);
    return step;
#elif defined(MB3D_SOFTWARE_EXT80)
    const numeric::ExtendedScope scope;
    double product = step * dmul;
    if (std::isnan(product) && !std::isnan(step) && !std::isnan(dmul))
        product = -std::numeric_limits<double>::quiet_NaN();
    // Original untyped Pascal literal, retained as Extended until this store.
    const auto halfMilli = numeric::Extended80::fromBits(0x3ff4, 0x83126e978d4fdf3bULL);
    return double(numeric::Extended80(product) + halfMilli);
#else
    const double product = step * dmul;
    return static_cast<double>(static_cast<long double>(product) + 0.0005L);
#endif
}

void RMdoBinSearchIt(PMCTparameter pMCTparas, void* depthAddress) {
    double ZZ;
    std::memcpy(&ZZ, depthAddress, sizeof(ZZ));
    struct DepthStore {
        void* address;
        double& value;
        void commit() const { std::memcpy(address, &value, sizeof(value)); }
        void reload() const { std::memcpy(&value, address, sizeof(value)); }
        ~DepthStore() { commit(); }
    } depthStore{depthAddress, ZZ};
    RenderPoll();
    auto It3Dex = pMCTparas->pIt3Dext;

    float YP = static_cast<float>(It3Dex->maxIt - 0.99);
    int MItmp = pMCTparas->iMaxIt;
    int MItmp2 = pMCTparas->iMaxitF2;

    if (It3Dex->DEoption != 20) pMCTparas->iMaxIt++;
    It3Dex->maxIt = pMCTparas->iMaxIt;
    if (pMCTparas->DEoption2 != 20) pMCTparas->iMaxitF2++;

    int itmp = pMCTparas->iDEAddSteps;
    It3Dex->CalcSIT = 1;
    double dT1 = 0;
    double dmul = 1;
    bool firstIt = true;
    float LastSI, LastDif;

    do {
        RenderPoll();
#if defined(_MSC_VER) && defined(_M_X64)
        // Preserve destination-first ADDSD: opposite-sign NaNs are not
        // commutative, and the depth codec deliberately inspects their sign.
        AccumulateIterationDepth_sse2(&ZZ, dT1);
#elif defined(MB3D_SOFTWARE_EXT80)
        if (std::isnan(ZZ)) {
            auto bits = std::bit_cast<uint64_t>(ZZ) | 0x0008000000000000ULL;
            ZZ = std::bit_cast<double>(bits);
        } else if (std::isnan(dT1)) {
            auto bits = std::bit_cast<uint64_t>(dT1) | 0x0008000000000000ULL;
            ZZ = std::bit_cast<double>(bits);
        } else {
            ZZ += dT1;
            if (std::isnan(ZZ)) ZZ = -std::numeric_limits<double>::quiet_NaN();
        }
#else
        ZZ += dT1;
#endif
        AddVecWeightToIt3DPos(It3Dex, pMCTparas->mVgradsFOV, dT1);
        depthStore.commit(); // preserve visibility to the DE callback
        pMCTparas->CalcDE(It3Dex, pMCTparas);
        depthStore.reload();

        if (!firstIt) {
            if (LastDif < std::fabs(YP - It3Dex->SmoothItD)) {
                ZZ -= dT1;
                AddVecWeightToIt3DPos(It3Dex, pMCTparas->mVgradsFOV, -dT1);
                It3Dex->SmoothItD = LastSI;
                if (dT1 > 0) dmul *= 0.5;
                else dmul *= 0.7;
            }
        }

        LastDif = std::fabs(YP - It3Dex->SmoothItD);
        LastSI = It3Dex->SmoothItD;

        if (It3Dex->SmoothItD > It3Dex->maxIt - 0.1f) {
            dT1 = -3.0;
        } else {
#if defined(_MSC_VER) && defined(_M_X64)
            OffsetIterationDepth_x87(&ZZ);
#elif defined(MB3D_SOFTWARE_EXT80)
            {
                const numeric::ExtendedScope scope;
                const auto milli = numeric::Extended80::fromBits(0x3ff5, 0x83126e978d4fdf3bULL);
                ZZ = double(numeric::Extended80(ZZ) - milli);
            }
#else
            ZZ = static_cast<double>(static_cast<long double>(ZZ) - 0.001L);
#endif
            AddVecWeightToIt3DPos(It3Dex, pMCTparas->mVgradsFOV, -0.001);
            depthStore.commit(); // preserve visibility to the DE callback
            pMCTparas->CalcDE(It3Dex, pMCTparas);
            depthStore.reload();
            dT1 = CalcIterationBoundaryStep(YP, LastSI, It3Dex->SmoothItD, dmul);
        }
        firstIt = false;
        itmp--;
    } while (itmp >= 0);

    It3Dex->maxIt = MItmp;
    pMCTparas->iMaxIt = MItmp;
    pMCTparas->iMaxitF2 = MItmp2;
}

// ============================================================================
// RMdoSecantSearch -- secant method refinement
// ============================================================================

void RMdoSecantSearch(PMCTparameter pMCTparas, double& DE, double& RLastStepWidth, double& RLastDE) {
    RenderPoll();
    auto It3Dex = pMCTparas->pIt3Dext;

    int itmp = (pMCTparas->iDEAddSteps >> 1) + 1;

    while (std::fabs(DE - pMCTparas->msDEstop) > 0.001) {
        RenderPoll();
        double nzRLastDE = NotZero(RLastDE - DE);
        double RSFmul = RLastStepWidth * (DE - pMCTparas->msDEstop) / nzRLastDE;

        if (DE < pMCTparas->msDEstop) {
            if (RSFmul >= 0 || RSFmul < std::fabs(RLastStepWidth) * -0.94) {
                RLastStepWidth = std::fabs(RLastStepWidth) * -0.5;
            } else {
                RLastStepWidth = RSFmul;
            }
        } else {
            if (RSFmul <= 0 || RSFmul > std::fabs(RLastStepWidth) * 0.94) {
                RLastStepWidth = std::fabs(RLastStepWidth) * 0.5;
            } else {
                RLastStepWidth = RSFmul;
            }
        }
        RLastDE = DE;
        pMCTparas->mZZ += RLastStepWidth;
        AddVecWeightToIt3DPos(It3Dex, pMCTparas->mVgradsFOV, RLastStepWidth);
        pMCTparas->msDEstop = pMCTparas->DEstop * (1.0f + static_cast<float>(std::fabs(pMCTparas->mZZ)) * pMCTparas->mctDEstopFactor);
        itmp--;
        if (itmp <= 0) break;
        DE = pMCTparas->CalcDE(It3Dex, pMCTparas);
    }
}

// ============================================================================
// RMdoColor -- coloring based on formula output
// ============================================================================



#ifdef MB3D_ENABLE_PARITY_TRACE
static thread_local double gRMTraceLastColorScalar = 0.0;
static thread_local double gRMTraceLastColorOTrap = 0.0;

double RMTraceLastColorScalar() { return gRMTraceLastColorScalar; }
double RMTraceLastColorOTrap() { return gRMTraceLastColorOTrap; }
#endif

void RMdoColor(PMCTparameter pMCTparas) {
    auto It3Dex = pMCTparas->pIt3Dext;
    if (!pMCTparas->mPsiLight) return;

    float s;
#ifdef MB3D_ENABLE_PARITY_TRACE
    gRMTraceLastColorOTrap = It3Dex->OTrap;
#endif
    if (pMCTparas->DEoption == 20) {
        // In dIFS mode, OTrap already contains the final color computed by doHybridIFS3D.
        s = static_cast<float>(It3Dex->OTrap * 4096.0);
    } else {
        switch (pMCTparas->ColorOption) {
            case 1:
                s = static_cast<float>(std::log(It3Dex->Rout / (It3Dex->Rold + 1.0)) * pMCTparas->mctColorMul);
                break;
            case 2:
                s = static_cast<float>((std::atan2(It3Dex->y - It3Dex->C2, It3Dex->x - It3Dex->C1) + Pi) * 5200.0);
                break;
            case 3:
                s = static_cast<float>((std::atan2(It3Dex->z - It3Dex->C3, It3Dex->x - It3Dex->C1) + Pi) * 5200.0);
                break;
            case 4:
                s = static_cast<float>((std::atan2(It3Dex->z - It3Dex->C3, It3Dex->y - It3Dex->C2) + Pi) * 5200.0);
                break;
            case 5: {
                s = static_cast<float>((std::atan2(It3Dex->x, It3Dex->y) + Pi) * 5215.0);
                double len = std::sqrt(It3Dex->x * It3Dex->x + It3Dex->y * It3Dex->y + It3Dex->z * It3Dex->z);
                if (len > 1e-200) {
                    float s2 = static_cast<float>((Pi + ArcSinSafe(It3Dex->z / len) * 2.0) * 5215.0);
                    MinMaxClip15bit(s2, pMCTparas->mPsiLight->SIgradient);
                }
                break;
            }
            default:
                s = static_cast<float>(It3Dex->OTrap * 4096.0);
                break;
        }
    }
#ifdef MB3D_ENABLE_PARITY_TRACE
    gRMTraceLastColorScalar = s;
#endif
    MinMaxClip15bit(s, pMCTparas->mPsiLight->OTrap);
}

// ============================================================================
// RMdoColorOnIt -- color on specific iteration
// ============================================================================

void RMdoColorOnIt(PMCTparameter pMCTparas) {
    auto It3Dex = pMCTparas->pIt3Dext;

    if (pMCTparas->ColorOnIt == 1) {
        It3Dex->x = It3Dex->C1;
        It3Dex->y = It3Dex->C2;
        It3Dex->z = It3Dex->C3;
    } else {
        int i = It3Dex->maxIt;
        It3Dex->maxIt = pMCTparas->ColorOnIt - 1;
        It3Dex->RStop = static_cast<float>(It3Dex->RStop * It3Dex->RStop * 64.0f);
        pMCTparas->CalcDE(It3Dex, pMCTparas);
        It3Dex->maxIt = i;
        It3Dex->RStop = pMCTparas->dRStop;
    }
}

// ============================================================================
// RMcalcNanglesForCut -- calculate normals for cut plane hits
// ============================================================================

void RMcalcNanglesForCut(PMCTparameter pMCTparas, int CutPlane) {
    if (!pMCTparas->mPsiLight) return;

    if (CutPlane > 0) {
        double NN_val = 8388352.0 - pMCTparas->ZcMul * (std::sqrt(pMCTparas->mZZ * pMCTparas->Zcorr + 1.0) - 1.0);
        int itmp_n;
        if (NN_val < 0) itmp_n = 0; else itmp_n = static_cast<int>(std::round(NN_val));
        *reinterpret_cast<uint32_t*>(&pMCTparas->mPsiLight->RoughZposFine) = static_cast<uint32_t>(itmp_n) << 8;

        int cp = CutPlane - 1;
        TVec3D N;
        double divisor = pMCTparas->Vgrads[2][cp];
        double NN_div;
        if (std::fabs(divisor) < 1e-40) NN_div = -1e40;
        else NN_div = -1.0 / divisor;

        N[0] = pMCTparas->Vgrads[0][cp] * NN_div;
        N[1] = pMCTparas->Vgrads[1][cp] * NN_div;
        N[2] = -1.0;

        TLNormals normals;
        MakeWNormalsFromDVec(normals, N);
        pMCTparas->mPsiLight->NormalX = normals[0];
        pMCTparas->mPsiLight->NormalY = normals[1];
        pMCTparas->mPsiLight->NormalZ = normals[2];
    } else {
        *reinterpret_cast<uint32_t*>(&pMCTparas->mPsiLight->RoughZposFine) = 0x7FFF0000u;
        pMCTparas->mPsiLight->NormalX = 0;
        pMCTparas->mPsiLight->NormalY = 0;
        pMCTparas->mPsiLight->NormalZ = -32767;
    }
}

// ============================================================================
// CalcZposAndRough -- encode Z position and roughness into TsiLight5
// ============================================================================

void CalcZposAndRough(TPsiLight5 siLight, PMCTparameter mct, double ZZ) {
    // Calc.pas tests the sign BIT (including negative NaNs), converts the
    // positive depth term with FISTP, then subtracts with 32-bit wraparound.
    // The commented Pascal expression has different overflow/rounding behavior.
    uint32_t encoded[3]{}; // packed result, signed depth before clamp, rough bits
#if defined(_MSC_VER) && defined(_M_X64)
    const double inputs[] = {ZZ, mct->Zcorr, mct->ZcMul, mct->sRoughness};
    EncodeDepthRough_x87(encoded, inputs, mct->iSmNormals);
#else
    const auto fistp32 = [](long double value) -> int32_t {
        const long double rounded = std::nearbyint(value);
        if (!std::isfinite(rounded) || rounded < -2147483648.0L || rounded > 2147483647.0L)
            return INT32_MIN;
        return static_cast<int32_t>(rounded);
    };
    const long double depth = std::signbit(ZZ) ? 0.0L : static_cast<long double>(ZZ);
    const int32_t distance = fistp32(mct->ZcMul * (std::sqrt(depth * mct->Zcorr + 1.0L) - 1.0L));
    encoded[1] = 8388352u - static_cast<uint32_t>(distance);
    int32_t before;
    std::memcpy(&before, &encoded[1], sizeof(before));
    encoded[0] = static_cast<uint32_t>(std::clamp(before, 0, 8388352)) << 8;
    const unsigned smoothByte = mct->iSmNormals & 255u;
    if (smoothByte > 0 && smoothByte < 128) {
        encoded[2] = static_cast<uint32_t>(fistp32(static_cast<long double>(mct->sRoughness) * 255.0L));
        encoded[0] |= encoded[2];
    }
#endif
    const uint32_t packed = encoded[0];
    int32_t beforeClamp, rough;
    std::memcpy(&beforeClamp, &encoded[1], sizeof(beforeClamp));
    std::memcpy(&rough, &encoded[2], sizeof(rough));
    const int itmp_v = std::clamp(beforeClamp, 0, 8388352);
    std::memcpy(&siLight->RoughZposFine, &packed, sizeof(packed));
    if (HasParityTraceContext()) {
        if (std::signbit(ZZ)) ZZ = 0;
        const double sqrtTerm = std::sqrt(ZZ * mct->Zcorr + 1.0);
        std::ostringstream payload;
        payload.setf(std::ios::fixed, std::ios::floatfield);
        payload << std::setprecision(12)
                << "ZZ=" << ZZ
                << " Zcorr=" << mct->Zcorr
                << " ZcMul=" << mct->ZcMul
                << " iSmNormals=" << mct->iSmNormals
                << " sRoughness=" << mct->sRoughness
                << " sqrt_term=" << sqrtTerm
                << " itmp_before_clamp=" << beforeClamp
                << " itmp_after_clamp=" << itmp_v
                << " rough_byte=" << rough
                << " packed_u32=" << packed
                << " RoughZposFine=" << siLight->RoughZposFine
                << " Zpos=" << siLight->Zpos;
        AppendParityTrace("z_encode", payload.str());
    }
}

// ============================================================================
// IniIt3D -- initialize TIteration3Dext from TMCTparameter
// ============================================================================

void IniIt3D(PMCTparameter PMCT, TPIteration3Dext It3Dex) {
    It3Dex->J4 = PMCT->dJUw;
    It3Dex->Ju4 = PMCT->dJUw;

    // Copy Julia constants
    It3Dex->J1 = PMCT->dJUx;
    It3Dex->J2 = PMCT->dJUy;
    It3Dex->J3 = PMCT->dJUz;
    It3Dex->Ju1 = PMCT->dJUx;
    It3Dex->Ju2 = PMCT->dJUy;
    It3Dex->Ju3 = PMCT->dJUz;

    // Copy hybrid chain parameters
    std::memcpy(It3Dex->nHybrid, PMCT->nHybrid, sizeof(int32_t) * 6);
    std::memcpy(It3Dex->fHPVar, PMCT->fHPVar, sizeof(void*) * 6);
    std::memcpy(It3Dex->fHybrid, PMCT->fHybrid, sizeof(ThybridIteration2) * 6);

    // Copy other settings
    It3Dex->CalcSIT = PMCT->bCalcSIT;
    It3Dex->EndTo = PMCT->wEndTo;
    It3Dex->DoJulia = PMCT->bDoJulia;
    It3Dex->LNRStop = PMCT->dLNRStop;
    It3Dex->DEoption = PMCT->DEoption;
    std::memcpy(It3Dex->fHln, PMCT->fHln, sizeof(float) * 6);
    It3Dex->iRepeatFrom = PMCT->RepeatFrom1;
    It3Dex->iStartFrom = PMCT->StartFrom1;
    It3Dex->RStop = PMCT->dRStop;
    It3Dex->maxIt = PMCT->iMaxIt;

    // Copy 4D rotation matrix
    std::memcpy(It3Dex->SMatrix4, PMCT->Smatrix4d, sizeof(TSMatrix4));
    std::memcpy(It3Dex->pInitialization, PMCT->pInitialization, sizeof(TFormulaInitialization) * 6);

    if (PMCT->DEoption == 20) It3Dex->RStopD = -1e10;

    // Set map function pointers (from Calc.pas IniIt3D)
    It3Dex->PMapFunc = MapFuncSphereSpline;
    It3Dex->PMapFunc2 = MapFuncDirectXYspline;
    It3Dex->Deriv1 = 1.0;

    // Link back
    PMCT->pIt3Dext = It3Dex;

    if (HasParityTraceContext()) {
        std::ostringstream payload;
        payload.setf(std::ios::fixed, std::ios::floatfield);
        payload << std::setprecision(12)
                << "J1=" << It3Dex->J1
                << " J2=" << It3Dex->J2
                << " J3=" << It3Dex->J3
                << " J4=" << It3Dex->J4
                << " Ju1=" << It3Dex->Ju1
                << " Ju2=" << It3Dex->Ju2
                << " Ju3=" << It3Dex->Ju3
                << " Ju4=" << It3Dex->Ju4
                << " DoJulia=" << static_cast<int>(It3Dex->DoJulia)
                << " EndTo=" << static_cast<int>(It3Dex->EndTo)
                << " iRepeatFrom=" << It3Dex->iRepeatFrom
                << " iStartFrom=" << It3Dex->iStartFrom
                << " RStop=" << It3Dex->RStop
                << " maxIt=" << It3Dex->maxIt
                << " DEoption=" << It3Dex->DEoption
                << " PMapFunc=" << ParityPointerState(reinterpret_cast<void*>(It3Dex->PMapFunc))
                << " PMapFunc2=" << ParityPointerState(reinterpret_cast<void*>(It3Dex->PMapFunc2));
        for (int i = 0; i < 6; ++i) {
            const bool activeSlot = It3Dex->nHybrid[i] != 0;
            payload << " nHybrid" << i << "=" << It3Dex->nHybrid[i]
                    << " fHPVar" << i << "=" << (activeSlot ? ParityPointerState(It3Dex->fHPVar[i]) : "inactive")
                    << " fHybrid" << i << "=" << (activeSlot ? ParityHybridName(reinterpret_cast<void*>(It3Dex->fHybrid[i])) : "inactive")
                    << " fHln" << i << "=" << It3Dex->fHln[i];
        }
        AppendParityTrace("it3d_init", payload.str());
    }
}

// ============================================================================
// RayMarch -- main ray marching loop
// ============================================================================

void RayMarch(TPRaymarchRec RMrec) {
    RenderPoll();
    auto It3Dex = RMrec->PIt3Dex;
    auto PMCTparas = RMrec->PMCTparas;
    auto captureFirstCalcDeEnter = [&](bool afterCutPlane) {
        RaySetupDumpRecord* rec = RMrec->RaySetupRecordOut;
        if (!rec) {
            return;
        }
        CaptureFirstCalcDEEnterDump(*rec, *PMCTparas, *It3Dex, RMrec->CutPlaneHit, afterCutPlane);
    };

    It3Dex->CalcSIT = 0;
    int itmp = 0;
    RMrec->StepCount = 0;
    RMrec->Zstepped = 0;
    RMrec->StartInside = 0;
    RMrec->CutPlaneHit = 0;
    PMCTparas->mZZ = 0.0;

    // Headless/reference path matches Calc.pas::RayMarch, not CalcThread.pas.
    RMrec->ZZposMul = DotOfVectorsNormalize(RMrec->MarchVec, RMrec->VievVec);

    PMCTparas->msDEstop = static_cast<float>(RMrec->StartDEstop);
    LongBool bFirstStep = PMCTparas->bMCTFirstStepRandom;
    LongBool bInsideTmp = PMCTparas->bInsideRendering;
    // Mirror of Pascal Calc.pas RayMarch (T06 patch): per-thread deterministic LCG
    // used for the first-step StepCount jitter.
    uint32_t lcgState = mb3d::PasLcgInitFromThread(PMCTparas->iThreadId);

    // Save start position
    RMrec->ActPos[0] = It3Dex->C1;
    RMrec->ActPos[1] = It3Dex->C2;
    RMrec->ActPos[2] = It3Dex->C3;

    // Apply initial step forward
    RMrec->Zstepped += static_cast<double>(RMrec->StepForward);
    PMCTparas->mZZ = RMrec->Zstepped;
    TVec3D currentPos{It3Dex->C1, It3Dex->C2, It3Dex->C3};
    mAddVecWeight(currentPos, RMrec->MarchVec, static_cast<double>(RMrec->StepForward));
    It3Dex->C1 = currentPos[0];
    It3Dex->C2 = currentPos[1];
    It3Dex->C3 = currentPos[2];

    double dTmp, dT1;

    // Handle cutting planes
    if (PMCTparas->iCutOptions > 0) {
        RMmaxLengthToCutPlane(PMCTparas, dT1, itmp, reinterpret_cast<TPPos3D>(&RMrec->ActPos));
        RMrec->CutPlaneHit = itmp;
        if (dT1 > RMrec->MaxRayLength) {
            if (RMrec->RaySetupRecordOut) {
                MarkNoFirstCalcDEDump(*RMrec->RaySetupRecordOut, RMrec->CutPlaneHit);
            }
            if (RMrec->RaySetupOnly) {
                RMrec->RMresult = 0;
                return;
            }
            // Pascal CalcThread keeps this miss on the normal bookkeeping path
            // instead of returning immediately.
            It3Dex->ItResultI = 0;
            PMCTparas->MaxItsResult = PMCTparas->iMaxIt;
            dTmp = static_cast<double>(PMCTparas->msDEstop) + 5.0;
            RMrec->Zstepped = RMrec->MaxRayLength + 1.0;
            PMCTparas->mZZ = RMrec->Zstepped;
        } else {
            RMrec->Zstepped = dT1;
            PMCTparas->mZZ = RMrec->Zstepped;
            AddVecWeightToIt3DPos(It3Dex, RMrec->MarchVec, dT1);
            if (DebugThreadMsDeStopEnabled()) {
                PMCTparas->msDEstop = static_cast<float>(RMrec->StartDEstop *
                    (1.0 + PMCTparas->mZZ * PMCTparas->mctDEstopFactor));
            } else {
                PMCTparas->msDEstop = static_cast<float>(RMrec->StartDEstop *
                    (1.0 + Clamp0D(RMrec->ActZpos + RMrec->Zstepped * RMrec->ZZposMul) *
                           PMCTparas->mctDEstopFactor));
            }
            captureFirstCalcDeEnter(true);
            if (RMrec->RaySetupOnly) {
                RMrec->RMresult = 0;
                return;
            }
            dTmp = PMCTparas->CalcDE(It3Dex, PMCTparas);
        }
    } else {
        captureFirstCalcDeEnter(false);
        if (RMrec->RaySetupOnly) {
            RMrec->RMresult = 0;
            return;
        }
        dTmp = PMCTparas->CalcDE(It3Dex, PMCTparas);
    }

    if (PrimaryTraceActive(RMrec)) {
        std::ostringstream payload;
        payload.setf(std::ios::fixed, std::ios::floatfield);
        payload << std::setprecision(12)
                << "cut=" << itmp
                << " mZZ=" << PMCTparas->mZZ
                << " msDEstop=" << PMCTparas->msDEstop
                << " dTmp=" << dTmp
                << " ItResultI=" << It3Dex->ItResultI
                << " SmoothItD=" << It3Dex->SmoothItD
                << " Rout=" << It3Dex->Rout
                << " OTrap=" << It3Dex->OTrap
                << " DEoptionResult=" << PMCTparas->DEoptionResult
                << " MaxItsResult=" << PMCTparas->MaxItsResult;
        AppendPrimaryTrace(RMrec, "de_initial", -1, payload.str());
    }

check_initial_hit:
    // Check if already inside
    if (It3Dex->ItResultI >= PMCTparas->MaxItsResult || dTmp < PMCTparas->msDEstop) {
        RMrec->StartInside = 1;
        if (dTmp < PMCTparas->msDEstop) RMrec->RMresult = 1;
        else RMrec->RMresult = 2;
        RMrec->ActPos[0] = It3Dex->C1;
        RMrec->ActPos[1] = It3Dex->C2;
        RMrec->ActPos[2] = It3Dex->C3;
        return;
    }

    // Main ray marching loop
    float RSFmul = 1.0f;
    double RLastStepWidth = dTmp * PMCTparas->sZstepDiv;
    double RLastDE = 0.0;
    int traceStep = 0;

    do {
        RenderPoll();
        if (PrimaryTraceActive(RMrec)) {
            std::ostringstream payload;
            payload.setf(std::ios::fixed, std::ios::floatfield);
            payload << std::setprecision(12)
                << "step_index=" << traceStep
                << " mZZ=" << PMCTparas->mZZ
                << " dTmp=" << dTmp
                << " RLastStepWidth=" << RLastStepWidth
                << " RLastDE=" << RLastDE
                << " msDEstop=" << PMCTparas->msDEstop
                << " msDEsub=" << PMCTparas->msDEsub
                << " sZstepDiv=" << PMCTparas->sZstepDiv
                << " ItResultI=" << It3Dex->ItResultI
                << " RSFmul=" << RSFmul;
            AppendPrimaryTrace(RMrec, "step_begin", traceStep, payload.str());
        }
        // Handle inside-while-stepping
        if (It3Dex->ItResultI >= PMCTparas->MaxItsResult) {
            dT1 = -0.5 * RLastStepWidth;
            RMrec->Zstepped += dT1;
            PMCTparas->mZZ = RMrec->Zstepped;
            AddVecWeightToIt3DPos(It3Dex, RMrec->MarchVec, dT1);
            if (DebugCalcPasInsideMsDeStopEnabled()) {
                PMCTparas->msDEstop = static_cast<float>(RMrec->StartDEstop *
                    (1.0 + Clamp0D(RMrec->ActZpos + RMrec->Zstepped * RMrec->ZZposMul) *
                           PMCTparas->mctDEstopFactor));
            } else {
                PMCTparas->msDEstop = static_cast<float>(RMrec->StartDEstop *
                    (1.0 + RMrec->Zstepped * PMCTparas->mctDEstopFactor));
            }
            dTmp = PMCTparas->CalcDE(It3Dex, PMCTparas);
            RLastStepWidth = -dT1;
        }

        if (It3Dex->ItResultI < PMCTparas->iMinIt ||
            (It3Dex->ItResultI < PMCTparas->MaxItsResult && dTmp >= PMCTparas->msDEstop)) {
            // ===== Next step =====
            RLastDE = dTmp;
            if (DebugCalcPasRaymarchDynFogEnabled()) {
                dTmp = dTmp * static_cast<double>(PMCTparas->sZstepDiv) *
                       static_cast<double>(RSFmul);
            } else {
                if (DebugDynFogX87StepEnabled()) {
                    dTmp = DynFogStepPasLike(dTmp, PMCTparas->msDEsub, PMCTparas->msDEstop,
                                             PMCTparas->sZstepDiv, RSFmul, s011f);
                } else {
                    const double dynFogStep =
                        (dTmp - static_cast<double>(PMCTparas->msDEsub) *
                                    static_cast<double>(PMCTparas->msDEstop)) *
                        static_cast<double>(PMCTparas->sZstepDiv) *
                        static_cast<double>(RSFmul);
                    if (DebugDynFogNoFloatCastEnabled()) {
                        dTmp = MaxCD(static_cast<double>(s011f), dynFogStep);
                    } else {
                        dTmp = MaxCS(s011f, static_cast<float>(dynFogStep));
                    }
                }
            }
            // FPC stores the Single*Single product before widening to Double.
            dT1 = CalcMarchStepLimit(PMCTparas->msDEstop, PMCTparas->mctMH04ZSD);

            if (dT1 < dTmp) {
                // Pascal Calc.pas RayMarch (post-T06):
                //   if not bFirstStep then StepCount := StepCount + dT1/dTmp
                //   else StepCount := StepCount + PasLcgU01(lcgState);
                const double stepCountAdd =
                    bFirstStep ? mb3d::PasLcgU01(lcgState)
                               : (dT1 / dTmp);
                if (PMCTparas->DFogOnIt == 0) {
                    RMrec->StepCount += static_cast<float>(stepCountAdd);
                } else if (It3Dex->ItResultI == PMCTparas->DFogOnIt) {
                    RMrec->StepCount += static_cast<float>(stepCountAdd);
                }
                dTmp = dT1;
            } else {
                if (PMCTparas->DFogOnIt == 0) {
                    RMrec->StepCount += 1.0f;
                } else if (It3Dex->ItResultI == PMCTparas->DFogOnIt) {
                    RMrec->StepCount += 1.0f;
                }
            }

            // Volume-light fog accumulation (Pascal CalcThread.pas DoDynFog,
            // else-branch when DFogOnIt == 65535). The clamped-dTmp branches
            // above are no-ops in this case; Pascal runs its sub-step loop
            // over the previous step's traversal to sample the precomputed
            // vol-light cube map and accumulate StepCount.
            if (PMCTparas->DFogOnIt == 65535) {
                const TVec3D curPos{It3Dex->C1, It3Dex->C2, It3Dex->C3};
                VolLightDoDynFogSubstep(curPos, RMrec->MarchVec,
                                        static_cast<float>(RLastStepWidth),
                                        PMCTparas->mZZ,
                                        PMCTparas->mctDEstopFactor,
                                        PMCTparas->VLstepmul,
                                        PMCTparas->VLmul,
                                        PMCTparas->StepWidth,
                                        RMrec->StepCount);
            }

            if (PrimaryTraceActive(RMrec)) {
                double stepAfterClamp = dTmp;
                std::ostringstream payload;
                payload.setf(std::ios::fixed, std::ios::floatfield);
                payload << std::setprecision(17)
                        << "step_index=" << traceStep
                        << " dT1=" << dT1
                        << " step_after_clamp=" << stepAfterClamp
                        << " msDEsub=" << PMCTparas->msDEsub
                        << " sZstepDiv=" << PMCTparas->sZstepDiv
                        << " StepCount=" << RMrec->StepCount;
                AppendPrimaryTrace(RMrec, "step_after_dynfog", traceStep, payload.str());
            }

            if (bFirstStep) {
                bFirstStep = 0;
                int32_t seedBefore = RMrec->seed;
                RMrec->seed = AdvancePascalSeed(RMrec->seed);
                double randomScale = PascalFirstStepScale(RMrec->seed);
                dTmp = randomScale * dTmp;
                if (PrimaryTraceActive(RMrec)) {
                    std::ostringstream payload;
                    payload.setf(std::ios::fixed, std::ios::floatfield);
                    payload << std::setprecision(17)
                            << "step_index=" << traceStep
                            << " seed_before=" << seedBefore
                            << " seed_after=" << RMrec->seed
                            << " random_scale=" << randomScale
                            << " step_after_random=" << dTmp;
                    AppendPrimaryTrace(RMrec, "step_after_random", traceStep, payload.str());
                }
            }

            RMrec->Zstepped += dTmp;
            PMCTparas->mZZ = RMrec->Zstepped;
            if (RMrec->Zstepped > RMrec->MaxRayLength) {
                // Pascal CalcThread.pas:887-895 -- if we overshoot the
                // ray end while volume light is active, accumulate one
                // final vol-light substep across the remainder of the
                // traversal. Without this, rays that never hit a
                // surface drop their final fog contribution and the
                // sky/background pixels appear unshaded.
                if (PMCTparas->DFogOnIt == 65535) {
                    double adj = RMrec->MaxRayLength - RMrec->Zstepped + dTmp;
                    TVec3D curPos{It3Dex->C1, It3Dex->C2, It3Dex->C3};
                    mAddVecWeight(curPos, RMrec->MarchVec, adj);
                    It3Dex->C1 = curPos[0];
                    It3Dex->C2 = curPos[1];
                    It3Dex->C3 = curPos[2];
                    VolLightDoDynFogSubstep(curPos, RMrec->MarchVec,
                                            static_cast<float>(adj),
                                            PMCTparas->mZZ,
                                            PMCTparas->mctDEstopFactor,
                                            PMCTparas->VLstepmul,
                                            PMCTparas->VLmul,
                                            PMCTparas->StepWidth,
                                            RMrec->StepCount);
                }
                break;
            }
            RLastStepWidth = dTmp;
            TVec3D currentPos{It3Dex->C1, It3Dex->C2, It3Dex->C3};
            mAddVecWeight(currentPos, RMrec->MarchVec, dTmp);
            It3Dex->C1 = currentPos[0];
            It3Dex->C2 = currentPos[1];
            It3Dex->C3 = currentPos[2];
            PMCTparas->msDEstop = static_cast<float>(RMrec->StartDEstop *
                (1.0 + Clamp0D(RMrec->ActZpos + RMrec->Zstepped * RMrec->ZZposMul) *
                       PMCTparas->mctDEstopFactor));
            ApplyDebugStepCalcdeEntryOverride(PMCTparas, It3Dex, traceStep);
            ApplyDebugFpBarrier(PMCTparas, It3Dex, &dTmp, &RLastStepWidth, &RSFmul,
                                traceStep, "before_calcde");
            dTmp = PMCTparas->CalcDE(It3Dex, PMCTparas);
            ApplyDebugStepDeResultOverride(dTmp, traceStep);

            // Clamp overshoot
            if (dTmp > RLastDE + RLastStepWidth) dTmp = RLastDE + RLastStepWidth;
            ApplyDebugStepAfterDeStateOverride(PMCTparas, It3Dex, dTmp, RSFmul, traceStep);
            ApplyDebugFpBarrier(PMCTparas, It3Dex, &dTmp, &RLastStepWidth, &RSFmul,
                                traceStep, "after_de");

            if (PrimaryTraceActive(RMrec)) {
                std::ostringstream payload;
                payload.setf(std::ios::fixed, std::ios::floatfield);
                payload << std::setprecision(17)
                        << "step_index=" << traceStep
                        << " mZZ=" << PMCTparas->mZZ
                        << " C1=" << It3Dex->C1
                        << " C2=" << It3Dex->C2
                        << " C3=" << It3Dex->C3
                        << " dTmp=" << dTmp
                        << " Rout=" << It3Dex->Rout
                        << " ItResultI=" << It3Dex->ItResultI
                        << " RSFmul=" << RSFmul
                        << " DEoptionResult=" << PMCTparas->DEoptionResult;
                AppendPrimaryTrace(RMrec, "step_after_de", traceStep, payload.str());
            }

            // Adaptive step factor
            if (RLastDE > dTmp + 1e-30) {
                dT1 = RLastStepWidth / (RLastDE - dTmp);
                if (dT1 < 1.0) RSFmul = MaxCS(0.5f, static_cast<float>(dT1));
                else RSFmul = 1.0f;
            } else {
                RSFmul = 1.0f;
            }
            ++traceStep;
        } else {
            // ===== Set found =====
            RMrec->finalDE = dTmp;
            LongBool DElimited = (It3Dex->ItResultI < PMCTparas->MaxItsResult) ||
                                 (dTmp < PMCTparas->msDEstop);
            double mZZBeforeRefine = PMCTparas->mZZ;
            double dTmpBeforeRefine = dTmp;
            double lastStepBeforeRefine = RLastStepWidth;
            const char* refinePath = "none";

            if (PMCTparas->iDEAddSteps > 0) {
                PMCTparas->mZZ = RMrec->Zstepped;
                if (DElimited) {
                    refinePath = "RMdoBinSearch";
                    ApplyDebugRefineEntryOverride(PMCTparas, It3Dex, dTmp, RLastStepWidth);
                    RMdoBinSearch(PMCTparas, dTmp, RLastStepWidth);
                } else {
                    refinePath = "RMdoBinSearchIt";
                    // Pascal passes mZZ by reference. Keep the depth used by
                    // CalcDE and normals synchronized with the refined point;
                    // the common writeback below must not restore the old depth.
                    RMdoBinSearchIt(PMCTparas, &PMCTparas->mZZ);
                }
                RMrec->Zstepped = PMCTparas->mZZ;
            }
            if (PrimaryTraceActive(RMrec)) {
                int deLimitedPas = DElimited ? -1 : 0;
                std::ostringstream payload;
                payload.setf(std::ios::fixed, std::ios::floatfield);
                payload << std::setprecision(17)
                        << "DElimited=" << deLimitedPas
                        << " mZZ_before_refine=" << mZZBeforeRefine
                        << " dTmp=" << dTmpBeforeRefine
                        << " RLastStepWidth=" << lastStepBeforeRefine
                        << " iDEAddSteps=" << PMCTparas->iDEAddSteps
                        << " refine_path=" << refinePath
                        << " mZZ_after_refine=" << PMCTparas->mZZ
                        << " ActZpos=" << Clamp0D(PMCTparas->mZZ)
                        << " Zstepped=" << RMrec->Zstepped
                        << " finalDE=" << dTmp
                        << " sRoughness=" << PMCTparas->sRoughness
                        << " RMresult=" << (DElimited ? 1 : 2);
                AppendPrimaryTrace(RMrec, "hit_after_refine", static_cast<int>(RMrec->StepCount), payload.str());
            }

            PMCTparas->mZZ = RMrec->Zstepped;
            RMrec->ActZpos = Clamp0D(RMrec->ActZpos + RMrec->Zstepped * RMrec->ZZposMul);
            RMrec->StartInside = 0;
            if (DElimited) RMrec->RMresult = 1;
            else RMrec->RMresult = 2;
            RMrec->ActPos[0] = It3Dex->C1;
            RMrec->ActPos[1] = It3Dex->C2;
            RMrec->ActPos[2] = It3Dex->C3;
            return;
        }
    } while (RMrec->Zstepped <= RMrec->MaxRayLength &&
             !(PMCTparas->PCalcThreadStats && PMCTparas->PCalcThreadStats->pLBcalcStop &&
               *PMCTparas->PCalcThreadStats->pLBcalcStop));

    RMrec->RMresult = 0;
    PMCTparas->mZZ = RMrec->Zstepped;
    RMrec->ActZpos = Clamp0D(RMrec->ActZpos + RMrec->Zstepped * RMrec->ZZposMul);
}

} // namespace mb3d
