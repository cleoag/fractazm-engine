#pragma once
#ifdef __METAL_VERSION__
#include <metal_stdlib>
namespace mb3d_zn { using U32=unsigned int;using I32=int; }
#else
#include <cstdint>
namespace mb3d_zn { using U32=uint32_t;using I32=int32_t; }
#endif
namespace mb3d_zn {
enum:U32 { Version=1,MaxExtent=4096 };
struct alignas(16) Params {
    U32 version,width,height,mode;
    U32 full_width,full_height;I32 left,top;
    float step,fov,aspect,optic_z;
    float inverse_z_mul,depth_correction,stop_factor,reserved_float;
    U32 count,reserved[3];
};
struct alignas(16) Coordinate {
    float x,y,z,depth;
    U32 foreground,fault,index,reserved;
};
struct alignas(16) Record {
    U32 version,index,foreground,changed;
    U32 neighbors,fault,reserved[2];
};
struct alignas(16) Launch {
    U32 count,source_count,coordinate_count,output_count;
    U32 record_count,reserved[3];
};
#ifndef __METAL_VERSION__
static_assert(sizeof(Params)==80&&sizeof(Coordinate)==32&&sizeof(Record)==32&&sizeof(Launch)==32);
#endif
}
