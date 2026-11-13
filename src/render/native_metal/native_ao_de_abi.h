#pragma once
#include "native_ao_abi.h"
#include "smoothing_abi.h"
#include "src/render/native_metal/formula_abi.h"

namespace mb3d_native_ao {
enum : U32 { DEVersion=4 };
// Pixels per independent DE AO command (pixel ABI v3+; v2 allowed 4096). Each
// pixel owns its lane and seed, so the batch changes only the schedule.
enum : U32 { kDEPixelBatch=262144 };
// Pixel-entry proof words. The device re-derives only the closed depth chain;
// the others are host proofs. A conditional (dIFS) contract holds only for a
// pass whose records count no difs_carried_reads, which the host verifies.
enum : U32 { kDEProofClosedDepth=1, kDEProofStateContract=5, kDEProofClosedChain=6, kDEProofConditionalState=7 };
// Separate ordinary DE AO contract. Scalar camera preparation is CPU Native;
// all surface reconstruction, normals, DE queries and accumulation run on GPU.
struct alignas(16) DEParams {
    U32 version,width,height,task_rows;
    U32 task_count,quality,dither,first_step_random;
    U32 max_steps,work_limit,vary_stop,initial_inside;
    U32 in_and_out,initial_seed,cut_options,reserved;
    float base_stop,stop_factor,step_width,max_distance_base;
    float z_corr,z_mul,minimum_dot,correction_weight;
    float step_multiplier,angular_spacing,padding[2];
    float view_matrix[3][4];
    float cut[4];
    U32 full_width,full_height; I32 left,top;
    U32 row_counts[4];
};
struct alignas(16) DERecord {
    Record common;
    U32 rays,refinements,selected_inside,completed;
    // Version 3: FP32 range exits of this pixel's DE queries (orbit_driver),
    // at most kNativeRangeExitsPerDistance per query.
    U32 formula_work,formula_calls,formula_fault,fp32_range_exits;
    mb3d_native::NativeSmoothingCounters smoothing;
    // Version 4: dIFS orbits of this pixel that returned carried tmp/dfree2.
    U32 difs_carried_reads,reserved[3];
};
// CPU CalcAO_DE zeros/IniIt3D once per row-range task. Formula scratch and RNG
// persist across that task's pixels, including successive device dispatches.
struct alignas(16) DETaskState {
    mb3d_native::NativeFormulaState lane;
    U32 version,fault,cursor,seed;
    U32 processed,finished,reserved[3];
};
#ifndef __METAL_VERSION__
static_assert(sizeof(DEParams)==208);
static_assert(sizeof(DERecord)==128);
static_assert(sizeof(DETaskState)==224);
#endif
}
