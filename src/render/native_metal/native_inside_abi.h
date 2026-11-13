#pragma once
#ifdef __METAL_VERSION__
#include <metal_stdlib>
#else
#include <cstdint>
#endif
namespace mb3d_inside {
#ifdef __METAL_VERSION__
using U32=uint; using I32=int;
#else
using U32=std::uint32_t; using I32=std::int32_t;
#endif
enum : U32 { Version=1, Invalid=1, NonFinite=4 };
// Preserve the CPU LongBool word. Header preparation uses +1, retry uses -1.
// dIFS drivers deliberately distinguish its sign; bool normalization is wrong.
// volume_light: DFogOnIt 65535 packs the step count with RMcalcVLight.
struct Params { U32 version; I32 initial_inside; U32 in_and_out,full_width; U32 volume_light,reserved[3]; };
struct Record {
    U32 version,fault,attempts,retries;
    I32 final_inside; U32 start_inside,smooth_calls,reserved;
};
struct RetryState { I32 inside,calc_inside; U32 retried,reserved; };
struct DeResult { float value; I32 iteration; U32 fault,reserved; };
struct PackedResult { U32 shadow,gradient,otrap,fault; };
// Small independent primitive probe. Full geometry requires the integration
// bridge; this probe must never be described as full-render qualification.
struct ProbeInput {
    Params params;
    I32 inside,calc_inside; U32 retried,start_inside;
    float raw_de,stop,step_count,smooth,color_multiplier;
    I32 iteration; U32 driver,de_option,otrap,hit,reserved[2];
};
struct ProbeOutput {
    Record record;
    DeResult de;
    PackedResult packed;
    U32 retry,reverse_normal,reserved[2];
};
#ifndef __METAL_VERSION__
static_assert(sizeof(Params)==32);
static_assert(sizeof(Record)==32);
static_assert(sizeof(ProbeInput)==96);
static_assert(sizeof(ProbeOutput)==80);
#endif
}
