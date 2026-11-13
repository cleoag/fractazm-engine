#pragma once
#ifndef MB3D_METAL_NATIVE_FLOAT
#error General reflections require Native GPU FP32
#endif
#include "generic_shadow.metal.h"
#include "generic_reflection_abi.h"
#include "comparison/metal_exact/native_reflection_math.metal.h"

namespace mb3d_generic_reflection {
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
using mb3d_native_reflection::Material;
using mb3d_native_reflection::material;
using mb3d_native_reflection::open_air_depth;
struct Context {
    constant NativeFormulaProgram* program;
    thread NativeResources* resources;
    thread NativeFormulaState* lane;
    thread NativeGeometryRecord* eval;
    NativeNormalParams normal_params;
    thread NativeNormalRecord* normal_record;
    thread NativeNormalResult* normal_result;
};
inline uint maximum_iterations(thread const Context& ctx){
    return uint(ctx.program->max_iterations)+(ctx.program->de_option==20?1u:0u);
}
inline void initialize_active_pixel(thread NativeFormulaState& lane,constant NativeFormulaProgram& p){
    // IniIt3D is deliberately partial: the task carries all other lanes.
    for(uint i=0;i<4;++i)lane.j[i]=lane.original_j[i]=p.julia_value[i];
    for(uint i=0;i<6;++i)lane.hybrid_counts[i]=p.hybrid_counts[i];
    lane.julia=p.julia;lane.inside=p.inside;lane.calc_smooth=p.calc_smooth;
    lane.rstop=p.radius_stop;lane.max_iterations=p.max_iterations;lane.derivative[0]=1.f;
    if(p.de_option==20)lane.rstop_d=-1.0e10f;
}
inline DE evaluate(float3 point,float stop,constant ExactScene&,
                   thread NativeReflectionRecord& record,thread Context& ctx){
    if(!check(point,record)||!check(stop,record))return {F64(0),0};
    const auto value=general_de(point,stop,*ctx.program,*ctx.resources,*ctx.lane,*ctx.eval);
    record.fault|=ctx.eval->fault;
    if(ctx.eval->fault==4)record.nonfinite=1;
    check(value.distance,record);
    return {F64::from_native(value.distance),uint(value.state.iteration)};
}
inline bool valid(constant NativeReflectionParams& p,constant ExactScene& scene,constant NativeFormulaProgram& program) {
    if(p.version!=1||p.width!=scene.width||p.height!=scene.height||
       !p.max_bounces||p.max_bounces>2||!p.max_steps||p.max_steps>65536||
       p.vary_de_stop>1||p.ao_enabled>1||p.shadows_enabled>1||p.first_step_random||
       !valid_general_scene(scene,program)||program.inside)return false;
    const uint scalars[]={p.amount,p.step_width,p.fov_y,p.z_mid_difference,
        p.z_corr,p.z_c_mul,p.back_distance};
    for(uint v:scalars)if(!metal::isfinite(f(v)))return false;
    if(f(p.amount)<0||f(p.amount)>1||f(p.step_width)<=0||f(p.fov_y)<=0||
       f(p.z_corr)<=0||f(p.z_c_mul)<=0||f(p.back_distance)<0)return false;
    for(uint i=0;i<3;++i)for(uint j=0;j<4;++j)
        if(!metal::isfinite(f(p.view_matrix[i][j]))||!metal::isfinite(f(p.ao_matrix[i][j])))return false;
    for(uint b=0;b<2;++b) {
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

inline void point_shadow(thread ExactSurface& surface,float3 position,float3 primaryRay,
                         float z,constant ExactScene& scene,constant ExactShadowParams& shadow,
                         constant NativeReflectionParams& params,thread NativeReflectionRecord& record,thread Context& ctx) {
    if(!params.shadows_enabled)return;
    ++record.shadow_rays;surface.shadow&=~1024u;
    const float3 direction={d(shadow.light_vectors[0]),d(shadow.light_vectors[1]),d(shadow.light_vectors[2])};
    const float3 normal=world(direction_normal(surface),scene);
    if(metal::dot(normal,direction)>0){surface.shadow|=1024;return;}
    float remaining=float(scene.width+scene.height)*0.6f*
        (1.0f+0.5f*metal::min(metal::fabs(z),d(scene.max_ray_length)*0.4f)*f(params.fov_y)/float(scene.height))*d(shadow.length_multiplier);
    const float projection=-metal::dot(direction,primaryRay)/metal::sqrt(metal::dot(direction,direction)*metal::dot(primaryRay,primaryRay));
    float stop=stop_at(z,scene),factor=1.0f;
    auto value=evaluate(position,stop,scene,record,ctx);
    if(!check(remaining,record)||!check(projection,record)||record.fault)return;
    bool ended=remaining<0;
    for(uint k=0;k<params.max_steps&&!ended;++k) {
        ++record.shadow_steps;
        const float previous=value.distance.value;
        const float step=metal::min(previous*f(scene.z_step_div)*factor,
                                    metal::max(stop,0.4f)*f(scene.step_limiter));
        remaining-=step;position-=direction*step;z+=step*projection;stop=stop_at(z,scene);
        value=evaluate(position,stop,scene,record,ctx);
        if(record.fault||!check(remaining,record))return;
        if(value.iteration>=maximum_iterations(ctx)||value.distance.value<stop){ended=true;break;}
        float current=metal::min(value.distance.value,previous+step);
        value.distance=F64::from_native(current);
        if(previous>current+1.0e-30f){const float ratio=step/(previous-current);factor=ratio<1?metal::max(0.5f,ratio):1.0f;}
        else factor=1;
        if(remaining<0)ended=true;
    }
    if(!ended){record.fault|=2;return;}
    if(remaining>0)surface.shadow|=1024;
}

inline void point_ao(thread ExactSurface& surface,float3 position,float z,uint bounce,
                     constant ExactScene& scene,constant NativeReflectionParams& params,
                     thread NativeReflectionRecord& record,thread Context& ctx) {
    if(!params.ao_enabled){surface.amb_shadow=5000;return;}
    surface.amb_shadow=0;
    if((surface.depth_packed>>16)>=32768||surface.si_gradient>=32768)return;
    constant auto& a=params.ao[bounce-1];
    const float stepMul=f(a.step_multiplier),deMul=f(a.de_multiplier);
    const float originalStop=metal::min(stop_at(z,scene),1.0e6f);
    const float stepAO=originalStop/f(scene.de_stop),maxDistance=f(a.max_distance_base)*metal::sqrt(stepAO);
    const float stop=(params.vary_de_stop?originalStop:f(scene.de_stop))/(stepMul*stepMul);
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
    for(uint i=0;i<a.ray_count;++i) {
        ++record.ao_rays;
        const float3 source=words(a.directions[i]);
        const float3 oriented={metal::dot(source,rotation[0]),metal::dot(source,rotation[1]),metal::dot(source,rotation[2])};
        const float3 ray=forward_matrix(oriented,params.ao_matrix);
        float travel=stepAO*stepMul,amount=1.0f;bool ended=false;
        if(!check(ray,record)||!check(travel,record))return;
        for(uint k=0;k<params.max_steps;++k) {
            ++record.ao_steps;
            const bool last=travel>maxDistance;if(last)travel=maxDistance;
            const auto value=evaluate(position+ray*travel,stop,scene,record,ctx);
            if(record.fault)return;
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
}

inline bool refine_primary(thread float3& position,float3 ray,thread float& z,
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
    float z,segment,stop;
    ExactSurface surface;
    bool hit;
};
inline SecondaryHit trace(float3 position,float3 direction,float3 primaryRay,float z,
                          constant ExactScene& scene,constant mb3d_native::NativeSurfaceParams& packing,
                          constant NativeReflectionParams& params,thread NativeReflectionRecord& record,thread Context& ctx) {
    SecondaryHit out={position,direction,z,0,0,{},false};
    const float priorZ=metal::fabs(z);
    const float projection=metal::dot(primaryRay,direction)/
        metal::sqrt(metal::dot(primaryRay,primaryRay)*metal::dot(direction,direction));
    float segment=metal::min(1.0f,f(scene.de_stop)*0.25f);
    z+=segment*projection;position+=direction*segment;
    float stop=stop_at(z,scene),stepCount=0,factor=1,lastStep=0;
    if(!check(projection,record))return out;
    auto value=evaluate(position,stop,scene,record,ctx);if(record.fault)return out;
    lastStep=value.distance.value*f(scene.z_step_div);
    bool hit=true,deLimited=true,ended=value.distance.value<=1.0e-10f;
    for(uint k=0;k<params.max_steps&&!ended;++k) {
        ++record.ray_steps;
        const float previousDE=value.distance.value;
        float step=metal::max(0.11f,(previousDE-f(scene.de_sub)*stop)*f(scene.z_step_div)*factor);
        const float limit=metal::max(stop,0.4f)*f(scene.step_limiter);
        stepCount+=limit<step?limit/step:1.0f;
        step=metal::min(step,limit);lastStep=step;segment+=step;
        if(!check(step,record)||!check(segment,record)||!check(stepCount,record))return out;
        if(segment>d(scene.max_ray_length)){hit=false;ended=true;break;}
        position+=direction*step;z+=step*projection;stop=stop_at(z,scene);
        value=evaluate(position,stop,scene,record,ctx);if(record.fault)return out;
        if(value.iteration>=maximum_iterations(ctx)){deLimited=false;ended=true;break;}
        if(value.distance.value<stop&&value.iteration>=scene.min_iterations){ended=true;break;}
        const float current=metal::min(value.distance.value,previousDE+lastStep);
        value.distance=F64::from_native(current);
        if(previousDE>current+1.0e-10f){const float ratio=lastStep/(previousDE-current);factor=ratio<1?metal::max(0.5f,ratio):1.0f;}
        else factor=1;
    }
    if(!ended){record.fault|=2;return out;}
    out.surface.shadow=uint(metal::round(metal::clamp((stepCount-1.0f)+1.0f,0.0f,1023.0f)))&1023u;
    if(!hit) {
        out.surface.depth_packed=0x80000000u;out.position=position;out.z=z;
        out.segment=segment;out.stop=stop;return out;
    }
    // CPU SR_CalcOpenAir skips both secondary refinement paths at count0.
    // Its primary-surface reconstruction above intentionally remains unconditional.
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
    const float farDepth=metal::sqrt((segment+priorZ)*(segment+priorZ)-priorZ+1.0e-30f);
    if(!check(farDepth,record))return out;
    out.surface.depth_packed=encoded_depth(farDepth,params,record)|n.roughness_byte;
    out.position=position;out.z=z;out.segment=segment;out.stop=stop;out.hit=true;
    return out;
}

inline ExactColorPixel reflect_pixel(constant ExactScene& scene,constant mb3d_native::NativeSurfaceParams& packing,constant ExactShadowParams& shadow,
    constant ExactColorScene& color,constant NativeReflectionParams& params,
    ExactPixel primary,ExactSurface surface,ExactColorPixel original,uint pixel,
    thread NativeReflectionRecord& record,thread Context& ctx) {
    if(!valid(params,scene,*ctx.program)||!valid_general_normals(ctx.normal_params,*ctx.program)||!valid_color(color)||!valid_generic_shadow(scene,*ctx.program,shadow)||
       color.width!=params.width||color.height!=params.height||shadow.enabled!=params.shadows_enabled||
       !valid_surface_color_descriptor(packing,native_selected_driver(),ctx.program->de_option)||primary.fault||surface.fault||original.fault||original.reserved0||original.reserved1||
       primary.reserved_tail||surface.reserved[0]||surface.reserved[1]||surface.reserved[2]||
       surface.si_gradient>65535||surface.o_trap>65535||surface.amb_shadow>65535||surface.shadow>65535) {
        record.fault|=1;return original;
    }
    const uint primaryDepth=surface.depth_packed>>16;
    if(primaryDepth>=32768)return original;
    record.active=1;
    initialize_active_pixel(*ctx.lane,*ctx.program);
    for(uint i=0;i<3;++i)if(surface.normal[i]< -32768||surface.normal[i]>32767){record.fault|=1;return original;}
    const float3 primaryRay={d(primary.ray[0]),d(primary.ray[1]),d(primary.ray[2])};
    const float3 start={d(primary.start[0]),d(primary.start[1]),d(primary.start[2])};
    float z=decoded_depth(int(surface.depth_packed>>8),params),stop=stop_at(z,scene);
    float3 position=start+primaryRay*z;
    if(!check(primaryRay,record)||!check(z,record)||!check(position,record))return original;
    ctx.lane->calc_smooth=0;
    for(uint axis=0;axis<3;++axis)ctx.lane->c[axis]=position[axis];
    if(surface.si_gradient>=32768) {
        // SR_ProcessRows performs partial IniIt3D and the decoded C advance
        // before the no-cut interior branch. With |Zend|>1 it keeps the color
        // and continues without any DE work or secondary ray. The remaining
        // short-range branch needs interior reflection material and stays open.
        if(metal::fabs(d(scene.max_ray_length))>1.f){record.primary_skipped=1;return original;}
        record.fault|=8;return original;
    }
    if(!refine_primary(position,primaryRay,z,stop,surface.depth_packed,scene,params,record,ctx))return original;
    const float rayLength=metal::length(primaryRay);
    if(!check(rayLength,record)||rayLength<=0){record.fault|=4;return original;}
    float3 view=inverse_matrix(primaryRay/rayLength,params.view_matrix);
    Material current=material(color,surface,view,float(pixel/scene.width)/float(scene.height),
        z*f(params.step_width)+f(params.z_mid_difference),z*f(params.step_width)+f(params.z_mid_difference),record);
    float3 accumulated=current.radiance;
    float3 throughput=float3(current.transparency*f(params.amount));
    float3 ray=primaryRay;
    for(uint bounce=1;bounce<=params.max_bounces&&!record.fault;++bounce) {
        if(metal::dot(metal::fabs(throughput),float3(0.3f,0.59f,0.11f))<1.0e-4f)break;
        const float3 normal=world(direction_normal(surface),scene);
        const float squared=metal::dot(normal,normal);
        if(!check(squared,record))break;
        if(squared<1.0e-10f)break;
        const float3 reflected=ray-normal*(2.0f*metal::dot(normal,ray)/squared);
        if(!check(reflected,record))break;
        ++record.rays;record.bounces=bounce;
        SecondaryHit hit=trace(position,reflected,primaryRay,z,scene,packing,params,record,ctx);
        if(record.fault)break;
        if(hit.hit)++record.hits;else ++record.misses;
        const float length=metal::length(reflected);
        if(!check(length,record)||length<=0){record.fault|=4;break;}
        view=inverse_matrix(reflected/length,params.view_matrix);
        const float y=metal::clamp(metal::asin(metal::clamp(view.y,-1.0f,1.0f))/f(params.fov_y)+0.5f,0.0f,1.0f);
        throughput*=current.specular;
        if(hit.hit) {
            point_shadow(hit.surface,hit.position,primaryRay,metal::fabs(hit.z),scene,shadow,params,record,ctx);
            if(record.fault)break;
            point_ao(hit.surface,hit.position,metal::fabs(hit.z),bounce,scene,params,record,ctx);
            if(record.fault)break;
            current=material(color,hit.surface,view,y,metal::fabs(hit.z)*f(params.step_width)+f(params.z_mid_difference),
                hit.segment*f(params.step_width)+f(params.z_mid_difference),record);
            accumulated+=throughput*current.radiance;
            throughput*=current.transparency;
            throughput*=f(params.amount);
            surface=hit.surface;position=hit.position;z=hit.z;ray=reflected;
        } else {
            hit.surface.depth_packed=open_air_depth(primaryDepth,color,record)<<16;
            const float back=f(params.back_distance)+f(params.z_mid_difference);
            const auto background=material(color,hit.surface,view,y,back,back,record);
            accumulated+=throughput*background.radiance;break;
        }
        if(!check(accumulated,record)||!check(throughput,record))break;
    }
    if(record.fault||!check(accumulated,record))return original;
    const uint3 rgb=uint3(metal::round(metal::clamp(accumulated,0.0f,1.0f)*255.0f));
    ExactColorPixel output={rgb.x|(rgb.y<<8)|(rgb.z<<16),0,0,0};
    record.changed_pixels=output.rgb!=original.rgb;
    return output;
}

} // namespace mb3d_generic_reflection

