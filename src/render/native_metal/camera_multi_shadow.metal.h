#pragma once
#include "camera_inside_shadow.metal.h"
#include "shadow_inside_bridge.metal.h"
#include "secondary_cut_candidate.metal.h"
namespace mb3d_native {
inline ExactSurface camera_multi_shadow_pixel(ExactPixel geometry,ExactSurface surface,uint y,
    constant ExactScene& scene,constant NativeFormulaProgram& program,constant ExactShadowParams& p,
    thread NativeResources& resources,thread NativeFormulaState& lane,thread NativeMultiShadowRowState& cache,
    thread NativeMultiShadowRecord& multi,constant mb3d_camera::Params& camera,
    constant mb3d_inside::Params& inside,thread NativeMultiShadowSpeculation& spec){
    thread NativeShadowRecord& record=multi.total;
    record.version=kGenericShadowVersion;record.eval.version=kGeneralGeometryVersion;
    if(!mb3d_camera::valid_camera(camera)||camera.width!=scene.width||camera.height!=scene.height||
       !mb3d_inside::valid(inside)||inside.full_width!=camera.full_width){record.fault=surface.fault=1;return surface;}
    if(geometry.fault||surface.fault){record.fault=surface.fault|geometry.fault|1u;surface.fault=record.fault;return surface;}
    if(!p.enabled)return surface;
    for(uint light=0;light<6;++light)if(cache.threshold[light]>0)--cache.threshold[light];
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
    const float margin=1.f+z*as_type<float>(scene.de_stop_factor);
    spec.kind=kShadowTraced;spec.z=z;spec.threshold=threshold;spec.margin=margin;
    // CPU order is pixel then original light index. Reuse reconstruction and
    // refinement, restore only position; carried formula state remains ordered.
    const float3 origin=point;
    for(uint light=0;light<6;++light){
    if(!(p.light_mask&(1u<<light)))continue;
    GenericShadowRow row{cache.last_found[light],cache.last_z,cache.threshold[light],0};
    thread NativeShadowLightRecord& work=multi.lights[light];
    point=origin;spec.last_reach[light]=INFINITY;
    float3 direction(general_double(p.light_vectors[3*light]),general_double(p.light_vectors[3*light+1]),general_double(p.light_vectors[3*light+2]));
    // calcHShard position light: march toward LN+Xmit over at most its
    // distance; beyond the squared reach it is shadowed without a march.
    float lightLength=length;bool beyond=false,coincident=false;
    if(p.position_mask&(1u<<light)){
        const float3 target(general_double(p.light_positions[3*light]),general_double(p.light_positions[3*light+1]),
                            general_double(p.light_positions[3*light+2]));
        const float3 toward=target-origin;const float squared=(toward.x*toward.x+toward.y*toward.y)+toward.z*toward.z;
        if(!isfinite(squared)){record.fault=surface.fault=4;return surface;}
        const float step=general_double(p.step_width);
        if(squared>as_type<float>(p.light_reach[light]))beyond=true;
        else if(squared==0.f)coincident=true; // CPU dMaxL 0: no march, lit
        else {
            if(squared<(lightLength*step)*(lightLength*step))lightLength=sqrt(squared)/step;
            direction=toward*(-step/sqrt(squared));
        }
    }
    float3 normal=generic_shadow_normal(surface,scene);
    float zmul=0.f;
    if(!beyond&&!coincident){
        float dscale=max(abs(direction.x),max(abs(direction.y),abs(direction.z)));
        float rscale=max(abs(ray.x),max(abs(ray.y),abs(ray.z)));
        if(dscale<=0.f||rscale<=0.f||!isfinite(length)||!isfinite(threshold)||threshold<=0.f||!all(isfinite(normal))){record.fault=surface.fault=4;return surface;}
        float3 dn=direction/dscale,rn=ray/rscale;
        zmul=-dot(dn,rn)/sqrt(dot(dn,dn)*dot(rn,rn));
        if(!isfinite(zmul)){record.fault=surface.fault=4;return surface;}
    }
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
    if(beyond){surface.shadow|=1u<<(10+light);work.beyond=1;} // keeps the decremented row threshold
    else if(coincident){}
    else if(dot(normal,direction)>0.f){
        surface.shadow|=1u<<(10+light);row.threshold=0;work.backface=1;spec.light_flags[light]|=kShadowBackface;
    }else{
        // Tiny overlapping tiles may have a negative screen row. A finite
        // nonpositive CPU shadow extent is a no-ray branch, not an ABI fault.
        float remaining=lightLength,zz=z,factor=2.f;
        if(remaining>0.f){
            const auto cut=mb3d_camera::secondary_cut_distance(camera,point,-direction,remaining,0.f);
            if(cut.fault){record.fault=surface.fault=cut.fault;return surface;}
            remaining=cut.depth;
        }
        if(remaining>0.f){
            work.rays=1;stop=generic_shadow_stop(abs(zz),scene);spec.light_flags[light]|=kShadowRayLoop;
            ++work.shadow_de_calls;value=general_de(point,stop,program,resources,lane,record.eval);
            bool ended=false;
            for(uint k=0;k<p.max_steps&&!record.eval.fault;++k){
                spec.last_reach[light]=remaining;
                if(skip&&row.last_found>remaining){remaining=-1.f;work.skipped=1;ended=true;break;}
                float previous=value.distance;
                float width=max(.11f,((value.distance-as_type<float>(scene.de_sub)*stop)*as_type<float>(scene.z_step_div))*factor);
                width=min(width,max(stop,.4f)*as_type<float>(scene.step_limiter));
                if(!isfinite(width)||width<=0.f){record.eval.fault=4;break;}
                remaining-=width;point-=direction*width;zz+=width*zmul;stop=generic_shadow_stop(abs(zz),scene);
                ++work.steps;++work.shadow_de_calls;value=general_de(point,stop,program,resources,lane,record.eval);
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
            work.hits=uint(remaining>0.f);work.misses=1u-work.hits;
        }
        if(remaining>0.f){surface.shadow|=1u<<(10+light);row.threshold=0;spec.light_flags[light]|=kShadowHit;}
    }
    cache.last_found[light]=row.last_found;cache.threshold[light]=row.threshold;
    spec.threshold_out[light]=row.threshold;spec.last_found[light]=row.last_found;
    record.backface+=work.backface;record.rays+=work.rays;record.hits+=work.hits;
    record.misses+=work.misses;record.skipped+=work.skipped;record.steps+=work.steps;
    record.shadow_de_calls+=work.shadow_de_calls;
    }
    cache.last_z=z;record.changed=uint(surface.shadow!=original_shadow);return surface;
}
inline ExactSurface camera_multi_shadow_pixel(ExactPixel geometry,ExactSurface surface,uint y,
    constant ExactScene& scene,constant NativeFormulaProgram& program,constant ExactShadowParams& p,
    thread NativeResources& resources,thread NativeFormulaState& lane,thread NativeMultiShadowRowState& cache,
    thread NativeMultiShadowRecord& multi,constant mb3d_camera::Params& camera,
    constant mb3d_inside::Params& inside){
    NativeMultiShadowSpeculation spec{};
    return camera_multi_shadow_pixel(geometry,surface,y,scene,program,p,resources,lane,cache,multi,camera,inside,spec);
}
// The single-light row scan for every enabled light, in original light order.
// Beyond and coincident position lights keep the cache their shortcut test
// left, as the ordered pixel does. A pixel is traced again when any light can
// skip from its ordered entry state.
inline uint2 camera_multi_shadow_scan_row(device NativeMultiShadowSpeculation* speculation,
    device NativeMultiShadowRowState* entry,uint first,uint width,uint light_mask){
    NativeMultiShadowRowState cache{};uint fault=0,marked=0;
    for(uint x=0;x<width&&!(fault&2u);++x){
        const uint index=first+x;device const auto& s=speculation[index];
        if(s.flags&kShadowFault)fault|=1u;
        if(s.kind==kShadowNoCache)continue;
        const auto incoming=cache;
        for(uint light=0;light<6;++light)if(cache.threshold[light]>0)--cache.threshold[light];
        if(s.kind==kShadowCounterOnly)continue;
        bool any=false;
        for(uint light=0;light<6;++light){
            if(!(light_mask&(1u<<light)))continue;
            GenericShadowRow row{cache.last_found[light],cache.last_z,cache.threshold[light],0};
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
            any|=skip;const uint flags=s.light_flags[light];
            if(flags&kShadowBackface)row.threshold=0;
            else if(flags&kShadowRayLoop){
                if(!skip){row.threshold=s.threshold_out[light];if(s.threshold_out[light]>0)row.last_found=s.last_found[light];}
                else if(!(row.last_found>s.last_reach[light])&&(flags&kShadowHit))row.threshold=0;
            }
            cache.last_found[light]=row.last_found;cache.threshold[light]=row.threshold;
        }
        if(fault&2u)break;
        if(any){entry[index]=incoming;speculation[index].flags=s.flags|kShadowRecompute;++marked;}
        cache.last_z=s.z;
    }
    return uint2(fault,marked);
}
}
kernel void native_camera_multi_shadow_abi(device uint* words [[buffer(0)]],uint id [[thread_position_in_grid]]){
    if(id)return;words[0]=1;words[1]=sizeof(mb3d_camera::Params);words[2]=sizeof(mb3d_inside::Params);
    words[3]=sizeof(mb3d_native::NativeMultiShadowRecord);words[4]=sizeof(mb3d_native::NativeMultiShadowRowState);
    words[5]=mb3d_native::kLogicalLaneBatchPixels;
}
kernel void native_camera_multi_hard_shadow(device const ExactPixel* primary [[buffer(0)]],
    device const ExactSurface* input [[buffer(1)]],device ExactSurface* output [[buffer(2)]],
    device mb3d_native::NativeMultiShadowRecord* records [[buffer(3)]],constant ExactScene& scene [[buffer(4)]],
    constant mb3d_native::NativeFormulaProgram& program [[buffer(5)]],constant ExactShadowParams& params [[buffer(6)]],
    device const mb3d_native::NativeMapDescriptor* maps [[buffer(7)]],device const float* pixels [[buffer(8)]],
    constant uint4& launch [[buffer(9)]],constant uint2& resource_counts [[buffer(10)]],constant mb3d_camera::Params& camera [[buffer(23)]],
    constant mb3d_inside::Params& inside [[buffer(27)]],uint local [[thread_position_in_grid]]){
    if(local>=launch.y||launch.x>=scene.height||local>=scene.height-launch.x||!scene.width||scene.width>4096)return;
    uint y=launch.x+local;ulong base=ulong(y)*scene.width;if(base>=launch.z)return;
    bool valid=!launch.w&&ulong(scene.width)*scene.height==launch.z&&mb3d_native::valid_generic_shadow(scene,program,params,true,true);
    mb3d_native::NativeResources resources{maps,pixels,resource_counts.x,resource_counts.y,nullptr};
    auto lane=mb3d_native::native_initial_state(float3(0),as_type<float>(scene.de_stop),program);
    mb3d_native::NativeMultiShadowRowState row{};
    for(uint x=0;x<scene.width&&base+x<launch.z;++x){
        uint index=uint(base)+x;auto surface=input[index];mb3d_native::NativeMultiShadowRecord record{};
        record.total.version=mb3d_native::kGenericShadowVersion;record.total.eval.version=mb3d_native::kGeneralGeometryVersion;
        if(valid)surface=mb3d_native::camera_multi_shadow_pixel(primary[index],surface,y,scene,program,params,resources,lane,row,record,camera,inside);
        else record.total.fault=surface.fault=1;
        output[index]=surface;records[index]=record;
    }
}

kernel void native_camera_multi_hard_shadow_lanes(device const ExactPixel* primary [[buffer(0)]],
    device const ExactSurface* input [[buffer(1)]],device ExactSurface* output [[buffer(2)]],
    device mb3d_native::NativeMultiShadowRecord* records [[buffer(3)]],constant ExactScene& scene [[buffer(4)]],
    constant mb3d_native::NativeFormulaProgram& program [[buffer(5)]],constant ExactShadowParams& params [[buffer(6)]],
    device const mb3d_native::NativeMapDescriptor* maps [[buffer(7)]],device const float* pixels [[buffer(8)]],
    constant uint4& launch [[buffer(9)]],constant uint2& resource_counts [[buffer(10)]],
    device const mb3d_native::NativeLogicalLane* descriptors [[buffer(11)]],
    device mb3d_native::NativeFormulaState* states [[buffer(12)]],
    device mb3d_native::NativeMultiShadowRowState* caches [[buffer(13)]],
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
       !mb3d_native::valid_generic_shadow(scene,program,params,true,true)){
        status.fault=1;lane_records[id]=status;return;
    }
    if(!descriptor.pixel_count){lane_records[id]=status;return;}
    auto lane=descriptor.cursor?states[id]:mb3d_native::native_initial_state(float3(0),as_type<float>(scene.de_stop),program);
    mb3d_native::NativeMultiShadowRowState row=descriptor.cursor?caches[id]:mb3d_native::NativeMultiShadowRowState{};
    bool badCache=row.reserved||!metal::isfinite(row.last_z);
    for(uint light=0;light<6;++light)badCache|=row.threshold[light]<0||row.threshold[light]>3||!metal::isfinite(row.last_found[light]);
    if(badCache){
        status.fault=1;lane_records[id]=status;return;
    }
    mb3d_native::NativeResources resources{maps,pixels,resource_counts.x,resource_counts.y,nullptr};
    for(uint k=0;k<descriptor.pixel_count;++k){
        const uint ordinal=descriptor.cursor+k,x=ordinal%scene.width;
        const uint y=descriptor.row_start+ordinal/scene.width,index=y*scene.width+x;
        if(x==0)row={};
        auto surface=input[index];mb3d_native::NativeMultiShadowRecord record{};
        surface=mb3d_native::camera_multi_shadow_pixel(primary[index],surface,y,scene,program,params,resources,lane,row,record,camera,inside);
        output[index]=surface;records[index]=record;++status.processed;++status.next_cursor;
        if(record.total.fault){status.fault=record.total.fault;break;}
    }
    states[id]=lane;caches[id]=row;lane_records[id]=status;
}

kernel void native_camera_multi_shadow_pixels_abi(device uint* words [[buffer(0)]],uint id [[thread_position_in_grid]]){
    if(id)return;words[0]=1;words[1]=sizeof(mb3d_native::NativeMultiShadowSpeculation);
    words[2]=sizeof(mb3d_native::NativeMultiShadowRowState);words[3]=sizeof(mb3d_native::NativeMultiShadowRecord);
}
// The multi-light form of native_camera_inside_hard_shadow_pixels.
kernel void native_camera_multi_hard_shadow_pixels(device const ExactPixel* primary [[buffer(0)]],
    device const ExactSurface* input [[buffer(1)]],device ExactSurface* output [[buffer(2)]],
    device mb3d_native::NativeMultiShadowRecord* records [[buffer(3)]],constant ExactScene& scene [[buffer(4)]],
    constant mb3d_native::NativeFormulaProgram& program [[buffer(5)]],constant ExactShadowParams& params [[buffer(6)]],
    device const mb3d_native::NativeMapDescriptor* maps [[buffer(7)]],device const float* pixels [[buffer(8)]],
    constant uint4& launch [[buffer(9)]],constant uint2& resource_counts [[buffer(10)]],
    device mb3d_native::NativeMultiShadowSpeculation* speculation [[buffer(11)]],
    device const mb3d_native::NativeMultiShadowRowState* entry [[buffer(12)]],
    constant mb3d_camera::Params& camera [[buffer(23)]],constant mb3d_inside::Params& inside [[buffer(27)]],
    uint local [[thread_position_in_grid]]){
    if(local>=launch.y||launch.x>=launch.z||local>=launch.z-launch.x||!scene.width||scene.width>4096)return;
    const uint index=launch.x+local,y=index/scene.width;const bool recompute=launch.w==1;
    if(recompute&&!(speculation[index].flags&mb3d_native::kShadowRecompute))return;
    const bool valid=launch.w<=1&&ulong(scene.width)*scene.height==launch.z&&
        mb3d_native::valid_generic_shadow(scene,program,params,true,true);
    mb3d_native::NativeResources resources{maps,pixels,resource_counts.x,resource_counts.y,nullptr};
    auto lane=mb3d_native::native_initial_state(float3(0),as_type<float>(scene.de_stop),program);
    mb3d_native::NativeMultiShadowRowState row=recompute?entry[index]:mb3d_native::NativeMultiShadowRowState{};
    auto surface=input[index];mb3d_native::NativeMultiShadowRecord record{};mb3d_native::NativeMultiShadowSpeculation spec{};
    record.total.version=mb3d_native::kGenericShadowVersion;record.total.eval.version=mb3d_native::kGeneralGeometryVersion;
    if(valid)surface=mb3d_native::camera_multi_shadow_pixel(primary[index],surface,y,scene,program,params,resources,lane,row,record,camera,inside,spec);
    else record.total.fault=surface.fault=1;
    if(record.total.fault)spec.flags|=mb3d_native::kShadowFault;
    output[index]=surface;records[index]=record;
    if(!recompute)speculation[index]=spec;
}
kernel void native_camera_multi_shadow_scan(device mb3d_native::NativeMultiShadowSpeculation* speculation [[buffer(0)]],
    device mb3d_native::NativeMultiShadowRowState* entry [[buffer(1)]],device uint2* status [[buffer(2)]],
    constant ExactScene& scene [[buffer(4)]],constant ExactShadowParams& params [[buffer(6)]],
    constant uint4& launch [[buffer(9)]],uint local [[thread_position_in_grid]]){
    if(local>=launch.y||launch.x>=scene.height||local>=scene.height-launch.x||!scene.width||scene.width>4096)return;
    const uint y=launch.x+local;
    status[y]=mb3d_native::camera_multi_shadow_scan_row(speculation,entry,y*scene.width,scene.width,params.light_mask);
}
