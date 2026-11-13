#pragma once
#include "native_camera.metal.h"
#include "native_camera_cut.metal.h"
namespace mb3d_camera {
// Separate per-pixel record; no ExactPixel reserved fields are repurposed.
inline bool valid_ray(Ray r) {
    if(r.version!=Version||r.fault||r.cut_axis>3||r.cut_miss>1||
       r.direction[3]!=0||r.origin[3]!=0||r.reserved[0]||r.reserved[1]||r.reserved[2]||
       !isfinite(r.cut_depth)||r.cut_depth<0)return false;
    for(uint c=0;c<3;++c)if(!isfinite(r.direction[c])||!isfinite(r.origin[c]))return false;
    return true;
}
// A cut miss still runs ordinary primary bookkeeping. In the CPU source it
// supplies DEstop+5 and Zend+1 without a first CalcDE call, then exits through
// the ordinary march loop; an early background return changes dynamic fog.
inline float initial_depth(Ray r,float maximum) {return r.cut_miss?maximum+1.f:r.cut_depth;}
inline float initial_stop(Ray r,float base,float factor,float axial_multiplier) {
    return r.cut_miss?base:base*(1.f+max(0.f,r.cut_depth*axial_multiplier)*factor);
}
inline float3 initial_point(Ray r) {
    const float3 start(r.origin[0],r.origin[1],r.origin[2]);
    const float3 direction(r.direction[0],r.direction[1],r.direction[2]);
    return r.cut_miss?start:start+direction*r.cut_depth;
}
}
