#pragma once
// Include from general_geometry.metal.h after general_de. FP32 forms of the
// CPU reflection.cpp volumetric-light map (VLM_ComputeOneRay), its lookups
// (SamplePositionalVolumeLightMap, DirectionalVolumeLightMapContains) and the
// DoDynFog sub-steps (VolLightDoDynFogSubstep).
#include "volume_light_abi.h"
namespace mb3d_native {
inline float3 volume_vec(constant float* v) {return float3(v[0],v[1],v[2]);}
// side_count 0: the light is disabled, CPU computes no map and the sub-steps add nothing.
inline bool valid_volume_light(constant NativeVolumeLight& v) {
    if(v.version==kNativeVolumeLightVersion&&!v.side_count)return true;
    if(v.version!=kNativeVolumeLightVersion||v.positional>1||v.in_and_out>1||v.reserved||
       v.side_count!=(v.positional?6u:1u)||v.cube_size<3||v.cube_size>16384||!(v.cube_size&1)||
       v.half_size!=int(v.cube_size/2)||(v.inside!=0&&v.inside!=1&&v.inside!=-1)||
       v.reserved_scalar[0]!=0.f||v.reserved_scalar[1]!=0.f)return false;
    const float s[10]={v.stretch,v.size_factor,v.csize,v.hsize,v.min_distance,v.max_distance,
        v.cell_step,v.step_width,v.vl_mul,v.vl_step_mul};
    for(uint i=0;i<10;++i)if(!isfinite(s[i]))return false;
    for(uint i=0;i<4;++i)if(!isfinite(v.light[i])||!isfinite(v.origin[i])||!isfinite(v.forward[i]))return false;
    for(uint i=0;i<3;++i)for(uint j=0;j<4;++j)if(!isfinite(v.rotation[i][j])||!isfinite(v.inverse[i][j]))return false;
    return v.step_width>0.f&&v.vl_step_mul>0.f&&v.stretch>0.f&&v.max_distance>0.f;
}
// Dominant-axis cube face; the CPU throws where the original SSE would index
// outside the map, the GPU faults (4) and the render falls back.
inline float volume_positional(constant NativeVolumeLight& v,device const float* map,float3 d,thread uint& fault) {
    const float ax=abs(d.x),ay=abs(d.y),az=abs(d.z);
    const uint axis=ax>=ay?(ax>=az?0u:2u):(ay>=az?1u:2u);
    const float c=d[axis];
    if(!isfinite(ax)||!isfinite(ay)||!isfinite(az)||c==0.f){fault|=4;return 0.f;}
    const uint face=axis*2u+(c<0.f?1u:0u);
    const float scale=v.size_factor/c; // signed component on negative faces too
    const float sx=d[axis==0u?1u:0u]*scale,sy=d[axis==2u?1u:2u]*scale;
    if(!isfinite(sx)||!isfinite(sy)){fault|=4;return 0.f;}
    const int x=int(rint(sx))+v.half_size,y=int(rint(sy))+v.half_size;
    if(x<0||y<0||x>=int(v.cube_size)||y>=int(v.cube_size)){fault|=4;return 0.f;}
    return map[(face*v.cube_size+uint(y))*v.cube_size+uint(x)];
}
// VolLightMapPosSSE clamps in Single before the conversion; NaN keeps CsizeS.
inline float volume_clamp(float value,float maximum) {
    value=value<maximum?value:maximum;return 0.f<value?value:0.f;
}
inline bool volume_directional_contains(constant NativeVolumeLight& v,device const float* map,float3 p) {
    const float3 d=p-volume_vec(v.light);
    // RotateSVectorS: x*m[0][j]+y*m[1][j]+z*m[2][j].
    const float3 r=d.x*volume_vec(v.rotation[0])+d.y*volume_vec(v.rotation[1])+d.z*volume_vec(v.rotation[2]);
    const int x=int(rint(volume_clamp(r.x*v.stretch+v.hsize,v.csize)));
    const int y=int(rint(volume_clamp(r.y*v.stretch+v.hsize,v.csize)));
    return map[uint(y)*v.cube_size+uint(x)]>r.z;
}
// DoDynFog with DFogOnIt 65535: walk the step just taken (last) back from
// point in sub-steps of (1+mZZ*DEstopFactor)*VLstepmul.
inline void volume_substep(float3 point,float3 ray,float last,float depth,float stop_factor,
    thread NativeResources& resources,thread float& count,thread NativeGeometryRecord& record) {
    constant NativeVolumeLight& v=*resources.volume;
    const float3 light=volume_vec(v.light);
    const float squared_width=v.step_width*v.step_width;
    float3 p=point-ray*last;
    float s1=(1.f+depth*stop_factor)*v.vl_step_mul,remaining=last;
    if(!isfinite(s1)||s1<=0.f||!isfinite(last)){record.fault|=4;return;}
    uint steps=0;
    do {
        if(++steps>kNativeVolumeMaxSubsteps){record.fault|=2;return;}
        if(s1>remaining)s1=remaining;
        p+=ray*s1;remaining-=s1;
        if(v.positional) {
            const float3 d=p-light;
            const float d1=(d.x*d.x+d.y*d.y)+d.z*d.z;
            const float reach=volume_positional(v,resources.volume_map,d,record.fault);
            if(record.fault)return;
            if(reach*reach>d1)count+=(v.vl_mul*s1)/(d1+squared_width);
        } else if(volume_directional_contains(v,resources.volume_map,p))count+=v.vl_mul*s1;
    } while(remaining>=.01f);
}
// VLM_ComputeOneRay for cell (x,y) of a face: the map distance from the light
// (or the directional plane) to the first surface, maxDist*1000 beyond reach.
inline float volume_map_ray(constant ExactScene& scene,constant NativeFormulaProgram& program,
    constant NativeVolumeLight& v,uint side,int x,int y,thread NativeResources& resources,
    thread NativeFormulaState& lane,thread NativeGeometryRecord& record) {
    float3 vec,position;
    const float3 light=volume_vec(v.light);
    if(v.positional) {
        const float h=float(v.half_size),fx=float(x),fy=float(y);
        switch(side) {
            case 0:vec=float3(h,fx,fy);break;    case 1:vec=float3(-h,-fx,-fy);break;
            case 2:vec=float3(fx,h,fy);break;    case 3:vec=float3(-fx,-h,-fy);break;
            case 4:vec=float3(fx,fy,h);break;    default:vec=float3(-fx,-fy,-h);break;
        }
        vec*=1.f/sqrt((vec.x*vec.x+vec.y*vec.y)+vec.z*vec.z);
        position=light+vec*v.min_distance;
    } else {
        vec=volume_vec(v.inverse[2]);
        position=light+vec*v.min_distance;
        position+=volume_vec(v.inverse[0])*(float(x)*v.cell_step);
        position+=volume_vec(v.inverse[1])*(float(y)*v.cell_step);
    }
    const float3 forward=volume_vec(v.forward);
    const float base=as_type<float>(scene.de_stop),factor=as_type<float>(scene.de_stop_factor);
    const float z_div=as_type<float>(scene.z_step_div),limiter=as_type<float>(scene.step_limiter);
    const float de_sub=as_type<float>(scene.de_sub);
    float zz=length(position-volume_vec(v.origin))/v.step_width;
    float stop=base*(1.f+zz*factor);
    const float zz_mul=dot(forward,vec)/sqrt(dot(forward,forward)*dot(vec,vec));
    if(!all(isfinite(position))||!isfinite(zz)||!isfinite(zz_mul)){record.fault|=4;return 0.f;}
    auto result=general_de(position,stop,program,resources,lane,record);
    if(record.fault)return 0.f;
    float de=result.distance;
    if(v.in_and_out&&de<stop) {
        // The C++ port stores !LongBool, i.e. 1, in both inside flags.
        lane.inside=as_type<uint>(v.inside?0:1);
        result=general_de(position,stop,program,resources,lane,record);
        if(record.fault)return 0.f;de=result.distance;
    }
    const uint program_maximum=uint(program.max_iterations)+(program.de_option==20?1u:0u);
    float total=0.f,slope=1.f;
    for(uint i=0;i<kNativeVolumeMapSteps;++i) {
        const float last_de=de;
        const float step=min(max(.11f,((de-de_sub*stop)*z_div)*slope),max(stop,.4f)*limiter);
        total+=step;position+=vec*(step*v.step_width);
        zz+=step*zz_mul;stop=base*(1.f+abs(zz)*factor);
        result=general_de(position,stop,program,resources,lane,record);
        if(record.fault)return 0.f;
        de=result.distance;
        if(uint(result.state.iteration)>=general_maximum(result.maximum,program_maximum)||de<=stop)break;
        if(de>last_de+step)de=last_de+step;
        if(last_de>de+1.e-30f){const float ratio=step/(last_de-de);slope=ratio<1.f?max(.5f,ratio):1.f;}
        else slope=1.f;
        if(total>=v.max_distance){total=v.max_distance*1000.f;break;}
    }
    return total*v.step_width+v.min_distance;
}
} // namespace mb3d_native
