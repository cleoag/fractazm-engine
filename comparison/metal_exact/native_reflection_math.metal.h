#pragma once
#ifndef MB3D_METAL_NATIVE_FLOAT
#error Native reflections must not enter the compatible arithmetic library
#endif
#include "native_reflection_abi.h"
#include "shadow_math.h"
#include "color_math.h"

namespace mb3d_native_reflection {
using namespace metal;
using namespace mb3d_metal_exact;
inline float f(uint bits) { return as_type<float>(bits); }
inline float d(ulong bits) { return native_decode_double(bits); }
inline Vec vec(float3 p) { return {F64::from_native(p.x),F64::from_native(p.y),F64::from_native(p.z)}; }
inline float3 native(Vec p) { return {p.x.value,p.y.value,p.z.value}; }
inline float3 words(constant uint* p) { return {f(p[0]),f(p[1]),f(p[2])}; }
inline bool finite(float3 p) { return all(metal::isfinite(p)); }
inline bool check(float x,thread NativeReflectionRecord& r) {
    if(metal::isfinite(x))return true;r.fault|=4;r.nonfinite=1;return false;
}
inline bool check(float3 x,thread NativeReflectionRecord& r) {
    if(finite(x))return true;r.fault|=4;r.nonfinite=1;return false;
}
inline float3 world(float3 p,ExactScene s) { return native(rotate(vec(p),s)); }
inline float3 inverse_matrix(float3 p,constant uint m[3][4]) {
    return {metal::dot(p,words(m[0])),metal::dot(p,words(m[1])),metal::dot(p,words(m[2]))};
}
inline float3 forward_matrix(float3 p,constant uint m[3][4]) {
    return words(m[0])*p.x+words(m[1])*p.y+words(m[2])*p.z;
}
inline float stop_at(float z,ExactScene s) {
    return f(s.de_stop)*(1.0f+metal::fabs(z)*f(s.de_stop_factor));
}
inline DE evaluate(float3 p,float stop,ExactScene s,thread NativeReflectionRecord& r) {
    if(!check(p,r)||!check(stop,r))return {F64(0),0};
    const auto value=distance(vec(p),F64::from_native(stop),s);
    check(value.distance.value,r);return value;
}
inline float decoded_depth(int packed,constant NativeReflectionParams& p) {
    const float q=(8388351.5f-float(packed))/f(p.z_c_mul)+1.0f;
    return (q*q-1.0f)/f(p.z_corr);
}
inline uint encoded_depth(float z,constant NativeReflectionParams& p,thread NativeReflectionRecord& r) {
    const float value=f(p.z_c_mul)*(metal::sqrt(metal::max(0.0f,z)*f(p.z_corr)+1.0f)-1.0f);
    if(!check(value,r))return 0;
    const uint raw=8388352u-uint(native_integer(value,true));
    const int signedRaw=as_type<int>(raw);
    return uint(metal::clamp(signedRaw,0,8388352))<<8;
}
inline float3 packed_normal(ExactSurface surface) {
    return float3(surface.normal[0],surface.normal[1],surface.normal[2])*3.051850947599719e-5f;
}
inline float3 direction_normal(ExactSurface surface) {
    // MakeDVecFromNormals normalizes for ray geometry; the colorizer's Single
    // packed normal deliberately remains unnormalized in packed_normal above.
    const float3 n=float3(surface.normal[0],surface.normal[1],surface.normal[2]);
    const float length=metal::length(n);
    return length>0?n/length:float3(0,0,-1);
}
inline bool valid(constant NativeReflectionParams& p,ExactScene scene) {
    if(p.version!=1||p.width!=scene.width||p.height!=scene.height||
       !p.max_bounces||p.max_bounces>2||!p.max_steps||p.max_steps>65536||
       p.vary_de_stop>1||p.ao_enabled>1||p.shadows_enabled>1||p.first_step_random||
       scene.formula_kind!=3||scene.bulb.power!=8||!valid_scene(scene))return false;
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

struct Material { float3 radiance,specular;float transparency; };
// Opaque CalcPixelColorSvec/CalcPixelColorSvecTrans share this admitted branch.
// Keep radiance before RGB quantization and carry palette specular separately.
inline Material material(constant ExactColorScene& s,ExactSurface p,float3 view,
                         float y,float zpos,float fogZ,thread NativeReflectionRecord& record) {
    Material out={float3(0),float3(0),1};
    const float3 dep=words(s.depth_colors[0])+y*(words(s.depth_colors[1])-words(s.depth_colors[0]));
    float3 result;float depthFactor;
    const uint depth=p.depth_packed>>16;
    if(depth<32768) {
        const float rough=float(p.depth_packed&255)*f(s.roughness);
        const float amb=color_ambient(s,p).value,fog=f(s.diffuse_shadowing)*(amb-1.0f)+1.0f;
        const float3 normal=packed_normal(p),light=words(s.light_direction),lightcol=words(s.light_color);
        const float weight=color_cos_table(s,F32::from_native(metal::dot(normal,light)),F32::from_native(rough)).value;
        float3 unshadowed=lightcol*weight,lit(0),spe(0);
        const bool noShadow=!(p.shadow&s.hs_mask)||!s.hs_calculated;
        const bool subtractAmbient=(s.hs_calculated^s.hs_enabled)!=0;
        if(noShadow)lit=lightcol*(weight*(subtractAmbient?amb:fog));
        const float reflected=metal::dot(light,view-normal*(2.0f*metal::dot(normal,view)));
        if(reflected>0) {
            float attenuation=(1.0f+metal::min(1.0f,rough*2.0f)*(1.0f/float(s.light_pow)-1.0f))*f(s.specular);
            if(attenuation>0) {
                unshadowed+=lightcol*(attenuation/float(s.light_pow));
                if(noShadow) {
                    float power=reflected;for(uint i=s.light_pow>>1;i;i>>=1)power*=power;
                    spe=lightcol*((attenuation*power)*(subtractAmbient?amb:fog));
                }
            }
        }
        const float raw=((float(p.si_gradient)-f(s.palette_start))*f(s.palette_mul)+f(s.palette_z_mul)*zpos)*16384.0f;
        if(!check(raw,record))return out;
        const int ir=native_integer(metal::clamp(raw,-1.0e9f,1.0e9f),true)&32767;
        int right=5;
        if(s.palette_positions[right]<ir){do{++right;}while(right<10&&s.palette_positions[right]<ir);}
        else {while(right>1&&s.palette_positions[right-1]>=ir)--right;}
        const int left=right-1;if(right>9)right=0;
        const float amount=float(ir-s.palette_positions[left])*f(s.palette_divisors[left]);
        const float3 diffuse=words(s.palette_diffuse[left])+amount*(words(s.palette_diffuse[right])-words(s.palette_diffuse[left]));
        out.specular=words(s.palette_specular[left])+amount*(words(s.palette_specular[right])-words(s.palette_specular[left]));
        const float ambB=(normal.y*0.5f+0.5f)*amb,ambA=amb-ambB;
        const float3 ambient=(words(s.ambient_colors[0])*ambA+words(s.ambient_colors[1])*ambB)*diffuse;
        const float3 scaledDiffuse=diffuse*f(s.diffuse);
        lit=(ambient+scaledDiffuse*lit)+out.specular*spe;
        unshadowed=ambient+(scaledDiffuse*unshadowed)*(amb*0.3f);
        const float remainder=metal::max(0.0f,1.0f-amb);
        float bounce=remainder*remainder;bounce*=bounce;
        const float3 scale=diffuse*(bounce*f(s.indirect_reflect))+(remainder-bounce);
        result=lit+unshadowed*(diffuse*scale);
        depthFactor=metal::max(0.0f,1.0f+float(int(depth)-28000)*f(s.depth));
    } else {
        result=dep;
        depthFactor=metal::max(0.0f,1.0f-(60768.0f-float(depth))*f(s.depth));
    }
    if(depthFactor<1.0f){const float v=1.0f-depthFactor;depthFactor=1.0f-v*v;}
    result*=depthFactor;
    const float shadow=float(p.shadow&1023);
    const float fog=(shadow-f(s.shadow_start)-f(s.shadow_z_mul)*fogZ)*f(s.shadow_gradient);
    const float fogB=metal::min(1.0f,shadow*f(s.dynamic_fog_mul))*fog;
    result+=dep*metal::max(0.0f,1.0f-depthFactor);
    result+=words(s.fog_colors[0])*(fog-fogB)+words(s.fog_colors[1])*fogB;
    if(!check(result,record)||!check(out.specular,record)||!check(depthFactor,record))return out;
    out.radiance=metal::clamp(result*(1.0f/255.0f),0.0f,8.0f);
    out.transparency=metal::min(1.0f,depthFactor);
    return out;
}

inline void point_shadow(thread ExactSurface& surface,float3 position,float3 primaryRay,
                         float z,ExactScene scene,ExactShadowParams shadow,
                         constant NativeReflectionParams& params,thread NativeReflectionRecord& record) {
    if(!params.shadows_enabled)return;
    ++record.shadow_rays;surface.shadow&=~1024u;
    const float3 direction={d(shadow.light_vectors[0]),d(shadow.light_vectors[1]),d(shadow.light_vectors[2])};
    const float3 normal=world(direction_normal(surface),scene);
    if(metal::dot(normal,direction)>0){surface.shadow|=1024;return;}
    float remaining=float(scene.width+scene.height)*0.6f*
        (1.0f+0.5f*metal::min(metal::fabs(z),d(scene.max_ray_length)*0.4f)*f(params.fov_y)/float(scene.height))*d(shadow.length_multiplier);
    const float projection=-metal::dot(direction,primaryRay)/metal::sqrt(metal::dot(direction,direction)*metal::dot(primaryRay,primaryRay));
    float stop=stop_at(z,scene),factor=1.0f;
    auto value=evaluate(position,stop,scene,record);
    if(!check(remaining,record)||!check(projection,record)||record.fault)return;
    bool ended=remaining<0;
    for(uint k=0;k<params.max_steps&&!ended;++k) {
        ++record.shadow_steps;
        const float previous=value.distance.value;
        const float step=metal::min(previous*f(scene.z_step_div)*factor,
                                    metal::max(stop,0.4f)*f(scene.step_limiter));
        remaining-=step;position-=direction*step;z+=step*projection;stop=stop_at(z,scene);
        value=evaluate(position,stop,scene,record);
        if(record.fault||!check(remaining,record))return;
        if(value.iteration>=maximum_iteration_result(scene)||value.distance.value<stop){ended=true;break;}
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
                     ExactScene scene,constant NativeReflectionParams& params,
                     thread NativeReflectionRecord& record) {
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
            const auto value=evaluate(position+ray*travel,stop,scene,record);
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
                           thread float& stop,uint packed,ExactScene scene,
                           constant NativeReflectionParams& params,thread NativeReflectionRecord& record) {
    auto value=evaluate(position,stop,scene,record);if(record.fault)return false;
    if(value.distance.value<stop*0.5f){record.primary_skipped=1;return false;}
    if(value.iteration<maximum_iteration_result(scene)||value.distance.value<=stop) {
        float step=(z-decoded_depth(int(packed>>8)-1,params))*(-0.5f);
        for(uint remain=scene.refinement_steps;remain&&metal::fabs(value.distance.value-stop)>0.001f;--remain) {
            z+=step;position+=ray*step;stop=stop_at(z,scene);
            if(!check(position,record)||!check(stop,record))return false;
            if(remain<=1)break;
            value=evaluate(position,stop,scene,record);if(record.fault)return false;
            step=metal::fabs(step)*(value.distance.value<stop?-0.55f:0.55f);
        }
    } else {
        const auto refined=refine_iteration_boundary(vec(position),vec(ray),F64::from_native(z),F64::from_native(stop),scene);
        position=native(refined.position);z=refined.depth.value;
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
                          ExactScene scene,ExactSurfaceParams packing,
                          constant NativeReflectionParams& params,thread NativeReflectionRecord& record) {
    SecondaryHit out={position,direction,z,0,0,{},false};
    const float priorZ=metal::fabs(z);
    const float projection=metal::dot(primaryRay,direction)/
        metal::sqrt(metal::dot(primaryRay,primaryRay)*metal::dot(direction,direction));
    float segment=metal::min(1.0f,f(scene.de_stop)*0.25f);
    z+=segment*projection;position+=direction*segment;
    float stop=stop_at(z,scene),stepCount=0,factor=1,lastStep=0;
    if(!check(projection,record))return out;
    auto value=evaluate(position,stop,scene,record);if(record.fault)return out;
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
        value=evaluate(position,stop,scene,record);if(record.fault)return out;
        if(value.iteration>=maximum_iteration_result(scene)){deLimited=false;ended=true;break;}
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
    if(deLimited) {
        float step=lastStep*(-0.5f);
        value=evaluate(position,stop,scene,record);if(record.fault)return out;
        for(uint remain=scene.refinement_steps;remain&&metal::fabs(value.distance.value-stop)>0.001f;--remain) {
            segment+=step;z+=step*projection;position+=direction*step;stop=stop_at(z,scene);
            if(!check(position,record)||!check(stop,record)||!check(segment,record))return out;
            if(remain<=1)break;
            value=evaluate(position,stop,scene,record);if(record.fault)return out;
            step=metal::fabs(step)*(value.distance.value<stop?-0.55f:0.55f);
        }
    } else {
        const float old=segment;
        const auto refined=refine_iteration_boundary(vec(position),vec(direction),F64::from_native(segment),F64::from_native(stop),scene);
        position=native(refined.position);segment=refined.depth.value;z+=(segment-old)*projection;
        if(!check(position,record)||!check(segment,record)||!check(z,record))return out;
    }
    const auto normal=raw_normal_with_trap(vec(position),F64::from_native(metal::fabs(z)),F64::from_native(stop),scene);
    const auto center=bulb_distance(vec(position),F64::from_native(stop),scene,true);
    const float smooth=center.state.smooth.value;
    float3 n=native(normal.normal);
    if(!check(n,record)||!check(normal.trap.value,record)||!check(smooth,record))return out;
    const float magnitude=metal::max(metal::fabs(n.x),metal::max(metal::fabs(n.y),metal::fabs(n.z)));
    if(magnitude>0) {
        // Scale first: squaring a finite tiny gradient or forming its reciprocal
        // can underflow/overflow FP32 even when its unit vector is representable.
        n/=magnitude;n=(n/metal::length(n))*32767.0f;
        if(!check(n,record))return out;
        for(uint i=0;i<3;++i) {
            int packedNormal=native_integer(n[i],true);
            if(packedNormal< -32768||packedNormal>32767)packedNormal=-32768;
            out.surface.normal[i]=packedNormal;
        }
    }
    // CPU's +1e-100 normal-packing term maps an exact zero gradient to zero,
    // then RMCalculateNormals applies this explicit nonzero-normal rule.
    if(!out.surface.normal[0]&&!out.surface.normal[1])out.surface.normal[2]=out.surface.normal[2]>0?32767:-32767;
    float gradient=32767.0f-smooth*d(packing.color_multiplier);
    if(deLimited)gradient=32767.0f-(smooth+d(packing.color_plus)+d(packing.color_stop_multiplier)*
        metal::log(stop*f(params.step_width)))*d(packing.color_multiplier);
    if(!check(gradient,record))return out;
    out.surface.si_gradient=uint(native_integer(metal::clamp(gradient,0.0f,32767.0f),true));
    const float trap=normal.trap.value*4096.0f;
    if(!check(trap,record))return out;
    out.surface.o_trap=uint(native_integer(metal::clamp(trap,0.0f,32767.0f),true));
    const float farDepth=metal::sqrt((segment+priorZ)*(segment+priorZ)-priorZ+1.0e-30f);
    if(!check(farDepth,record))return out;
    out.surface.depth_packed=encoded_depth(farDepth,params,record);
    out.position=position;out.z=z;out.segment=segment;out.stop=stop;out.hit=true;
    return out;
}

// SR_CalcOpenAir with bFarFog or bCalcPixColSqr; with both (twice) each
// fog term is squared and the difference rooted once more.
inline uint open_air_depth(uint primaryDepth,constant ExactColorScene& color,thread NativeReflectionRecord& record,bool twice=false) {
    const float depth=f(color.depth);
    if(metal::fabs(depth)<=1.0e-10f)return 32768;
    float current=1.0f-metal::max(0.0f,1.0f+float(int(primaryDepth)-28000)*depth);
    if(current>0){current*=current;if(twice)current*=current;}
    float background=1.0f-metal::max(0.0f,1.0f-28000.0f*depth);
    if(background>0){background*=background;if(twice)background*=background;}
    float delta=background-current;if(delta>0){delta=metal::sqrt(delta);if(twice)delta=metal::sqrt(delta);}
    const float value=60768.0f-delta/depth;
    if(!check(value,record))return 32768;
    return uint(metal::round(metal::clamp(value,32768.0f,65535.0f)));
}

inline ExactColorPixel reflect_pixel(ExactScene scene,ExactSurfaceParams packing,ExactShadowParams shadow,
    constant ExactColorScene& color,constant NativeReflectionParams& params,
    ExactPixel primary,ExactSurface surface,ExactColorPixel original,uint pixel,
    thread NativeReflectionRecord& record) {
    if(!valid(params,scene)||!valid_color(color)||!valid_shadow_scene(scene,shadow)||
       color.width!=params.width||color.height!=params.height||shadow.enabled!=params.shadows_enabled||
       packing.version!=2||primary.fault||surface.fault||original.fault||original.reserved0||original.reserved1||
       primary.reserved_tail||surface.reserved[0]||surface.reserved[1]||surface.reserved[2]||
       surface.si_gradient>32767||surface.o_trap>65535||surface.amb_shadow>65535||surface.shadow>65535) {
        record.fault|=1;return original;
    }
    const uint primaryDepth=surface.depth_packed>>16;
    if(primaryDepth>=32768)return original;
    record.active=1;
    for(uint i=0;i<3;++i)if(surface.normal[i]< -32768||surface.normal[i]>32767){record.fault|=1;return original;}
    const float3 primaryRay={d(primary.ray[0]),d(primary.ray[1]),d(primary.ray[2])};
    const float3 start={d(primary.start[0]),d(primary.start[1]),d(primary.start[2])};
    float z=decoded_depth(int(surface.depth_packed>>8),params),stop=stop_at(z,scene);
    float3 position=start+primaryRay*z;
    if(!check(primaryRay,record)||!check(z,record))return original;
    if(!refine_primary(position,primaryRay,z,stop,surface.depth_packed,scene,params,record))return original;
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
        SecondaryHit hit=trace(position,reflected,primaryRay,z,scene,packing,params,record);
        if(record.fault)break;
        if(hit.hit)++record.hits;else ++record.misses;
        const float length=metal::length(reflected);
        if(!check(length,record)||length<=0){record.fault|=4;break;}
        view=inverse_matrix(reflected/length,params.view_matrix);
        const float y=metal::clamp(metal::asin(metal::clamp(view.y,-1.0f,1.0f))/f(params.fov_y)+0.5f,0.0f,1.0f);
        throughput*=current.specular;
        if(hit.hit) {
            point_shadow(hit.surface,hit.position,primaryRay,metal::fabs(hit.z),scene,shadow,params,record);
            if(record.fault)break;
            point_ao(hit.surface,hit.position,metal::fabs(hit.z),bounce,scene,params,record);
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
} // namespace mb3d_native_reflection
