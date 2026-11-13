#pragma once
#include "native_camera.metal.h"
namespace mb3d_camera {
// RMcalcNanglesForCut: caller already established an initial-inside hit.
// Color/interior palette calculation remains in the ordinary surface pipeline.
inline CutSurface cut_surface_values(constant Params& p,float z_corr,float z_mul,
    uint cut_axis,float ray_depth){
    CutSurface out{};
    if(!valid_camera(p)||cut_axis>3||
       !isfinite(z_corr)||z_corr<=0||!isfinite(z_mul)||z_mul<=0||
       !isfinite(ray_depth)||ray_depth<0){out.fault=1;return out;}
    if(!cut_axis){out.depth=0x7fff0000u;out.normal[2]=-32767;return out;}
    if(!(p.cut_options&(1u<<(cut_axis-1)))){out.fault=1;return out;}
    const float radicand=ray_depth*z_corr+1.f;
    if(!isfinite(radicand)||radicand<1.f){out.fault=4;return out;}
    const float encoded=z_mul*(sqrt(radicand)-1.f);
    if(!isfinite(encoded)){out.fault=4;return out;}
    // Deliberately subtract before round-to-nearest-away. This cut branch uses
    // C++ std::round, unlike the ordinary packed-depth codec.
    const float key=8388352.f-encoded;
    if(!isfinite(key)){out.fault=4;return out;}
    out.depth=uint(round(max(0.f,key)))<<8;
    const uint cp=cut_axis-1;
    const float divisor=p.matrix[2][cp];
    // Scale the CPU (-v0/divisor,-v1/divisor,-1) by abs(divisor)
    // before normalization, avoiding a manufactured FP32 reciprocal overflow.
    // CPU's near-zero branch instead uses the finite Double factor -1e40.
    float3 n;
    if(abs(divisor)<1.e-40f)n=float3(-p.matrix[0][cp],-p.matrix[1][cp],-1.e-40f);
    else {const float sign=divisor<0.f?-1.f:1.f;
        n=float3(-p.matrix[0][cp]*sign,-p.matrix[1][cp]*sign,-abs(divisor));}
    const float scale=max(abs(n.x),max(abs(n.y),abs(n.z)));
    if(!isfinite(scale)){out.fault=4;return out;}
    if(scale==0.f){out.normal[2]=-32767;return out;}
    n/=scale;const float norm=(n.x*n.x+n.y*n.y)+n.z*n.z;
    if(!isfinite(norm)||norm<=0.f){out.fault=4;return out;}
    n*=32767.f/sqrt(norm);
    if(!all(isfinite(n))){out.fault=4;return out;}
    for(uint i=0;i<3;++i)out.normal[i]=int(rint(n[i]));
    return out;
}
inline CutSurface cut_surface(constant Params& p,constant DepthPacking& depth,
    uint cut_axis,float ray_depth){
    if(depth.reserved[0]||depth.reserved[1]){CutSurface out{};out.fault=1;return out;}
    return cut_surface_values(p,depth.z_corr,depth.z_mul,cut_axis,ray_depth);
}
}
