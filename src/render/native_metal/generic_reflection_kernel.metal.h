#pragma once
#include "generic_reflection_math.metal.h"

kernel void native_general_reflection_abi(device uint* words [[buffer(0)]],uint tid [[thread_position_in_grid]]){
    if(tid)return;words[0]=mb3d_native::kGenericReflectionVersion;words[1]=sizeof(NativeReflectionParams);
    words[2]=sizeof(mb3d_native::NativeGenericReflectionRecord);
    words[3]=sizeof(mb3d_native::NativeLogicalLane);words[4]=sizeof(mb3d_native::NativeFormulaState);
    words[5]=sizeof(mb3d_native::NativeLogicalLaneRecord);words[6]=mb3d_native::kLogicalLaneBatchPixels;
}
kernel void native_general_reflection_lanes(constant ExactScene& scene [[buffer(0)]],
    constant mb3d_native::NativeSurfaceParams& packing [[buffer(1)]],constant ExactShadowParams& shadow [[buffer(2)]],
    constant ExactColorScene& color [[buffer(3)]],constant NativeReflectionParams& params [[buffer(4)]],
    device const ExactPixel* primary [[buffer(5)]],device const ExactSurface* surface [[buffer(6)]],
    device ExactColorPixel* rgb [[buffer(7)]],device mb3d_native::NativeGenericReflectionRecord* records [[buffer(8)]],
    constant uint4& launch [[buffer(9)]],constant mb3d_native::NativeFormulaProgram& program [[buffer(10)]],
    device const mb3d_native::NativeMapDescriptor* maps [[buffer(11)]],device const float* pixels [[buffer(12)]],
    constant uint2& resource_counts [[buffer(13)]],device const mb3d_native::NativeLogicalLane* descriptors [[buffer(14)]],
    device mb3d_native::NativeFormulaState* states [[buffer(15)]],
    device mb3d_native::NativeLogicalLaneRecord* lane_records [[buffer(16)]],
    constant uint& batch_rows [[buffer(17)]],uint local [[thread_position_in_grid]]){
    if(local>=launch.y||launch.x>=launch.z||local>=launch.z-launch.x)return;
    const uint id=launch.x+local;const auto descriptor=descriptors[id];
    mb3d_native::NativeLogicalLaneRecord status{1,0,0,descriptor.cursor};
    if(!batch_rows||batch_rows>scene.height||launch.z!=(scene.height+batch_rows-1)/batch_rows||
       !mb3d_native::valid_logical_lane(descriptor,scene.width,scene.height,launch.w)||
       descriptor.row_start!=id*batch_rows||descriptor.row_stride!=1||
       descriptor.row_count!=metal::min(batch_rows,scene.height-descriptor.row_start)||
       !mb3d_generic_reflection::valid(params,scene,program)){
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
        mb3d_native::NativeNormalParams normal_params{mb3d_native::kGeneralNormalVersion,1,0,uint(program.inside!=0),{0,0,0,0}};
        mb3d_native::NativeNormalRecord normal_record{};normal_record.version=mb3d_native::kGeneralNormalVersion;
        mb3d_native::NativeNormalResult normal_result{};
        mb3d_generic_reflection::Context ctx{&program,&resources,&lane,&record.eval,normal_params,&normal_record,&normal_result};
        const auto value=mb3d_generic_reflection::reflect_pixel(scene,packing,shadow,color,params,
            primary[index],surface[index],rgb[index],index,record.activity,ctx);
        record.activity.fault|=record.eval.fault;
        if(record.eval.fault&4u)record.activity.nonfinite=1;
        if(!record.activity.fault)rgb[index]=value;
        records[index]=record;++status.processed;++status.next_cursor;
        if(record.activity.fault){status.fault=record.activity.fault;break;}
    }
    states[id]=lane;lane_records[id]=status;
}

kernel void native_general_reflection_lanes_with_normals(constant ExactScene& scene [[buffer(0)]],
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
    constant mb3d_native::NativeNormalParams& normal_params [[buffer(20)]],
    device mb3d_native::NativeNormalRecord* normal_records [[buffer(21)]],
    device mb3d_native::NativeNormalResult* normal_results [[buffer(22)]],uint local [[thread_position_in_grid]]){
    if(local>=launch.y||launch.x>=launch.z||local>=launch.z-launch.x)return;
    const uint id=launch.x+local;const auto descriptor=descriptors[id];
    mb3d_native::NativeLogicalLaneRecord status{1,0,0,descriptor.cursor};
    if(!batch_rows||batch_rows>scene.height||launch.z!=(scene.height+batch_rows-1)/batch_rows||
       !mb3d_native::valid_logical_lane(descriptor,scene.width,scene.height,launch.w)||
       descriptor.row_start!=id*batch_rows||descriptor.row_stride!=1||
       descriptor.row_count!=metal::min(batch_rows,scene.height-descriptor.row_start)||
       !mb3d_generic_reflection::valid(params,scene,program)||!mb3d_native::valid_general_normals(normal_params,program)){
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
        mb3d_native::NativeNormalRecord normal_record{};normal_record.version=mb3d_native::kGeneralNormalVersion;
        mb3d_native::NativeNormalResult normal_result{};
        mb3d_generic_reflection::Context ctx{&program,&resources,&lane,&record.eval,normal_params,&normal_record,&normal_result};
        const auto value=mb3d_generic_reflection::reflect_pixel(scene,packing,shadow,color,params,
            primary[index],surface[index],rgb[index],index,record.activity,ctx);
        record.activity.fault|=record.eval.fault;
        if(record.eval.fault&4u)record.activity.nonfinite=1;
        if(!record.activity.fault)rgb[index]=value;
        records[index]=record;normal_records[index]=normal_record;normal_results[index]=normal_result;++status.processed;++status.next_cursor;
        if(record.activity.fault){status.fault=record.activity.fault;break;}
    }
    states[id]=lane;lane_records[id]=status;
}
