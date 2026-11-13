// Ordered Native FP32 DOF contribution algebra.
// Build with floating contraction disabled. Caller gathers in exact source order,
// keeps (color, transparency) across bounded slices, and quantizes every pass.
#include <metal_stdlib>
#include "dof_abi.h"
using namespace metal;
namespace mb3d_dof {
inline void add(thread Accum& a, float3 rgb, float w) {
    a.r = a.r + rgb.x*w; a.g = a.g + rgb.y*w; a.b = a.b + rgb.z*w;
    a.transparency = a.transparency + w;
}
inline void scale(thread Accum& a, float s) {
    a.r=a.r*s; a.g=a.g*s; a.b=a.b*s; a.transparency=a.transparency*s;
}
inline void plain_contribution(thread Accum& a, float3 rgb, float source_radius,
                               float target_radius, float inverse_max_radius,
                               int dx, int dy) {
    const float extent=abs(source_radius)+1.0f;
    const float sq=extent*extent;
    float w=sq-float(dx*dx)-float(dy*dy);
    if (w<=0.0f) return;
    w=min(w,1.0f);
    const float sw=0.5f/sq;
    w=target_radius>=source_radius ? w*sw : ((w*sw)*abs(target_radius))*inverse_max_radius;
    add(a,rgb,w);
}
inline void sorted_contribution(thread Accum& a, float3 rgb, float radius,
                                bool descale, bool background, int dx, int dy) {
    const float extent=radius+1.0f;
    const int distance_squared=dx*dx+dy*dy;
    if (distance_squared>int(extent*extent)) return;
    float w=extent-sqrt(float(distance_squared));
    const float ratio=(0.8f*w)/extent;
    const float threshold=1.0f-ratio*ratio;
    const float sw=1.0f/((2.5f*radius)*radius+1.0f);
    w=w>1.0f ? sw : w*sw;
    if (descale && a.transparency>threshold) scale(a,threshold/a.transparency);
    if (!background && w+a.transparency>1.01f) {
        const float s=(1.01f-w)/a.transparency;
        a.r=a.r*s+rgb.x*w; a.g=a.g*s+rgb.y*w; a.b=a.b*s+rgb.z*w;
        a.transparency=1.01f;
    } else add(a,rgb,w);
}
inline uint quantize(thread const Accum& a) {
    const float inv=a.transparency<1.0e-10f ? 1.0e10f : 1.0f/a.transparency;
    const uint r=uint(clamp(a.r*inv,0.0f,255.0f)+0.5f);
    const uint g=uint(clamp(a.g*inv,0.0f,255.0f)+0.5f);
    const uint b=uint(clamp(a.b*inv,0.0f,255.0f)+0.5f);
    return r|(g<<8)|(b<<16);
}
} // namespace mb3d_dof
