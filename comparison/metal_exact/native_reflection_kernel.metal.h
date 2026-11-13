#pragma once
#include "native_reflection_math.metal.h"

kernel void native_reflection_abi(device uint* output [[buffer(0)]]) {
    output[0]=1;output[1]=sizeof(NativeReflectionParams);output[2]=sizeof(NativeReflectionRecord);
}

kernel void native_reflection(constant ExactScene& scene [[buffer(0)]],
    constant ExactSurfaceParams& packing [[buffer(1)]],
    constant ExactShadowParams& shadow [[buffer(2)]],
    constant ExactColorScene& color [[buffer(3)]],
    constant NativeReflectionParams& params [[buffer(4)]],
    device const ExactPixel* primary [[buffer(5)]],
    device const ExactSurface* surface [[buffer(6)]],
    device ExactColorPixel* rgb [[buffer(7)]],
    device NativeReflectionRecord* records [[buffer(8)]],
    constant uint* launch [[buffer(9)]],uint local [[thread_position_in_grid]]) {
    if(local>=launch[1])return;
    // The launch bound comes from the actual allocation, independently of all
    // scene ABI words. A malformed dimension must not permit an out-of-bounds
    // geometry read or leave an admitted pixel silently unprocessed.
    const uint size=launch[2];
    if(launch[0]>=size||local>=size-launch[0])return;
    const uint i=launch[0]+local;
    NativeReflectionRecord record={};
    if(params.width==0||params.width>4096||params.height==0||params.height>4096||
       params.width*params.height!=size||scene.width!=params.width||scene.height!=params.height) {
        record.fault=1;records[i]=record;return;
    }
    const auto output=mb3d_native_reflection::reflect_pixel(scene,packing,shadow,color,params,
        primary[i],surface[i],rgb[i],i,record);
    // The adapter validates every record before publishing any output image.
    // Failed pixels leave their previous ordinary color unchanged in the buffer.
    if(!record.fault)rgb[i]=output;
    records[i]=record;
}
