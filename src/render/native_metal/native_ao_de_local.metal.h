#pragma once
#include "native_ao_de_math.h"
namespace mb3d_native_ao {
using namespace metal;
inline float3 de_direction(float elevation,float azimuth) {
    const float sy=sin(elevation),cy=cos(elevation),sz=sin(azimuth),cz=cos(azimuth);
    return float3(sy*cz,-sy*sz,-cy);
}
inline uint de_local_rays(constant DEParams& p,int x,int y,thread float3* rays) {
    constexpr float pi=3.14159265358979323846f;
    const float dy=p.dither?float(y%int(p.dither+1u))*.5f/float(p.dither):0.f;
    const float dx=p.dither?float(x%int(p.dither+1u))*.5f/float(p.dither):0.f;
    if(!p.quality){
        for(uint i=0;i<3;++i)rays[i]=de_direction(p.dither?(dy+.5f)*50.f*(pi/180.f):.5f*p.angular_spacing,
            (float(i)+dx)*(2.f*pi)*(1.f/3.f));
        return 3;
    }
    uint count=p.dither?uint(dy>.1f):1u;rays[0]=float3(0,0,-1);
    for(uint row=1;row<=p.quality;++row)for(uint i=0;i<p.row_counts[row];++i){
        if(count>=33)return 0;
        rays[count++]=de_direction(p.angular_spacing*(float(row)+(p.dither?dy-.25f:0.f)),
            (float(i)+dx)*(2.f*pi)/float(p.row_counts[row]));
    }
    return count;
}
inline bool de_at_cut(float3 point,float margin,constant DEParams& p) {
    for(uint axis=0;axis<3;++axis)if((p.cut_options&(1u<<axis))&&(abs(point[axis]-p.cut[axis])-margin)<0)return true;
    return false;
}
}
