#pragma once
#include "native_refinement_probe_abi.h"
namespace p2_primary {
using namespace mb3d_native;using namespace mb3d_native::continuation;
using p2_refinement::Query;
enum : U32 { Capacity=64,SyntheticProbeAbi=0x50325053u,RealProbeAbi=0x50325052u };
struct Trace {U32 count,overflow;Query queries[Capacity];};
struct Input {
 ExactScene scene;NativeFormulaProgram program;mb3d_camera::Ray camera;NativeNormalParams normal;
 U32 x,y,initial_refine,initial_de,initial_work,initial_ray,initial_fault,fault_at;
 float distances[Capacity],smooth[Capacity];U32 iterations[Capacity];
};
struct Output {ExactPixel pixel;NativeFormulaState lane;NativeGeometryRecord geometry;NativeNormalRecord normal_record;NativeNormalResult normal_result;Trace trace;};
struct State {RayFrame frame;Trace trace;U32 initialized,done;};
// Unobserved real final-state gate carries only production frame state.
struct RealState {RayFrame frame;U32 initialized,done;};
}
