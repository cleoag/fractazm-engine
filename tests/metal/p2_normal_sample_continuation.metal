#define MB3D_METAL_NATIVE_FLOAT 1
#include "src/render/native_metal/general_geometry.metal.h"
using namespace metal;using namespace mb3d_native;
constant uint ProbeMagic=0x50325001;
constant uint observer_snapshot_policy [[function_constant(23)]];
struct Input {float point[3],params[64];uint id,local_limit,limit,work;NativeFormulaState lane;uint pass_limit,has_radius;float stop,radius_stop;uint inside_mode,observe_mask,temporary;NativeGeometryRecord geometry;NativeNormalRecord normal;};
struct Output {GeneralNormalSample result;NativeFormulaState lane;NativeSmoothingCounters smoothing;uint carried,range,clamps;NativeGeometryRecord geometry;NativeNormalRecord normal;};
#ifndef P2_REFERENCE
#include "src/render/native_metal/continuation_normal_sample_step.metal.h"
using namespace mb3d_native::continuation;
struct Checkpoint {uint magic,job,sequence,done,transitions;NativeFormulaState lane;CombinedFrame combined;DistanceFrame distance;OrbitReplayFrame replay;OrbitPassFrame orbit;FormulaStepFrame step;WrapperFrame wrapper;BodyFrame body;NativeSmoothingCounters smoothing;uint carried,range,clamps;NativeGeometryRecord geometry;DistanceEntryFrame entry;NativeNormalRecord normal;NormalSampleFrame sample;};
struct SampleObserver {
 device uint* trace;uint tid,transitions;
 __attribute__((noinline)) void record(uint phase,thread const NativeFormulaState& lane,thread const NativeGeometryRecord& geometry,thread const NativeNormalRecord& normal,thread const NativeOrbitResult& child,thread const NativeFormulaFault& fault) thread {
  if(transitions>=64)return;
  device uint* target=trace+(tid*64+transitions)*256;
  for(uint k=0;k<256;++k)target[k]=0;
  target[0]=phase;
  *reinterpret_cast<device NativeFormulaState*>(target+8)=lane;
  *reinterpret_cast<device NativeGeometryRecord*>(target+55)=geometry;
  *reinterpret_cast<device NativeNormalRecord*>(target+83)=normal;
  *reinterpret_cast<device NativeOrbitResult*>(target+91)=child;
  *reinterpret_cast<device NativeFormulaFault*>(target+150)=fault;++transitions;
 }
};
struct SampleCapture {
 uint count,phase[2];NativeFormulaState lane[2];NativeGeometryRecord geometry[2];NativeNormalRecord normal[2];NativeOrbitResult child[2];NativeFormulaFault fault[2];
 void record(uint tag,thread const NativeFormulaState& a,thread const NativeGeometryRecord& b,thread const NativeNormalRecord& c,thread const NativeOrbitResult& d,thread const NativeFormulaFault& e) thread {
  const uint i=count++;phase[i]=tag;lane[i]=a;geometry[i]=b;normal[i]=c;child[i]=d;fault[i]=e;
 }
};
#include "normal_sample_observer.metal.h"
#endif
kernel void p2_abi(device uint* out [[buffer(0)]]){
 out[0]=ProbeMagic;out[1]=sizeof(Input);out[2]=sizeof(Output);
#ifdef P2_REFERENCE
 out[3]=0;
#else
 out[3]=sizeof(Checkpoint);
#endif
}
#define BIND_RESOURCES(owner) NativeResources resources{};resources.radius_stop=in.radius_stop;resources.smoothing=(in.observe_mask&1)?&owner.smoothing:nullptr;resources.carried_reads=(in.observe_mask&2)?&owner.carried:nullptr;resources.range_exits=(in.observe_mask&4)?&owner.range:nullptr
kernel void p2_original(device const Input* inputs [[buffer(0)]],device Output* outputs [[buffer(1)]],constant NativeFormulaProgram* programs [[buffer(2)]],uint tid [[thread_position_in_grid]]){
 auto in=inputs[tid];Output out{};out.lane=in.lane;out.geometry=in.geometry;out.normal=in.normal;BIND_RESOURCES(out);
 out.result=general_normal_sample(float3(in.point[0],in.point[1],in.point[2]),in.stop,bool(in.has_radius),bool(in.lane.calc_smooth),programs[2*tid],resources,out.lane,out.geometry,out.normal);outputs[tid]=out;
}
#ifndef P2_REFERENCE
kernel void p2_admission(constant NativeFormulaProgram* programs [[buffer(0)]],device uint* admitted [[buffer(1)]],uint tid [[thread_position_in_grid]]){admitted[tid]=uint(native_program_valid(programs[2*tid]));}
kernel void p2_observer(device const Input* inputs [[buffer(0)]],device Output* outputs [[buffer(1)]],constant NativeFormulaProgram* programs [[buffer(2)]],device uint* trace [[buffer(6)]],uint tid [[thread_position_in_grid]]){
 auto in=inputs[tid];Output out{};out.lane=in.lane;out.geometry=in.geometry;out.normal=in.normal;BIND_RESOURCES(out);
 if(is_function_constant_defined(observer_snapshot_policy)&&observer_snapshot_policy==1u){
  SampleCapture observer{};out.result=normal_sample_original_observed(float3(in.point[0],in.point[1],in.point[2]),in.stop,bool(in.has_radius),bool(in.lane.calc_smooth),programs[2*tid],resources,out.lane,out.geometry,out.normal,observer);
  SampleObserver sink{trace,tid,0};for(uint k=0;k<observer.count;++k)sink.record(observer.phase[k],observer.lane[k],observer.geometry[k],observer.normal[k],observer.child[k],observer.fault[k]);
 }else{SampleObserver observer{trace,tid,0};out.result=normal_sample_original_observed(float3(in.point[0],in.point[1],in.point[2]),in.stop,bool(in.has_radius),bool(in.lane.calc_smooth),programs[2*tid],resources,out.lane,out.geometry,out.normal,observer);}
 outputs[tid]=out;
}
kernel void p2_advance(device const Input* inputs [[buffer(0)]],device Output* outputs [[buffer(1)]],constant NativeFormulaProgram* programs [[buffer(2)]],device Checkpoint* checkpoints [[buffer(3)]],device uint* status [[buffer(4)]],constant uint4& control [[buffer(5)]],device uint* trace [[buffer(6)]],uint tid [[thread_position_in_grid]]){
 if(control.x!=ProbeMagic||!control.z||control.z>(1u<<20)){status[tid]=0x80000001;return;}
 if(checkpoints[tid].magic&&(checkpoints[tid].magic!=control.x||checkpoints[tid].job!=control.y||checkpoints[tid].sequence!=control.w)){status[tid]=0x80000002;return;}
 auto s=checkpoints[tid];auto in=inputs[tid];
 if(!s.magic){if(control.w){status[tid]=0x80000002;return;}s={};s.magic=control.x;s.job=control.y;s.lane=in.lane;s.geometry=in.geometry;s.normal=in.normal;
#define POISON(field) for(uint k=0;k<sizeof(s.field);++k)reinterpret_cast<thread uchar*>(&s.field)[k]=0xa5
  POISON(combined);POISON(distance);POISON(replay);POISON(orbit);POISON(step);POISON(wrapper);POISON(body);POISON(entry);
#undef POISON
  for(uint k=0;k<3;++k)s.sample.point[k]=in.point[k];s.sample.stop=in.stop;s.sample.direct=in.has_radius;s.sample.want_smooth=in.lane.calc_smooth;
 }
 if(s.done){++checkpoints[tid].sequence;status[tid]=99;return;}
 BIND_RESOURCES(s);SampleObserver observer{trace,tid,s.transitions};
 for(uint step=0;step<control.z&&!s.done;++step){
  uint before=s.sample.header.pc;
  s.done=normal_sample_advance(s.sample,s.entry,s.combined,s.distance,s.replay,s.orbit,s.step,s.wrapper,s.body,programs[2*tid],resources,s.lane,s.geometry,s.normal)==99;
  if(resources.smoothing!=((in.observe_mask&1)?&s.smoothing:nullptr)||resources.carried_reads!=((in.observe_mask&2)?&s.carried:nullptr)||resources.range_exits!=((in.observe_mask&4)?&s.range:nullptr)){status[tid]=0x80000004;return;}
  uint after=s.sample.header.pc;
  if(after!=before&&after!=99)observer.record(after,s.lane,s.geometry,s.normal,s.sample.child,s.sample.fault);
 }
 s.transitions=observer.transitions;++s.sequence;checkpoints[tid]=s;status[tid]=s.done?99:s.sample.header.pc;
 if(s.done)outputs[tid]={{s.sample.value,s.sample.smooth},s.lane,s.smoothing,s.carried,s.range,s.clamps,s.geometry,s.normal};
}
#endif
