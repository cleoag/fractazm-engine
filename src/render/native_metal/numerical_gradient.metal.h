#pragma once
#include <metal_stdlib>
namespace mb3d_native_gradient {
using namespace metal;
struct Sample { float coordinate, scale; bool valid; };
// Local evidence about this orbit's last executed radius update. Never stored
// in carried formula state; a skipped/initialization-only orbit cannot inherit it.
struct RadiusRange { bool certified=false; };
inline bool positive_normal(float value) {
    const uint bits=as_type<uint>(value);
    return bits>=0x00800000u&&bits<0x7f800000u;
}
inline RadiusRange radius_range(float4 xyz,bool four,float radius) {
    const float magnitude=max(max(abs(xyz.x),abs(xyz.y)),max(abs(xyz.z),four?abs(xyz.w):0.f));
    return {all(isfinite(xyz))&&as_type<uint>(radius)==0x7f800000u&&magnitude>=0x1p62f};
}
// A component >=2^62 proves the exact squared radius of the stored FP32
// coordinates >=2^124. With center<=2^122 the gap is >=3*2^122.
// Use a six-times-smaller bound and a further 16x quotient margin. This only
// certifies the existing minimum-DE clamp, never an approximate finite radius.
inline bool minimum_clamp_from_component_bound(float center,float scale,
    float numerator,float floor,float stop) {
    const uint floor_bits=as_type<uint>(floor);
    if(!positive_normal(center)||center>0x1p122f||
       !positive_normal(scale)||scale>2.f||!isfinite(numerator)||
       !isfinite(floor)||((floor_bits&0x80000000u)&&(floor_bits&0x7fffffffu)))return false;
    const float lower=0x1p121f*scale,clamp=stop*.25f,margin=clamp*.0625f;
    if(!positive_normal(lower)||!positive_normal(clamp)||!positive_normal(margin))return false;
    // Classify stored sign/zero before FTZ can turn a positive subnormal into
    // zero. Finite nonpositive numerator / positive denominator proves clamp.
    const uint numerator_bits=as_type<uint>(numerator);
    if((numerator_bits&0x80000000u)||(numerator_bits&0x7fffffffu)==0)return true;
    if(!positive_normal(numerator))return false;
    const float upper=numerator/lower;
    if(positive_normal(upper))return upper<margin;
    // Division can itself underflow. Prove the inequality without using its
    // flushed quotient: n/l < 2^(en-el+1), margin >= 2^(em-1).
    // All frexp operands are normal; no extended mantissa is simulated.
    int en=0,el=0,em=0;
    (void)frexp(numerator,en);(void)frexp(lower,el);(void)frexp(margin,em);
    return en-el<=em-2;
}
inline bool minimum_clamp_proven(RadiusRange range,float center,float scale,
    float numerator,float floor,float stop) {
    return range.certified&&minimum_clamp_from_component_bound(center,scale,numerator,floor,stop);
}
// A finite stored radius difference >=2^124 proves a component magnitude
// >=2^124*scale before its normalization can overflow. The shared lower bound
// is only2^121*scale, with a further16x margin on the quotient. This certifies
// the existing minimum DE; it neither saturates a derivative nor invents a
// finite orbit radius. The caller retains every remaining gradient query.
inline bool difference_minimum_clamp_proven(float difference,float center,float scale,
    float numerator,float floor,float stop) {
    return isfinite(difference)&&abs(difference)>=0x1p124f&&
        minimum_clamp_from_component_bound(center,scale,numerator,floor,stop);
}
// Native FP32 finite differences must use a representable displacement. The
// host DE numerator already contains the requested step, so normalize every
// rounded displacement back to that step before squaring the radius delta.
inline Sample sample(float coordinate, float requested) {
    // Metal may flush subnormal operands before arithmetic. Reject them by
    // their stored bits; otherwise a valid flag could describe a displacement
    // from zero instead of from the caller's actual coordinate.
    const uint coordinateBits=as_type<uint>(coordinate)&0x7fffffffu;
    const uint requestedBits=as_type<uint>(requested)&0x7fffffffu;
    if ((coordinateBits && coordinateBits<0x00800000u) ||
        (requestedBits && requestedBits<0x00800000u))
        return {coordinate, 0.f, false};
    if (!isfinite(coordinate) || !isfinite(requested) || requested <= 0.f)
        return {coordinate, 0.f, false};
    // A single-ULP coordinate perturbation still leaves the orbit-radius
    // subtraction dominated by FP32 rounding. Use a small relative floor,
    // measured in this coordinate's upward intervals, without world units.
    const float spacing=nextafter(coordinate,as_type<float>(0x7f800000u))-coordinate;
    float shifted = coordinate + max(requested,spacing*8.f);
    if (shifted == coordinate)
        shifted = nextafter(coordinate, as_type<float>(0x7f800000u));
    const float actual = shifted - coordinate;
    if (!isfinite(shifted) || !isfinite(actual) || actual <= 0.f)
        return {coordinate, 0.f, false};
    const float scale = actual == requested ? 1.f : requested / actual;
    return {shifted, scale, isfinite(scale) && scale > 0.f};
}
inline float normalize(float difference, Sample sample) {
    return sample.scale == 1.f ? difference : difference * sample.scale;
}
inline float length(float3 delta) {
    const float sum=(delta.z*delta.z+delta.y*delta.y)+delta.x*delta.x;
    if (isfinite(sum)) return sqrt(sum);
    // A finite vector may have overflowing squares but a representable norm.
    // Preserve ordinary operation order; scale only the overflow branch.
    const float magnitude=max(abs(delta.x),max(abs(delta.y),abs(delta.z)));
    if (!isfinite(magnitude) || magnitude<=0.f) return as_type<float>(0x7fc00000u);
    const float3 unit=delta/magnitude;
    return magnitude*sqrt((unit.z*unit.z+unit.y*unit.y)+unit.x*unit.x);
}
// CalcDE: radius*ln(radius)*scale/(|delta|+floor), in the CPU operation
// order. An escaped radius near FLT_MAX may overflow the numerator or
// |delta| although the quotient is representable; only those branches are
// rescaled. NaN for a non-positive denominator or an unrepresentable result.
inline float distance(float radius, float de_scale, float3 delta, float floor) {
    const float numerator=radius*log(radius)*de_scale;
    const float denominator=length(delta)+floor;
    if (isfinite(denominator)) {
        if (denominator<=0.f) return as_type<float>(0x7fc00000u);
        return isfinite(numerator)?numerator/denominator:radius*(log(radius)*de_scale/denominator);
    }
    const float magnitude=max(abs(delta.x),max(abs(delta.y),abs(delta.z)));
    if (!isfinite(magnitude) || magnitude<=0.f) return as_type<float>(0x7fc00000u);
    const float3 unit=delta/magnitude;
    return radius/magnitude*(log(radius)*de_scale)/
        (sqrt((unit.z*unit.z+unit.y*unit.y)+unit.x*unit.x)+floor/magnitude);
}
} // namespace mb3d_native_gradient
