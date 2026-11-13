#pragma once
#include "general_geometry.metal.h"
#include "generic_shadow_abi.h"

namespace mb3d_native {
using GenericShadowRow=NativeShadowRowState;
inline float generic_shadow_stop(float z,constant ExactScene& s){return as_type<float>(s.de_stop)*(1.f+z*as_type<float>(s.de_stop_factor));}
inline float generic_shadow_depth(uint packed,constant ExactShadowParams& p,float offset){
    float q=(offset-float(packed>>8))/general_double(p.z_c_mul)+1.f;
    return (q*q-1.f)/general_double(p.z_corr);
}
// positional: the entry implements calcHShard/CalcHShardPoint position lights.
inline bool valid_generic_shadow(constant ExactScene& s,constant NativeFormulaProgram& program,constant ExactShadowParams& p,
    bool multi=false,bool positional=false){
    if(!valid_general_scene(s,program)||p.version!=1||p.enabled>1||p.light_mask>(multi?63u:1u)||
       (p.enabled&&!p.light_mask)||!p.max_steps||p.max_steps>65536||
       p.max_iterations_result!=uint(program.max_iterations)+(program.de_option==20?1u:0u)||
       (p.position_mask&&!positional)||(p.position_mask&~p.light_mask)||p.reserved[0]||p.reserved[1])return false;
    if(p.position_mask&&(!isfinite(general_double(p.step_width))||general_double(p.step_width)<=0.f))return false;
    for(uint i=0;i<6;++i){
        const bool position=p.position_mask&(1u<<i);
        if(!position&&(p.light_reach[i]||p.light_positions[3*i]||p.light_positions[3*i+1]||p.light_positions[3*i+2]))return false;
        if(position&&(!isfinite(as_type<float>(p.light_reach[i]))||as_type<float>(p.light_reach[i])<0.f||
           !isfinite(general_double(p.light_positions[3*i]))||!isfinite(general_double(p.light_positions[3*i+1]))||
           !isfinite(general_double(p.light_positions[3*i+2]))))return false;
    }
    if(!p.position_mask&&p.step_width)return false;
    if(!isfinite(general_double(p.z_corr))||general_double(p.z_corr)<=0.f||
       !isfinite(general_double(p.z_c_mul))||general_double(p.z_c_mul)<=0.f||
       !isfinite(general_double(p.fov_y))||general_double(p.fov_y)<0.f||
       !isfinite(general_double(p.length_multiplier))||general_double(p.length_multiplier)<=0.f)return false;
    for(uint i=0;i<18;++i)if(!isfinite(general_double(p.light_vectors[i])))return false;
    for(uint i=0;i<6;++i)if(p.enabled&&(p.light_mask&~p.position_mask&(1u<<i))&&
        general_double(p.light_vectors[3*i])==0.f&&general_double(p.light_vectors[3*i+1])==0.f&&general_double(p.light_vectors[3*i+2])==0.f)return false;
    return true;
}
inline float3 generic_shadow_normal(ExactSurface surface,constant ExactScene& s){
    float3 n(float(surface.normal[0]),float(surface.normal[1]),float(surface.normal[2]));
    if(all(n==float3(0)))n=float3(0,0,-1);else n/=sqrt((n.x*n.x+n.y*n.y)+n.z*n.z);
    return (general_column(s,0)*n.x+general_column(s,1)*n.y)+general_column(s,2)*n.z;
}
inline bool generic_shadow_refine(thread float3& point,float3 ray,thread float& depth,thread float& stop,
    thread NativeOrbitResult& value,uint packed,constant ExactScene& scene,constant ExactShadowParams& p,
    constant NativeFormulaProgram& program,thread NativeResources& resources,thread NativeFormulaState& lane,
    thread NativeGeometryRecord& record){
    if(uint(value.state.iteration)>=native_shadow_maximum(program,value,p.max_iterations_result)&&!(value.distance<stop)){
        uint iteration=uint(value.state.iteration);
        return general_refine_iteration_count(point,ray,depth,stop,scene,program,resources,lane,record,iteration,
            kGeneralShadowRefinementSteps);
    }
    // RMdoBinSearch moves mct.mZZ but the hard-shadow caller's local mZZ
    // intentionally retains its reconstructed depth in this DE-limited branch.
    float step=(generic_shadow_depth(packed,p,8388351.9f)-depth)*-.5f;
    float refined_depth=depth;
    for(uint remaining=kGeneralShadowRefinementSteps;remaining>0&&abs(value.distance-stop)>.001f;--remaining){
        refined_depth+=step;point+=ray*step;stop=generic_shadow_stop(refined_depth,scene);++record.refine_calls;
        if(!all(isfinite(point))||!isfinite(stop)||stop<=0.f){record.fault=4;return false;}
        if(remaining<=1)break;
        value=general_de(point,stop,program,resources,lane,record);
        if(record.fault)return false;
        step=abs(step)*(value.distance<stop?-.55f:.55f);
    }
    return true;
}
inline ExactSurface generic_shadow_pixel(ExactPixel geometry,ExactSurface surface,uint y,
    constant ExactScene& scene,constant NativeFormulaProgram& program,constant ExactShadowParams& p,
    thread NativeResources& resources,thread NativeFormulaState& lane,thread GenericShadowRow& row,
    thread NativeShadowRecord& record){
    record.version=kGenericShadowVersion;record.eval.version=kGeneralGeometryVersion;
    if(geometry.fault||surface.fault){record.fault=surface.fault|geometry.fault|1u;surface.fault=record.fault;return surface;}
    if(!p.enabled)return surface;
    if(row.threshold>0)--row.threshold;
    uint original_shadow=surface.shadow;surface.shadow&=~(p.light_mask<<10);
    if((surface.depth_packed>>16)>=32768||surface.si_gradient>=32768){record.changed=uint(surface.shadow!=original_shadow);return surface;}
    record.active=1;
    float3 ray(general_double(geometry.ray[0]),general_double(geometry.ray[1]),general_double(geometry.ray[2]));
    float3 start(general_double(geometry.start[0]),general_double(geometry.start[1]),general_double(geometry.start[2]));
    float z=generic_shadow_depth(surface.depth_packed,p,8388351.5f),stop=generic_shadow_stop(z,scene);
    float3 point=start+ray*z;
    if(!isfinite(z)||!isfinite(stop)||stop<=0.f||!all(isfinite(ray))||!all(isfinite(point))){record.fault=surface.fault=4;return surface;}
    auto value=general_de(point,stop,program,resources,lane,record.eval);
    if(record.eval.fault){record.fault=surface.fault=record.eval.fault;return surface;}
    if(!generic_shadow_refine(point,ray,z,stop,value,surface.depth_packed,scene,p,program,resources,lane,record.eval)){
        record.fault=surface.fault=record.eval.fault;return surface;
    }
    z-=.1f;point-=ray*.1f;
    float length=float(scene.width+y)*.6f*(1.f+(.5f*min(z,general_double(scene.max_ray_length)*.4f)*general_double(p.fov_y))/float(scene.height))*general_double(p.length_multiplier);
    float threshold=(1.f+(z+length)*as_type<float>(scene.de_stop_factor))*3.3f/as_type<float>(scene.z_step_div);
    float3 direction(general_double(p.light_vectors[0]),general_double(p.light_vectors[1]),general_double(p.light_vectors[2]));
    float3 normal=generic_shadow_normal(surface,scene);
    float dscale=max(abs(direction.x),max(abs(direction.y),abs(direction.z)));
    float rscale=max(abs(ray.x),max(abs(ray.y),abs(ray.z)));
    if(dscale<=0.f||rscale<=0.f||!isfinite(length)||!isfinite(threshold)||threshold<=0.f||!all(isfinite(normal))){record.fault=surface.fault=4;return surface;}
    float3 dn=direction/dscale,rn=ray/rscale;
    float zmul=-dot(dn,rn)/sqrt(dot(dn,dn)*dot(rn,rn));
    if(!isfinite(zmul)){record.fault=surface.fault=4;return surface;}
    bool skip=row.threshold>0;
    if(skip){
        float difference=abs(row.last_z-z);
        float decrement=rint(difference*3.3f/threshold);
        if(!isfinite(decrement)||decrement<0.f){record.fault=surface.fault=4;return surface;}
        // The source's counter is at most3. Test before integer conversion so
        // large finite depth jumps cannot overflow a signed device integer.
        row.threshold=decrement>=float(row.threshold)?0:row.threshold-int(decrement);
        if(row.threshold<=0){skip=false;row.threshold=0;}
        else {row.last_found=(row.last_found-difference)-(1.f+z*as_type<float>(scene.de_stop_factor));
            if(row.last_found<=0.f){skip=false;row.threshold=0;}}
    }
    if(dot(normal,direction)>0.f){
        surface.shadow|=1u<<10;row.threshold=0;record.backface=1;
    }else{
        float remaining=length,zz=z,factor=2.f;
        if(remaining>0.f){
            record.rays=1;stop=generic_shadow_stop(abs(zz),scene);
            ++record.shadow_de_calls;value=general_de(point,stop,program,resources,lane,record.eval);
            bool ended=false;
            for(uint k=0;k<p.max_steps&&!record.eval.fault;++k){
                if(skip&&row.last_found>remaining){remaining=-1.f;record.skipped=1;ended=true;break;}
                float previous=value.distance;
                float width=max(.11f,((value.distance-as_type<float>(scene.de_sub)*stop)*as_type<float>(scene.z_step_div))*factor);
                width=min(width,max(stop,.4f)*as_type<float>(scene.step_limiter));
                if(!isfinite(width)||width<=0.f){record.eval.fault=4;break;}
                remaining-=width;point-=direction*width;zz+=width*zmul;stop=generic_shadow_stop(abs(zz),scene);
                ++record.steps;++record.shadow_de_calls;value=general_de(point,stop,program,resources,lane,record.eval);
                if(record.eval.fault)break;
                if(!isfinite(remaining)||!isfinite(stop)||stop<=0.f||!all(isfinite(point))){record.eval.fault=4;break;}
                if(uint(value.state.iteration)>=native_shadow_maximum(program,value,p.max_iterations_result)||value.distance<=stop){ended=true;break;}
                if(value.distance>previous+width)value.distance=previous+width;
                if(!skip){
                    if(value.distance>threshold){if(row.threshold<3){row.last_found=remaining;row.threshold=3;}}
                    else row.threshold=0;
                }
                if(previous>value.distance+1.e-30f){float ratio=width/(previous-value.distance);factor=ratio<1.f?max(.5f,ratio):1.f;}
                else factor=1.f;
                if(remaining<0.f){ended=true;break;}
            }
            if(!ended&&!record.eval.fault)record.eval.fault=2;
            if(record.eval.fault){record.fault=surface.fault=record.eval.fault;return surface;}
            record.hits=uint(remaining>0.f);record.misses=1u-record.hits;
        }
        if(remaining>0.f){surface.shadow|=1u<<10;row.threshold=0;}
    }
    row.last_z=z;record.changed=uint(surface.shadow!=original_shadow);return surface;
}
} // namespace mb3d_native

kernel void native_general_shadow_abi(device uint* words [[buffer(0)]],uint tid [[thread_position_in_grid]]){
    if(tid)return;words[0]=mb3d_native::kGenericShadowVersion;words[1]=sizeof(mb3d_native::NativeShadowRecord);words[2]=32;words[3]=1;
}
kernel void native_general_hard_shadow(device const ExactPixel* primary [[buffer(0)]],
    device const ExactSurface* input [[buffer(1)]],device ExactSurface* output [[buffer(2)]],
    device mb3d_native::NativeShadowRecord* records [[buffer(3)]],constant ExactScene& scene [[buffer(4)]],
    constant mb3d_native::NativeFormulaProgram& program [[buffer(5)]],constant ExactShadowParams& params [[buffer(6)]],
    device const mb3d_native::NativeMapDescriptor* maps [[buffer(7)]],device const float* pixels [[buffer(8)]],
    constant uint4& launch [[buffer(9)]],constant uint2& resource_counts [[buffer(10)]],uint local [[thread_position_in_grid]]){
    if(local>=launch.y||launch.x>=scene.height||local>=scene.height-launch.x||!scene.width||scene.width>4096)return;
    uint y=launch.x+local;ulong base=ulong(y)*scene.width;if(base>=launch.z)return;
    bool valid=!launch.w&&ulong(scene.width)*scene.height==launch.z&&mb3d_native::valid_generic_shadow(scene,program,params);
    mb3d_native::NativeResources resources{maps,pixels,resource_counts.x,resource_counts.y,nullptr};
    auto lane=mb3d_native::native_initial_state(float3(0),as_type<float>(scene.de_stop),program);
    mb3d_native::GenericShadowRow row{};
    for(uint x=0;x<scene.width&&base+x<launch.z;++x){
        uint index=uint(base)+x;auto surface=input[index];mb3d_native::NativeShadowRecord record{};
        record.version=mb3d_native::kGenericShadowVersion;record.eval.version=mb3d_native::kGeneralGeometryVersion;
        if(valid)surface=mb3d_native::generic_shadow_pixel(primary[index],surface,y,scene,program,params,resources,lane,row,record);
        else record.fault=surface.fault=1;
        output[index]=surface;records[index]=record;
    }
}

kernel void native_general_shadow_lane_abi(device uint* words [[buffer(0)]],uint tid [[thread_position_in_grid]]){
    if(tid)return;words[0]=1;words[1]=sizeof(mb3d_native::NativeLogicalLane);
    words[2]=sizeof(mb3d_native::NativeFormulaState);words[3]=sizeof(mb3d_native::NativeShadowRowState);
    words[4]=sizeof(mb3d_native::NativeLogicalLaneRecord);words[5]=mb3d_native::kLogicalLaneBatchPixels;
}
kernel void native_general_hard_shadow_lanes(device const ExactPixel* primary [[buffer(0)]],
    device const ExactSurface* input [[buffer(1)]],device ExactSurface* output [[buffer(2)]],
    device mb3d_native::NativeShadowRecord* records [[buffer(3)]],constant ExactScene& scene [[buffer(4)]],
    constant mb3d_native::NativeFormulaProgram& program [[buffer(5)]],constant ExactShadowParams& params [[buffer(6)]],
    device const mb3d_native::NativeMapDescriptor* maps [[buffer(7)]],device const float* pixels [[buffer(8)]],
    constant uint4& launch [[buffer(9)]],constant uint2& resource_counts [[buffer(10)]],
    device const mb3d_native::NativeLogicalLane* descriptors [[buffer(11)]],
    device mb3d_native::NativeFormulaState* states [[buffer(12)]],
    device mb3d_native::NativeShadowRowState* caches [[buffer(13)]],
    device mb3d_native::NativeLogicalLaneRecord* lane_records [[buffer(14)]],
    constant uint& batch_rows [[buffer(15)]],uint local [[thread_position_in_grid]]){
    if(local>=launch.y||launch.x>=launch.z||local>=launch.z-launch.x)return;
    const uint id=launch.x+local;const auto descriptor=descriptors[id];
    mb3d_native::NativeLogicalLaneRecord status{1,0,0,descriptor.cursor};
    // Contiguous CPU shadow tasks own disjoint rows. Validate this mapping,
    // independently of each uploaded descriptor, before writing image memory.
    if(!batch_rows||batch_rows>scene.height||launch.z!=(scene.height+batch_rows-1)/batch_rows||
       !mb3d_native::valid_logical_lane(descriptor,scene.width,scene.height,launch.w)||
       descriptor.row_start!=id*batch_rows||descriptor.row_stride!=1||
       descriptor.row_count!=metal::min(batch_rows,scene.height-descriptor.row_start)||
       !mb3d_native::valid_generic_shadow(scene,program,params)){
        status.fault=1;lane_records[id]=status;return;
    }
    if(!descriptor.pixel_count){lane_records[id]=status;return;}
    auto lane=descriptor.cursor?states[id]:mb3d_native::native_initial_state(float3(0),as_type<float>(scene.de_stop),program);
    mb3d_native::NativeShadowRowState row=descriptor.cursor?caches[id]:mb3d_native::NativeShadowRowState{};
    if(row.reserved||row.threshold<0||row.threshold>3||!metal::isfinite(row.last_found)||!metal::isfinite(row.last_z)){
        status.fault=1;lane_records[id]=status;return;
    }
    mb3d_native::NativeResources resources{maps,pixels,resource_counts.x,resource_counts.y,nullptr};
    for(uint k=0;k<descriptor.pixel_count;++k){
        const uint ordinal=descriptor.cursor+k,x=ordinal%scene.width;
        const uint y=descriptor.row_start+ordinal/scene.width,index=y*scene.width+x;
        if(x==0)row={};
        auto surface=input[index];mb3d_native::NativeShadowRecord record{};
        surface=mb3d_native::generic_shadow_pixel(primary[index],surface,y,scene,program,params,resources,lane,row,record);
        output[index]=surface;records[index]=record;++status.processed;++status.next_cursor;
        if(record.fault){status.fault=record.fault;break;}
    }
    states[id]=lane;caches[id]=row;lane_records[id]=status;
}
