#pragma once
#include "native_inside_continuation_probe_abi.h"
namespace p2_inside_trace {
using namespace mb3d_native;
enum : U32 { Version=1,Capacity=8 };
struct Snapshot {
 U32 version,site,phase,mask;mb3d_inside::RetryState retry;U32 attempt,x,y,reserved0;
 mb3d_inside::Params policy;mb3d_camera::Ray camera;NativeNormalParams params;
 float point[3],stop;ExactPixel pixel;NativeFormulaState lane;NativeGeometryRecord geometry;
 NativeNormalRecord normal_record;NativeNormalResult normal_result;mb3d_inside::Record activity;
 NativeOrbitResult child;U32 reserved[2];
};
struct Trace {U32 count,overflow,reserved[2];Snapshot snapshots[Capacity];};
static_assert(sizeof(Snapshot)==1040,"inside snapshot layout");
static_assert(sizeof(Trace)==8336,"inside trace layout");
}
