#pragma once
#ifdef __METAL_VERSION__
#include <metal_stdlib>
#else
#include <cstdint>
#endif

namespace mb3d_native_ao {
#ifdef __METAL_VERSION__
using U32=uint; using I32=int;
#else
using U32=std::uint32_t; using I32=std::int32_t;
#endif
enum : U32 { Version=1, Threshold15=0, Threshold15T0=1, HiQ=2,
    HiQT0=3, HiQRandom=4, HiQRandomT0=5, HiQPano=6, HiQT0Pano=7,
    HiQRandomPano=8, HiQRandomT0Pano=9, DistanceEstimate=16,
    Invalid=1, Bounds=2, NonFinite=4, WorkExhausted=8 };
// Separate from exact AO; no library ABI reinterpretation. The first four
// marker is Version,Params,Level,Record,LaneState,32,Range15,MedianState,PreprocessRecord.
// width/height describe the complete image-space AO buffer, not full camera
// extents. DE AO separately consumes camera Params23 / output tile geometry.
struct alignas(16) Params {
    U32 version,width,height,algorithm;
    U32 levels,repeats,logical_lanes,quality;
    I32 w_lo,w_hi,h_lo,h_hi;
    U32 work_limit,first_step_random,dither,output_count;
    float threshold,depth_scale,sum_multiplier,padding;
    float directions[64];
};
struct alignas(16) Level {
    U32 version,index,step,samples;
    float minimum_radius,z_threshold,sit,sum_multiplier;
    I32 maximum_radius; U32 repeat_index,add_limit,reserved;
    float reciprocal_radius[6]; U32 reserved2[2];
};
// One accumulated record per output pixel. Counters describe actual GPU work,
// including rejected-border attempts. Counters never replace explicit faults.
struct alignas(16) Record {
    U32 version,fault,algorithm,active;
    U32 samples,depth_reads,border_rejects,rng_advances;
    U32 levels,repeats,changed,final_ambient;
    U32 de_queries,de_steps,cut_skips,initial_ambient;
};
struct Launch { U32 offset,count,allocated_pixels,reserved; };
struct SeedRecord { U32 version,start,end,draws,fault,pixel,level,repeat; U32 shortcut,reserved[3]; };
struct LaneState { U32 seed,x,y,level,repeat_index,finished,fault,reserved; };
struct Range15 { U32 version,fault,minimum,maximum;float correction,angle_scale,threshold;I32 subtract; };
struct MedianState { U32 version,fault,cursor,processed,reads,writes,finished,reserved; };
struct PreprocessRecord { U32 version,fault,normalizations,wavelet_passes,reads,writes,reserved[2]; };
struct BoundaryResult { I32 x,y; U32 admitted,fault; };
struct PrimitiveInput {
    Params params;
    I32 x,y; U32 seed,distance_squared;
    float sample,sit,threshold,pad;
};
struct PrimitiveOutput {
    BoundaryResult boundary;
    U32 seed,angle_bits,dispatch,fault;
};
#ifndef __METAL_VERSION__
static_assert(sizeof(Params)==336);
static_assert(sizeof(Level)==80);
static_assert(sizeof(Record)==64);
static_assert(sizeof(Launch)==16);
static_assert(sizeof(LaneState)==32);
static_assert(sizeof(Range15)==32);
static_assert(sizeof(MedianState)==32);
static_assert(sizeof(PreprocessRecord)==32);
static_assert(sizeof(PrimitiveInput)==368);
static_assert(sizeof(PrimitiveOutput)==32);
#endif
}
