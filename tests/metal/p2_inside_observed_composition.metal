#define abi primary_probe_abi
#define original primary_probe_original
#define advance primary_probe_advance
#include "p2_primary_composition.metal"
#undef advance
#undef original
#undef abi
#include "native_inside_trace_abi.h"
#include "src/render/native_metal/native_inside_math.h"
#include "inside_control.metal.h"
#ifndef P2_REFERENCE
struct InsideCapture{device p2_inside_trace::Trace* trace;void record(uint site,uint phase,uint mask,mb3d_inside::RetryState retry,uint attempt,constant mb3d_inside::Params& policy,mb3d_camera::Ray camera,uint x,uint y,NativeNormalParams params,float3 point,float stop,ExactPixel pixel,thread const NativeFormulaState& lane,thread const NativeGeometryRecord& geometry,thread const NativeNormalRecord& normal_record,thread const NativeNormalResult& normal_result,thread const mb3d_inside::Record& activity,NativeOrbitResult child) thread {const uint index=trace->count++;if(index>=p2_inside_trace::Capacity){trace->overflow=1;return;}p2_inside_trace::Snapshot s{};
 s.version=p2_inside_trace::Version;s.site=site;s.phase=phase;s.mask=mask;s.retry=retry;s.attempt=attempt;s.policy=policy;s.camera=camera;s.x=x;s.y=y;s.params=params;for(uint k=0;k<3;++k)s.point[k]=point[k];s.stop=stop;s.pixel=pixel;s.lane=lane;s.geometry=geometry;s.normal_record=normal_record;s.normal_result=normal_result;s.activity=activity;s.child=child;trace->snapshots[index]=s;}};
struct InsideObservedCapture{device p2_inside_trace::Trace* trace;__attribute__((noinline)) void record(uint site,uint phase,uint mask,mb3d_inside::RetryState retry,uint attempt,constant mb3d_inside::Params& policy,mb3d_camera::Ray camera,uint x,uint y,NativeNormalParams params,float3 point,float stop,ExactPixel pixel,thread const NativeFormulaState& lane,thread const NativeGeometryRecord& geometry,thread const NativeNormalRecord& normal_record,thread const NativeNormalResult& normal_result,thread const mb3d_inside::Record& activity,NativeOrbitResult child) thread {const uint index=trace->count++;if(index>=p2_inside_trace::Capacity){trace->overflow=1;return;}p2_inside_trace::Snapshot s{};
 s.version=p2_inside_trace::Version;s.site=site;s.phase=phase;s.mask=mask;s.retry=retry;s.attempt=attempt;s.policy=policy;s.camera=camera;s.x=x;s.y=y;s.params=params;for(uint k=0;k<3;++k)s.point[k]=point[k];s.stop=stop;s.pixel=pixel;s.lane=lane;s.geometry=geometry;s.normal_record=normal_record;s.normal_result=normal_result;s.activity=activity;s.child=child;trace->snapshots[index]=s;}};
#include "inside_observer.metal.h"
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
kernel void trace_abi(device uint* out [[buffer(0)]]){out[0]=p2_inside_trace::Version;out[1]=sizeof(p2_inside_trace::Snapshot);out[2]=sizeof(p2_inside_trace::Trace);out[3]=p2_inside_trace::Capacity;}
kernel void observe(constant p2_inside::Input* inputs [[buffer(0)]],device p2_inside::Output* outputs [[buffer(1)]],device p2_inside_trace::Trace* traces [[buffer(2)]],uint tid [[thread_position_in_grid]]){
 constant auto& in=inputs[tid].base;p2_inside::Output out{};out.base=initial_output(in);NativeResources resources{};InsideObservedCapture capture{&traces[tid]};
 out.base.pixel=inside_observed(in.scene,in.program,inputs[tid].policy,in.camera,in.x,in.y,resources,out.base.geometry,out.base.lane,out.activity,in.normal,out.base.normal_record,out.base.normal_result,capture);outputs[tid]=out;
}

kernel void advance(constant p2_inside::Input* inputs [[buffer(0)]],device p2_inside::Output* outputs [[buffer(1)]],device p2_inside::RealState* states [[buffer(2)]],constant uint& quantum [[buffer(3)]],device p2_inside_trace::Trace* traces [[buffer(4)]],uint tid [[thread_position_in_grid]]){
 auto s=states[tid];constant auto& in=inputs[tid].base;
 if(!s.initialized){s={};s.initialized=1;for(uint k=0;k<sizeof(s.frame);++k)reinterpret_cast<thread uchar*>(&s.frame)[k]=0xa5;
 auto out=initial_output(in);s.frame.lane=out.lane;s.frame.geometry=out.geometry;s.frame.normal_record=out.normal_record;s.frame.normal_result=out.normal_result;s.frame.pixel=out.pixel;s.frame.inside_record={};
 s.frame.inside.header={};s.frame.camera=in.camera;s.frame.normal_params=in.normal;s.frame.owner.x=in.x;s.frame.owner.y=in.y;}
 NativeResources resources{};InsideCapture capture{&traces[tid]};
 for(uint i=0;i<quantum&&!s.done;++i)s.done=inside_advance(s.frame,in.scene,in.program,inputs[tid].policy,resources,capture)==99;
 states[tid]=s;if(s.done){p2_inside::Output out{};out.base.pixel=s.frame.pixel;out.base.lane=s.frame.lane;out.base.geometry=s.frame.geometry;out.base.normal_record=s.frame.normal_record;out.base.normal_result=s.frame.normal_result;out.activity=s.frame.inside_record;outputs[tid]=out;}
}
#endif
