#define MB3D_METAL_NATIVE_FLOAT 1
#include "src/render/native_metal/general_geometry.metal.h"
#include "native_primary_probe_abi.h"
using namespace metal;using namespace mb3d_native;using namespace mb3d_native::continuation;using namespace p2_primary;
#ifndef P2_REFERENCE
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
kernel void advance(constant Input* inputs [[buffer(0)]],device Output* outputs [[buffer(1)]],device RealState* states [[buffer(2)]],constant uint& quantum [[buffer(3)]],uint tid [[thread_position_in_grid]]){
 auto s=states[tid];constant auto& in=inputs[tid];
 if(!s.initialized){s={};s.initialized=1;for(uint k=0;k<sizeof(s.frame);++k)reinterpret_cast<thread uchar*>(&s.frame)[k]=0xa5;
 auto out=initial_output(in);s.frame.lane=out.lane;s.frame.geometry=out.geometry;s.frame.normal_record=out.normal_record;s.frame.normal_result=out.normal_result;s.frame.pixel=out.pixel;
 s.frame.primary.header={};s.frame.primary.inside_word=as_type<int>(in.program.inside);s.frame.camera=in.camera;s.frame.normal_params=in.normal;s.frame.owner.x=in.x;s.frame.owner.y=in.y;}
 NativeResources resources{};
 for(uint i=0;i<quantum&&!s.done;++i)s.done=primary_advance(s.frame,in.scene,in.program,resources)==99;
 states[tid]=s;if(s.done){Output out{};out.pixel=s.frame.pixel;out.lane=s.frame.lane;out.geometry=s.frame.geometry;out.normal_record=s.frame.normal_record;out.normal_result=s.frame.normal_result;outputs[tid]=out;}
}
#endif
