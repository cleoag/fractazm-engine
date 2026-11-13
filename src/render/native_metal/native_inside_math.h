#pragma once
#include "native_inside_abi.h"
#ifndef __METAL_VERSION__
#include <algorithm>
#include <cmath>
#define INSIDE_THREAD
#define INSIDE_CONSTANT const
#else
#define INSIDE_THREAD thread
#define INSIDE_CONSTANT constant
#endif
namespace mb3d_inside {
#ifdef __METAL_VERSION__
using namespace metal;
#else
using std::isfinite; using std::rint; using std::round; using std::min; using std::max;
#endif
inline bool flag_word(I32 v) { return v==0 || v==1 || v==-1; }
inline bool valid(INSIDE_CONSTANT Params& p) {
    if(p.version!=Version || !flag_word(p.initial_inside) || p.in_and_out>1 ||
       !p.full_width || p.full_width>16777216u || p.volume_light>1)return false;
    for(U32 i=0;i<3;++i)if(p.reserved[i])return false;
    return true;
}
inline bool valid_state(INSIDE_THREAD const RetryState& s) {
    return flag_word(s.inside)&&flag_word(s.calc_inside)&&s.retried<=1&&!s.reserved;
}
inline RetryState begin(INSIDE_CONSTANT Params& p) {return {p.initial_inside,p.initial_inside,0,0};}
inline bool retry(INSIDE_CONSTANT Params& p,INSIDE_THREAD RetryState& state,U32 start_inside) {
    // renderer.cpp:1256-1267. Do not compare only the truth values of LongBool.
    const bool again=start_inside!=0 && p.in_and_out &&
        state.calc_inside==p.initial_inside && !state.retried;
    if(again){state.retried=1;state.inside=state.calc_inside=p.initial_inside?0:-1;}
    return again;
}
inline DeResult transform_de(float value,float stop,I32 iteration,U32 driver,I32 option,I32 calc_inside) {
    DeResult r{value,iteration,0,0};
    if(!flag_word(calc_inside)||driver>9||!isfinite(value)||!isfinite(stop)||stop<=0.f){r.fault=Invalid;return r;}
    if(!calc_inside)return r;
    // Called AFTER the existing ordinary DE minimum clamp. Do not clamp again.
    if(option==20)r.value=stop*2.f-value;
    else {
        r.value=stop*4.f-value*3.f;
        // CalcDEanalytic decrements only positive iterations. Numerical CalcDE
        // decrements unconditionally; native driver ids0/2/4/6 are numerical.
        const bool numerical=driver==0||driver==2||driver==4||driver==6;
        if(r.value>=stop&&(numerical||iteration>0)) {
            if(iteration==(-2147483647-1)){r.fault=Invalid;return r;}
            --r.iteration;
        }
    }
    if(!isfinite(r.value))r.fault=NonFinite;
    return r;
}
inline I32 round_indefinite(float v) {
    // PascalRoundToInt's defined overflow result; never convert an out-of-range
    // FP32 value to a Metal int (undefined/implementation-dependent).
    if(v>=2147483648.f||v< -2147483648.f)return (-2147483647-1);
    return I32(rint(v));
}
// RMcalcVLight: Round (half away from zero) capped at 16383, kept as a 7-bit
// mantissa with its shift in bits 7..9 (ConvertVLight decodes it).
inline U32 volume_light_code(float count) {
    if(!(count>=.5f))return 0u;
    const U32 i=count>16383.f?16383u:U32(round(count));
    U32 top=0;for(U32 v=i;v>1u;v>>=1)++top;
    if(top<=6u)return i;
    const U32 shift=top-6u;
    return (i>>shift)|(shift<<7);
}
inline PackedResult pack(INSIDE_CONSTANT Params& p,I32 final_inside,U32 start_inside,
    U32 hit,float step_count,float base_stop,float smooth,float multiplier,U32 prior_gradient,U32 prior_otrap) {
    PackedResult r{0,prior_gradient,prior_otrap,0};
    if(!valid(p)||!flag_word(final_inside)||start_inside>1||hit>1||prior_gradient>65535||prior_otrap>65535||
       !isfinite(step_count)||!isfinite(base_stop)||base_stop<=0.f){r.fault=Invalid;return r;}
    float count=step_count;
    if(final_inside){const float factor=(200.f*base_stop)/float(p.full_width);
        if(!isfinite(factor)){r.fault=NonFinite;return r;}count*=factor;}
    if(!isfinite(count)){r.fault=NonFinite;return r;}
    if(p.volume_light)r.shadow=volume_light_code(count)&1023u;
    else {count=(count-1.f)+1.f;r.shadow=U32(rint(min(1023.f,max(0.f,count))))&1023u;}
    if(!hit)return r;
    if(start_inside&&final_inside){
        const float product=smooth*multiplier;
        if(!isfinite(smooth)||!isfinite(multiplier)||!isfinite(product)){r.fault=NonFinite;return r;}
        r.gradient=0x8000u|U32(min(I32(32767),max(I32(0),round_indefinite(product))));
    }
    // Initial-inside branch never sets this marker, even after a retry.
    if(!start_inside&&p.in_and_out&&!final_inside)r.otrap|=0x8000u;
    return r;
}
inline ProbeOutput probe(INSIDE_CONSTANT ProbeInput& in) {
    ProbeOutput out{};out.record.version=Version;
    RetryState s{in.inside,in.calc_inside,in.retried,0};
    if(!valid(in.params)||!valid_state(s)||in.start_inside>1||in.hit>1||in.reserved[0]||in.reserved[1]){
        out.record.fault=Invalid;return out;}
    out.de=transform_de(in.raw_de,in.stop,in.iteration,in.driver,I32(in.de_option),s.calc_inside);
    out.retry=U32(retry(in.params,s,in.start_inside));
    out.record.attempts=1+out.retry;out.record.retries=s.retried;
    out.record.final_inside=s.inside;out.record.start_inside=in.start_inside;
    out.record.smooth_calls=U32(in.hit&&in.start_inside&&s.inside);
    out.packed=pack(in.params,s.inside,in.start_inside,in.hit,in.step_count,in.stop,
        in.smooth,in.color_multiplier,0,in.otrap);
    out.reverse_normal=U32(in.hit&&!in.start_inside&&s.inside);
    out.record.fault=out.de.fault|out.packed.fault;return out;
}
}
#undef INSIDE_THREAD
#undef INSIDE_CONSTANT
