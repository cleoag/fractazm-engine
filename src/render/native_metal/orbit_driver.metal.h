#pragma once
#include "formula_abi.h"
#include "native_inside_math.h"
#include "internal_formulas.metal.h"
#include "native_smoothing_math.h"
#include "numerical_gradient.metal.h"
#include "interpolation_radius.h"
#include "generated/formula_indefinite.metal.h"
namespace mb3d_native {
using namespace metal;
constant uint native_smoothing_policy [[function_constant(17)]];
inline uint native_selected_smoothing_policy() {
    return is_function_constant_defined(native_smoothing_policy)?native_smoothing_policy:1u;
}
constant uint native_orbit_driver [[function_constant(16)]];
inline uint native_selected_driver() {
    return is_function_constant_defined(native_orbit_driver)?native_orbit_driver:0u;
}
// DE combination: FormulaType and the second hybrid part's orbit driver.
// Undefined means FormulaType 0, which never reads the second program.
constant uint native_formula_type [[function_constant(19)]];
constant uint native_orbit_driver2 [[function_constant(20)]];
inline uint native_selected_formula_type() {
    return is_function_constant_defined(native_formula_type)?native_formula_type:0u;
}
inline uint native_selected_driver2() {
    return is_function_constant_defined(native_orbit_driver2)?native_orbit_driver2:0xffffffffu;
}
// Driver of a hybrid part (0, or the combination's second part 1). A template
// keeps it a pipeline constant even where the compiler does not inline the
// orbit; a runtime driver would retain every driver's branches.
template<uint Part> inline uint native_part_driver() {
    return Part?native_selected_driver2():native_selected_driver();
}
// Defined by the offline external-formula generator, not a device callback.
inline bool native_formula_step(thread NativeFormulaState&,constant NativeFormulaSlot&,
    thread const NativeResources&,thread NativeFormulaFault&,uint);
inline bool native_formula_specialization_matches(constant NativeFormulaProgram&);

inline float native_radius(thread const NativeFormulaState& s,bool four) {
    float r=(s.xyz[0]*s.xyz[0]+s.xyz[1]*s.xyz[1])+s.xyz[2]*s.xyz[2];
    return four?r+s.xyz[3]*s.xyz[3]:r;
}
// Preserve doHybridPasDE's separate products/adds in the declared binary32
// projection, with its original left association. Scope this rule to DE2.
// Keep the helper outlined so CSE cannot replace the generic radius with these
// noncontracting expressions. Pass coordinates by value, not the complete state.
__attribute__((noinline)) inline float native_de2_radius(float3 xyz) {
#pragma clang fp contract(off)
    return (xyz.x*xyz.x+xyz.y*xyz.y)+xyz.z*xyz.z;
}
inline bool native_fail(thread NativeFormulaFault& f,uint code,
    thread const NativeFormulaState& s,uint id=0) {
    if(!f.code){f.code=code;f.formula_id=id;f.slot=s.current_slot;f.iteration=uint(s.iteration);}
    return false;
}
inline bool native_combination_valid(constant NativeFormulaProgram& p) {
    const uint type=native_selected_formula_type();
    if(p.formula_type!=type)return false;
    if(!type)return !p.mix_color && !as_type<uint>(p.mix_power) && p.combination_smooth==0.f;
    return type<=CombineMix && p.mix_color<=2 && isfinite(p.mix_power) &&
        isfinite(p.combination_smooth) && p.combination_smooth>0.f;
}
inline bool native_program_valid(constant NativeFormulaProgram& p,uint driver=native_selected_driver()) {
    if(native_selected_smoothing_policy()>1 || p.version!=kFormulaAbiVersion || !native_formula_specialization_matches(p) ||
        !is_function_constant_defined(native_orbit_driver) || p.driver!=driver ||
        p.slot_count!=6 || p.driver>HybridDifsNoInitialization ||
        p.start_slot>p.end_slot || p.repeat_slot>p.end_slot || p.end_slot>=6 ||
        p.max_iterations<1 || p.max_iterations>(1<<20) || !p.work_limit || p.work_limit>(1u<<24) ||
        !p.local_work_limit || p.local_work_limit>(1u<<20) || !native_combination_valid(p))return false;
    for(uint i=0;i<4;++i){if(!isfinite(p.julia_value[i]))return false;
        for(uint j=0;j<4;++j)if(!isfinite(p.rotation[i][j]))return false;}
    for(uint i=0;i<6;++i){constant auto& s=p.slots[i];
        if(s.version!=kFormulaAbiVersion || s.namespace_id>1 || s.enabled>1 ||
            s.param_count>58 || s.reserved[0] || s.reserved[1])return false;}
    return isfinite(p.radius_stop)&&p.radius_stop>0.f&&isfinite(p.log_radius_stop)&&isfinite(p.de_scale)&&
        isfinite(p.de_offset)&&isfinite(p.de_offset_floor)&&isfinite(p.stop_scale)&&p.stop_scale>0.f&&
        isfinite(p.gradient_radius_stop)&&p.gradient_radius_stop>0.f;
}
// FP32 range exit: a step whose orbit leaves the binary32 range (overflow,
// or NaN from overflowed intermediates) reports overflow, not a fault. The
// orbit driver then escapes from the last finite state; double-precision CPU
// orbits at these points are astronomically large and escape as well.
inline bool native_step(thread NativeFormulaState& s,constant NativeFormulaProgram& p,
    thread const NativeResources& resources,thread NativeFormulaFault& f,uint slot,thread bool& overflow) {
    overflow=false;
    if(slot>=6)return native_fail(f,InvalidFormulaAbi,s);
    s.current_slot=slot;constant auto& spec=p.slots[slot];
    if(f.work>=f.limit)return native_fail(f,FormulaWorkExhausted,s,spec.id);
    ++f.work;
    if(!spec.enabled)return true; // CPU's explicit EmptyFormula slot
    bool ok=spec.namespace_id==InternalFormula?
        native_internal_formula_step(s,spec,resources,f):
        (spec.id>=10353&&spec.id<=10357?native_conditional_step(s,spec):native_formula_step(s,spec,resources,f,p.local_work_limit));
    if(!ok || f.code){
        if(!f.code)f.code=UnsupportedFormula;
        f.formula_id=spec.id;f.slot=slot;f.iteration=uint(s.iteration);return false;
    }
    overflow=!all(isfinite(float4(s.xyz[0],s.xyz[1],s.xyz[2],s.xyz[3])))||!isfinite(s.rout)||!isfinite(s.vary_scale);
    return true;
}
// Single-step probes keep the strict contract: a range exit is a fault.
inline bool native_step(thread NativeFormulaState& s,constant NativeFormulaProgram& p,
    thread const NativeResources& resources,thread NativeFormulaFault& f,uint slot) {
    bool overflow=false;
    if(!native_step(s,p,resources,f,slot,overflow))return false;
    return overflow?native_fail(f,NonFiniteFormula,s,p.slots[slot].id):true;
}
inline void native_smooth_legacy(thread NativeFormulaState& s,constant NativeFormulaProgram& p,uint n) {
    if(s.rout<=1.f)s.smooth=float(s.iteration);
    else if(s.rold<1.f)s.smooth=float(s.iteration)+p.log_radius_stop-log(.5f*log(s.rout))*p.smoothing_power[n];
    else {float d=log(.5f*log(s.rout)),old=log(.5f*log(s.rold));
        s.smooth=float(s.iteration)+(p.log_radius_stop-d)/(d-old);}
}
inline void native_smooth(thread NativeFormulaState& s,constant NativeFormulaProgram& p,uint n,
    thread const NativeResources& resources,thread NativeFormulaFault& fault) {
    thread NativeSmoothingCounters* counts=resources.smoothing;
    if(counts&&counts->calls>=kNativeSmoothingMaxCalls){native_fail(fault,FormulaWorkExhausted,s);return;}
    const uint policy=native_selected_smoothing_policy();
    uint status=mb3d_native_smoothing::Invalid;
    if(policy==0){native_smooth_legacy(s,p,n);status=isfinite(s.smooth)?0u:3u;}
    else if(policy==1){const auto result=mb3d_native_smoothing::calculate(s.rout,s.rold,
        p.log_radius_stop,p.smoothing_power[n],s.iteration);s.smooth=result.value;status=uint(result.status);}
    else{native_fail(fault,InvalidFormulaAbi,s);return;}
    if(counts){++counts->calls;counts->recovered+=status==1;counts->stagnant+=status==2;counts->invalid+=status==3;}
}

inline bool native_advance(thread NativeFormulaState& s,constant NativeFormulaProgram& p,
    thread NativeFormulaFault& f,thread uint& slot,thread int& remaining,bool allow_disabled_exit,
    thread bool& disabled_exit) {
    uint visited=0;
    while(remaining<=0){
        if(visited&(1u<<slot))return native_fail(f,InvalidHybridCycle,s);
        visited|=1u<<slot;
        slot=slot>=p.end_slot?p.repeat_slot:slot+1;
        remaining=int(uint(s.hybrid_counts[slot])&0x7fffffffu);
        if(allow_disabled_exit && remaining<=0 && slot==p.start_slot){
            s.iteration=s.max_iterations;disabled_exit=true;return true;}
    }
    return true;
}
inline float native_analytic_result(thread const NativeFormulaState& s,
    constant NativeFormulaProgram& p,bool four,bool interpolated,uint slot) {
    if(!four && !interpolated && (p.de_option&0x38)==32)
        return sqrt(s.rout)*.5f*log(s.rout)/s.derivative[0];
    float derivative=four?s.derivative[0]:s.xyz[3];
    switch(p.de_option&7){
    case 4:{float v=abs(s.xyz[(four||interpolated)?2:1]);return v*log(v)/derivative;}
    case 7:return sqrt(s.rout/s.rstop)*pow(abs(p.slots[slot].negative_params[1]),-float(s.iteration));
    default:return sqrt(s.rout)/abs(derivative);
    }
}

inline void native_range_exit(thread const NativeResources& resources) {
    if(resources.range_exits)++*resources.range_exits;
}
// Orbit passes from s, stopping before pass pass_limit. A pass leaving the
// FP32 range returns at once with exit_pass set; s is then undefined.
template<uint Part=0> inline float native_orbit_passes(constant NativeFormulaProgram& p,thread NativeFormulaState& s,
    thread const NativeResources& resources,thread NativeFormulaFault& f,
    thread mb3d_native_gradient::RadiusRange* radius_range,uint pass_limit,thread uint& exit_pass,thread bool& inert) {
    const uint driver=native_part_driver<Part>();
    if(radius_range)*radius_range={};
    const bool difs=driver>=HybridDifs;
    const bool interpolated=driver>=Interpolated3D && driver<=Interpolated4DAnalytic;
    const bool four=driver==Hybrid4D || driver==Hybrid4DAnalytic || driver==Interpolated4D || driver==Interpolated4DAnalytic;
    const bool analytic=driver==Hybrid3DAnalytic || driver==Hybrid4DAnalytic || driver==Interpolated3DAnalytic || driver==Interpolated4DAnalytic;
    // The compiled-original unsmoothed DE2 epilogue loads Rout/W from memory
    // and overwrites both XMM registers regardless of active modifier counts.
    // Entry/callback transport still has the older positive-count proof only.
    // Keep SSE-consuming and inactive-negative profiles outside both proofs.
    bool analytic_return_transport=driver==Hybrid3DAnalytic && p.de_option==2 && !s.calc_smooth;
    bool analytic_transport=analytic_return_transport;
    if(analytic_return_transport)for(uint i=0;i<6;++i){
        const bool negative=s.hybrid_counts[i]<0;
        const bool unproved=(p.slots[i].enabled && (p.slots[i].traits&(1u<<3))) ||
            (negative && (!p.slots[i].enabled || !(uint(s.hybrid_counts[i])&0x7fffffffu) ||
                          i<min(p.start_slot,p.repeat_slot) || i>p.end_slot));
        if(negative || unproved)analytic_transport=false;
        if(unproved)analytic_return_transport=false;
    }
    // The separately compiled ordinary Pascal callers overwrite these final
    // registers. Their x87 smoothing helper preserves XMM0/XMM1. This proof
    // does not extend intermediate transport to SSE-consuming formula bodies.
    bool caller_return_transport=!difs;
    for(uint i=0;i<6;++i)
        if(p.slots[i].enabled && (p.slots[i].traits&(1u<<3)))caller_return_transport=false;
    const bool default_analytic_return=analytic && (p.de_option&7)!=4 &&
        (p.de_option&7)!=7 && !(driver==Hybrid3DAnalytic&&(p.de_option&0x38)==32);
    if(driver!=HybridDifsNoInitialization){
        for(uint i=0;i<3;++i)s.xyz[i]=s.c[i];
        // Hybrid3D analytic XYZ-derivative mode retains the incoming w lane.
        if(!(driver==Hybrid3DAnalytic&&(p.de_option&0x38)==32))s.xyz[3]=0.f;
        if(four)for(uint i=0;i<4;++i)
            s.xyz[i]=((s.c[0]*p.rotation[i][0]+s.c[1]*p.rotation[i][1])+s.c[2]*p.rotation[i][2]);
        for(uint i=0;i<3;++i)s.j[i]=s.julia?s.original_j[i]:(four?s.xyz[i]:s.c[i]);
        if(four)s.j[3]=s.julia?s.original_j[3]:s.xyz[3];
    }
    s.first=0;s.iteration=0;
    if(difs)s.vary_scale=1.f;
    s.rout=difs?0.f:interpolated?native_interpolation_radius(s.xyz[0],s.xyz[1],s.xyz[2],s.xyz[3],four):(driver==Hybrid3DAnalytic && p.de_option==2?
        native_de2_radius(float3(s.xyz[0],s.xyz[1],s.xyz[2])):native_radius(s,four));
    s.otrap=difs?65535.f:s.rout;
    if(analytic){
        if(four){s.derivative[0]=(p.de_option&0x38)==16?s.rout:1.f;
            if((p.de_option&0x38)==32){s.derivative[1]=0.f;s.derivative[2]=0.f;}}
        else if(!interpolated && (p.de_option&0x38)==32){s.derivative[0]=1.f;s.derivative[1]=0.f;s.derivative[2]=0.f;}
        else s.xyz[3]=(p.de_option&(interpolated?0x18:0x38))==16?s.rout:1.f;
    }
    if(driver==Hybrid3D){s.sse[0]=s.xyz[0]*s.xyz[0]+s.xyz[1]*s.xyz[1];s.sse[1]=0;s.sse[2]=s.rout;s.sse[3]=0;}
    if(analytic_transport){s.sse[0]=s.rout;s.sse[1]=0;s.sse[2]=s.xyz[0]*s.xyz[0]+s.xyz[1]*s.xyz[1];s.sse[3]=0;}
    if(caller_return_transport&&driver==Hybrid4D){
        // Original numeric4D entry leaves initial XYZ squared radius in XMM1.
        // A negative callback can bail out without ever updating that radius.
        // Reuse the outlined source-order sum, keeping generic radius CSE apart.
        s.sse[0]=s.rout;s.sse[1]=0;
        s.sse[2]=native_de2_radius(float3(s.xyz[0],s.xyz[1],s.xyz[2]));s.sse[3]=0;
    }
    uint slot=interpolated?0:p.start_slot;
    int remaining=int(uint(s.hybrid_counts[slot])&0x7fffffffu);
    // The entry minimum is carried from the previous call (HybridDifs) and the
    // result's trap is carried dfree2 until an iteration improves on 65535.
    int minimum_iteration=driver==HybridDifsNoInitialization?0:s.tmp;bool improved=false;
    if(driver==HybridDifsNoInitialization)s.tmp=0;
    if(difs)s.dfree1=0.f;else if(!interpolated)s.tmp=remaining;
    // dIFS: whether the last counted pass lowered the minimum.
    bool last_improved=false;
    uint pass=0;
    do {
        const uint index=pass++;bool overflow=false;
        if(index==pass_limit)break;
        if(interpolated){
#pragma clang fp contract(off)
            s.rold=s.rout;float saved[4],first[4];
            for(uint i=0;i<4;++i){saved[i]=s.xyz[i];s.sse[i]=s.xyz[i];}
            float old_derivative=s.derivative[0];
            if(four&&analytic){s.sse[0]=old_derivative;s.sse[1]=0;}
            if(!native_step(s,p,resources,f,0,overflow))return 0.f;
            if(overflow){exit_pass=index;return 0.f;}
            for(uint i=0;i<4;++i){first[i]=s.xyz[i];s.xyz[i]=saved[i];s.sse[i]=saved[i];}
            float first_derivative=s.derivative[0];
            if(four&&analytic){s.derivative[0]=old_derivative;s.sse[0]=old_derivative;s.sse[1]=0;}
            if(!native_step(s,p,resources,f,1,overflow))return 0.f;
            if(overflow){exit_pass=index;return 0.f;}
            float a=as_type<float>(s.hybrid_counts[0]),b=as_type<float>(s.hybrid_counts[1]);
            float first_r=(first[0]*first[0]+first[1]*first[1])+first[2]*first[2];
            if(four)first_r+=first[3]*first[3];
            float target=sqrt(first_r)*a+sqrt(native_interpolation_radius(s.xyz[0],s.xyz[1],s.xyz[2],s.xyz[3],four))*b;
            for(uint i=0;i<4;++i)s.xyz[i]=(i==3 && analytic && !four)?abs(first[i])*a+abs(s.xyz[i])*b:first[i]*a+s.xyz[i]*b;
            if(four&&analytic)s.derivative[0]=abs(first_derivative)*a+abs(s.derivative[0])*b;
            // Numeric caller return transport retains this partial sum before
            // normalization (XYZ in 3D, XY in 4D), not the final Rout.
            const float transport_radius=four?s.xyz[0]*s.xyz[0]+s.xyz[1]*s.xyz[1]:
                native_interpolation_radius(s.xyz[0],s.xyz[1],s.xyz[2],s.xyz[3],false);
            float factor=target/native_interpolation_denominator(
                native_interpolation_radius(s.xyz[0],s.xyz[1],s.xyz[2],s.xyz[3],four));
            for(uint i=0;i<(four?4u:3u);++i)s.xyz[i]*=factor;
            ++s.iteration;s.rout=target*target;
            if(!isfinite(s.rout)||!all(isfinite(float4(s.xyz[0],s.xyz[1],s.xyz[2],s.xyz[3]))))
                {exit_pass=index;return 0.f;}
            if(s.rout<s.otrap)s.otrap=s.rout;
            if(caller_return_transport&&!analytic){
                s.sse[0]=s.iteration>=s.max_iterations?s.rout:s.rstop;
                s.sse[1]=0;s.sse[2]=transport_radius;s.sse[3]=0;
            }
        } else {
            if(!difs)s.rold=s.rout;
            bool disabled_exit=false;
            if(!native_advance(s,p,f,slot,remaining,driver==Hybrid3D,disabled_exit))return 0.f;
            if(disabled_exit)return 0.f;
            if(!difs)s.tmp=remaining;
            if(!native_step(s,p,resources,f,slot,overflow))return 0.f;
            if(overflow){
                // A dIFS orbit that left binary32 after its minimum stopped
                // improving ends its search there: the minimum stands, as an
                // escaped orbit's would in binary64. A still-falling minimum, a
                // carried one or an x87 indefinite NaN remains a range exit.
                constant auto& spec=p.slots[slot];
                inert=difs&&improved&&!last_improved&&
                    !(spec.namespace_id==ExternalFormula&&native_formula_x87_indefinite(spec.id));
                exit_pass=index;return 0.f;
            }
            --remaining;if(!difs)s.tmp=remaining;
            if(s.hybrid_counts[slot]<0){
                // doHybridPas tests maxIt before loading RStop into XMM0.
                // A skipped count retains the formula's other SSE lanes.
                if((driver==Hybrid3D||(caller_return_transport&&driver==Hybrid4D))&&
                   s.iteration<s.max_iterations)s.sse[0]=s.rstop;
                continue;
            }
            ++s.iteration;
            if(difs){
                float de=s.rout/s.vary_scale;last_improved=!(de>=s.otrap);
                if(!(de>=s.otrap)){
                    minimum_iteration=s.iteration;s.tmp=minimum_iteration;s.dfree2=s.dfree1;s.otrap=de;improved=true;
                    bool outside=driver==HybridDifs?(as_type<int>(s.inside)>=0):(s.inside==0);
                    if(outside && !(de>=s.rstop_d))break;
                }
            } else {
                s.rout=driver==Hybrid3DAnalytic && p.de_option==2?
                    native_de2_radius(float3(s.xyz[0],s.xyz[1],s.xyz[2])):native_radius(s,four);
                if(radius_range)*radius_range=mb3d_native_gradient::radius_range(
                    float4(s.xyz[0],s.xyz[1],s.xyz[2],s.xyz[3]),four,s.rout);
                // A certified +inf radius of finite coordinates stays the
                // gradient sample's minimum-clamp evidence (an ordinary escape).
                if(!isfinite(s.rout)&&!(radius_range&&radius_range->certified)){exit_pass=index;return 0.f;}
                if(s.rout<s.otrap)s.otrap=s.rout;
                if(driver==Hybrid3D){
                    // The original FPC/SSE2 caller reloads Rout for OTrap,
                    // then short-circuits maxIt before converting RStop.
                    s.sse[0]=s.iteration>=s.max_iterations?s.rout:s.rstop;
                    s.sse[1]=0;s.sse[2]=s.rout;s.sse[3]=0;
                }
                if(caller_return_transport&&driver==Hybrid4D){
                    s.sse[0]=s.iteration>=s.max_iterations?s.rout:s.rstop;
                    s.sse[1]=0;s.sse[2]=native_radius(s,false);s.sse[3]=0;
                }
                if(analytic_transport){
                    s.sse[0]=s.iteration>=s.max_iterations?s.rout:s.rstop;
                    s.sse[1]=0;s.sse[2]=s.xyz[0]*s.xyz[0]+s.xyz[1]*s.xyz[1];s.sse[3]=0;
                }
            }
        }
    }while(s.iteration<s.max_iterations && (difs || !(s.rout>s.rstop)));
    if(difs){if(!improved&&resources.carried_reads)++*resources.carried_reads;
        float distance=s.otrap;s.smooth=float(minimum_iteration);
        s.iteration=minimum_iteration;s.tmp=minimum_iteration;s.otrap=s.dfree2;return distance;}
    float result=analytic?native_analytic_result(s,p,four,interpolated,interpolated?1:slot):0.f;
    if(analytic_return_transport){s.sse[0]=result;s.sse[1]=0;s.sse[2]=abs(s.xyz[3]);s.sse[3]=0;}
    if(caller_return_transport&&default_analytic_return){
        s.sse[0]=result;s.sse[1]=0;
        s.sse[2]=abs(four?s.derivative[0]:s.xyz[3]);s.sse[3]=0;
    }
    if(s.calc_smooth)native_smooth(s,p,interpolated?0:slot,resources,f);
    return result;
}
// FP32 range exit: the orbit escapes from its state before the exiting pass.
// Only that rare path keeps a cost: the orbit reruns deterministically from
// its entry state, without the exiting pass. No per-iteration snapshot. Work
// counts every executed step, including the exiting run.
template<uint Part=0> inline float native_run_orbit(constant NativeFormulaProgram& p,thread NativeFormulaState& s,
    thread const NativeResources& resources,thread NativeFormulaFault& f,
    thread mb3d_native_gradient::RadiusRange* radius_range=nullptr,uint first_limit=0xffffffffu) {
    const NativeFormulaState entry=s;
    uint pass_limit=first_limit;
    for(;;){
        uint exit_pass=0xffffffffu;bool inert=false;
        const float result=native_orbit_passes<Part>(p,s,resources,f,radius_range,pass_limit,exit_pass,inert);
        if(exit_pass==0xffffffffu||f.code)return result;
        s=entry;pass_limit=exit_pass;if(!inert)native_range_exit(resources);
    }
}

inline NativeFormulaState native_initial_state(float3 point,float stop,constant NativeFormulaProgram& p) {
    NativeFormulaState s{};s.c[0]=point.x;s.c[1]=point.y;s.c[2]=point.z;
    s.max_iterations=p.max_iterations;s.rstop=p.radius_stop;s.rstop_d=stop*p.stop_scale;s.rold=s.rstop_d;
    s.julia=p.julia;s.inside=p.inside;s.calc_smooth=p.calc_smooth;
    // IniIt3D initializes J even for non-Julia and dIFS drivers, which may
    // retain J4. Deriv1 starts at one independently of the first driver.
    for(uint i=0;i<4;++i)s.j[i]=s.original_j[i]=p.julia_value[i];
    s.derivative[0]=1.f;
    for(uint i=0;i<6;++i)s.hybrid_counts[i]=p.hybrid_counts[i];
    return s;
}

// Which CalcDE inside transform a part applies: the ray's own (FormulaType 0
// and the combined result), none (the first part of a combination, transformed
// after combining), or forced (the inverted second part of FormulaType 3).
enum NativeInsideMode : uint { InsideFromLane=0, InsideDeferred=1, InsideForced=2 };
// Stateful entry preserves the caller's logical lane (conditional counts and
// dIFS retained color). The convenience overload below starts a fresh lane.
template<uint Part=0> inline NativeOrbitResult native_distance_controlled(float3 point,float stop,constant NativeFormulaProgram& p,
    thread NativeResources& resources,thread NativeFormulaState& lane,uint calc_smooth,int max_iterations,
    thread uint* range_clamps=nullptr,uint inside_mode=InsideFromLane) {
    const uint driver=native_part_driver<Part>();
    NativeOrbitResult out{};out.state=lane;thread auto& s=out.state;thread auto& fault=out.fault;fault.limit=p.work_limit;
    if(!native_program_valid(p,driver) || !all(isfinite(point)) || !isfinite(stop) || stop<=0.f ||
        calc_smooth>1 || max_iterations<1 || max_iterations>p.max_iterations+1){native_fail(fault,InvalidFormulaAbi,s);return out;}
    s.c[0]=point.x;s.c[1]=point.y;s.c[2]=point.z;
    s.max_iterations=max_iterations;s.rstop=resources.radius_stop>0.f?resources.radius_stop:p.radius_stop;s.calc_smooth=calc_smooth;
    s.rstop_d=stop*p.stop_scale;if(driver>=HybridDifs)s.rold=s.rstop_d;
    float value=native_run_orbit<Part>(p,s,resources,fault,nullptr);
    if(fault.code)return out;
    bool numerical=driver==Hybrid3D||driver==Hybrid4D||driver==Interpolated3D||driver==Interpolated4D;
    bool bounded_minimum=false;
    if(numerical){
        float radius=s.rout;int iterations=s.iteration;uint smooth=s.calc_smooth;
        if((s.inside && iterations==s.max_iterations) || radius<=0.f)value=0.f;
        else {
            s.calc_smooth=0;s.rstop=p.gradient_radius_stop;
            // FP32 range retry: a gradient not representable at the escape
            // iteration is evaluated once at the previous one, rerunning the
            // center like a sample. Color/smoothing keep the original orbit.
            float center=radius;int steps=iterations;uint limit=0xffffffffu;
            for(bool retried=false;;retried=true){
                s.max_iterations=steps;bounded_minimum=false;
                float3 delta;bool representable=true;
                for(uint axis=0;axis<3&&representable;++axis){float original=s.c[axis];
                    const auto sample=mb3d_native_gradient::sample(original,p.de_offset);
                    if(!sample.valid){native_fail(fault,InvalidFormulaAbi,s);return out;}
                    s.c[axis]=sample.coordinate;
                    mb3d_native_gradient::RadiusRange range;
                    native_run_orbit<Part>(p,s,resources,fault,&range,limit);s.c[axis]=original;if(fault.code)return out;
                    if((driver==Hybrid3D||driver==Hybrid4D)&&s.rout>s.rstop&&
                       mb3d_native_gradient::minimum_clamp_proven(range,center,sample.scale,
                           center*log(center)*p.de_scale,p.de_offset_floor,stop)) {
                        bounded_minimum=true;delta[axis]=0.f;continue;
                    }
                    const float difference=center-s.rout;
                    delta[axis]=mb3d_native_gradient::normalize(difference,sample);
                    representable=isfinite(delta[axis]);
                    if(!representable&&(driver==Hybrid3D||driver==Hybrid4D)&&
                       mb3d_native_gradient::difference_minimum_clamp_proven(difference,center,sample.scale,
                           center*log(center)*p.de_scale,p.de_offset_floor,stop)) {
                        bounded_minimum=true;delta[axis]=0.f;representable=true;
                    }
                }
                if(representable){
                    if(bounded_minimum){value=stop*.25f;break;}
                    value=mb3d_native_gradient::distance(center,p.de_scale,delta,p.de_offset_floor);
                    if(isfinite(value))break;
                }
                // At the first iteration, escaped there or reached by the retry, no
                // earlier one exists: the orbit escapes before its first pass
                // (iteration 0), as a range exit escapes an orbit. Iteration 0 is
                // below iMinIt, so the march never takes the point as a hit; the
                // gradient of its entry state sizes the step.
                if(steps<=1&&limit!=0u){
                    native_range_exit(resources);limit=0;iterations=0;
                    native_run_orbit<Part>(p,s,resources,fault,nullptr,0);if(fault.code)return out;
                    center=radius=s.rout;
                    if(!isfinite(center)||center<=0.f){native_fail(fault,NonFiniteFormula,s);return out;}
                    continue;
                }
                if(retried||steps<=1){native_fail(fault,NonFiniteFormula,s);return out;}
                native_range_exit(resources);s.max_iterations=--steps;
                native_run_orbit<Part>(p,s,resources,fault,nullptr);if(fault.code)return out;
                center=s.rout;
                if(!isfinite(center)||center<=0.f){native_fail(fault,NonFiniteFormula,s);return out;}
            }
            s.rout=radius;s.iteration=iterations;s.max_iterations=max_iterations;s.calc_smooth=smooth;s.rstop=p.radius_stop;
        }
    } else value*=p.de_scale;
    if(driver<HybridDifs && value<stop*.25f)value=stop*.25f;
    // Preserve the raw LongBool selected for this ray, including -1 on retry.
    const int calc_inside=inside_mode==InsideFromLane?as_type<int>(s.inside):inside_mode==InsideForced?-1:0;
    const auto interior=mb3d_inside::transform_de(value,stop,s.iteration,driver,p.de_option,calc_inside);
    if(interior.fault){native_fail(fault,interior.fault==mb3d_inside::NonFinite?NonFiniteFormula:InvalidFormulaAbi,s);return out;}
    value=interior.value;s.iteration=interior.iteration;
    if(!isfinite(value) || !isfinite(s.rout) || !isfinite(s.otrap) || (s.calc_smooth&&!isfinite(s.smooth))){
        native_fail(fault,NonFiniteFormula,s);return out;}
    out.distance=value;out.completed=1;out.escaped=uint(s.rout>s.rstop);lane=s;
    if(range_clamps&&bounded_minimum)++*range_clamps;
    return out;
}
// CPU MaxItsResult of one part: its maximum, plus one for dIFS.
inline uint native_part_maximum(constant NativeFormulaProgram& p,int max_iterations) {
    return uint(max_iterations)+(p.de_option==20?1u:0u);
}
// DE combination (Calc.pas CalcDEfull, FormulaType>0). The program buffer
// holds the second hybrid part after the first. The first part keeps its DE
// minimum clamp; its inside transform is deferred. The second part continues
// the same lane like CalcDEnoADE/CalcDEanalytic, with its own driver, DE
// scale, DE option and maximum, and inside disabled (FormulaType 3: forced).
// The combined DE then takes the ray's inside transform under the winning
// part's DE option. FormulaType 0 is the single-part DE unchanged.
inline NativeOrbitResult native_distance_combined(float3 point,float stop,constant NativeFormulaProgram& p,
    thread NativeResources& resources,thread NativeFormulaState& lane,uint calc_smooth,int max_iterations,
    bool temporary_maximum,thread uint* range_clamps=nullptr) {
    const uint type=native_selected_formula_type();
    if(!type)return native_distance_controlled(point,stop,p,resources,lane,calc_smooth,max_iterations,range_clamps);
    constant NativeFormulaProgram& q=(&p)[1];
    NativeFormulaState state=lane;
    auto first=native_distance_controlled<0>(point,stop,p,resources,state,calc_smooth,max_iterations,range_clamps,
        InsideDeferred);
    if(first.fault.code||!first.completed)return first;
    // RMdoBinSearchIt raises each non-dIFS part's maximum by one.
    const int second_maximum=q.max_iterations+(temporary_maximum&&q.de_option!=20?1:0);
    NativeOrbitResult out=native_distance_controlled<1>(point,stop,q,resources,state,calc_smooth,second_maximum,
        range_clamps,type==CombineMaximumInverse?InsideForced:InsideDeferred);
    out.fault.work+=first.fault.work;
    if(out.fault.code||!out.completed)return out;
    thread auto& s=out.state;const thread auto& a=first.state;
    float value=first.distance;const float other=out.distance;
    uint maximum=native_part_maximum(q,second_maximum);int de_option=q.de_option;
    const bool restore=type<=CombineMaximumInverse?!(type==CombineMinimum?other<value:other>value):
        type<=CombineQuadraticBlend&&other>=value;
    if(restore){
        s.iteration=a.iteration;maximum=native_part_maximum(p,max_iterations);de_option=p.de_option;
        if(type<=CombineMaximumInverse){s.smooth=a.smooth;s.otrap=a.otrap;}
    }
    const float k=p.combination_smooth;
    if(type<=CombineMaximumInverse){if(!restore)value=other;}
    else if(type<=CombineQuadraticBlend){
        if(type==CombineLinearBlend)value=min(other-max(0.f,k-value),value-max(0.f,k-other));
        else {const float first_cut=max(0.f,k-value-stop),second_cut=max(0.f,k-other-stop);
            value=min(other-first_cut*(k-second_cut)/k,value-second_cut*(k-first_cut)/k);}
        const float weight=abs(value),other_weight=abs(other),inverse=1.f/(weight+other_weight+1e-10f);
        s.smooth=(s.smooth*weight+a.smooth*other_weight)*inverse;
        s.otrap=(s.otrap*weight+a.otrap*other_weight)*inverse;
    } else {
        if(p.mix_color==0){s.smooth=s.smooth+a.smooth;s.otrap=(s.otrap+a.otrap)*.5f;}
        else if(p.mix_color==1){s.smooth=a.smooth;s.otrap=a.otrap;}
        // CalcDEanalytic leaves Deriv1 and w of the first part's orbit.
        if(p.de_option==5||p.de_option==6)value=other/abs(a.derivative[0]);
        else if(p.de_option==2||p.de_option==11)value=other/abs(a.xyz[3]);
        else value=other*pow(abs(p.mix_power),-1.f-float(a.iteration));
    }
    // RestoreF1DEcomb; the transform decrements unconditionally like CalcDEnoADE.
    s.max_iterations=a.max_iterations;
    const auto interior=mb3d_inside::transform_de(value,stop,s.iteration,Hybrid3D,de_option,as_type<int>(s.inside));
    if(interior.fault){native_fail(out.fault,interior.fault==mb3d_inside::NonFinite?NonFiniteFormula:InvalidFormulaAbi,s);
        out.completed=0;return out;}
    value=interior.value;s.iteration=interior.iteration;
    if(!isfinite(value) || !isfinite(s.rout) || !isfinite(s.otrap) || (s.calc_smooth&&!isfinite(s.smooth))){
        native_fail(out.fault,NonFiniteFormula,s);out.completed=0;return out;}
    out.distance=value;out.maximum=maximum;out.escaped=uint(s.rout>s.rstop);lane=s;
    return out;
}
// Iteration maximum a DE result is compared against (CPU MaxItsResult).
inline uint native_iteration_maximum(constant NativeFormulaProgram& p,thread const NativeOrbitResult& r) {
    return native_selected_formula_type()?r.maximum:uint(p.max_iterations)+(p.de_option==20?1u:0u);
}
// Shadow descriptors carry the FormulaType 0 maximum as a validated constant.
inline uint native_shadow_maximum(constant NativeFormulaProgram&,thread const NativeOrbitResult& r,uint fixed) {
    return native_selected_formula_type()?r.maximum:fixed;
}
inline NativeOrbitResult native_distance(float3 point,float stop,constant NativeFormulaProgram& p,
    thread NativeResources& resources,thread NativeFormulaState& lane) {
    return native_distance_controlled(point,stop,p,resources,lane,p.calc_smooth,p.max_iterations);
}
inline NativeOrbitResult native_distance(float3 point,float stop,constant NativeFormulaProgram& p,
    thread NativeResources& resources) {
    auto lane=native_initial_state(point,stop,p);return native_distance(point,stop,p,resources,lane);
}
#ifdef MB3D_NATIVE_FORMULA_PROBE_KERNELS
[[host_name("native_gradient_difference_certificate")]] kernel void native_gradient_difference_certificate(
    device const float* inputs [[buffer(0)]],device uint2* outputs [[buffer(1)]],
    constant uint& count [[buffer(2)]],uint tid [[thread_position_in_grid]]) {
    if(tid>=count)return;
    const uint base=tid*6;
    outputs[tid]=uint2(uint(mb3d_native_gradient::difference_minimum_clamp_proven(
        inputs[base],inputs[base+1],inputs[base+2],inputs[base+3],inputs[base+4],inputs[base+5])),
        as_type<uint>(inputs[base]*inputs[base+2]));
}
[[host_name("native_formula_abi")]] kernel void native_formula_abi(device uint* result [[buffer(0)]],uint tid [[thread_position_in_grid]]) {
    if(tid)return;
    result[0]=kFormulaAbiVersion;result[1]=sizeof(NativeFormulaProgram);
    result[2]=sizeof(NativeFormulaState);result[3]=sizeof(NativeOrbitResult);
    result[4]=32; // hardware arithmetic width; never an exact/compatibility claim
}
[[host_name("native_formula_probe")]] kernel void native_formula_probe(constant NativeFormulaProgram& program [[buffer(0)]],
    device const NativeFormulaState* inputs [[buffer(1)]],
    device NativeOrbitResult* outputs [[buffer(2)]],
    device const NativeMapDescriptor* maps [[buffer(3)]],device const float* pixels [[buffer(4)]],
    constant uint4& launch [[buffer(5)]],constant uint2& resource_counts [[buffer(6)]],
    uint tid [[thread_position_in_grid]]) {
    // Allocation limit is independent from requested work. An over-dispatched
    // lane must not read inputs or touch the guard output.
    if(tid>=launch.x || tid>=launch.w)return;
    NativeOrbitResult result{};result.state=inputs[tid];result.fault.limit=program.work_limit;
    NativeResources resources{maps,pixels,resource_counts.x,resource_counts.y,nullptr};
    if(!native_program_valid(program) || launch.y>2 || launch.z>=6) {
        native_fail(result.fault,InvalidFormulaAbi,result.state);
    } else if(launch.y==0) {
        result.completed=uint(native_step(result.state,program,resources,result.fault,launch.z));
    } else if(launch.y==1) {
        result.distance=native_run_orbit(program,result.state,resources,result.fault);
        result.completed=uint(!result.fault.code && isfinite(result.distance));
        if(!result.completed && !result.fault.code)native_fail(result.fault,NonFiniteFormula,result.state);
        result.escaped=uint(result.state.rout>result.state.rstop);
    } else {
        float3 point(result.state.c[0],result.state.c[1],result.state.c[2]);
        float stop=result.state.rstop_d/program.stop_scale;
        result=native_distance(point,stop,program,resources,result.state);
    }
    outputs[tid]=result;
}
// Diagnostic side channel only. Keep the ordinary kernel above unchanged;
// the host requires every 59-word result to match it before trusting counts.
[[host_name("native_formula_range_probe")]] kernel void native_formula_range_probe(constant NativeFormulaProgram& program [[buffer(0)]],
    device const NativeFormulaState* inputs [[buffer(1)]],
    device NativeOrbitResult* outputs [[buffer(2)]],
    device const NativeMapDescriptor* maps [[buffer(3)]],device const float* pixels [[buffer(4)]],
    constant uint4& launch [[buffer(5)]],constant uint2& resource_counts [[buffer(6)]],
    device uint* range_outputs [[buffer(7)]],uint tid [[thread_position_in_grid]]) {
    // Allocation limit is independent from requested work. An over-dispatched
    // lane must not read inputs or touch the guard output.
    if(tid>=launch.x || tid>=launch.w)return;
    NativeOrbitResult result{};result.state=inputs[tid];result.fault.limit=program.work_limit;
    NativeResources resources{maps,pixels,resource_counts.x,resource_counts.y,nullptr};
    uint range_count=0;resources.range_exits=&range_count;
    if(!native_program_valid(program) || launch.y>2 || launch.z>=6) {
        native_fail(result.fault,InvalidFormulaAbi,result.state);
    } else if(launch.y==0) {
        result.completed=uint(native_step(result.state,program,resources,result.fault,launch.z));
    } else if(launch.y==1) {
        result.distance=native_run_orbit(program,result.state,resources,result.fault);
        result.completed=uint(!result.fault.code && isfinite(result.distance));
        if(!result.completed && !result.fault.code)native_fail(result.fault,NonFiniteFormula,result.state);
        result.escaped=uint(result.state.rout>result.state.rstop);
    } else {
        float3 point(result.state.c[0],result.state.c[1],result.state.c[2]);
        float stop=result.state.rstop_d/program.stop_scale;
        result=native_distance(point,stop,program,resources,result.state);
    }
    outputs[tid]=result;
    range_outputs[tid]=range_count;
}
#endif
} // namespace mb3d_native
