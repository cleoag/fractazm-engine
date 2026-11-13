#pragma once
// Native-only ABI. Integer fields and IEEE binary32, no FP64 arithmetic.
#ifdef __METAL_VERSION__
#include <metal_stdlib>
namespace mb3d_dof { using U32=uint; using I32=int; using U64=ulong; }
#else
#include <cstdint>
namespace mb3d_dof { using U32=uint32_t; using I32=int32_t; using U64=uint64_t; }
#endif
namespace mb3d_dof {
enum : U32 { Version=1, TileSize=16, MaxRadius=1000, MaxSlice=65536 };
struct Params {
    U32 version,width,height,plain;
    U32 passes,reserved0,reserved1,reserved2;
    float inverse_z_mul,inverse_z_corr,fov_factor,focus_low;
    float focus_high,aperture,clip_radius,reserved_float;
};
struct Pass { U32 version,index,fault,reserved; float aperture,focus,rsub,max_radius; };
struct Source { I32 key; U32 index; float radius; U32 flags; U32 bound,fault,raw_key,reserved; };
struct Global { U32 nearest_z,far_z,max_radius_bits,max_bound; U32 fault,reserved0,reserved1,reserved2; };
struct Item { I32 key; U32 source; };
struct SortState {
    U32 version,count,done,fault,phase,stack_size,reserved0,reserved1;
    I32 lo,hi,left,right,pivot; U32 reserved2; U64 work;
    I32 stack_lo[32],stack_hi[32];
};
struct SortLaunch { U32 count,allocation_count,quota,reserved; U64 total_limit; };
struct Tile { U32 x,y,width,height,count,cursor,fault,reserved; };
struct SpatialRecord { U32 version,checked,kept,fault; };
struct Accum { float r,g,b,transparency; U32 visited,contributed,fault,changed; };
struct Launch { U32 tile_count,tile_allocation,candidate_capacity,candidate_allocation;
                U32 source_begin,source_end,pixel_allocation,reserved; };
}
#ifndef __METAL_VERSION__
static_assert(sizeof(mb3d_dof::Params)==64);
static_assert(sizeof(mb3d_dof::Pass)==32);
static_assert(sizeof(mb3d_dof::Source)==32);
static_assert(sizeof(mb3d_dof::Global)==32);
static_assert(sizeof(mb3d_dof::SortState)==320);
static_assert(sizeof(mb3d_dof::Tile)==32);
static_assert(sizeof(mb3d_dof::Accum)==32);
static_assert(sizeof(mb3d_dof::Launch)==32);
#endif
