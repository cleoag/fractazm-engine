#define MB3D_METAL_NATIVE_FLOAT 1
#include "src/render/native_metal/formula_wrapper.metal.h"
#include "src/render/native_metal/orbit_driver.metal.h"
using namespace metal;using namespace mb3d_native;
constant uint ProbeMagic=0x50325001;
struct Input {float point[3],params[64];uint id,local_limit,limit,work;NativeFormulaState lane;uint pass_limit,has_radius;float stop,radius_stop;uint inside_mode,observe_mask,temporary;};
struct Output {NativeOrbitResult result;NativeFormulaState lane;NativeSmoothingCounters smoothing;uint carried,range,clamps;};
#ifndef P2_REFERENCE
#include "src/render/native_metal/continuation_combined_step.metal.h"
using namespace mb3d_native::continuation;
struct Checkpoint {uint magic,job,sequence,done,transitions;NativeFormulaState lane;CombinedFrame combined;DistanceFrame distance;OrbitReplayFrame replay;OrbitPassFrame orbit;FormulaStepFrame step;WrapperFrame wrapper;BodyFrame body;NativeSmoothingCounters smoothing;uint carried,range,clamps;};
struct CombinedObserver {
 device uint* trace;uint tid,transitions;thread NativeFormulaState* external;thread NativeResources* resources;thread uint* clamps;
 __attribute__((noinline)) void record(uint phase,thread const NativeFormulaState& state,thread const NativeOrbitResult& child,thread const NativeOrbitResult& first,uint calc,int maximum,uint inside) thread {
  if(transitions>=64)return;
  device uint* target=trace+(tid*64+transitions)*256;
  for(uint k=0;k<256;++k)target[k]=0;
  target[0]=phase;target[1]=calc;target[2]=uint(maximum);target[3]=inside;
  target[4]=resources->range_exits?*resources->range_exits:0;target[5]=resources->carried_reads?*resources->carried_reads:0;target[6]=clamps?*clamps:0;target[7]=as_type<uint>(resources->radius_stop);
  *reinterpret_cast<device NativeFormulaState*>(target+8)=state;
  *reinterpret_cast<device NativeFormulaState*>(target+55)=*external;
  *reinterpret_cast<device NativeOrbitResult*>(target+102)=child;
  *reinterpret_cast<device NativeOrbitResult*>(target+161)=first;
  if(resources->smoothing)*reinterpret_cast<device NativeSmoothingCounters*>(target+220)=*resources->smoothing;
  target[224]=native_selected_formula_type();++transitions;
 }
 void frame(uint site,bool after,thread const Checkpoint& c) thread {
  thread const auto& p=c.combined;
  uint inside=site==1?InsideFromLane:site==3&&p.type==CombineMaximumInverse?InsideForced:InsideDeferred;
  record(site+(after?100u:0u),site==1?c.lane:p.state,after?c.distance.result:NativeOrbitResult{},site==3?p.first:NativeOrbitResult{},p.calc_smooth,site==3?p.second_maximum:p.max_iterations,inside);
 }
};
#include "combined_observer.metal.h"
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
 out.result=native_distance_combined(float3(in.point[0],in.point[1],in.point[2]),in.stop,programs[2*tid],resources,out.lane,in.lane.calc_smooth,in.lane.max_iterations,bool(in.temporary),clamps);outputs[tid]=out;
}
#ifndef P2_REFERENCE
kernel void p2_observer(device const Input* inputs [[buffer(0)]],device Output* outputs [[buffer(1)]],constant NativeFormulaProgram* programs [[buffer(2)]],device uint* trace [[buffer(6)]],uint tid [[thread_position_in_grid]]){
 auto in=inputs[tid];Output out{};out.lane=in.lane;BIND_RESOURCES(out);CombinedObserver observer{trace,tid,0,&out.lane,&resources,clamps};
 out.result=combined_original_observed(float3(in.point[0],in.point[1],in.point[2]),in.stop,programs[2*tid],resources,out.lane,in.lane.calc_smooth,in.lane.max_iterations,bool(in.temporary),clamps,observer);outputs[tid]=out;
}
kernel void p2_advance(device const Input* inputs [[buffer(0)]],device Output* outputs [[buffer(1)]],constant NativeFormulaProgram* programs [[buffer(2)]],device Checkpoint* checkpoints [[buffer(3)]],device uint* status [[buffer(4)]],constant uint4& control [[buffer(5)]],device uint* trace [[buffer(6)]],uint tid [[thread_position_in_grid]]){
 if(control.x!=ProbeMagic||!control.z||control.z>(1u<<20)){status[tid]=0x80000001;return;}
 if(checkpoints[tid].magic&&(checkpoints[tid].magic!=control.x||checkpoints[tid].job!=control.y||checkpoints[tid].sequence!=control.w)){status[tid]=0x80000002;return;}
 auto s=checkpoints[tid];auto in=inputs[tid];
 if(!s.magic){if(control.w){status[tid]=0x80000002;return;}s={};s.magic=control.x;s.job=control.y;s.lane=in.lane;
  for(uint k=0;k<3;++k)s.combined.point[k]=in.point[k];s.combined.stop=in.stop;s.combined.calc_smooth=in.lane.calc_smooth;s.combined.max_iterations=in.lane.max_iterations;s.combined.temporary_maximum=in.temporary;
 }
 if(s.done){++checkpoints[tid].sequence;status[tid]=99;return;}
 BIND_RESOURCES(s);CombinedObserver observer{trace,tid,s.transitions,&s.lane,&resources,clamps};
 for(uint step=0;step<control.z&&!s.done;++step){
  if(observer.transitions>=64){status[tid]=0x80000003;return;}
  uint before=s.combined.header.pc;
  s.done=combined_advance(s.combined,s.distance,s.replay,s.orbit,s.step,s.wrapper,s.body,programs[2*tid],resources,s.lane,clamps)==99;
  uint after=s.combined.header.pc;
  if(after!=before){
   if(after==1||after==2||after==4)observer.frame(after==4?3:after,false,s);
   if(before==1||before==2||before==4)observer.frame(before==4?3:before,true,s);
  }
 }
 s.transitions=observer.transitions;++s.sequence;checkpoints[tid]=s;status[tid]=s.done?99:s.combined.header.pc;
 if(s.done)outputs[tid]={s.combined.result,s.lane,s.smoothing,s.carried,s.range,s.clamps};
}
#endif
