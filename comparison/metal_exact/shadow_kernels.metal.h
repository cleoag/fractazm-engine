#pragma once
#include "shadow_math.h"
using namespace mb3d_metal_exact;
kernel void exact_surface(device const ExactPixel* primary [[buffer(0)]],
                          device ExactSurface* output [[buffer(1)]],
                          constant ExactScene& scene [[buffer(2)]],
                          constant ExactSurfaceParams& params [[buffer(3)]],
                          constant uint& count [[buffer(4)]],uint i [[thread_position_in_grid]]) {
    if(i>=count)return;
    ExactSurface value=pack_surface(scene,params,primary[i]);
    if(!valid_scene(scene)||ulong(scene.width)*scene.height!=count)value.fault|=1;
    output[i]=value;
}
kernel void exact_hard_shadow(device const ExactPixel* primary [[buffer(0)]],
                              device const ExactSurface* input [[buffer(1)]],
                              device ExactSurface* output [[buffer(2)]],
                              constant ExactScene& scene [[buffer(3)]],
                              constant ExactShadowParams& params [[buffer(4)]],
                              constant uint* launch [[buffer(5)]],uint lane [[thread_position_in_grid]]) {
    uint y=lane+launch[0];if(y>=launch[2])return;
    ShadowRowState state={};
    const bool valid=scene.width==launch[1]&&scene.height==launch[2]&&valid_shadow_scene(scene,params);
    for(uint x=0;x<launch[1];++x){
        uint i=y*launch[1]+x;ExactSurface value=input[i];
        if(valid)value=shadow_pixel(scene,params,primary[i],value,state,y);
        else value.fault|=1;
        output[i]=value;
    }
}
kernel void exact_shadow_prepare(device const ExactPixel* primary [[buffer(0)]],
                                  device const ExactSurface* surface [[buffer(1)]],
                                  device ExactShadowRay* output [[buffer(2)]],
                                  constant ExactScene& scene [[buffer(3)]],
                                  constant ExactShadowParams& params [[buffer(4)]],
                                  constant uint& count [[buffer(5)]],
                                  uint pixel [[thread_position_in_grid]]) {
    if(pixel>=count)return;
    if(!valid_shadow_scene(scene,params)||ulong(scene.width)*scene.height!=count){
        ExactShadowRay error={};error.version=1;error.fault=1;output[pixel]=error;return;
    }
    output[pixel]=prepare_shadow_ray(scene,params,primary[pixel],surface[pixel],pixel/scene.width);
}
kernel void exact_hard_shadow_prepared(device const ExactShadowRay* rays [[buffer(0)]],
                                       device const ExactSurface* input [[buffer(1)]],
                                       device ExactSurface* output [[buffer(2)]],
                                       constant ExactScene& scene [[buffer(3)]],
                                       constant ExactShadowParams& params [[buffer(4)]],
                                       constant uint* launch [[buffer(5)]],
                                       uint lane [[thread_position_in_grid]]) {
    uint y=lane+launch[0];if(y>=launch[2])return;
    ShadowRowState state={};
    const bool valid=scene.width==launch[1]&&scene.height==launch[2]&&valid_shadow_scene(scene,params);
    for(uint x=0;x<launch[1];++x){
        uint pixel=y*launch[1]+x;ExactSurface value=input[pixel];
        if(valid)value=shadow_pixel_prepared(scene,params,rays[pixel],value,state);
        else value.fault|=1;
        output[pixel]=value;
    }
}
