#pragma once
#include "native_refinement_probe_abi.h"
namespace p2_de_refinement {
using namespace mb3d_native;using namespace mb3d_native::continuation;
using p2_refinement::Query;using p2_refinement::Trace;
enum : U32 { Capacity=p2_refinement::Capacity,SyntheticProbeAbi=0x50324453u,RealProbeAbi=0x50324452u };
struct Input {
 ExactScene scene;NativeFormulaProgram program;
 float point[3],ray[3],depth,stop,de,last_step,base_stop,stop_factor;
 U32 iteration,initial_refine,initial_de,initial_work,initial_fault,fault_at;
 float distances[Capacity];U32 iterations[Capacity];
};
struct Output {float point[3],depth,stop,de;U32 iteration;NativeFormulaState lane;NativeGeometryRecord geometry;Trace trace;};
struct State {
 PrimaryFrame primary;DistanceEntryFrame entry;CombinedFrame combined;DistanceFrame distance;
 OrbitReplayFrame replay;OrbitPassFrame orbit;FormulaStepFrame step;WrapperFrame wrapper;BodyFrame body;
 Output output;U32 initialized,done;
};
}
