#define MB3D_METAL_NATIVE_FLOAT 1
#include "src/render/native_metal/general_geometry.metal.h"
#include "native_de_refinement_probe_abi.h"
using namespace metal;using namespace mb3d_native;using namespace mb3d_native::continuation;using namespace p2_de_refinement;
#include "de_refinement_control.metal.h"
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
#include "de_refinement_outer_observer.metal.h"
#include "src/render/native_metal/continuation_de_refinement_step.metal.h"
#endif
inline Output initial_output(constant Input& in){Output out{};for(uint k=0;k<3;++k)out.point[k]=in.point[k];out.depth=in.depth;out.stop=in.stop;out.de=in.de;out.iteration=in.iteration;
 out.lane=native_initial_state(float3(in.point[0],in.point[1],in.point[2]),in.stop,in.program);out.lane.tmp=17;out.geometry.refine_calls=in.initial_refine;out.geometry.de_calls=in.initial_de;out.geometry.formula_work=in.initial_work;out.geometry.fault=in.initial_fault;return out;}
kernel void abi(device uint* out [[buffer(0)]]){out[0]=sizeof(Input);out[1]=sizeof(Output);out[3]=RealProbeAbi;
#ifdef P2_REFERENCE
out[2]=0;
#else
out[2]=sizeof(State);
#endif
}
kernel void original(constant Input* inputs [[buffer(0)]],device Output* outputs [[buffer(1)]],uint tid [[thread_position_in_grid]]){
 constant auto& in=inputs[tid];auto out=initial_output(in);float3 point(in.point[0],in.point[1],in.point[2]);const float3 ray(in.ray[0],in.ray[1],in.ray[2]);NativeResources resources{};
 de_refinement_control(point,ray,out.depth,out.stop,out.de,in.last_step,in.base_stop,in.stop_factor,out.iteration,in.scene,in.program,resources,out.lane,out.geometry);for(uint k=0;k<3;++k)out.point[k]=point[k];outputs[tid]=out;
}
#ifndef P2_REFERENCE
kernel void observe(constant Input* inputs [[buffer(0)]],device Output* outputs [[buffer(1)]],uint tid [[thread_position_in_grid]]){
 constant auto& in=inputs[tid];auto out=initial_output(in);float3 point(in.point[0],in.point[1],in.point[2]);const float3 ray(in.ray[0],in.ray[1],in.ray[2]);NativeResources resources{};ObservedCapture capture{};
 de_refinement_observed(point,ray,out.depth,out.stop,out.de,in.last_step,in.base_stop,in.stop_factor,out.iteration,in.scene,in.program,resources,out.lane,out.geometry,capture);for(uint k=0;k<3;++k)out.point[k]=point[k];out.trace=capture.value;outputs[tid]=out;
}

kernel void advance(constant Input* inputs [[buffer(0)]],device Output* outputs [[buffer(1)]],device State* states [[buffer(2)]],constant uint& quantum [[buffer(3)]],uint tid [[thread_position_in_grid]]){
 auto s=states[tid];constant auto& in=inputs[tid];
 if(!s.initialized){s={};s.initialized=1;s.output=initial_output(in);
#define POISON(field) for(uint k=0;k<sizeof(s.field);++k)reinterpret_cast<thread uchar*>(&s.field)[k]=0xa5
 POISON(primary);POISON(entry);POISON(combined);POISON(distance);POISON(replay);POISON(orbit);POISON(step);POISON(wrapper);POISON(body);
#undef POISON
 s.primary.header={};s.primary.header.pc=40;for(uint k=0;k<3;++k){s.primary.point[k]=in.point[k];s.primary.ray[k]=in.ray[k];}
 s.primary.depth=in.depth;s.primary.stop=in.stop;s.primary.de=in.de;s.primary.last_step=in.last_step;s.primary.base_stop=in.base_stop;s.primary.stop_factor=in.stop_factor;s.primary.iteration=in.iteration;}
 NativeResources resources{};Capture capture{};capture.value=s.output.trace;
 for(uint i=0;i<quantum&&!s.done;++i)s.done=de_refinement_advance(s.primary,s.entry,s.combined,s.distance,s.replay,s.orbit,s.step,s.wrapper,s.body,in.scene,in.program,resources,s.output.lane,s.output.geometry,capture)==49;
 s.output.trace=capture.value;states[tid]=s;if(s.done){for(uint k=0;k<3;++k)s.output.point[k]=s.primary.point[k];s.output.depth=s.primary.depth;s.output.stop=s.primary.stop;s.output.de=s.primary.de;s.output.iteration=s.primary.iteration;outputs[tid]=s.output;}
}
#endif
