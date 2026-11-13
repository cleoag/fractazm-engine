#define MB3D_METAL_NATIVE_FLOAT 1
#include "src/render/native_metal/general_geometry.metal.h"
#include "src/render/native_metal/continuation_abi.h"
using namespace metal;using namespace mb3d_native;using namespace mb3d_native::continuation;
struct Trace {uint count;float points[7][3];uint direct[7],smooth[7];};
struct ScriptResources {thread Trace* trace;};
// Synthetic child: isolates outer ordering, coordinate preparation and packing.
// It never evaluates a fractal or provides renderer admission evidence.
inline GeneralNormalSample scripted_sample(float3 point,float stop,bool direct,bool smooth,constant NativeFormulaProgram& p,
 thread ScriptResources& resources,thread NativeFormulaState& lane,thread NativeGeometryRecord& g,thread NativeNormalRecord& n){
 if(g.fault||n.fault)return {};
 const uint i=resources.trace->count++;
 if(i>=7){general_normal_fault(g,n,8);return {};}
 for(uint k=0;k<3;++k){resources.trace->points[i][k]=point[k];lane.c[k]=point[k];}
 resources.trace->direct[i]=uint(direct);resources.trace->smooth[i]=uint(smooth);
 ++n.sample_calls;++g.normal_calls;if(direct)++n.direct_orbit_calls;else ++n.de_calls;
 lane.otrap=point.y;
 if(p.de_option>0&&i+1==uint(p.de_option))general_normal_fault(g,n,4);
 return {point.x,point.z};
}
namespace mb3d_native {namespace continuation {
template<bool OrderedFirst=false> inline uint normal_sample_advance(thread NormalSampleFrame& s,thread DistanceEntryFrame&,thread CombinedFrame&,thread DistanceFrame&,thread OrbitReplayFrame&,
 thread OrbitPassFrame&,thread FormulaStepFrame&,thread WrapperFrame&,thread BodyFrame&,constant NativeFormulaProgram& p,thread ScriptResources& resources,
 thread NativeFormulaState& lane,thread NativeGeometryRecord& g,thread NativeNormalRecord& n){
 if(s.header.pc==0){s.header.pc=1;return 1;}
 const auto out=scripted_sample(float3(s.point[0],s.point[1],s.point[2]),s.stop,bool(s.direct),bool(s.want_smooth),p,resources,lane,g,n);
 s.value=out.value;s.smooth=out.smooth;s.header.pc=99;return 99;
}
}}
#define NativeResources ScriptResources
#include "normal_regions.metal.h"
#include "normal_control.metal.h"
#undef NativeResources
struct Input {ExactScene scene;NativeFormulaProgram program;NativeNormalParams params;float center[3],depth,stop;};
struct Output {NativeNormalResult result;NativeFormulaState lane;NativeGeometryRecord geometry;NativeNormalRecord record;Trace trace;};
struct State {NormalFrame normal;NormalSampleFrame sample;Output output;uint initialized,done;};
kernel void abi(device uint* out [[buffer(0)]]){out[0]=sizeof(Input);out[1]=sizeof(Output);out[2]=sizeof(State);}
kernel void original(constant Input* inputs [[buffer(0)]],device Output* outputs [[buffer(1)]],uint tid [[thread_position_in_grid]]){
 constant auto& in=inputs[tid];Output out{};ScriptResources resources{&out.trace};
 out.result=normal_scripted_control(float3(in.center[0],in.center[1],in.center[2]),in.depth,in.stop,in.scene,in.program,in.params,resources,out.lane,out.geometry,out.record);
 outputs[tid]=out;
}
kernel void advance(constant Input* inputs [[buffer(0)]],device Output* outputs [[buffer(1)]],device State* states [[buffer(2)]],constant uint& quantum [[buffer(3)]],uint tid [[thread_position_in_grid]]){
 auto s=states[tid];constant auto& in=inputs[tid];
 if(!s.initialized){s={};s.initialized=1;for(uint k=0;k<sizeof(s.normal);++k)reinterpret_cast<thread uchar*>(&s.normal)[k]=0xa5;s.normal.header={};for(uint k=0;k<3;++k)s.normal.center[k]=in.center[k];s.normal.depth=in.depth;s.normal.stop=in.stop;}
 ScriptResources resources{&s.output.trace};DistanceEntryFrame e;CombinedFrame c;DistanceFrame d;OrbitReplayFrame r;OrbitPassFrame o;FormulaStepFrame step;WrapperFrame w;BodyFrame body;
 for(uint i=0;i<quantum&&!s.done;++i)s.done=normal_advance(s.normal,s.sample,e,c,d,r,o,step,w,body,in.scene,in.program,in.params,resources,s.output.lane,s.output.geometry,s.output.record)==99;
 states[tid]=s;if(s.done){s.output.result=s.normal.result;outputs[tid]=s.output;}
}
