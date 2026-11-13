#pragma once
#include "camera_inside_shadow.metal.h"
#include "soft_shadow_abi.h"
#include "soft_shadow_state_contract.h"
namespace mb3d_native {
// The soft light is the last shadow light (iHSnr). ZRSmul is 80/radius for a
// directional light; a position light uses 70 or 40 by iLightPos&6. The
// reflection pass's CalcHSsoftPoint shares this descriptor.
inline bool valid_soft_light(constant ExactShadowParams& p,constant NativeSoftShadowConfig& soft) {
    if(soft.version!=kNativeSoftShadowVersion||soft.light>=6||!p.enabled||!isfinite(as_type<float>(soft.radius))||
       as_type<float>(soft.radius)<=0.f||!isfinite(80.f/as_type<float>(soft.radius)))return false;
    uint last=0;for(uint i=0;i<6;++i)if(p.light_mask&(1u<<i))last=i;
    const bool position=(p.position_mask>>soft.light)&1u;
    return last==soft.light&&(position?(soft.numerator==70u||soft.numerator==40u):soft.numerator==80u);
}
inline bool valid_soft_shadow(constant ExactScene& scene,constant NativeFormulaProgram& program,
    constant ExactShadowParams& p,constant NativeSoftShadowConfig& soft) {
    return valid_generic_shadow(scene,program,p,true,true)&&valid_soft_light(p,soft);
}
inline ExactSurface camera_soft_shadow_pixel(ExactPixel geometry,ExactSurface surface,uint y,
    constant ExactScene& scene,constant NativeFormulaProgram& program,constant ExactShadowParams& p,
    constant NativeSoftShadowConfig& soft,thread NativeResources& resources,thread NativeFormulaState& lane,
    thread NativeSoftShadowRecord& result,constant mb3d_camera::Params& camera,
    constant mb3d_inside::Params& inside) {
    thread auto& record=result.total;result.version=kNativeSoftShadowVersion;
    record.version=kGenericShadowVersion;record.eval.version=kGeneralGeometryVersion;
    if(!mb3d_camera::valid_camera(camera)||camera.width!=scene.width||camera.height!=scene.height||
       !mb3d_inside::valid(inside)||inside.full_width!=camera.full_width||geometry.fault||surface.fault){
        record.fault=surface.fault=1;return surface;
    }
    const uint original=surface.shadow;surface.shadow|=0xfc00u;result.intensity=63;
    if((surface.depth_packed>>16)>=32768||surface.si_gradient>=32768){record.changed=uint(surface.shadow!=original);return surface;}
    record.active=1;
    const float3 ray(general_double(geometry.ray[0]),general_double(geometry.ray[1]),general_double(geometry.ray[2]));
    const float3 start(general_double(geometry.start[0]),general_double(geometry.start[1]),general_double(geometry.start[2]));
    float z=generic_shadow_depth(surface.depth_packed,p,8388351.5f),stop=generic_shadow_stop(z,scene);
    float3 point=start+ray*z;
    if(!isfinite(z)||!isfinite(stop)||stop<=0.f||!all(isfinite(ray))||!all(isfinite(point))){record.fault=surface.fault=4;return surface;}
    auto value=general_shadow_inside_start(point,stop,program,inside,surface.o_trap,resources,lane,record.eval);
    if(record.eval.fault){record.fault=surface.fault=record.eval.fault;return surface;}
    if(!generic_shadow_refine(point,ray,z,stop,value,surface.depth_packed,scene,p,program,resources,lane,record.eval)){
        record.fault=surface.fault=record.eval.fault;return surface;
    }
    z-=.1f;point-=ray*.1f;
    const float length=float(int(camera.full_width)+mb3d_camera::screen_y(camera,y))*.6f*
        (1.f+(.5f*min(z,general_double(scene.max_ray_length)*.4f)*general_double(p.fov_y))/float(camera.full_height))*general_double(p.length_multiplier);
    const uint l=3*soft.light;
    float3 direction(general_double(p.light_vectors[l]),general_double(p.light_vectors[l+1]),general_double(p.light_vectors[l+2]));
    if(!isfinite(length)){record.fault=surface.fault=4;return surface;}
    // calcHSsoft position light: march toward LN+Xmit over at most its
    // distance. Beyond the squared reach the pixel stays lit (unlike calcHShard);
    // at the light itself dMaxL is 0 and nothing is marched.
    float extent=length;
    if((p.position_mask>>soft.light)&1u){
        const float3 target(general_double(p.light_positions[l]),general_double(p.light_positions[l+1]),
                            general_double(p.light_positions[l+2]));
        const float3 toward=target-point;const float squared=(toward.x*toward.x+toward.y*toward.y)+toward.z*toward.z;
        if(!isfinite(squared)){record.fault=surface.fault=4;return surface;}
        if(squared>as_type<float>(p.light_reach[soft.light])||squared==0.f){record.changed=uint(surface.shadow!=original);return surface;}
        const float step=general_double(p.step_width);
        if(squared<(extent*step)*(extent*step))extent=sqrt(squared)/step;
        direction=toward*(-step/sqrt(squared));
    }
    const float3 normal=generic_shadow_normal(surface,scene);
    const float ds=max(abs(direction.x),max(abs(direction.y),abs(direction.z)));
    const float rs=max(abs(ray.x),max(abs(ray.y),abs(ray.z)));
    if(ds<=0.f||rs<=0.f||!isfinite(extent)||!all(isfinite(normal))||!all(isfinite(direction))){record.fault=surface.fault=4;return surface;}
    const float3 dn=direction/ds,rn=ray/rs;
    const float zmul=-dot(dn,rn)/sqrt(dot(dn,dn)*dot(rn,rn));
    if(!isfinite(zmul)){record.fault=surface.fault=4;return surface;}
    if(extent>0.f){const auto cut=mb3d_camera::secondary_cut_distance(camera,point,-direction,extent,0.f);
        if(cut.fault){record.fault=surface.fault=cut.fault;return surface;}extent=cut.depth;}
    float visibility=1.f;
    if(dot(normal,direction)>0.f){record.backface=1;visibility=0.f;}
    else if(extent>0.f){
        record.rays=1;float remaining=extent,zz=z,factor=2.f;
        stop=generic_shadow_stop(abs(zz),scene);
        ++record.shadow_de_calls;value=general_de(point,stop,program,resources,lane,record.eval);
        const float radius_mul=float(soft.numerator)/as_type<float>(soft.radius);
        bool ended=false;
        for(uint k=0;k<p.max_steps&&!record.eval.fault;++k){
            float previous=value.distance;
            float width=max(.11f,((value.distance-as_type<float>(scene.de_sub)*stop)*as_type<float>(scene.z_step_div))*factor);
            width=min(width,max(stop,.4f)*as_type<float>(scene.step_limiter));
            if(!isfinite(width)||width<=0.f){record.eval.fault=4;break;}
            remaining-=width;point-=direction*width;zz+=width*zmul;
            stop=generic_shadow_stop(abs(zz),scene);
            ++record.steps;++record.shadow_de_calls;value=general_de(point,stop,program,resources,lane,record.eval);
            if(record.eval.fault)break;
            if(!isfinite(remaining)||!isfinite(stop)||stop<=0.f||!all(isfinite(point))){record.eval.fault=4;break;}
            // CPU uses the original MaxLHS, not the cut-clipped extent, for x^8.
            const float traveled=extent-remaining,dist=traveled+.11f;
            float threshold=traveled/length;threshold*=threshold;threshold*=threshold;threshold*=threshold;
            const float candidate=(value.distance-stop)*radius_mul/dist+threshold;
            if(!isfinite(candidate)){record.eval.fault=4;break;}
            visibility=min(visibility,candidate);
            // Closest approach is observed before both termination and DE clamp.
            if(uint(value.state.iteration)>=native_shadow_maximum(program,value,p.max_iterations_result)||value.distance<=stop){ended=true;break;}
            if(value.distance>previous+width)value.distance=previous+width;
            if(previous>value.distance+1.e-30f){float ratio=width/(previous-value.distance);factor=ratio<1.f?max(.5f,ratio):1.f;}
            else factor=1.f;
            if(remaining<0.f){ended=true;break;}
        }
        if(!ended&&!record.eval.fault)record.eval.fault=2;
        if(record.eval.fault){record.fault=surface.fault=record.eval.fault;return surface;}
        record.hits=uint(remaining>0.f);record.misses=1u-record.hits;
    }
    const uint intensity=uint(round(clamp(visibility,0.f,1.f)*63.4f));
    surface.shadow=(surface.shadow&0x3ffu)|(intensity<<10);
    result.intensity=intensity;result.partial=uint(intensity>0&&intensity<63);
    record.changed=uint(surface.shadow!=original);return surface;
}
}
kernel void native_camera_soft_shadow_abi(device uint* words [[buffer(0)]],uint id [[thread_position_in_grid]]) {
    if(id)return;words[0]=mb3d_native::kNativeSoftShadowVersion;words[1]=sizeof(mb3d_camera::Params);
    words[2]=sizeof(mb3d_inside::Params);words[3]=sizeof(mb3d_native::NativeSoftShadowRecord);
    words[4]=sizeof(mb3d_native::NativeSoftShadowConfig);words[5]=mb3d_native::kLogicalLaneBatchPixels;
}
kernel void native_camera_soft_shadow(device const ExactPixel* primary [[buffer(0)]],
    device const ExactSurface* input [[buffer(1)]],device ExactSurface* output [[buffer(2)]],
    device mb3d_native::NativeSoftShadowRecord* records [[buffer(3)]],constant ExactScene& scene [[buffer(4)]],
    constant mb3d_native::NativeFormulaProgram& program [[buffer(5)]],constant ExactShadowParams& params [[buffer(6)]],
    device const mb3d_native::NativeMapDescriptor* maps [[buffer(7)]],device const float* pixels [[buffer(8)]],
    constant uint4& launch [[buffer(9)]],constant uint2& resource_counts [[buffer(10)]],
    constant mb3d_camera::Params& camera [[buffer(23)]],constant mb3d_inside::Params& inside [[buffer(27)]],
    constant mb3d_native::NativeSoftShadowConfig& soft [[buffer(28)]],uint local [[thread_position_in_grid]]) {
    if(local>=launch.y||launch.x>=scene.height||local>=scene.height-launch.x||!scene.width||scene.width>4096)return;
    const uint y=launch.x+local;const ulong base=ulong(y)*scene.width;if(base>=launch.z)return;
    const bool valid=!launch.w&&ulong(scene.width)*scene.height==launch.z&&mb3d_native::valid_soft_shadow(scene,program,params,soft);
    mb3d_native::NativeResources resources{maps,pixels,resource_counts.x,resource_counts.y,nullptr};
    auto lane=mb3d_native::native_initial_state(float3(0),as_type<float>(scene.de_stop),program);
    for(uint x=0;x<scene.width&&base+x<launch.z;++x){
        const uint index=uint(base)+x;auto surface=input[index];mb3d_native::NativeSoftShadowRecord record{};
        if(valid)surface=mb3d_native::camera_soft_shadow_pixel(primary[index],surface,y,scene,program,params,soft,resources,lane,record,camera,inside);
        else record.total.fault=surface.fault=1;
        output[index]=surface;records[index]=record;
    }
}
kernel void native_camera_soft_shadow_lanes(device const ExactPixel* primary [[buffer(0)]],
    device const ExactSurface* input [[buffer(1)]],device ExactSurface* output [[buffer(2)]],
    device mb3d_native::NativeSoftShadowRecord* records [[buffer(3)]],constant ExactScene& scene [[buffer(4)]],
    constant mb3d_native::NativeFormulaProgram& program [[buffer(5)]],constant ExactShadowParams& params [[buffer(6)]],
    device const mb3d_native::NativeMapDescriptor* maps [[buffer(7)]],device const float* pixels [[buffer(8)]],
    constant uint4& launch [[buffer(9)]],constant uint2& resource_counts [[buffer(10)]],
    device const mb3d_native::NativeLogicalLane* descriptors [[buffer(11)]],
    device mb3d_native::NativeFormulaState* states [[buffer(12)]],
    device mb3d_native::NativeLogicalLaneRecord* lane_records [[buffer(14)]],constant uint& batch_rows [[buffer(15)]],
    constant mb3d_camera::Params& camera [[buffer(23)]],constant mb3d_inside::Params& inside [[buffer(27)]],
    constant mb3d_native::NativeSoftShadowConfig& soft [[buffer(28)]],uint local [[thread_position_in_grid]]) {
    if(local>=launch.y||launch.x>=launch.z||local>=launch.z-launch.x)return;
    const uint id=launch.x+local;const auto descriptor=descriptors[id];
    mb3d_native::NativeLogicalLaneRecord status{1,0,0,descriptor.cursor};
    if(!batch_rows||batch_rows>scene.height||launch.z!=(scene.height+batch_rows-1)/batch_rows||
       !mb3d_native::valid_logical_lane(descriptor,scene.width,scene.height,launch.w)||
       descriptor.row_start!=id*batch_rows||descriptor.row_stride!=1||
       descriptor.row_count!=metal::min(batch_rows,scene.height-descriptor.row_start)||
       !mb3d_native::valid_soft_shadow(scene,program,params,soft)){
        status.fault=1;lane_records[id]=status;return;
    }
    if(!descriptor.pixel_count){lane_records[id]=status;return;}
    auto lane=descriptor.cursor?states[id]:mb3d_native::native_initial_state(float3(0),as_type<float>(scene.de_stop),program);
    mb3d_native::NativeResources resources{maps,pixels,resource_counts.x,resource_counts.y,nullptr};
    for(uint k=0;k<descriptor.pixel_count;++k){
        const uint ordinal=descriptor.cursor+k,x=ordinal%scene.width;
        const uint y=descriptor.row_start+ordinal/scene.width,index=y*scene.width+x;
        auto surface=input[index];mb3d_native::NativeSoftShadowRecord record{};
        surface=mb3d_native::camera_soft_shadow_pixel(primary[index],surface,y,scene,program,params,soft,resources,lane,record,camera,inside);
        output[index]=surface;records[index]=record;++status.processed;++status.next_cursor;
        if(record.total.fault){status.fault=record.total.fault;break;}
    }
    states[id]=lane;lane_records[id]=status;
}

// Soft-shadow pixel entry. Every pixel retains its complete ordered
// initial/refinement/march queries; no neighboring row cache exists in
// camera_soft_shadow_pixel and its start query sets inside. Proof 1 is the
// device-checked chain; 5 (state contract) and 6 (legacy pixel qualification)
// are host proofs for programs whose rows already start fresh lanes.
kernel void native_camera_soft_shadow_pixels(device const ExactPixel* primary [[buffer(0)]],
    device const ExactSurface* input [[buffer(1)]],device ExactSurface* output [[buffer(2)]],
    device mb3d_native::NativeSoftShadowRecord* records [[buffer(3)]],constant ExactScene& scene [[buffer(4)]],
    constant mb3d_native::NativeFormulaProgram& program [[buffer(5)]],constant ExactShadowParams& params [[buffer(6)]],
    device const mb3d_native::NativeMapDescriptor* maps [[buffer(7)]],device const float* pixels [[buffer(8)]],
    constant uint4& launch [[buffer(9)]],constant uint2& resource_counts [[buffer(10)]],
    constant mb3d_camera::Params& camera [[buffer(23)]],constant mb3d_inside::Params& inside [[buffer(27)]],
    constant mb3d_native::NativeSoftShadowConfig& soft [[buffer(28)]],constant uint& proof [[buffer(29)]],
    uint local [[thread_position_in_grid]]) {
    if(local>=launch.y||launch.x>=launch.z||local>=launch.z-launch.x)return;
    const uint index=launch.x+local;
    ExactSurface surface{};mb3d_native::NativeSoftShadowRecord record{};
    const bool chain=proof==1&&mb3d_native::SoftShadowPixelChain(program)&&!inside.initial_inside&&!inside.in_and_out&&
        !resource_counts.x&&!resource_counts.y;
    if(launch.w||!scene.width||scene.width>4096||!scene.height||scene.height>4096||
       ulong(scene.width)*scene.height!=launch.z||
       !mb3d_native::valid_soft_shadow(scene,program,params,soft)||!(chain||proof==5||proof==6)) {
        record.version=mb3d_native::kNativeSoftShadowVersion;
        record.total.version=mb3d_native::kGenericShadowVersion;
        record.total.eval.version=mb3d_native::kGeneralGeometryVersion;
        record.total.fault=surface.fault=1;
    }else{
        mb3d_native::NativeResources resources{maps,pixels,resource_counts.x,resource_counts.y,nullptr};
        auto lane=mb3d_native::native_initial_state(float3(0),as_type<float>(scene.de_stop),program);
        surface=mb3d_native::camera_soft_shadow_pixel(primary[index],input[index],index/scene.width,
            scene,program,params,soft,resources,lane,record,camera,inside);
    }
    output[index]=surface;records[index]=record;
}
kernel void native_camera_soft_shadow_pixel_abi(device uint* words [[buffer(0)]],uint id [[thread_position_in_grid]]) {
    if(id)return;
    words[0]=2;words[1]=sizeof(mb3d_native::NativeSoftShadowRecord);
    words[2]=sizeof(ExactSurface);words[3]=mb3d_native::kGeneralGeometryVersion;
    words[4]=32;
}
