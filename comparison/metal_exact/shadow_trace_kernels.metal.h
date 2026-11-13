#pragma once
#include "shadow_trace_math.h"
using namespace mb3d_metal_exact;

kernel void exact_shadow_trace(device const ExactShadowRay* rays [[buffer(0)]],
                               device ExactShadowTrace* output [[buffer(1)]],
                               constant ExactScene& scene [[buffer(2)]],
                               constant ExactShadowParams& params [[buffer(3)]],
                               constant uint& count [[buffer(4)]],
                               uint pixel [[thread_position_in_grid]]) {
    if(pixel>=count)return;
    if(!valid_shadow_scene(scene,params)||ulong(scene.width)*scene.height!=count){
        ExactShadowTrace error={};error.version=1;error.fault=1;output[pixel]=error;return;
    }
    output[pixel]=trace_shadow_ray(scene,params,rays[pixel]);
}

kernel void exact_hard_shadow_traced(device const ExactShadowRay* rays [[buffer(0)]],
                                     device const ExactShadowTrace* traces [[buffer(1)]],
                                     device const ExactSurface* input [[buffer(2)]],
                                     device ExactSurface* output [[buffer(3)]],
                                     constant ExactScene& scene [[buffer(4)]],
                                     constant ExactShadowParams& params [[buffer(5)]],
                                     constant uint* launch [[buffer(6)]],
                                     uint lane [[thread_position_in_grid]]) {
    uint y=lane+launch[0];if(y>=launch[2])return;
    ShadowRowState state={};
    const bool valid=scene.width==launch[1]&&scene.height==launch[2]&&valid_shadow_scene(scene,params);
    for(uint x=0;x<launch[1];++x){
        uint pixel=y*launch[1]+x;ExactSurface value=input[pixel];
        if(valid)value=shadow_pixel_traced(scene,params,rays[pixel],traces[pixel],value,state);
        else value.fault|=1;
        output[pixel]=value;
    }
}
