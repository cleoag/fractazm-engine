#pragma once
#include "shadow_math.h"
#include "shadow_trace_abi.h"
#ifdef __METAL_VERSION__
#define EXACT_TRACE_THREAD thread
#else
#define EXACT_TRACE_THREAD
#endif

namespace mb3d_metal_exact {

// Run precisely the same arithmetic as shadow_pixel_prepared without its row
// shortcut. The DE trajectory does not depend on skip or on the row counters.
inline ExactShadowTrace trace_shadow_ray(ExactScene s, ExactShadowParams p,
                                         ExactShadowRay ray) {
    ExactShadowTrace out={};out.version=1;
    if(!valid_shadow_scene(s,p)||!valid_prepared_shadow_ray(ray)||(ray.fault&1)){
        out.fault=1;return out;
    }
    if(!p.enabled)return out;
    out.active=ray.active;
    if(!ray.active||ray.fault)return out;
    F64 remaining=D(ray.length);
    out.final_remaining=remaining.to_bits();
    if(ray.backface||!(remaining>F64(0)))return out;

    Vec direction={D(p.light_vectors[0]),D(p.light_vectors[1]),D(p.light_vectors[2])};
    Vec sample={D(ray.position[0]),D(ray.position[1]),D(ray.position[2])};
    F64 zz=D(ray.z),zmul=D(ray.zmul),threshold=D(ray.threshold);
    F64 stop=S(ray.initial_stop),factor(2);
    DE value={D(ray.initial_de),ray.initial_iterations};
    bool ended=false;
    for(ExactU32 k=0;k<p.max_steps;++k){
        // This checkpoint precedes the DE operation, its fault/hit checks and
        // the final budget check. Do not include the post-step terminal value
        // or a hypothetical checkpoint after the last admitted iteration.
        if(!out.has_loop||remaining<D(out.min_remaining))out.min_remaining=remaining.to_bits();
        out.has_loop=1;

        F64 previous=value.distance;
        F64 width=pasmax(S(0x3de147aeU),single(((value.distance-single(S(s.de_sub)*stop))*S(s.z_step_div))*factor));
        F64 limit=single(pasmax(stop,S(0x3ecccccdU))*S(s.step_limiter));if(limit<width)width=limit;
        remaining=remaining-width;sample=add(sample,mul(direction,-width));zz=zz+width*zmul;
        stop=shadow_stop(abs(zz),s);value=distance(sample,stop,s);
        if(!isfinite(value.distance)||!isfinite(stop)||!isfinite(remaining)){
            out.terminal_fault=4;
            // The row never consumes remaining when this fault is reached.
            // Keep its ABI finite even when the failed computation was not.
            out.final_remaining=isfinite(remaining)?remaining.to_bits():F64(0).to_bits();
            return out;
        }
        out.final_remaining=remaining.to_bits();
        if(value.iteration>=p.max_iterations_result||value.distance<=stop){ended=true;break;}
        if(value.distance>previous+width)value.distance=previous+width;
        // A reachable non-skip row starts this part with threshold zero.
        // Retain the last write even if a later low DE clears the counter.
        if(value.distance>threshold){
            if(out.final_threshold<3){
                out.last_found=remaining.to_bits();out.writes_last_found=1;out.final_threshold=3;
            }
        }else out.final_threshold=0;
        if(previous>value.distance+D(0x39b4484bfeebc2a0ULL)){
            F64 ratio=single(width/single(previous-value.distance));factor=ratio<F64(1)?pasmax(S(0x3f000000U),ratio):F64(1);
        }else factor=F64(1);
        if(remaining<F64(0)){ended=true;break;}
    }
    if(!ended)out.terminal_fault=2;
    return out;
}

inline bool valid_shadow_trace(ExactShadowTrace trace) {
    if(trace.version!=1||trace.active>1||trace.fault>1||
       (trace.terminal_fault!=0&&trace.terminal_fault!=2&&trace.terminal_fault!=4)||
       trace.has_loop>1||(trace.final_threshold!=0&&trace.final_threshold!=3)||
       trace.writes_last_found>1||trace.reserved[0]||trace.reserved[1]||trace.reserved[2])return false;
    if(!isfinite(D(trace.min_remaining))||!isfinite(D(trace.final_remaining))||
       !isfinite(D(trace.last_found)))return false;
    if(trace.has_loop&&(!trace.active||D(trace.min_remaining)<F64(0)))return false;
    if(!trace.has_loop&&(trace.min_remaining||trace.terminal_fault||
                        trace.final_threshold||trace.writes_last_found))return false;
    if(!trace.writes_last_found&&trace.last_found)return false;
    if(trace.final_threshold==3&&!trace.writes_last_found)return false;
    if(trace.fault&&(trace.active||trace.has_loop||trace.final_remaining))return false;
    return true;
}

// The only ordered work is the original row state transition. For skip=true,
// last_found is constant throughout the march, so the first matching checkpoint
// exists exactly when last_found > min_remaining. No monotonic-width assumption
// is needed. A shortcut precedes any terminal hit or fault in the full trace.
inline ExactSurface shadow_pixel_traced(ExactScene s, ExactShadowParams p,
                                        ExactShadowRay ray, ExactShadowTrace trace,
                                        ExactSurface surface, EXACT_TRACE_THREAD ShadowRowState& state) {
    if(!valid_shadow_scene(s,p)||!valid_prepared_shadow_ray(ray)||
       (ray.fault&1)||surface.fault||!valid_shadow_trace(trace)||trace.fault){
        surface.fault|=1;return surface;
    }
    const ExactU32 expectedActive=p.enabled?ray.active:0;
    const ExactU32 expectedLoop=expectedActive&&!ray.fault&&!ray.backface&&D(ray.length)>F64(0);
    if(trace.active!=expectedActive||trace.has_loop!=expectedLoop||
       (trace.has_loop&&D(trace.min_remaining)>D(ray.length))){
        surface.fault|=1;return surface;
    }
    if(!p.enabled)return surface;
    for(int light=0;light<6;++light)if(state.threshold[light]>0)--state.threshold[light];
    surface.shadow&=~(p.light_mask<<10);
    if((surface.depth_packed>>16)>=32768||surface.si_gradient>=32768)return surface;
    if(ray.fault){surface.fault|=ray.fault;return surface;}
    if(!ray.active){surface.fault|=1;return surface;}
    F64 z=D(ray.z),length=D(ray.length),threshold=D(ray.threshold);
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
        if(ray.backface){surface.shadow|=1u<<(10+light);state.threshold[light]=0;continue;}
        F64 remaining=length;
        if(remaining>F64(0)){
            if(skip&&state.last_found[light]>D(trace.min_remaining))remaining=F64(-1);
            else{
                if(!skip){
                    if(trace.writes_last_found)state.last_found[light]=D(trace.last_found);
                    state.threshold[light]=ExactI32(trace.final_threshold);
                }
                if(trace.terminal_fault){surface.fault|=trace.terminal_fault;return surface;}
                remaining=D(trace.final_remaining);
            }
        }
        if(remaining>F64(0)){surface.shadow|=1u<<(10+light);state.threshold[light]=0;}
    }
    state.last_z=z;return surface;
}

} // namespace mb3d_metal_exact
#undef EXACT_TRACE_THREAD
