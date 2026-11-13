#pragma once
// Include after the staged general_geometry.metal.h. Camera/cut ownership is
// separate: a camera-aware attempt must reconstruct its original start on each
// call; it must not rerun camera setup from the previous attempt's endpoint.
#include "native_inside_math.h"
namespace mb3d_native {
inline ExactPixel general_primary_inside(constant ExactScene& scene,
    constant NativeFormulaProgram& program,constant mb3d_inside::Params& policy,
    mb3d_camera::Ray camera,uint x,uint y,thread NativeResources& resources,
    thread NativeGeometryRecord& geometry,thread NativeFormulaState& lane,
    thread mb3d_inside::Record& activity,NativeNormalParams normal_params,
    thread NativeNormalRecord& normal_record,thread NativeNormalResult& normal_result,
    thread PrimaryRandomState* random=nullptr) {
    activity={};activity.version=mb3d_inside::Version;
    ExactPixel pixel{};
    if(!mb3d_inside::valid(policy)||as_type<uint>(policy.initial_inside)!=program.inside||
       policy.volume_light!=uint(scene.fog_iteration==kNativeVolumeFog)){
        activity.fault=geometry.fault=pixel.fault=1;return pixel;
    }
    auto state=mb3d_inside::begin(policy);
    // The attempt owns ray state; the evolving formula lane and the total work
    // record intentionally survive the retry, matching renderer.cpp's loop.
    for(uint attempt=0;attempt<2;++attempt){
        ++activity.attempts;
        pixel=general_primary_camera_attempt(scene,program,camera,x,y,resources,geometry,lane,
            normal_params,normal_record,normal_result,state.inside,random);
        if(pixel.fault||geometry.fault)break;
        if(!mb3d_inside::retry(policy,state,pixel.start_inside))break;
        ++activity.retries;
    }
    activity.final_inside=state.inside;activity.start_inside=pixel.start_inside;
    if(!pixel.fault&&!geometry.fault&&pixel.rm_result&&pixel.start_inside&&state.inside){
        // RMdoColor/cut-normal happen before this smooth CalcDE. Preserve the
        // already captured orbit trap and initial radius for surface packing;
        // only SmoothItD and the persistent formula lane take the later values.
        const float3 point(general_double(pixel.position[0]),general_double(pixel.position[1]),general_double(pixel.position[2]));
        ++activity.smooth_calls;
        const auto result=general_de(point,as_type<float>(pixel.final_stop),program,resources,lane,geometry,true);
        pixel.smooth_iterations=as_type<uint>(result.state.smooth);
    }
    activity.fault=pixel.fault|geometry.fault;pixel.fault=activity.fault;return pixel;
}
// Primary jitter probe: 1 when the final attempt marches, 0 when it starts
// inside, 2 on a fault. It repeats general_primary_camera_attempt's start and
// initial check per attempt, with the same lane; an attempt that passes that
// check always takes a marching step first. Only a start-inside attempt runs
// RMdoColorOnIt before the retry, which reads no lane an independent program
// carries; the seeded pass checks every pixel's advance against its successor.
inline uint general_primary_probe(constant ExactScene& scene,constant NativeFormulaProgram& program,
    constant mb3d_inside::Params& policy,mb3d_camera::Ray camera,thread NativeResources& resources,
    thread NativeGeometryRecord& record,thread NativeFormulaState& lane) {
    if(!mb3d_inside::valid(policy)||as_type<uint>(policy.initial_inside)!=program.inside||
       policy.volume_light!=uint(scene.fog_iteration==kNativeVolumeFog)||!valid_general_scene(scene,program)||
       !mb3d_camera::valid_ray(camera))return 2;
    const float3 ray(camera.direction[0],camera.direction[1],camera.direction[2]);
    const float3 start(camera.origin[0],camera.origin[1],camera.origin[2]);
    if(!all(isfinite(ray))||!all(isfinite(start)))return 2;
    const float radius=(ray.x*ray.x+ray.y*ray.y)+ray.z*ray.z;
    const float axial_multiplier=radius/sqrt(radius*radius);
    const float base_stop=as_type<float>(scene.de_stop),stop_factor=as_type<float>(scene.de_stop_factor);
    const uint program_maximum=uint(program.max_iterations)+(program.de_option==20?1u:0u);
    if(scene.fog_iteration==kNativeVolumeFog&&(!resources.volume||!resources.volume_map||!valid_volume_light(*resources.volume)))return 2;
    auto state=mb3d_inside::begin(policy);bool start_inside=false;
    for(uint attempt=0;attempt<2;++attempt){
        const float depth=mb3d_camera::initial_depth(camera,general_double(scene.max_ray_length));
        const float stop=mb3d_camera::initial_stop(camera,base_stop,stop_factor,axial_multiplier);
        const float3 point=mb3d_camera::initial_point(camera);
        if(!isfinite(depth)||!isfinite(stop)||stop<=0.f||!all(isfinite(point)))return 2;
        lane.inside=as_type<uint>(state.inside);
        NativeOrbitResult result{};
        if(!camera.cut_miss)result=general_de(point,stop,program,resources,lane,record);
        if(record.fault||!isfinite(axial_multiplier))return 2;
        const float de=camera.cut_miss?base_stop+5.f:result.distance;
        const uint iteration=camera.cut_miss?0u:uint(result.state.iteration);
        const uint combined_maximum=camera.cut_miss?program_maximum:result.maximum;
        start_inside=iteration>=general_maximum(combined_maximum,program_maximum)||de<stop;
        if(!mb3d_inside::retry(policy,state,start_inside))break;
    }
    return start_inside?0u:1u;
}
inline ExactSurface general_surface_inside(ExactPixel pixel,constant ExactScene& scene,
    constant mb3d_native::NativeSurfaceParams& packing,constant NativeFormulaProgram& program,
    constant mb3d_inside::Params& policy,mb3d_inside::Record inside,float initial_rout,
    constant mb3d_camera::Params& camera,mb3d_camera::Ray ray,uint roughness_byte,NativeHitColorPayload hit_color) {
    ExactSurface result{};result.depth_packed=0x80000000u;
    if(!mb3d_inside::valid(policy)||policy.volume_light!=uint(scene.fog_iteration==kNativeVolumeFog)||
       inside.version!=mb3d_inside::Version||inside.reserved||
       inside.fault||inside.retries>1||inside.attempts!=inside.retries+1||
       inside.start_inside!=pixel.start_inside||inside.smooth_calls>1||
       !mb3d_inside::flag_word(inside.final_inside)||
       inside.final_inside!=(inside.retries?(policy.initial_inside?0:-1):policy.initial_inside)||
       inside.smooth_calls!=uint(pixel.rm_result&&pixel.start_inside&&inside.final_inside)){
        result.fault=pixel.fault|inside.fault|1u;return result;
    }
    result=general_surface_camera(pixel,scene,packing,program,initial_rout,camera,ray,roughness_byte,hit_color);
    if(result.fault)return result;
    const auto values=mb3d_inside::pack(policy,inside.final_inside,pixel.start_inside,
        uint(pixel.rm_result!=0),as_type<float>(pixel.step_count),as_type<float>(scene.de_stop),
        as_type<float>(pixel.smooth_iterations),general_double(packing.color_multiplier),
        result.si_gradient,result.o_trap);
    if(values.fault){result.fault=values.fault;return result;}
    result.shadow=(result.shadow&0xfc00u)|values.shadow;
    result.si_gradient=values.gradient;result.o_trap=values.otrap;
    // Camera/cut integration must replace depth+normal using its CutSurface
    // only for start_inside, after this function. It must not rerun DE/color.
    return result;
}
}
