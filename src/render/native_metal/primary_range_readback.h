#pragma once
// Host admission of a discarded primary pass. A range decision may skip later
// effects, but must not turn a corrupt primary record into a CPU success.
#include "smoothing_readback.h"
#include "general_normals_abi.h"
#include "native_camera_abi.h"
#include "inside_readback.h"
#include "comparison/metal_exact/shadow_abi.h"
#include <bit>
#include <cmath>
namespace mb3d_native {
inline bool valid_primary_normal(const NativeNormalParams& params,
    const NativeNormalRecord& n,const NativeNormalResult& v,U32 evaluations,
    const NativeGeometryRecord& geometry) {
    const U32 samples=GeneralNormalSamples(params.normals_on_de,params.smoothing);
    if(n.version!=kGeneralNormalVersion||n.fault||n.reserved[0]||n.reserved[1]||
       n.completed_evaluations!=evaluations||n.sample_calls!=evaluations*samples||
       n.sample_calls!=n.de_calls+n.direct_orbit_calls||n.sample_calls!=geometry.normal_calls||
       n.sample_calls>kGeneralNormalSampleBudget||n.direct_orbit_calls>kGeneralNormalOrbitBudget||
       (params.normals_on_de?n.direct_orbit_calls!=0:n.de_calls!=0)||
       v.fault||v.completed!=U32(evaluations!=0)||v.reserved[0]||v.reserved[1]||v.reserved[2]||v.reserved[3]||
       v.roughness_byte>255||(!params.smoothing&&v.roughness_byte)||
       !std::isfinite(v.smooth)||!std::isfinite(v.orbit_trap)||!std::isfinite(v.roughness)||v.roughness<0||v.roughness>1)return false;
    for(unsigned axis=0;axis<3;++axis)
        if(!std::isfinite(v.raw_normal[axis])||v.packed[axis]<-32768||v.packed[axis]>32767)return false;
    return true;
}
inline const char* primary_range_error(const NativeGeometryRecord& g,const ExactPixel& p,
    const ExactSurface& surface,const mb3d_camera::Ray& ray,U32 cutOptions,double maxRay,
    const NativeNormalParams& normalParams,const NativeNormalRecord& normal,
    const NativeNormalResult& result,const mb3d_inside::Params& insideParams,
    const mb3d_inside::Record& inside,U32 smoothingPolicy,U32 colorOption) {
    if(!valid_smoothing_observation(g,smoothingPolicy)||g.fault||g.formula_fault.code||
       g.smoothing.invalid||!std::isfinite(g.initial_rout))return "geometry";
    if(ray.version!=mb3d_camera::Version||ray.fault||ray.cut_axis>3||ray.cut_miss>1||
       ray.direction[3]!=0||ray.origin[3]!=0||ray.reserved[0]||ray.reserved[1]||ray.reserved[2]||
       !std::isfinite(ray.cut_depth)||ray.cut_depth<0||ray.cut_miss!=U32(ray.cut_depth>maxRay)||
       (ray.cut_axis&&!(cutOptions&(1u<<(ray.cut_axis-1)))))return "camera";
    for(unsigned axis=0;axis<3;++axis) {
        if(!std::isfinite(ray.direction[axis])||!std::isfinite(ray.origin[axis]))return "camera finite";
        for(auto v:{p.ray[axis],p.start[axis],p.position[axis],p.raw_normal[axis]})
            if(!std::isfinite(std::bit_cast<double>(v)))return "pixel finite";
    }
    for(auto v:{p.depth,p.axial_depth,p.final_de,p.orbit_trap})
        if(!std::isfinite(std::bit_cast<double>(v)))return "pixel finite";
    for(auto v:{p.step_count,p.smooth_iterations,p.final_stop})
        if(!std::isfinite(std::bit_cast<float>(v)))return "pixel finite";
    if(p.fault||p.reserved_tail||p.rm_result>2||p.start_inside>1||g.hit!=U32(p.rm_result!=0)||
       (ray.cut_miss?(g.de_calls||g.formula_work):(!g.de_calls||!g.formula_work)))return "pixel/work";
    if(p.rm_result&&colorOption&&(g.hit_color.version!=kNativeHitColorVersion||g.hit_color.reserved||
       !std::isfinite(g.hit_color.rout)||!std::isfinite(g.hit_color.rold)))return "orbit color";
    if(mb3d_inside::validate_primary_record(insideParams,inside,{p.fault,U32(p.rm_result!=0),p.start_inside}))return "inside";
    if(!valid_primary_normal(normalParams,normal,result,U32(p.rm_result&&!p.start_inside),g))return "normal";
    if(surface.fault||surface.reserved[0]||surface.reserved[1]||surface.reserved[2]||
       (surface.depth_packed&255u)!=result.roughness_byte)return "surface";
    return nullptr;
}
}
