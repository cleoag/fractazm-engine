#pragma once
#include "src/render/native_metal/continuation_abi.h"
namespace p2_refinement {
using namespace mb3d_native;using namespace mb3d_native::continuation;
enum : U32 { Capacity=16 };
struct Input {
 ExactScene scene;NativeFormulaProgram program;
 float point[3],ray[3],depth,stop;U32 steps,iteration,initial_refine,initial_de,initial_work,initial_fault,fault_at;
 float smooth[Capacity];U32 iterations[Capacity];
};
struct Query {
 float point[3],stop;I32 maximum;U32 smooth,temporary;
 NativeFormulaState before,after;NativeGeometryRecord before_record,after_record;NativeOrbitResult result;
};
struct Trace {U32 count,overflow;Query queries[Capacity];};
struct Output {float point[3],depth;U32 iteration,success;NativeFormulaState lane;NativeGeometryRecord geometry;Trace trace;};
struct State {RefinementFrame refinement;DistanceEntryFrame entry;Output output;U32 initialized,done;};
}
