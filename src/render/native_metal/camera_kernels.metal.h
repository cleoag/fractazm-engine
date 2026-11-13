#pragma once
#include "general_geometry.metal.h"
#include "primary_inside_bridge.metal.h"
#include "camera_color.metal.h"

kernel void native_camera_inside_abi(device uint* words [[buffer(0)]],uint id [[thread_position_in_grid]]){
    if(id)return;words[0]=mb3d_inside::Version;words[1]=sizeof(mb3d_inside::Params);
    words[2]=sizeof(mb3d_inside::Record);words[3]=sizeof(mb3d_camera::Params);
    words[4]=sizeof(mb3d_camera::Ray);words[5]=sizeof(mb3d_native::NativeNormalParams);
}
kernel void native_camera_abi(device uint* words [[buffer(0)]],uint id [[thread_position_in_grid]]){
    if(id)return;words[0]=mb3d_camera::Version;words[1]=sizeof(mb3d_camera::Params);
    words[2]=sizeof(mb3d_camera::Axis);words[3]=sizeof(mb3d_camera::Ray);words[4]=sizeof(mb3d_camera::Paint);
    words[5]=sizeof(mb3d_camera::CutSurface);words[6]=sizeof(mb3d_camera::DepthPacking);
}
kernel void native_volume_light_abi(device uint* words [[buffer(0)]],uint id [[thread_position_in_grid]]){
    if(id)return;words[0]=mb3d_native::kNativeVolumeLightVersion;words[1]=sizeof(mb3d_native::NativeVolumeLight);
    words[2]=sizeof(mb3d_native::NativeVolumeCell);words[3]=mb3d_native::kNativeVolumeMapSteps;
}
// One independent thread per volumetric map cell, face-major; a fresh
// formula lane per cell (only independent-state programs are admitted).
kernel void native_volume_light_map(device float* cells [[buffer(0)]],
    device mb3d_native::NativeVolumeCell* cell_records [[buffer(1)]],constant ExactScene& scene [[buffer(2)]],
    constant mb3d_native::NativeFormulaProgram& program [[buffer(3)]],
    constant mb3d_native::NativeVolumeLight& volume [[buffer(4)]],
    device const mb3d_native::NativeMapDescriptor* maps [[buffer(6)]],device const float* pixels [[buffer(7)]],
    constant uint4& launch [[buffer(8)]],constant uint2& resource_counts [[buffer(9)]],
    uint local [[thread_position_in_grid]]) {
    if(local>=launch.y||launch.x>=launch.z||local>=launch.z-launch.x)return;
    const uint index=launch.x+local;
    mb3d_native::NativeVolumeCell cell{};float value=0.f;
    if(launch.w||!mb3d_native::valid_general_scene(scene,program)||scene.fog_iteration!=mb3d_native::kNativeVolumeFog||
       !mb3d_native::valid_volume_light(volume)||ulong(volume.cube_size)*volume.cube_size*volume.side_count!=launch.z)cell.fault=1;
    else {
        const uint area=volume.cube_size*volume.cube_size,side=index/area,at=index%area;
        mb3d_native::NativeGeometryRecord record{};record.version=mb3d_native::kGeneralGeometryVersion;
        mb3d_native::NativeResources resources{maps,pixels,resource_counts.x,resource_counts.y,nullptr};
        auto lane=mb3d_native::native_initial_state(float3(0),as_type<float>(scene.de_stop),program);
        lane.inside=as_type<uint>(volume.inside);
        value=mb3d_native::volume_map_ray(scene,program,volume,side,int(at%volume.cube_size)-volume.half_size,
            int(at/volume.cube_size)-volume.half_size,resources,lane,record);
        cell.fault=record.fault;cell.range_exits=record.fp32_range_exits;
        if(!cell.fault&&!metal::isfinite(value))cell.fault=4;
    }
    cells[index]=value;cell_records[index]=cell;
}
kernel void native_camera_general_primary(device ExactPixel* primary [[buffer(0)]],
    device mb3d_native::NativeGeometryRecord* records [[buffer(1)]],constant ExactScene& scene [[buffer(2)]],
    constant mb3d_native::NativeFormulaProgram& program [[buffer(3)]],
    device const mb3d_native::NativeMapDescriptor* maps [[buffer(6)]],device const float* pixels [[buffer(7)]],
    constant uint4& launch [[buffer(8)]],constant uint2& resource_counts [[buffer(9)]],
    constant mb3d_native::NativeNormalParams& normal_params [[buffer(20)]],
    device mb3d_native::NativeNormalRecord* normal_records [[buffer(21)]],
    device mb3d_native::NativeNormalResult* normal_results [[buffer(22)]],
    constant mb3d_camera::Params& camera [[buffer(23)]],device const mb3d_camera::Axis* columns [[buffer(24)]],
    device const mb3d_camera::Axis* rows [[buffer(25)]],device mb3d_camera::Ray* rays [[buffer(26)]],
    constant mb3d_inside::Params& inside [[buffer(27)]],device mb3d_inside::Record* inside_records [[buffer(28)]],
    constant mb3d_native::NativeVolumeLight& volume [[buffer(29)]],device const float* volume_map [[buffer(30)]],
    constant mb3d_native::NativePrimaryRandom& random [[buffer(13)]],device uint* random_words [[buffer(14)]],
    uint local [[thread_position_in_grid]]) {
    if(local>=launch.y||launch.x>=launch.z||local>=launch.z-launch.x)return;
    const uint index=launch.x+local;
    mb3d_native::NativeGeometryRecord record{};record.version=mb3d_native::kGeneralGeometryVersion;
    mb3d_native::NativeNormalRecord normal_record{};normal_record.version=mb3d_native::kGeneralNormalVersion;
    mb3d_native::NativeNormalResult normal_result{};
    ExactPixel value{};mb3d_camera::Ray ray{};ray.version=mb3d_camera::Version;
    mb3d_inside::Record inside_record{};inside_record.version=mb3d_inside::Version;
    const bool seeded=mb3d_native::native_primary_random();
    if(launch.w||!mb3d_camera::valid_camera(camera)||camera.width!=scene.width||camera.height!=scene.height||
       ulong(scene.width)*scene.height!=launch.z||!mb3d_native::valid_primary_random(random,scene.height)||
       random.mode!=(seeded?mb3d_native::PrimaryRandomSeeded:mb3d_native::PrimaryRandomOff)){record.fault=value.fault=ray.fault=1;}
    else {
        const uint x=index%scene.width,y=index/scene.width;
        ray=mb3d_camera::camera_ray(camera,columns[x],rows[y],x,y,mb3d_native::general_double(scene.max_ray_length));
        mb3d_native::NativeResources resources{maps,pixels,resource_counts.x,resource_counts.y,nullptr};
        resources.volume=&volume;resources.volume_map=volume_map;
        auto lane=mb3d_native::native_initial_state(float3(0),as_type<float>(scene.de_stop),program);
        const uint task=seeded?y%random.workers:0u;
        auto jitter=mb3d_native::primary_random_state(seeded?random_words[index]:0u,task);
        value=mb3d_native::general_primary_inside(scene,program,inside,ray,x,y,resources,record,lane,inside_record,
            normal_params,normal_record,normal_result,&jitter);
        // The next pixel of this CPU task entered with this pixel's exit seed;
        // a task's last pixel compares with its slot after the pixel words.
        if(seeded&&!record.fault){
            const uint next=x+1<scene.width?index+1:y+random.workers<scene.height?index+1+(random.workers-1)*scene.width:
                launch.z+task;
            if(random_words[next]!=jitter.seed)record.fault=value.fault=1;
        }
    }
    inside_record.fault|=record.fault;inside_records[index]=inside_record;
    primary[index]=value;records[index]=record;rays[index]=ray;normal_records[index]=normal_record;normal_results[index]=normal_result;
}
// Primary jitter probe (general_geometry_abi.h): one word per pixel, 1 when
// its final attempt marches, 0 when it starts inside, 2 on a fault. Only the
// attempts' initial DE queries run; the host turns the words into seeds.
kernel void native_camera_general_primary_probe(constant ExactScene& scene [[buffer(2)]],
    constant mb3d_native::NativeFormulaProgram& program [[buffer(3)]],
    device const mb3d_native::NativeMapDescriptor* maps [[buffer(6)]],device const float* pixels [[buffer(7)]],
    constant uint4& launch [[buffer(8)]],constant uint2& resource_counts [[buffer(9)]],
    constant mb3d_native::NativePrimaryRandom& random [[buffer(13)]],device uint* random_words [[buffer(14)]],
    constant mb3d_camera::Params& camera [[buffer(23)]],device const mb3d_camera::Axis* columns [[buffer(24)]],
    device const mb3d_camera::Axis* rows [[buffer(25)]],
    constant mb3d_inside::Params& inside [[buffer(27)]],
    constant mb3d_native::NativeVolumeLight& volume [[buffer(29)]],device const float* volume_map [[buffer(30)]],
    uint local [[thread_position_in_grid]]) {
    if(local>=launch.y||launch.x>=launch.z||local>=launch.z-launch.x)return;
    const uint index=launch.x+local;
    if(launch.w||!mb3d_camera::valid_camera(camera)||camera.width!=scene.width||camera.height!=scene.height||
       ulong(scene.width)*scene.height!=launch.z||!mb3d_native::valid_primary_random(random,scene.height)||
       random.mode!=mb3d_native::PrimaryRandomProbe){random_words[index]=2u;return;}
    const uint x=index%scene.width,y=index/scene.width;
    const auto ray=mb3d_camera::camera_ray(camera,columns[x],rows[y],x,y,mb3d_native::general_double(scene.max_ray_length));
    mb3d_native::NativeGeometryRecord record{};record.version=mb3d_native::kGeneralGeometryVersion;
    mb3d_native::NativeResources resources{maps,pixels,resource_counts.x,resource_counts.y,nullptr};
    resources.volume=&volume;resources.volume_map=volume_map;
    auto lane=mb3d_native::native_initial_state(float3(0),as_type<float>(scene.de_stop),program);
    random_words[index]=ray.fault?2u:mb3d_native::general_primary_probe(scene,program,inside,ray,resources,record,lane);
}
kernel void native_camera_general_primary_lanes(device ExactPixel* primary [[buffer(0)]],
    device mb3d_native::NativeGeometryRecord* records [[buffer(1)]],constant ExactScene& scene [[buffer(2)]],
    constant mb3d_native::NativeFormulaProgram& program [[buffer(3)]],
    device const mb3d_native::NativeMapDescriptor* maps [[buffer(6)]],device const float* pixels [[buffer(7)]],
    constant uint4& launch [[buffer(8)]],constant uint2& resource_counts [[buffer(9)]],
    device const mb3d_native::NativeLogicalLane* descriptors [[buffer(10)]],
    device mb3d_native::NativeFormulaState* states [[buffer(11)]],
    device mb3d_native::NativeLogicalLaneRecord* lane_records [[buffer(12)]],
    constant mb3d_native::NativeNormalParams& normal_params [[buffer(20)]],
    device mb3d_native::NativeNormalRecord* normal_records [[buffer(21)]],
    device mb3d_native::NativeNormalResult* normal_results [[buffer(22)]],
    constant mb3d_camera::Params& camera [[buffer(23)]],device const mb3d_camera::Axis* columns [[buffer(24)]],
    device const mb3d_camera::Axis* rows [[buffer(25)]],device mb3d_camera::Ray* rays [[buffer(26)]],
    constant mb3d_inside::Params& inside [[buffer(27)]],device mb3d_inside::Record* inside_records [[buffer(28)]],
    constant mb3d_native::NativeVolumeLight& volume [[buffer(29)]],device const float* volume_map [[buffer(30)]],
    constant mb3d_native::NativePrimaryRandom& random [[buffer(13)]],device uint* random_words [[buffer(14)]],
    uint local [[thread_position_in_grid]]) {
    if(local>=launch.y||launch.x>=launch.z||local>=launch.z-launch.x)return;
    const uint id=launch.x+local;const auto descriptor=descriptors[id];
    mb3d_native::NativeLogicalLaneRecord status{1,0,0,descriptor.cursor};
    const bool carried=mb3d_native::native_primary_random();
    if(!mb3d_native::valid_logical_lane(descriptor,scene.width,scene.height,launch.w)||
       launch.z>scene.height||descriptor.row_start!=id||descriptor.row_stride!=launch.z||
       descriptor.row_count!=1+(scene.height-1-id)/launch.z||
       !mb3d_native::valid_general_scene(scene,program)||!mb3d_native::valid_general_normals(normal_params,program)||!mb3d_camera::valid_camera(camera)||
       camera.width!=scene.width||camera.height!=scene.height||!mb3d_native::valid_primary_random(random,scene.height)||
       random.mode!=(carried?mb3d_native::PrimaryRandomCarried:mb3d_native::PrimaryRandomOff)||
       (carried&&random.workers!=launch.z)){status.fault=1;lane_records[id]=status;return;}
    if(!descriptor.pixel_count){lane_records[id]=status;return;}
    auto lane=descriptor.cursor?states[id]:
        mb3d_native::native_initial_state(float3(0),as_type<float>(scene.de_stop),program);
    // The task's running seed survives between bounded batches after the pixel words.
    uint seed=!carried?0u:descriptor.cursor?random_words[launch.w+id]:mb3d_native::primary_random_task_seed(id);
    mb3d_native::NativeResources resources{maps,pixels,resource_counts.x,resource_counts.y,nullptr};
    resources.volume=&volume;resources.volume_map=volume_map;
    for(uint k=0;k<descriptor.pixel_count;++k){
        const uint ordinal=descriptor.cursor+k,x=ordinal%scene.width;
        const uint y=descriptor.row_start+(ordinal/scene.width)*descriptor.row_stride,index=y*scene.width+x;
        const auto ray=mb3d_camera::camera_ray(camera,columns[x],rows[y],x,y,mb3d_native::general_double(scene.max_ray_length));
        mb3d_native::NativeGeometryRecord record{};
        mb3d_inside::Record inside_record{};inside_record.version=mb3d_inside::Version;
        mb3d_native::NativeNormalRecord normal_record{};mb3d_native::NativeNormalResult normal_result{};
        auto jitter=mb3d_native::primary_random_state(seed,id);
        const auto value=mb3d_native::general_primary_inside(scene,program,inside,ray,x,y,resources,record,lane,inside_record,
            normal_params,normal_record,normal_result,&jitter);
        seed=jitter.seed;
        inside_record.fault|=record.fault;inside_records[index]=inside_record;
    primary[index]=value;records[index]=record;rays[index]=ray;normal_records[index]=normal_record;normal_results[index]=normal_result;++status.processed;++status.next_cursor;
        if(record.fault){status.fault=record.fault;break;}
    }
    if(carried)random_words[launch.w+id]=seed;
    states[id]=lane;lane_records[id]=status;
}
kernel void native_camera_general_surface(device const ExactPixel* primary [[buffer(0)]],
    device ExactSurface* output [[buffer(1)]],constant ExactScene& scene [[buffer(2)]],
    constant mb3d_native::NativeSurfaceParams& packing [[buffer(3)]],constant mb3d_native::NativeFormulaProgram& program [[buffer(4)]],
    constant uint4& launch [[buffer(5)]],device const mb3d_native::NativeGeometryRecord* geometry [[buffer(6)]],
    constant mb3d_native::NativeNormalParams& normal_params [[buffer(20)]],
    device const mb3d_native::NativeNormalRecord* normal_records [[buffer(21)]],
    device const mb3d_native::NativeNormalResult* normal_results [[buffer(22)]],
    constant mb3d_camera::Params& camera [[buffer(23)]],device const mb3d_camera::Ray* rays [[buffer(26)]],
    constant mb3d_inside::Params& inside [[buffer(27)]],device const mb3d_inside::Record* inside_records [[buffer(28)]],
    uint local [[thread_position_in_grid]]) {
    if(local>=launch.y||launch.x>=launch.z||local>=launch.z-launch.x)return;
    const uint index=launch.x+local;ExactSurface value{};value.depth_packed=0x80000000u;
    if(launch.w||!scene.width||scene.width>4096||!scene.height||scene.height>4096||
       ulong(scene.width)*scene.height!=launch.z)value.fault=1;
    else {
        const auto record=geometry[index];const auto n=normal_records[index];const auto v=normal_results[index];
        if(!mb3d_native::valid_general_normals(normal_params,program)||n.version!=mb3d_native::kGeneralNormalVersion||
           n.fault||n.reserved[0]||n.reserved[1]||v.fault||v.reserved[0]||v.reserved[1]||v.reserved[2]||v.reserved[3]||
           v.roughness_byte>255u||v.completed!=uint(primary[index].rm_result&&!primary[index].start_inside)||
           record.version!=mb3d_native::kGeneralGeometryVersion||!metal::isfinite(record.initial_rout)||
           !mb3d_native::valid_smoothing_counters(record.smoothing,record.formula_work,mb3d_native::native_selected_smoothing_policy())||
           !mb3d_native::valid_difs_carried_reads(record.difs_carried_reads,record.de_calls,record.normal_calls)||record.de_calls>mb3d_native::kGeneralMaxDistanceCalls||record.gradient_range_clamps>record.de_calls||
           !mb3d_native::valid_fp32_range_exits(record.fp32_range_exits,record.de_calls,record.normal_calls))value.fault=1;
        else value=mb3d_native::general_surface_inside(primary[index],scene,packing,program,inside,inside_records[index],
            record.initial_rout,camera,rays[index],v.roughness_byte,record.hit_color);
    }
    output[index]=value;
}
kernel void native_camera_general_color(constant ExactColorScene& scene [[buffer(0)]],
    device const ExactSurface* surfaces [[buffer(3)]],device ExactColorPixel* output [[buffer(4)]],
    constant uint* launch [[buffer(5)]],constant mb3d_native::NativeInteriorColorParams& interior [[buffer(6)]],
    constant mb3d_native::NativeLightGeometry& lights [[buffer(7)]],
    constant mb3d_native::NativeMaterialMaps& maps [[buffer(8)]],device const float* map_samples [[buffer(9)]],
    constant mb3d_camera::Params& camera [[buffer(23)]],device const mb3d_camera::Axis* columns [[buffer(24)]],
    device const mb3d_camera::Axis* rows [[buffer(25)]],uint2 id [[thread_position_in_grid]]) {
    // The launch dimensions bind the independently allocated output and axes.
    // Validate the offset before addition; a mismatched scene writes a fault
    // to every launched allocated pixel instead of silently leaving zeros.
    const uint width=launch[1],height=launch[2];
    if(!width||width>4096||!height||height>4096||launch[0]>=width||
       id.x>=width-launch[0]||id.y>=height)return;
    const uint x=launch[0]+id.x,y=id.y,index=y*width+x;
    ExactColorPixel result{};
    if(scene.width!=width||scene.height!=height||!mb3d_camera::valid_camera(camera)||
       camera.width!=width||camera.height!=height)result.fault=1;
    else {
        const auto view=mb3d_camera::paint_view(camera,columns[x],rows[y],x,y);
        const mb3d_native::ColorVec direction={mb3d_native::F32::from_native(view.direction[0]),
            mb3d_native::F32::from_native(view.direction[1]),mb3d_native::F32::from_native(view.direction[2])};
        const bool positioned=(interior.flags&mb3d_native::kNativeLightGeometryFlag)!=0;
        const bool mapped=(interior.flags&mb3d_native::kNativeMaterialMapFlag)!=0;
        constant mb3d_native::NativeMaterialMaps* material=mapped?&maps:nullptr;
        if(view.fault)result.fault=view.fault;
        else if((positioned&&!mb3d_native::valid_light_geometry(lights,interior))||
                (mapped&&!mb3d_native::valid_material_maps(maps,interior)))result.fault=1;
        else if(positioned)result=mb3d_native::camera_general_color_pixel(scene,interior,direction,
            mb3d_native::F32::from_native(view.y_position),surfaces[index],y,&lights,
            mb3d_native::native_paint_position(lights,scene,direction,surfaces[index],mb3d_camera::screen_x(camera,x),mb3d_camera::screen_y(camera,y),camera.optic==2),
            material,map_samples);
        else result=mb3d_native::camera_general_color_pixel(scene,interior,direction,
            mb3d_native::F32::from_native(view.y_position),surfaces[index],y,nullptr,mb3d_native::NativePaintPosition{},
            material,map_samples);
    }
    output[index]=result;
}
