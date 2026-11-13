#pragma once
#include "generic_shadow.metal.h"
#include "shadow_inside_bridge.metal.h"
#include "secondary_cut_candidate.metal.h"
namespace mb3d_native {
inline ExactSurface camera_inside_shadow_pixel(ExactPixel geometry,ExactSurface surface,uint y,
    constant ExactScene& scene,constant NativeFormulaProgram& program,constant ExactShadowParams& p,
    thread NativeResources& resources,thread NativeFormulaState& lane,thread GenericShadowRow& row,
    thread NativeShadowRecord& record,constant mb3d_camera::Params& camera,
    constant mb3d_inside::Params& inside,thread NativeShadowSpeculation& spec){
    record.version=kGenericShadowVersion;record.eval.version=kGeneralGeometryVersion;
    if(!mb3d_camera::valid_camera(camera)||camera.width!=scene.width||camera.height!=scene.height||
       !mb3d_inside::valid(inside)||inside.full_width!=camera.full_width){record.fault=surface.fault=1;return surface;}
    if(geometry.fault||surface.fault){record.fault=surface.fault|geometry.fault|1u;surface.fault=record.fault;return surface;}
    if(!p.enabled)return surface;
    if(row.threshold>0)--row.threshold;
    spec.kind=kShadowCounterOnly;
    uint original_shadow=surface.shadow;surface.shadow&=~(p.light_mask<<10);
    if((surface.depth_packed>>16)>=32768||surface.si_gradient>=32768){record.changed=uint(surface.shadow!=original_shadow);return surface;}
    record.active=1;
    float3 ray(general_double(geometry.ray[0]),general_double(geometry.ray[1]),general_double(geometry.ray[2]));
    float3 start(general_double(geometry.start[0]),general_double(geometry.start[1]),general_double(geometry.start[2]));
    float z=generic_shadow_depth(surface.depth_packed,p,8388351.5f),stop=generic_shadow_stop(z,scene);
    float3 point=start+ray*z;
    if(!isfinite(z)||!isfinite(stop)||stop<=0.f||!all(isfinite(ray))||!all(isfinite(point))){record.fault=surface.fault=4;return surface;}
    auto value=general_shadow_inside_start(point,stop,program,inside,surface.o_trap,resources,lane,record.eval);
    if(record.eval.fault){record.fault=surface.fault=record.eval.fault;return surface;}
    if(!generic_shadow_refine(point,ray,z,stop,value,surface.depth_packed,scene,p,program,resources,lane,record.eval)){
        record.fault=surface.fault=record.eval.fault;return surface;
    }
    z-=.1f;point-=ray*.1f;
    float length=float(int(camera.full_width)+mb3d_camera::screen_y(camera,y))*.6f*(1.f+(.5f*min(z,general_double(scene.max_ray_length)*.4f)*general_double(p.fov_y))/float(camera.full_height))*general_double(p.length_multiplier);
    float threshold=(1.f+(z+length)*as_type<float>(scene.de_stop_factor))*3.3f/as_type<float>(scene.z_step_div);
    float3 direction(general_double(p.light_vectors[0]),general_double(p.light_vectors[1]),general_double(p.light_vectors[2]));
    float3 normal=generic_shadow_normal(surface,scene);
    float dscale=max(abs(direction.x),max(abs(direction.y),abs(direction.z)));
    float rscale=max(abs(ray.x),max(abs(ray.y),abs(ray.z)));
    if(dscale<=0.f||rscale<=0.f||!isfinite(length)||!isfinite(threshold)||threshold<=0.f||!all(isfinite(normal))){record.fault=surface.fault=4;return surface;}
    float3 dn=direction/dscale,rn=ray/rscale;
    float zmul=-dot(dn,rn)/sqrt(dot(dn,dn)*dot(rn,rn));
    if(!isfinite(zmul)){record.fault=surface.fault=4;return surface;}
    const float margin=1.f+z*as_type<float>(scene.de_stop_factor);
    spec.kind=kShadowTraced;spec.z=z;spec.threshold=threshold;spec.margin=margin;spec.last_reach=INFINITY;
    bool skip=row.threshold>0;
    if(skip){
        float difference=abs(row.last_z-z);
        float decrement=rint(difference*3.3f/threshold);
        if(!isfinite(decrement)||decrement<0.f){record.fault=surface.fault=4;return surface;}
        // The source's counter is at most3. Test before integer conversion so
        // large finite depth jumps cannot overflow a signed device integer.
        row.threshold=decrement>=float(row.threshold)?0:row.threshold-int(decrement);
        if(row.threshold<=0){skip=false;row.threshold=0;}
        else {row.last_found=(row.last_found-difference)-margin;
            if(row.last_found<=0.f){skip=false;row.threshold=0;}}
    }
    if(dot(normal,direction)>0.f){
        surface.shadow|=1u<<10;row.threshold=0;record.backface=1;spec.flags|=kShadowBackface;
    }else{
        // Tiny overlapping tiles may have a negative screen row. A finite
        // nonpositive CPU shadow extent is a no-ray branch, not an ABI fault.
        float remaining=length,zz=z,factor=2.f;
        if(remaining>0.f){
            const auto cut=mb3d_camera::secondary_cut_distance(camera,point,-direction,remaining,0.f);
            if(cut.fault){record.fault=surface.fault=cut.fault;return surface;}
            remaining=cut.depth;
        }
        if(remaining>0.f){
            record.rays=1;stop=generic_shadow_stop(abs(zz),scene);spec.flags|=kShadowRayLoop;
            ++record.shadow_de_calls;value=general_de(point,stop,program,resources,lane,record.eval);
            bool ended=false;
            for(uint k=0;k<p.max_steps&&!record.eval.fault;++k){
                spec.last_reach=remaining;
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
        if(remaining>0.f){surface.shadow|=1u<<10;row.threshold=0;spec.flags|=kShadowHit;}
    }
    spec.threshold_out=row.threshold;spec.last_found=row.last_found;
    row.last_z=z;record.changed=uint(surface.shadow!=original_shadow);return surface;
}
inline ExactSurface camera_inside_shadow_pixel(ExactPixel geometry,ExactSurface surface,uint y,
    constant ExactScene& scene,constant NativeFormulaProgram& program,constant ExactShadowParams& p,
    thread NativeResources& resources,thread NativeFormulaState& lane,thread GenericShadowRow& row,
    thread NativeShadowRecord& record,constant mb3d_camera::Params& camera,
    constant mb3d_inside::Params& inside){
    NativeShadowSpeculation spec{};
    return camera_inside_shadow_pixel(geometry,surface,y,scene,program,p,resources,lane,row,record,camera,inside,spec);
}
// Replays the ordered row cache from fresh-entry speculation records. Marks
// each pixel whose ordered entry state can skip, with that entry state, and
// returns {fault, marked}. A fault means the ordered trace would differ from
// the fresh one in a way the records cannot resolve; the caller then traces
// the rows in order. The scan reuses the pixel kernel's rounded z, threshold
// and margin, and its own operations do not contract.
inline uint2 camera_inside_shadow_scan_row(device NativeShadowSpeculation* speculation,
    device NativeShadowRowState* entry,uint first,uint width){
    GenericShadowRow row{};uint fault=0,marked=0;
    for(uint x=0;x<width;++x){
        const uint index=first+x;const auto s=speculation[index];
        if(s.flags&kShadowFault)fault|=1u;
        if(s.kind==kShadowNoCache)continue;
        const auto incoming=row;
        if(row.threshold>0)--row.threshold;
        if(s.kind==kShadowCounterOnly)continue;
        bool skip=row.threshold>0;
        if(skip){
            float difference=abs(row.last_z-s.z);
            float decrement=rint(difference*3.3f/s.threshold);
            if(!isfinite(decrement)||decrement<0.f){fault|=2u;break;}
            row.threshold=decrement>=float(row.threshold)?0:row.threshold-int(decrement);
            if(row.threshold<=0){skip=false;row.threshold=0;}
            else {row.last_found=(row.last_found-difference)-s.margin;
                if(row.last_found<=0.f){skip=false;row.threshold=0;}}
        }
        if(skip){entry[index]=incoming;speculation[index].flags=s.flags|kShadowRecompute;++marked;}
        // A traced pixel that does not skip updates the cache exactly as its
        // fresh-entry trace did; a skipping one keeps the cache unless its ray
        // reaches the surface before the shortcut ends it.
        if(s.flags&kShadowBackface)row.threshold=0;
        else if(s.flags&kShadowRayLoop){
            if(!skip){row.threshold=s.threshold_out;if(s.threshold_out>0)row.last_found=s.last_found;}
            else if(!(row.last_found>s.last_reach)&&(s.flags&kShadowHit))row.threshold=0;
        }
        row.last_z=s.z;
    }
    return uint2(fault,marked);
}
}
kernel void native_camera_inside_shadow_abi(device uint* words [[buffer(0)]],uint id [[thread_position_in_grid]]){
    if(id)return;words[0]=1;words[1]=sizeof(mb3d_camera::Params);words[2]=sizeof(mb3d_inside::Params);
    words[3]=sizeof(mb3d_native::NativeShadowRecord);words[4]=sizeof(mb3d_native::NativeShadowRowState);
    words[5]=mb3d_native::kLogicalLaneBatchPixels;
}
kernel void native_camera_inside_hard_shadow(device const ExactPixel* primary [[buffer(0)]],
    device const ExactSurface* input [[buffer(1)]],device ExactSurface* output [[buffer(2)]],
    device mb3d_native::NativeShadowRecord* records [[buffer(3)]],constant ExactScene& scene [[buffer(4)]],
    constant mb3d_native::NativeFormulaProgram& program [[buffer(5)]],constant ExactShadowParams& params [[buffer(6)]],
    device const mb3d_native::NativeMapDescriptor* maps [[buffer(7)]],device const float* pixels [[buffer(8)]],
    constant uint4& launch [[buffer(9)]],constant uint2& resource_counts [[buffer(10)]],constant mb3d_camera::Params& camera [[buffer(23)]],
    constant mb3d_inside::Params& inside [[buffer(27)]],uint local [[thread_position_in_grid]]){
    if(local>=launch.y||launch.x>=scene.height||local>=scene.height-launch.x||!scene.width||scene.width>4096)return;
    uint y=launch.x+local;ulong base=ulong(y)*scene.width;if(base>=launch.z)return;
    bool valid=!launch.w&&ulong(scene.width)*scene.height==launch.z&&mb3d_native::valid_generic_shadow(scene,program,params);
    mb3d_native::NativeResources resources{maps,pixels,resource_counts.x,resource_counts.y,nullptr};
    auto lane=mb3d_native::native_initial_state(float3(0),as_type<float>(scene.de_stop),program);
    mb3d_native::GenericShadowRow row{};
    for(uint x=0;x<scene.width&&base+x<launch.z;++x){
        uint index=uint(base)+x;auto surface=input[index];mb3d_native::NativeShadowRecord record{};
        record.version=mb3d_native::kGenericShadowVersion;record.eval.version=mb3d_native::kGeneralGeometryVersion;
        if(valid)surface=mb3d_native::camera_inside_shadow_pixel(primary[index],surface,y,scene,program,params,resources,lane,row,record,camera,inside);
        else record.fault=surface.fault=1;
        output[index]=surface;records[index]=record;
    }
}

kernel void native_camera_inside_shadow_pixels_abi(device uint* words [[buffer(0)]],uint id [[thread_position_in_grid]]){
    if(id)return;words[0]=1;words[1]=sizeof(mb3d_native::NativeShadowSpeculation);
    words[2]=sizeof(mb3d_native::NativeShadowRowState);words[3]=sizeof(mb3d_native::NativeShadowRecord);
}
// launch: first pixel, pixels in this dispatch, frame pixels, mode. Mode 0
// traces every pixel from a fresh row cache and records its speculation;
// mode 1 traces the pixels the row scan marked, from their ordered entry
// state. Both modes run this one function, so a pixel's trace arithmetic does
// not depend on its entry state.
kernel void native_camera_inside_hard_shadow_pixels(device const ExactPixel* primary [[buffer(0)]],
    device const ExactSurface* input [[buffer(1)]],device ExactSurface* output [[buffer(2)]],
    device mb3d_native::NativeShadowRecord* records [[buffer(3)]],constant ExactScene& scene [[buffer(4)]],
    constant mb3d_native::NativeFormulaProgram& program [[buffer(5)]],constant ExactShadowParams& params [[buffer(6)]],
    device const mb3d_native::NativeMapDescriptor* maps [[buffer(7)]],device const float* pixels [[buffer(8)]],
    constant uint4& launch [[buffer(9)]],constant uint2& resource_counts [[buffer(10)]],
    device mb3d_native::NativeShadowSpeculation* speculation [[buffer(11)]],
    device const mb3d_native::NativeShadowRowState* entry [[buffer(12)]],
    constant mb3d_camera::Params& camera [[buffer(23)]],constant mb3d_inside::Params& inside [[buffer(27)]],
    uint local [[thread_position_in_grid]]){
    if(local>=launch.y||launch.x>=launch.z||local>=launch.z-launch.x||!scene.width||scene.width>4096)return;
    const uint index=launch.x+local,y=index/scene.width;const bool recompute=launch.w==1;
    if(recompute&&!(speculation[index].flags&mb3d_native::kShadowRecompute))return;
    const bool valid=launch.w<=1&&ulong(scene.width)*scene.height==launch.z&&
        mb3d_native::valid_generic_shadow(scene,program,params);
    mb3d_native::NativeResources resources{maps,pixels,resource_counts.x,resource_counts.y,nullptr};
    auto lane=mb3d_native::native_initial_state(float3(0),as_type<float>(scene.de_stop),program);
    mb3d_native::GenericShadowRow row=recompute?entry[index]:mb3d_native::GenericShadowRow{};
    auto surface=input[index];mb3d_native::NativeShadowRecord record{};mb3d_native::NativeShadowSpeculation spec{};
    record.version=mb3d_native::kGenericShadowVersion;record.eval.version=mb3d_native::kGeneralGeometryVersion;
    if(valid)surface=mb3d_native::camera_inside_shadow_pixel(primary[index],surface,y,scene,program,params,resources,lane,row,record,camera,inside,spec);
    else record.fault=surface.fault=1;
    if(record.fault)spec.flags|=mb3d_native::kShadowFault;
    output[index]=surface;records[index]=record;
    if(!recompute)speculation[index]=spec;
}
// One thread per row: launch is first row, rows in this dispatch.
kernel void native_camera_inside_shadow_scan(device mb3d_native::NativeShadowSpeculation* speculation [[buffer(0)]],
    device mb3d_native::NativeShadowRowState* entry [[buffer(1)]],device uint2* status [[buffer(2)]],
    constant ExactScene& scene [[buffer(4)]],constant uint4& launch [[buffer(9)]],uint local [[thread_position_in_grid]]){
    if(local>=launch.y||launch.x>=scene.height||local>=scene.height-launch.x||!scene.width||scene.width>4096)return;
    const uint y=launch.x+local;
    status[y]=mb3d_native::camera_inside_shadow_scan_row(speculation,entry,y*scene.width,scene.width);
}
kernel void native_camera_inside_hard_shadow_lanes(device const ExactPixel* primary [[buffer(0)]],
    device const ExactSurface* input [[buffer(1)]],device ExactSurface* output [[buffer(2)]],
    device mb3d_native::NativeShadowRecord* records [[buffer(3)]],constant ExactScene& scene [[buffer(4)]],
    constant mb3d_native::NativeFormulaProgram& program [[buffer(5)]],constant ExactShadowParams& params [[buffer(6)]],
    device const mb3d_native::NativeMapDescriptor* maps [[buffer(7)]],device const float* pixels [[buffer(8)]],
    constant uint4& launch [[buffer(9)]],constant uint2& resource_counts [[buffer(10)]],
    device const mb3d_native::NativeLogicalLane* descriptors [[buffer(11)]],
    device mb3d_native::NativeFormulaState* states [[buffer(12)]],
    device mb3d_native::NativeShadowRowState* caches [[buffer(13)]],
    device mb3d_native::NativeLogicalLaneRecord* lane_records [[buffer(14)]],
    constant uint& batch_rows [[buffer(15)]],constant mb3d_camera::Params& camera [[buffer(23)]],
    constant mb3d_inside::Params& inside [[buffer(27)]],uint local [[thread_position_in_grid]]){
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
        surface=mb3d_native::camera_inside_shadow_pixel(primary[index],surface,y,scene,program,params,resources,lane,row,record,camera,inside);
        output[index]=surface;records[index]=record;++status.processed;++status.next_cursor;
        if(record.fault){status.fault=record.fault;break;}
    }
    states[id]=lane;caches[id]=row;lane_records[id]=status;
}
