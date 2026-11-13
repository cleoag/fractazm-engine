#pragma once
#include "soft_double.h"
#include "soft_extended.h"
#include "scene_abi.h"

namespace mb3d_metal_exact {
#ifdef __METAL_VERSION__
// Specialize the shared traversal at pipeline creation; the actual descriptor
// is still validated against these selectors before any scene work executes.
constant ExactU32 exact_formula_selection [[function_constant(0)]];
constant ExactU32 exact_bulb_power_selection [[function_constant(1)]];
constant ExactU32 exact_capability_selection [[function_constant(2)]];
inline ExactU32 selected_formula(ExactScene) { return exact_formula_selection; }
inline ExactU32 selected_bulb_power(ExactScene) { return exact_bulb_power_selection; }
inline ExactU32 selected_capability(ExactScene) { return exact_capability_selection; }
#else
inline ExactU32 selected_formula(ExactScene s) { return s.formula_kind; }
inline ExactU32 selected_bulb_power(ExactScene s) { return s.bulb.power; }
inline ExactU32 selected_capability(ExactScene s) { return s.capability_id; }
#endif
inline F64 D(ExactU64 b) { return F64::from_bits(b); }
inline F64 S(ExactU32 b) { return F64::from_f32_bits(b); }
inline F64 single(F64 v) { return S(v.to_f32_bits()); }
inline F64 pasmax(F64 a,F64 b) { return a>b?a:b; }
inline F64 clamp0(F64 a) { return a<=F64(0)?F64(0):a; }
struct Vec { F64 x,y,z; };
inline bool finite_vec(Vec p) { return isfinite(p.x)&&isfinite(p.y)&&isfinite(p.z); }
inline Vec add(Vec a,Vec b) { return {a.x+b.x,a.y+b.y,a.z+b.z}; }
inline Vec mul(Vec a,F64 b) { return {a.x*b,a.y*b,a.z*b}; }
inline F64 dot(Vec a,Vec b) { return (a.x*b.x+a.y*b.y)+a.z*b.z; }
inline Vec column(ExactScene s,int i) { return {D(s.vgrads[i*3]),D(s.vgrads[i*3+1]),D(s.vgrads[i*3+2])}; }
inline Vec normalise(Vec p) { return mul(p,F64(1)/sqrt(dot(p,p)+D(0x2b2bff2ee48e0530ULL))); }
inline Vec rotate(Vec p,ExactScene s) {
    return {(p.x*D(s.vgrads[0])+p.y*D(s.vgrads[3]))+p.z*D(s.vgrads[6]),
            (p.x*D(s.vgrads[1])+p.y*D(s.vgrads[4]))+p.z*D(s.vgrads[7]),
            (p.x*D(s.vgrads[2])+p.y*D(s.vgrads[5]))+p.z*D(s.vgrads[8])};
}
#ifdef MB3D_METAL_NATIVE_FLOAT
inline E80 camera_sine(ExactTrig t) { return E80(D(t.sin_value)); }
inline E80 camera_cosine(ExactTrig t) { return E80(D(t.cos_value)); }
#else
inline E80 camera_sine(ExactTrig t) { return E80::from_bits(t.sin_sign_exponent,t.sin_significand); }
inline E80 camera_cosine(ExactTrig t) { return E80::from_bits(t.cos_sign_exponent,t.cos_significand); }
#endif
inline bool valid_camera_operand(ExactU32 exponent,ExactU64 significand) {
    // Finite canonical ext80 operands, including signed zero/subnormals.
    if(exponent>0xffffu||(exponent&0x7fff)==0x7fff)return false;
    return (exponent&0x7fff)?(significand>>63)!=0:(significand>>63)==0;
}
inline bool valid_camera_trig(ExactTrig t) {
#ifdef MB3D_METAL_NATIVE_FLOAT
    return !t.reserved[0]&&!t.reserved[1]&&!t.sin_significand&&!t.cos_significand&&
        !t.sin_sign_exponent&&!t.cos_sign_exponent&&
        isfinite(D(t.sin_value))&&isfinite(D(t.cos_value));
#else
    return !t.reserved[0]&&!t.reserved[1]&&
        valid_camera_operand(t.sin_sign_exponent,t.sin_significand)&&
        valid_camera_operand(t.cos_sign_exponent,t.cos_significand)&&
        isfinite(D(t.sin_value))&&isfinite(D(t.cos_value))&&
        camera_sine(t).to_f64().bits==t.sin_value&&
        camera_cosine(t).to_f64().bits==t.cos_value;
#endif
}
inline Vec camera_ray(ExactTrig columnTrig,ExactTrig rowTrig,ExactScene s) {
    const E80 x=-camera_sine(columnTrig),y=camera_sine(rowTrig);
    const E80 z=camera_cosine(rowTrig)*camera_cosine(columnTrig);
    const E80 inverseLength=E80(1)/sqrt((x*x+z*z)+y*y);
    // Original BuildViewVectorDFOV stores Double before rotation reloads.
    const E80 vx((x*inverseLength).to_f64()),vy((y*inverseLength).to_f64());
    const E80 vz((z*inverseLength).to_f64());
    return {((E80(D(s.vgrads[0]))*vx+E80(D(s.vgrads[3]))*vy)+E80(D(s.vgrads[6]))*vz).to_f64(),
            ((E80(D(s.vgrads[1]))*vx+E80(D(s.vgrads[4]))*vy)+E80(D(s.vgrads[7]))*vz).to_f64(),
            ((E80(D(s.vgrads[2]))*vx+E80(D(s.vgrads[5]))*vy)+E80(D(s.vgrads[8]))*vz).to_f64()};
}
struct DE { F64 distance; ExactU32 iteration; };
} // namespace mb3d_metal_exact
