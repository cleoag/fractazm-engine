#define MB3D_METAL_NATIVE_FLOAT 1
#include "src/render/native_metal/formula_wrapper.metal.h"
#include "src/render/native_metal/orbit_driver.metal.h"
#include "src/render/native_metal/general_geometry_abi.h"
#include "entry_original.metal.h"
using namespace metal;using namespace mb3d_native;
constant uint ProbeMagic=0x50325001;
constant uint observer_snapshot_policy [[function_constant(23)]];
struct Input {float point[3],params[64];uint id,local_limit,limit,work;NativeFormulaState lane;uint pass_limit,has_radius;float stop,radius_stop;uint inside_mode,observe_mask,temporary;NativeGeometryRecord geometry;};
struct Output {NativeOrbitResult result;NativeFormulaState lane;NativeSmoothingCounters smoothing;uint carried,range,clamps;NativeGeometryRecord geometry;};
#ifndef P2_REFERENCE
#include "src/render/native_metal/continuation_entry_step.metal.h"
using namespace mb3d_native::continuation;
struct Checkpoint {uint magic,job,sequence,done,transitions;NativeFormulaState lane;CombinedFrame combined;DistanceFrame distance;OrbitReplayFrame replay;OrbitPassFrame orbit;FormulaStepFrame step;WrapperFrame wrapper;BodyFrame body;NativeSmoothingCounters smoothing;uint carried,range,clamps;NativeGeometryRecord geometry;DistanceEntryFrame entry;};
struct EntryObserver {
 device uint* trace;uint tid,transitions;
 __attribute__((noinline)) void record(uint phase,thread const NativeFormulaState& lane,thread const NativeOrbitResult& result,thread const NativeGeometryRecord& geometry,uint calc,int maximum,uint temporary,float radius,uint clamps) thread {
  if(transitions>=64)return;
  device uint* target=trace+(tid*64+transitions)*256;
  for(uint k=0;k<256;++k)target[k]=0;
  target[0]=phase;target[1]=calc;target[2]=uint(maximum);target[3]=temporary;
  *reinterpret_cast<device NativeFormulaState*>(target+8)=lane;
  *reinterpret_cast<device NativeOrbitResult*>(target+55)=result;
  *reinterpret_cast<device NativeGeometryRecord*>(target+114)=geometry;
  *reinterpret_cast<device NativeSmoothingCounters*>(target+142)=geometry.smoothing;
  target[146]=geometry.fp32_range_exits;target[147]=geometry.difs_carried_reads;target[148]=as_type<uint>(radius);target[149]=clamps;++transitions;
 }
};
// Copy into thread-owned snapshots first; write diagnostics after the original
// query returns, keeping device trace addresses out of the numerical call.
struct EntryCapture {
 uint count;NativeFormulaState lane[2];NativeOrbitResult result[2];NativeGeometryRecord geometry[2];
 uint calc[2],maximum[2],temporary[2],clamps[2];float radius[2];
 void record(uint phase,thread const NativeFormulaState& a,thread const NativeOrbitResult& b,thread const NativeGeometryRecord& c,uint d,int e,uint f,float g,uint h) thread {
  const uint i=phase-1;lane[i]=a;result[i]=b;geometry[i]=c;calc[i]=d;maximum[i]=uint(e);temporary[i]=f;radius[i]=g;clamps[i]=h;count=phase;
 }
};
struct EntryCaptureOutlined {
 EntryCapture storage;
 __attribute__((noinline)) void record(uint phase,thread const NativeFormulaState& a,thread const NativeOrbitResult& b,thread const NativeGeometryRecord& c,uint d,int e,uint f,float g,uint h) thread {
  storage.record(phase,a,b,c,d,e,f,g,h);
 }
};
#include "entry_observer.metal.h"
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
 auto in=inputs[tid];Output out{};out.lane=in.lane;out.geometry=in.geometry;BIND_RESOURCES(out);
 out.result=entry_original(float3(in.point[0],in.point[1],in.point[2]),in.stop,programs[2*tid],resources,out.lane,out.geometry,bool(in.lane.calc_smooth),in.lane.max_iterations,bool(in.temporary));outputs[tid]=out;
}
#ifndef P2_REFERENCE
kernel void p2_observer(device const Input* inputs [[buffer(0)]],device Output* outputs [[buffer(1)]],constant NativeFormulaProgram* programs [[buffer(2)]],device uint* trace [[buffer(6)]],uint tid [[thread_position_in_grid]]){
 auto in=inputs[tid];Output out{};out.lane=in.lane;out.geometry=in.geometry;BIND_RESOURCES(out);
 if(is_function_constant_defined(observer_snapshot_policy)&&observer_snapshot_policy==2u){
 EntryCaptureOutlined observer{};
 out.result=entry_original_observed(float3(in.point[0],in.point[1],in.point[2]),in.stop,programs[2*tid],resources,out.lane,out.geometry,bool(in.lane.calc_smooth),in.lane.max_iterations,bool(in.temporary),observer);
 EntryObserver sink{trace,tid,0};thread auto& snapshots=observer.storage;
 for(uint k=0;k<snapshots.count;++k)sink.record(k+1,snapshots.lane[k],snapshots.result[k],snapshots.geometry[k],snapshots.calc[k],int(snapshots.maximum[k]),snapshots.temporary[k],snapshots.radius[k],snapshots.clamps[k]);
 }else if(is_function_constant_defined(observer_snapshot_policy)?observer_snapshot_policy==1u:(native_formula_id0==10089u&&native_selected_formula_type()!=0&&native_selected_driver()!=Interpolated4DAnalytic)){
 EntryCapture observer{};
 out.result=entry_original_observed(float3(in.point[0],in.point[1],in.point[2]),in.stop,programs[2*tid],resources,out.lane,out.geometry,bool(in.lane.calc_smooth),in.lane.max_iterations,bool(in.temporary),observer);
 EntryObserver sink{trace,tid,0};for(uint k=0;k<observer.count;++k)sink.record(k+1,observer.lane[k],observer.result[k],observer.geometry[k],observer.calc[k],int(observer.maximum[k]),observer.temporary[k],observer.radius[k],observer.clamps[k]);
 }else{EntryObserver observer{trace,tid,0};out.result=entry_original_observed(float3(in.point[0],in.point[1],in.point[2]),in.stop,programs[2*tid],resources,out.lane,out.geometry,bool(in.lane.calc_smooth),in.lane.max_iterations,bool(in.temporary),observer);}

 outputs[tid]=out;
}
kernel void p2_advance(device const Input* inputs [[buffer(0)]],device Output* outputs [[buffer(1)]],constant NativeFormulaProgram* programs [[buffer(2)]],device Checkpoint* checkpoints [[buffer(3)]],device uint* status [[buffer(4)]],constant uint4& control [[buffer(5)]],device uint* trace [[buffer(6)]],uint tid [[thread_position_in_grid]]){
 if(control.x!=ProbeMagic||!control.z||control.z>(1u<<20)){status[tid]=0x80000001;return;}
 if(checkpoints[tid].magic&&(checkpoints[tid].magic!=control.x||checkpoints[tid].job!=control.y||checkpoints[tid].sequence!=control.w)){status[tid]=0x80000002;return;}
 auto s=checkpoints[tid];auto in=inputs[tid];
 if(!s.magic){if(control.w){status[tid]=0x80000002;return;}s={};s.magic=control.x;s.job=control.y;s.lane=in.lane;s.geometry=in.geometry;
  // Poison inactive child storage. Each owning tag must initialize its frame
  // before reading it; unentered body alternatives intentionally retain poison.
  for(uint k=0;k<sizeof(s.combined);++k)reinterpret_cast<thread uchar*>(&s.combined)[k]=0xa5;
  for(uint k=0;k<sizeof(s.distance);++k)reinterpret_cast<thread uchar*>(&s.distance)[k]=0xa5;
  for(uint k=0;k<sizeof(s.replay);++k)reinterpret_cast<thread uchar*>(&s.replay)[k]=0xa5;
  for(uint k=0;k<sizeof(s.orbit);++k)reinterpret_cast<thread uchar*>(&s.orbit)[k]=0xa5;
  for(uint k=0;k<sizeof(s.step);++k)reinterpret_cast<thread uchar*>(&s.step)[k]=0xa5;
  for(uint k=0;k<sizeof(s.wrapper);++k)reinterpret_cast<thread uchar*>(&s.wrapper)[k]=0xa5;
  for(uint k=0;k<sizeof(s.body);++k)reinterpret_cast<thread uchar*>(&s.body)[k]=0xa5;
  for(uint k=0;k<3;++k)s.entry.point[k]=in.point[k];s.entry.stop=in.stop;s.entry.smooth=in.lane.calc_smooth;s.entry.max_iterations=in.lane.max_iterations;s.entry.temporary_maximum=in.temporary;
 }
 if(s.done){++checkpoints[tid].sequence;status[tid]=99;return;}
 BIND_RESOURCES(s);EntryObserver observer{trace,tid,s.transitions};
 for(uint step=0;step<control.z&&!s.done;++step){
  uint before=s.entry.header.pc;
  s.done=entry_advance(s.entry,s.combined,s.distance,s.replay,s.orbit,s.step,s.wrapper,s.body,programs[2*tid],resources,s.lane,s.geometry)==99;
  // A physical return must restore every caller-owned observer address.
  if(resources.smoothing!=((in.observe_mask&1)?&s.smoothing:nullptr)||resources.carried_reads!=((in.observe_mask&2)?&s.carried:nullptr)||resources.range_exits!=((in.observe_mask&4)?&s.range:nullptr)){status[tid]=0x80000004;return;}
  uint after=s.entry.header.pc;
  if(after!=before&&(after==1||after==2))observer.record(after,s.lane,s.entry.child,s.geometry,uint(bool(s.entry.smooth)),s.combined.max_iterations,s.entry.temporary_maximum,resources.radius_stop,s.entry.range_clamps);
 }
 s.transitions=observer.transitions;++s.sequence;checkpoints[tid]=s;status[tid]=s.done?99:s.entry.header.pc;
 if(s.done)outputs[tid]={s.entry.child,s.lane,s.smoothing,s.carried,s.range,s.clamps,s.geometry};
}
#endif
