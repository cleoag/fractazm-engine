#pragma once
#include <metal_stdlib>
namespace mb3d_native_riemann {
using namespace metal;
enum Status:uint {Preserved=0,Recovered=1,Rejected=2};
struct Result {float3 value;Status status;};
// Native-only stable reconstruction. The caller retains its input projection,
// angle work/range guards, and original result calculation. No FP64 or x87.
inline Result reconstruct(float3 original,float angleA,float angleB,float radius,float3 offset,bool finiteProjection) {
    if(all(isfinite(original)))return {original,Preserved};
    if(!finiteProjection||!isfinite(angleA)||!isfinite(angleB)||!isfinite(radius)||radius<0.f||
       !all(isfinite(offset))||abs(angleA)>=0x1p63f||abs(angleB)>=0x1p63f)
        return {original,Rejected};
    const float sa=sin(.5f*angleA),ca=cos(.5f*angleA);
    const float sb=sin(.5f*angleB),cb=cos(.5f*angleB);
    // Multiply the stereographic numerator/denominator by ca²*cb² before
    // division. This avoids sin(angle)/(cos(angle)+1) at rounded cos=-1.
    const float ca2=ca*ca,cb2=cb*cb;
    const float u=(sa*sa)*cb2,v=(sb*sb)*ca2,w=ca2*cb2;
    const float denominator=(u+v)+w;
    if(!(denominator>0.f)||!isfinite(denominator))return {original,Rejected};
    const float3 direction=float3(((2.f*sa)*ca)*cb2,(u+v)-w,((2.f*sb)*cb)*ca2)/denominator;
    const float3 result=direction*radius+offset;
    if(!all(isfinite(result)))return {original,Rejected};
    return {result,Recovered};
}
}
