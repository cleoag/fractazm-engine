#define MB3D_METAL_NATIVE_FLOAT 1
#include "src/render/native_metal/general_geometry.metal.h"
#include "native_primary_probe_abi.h"
using namespace metal;using namespace mb3d_native;using namespace mb3d_native::continuation;using namespace p2_primary;
struct ScriptResources {thread Trace* trace;constant Input* input;constant NativeVolumeLight* volume;device const float* volume_map;};
inline NativeOrbitResult scripted_de(float3 point,float stop,constant NativeFormulaProgram& p,
 thread ScriptResources& resources,thread NativeFormulaState& lane,thread NativeGeometryRecord& record,
 bool smooth=false,int maximum=0,bool temporary=false){
 NativeOrbitResult out{};const uint i=resources.trace->count++;if(i>=Capacity){resources.trace->overflow=1;record.fault=8;return out;}
 thread auto& q=resources.trace->queries[i];for(uint k=0;k<3;++k)q.point[k]=point[k];q.stop=stop;q.maximum=maximum;q.smooth=uint(smooth);q.temporary=uint(temporary);q.before=lane;q.before_record=record;
 if(!record.fault){
 ++record.de_calls;record.formula_work+=1+i%3;record.fp32_range_exits+=uint(i%3==0);
 for(uint k=0;k<3;++k)lane.c[k]=point[k];lane.smooth=resources.input->smooth[i];lane.iteration=int(resources.input->iterations[i]);++lane.tmp;
 lane.rout=stop;lane.otrap=point.z;out.state=lane;out.distance=resources.input->distances[i];out.maximum=uint(p.max_iterations);out.completed=1;
 if(resources.input->fault_at==i+1)record.fault=32;
 }
 q.after=lane;q.after_record=record;q.result=out;return out;
}
inline NativeNormalResult scripted_normal(float3 point,float depth,float stop,constant ExactScene&,constant NativeFormulaProgram&,
 NativeNormalParams params,thread ScriptResources& resources,thread NativeFormulaState& lane,thread NativeGeometryRecord& geometry,thread NativeNormalRecord& record){
 NativeNormalResult result{};const uint i=resources.trace->count++;if(i>=Capacity){resources.trace->overflow=1;geometry.fault=8;return result;}
 thread auto& q=resources.trace->queries[i];for(uint k=0;k<3;++k)q.point[k]=point[k];q.stop=stop;q.maximum=-7;q.smooth=params.normals_on_de;q.temporary=params.inside;q.before=lane;q.before_record=geometry;
 ++record.sample_calls;++record.completed_evaluations;++geometry.normal_calls;lane.c[0]=point.x;lane.smooth=depth;lane.otrap=point.y;
 result.smooth=depth;result.orbit_trap=point.y;result.completed=1;for(uint k=0;k<3;++k){result.raw_normal[k]=point[k];result.packed[k]=int(k)+int(params.inside);}
 if(resources.input->fault_at==i+1)record.fault=geometry.fault=result.fault=32;
 q.after=lane;q.after_record=geometry;q.result.state=lane;return result;
}
inline NativeOrbitResult scripted_color(float3,float,constant ExactScene&,constant NativeFormulaProgram&,thread ScriptResources&,thread NativeFormulaState&,thread NativeGeometryRecord& g){g.fault=0x10000000u;return {};}
inline void scripted_volume(float3,float3,float,float,float,thread ScriptResources&,thread float&,thread NativeGeometryRecord& g){g.fault=0x10000000u;}
#define NativeResources ScriptResources
#define general_de scripted_de
#define general_color_on_it scripted_color
#define volume_substep scripted_volume
#define general_calculate_normals scripted_normal
#include "refinement_control.metal.h"
inline bool scripted_refine(thread float3& point,float3 ray,thread float& depth,float stop,constant ExactScene& scene,constant NativeFormulaProgram& p,thread ScriptResources& resources,thread NativeFormulaState& lane,thread NativeGeometryRecord& record,thread uint& iteration){return refinement_control(point,ray,depth,stop,scene,p,resources,lane,record,iteration,scene.refinement_steps);}
#define general_refine_iteration scripted_refine
#include "primary_control.metal.h"
#undef general_refine_iteration
#ifndef P2_REFERENCE
namespace mb3d_native {namespace continuation {
template<uint Context> inline uint entry_advance(thread DistanceEntryFrame& e,thread CombinedFrame&,thread DistanceFrame&,thread OrbitReplayFrame&,thread OrbitPassFrame&,
 thread FormulaStepFrame&,thread WrapperFrame&,thread BodyFrame&,constant NativeFormulaProgram& p,thread ScriptResources& resources,thread NativeFormulaState& lane,thread NativeGeometryRecord& record){
 if(e.header.pc==0){e.header.pc=1;return 1;}
 e.child=scripted_de(float3(e.point[0],e.point[1],e.point[2]),e.stop,p,resources,lane,record,bool(e.smooth),e.max_iterations,bool(e.temporary_maximum));e.header.pc=99;return 99;
}
inline uint normal_advance(thread NormalFrame& n,thread NormalSampleFrame&,thread DistanceEntryFrame&,thread CombinedFrame&,thread DistanceFrame&,thread OrbitReplayFrame&,thread OrbitPassFrame&,thread FormulaStepFrame&,thread WrapperFrame&,thread BodyFrame&,constant ExactScene& scene,constant NativeFormulaProgram& p,NativeNormalParams params,thread ScriptResources& resources,thread NativeFormulaState& lane,thread NativeGeometryRecord& geometry,thread NativeNormalRecord& record){
 if(n.header.pc==0){n.header.pc=1;return 1;}
 n.result=scripted_normal(float3(n.center[0],n.center[1],n.center[2]),n.depth,n.stop,scene,p,params,resources,lane,geometry,record);n.header.pc=99;return 99;
}
}}
#include "refinement_regions.metal.h"
#include "de_refinement_regions.metal.h"
#include "primary_regions.metal.h"
#endif
#undef general_calculate_normals
#undef volume_substep
#undef general_color_on_it
#undef general_de
#undef NativeResources
inline Output initial_output(constant Input& in){Output out{};out.lane.tmp=17;out.lane.xyz[3]=.375f;
 out.geometry.refine_calls=in.initial_refine;out.geometry.de_calls=in.initial_de;out.geometry.formula_work=in.initial_work;out.geometry.ray_steps=in.initial_ray;out.geometry.fault=in.initial_fault;return out;}
kernel void abi(device uint* out [[buffer(0)]]){out[0]=sizeof(Input);out[1]=sizeof(Output);out[3]=SyntheticProbeAbi;
#ifdef P2_REFERENCE
out[2]=0;
#else
out[2]=sizeof(State);
#endif
}
kernel void original(constant Input* inputs [[buffer(0)]],device Output* outputs [[buffer(1)]],uint tid [[thread_position_in_grid]]){
 constant auto& in=inputs[tid];auto out=initial_output(in);ScriptResources resources{&out.trace,&in,nullptr,nullptr};
 out.pixel=primary_control(in.scene,in.program,in.camera,in.x,in.y,resources,out.geometry,out.lane,in.normal,out.normal_record,out.normal_result,as_type<int>(in.program.inside));outputs[tid]=out;
}
#ifndef P2_REFERENCE
kernel void advance(constant Input* inputs [[buffer(0)]],device Output* outputs [[buffer(1)]],device State* states [[buffer(2)]],constant uint& quantum [[buffer(3)]],uint tid [[thread_position_in_grid]]){
 auto s=states[tid];constant auto& in=inputs[tid];
 if(!s.initialized){s={};s.initialized=1;for(uint k=0;k<sizeof(s.frame);++k)reinterpret_cast<thread uchar*>(&s.frame)[k]=0xa5;
 auto out=initial_output(in);s.frame.lane=out.lane;s.frame.geometry=out.geometry;s.frame.normal_record=out.normal_record;s.frame.normal_result=out.normal_result;s.frame.pixel=out.pixel;
 s.frame.primary.header={};s.frame.primary.inside_word=as_type<int>(in.program.inside);s.frame.camera=in.camera;s.frame.normal_params=in.normal;s.frame.owner.x=in.x;s.frame.owner.y=in.y;}
 ScriptResources resources{&s.trace,&in,nullptr,nullptr};
 for(uint i=0;i<quantum&&!s.done;++i)s.done=primary_advance(s.frame,in.scene,in.program,resources)==99;
 states[tid]=s;if(s.done){Output out{};out.pixel=s.frame.pixel;out.lane=s.frame.lane;out.geometry=s.frame.geometry;out.normal_record=s.frame.normal_record;out.normal_result=s.frame.normal_result;out.trace=s.trace;outputs[tid]=out;}
}
#endif
