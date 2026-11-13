#define abi primary_probe_abi
#define original primary_probe_original
#define advance primary_probe_advance
#include "p2_primary_continuation.metal"
#undef advance
#undef original
#undef abi
#include "native_inside_continuation_probe_abi.h"
#include "src/render/native_metal/native_inside_math.h"
// The post-smooth synthetic child deliberately changes trap/radius as well as
// smoothing; its parent must preserve the earlier captured hit payload.
inline NativeOrbitResult inside_scripted_de(float3 point,float stop,constant NativeFormulaProgram& p,thread ScriptResources& resources,thread NativeFormulaState& lane,thread NativeGeometryRecord& record,bool smooth){
 auto result=scripted_de(point,stop,p,resources,lane,record,smooth);
 if(resources.trace->count&&resources.trace->count<=Capacity&&!record.fault){
  lane.rout=91.f;lane.otrap=73.f;lane.smooth=52.f;result.state=lane;
  thread auto& q=resources.trace->queries[resources.trace->count-1];q.after=lane;q.result=result;
 }
 return result;
}
#define NativeResources ScriptResources
#define general_primary_camera_attempt primary_control
#define general_de inside_scripted_de
#include "inside_control.metal.h"
#undef general_de
#undef general_primary_camera_attempt
#ifndef P2_REFERENCE
namespace mb3d_native {namespace continuation {
template<> inline uint entry_advance<8>(thread DistanceEntryFrame& e,thread CombinedFrame&,thread DistanceFrame&,thread OrbitReplayFrame&,thread OrbitPassFrame&,thread FormulaStepFrame&,thread WrapperFrame&,thread BodyFrame&,constant NativeFormulaProgram& p,thread ScriptResources& resources,thread NativeFormulaState& lane,thread NativeGeometryRecord& record){
 if(e.header.pc==0){e.header.pc=1;return 1;}
 e.child=inside_scripted_de(float3(e.point[0],e.point[1],e.point[2]),e.stop,p,resources,lane,record,bool(e.smooth));e.header.pc=99;return 99;
}
}}
#include "inside_regions.metal.h"
#endif
#undef NativeResources
kernel void abi(device uint* out [[buffer(0)]]){out[0]=sizeof(p2_inside::Input);out[1]=sizeof(p2_inside::Output);out[3]=p2_inside::SyntheticProbeAbi;
#ifdef P2_REFERENCE
out[2]=0;
#else
out[2]=sizeof(p2_inside::State);
#endif
}
kernel void original(constant p2_inside::Input* inputs [[buffer(0)]],device p2_inside::Output* outputs [[buffer(1)]],uint tid [[thread_position_in_grid]]){
 constant auto& in=inputs[tid].base;p2_inside::Output out{};out.base=initial_output(in);ScriptResources resources{&out.base.trace,&in,nullptr,nullptr};
 out.base.pixel=inside_control(in.scene,in.program,inputs[tid].policy,in.camera,in.x,in.y,resources,out.base.geometry,out.base.lane,out.activity,in.normal,out.base.normal_record,out.base.normal_result);outputs[tid]=out;
}
#ifndef P2_REFERENCE
kernel void advance(constant p2_inside::Input* inputs [[buffer(0)]],device p2_inside::Output* outputs [[buffer(1)]],device p2_inside::State* states [[buffer(2)]],constant uint& quantum [[buffer(3)]],uint tid [[thread_position_in_grid]]){
 auto s=states[tid];constant auto& in=inputs[tid].base;
 if(!s.initialized){s={};s.initialized=1;for(uint k=0;k<sizeof(s.frame);++k)reinterpret_cast<thread uchar*>(&s.frame)[k]=0xa5;
 auto out=initial_output(in);s.frame.lane=out.lane;s.frame.geometry=out.geometry;s.frame.normal_record=out.normal_record;s.frame.normal_result=out.normal_result;s.frame.pixel=out.pixel;s.frame.inside_record={};
 s.frame.inside.header={};s.frame.camera=in.camera;s.frame.normal_params=in.normal;s.frame.owner.x=in.x;s.frame.owner.y=in.y;}
 ScriptResources resources{&s.trace,&in,nullptr,nullptr};
 for(uint i=0;i<quantum&&!s.done;++i)s.done=inside_advance(s.frame,in.scene,in.program,inputs[tid].policy,resources)==99;
 states[tid]=s;if(s.done){p2_inside::Output out{};out.base.pixel=s.frame.pixel;out.base.lane=s.frame.lane;out.base.geometry=s.frame.geometry;out.base.normal_record=s.frame.normal_record;out.base.normal_result=s.frame.normal_result;out.base.trace=s.trace;out.activity=s.frame.inside_record;outputs[tid]=out;}
}
#endif
