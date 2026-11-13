// Experimental raw-body differential probe. Not a renderer/production entry.
#define MB3D_METAL_NATIVE_FLOAT 1
#include "src/render/native_metal/formula_math.metal.h"
#include "src/render/native_metal/generated/formula_10274.metal.h"
#include "src/render/native_metal/generated/formula_10265.metal.h"
#include "src/render/native_metal/generated/formula_10220.metal.h"
#include "continuation_10220.metal.h"
#include "src/render/native_metal/generated/formula_10321.metal.h"
#include "continuation_10321.metal.h"
#include "continuation_10274.metal.h"
#include "continuation_10265.metal.h"
#include "src/render/native_metal/generated/formula_10089.metal.h"
#include "continuation_10089.metal.h"
using namespace metal;
using namespace mb3d_native;
struct Input {float point[3],params[64];uint id,local_limit,limit,work;};
struct Output {float value[3],de,params[64];int counter;uint written;NativeFormulaFault fault;};
struct ProbeObserver {
 device uint* trace; uint id,tid,transitions;
 template<class Frame> void record(thread const Frame& frame,FormulaParams params,float de) thread {
  const bool done=frame.pc==99;if(transitions>=128){params.fault->code=InvalidFormulaAbi;return;}
  uint canonical[128]{};canonical[0]=id;canonical[1]=done;
  for(uint k=0;k<64;++k)canonical[2+k]=as_type<uint>(params.data[k]);
  canonical[66]=params.fault->code;canonical[67]=params.fault->formula_id;canonical[68]=params.fault->slot;canonical[69]=params.fault->iteration;
  canonical[70]=params.fault->work;canonical[71]=params.fault->limit;canonical[72]=params.fault->reserved[0];canonical[73]=params.fault->reserved[1];
  canonical[74]=as_type<uint>(de);canonical[75]=uint(*params.counter);canonical[76]=uint(*params.counter_written);
  observe(frame,canonical+77);
  for(uint k=0;k<128;++k)trace[(tid*128+transitions)*128+k]=canonical[k];
  ++transitions;
 }
};

#include "observer_10274.metal.h"
#include "observer_10265.metal.h"
#include "observer_10220.metal.h"
#include "observer_10321.metal.h"
#include "observer_10089.metal.h"
struct Checkpoint {
 uint magic,job,sequence,done,transitions;
 float data[64];NativeFormulaFault fault;float de;int counter;bool written;
 continuation_10274::Frame tess;
 continuation_10265::Frame pine;
 continuation_10220::Frame columns;
 continuation_10321::Frame quat;
 continuation_10089::Frame gnarly;
};
inline void publish(device Output& o,FormulaVec3 value,float de,thread float* data,int counter,bool written,NativeFormulaFault fault){
 o.value[0]=value.x;o.value[1]=value.y;o.value[2]=value.z;o.de=de;
 for(uint i=0;i<64;++i)o.params[i]=data[i];o.counter=counter;o.written=written;o.fault=fault;
}
kernel void p2_abi(device uint* out [[buffer(0)]]){
 out[0]=0x50320004;out[1]=sizeof(Input);out[2]=sizeof(Output);out[3]=sizeof(Checkpoint);
}
kernel void p2_original(device const Input* inputs [[buffer(0)]],device Output* outputs [[buffer(1)]],constant NativeFormulaSlot* slots [[buffer(2)]],uint tid [[thread_position_in_grid]]){
 const auto in=inputs[tid];float data[64];for(uint i=0;i<64;++i)data[i]=in.params[i];
 NativeFormulaFault fault{};fault.limit=in.limit;fault.work=in.work;
 NativeResources resources{};float de=0;int counter=0;bool written=false;
 FormulaParams params{data,&slots[tid],&resources,&fault,&counter,&written,in.local_limit};
 FormulaVec3 point(in.point[0],in.point[1],in.point[2]);
 FormulaVec3 result=in.id==10274?formula_10274::execute_formula<10274>(point,params,de):in.id==10265?formula_10265::execute_formula<10265>(point,params,de):in.id==10220?formula_10220::execute_formula<10220>(point,params,de):in.id==10321?formula_10321::execute_formula<10321>(point,params,de):formula_10089::execute_formula<10089>(point,params,de);
 publish(outputs[tid],result,de,data,counter,written,fault);
}
kernel void p2_observer(device const Input* inputs [[buffer(0)]],device Output* outputs [[buffer(1)]],constant NativeFormulaSlot* slots [[buffer(2)]],device uint* trace [[buffer(6)]],uint tid [[thread_position_in_grid]]){
 const auto in=inputs[tid];float data[64];for(uint i=0;i<64;++i)data[i]=in.params[i];
 NativeFormulaFault fault{};fault.limit=in.limit;fault.work=in.work;
 NativeResources resources{};float de=0;int counter=0;bool written=false;
 FormulaParams params{data,&slots[tid],&resources,&fault,&counter,&written,in.local_limit};
 FormulaVec3 point(in.point[0],in.point[1],in.point[2]);
 ProbeObserver observer{trace,in.id,tid,0};
 FormulaVec3 result=in.id==10274?continuation_10274::original_observed(point,params,de,observer):in.id==10265?continuation_10265::original_observed(point,params,de,observer):in.id==10220?continuation_10220::original_observed(point,params,de,observer):in.id==10321?continuation_10321::original_observed(point,params,de,observer):continuation_10089::original_observed(point,params,de,observer);
 publish(outputs[tid],result,de,data,counter,written,fault);
}
kernel void p2_advance(device const Input* inputs [[buffer(0)]],device Output* outputs [[buffer(1)]],constant NativeFormulaSlot* slots [[buffer(2)]],device Checkpoint* checkpoints [[buffer(3)]],device uint* status [[buffer(4)]],constant uint4& control [[buffer(5)]],device uint* trace [[buffer(6)]],uint tid [[thread_position_in_grid]]){
 if(control.x!=0x50320004||!control.z||control.z>(1u<<20)){status[tid]=0x80000001;return;}
 if(checkpoints[tid].magic&&(checkpoints[tid].magic!=control.x||checkpoints[tid].job!=control.y||checkpoints[tid].sequence!=control.w)){
  status[tid]=0x80000002;return;
 }
 auto s=checkpoints[tid];const auto in=inputs[tid];
 if(!s.magic){
  if(control.w){status[tid]=0x80000002;return;}
  s={};s.magic=control.x;s.job=control.y;
  for(uint i=0;i<64;++i)s.data[i]=in.params[i];s.fault.limit=in.limit;s.fault.work=in.work;
 }
 if(s.done){++checkpoints[tid].sequence;status[tid]=99;return;}
 NativeResources resources{};
 FormulaParams params{s.data,&slots[tid],&resources,&s.fault,&s.counter,&s.written,in.local_limit};
 FormulaVec3 point(in.point[0],in.point[1],in.point[2]);
 for(uint step=0;step<control.z&&!s.done;++step){
  if(s.transitions>=128){status[tid]=0x80000003;return;}
  if(in.id==10274){continuation_10274::step(s.tess,point,params,s.de);s.done=s.tess.pc==99;}
  else if(in.id==10265){continuation_10265::step(s.pine,point,params,s.de);s.done=s.pine.pc==99;}
  else if(in.id==10220){continuation_10220::step(s.columns,point,params,s.de);s.done=s.columns.pc==99;}
  else if(in.id==10321){continuation_10321::step(s.quat,point,params,s.de);s.done=s.quat.pc==99;}
  else if(in.id==10089){continuation_10089::step(s.gnarly,point,params,s.de);s.done=s.gnarly.pc==99;}
  else {s.fault.code=UnsupportedFormula;s.done=1;}
  ProbeObserver observer{trace,in.id,tid,s.transitions};
  if(in.id==10274)observer.record(s.tess,params,s.de);
  else if(in.id==10265)observer.record(s.pine,params,s.de);
  else if(in.id==10220)observer.record(s.columns,params,s.de);
  else if(in.id==10321)observer.record(s.quat,params,s.de);
  else if(in.id==10089)observer.record(s.gnarly,params,s.de);
  ++s.transitions;
 }
 ++s.sequence;checkpoints[tid]=s;
 status[tid]=s.done?99:(in.id==10274?s.tess.pc:in.id==10265?s.pine.pc:in.id==10220?s.columns.pc:in.id==10321?s.quat.pc:s.gnarly.pc);
 if(s.done)publish(outputs[tid],in.id==10274?s.tess.result:in.id==10265?s.pine.result:in.id==10220?s.columns.result:in.id==10321?s.quat.result:s.gnarly.result,s.de,s.data,s.counter,s.written,s.fault);
}
