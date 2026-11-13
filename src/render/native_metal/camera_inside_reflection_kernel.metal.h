#pragma once
#include "camera_inside_reflection_math.metal.h"

kernel void native_general_inside_reflection_abi(device uint* words [[buffer(0)]],uint tid [[thread_position_in_grid]]){
    // Version 3 binds the reflection material descriptor (buffer 19) and its
    // map samples (buffer 24); version 4 defers shading to shade slots (buffer
    // 25); version 5 binds the soft-shadow descriptor (buffer 28, version 0
    // for hard shadows); version 6 sizes shade records by their event
    // capacity (buffer 26) for transmission trees. Older libraries must fail
    // before dispatch.
    if(tid)return;words[0]=6;words[1]=sizeof(mb3d_inside::Params);words[2]=sizeof(mb3d_inside::ReflectionConfig);
    words[3]=sizeof(mb3d_inside::ReflectionRecord);words[4]=sizeof(mb3d_camera::Params);words[5]=sizeof(mb3d_native::NativeInteriorColorParams);words[6]=32;
    words[7]=sizeof(mb3d_native::NativeReflectionMaterial);words[8]=sizeof(mb3d_native::NativeReflectionShadeRecord);
    words[9]=sizeof(mb3d_native::NativeReflectionShadeEvent);
}
// Shade records are a header followed by layout.y events; layout.x is a
// logical lane's slot stride.
inline device mb3d_native::NativeReflectionShadeRecord* shade_record(device mb3d_native::NativeReflectionShadeRecord* shade,
                                                                    uint slot,uint capacity){
    return reinterpret_cast<device mb3d_native::NativeReflectionShadeRecord*>(
        reinterpret_cast<device uchar*>(shade)+ulong(slot)*mb3d_native::native_reflection_shade_bytes(capacity));
}
kernel void native_camera_inside_reflection_lanes(constant ExactScene& scene [[buffer(0)]],
    constant mb3d_native::NativeSurfaceParams& packing [[buffer(1)]],constant ExactShadowParams& shadow [[buffer(2)]],
    constant ExactColorScene& color [[buffer(3)]],constant NativeReflectionParams& params [[buffer(4)]],
    device const ExactPixel* primary [[buffer(5)]],device const ExactSurface* surface [[buffer(6)]],
    device ExactColorPixel* rgb [[buffer(7)]],device mb3d_native::NativeGenericReflectionRecord* records [[buffer(8)]],
    constant uint4& launch [[buffer(9)]],constant mb3d_native::NativeFormulaProgram& program [[buffer(10)]],
    device const mb3d_native::NativeMapDescriptor* maps [[buffer(11)]],device const float* pixels [[buffer(12)]],
    constant uint2& resource_counts [[buffer(13)]],device const mb3d_native::NativeLogicalLane* descriptors [[buffer(14)]],
    device mb3d_native::NativeFormulaState* states [[buffer(15)]],
    device mb3d_native::NativeLogicalLaneRecord* lane_records [[buffer(16)]],
    constant uint& batch_rows [[buffer(17)]],
    constant mb3d_native::NativeInteriorColorParams& interior [[buffer(18)]],
    constant mb3d_native::NativeReflectionMaterial& material [[buffer(19)]],
    constant mb3d_native::NativeNormalParams& normal_params [[buffer(20)]],
    device mb3d_native::NativeNormalRecord* normal_records [[buffer(21)]],
    device mb3d_native::NativeNormalResult* normal_results [[buffer(22)]],
    constant mb3d_camera::Params& camera [[buffer(23)]],
    device const float* material_samples [[buffer(24)]],
    device mb3d_native::NativeReflectionShadeRecord* shade [[buffer(25)]],constant uint2& shade_layout [[buffer(26)]],
    constant mb3d_inside::Params& inside [[buffer(27)]],
    constant mb3d_native::NativeSoftShadowConfig& soft [[buffer(28)]],
    constant mb3d_inside::ReflectionConfig& inside_config [[buffer(29)]],
    device mb3d_inside::ReflectionRecord* inside_records [[buffer(30)]],uint local [[thread_position_in_grid]]){
    if(local>=launch.y||launch.x>=launch.z||local>=launch.z-launch.x)return;
    const uint id=launch.x+local;const auto descriptor=descriptors[id];
    mb3d_native::NativeLogicalLaneRecord status{1,0,0,descriptor.cursor};
    if(!batch_rows||batch_rows>scene.height||launch.z!=(scene.height+batch_rows-1)/batch_rows||
       !mb3d_native::valid_logical_lane(descriptor,scene.width,scene.height,launch.w)||
       descriptor.row_start!=id*batch_rows||descriptor.row_stride!=1||
       descriptor.row_count!=metal::min(batch_rows,scene.height-descriptor.row_start)||descriptor.pixel_count>shade_layout.x||
       !mb3d_camera_inside_reflection::valid(params,scene,program)||!mb3d_native::valid_general_normals(normal_params,program)){
        status.fault=1;lane_records[id]=status;return;
    }
    if(!descriptor.pixel_count){lane_records[id]=status;return;}
    // CalcReflectionsRows clears the iteration record once per task; subsequent
    // foreground pixels apply IniIt3D's partial reset in reflect_pixel.
    auto lane=descriptor.cursor?states[id]:mb3d_native::NativeFormulaState{};
    mb3d_native::NativeResources resources{maps,pixels,resource_counts.x,resource_counts.y,nullptr};
    for(uint k=0;k<descriptor.pixel_count;++k){
        const uint ordinal=descriptor.cursor+k,x=ordinal%scene.width;
        const uint y=descriptor.row_start+ordinal/scene.width,index=y*scene.width+x;
        mb3d_native::NativeGenericReflectionRecord record{};record.eval.version=mb3d_native::kGeneralGeometryVersion;
        mb3d_inside::ReflectionRecord inside_record{};inside_record.version=1;
        mb3d_native::NativeNormalRecord normal_record{};normal_record.version=mb3d_native::kGeneralNormalVersion;
        mb3d_native::NativeNormalResult normal_result{};
        mb3d_camera_inside_reflection::Context ctx{&program,&resources,&lane,&record.eval,normal_params,&normal_record,&normal_result,&inside,&inside_config,&camera,&interior,&inside_record,0,false,&material,material_samples,&soft};
        device auto* slot=shade_record(shade,local*shade_layout.x+k,shade_layout.y);
        mb3d_camera_inside_reflection::reflect_pixel(scene,packing,shadow,color,params,
            primary[index],surface[index],rgb[index],index,record.activity,ctx,slot,shade_layout.y);
        record.activity.fault|=record.eval.fault;
        if(record.eval.fault&4u)record.activity.nonfinite=1;
        if(record.activity.fault)slot->count=0;
        inside_record.fault=record.activity.fault;inside_records[index]=inside_record;
        records[index]=record;normal_records[index]=normal_record;normal_results[index]=normal_result;++status.processed;++status.next_cursor;
        if(record.activity.fault){status.fault=record.activity.fault;break;}
    }
    states[id]=lane;lane_records[id]=status;
}

// Independent rays keep all dependent bounces and samples within one thread.
kernel void native_camera_inside_reflection_pixels(constant ExactScene& scene [[buffer(0)]],
    constant mb3d_native::NativeSurfaceParams& packing [[buffer(1)]],constant ExactShadowParams& shadow [[buffer(2)]],
    constant ExactColorScene& color [[buffer(3)]],constant NativeReflectionParams& params [[buffer(4)]],
    device const ExactPixel* primary [[buffer(5)]],device const ExactSurface* surface [[buffer(6)]],
    device ExactColorPixel* rgb [[buffer(7)]],device mb3d_native::NativeGenericReflectionRecord* records [[buffer(8)]],
    constant uint4& launch [[buffer(9)]],constant mb3d_native::NativeFormulaProgram& program [[buffer(10)]],
    device const mb3d_native::NativeMapDescriptor* maps [[buffer(11)]],device const float* pixels [[buffer(12)]],
    constant uint2& resource_counts [[buffer(13)]],device const mb3d_native::NativeLogicalLane* descriptors [[buffer(14)]],
    device mb3d_native::NativeFormulaState* states [[buffer(15)]],
    device mb3d_native::NativeLogicalLaneRecord* lane_records [[buffer(16)]],
    constant uint& batch_rows [[buffer(17)]],
    constant mb3d_native::NativeInteriorColorParams& interior [[buffer(18)]],
    constant mb3d_native::NativeReflectionMaterial& material [[buffer(19)]],
    constant mb3d_native::NativeNormalParams& normal_params [[buffer(20)]],
    device mb3d_native::NativeNormalRecord* normal_records [[buffer(21)]],
    device mb3d_native::NativeNormalResult* normal_results [[buffer(22)]],
    constant mb3d_camera::Params& camera [[buffer(23)]],
    device const float* material_samples [[buffer(24)]],
    device mb3d_native::NativeReflectionShadeRecord* shade [[buffer(25)]],constant uint2& shade_layout [[buffer(26)]],
    constant mb3d_inside::Params& inside [[buffer(27)]],
    constant mb3d_native::NativeSoftShadowConfig& soft [[buffer(28)]],
    constant mb3d_inside::ReflectionConfig& inside_config [[buffer(29)]],
    device mb3d_inside::ReflectionRecord* inside_records [[buffer(30)]],uint local [[thread_position_in_grid]]){
    if(local>=launch.y||launch.x>=launch.z||local>=launch.z-launch.x)return;
    const uint index=launch.x+local;
    if(launch.z!=scene.width*scene.height || index>=launch.z)return;
    auto lane=mb3d_native::NativeFormulaState{};
    mb3d_native::NativeResources resources{maps,pixels,resource_counts.x,resource_counts.y,nullptr};
        mb3d_native::NativeGenericReflectionRecord record{};record.eval.version=mb3d_native::kGeneralGeometryVersion;
        mb3d_inside::ReflectionRecord inside_record{};inside_record.version=1;
        mb3d_native::NativeNormalRecord normal_record{};normal_record.version=mb3d_native::kGeneralNormalVersion;
        mb3d_native::NativeNormalResult normal_result{};
        mb3d_camera_inside_reflection::Context ctx{&program,&resources,&lane,&record.eval,normal_params,&normal_record,&normal_result,&inside,&inside_config,&camera,&interior,&inside_record,0,false,&material,material_samples,&soft};
        device auto* slot=shade_record(shade,local,shade_layout.y);
        mb3d_camera_inside_reflection::reflect_pixel(scene,packing,shadow,color,params,
            primary[index],surface[index],rgb[index],index,record.activity,ctx,slot,shade_layout.y);
        if(!mb3d_native::valid_general_normals(normal_params,program))record.eval.fault|=1u;
        record.activity.fault|=record.eval.fault;
        if(record.eval.fault&4u)record.activity.nonfinite=1;
        if(record.activity.fault)slot->count=0;
        inside_record.fault=record.activity.fault;inside_records[index]=inside_record;
        records[index]=record;normal_records[index]=normal_record;normal_results[index]=normal_result;

}

// Formula-free shade pass over one trace dispatch's slots. A slot names its
// pixel and is consumed (version cleared), so a slot a lane never reached in
// this dispatch cannot be shaded again. Faults join the pixel's trace record.
kernel void native_camera_inside_reflection_shade(constant ExactColorScene& color [[buffer(3)]],
    device ExactColorPixel* rgb [[buffer(7)]],device mb3d_native::NativeGenericReflectionRecord* records [[buffer(8)]],
    constant uint4& launch [[buffer(9)]],constant mb3d_native::NativeInteriorColorParams& interior [[buffer(18)]],
    constant mb3d_native::NativeReflectionMaterial& material [[buffer(19)]],device const float* material_samples [[buffer(24)]],
    device mb3d_native::NativeReflectionShadeRecord* shade [[buffer(25)]],constant uint2& shade_layout [[buffer(26)]],
    device mb3d_inside::ReflectionRecord* inside_records [[buffer(30)]],uint slot [[thread_position_in_grid]]){
    if(slot>=launch.y)return;
    device auto* in=shade_record(shade,slot,shade_layout.y);
    if(in->version!=mb3d_native::kNativeReflectionShadeVersion)return;
    in->version=0;
    const uint pixel=in->pixel;
    if(pixel>=launch.z||!in->count)return;
    NativeReflectionRecord record{};
    if(in->capacity!=shade_layout.y)record.fault|=1;
    const auto value=record.fault?rgb[pixel]:
        mb3d_camera_inside_reflection::shade_pixel(color,interior,material,material_samples,in,rgb[pixel],record);
    device auto& activity=records[pixel].activity;
    activity.fault|=record.fault;activity.nonfinite|=record.nonfinite;activity.changed_pixels=record.changed_pixels;
    inside_records[pixel].fault|=record.fault;
    if(!record.fault)rgb[pixel]=value;
}

// Bounded material witness: same helper used by secondary rays, independent of
// marching. Returns unquantized radiance, specular, diffuse and fault state.
kernel void native_inside_material_probe(constant ExactColorScene& color [[buffer(0)]],
    constant mb3d_native::NativeInteriorColorParams& interior [[buffer(1)]],
    constant ExactSurface& surface [[buffer(2)]],constant float4* inputs [[buffer(3)]],
    device float4* output [[buffer(4)]],uint tid [[thread_position_in_grid]]) {
    if(tid)return;
    NativeReflectionRecord record{};
    const auto value=mb3d_camera_inside_reflection::inside_material(color,interior,surface,
        inputs[0].xyz,inputs[1].x,inputs[1].y,inputs[1].z,inputs[0].w,record,inputs[1].w!=0.f);
    output[0]=float4(value.radiance,value.transparency);
    output[1]=float4(value.specular,0.f);
    output[2]=float4(value.diffuse,as_type<float>(record.fault));
}
// The same witness through the reflection material descriptor; inputs[2].xyz
// is the absolute hit position.
kernel void native_inside_material_descriptor_probe(constant ExactColorScene& color [[buffer(0)]],
    constant mb3d_native::NativeInteriorColorParams& interior [[buffer(1)]],
    constant ExactSurface& surface [[buffer(2)]],constant float4* inputs [[buffer(3)]],
    device float4* output [[buffer(4)]],constant mb3d_native::NativeReflectionMaterial& material [[buffer(5)]],
    device const float* samples [[buffer(6)]],uint tid [[thread_position_in_grid]]) {
    if(tid)return;
    NativeReflectionRecord record{};
    const auto value=mb3d_camera_inside_reflection::inside_material(color,interior,surface,
        inputs[0].xyz,inputs[1].x,inputs[1].y,inputs[1].z,inputs[0].w,record,inputs[1].w!=0.f,
        &material,samples,inputs[2].xyz,inputs[3].xyz,inputs[4].xyz);
    output[0]=float4(value.radiance,value.transparency);
    output[1]=float4(value.specular,0.f);
    output[2]=float4(value.diffuse,as_type<float>(record.fault));
}
