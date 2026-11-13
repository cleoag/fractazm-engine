#pragma once
#include "scene_math.h"
#include "scene_catalog.h"
#include "shadow_abi.h"
#ifdef __METAL_VERSION__
#define EXACT_SHADOW_THREAD thread
#else
#define EXACT_SHADOW_THREAD
#endif
namespace mb3d_metal_exact {
inline ExactU32 clip15(F64 value) {
    value=single(value);
    value=value<F64(32767)?value:F64(32767);
    value=value>F64(0)?value:F64(0);
    return ExactU32(E80(value).to_i32_rne());
}
inline ExactSurface pack_surface(ExactScene scene,ExactSurfaceParams params,ExactPixel pixel) {
    ExactSurface out={};out.depth_packed=0x80000000U;out.amb_shadow=5000;
    out.fault=pixel.fault;
    if(params.version!=2||params.reserved[0]||params.reserved[1]||params.reserved[2]||
       !isfinite(D(params.z_corr))||D(params.z_corr)<=F64(0)||
       !isfinite(D(params.z_c_mul))||D(params.z_c_mul)<=F64(0)||
       !isfinite(D(params.color_plus))||!isfinite(D(params.color_multiplier))||
       !isfinite(D(params.color_stop_multiplier))||
       !isfinite(D(params.step_width))||D(params.step_width)<=F64(0)||
       pixel.reserved_tail)out.fault|=1;
    if(out.fault)return out;
    // Production evaluates the subtract/add as separate Single operations.
    F64 count=single(single(S(pixel.step_count)-F64(1))+F64(1));
    count=clamp0(count);if(count>F64(1023))count=F64(1023);
    out.shadow=ExactU32(E80(single(count)).to_i32_rne())&1023;
    if(!pixel.rm_result)return out;
    if(pixel.start_inside){out.fault|=8;return out;} // outside-only capability
    for(int i=0;i<3;++i)out.normal[i]=pixel.normal[i];
    if(pixel.rm_result==2)
        out.si_gradient=clip15(F64(32767)-S(pixel.smooth_iterations)*D(params.color_multiplier));
    else if(selected_formula(scene)==3) {
        const F64 log_term=D(params.color_stop_multiplier)*
            native_log(pasmax(S(scene.de_stop),S(pixel.final_stop))*D(params.step_width));
        out.si_gradient=clip15(F64(32767)-
            ((S(pixel.smooth_iterations)+D(params.color_plus))+log_term)*D(params.color_multiplier));
    } else
        out.si_gradient=clip15(F64(32767)-(S(pixel.smooth_iterations)+D(params.color_plus))*D(params.color_multiplier));
    // Normal probes leave the formula's final orbit color at the Y-minus
    // sample. The primary record exports that source value without narrowing.
    const F64 trap=D(pixel.orbit_trap);
    out.o_trap=clip15(trap*F64(4096));
    F64 depth=D(pixel.depth);if(signbit(depth))depth=F64(0);
    F64 encoded=D(params.z_c_mul)*(sqrt(depth*D(params.z_corr)+F64(1))-F64(1));
    ExactU32 raw=8388352U-ExactU32(E80(encoded).to_i32_rne());
    ExactI32 signed_raw=raw>0x7fffffffU?-ExactI32(~raw)-1:ExactI32(raw);
    if(signed_raw<0)signed_raw=0;if(signed_raw>8388352)signed_raw=8388352;
    out.depth_packed=ExactU32(signed_raw)<<8;
    if(!isfinite(encoded)||!isfinite(trap))out.fault|=4;
    return out;
}
// Hard-shadow row state is observable: preserve the production open-air skip
// across increasing x; independent rows may execute concurrently.
struct ShadowRowState { F64 last_found[6];ExactI32 threshold[6];F64 last_z; };
inline F64 shadow_stop(F64 z,ExactScene s) {
    return single(S(s.de_stop)*single(F64(1)+single(single(z)*S(s.de_stop_factor))));
}
inline F64 decode_shadow_depth(ExactU32 packed,ExactShadowParams p,F64 offset) {
    F64 q=(offset-F64(packed>>8))/D(p.z_c_mul)+F64(1);
    return (q*q-F64(1))/D(p.z_corr);
}
inline bool valid_shadow(ExactShadowParams p) {
    if(p.version!=1||p.enabled>1||p.light_mask!=1||!p.max_steps||p.max_steps>65536||
       !p.max_iterations_result||
       !(valid_formula_iteration_limit(p.max_iterations_result)||
         valid_formula_iteration_limit(p.max_iterations_result-1))||
       p.position_mask||p.reserved[0]||p.reserved[1])return false;
    if(!isfinite(D(p.z_corr))||D(p.z_corr)<=F64(0)||!isfinite(D(p.z_c_mul))||D(p.z_c_mul)<=F64(0)||
       !isfinite(D(p.fov_y))||D(p.fov_y)<F64(0)||!isfinite(D(p.length_multiplier))||D(p.length_multiplier)<=F64(0))return false;
    for(int i=0;i<18;++i)if(!isfinite(D(p.light_vectors[i])))return false;
    return true;
}
inline bool valid_shadow_scene(ExactScene scene,ExactShadowParams params) {
    return valid_scene(scene)&&valid_shadow(params)&&
           params.max_iterations_result==maximum_iteration_result(scene);
}
struct ShadowRefinement {
    Vec position;
    F64 depth,stop;
    ExactU32 fault;
};
inline ShadowRefinement refine_shadow_surface(Vec position,Vec ray,F64 z,F64 stop,
                                              DE value,ExactScene s,ExactShadowParams p,
                                              ExactU32 packed_depth) {
    const bool de_limited=value.iteration<p.max_iterations_result||value.distance<stop;
    if(!de_limited) {
        if(selected_formula(s)!=3)return {position,z,stop,8};
        const auto refined=refine_iteration_boundary(position,ray,z,stop,s);
        if(!finite_vec(refined.position)||!isfinite(refined.depth))
            return {refined.position,refined.depth,stop,4};
        // RMdoBinSearchIt receives the reconstructed local mZZ by reference.
        // In contrast, RMdoBinSearch below changes only the mct depth.
        return {refined.position,refined.depth,stop,0};
    }
    F64 laststep=decode_shadow_depth(packed_depth,p,D(0x415fffbff999999aULL))-z;
    F64 step=laststep*D(0xbfe0000000000000ULL),refined_z=z;
    for(ExactU32 remain=s.refinement_steps;remain>0&&abs(value.distance-stop)>D(0x3f50624dd2f1a9fcULL);--remain){
        refined_z=(E80(refined_z)+E80(step)).to_f64();position=refine_position(position,ray,step);
        stop=S(((E80(refined_z)*E80(S(s.de_stop_factor))+E80(F64(1)))*E80(S(s.de_stop))).to_f32_bits());
        if(remain<=1)break;
        value=distance(position,stop,s);
        step=abs(step)*(value.distance<stop?-S(0x3f0ccccdU):S(0x3f0ccccdU));
    }
    return {position,z,stop,(!finite_vec(position)||!isfinite(stop)||!isfinite(value.distance))?4u:0u};
}
inline ExactSurface shadow_pixel(ExactScene s,ExactShadowParams p,ExactPixel geometry,
                                 ExactSurface surface,EXACT_SHADOW_THREAD ShadowRowState& state,ExactU32 y) {
    if(!valid_shadow_scene(s,p)||geometry.fault||surface.fault){surface.fault|=1;return surface;}
    if(!p.enabled)return surface;
    for(int light=0;light<6;++light)if(state.threshold[light]>0)--state.threshold[light];
    surface.shadow&=~(p.light_mask<<10);
    if((surface.depth_packed>>16)>=32768||surface.si_gradient>=32768)return surface;
    Vec n=rotate(normalise({F64(surface.normal[0]),F64(surface.normal[1]),F64(surface.normal[2])}),s);
    Vec ray={D(geometry.ray[0]),D(geometry.ray[1]),D(geometry.ray[2])};
    Vec start={D(geometry.start[0]),D(geometry.start[1]),D(geometry.start[2])};
    F64 z=decode_shadow_depth(surface.depth_packed,p,D(0x415fffbfe0000000ULL)); // 8388351.5
    Vec position=add(start,mul(ray,z));
    F64 stop=shadow_stop(z,s);DE value=distance(position,stop,s);
    if(!isfinite(z)||!isfinite(value.distance)){surface.fault|=4;return surface;}
    const auto refined=refine_shadow_surface(position,ray,z,stop,value,s,p,surface.depth_packed);
    if(refined.fault){surface.fault|=refined.fault;return surface;}
    position=refined.position;z=refined.depth;stop=refined.stop;
    z=z-D(0x3fb999999999999aULL);position=add(position,mul(ray,D(0xbfb999999999999aULL)));
    F64 cap=D(s.max_ray_length)*D(0x3fd999999999999aULL);
    F64 minz=z<cap?z:cap;
    F64 length=single((F64(s.width+y)*D(0x3fe3333333333333ULL))*
        (F64(1)+((D(0x3fe0000000000000ULL)*minz)*D(p.fov_y))/F64(s.height))*D(p.length_multiplier));
    F64 threshold=(F64(1)+(z+length)*S(s.de_stop_factor))*D(0x400a666666666666ULL)/S(s.z_step_div);
    if(!finite_vec(position)||!isfinite(length)||!isfinite(threshold)||threshold<=F64(0)){surface.fault|=4;return surface;}
    for(int light=0;light<6;++light){
        if(!(p.light_mask&(1u<<light)))continue;
        bool skip=state.threshold[light]>0;
        if(skip){
            state.threshold[light]-=E80(abs(state.last_z-z)*D(0x400a666666666666ULL)/threshold).to_i32_rne();
            if(state.threshold[light]<=0){skip=false;state.threshold[light]=0;}
            else{
                state.last_found[light]=(state.last_found[light]-abs(state.last_z-z))-(F64(1)+z*S(s.de_stop_factor));
                if(state.last_found[light]<=F64(0)){skip=false;state.threshold[light]=0;}
            }
        }
        Vec direction={D(p.light_vectors[light*3]),D(p.light_vectors[light*3+1]),D(p.light_vectors[light*3+2])};
        F64 zmul=-dot(direction,ray)/sqrt(dot(direction,direction)*dot(ray,ray));
        if(dot(n,direction)>F64(0)){surface.shadow|=1u<<(10+light);state.threshold[light]=0;continue;}
        F64 remaining=length,zz=z;Vec sample=position;
        if(remaining>F64(0)){
            stop=shadow_stop(abs(zz),s);F64 factor(2);value=distance(sample,stop,s);
            bool ended=false;
            for(ExactU32 k=0;k<p.max_steps;++k){
                if(skip&&state.last_found[light]>remaining){remaining=F64(-1);ended=true;break;}
                F64 previous=value.distance;
                F64 width=pasmax(S(0x3de147aeU),single(((value.distance-single(S(s.de_sub)*stop))*S(s.z_step_div))*factor));
                F64 limit=single(pasmax(stop,S(0x3ecccccdU))*S(s.step_limiter));if(limit<width)width=limit;
                remaining=remaining-width;sample=add(sample,mul(direction,-width));zz=zz+width*zmul;
                stop=shadow_stop(abs(zz),s);value=distance(sample,stop,s);
                if(!isfinite(value.distance)||!isfinite(stop)||!isfinite(remaining)){surface.fault|=4;return surface;}
                if(value.iteration>=p.max_iterations_result||value.distance<=stop){ended=true;break;}
                if(value.distance>previous+width)value.distance=previous+width;
                if(!skip){if(value.distance>threshold){if(state.threshold[light]<3){state.last_found[light]=remaining;state.threshold[light]=3;}}
                          else state.threshold[light]=0;}
                if(previous>value.distance+D(0x39b4484bfeebc2a0ULL)){
                    F64 ratio=single(width/single(previous-value.distance));factor=ratio<F64(1)?pasmax(S(0x3f000000U),ratio):F64(1);
                }else factor=F64(1);
                if(remaining<F64(0)){ended=true;break;}
            }
            if(!ended){surface.fault|=2;return surface;}
        }
        if(remaining>F64(0)){surface.shadow|=1u<<(10+light);state.threshold[light]=0;}
    }
    state.last_z=z;return surface;
}

// Keep the legacy combined schedule above. All work below up to the
// ordered state transition depends only on this pixel and immutable scene data.
inline ExactShadowRay prepare_shadow_ray(ExactScene s,ExactShadowParams p,
                                         ExactPixel geometry,ExactSurface surface,ExactU32 y) {
    ExactShadowRay out={};out.version=1;
    if(!valid_shadow_scene(s,p)||geometry.fault||surface.fault||y>=s.height){
        out.fault=1;return out;
    }
    if(!p.enabled||(surface.depth_packed>>16)>=32768||surface.si_gradient>=32768)return out;
    out.active=1;
    Vec n=rotate(normalise({F64(surface.normal[0]),F64(surface.normal[1]),F64(surface.normal[2])}),s);
    Vec ray={D(geometry.ray[0]),D(geometry.ray[1]),D(geometry.ray[2])};
    Vec start={D(geometry.start[0]),D(geometry.start[1]),D(geometry.start[2])};
    F64 z=decode_shadow_depth(surface.depth_packed,p,D(0x415fffbfe0000000ULL)); // 8388351.5
    Vec position=add(start,mul(ray,z));
    F64 stop=shadow_stop(z,s);DE value=distance(position,stop,s);
    if(!isfinite(z)||!isfinite(value.distance)){out.fault=4;return out;}
    const auto refined=refine_shadow_surface(position,ray,z,stop,value,s,p,surface.depth_packed);
    if(refined.fault){out.fault=refined.fault;return out;}
    position=refined.position;z=refined.depth;stop=refined.stop;
    z=z-D(0x3fb999999999999aULL);position=add(position,mul(ray,D(0xbfb999999999999aULL)));
    F64 cap=D(s.max_ray_length)*D(0x3fd999999999999aULL);
    F64 minz=z<cap?z:cap;
    F64 length=single((F64(s.width+y)*D(0x3fe3333333333333ULL))*
        (F64(1)+((D(0x3fe0000000000000ULL)*minz)*D(p.fov_y))/F64(s.height))*D(p.length_multiplier));
    F64 threshold=(F64(1)+(z+length)*S(s.de_stop_factor))*D(0x400a666666666666ULL)/S(s.z_step_div);
    if(!finite_vec(position)||!isfinite(length)||!isfinite(threshold)||threshold<=F64(0)){
        out.fault=4;return out;
    }
    // valid_shadow admits light_mask == 1. No per-light or row-carried state is
    // consulted when computing its direction, backface test or first DE.
    Vec direction={D(p.light_vectors[0]),D(p.light_vectors[1]),D(p.light_vectors[2])};
    F64 zmul=-dot(direction,ray)/sqrt(dot(direction,direction)*dot(ray,ray));
    out.backface=dot(n,direction)>F64(0);
    out.position[0]=position.x.to_bits();out.position[1]=position.y.to_bits();out.position[2]=position.z.to_bits();
    out.z=z.to_bits();out.length=length.to_bits();out.threshold=threshold.to_bits();out.zmul=zmul.to_bits();
    if(!out.backface&&length>F64(0)){
        stop=shadow_stop(abs(z),s);value=distance(position,stop,s);
        out.initial_stop=stop.to_f32_bits();out.initial_de=value.distance.to_bits();
        out.initial_iterations=value.iteration;
    }
    return out;
}

inline bool valid_prepared_shadow_ray(ExactShadowRay ray) {
    if(ray.version!=1||ray.active>1||ray.backface>1||
       (ray.fault&~13u)||ray.reserved[0]||ray.reserved[1])return false;
    if(!ray.active&&(ray.backface||ray.initial_stop||ray.initial_iterations))return false;
    if(ray.fault)return true;
    for(unsigned axis=0;axis<3;++axis)if(!isfinite(D(ray.position[axis])))return false;
    if(!isfinite(D(ray.z))||!isfinite(D(ray.length))||!isfinite(D(ray.threshold))||
       !isfinite(D(ray.zmul))||!isfinite(D(ray.initial_de))||!isfinite(S(ray.initial_stop)))return false;
    if(ray.active&&D(ray.threshold)<=F64(0))return false;
    if(ray.active&&!ray.backface&&D(ray.length)>F64(0)&&S(ray.initial_stop)<=F64(0))return false;
    return true;
}

// Same row transition and ray march as shadow_pixel. Preparation does not
// consume or replace this state: even background pixels decrement counters,
// and the open-air decision still observes preceding x in the original order.
inline ExactSurface shadow_pixel_prepared(ExactScene s,ExactShadowParams p,ExactShadowRay ray,
                                          ExactSurface surface,EXACT_SHADOW_THREAD ShadowRowState& state) {
    if(!valid_shadow_scene(s,p)||!valid_prepared_shadow_ray(ray)||
       (ray.fault&1)||surface.fault){surface.fault|=1;return surface;}
    if(!p.enabled)return surface;
    for(int light=0;light<6;++light)if(state.threshold[light]>0)--state.threshold[light];
    surface.shadow&=~(p.light_mask<<10);
    if((surface.depth_packed>>16)>=32768||surface.si_gradient>=32768)return surface;
    if(ray.fault){surface.fault|=ray.fault;return surface;}
    if(!ray.active){surface.fault|=1;return surface;}
    F64 z=D(ray.z),length=D(ray.length),threshold=D(ray.threshold);
    Vec position={D(ray.position[0]),D(ray.position[1]),D(ray.position[2])};
    for(int light=0;light<6;++light){
        if(!(p.light_mask&(1u<<light)))continue;
        bool skip=state.threshold[light]>0;
        if(skip){
            state.threshold[light]-=E80(abs(state.last_z-z)*D(0x400a666666666666ULL)/threshold).to_i32_rne();
            if(state.threshold[light]<=0){skip=false;state.threshold[light]=0;}
            else{
                state.last_found[light]=(state.last_found[light]-abs(state.last_z-z))-(F64(1)+z*S(s.de_stop_factor));
                if(state.last_found[light]<=F64(0)){skip=false;state.threshold[light]=0;}
            }
        }
        Vec direction={D(p.light_vectors[light*3]),D(p.light_vectors[light*3+1]),D(p.light_vectors[light*3+2])};
        F64 zmul=D(ray.zmul);
        if(ray.backface){surface.shadow|=1u<<(10+light);state.threshold[light]=0;continue;}
        F64 remaining=length,zz=z;Vec sample=position;
        if(remaining>F64(0)){
            F64 stop=S(ray.initial_stop),factor(2);DE value={D(ray.initial_de),ray.initial_iterations};
            bool ended=false;
            for(ExactU32 k=0;k<p.max_steps;++k){
                if(skip&&state.last_found[light]>remaining){remaining=F64(-1);ended=true;break;}
                F64 previous=value.distance;
                F64 width=pasmax(S(0x3de147aeU),single(((value.distance-single(S(s.de_sub)*stop))*S(s.z_step_div))*factor));
                F64 limit=single(pasmax(stop,S(0x3ecccccdU))*S(s.step_limiter));if(limit<width)width=limit;
                remaining=remaining-width;sample=add(sample,mul(direction,-width));zz=zz+width*zmul;
                stop=shadow_stop(abs(zz),s);value=distance(sample,stop,s);
                if(!isfinite(value.distance)||!isfinite(stop)||!isfinite(remaining)){surface.fault|=4;return surface;}
                if(value.iteration>=p.max_iterations_result||value.distance<=stop){ended=true;break;}
                if(value.distance>previous+width)value.distance=previous+width;
                if(!skip){if(value.distance>threshold){if(state.threshold[light]<3){state.last_found[light]=remaining;state.threshold[light]=3;}}
                          else state.threshold[light]=0;}
                if(previous>value.distance+D(0x39b4484bfeebc2a0ULL)){
                    F64 ratio=single(width/single(previous-value.distance));factor=ratio<F64(1)?pasmax(S(0x3f000000U),ratio):F64(1);
                }else factor=F64(1);
                if(remaining<F64(0)){ended=true;break;}
            }
            if(!ended){surface.fault|=2;return surface;}
        }
        if(remaining>F64(0)){surface.shadow|=1u<<(10+light);state.threshold[light]=0;}
    }
    state.last_z=z;return surface;
}
} // namespace mb3d_metal_exact
#undef EXACT_SHADOW_THREAD
