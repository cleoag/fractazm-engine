#include "scene_host_internal.h"
#include "core/external_formulas.h"
#include "core/formula_registry.h"
#include "core/math3d.h"
#include "core/ray_marcher.h"
#include "core/render_context.h"
#include "core/extended_transcendentals.h"
#include "io/m3p_parser.h"
#include <CommonCrypto/CommonDigest.h>
#include <algorithm>
#include <bit>
#include <cmath>
#include <cstring>
#include <fstream>
#include <future>
#include <iomanip>
#include <sstream>
#include <stdexcept>

namespace mb3d::metal_exact {

static uint64_t bits(double value) { return std::bit_cast<uint64_t>(value); }
static uint32_t bits(float value) { return std::bit_cast<uint32_t>(value); }

// One compatible transcendental evaluation per row/column. The GPU retains
// responsibility for every ray's extended build, normalization and rotation.
static ExactTrig CameraTrig(double angle) {
    if (NativePrecisionEnabled())
        return {bits(std::sin(angle)),bits(std::cos(angle)),0,0,0,0,{0,0}};
    const numeric::ExtendedScope arithmetic;
    const auto value=numeric::extendedSinCos(numeric::Extended80(angle));
    return {bits(double(value.sin)),bits(double(value.cos)),
            value.sin.significand(),value.cos.significand(),
            value.sin.signExponent(),value.cos.signExponent(),{0,0}};
}

static void ValidateScenePasses(const TMandHeader10& header, bool native_reflections = false) {
    if (!header.bCalc3D || header.TilingOptions || header.bStereoMode ||
        (header.byCalcNsOnZBufAuto & 1) ||
        ((header.bCalcSRautomatic & 1) && !native_reflections) ||
        (header.bCalcDOFtype & 1) || (header.bVolLightNr & 7))
        throw std::invalid_argument("Unsupported exact Metal scene pass or camera mode");
}

static std::string FileHash(const std::string& path) {
    std::ifstream input(path, std::ios::binary);
    if (!input) throw std::runtime_error("Cannot open M3P scene: " + path);
    // These capabilities accept only small parameter files, never an
    // arbitrarily large image cache or an input silently normalized to it.
    input.seekg(0, std::ios::end);
    const auto size = input.tellg();
    if (size < 0 || size > 1048576) throw std::runtime_error("Unsupported M3P file size");
    input.seekg(0, std::ios::beg);
    std::vector<unsigned char> data(static_cast<size_t>(size));
    if (!input.read(reinterpret_cast<char*>(data.data()), size))
        throw std::runtime_error("Cannot read complete M3P scene");
    // Current raw M3P headers retain all these flags. Check before loading:
    // the legacy loader clears stereo state and validates tiling cache extents.
    // Older/compressed inputs still receive the prepared-header check below.
    if (data.size() >= sizeof(TMandHeader10)) {
        TMandHeader10 raw;
        std::memcpy(&raw, data.data(), sizeof(raw));
        if (raw.MandId >= 35 && raw.MandId <= 250) ValidateScenePasses(raw);
    }
    unsigned char digest[CC_SHA256_DIGEST_LENGTH];
    CC_SHA256(data.data(), static_cast<CC_LONG>(data.size()), digest);
    std::ostringstream out;
    out << std::hex << std::setfill('0');
    for (auto c : digest) out << std::setw(2) << unsigned(c);
    return out.str();
}

static PreparedExactScene PrepareScene(const std::string& path, const std::string& source_hash,
                                      const M3PLoadResult* loaded, const std::string& resource_root,
                                      unsigned width, unsigned height, unsigned max_steps,
                                      bool hard_shadows, PrecisionMode mode) {
    const ScopedNumericPrecision precision(mode);
    if (width == 0 || height == 0 || width > 4096 || height > 4096 ||
        max_steps == 0 || max_steps > 65536)
        throw std::invalid_argument("Unsupported exact Metal dimensions or work limit");
    PreparedExactScene result;
    result.precision = mode;
    result.source_path = path;
    result.source_sha256 = source_hash;
    const auto* capability = FindExactSceneCapability(result.source_sha256);
    result.host = std::make_shared<SceneHostState>();
    auto& host = *result.host;
    host.context = std::make_shared<RenderContext>(resource_root);
    if (loaded) {
        const auto caller = CaptureRenderContext();
        host.context->maps.sequences = caller->maps.sequences;
        host.context->maps.frame_number = caller->maps.frame_number;
    }
    host.context->precision = mode;
    ScopedRenderContext binding(host.context);
    host.file = loaded ? *loaded : LoadM3PFile(path);
    if (!host.file.success) throw std::runtime_error(host.file.error);
    const auto* name = reinterpret_cast<const char*>(host.file.addon.Formulas[0].CustomFname);
    result.formula_name.assign(name, strnlen(name, sizeof(host.file.addon.Formulas[0].CustomFname)));
    result.original_shadow_bits = host.file.header.bCalculateHardShadow;
    unsigned formula_kind = 0, capability_id = 0, internal_power = 0;
    if (capability) {
        if (result.original_shadow_bits != capability->original_shadow_bits ||
            result.formula_name != capability->formula_name)
            throw std::invalid_argument("Loaded scene identity differs from its declared exact capability");
        result.scene_id = capability->scene_id;
        formula_kind = capability->formula_kind;
        capability_id = capability->capability_id;
    } else if (host.file.addon.Formulas[0].iFnr == 0) {
        const double requested_power = host.file.addon.Formulas[0].dOptionValue[0];
        // Validate before production preparation converts this option to int.
        if (!(requested_power >= 2 && requested_power <= 8))
            throw std::invalid_argument("Unsupported exact Integer Power exponent");
        internal_power = unsigned(requested_power);
        if (double(internal_power) != requested_power)
            throw std::invalid_argument("Unsupported noninteger exact Integer Power exponent");
        result.scene_id = "integer_power_runtime";
        result.formula_name = "IntegerPower";
        formula_kind = 3; capability_id = mb3d_metal_exact::kRuntimeBulbCapability;
    } else if (result.formula_name == "SphereIFS") {
        result.scene_id = "sphereifs_runtime";
        formula_kind = 1; capability_id = mb3d_metal_exact::kRuntimeSphereCapability;
    } else if (result.formula_name == "boxIFS") {
        result.scene_id = "boxifs_runtime";
        formula_kind = 2; capability_id = mb3d_metal_exact::kRuntimeBoxCapability;
    } else {
        throw std::invalid_argument("Unsupported exact Metal formula: " + result.formula_name);
    }
    // These production passes are outside the implemented GPU pipeline. The
    // source catalog used to imply their absence; runtime admission must check
    // them explicitly before accepting a camera or coefficient variation.
    // Only the main Native adapter dispatches the restricted reflection pass.
    // File-based standalone qualification/benchmark preparation still rejects
    // reflection scenes, including its Native mode, before any GPU work.
    ValidateScenePasses(host.file.header, loaded && mode == PrecisionMode::Native);
    host.file.header.Width = static_cast<int32_t>(width);
    host.file.header.Height = static_cast<int32_t>(height);
    if (hard_shadows) host.file.header.bCalculateHardShadow |= 1;
    host.mct = GetMCTparasFromHeader(host.file.header, &host.file.addon, 0);
    const auto& m = host.mct;
    if (!m.bMCTisValid || m.DEoption != (formula_kind == 3 ? 0 : 20) || m.FormulaType != 0 ||
        m.MCTCameraOptic != 0 || m.iCutOptions != 0 || m.bInsideRendering ||
        m.bInAndOutside || m.bMCTFirstStepRandom || m.DFogOnIt || !m.NormalsOnDE ||
        m.iSmNormals != 0 || m.StartFrom1 != 0 || m.wEndTo != 0 ||
        m.RepeatFrom1 != 0 || m.nHybrid[0] < 1 || m.nHybrid[0] > 65535 ||
        m.iMaxIt < 1 || m.iMaxIt > 256 ||
        (capability && (m.nHybrid[0] != static_cast<int>(capability->slot_count) ||
                        m.iMaxIt != static_cast<int>(capability->max_iterations))) ||
        m.iMinIt != 1 || m.iDEAddSteps != 8 || m.bDoJulia ||
        (formula_kind == 3 ? (m.CalcDE != CalcDEnoADE || m.mMandFunction != doHybridPas) :
                            (m.CalcDE != CalcDEanalytic || m.mMandFunctionDE != doHybridIFS3D)))
        throw std::invalid_argument("Prepared " + result.scene_id + " differs from its declared primary-stage contract");
    for (unsigned slot = 1; slot < 6; ++slot)
        if (m.nHybrid[slot] != 0) throw std::runtime_error("Unsupported additional formula slot");

    auto& d = result.device;
    d.version = 4; d.width = width; d.height = height; d.max_steps = max_steps;
    d.formula_kind = formula_kind; d.capability_id = capability_id;
    d.max_iterations = m.iMaxIt; d.slot_count = m.nHybrid[0];
    d.min_iterations = m.iMinIt; d.refinement_steps = m.iDEAddSteps;
    if (formula_kind == 3) {
        const unsigned power = internal_power;
        if (m.fHybrid[0] != reinterpret_cast<ThybridIteration2>(GetInternalFormula(0, power, false)) ||
            !m.fHPVar[0])
            throw std::invalid_argument("Prepared internal formula does not match its static metadata");
        const int offsets[8] = {-16, 24, 32, 144, 152, 160, 192, 208};
        const auto* constants = static_cast<const unsigned char*>(m.fHPVar[0]);
        for (unsigned index = 0; index < 8; ++index)
            std::memcpy(&d.params[index], constants + offsets[index], sizeof(ExactU64));
        d.bulb.bailout = bits(double(m.dRStop));
        d.bulb.gradient_bailout = bits(double(m.Rstop3D));
        d.bulb.de_offset = bits(double(m.mctDEoffset));
        d.bulb.de_offset006 = bits(double(m.mctDEoffset006));
        d.bulb.smooth_hln = bits(m.fHln[0]);
        d.bulb.smooth_lnrstop = bits(m.dLNRStop);
        d.bulb.power = power;
    } else {
        const auto* formula = FindExternalFormula(result.formula_name.c_str());
        if (!formula || !formula->paramMap || formula->DEoption != 20 || formula->paramCount != 13 ||
            m.fHybrid[0] != reinterpret_cast<ThybridIteration2>(formula->wrapperFunc))
            throw std::runtime_error("Prepared formula does not match its declared static metadata");
        double params[MAX_FORMULA_PARAMS]{};
        if (ReadFormulaParams(m.fHPVar[0], *formula->paramMap, params) != 13)
            throw std::runtime_error("Unexpected prepared " + result.formula_name + " constant layout");
        for (unsigned i = 0; i < 13; ++i) {
            if (capability && bits(params[i]) != capability->params[i])
                throw std::runtime_error("Unexpected prepared " + result.formula_name + " coefficient " + std::to_string(i));
            d.params[i] = bits(params[i]);
        }
    }
    if (!mb3d_metal_exact::valid_formula_scene(d))
        throw std::invalid_argument("Prepared formula fails the shared device capability contract");
    for (unsigned i = 0; i < 3; ++i) {
        d.y_start[i] = bits(m.Ystart[i]);
        for (unsigned j = 0; j < 3; ++j) d.vgrads[i*3+j] = bits(m.Vgrads[i][j]);
    }
    d.max_ray_length = bits(m.Zend);
    d.de_stop = bits(m.DEstop); d.de_stop_factor = bits(m.mctDEstopFactor);
    d.z_step_div = bits(m.sZstepDiv); d.step_limiter = bits(m.mctMH04ZSD);
    d.de_scale = bits(m.dDEscale); d.step_width_103 = bits(m.sStepWm103);
    d.de_sub = bits(m.msDEsub);

    result.columns.resize(width);
    result.rows.resize(height);
    for (unsigned x = 0; x < width; ++x) {
        const double angle = (double(m.FOVXoff) - double(x + 1)) * double(m.FOVXmul);
        result.columns[x] = CameraTrig(angle);
    }
    for (unsigned y = 0; y < height; ++y) {
        const double angle = CalcCameraRowAngle(y, m.iMandHeight, m.FOVy);
        result.rows[y] = CameraTrig(angle);
    }
    return result;
}

PreparedExactScene PrepareExactScene(const std::string& path, unsigned width,
                                    unsigned height, unsigned max_steps, bool hard_shadows,
                                    PrecisionMode mode) {
    return PrepareScene(path, FileHash(path), nullptr, {}, width, height, max_steps,
                        hard_shadows, mode);
}

PreparedExactScene PrepareLoadedExactScene(const TMandHeader10& header,
                                         const THeaderCustomAddon& addon,
                                         unsigned width, unsigned height,
                                         const std::string& resource_root,
                                         unsigned max_steps, PrecisionMode mode) {
    M3PLoadResult loaded{};
    loaded.header = header;
    loaded.addon = addon;
    loaded.success = true;
    return PrepareScene({}, {}, &loaded, resource_root, width, height, max_steps,
                        false, mode);
}

static void RawNormalReference(TMCTparameter m, TIteration3Dext it, ExactPixel& out) {
    m.pIt3Dext = &it;
    const double offset = MinCS(1.0f, m.DEstop) * (1.0 + m.mZZ * m.mctDEstopFactor) * 0.15;
    const TVec3D origin{it.C1, it.C2, it.C3};
    it.CalcSIT = 1;
    m.CalcDE(&it, &m);
    it.CalcSIT = 0;
    // Same six production DE probes, independently of the device formula.
    for (const unsigned axis : {2u, 0u, 1u}) {
        TVec3D pos;
        mCopyAddVecWeight(pos, origin, m.Vgrads[axis], offset);
        it.C1 = pos[0]; it.C2 = pos[1]; it.C3 = pos[2];
        const double plus = m.CalcDE(&it, &m);
        mAddVecWeight(pos, m.Vgrads[axis], -2.0 * offset);
        it.C1 = pos[0]; it.C2 = pos[1]; it.C3 = pos[2];
        const double minus = m.CalcDE(&it, &m);
        out.raw_normal[axis] = bits((plus - minus) * 0.5);
    }
}

std::vector<ExactPixel> ReferencePrimary(const PreparedExactScene& scene, unsigned threads) {
    if (scene.precision != PrecisionMode::Compatible)
        throw std::invalid_argument("Exact CPU oracle requires compatible precision");
    if (!scene.host) throw std::invalid_argument("Missing prepared CPU scene owner");
    threads = std::clamp(threads, 1u, scene.device.height);
    std::vector<ExactPixel> output(size_t(scene.device.width) * scene.device.height);
    std::vector<std::future<void>> workers;
    for (unsigned lane = 0; lane < threads; ++lane) {
        workers.emplace_back(std::async(std::launch::async, [&, lane] {
            ScopedRenderContext binding(scene.host->context);
            const ScopedNumericPrecision precision(PrecisionMode::Compatible);
            TMCTparameter m = scene.host->mct;
            TIteration3Dext it{};
            m.pIt3Dext = &it; m.iThreadId = lane+1; m.iThreadCount = threads;
            IniIt3D(&m, &it);
            TRaymarchRec ray{};
            ray.PMCTparas = &m; ray.PIt3Dex = &it;
            for (unsigned y = lane; y < scene.device.height; y += threads) {
                m.CAFY = CalcCameraRowAngle(y, m.iMandHeight, m.FOVy);
                for (unsigned x = 0; x < scene.device.width; ++x) {
                    auto& out = output[size_t(y)*scene.device.width+x];
                    RMCalculateVgradsFOV(&m, x+1);
                    RMCalculateStartPos(&m, x, y);
                    m.msDEstop = m.DEstop; m.mZZ = 0; it.CalcSIT = 0;
                    ray.ActZpos = 0; ray.StartDEstop = m.DEstop;
                    ray.MaxRayLength = m.Zend; ray.BinSteps = m.iDEAddSteps;
                    ray.RMresult = 0; ray.StepCount = 0; ray.StepForward = 0;
                    ray.Zstepped = 0; ray.finalDE = 0;
                    ray.DEmulConst = 1; ray.DEmulVary = 0; ray.ZZposMul = 1;
                    std::memcpy(ray.VievVec, m.mVgradsFOV, sizeof(TVec3D));
                    std::memcpy(ray.MarchVec, m.mVgradsFOV, sizeof(TVec3D));
                    const double start[] = {it.C1,it.C2,it.C3};
                    for (unsigned i = 0; i < 3; ++i) {
                        out.ray[i] = bits(ray.VievVec[i]); out.start[i] = bits(start[i]);
                    }
                    RayMarch(&ray);
                    for (unsigned i = 0; i < 3; ++i) out.position[i] = bits(ray.ActPos[i]);
                    out.depth = bits(ray.Zstepped); out.axial_depth = bits(ray.ActZpos);
                    out.final_de = bits(ray.finalDE); out.rm_result = ray.RMresult;
                    out.start_inside = ray.StartInside; out.step_count = bits(ray.StepCount);
                    out.final_iterations = it.ItResultI;
                    out.final_stop = bits(m.msDEstop);
                    if (ray.RMresult > 0 && !ray.StartInside) {
                        it.C1 = ray.ActPos[0]; it.C2 = ray.ActPos[1]; it.C3 = ray.ActPos[2];
                        m.mZZ = ray.Zstepped;
                        RawNormalReference(m, it, out);
                        TsiLight5 light{}; m.mPsiLight = &light;
                        float smooth = 0; RMCalculateNormals(&m, smooth);
                        out.smooth_iterations = bits(smooth);
                        // Production color consumes the orbit left by the last
                        // Y-minus normal probe; only its input position is
                        // restored by RMCalculateNormals.
                        out.orbit_trap = bits(it.OTrap);
                        out.normal[0] = light.NormalX;
                        out.normal[1] = light.NormalY;
                        out.normal[2] = light.NormalZ;
                    }
                }
            }
        }));
    }
    for (auto& task : workers) task.get();
    return output;
}
} // namespace mb3d::metal_exact
