#define MB3D_METAL_NATIVE_FLOAT 1
#include "src/render/native_metal/formula_wrapper.metal.h"
#include "src/render/native_metal/orbit_driver.metal.h"
using namespace metal;using namespace mb3d_native;
constant uint ProbeMagic=0x50324001;
struct Input {float point[3],params[64];uint id,local_limit,limit,work;NativeFormulaState lane;uint pass_limit,has_radius;float stop,radius_stop;uint inside_mode,observe_mask;};
struct Output {NativeOrbitResult result;NativeFormulaState lane;NativeSmoothingCounters smoothing;uint carried,range,clamps;};
#ifndef P2_REFERENCE
#include "src/render/native_metal/continuation_distance_step.metal.h"
using namespace mb3d_native::continuation;
struct Checkpoint {uint magic,job,sequence,done,transitions;NativeFormulaState lane;DistanceFrame distance;OrbitReplayFrame replay;OrbitPassFrame orbit;FormulaStepFrame step;WrapperFrame wrapper;BodyFrame body;NativeSmoothingCounters smoothing;uint carried,range,clamps;};
struct DistanceObservation {float held[10];uint meta[12];};
inline DistanceObservation distance_observation(float value,float radius,float center,float3 delta,float original,float coordinate,float scale,float stop,int maximum,int iterations,int steps,uint smooth,uint calc,uint inside,uint axis,uint limit,bool retried,bool bounded,bool representable,bool sample){
 return {{value,radius,center,delta.x,delta.y,delta.z,original,coordinate,scale,stop},{uint(maximum),uint(iterations),uint(steps),smooth,calc,inside,axis,limit,uint(retried),uint(bounded),uint(representable),uint(sample)}};
}
struct DistanceObserver {
 device uint* trace;uint tid,transitions;thread NativeFormulaState* entry;thread NativeResources* resources;thread uint* clamps;
 void record(uint phase,NativeFormulaState s,NativeFormulaFault f,uint limit,thread mb3d_native_gradient::RadiusRange* radius,DistanceObservation held) thread {
  if(transitions>=64)return;
  device uint* target=trace+(tid*64+transitions)*256;
  for(uint k=0;k<256;++k)target[k]=0;
  target[0]=phase;target[1]=limit;target[2]=radius?uint(radius->certified):0;
  target[3]=resources->range_exits?*resources->range_exits:0;target[4]=resources->carried_reads?*resources->carried_reads:0;target[5]=clamps?*clamps:0;
  target[6]=as_type<uint>(resources->radius_stop);
  *reinterpret_cast<device NativeFormulaState*>(target+8)=s;
  *reinterpret_cast<device NativeFormulaState*>(target+55)=*entry;
  *reinterpret_cast<device NativeFormulaFault*>(target+102)=f;
  if(resources->smoothing)*reinterpret_cast<device NativeSmoothingCounters*>(target+110)=*resources->smoothing;
  for(uint k=0;k<10;++k)target[114+k]=as_type<uint>(held.held[k]);
  for(uint k=0;k<12;++k)target[124+k]=held.meta[k];
  ++transitions;
 }
 void frame(uint site,bool after,thread const Checkpoint& c) thread {
  thread const auto& d=c.distance;DistanceObservation held;
  if(site==1)held=distance_observation(after?d.value:0.f,0.f,0.f,float3(0),0.f,0.f,0.f,d.stop,d.max_iterations,0,0,0,d.calc_smooth,d.inside_mode,0,0,false,false,false,false);
  else if(site==2)held=distance_observation(d.value,d.radius,d.center,float3(d.axis>0?d.delta[0]:0.f,d.axis>1?d.delta[1]:0.f,0.f),d.original,d.sample_coordinate,d.sample_scale,d.stop,d.max_iterations,d.iterations,d.steps,d.saved_smooth,d.calc_smooth,d.inside_mode,d.axis,d.limit,d.retried,d.bounded_minimum,d.representable,true);
  else held=distance_observation(d.value,d.radius,d.center,float3(0),0.f,0.f,0.f,d.stop,d.max_iterations,d.iterations,d.steps,d.saved_smooth,d.calc_smooth,d.inside_mode,0,d.limit,d.retried,d.bounded_minimum,d.representable,false);
  mb3d_native_gradient::RadiusRange radius;radius.certified=d.radius_certified;
  record(site+(after?100u:0u),d.result.state,d.result.fault,site==2?d.limit:site==3?0u:0xffffffffu,site==2?&radius:nullptr,held);
 }
};
template<uint Part=0,class Observer> inline float distance_observed_orbit(constant NativeFormulaProgram& p,thread NativeFormulaState& s,thread const NativeResources& resources,thread NativeFormulaFault& f,thread mb3d_native_gradient::RadiusRange* radius,uint limit,thread Observer& observer,uint site,DistanceObservation held){
 observer.record(site,s,f,limit,radius,held);
 const float result=native_run_orbit<Part>(p,s,resources,f,radius,limit);
 if(site==1)held.held[0]=result;
 observer.record(site+100,s,f,limit,radius,held);return result;
}
#include "distance_observer.metal.h"
#endif
kernel void p2_abi(device uint* out [[buffer(0)]]){
 out[0]=ProbeMagic;out[1]=sizeof(Input);out[2]=sizeof(Output);
#ifdef P2_REFERENCE
 out[3]=0;
#else
 out[3]=sizeof(Checkpoint);
#endif
}
#define BIND_RESOURCES(owner) NativeResources resources{};resources.radius_stop=in.radius_stop;resources.smoothing=(in.observe_mask&1)?&owner.smoothing:nullptr;resources.carried_reads=(in.observe_mask&2)?&owner.carried:nullptr;resources.range_exits=(in.observe_mask&4)?&owner.range:nullptr;thread uint* clamps=(in.observe_mask&8)?&owner.clamps:nullptr
kernel void p2_original(device const Input* inputs [[buffer(0)]],device Output* outputs [[buffer(1)]],constant NativeFormulaProgram* programs [[buffer(2)]],uint tid [[thread_position_in_grid]]){
 auto in=inputs[tid];Output out{};out.lane=in.lane;BIND_RESOURCES(out);
 out.result=native_distance_controlled(float3(in.point[0],in.point[1],in.point[2]),in.stop,programs[tid],resources,out.lane,in.lane.calc_smooth,in.lane.max_iterations,clamps,in.inside_mode);outputs[tid]=out;
}
#ifndef P2_REFERENCE
kernel void p2_observer(device const Input* inputs [[buffer(0)]],device Output* outputs [[buffer(1)]],constant NativeFormulaProgram* programs [[buffer(2)]],device uint* trace [[buffer(6)]],uint tid [[thread_position_in_grid]]){
 auto in=inputs[tid];Output out{};out.lane=in.lane;BIND_RESOURCES(out);
 DistanceObserver observer{trace,tid,0,&out.lane,&resources,clamps};
 out.result=distance_original_observed(float3(in.point[0],in.point[1],in.point[2]),in.stop,programs[tid],resources,out.lane,in.lane.calc_smooth,in.lane.max_iterations,clamps,in.inside_mode,observer);outputs[tid]=out;
}
kernel void p2_advance(device const Input* inputs [[buffer(0)]],device Output* outputs [[buffer(1)]],constant NativeFormulaProgram* programs [[buffer(2)]],device Checkpoint* checkpoints [[buffer(3)]],device uint* status [[buffer(4)]],constant uint4& control [[buffer(5)]],device uint* trace [[buffer(6)]],uint tid [[thread_position_in_grid]]){
 if(control.x!=ProbeMagic||!control.z||control.z>(1u<<20)){status[tid]=0x80000001;return;}
 if(checkpoints[tid].magic&&(checkpoints[tid].magic!=control.x||checkpoints[tid].job!=control.y||checkpoints[tid].sequence!=control.w)){status[tid]=0x80000002;return;}
 auto s=checkpoints[tid];auto in=inputs[tid];
 if(!s.magic){if(control.w){status[tid]=0x80000002;return;}s={};s.magic=control.x;s.job=control.y;s.lane=in.lane;
  for(uint k=0;k<3;++k)s.distance.point[k]=in.point[k];s.distance.stop=in.stop;s.distance.calc_smooth=in.lane.calc_smooth;s.distance.max_iterations=in.lane.max_iterations;s.distance.inside_mode=in.inside_mode;
 }
 if(s.done){++checkpoints[tid].sequence;status[tid]=99;return;}
 BIND_RESOURCES(s);DistanceObserver observer{trace,tid,s.transitions,&s.lane,&resources,clamps};
 for(uint step=0;step<control.z&&!s.done;++step){
  if(observer.transitions>=64){status[tid]=0x80000003;return;}
  uint before=s.distance.header.pc;
  s.done=distance_advance(s.distance,s.replay,s.orbit,s.step,s.wrapper,s.body,programs[tid],resources,s.lane,clamps)==99;
  uint after=s.distance.header.pc;
  if(after!=before){
   if(after==1||after==5||after==8||after==10)observer.frame(after==1?1:after==5?2:after==8?3:4,false,s);
   if(before==1||before==5||before==8||before==10)observer.frame(before==1?1:before==5?2:before==8?3:4,true,s);
  }
 }
 s.transitions=observer.transitions;++s.sequence;checkpoints[tid]=s;status[tid]=s.done?99:s.distance.header.pc;
 if(s.done)outputs[tid]={s.distance.result,s.lane,s.smoothing,s.carried,s.range,s.clamps};
}
#endif
