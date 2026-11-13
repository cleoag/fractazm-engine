#define MB3D_METAL_NATIVE_FLOAT 1
#include "src/render/native_metal/general_geometry.metal.h"
#include "native_refinement_probe_abi.h"
using namespace metal;using namespace mb3d_native;using namespace mb3d_native::continuation;using namespace p2_refinement;
struct ScriptResources {thread Trace* trace;constant Input* input;};
// A deterministic child tape tests outer control/state only, not native DE.
inline NativeOrbitResult scripted_de(float3 point,float stop,constant NativeFormulaProgram&,
 thread ScriptResources& resources,thread NativeFormulaState& lane,thread NativeGeometryRecord& record,
 bool smooth,int maximum,bool temporary){
 NativeOrbitResult out{};if(record.fault)return out;
 const uint i=resources.trace->count++;if(i>=Capacity){resources.trace->overflow=1;record.fault=8;return out;}
 thread auto& q=resources.trace->queries[i];for(uint k=0;k<3;++k)q.point[k]=point[k];q.stop=stop;q.maximum=maximum;q.smooth=uint(smooth);q.temporary=uint(temporary);q.before=lane;q.before_record=record;
 ++record.de_calls;record.formula_work+=1+i%3;record.fp32_range_exits+=uint(i%3==0);
 for(uint k=0;k<3;++k)lane.c[k]=point[k];lane.smooth=resources.input->smooth[i];lane.iteration=int(resources.input->iterations[i]);++lane.tmp;
 out.state=lane;out.distance=point.x;out.maximum=uint(maximum);
 if(resources.input->fault_at==i+1)record.fault=32;
 q.after=lane;q.after_record=record;q.result=out;return out;
}
#define NativeResources ScriptResources
#include "refinement_control.metal.h"
#undef NativeResources
#ifndef P2_REFERENCE
namespace mb3d_native {namespace continuation {
template<uint Context> inline uint entry_advance(thread DistanceEntryFrame& e,thread CombinedFrame&,thread DistanceFrame&,thread OrbitReplayFrame&,thread OrbitPassFrame&,
 thread FormulaStepFrame&,thread WrapperFrame&,thread BodyFrame&,constant NativeFormulaProgram& p,thread ScriptResources& resources,thread NativeFormulaState& lane,thread NativeGeometryRecord& record){
 if(e.header.pc==0){e.header.pc=1;return 1;}
 e.child=scripted_de(float3(e.point[0],e.point[1],e.point[2]),e.stop,p,resources,lane,record,bool(e.smooth),e.max_iterations,bool(e.temporary_maximum));e.header.pc=99;return 99;
}
}}
#define NativeResources ScriptResources
#include "refinement_regions.metal.h"
#undef NativeResources
#endif
inline Output initial_output(constant Input& in){Output out{};for(uint k=0;k<3;++k)out.point[k]=in.point[k];out.depth=in.depth;out.iteration=in.iteration;
 out.lane.tmp=17;out.lane.xyz[3]=.375f;out.geometry.refine_calls=in.initial_refine;out.geometry.de_calls=in.initial_de;out.geometry.formula_work=in.initial_work;out.geometry.fault=in.initial_fault;return out;}
kernel void abi(device uint* out [[buffer(0)]]){out[0]=sizeof(Input);out[1]=sizeof(Output);
#ifdef P2_REFERENCE
out[2]=0;
#else
out[2]=sizeof(State);
#endif
}
kernel void original(constant Input* inputs [[buffer(0)]],device Output* outputs [[buffer(1)]],uint tid [[thread_position_in_grid]]){
 constant auto& in=inputs[tid];auto out=initial_output(in);float3 point(in.point[0],in.point[1],in.point[2]);const float3 ray(in.ray[0],in.ray[1],in.ray[2]);ScriptResources resources{&out.trace,&in};
 out.success=uint(refinement_scripted_control(point,ray,out.depth,in.stop,in.scene,in.program,resources,out.lane,out.geometry,out.iteration,in.steps));for(uint k=0;k<3;++k)out.point[k]=point[k];outputs[tid]=out;
}
#ifndef P2_REFERENCE
kernel void advance(constant Input* inputs [[buffer(0)]],device Output* outputs [[buffer(1)]],device State* states [[buffer(2)]],constant uint& quantum [[buffer(3)]],uint tid [[thread_position_in_grid]]){
 auto s=states[tid];constant auto& in=inputs[tid];
 if(!s.initialized){s={};s.initialized=1;s.output=initial_output(in);
 for(uint k=0;k<sizeof(s.refinement);++k)reinterpret_cast<thread uchar*>(&s.refinement)[k]=0xa5;
 for(uint k=0;k<sizeof(s.entry);++k)reinterpret_cast<thread uchar*>(&s.entry)[k]=0xa5;
 s.refinement.header={};for(uint k=0;k<3;++k){s.refinement.point[k]=in.point[k];s.refinement.ray[k]=in.ray[k];}
 s.refinement.depth=in.depth;s.refinement.stop=in.stop;s.refinement.refinement_steps=in.steps;s.refinement.iteration=in.iteration;}
 ScriptResources resources{&s.output.trace,&in};CombinedFrame c;DistanceFrame d;OrbitReplayFrame r;OrbitPassFrame o;FormulaStepFrame step;WrapperFrame w;BodyFrame body;
 for(uint i=0;i<quantum&&!s.done;++i)s.done=refinement_advance(s.refinement,s.entry,c,d,r,o,step,w,body,in.scene,in.program,resources,s.output.lane,s.output.geometry)==99;
 states[tid]=s;if(s.done){for(uint k=0;k<3;++k)s.output.point[k]=s.refinement.point[k];s.output.depth=s.refinement.depth;s.output.iteration=s.refinement.iteration;s.output.success=uint(refinement_success(s.refinement,s.output.geometry));outputs[tid]=s.output;}
}
#endif
