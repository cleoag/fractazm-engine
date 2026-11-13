#pragma once
#ifndef MB3D_METAL_NATIVE_FLOAT
#error Generic formula geometry requires the hardware FP32 profile
#endif
#include "general_geometry_abi.h"
#include "surface_color.metal.h"
#include "logical_lane_abi.h"
#include "camera_geometry_bridge.metal.h"
#include "formula_wrapper.metal.h"
#include "orbit_driver.metal.h"
#include "comparison/metal_exact/scene_types.h"
#include "comparison/metal_exact/shadow_abi.h"
#include "volume_light_abi.h"
#include "native_inside_math.h"

namespace mb3d_native {
using namespace metal;
using mb3d_metal_exact::D;
using mb3d_metal_exact::F64;
inline float general_double(ulong word) {return D(word).value;}
inline ulong general_word(float value) {return F64::from_native(value).to_bits();}
inline float3 general_column(constant ExactScene& s,uint i) {
    return float3(general_double(s.vgrads[i*3]),general_double(s.vgrads[i*3+1]),general_double(s.vgrads[i*3+2]));
}
inline bool valid_general_scene(constant ExactScene& s,constant NativeFormulaProgram& program) {
    if(s.version!=4 || s.formula_kind!=kGeneralFormulaKind || s.capability_id!=kGeneralCapability ||
        (s.fog_iteration>255 && s.fog_iteration!=kNativeVolumeFog) || s.color_on_it>s.max_iterations+1u ||
        (s.color_on_it?!(as_type<float>(s.color_radius_stop)>0.f&&isfinite(as_type<float>(s.color_radius_stop))):s.color_radius_stop!=0u) || !s.width || !s.height ||
        s.width>4096 || s.height>4096 || !s.max_steps || s.max_steps>65536 ||
        s.max_iterations!=uint(program.max_iterations) || s.slot_count!=6 ||
        !s.min_iterations || s.min_iterations>s.max_iterations || !valid_general_primary_limits(s.refinement_steps,s.max_iterations) ||
        !native_program_valid(program))return false;
    for(uint i=0;i<9;++i)if(!isfinite(general_double(s.vgrads[i])))return false;
    for(uint i=0;i<3;++i)if(!isfinite(general_double(s.y_start[i])))return false;
    return isfinite(general_double(s.max_ray_length))&&general_double(s.max_ray_length)>0.f&&
        isfinite(as_type<float>(s.de_stop))&&as_type<float>(s.de_stop)>0.f&&
        isfinite(as_type<float>(s.de_stop_factor))&&as_type<float>(s.de_stop_factor)>=0.f&&
        isfinite(as_type<float>(s.z_step_div))&&as_type<float>(s.z_step_div)>0.f&&
        isfinite(as_type<float>(s.step_limiter))&&as_type<float>(s.step_limiter)>0.f&&
        isfinite(as_type<float>(s.de_scale))&&as_type<float>(s.de_scale)>0.f&&
        isfinite(as_type<float>(s.step_width_103))&&as_type<float>(s.step_width_103)>0.f&&
        isfinite(as_type<float>(s.de_sub));
}
// temporary_maximum marks RMdoBinSearchIt's raised maximum, which a DE
// combination applies to its second part as well.
inline NativeOrbitResult general_de_inline(float3 point,float stop,constant NativeFormulaProgram& program,
    thread NativeResources& resources,thread NativeFormulaState& lane,thread NativeGeometryRecord& record,
    bool smooth=false,int max_iterations=0,bool temporary_maximum=false) {
    NativeOrbitResult result{};
    if(record.fault)return result;
    if(record.de_calls>=kGeneralMaxDistanceCalls || record.formula_work>=kGeneralMaxFormulaWork) {
        record.fault=2;record.formula_fault.code=FormulaWorkExhausted;return result;
    }
    ++record.de_calls;
    thread NativeSmoothingCounters* previous_smoothing=resources.smoothing;resources.smoothing=&record.smoothing;
    thread uint* previous_exits=resources.range_exits;resources.range_exits=&record.fp32_range_exits;
    thread uint* previous_carried=resources.carried_reads;resources.carried_reads=&record.difs_carried_reads;
    uint range_clamps=0;
    result=native_distance_combined(point,stop,program,resources,lane,uint(smooth),
        max_iterations?max_iterations:program.max_iterations,temporary_maximum,&range_clamps);
    resources.smoothing=previous_smoothing;resources.range_exits=previous_exits;resources.carried_reads=previous_carried;
    record.formula_work+=result.fault.work;
    if(result.fault.code || !result.completed) {
        record.formula_fault=result.fault;
        record.fault=result.fault.code==FormulaWorkExhausted?2:result.fault.code==NonFiniteFormula?4:8;
    } else if(record.formula_work>kGeneralMaxFormulaWork) {
        record.fault=2;record.formula_fault=result.fault;record.formula_fault.code=FormulaWorkExhausted;
    // A combined DE may certify both parts. This counter measures successful
    // distance queries, matching de_calls, rather than individual parts.
    } else record.gradient_range_clamps+=uint(range_clamps!=0);
    return result;
}
// Keep the large height-map orbit out of each march/refinement/normal call
// site. This is an offline-compiled device call, not a different evaluator.
// Bind the exception to the two qualified dIFS chains; all other selectors
// retain their previous inlining and arithmetic path.
__attribute__((noinline)) NativeOrbitResult general_de_outlined(float3 point,float stop,
    constant NativeFormulaProgram& program,thread NativeResources& resources,
    thread NativeFormulaState& lane,thread NativeGeometryRecord& record,
    bool smooth,int max_iterations,bool temporary_maximum) {
    return general_de_inline(point,stop,program,resources,lane,record,smooth,max_iterations,temporary_maximum);
}
// Host policy (function constant 21): 1 outlines every formula call site of
// the specialized pipeline, for programs whose inlined copies exceed the
// backend compiler; 0 or undefined keeps the selector-specific choice below.
constant uint native_outline_de_policy [[function_constant(21)]];
inline bool native_outline_de() {
    return is_function_constant_defined(native_outline_de_policy)&&native_outline_de_policy==1u;
}
inline NativeOrbitResult general_de(float3 point,float stop,constant NativeFormulaProgram& program,
    thread NativeResources& resources,thread NativeFormulaState& lane,thread NativeGeometryRecord& record,
    bool smooth=false,int max_iterations=0,bool temporary_maximum=false) {
    if(native_outline_de())
        return general_de_outlined(point,stop,program,resources,lane,record,smooth,max_iterations,temporary_maximum);
    if(native_selected_driver()==HybridDifs && native_formula_id0==10189u &&
       native_formula_id1==5521u && native_formula_id2==10244u && native_formula_id3==10244u &&
       (native_formula_id4==5622u || native_formula_id4==5521u) && native_formula_id5==0xffffffffu)
        return general_de_outlined(point,stop,program,resources,lane,record,smooth,max_iterations,temporary_maximum);
    return general_de_inline(point,stop,program,resources,lane,record,smooth,max_iterations,temporary_maximum);
}
// RMdoColorOnIt (ColorOnIt>1): one more CalcDE at the hit, after normals and
// SIgradient, with maxIt=ColorOnIt-1 and RStop=Single(RStop*RStop*64); both are
// restored afterwards. RMdoColor then reads this orbit's Rout, Rold and OTrap.
inline NativeOrbitResult general_color_on_it(float3 point,float stop,constant ExactScene& scene,
    constant NativeFormulaProgram& program,thread NativeResources& resources,
    thread NativeFormulaState& lane,thread NativeGeometryRecord& record) {
    resources.radius_stop=as_type<float>(scene.color_radius_stop);
    const auto result=general_de(point,stop,program,resources,lane,record,false,int(scene.color_on_it));
    resources.radius_stop=0.f;return result;
}
// The CPU MaxItsResult a primary step compares against: the last DE's for a
// DE combination, else the program's own (dead combined state is removed).
inline uint general_maximum(uint combined,uint program_maximum) {
    return native_selected_formula_type()?combined:program_maximum;
}
inline float general_boundary_step(float target,float last,float smooth,float multiplier) {
    const float difference=last-smooth;float step;
    if(abs(difference)<1.e-30f)step=(last<smooth?1.f:0.f)-.5f;
    else step=(target-smooth)/(difference*(difference<0.f?500.f:1000.f));
    if(step>4.f)step=sqrt(step)*2.f;else if(step< -9.f)step=sqrt(-step)*-3.f;
    return step*multiplier+.0005f;
}
inline bool general_refine_iteration_count(thread float3& point,float3 ray,thread float& depth,float stop,
    constant ExactScene& scene,constant NativeFormulaProgram& program,thread NativeResources& resources,
    thread NativeFormulaState& lane,thread NativeGeometryRecord& record,thread uint& iteration,uint refinement_steps) {
    if(refinement_steps>kGeneralMaxRefinementSteps){record.fault=1;return false;}
    const float target=float(program.max_iterations)-.99f;
    const int temporary_max=program.max_iterations+(program.de_option!=20?1:0);
    float step=0.f,multiplier=1.f,last=0.f,last_difference=0.f,smooth=0.f;
    bool first=true;
    for(uint pass=0;pass<=refinement_steps;++pass) {
        ++record.refine_calls;depth+=step;point+=ray*step;
        auto value=general_de(point,stop,program,resources,lane,record,true,temporary_max,true);
        if(record.fault)return false;smooth=value.state.smooth;iteration=uint(value.state.iteration);
        if(!first && last_difference<abs(target-smooth)) {
            depth-=step;point-=ray*step;smooth=last;
            multiplier*=step>0.f?.5f:.7f;
        }
        last_difference=abs(target-smooth);last=smooth;
        if(smooth>float(temporary_max)-.1f)step=-3.f;
        else {
            depth-=.001f;point+=ray*-.001f;++record.refine_calls;
            value=general_de(point,stop,program,resources,lane,record,true,temporary_max,true);
            if(record.fault)return false;smooth=value.state.smooth;iteration=uint(value.state.iteration);
            step=general_boundary_step(target,last,smooth,multiplier);
        }
        if(!isfinite(step)||!isfinite(depth)||!all(isfinite(point))){record.fault=4;return false;}
        first=false;
    }
    return true;
}
inline bool general_refine_iteration(thread float3& point,float3 ray,thread float& depth,float stop,
    constant ExactScene& scene,constant NativeFormulaProgram& program,thread NativeResources& resources,
    thread NativeFormulaState& lane,thread NativeGeometryRecord& record,thread uint& iteration) {
    return general_refine_iteration_count(point,ray,depth,stop,scene,program,resources,lane,record,
        iteration,scene.refinement_steps);
}
} // namespace mb3d_native
#include "volume_light.metal.h"
#include "general_normals.metal.h"
namespace mb3d_native {
// One pixel's share of the primary first-step jitter (general_geometry_abi.h).
// RayMarch draws the first step's StepCount addend from a fresh per-ray LCG
// of the task's thread, then scales that step by the next carried task seed.
struct PrimaryRandomState { uint seed; float step_add; bool consumed; };
inline PrimaryRandomState primary_random_state(uint seed,uint task) {
    const uint draw=primary_random_advance(primary_random_task_seed(task));
    return {seed,float(draw>>8)*(1.f/16777216.f),false};
}
// Host policy (function constant 22): 1 compiles the jitter into the primary
// pipelines; 0 or undefined leaves them exactly as without it, so scenes
// without the jitter keep their code (and the backend compiler its limits).
constant uint native_primary_random_policy [[function_constant(22)]];
inline bool native_primary_random() {
    return is_function_constant_defined(native_primary_random_policy)&&native_primary_random_policy==1u;
}
inline ExactPixel general_primary_camera_attempt(constant ExactScene& scene,constant NativeFormulaProgram& program,
    mb3d_camera::Ray camera,uint x,uint y,thread NativeResources& resources,thread NativeGeometryRecord& record,
    thread NativeFormulaState& lane,NativeNormalParams normal_params,
    thread NativeNormalRecord& normal_record,thread NativeNormalResult& normal_result,int inside_word,
    thread PrimaryRandomState* random=nullptr) {
    normal_record={};normal_result={};record.initial_rout=0.f;record.hit_color={};
    normal_params.inside=uint(inside_word!=0);
    normal_record.version=kGeneralNormalVersion;
    ExactPixel out{};record.version=kGeneralGeometryVersion;
    if(!valid_general_scene(scene,program) || !valid_general_normals(normal_params,program) || x>=scene.width || y>=scene.height ||
        !mb3d_camera::valid_ray(camera)) {
        record.fault=out.fault=1;return out;
    }
    const float3 ray(camera.direction[0],camera.direction[1],camera.direction[2]);
    const float3 start(camera.origin[0],camera.origin[1],camera.origin[2]);
    for(uint i=0;i<3;++i){out.ray[i]=general_word(ray[i]);out.start[i]=general_word(start[i]);}
    if(!all(isfinite(ray))||!all(isfinite(start))){record.fault=out.fault=4;return out;}
    const float radius=(ray.x*ray.x+ray.y*ray.y)+ray.z*ray.z;
    const float axial_multiplier=radius/sqrt(radius*radius);
    const float base_stop=as_type<float>(scene.de_stop),stop_factor=as_type<float>(scene.de_stop_factor);
    const float z_div=as_type<float>(scene.z_step_div),limiter=as_type<float>(scene.step_limiter);
    float depth=mb3d_camera::initial_depth(camera,general_double(scene.max_ray_length));
    float stop=mb3d_camera::initial_stop(camera,base_stop,stop_factor,axial_multiplier);
    float3 point=mb3d_camera::initial_point(camera),active=start;
    float count=0.f,slope=1.f,final_de=0.f,axial=0.f;
    if(!isfinite(depth)||!isfinite(stop)||stop<=0.f||!all(isfinite(point))){record.fault=out.fault=4;return out;}
    lane.inside=as_type<uint>(inside_word); // Raw per-attempt LongBool; normal helper owns orientation.
    NativeOrbitResult result{};
    if(!camera.cut_miss)result=general_de(point,stop,program,resources,lane,record);
    float de=camera.cut_miss?base_stop+5.f:result.distance;
    uint iteration=camera.cut_miss?0u:uint(result.state.iteration);
    // CPU MaxItsResult follows each DE; only a DE combination changes it.
    // FormulaType 0 keeps the program's constant: a loop-carried maximum
    // there trips an AGX internal compiler error (BulboxCut).
    const uint program_maximum=uint(program.max_iterations)+(program.de_option==20?1u:0u);
    uint combined_maximum=camera.cut_miss?program_maximum:result.maximum;
    if(record.fault){out.fault=record.fault;return out;}
    if(!isfinite(axial_multiplier)){record.fault=out.fault=4;return out;}
    // DFogOnIt 65535: volumetric light sub-steps replace the fog step count.
    const bool volume=scene.fog_iteration==kNativeVolumeFog;
    if(volume&&(!resources.volume||!resources.volume_map||!valid_volume_light(*resources.volume))){
        record.fault=out.fault=1;return out;
    }
    const bool volume_map=volume&&resources.volume->side_count;
    if(iteration>=general_maximum(combined_maximum,program_maximum)||de<stop) {
        out.rm_result=de<stop?1:2;out.start_inside=1;active=point;
        if(scene.color_on_it){
            result=general_color_on_it(point,stop,scene,program,resources,lane,record);
            if(record.fault){out.fault=record.fault;return out;}
        }
        record.initial_rout=result.state.rout;
        out.orbit_trap=general_word(result.state.otrap);
        record.hit_color=capture_hit_color(lane);
    } else {
        float last_step=de*z_div,last_de=0.f;bool ended=false;
        for(uint step_index=0;step_index<scene.max_steps;++step_index) {
            ++record.ray_steps;
            if(iteration>=general_maximum(combined_maximum,program_maximum)) {
                const float step=last_step*-.5f;depth+=step;point+=ray*step;
                stop=base_stop*(1.f+depth*stop_factor);
                result=general_de(point,stop,program,resources,lane,record);
                if(record.fault)break;de=result.distance;iteration=uint(result.state.iteration);last_step=-step;
                combined_maximum=result.maximum;
            }
            if(iteration<scene.min_iterations || (iteration<general_maximum(combined_maximum,program_maximum) && de>=stop)) {
                // The first marching step of a pixel consumes its jitter.
                const bool first_random=native_primary_random()&&random&&!random->consumed;
                last_de=de;de=max(.11f,((de-as_type<float>(scene.de_sub)*stop)*z_div)*slope);
                const float limit=max(stop,.4f)*limiter;
                // RayMarch: DFogOnIt counts only steps after a DE of that iteration count.
                if(!scene.fog_iteration||iteration==scene.fog_iteration)
                    count+=limit<de?(first_random?random->step_add:limit/de):1.f;
                if(limit<de)de=limit;
                if(volume_map){volume_substep(point,ray,last_step,depth,stop_factor,resources,count,record);if(record.fault)break;}
                if(first_random){
                    random->consumed=true;random->seed=primary_random_advance(random->seed);
                    de*=float(random->seed&0x7fffffffu)*(1.f/2147483647.f);
                }
                depth+=de;
                if(depth>general_double(scene.max_ray_length)){
                    // CalcThread: one last sub-step walk up to MaxRayLength.
                    if(volume_map){const float rest=general_double(scene.max_ray_length)-depth+de;
                        volume_substep(point+ray*rest,ray,rest,depth,stop_factor,resources,count,record);}
                    ended=true;break;
                }
                last_step=de;point+=ray*de;stop=base_stop*(1.f+max(0.f,depth*axial_multiplier)*stop_factor);
                result=general_de(point,stop,program,resources,lane,record);
                if(record.fault)break;de=result.distance;iteration=uint(result.state.iteration);
                combined_maximum=result.maximum;
                if(de>last_de+last_step)de=last_de+last_step;
                if(last_de>de+1.e-30f){float ratio=last_step/(last_de-de);slope=ratio<1.f?max(.5f,ratio):1.f;}
                else slope=1.f;
            } else {
                final_de=de;const bool de_limited=iteration<general_maximum(combined_maximum,program_maximum)||de<stop;
                // CPU primary skips both refinement paths at header count0.
                if(scene.refinement_steps) {
                if(de_limited) {
                    float step=last_step*-.5f;
                    for(uint remaining=scene.refinement_steps;remaining>0&&abs(de-stop)>.001f;--remaining) {
                        ++record.refine_calls;depth+=step;point+=ray*step;stop=(depth*stop_factor+1.f)*base_stop;
                        if(remaining<=1)break;
                        result=general_de(point,stop,program,resources,lane,record);
                        if(record.fault)break;de=result.distance;iteration=uint(result.state.iteration);
                        step=abs(step)*(de<stop?-.55f:.55f);
                    }
                } else if(!general_refine_iteration(point,ray,depth,stop,scene,program,resources,lane,record,iteration))break;
                }
                axial=max(0.f,depth*axial_multiplier);active=point;out.rm_result=de_limited?1:2;ended=true;break;
            }
        }
        if(!ended&&!record.fault)record.fault=2;
        if(!out.rm_result)axial=max(0.f,depth*axial_multiplier);
    }
    for(uint i=0;i<3;++i)out.position[i]=general_word(active[i]);
    out.depth=general_word(depth);out.axial_depth=general_word(axial);out.final_de=general_word(final_de);
    out.final_stop=as_type<uint>(stop);out.step_count=as_type<uint>(count);out.final_iterations=iteration;
    if(out.rm_result&&!out.start_inside&&!record.fault) {
        normal_result=general_calculate_normals(active,depth,stop,scene,program,normal_params,
            resources,lane,record,normal_record);
        out.smooth_iterations=as_type<uint>(normal_result.smooth);
        out.orbit_trap=general_word(normal_result.orbit_trap);
        for(uint i=0;i<3;++i){out.raw_normal[i]=general_word(normal_result.raw_normal[i]);out.normal[i]=normal_result.packed[i];}
        if(scene.color_on_it&&!record.fault){
            const auto color=general_color_on_it(active,stop,scene,program,resources,lane,record);
            if(!record.fault)out.orbit_trap=general_word(color.state.otrap);
        }
        record.hit_color=capture_hit_color(lane);
    }
    if(!isfinite(depth)||!isfinite(axial)||!isfinite(final_de)||!isfinite(stop)||!isfinite(de)||
        !isfinite(count)||!all(isfinite(active))||!isfinite(as_type<float>(out.smooth_iterations))||
        !isfinite(record.initial_rout)||!isfinite(general_double(out.orbit_trap)))record.fault|=4;
    out.fault=record.fault;record.hit=out.rm_result!=0;return out;
}
inline ExactPixel general_primary_camera(constant ExactScene& scene,constant NativeFormulaProgram& program,
    mb3d_camera::Ray camera,uint x,uint y,thread NativeResources& resources,thread NativeGeometryRecord& record,
    thread NativeFormulaState& lane,NativeNormalParams normal_params,
    thread NativeNormalRecord& normal_record,thread NativeNormalResult& normal_result) {
    return general_primary_camera_attempt(scene,program,camera,x,y,resources,record,lane,
        normal_params,normal_record,normal_result,as_type<int>(program.inside));
}
// Preserve the old primitive entry point. Main rendering supplies the new
// validated camera tables; these tests retain their original perspective ABI.
inline ExactPixel general_primary(constant ExactScene& scene,constant NativeFormulaProgram& program,
    ExactTrig tx,ExactTrig ty,uint x,uint y,thread NativeResources& resources,thread NativeGeometryRecord& record,
    thread NativeFormulaState& lane,NativeNormalParams normal_params,
    thread NativeNormalRecord& normal_record,thread NativeNormalResult& normal_result) {
    mb3d_camera::Ray camera{};camera.version=mb3d_camera::Version;
    if(!mb3d_metal_exact::valid_camera_trig(tx)||!mb3d_metal_exact::valid_camera_trig(ty))camera.fault=1;
    else {
        const auto v=mb3d_metal_exact::camera_ray(tx,ty,scene);
        const float3 ray(v.x.value,v.y.value,v.z.value);
        const float3 origin(general_double(scene.y_start[0]),general_double(scene.y_start[1]),general_double(scene.y_start[2]));
        const float3 start=(general_column(scene,0)*float(x)+general_column(scene,1)*float(y))+origin;
        for(uint c=0;c<3;++c){camera.direction[c]=ray[c];camera.origin[c]=start[c];}
    }
    return general_primary_camera(scene,program,camera,x,y,resources,record,lane,normal_params,normal_record,normal_result);
}
inline ExactPixel general_primary(constant ExactScene& scene,constant NativeFormulaProgram& program,
    ExactTrig tx,ExactTrig ty,uint x,uint y,thread NativeResources& resources,thread NativeGeometryRecord& record,
    thread NativeFormulaState& lane) {
    NativeNormalParams params{kGeneralNormalVersion,1,0,uint(program.inside!=0),{0,0,0,0}};
    NativeNormalRecord normal_record{};NativeNormalResult normal_result{};
    return general_primary(scene,program,tx,ty,x,y,resources,record,lane,params,normal_record,normal_result);
}
inline ExactPixel general_primary(constant ExactScene& scene,constant NativeFormulaProgram& program,
    ExactTrig tx,ExactTrig ty,uint x,uint y,thread NativeResources& resources,thread NativeGeometryRecord& record) {
    auto lane=native_initial_state(float3(0),as_type<float>(scene.de_stop),program);
    return general_primary(scene,program,tx,ty,x,y,resources,record,lane);
}
inline uint general_clip15(float value) {return uint(rint(clamp(value,0.f,32767.f)));}
inline ExactSurface general_surface(ExactPixel pixel,constant ExactScene& scene,
    constant mb3d_native::NativeSurfaceParams& params,constant NativeFormulaProgram& program,float initial_rout=0.f,uint roughness_byte=0u,
    NativeHitColorPayload hit_color={}) {
    ExactSurface out{};out.depth_packed=0x80000000u;out.amb_shadow=5000;out.fault=pixel.fault;
    const float zcorr=general_double(params.z_corr),zmul=general_double(params.z_c_mul);
    const float plus=general_double(params.color_plus),cmul=general_double(params.color_multiplier);
    const float stopmul=general_double(params.color_stop_multiplier),width=general_double(params.step_width);
    if(!valid_general_scene(scene,program)||!valid_surface_color_descriptor(params,native_selected_driver(),program.de_option)||
        !isfinite(zcorr)||zcorr<=0.f||!isfinite(zmul)||zmul<=0.f||!isfinite(plus)||!isfinite(cmul)||
        !isfinite(stopmul)||!isfinite(width)||width<=0.f||pixel.reserved_tail)out.fault|=1;
    if(roughness_byte>255u)out.fault|=1;
    if(out.fault)return out;
    out.shadow=scene.fog_iteration==kNativeVolumeFog?mb3d_inside::volume_light_code(as_type<float>(pixel.step_count))&1023u:
        uint(rint(clamp((as_type<float>(pixel.step_count)-1.f)+1.f,0.f,1023.f)))&1023u;
    if(!pixel.rm_result)return out;
    uint orbit_color=0u;
    if(params.color_option){orbit_color=general_orbit_color(hit_color,params.orbit_color_multiplier,out.fault);if(out.fault)return out;}
    if(pixel.start_inside){
        // CPU renderer's no-cut initial-inside path performs no normal probes.
        // Color is captured before any inside smooth evaluation or cut packing.
        const float trap=general_double(pixel.orbit_trap);
        if(!isfinite(initial_rout)||!isfinite(trap)){
            out.fault|=4;return out;
        }
        // Clamp before scaling so a finite saturating source value cannot
        // manufacture FP32 infinity in a result whose CPU codec is bounded.
        const float ratio=clamp(initial_rout,0.f,program.radius_stop)/program.radius_stop;
        if(!isfinite(ratio)){out.fault|=4;return out;}
        out.depth_packed=0x7fff0000u;
        out.normal[0]=out.normal[1]=0;out.normal[2]=-32767;
        out.si_gradient=32768u+general_clip15(32767.f*clamp(ratio,0.f,1.f));
        out.o_trap=params.color_option?orbit_color:general_clip15(clamp(trap,0.f,32767.f/4096.f)*4096.f);
        return out;
    }
    for(uint i=0;i<3;++i)out.normal[i]=pixel.normal[i];
    float gradient=as_type<float>(pixel.smooth_iterations);
    // CalcThread: every DE-limited hit adds the DE-stop log term, analytic
    // drivers included; dIFS preparation sets its multiplier to zero.
    if(pixel.rm_result!=2) {
        gradient+=plus;
        gradient+=stopmul*log(max(as_type<float>(scene.de_stop),as_type<float>(pixel.final_stop))*width);
    }
    const float trap=general_double(pixel.orbit_trap),depth=max(0.f,general_double(pixel.depth));
    const float encoded=zmul*(sqrt(depth*zcorr+1.f)-1.f);
    if(!isfinite(encoded)||!isfinite(trap)||!isfinite(gradient)||!isfinite(gradient*cmul)) {out.fault|=4;return out;}
    out.si_gradient=general_clip15(32767.f-gradient*cmul);out.o_trap=params.color_option?orbit_color:general_clip15(trap*4096.f);
    // Match the CPU wrapping integer codec, including its indefinite conversion.
    const uint raw=8388352u-uint(mb3d_metal_exact::native_integer(encoded,true));
    out.depth_packed=(uint(clamp(as_type<int>(raw),0,8388352))<<8)|roughness_byte;return out;
}
inline ExactSurface general_surface_camera(ExactPixel pixel,constant ExactScene& scene,
    constant mb3d_native::NativeSurfaceParams& params,constant NativeFormulaProgram& program,float initial_rout,
    constant mb3d_camera::Params& camera,mb3d_camera::Ray ray,uint roughness_byte=0u,NativeHitColorPayload hit_color={}) {
    auto out=general_surface(pixel,scene,params,program,initial_rout,roughness_byte,hit_color);
    if(!mb3d_camera::valid_camera(camera)||!mb3d_camera::valid_ray(ray)||
       camera.width!=scene.width||camera.height!=scene.height){out.fault|=1;return out;}
    if(out.fault||!pixel.rm_result||!pixel.start_inside||!ray.cut_axis)return out;
    // Device address-space-local constants are passed by value in the bridge.
    const auto packed=mb3d_camera::cut_surface_values(camera,
        general_double(params.z_corr),general_double(params.z_c_mul),ray.cut_axis,general_double(pixel.depth));
    out.fault|=packed.fault;if(out.fault)return out;
    out.depth_packed=packed.depth;
    for(uint c=0;c<3;++c)out.normal[c]=packed.normal[c];
    return out;
}
} // namespace mb3d_native

// Separate AO AIR uses the same helper bodies without re-exporting geometry kernels.
#ifndef MB3D_NATIVE_GENERAL_HELPERS_ONLY
kernel void native_general_gradient_abi(device uint* words [[buffer(0)]],uint tid [[thread_position_in_grid]]) {
    if(tid)return;words[0]=4;words[1]=32;
}
// Bounded diagnostic entry point for the same production color codec.
kernel void native_general_orbit_color_probe(device const mb3d_native::NativeHitColorPayload* input [[buffer(0)]],
    device const float* multipliers [[buffer(1)]],device uint2* output [[buffer(2)]],
    constant uint& count [[buffer(3)]],uint tid [[thread_position_in_grid]]) {
    if(tid>=count)return;
    uint fault=0;const uint value=mb3d_native::general_orbit_color(input[tid],multipliers[tid],fault);
    output[tid]=uint2(value,fault);
}
// Diagnostic entry exercises the production DE wrapper and independently
// exposes the number of certified parts before query-level aggregation.
kernel void native_general_gradient_count_probe(constant mb3d_native::NativeFormulaProgram& program [[buffer(0)]],
    device const mb3d_native::NativeFormulaState* inputs [[buffer(1)]],device mb3d_native::NativeGeometryRecord* records [[buffer(2)]],
    device uint4* outputs [[buffer(3)]],constant uint& count [[buffer(4)]],uint tid [[thread_position_in_grid]]) {
    if(tid>=count)return;
    using namespace mb3d_native;
    const auto input=inputs[tid];const float3 point(input.c[0],input.c[1],input.c[2]);
    NativeResources resources{};NativeFormulaState lane=input;NativeGeometryRecord record{};
    const auto wrapped=general_de(point,input.rstop_d,program,resources,lane,record);
    NativeResources direct_resources{};lane=input;uint certified_parts=0;
    const auto direct=native_distance_combined(point,input.rstop_d,program,direct_resources,lane,
        0u,program.max_iterations,false,&certified_parts);
    records[tid]=record;
    outputs[tid]=uint4(certified_parts,as_type<uint>(wrapped.distance),as_type<uint>(direct.distance),direct.fault.code);
}
kernel void native_general_abi(device uint* words [[buffer(0)]],uint tid [[thread_position_in_grid]]) {
    if(tid)return;words[0]=mb3d_native::kGeneralGeometryVersion;words[1]=sizeof(ExactScene);words[2]=sizeof(mb3d_native::NativeFormulaProgram);
    words[3]=sizeof(ExactPixel);words[4]=sizeof(ExactSurface);words[5]=sizeof(mb3d_native::NativeGeometryRecord);words[6]=32;
}
kernel void native_general_primary(device ExactPixel* primary [[buffer(0)]],
    device mb3d_native::NativeGeometryRecord* records [[buffer(1)]],constant ExactScene& scene [[buffer(2)]],
    constant mb3d_native::NativeFormulaProgram& program [[buffer(3)]],
    device const ExactTrig* columns [[buffer(4)]],device const ExactTrig* rows [[buffer(5)]],
    device const mb3d_native::NativeMapDescriptor* maps [[buffer(6)]],device const float* pixels [[buffer(7)]],
    constant uint4& launch [[buffer(8)]],constant uint2& resource_counts [[buffer(9)]],uint local [[thread_position_in_grid]]) {
    if(local>=launch.y||launch.x>=launch.z||local>=launch.z-launch.x)return;const uint index=launch.x+local;
    mb3d_native::NativeGeometryRecord record{};record.version=mb3d_native::kGeneralGeometryVersion;ExactPixel value{};
    if(launch.w||!scene.width||scene.width>4096||!scene.height||scene.height>4096||scene.width*scene.height!=launch.z)record.fault=value.fault=1;
    else {mb3d_native::NativeResources resources{maps,pixels,resource_counts.x,resource_counts.y,nullptr};
        value=mb3d_native::general_primary(scene,program,columns[index%scene.width],rows[index/scene.width],
            index%scene.width,index/scene.width,resources,record);}
    primary[index]=value;records[index]=record;
}
kernel void native_general_surface(device const ExactPixel* primary [[buffer(0)]],device ExactSurface* output [[buffer(1)]],
    constant ExactScene& scene [[buffer(2)]],constant mb3d_native::NativeSurfaceParams& packing [[buffer(3)]],
    constant mb3d_native::NativeFormulaProgram& program [[buffer(4)]],constant uint4& launch [[buffer(5)]],
    device const mb3d_native::NativeGeometryRecord* geometry [[buffer(6)]],
    uint local [[thread_position_in_grid]]) {
    if(local>=launch.y||launch.x>=launch.z||local>=launch.z-launch.x)return;const uint index=launch.x+local;
    ExactSurface value{};value.depth_packed=0x80000000u;
    if(launch.w||!scene.width||scene.width>4096||!scene.height||scene.height>4096||scene.width*scene.height!=launch.z)value.fault=1;
    else {
        const auto record=geometry[index];
        if(record.version!=mb3d_native::kGeneralGeometryVersion||!mb3d_native::valid_smoothing_counters(record.smoothing,record.formula_work,mb3d_native::native_selected_smoothing_policy())||!metal::isfinite(record.initial_rout)||
           !mb3d_native::valid_difs_carried_reads(record.difs_carried_reads,record.de_calls,record.normal_calls)||record.de_calls>mb3d_native::kGeneralMaxDistanceCalls||record.gradient_range_clamps>record.de_calls||
           !mb3d_native::valid_fp32_range_exits(record.fp32_range_exits,record.de_calls,record.normal_calls))value.fault=1;
        else value=mb3d_native::general_surface(primary[index],scene,packing,program,record.initial_rout,0u,record.hit_color);
    }
    output[index]=value;
}

kernel void native_general_lane_abi(device uint* words [[buffer(0)]],uint tid [[thread_position_in_grid]]) {
    if(tid)return;words[0]=1;words[1]=sizeof(mb3d_native::NativeLogicalLane);
    words[2]=sizeof(mb3d_native::NativeFormulaState);words[3]=sizeof(mb3d_native::NativeLogicalLaneRecord);
    words[4]=mb3d_native::kLogicalLaneBatchPixels;
}
kernel void native_general_primary_lanes(device ExactPixel* primary [[buffer(0)]],
    device mb3d_native::NativeGeometryRecord* records [[buffer(1)]],constant ExactScene& scene [[buffer(2)]],
    constant mb3d_native::NativeFormulaProgram& program [[buffer(3)]],
    device const ExactTrig* columns [[buffer(4)]],device const ExactTrig* rows [[buffer(5)]],
    device const mb3d_native::NativeMapDescriptor* maps [[buffer(6)]],device const float* pixels [[buffer(7)]],
    constant uint4& launch [[buffer(8)]],constant uint2& resource_counts [[buffer(9)]],
    device const mb3d_native::NativeLogicalLane* descriptors [[buffer(10)]],
    device mb3d_native::NativeFormulaState* states [[buffer(11)]],
    device mb3d_native::NativeLogicalLaneRecord* lane_records [[buffer(12)]],uint local [[thread_position_in_grid]]) {
    if(local>=launch.y||launch.x>=launch.z||local>=launch.z-launch.x)return;
    const uint id=launch.x+local;const auto descriptor=descriptors[id];
    mb3d_native::NativeLogicalLaneRecord status{1,0,0,descriptor.cursor};
    if(!mb3d_native::valid_logical_lane(descriptor,scene.width,scene.height,launch.w)||
       launch.z>scene.height||descriptor.row_start!=id||descriptor.row_stride!=launch.z||
       descriptor.row_count!=1+(scene.height-1-id)/launch.z||
       !mb3d_native::valid_general_scene(scene,program)) {
        status.fault=1;lane_records[id]=status;return;
    }
    if(!descriptor.pixel_count){lane_records[id]=status;return;}
    auto lane=descriptor.cursor?states[id]:
        mb3d_native::native_initial_state(float3(0),as_type<float>(scene.de_stop),program);
    mb3d_native::NativeResources resources{maps,pixels,resource_counts.x,resource_counts.y,nullptr};
    for(uint k=0;k<descriptor.pixel_count;++k) {
        const uint ordinal=descriptor.cursor+k,x=ordinal%scene.width;
        const uint y=descriptor.row_start+(ordinal/scene.width)*descriptor.row_stride;
        const uint index=y*scene.width+x;
        mb3d_native::NativeGeometryRecord record{};
        const auto value=mb3d_native::general_primary(scene,program,columns[x],rows[y],x,y,resources,record,lane);
        primary[index]=value;records[index]=record;++status.processed;++status.next_cursor;
        if(record.fault){status.fault=record.fault;break;}
    }
    states[id]=lane;lane_records[id]=status;
}

kernel void native_general_normal_abi(device uint* words [[buffer(0)]],uint tid [[thread_position_in_grid]]) {
    if(tid)return;words[0]=mb3d_native::kGeneralNormalVersion;words[1]=sizeof(mb3d_native::NativeNormalParams);
    words[2]=sizeof(mb3d_native::NativeNormalRecord);words[3]=sizeof(mb3d_native::NativeNormalResult);
    words[4]=mb3d_native::kGeneralNormalMaxSamples;
}
kernel void native_general_primary_with_normals(device ExactPixel* primary [[buffer(0)]],
    device mb3d_native::NativeGeometryRecord* records [[buffer(1)]],constant ExactScene& scene [[buffer(2)]],
    constant mb3d_native::NativeFormulaProgram& program [[buffer(3)]],
    device const ExactTrig* columns [[buffer(4)]],device const ExactTrig* rows [[buffer(5)]],
    device const mb3d_native::NativeMapDescriptor* maps [[buffer(6)]],device const float* pixels [[buffer(7)]],
    constant uint4& launch [[buffer(8)]],constant uint2& resource_counts [[buffer(9)]],
    constant mb3d_native::NativeNormalParams& normal_params [[buffer(20)]],
    device mb3d_native::NativeNormalRecord* normal_records [[buffer(21)]],
    device mb3d_native::NativeNormalResult* normal_results [[buffer(22)]],uint local [[thread_position_in_grid]]) {
    if(local>=launch.y||launch.x>=launch.z||local>=launch.z-launch.x)return;const uint index=launch.x+local;
    mb3d_native::NativeGeometryRecord record{};record.version=mb3d_native::kGeneralGeometryVersion;ExactPixel value{};
    mb3d_native::NativeNormalRecord normal_record{};normal_record.version=mb3d_native::kGeneralNormalVersion;
    mb3d_native::NativeNormalResult normal_result{};
    if(launch.w||!scene.width||scene.width>4096||!scene.height||scene.height>4096||scene.width*scene.height!=launch.z)record.fault=value.fault=1;
    else {mb3d_native::NativeResources resources{maps,pixels,resource_counts.x,resource_counts.y,nullptr};
        auto lane=mb3d_native::native_initial_state(float3(0),as_type<float>(scene.de_stop),program);
        value=mb3d_native::general_primary(scene,program,columns[index%scene.width],rows[index/scene.width],
            index%scene.width,index/scene.width,resources,record,lane,normal_params,normal_record,normal_result);}
    primary[index]=value;records[index]=record;normal_records[index]=normal_record;normal_results[index]=normal_result;
}
kernel void native_general_surface_with_normals(device const ExactPixel* primary [[buffer(0)]],device ExactSurface* output [[buffer(1)]],
    constant ExactScene& scene [[buffer(2)]],constant mb3d_native::NativeSurfaceParams& packing [[buffer(3)]],
    constant mb3d_native::NativeFormulaProgram& program [[buffer(4)]],constant uint4& launch [[buffer(5)]],
    device const mb3d_native::NativeGeometryRecord* geometry [[buffer(6)]],
    constant mb3d_native::NativeNormalParams& normal_params [[buffer(20)]],
    device const mb3d_native::NativeNormalRecord* normal_records [[buffer(21)]],
    device const mb3d_native::NativeNormalResult* normal_results [[buffer(22)]],uint local [[thread_position_in_grid]]) {
    if(local>=launch.y||launch.x>=launch.z||local>=launch.z-launch.x)return;const uint index=launch.x+local;
    ExactSurface value{};value.depth_packed=0x80000000u;
    if(launch.w||!scene.width||scene.width>4096||!scene.height||scene.height>4096||scene.width*scene.height!=launch.z)value.fault=1;
    else {
        const auto record=geometry[index];const auto n=normal_records[index];const auto v=normal_results[index];
        if(!mb3d_native::valid_general_normals(normal_params,program)||n.version!=mb3d_native::kGeneralNormalVersion||
           n.fault||n.reserved[0]||n.reserved[1]||v.fault||v.reserved[0]||v.reserved[1]||v.reserved[2]||v.reserved[3]||
           v.roughness_byte>255u||v.completed!=uint(primary[index].rm_result&&!primary[index].start_inside)||
           record.version!=mb3d_native::kGeneralGeometryVersion||!mb3d_native::valid_smoothing_counters(record.smoothing,record.formula_work,mb3d_native::native_selected_smoothing_policy())||!metal::isfinite(record.initial_rout)||
           !mb3d_native::valid_difs_carried_reads(record.difs_carried_reads,record.de_calls,record.normal_calls)||record.de_calls>mb3d_native::kGeneralMaxDistanceCalls||record.gradient_range_clamps>record.de_calls||
           !mb3d_native::valid_fp32_range_exits(record.fp32_range_exits,record.de_calls,record.normal_calls))value.fault=1;
        else value=mb3d_native::general_surface(primary[index],scene,packing,program,record.initial_rout,v.roughness_byte,record.hit_color);
    }
    output[index]=value;
}

kernel void native_general_primary_lanes_with_normals(device ExactPixel* primary [[buffer(0)]],
    device mb3d_native::NativeGeometryRecord* records [[buffer(1)]],constant ExactScene& scene [[buffer(2)]],
    constant mb3d_native::NativeFormulaProgram& program [[buffer(3)]],
    device const ExactTrig* columns [[buffer(4)]],device const ExactTrig* rows [[buffer(5)]],
    device const mb3d_native::NativeMapDescriptor* maps [[buffer(6)]],device const float* pixels [[buffer(7)]],
    constant uint4& launch [[buffer(8)]],constant uint2& resource_counts [[buffer(9)]],
    device const mb3d_native::NativeLogicalLane* descriptors [[buffer(10)]],
    device mb3d_native::NativeFormulaState* states [[buffer(11)]],
    device mb3d_native::NativeLogicalLaneRecord* lane_records [[buffer(12)]],
    constant mb3d_native::NativeNormalParams& normal_params [[buffer(20)]],
    device mb3d_native::NativeNormalRecord* normal_records [[buffer(21)]],
    device mb3d_native::NativeNormalResult* normal_results [[buffer(22)]],uint local [[thread_position_in_grid]]) {
    if(local>=launch.y||launch.x>=launch.z||local>=launch.z-launch.x)return;
    const uint id=launch.x+local;const auto descriptor=descriptors[id];
    mb3d_native::NativeLogicalLaneRecord status{1,0,0,descriptor.cursor};
    if(!mb3d_native::valid_logical_lane(descriptor,scene.width,scene.height,launch.w)||
       launch.z>scene.height||descriptor.row_start!=id||descriptor.row_stride!=launch.z||
       descriptor.row_count!=1+(scene.height-1-id)/launch.z||
       !mb3d_native::valid_general_scene(scene,program)||!mb3d_native::valid_general_normals(normal_params,program)) {
        status.fault=1;lane_records[id]=status;return;
    }
    if(!descriptor.pixel_count){lane_records[id]=status;return;}
    auto lane=descriptor.cursor?states[id]:
        mb3d_native::native_initial_state(float3(0),as_type<float>(scene.de_stop),program);
    mb3d_native::NativeResources resources{maps,pixels,resource_counts.x,resource_counts.y,nullptr};
    for(uint k=0;k<descriptor.pixel_count;++k) {
        const uint ordinal=descriptor.cursor+k,x=ordinal%scene.width;
        const uint y=descriptor.row_start+(ordinal/scene.width)*descriptor.row_stride;
        const uint index=y*scene.width+x;
        mb3d_native::NativeGeometryRecord record{};
        mb3d_native::NativeNormalRecord normal_record{};mb3d_native::NativeNormalResult normal_result{};
        const auto value=mb3d_native::general_primary(scene,program,columns[x],rows[y],x,y,resources,record,lane,normal_params,normal_record,normal_result);
        primary[index]=value;records[index]=record;normal_records[index]=normal_record;normal_results[index]=normal_result;++status.processed;++status.next_cursor;
        if(record.fault){status.fault=record.fault;break;}
    }
    states[id]=lane;lane_records[id]=status;
}

#endif // MB3D_NATIVE_GENERAL_HELPERS_ONLY

#ifndef MB3D_NATIVE_GENERAL_HELPERS_ONLY
kernel void native_general_smoothing_abi(device uint* words [[buffer(0)]],uint tid [[thread_position_in_grid]]) {
    if(tid)return;words[0]=mb3d_native::kNativeSmoothingVersion;
    words[1]=mb3d_native::native_selected_smoothing_policy();
    words[2]=sizeof(mb3d_native::NativeSmoothingCounters);words[3]=mb3d_native::kGeneralGeometryVersion;
}

#endif // MB3D_NATIVE_GENERAL_HELPERS_ONLY: single-TU smoothing marker
