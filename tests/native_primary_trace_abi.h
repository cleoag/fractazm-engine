#pragma once
#include "native_primary_probe_abi.h"
namespace p2_primary_trace {
using namespace mb3d_native;
enum : U32 { Version=1,Capacity=128 };
// Scalar arrays use masks to canonicalize fields outside their initialized scope.
// No pointers or renderer-owned resources are persisted in this diagnostic trace.
struct Snapshot {
 U32 version,site,phase,mask;
 float vectors[12],scalars[16];U32 integers[8];
 ExactPixel pixel;NativeFormulaState lane;NativeGeometryRecord geometry;
 NativeNormalRecord normal_record;NativeNormalResult normal_result;NativeOrbitResult child;U32 reserved[2];
};
struct Trace {U32 count,overflow,reserved[2];Snapshot snapshots[Capacity];};
static_assert(sizeof(Snapshot)==976,"primary trace snapshot layout");
static_assert(sizeof(Trace)==124944,"primary trace buffer layout");
}
