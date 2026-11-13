#include "native_reflection_host.h"
#include "src/render/native_metal/generic_reflection_abi.h"
#include "scene_host_internal.h"
#include "core/colorizer.h"
#include "core/header_trafos.h"
#include "core/math3d.h"
#include <bit>
#include <cmath>
#include <cstring>
#include <stdexcept>

namespace mb3d::metal_exact {
namespace {
ExactU32 FloatWord(double value) {
    const float narrowed = static_cast<float>(value);
    if (!std::isfinite(value) || !std::isfinite(narrowed))
        throw std::invalid_argument("Nonfinite native reflection preparation");
    return std::bit_cast<ExactU32>(narrowed);
}

// CalcAmbientOcclusionForPoint uses a private BuildRotMatrixS. Only its third
// row, negated, is needed for the fixed undithered hemisphere directions. Keep
// its Single trigonometry and operation order, including the zero x rotation.
void AODirection(float y_angle, float z_angle, ExactU32 (&out)[4]) {
    const float sx = std::sin(0.0f), cx = std::cos(0.0f);
    const float sy = std::sin(y_angle), cy = std::cos(y_angle);
    const float sz = std::sin(z_angle), cz = std::cos(z_angle);
    out[0] = FloatWord((sx * sz - cx * sy * cz) * -1.0f);
    out[1] = FloatWord((cx * sy * sz + sx * cz) * -1.0f);
    out[2] = FloatWord((cx * cy) * -1.0f);
    out[3] = FloatWord(0.0f);
}

NativeReflectionAO PreparePointAO(unsigned quality, float max_distance) {
    NativeReflectionAO out{};
    if (quality == 0) {
        // CPU's quality 0 is its own fixed cone, not the angle formula below:
        // three rays half of 60 degrees off the normal, 120 degrees apart.
        const float cone = 60.0f * static_cast<float>(Pid180);
        out.ray_count = 3;
        for (int i = 0; i < 3; ++i)
            AODirection(0.5f * cone, static_cast<float>(i) * static_cast<float>(PiM2) * (1.0f / 3.0f),
                        out.directions[i]);
        const float multiplier = std::sqrt(out.ray_count * 0.5f);
        out.step_multiplier = FloatWord(1.8f);
        out.de_multiplier = FloatWord(multiplier);
        out.correction_weight = FloatWord(0.3f);
        out.max_distance_base = FloatWord(max_distance);
        out.minimum_overlap_dot = FloatWord(-1.0f);
        out.overlap_angle_scale = FloatWord(1.2f / static_cast<float>(ArcSinSafe(1.0 / multiplier)));
        return out;
    }
    // Reflection explicitly selects AOdither=0: dither1=.25, dither2=0.
    // Quality 2 is seven rays; quality 1 is three rays, independent of pixels.
    const float angle = static_cast<float>(Pi) * 0.5f / (quality + 0.9f);
    const int count = static_cast<int>(std::lround(std::sin(0.25) * PiM2 / angle));
    out.ray_count = 1;
    AODirection(0.0f, 0.0f, out.directions[0]);
    for (unsigned row = 1; row <= quality; ++row) {
        for (int column = 0; column < count; ++column) {
            if (out.ray_count >= 8)
                throw std::invalid_argument("Unsupported native reflection AO ray count");
            AODirection(angle * static_cast<float>(row),
                        static_cast<float>(column * PiM2 / count),
                        out.directions[out.ray_count++]);
        }
    }
    if (out.ray_count != (quality == 2 ? 7u : 3u))
        throw std::invalid_argument("Unexpected native reflection AO preparation");
    const float multiplier = std::sqrt(out.ray_count * 0.5f);
    out.step_multiplier = FloatWord(1.0f + std::sin(angle));
    out.de_multiplier = FloatWord(multiplier);
    out.correction_weight = FloatWord(quality == 1 ? 0.2f : 0.1666f);
    out.max_distance_base = FloatWord(max_distance);
    out.minimum_overlap_dot = FloatWord(std::cos(angle * 1.2f));
    out.overlap_angle_scale = FloatWord(1.2f / static_cast<float>(ArcSinSafe(1.0 / multiplier)));
    return out;
}

void ValidateOpaqueLights(const TMandHeader10& header,bool native_color_options=false) {
    // Reflection's Svec shading uses the same admitted color branch as the
    // primary GPU color pass. Check flags here too so this preparer cannot
    // silently admit transparency, maps, gamma, visible/positional lights or
    // a different ambient/background model when used independently.
    auto copy = header;
    TLightVals light{};
    TLValigned aligned{};
    light.PLValigned = &aligned;
    MakeLightValsFromHeaderLight(&copy, &light, 1.0, 0);
    CompleteLightValsSetup(&copy, &light);
    // The Native camera-aware material also shades squared color, gamma,
    // depth-gradient functions, CalcTotalLight2, diffuse/light/background maps
    // and several positional lights; the general color preparation validates
    // those descriptors.
    const bool native = native_color_options;
    if ((light.bCalcPixColSqr&&!native) || (light.bAmbRelObj&&!native) || (light.bBackBMP&&!native) ||
        (light.bUseSmallBGpicForAmb&&!native) || light.bVolLight || (light.bYCcomb&&!native) ||
        (light.iGammaH&&!native) || (light.iDfunc&&!native) || (light.iExModes&&!native) ||
        (native ? (light.bDFogOptions&~3) :
            (light.iColOnOT||!light.bColCycling||light.bNoColIpol||!light.bFarFog||light.bDFogOptions)) ||
        (light.DiffColLightMap&&!native) || (light.BGLightMap&&!native) || (light.BGsmallLM&&!native))
        throw std::invalid_argument("Unsupported native reflection color capability");
    for (unsigned index = 0; index < 6; ++index) {
        if (native ? (light.iLightOption[index] < 0 || light.iLightOption[index] > 2 ||
                      (light.iLightOption[index] == 2) != (light.LLightMaps[index] != nullptr) ||
                      (light.iLightPos[index] & ~15)) :
            (light.iLightOption[index] != (index == 0 ? 0 : 1) ||
             light.iLightPos[index] || light.LLightMaps[index]))
            throw std::invalid_argument("Unsupported native reflection light capability");
    }
}
}

static NativeReflectionParams PrepareNativeReflectionImpl(const PreparedExactScene& scene,
                                              unsigned max_steps,bool general_formula,bool camera_inside=false) {
    if (!scene.host)
        throw std::invalid_argument("Missing native reflection scene owner");
    const auto& header = scene.host->file.header;
    if (!(header.bCalcSRautomatic & 1)) return {};
    if (scene.precision != PrecisionMode::Native || !max_steps || max_steps > 65536)
        throw std::invalid_argument("Invalid native reflection profile or work bound");
    ScopedRenderContext binding(scene.host->context);
    const ScopedNumericPrecision precision(PrecisionMode::Native);
    const auto& m = scene.host->mct;
    const bool formula_allowed=general_formula?
        (scene.device.formula_kind==4&&scene.device.capability_id==0x4e415431u):
        (scene.device.formula_kind==3&&scene.device.bulb.power==8);
    // A generic orbit can start inside even for an outside-only scene. The
    // no-cut CPU branch skips that secondary pass when Zend is greater than
    // one; nearer interior reflection material is not implemented yet.
    if (!formula_allowed || (camera_inside&&!general_formula) || (general_formula && !camera_inside && !(m.Zend > 1.0)) ||
        // Bit 4 (only dIFS transmits) selects transmitting surfaces; without
        // transmission (bit 2) SR_CalcRay never reads it. Camera reflections
        // trace transmission; the inside configuration validates its start.
        (header.bCalcSRautomatic & ~(camera_inside ? 6u : 4u)) != 1 ||
        header.SRreflectioncount < 1 ||
        header.SRreflectioncount > (camera_inside ? mb3d_native::kNativeReflectionMaxBounces : 2u) ||
        // The M3P loader bounds SRamount to [0,100]; above one it amplifies.
        !std::isfinite(header.SRamount) || header.SRamount < 0 || header.SRamount > (camera_inside ? 100.0f : 1.0f) ||
        // Camera reflections march CalcHSsoftPoint with the primary soft
        // descriptor; the plan requires that pass.
        ((header.bCalc1HSsoft & 1) && !camera_inside) || (header.bVolLightNr & 7) ||
        (!camera_inside&&(header.bPlanarOptic || header.TilingOptions || header.bStereoMode)) ||
        !m.bMCTisValid || (!general_formula&&m.DEoption != 0) || (m.FormulaType&&!camera_inside) ||
        (!camera_inside&&(m.MCTCameraOptic || m.iCutOptions || m.bInsideRendering || m.bCalcInside || m.bInAndOutside)) ||
        (!general_formula&&m.bIsIFS) || (m.bMCTFirstStepRandom && !(general_formula&&camera_inside)) || (camera_inside ? m.DFogOnIt>255 : m.DFogOnIt!=0) ||
        (!general_formula&&(!m.NormalsOnDE||m.iSmNormals)) || (m.ColorOnIt&&!general_formula) ||
        (m.ColorOption && (!(general_formula&&camera_inside) || m.ColorOption!=1 || m.DEoption==20)) ||
        // SR_CalcRay stops only on a DE of at least iMinIt iterations.
        (camera_inside ? (m.iMinIt < 1 || m.iMinIt > m.iMaxIt) : m.iMinIt != 1) ||
        (general_formula ? !mb3d_native::valid_general_primary_limits(
            static_cast<unsigned>(m.iDEAddSteps),static_cast<unsigned>(m.iMaxIt)) : m.iDEAddSteps != 8) ||
        !(m.StepWidth > 0) || !(m.FOVy > 0) ||
        !(m.Zcorr > 0) || !(m.ZcMul > 0) || !(m.DEstop > 0))
        throw std::invalid_argument("Unsupported native opaque reflection capability");
    ValidateOpaqueLights(header,camera_inside);

    NativeReflectionParams out{};
    out.version = 1;
    out.width = scene.device.width;
    out.height = scene.device.height;
    out.max_bounces = header.SRreflectioncount;
    out.max_steps = max_steps;
    // SR_CalcRay marches a secondary ray until it passes its maximum length,
    // with no step cap; near-surface rays of slow scenes take hundreds of
    // thousands of steps. Camera reflections get the secondary step budget.
    if (camera_inside) out.max_steps = mb3d_native::kGeneralMaxSecondarySteps;
    out.vary_de_stop = m.bVaryDEstop != 0;
    out.ao_enabled = m.bCalcAmbShadow != 0;
    out.shadows_enabled = (header.bCalculateHardShadow & 1) != 0;
    out.first_step_random = m.bMCTFirstStepRandom != 0;
    out.amount = FloatWord(header.SRamount);
    out.step_width = FloatWord(m.StepWidth);
    out.fov_y = FloatWord(m.FOVy);
    out.z_mid_difference = FloatWord(m.sZZstmitDif);
    out.z_corr = FloatWord(m.Zcorr);
    out.z_c_mul = FloatWord(m.ZcMul);
    const double ratio = 8388352.0 / m.ZcMul;
    out.back_distance = FloatWord((std::pow(ratio + 1.0, 2.0) - 1.0) * m.StepWidth / m.Zcorr);

    TMatrix3 gradients{};
    std::memcpy(gradients, m.Vgrads, sizeof(gradients));
    TSMatrix3 view{};
    NormaliseMatrixToS(view, 1.0f, &gradients);
    for (unsigned row = 0; row < 3; ++row) {
        const double length = std::sqrt(gradients[row][0] * gradients[row][0] +
                                       gradients[row][1] * gradients[row][1] +
                                       gradients[row][2] * gradients[row][2]);
        if (!(length > 1e-30) || !std::isfinite(length))
            throw std::invalid_argument("Invalid native reflection camera matrix");
        // This is the private CalcAmbientOcclusionForPoint normalization:
        // its scale starts from a Single StepWidth, unlike the view matrix.
        const double scale = static_cast<float>(m.StepWidth) / length;
        for (unsigned column = 0; column < 3; ++column) {
            out.view_matrix[row][column] = FloatWord(view[row][column]);
            out.ao_matrix[row][column] = FloatWord(gradients[row][column] * scale);
        }
    }
    // Point AO is sized against the original full frame, not the allocated tile.
    if(camera_inside&&(m.iMandWidth<1||m.iMandWidth>4096||m.iMandHeight<1||m.iMandHeight>4096))
        throw std::invalid_argument("Native camera reflection full extent outside bounds");
    const unsigned fullWidth=camera_inside?unsigned(m.iMandWidth):out.width;
    const unsigned fullHeight=camera_inside?unsigned(m.iMandHeight):out.height;
    const float max_distance = m.DEAOmaxL * 0.5f *
        std::sqrt(static_cast<float>(fullWidth*fullWidth+fullHeight*fullHeight));
    if (out.ao_enabled && (!(max_distance > 0) || !std::isfinite(max_distance)))
        throw std::invalid_argument("Invalid native reflection AO distance");
    out.ao[0] = PreparePointAO(2, max_distance);
    out.ao[1] = PreparePointAO(1, max_distance);
    out.ao[2] = PreparePointAO(0, max_distance);
    return out;
}
NativeReflectionParams PrepareNativeReflection(const PreparedExactScene& scene,unsigned max_steps) {
    return PrepareNativeReflectionImpl(scene,max_steps,false);
}
NativeReflectionParams PrepareGeneralNativeReflection(const PreparedExactScene& scene,unsigned max_steps,bool camera_inside) {
    return PrepareNativeReflectionImpl(scene,max_steps,true,camera_inside);
}
}
