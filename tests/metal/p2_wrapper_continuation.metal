// P1 source-bound caller proof. No production render entry point.
#define MB3D_METAL_NATIVE_FLOAT 1
#include "src/render/native_metal/formula_wrapper.metal.h"
using namespace metal;
using namespace mb3d_native;
struct Input {float point[3],params[64];uint id,local_limit,limit,work;NativeFormulaState lane;};
struct Output {NativeFormulaState lane;NativeFormulaFault fault;uint success;};
#ifndef P2_REFERENCE
#include "src/render/native_metal/continuation_step.metal.h"
using namespace mb3d_native::continuation;
struct Checkpoint {uint magic,job,sequence,done,success;NativeFormulaState lane;NativeFormulaFault fault;WrapperFrame wrapper;BodyFrame body;};
struct WrapperObserver {
 device uint* trace;uint tid;
 void record(thread const WrapperFrame& w,uint phase,thread const NativeFormulaFault& fault) thread {
  uint v[128]{};uint n=0;
  for(uint i=0;i<64;++i)v[n++]=as_type<uint>(w.a[i]);
  v[n++]=w.count;v[n++]=w.traits;v[n++]=uint(w.option);v[n++]=w.flags;
  v[n++]=w.n;v[n++]=w.c;v[n++]=uint(w.first_counter);v[n++]=w.first_counter_written;
  for(uint i=0;i<3;++i)v[n++]=as_type<uint>(w.p[i]);
  for(uint i=0;i<3;++i)v[n++]=as_type<uint>(w.result[i]);
  v[n++]=as_type<uint>(w.de);v[n++]=as_type<uint>(w.vs);v[n++]=as_type<uint>(w.saved);
  v[n++]=fault.code;v[n++]=fault.formula_id;v[n++]=fault.slot;v[n++]=fault.iteration;
  v[n++]=fault.work;v[n++]=fault.limit;v[n++]=fault.reserved[0];v[n++]=fault.reserved[1];
  for(uint k=0;k<128;++k)trace[(tid*2+phase-1)*128+k]=v[k];
 }
};
#endif
kernel void p2_abi(device uint* out [[buffer(0)]]){
 out[0]=0x50321001;out[1]=sizeof(Input);out[2]=sizeof(Output);
#ifdef P2_REFERENCE
 out[3]=0;
#else
 out[3]=sizeof(Checkpoint);
#endif
}
kernel void p2_original(device const Input* inputs [[buffer(0)]],device Output* outputs [[buffer(1)]],constant NativeFormulaSlot* slots [[buffer(2)]],uint tid [[thread_position_in_grid]]){
 const auto in=inputs[tid];auto lane=in.lane;NativeFormulaFault fault{};fault.limit=in.limit;fault.work=in.work;
 NativeResources resources{};bool success=native_formula_step_body<>(lane,slots[tid],resources,fault,in.local_limit);
 outputs[tid]={lane,fault,uint(success)};
}
#ifndef P2_REFERENCE
kernel void p2_observer(device const Input* inputs [[buffer(0)]],device Output* outputs [[buffer(1)]],constant NativeFormulaSlot* slots [[buffer(2)]],device uint* trace [[buffer(6)]],uint tid [[thread_position_in_grid]]){
 const auto in=inputs[tid];auto lane=in.lane;NativeFormulaFault fault{};fault.limit=in.limit;fault.work=in.work;
 NativeResources resources{};WrapperObserver observer{trace,tid};
 bool success=wrapper_original_observed(lane,slots[tid],resources,fault,in.local_limit,observer);
 outputs[tid]={lane,fault,uint(success)};
}
kernel void p2_advance(device const Input* inputs [[buffer(0)]],device Output* outputs [[buffer(1)]],constant NativeFormulaSlot* slots [[buffer(2)]],device Checkpoint* checkpoints [[buffer(3)]],device uint* status [[buffer(4)]],constant uint4& control [[buffer(5)]],device uint* trace [[buffer(6)]],uint tid [[thread_position_in_grid]]){
 if(control.x!=0x50321001||!control.z||control.z>(1u<<20)){status[tid]=0x80000001;return;}
 if(checkpoints[tid].magic&&(checkpoints[tid].magic!=control.x||checkpoints[tid].job!=control.y||checkpoints[tid].sequence!=control.w)){status[tid]=0x80000002;return;}
 auto s=checkpoints[tid];const auto in=inputs[tid];
 if(!s.magic){if(control.w){status[tid]=0x80000002;return;}s={};s.magic=control.x;s.job=control.y;s.lane=in.lane;s.fault.limit=in.limit;s.fault.work=in.work;}
 if(s.done){++checkpoints[tid].sequence;status[tid]=99;return;}
 NativeResources resources{};WrapperObserver observer{trace,tid};bool success=bool(s.success);
 for(uint step=0;step<control.z&&!s.done;++step){
  uint before=s.wrapper.header.pc;
  s.done=wrapper_advance(s.wrapper,s.body,s.lane,slots[tid],resources,s.fault,in.local_limit,success)==99;
  if(s.wrapper.header.pc==1&&before==0)observer.record(s.wrapper,1,s.fault);
  if(s.wrapper.header.pc==2&&before==1)observer.record(s.wrapper,2,s.fault);
 }
 s.success=uint(success);++s.sequence;checkpoints[tid]=s;
 status[tid]=s.done?99:s.wrapper.header.pc;
 if(s.done)outputs[tid]={s.lane,s.fault,s.success};
}
#endif
