#pragma once
#ifndef MB3D_METAL_NATIVE_FLOAT
#error General reflections require Native GPU FP32
#endif
#include "generic_shadow.metal.h"
#include "generic_reflection_abi.h"
#include "native_inside_reflection_math.h"
#include "inside_palette_math.h"
#include "generic_color.metal.h"
#include "camera_color.metal.h"
#include "secondary_cut_candidate.metal.h"
#include "camera_soft_shadow.metal.h"
#include "comparison/metal_exact/native_reflection_math.metal.h"

namespace mb3d_camera_inside_reflection {
using namespace metal;
using namespace mb3d_native;
using namespace mb3d_metal_exact;
using mb3d_native_reflection::f;
using mb3d_native_reflection::d;
using mb3d_native_reflection::check;
using mb3d_native_reflection::words;
using mb3d_native_reflection::world;
using mb3d_native_reflection::inverse_matrix;
using mb3d_native_reflection::forward_matrix;
using mb3d_native_reflection::stop_at;
using mb3d_native_reflection::decoded_depth;
using mb3d_native_reflection::encoded_depth;
using mb3d_native_reflection::packed_normal;
using mb3d_native_reflection::direction_normal;
#include "inside_material.metal.h"
using mb3d_native_reflection::open_air_depth;
struct Context {
    constant NativeFormulaProgram* program;
    thread NativeResources* resources;
    thread NativeFormulaState* lane;
    thread NativeGeometryRecord* eval;
    NativeNormalParams normal_params;
    thread NativeNormalRecord* normal_record;
    thread NativeNormalResult* normal_result;
    constant mb3d_inside::Params* inside_policy;
    constant mb3d_inside::ReflectionConfig* inside_config;
    constant mb3d_camera::Params* camera;
    constant NativeInteriorColorParams* interior;
    thread mb3d_inside::ReflectionRecord* inside_record;
    int inside_word;
    bool primary_initial;
    constant NativeReflectionMaterial* material;
    device const float* material_samples;
    // Version 0: CalcHShardPoint. Otherwise CalcHSsoftPoint for the primary
    // soft pass's light, radius and ZRSmul numerator.
    constant NativeSoftShadowConfig* soft;
    uint result_maximum; // MaxItsResult of the last DE (a combination's winning part)

};
// CPU SR compares each CalcDE result with mcp.MaxItsResult, which a DE
// combination sets to its winning part's maximum on every call.
// trace, refine_primary, point_shadow, point_ao and inside_material are
// outlined device functions. Inlined into the bounce loop they cost the
// backend compiler about a minute per formula specialization (formula 10181:
// 60 s, outlined 19 s); their results return through stores or selects.
inline uint maximum_iterations(thread const Context& ctx){
    return ctx.result_maximum;
}
inline void initialize_active_pixel(thread NativeFormulaState& lane,constant NativeFormulaProgram& p){
    // IniIt3D is deliberately partial: the task carries all other lanes.
    for(uint i=0;i<4;++i)lane.j[i]=lane.original_j[i]=p.julia_value[i];
    for(uint i=0;i<6;++i)lane.hybrid_counts[i]=p.hybrid_counts[i];
    lane.julia=p.julia;lane.inside=p.inside;lane.calc_smooth=p.calc_smooth;
    lane.rstop=p.radius_stop;lane.max_iterations=p.max_iterations;lane.derivative[0]=1.f;
    if(p.de_option==20)lane.rstop_d=-1.0e10f;
}
inline NativeOrbitResult reflection_de(float3 point,float stop,bool smooth,thread Context& ctx){
    // Policy is MCT state, not mutable formula scratch. Restore it before DE.
    ctx.lane->inside=as_type<uint>(ctx.inside_word);
    return general_de(point,stop,*ctx.program,*ctx.resources,*ctx.lane,*ctx.eval,smooth);
}
// open_air: SR_CalcRay's secondary march treats a NaN/Inf distance as open
// air; every other caller faults on it.
inline DE evaluate(float3 point,float stop,constant ExactScene&,
                   thread NativeReflectionRecord& record,thread Context& ctx,bool open_air=false){
    if(!check(point,record)||!check(stop,record))return {F64(0),0};
    const auto value=reflection_de(point,stop,false,ctx);
    ctx.result_maximum=native_iteration_maximum(*ctx.program,value);
    record.fault|=ctx.eval->fault;
    if(ctx.eval->fault==4)record.nonfinite=1;
    if(!open_air||metal::isfinite(value.distance))check(value.distance,record);
    return {F64::from_native(value.distance),uint(value.state.iteration)};
}
inline bool valid(constant NativeReflectionParams& p,constant ExactScene& scene,constant NativeFormulaProgram& program) {
    if(p.version!=1||p.width!=scene.width||p.height!=scene.height||
       !p.max_bounces||p.max_bounces>kNativeReflectionMaxBounces||!p.max_steps||p.max_steps>kGeneralMaxSecondarySteps||
       p.vary_de_stop>1||p.ao_enabled>1||p.shadows_enabled>1||p.first_step_random>1||
       !valid_general_scene(scene,program))return false;
    const uint scalars[]={p.amount,p.step_width,p.fov_y,p.z_mid_difference,
        p.z_corr,p.z_c_mul,p.back_distance};
    for(uint v:scalars)if(!metal::isfinite(f(v)))return false;
    if(f(p.amount)<0||f(p.amount)>100||f(p.step_width)<=0||f(p.fov_y)<=0||
       f(p.z_corr)<=0||f(p.z_c_mul)<=0||f(p.back_distance)<0)return false;
    for(uint i=0;i<3;++i)for(uint j=0;j<4;++j)
        if(!metal::isfinite(f(p.view_matrix[i][j]))||!metal::isfinite(f(p.ao_matrix[i][j])))return false;
    for(uint b=0;b<3;++b) {
        constant auto& a=p.ao[b];
        if(a.ray_count!=(b?3u:7u)||a.reserved)return false;
        const uint values[]={a.step_multiplier,a.de_multiplier,a.correction_weight,
            a.max_distance_base,a.minimum_overlap_dot,a.overlap_angle_scale};
        for(uint v:values)if(!metal::isfinite(f(v)))return false;
        if(f(a.step_multiplier)<=1||f(a.de_multiplier)<=0||
           (p.ao_enabled&&f(a.max_distance_base)<=0))return false;
        for(uint i=0;i<8;++i)for(uint j=0;j<4;++j)
            if(!metal::isfinite(f(a.directions[i][j])))return false;
    }
    return true;
}

inline bool valid_point_shadow(constant ExactShadowParams& shadow,constant NativeSoftShadowConfig& soft) {
    if(!soft.version)return !soft.radius&&!soft.light&&!soft.numerator;
    return valid_soft_light(shadow,soft);
}
// CalcHShardPoint: one bounded march per enabled light (iHSmask bit 10+i).
// The bit marks an occluded or back-facing light. With a soft descriptor this
// is CalcHSsoftPoint: only the soft light is marched and its closest approach
// becomes the six-bit intensity; a position light beyond its reach stays lit.
// One shared march keeps a single formula call site in the kernel.
__attribute__((noinline)) void point_shadow(thread ExactSurface& surface,float3 position,float3 primaryRay,
                         float z,constant ExactScene& scene,constant ExactShadowParams& shadow,
                         constant NativeReflectionParams& params,thread NativeReflectionRecord& record,thread Context& ctx) {
    if(!params.shadows_enabled)return;
    const bool soft=ctx.soft->version!=0;
    if(soft)surface.shadow|=0xfc00u;else surface.shadow&=~(shadow.light_mask<<10);
    const float maxLength=float(ctx.camera->full_width+ctx.camera->full_height)*0.6f*
        (1.0f+0.5f*metal::min(metal::fabs(z),d(scene.max_ray_length)*0.4f)*f(params.fov_y)/float(ctx.camera->full_height))*d(shadow.length_multiplier);
    if(!check(maxLength,record)||maxLength<=0)return;
    const float3 normal=world(direction_normal(surface),scene);
    const float3 origin=position;const float originZ=z;
    const float radiusMul=soft?float(ctx.soft->numerator)/f(ctx.soft->radius):0.0f;
    for(uint light=0;light<6&&!record.fault;++light) {
        if(!(shadow.light_mask&(1u<<light))||(soft&&light!=ctx.soft->light))continue;
        ++record.shadow_rays;
        float3 direction={d(shadow.light_vectors[3*light]),d(shadow.light_vectors[3*light+1]),d(shadow.light_vectors[3*light+2])};
        float lightLength=maxLength;
        if(shadow.position_mask&(1u<<light)){
            // Position light: toward LN+Xmit, shadowed beyond its squared reach.
            const float3 toward=float3(d(shadow.light_positions[3*light]),d(shadow.light_positions[3*light+1]),
                                       d(shadow.light_positions[3*light+2]))-origin;
            const float squared=(toward.x*toward.x+toward.y*toward.y)+toward.z*toward.z;
            if(!check(squared,record))return;
            if(squared>as_type<float>(shadow.light_reach[light])){if(!soft)surface.shadow|=1024u<<light;continue;}
            if(squared==0)continue; // CPU dT1 0: no march, lit
            const float step=d(shadow.step_width);
            if(squared<(lightLength*step)*(lightLength*step))lightLength=metal::sqrt(squared)/step;
            direction=toward*(-step/metal::sqrt(squared));
        }
        const auto cut=mb3d_camera::secondary_cut_distance(*ctx.camera,origin,-direction,lightLength,0.f);
        if(cut.fault){record.fault|=cut.fault;return;}
        if(cut.axis)++ctx.inside_record->cut_shadow_limits;
        float remaining=cut.depth;const float extent=remaining;
        if(remaining<=0)continue;
        if(metal::dot(normal,direction)>0){if(soft)surface.shadow&=0x3ffu;else surface.shadow|=1024u<<light;continue;}
        position=origin;z=originZ;
        const float projection=-metal::dot(direction,primaryRay)/metal::sqrt(metal::dot(direction,direction)*metal::dot(primaryRay,primaryRay));
        float stop=stop_at(z,scene),factor=1.0f,visibility=1.0f;
        auto value=evaluate(position,stop,scene,record,ctx);
        if(!check(projection,record)||record.fault)return;
        bool ended=false;
        for(uint k=0;k<params.max_steps&&!ended;++k) {
            ++record.shadow_steps;
            const float previous=value.distance.value;
            // MinCS and the RLastDE bound compare like CPU: NaN keeps the limiter.
            const float scaled=previous*f(scene.z_step_div)*factor,limit=metal::max(stop,0.4f)*f(scene.step_limiter);
            const float step=scaled<limit?scaled:limit;
            remaining-=step;position-=direction*step;z+=step*projection;stop=stop_at(z,scene);
            // A negative start distance marches backwards with a geometrically
            // growing step, which leaves binary32 long before CPU double. The
            // light completes as CPU's still-positive dT1 (occluded; a soft
            // light keeps the approach so far) and counts one FP32 range exit.
            if(!metal::isfinite(remaining)||!metal::isfinite(stop)||!metal::all(metal::isfinite(position))){
                ++ctx.eval->fp32_range_exits;ended=true;break;}
            value=evaluate(position,stop,scene,record,ctx);
            if(record.fault)return;
            if(soft) {
                // ZRsoft: the threshold divides by the uncut MaxLHS.
                const float traveled=extent-remaining;
                float threshold=traveled/maxLength;threshold*=threshold;threshold*=threshold;threshold*=threshold;
                const float candidate=(value.distance.value-stop)*radiusMul/(traveled+0.11f)+threshold;
                if(!check(candidate,record))return;
                visibility=metal::min(visibility,candidate);
            }
            // CalcHShardPoint stops below the DE stop, CalcHSsoftPoint at it.
            if(value.iteration>=maximum_iterations(ctx)||
               (soft?value.distance.value<=stop:value.distance.value<stop)){ended=true;break;}
            const float bound=previous+step;
            const float current=value.distance.value>bound?bound:value.distance.value;
            value.distance=F64::from_native(current);
            if(previous>current+1.0e-30f){const float ratio=step/(previous-current);factor=ratio<1?metal::max(0.5f,ratio):1.0f;}
            else factor=1;
            if(remaining<0)ended=true;
        }
        if(!ended){record.fault|=2;return;}
        if(soft)surface.shadow=(surface.shadow&0x3ffu)|(uint(metal::round(metal::clamp(visibility,0.0f,1.0f)*63.4f))<<10);
        else if(remaining>0)surface.shadow|=1024u<<light;
    }
    for(uint axis=0;axis<3;++axis)ctx.lane->c[axis]=origin[axis];
}

__attribute__((noinline)) void point_ao(thread ExactSurface& surface,float3 position,float z,uint bounce,
                     constant ExactScene& scene,constant NativeReflectionParams& params,
                     thread NativeReflectionRecord& record,thread Context& ctx) {
    if(!params.ao_enabled){surface.amb_shadow=5000;return;}
    surface.amb_shadow=0;
    if((surface.depth_packed>>16)>=32768||surface.si_gradient>=32768)return;
    ctx.lane->calc_smooth=0;
    // SR_CalcRay: quality 2 on the first bounce, 1 on the second, 0 after.
    constant auto& a=params.ao[metal::min(bounce,3u)-1];
    const auto insideAO=mb3d_inside::reflection_ao_policy(ctx.inside_word,0,
        params.vary_de_stop,f(scene.de_stop),metal::min(stop_at(z,scene),1.0e6f),
        f(a.max_distance_base),f(a.step_multiplier),f(a.de_multiplier));
    if(insideAO.fault){record.fault|=insideAO.fault;return;}
    const bool interior=insideAO.inside_non_ifs;
    const float stepMul=interior?insideAO.step_multiplier:f(a.step_multiplier);
    const float deMul=interior?insideAO.de_multiplier:f(a.de_multiplier);
    const float originalStop=metal::min(stop_at(z,scene),1.0e6f);
    const float stepAO=originalStop/f(scene.de_stop),maxDistance=f(a.max_distance_base)*metal::sqrt(stepAO);
    const float stop=interior?insideAO.stop:
        (params.vary_de_stop?originalStop:f(scene.de_stop))/(stepMul*stepMul);
    const float md=0.1f/(maxDistance*deMul);
    float3 n=packed_normal(surface);const float nlength=metal::length(n);
    if(!check(nlength,record)||nlength<=0||!check(maxDistance,record)||maxDistance<=0){record.fault|=4;return;}
    n/=nlength;
    const float angle=metal::acos(metal::clamp(-n.z,-1.0f,1.0f))*0.5f;
    float sinA=metal::sin(angle),cosA=metal::cos(angle),xy=metal::sqrt(n.x*n.x+n.y*n.y);
    const float qx=xy<1.0e-25f?0.0f:-n.y*(sinA/xy),qy=xy<1.0e-25f?0.0f:n.x*(sinA/xy);
    const float3 rotation[3]={
        {1.0f-2.0f*qy*qy,2.0f*qx*qy,-2.0f*cosA*qy},
        {2.0f*qx*qy,1.0f-2.0f*qx*qx,2.0f*cosA*qx},
        {2.0f*cosA*qy,-2.0f*cosA*qx,1.0f-2.0f*(qx*qx+qy*qy)}};
    float minimum[8]={};
    // First-step jitter: every call restarts PasLcgInitFromThread(Max(1,
    // iThreadId)); the reflection pass keeps the scene's iThreadId 0. Each ray
    // scales its first step by U01*1.5+0.5, and that step skips the MaxDist cap.
    uint jitter_seed=0x324594A1u*2u+0x24563487u;
    for(uint i=0;i<a.ray_count;++i) {
        ++record.ao_rays;if(interior)++ctx.inside_record->inside_ao_rays;
        const float3 source=words(a.directions[i]);
        const float3 oriented={metal::dot(source,rotation[0]),metal::dot(source,rotation[1]),metal::dot(source,rotation[2])};
        const float3 ray=forward_matrix(oriented,params.ao_matrix);
        float travel=stepAO*stepMul,amount=1.0f;bool ended=false;
        if(params.first_step_random){
            jitter_seed=jitter_seed*214013u+2531011u;
            travel*=float(jitter_seed>>8)*(1.0f/16777216.0f)*1.5f+0.5f;
        }
        if(!check(ray,record)||!check(travel,record))return;
        for(uint k=0;k<params.max_steps;++k) {
            ++record.ao_steps;
            const bool last=!(params.first_step_random&&k==0)&&travel>maxDistance;if(last)travel=maxDistance;
            const float3 sample=position+ray*travel;
            const float margin=ctx.camera->cut_options?travel*(f(params.step_width)*.5f):0.f;
            if(!check(sample,record)||!check(margin,record))return;
            for(uint axis=0;axis<3;++axis)ctx.lane->c[axis]=sample[axis];
            bool at_cut=false;
            for(uint axis=0;axis<3;++axis)if(ctx.camera->cut_options&(1u<<axis)){
                uint fault=0;at_cut=mb3d_inside::ao_cut_axis(sample[axis],ctx.camera->cut[axis],margin,fault)!=0;
                if(fault){record.fault|=fault;return;}if(at_cut)break;
            }
            if(at_cut){++ctx.inside_record->cut_ao_skips;ended=true;break;}
            const auto value=evaluate(sample,stop,scene,record,ctx);
            if(record.fault)return;
            if(interior){
                const auto next=mb3d_inside::reflection_inside_ao_step(value.distance.value,stop,travel,maxDistance,stepMul,uint(last));
                if(next.fault){record.fault|=next.fault;return;}
                amount=next.amount;if(next.ended){ended=true;break;}travel=next.next;continue;
            }
            amount=metal::min(amount,(value.distance.value-stop+travel*md)/travel);
            if(!check(amount,record))return;
            if(amount<0.02f||last){ended=true;break;}
            const float next=travel+metal::max(value.distance.value,travel*stepMul);
            if(!check(next,record))return;
            if(next<=travel){record.fault|=2;return;}travel=next;
        }
        if(!ended){record.fault|=2;return;}
        minimum[i]=metal::max(0.0f,amount)*deMul;
    }
    float sum=0;
    for(uint i=0;i<a.ray_count;++i) {
        if(interior){sum+=minimum[i];continue;}
        float correction=0;
        if(minimum[i]<1.0f)for(uint j=0;j<a.ray_count;++j) {
            if(i==j)continue;
            const float dot=metal::dot(words(a.directions[i]),words(a.directions[j]));
            if(dot>f(a.minimum_overlap_dot)) {
                const float overlap=minimum[j]-metal::acos(metal::clamp(dot,-1.0f,1.0f))*f(a.overlap_angle_scale)+1.0f;
                if(overlap>0)correction+=metal::min(1.0f-minimum[i],overlap)*f(a.correction_weight);
            }
        }
        sum+=metal::min(1.0f,correction+minimum[i]);
    }
    if(!check(sum,record))return;
    const uint amount=uint(metal::max(0.0f,metal::round(16383.0f*(1.0f-sum/float(a.ray_count)))));
    surface.amb_shadow=(amount*8u)/(8u+bounce);
    for(uint axis=0;axis<3;++axis)ctx.lane->c[axis]=position[axis];
}

__attribute__((noinline)) bool refine_primary(thread float3& position,float3 ray,thread float& z,
                           thread float& stop,uint packed,constant ExactScene& scene,
                           constant NativeReflectionParams& params,thread NativeReflectionRecord& record,thread Context& ctx) {
    auto value=evaluate(position,stop,scene,record,ctx);if(record.fault)return false;
    if(value.distance.value<stop*0.5f){record.primary_skipped=1;return false;}
    if(value.iteration<maximum_iterations(ctx)||value.distance.value<=stop) {
        float step=(z-decoded_depth(int(packed>>8)-1,params))*(-0.5f);
        for(uint remain=scene.refinement_steps;remain&&metal::fabs(value.distance.value-stop)>0.001f;--remain) {
            z+=step;position+=ray*step;stop=stop_at(z,scene);
            if(!check(position,record)||!check(stop,record))return false;
            if(remain<=1)break;
            value=evaluate(position,stop,scene,record,ctx);if(record.fault)return false;
            step=metal::fabs(step)*(value.distance.value<stop?-0.55f:0.55f);
        }
    } else {
        uint iteration=value.iteration;
        if(!general_refine_iteration(position,ray,z,stop,scene,*ctx.program,*ctx.resources,*ctx.lane,*ctx.eval,iteration)){
            record.fault|=ctx.eval->fault;return false;
        }
        if(!check(position,record)||!check(z,record))return false;
    }
    return true;
}

struct SecondaryHit {
    float3 position,direction;
    float z,segment,stop,max_length;
    ExactSurface surface;
    bool hit;
};
__attribute__((noinline)) SecondaryHit trace(float3 position,float3 direction,float3 primaryRay,float z,
                          constant ExactScene& scene,constant mb3d_native::NativeSurfaceParams& packing,
                          constant NativeReflectionParams& params,thread NativeReflectionRecord& record,thread Context& ctx) {
    SecondaryHit out={position,direction,z,0,0,0,{},false};
    const float priorZ=metal::fabs(z);
    const auto cut=mb3d_camera::secondary_cut_distance(*ctx.camera,position,direction,d(scene.max_ray_length),1.f);
    if(cut.fault){record.fault|=cut.fault;return out;}
    const float maxLength=cut.depth;out.max_length=maxLength;
    const float projection=metal::dot(primaryRay,direction)/
        metal::sqrt(metal::dot(primaryRay,primaryRay)*metal::dot(direction,direction));
    float segment=metal::min(1.0f,f(scene.de_stop)*0.25f);
    z+=segment*projection;position+=direction*segment;
    float stop=stop_at(z,scene),stepCount=0,factor=1,lastStep=0;
    if(!check(projection,record)||!check(position,record)||!check(stop,record))return out;
    if(ctx.primary_initial){
        out.surface.depth_packed=0x80000000u;out.position=position;out.z=z;
        out.segment=d(scene.max_ray_length);out.stop=stop;return out;
    }
    auto value=evaluate(position,stop,scene,record,ctx);if(record.fault)return out;
    lastStep=value.distance.value*f(scene.z_step_div);
    bool hit=true,deLimited=true,ended=value.distance.value<=1.0e-10f;
    // CalcSR.pas: the 100001st march iteration and a NaN/Inf DE end the ray
    // as open air. Point shadows and AO keep the secondary step budget.
    for(uint k=0;k<params.max_steps&&!ended;++k) {
        if(k==kSecondaryRayStepLimit){hit=false;ended=true;break;}
        ++record.ray_steps;
        const float previousDE=value.distance.value;
        float step=metal::max(0.11f,(previousDE-f(scene.de_sub)*stop)*f(scene.z_step_div)*factor);
        const float limit=metal::max(stop,0.4f)*f(scene.step_limiter);
        if(!scene.fog_iteration||value.iteration==scene.fog_iteration)stepCount+=limit<step?limit/step:1.0f;
        step=metal::min(step,limit);lastStep=step;segment+=step;
        if(!check(step,record)||!check(segment,record)||!check(stepCount,record))return out;
        if(segment>maxLength){hit=false;ended=true;break;}
        position+=direction*step;z+=step*projection;stop=stop_at(z,scene);
        value=evaluate(position,stop,scene,record,ctx,true);if(record.fault)return out;
        if(!metal::isfinite(value.distance.value)){hit=false;ended=true;break;}
        if(value.iteration>=maximum_iterations(ctx)){deLimited=false;ended=true;break;}
        if(value.distance.value<stop&&value.iteration>=scene.min_iterations){ended=true;break;}
        const float current=metal::min(value.distance.value,previousDE+lastStep);
        value.distance=F64::from_native(current);
        if(previousDE>current+1.0e-10f){const float ratio=lastStep/(previousDE-current);factor=ratio<1?metal::max(0.5f,ratio):1.0f;}
        else factor=1;
    }
    if(!ended){record.fault|=2;return out;}
    const auto secondaryPack=mb3d_inside::secondary_shadow(*ctx.inside_policy,ctx.inside_word,stepCount,f(scene.de_stop));
    if(secondaryPack.fault){record.fault|=secondaryPack.fault;return out;}
    out.surface.shadow=secondaryPack.shadow;
    if(!hit) {
        out.surface.depth_packed=0x80000000u;out.position=position;out.z=z;
        out.segment=segment;out.stop=stop;return out;
    }
    if(scene.refinement_steps) {
    if(deLimited) {
        float step=lastStep*(-0.5f);
        value=evaluate(position,stop,scene,record,ctx);if(record.fault)return out;
        for(uint remain=scene.refinement_steps;remain&&metal::fabs(value.distance.value-stop)>0.001f;--remain) {
            segment+=step;z+=step*projection;position+=direction*step;stop=stop_at(z,scene);
            if(!check(position,record)||!check(stop,record)||!check(segment,record))return out;
            if(remain<=1)break;
            value=evaluate(position,stop,scene,record,ctx);if(record.fault)return out;
            step=metal::fabs(step)*(value.distance.value<stop?-0.55f:0.55f);
        }
    } else {
        const float old=segment;
        uint iteration=value.iteration;
        if(!general_refine_iteration(position,direction,segment,stop,scene,*ctx.program,*ctx.resources,*ctx.lane,*ctx.eval,iteration)){
            record.fault|=ctx.eval->fault;return out;
        }
        z+=(segment-old)*projection;
        if(!check(position,record)||!check(segment,record)||!check(z,record))return out;
    }
    }
    ctx.normal_params.inside=uint(ctx.inside_word!=0);
    ctx.lane->inside=as_type<uint>(ctx.inside_word);
    *ctx.normal_result=general_calculate_normals(position,metal::fabs(z),stop,scene,*ctx.program,
        ctx.normal_params,*ctx.resources,*ctx.lane,*ctx.eval,*ctx.normal_record,true);
    const auto n=*ctx.normal_result;
    if(n.fault){record.fault|=n.fault;if(n.fault&4u)record.nonfinite=1;return out;}
    ExactPixel packingInput{};packingInput.rm_result=deLimited?1:2;
    packingInput.final_stop=as_type<uint>(stop);packingInput.smooth_iterations=as_type<uint>(n.smooth);
    packingInput.orbit_trap=general_word(n.orbit_trap);packingInput.depth=general_word(0.f);
    if(scene.color_on_it){
        const auto color=general_color_on_it(position,stop,scene,*ctx.program,*ctx.resources,*ctx.lane,*ctx.eval);
        if(ctx.eval->fault){record.fault|=ctx.eval->fault;return out;}
        packingInput.orbit_trap=general_word(color.state.otrap);
    }
    packingInput.step_count=as_type<uint>(stepCount);
    for(uint i=0;i<3;++i)packingInput.normal[i]=n.packed[i];
    const auto hit_color=capture_hit_color(*ctx.lane);
    out.surface=general_surface(packingInput,scene,packing,*ctx.program,0.f,n.roughness_byte,hit_color);
    if(out.surface.fault){record.fault|=out.surface.fault;return out;}
    // Keep secondary lround encoding; general_surface uses primary ties-even.
    out.surface.shadow=(out.surface.shadow&~1023u)|secondaryPack.shadow;
    const auto farDepth=mb3d_inside::reflection_color_depth(segment,priorZ,uint((ctx.interior->flags&4u)!=0));
    if(farDepth.fault){record.fault|=farDepth.fault;record.nonfinite|=uint((farDepth.fault&4u)!=0);return out;}
    out.surface.depth_packed=(farDepth.zero_packed?0u:encoded_depth(farDepth.value,params,record))|n.roughness_byte;
    out.position=position;out.z=z;out.segment=segment;out.stop=stop;out.hit=true;
    return out;
}


// One radiance term of SR_CalcRay, written straight to the dispatch's shade
// record; its events follow the record header.
inline void shade_event(device NativeReflectionShadeRecord* out,thread NativeReflectionRecord& record,ExactSurface surface,float3 view,float y,
    float zpos,float fogZ,float world_view_y,float3 position,bool suppress_fog,float3 weight,
    bool absorbs,float3 absorbed,float amount,float3 cam,float3 absolute,float scale_amount) {
    if(out->count>=out->capacity){record.fault|=2;return;}
    device auto& e=reinterpret_cast<device NativeReflectionShadeEvent*>(out+1)[out->count++];
    e.camera[0]=cam.x;e.camera[1]=cam.y;e.camera[2]=cam.z;e.camera[3]=0;
    e.absolute[0]=absolute.x;e.absolute[1]=absolute.y;e.absolute[2]=absolute.z;e.absolute[3]=0;
    e.surface=surface;e.view[0]=view.x;e.view[1]=view.y;e.view[2]=view.z;e.world_view_y=world_view_y;
    e.position[0]=position.x;e.position[1]=position.y;e.position[2]=position.z;e.y=y;
    e.zpos=zpos;e.fog_z=fogZ;e.suppress_fog=suppress_fog;e.absorbs=absorbs;
    e.scale_amount=scale_amount;e.reserved[0]=0;e.reserved[1]=0;e.reserved[2]=0;
    e.weight[0]=weight.x;e.weight[1]=weight.y;e.weight[2]=weight.z;e.weight[3]=0;
    e.absorbed[0]=absorbed.x;e.absorbed[1]=absorbed.y;e.absorbed[2]=absorbed.z;e.absorbed[3]=amount;
}

// A transmitting frame of SR_CalcRay after its refraction: the state its
// Fresnel reflection restores once the refracted subtree is done.
struct PendingReflection {
    ExactSurface surface;
    float3 position,ray,throughput,specular,diffuse;
    float z,specular_w,fresnel;
    uint bounce;int inside;
};

// Trace half of SR_CalcRay. Only what steers the rays is evaluated here
// (palette specular, absorption diffuse, transparency); every radiance term
// becomes an event for shade_pixel, which reaches no formula code. A pixel
// that keeps its primary color leaves no events.
inline void reflect_pixel(constant ExactScene& scene,constant mb3d_native::NativeSurfaceParams& packing,constant ExactShadowParams& shadow,
    constant ExactColorScene& color,constant NativeReflectionParams& params,
    ExactPixel primary,ExactSurface surface,ExactColorPixel original,uint pixel,
    thread NativeReflectionRecord& record,thread Context& ctx,device NativeReflectionShadeRecord* out,uint capacity) {
    out->version=kNativeReflectionShadeVersion;out->pixel=pixel;out->count=0;out->capacity=capacity;
    if(!mb3d_inside::valid(*ctx.inside_policy)||!mb3d_inside::valid_reflection(*ctx.inside_config)||
       !mb3d_camera::valid_camera(*ctx.camera)||ctx.camera->width!=scene.width||ctx.camera->height!=scene.height||
       !valid_interior_color(*ctx.interior,kInsideMaterialFlags)||ctx.inside_policy->full_width!=ctx.camera->full_width||
       as_type<int>(ctx.program->inside)!=ctx.inside_policy->initial_inside||
       !valid(params,scene,*ctx.program)||!valid_general_normals(ctx.normal_params,*ctx.program)||!ctx.material||!valid_inside_material(color,*ctx.interior,ctx.material)||!valid_generic_shadow(scene,*ctx.program,shadow,true,true)||
       !ctx.soft||!valid_point_shadow(shadow,*ctx.soft)||
       color.width!=params.width||color.height!=params.height||shadow.enabled!=params.shadows_enabled||
       !valid_surface_color_descriptor(packing,native_selected_driver(),ctx.program->de_option)||primary.fault||surface.fault||original.fault||original.reserved0||original.reserved1||
       primary.reserved_tail||surface.reserved[0]||surface.reserved[1]||surface.reserved[2]||
       surface.si_gradient>65535||surface.o_trap>65535||surface.amb_shadow>65535||surface.shadow>65535||
       capacity!=native_reflection_shade_capacity(params.max_bounces,ctx.inside_config->transmission!=0)||
       // Transmission starts outside: SR's NotInsideXor then holds on every ray.
       (ctx.inside_config->transmission&&(ctx.inside_policy->initial_inside||ctx.inside_policy->in_and_out))) {
        record.fault|=1;return;
    }
    const uint primaryDepth=surface.depth_packed>>16;
    if(primaryDepth>=32768)return;
    record.active=1;
    initialize_active_pixel(*ctx.lane,*ctx.program);
    ctx.inside_word=mb3d_inside::select_reflection_inside(*ctx.inside_policy,surface.o_trap);
    ctx.lane->inside=as_type<uint>(ctx.inside_word);
    ctx.inside_record->version=1;ctx.inside_record->selected_inside=ctx.inside_word;
    ctx.primary_initial=surface.si_gradient>=32768;
    for(uint i=0;i<3;++i)if(surface.normal[i]< -32768||surface.normal[i]>32767){record.fault|=1;return;}
    const float3 primaryRay={d(primary.ray[0]),d(primary.ray[1]),d(primary.ray[2])};
    const float3 start={d(primary.start[0]),d(primary.start[1]),d(primary.start[2])};
    float z=decoded_depth(int(surface.depth_packed>>8),params),stop=stop_at(z,scene);
    float3 position=start+primaryRay*z;
    if(!check(primaryRay,record)||!check(z,record)||!check(position,record))return;
    ctx.lane->calc_smooth=0;
    for(uint axis=0;axis<3;++axis)ctx.lane->c[axis]=position[axis];
    if(ctx.primary_initial){
        const auto cut=mb3d_camera::secondary_cut_distance(*ctx.camera,position,primaryRay,d(scene.max_ray_length),-2.f);
        if(cut.fault){record.fault|=cut.fault;return;}
        const auto branch=mb3d_inside::initial_reflection(surface.si_gradient,cut.depth);
        if(branch.fault){record.fault|=branch.fault;return;}
        ctx.inside_record->initial_primary=1;
        if(branch.skip){record.primary_skipped=1;return;}
        z+=branch.distance;position+=primaryRay*branch.distance;stop=stop_at(z,scene);
        if(!check(z,record)||!check(position,record)||!check(stop,record))return;
        for(uint axis=0;axis<3;++axis)ctx.lane->c[axis]=position[axis];
    } else if(!refine_primary(position,primaryRay,z,stop,surface.depth_packed,scene,params,record,ctx))return;
    const float rayLength=metal::length(primaryRay);
    if(!check(rayLength,record)||rayLength<=0){record.fault|=4;return;}
    float3 view=inverse_matrix(primaryRay/rayLength,params.view_matrix);
    const float primaryY=float(int(pixel/scene.width)+ctx.camera->top)/float(ctx.camera->full_height);
    const float primaryZ=z*f(params.step_width)+f(params.z_mid_difference);
    // SR's light setup: CamPos is the pixel's ray start, AbsViewVec its unit
    // direction; each bounce moves CamPos to the biased start of its ray.
    const float3 mid(f(ctx.material->mid[0]),f(ctx.material->mid[1]),f(ctx.material->mid[2]));
    float3 cam=start-mid,absolute=primaryRay/rayLength;
    InsideMaterial current=inside_material<false>(color,*ctx.interior,surface,view,primaryY,primaryZ,primaryZ,
        primaryRay.y/rayLength,record,false,ctx.material,ctx.material_samples,position,cam,absolute);
    if(record.fault)return;
    // CalcPixelColorSvec scales a transmitting primary surface down (bCalcTransR).
    const bool transmission=ctx.inside_config->transmission!=0;
    const float lightAmount=f(params.amount);
    shade_event(out,record,surface,view,primaryY,primaryZ,primaryZ,primaryRay.y/rayLength,position,false,float3(1),false,float3(0),0,cam,absolute,
        transmission?lightAmount:0.0f);
    if(record.fault)return;
    // A diffuse map changes the scattering color the trace would absorb with.
    const bool mappedDiffuse=(ctx.interior->flags&kNativeMaterialMapFlag)&&ctx.material->maps.diffuse_mode;
    float3 throughput=float3(current.transparency*lightAmount);
    float3 ray=primaryRay;
    // SR_CalcRay's recursion as a loop. With transmission a frame refracts
    // first (toggling bInsideRendering) and keeps its Fresnel reflection
    // pending until the refracted subtree is done; each bounce level holds at
    // most one pending reflection. Without it every frame only reflects.
    PendingReflection pending[kNativeReflectionMaxBounces];uint depth=0;
    uint bounce=1;bool refract=transmission,complete=false;
    for(uint step=0;step<2u*kNativeReflectionTreeEvents&&!record.fault;++step) {
        // lab1: a frame whose absorbed light is negligible ends.
        bool finished=metal::dot(metal::fabs(throughput),float3(0.3f,0.59f,0.11f))<1.0e-4f;
        float3 next(0);bool refracted=false;
        if(!finished) {
            const float3 normal=world(direction_normal(surface),scene);
            if(refract) {
                // Snell refraction against the surface normal; total internal
                // reflection dims SDsvecs[0] and reflects instead.
                const float strongest=metal::max(current.specular.x,metal::max(current.specular.y,current.specular.z));
                const bool inMaterial=(ctx.inside_policy->initial_inside^ctx.inside_word)!=0;
                const float eta=inMaterial?ctx.inside_config->refraction:1.0f/ctx.inside_config->refraction;
                const float rayLength=metal::length(ray),normalLength=metal::length(normal);
                const float cosine=-metal::dot(ray,normal)/(rayLength*normalLength);
                const float radicand=1.0f-eta*eta*(1.0f-cosine*cosine);
                if(!check(cosine,record)||!check(radicand,record))break;
                if(radicand<=0.0f) {
                    const float dim=1.0f-current.specular_w+current.specular_w*current.specular_w/(strongest+0.01f);
                    current.specular*=dim;current.specular_w*=dim;
                    if(!check(current.specular,record)||!check(current.specular_w,record))break;
                } else {
                    next=ray-normal*(rayLength/(normalLength*eta)*(metal::sqrt(radicand)-eta*cosine));
                    const float nextLength=metal::length(next);
                    if(nextLength>1.0e-20f)next*=rayLength/nextLength;
                    // Fresnel: the reflected share of unpolarized light.
                    const float transmitted=metal::fabs(metal::dot(next,normal))/(rayLength*normalLength);
                    float fresnel=metal::fabs(cosine);
                    if(metal::fabs(transmitted)>1.0e-16f) {
                        const float r1=(fresnel-eta*transmitted)/(fresnel+eta*transmitted);
                        const float r2=(transmitted-eta*fresnel)/(transmitted+eta*fresnel);
                        fresnel=(r1*r1+r2*r2)*0.5f;
                    }
                    if(!check(next,record)||!check(fresnel,record))break;
                    if(depth>=kNativeReflectionMaxBounces){record.fault|=2;break;}
                    pending[depth++]={surface,position,ray,throughput,current.specular,current.diffuse,
                        z,current.specular_w,fresnel,bounce,ctx.inside_word};
                    throughput*=1.0f-fresnel*strongest;
                    ctx.inside_word=ctx.inside_word?0:1;
                    refracted=true;
                }
            }
            if(!refracted) {
                const float squared=metal::dot(normal,normal);
                if(!check(squared,record))break;
                if(squared<1.0e-10f)finished=true;
                else next=ray-normal*(2.0f*metal::dot(normal,ray)/squared);
                if(!finished&&!check(next,record))break;
            }
        }
        if(!finished) {
        ++record.rays;record.bounces=metal::max(record.bounces,bounce);
        SecondaryHit hit=trace(position,next,primaryRay,z,scene,packing,params,record,ctx);
        if(record.fault)break;
        if(hit.hit)++record.hits;else ++record.misses;
        const float length=metal::length(next);
        if(!check(length,record)||length<=0){record.fault|=4;break;}
        view=inverse_matrix(next/length,params.view_matrix);
        float y=metal::asin(metal::clamp(view.y,-1.0f,1.0f))/f(params.fov_y)+0.5f;
        if((ctx.interior->flags&kNativeMaterialMapFlag)&&(ctx.material->maps.background&kNativeBackgroundMap)) {
            // SR_CalcRay with bBackBMP folds yPos beyond the frame back into it.
            const float fov=f(params.fov_y),fold=fov/metal::max(0.1f,2.0f*M_PI_F-fov);
            if(y>1.0f){y=1.0f+fold*(1.0f-y);if(y<0.75f)y=1.5f-y;}
            else if(y<0.0f){y=metal::fabs(y)*fold;if(y>0.25f)y=0.5f-y;}
        }
        y=metal::clamp(y,0.0f,1.0f);
        cam=position+next*metal::min(1.0f,f(scene.de_stop)*0.25f)-mid;absolute=next/length;
        // SDsvecs[0] of the surface the ray leaves: its transparency through a
        // refraction, its specular color on a reflection.
        if(refracted)throughput*=current.specular_w;else throughput*=current.specular;
        if(hit.hit) {
            point_shadow(hit.surface,hit.position,primaryRay,metal::fabs(hit.z),scene,shadow,params,record,ctx);
            if(record.fault)break;
            point_ao(hit.surface,hit.position,metal::fabs(hit.z),bounce,scene,params,record,ctx);
            if(record.fault)break;
            const bool absorbs=mb3d_inside::material_absorption(*ctx.inside_policy,ctx.inside_word);
            float3 absorbed(0);float amount=0;
            if(absorbs){
                if(mappedDiffuse){record.fault|=8;break;}
                ++ctx.inside_record->absorption_bounces;
                const float exponent=metal::min(hit.segment,hit.max_length)*ctx.inside_config->absorption;
                if(!check(exponent,record))break;
                const float3 factor=exponent==0.f?float3(1):metal::pow(current.diffuse,float3(exponent));
                if(!check(factor,record))break;
                const float luminance=metal::max(0.f,(factor.x*.3f+factor.y*.59f)+factor.z*.11f);
                const float3 before=throughput;throughput*=factor;
                amount=(1.f-luminance)*ctx.inside_config->scattering;
                // SR_AddLight: every enabled directional light at full weight. A
                // positional light (LN relative to the scene center) weighs the
                // segment from CamPos, the biased start of this ray, to ObjPos.
                float3 lights(0);
                const float3 camPos=cam,objPos=hit.position-mid;
                const float width2=f(params.step_width)*f(params.step_width);
                for(uint i=0;i<6;++i)if(ctx.interior->lights[i].enabled) {
                    float weight=1.f;
                    if((ctx.interior->flags&kNativeLightGeometryFlag)&&(ctx.material->geometry.lights[i].options&1u)){
                        const float3 ln=words(ctx.interior->lights[i].direction);
                        const float3 sv=objPos-camPos,sv2=ln-camPos,sv3=ln-objPos;
                        const float d2=metal::dot(sv2,sv2)+width2,d3=metal::dot(sv3,sv3)+width2;
                        const float along=metal::dot(sv2,sv);
                        if(along<=0.f)weight=0.25f/d3+1.f/d2;
                        else {
                            const float length2=metal::dot(sv,sv);
                            if(length2<=along)weight=0.25f/d2+1.f/d3;
                            else {const float3 nearest=ln-(camPos+sv*(along/length2));
                                weight=1.f/(metal::dot(nearest,nearest)+width2)+0.25f/metal::max(d3,d2);}
                        }
                    }
                    lights+=words(ctx.interior->lights[i].color)*weight;
                }
                absorbed=(lights*current.diffuse)*((throughput+before)*.5f);
                if(!check(throughput,record)||!check(amount,record)||!check(absorbed,record))break;
            }
            const float hitZ=metal::fabs(hit.z)*f(params.step_width)+f(params.z_mid_difference);
            const float fogZ=hit.segment*f(params.step_width)+f(params.z_mid_difference);
            // bScaleAmbDiffDown: a hit the transmitting path continues from.
            const float scale=transmission&&bounce<params.max_bounces?lightAmount:0.0f;
            current=inside_material<false>(color,*ctx.interior,hit.surface,view,y,hitZ,fogZ,next.y/length,record,absorbs,
                ctx.material,ctx.material_samples,hit.position,cam,absolute);
            if(record.fault)break;
            shade_event(out,record,hit.surface,view,y,hitZ,fogZ,next.y/length,hit.position,absorbs,throughput,absorbs,absorbed,amount,cam,absolute,scale);
            if(record.fault)break;
            throughput*=current.transparency;
            throughput*=lightAmount;
            if(bounce<params.max_bounces){surface=hit.surface;position=hit.position;z=hit.z;ray=next;++bounce;refract=transmission;}
            else finished=true;
        } else {
            const bool farFog=(ctx.interior->flags&4u)!=0,squaredColor=ctx.interior->squared!=0;
            hit.surface.depth_packed=(farFog||squaredColor?open_air_depth(primaryDepth,color,record,farFog&&squaredColor):32768u)<<16;
            if(record.fault)break;
            const float back=f(params.back_distance)+f(params.z_mid_difference);
            shade_event(out,record,hit.surface,view,y,back,back,next.y/length,hit.position,false,throughput,false,float3(0),0,cam,absolute,0.0f);
            if(record.fault)break;
            finished=true;
        }
        if(!check(throughput,record))break;
        }
        if(finished) {
            // The frame ends: resume the innermost pending Fresnel reflection
            // with the restored surface and inside state.
            if(!depth){complete=true;break;}
            const PendingReflection p=pending[--depth];
            surface=p.surface;position=p.position;ray=p.ray;z=p.z;bounce=p.bounce;ctx.inside_word=p.inside;
            current.specular=p.specular;current.diffuse=p.diffuse;current.specular_w=p.specular_w;
            throughput=p.throughput*(p.fresnel+(1.0f-p.fresnel)*(1.0f-p.specular_w));
            refract=false;
        }
    }
    if(!record.fault&&!complete)record.fault|=2;
    if(record.fault)out->count=0;
}

// Shade half: the full reflection material of every recorded event, summed in
// SR_CalcRay's operation order, then SR's final output.
inline ExactColorPixel shade_pixel(constant ExactColorScene& color,constant NativeInteriorColorParams& interior,
    constant NativeReflectionMaterial& material,device const float* samples,
    device const NativeReflectionShadeRecord* in,ExactColorPixel original,thread NativeReflectionRecord& record) {
    if(!in->count||in->count>in->capacity||in->capacity>kNativeReflectionTreeEvents){record.fault|=1;return original;}
    device const auto* events=reinterpret_cast<device const NativeReflectionShadeEvent*>(in+1);
    float3 accumulated(0);
    for(uint k=0;k<in->count&&!record.fault;++k) {
        device const auto& e=events[k];
        if(e.suppress_fog>1||e.absorbs>1||e.weight[3]!=0||(!k&&(e.absorbs||e.suppress_fog))){record.fault|=1;break;}
        if(e.camera[3]!=0||e.absolute[3]!=0||e.reserved[0]||e.reserved[1]||e.reserved[2]||
           !metal::isfinite(e.scale_amount)||e.scale_amount<0||e.scale_amount>100){record.fault|=1;break;}
        const auto value=inside_material(color,interior,e.surface,float3(e.view[0],e.view[1],e.view[2]),e.y,e.zpos,e.fog_z,
            e.world_view_y,record,e.suppress_fog!=0,&material,samples,float3(e.position[0],e.position[1],e.position[2]),
            float3(e.camera[0],e.camera[1],e.camera[2]),float3(e.absolute[0],e.absolute[1],e.absolute[2]),e.scale_amount);
        if(record.fault)break;
        if(!k){accumulated=value.radiance;continue;}
        if(e.absorbs){
            accumulated+=float3(e.absorbed[0],e.absorbed[1],e.absorbed[2])*e.absorbed[3];
            if(!check(accumulated,record))break;
        }
        accumulated+=float3(e.weight[0],e.weight[1],e.weight[2])*value.radiance;
        if(!check(accumulated,record))break;
    }
    if(record.fault||!check(accumulated,record))return original;
    // SR final output: clamp, square-root color, then the gamma blend.
    float3 shaded=metal::clamp(accumulated,0.0f,1.0f);
    if(interior.squared)shaded=metal::sqrt(shaded);
    if(interior.gamma_mode) {
        const float3 adjusted=interior.gamma_mode>0?metal::sqrt(shaded):shaded*shaded;
        shaded+=(adjusted-shaded)*f(interior.gamma);
    }
    const uint3 rgb=uint3(metal::round(shaded*255.0f));
    ExactColorPixel output={rgb.x|(rgb.y<<8)|(rgb.z<<16),0,0,0};
    record.changed_pixels=output.rgb!=original.rgb;
    return output;
}

} // namespace mb3d_camera_inside_reflection

