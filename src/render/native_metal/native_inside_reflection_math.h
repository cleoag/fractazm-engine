#pragma once
#include "native_inside_math.h"
#include "native_inside_reflection_abi.h"
#ifdef __METAL_VERSION__
#define IR_THREAD thread
#define IR_CONSTANT constant
#else
#define IR_THREAD
#define IR_CONSTANT const
#endif
namespace mb3d_inside {
#ifndef __METAL_VERSION__
using std::round;using std::sqrt;using std::fabs;using std::pow;
#endif
// ARM Native CPU sqrt of a finite negative color-depth radicand yields a
// positive quiet NaN. CalcZposAndRough's integer-indefinite conversion and
// 32-bit subtraction clip that to packed depth zero. Express the final integer
// encoding before sqrt; nonfinite inputs and overflow still fail closed.
struct ReflectionColorDepth { float value; U32 fault,zero_packed; };
inline ReflectionColorDepth reflection_color_depth(float segment,float prior_z,U32 far_color) {
    ReflectionColorDepth out{};
    if(far_color>1||prior_z<0.f){out.fault=Invalid;return out;}
    if(!isfinite(segment)||!isfinite(prior_z)){out.fault=NonFinite;return out;}
    if(!far_color){out.value=segment;return out;}
    const float sum=segment+prior_z;
    const float radicand=sum*sum-prior_z+1.0e-30f;
    if(!isfinite(radicand)){out.fault=NonFinite;return out;}
    if(radicand<0.f){out.zero_packed=1;return out;}
    out.value=sqrt(radicand);
    if(!isfinite(out.value))out.fault=NonFinite;
    return out;
}
inline bool valid_reflection(IR_CONSTANT ReflectionConfig& c) {
    if(c.version!=Version||c.is_ifs>1||!isfinite(c.absorption)||!isfinite(c.scattering))return false;
    if(c.transmission>1||!isfinite(c.refraction)||(c.transmission?!(c.refraction>0.f):c.refraction!=0.f))return false;
    for(U32 k=0;k<2;++k)if(c.reserved[k])return false;
    return true;
}
inline I32 select_reflection_inside(IR_CONSTANT Params& p,U32 otrap) {
    // reflection.cpp:1269: raw initial LongBool, not truth normalization.
    return p.in_and_out&&(otrap&0x8000u)?0:p.initial_inside;
}
inline bool material_absorption(IR_CONSTANT Params& p,I32 selected) {
    // Opaque bounces do not flip this state. In-and-out's selected outside
    // surface can still enter CPU's absorption branch.
    return (p.initial_inside^selected)!=0;
}
inline PackedResult secondary_shadow(IR_CONSTANT Params& p,I32 inside,float count,float stop) {
    PackedResult out{};
    if(!valid(p)||!flag_word(inside)||!isfinite(count)||!isfinite(stop)||stop<=0){out.fault=Invalid;return out;}
    if(inside){const float scale=(200.f*stop)/float(p.full_width);
        if(!isfinite(scale)){out.fault=NonFinite;return out;}count*=scale;}
    if(!isfinite(count)){out.fault=NonFinite;return out;}
    // Secondary CPU uses lround (ties away), unlike primary's PascalRound.
    out.shadow=U32(round(min(1023.f,max(0.f,(count-1.f)+1.f))))&1023u;
    return out;
}
inline InitialReflection initial_reflection(U32 gradient,float candidate_distance) {
    InitialReflection out{0,0,0,0};
    if(gradient>65535||!isfinite(candidate_distance)){out.fault=Invalid;return out;}
    if(gradient<32768)return out;
    out.distance=candidate_distance;out.skip=U32(fabs(candidate_distance)>1.f);
    // CPU original primary SIgradient stays high for all opaque bounces;
    // SR_CalcRay biases position then goes directly to OpenAir, without DE.
    out.open_air=1;return out;
}
inline AOPolicy reflection_ao_policy(I32 inside,U32 is_ifs,U32 vary,float base_stop,
    float original_stop,float max_base,float outside_step,float outside_de) {
    AOPolicy out{};
    if(!flag_word(inside)||is_ifs>1||vary>1||!isfinite(base_stop)||base_stop<=0||
       !isfinite(original_stop)||original_stop<=0||!isfinite(max_base)||max_base<=0||
       !isfinite(outside_step)||outside_step<=1||!isfinite(outside_de)||outside_de<=0){out.fault=Invalid;return out;}
    out.inside_non_ifs=U32(inside&&!is_ifs);
    out.step_multiplier=out.inside_non_ifs?outside_step*.5f:outside_step;
    out.de_multiplier=out.inside_non_ifs?1.f:outside_de;
    const float step_ao=original_stop/base_stop;
    out.max_distance=max_base*sqrt(step_ao);
    const float denominator=out.step_multiplier*out.step_multiplier;
    out.stop=vary?(out.inside_non_ifs?original_stop:original_stop/denominator):base_stop/denominator;
    out.initial_travel=step_ao*out.step_multiplier;
    const float md_denom=out.max_distance*out.de_multiplier;
    out.md=.1f/md_denom;
    if(!isfinite(step_ao)||step_ao<=0||!isfinite(denominator)||denominator<=0||
       !isfinite(out.max_distance)||out.max_distance<=0||!isfinite(out.stop)||out.stop<=0||
       !isfinite(out.initial_travel)||out.initial_travel<=0||!isfinite(md_denom)||md_denom<=0||
       !isfinite(out.md))out.fault=NonFinite;
    return out;
}
inline AOStep reflection_inside_ao_step(float de,float stop,float travel,float maximum,float step,U32 last) {
    AOStep out{1.f,travel,0,0};
    if(!isfinite(de)||!isfinite(stop)||stop<=0||!isfinite(travel)||travel<=0||
       !isfinite(maximum)||maximum<=0||travel>maximum||!isfinite(step)||step<=0||last>1){out.fault=Invalid;return out;}
    if(de<stop){const float ratio=travel/maximum;const float square=ratio*ratio;
        out.amount=square*square;out.ended=1;return out;}
    out.next=(travel+travel*step*.1f)+stop;out.ended=last;
    if(!isfinite(out.next))out.fault=NonFinite;
    else if(out.next<=travel&&!last)out.fault=2;
    return out;
}
inline AbsorptionChannel absorption_channel(float diffuse,float exponent,float throughput,
    float light,float luminance,float scattering) {
    AbsorptionChannel out{};
    if(!isfinite(diffuse)||!isfinite(exponent)||!isfinite(throughput)||!isfinite(light)||
       !isfinite(luminance)||!isfinite(scattering)){out.fault=Invalid;return out;}
    // CPU Math.Power defines0^0 as1. Do not replace it with log/exp, nor clamp
    // tiny/negative bases to hide nonfinite results.
    out.factor=exponent==0.f?1.f:pow(diffuse,exponent);
    out.throughput=throughput*out.factor;
    const float amount=(1.f-max(0.f,luminance))*scattering;
    const float interpolation=(out.throughput+throughput)*.5f;
    out.scatter=(light*diffuse)*interpolation*amount;
    if(!isfinite(out.factor)||!isfinite(out.throughput)||!isfinite(amount)||
       !isfinite(interpolation)||!isfinite(out.scatter))out.fault=NonFinite;
    return out;
}
inline U32 ao_cut_axis(float position,float plane,float margin,IR_THREAD U32& fault){
    if(!isfinite(position)||!isfinite(plane)||!isfinite(margin)||margin<0){fault=Invalid;return 0;}
    const float delta=position-plane;
    const float distance=fabs(delta)-margin;
    if(!isfinite(delta)||!isfinite(distance)){fault=NonFinite;return 0;}
    // AtCutPlane2 uses distance to the plane, not its selected halfspace.
    return U32(distance<0.f);
}
inline ReflectionProbeOutput reflection_probe(IR_CONSTANT ReflectionProbeInput& in) {
    ReflectionProbeOutput out{};
    if(!valid(in.policy)||!valid_reflection(in.config)){out.words[0]=Invalid;return out;}
    const I32 selected=select_reflection_inside(in.policy,in.words[1]);
    if(in.words[1]>65535){out.words[0]=Invalid;return out;}
    if(in.words[0]==SelectPolicy){out.words[1]=U32(selected);out.words[2]=U32(selected&&!in.config.is_ifs);
        out.words[3]=U32(material_absorption(in.policy,selected));}
    else if(in.words[0]==SecondaryShadow){const auto r=secondary_shadow(in.policy,selected,in.values[0],in.values[1]);
        out.words[0]=r.fault;out.words[1]=r.shadow;}
    // CalcAmbientOcclusionForPoint explicitly sets bIsIFS=0 until return.
    else if(in.words[0]==PrepareAO){const auto a=reflection_ao_policy(selected,0,in.words[2],
        in.values[0],in.values[1],in.values[2],in.values[3],in.values[4]);
        out.words[0]=a.fault;out.words[1]=a.inside_non_ifs;out.values[0]=a.step_multiplier;out.values[1]=a.de_multiplier;
        out.values[2]=a.stop;out.values[3]=a.max_distance;out.values[4]=a.initial_travel;out.values[5]=a.md;}
    else if(in.words[0]==InsideAOStep){const auto a=reflection_inside_ao_step(in.values[0],in.values[1],in.values[2],
        in.values[3],in.values[4],in.words[2]);out.words[0]=a.fault;out.words[1]=a.ended;out.values[0]=a.amount;out.values[1]=a.next;}
    else if(in.words[0]==AbsorbChannel){const auto a=absorption_channel(in.values[0],in.values[1],in.values[2],
        in.values[3],in.values[4],in.values[5]);out.words[0]=a.fault;out.values[0]=a.factor;out.values[1]=a.throughput;out.values[2]=a.scatter;}
    else if(in.words[0]==InitialBranch){const auto a=initial_reflection(in.words[2],in.values[0]);
        out.words[0]=a.fault;out.words[1]=a.skip;out.words[2]=a.open_air;out.values[0]=a.distance;}
    else if(in.words[0]==AOCutPlane){
        if(in.words[2]&~0x77u){out.words[0]=Invalid;return out;}
        for(U32 axis=0;axis<3;++axis)if(in.words[2]&(1u<<axis)){
            U32 fault=0;const U32 hit=ao_cut_axis(in.values[axis],in.values[3+axis],in.values[6],fault);
            if(fault){out.words[0]=fault;return out;}if(hit){out.words[1]=1;out.words[2]=axis+1;break;}}
    }
    else out.words[0]=Invalid;
    return out;
}
}
#undef IR_THREAD
#undef IR_CONSTANT
