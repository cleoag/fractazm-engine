#pragma once
#include "formula_abi.h"
namespace mb3d_native {
enum : U32 { kLogicalLaneVersion=1, kLogicalLaneBatchPixels=32 };
// A logical CPU task's pixel order. Ordinal / width selects a row, then
// row_start + row * row_stride. Cursor is advanced only after a successful GPU
// batch. The independent allocation count is the complete image size.
struct NativeLogicalLane {
    U32 row_start,row_stride,row_count,cursor;
    U32 pixel_count,allocation_count,reserved[2];
};
struct NativeLogicalLaneRecord { U32 version,fault,processed,next_cursor; };
#ifdef __METAL_VERSION__
inline bool valid_logical_lane(NativeLogicalLane lane,uint width,uint height,uint allocation) {
    if(!width||width>4096||!height||height>4096||!lane.row_stride||!lane.row_count||
       lane.row_start>=height||lane.reserved[0]||lane.reserved[1]||
       lane.allocation_count!=allocation||ulong(width)*height!=allocation||
       lane.row_count-1>(height-1-lane.row_start)/lane.row_stride||
       lane.pixel_count>kLogicalLaneBatchPixels)return false;
    const ulong count=ulong(width)*lane.row_count;
    return ulong(lane.cursor)<=count&&ulong(lane.pixel_count)<=count-lane.cursor;
}
#else
static_assert(sizeof(NativeLogicalLane)==32);
static_assert(sizeof(NativeLogicalLaneRecord)==16);
#endif
}
