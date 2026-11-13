#pragma once
// Static FP32 spline implementation of the ordinary lightmap contract.
// Buffers contain raw interleaved RGB values, including the duplicated seam.
namespace mb3d_native {
inline float3 native_map_failure(thread NativeFormulaFault& f,uint code){if(!f.code)f.code=code;return float3(NAN);}
inline float4 native_spline_coeff(float x){
    float c3=((x*x)*x)*(1.0f/6.0f);
    float c0=(1.0f/6.0f)-((1.0f-x)*x)*0.5f-c3;
    float c2=((x+c0)-c3)-c3;
    return float4(c0,((1.0f-c2)-c3)-c0,c2,c3);
}
inline int native_map_index(int id,thread const NativeResources& r){
    if(r.map_count>256)return -1;
    for(uint i=0;i<r.map_count;++i)if(r.maps[i].map_id==id)return int(i);
    return -1;
}
inline float3 native_spline_map(float2 uv,device const NativeMapDescriptor& m,
    thread const NativeResources& r,thread NativeFormulaFault& f){
    if(m.flags==FormulaMapEmpty && m.map_id>=0 && !m.width && !m.height &&
        !m.element_count && m.channels==3 && !m.offset && !m.reserved)return float3(0);
    if(m.reserved||m.flags>1||m.channels!=3||!m.width||!m.height||m.width>65535||m.height>65535)
        return native_map_failure(f,FormulaResourceBounds);
    ulong required=ulong(m.width+1)*ulong(m.height+1)*3;
    if(required!=m.element_count||ulong(m.offset)+required>r.pixel_count)
        return native_map_failure(f,FormulaResourceBounds);
    if(!all(isfinite(uv))||any(uv<0)||uv.x>float(m.width)||uv.y>float(m.height))
        return native_map_failure(f,NonFiniteFormula);
    int x=int(uv.x),y=int(uv.y),w=int(m.width),h=int(m.height);
    float4 wx=native_spline_coeff(uv.x-float(x)),wy=native_spline_coeff(uv.y-float(y));
    y=y>0?y-1:y+h-1;
    int columns[4]={x+(x>0?-1:w-1),x,x+(x<w?1:1-w),x+(x<w-1?2:2-w)};
    float3 rows[4];float scale=m.flags&1?1.0f/65535.0f:1.0f/255.0f;
    for(uint row=0;row<4;++row){
        int yy=(y<0||y>=h)?0:y;float3 sum[4];
        for(uint k=0;k<4;++k){
            if(columns[k]<0||columns[k]>w)return native_map_failure(f,FormulaResourceBounds);
            ulong index=ulong(m.offset)+(ulong(yy)*(w+1)+uint(columns[k]))*3;
            float3 pixel(r.pixels[index],r.pixels[index+1],r.pixels[index+2]);
            if(!all(isfinite(pixel)))return native_map_failure(f,NonFiniteFormula);
            sum[k]=pixel*(wx[k]*scale);
        }
        rows[row]=(sum[0]+sum[2])+(sum[1]+sum[3]);
        ++y;if(y>=h)y=0;
    }
    float3 result=((rows[0]*wy[0]+rows[1]*wy[1])+rows[2]*wy[2])+rows[3]*wy[3];
    if(!all(isfinite(result)))return native_map_failure(f,NonFiniteFormula);
    return result;
}
inline float native_map_fraca(float x){return x<0?1.0f+(x-trunc(x)):x-trunc(x);}
inline float3 native_sample_direct(float3 uvw,int id,thread const NativeResources& r,thread NativeFormulaFault& f){
    if(!all(isfinite(uvw)))return native_map_failure(f,NonFiniteFormula);
    int index=native_map_index(id,r);if(index<0)return native_map_failure(f,MissingFormulaResource);
    device const auto& m=r.maps[index];
    return native_spline_map(float2(native_map_fraca(uvw.x)*m.width,native_map_fraca(uvw.y)*m.height),m,r,f);
}
inline float3 native_sample_sphere(float3 xyz,int id,thread const NativeResources& r,thread NativeFormulaFault& f){
    if(!all(isfinite(xyz)))return native_map_failure(f,NonFiniteFormula);
    int index=native_map_index(id,r);if(index<0)return native_map_failure(f,MissingFormulaResource);
    device const auto& m=r.maps[index];
    // Source adds a tiny positive term before reciprocal sqrt. Preserve its
    // defined zero-vector result without FP32 0*infinity or overflow.
    float magnitude=max(abs(xyz.x),max(abs(xyz.y),abs(xyz.z)));
    float3 v=magnitude==0?float3(0):xyz/magnitude;
    if(magnitude!=0)v/=sqrt((v.x*v.x+v.y*v.y)+v.z*v.z);
    // MPi05d is -0.5/pi: longitude runs against atan2, as in the CPU sampler.
    float u=atan2(v.x,v.y)*(-0.5f/M_PI)+0.5f;
    float vv=0.5f-asin(clamp(v.z,-1.0f,1.0f))*(1.0f/M_PI);
    return native_spline_map(clamp(float2(u*m.width,vv*m.height),float2(0),float2(m.width,m.height)),m,r,f);
}
inline FormulaVec3 native_formula_map_direct(FormulaParams p,float u,float v,int id){return native_sample_direct(float3(u,v,0),id,*p.resources,*p.fault);}
inline FormulaVec3 native_formula_map_sphere(FormulaParams p,float x,float y,float z,int id){return native_sample_sphere(float3(x,y,z),id,*p.resources,*p.fault);}
using TVec3D=FormulaVec3;
inline void native_formula_map(FormulaParams p,thread TVec3D* out,thread const TVec3D* in,int id,bool sphere){
    *out=sphere?native_sample_sphere(float3(*in),id,*p.resources,*p.fault):native_sample_direct(float3(*in),id,*p.resources,*p.fault);
}
} // namespace mb3d_native

#ifndef MB3D_NATIVE_GENERAL_HELPERS_ONLY
// Resource contract v1: 32-byte descriptors, RGB channels, canonical-empty
// validation revision 1. Separate from the unchanged program/state wire ABI.
kernel void native_formula_map_abi(device uint* out [[buffer(0)]], uint tid [[thread_position_in_grid]]) {
    if(tid==0) {out[0]=1;out[1]=sizeof(mb3d_native::NativeMapDescriptor);out[2]=3;out[3]=1;}
}
#endif
