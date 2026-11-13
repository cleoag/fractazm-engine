#define MB3D_METAL_NATIVE_FLOAT 1
#include "src/render/native_metal/general_geometry.metal.h"
#include "native_refinement_composition_abi.h"
using namespace metal;using namespace mb3d_native;using namespace mb3d_native::continuation;using namespace p2_refinement;
#ifndef P2_REFERENCE
struct Capture {
 Trace value;
 void before(float3 point,float stop,bool smooth,int maximum,bool temporary,thread const NativeFormulaState& lane,thread const NativeGeometryRecord& record) thread {
  const uint i=value.count++;if(i>=Capacity){value.overflow=1;return;}thread auto& q=value.queries[i];
  for(uint k=0;k<3;++k)q.point[k]=point[k];q.stop=stop;q.maximum=maximum;q.smooth=uint(smooth);q.temporary=uint(temporary);q.before=lane;q.before_record=record;
 }
 void after(thread const NativeFormulaState& lane,thread const NativeGeometryRecord& record,NativeOrbitResult result) thread {
  const uint i=value.count-1;if(i>=Capacity)return;thread auto& q=value.queries[i];q.after=lane;q.after_record=record;q.result=result;
 }
};
// The original observer can be outlined independently of continuation snapshots.
#ifdef P2_OUTLINE_OBSERVER
struct ObservedCapture {
 Trace value;
 __attribute__((noinline)) void before(float3 point,float stop,bool smooth,int maximum,bool temporary,thread const NativeFormulaState& lane,thread const NativeGeometryRecord& record) thread {
  const uint i=value.count++;if(i>=Capacity){value.overflow=1;return;}thread auto& q=value.queries[i];
  for(uint k=0;k<3;++k)q.point[k]=point[k];q.stop=stop;q.maximum=maximum;q.smooth=uint(smooth);q.temporary=uint(temporary);q.before=lane;q.before_record=record;
 }
 __attribute__((noinline)) void after(thread const NativeFormulaState& lane,thread const NativeGeometryRecord& record,NativeOrbitResult result) thread {
  const uint i=value.count-1;if(i>=Capacity)return;thread auto& q=value.queries[i];q.after=lane;q.after_record=record;q.result=result;
 }
};
#else
using ObservedCapture=Capture;
#endif
#include "refinement_outer_observer.metal.h"
#include "src/render/native_metal/continuation_refinement_step.metal.h"
#endif
inline Output initial_output(constant Input& in){Output out{};for(uint k=0;k<3;++k)out.point[k]=in.point[k];out.depth=in.depth;out.iteration=in.iteration;
 out.lane=native_initial_state(float3(in.point[0],in.point[1],in.point[2]),in.stop,in.program);out.lane.tmp=17;
 out.geometry.refine_calls=in.initial_refine;out.geometry.de_calls=in.initial_de;out.geometry.formula_work=in.initial_work;out.geometry.fault=in.initial_fault;return out;}
kernel void abi(device uint* out [[buffer(0)]]){out[0]=sizeof(Input);out[1]=sizeof(Output);out[3]=RealProbeAbi;
#ifdef P2_REFERENCE
out[2]=0;
#else
out[2]=sizeof(RealState);
#endif
}
kernel void original(constant Input* inputs [[buffer(0)]],device Output* outputs [[buffer(1)]],uint tid [[thread_position_in_grid]]){
 constant auto& in=inputs[tid];auto out=initial_output(in);float3 point(in.point[0],in.point[1],in.point[2]);const float3 ray(in.ray[0],in.ray[1],in.ray[2]);NativeResources resources{};
 out.success=uint(general_refine_iteration_count(point,ray,out.depth,in.stop,in.scene,in.program,resources,out.lane,out.geometry,out.iteration,in.steps));for(uint k=0;k<3;++k)out.point[k]=point[k];outputs[tid]=out;
}
#ifndef P2_REFERENCE
kernel void observe(constant Input* inputs [[buffer(0)]],device Output* outputs [[buffer(1)]],uint tid [[thread_position_in_grid]]){
 constant auto& in=inputs[tid];auto out=initial_output(in);float3 point(in.point[0],in.point[1],in.point[2]);const float3 ray(in.ray[0],in.ray[1],in.ray[2]);NativeResources resources{};ObservedCapture capture{};
 out.success=uint(refinement_observed(point,ray,out.depth,in.stop,in.scene,in.program,resources,out.lane,out.geometry,out.iteration,in.steps,capture));for(uint k=0;k<3;++k)out.point[k]=point[k];out.trace=capture.value;outputs[tid]=out;
}
kernel void advance(constant Input* inputs [[buffer(0)]],device Output* outputs [[buffer(1)]],device RealState* states [[buffer(2)]],constant uint& quantum [[buffer(3)]],uint tid [[thread_position_in_grid]]){
 auto s=states[tid];constant auto& in=inputs[tid];
 if(!s.initialized){s={};s.initialized=1;s.output=initial_output(in);
#define POISON(field) for(uint k=0;k<sizeof(s.field);++k)reinterpret_cast<thread uchar*>(&s.field)[k]=0xa5
 POISON(refinement);POISON(entry);POISON(combined);POISON(distance);POISON(replay);POISON(orbit);POISON(step);POISON(wrapper);POISON(body);
#undef POISON
 s.refinement.header={};for(uint k=0;k<3;++k){s.refinement.point[k]=in.point[k];s.refinement.ray[k]=in.ray[k];}
 s.refinement.depth=in.depth;s.refinement.stop=in.stop;s.refinement.refinement_steps=in.steps;s.refinement.iteration=in.iteration;}
 NativeResources resources{};Capture capture{};capture.value=s.output.trace;
 for(uint i=0;i<quantum&&!s.done;++i)s.done=refinement_advance(s.refinement,s.entry,s.combined,s.distance,s.replay,s.orbit,s.step,s.wrapper,s.body,in.scene,in.program,resources,s.output.lane,s.output.geometry,capture)==99;
 s.output.trace=capture.value;states[tid]=s;if(s.done){for(uint k=0;k<3;++k)s.output.point[k]=s.refinement.point[k];s.output.depth=s.refinement.depth;s.output.iteration=s.refinement.iteration;s.output.success=uint(refinement_success(s.refinement,s.output.geometry));outputs[tid]=s.output;}
}
#endif
