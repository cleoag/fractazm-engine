// Native FP32 neighborhood pass. Compile separately with contraction disabled.
// CPU source: core/zbuffer_normals.cpp; no CPU per-pixel preparation.
#include <metal_stdlib>
#include "zbuffer_abi.h"
#include "comparison/metal_exact/shadow_abi.h"
using namespace metal;
using namespace mb3d_zn;
inline void fault(device atomic_uint& status,uint value){atomic_fetch_or_explicit(&status,value,memory_order_relaxed);}
inline bool valid_surface(ExactSurface pixel){
    return !pixel.fault&&!pixel.reserved[0]&&!pixel.reserved[1]&&!pixel.reserved[2]&&pixel.si_gradient<=65535&&
       pixel.o_trap<=65535&&pixel.shadow<=65535&&pixel.amb_shadow<=65535&&
       pixel.normal[0]>=-32768&&pixel.normal[0]<=32767&&pixel.normal[1]>=-32768&&pixel.normal[1]<=32767&&
       pixel.normal[2]>=-32768&&pixel.normal[2]<=32767;
}
inline bool valid(constant Params& p,constant Launch& l) {
    return p.version==Version&&p.width&&p.height&&p.width<=MaxExtent&&p.height<=MaxExtent&&
        p.full_width&&p.full_height&&p.full_width<=MaxExtent&&p.full_height<=MaxExtent&&p.mode<=3&&
        p.left>=-4096&&p.left<=4096&&p.top>=-4096&&p.top<=4096&&p.count==p.width*p.height&&
        l.count==p.count&&l.source_count>=p.count&&l.coordinate_count>=p.count&&l.output_count>=p.count&&
        l.record_count>=p.count&&!l.reserved[0]&&!l.reserved[1]&&!l.reserved[2]&&
        !p.reserved[0]&&!p.reserved[1]&&!p.reserved[2]&&p.reserved_float==0&&
        isfinite(p.step)&&p.step>0&&isfinite(p.fov)&&p.fov>=0&&isfinite(p.aspect)&&p.aspect>0&&
        isfinite(p.optic_z)&&p.optic_z>0&&isfinite(p.inverse_z_mul)&&p.inverse_z_mul>0&&
        isfinite(p.depth_correction)&&p.depth_correction>0&&isfinite(p.stop_factor);
}
kernel void native_zn_abi(device uint* out [[buffer(0)]],uint i [[thread_position_in_grid]]){
    if(i)return;out[0]=Version;out[1]=sizeof(Params);out[2]=sizeof(Coordinate);out[3]=sizeof(Record);out[4]=sizeof(Launch);
}
kernel void native_zn_reconstruct(constant Params& p [[buffer(0)]],constant Launch& l [[buffer(1)]],
    device const ExactSurface* source [[buffer(2)]],device Coordinate* positions [[buffer(3)]],
    device atomic_uint& status [[buffer(4)]],uint i [[thread_position_in_grid]]) {
    if(i>=l.count)return;if(!valid(p,l)){fault(status,1);return;}
    Coordinate c{};c.x=-1e20f;c.index=i;const auto pixel=source[i];
    if(!valid_surface(pixel)){
        c.fault=2;positions[i]=c;fault(status,2);return;}
    if((pixel.depth_packed>>16)>=32768||pixel.si_gradient>=32768){positions[i]=c;return;}
    const uint x=i%p.width,y=i/p.width;
    const float xpos=float(int(x)+1+p.left)/float(p.full_width),ypos=float(int(y)+p.top)/float(p.full_height);
    const float cx=((.5f-xpos)*p.fov)*p.aspect,cy=(ypos-.5f)*p.fov;
    float3 view;
    if(p.mode==1)view=float3(-cx,cy,p.optic_z);
    else if(p.mode==2)view=float3(-sin(cx)*cos(cy),sin(cy),cos(cx)*cos(cy));
    else view=float3(-sin(cx),sin(cy),cos(cx)*cos(cy));
    if(p.mode!=2){const float length=dot(view,view);
        if(!isfinite(length)){c.fault=4;positions[i]=c;fault(status,4);return;}
        if(length>1e-30f)view/=sqrt(length);}
    const float encoded=float(8388352-int(pixel.depth_packed>>8))*p.inverse_z_mul+1.f;
    c.depth=(encoded*encoded-1.f)*p.depth_correction;
    const float originX=p.mode==2?0.f:float(int(x)+p.left)*p.step;
    // Original initial two rows omit the tile top in their Y origin.
    const float originY=p.mode==2?0.f:float(int(y)+1+(y<2?0:p.top))*p.step;
    const float3 position=view*c.depth+float3(originX,originY,0);
    c.x=position.x;c.y=position.y;c.z=position.z;c.foreground=uint(position.x> -1e19f);
    if(!all(isfinite(view))||!all(isfinite(position))||!isfinite(c.depth)){c.fault=4;fault(status,4);}
    positions[i]=c;
}
inline float3 xyz(Coordinate c){return float3(c.x,c.y,c.z);}
inline bool valid_coordinate(Coordinate c,uint index){
    return c.index==index&&!c.reserved&&!c.fault&&c.foreground<=1&&
        c.foreground==uint(c.x> -1e19f)&&all(isfinite(xyz(c)))&&isfinite(c.depth);
}
inline float3 tangent(Coordinate before,Coordinate after,Coordinate center,float3 fallback,float distance) {
    if(before.x< -1e19f)return after.x> -1e19f?xyz(center)-xyz(after):fallback;
    if(!(after.x> -1e19f))return xyz(before)-xyz(center);
    const float3 a=xyz(before)-xyz(center),b=xyz(center)-xyz(after);
    const float d=distance*distance,aa=dot(a,a),bb=dot(b,b),da=aa+d,db=bb+d;
    if(!all(isfinite(a))||!all(isfinite(b))||!isfinite(d)||!isfinite(aa)||!isfinite(bb)||
       !isfinite(da)||!isfinite(db)||da<=0||db<=0)return float3(as_type<float>(0x7fc00000u));
    float wa=1.f/da,wb=1.f/db;const float sum=wa+wb,inv=1.f/sum;
    if(!isfinite(wa)||!isfinite(wb)||!isfinite(sum)||sum<=0||!isfinite(inv))return float3(as_type<float>(0x7fc00000u));
    wa*=inv;wb*=inv;return a*wa+b*wb;
}
kernel void native_zn_normals(constant Params& p [[buffer(0)]],constant Launch& l [[buffer(1)]],
    device const ExactSurface* source [[buffer(2)]],device const Coordinate* positions [[buffer(3)]],
    device ExactSurface* output [[buffer(4)]],device Record* records [[buffer(5)]],
    device atomic_uint& status [[buffer(6)]],uint i [[thread_position_in_grid]]) {
    if(i>=l.count)return;if(!valid(p,l)){fault(status,1);return;}
    Record r{};r.version=Version;r.index=i;ExactSurface pixel=source[i];const auto center=positions[i];
    if(!valid_coordinate(center,i)||!valid_surface(pixel)||
       (center.foreground&&((pixel.depth_packed>>16)>=32768||pixel.si_gradient>=32768))){
        r.fault=8;records[i]=r;fault(status,8);return;}
    if(center.foreground){
        const uint x=i%p.width,y=i/p.width;Coordinate up{},down{},left{},right{};
        up.x=down.x=left.x=right.x=-1e20f;
        if(y){up=positions[i-p.width];++r.neighbors;}
        if(y+1<p.height){down=positions[i+p.width];++r.neighbors;}
        if(x){left=positions[i-1];++r.neighbors;}
        if(x+1<p.width){right=positions[i+1];++r.neighbors;}
        if((y&&!valid_coordinate(up,i-p.width))||(y+1<p.height&&!valid_coordinate(down,i+p.width))||
           (x&&!valid_coordinate(left,i-1))||(x+1<p.width&&!valid_coordinate(right,i+1))){
            r.fault=8;records[i]=r;fault(status,8);return;}
        const float distance=p.step+center.depth*p.stop_factor;
        const float3 ty=tangent(up,down,center,float3(0,1,0),distance);
        const float3 tx=tangent(left,right,center,float3(1,0,0),distance);
        float3 normal=cross(ty,tx);const float magnitude=max(abs(normal.x),max(abs(normal.y),abs(normal.z)));
        if(!isfinite(distance)||!all(isfinite(ty))||!all(isfinite(tx))||!all(isfinite(normal))){r.fault=16;records[i]=r;fault(status,16);return;}
        if(magnitude>0){normal/=magnitude;normal*=32767.f/sqrt(dot(normal,normal));}
        const int3 packed=int3(rint(normal));r.foreground=1;
        for(uint k=0;k<3;++k){r.changed|=uint(pixel.normal[k]!=packed[k]);pixel.normal[k]=packed[k];}
    }
    output[i]=pixel;records[i]=r;
}
