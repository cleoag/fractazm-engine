#pragma once
#include "native_camera_abi.h"
namespace mb3d_camera {
using namespace metal;
inline bool valid_camera(constant Params& p){
    if(p.version!=Version||p.optic>2||!p.full_width||!p.full_height||!p.width||!p.height||
       p.full_width>MaxExtent||p.full_height>MaxExtent||p.width>MaxExtent||p.height>MaxExtent||
       p.left< -3||p.top< -3||p.left>520192||p.top>520192||p.stride_extra>15u||(p.cut_options&~0x77u)||
       p.origin[3]!=0||p.cut[3]!=0||!isfinite(p.primary_planar_z)||!isfinite(p.paint_planar_z))return false;
    for(uint i=0;i<3;++i){if(!isfinite(p.origin[i])||!isfinite(p.cut[i])||p.matrix[i][3]!=0)return false;
        for(uint j=0;j<3;++j)if(!isfinite(p.matrix[i][j]))return false;}
    return true;
}
inline bool valid_axis(Axis a){return isfinite(a.angle)&&isfinite(a.sine)&&isfinite(a.cosine)&&
    abs(a.sine)<=1.f&&abs(a.cosine)<=1.f&&a.reserved==0;}
inline int screen_x(constant Params& p,uint x){return int(x*(p.stride_extra+1u))+p.left;}
inline int screen_y(constant Params& p,uint y){return int(y*(p.stride_extra+1u))+p.top;}
inline float3 column(constant Params& p,uint i){return float3(p.matrix[i][0],p.matrix[i][1],p.matrix[i][2]);}
inline bool normalize_checked(thread float3& v,bool primary){
    // Explicit source addition order; scaled fallback avoids a false FP32
    // overflow for finite large planar angles. No normalization for panorama.
    float n=primary?(v.x*v.x+v.z*v.z)+v.y*v.y:(v.x*v.x+v.y*v.y)+v.z*v.z;
    if(!isfinite(n)){
        float scale=max(abs(v.x),max(abs(v.y),abs(v.z)));
        if(!isfinite(scale)||scale<=0)return false;v/=scale;
        n=primary?(v.x*v.x+v.z*v.z)+v.y*v.y:(v.x*v.x+v.y*v.y)+v.z*v.z;
    }
    if(!isfinite(n)||n<=0)return false;v*=1.f/sqrt(n);return all(isfinite(v));
}
inline Ray camera_ray(constant Params& p,Axis xaxis,Axis yaxis,uint x,uint y,float max_length){
    Ray out{};out.version=Version;
    if(!valid_camera(p)||!valid_axis(xaxis)||!valid_axis(yaxis)||x>=p.width||y>=p.height||
       !isfinite(max_length)||max_length<=0){out.fault=1;return out;}
    float3 view;
    if(p.optic==1){view=float3(-xaxis.angle,yaxis.angle,p.primary_planar_z);
        if(!normalize_checked(view,false)){out.fault=4;return out;}}
    else if(p.optic==2)view=float3(-xaxis.sine*yaxis.cosine,yaxis.sine,yaxis.cosine*xaxis.cosine);
    else {view=float3(-xaxis.sine,yaxis.sine,yaxis.cosine*xaxis.cosine);
        if(!normalize_checked(view,true)){out.fault=4;return out;}}
    const float3 ray=(column(p,0)*view.x+column(p,1)*view.y)+column(p,2)*view.z;
    float3 start=float3(p.origin[0],p.origin[1],p.origin[2]);
    if(p.optic!=2)start=(column(p,0)*float(screen_x(p,x))+column(p,1)*float(screen_y(p,y)))+start;
    if(!all(isfinite(ray))||!all(isfinite(start))){out.fault=4;return out;}
    for(uint i=0;i<3;++i){out.direction[i]=ray[i];out.origin[i]=start[i];}
    // Exact CPU axis priority: strictly greater depth replaces the winner;
    // equal intersections keep the earlier X/Y/Z plane. Opposite halfspace
    // uses a finite1e20 sentinel; rays beyond MaxRayLength do no DE work.
    float depth=0;
    for(uint i=0;i<3;++i)if((p.cut_options&(1u<<i))&&abs(ray[i])>1.e-20f){
        const bool sign=ray[i]<0,cut_sign=(p.cut_options&(16u<<i))!=0;
        if(sign==cut_sign)depth=1.e20f;
        else {const float value=(p.cut[i]-start[i])/ray[i];
            if(!isfinite(value)){out.fault=4;return out;}
            if(value>depth){depth=value;out.cut_axis=i+1;}}
    }
    out.cut_depth=depth;out.cut_miss=depth>max_length;return out;
}
inline Paint paint_view(constant Params& p,Axis xaxis,Axis yaxis,uint x,uint y){
    Paint out{};
    if(!valid_camera(p)||!valid_axis(xaxis)||!valid_axis(yaxis)||x>=p.width||y>=p.height){out.fault=1;return out;}
    float3 view;
    if(p.optic==1){view=float3(-xaxis.angle,yaxis.angle,p.paint_planar_z);
        if(!normalize_checked(view,false)){out.fault=4;return out;}}
    else if(p.optic==2)view=float3(-xaxis.sine*yaxis.cosine,yaxis.sine,yaxis.cosine*xaxis.cosine);
    else {view=float3(-xaxis.sine,yaxis.sine,yaxis.cosine*xaxis.cosine);
        if(!normalize_checked(view,false)){out.fault=4;return out;}}
    for(uint i=0;i<3;++i)out.direction[i]=view[i];
    out.y_position=float(screen_y(p,y))/float(p.full_height);return out;
}
}
