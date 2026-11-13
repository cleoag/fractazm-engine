#define MB3D_METAL_NATIVE_FLOAT 1
#include "src/render/native_metal/formula_wrapper.metal.h"
#include "src/render/native_metal/orbit_driver.metal.h"
using namespace metal;using namespace mb3d_native;
#ifdef P2_REPLAY
constant uint ProbeMagic=0x50323001;
#else
constant uint ProbeMagic=0x50322001;
#endif
struct Input {float point[3],params[64];uint id,local_limit,limit,work;NativeFormulaState lane;uint pass_limit,has_radius;};
struct Output {NativeFormulaState lane;NativeFormulaFault fault;float value;uint exit_pass,inert,radius;NativeSmoothingCounters smoothing;uint carried;
#ifdef P2_REPLAY
uint range;
#endif
};
#ifndef P2_REFERENCE
#include "src/render/native_metal/continuation_orbit_step.metal.h"
#ifdef P2_REPLAY
#include "src/render/native_metal/continuation_replay_step.metal.h"
#endif
using namespace mb3d_native::continuation;
struct Checkpoint {uint magic,job,sequence,done,transitions;NativeFormulaState lane;NativeFormulaFault fault;OrbitPassFrame orbit;FormulaStepFrame step;WrapperFrame wrapper;BodyFrame body;NativeSmoothingCounters smoothing;uint carried,radius;
#ifdef P2_REPLAY
OrbitReplayFrame replay;uint range;
#endif
};
struct OrbitObserver {
 device uint* trace;uint tid,transitions;
#ifdef P2_REPLAY
 thread uint* ranges;
#ifdef P2_OUTLINE_OBSERVER
 __attribute__((noinline))
#endif
 void outer(uint phase,NativeFormulaState s,NativeFormulaState entry,thread const NativeFormulaFault& f,uint limit,float result,uint exit_pass,bool inert,thread mb3d_native_gradient::RadiusRange* radius) thread {
  if(transitions>=128)return;
  device uint* target=trace+(tid*128+transitions)*128;
  for(uint k=0;k<128;++k)target[k]=0;
  target[0]=phase;target[1]=limit;target[2]=as_type<uint>(result);target[3]=exit_pass;target[4]=uint(inert);target[5]=radius?uint(radius->certified):0;target[6]=*ranges;
  *reinterpret_cast<device NativeFormulaState*>(target+8)=s;
  *reinterpret_cast<device NativeFormulaState*>(target+55)=entry;
  *reinterpret_cast<device NativeFormulaFault*>(target+102)=f;
  ++transitions;
 }
#endif
 void record(uint phase,bool ok,NativeFormulaState s,thread const NativeFormulaFault& f,uint slot,int remaining,int minimum,bool improved,bool last,uint pass,uint index,bool overflow,bool inert,thread mb3d_native_gradient::RadiusRange* radius,float4 held,float derivative) thread {
  if(transitions>=128)return;
  uint v[128]{};uint n=0;v[n++]=phase;v[n++]=ok;v[n++]=slot;v[n++]=uint(remaining);v[n++]=uint(minimum);v[n++]=improved;v[n++]=last;v[n++]=pass;v[n++]=index;v[n++]=overflow;v[n++]=inert;v[n++]=radius?uint(radius->certified):0;
  static_assert(sizeof(NativeFormulaState)==47*4,"canonical lane layout");
  device NativeFormulaState* target=reinterpret_cast<device NativeFormulaState*>(trace+(tid*128+transitions)*128+12);
  *target=s;n+=47;
  v[n++]=f.code;v[n++]=f.formula_id;v[n++]=f.slot;v[n++]=f.iteration;v[n++]=f.work;v[n++]=f.limit;v[n++]=f.reserved[0];v[n++]=f.reserved[1];
  for(uint k=0;k<4;++k)v[n++]=as_type<uint>(held[k]);v[n++]=as_type<uint>(derivative);
  for(uint k=0;k<128;++k)if(k<12||k>=59)trace[(tid*128+transitions)*128+k]=v[k];++transitions;
 }
 void frame(uint phase,bool ok,thread const Checkpoint& c,thread mb3d_native_gradient::RadiusRange* radius) thread {
  thread const auto& o=c.orbit;float4 held(0);float derivative=0;
  if(phase==2||phase==102){for(uint k=0;k<4;++k)held[k]=o.saved[k];derivative=o.old_derivative;}
  if(phase==4||phase==104){for(uint k=0;k<4;++k)held[k]=o.first[k];derivative=o.first_derivative;}
  record(phase,ok,c.lane,c.fault,o.slot,o.remaining,o.minimum_iteration,o.improved,o.last_improved,o.pass,o.index,o.overflow,o.inert,radius,held,derivative);
 }
};
#include "orbit_observer.metal.h"
#ifdef P2_REPLAY
#include "replay_observer.metal.h"
#endif
#endif
kernel void p2_abi(device uint* out [[buffer(0)]]){
 out[0]=ProbeMagic;out[1]=sizeof(Input);out[2]=sizeof(Output);
#ifdef P2_REFERENCE
 out[3]=0;
#else
 out[3]=sizeof(Checkpoint);
#endif
}
kernel void p2_original(device const Input* inputs [[buffer(0)]],device Output* outputs [[buffer(1)]],constant NativeFormulaProgram* programs [[buffer(2)]],uint tid [[thread_position_in_grid]]){
 auto in=inputs[tid];auto lane=in.lane;NativeFormulaFault f{};f.limit=in.limit;f.work=in.work;
 NativeSmoothingCounters smoothing{};uint carried=0;NativeResources resources{};resources.smoothing=&smoothing;resources.carried_reads=&carried;
 mb3d_native_gradient::RadiusRange radius{};uint exit_pass=0xffffffff;bool inert=false;
#ifdef P2_REPLAY
 uint ranges=0;resources.range_exits=&ranges;
 float value=native_run_orbit(programs[tid],lane,resources,f,in.has_radius?&radius:nullptr,in.pass_limit);
 outputs[tid]={lane,f,value,exit_pass,uint(inert),uint(radius.certified),smoothing,carried,ranges};
#else
 float value=native_orbit_passes(programs[tid],lane,resources,f,in.has_radius?&radius:nullptr,in.pass_limit,exit_pass,inert);
 outputs[tid]={lane,f,value,exit_pass,uint(inert),uint(radius.certified),smoothing,carried};
#endif
}
#ifndef P2_REFERENCE
kernel void p2_observer(device const Input* inputs [[buffer(0)]],device Output* outputs [[buffer(1)]],constant NativeFormulaProgram* programs [[buffer(2)]],device uint* trace [[buffer(6)]],uint tid [[thread_position_in_grid]]){
 auto in=inputs[tid];auto lane=in.lane;NativeFormulaFault f{};f.limit=in.limit;f.work=in.work;
 NativeSmoothingCounters smoothing{};uint carried=0;NativeResources resources{};resources.smoothing=&smoothing;resources.carried_reads=&carried;
 mb3d_native_gradient::RadiusRange radius{};uint exit_pass=0xffffffff;bool inert=false;
#ifdef P2_REPLAY
 uint ranges=0;resources.range_exits=&ranges;
 OrbitObserver observer{trace,tid,0,&ranges};
 float value=replay_original_observed(programs[tid],lane,resources,f,in.has_radius?&radius:nullptr,in.pass_limit,observer);
 outputs[tid]={lane,f,value,exit_pass,uint(inert),uint(radius.certified),smoothing,carried,ranges};
#else
 OrbitObserver observer{trace,tid,0};
 float value=orbit_original_observed(programs[tid],lane,resources,f,in.has_radius?&radius:nullptr,in.pass_limit,exit_pass,inert,observer);
 outputs[tid]={lane,f,value,exit_pass,uint(inert),uint(radius.certified),smoothing,carried};
#endif
}
kernel void p2_advance(device const Input* inputs [[buffer(0)]],device Output* outputs [[buffer(1)]],constant NativeFormulaProgram* programs [[buffer(2)]],device Checkpoint* checkpoints [[buffer(3)]],device uint* status [[buffer(4)]],constant uint4& control [[buffer(5)]],device uint* trace [[buffer(6)]],uint tid [[thread_position_in_grid]]){
 if(control.x!=ProbeMagic||!control.z||control.z>(1u<<20)){status[tid]=0x80000001;return;}
 if(checkpoints[tid].magic&&(checkpoints[tid].magic!=control.x||checkpoints[tid].job!=control.y||checkpoints[tid].sequence!=control.w)){status[tid]=0x80000002;return;}
 auto s=checkpoints[tid];auto in=inputs[tid];
 if(!s.magic){if(control.w){status[tid]=0x80000002;return;}s={};s.magic=control.x;s.job=control.y;s.lane=in.lane;s.fault.limit=in.limit;s.fault.work=in.work;s.orbit.pass_limit=in.pass_limit;s.orbit.exit_pass=0xffffffff;}
 if(s.done){++checkpoints[tid].sequence;status[tid]=99;return;}
 NativeResources resources{};resources.smoothing=&s.smoothing;resources.carried_reads=&s.carried;
 mb3d_native_gradient::RadiusRange radius{};radius.certified=bool(s.radius);thread auto* rp=in.has_radius?&radius:nullptr;
#ifdef P2_REPLAY
 resources.range_exits=&s.range;
 OrbitObserver observer{trace,tid,s.transitions,&s.range};
#else
 OrbitObserver observer{trace,tid,s.transitions};
#endif
 for(uint step=0;step<control.z&&!s.done;++step){
  if(observer.transitions>=128){status[tid]=0x80000003;return;}
#ifndef P2_REPLAY
  uint before=s.orbit.header.pc;
#endif
#ifdef P2_REPLAY
  uint outer_before=s.replay.header.pc;
  s.done=orbit_replay_advance(s.replay,s.orbit,s.step,s.wrapper,s.body,programs[tid],s.lane,resources,s.fault,rp,in.pass_limit)==99;
  if(outer_before==1&&s.replay.header.pc==2)observer.outer(201,s.lane,s.replay.entry,s.fault,s.replay.pass_limit,0.f,s.replay.exit_pass,bool(s.replay.inert),rp);
#else
  s.done=orbit_pass_advance(s.orbit,s.step,s.wrapper,s.body,programs[tid],s.lane,resources,s.fault,rp)==99;
#endif
#ifndef P2_REPLAY
  uint after=s.orbit.header.pc;
  if((after==2||after==4||after==7)&&after!=before)observer.frame(after,false,s,rp);
  if((before==2||before==4||before==7)&&after!=before)observer.frame(before+100,bool(s.step.child_success),s,rp);
#else
  if(outer_before==2&&s.replay.header.pc==3)observer.outer(202,s.lane,s.replay.entry,s.fault,s.replay.pass_limit,s.replay.result,s.replay.exit_pass,bool(s.replay.inert),rp);
#endif
 }
 s.transitions=observer.transitions;s.radius=uint(radius.certified);++s.sequence;checkpoints[tid]=s;status[tid]=s.done?99:
#ifdef P2_REPLAY
 s.replay.header.pc;
#else
 s.orbit.header.pc;
#endif
#ifdef P2_REPLAY
 if(s.done)outputs[tid]={s.lane,s.fault,s.replay.result,0xffffffffu,0,s.radius,s.smoothing,s.carried,s.range};
#else
 if(s.done)outputs[tid]={s.lane,s.fault,s.orbit.result,s.orbit.exit_pass,uint(s.orbit.inert),s.radius,s.smoothing,s.carried};
#endif
}
#endif
