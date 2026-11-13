#include "core/render_context.h"
#include <filesystem>
#include "core/calc_monte_carlo.h"
#include "core/renderer.h"
#include "core/header_trafos.h"
#include "core/ray_marcher.h"
#include "core/shadow.h"
#include "core/ambient_occlusion.h"
#include "core/zbuffer_normals.h"
#include "core/colorizer.h"
#include "core/reflection.h"
#include "core/dof.h"
#include "core/math3d.h"
#include "core/formulas.h"
#include "core/formula_validation.h"
#include "core/parity_trace.h"
#include "core/pas_lcg.h"
#include "core/thread_pool.h"
#include "core/calc_2d.h"
#include <cmath>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <algorithm>
#include <atomic>
#include <chrono>
#include <functional>
#include <sstream>
#include <iomanip>
#if defined(_MSC_VER) && defined(_M_X64)
#include <xmmintrin.h>
#endif

namespace mb3d {

const char* RenderProfileStageName(RenderProfileStage stage) {
    static constexpr const char* names[] = {
        "preparation", "volume", "primary", "normals", "shadows", "ao", "color", "reflections", "dof"
    };
    const auto index = static_cast<size_t>(stage);
    return index < static_cast<size_t>(RenderProfileStage::Count) ? names[index] : "unknown";
}

// Only the coordinating thread writes timings, after joined stage workers.
// An unfinished scope retains elapsed work but is never labelled completed.
class StageTimer {
public:
    StageTimer(RenderProfile& profile, RenderProfileStage stage)
        : timing_(profile.enabled ? &profile.stages[static_cast<size_t>(stage)] : nullptr) {
        NoteRenderStage(stage);
        if (timing_) {
            timing_->state = "interrupted";
            start_ = std::chrono::steady_clock::now();
        }
    }
    ~StageTimer() { stop(false); }
    void complete() { stop(true); }
private:
    void stop(bool completed) {
        if (!timing_) return;
        timing_->seconds += std::chrono::duration<double>(std::chrono::steady_clock::now() - start_).count();
        timing_->state = completed ? "completed" : "interrupted";
        timing_ = nullptr;
    }
    RenderStageTiming* timing_;
    std::chrono::steady_clock::time_point start_;
};

#if defined(_MSC_VER) && defined(_M_X64)
extern "C" unsigned short GetX87ControlWord();
#endif

static inline int PascalRoundToInt(double value) {
    return static_cast<int>(std::lrint(std::min(1e9, std::max(-1e9, value))));
}

static inline int32_t DeterministicThreadSeed(int threadId) {
    const uint64_t mixed =
                           static_cast<uint64_t>(threadId) * 0x324594A1ull +
                           0x24563487ull;
    return static_cast<int32_t>(static_cast<uint32_t>(mixed & 0xFFFFFFFFu));
}

static bool ApplyDebugNormalsEntryOverride(TMCTparameter& mct, TIteration3Dext& it3d) {
    if (!HasParityTraceContext()) {
        return false;
    }
    const char* spec = std::getenv("MB3D_DEBUG_NORMALS_ENTRY_OVERRIDE");
    if (!spec || !*spec) {
        return false;
    }

    int px = 0;
    int py = 0;
    double c1 = 0.0;
    double c2 = 0.0;
    double c3 = 0.0;
    double mzz = 0.0;
    double msDEstop = 0.0;
    if (std::sscanf(spec, "%d,%d,%lf,%lf,%lf,%lf,%lf",
                    &px, &py, &c1, &c2, &c3, &mzz, &msDEstop) != 7) {
        return false;
    }
    if (px != GetParityTracePixelX() || py != GetParityTracePixelY()) {
        return false;
    }

    it3d.C1 = c1;
    it3d.C2 = c2;
    it3d.C3 = c3;
    mct.mZZ = mzz;
    mct.msDEstop = static_cast<float>(msDEstop);

    std::ostringstream payload;
    payload.setf(std::ios::fixed, std::ios::floatfield);
    payload << std::setprecision(12)
            << "C1=" << it3d.C1
            << " C2=" << it3d.C2
            << " C3=" << it3d.C3
            << " mZZ=" << mct.mZZ
            << " msDEstop=" << mct.msDEstop;
    AppendParityTrace("normals_entry_override", payload.str());
    return true;
}

static bool ApplyDebugRayReadyVecOverride(TMCTparameter& mct, int x, int y) {
    const char* spec = std::getenv("MB3D_DEBUG_RAY_READY_VEC_OVERRIDE");
    if (!spec || !*spec) {
        return false;
    }

    int px = 0;
    int py = 0;
    double vx = 0.0;
    double vy = 0.0;
    double vz = 0.0;
    if (std::sscanf(spec, "%d,%d,%lf,%lf,%lf", &px, &py, &vx, &vy, &vz) != 5) {
        return false;
    }
    if (px != x || py != y) {
        return false;
    }

    mct.mVgradsFOV[0] = vx;
    mct.mVgradsFOV[1] = vy;
    mct.mVgradsFOV[2] = vz;

    if (HasParityTraceContext()) {
        std::ostringstream payload;
        payload.setf(std::ios::fixed, std::ios::floatfield);
        payload << std::setprecision(17)
                << "vx=" << vx
                << " vy=" << vy
                << " vz=" << vz;
        AppendParityTrace("ray_ready_vec_override", payload.str());
    }
    return true;
}

static inline bool NeedsLatePassTraceInDiagOnly(const RenderOptions& opts) {
    auto mentionsLatePassStage = [](const std::string& stage) {
        return stage.compare(0, 6, "paint_") == 0 ||
               stage.compare(0, 3, "sr_") == 0;
    };
    return mentionsLatePassStage(opts.parity_trace_start_stage) ||
           mentionsLatePassStage(opts.parity_trace_end_stage);
}

static inline bool ShouldRunHardShadowPass(const TMandHeader10& header,
                                           const RenderOptions& opts) {
    return !opts.no_shadows &&
           header.bCalc3D > 0 &&
           (header.bCalculateHardShadow & 1) != 0 &&
           (header.bCalculateHardShadow & 0xFC) != 0;
}

static bool SiLightCarriesHardShadowBits(const TMandHeader10& header,
                                         const std::vector<TsiLight5>& silight) {
    uint16_t activeMask = 0;
    for (int light = 0; light < 6; ++light) {
        RenderPoll();
        if ((header.bCalculateHardShadow & (4 << light)) != 0) {
            activeMask |= static_cast<uint16_t>(0x400 << light);
        }
    }
    if (activeMask == 0) {
        return false;
    }
    for (const auto& psi : silight) {
        RenderPoll();
        if ((psi.Shadow & activeMask) != 0) {
            return true;
        }
    }
    return false;
}

static uint8_t BuildCompletedHardShadowState(const TMandHeader10& header) {
    const int hsBaseState = header.bHScalculated;
    if ((header.bCalc1HSsoft & 1) != 0) {
        return static_cast<uint8_t>((hsBaseState & 1) | header.bCalculateHardShadow);
    }
    return static_cast<uint8_t>((hsBaseState & 0xFD) | header.bCalculateHardShadow);
}

static void ApplyCompletedHardShadowStateToPaint(const TMandHeader10& header,
                                                 TLightVals& lightVals) {
    const uint8_t completedState = BuildCompletedHardShadowState(header);
    for (int x = 0; x < 6; ++x) {
        RenderPoll();
        const int y = lightVals.SortTab[x];
        lightVals.iHScalced[y] = (completedState >> (x + 2)) & 1;
        lightVals.iHSmask[y] = 0x400 << x;
        if ((header.bCalc1HSsoft & 1) != 0 && lightVals.iHScalced[y] != 0) {
            lightVals.iHSmask[y] = -1;
        }
        if ((completedState & 2) != 0 && ((header.bCalculateHardShadow >> (x + 2)) & 1) != 0) {
            lightVals.iLightFuncDiff[y] = 0;
        }
    }
}

static bool DebugDisablePaintReplayCompletedHardShadowState() {
    const char* spec = std::getenv("MB3D_DEBUG_DISABLE_REPLAY_HS_STATE");
    return spec && *spec && std::strcmp(spec, "0") != 0;
}

// Helper: distribute row ranges across a thread pool.
// work_fn(y_start, y_end) processes rows [y_start, y_end).
static void ParallelRowRange(ThreadPool& pool, int height, int batch_size,
                              std::function<void(int y_start, int y_end)> work_fn) {
    std::vector<std::future<void>> tasks;
    tasks.reserve((height + batch_size - 1) / batch_size);
    try {
        for (int y = 0; y < height; y += batch_size) {
            RenderPoll();
            int y_end = std::min(y + batch_size, height);
            tasks.push_back(pool.submit([work_fn, y, y_end]() {
                work_fn(y, y_end);
            }));
        }
    } catch (...) {
        // A failed submission must not leave workers using caller-owned data
        // after that data has been destroyed during stack unwinding.
        FailRender(std::current_exception());
        pool.wait_all();
        throw;
    }
    pool.wait_all();
    RenderPoll();
    for (auto& task : tasks) task.get();
}

static void RunColorPass(ThreadPool& pool,
                         int numThreads,
                         int batchSize,
                         TMandHeader10& header,
                         const RenderOptions& opts,
                         const ParityTraceStageRangeStateHandle& parityTraceStageState,
                         TLightVals& lightVals,
                         RenderResult& result,
                         int width,
                         int height,
                         bool hasParityPoint,
                         const DiagnosticPoint& parityPoint,
                         RenderProfile& profile) {
    if (opts.verbose) {
        std::fprintf(stderr, "Starting color pass...\n");
    }
    StageTimer colorTimer(profile, RenderProfileStage::Color);
    auto _cp_t0 = std::chrono::steady_clock::now();

    TPaintParameter paintParams;
    std::memset(&paintParams, 0, sizeof(TPaintParameter));
    paintParams.ppWidth = header.Width;
    paintParams.ppHeight = header.Height;
    paintParams.ppYinc = 1;
    paintParams.PLVals = &lightVals;
    TMatrix3 paintVGrads;

    CalcStepWidth(&header);
    paintParams.StepWidth = header.dStepWidth;
    TMatrix3 sourceVGrads{};
    std::memcpy(&sourceVGrads, static_cast<const void*>(header.hVGrads), sizeof(sourceVGrads));
    NormaliseMatrixTo(paintVGrads, header.dStepWidth, sourceVGrads);
    paintParams.pVgrads = &paintVGrads;

    if (header.bPlanarOptic == 2)
        paintParams.sFOVy = static_cast<float>(Pi);
    else
        paintParams.sFOVy = static_cast<float>(header.dFOVy * Pid180);

    CalcPPZvals(header, paintParams.Zcorr, paintParams.ZcMul, paintParams.ZZstmitDif);

    paintParams.ppPlanarOptic = header.bPlanarOptic & 3;
    double d_fov = std::min(1.5, std::max(0.001, static_cast<double>(paintParams.sFOVy) * 0.5));
    paintParams.ppPlOpticZ = static_cast<float>(std::cos(d_fov) * d_fov / std::sin(d_fov));
    paintParams.ppXOff = CalcXoff(&header);
    // Compute tiling offsets from header (no-op when TilingOptions == 0)
    {
        int tileW = 0, tileH = 0, xPlus = 0, yPlus = 0;
        GetPaintTileSizes(&header, tileW, tileH, xPlus, yPlus);
        paintParams.ppPaintWidth = tileW;
        paintParams.ppPaintHeight = tileH;
        paintParams.ppXplus = xPlus;
        paintParams.ppYplus = yPlus;
    }

    TPaintLightVals plvTemplate;
    std::memset(&plvTemplate, 0, sizeof(TPaintLightVals));
    TSVec SPosYstart, SPosYadd, SPosXadd;
    GetStartSPosAndAddVecs(plvTemplate, paintParams, SPosYstart, SPosYadd, SPosXadd);

    float aspect;
    if (paintParams.ppPlanarOptic == 2)
        aspect = 2.0f;
    else
        aspect = static_cast<float>(header.Width) / static_cast<float>(header.Height);

    std::atomic<int> completedRows{0};

    const int ppXplus = paintParams.ppXplus;
    const int ppYplus = paintParams.ppYplus;

    ParallelRowRange(pool, height, batchSize,
        [&](int y_start, int y_end) {
            TPaintLightVals plv = plvTemplate;
#ifdef MB3D_ENABLE_PARITY_TRACE
            if (parityTraceStageState &&
                hasParityPoint &&
                parityPoint.y >= y_start &&
                parityPoint.y < y_end) {
                RestoreParityTraceStageRangeState(parityTraceStageState);
            }
#endif

            for (int y = y_start; y < y_end; y++) {
                RenderPoll();
                TSVec SPosY;
                float yOff = static_cast<float>(ppYplus + y);
                SPosY[0] = SPosYstart[0] + SPosYadd[0] * yOff;
                SPosY[1] = SPosYstart[1] + SPosYadd[1] * yOff;
                SPosY[2] = SPosYstart[2] + SPosYadd[2] * yOff;
                SPosY[3] = 0;

                plv.yPos = static_cast<float>(y + ppYplus) / header.Height;
                PreCalcDepthCol(lightVals.iDfunc, &plv, lightVals.PLValigned);

                for (int x = 0; x < width; x++) {
                    RenderPoll();
                    int idx = y * width + x;
                    TsiLight5* psi = &result.silight[idx];
                    const bool tracePaintPixel = hasParityPoint &&
                                                 x == parityPoint.x &&
                                                 y == parityPoint.y;

                    TSVec SPosX;
                    if (paintParams.ppPlanarOptic == 2) {
                        std::memcpy(SPosX, SPosYstart, sizeof(TSVec));
                    } else {
                        float xOff = static_cast<float>(ppXplus + x);
                        SPosX[0] = SPosY[0] + SPosXadd[0] * xOff;
                        SPosX[1] = SPosY[1] + SPosXadd[1] * xOff;
                        SPosX[2] = SPosY[2] + SPosXadd[2] * xOff;
                        SPosX[3] = 0;
                    }

                    plv.xPos = static_cast<float>(x + 1 + ppXplus) / header.Width;
                    CalcViewVec(&plv, aspect);
                    CalcObjPos(plv, paintParams, psi, SPosX);

                    uint32_t pixelColor = 0;
                    if (tracePaintPixel) {
                        SetParityTraceContext(x, y, &result.parity_trace_lines);
                        uint32_t packedZRough = 0;
                        std::memcpy(&packedZRough, &psi->RoughZposFine, sizeof(uint32_t));
                        std::ostringstream payload;
                        payload.setf(std::ios::fixed, std::ios::floatfield);
                        payload << std::setprecision(12)
                                << "mode=" << (lightVals.bCalcPixColSqr ? "CalcPixelColorSqr" : "CalcPixelColor2")
                                << " Zpos=" << psi->Zpos
                                << " RoughZposFine=" << packedZRough
                                << " Shadow=" << psi->Shadow
                                << " AmbShadow=" << psi->AmbShadow
                                << " SIgradient=" << psi->SIgradient
                                << " OTrap=" << psi->OTrap
                                << " xPos=" << plv.xPos
                                << " yPos=" << plv.yPos
                                << " zPos=" << plv.zPos
                                << " ViewVecX=" << plv.ViewVec[0]
                                << " ViewVecY=" << plv.ViewVec[1]
                                << " ViewVecZ=" << plv.ViewVec[2]
                                << " AbsViewVecX=" << plv.AbsViewVec[0]
                                << " AbsViewVecY=" << plv.AbsViewVec[1]
                                << " AbsViewVecZ=" << plv.AbsViewVec[2]
                                << " CamPosX=" << plv.CamPos[0]
                                << " CamPosY=" << plv.CamPos[1]
                                << " CamPosZ=" << plv.CamPos[2]
                                << " ObjPosX=" << plv.ObjPos[0]
                                << " ObjPosY=" << plv.ObjPos[1]
                                << " ObjPosZ=" << plv.ObjPos[2]
                                << " PreDepthColX=" << plv.PreDepthCol[0]
                                << " PreDepthColY=" << plv.PreDepthCol[1]
                                << " PreDepthColZ=" << plv.PreDepthCol[2]
                                << " sAmbShad=" << lightVals.sAmbShad
                                << " sDiffuseShadowing=" << lightVals.sDiffuseShadowing
                                << " sIndLightReflect=" << lightVals.sIndLightReflect
                                << " sDiff=" << lightVals.sDiff
                                << " sSpec=" << lightVals.sSpec
                                << " sDepth=" << lightVals.sDepth
                                << " sDynFogMul=" << lightVals.sDynFogMul
                                << " sShad=" << lightVals.sShad
                                << " sShadZmul=" << lightVals.sShadZmul
                                << " sShadGr=" << lightVals.sShadGr
                                << " sGamma=" << lightVals.sGamma
                                << " iExModes=" << lightVals.iExModes
                                << " iGammaH=" << lightVals.iGammaH
                                << " bUseSmallBGpicForAmb=" << (lightVals.bUseSmallBGpicForAmb ? 1 : 0)
                                << " bBackBMP=" << (lightVals.bBackBMP ? 1 : 0)
                                << " bAddBGlight=" << (lightVals.bAddBGlight ? 1 : 0)
                                << " bDirectImageCoord=" << (lightVals.bDirectImageCoord ? 1 : 0)
                                << " bFarFog=" << (lightVals.bFarFog ? -1 : 0)
                                << " bVolLight=" << (lightVals.bVolLight ? 1 : 0)
                                << " bAmbRelObj=" << (lightVals.bAmbRelObj ? 1 : 0)
                                << " LightOpt0=" << lightVals.iLightOption[0]
                                << " LightPos0=" << lightVals.iLightPos[0]
                                << " LmaxL0=" << lightVals.sLmaxL[0]
                                << " PosLP0=" << lightVals.sPosLP[0]
                                << " PosLightZ0=" << lightVals.sPosLightZpos[0]
                                << " LightFunc0=" << lightVals.iLightFuncDiff[0]
                                << " LN0X=" << lightVals.PLValigned->LN[0][0]
                                << " LN0Y=" << lightVals.PLValigned->LN[0][1]
                                << " LN0Z=" << lightVals.PLValigned->LN[0][2]
                                << " LCol0X=" << lightVals.PLValigned->sLCols[0][0]
                                << " LCol0Y=" << lightVals.PLValigned->sLCols[0][1]
                                << " LCol0Z=" << lightVals.PLValigned->sLCols[0][2]
                                << " LightOpt1=" << lightVals.iLightOption[1]
                                << " LightPos1=" << lightVals.iLightPos[1]
                                << " LmaxL1=" << lightVals.sLmaxL[1]
                                << " PosLP1=" << lightVals.sPosLP[1]
                                << " PosLightZ1=" << lightVals.sPosLightZpos[1]
                                << " LightFunc1=" << lightVals.iLightFuncDiff[1]
                                << " LN1X=" << lightVals.PLValigned->LN[1][0]
                                << " LN1Y=" << lightVals.PLValigned->LN[1][1]
                                << " LN1Z=" << lightVals.PLValigned->LN[1][2]
                                << " LCol1X=" << lightVals.PLValigned->sLCols[1][0]
                                << " LCol1Y=" << lightVals.PLValigned->sLCols[1][1]
                                << " LCol1Z=" << lightVals.PLValigned->sLCols[1][2]
                                << " LightOpt2=" << lightVals.iLightOption[2]
                                << " LightPos2=" << lightVals.iLightPos[2]
                                << " LmaxL2=" << lightVals.sLmaxL[2]
                                << " PosLP2=" << lightVals.sPosLP[2]
                                << " PosLightZ2=" << lightVals.sPosLightZpos[2]
                                << " LightFunc2=" << lightVals.iLightFuncDiff[2]
                                << " LN2X=" << lightVals.PLValigned->LN[2][0]
                                << " LN2Y=" << lightVals.PLValigned->LN[2][1]
                                << " LN2Z=" << lightVals.PLValigned->LN[2][2]
                                << " LCol2X=" << lightVals.PLValigned->sLCols[2][0]
                                << " LCol2Y=" << lightVals.PLValigned->sLCols[2][1]
                                << " LCol2Z=" << lightVals.PLValigned->sLCols[2][2]
                                << " LightOpt3=" << lightVals.iLightOption[3]
                                << " LightPos3=" << lightVals.iLightPos[3]
                                << " LmaxL3=" << lightVals.sLmaxL[3]
                                << " PosLP3=" << lightVals.sPosLP[3]
                                << " PosLightZ3=" << lightVals.sPosLightZpos[3]
                                << " LN3X=" << lightVals.PLValigned->LN[3][0]
                                << " LN3Y=" << lightVals.PLValigned->LN[3][1]
                                << " LN3Z=" << lightVals.PLValigned->LN[3][2]
                                << " LightOpt4=" << lightVals.iLightOption[4]
                                << " LightPos4=" << lightVals.iLightPos[4]
                                << " LmaxL4=" << lightVals.sLmaxL[4]
                                << " PosLP4=" << lightVals.sPosLP[4]
                                << " PosLightZ4=" << lightVals.sPosLightZpos[4]
                                << " LN4X=" << lightVals.PLValigned->LN[4][0]
                                << " LN4Y=" << lightVals.PLValigned->LN[4][1]
                                << " LN4Z=" << lightVals.PLValigned->LN[4][2]
                                << " LightOpt5=" << lightVals.iLightOption[5]
                                << " LightPos5=" << lightVals.iLightPos[5]
                                << " LmaxL5=" << lightVals.sLmaxL[5]
                                << " PosLP5=" << lightVals.sPosLP[5]
                                << " PosLightZ5=" << lightVals.sPosLightZpos[5]
                                << " LN5X=" << lightVals.PLValigned->LN[5][0]
                                << " LN5Y=" << lightVals.PLValigned->LN[5][1]
                                << " LN5Z=" << lightVals.PLValigned->LN[5][2];
                        AppendParityTrace("paint_pre", payload.str());
                    }
                    if (lightVals.bCalcPixColSqr)
                        CalcPixelColorSqr(&pixelColor, psi, &lightVals, &plv);
                    else
                        CalcPixelColor2(&pixelColor, psi, &lightVals, &plv);

                    if (tracePaintPixel) {
                        SetParityTraceContext(x, y, &result.parity_trace_lines);
                        std::ostringstream payload;
                        payload << "mode=" << (lightVals.bCalcPixColSqr ? "CalcPixelColorSqr" : "CalcPixelColor2")
                                << " pixelColor=" << pixelColor
                                << " R=" << ((pixelColor >> 16) & 0xFF)
                                << " G=" << ((pixelColor >> 8) & 0xFF)
                                << " B=" << (pixelColor & 0xFF);
                        AppendParityTrace("paint_result", payload.str());
                        ClearParityTraceContext();
                    }

                    result.image[idx * 3 + 0] = (pixelColor >> 16) & 0xFF;
                    result.image[idx * 3 + 1] = (pixelColor >> 8) & 0xFF;
                    result.image[idx * 3 + 2] = pixelColor & 0xFF;
                }

                if (opts.verbose) {
                    int done = completedRows.fetch_add(1) + 1;
                    if (done % 50 == 0) {
                        std::fprintf(stderr, "  Coloring: %d/%d rows\n", done, height);
                    }
                }
            }
        }
    );

    if (opts.verbose) {
        auto _cp_t1 = std::chrono::steady_clock::now();
        double _cp_s = std::chrono::duration<double>(_cp_t1 - _cp_t0).count();
        std::fprintf(stderr, "Color pass complete.  [%.2fs]\n", _cp_s);
    }
    colorTimer.complete();
}

static void RunLateImagePasses(ThreadPool& pool,
                               int numThreads,
                               int batchSize,
                               TMandHeader10& header,
                               const RenderOptions& opts,
                               THeaderCustomAddon& addon,
                               ParityTraceStageRangeStateHandle parityTraceStageState,
                               TMCTparameter& mctParams,
                               TLightVals& lightVals,
                               TLValigned& alignedStorage,
                               RenderResult& result,
                               int width,
                               int height,
                               bool hasParityPoint,
                               const DiagnosticPoint& parityPoint,
                               RenderProfile& profile) {
    RenderPoll();
    if (!opts.no_reflections && (header.bCalcSRautomatic & 1)) {
        StageTimer reflectionTimer(profile, RenderProfileStage::Reflections);
        if (opts.verbose) {
            std::fprintf(stderr, "Starting reflection/transparency pass...\n");
            std::fprintf(stderr, "  SRamount: %g, SRreflectioncount: %d\n",
                        static_cast<double>(header.SRamount),
                        static_cast<int>(header.SRreflectioncount));
            std::fprintf(stderr, "  bCalcSRautomatic: 0x%02X\n", header.bCalcSRautomatic);
            if (header.bCalcSRautomatic & 2)
                std::fprintf(stderr, "  Transmission enabled (sTRIndex=%g, sTRscattering=%g)\n",
                            static_cast<double>(header.sTRIndex),
                            static_cast<double>(header.sTRscattering));
        }

        lightVals.SRLightAmount = Min0MaxCS(header.SRamount, 100.0f);
        lightVals.sStepWidth = static_cast<float>(header.dStepWidth);

        // Precompute the volume light map ONCE on the main thread before any
        // parallel reflection worker is spawned. Mirrors Pascal CalcSR.pas:107
        // (CalcVolLightMap is invoked on the main thread just before the
        // TSRCalcThread workers start). Preparation mutates this job's vector
        // storage, so it must finish before reflection workers sample it.
        // The function is a no-op when vol-light is
        // disabled for this scene (bVolLightNr == 0 or the light is disabled).
        if ((header.bVolLightNr & 7) != 0) {
            lightVals.PLValigned = &alignedStorage;
            mctParams.PLVals = &lightVals;
            CalcVolLightMap(&header, &lightVals, &mctParams, opts.thread_count);
        }

        if (opts.diag_only &&
            hasParityPoint &&
            parityPoint.x >= 0 && parityPoint.x < width &&
            parityPoint.y >= 0 && parityPoint.y < height) {
            TMCTparameter localMct = mctParams;
            TLightVals localLV = lightVals;
            TLValigned localAligned = alignedStorage;
            localLV.PLValigned = &localAligned;
            localMct.mPsiLight = result.silight.data();
            localMct.pSiLight = result.silight.data();
            localMct.PLVals = &localLV;
            SetParityTraceContext(parityPoint.x, parityPoint.y, &result.parity_trace_lines);
            CalcReflectionsRows(result.silight.data(), result.image.data(),
                                width, height, width,
                                &localMct, &localLV, &header, &addon,
                                parityPoint.y, parityPoint.y + 1,
                                parityPoint.x, parityPoint.x + 1);
            ClearParityTraceContext();
        } else if (numThreads <= 1) {
            if (hasParityPoint && parityPoint.x >= 0 && parityPoint.x < width &&
                parityPoint.y >= 0 && parityPoint.y < height) {
                SetParityTraceContext(parityPoint.x, parityPoint.y, &result.parity_trace_lines);
            }
            CalcReflections(result.silight.data(), result.image.data(), width, height,
                            width, &mctParams, &lightVals, &header, &addon);
            if (hasParityPoint && parityPoint.x >= 0 && parityPoint.x < width &&
                parityPoint.y >= 0 && parityPoint.y < height) {
                ClearParityTraceContext();
            }
        } else {
            ParallelRowRange(pool, height, batchSize,
                [&](int y_start, int y_end) {
                    TMCTparameter localMct = mctParams;
                    TLightVals localLV = lightVals;
                    TLValigned localAligned = alignedStorage;
                    localLV.PLValigned = &localAligned;
                    localMct.mPsiLight = result.silight.data();
                    localMct.pSiLight = result.silight.data();
                    localMct.PLVals = &localLV;
                    const bool tracesPoint = hasParityPoint && parityPoint.y >= y_start &&
                        parityPoint.y < y_end && parityPoint.x >= 0 && parityPoint.x < width;
                    if (tracesPoint) {
                        RestoreParityTraceStageRangeState(parityTraceStageState);
                        SetParityTraceContext(parityPoint.x, parityPoint.y, &result.parity_trace_lines);
                    }
                    CalcReflectionsRows(result.silight.data(), result.image.data(),
                                        width, height, width,
                                        &localMct, &localLV, &header, &addon,
                                        y_start, y_end);
                    if (tracesPoint) ClearParityTraceContext();
                }
            );
        }

        if (opts.verbose) {
            std::fprintf(stderr, "Reflection/transparency pass complete.\n");
        }
        reflectionTimer.complete();
    }

    RenderPoll();
    if (!opts.no_dof && (header.bCalcDOFtype & 1) != 0) {
        StageTimer dofTimer(profile, RenderProfileStage::DOF);
        if (opts.verbose) {
            std::fprintf(stderr, "Starting depth of field pass...\n");
            std::fprintf(stderr, "  DOFtype: 0x%02X, Zsharp: %g, Zsharp2: %g\n",
                        header.bCalcDOFtype,
                        static_cast<double>(header.sDOFZsharp),
                        static_cast<double>(header.sDOFZsharp2));
            std::fprintf(stderr, "  Aperture: %g, ClipR: %g\n",
                        static_cast<double>(header.sDOFaperture),
                        static_cast<double>(header.sDOFclipR));
        }

        // Pascal's DOF always processes the full-frame-sized prefix with a
        // full-frame stride, even when the allocation is an overlapping tile.
        // ValidateOrdinaryTileStages rejects only undersized allocations.
        CalcDepthOfField(result.image.data(), result.silight.data(), header.Width, header.Height,
                         header.Width, &mctParams, &header);

        if (opts.verbose) {
            std::fprintf(stderr, "Depth of field pass complete.\n");
        }
        dofTimer.complete();
    }
}

static void ValidateOrdinaryTileStages(const TMandHeader10& header, const RenderOptions& opts,
                                       bool paintReplay) {
    if (header.TilingOptions == 0) return;
    if ((header.TilingOptions & 0x7f) == 0 || ((header.TilingOptions >> 7) & 0x7f) == 0)
        throw std::invalid_argument("Invalid tile count: horizontal and vertical counts must be positive");
    if (!paintReplay && header.bCalc3D == 0) return;
    // Pascal allocates tile-sized buffers, but these image-space postprocessors
    // index them using the full header Width/Height (Mand/DOF/ImageProcess/AmbHiQ).
    // A sufficiently large tile (e.g. 1x1 plus overlap) has a defined prefix
    // result. Reject only the cases that would overrun the actual allocation.
    int tileWidth = 0, tileHeight = 0, tileX = 0, tileY = 0;
    GetPaintTileSizes(&header, tileWidth, tileHeight, tileX, tileY);
    if (static_cast<int64_t>(tileWidth) * tileHeight >=
        static_cast<int64_t>(header.Width) * header.Height) return;
    RenderPoll();
    if (!opts.no_dof && (header.bCalcDOFtype & 1) != 0)
        throw std::invalid_argument("Unsupported DOF with tiling: Pascal indexes tile buffers as a full frame");
    if (!paintReplay && !opts.no_ao && (header.bCalcAmbShadowAutomatic & 1) != 0 &&
        (header.bCalcAmbShadowAutomatic & 12) != 12)
        throw std::invalid_argument("Unsupported screen-space AO with tiling: Pascal indexes tile buffers as a full frame");
}

static RenderResult RenderOrdinary(TMandHeader10& header, THeaderCustomAddon& addon, const RenderOptions& opts,
                                   RenderProfile& profile) {
    StageTimer preparationTimer(profile, RenderProfileStage::Preparation);
    RenderResult result;
    result.success = false;

    RenderPoll();
    // Initialize this job's volume state before any stage workers are started.
    ResetVolLightMap();

    if (opts.deterministic_mode) {
        // Parity-only mode: keep primary and AO first-step jitter disabled for reproducible tests.
        header.iOptions &= ~1;
        header.bCalcAmbShadowAutomatic &= static_cast<uint8_t>(~128u);
    }

    // Apply size overrides
    int width = (opts.width_override > 0) ? opts.width_override : header.Width;
    int height = (opts.height_override > 0) ? opts.height_override : header.Height;
    if (width <= 0 || height <= 0 || width > 16384 || height > 16384) {
        result.error = "Invalid image dimensions";
        return result;
    }
    header.Width = width;
    header.Height = height;
    ValidateOrdinaryTileStages(header, opts, false);
    // The header keeps the full-frame camera dimensions. Tile buffers include
    // their overlap border and use local indices; CalcRect retains their origin.
    int tileX = 0, tileY = 0;
    GetPaintTileSizes(&header, width, height, tileX, tileY);
    if (width <= 0 || height <= 0 || width > 16384 || height > 16384) {
        result.error = "Invalid tile dimensions";
        return result;
    }
    result.width = width;
    result.height = height;
    result.tiling_crop = GetTilingCrop(&header);

    std::vector<int> diagSlotByPixel;
    DiagnosticPoint parityPoint{};
    bool hasParityPoint = !opts.parity_trace_dump_path.empty() && !opts.diag_points.empty();
    if (hasParityPoint) {
        parityPoint = opts.diag_points.front();
    }
#ifdef MB3D_ENABLE_PARITY_TRACE
    ParityTraceStageRangeStateHandle parityTraceStageState = opts.parity_trace_stage_state;
    if (hasParityPoint) {
        if (parityTraceStageState) {
            RestoreParityTraceStageRangeState(parityTraceStageState);
        } else {
            ResetParityTraceStageRange();
            parityTraceStageState = CaptureParityTraceStageRangeState();
        }
    }
#else
    ParityTraceStageRangeStateHandle parityTraceStageState;
#endif
    if (!opts.diag_points.empty()) {
        result.diagnostics.resize(opts.diag_points.size());
        diagSlotByPixel.assign(static_cast<size_t>(width) * height, -1);
        for (size_t pointIdx = 0; pointIdx < opts.diag_points.size(); pointIdx++) {
            RenderPoll();
            const auto& point = opts.diag_points[pointIdx];
            auto& record = result.diagnostics[pointIdx];
            record.x = point.x;
            record.y = point.y;
            record.in_bounds = point.x >= 0 && point.x < width && point.y >= 0 && point.y < height;
            if (record.in_bounds) {
                diagSlotByPixel[static_cast<size_t>(point.y) * width + point.x] = static_cast<int>(pointIdx);
            }
        }
    }

    // Determine thread count
    int numThreads = opts.thread_count;
    if (numThreads <= 0) {
        numThreads = static_cast<int>(std::thread::hardware_concurrency());
        if (numThreads <= 0) numThreads = 4;
    }
    numThreads = std::max(1, std::min(numThreads, height));
    if (profile.enabled) {
        profile.width = width;
        profile.height = height;
        profile.workers = numThreads;
    }

    // Calculate batch size for load balancing
    int batchSize = std::max(1, height / (numThreads * 4));

    if (opts.verbose) {
        std::fprintf(stderr, "Image size: %d x %d\n", width, height);
        std::fprintf(stderr, "Thread count: %d, batch size: %d rows\n", numThreads, batchSize);
    }

    std::vector<std::future<void>> primaryTasks;

    // Step 1: Setup MCT parameters from header
    if (hasParityPoint) {
        SetParityTraceContext(-1, -1, &result.parity_trace_lines);
    }
    TMCTparameter mctParams = GetMCTparasFromHeader(header, &addon, 0);
    if (hasParityPoint) {
        std::ostringstream payload;
        payload.setf(std::ios::fixed, std::ios::floatfield);
        payload << std::setprecision(12)
                << "bMCTisValid=" << static_cast<int>(mctParams.bMCTisValid)
                << " FormulaType=" << mctParams.FormulaType
                << " DEoption=" << mctParams.DEoption
                << " DEoption2=" << mctParams.DEoption2
                << " CalcDE=" << ParityCalcDEName(reinterpret_cast<void*>(mctParams.CalcDE))
                << " mMandFunction=" << ParityMandFunctionName(reinterpret_cast<void*>(mctParams.mMandFunction))
                << " mMandFunctionDE=" << ParityMandFunctionDEName(reinterpret_cast<void*>(mctParams.mMandFunctionDE))
                << " StepWidth=" << mctParams.StepWidth
                << " Zend=" << mctParams.Zend
                << " msDEstop=" << mctParams.msDEstop
                << " DEstop=" << mctParams.DEstop
                << " mctDEstopFactor=" << mctParams.mctDEstopFactor
                << " mctMH04ZSD=" << mctParams.mctMH04ZSD
                << " dDEscale=" << mctParams.dDEscale
                << " dDEscale2=" << mctParams.dDEscale2
                << " dColPlus=" << mctParams.dColPlus
                << " mctColVarDEstopMul=" << mctParams.mctColVarDEstopMul
                << " wEndTo=" << static_cast<int>(mctParams.wEndTo)
                << " RepeatFrom1=" << mctParams.RepeatFrom1
                << " StartFrom2=" << mctParams.StartFrom2
                << " iEnd2=" << mctParams.iEnd2
                << " RepeatFrom2=" << mctParams.RepeatFrom2;
        for (int i = 0; i < 6; ++i) {
            RenderPoll();
            const bool activeSlot = mctParams.nHybrid[i] != 0;
            payload << " nHybrid" << i << "=" << mctParams.nHybrid[i]
                    << " fHPVar" << i << "=" << (activeSlot ? ParityPointerState(mctParams.fHPVar[i]) : "inactive")
                    << " fHybrid" << i << "=" << (activeSlot ? ParityHybridName(reinterpret_cast<void*>(mctParams.fHybrid[i])) : "inactive")
                    << " fHln" << i << "=" << mctParams.fHln[i];
        }
        AppendParityTrace("mct_ready", payload.str());
        ClearParityTraceContext();
    }
    for (const auto& issue : ValidateFormulaInputs(mctParams, addon)) {
        RenderPoll();
        throw std::invalid_argument(issue.message);
    }
    if (!mctParams.bMCTisValid) {
        result.error = "Failed to setup MCT parameters (invalid formulas?)";
        return result;
    }
    ThreadPool pool(numThreads);

    if (opts.verbose) {
        std::fprintf(stderr, "MCT parameters set up successfully\n");
        std::fprintf(stderr, "  StepWidth: %g\n", mctParams.StepWidth);
        std::fprintf(stderr, "  Zend: %g\n", mctParams.Zend);
        std::fprintf(stderr, "  FOVy: %g\n", mctParams.FOVy);
        std::fprintf(stderr, "  DEstop: %g\n", static_cast<double>(mctParams.DEstop));
        std::fprintf(stderr, "  bCalcInside: %d\n", mctParams.bCalcInside);
    }

    // Step 2: Setup light values
    TLightVals lightVals;
    std::memset(&lightVals, 0, sizeof(TLightVals));

    // Allocate aligned storage
    TLValigned alignedStorage;
    std::memset(&alignedStorage, 0, sizeof(TLValigned));
    lightVals.PLValigned = &alignedStorage;

    MakeLightValsFromHeaderLight(&header, &lightVals, 1.0, 0);
    CompleteLightValsSetup(&header, &lightVals);

    // Calculate hard shadow vectors
    CalcHSVecsFromLights(&lightVals, &mctParams);

    if (opts.verbose) {
        std::fprintf(stderr, "Light values set up successfully\n");
        std::fprintf(stderr, "  sDiff: %g, sSpec: %g\n",
                    static_cast<double>(lightVals.sDiff), static_cast<double>(lightVals.sSpec));
        std::fprintf(stderr, "  sDepth: %g, sShad: %g\n",
                    static_cast<double>(lightVals.sDepth), static_cast<double>(lightVals.sShad));
    }

    // Step 3: Allocate buffers
    int totalPixels = width * height;
    result.image.resize(totalPixels * 3, 0); // RGB
    result.silight.resize(totalPixels);
    std::memset(result.silight.data(), 0, totalPixels * sizeof(TsiLight5));
    if (!opts.ray_setup_dump_path.empty()) {
        result.ray_setup_records.resize(totalPixels);
    }

    // Setup iteration state (for single-threaded fallback and initialization)
    TIteration3Dext it3Dext;
    std::memset(&it3Dext, 0, sizeof(TIteration3Dext));
    mctParams.pIt3Dext = &it3Dext;

    // Initialize iteration parameters
    IniIt3D(&mctParams, &it3Dext);

    // Point mPsiLight to our buffer
    mctParams.mPsiLight = result.silight.data();
    mctParams.pSiLight = result.silight.data();
    mctParams.PLVals = &lightVals;

    preparationTimer.complete();

    // Volume light map: must be ready before the primary ray march because
    // TMandCalcThread.Execute / DoDynFog (Pascal CalcThread.pas) calls
    // VolLightMapPos / GetVolLightMapVec inside its fog-substep loop when
    // DFogOnIt == 65535 (i.e. (bVolLightNr & 7) != 0). In the Pascal GUI
    // flow the map is precomputed in Mand.pas:1622-1641 before CalcMandT
    // spawns ray-march workers. The patched headless Pascal oracle follows
    // that same path; compute the map before primary rendering here too.
    if ((header.bVolLightNr & 7) != 0) {
        if (opts.verbose) {
            std::fprintf(stderr, "Precomputing volume light map (bVolLightNr=0x%02X)...\n",
                         header.bVolLightNr);
        }
        StageTimer volumeTimer(profile, RenderProfileStage::Volume);
        CalcVolLightMap(&header, &lightVals, &mctParams, numThreads);
        volumeTimer.complete();
        if (opts.verbose) {
            std::fprintf(stderr, "Volume light map ready.\n");
        }
    }

    if (opts.verbose) {
        std::fprintf(stderr, "Buffers allocated. Starting ray march pass...\n");
    }

    StageTimer primaryTimer(profile, RenderProfileStage::Primary);
    auto _rm_t0 = std::chrono::steady_clock::now();

    // Determine normal calculation function
    TCalculateNormalsFunc calcNormals;
    if (mctParams.NormalsOnDE) {
        calcNormals = RMCalculateNormals;
    } else {
        calcNormals = RMCalculateNormalsOnSmoothIt;
    }

    // ========================================================================
    // Pass 1: Ray marching or 2D slice (parallelized by row ranges)
    // ========================================================================
    std::atomic<int> hitCount{0};
    std::atomic<int> completedRows{0};

    // Check for 2D slice mode (bCalc3D == 0)
    if (!mctParams.calc3D) {
        if (opts.verbose) {
            std::fprintf(stderr, "2D slice mode active (bCalc3D=0). Running 2D slice calculation...\n");
        }

        ParallelRowRange(pool, height, batchSize,
            [&](int y_start, int y_end) {
                TMCTparameter localMct = mctParams;
                TIteration3Dext localIt3D;
                std::memset(&localIt3D, 0, sizeof(TIteration3Dext));
                localMct.pIt3Dext = &localIt3D;
                IniIt3D(&localMct, &localIt3D);

                Calc2DSliceRows(result.silight.data(), width, height, width, &localMct, y_start, y_end);

                if (opts.verbose) {
                    int done = completedRows.fetch_add(y_end - y_start) + (y_end - y_start);
                    if (done % 50 == 0 || done >= height) {
                        std::fprintf(stderr, "  2D slice: %d/%d rows\n", done, height);
                    }
                }
            }
        );

        if (opts.verbose) {
            std::fprintf(stderr, "2D slice calculation complete.\n");
        }

        primaryTimer.complete();
        // Skip 3D-specific passes (hard shadows, AO) and go straight to coloring
        goto color_pass;
    }

    primaryTasks.reserve(numThreads);
    try {
    for (int threadIndex = 0; threadIndex < numThreads; threadIndex++) {
        RenderPoll();
        primaryTasks.push_back(pool.submit([&, threadIndex]() {
            // Parity path: same logical thread ids and interleaved row ownership as Pascal CalcThread.pas.
            TMCTparameter localMct = mctParams;
            localMct.iThreadId = threadIndex + 1;
            localMct.iThreadCount = numThreads;
#ifdef MB3D_ENABLE_PARITY_TRACE
            const bool parityThread = parityTraceStageState &&
                hasParityPoint &&
                parityPoint.y >= 0 &&
                parityPoint.y < height &&
                (parityPoint.y % numThreads) == threadIndex;
            if (parityThread) {
                RestoreParityTraceStageRangeState(parityTraceStageState);
            }
#endif
            TIteration3Dext localIt3D;
            std::memset(&localIt3D, 0, sizeof(TIteration3Dext));
            localMct.pIt3Dext = &localIt3D;
            if (hasParityPoint && parityPoint.y >= 0 && parityPoint.y < height &&
                (parityPoint.y % numThreads) == threadIndex) {
                SetParityTraceContext(parityPoint.x, parityPoint.y, &result.parity_trace_lines);
                IniIt3D(&localMct, &localIt3D);
                ClearParityTraceContext();
            } else {
                IniIt3D(&localMct, &localIt3D);
            }

            TRaymarchRec rmRec;
            std::memset(&rmRec, 0, sizeof(TRaymarchRec));
            rmRec.PMCTparas = &localMct;
            rmRec.PIt3Dex = &localIt3D;
            // Mirrors Pascal CalcThread.pas post-T10:
            //   if RaySetupDeterministic then
            //     seed := RaySetupDeterministicThreadSeed(iThreadId)  // ThreadId*$324594A1+$24563487
            //   else
            //     seed := PasLcgInitFromThread(iThreadId);            // (ThreadId+1)*$324594A1+$24563487
            uint32_t threadSeed = opts.deterministic_mode
                ? static_cast<uint32_t>(DeterministicThreadSeed(localMct.iThreadId))
                : mb3d::PasLcgInitFromThread(localMct.iThreadId);

            int localHits = 0;
            const LongBool initialInsideRendering = localMct.bInsideRendering;

            for (int y = threadIndex; y < height; y += numThreads) {
                RenderPoll();
                // I1: Set CAFY per row
                const int screenY = y + localMct.CalcRect.Top;
                localMct.CAFY = CalcCameraRowAngle(screenY, localMct.iMandHeight, localMct.FOVy);

                for (int x = 0; x < width; x++) {
                    RenderPoll();
                    int idx = y * width + x;
                    int diagSlot = -1;
                    if (!diagSlotByPixel.empty()) {
                        diagSlot = diagSlotByPixel[idx];
                    }
                    if (opts.diag_only && diagSlot < 0) {
                        continue;
                    }

                    RenderWork(); // One admitted primary pixel; DE/callbacks are separate events.

                    bool traceParityPixel = hasParityPoint &&
                        parityPoint.x == x && parityPoint.y == y;

                    if (mb3d::GetParityTraceSchemaMode() && traceParityPixel) {
                        // Schema mode uses the Parity* builder API (HasParityTraceContext
                        // returns false in schema mode so legacy emitters stay dormant,
                        // but the builder API needs g_parityTraceSession.lines set).
                        mb3d::SetParityTraceContext(x, y, &result.parity_trace_lines);
                    }
                    mb3d::ParityEnterPixel(x, y);

                    // Pascal resets inside flags per pixel before any DE work.
                    localMct.bInsideRendering = initialInsideRendering;
                    localMct.bCalcInside = initialInsideRendering;

                    // NC2: VgradsFOV per-pixel with x+1
                    const int screenX = x + localMct.CalcRect.Left;
                    RMCalculateVgradsFOV(&localMct, screenX + 1);
                    ApplyDebugRayReadyVecOverride(localMct, x, y);

                    TsiLight5* psi = &result.silight[idx];

                    psi->RoughZposFine = 0;
                    psi->Zpos = 32768;
                    psi->Shadow = 0;
                    psi->AmbShadow = 5000;

                    // Calculate start position for this pixel
                    RMCalculateStartPos(&localMct, screenX, screenY);

                    if (mb3d::ParityActive()) {
                        // ray_ready: mirrors Pascal emission right after RaySetupCaptureRayReady,
                        // when mZZ is 0 (ray has not stepped yet) and CAFX/CAFY are set.
                        mb3d::ParityBeginCurrentStage("ray_ready");
                        mb3d::ParityAddDouble12("mZZ", 0.0);
                        mb3d::ParityAddDouble12("CAFX", localMct.CAFX);
                        mb3d::ParityAddDouble12("CAFY", localMct.CAFY);
                        mb3d::ParityEndStage();
                    }

                    RaySetupDumpRecord* raySetup = nullptr;
                    if (!result.ray_setup_records.empty()) {
                        raySetup = &result.ray_setup_records[static_cast<size_t>(idx)];
                        InitRaySetupDumpRecord(*raySetup, x, y, localMct.iThreadId,
                                               static_cast<int32_t>(threadSeed));
                    }

                    if (diagSlot >= 0) {
                        auto& record = result.diagnostics[diagSlot];
                        record.cafx = localMct.CAFX;
                        record.cafy = localMct.CAFY;
                        record.view_vec = {localMct.mVgradsFOV[0], localMct.mVgradsFOV[1], localMct.mVgradsFOV[2]};
                        record.march_vec = {localMct.mVgradsFOV[0], localMct.mVgradsFOV[1], localMct.mVgradsFOV[2]};
                        record.start_pos = {localIt3D.C1, localIt3D.C2, localIt3D.C3};
                        record.ms_destop = localMct.msDEstop;
                        record.de_stop = localMct.DEstop;
                        record.inside_rendering = localMct.bInsideRendering != 0;
                        if (!opts.primary_trace_dump_path.empty()) {
                            std::ostringstream pixelEnter;
                            pixelEnter.setf(std::ios::fixed, std::ios::floatfield);
                            pixelEnter << std::setprecision(12)
                                       << "thread=" << localMct.iThreadId
                                       << " inside=" << static_cast<int>(localMct.bInsideRendering)
                                       << " calcInside=" << static_cast<int>(localMct.bCalcInside)
                                       << " inAndOut=" << static_cast<int>(localMct.bInAndOutside)
                                       << " DEstop=" << localMct.DEstop
                                       << " Zend=" << localMct.Zend;
                            record.primary_trace_lines.push_back(
                                "stage=pixel_enter x=" + std::to_string(x) +
                                " y=" + std::to_string(y) + " " + pixelEnter.str());

                            std::ostringstream rayReady;
                            rayReady.setf(std::ios::fixed, std::ios::floatfield);
                            rayReady << std::setprecision(12)
                                     << "CAFX=" << localMct.CAFX
                                     << " CAFY=" << localMct.CAFY
                                     << " view=" << localMct.mVgradsFOV[0] << "," << localMct.mVgradsFOV[1] << "," << localMct.mVgradsFOV[2]
                                     << " start=" << localIt3D.C1 << "," << localIt3D.C2 << "," << localIt3D.C3
                                     << " carry_calc_sit=" << (localIt3D.CalcSIT ? -1 : 0)
                                     << " carry_it=" << localIt3D.ItResultI
                                     << " carry_smooth=" << localIt3D.SmoothItD
                                     << " carry_rout=" << localIt3D.Rout
                                     << " carry_otrap=" << localIt3D.OTrap
                                     << " Ystart=" << localMct.Ystart[0] << "," << localMct.Ystart[1] << "," << localMct.Ystart[2]
                                     << " Vgrads0=" << localMct.Vgrads[0][0] << "," << localMct.Vgrads[0][1] << "," << localMct.Vgrads[0][2]
                                     << " Vgrads1=" << localMct.Vgrads[1][0] << "," << localMct.Vgrads[1][1] << "," << localMct.Vgrads[1][2]
                                     << " Vgrads2=" << localMct.Vgrads[2][0] << "," << localMct.Vgrads[2][1] << "," << localMct.Vgrads[2][2];
                            record.primary_trace_lines.push_back(
                                "stage=ray_ready x=" + std::to_string(x) +
                                " y=" + std::to_string(y) + " " + rayReady.str());
                        }
                    }
                    if (traceParityPixel) {
                        SetParityTraceContext(x, y, &result.parity_trace_lines);
                        const int insideRenderingTrace = localMct.bInsideRendering ? -1 : 0;
                        const int calcInsideTrace = localMct.bCalcInside ? -1 : 0;
                        std::ostringstream pixelEnter;
                        pixelEnter.setf(std::ios::fixed, std::ios::floatfield);
                        pixelEnter << std::setprecision(12)
                                   << "thread=" << localMct.iThreadId
                                   << " inside_rendering=" << insideRenderingTrace
                                   << " calc_inside=" << calcInsideTrace
                                   << " in_and_out=" << (localMct.bInAndOutside ? -1 : 0)
                                   << " destop=" << localMct.DEstop
                                   << " msdestop=" << localMct.msDEstop
                                   << " zpos0=" << psi->Zpos
                                   << " rough0=" << psi->RoughZposFine
                                   << " shadow0=" << psi->Shadow
                                   << " amb_shadow0=" << psi->AmbShadow;
                        AppendParityTrace("pixel_enter", pixelEnter.str());

                        std::ostringstream rayReady;
                        rayReady.setf(std::ios::fixed, std::ios::floatfield);
                        rayReady << std::setprecision(12)
                                 << "CAFX=" << localMct.CAFX
                                 << " CAFY=" << localMct.CAFY
                                 << " mVgradsFOV_x=" << localMct.mVgradsFOV[0]
                                 << " mVgradsFOV_y=" << localMct.mVgradsFOV[1]
                                 << " mVgradsFOV_z=" << localMct.mVgradsFOV[2]
                                 << " view_x=" << localMct.mVgradsFOV[0]
                                 << " view_y=" << localMct.mVgradsFOV[1]
                                 << " view_z=" << localMct.mVgradsFOV[2]
                                 << " march_x=" << localMct.mVgradsFOV[0]
                                 << " march_y=" << localMct.mVgradsFOV[1]
                                 << " march_z=" << localMct.mVgradsFOV[2]
                                 << " start_x=" << localIt3D.C1
                                 << " start_y=" << localIt3D.C2
                                 << " start_z=" << localIt3D.C3
                                 << " carry_calc_sit=" << (localIt3D.CalcSIT ? -1 : 0)
                                 << " carry_it=" << localIt3D.ItResultI
                                 << " carry_smooth=" << localIt3D.SmoothItD
                                 << " carry_rout=" << localIt3D.Rout
                                 << " carry_otrap=" << localIt3D.OTrap
                                 << " FOVXoff=" << localMct.FOVXoff
                                 << " FOVXmul=" << localMct.FOVXmul
                                 << " FOVy=" << localMct.FOVy
                                 << " Ystart_x=" << localMct.Ystart[0]
                                 << " Ystart_y=" << localMct.Ystart[1]
                                 << " Ystart_z=" << localMct.Ystart[2]
                                 << " Vgrads2_x=" << localMct.Vgrads[2][0]
                                 << " Vgrads2_y=" << localMct.Vgrads[2][1]
                                 << " Vgrads2_z=" << localMct.Vgrads[2][2]
#if defined(_MSC_VER) && defined(_M_X64)
                                 << " x87_cw=0x" << std::hex << std::uppercase << GetX87ControlWord()
                                 << " mxcsr=0x" << _mm_getcsr() << std::dec
#endif
                                 << " StepWidth=" << localMct.StepWidth;
                        AppendParityTrace("ray_ready", rayReady.str());
                    }

                    rmRec.TracePixelX = x;
                    rmRec.TracePixelY = y;
                    rmRec.TraceLines =
                        (diagSlot >= 0 && !opts.primary_trace_dump_path.empty())
                        ? &result.diagnostics[diagSlot].primary_trace_lines
                        : nullptr;
                    rmRec.RaySetupRecordOut = raySetup;
                    rmRec.RaySetupOnly = !opts.ray_setup_dump_path.empty();
                    bool retriedInsideOutside = false;
                    for (;;) {
                        RenderPoll();
                        // NC10: msDEstop reset per pixel / retry
                        localMct.msDEstop = localMct.DEstop;
                        localMct.mZZ = 0.0;
                        localIt3D.CalcSIT = 0;

                        RMCalculateStartPos(&localMct, screenX, screenY);

                        // Setup ray march record
                        rmRec.ActZpos = 0;
                        rmRec.StartDEstop = localMct.DEstop;
                        rmRec.MaxRayLength = localMct.Zend;
                        rmRec.BinSteps = localMct.iDEAddSteps;
                        rmRec.RMresult = 0;
                        rmRec.StepCount = 0;
                        rmRec.StepForward = 0;
                        rmRec.Zstepped = 0;
                        rmRec.seed = static_cast<int32_t>(threadSeed);

                        // DEmul values
                        if (localMct.bVaryDEstop) {
                            double aspect;
                            if (localMct.MCTCameraOptic == 2) {
                                aspect = 1.0;
                            } else {
                                double cosVal = std::cos(std::sqrt(localMct.CAFX * localMct.CAFX +
                                                                   localMct.CAFY * localMct.CAFY));
                                aspect = std::max(0.1, cosVal);
                            }
                            rmRec.DEmulVary = localMct.mctDEstopFactor / aspect;
                            rmRec.DEmulConst = 1.0;
                        } else {
                            rmRec.DEmulVary = 0;
                            rmRec.DEmulConst = 1.0;
                        }

                        rmRec.ZZposMul = 1.0;

                        // Reuse the same parity path as RMCalculateVgradsFOV/RMCalculateStartPos.
                        std::memcpy(rmRec.VievVec, localMct.mVgradsFOV, sizeof(TVec3D));
                        std::memcpy(rmRec.MarchVec, localMct.mVgradsFOV, sizeof(TVec3D));
                        std::memcpy(rmRec.ActPos, &localIt3D.C1, sizeof(TVec3D));
                        if (raySetup && raySetup->ray_ready_captured == 0) {
                            raySetup->view_vec[0] = rmRec.VievVec[0];
                            raySetup->view_vec[1] = rmRec.VievVec[1];
                            raySetup->view_vec[2] = rmRec.VievVec[2];
                            raySetup->march_vec[0] = rmRec.MarchVec[0];
                            raySetup->march_vec[1] = rmRec.MarchVec[1];
                            raySetup->march_vec[2] = rmRec.MarchVec[2];
                            raySetup->act_pos[0] = rmRec.ActPos[0];
                            raySetup->act_pos[1] = rmRec.ActPos[1];
                            raySetup->act_pos[2] = rmRec.ActPos[2];
                            raySetup->start_destop = rmRec.StartDEstop;
                            raySetup->max_ray_length = rmRec.MaxRayLength;
                            raySetup->zzpos_mul = rmRec.ZZposMul;
                            raySetup->de_mul_vary = rmRec.DEmulVary;
                            raySetup->de_mul_const = rmRec.DEmulConst;
                            CaptureRayReadyDump(*raySetup, localMct, localIt3D);
                        }

                        SetFormulaTraceContext(rmRec.TracePixelX, rmRec.TracePixelY, rmRec.TraceLines);
                        RayMarch(&rmRec);
                        SetFormulaTraceContext(-1, -1, nullptr);
                        threadSeed = static_cast<uint32_t>(rmRec.seed);

                        if (rmRec.RaySetupOnly) {
                            break;
                        }

                        const bool retryInsideOutside =
                            (rmRec.StartInside != 0) &&
                            (localMct.bInAndOutside != 0) &&
                            (localMct.bCalcInside == initialInsideRendering) &&
                            !retriedInsideOutside;
                        if (!retryInsideOutside) {
                            break;
                        }

                        retriedInsideOutside = true;
                        localMct.bCalcInside = initialInsideRendering ? 0 : -1;
                        localMct.bInsideRendering = initialInsideRendering ? 0 : -1;
                        if (diagSlot >= 0 && !opts.primary_trace_dump_path.empty()) {
                            auto& record = result.diagnostics[diagSlot];
                            std::ostringstream payload;
                            payload.setf(std::ios::fixed, std::ios::floatfield);
                            payload << std::setprecision(12)
                                    << "inside=" << (localMct.bInsideRendering ? -1 : 0)
                                    << " calcInside=" << (localMct.bCalcInside ? -1 : 0)
                                    << " startInside=" << rmRec.StartInside
                                    << " rmResult=" << rmRec.RMresult;
                            record.primary_trace_lines.push_back(
                                "stage=inside_outside_retry x=" + std::to_string(x) +
                                " y=" + std::to_string(y) + " " + payload.str());
                        }
                        if (traceParityPixel) {
                            std::ostringstream payload;
                            payload.setf(std::ios::fixed, std::ios::floatfield);
                            payload << std::setprecision(12)
                                    << "inside=" << (localMct.bInsideRendering ? -1 : 0)
                                    << " calcInside=" << (localMct.bCalcInside ? -1 : 0)
                                    << " startInside=" << rmRec.StartInside
                                    << " rmResult=" << rmRec.RMresult;
                            AppendParityTrace("inside_outside_retry", payload.str());
                        }
                    }

                    if (rmRec.RaySetupOnly) {
                        continue;
                    }

                    if (diagSlot >= 0) {
                        auto& record = result.diagnostics[diagSlot];
                        record.hit = rmRec.RMresult > 0;
                        record.rm_result = rmRec.RMresult;
                        record.start_inside = rmRec.StartInside;
                        record.cut_plane_hit = rmRec.CutPlaneHit;
                        record.act_zpos = rmRec.ActZpos;
                        record.z_stepped = localMct.mZZ;
                        record.final_de = rmRec.finalDE;
                        record.step_count = rmRec.StepCount;
                        record.inside_rendering = localMct.bInsideRendering != 0;
                        record.hit_pos = {rmRec.ActPos[0], rmRec.ActPos[1], rmRec.ActPos[2]};
                    }

                    int shadow = 0;
                    float stepCountForShadow = rmRec.StepCount;
                    if (localMct.bInsideRendering) {
                        stepCountForShadow *= 200.0f * localMct.DEstop / static_cast<float>(localMct.iMandWidth);
                    }
                    if (localMct.DFogOnIt == 65535) {
                        shadow = RMcalcVLight(stepCountForShadow);
                    } else {
                        shadow = PascalRoundToInt(static_cast<float>(Min0MaxCS(
                            (stepCountForShadow - 1.0f) + 1.0f, 1023.0)));
                    }
                    psi->Shadow = static_cast<uint16_t>((psi->Shadow & 0xFC00u) |
                                                        static_cast<uint16_t>(shadow & 0x03FF));

                    if (rmRec.RMresult > 0) {
                        localHits++;

                        // Pascal leaves the iteration position at the refined hit point.
                        // Post-hit surface work re-enters CalcDE from that position.
                        localIt3D.C1 = rmRec.ActPos[0];
                        localIt3D.C2 = rmRec.ActPos[1];
                        localIt3D.C3 = rmRec.ActPos[2];

                        // Point mPsiLight to current pixel for normal calculation
                        localMct.mPsiLight = psi;
                        localMct.mZZ = rmRec.Zstepped;

                        const bool insideStartHit = (rmRec.StartInside != 0);
                        if (insideStartHit) {
                            // Pascal treats inside/max-it hits as a distinct rendering path:
                            // default normals, orbit-trap color, and SIgradient marked with
                            // the interior high bit.
                            if (localMct.ColorOnIt != 0) {
                                RMdoColorOnIt(&localMct);
                            }
                            RMdoColor(&localMct);
                            RMcalcNanglesForCut(&localMct, rmRec.CutPlaneHit);

                            if (localMct.ColorOption > 4) {
                                psi->SIgradient = static_cast<uint16_t>(0x8000u | psi->SIgradient);
                            } else if (localMct.bInsideRendering) {
                                if (diagSlot >= 0 && !opts.primary_trace_dump_path.empty()) {
                                    auto& record = result.diagnostics[diagSlot];
                                    std::ostringstream payload;
                                    payload.setf(std::ios::fixed, std::ios::floatfield);
                                    payload << std::setprecision(12)
                                            << "CalcSIT=" << (localIt3D.CalcSIT ? -1 : 0)
                                            << " ItResultI=" << localIt3D.ItResultI
                                            << " SmoothItD=" << localIt3D.SmoothItD
                                            << " Rout=" << localIt3D.Rout
                                            << " OTrap=" << localIt3D.OTrap;
                                    record.primary_trace_lines.push_back(
                                        "stage=inside_sigradient_pre x=" + std::to_string(x) +
                                        " y=" + std::to_string(y) + " " + payload.str());
                                }
                                if (traceParityPixel) {
                                    std::ostringstream payload;
                                    payload.setf(std::ios::fixed, std::ios::floatfield);
                                    payload << std::setprecision(12)
                                            << "CalcSIT=" << (localIt3D.CalcSIT ? -1 : 0)
                                            << " ItResultI=" << localIt3D.ItResultI
                                            << " SmoothItD=" << localIt3D.SmoothItD
                                            << " Rout=" << localIt3D.Rout
                                            << " OTrap=" << localIt3D.OTrap;
                                    AppendParityTrace("inside_sigradient_pre", payload.str());
                                }
                                localIt3D.CalcSIT = 1;
                                localMct.CalcDE(&localIt3D, &localMct);
                                if (diagSlot >= 0 && !opts.primary_trace_dump_path.empty()) {
                                    auto& record = result.diagnostics[diagSlot];
                                    std::ostringstream payload;
                                    payload.setf(std::ios::fixed, std::ios::floatfield);
                                    payload << std::setprecision(12)
                                            << "CalcSIT=" << (localIt3D.CalcSIT ? -1 : 0)
                                            << " ItResultI=" << localIt3D.ItResultI
                                            << " SmoothItD=" << localIt3D.SmoothItD
                                            << " Rout=" << localIt3D.Rout
                                            << " OTrap=" << localIt3D.OTrap;
                                    record.primary_trace_lines.push_back(
                                        "stage=inside_sigradient_post x=" + std::to_string(x) +
                                        " y=" + std::to_string(y) + " " + payload.str());
                                }
                                if (traceParityPixel) {
                                    std::ostringstream payload;
                                    payload.setf(std::ios::fixed, std::ios::floatfield);
                                    payload << std::setprecision(12)
                                            << "CalcSIT=" << (localIt3D.CalcSIT ? -1 : 0)
                                            << " ItResultI=" << localIt3D.ItResultI
                                            << " SmoothItD=" << localIt3D.SmoothItD
                                            << " Rout=" << localIt3D.Rout
                                            << " OTrap=" << localIt3D.OTrap;
                                    AppendParityTrace("inside_sigradient_post", payload.str());
                                }
                                int sigI = std::clamp(
                                    PascalRoundToInt(localIt3D.SmoothItD * localMct.mctsM),
                                    0, 32767);
                                psi->SIgradient = static_cast<uint16_t>(0x8000u | sigI);
                            } else {
                                int sigI = 32768 + PascalRoundToInt(
                                    32767.0f * Clamp01S(static_cast<float>(localIt3D.Rout / localMct.dRStop)));
                                psi->SIgradient = static_cast<uint16_t>(std::clamp(sigI, 32768, 65535));
                            }
                        } else {
                            // NI7: Post-hit order: normals -> SIgradient -> RMdoColor -> CalcZposAndRough

                            // 1. Calculate normals - writes directly to mPsiLight
                            float NN = 0;
                            ApplyDebugNormalsEntryOverride(localMct, localIt3D);
                            calcNormals(&localMct, NN);
                            if (traceParityPixel) {
                                SetParityTraceContext(x, y, &result.parity_trace_lines);
                                int deLimitedPas = rmRec.RMresult == 1 ? -1 : 0;
                                std::ostringstream payload;
                                payload.setf(std::ios::fixed, std::ios::floatfield);
                                payload << std::setprecision(12)
                                        << "NormalX=" << psi->NormalX
                                        << " NormalY=" << psi->NormalY
                                        << " NormalZ=" << psi->NormalZ
                                        << " NN=" << NN
                                        << " C1=" << localIt3D.C1
                                        << " C2=" << localIt3D.C2
                                        << " C3=" << localIt3D.C3
                                        << " mZZ=" << localMct.mZZ
                                        << " iSmNormals=" << localMct.iSmNormals
                                        << " DElimited=" << deLimitedPas;
                                AppendParityTrace("normals", payload.str());
                            }

                            // 2. C18: SIgradient with 32767 inversion, dColPlus, log DE-stop term
                            {
                                double RSFmul = static_cast<double>(NN);
                                // CalcThread.pas saves this decision before refinement.
                                // Normal probes overwrite ItResultI; recomputing here
                                // incorrectly changes the coloring of max-iteration hits.
                                bool DElimited = rmRec.RMresult == 1;
                                double logTerm = 0.0;
                                double sigF;
                                if (DElimited) {
                                    logTerm = static_cast<double>(localMct.mctColVarDEstopMul) *
                                        std::log(MaxCS(static_cast<double>(localMct.DEstop),
                                                       static_cast<double>(localMct.msDEstop)) *
                                                 static_cast<double>(localMct.StepWidth));
                                    sigF = 32767.0 - (RSFmul + static_cast<double>(localMct.dColPlus) + logTerm) *
                                                      static_cast<double>(localMct.mctsM);
                                } else {
                                    sigF = 32767.0 - RSFmul * static_cast<double>(localMct.mctsM);
                                }
                                float sigF32 = static_cast<float>(sigF);
                                MinMaxClip15bit(sigF32, psi->SIgradient);
                                if (traceParityPixel) {
                                    std::ostringstream payload;
                                    payload.setf(std::ios::fixed, std::ios::floatfield);
                                    payload << std::setprecision(12)
                                            << "dColPlus=" << localMct.dColPlus
                                            << " mctColVarDEstopMul=" << localMct.mctColVarDEstopMul
                                            << " logTerm=" << logTerm
                                            << " mctsM=" << localMct.mctsM
                                            << " SIgradient_raw=" << sigF32
                                            << " SIgradient_packed=" << psi->SIgradient;
                                    AppendParityTrace("sigradient", payload.str());
                                }
                            }

                            // 3. I3: OTrap via RMdoColor
                            if (localMct.ColorOnIt != 0) {
                                RMdoColorOnIt(&localMct);
                            }
                            uint16_t otrapBefore = psi->OTrap;
                            RMdoColor(&localMct);
                            if (traceParityPixel) {
                                std::ostringstream payload;
                                payload.setf(std::ios::fixed, std::ios::floatfield);
                                payload << std::setprecision(12)
                                        << "OTrap_before=" << otrapBefore
                                        << " OTrap_after=" << psi->OTrap
                                        << " OTrap_source=" << RMTraceLastColorOTrap()
                                        << " scalar=" << RMTraceLastColorScalar()
                                        << " ColorOnIt=" << static_cast<int>(localMct.ColorOnIt)
                                        << " ColorOption=" << static_cast<int>(localMct.ColorOption);
                                AppendParityTrace("color_otrap", payload.str());
                            }

                            // 4. C15: Z position encoding via CalcZposAndRough
                            CalcZposAndRough(psi, &localMct, localMct.mZZ);

                            if (localMct.bInAndOutside && !localMct.bInsideRendering) {
                                psi->OTrap = static_cast<uint16_t>(psi->OTrap | 0x8000u);
                            }
                            if (traceParityPixel) {
                                ClearParityTraceContext();
                            }
                        }

                        if (traceParityPixel && insideStartHit) {
                            ClearParityTraceContext();
                        }
                    } else {
                        // Background pixel
                        psi->Zpos = 32768;  // Mark as background
                        psi->NormalX = 0;
                        psi->NormalY = 0;
                        psi->NormalZ = 0;
                        psi->SIgradient = 0;
                        psi->OTrap = 0;
                        psi->AmbShadow = 5000;
                        psi->RoughZposFine = 0;
                        if (traceParityPixel) {
                            SetParityTraceContext(x, y, &result.parity_trace_lines);
                            std::ostringstream payload;
                            payload.setf(std::ios::fixed, std::ios::floatfield);
                            payload << std::setprecision(12)
                                    << "RMresult=" << rmRec.RMresult
                                    << " Zpos=" << psi->Zpos
                                    << " RoughZposFine=" << psi->RoughZposFine
                                    << " NormalX=" << psi->NormalX
                                    << " NormalY=" << psi->NormalY
                                    << " NormalZ=" << psi->NormalZ
                                    << " SIgradient=" << psi->SIgradient
                                    << " OTrap=" << psi->OTrap
                                    << " Shadow=" << psi->Shadow
                                    << " AmbShadow=" << psi->AmbShadow;
                            AppendParityTrace("background", payload.str());
                            ClearParityTraceContext();
                        }
                    }
                }

                if (opts.verbose) {
                    int done = completedRows.fetch_add(1) + 1;
                    if (done % 50 == 0) {
                        std::fprintf(stderr, "  Ray marching: %d/%d rows\n", done, height);
                    }
                }
            }

            hitCount.fetch_add(localHits);
        }));
    }
    } catch (...) {
        FailRender(std::current_exception());
        pool.wait_all();
        throw;
    }
    pool.wait_all();
    RenderPoll();
    for (auto& task : primaryTasks) task.get();
    primaryTimer.complete();

    if (opts.verbose) {
        auto _rm_t1 = std::chrono::steady_clock::now();
        double _rm_s = std::chrono::duration<double>(_rm_t1 - _rm_t0).count();
        int hc = hitCount.load();
        std::fprintf(stderr, "Ray march complete. Hits: %d/%d (%.1f%%)  [%.2fs]\n",
                    hc, totalPixels,
                    100.0 * hc / std::max(1, totalPixels), _rm_s);
    }

    if (opts.verbose) {
        int hits = 0, maxZpos = 0, minZpos = 65535;
        float maxSI = 0;
        for (int idx = 0; idx < totalPixels; idx++) {
            RenderPoll();
            auto& sl = result.silight[idx];
            if (sl.Zpos <= 32767) {
                hits++;
                maxZpos = std::max(maxZpos, static_cast<int>(sl.Zpos));
                minZpos = std::min(minZpos, static_cast<int>(sl.Zpos));
                maxSI = std::max(maxSI, static_cast<float>(sl.SIgradient));
            }
        }
        std::fprintf(stderr, "Z-buffer stats: hits=%d, Zpos range=[%d, %d], max SIgradient=%.0f\n",
                     hits, minZpos, maxZpos, maxSI);
    }

    if (!opts.ray_setup_dump_path.empty()) {
        result.success = true;
        return result;
    }

    if (opts.diag_only && !NeedsLatePassTraceInDiagOnly(opts)) {
        if (!result.diagnostics.empty()) {
            for (auto& record : result.diagnostics) {
                RenderPoll();
                if (!record.in_bounds) continue;
                int idx = record.y * width + record.x;
                const auto& psi = result.silight[idx];
                record.zpos = psi.Zpos;
                record.rough_zpos_fine = psi.RoughZposFine;
                record.shadow = psi.Shadow;
                record.amb_shadow = psi.AmbShadow;
                record.sigradient = psi.SIgradient;
                record.otrap = psi.OTrap;
                record.normal_x = psi.NormalX;
                record.normal_y = psi.NormalY;
                record.normal_z = psi.NormalZ;
                record.rgb = {0, 0, 0};
            }
        }

        result.success = true;
        return result;
    }

    // ========================================================================
    // Pascal AllAutoProcessings stage 2 replaces surface normals before all
    // lighting-dependent postprocessing, including hard shadows and AO.
    // Primary workers have finished before this neighborhood operation begins.
    if (header.bCalc3D > 0 && (header.byCalcNsOnZBufAuto & 1) != 0) {
        if (opts.verbose) std::fprintf(stderr, "Recalculating normals from Z-buffer...\n");
        StageTimer normalsTimer(profile, RenderProfileStage::Normals);
        RecalculateNormalsFromZBuffer(header, result.silight);
        normalsTimer.complete();
    }

    // ========================================================================
    // Pass 2: Hard Shadows (parallelized by row ranges)
    // ========================================================================
    // Pascal AllAutoProcessings() only schedules hard shadows when bit0 is set
    // and at least one shadow-light bit is enabled.
    if (ShouldRunHardShadowPass(header, opts)) {
        StageTimer shadowTimer(profile, RenderProfileStage::Shadows);
        if (opts.verbose) {
            std::fprintf(stderr, "Starting hard shadow pass...\n");
        }

        // Pascal CalcHardShadowT builds a dedicated hard-shadow header state,
        // then derives MCT/light values from that mutated header, not from the
        // main render header.
        TMandHeader10 hsHeader = header;
        int hsBaseState = header.bHScalculated;
        if ((hsHeader.bCalc1HSsoft & 1) != 0) {
            hsHeader.bHScalculated = static_cast<uint8_t>((hsBaseState & 1) | hsHeader.bCalculateHardShadow);
        } else {
            hsHeader.bHScalculated = static_cast<uint8_t>((hsBaseState & 0xFD) | hsHeader.bCalculateHardShadow);
        }
        if ((hsHeader.bHScalculated & 2) != 0) {
            for (int x = 0; x < 6; ++x) {
                RenderPoll();
                if (((hsHeader.bCalculateHardShadow >> (x + 2)) & 1) != 0) {
                    hsHeader.Light.Lights[x].LFunction &= 0xCF;
                }
            }
        }

        TMCTparameter hsMct = GetMCTparasFromHeader(hsHeader, &addon, 1);
        if (!hsMct.bMCTisValid) {
            result.error = "Failed to setup hard shadow MCT parameters";
            return result;
        }

        TLightVals hsLightVals = lightVals;
        TLValigned hsAlignedStorage = alignedStorage;
        hsLightVals.PLValigned = &hsAlignedStorage;
        CalcHSVecsFromLights(&hsLightVals, &hsMct);

        hsMct.calcHardShadow = hsHeader.bCalculateHardShadow;
        if (hsHeader.bCalc1HSsoft & 1) {
            hsMct.calcHardShadow |= 256;
        }

        if (hsHeader.HSmaxLengthMultiplier > 0.0f) {
            hsMct.sHSmaxLengthMultiplier = hsHeader.HSmaxLengthMultiplier;
        } else {
            hsMct.sHSmaxLengthMultiplier = 1.0f;
        }

        float ssr = ShortFloatToSingle(hsHeader.MCSoftShadowRadius);
        if (ssr > 0.0f) {
            hsMct.SoftShadowRadius = ssr;
        } else {
            hsMct.SoftShadowRadius = 1.0f;
        }

        if (numThreads <= 1) {
            if (hasParityPoint && parityPoint.x >= 0 && parityPoint.x < width &&
                parityPoint.y >= 0 && parityPoint.y < height) {
                SetParityTraceContext(parityPoint.x, parityPoint.y, &result.parity_trace_lines);
            }
            CalcHardShadows(result.silight.data(), width, height, width,
                            &hsMct, &hsLightVals);
            if (hasParityPoint && parityPoint.x >= 0 && parityPoint.x < width &&
                parityPoint.y >= 0 && parityPoint.y < height) {
                ClearParityTraceContext();
            }
        } else {
            completedRows.store(0);
            ParallelRowRange(pool, height, batchSize,
                [&](int y_start, int y_end) {
                    TMCTparameter localMct = hsMct;
                    TLightVals localLV = hsLightVals;
                    TLValigned localAligned = hsAlignedStorage;
                    localLV.PLValigned = &localAligned;
                    CalcHardShadowsRows(result.silight.data(), width, height, width,
                                        &localMct, &localLV, y_start, y_end);
                }
            );
        }

        if (opts.verbose) {
            std::fprintf(stderr, "Hard shadow pass complete.\n");
        }

        // CalcHardShadowT mutates the completed hard-shadow state that the main
        // paint path then consumes through HeaderLightVals.
        ApplyCompletedHardShadowStateToPaint(header, lightVals);

        if (hasParityPoint && parityPoint.x >= 0 && parityPoint.x < width &&
            parityPoint.y >= 0 && parityPoint.y < height) {
            const auto& psi = result.silight[static_cast<size_t>(parityPoint.y) * width + parityPoint.x];
            SetParityTraceContext(parityPoint.x, parityPoint.y, &result.parity_trace_lines);
            std::ostringstream payload;
            payload << "Shadow=" << psi.Shadow
                    << " AmbShadow=" << psi.AmbShadow;
            AppendParityTrace("shadow_post", payload.str());
            ClearParityTraceContext();
        }
        shadowTimer.complete();
    }

    // ========================================================================
    // Pass 3: Ambient Occlusion (type 3 parallelized, types 0-2 single-threaded)
    // ========================================================================
    if (!opts.no_ao && mctParams.bCalcAmbShadow) {
        StageTimer aoTimer(profile, RenderProfileStage::AO);
        int aoType = (header.bCalcAmbShadowAutomatic >> 2) & 3;
        int aoQuality = (header.bCalcAmbShadowAutomatic >> 4) & 3;
        const bool hiqAo = (header.bCalcAmbShadowAutomatic & 12) == 8;
        const int aoIterations =
            (hiqAo && header.SSAORcount > 1) ? static_cast<int>(header.SSAORcount) : 1;

        if (opts.verbose) {
            std::fprintf(stderr, "Starting ambient occlusion pass (type=%d, quality=%d, iterations=%d)...\n",
                        aoType, aoQuality, aoIterations);
        }

        // Random HiQ AO accumulates into this buffer even for a single pass.
        // Mand.pas clears it before SSAORiteration 1, independent of Rcount.
        if (hiqAo) {
            for (auto& psi : result.silight) {
                RenderPoll();
                psi.AmbShadow = 0;
            }
        }

        for (int aoIter = 0; aoIter < aoIterations; ++aoIter) {
            RenderPoll();
            if (numThreads <= 1 || aoType != 3) {
                // Types 0-2 are image-space (wavelet/screen-space), run single-threaded
                if (hasParityPoint && parityPoint.x >= 0 && parityPoint.x < width &&
                    parityPoint.y >= 0 && parityPoint.y < height) {
                    SetParityTraceContext(parityPoint.x, parityPoint.y, &result.parity_trace_lines);
                }
                const int aoWidth = aoType == 3 ? width : header.Width;
                const int aoHeight = aoType == 3 ? height : header.Height;
                CalcAmbientOcclusion(result.silight.data(), aoWidth, aoHeight, aoWidth,
                                     &mctParams, &lightVals, aoType, aoQuality, &header, numThreads);
                if (HasParityTraceContext()) {
                    ClearParityTraceContext();
                }
            } else {
                // Type 3 (DE-based) can be parallelized by rows
                completedRows.store(0);
                ParallelRowRange(pool, height, batchSize,
                    [&](int y_start, int y_end) {
                        TMCTparameter localMct = mctParams;
                        TLightVals localLV = lightVals;
                        TLValigned localAligned = alignedStorage;
                        localLV.PLValigned = &localAligned;
                        CalcAmbientOcclusionRows(result.silight.data(), width, height, width,
                                                 &localMct, &localLV, aoType, aoQuality,
                                                 &header, y_start, y_end);
                    }
                );
            }
        }

        if (opts.verbose) {
            std::fprintf(stderr, "Ambient occlusion pass complete.\n");
        }

        if (hasParityPoint && parityPoint.x >= 0 && parityPoint.x < width &&
            parityPoint.y >= 0 && parityPoint.y < height) {
            const auto& psi = result.silight[static_cast<size_t>(parityPoint.y) * width + parityPoint.x];
            SetParityTraceContext(parityPoint.x, parityPoint.y, &result.parity_trace_lines);
            std::ostringstream payload;
            payload << "Shadow=" << psi.Shadow
                    << " AmbShadow=" << psi.AmbShadow;
            AppendParityTrace("ao_post", payload.str());
            ClearParityTraceContext();
        }
        aoTimer.complete();
    }

    // ========================================================================
    // Pass 4: Coloring (parallelized by row ranges)
    // ========================================================================
    color_pass:
    RunColorPass(pool, numThreads, batchSize, header, opts, parityTraceStageState, lightVals, result,
                 width, height, hasParityPoint, parityPoint, profile);

    if (opts.verbose) {
        int maxR = 0, maxG = 0, maxB = 0;
        double sumBright = 0;
        for (int idx = 0; idx < totalPixels * 3; idx += 3) {
            RenderPoll();
            maxR = std::max(maxR, static_cast<int>(result.image[idx]));
            maxG = std::max(maxG, static_cast<int>(result.image[idx + 1]));
            maxB = std::max(maxB, static_cast<int>(result.image[idx + 2]));
            sumBright += result.image[idx] + result.image[idx + 1] + result.image[idx + 2];
        }
        std::fprintf(stderr, "Color stats: max RGB=(%d,%d,%d), avg brightness=%.1f\n",
                     maxR, maxG, maxB, sumBright / (totalPixels * 3));
    }

    // ========================================================================
    // Pass 5: Reflections / Transparency + DOF (parallelized by row ranges)
    // ========================================================================
    // Pascal DivUtils.pas::AllAutoProcessings gates reflection/transparency and
    // DOF behind `bCalc3D > 0`. For 2D slice renders these passes are not
    // scheduled at all -- the escape-time / smooth-iteration output is the
    // final image. Running DOF on 2D data catastrophically blurs the fine
    // fractal detail using the pixels' default Zpos value. See CalcThread2D.pas
    // and Mand.pas line 1756 where bCalc3D := 0 for 2D slice mode.
    if (mctParams.calc3D) {
        RunLateImagePasses(pool, numThreads, batchSize, header, opts, addon, parityTraceStageState,
                           mctParams, lightVals, alignedStorage, result,
                           width, height, hasParityPoint, parityPoint, profile);
    }

    if (!result.diagnostics.empty()) {
        for (auto& record : result.diagnostics) {
            RenderPoll();
            if (!record.in_bounds) continue;
            int idx = record.y * width + record.x;
            const auto& psi = result.silight[idx];
            record.zpos = psi.Zpos;
            record.rough_zpos_fine = psi.RoughZposFine;
            record.shadow = psi.Shadow;
            record.amb_shadow = psi.AmbShadow;
            record.sigradient = psi.SIgradient;
            record.otrap = psi.OTrap;
            record.normal_x = psi.NormalX;
            record.normal_y = psi.NormalY;
            record.normal_z = psi.NormalZ;
            record.rgb = {
                result.image[idx * 3 + 0],
                result.image[idx * 3 + 1],
                result.image[idx * 3 + 2]
            };
        }
    }

    RenderPoll();
    result.success = true;
    return result;
}

static RenderContextHandle MakeRenderContext(const RenderOptions& opts) {
    auto context = std::make_shared<RenderContext>(opts.resource_root.empty()
        ? std::filesystem::current_path().string() : opts.resource_root);
    // Preserve explicitly configured caller frame/sequence settings, but never
    // inherit cache allocations, failed lookups or the previous scene's maps.
    const auto caller = CaptureRenderContext();
    context->maps.sequences = caller->maps.sequences;
    context->maps.frame_number = caller->maps.frame_number;
    context->control = opts.control;
    context->precision = opts.precision;
    return context;
}

static RenderOptions PrepareRenderOptions(const RenderOptions& opts) {
    RenderOptions prepared = opts;
    // The CLI continuation is input state, not shared mutable job state.
    prepared.parity_trace_stage_state = opts.parity_trace_stage_state
        ? std::make_shared<ParityTraceStageRangeState>(*opts.parity_trace_stage_state)
        : std::make_shared<ParityTraceStageRangeState>();
    if (!opts.parity_trace_stage_state) {
        prepared.parity_trace_stage_state->start_stage = opts.parity_trace_start_stage;
        prepared.parity_trace_stage_state->end_stage = opts.parity_trace_end_stage;
        prepared.parity_trace_stage_state->start_reached = opts.parity_trace_start_stage.empty();
    }
    return prepared;
}

static void ConfigureRenderTrace(const RenderOptions& opts) {
    SetParityTraceSchemaMode(opts.parity_trace_schema_mode);
    SetParityTraceLevel(opts.parity_trace_level == "all" ? ParityTraceLevel::DetailAll
        : opts.parity_trace_level == "detail" ? ParityTraceLevel::DetailSelected
        : ParityTraceLevel::Checkpoint);
    SetParityTraceDetailStages(opts.parity_trace_detail);
    if (!opts.parity_trace_stage_state)
        ConfigureParityTraceStageRange(opts.parity_trace_start_stage, opts.parity_trace_end_stage);
}

// The result is published only after all stages/tasks return and the terminal
// transition succeeds. Failure discards every partially produced output buffer.
template<class Work>
static RenderResult ExecuteOrdinary(const RenderOptions& opts, Work&& work) {
    const auto control = opts.control;
    bool ownsControl = false;
    RenderResult failed;
    std::exception_ptr failure;
    RenderProfile profile;
    profile.enabled = opts.profile_timing;
    profile.precision = opts.precision;
    const auto profileStart = profile.enabled ? std::chrono::steady_clock::now()
        : std::chrono::steady_clock::time_point{};
    auto finishProfile = [&](RenderResult& result) {
        if (profile.enabled) {
            profile.total_seconds = std::chrono::duration<double>(std::chrono::steady_clock::now() - profileStart).count();
        }
        result.profile = profile;
    };
    try {
        if (control) {
            control->start();
            ownsControl = true;
            control->poll();
        }
        ScopedRenderContext context(MakeRenderContext(opts));
        ScopedParityTraceSession traceSession;
        ScopedFormulaTraceContext formulaTraceSession;
        auto prepared = PrepareRenderOptions(opts);
        ConfigureRenderTrace(prepared);
        RenderPoll();
        auto result = work(prepared, profile);
        if (!result.success)
            throw std::runtime_error(result.error.empty() ? "Ordinary render failed" : result.error);
        RenderPoll();
        if (control) control->complete();
        result.status = RenderStatus::Completed;
        result.work_units = control ? control->work_used() : 0;
        finishProfile(result);
        return result;
    } catch (const RenderStopped& error) {
        failure = std::current_exception();
        failed.status = error.status();
        failed.error = error.what();
    } catch (const std::exception& error) {
        failure = std::current_exception();
        failed.error = error.what();
    } catch (...) {
        failure = std::current_exception();
        failed.error = "Unknown ordinary render failure";
    }
    if (ownsControl) {
        control->fail(failure);
        failed.work_units = control->work_used();
        failed.status = control->status();
        if (failed.status == RenderStatus::Cancelled || failed.status == RenderStatus::WorkExhausted)
            failed.error = RenderStopped(failed.status).what();
    }
    finishProfile(failed);
    return failed;
}

RenderResult Render(TMandHeader10& header, THeaderCustomAddon& addon, const RenderOptions& opts) {
    if (opts.lattice_stride != 1 || opts.lattice_x || opts.lattice_y || opts.admission_only) {
        RenderResult unsupported;
        unsupported.status = RenderStatus::Unsupported;
        unsupported.error = "A lattice frame needs the Native general Metal renderer";
        return unsupported;
    }
    if (opts.mc_samples != 0) {
        if (opts.control || opts.profile_timing) {
            RenderResult unsupported;
            unsupported.status = RenderStatus::Unsupported;
            unsupported.error = "Render control and profiling are supported only for ordinary rendering";
            return unsupported; // The control is not admitted or consumed.
        }
        return RenderMonteCarlo(header, addon, opts);
    }
    return ExecuteOrdinary(opts, [&](const RenderOptions& prepared, RenderProfile& profile) {
        return RenderOrdinary(header, addon, prepared, profile);
    });
}

static RenderResult PaintFromSiLightImpl(TMandHeader10& header, THeaderCustomAddon& addon,
                              const std::vector<TsiLight5>& silight, const RenderOptions& opts, RenderProfile& profile) {
    StageTimer preparationTimer(profile, RenderProfileStage::Preparation);
    RenderResult result;
    result.success = false;

    int width = (opts.width_override > 0) ? opts.width_override : header.Width;
    int height = (opts.height_override > 0) ? opts.height_override : header.Height;
    if (width <= 0 || height <= 0 || width > 16384 || height > 16384) {
        result.error = "Invalid image dimensions";
        return result;
    }
    header.Width = width;
    header.Height = height;
    ValidateOrdinaryTileStages(header, opts, true);
    int tileX = 0, tileY = 0;
    GetPaintTileSizes(&header, width, height, tileX, tileY);
    if (width <= 0 || height <= 0 || width > 16384 || height > 16384) {
        result.error = "Invalid tile dimensions";
        return result;
    }
    if (silight.size() != static_cast<size_t>(width) * height) {
        result.error = "siLight5 size mismatch for paint replay";
        return result;
    }
    result.width = width;
    result.height = height;
    result.tiling_crop = GetTilingCrop(&header);
    result.image.resize(static_cast<size_t>(width) * height * 3, 0);
    result.silight = silight;

    DiagnosticPoint parityPoint{};
    bool hasParityPoint = !opts.parity_trace_dump_path.empty() && !opts.diag_points.empty();
    if (hasParityPoint) {
        parityPoint = opts.diag_points.front();
    }
#ifdef MB3D_ENABLE_PARITY_TRACE
    ParityTraceStageRangeStateHandle parityTraceStageState = opts.parity_trace_stage_state;
    if (hasParityPoint) {
        if (parityTraceStageState) {
            RestoreParityTraceStageRangeState(parityTraceStageState);
        } else {
            ResetParityTraceStageRange();
            parityTraceStageState = CaptureParityTraceStageRangeState();
        }
    }
#else
    ParityTraceStageRangeStateHandle parityTraceStageState;
#endif

    int numThreads = opts.thread_count;
    if (numThreads <= 0) {
        numThreads = static_cast<int>(std::thread::hardware_concurrency());
        if (numThreads <= 0) numThreads = 4;
    }
    numThreads = std::max(1, std::min(numThreads, height));
    if (profile.enabled) {
        profile.width = width;
        profile.height = height;
        profile.workers = numThreads;
    }
    int batchSize = std::max(1, height / (numThreads * 4));

    TLightVals lightVals;
    std::memset(&lightVals, 0, sizeof(TLightVals));
    TLValigned alignedStorage;
    std::memset(&alignedStorage, 0, sizeof(TLValigned));
    lightVals.PLValigned = &alignedStorage;
    MakeLightValsFromHeaderLight(&header, &lightVals, 1.0, 0);
    CompleteLightValsSetup(&header, &lightVals);

    TMCTparameter mctParams = GetMCTparasFromHeader(header, &addon, 0);
    for (const auto& issue : ValidateFormulaInputs(mctParams, addon)) {
        RenderPoll();
        throw std::invalid_argument(issue.message);
    }
    if (!mctParams.bMCTisValid) {
        result.error = "Failed to setup MCT parameters for paint replay";
        return result;
    }
    ThreadPool pool(numThreads);
    CalcHSVecsFromLights(&lightVals, &mctParams);

    if (!opts.no_shadows &&
        !DebugDisablePaintReplayCompletedHardShadowState() &&
        SiLightCarriesHardShadowBits(header, silight)) {
        ApplyCompletedHardShadowStateToPaint(header, lightVals);
    }

    TIteration3Dext it3Dext;
    std::memset(&it3Dext, 0, sizeof(TIteration3Dext));
    mctParams.pIt3Dext = &it3Dext;
    IniIt3D(&mctParams, &it3Dext);
    mctParams.mPsiLight = result.silight.data();
    mctParams.pSiLight = result.silight.data();
    mctParams.PLVals = &lightVals;

    preparationTimer.complete();
    RunColorPass(pool, numThreads, batchSize, header, opts, parityTraceStageState, lightVals, result,
                 width, height, hasParityPoint, parityPoint, profile);

    RunLateImagePasses(pool, numThreads, batchSize, header, opts, addon, parityTraceStageState,
                       mctParams, lightVals, alignedStorage, result,
                       width, height, hasParityPoint, parityPoint, profile);

    RenderPoll();
    result.success = true;
    return result;
}

RenderResult PaintFromSiLight(TMandHeader10& header, THeaderCustomAddon& addon,
                              const std::vector<TsiLight5>& silight, const RenderOptions& opts) {
    return ExecuteOrdinary(opts, [&](const RenderOptions& prepared, RenderProfile& profile) {
        return PaintFromSiLightImpl(header, addon, silight, prepared, profile);
    });
}

} // namespace mb3d
