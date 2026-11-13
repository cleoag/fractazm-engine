#define MB3D_METAL_NATIVE_FLOAT 1
#include "src/render/native_metal/general_geometry.metal.h"
using namespace metal;using namespace mb3d_native;
#ifndef P2_REFERENCE
#include "src/render/native_metal/continuation_normal_step.metal.h"
using namespace mb3d_native::continuation;
#endif
struct Input {ExactScene scene;NativeFormulaProgram program;NativeNormalParams params;float center[3],depth,stop;};
struct Output {NativeNormalResult result;NativeFormulaState lane;NativeGeometryRecord geometry;NativeNormalRecord record;};
#ifndef P2_REFERENCE
struct SampleWitness {uint phase,direct,smooth;float point[3],stop;NativeFormulaState lane;NativeGeometryRecord geometry;NativeNormalRecord normal;GeneralNormalSample result;};
struct Witness {uint count,overflow;SampleWitness samples[14];};
struct Capture {Witness value;void record(uint phase,float3 point,float stop,bool direct,bool smooth,thread const NativeFormulaState& lane,thread const NativeGeometryRecord& geometry,thread const NativeNormalRecord& normal,GeneralNormalSample result) thread {
 const uint i=value.count++;if(i>=14){value.overflow=1;return;}value.samples[i]={phase,uint(direct),uint(smooth),{point.x,point.y,point.z},stop,lane,geometry,normal,result};
}};
#ifdef P2_OUTLINE_OBSERVER
struct ObservedCapture {Witness value;__attribute__((noinline)) void record(uint phase,float3 point,float stop,bool direct,bool smooth,thread const NativeFormulaState& lane,thread const NativeGeometryRecord& geometry,thread const NativeNormalRecord& normal,GeneralNormalSample result) thread {
 const uint i=value.count++;if(i>=14){value.overflow=1;return;}value.samples[i]={phase,uint(direct),uint(smooth),{point.x,point.y,point.z},stop,lane,geometry,normal,result};
}};
#else
using ObservedCapture=Capture;
#endif
#include "normal_outer_observer.metal.h"
struct State {NormalFrame normal;NormalSampleFrame sample;DistanceEntryFrame entry;CombinedFrame combined;DistanceFrame distance;OrbitReplayFrame replay;OrbitPassFrame orbit;FormulaStepFrame step;WrapperFrame wrapper;BodyFrame body;Output output;uint initialized,done;};
#endif
kernel void abi(device uint* out [[buffer(0)]]){out[0]=sizeof(Input);out[1]=sizeof(Output);
#ifdef P2_REFERENCE
out[2]=0;
#else
out[2]=sizeof(State);out[3]=sizeof(Witness);
#endif
}
kernel void original(constant Input* inputs [[buffer(0)]],device Output* outputs [[buffer(1)]],uint tid [[thread_position_in_grid]]){
 constant auto& in=inputs[tid];Output out{};NativeResources resources{};const float3 center(in.center[0],in.center[1],in.center[2]);out.lane=native_initial_state(center,in.stop,in.program);
 if(in.params.smoothing){general_normal_fault(out.geometry,out.record,1);out.result.fault=1;outputs[tid]=out;return;}
 auto params=in.params;params.smoothing=0;
 out.result=general_calculate_normals(center,in.depth,in.stop,in.scene,in.program,params,resources,out.lane,out.geometry,out.record);
 outputs[tid]=out;
}
#ifndef P2_REFERENCE
kernel void observe(constant Input* inputs [[buffer(0)]],device Output* outputs [[buffer(1)]],device Witness* witnesses [[buffer(4)]],uint tid [[thread_position_in_grid]]){
 constant auto& in=inputs[tid];Output out{};NativeResources resources{};const float3 center(in.center[0],in.center[1],in.center[2]);out.lane=native_initial_state(center,in.stop,in.program);auto params=in.params;params.smoothing=0;ObservedCapture capture{};
 out.result=normal_outer_observed(center,in.depth,in.stop,in.scene,in.program,params,resources,out.lane,out.geometry,out.record,capture);outputs[tid]=out;witnesses[tid]=capture.value;
}
kernel void advance(constant Input* inputs [[buffer(0)]],device Output* outputs [[buffer(1)]],device State* states [[buffer(2)]],constant uint& quantum [[buffer(3)]],device Witness* witnesses [[buffer(4)]],uint tid [[thread_position_in_grid]]){
 auto s=states[tid];const bool fresh=!s.initialized;constant auto& in=inputs[tid];
 if(!s.initialized){s={};s.initialized=1;
#define POISON(field) for(uint k=0;k<sizeof(s.field);++k)reinterpret_cast<thread uchar*>(&s.field)[k]=0xa5
 POISON(normal);POISON(sample);POISON(entry);POISON(combined);POISON(distance);POISON(replay);POISON(orbit);POISON(step);POISON(wrapper);POISON(body);
#undef POISON
 s.normal.header={};for(uint k=0;k<3;++k)s.normal.center[k]=in.center[k];s.normal.depth=in.depth;s.normal.stop=in.stop;s.output.lane=native_initial_state(float3(in.center[0],in.center[1],in.center[2]),in.stop,in.program);}
 NativeResources resources{};Capture capture{};if(!fresh)capture.value=witnesses[tid];
 for(uint i=0;i<quantum&&!s.done;++i){s.done=normal_advance(s.normal,s.sample,s.entry,s.combined,s.distance,s.replay,s.orbit,s.step,s.wrapper,s.body,in.scene,in.program,in.params,resources,s.output.lane,s.output.geometry,s.output.record,capture)==99;}
 witnesses[tid]=capture.value;
 states[tid]=s;if(s.done){s.output.result=s.normal.result;outputs[tid]=s.output;}
}
#endif
