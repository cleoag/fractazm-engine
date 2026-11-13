#define abi primary_probe_abi
#define original primary_probe_original
#define advance primary_probe_advance
#include "p2_primary_composition.metal"
#undef advance
#undef original
#undef abi
#include "native_inside_continuation_probe_abi.h"
#include "src/render/native_metal/native_inside_math.h"
#include "inside_control.metal.h"
#ifndef P2_REFERENCE
#include "src/render/native_metal/continuation_inside_step.metal.h"
#endif
kernel void abi(device uint* out [[buffer(0)]]){out[0]=sizeof(p2_inside::Input);out[1]=sizeof(p2_inside::Output);out[3]=p2_inside::RealProbeAbi;
#ifdef P2_REFERENCE
out[2]=0;
#else
out[2]=sizeof(p2_inside::RealState);
#endif
}
kernel void original(constant p2_inside::Input* inputs [[buffer(0)]],device p2_inside::Output* outputs [[buffer(1)]],uint tid [[thread_position_in_grid]]){
 constant auto& in=inputs[tid].base;p2_inside::Output out{};out.base=initial_output(in);NativeResources resources{};
 out.base.pixel=inside_control(in.scene,in.program,inputs[tid].policy,in.camera,in.x,in.y,resources,out.base.geometry,out.base.lane,out.activity,in.normal,out.base.normal_record,out.base.normal_result);outputs[tid]=out;
}
#ifndef P2_REFERENCE
kernel void advance(constant p2_inside::Input* inputs [[buffer(0)]],device p2_inside::Output* outputs [[buffer(1)]],device p2_inside::RealState* states [[buffer(2)]],constant uint& quantum [[buffer(3)]],uint tid [[thread_position_in_grid]]){
 auto s=states[tid];constant auto& in=inputs[tid].base;
 if(!s.initialized){s={};s.initialized=1;for(uint k=0;k<sizeof(s.frame);++k)reinterpret_cast<thread uchar*>(&s.frame)[k]=0xa5;
 auto out=initial_output(in);s.frame.lane=out.lane;s.frame.geometry=out.geometry;s.frame.normal_record=out.normal_record;s.frame.normal_result=out.normal_result;s.frame.pixel=out.pixel;s.frame.inside_record={};
 s.frame.inside.header={};s.frame.camera=in.camera;s.frame.normal_params=in.normal;s.frame.owner.x=in.x;s.frame.owner.y=in.y;}
 NativeResources resources{};
 for(uint i=0;i<quantum&&!s.done;++i)s.done=inside_advance(s.frame,in.scene,in.program,inputs[tid].policy,resources)==99;
 states[tid]=s;if(s.done){p2_inside::Output out{};out.base.pixel=s.frame.pixel;out.base.lane=s.frame.lane;out.base.geometry=s.frame.geometry;out.base.normal_record=s.frame.normal_record;out.base.normal_result=s.frame.normal_result;out.activity=s.frame.inside_record;outputs[tid]=out;}
}
#endif
