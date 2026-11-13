#define MB3D_METAL_NATIVE_FLOAT 1
#include "src/render/native_metal/general_geometry.metal.h"
#include "native_primary_trace_abi.h"
using namespace metal;using namespace mb3d_native;using namespace mb3d_native::continuation;using namespace p2_primary;
#ifndef P2_REFERENCE
struct Capture {device p2_primary_trace::Trace* trace;void record(uint site,uint phase,uint mask,float3 ray,float3 start,float3 point,float3 active,float v0,float v1,float v2,float v3,float v4,float v5,float v6,float v7,float v8,float v9,float v10,float v11,float v12,float v13,float v14,float v15,uint i0,uint i1,uint i2,uint i3,uint i4,uint i5,uint i6,uint i7,ExactPixel pixel,thread const NativeFormulaState& lane,thread const NativeGeometryRecord& geometry,thread const NativeNormalRecord& normal_record,thread const NativeNormalResult& normal_result,NativeOrbitResult child) thread {const uint index=trace->count++;if(index>=p2_primary_trace::Capacity){trace->overflow=1;return;}
 p2_primary_trace::Snapshot s{};s.version=p2_primary_trace::Version;s.site=site;s.phase=phase;s.mask=mask;
 for(uint k=0;k<3;++k){s.vectors[k]=ray[k];s.vectors[k+3]=start[k];s.vectors[k+6]=point[k];s.vectors[k+9]=active[k];}
s.scalars[0]=v0;s.scalars[1]=v1;s.scalars[2]=v2;s.scalars[3]=v3;s.scalars[4]=v4;s.scalars[5]=v5;s.scalars[6]=v6;s.scalars[7]=v7;s.scalars[8]=v8;s.scalars[9]=v9;s.scalars[10]=v10;s.scalars[11]=v11;s.scalars[12]=v12;s.scalars[13]=v13;s.scalars[14]=v14;s.scalars[15]=v15;s.integers[0]=i0;s.integers[1]=i1;s.integers[2]=i2;s.integers[3]=i3;s.integers[4]=i4;s.integers[5]=i5;s.integers[6]=i6;s.integers[7]=i7;s.pixel=pixel;s.lane=lane;s.geometry=geometry;s.normal_record=normal_record;s.normal_result=normal_result;s.child=child;trace->snapshots[index]=s;}};
struct ObservedCapture {device p2_primary_trace::Trace* trace;__attribute__((noinline)) void record(uint site,uint phase,uint mask,float3 ray,float3 start,float3 point,float3 active,float v0,float v1,float v2,float v3,float v4,float v5,float v6,float v7,float v8,float v9,float v10,float v11,float v12,float v13,float v14,float v15,uint i0,uint i1,uint i2,uint i3,uint i4,uint i5,uint i6,uint i7,ExactPixel pixel,thread const NativeFormulaState& lane,thread const NativeGeometryRecord& geometry,thread const NativeNormalRecord& normal_record,thread const NativeNormalResult& normal_result,NativeOrbitResult child) thread {const uint index=trace->count++;if(index>=p2_primary_trace::Capacity){trace->overflow=1;return;}
 p2_primary_trace::Snapshot s{};s.version=p2_primary_trace::Version;s.site=site;s.phase=phase;s.mask=mask;
 for(uint k=0;k<3;++k){s.vectors[k]=ray[k];s.vectors[k+3]=start[k];s.vectors[k+6]=point[k];s.vectors[k+9]=active[k];}
s.scalars[0]=v0;s.scalars[1]=v1;s.scalars[2]=v2;s.scalars[3]=v3;s.scalars[4]=v4;s.scalars[5]=v5;s.scalars[6]=v6;s.scalars[7]=v7;s.scalars[8]=v8;s.scalars[9]=v9;s.scalars[10]=v10;s.scalars[11]=v11;s.scalars[12]=v12;s.scalars[13]=v13;s.scalars[14]=v14;s.scalars[15]=v15;s.integers[0]=i0;s.integers[1]=i1;s.integers[2]=i2;s.integers[3]=i3;s.integers[4]=i4;s.integers[5]=i5;s.integers[6]=i6;s.integers[7]=i7;s.pixel=pixel;s.lane=lane;s.geometry=geometry;s.normal_record=normal_record;s.normal_result=normal_result;s.child=child;trace->snapshots[index]=s;}};
#include "primary_observer.metal.h"
#include "src/render/native_metal/continuation_primary_step.metal.h"
#endif
inline Output initial_output(constant Input& in){Output out{};out.lane=native_initial_state(float3(in.camera.origin[0],in.camera.origin[1],in.camera.origin[2]),as_type<float>(in.scene.de_stop),in.program);out.lane.tmp=17;
 out.geometry.refine_calls=in.initial_refine;out.geometry.de_calls=in.initial_de;out.geometry.formula_work=in.initial_work;out.geometry.ray_steps=in.initial_ray;out.geometry.fault=in.initial_fault;return out;}
kernel void abi(device uint* out [[buffer(0)]]){out[0]=sizeof(Input);out[1]=sizeof(Output);out[3]=RealProbeAbi;
#ifdef P2_REFERENCE
out[2]=0;
#else
out[2]=sizeof(RealState);
#endif
}
kernel void original(constant Input* inputs [[buffer(0)]],device Output* outputs [[buffer(1)]],uint tid [[thread_position_in_grid]]){
 constant auto& in=inputs[tid];auto out=initial_output(in);NativeResources resources{};
 out.pixel=general_primary_camera_attempt(in.scene,in.program,in.camera,in.x,in.y,resources,out.geometry,out.lane,in.normal,out.normal_record,out.normal_result,as_type<int>(in.program.inside));outputs[tid]=out;
}
#ifndef P2_REFERENCE
kernel void trace_abi(device uint* out [[buffer(0)]]){out[0]=p2_primary_trace::Version;out[1]=sizeof(p2_primary_trace::Snapshot);out[2]=sizeof(p2_primary_trace::Trace);out[3]=p2_primary_trace::Capacity;}
kernel void observe(constant Input* inputs [[buffer(0)]],device Output* outputs [[buffer(1)]],device p2_primary_trace::Trace* traces [[buffer(2)]],uint tid [[thread_position_in_grid]]){
 constant auto& in=inputs[tid];auto out=initial_output(in);NativeResources resources{};ObservedCapture capture{&traces[tid]};
 out.pixel=primary_observed(in.scene,in.program,in.camera,in.x,in.y,resources,out.geometry,out.lane,in.normal,out.normal_record,out.normal_result,as_type<int>(in.program.inside),capture);outputs[tid]=out;
}

kernel void advance(constant Input* inputs [[buffer(0)]],device Output* outputs [[buffer(1)]],device RealState* states [[buffer(2)]],constant uint& quantum [[buffer(3)]],device p2_primary_trace::Trace* traces [[buffer(4)]],uint tid [[thread_position_in_grid]]){
 auto s=states[tid];constant auto& in=inputs[tid];
 if(!s.initialized){s={};s.initialized=1;for(uint k=0;k<sizeof(s.frame);++k)reinterpret_cast<thread uchar*>(&s.frame)[k]=0xa5;
 auto out=initial_output(in);s.frame.lane=out.lane;s.frame.geometry=out.geometry;s.frame.normal_record=out.normal_record;s.frame.normal_result=out.normal_result;s.frame.pixel=out.pixel;
 s.frame.primary.header={};s.frame.primary.inside_word=as_type<int>(in.program.inside);s.frame.camera=in.camera;s.frame.normal_params=in.normal;s.frame.owner.x=in.x;s.frame.owner.y=in.y;}
 NativeResources resources{};Capture capture{&traces[tid]};
 for(uint i=0;i<quantum&&!s.done;++i)s.done=primary_advance(s.frame,in.scene,in.program,resources,capture)==99;
 states[tid]=s;if(s.done){Output out{};out.pixel=s.frame.pixel;out.lane=s.frame.lane;out.geometry=s.frame.geometry;out.normal_record=s.frame.normal_record;out.normal_result=s.frame.normal_result;outputs[tid]=out;}
}
#endif
