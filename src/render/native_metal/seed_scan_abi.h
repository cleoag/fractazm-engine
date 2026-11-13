#pragma once
#if defined(__METAL_VERSION__)
#include <metal_stdlib>
#else
#include <cstdint>
#endif
namespace mb3d_seed_scan {
#if defined(__METAL_VERSION__)
using U32=metal::uint;
#else
using U32=std::uint32_t;
#endif
enum : U32 { Version=1, Group=256, Interleaved=1, Contiguous=2,
    PrimaryFlags=1, DrawCounts=2 };
struct Params {
    U32 version,width,height,owners;
    U32 rows_per_owner,mapping,input_kind,initial_seed;
    U32 items,groups,first,reserved;
    U32 offset,count;
};
}
