#pragma once
// SOURCE-ONLY Native FP32 proposal. Never include in Compatible/x87 paths.
#if defined(__METAL_VERSION__)
#include <metal_stdlib>
#else
#include <cmath>
#include <limits>
#endif
namespace mb3d_native_smoothing {
#if defined(__METAL_VERSION__)
using metal::isfinite;using metal::log;
#else
using std::isfinite;using std::log;
#endif
enum Status : unsigned { Ordinary=0, RecoveredLogDifference=1,
    StagnantRadius=2, Invalid=3 };
struct Result { float value; Status status; };
inline float invalid_value() {
#if defined(__METAL_VERSION__)
    return as_type<float>(0x7fc00000u);
#else
    return std::numeric_limits<float>::quiet_NaN();
#endif
}
// FP32-only compensated log(1+x). u==1 retains tiny x instead of losing it
// during addition. The correction x/(u-1) removes that addition's rounding
// to first order; all operations remain hardware FP32. Domain failures remain
// nonfinite and visible. This is not an emulated higher-precision logarithm.
inline float stable_log1p(float x) {
    if(!isfinite(x)||x<=-1.f)return invalid_value();
    const float u=1.f+x;
    if(u==1.f)return x;
    if(!isfinite(u)||u<=0.f)return invalid_value();
    return log(u)*(x/(u-1.f));
}
inline float stable_loglog_difference(float radius,float previous) {
    // Caller restricts distinct positive radii to ratio[.5,2]. Subtraction is
    // exact by Sterbenz; do not form radius/previous and then subtract1.
    const float delta=(radius-previous)/previous;
    return stable_log1p(stable_log1p(delta)/log(previous));
}
inline Result calculate(float radius,float previous,float log_stop,
                        float inverse_log_power,int iterations) {
    // Radius is independently required finite by native_distance's final
    // guard. Other fields remain branch-local: partial caller state may leave
    // an unused field nonfinite, and this helper must not newly observe it.
    if(!isfinite(radius)||iterations<0||iterations>(1<<20))
        return {invalid_value(),Invalid};
    if(radius<=1.f)return {float(iterations),Ordinary};
    const float d=log(.5f*log(radius));
    if(previous<1.f){
        const float value=float(iterations)+log_stop-d*inverse_log_power;
        return {value,isfinite(value)?Ordinary:Invalid};
    }
    if(radius==previous&&isfinite(log_stop)){
        // Equal stored FP32 radii contain no interpolation slope. Binary64
        // radii of such an orbit differ by about one ulp or not at all, so
        // CPU's d-old (+1e-100) makes the quotient huge, with the numerator's
        // sign when they are equal. Take one binary64 ulp of the radius, which
        // moves d by 2^-52/log(radius): iteration-boundary refinement then
        // sees the point as far off its target, as CPU does. Counted apart.
        const float value=float(iterations)+(log_stop-d)*log(radius)*0x1p52f;
        return {value,isfinite(value)?StagnantRadius:Invalid};
    }
    const float old=log(.5f*log(previous));
    // Preserve the old expression whenever it has a finite result, including
    // previous==1 (old=-infinity and the finite numerator/denominator ratio0).
    const float original=float(iterations)+(log_stop-d)/(d-old);
    if(isfinite(original))return {original,Ordinary};
    const float ratio=radius/previous;
    if(previous<=1.f||ratio<.5f||ratio>2.f||d!=old)
        return {original,Invalid};
    const float denominator=stable_loglog_difference(radius,previous);
    if(!isfinite(denominator)||denominator==0.f)return {original,Invalid};
    const float recovered=float(iterations)+(log_stop-d)/denominator;
    return {recovered,isfinite(recovered)?RecoveredLogDifference:Invalid};
}
} // namespace mb3d_native_smoothing
