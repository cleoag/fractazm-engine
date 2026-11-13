#pragma once
#include "ao_abi.h"
#include "soft_extended.h"

#ifdef __METAL_VERSION__
#define MB3D_AO_DEVICE device
#else
#define MB3D_AO_DEVICE
#endif

namespace mb3d_metal_exact {
inline F64 ao_single(F64 value) { return F64::from_f32_bits(value.to_f32_bits()); }
inline F64 ao_s(ExactU32 bits) { return F64::from_f32_bits(bits); }
inline int ao_round(F64 value) { return E80(value).to_i32_rne(); }
inline bool ao_finite_single(ExactU32 bits) {
    return (bits & 0x7f800000u) != 0x7f800000u;
}
inline bool ao_positive_single(ExactU32 bits) {
    return (bits & 0x80000000u) == 0 && bits != 0 && ao_finite_single(bits);
}

inline bool valid_ao(const ExactAOScene scene) {
    for (unsigned component = 0; component < 64; ++component)
        if (!ao_finite_single(scene.directions[component])) return false;
    return scene.version == 1 && scene.width > 0 && scene.height > 0 &&
           scene.width <= 4096 && scene.height <= 4096 &&
           scene.num_levels >= 2 && scene.num_levels <= 15 &&
           scene.w_lo == 0 && scene.h_lo == 0 &&
           scene.w_hi == int(scene.width)-1 && scene.h_hi == int(scene.height)-1;
}

inline bool valid_ao_level(const ExactAOScene scene, const ExactAOLevel level) {
    if (!ao_positive_single(level.sit) || !ao_positive_single(level.z_threshold) ||
        !ao_positive_single(level.sum_multiplier) || !ao_positive_single(level.min_radius))
        return false;
    if (level.step_count > 6) return false;
    for (unsigned sample = 0; sample < level.step_count; ++sample)
        if (!ao_positive_single(level.reciprocal_radius[sample])) return false;
    return level.version == 1 && level.index >= 1 && level.index <= scene.num_levels &&
           level.i_step == (1u << (level.index-1)) &&
           level.step_count == (level.index == 1 ? 5u : 3u) &&
           level.max_radius > 0 && level.reserved == 0;
}

// One surface pixel's production HiQ angular scan. The legacy interior quartet
// path rounds the first lane's sample coordinates, then adds each lane offset;
// rounding independently from x would change ties. Background lanes can be
// omitted here: their angular scratch is never consumed by a surface pixel or
// the final background output.
inline bool ao_scan_pixel(const ExactAOScene scene, const ExactAOLevel level,
                          const MB3D_AO_DEVICE ExactU32* at_level,
                          MB3D_AO_DEVICE short* angles,
                          ExactU32 pixel, ExactU32 packed_z) {
    const int width = int(scene.width), height = int(scene.height);
    const int x = int(pixel % scene.width), y = int(pixel / scene.width);
    const int zp = int((packed_z & 0xffffff00u) >> 1);
    int anchor = x, lane = 0;
    if (level.index != scene.num_levels && x >= level.max_radius) {
        const int candidate = level.max_radius + ((x-level.max_radius)/4)*4;
        if (candidate < width-level.max_radius-3) { anchor = candidate; lane = x-candidate; }
    }
    const bool quartet = anchor != x || (level.index != scene.num_levels &&
        x >= level.max_radius && anchor < width-level.max_radius-3);
    for (unsigned dir = 0; dir < 32; ++dir) {
        const F64 dx = ao_s(scene.directions[2*dir]);
        const F64 dy = ao_s(scene.directions[2*dir+1]);
        F64 sx = ao_single(F64(anchor) + ao_single(dx*ao_s(level.min_radius)));
        F64 sy = ao_single(F64(y) + ao_single(dy*ao_s(level.min_radius)));
        for (unsigned sample = 0; sample < level.step_count; ++sample) {
            int x2 = ao_round(sx), y2 = ao_round(sy);
            if (!quartet) {
                if (x2 < 0) { x2 = -x2; if (x2 >= scene.w_lo) break; }
                else if (x2 >= width) { x2 = 2*(width-1)-x2; if (x2 < scene.w_hi) break; }
            }
            if (y2 < 0) { y2 = -y2; if (y2 >= scene.h_lo) break; }
            else if (y2 >= height) { y2 = 2*(height-1)-y2; if (y2 < scene.h_hi) break; }
            x2 += lane;
            if (x2 < 0 || x2 >= width || y2 < 0 || y2 >= height) return false;
            const int delta = int(at_level[y2*width+x2])-zp;
            const F64 st = ao_single(F64(delta)*ao_s(level.reciprocal_radius[sample]));
            const F64 value = st*ao_s(level.sit)/(ao_s(level.z_threshold)+abs(st));
            int angle = ao_round(value);
            if (angle > 32767) angle = 32767;
            const unsigned idx = pixel*32+dir;
            if (int(angles[idx]) < angle) angles[idx] = short(angle);
            sx = ao_single(sx + ao_single(dx*F64(level.i_step)));
            sy = ao_single(sy + ao_single(dy*F64(level.i_step)));
        }
    }
    return true;
}

// Scalar atan is prepared once for its complete finite int16 domain. The table
// is independent of image pixels and reproduces the native production libm
// value exactly; GPU performs all indexing and ordered Single accumulation.
inline ExactU32 ao_sum_pixel(const MB3D_AO_DEVICE short* angles,
                             const MB3D_AO_DEVICE ExactU64* atan_terms,
                             const ExactAOLevel level, ExactU32 pixel) {
    F64 sum;
    for (unsigned dir = 0; dir < 32; ++dir) {
        const int angle = angles[pixel*32+dir];
        if (angle > -32768)
            sum = ao_single(sum + F64::from_bits(atan_terms[angle+32768]));
    }
    F64 value = ao_single(sum*ao_s(level.sum_multiplier));
    value = min(max(value,F64(0)),F64(16383));
    return ExactU32(ao_round(value));
}
} // namespace mb3d_metal_exact
