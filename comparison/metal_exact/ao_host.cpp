#include "ao_host.h"
#include "ao_math.h"
#include "scene_host_internal.h"
#include "shadow_host.h"
#include "core/ambient_occlusion.h"
#include "core/header_trafos.h"
#include "core/math3d.h"
#include <algorithm>
#include <bit>
#include <cmath>
#include <stdexcept>

namespace mb3d::metal_exact {
static uint32_t bits(float value) { return std::bit_cast<uint32_t>(value); }
static int rounded(double value) { return static_cast<int>(std::nearbyint(value)); }

PreparedExactAO PrepareExactAO(const PreparedExactScene& scene, unsigned) {
    if (!scene.host) throw std::invalid_argument("Missing prepared exact AO scene");
    ScopedRenderContext binding(scene.host->context);
    const auto& header = scene.host->file.header;
    if (header.bCalcAmbShadowAutomatic != 21 || header.SSAORcount != 1 ||
        header.bSSAO24BorderMirrorSize != 0 || header.bPlanarOptic != 0 ||
        header.sAmbShadowThreshold != 2.0f || !scene.host->mct.bCalcAmbShadow)
        throw std::invalid_argument("Unsupported exact AO: requires pinned standard HiQ parameters");
    PreparedExactAO out;
    auto& d = out.device;
    d.version = 1; d.width = scene.device.width; d.height = scene.device.height;
    const int width = int(d.width), height = int(d.height);
    const int radius = rounded(std::sqrt(float(width*width+height*height))*0.5f);
    int levels = 1, bound = 5;
    do { ++levels; bound <<= 1; } while (levels < 15 && bound <= radius);
    d.num_levels = unsigned(levels);
    d.w_hi = width-1; d.h_hi = height-1;
    for (unsigned direction = 0; direction < 32; ++direction) {
        double s, c;
        SinCosD(double(direction)*Pi/16.0, s, c);
        d.directions[2*direction] = bits(float(s));
        d.directions[2*direction+1] = bits(float(c));
    }
    TMandHeader10 copy = header;
    double z_corr, z_c_mul, zz_difference;
    CalcPPZvals(copy,z_corr,z_c_mul,zz_difference);
    const float z_scale = float((std::pow(256.0/z_c_mul+1.0,2.0)-1.0)/z_corr);
    for (int index = 1; index <= levels; ++index) {
        ExactAOLevel level{};
        level.version = 1; level.index = index;
        float sit = z_scale/22000.0f*4096.0f;
        const double level_factor = std::sqrt(std::sqrt(double(levels)/double(index)));
        const float threshold = float(double(header.sAmbShadowThreshold)*0.7*4096.0/double(sit)*level_factor);
        const double max_angle = std::atan(double(header.sAmbShadowThreshold)*0.6*std::sqrt(std::sqrt(double(levels))));
        const float sum_multiplier = float(1.5*32767.0/(double(Pi)*32.0*std::pow(max_angle,0.9)));
        level.i_step = 1u << (index-1);
        const float min_radius = level.i_step < 2 ? 1.0f : 3.25f*level.i_step;
        level.step_count = level.i_step < 2 ? 5u : 3u;
        level.max_radius = rounded(double(min_radius)+double(level.i_step*level.step_count)+0.1);
        level.min_radius = bits(min_radius);
        for (unsigned sample = 0; sample < level.step_count; ++sample)
            level.reciprocal_radius[sample] = bits(float(1.0/(double(min_radius)+double(sample*level.i_step)+0.1)));
        level.sit = bits(sit*threshold);
        level.z_threshold = bits(threshold);
        level.sum_multiplier = bits(sum_multiplier);
        out.levels.push_back(level);
    }
    if (!mb3d_metal_exact::valid_ao(out.device))
        throw std::invalid_argument("Invalid prepared exact AO scalar directions");
    for (const auto& level : out.levels)
        if (!mb3d_metal_exact::valid_ao_level(out.device,level))
            throw std::invalid_argument("Invalid prepared exact AO level coefficients");
    // Complete int16 domain, independent of image contents or resolution. This
    // exactly matches CalcAOHiQSumPasLike's native binary64 argument and libm.
    out.atan_terms.resize(65536);
    for (int angle = -32768; angle <= 32767; ++angle)
        out.atan_terms[angle+32768] = std::bit_cast<uint64_t>(std::atan(double(angle)*0.0002441406));
    return out;
}

std::vector<ExactSurface> ReferenceAO(const PreparedExactScene& scene,
                                    const std::vector<ExactSurface>& surface,
                                    unsigned threads) {
    if (!scene.host || surface.size() != size_t(scene.device.width)*scene.device.height)
        throw std::invalid_argument("Invalid exact AO reference input");
    (void)PrepareExactAO(scene,threads); // Fail closed on unsupported scene state.
    ScopedRenderContext binding(scene.host->context);
    std::vector<TsiLight5> packed;
    packed.reserve(surface.size());
    for (const auto& entry : surface) {
        if (entry.fault) throw std::invalid_argument("Faulted exact AO reference surface");
        packed.push_back(ImportSurface(entry));
    }
    auto mct = scene.host->mct;
    auto header = scene.host->file.header;
    ReplayAmbientOcclusion(packed.data(),int(scene.device.width),int(scene.device.height),
        int(scene.device.width),&mct,nullptr,(header.bCalcAmbShadowAutomatic>>2)&3,
        (header.bCalcAmbShadowAutomatic>>4)&3,&header,int(threads));
    std::vector<ExactSurface> result;
    result.reserve(packed.size());
    for (const auto& entry : packed) result.push_back(ExportSurface(entry));
    return result;
}

std::vector<ExactSurface> SoftwareAO(const PreparedExactAO& ao,
                                   const std::vector<ExactSurface>& surface) {
    using namespace mb3d_metal_exact;
    if (!valid_ao(ao.device) || surface.size() != size_t(ao.device.width)*ao.device.height ||
        ao.levels.size() != ao.device.num_levels || ao.atan_terms.size() != 65536)
        throw std::invalid_argument("Invalid software AO input");
    auto result = surface;
    std::vector<uint32_t> depth(surface.size()), temporary(surface.size());
    std::vector<short> angles(surface.size()*32,short(-32768));
    for (size_t pixel = 0; pixel < surface.size(); ++pixel) {
        if (surface[pixel].fault) throw std::invalid_argument("Faulted software AO surface");
        depth[pixel] = surface[pixel].depth_packed & 0x80000000u ? 0 :
                      (surface[pixel].depth_packed & 0xffffff00u)>>1;
    }
    const unsigned width = ao.device.width, height = ao.device.height;
    for (const auto& level : ao.levels) {
        if (!valid_ao_level(ao.device,level)) throw std::invalid_argument("Invalid software AO level");
        for (unsigned pixel = 0; pixel < surface.size(); ++pixel) {
            if (surface[pixel].depth_packed & 0x80000000u) continue;
            if (!ao_scan_pixel(ao.device,level,depth.data(),angles.data(),pixel,surface[pixel].depth_packed))
                throw std::runtime_error("AO sample outside supported image");
            if (level.index == ao.device.num_levels)
                result[pixel].amb_shadow = ao_sum_pixel(angles.data(),ao.atan_terms.data(),level,pixel);
        }
        if (level.index < ao.device.num_levels) {
            const int step = 1 << level.index;
            for (unsigned y = 0; y < height; ++y)
                for (unsigned x = 0; x < width; ++x) {
                    const int a = std::max(0,int(x)-step), b = std::min(int(width)-1,int(x)+step);
                    temporary[y*width+x] = (depth[y*width+x]+(depth[y*width+a]+depth[y*width+b])/2)/2;
                }
            for (unsigned y = 0; y < height; ++y)
                for (unsigned x = 0; x < width; ++x) {
                    const int a = std::max(0,int(y)-step), b = std::min(int(height)-1,int(y)+step);
                    depth[y*width+x] = (temporary[y*width+x]+(temporary[a*width+x]+temporary[b*width+x])/2)/2;
                }
        }
    }
    return result;
}
} // namespace mb3d::metal_exact
