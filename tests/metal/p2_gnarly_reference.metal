// Independent original-only raw control; no continuation headers.
#define MB3D_METAL_NATIVE_FLOAT 1
#include "src/render/native_metal/formula_math.metal.h"
#include "src/render/native_metal/generated/formula_10089.metal.h"
using namespace metal;
using namespace mb3d_native;
struct Input {float point[3],params[64];uint id,local_limit,limit,work;};
struct Output {float value[3],de,params[64];int counter;uint written;NativeFormulaFault fault;};
inline void publish(device Output& o,FormulaVec3 value,float de,thread float* data,int counter,bool written,NativeFormulaFault fault){
 o.value[0]=value.x;o.value[1]=value.y;o.value[2]=value.z;o.de=de;
 for(uint i=0;i<64;++i)o.params[i]=data[i];o.counter=counter;o.written=written;o.fault=fault;
}
kernel void p2_abi(device uint* out [[buffer(0)]]) {out[0]=0x50320003;out[1]=sizeof(Input);out[2]=sizeof(Output);out[3]=0;}
kernel void p2_original(device const Input* inputs [[buffer(0)]],device Output* outputs [[buffer(1)]],constant NativeFormulaSlot* slots [[buffer(2)]],uint tid [[thread_position_in_grid]]){
 const auto in=inputs[tid];float data[64];for(uint i=0;i<64;++i)data[i]=in.params[i];
 NativeFormulaFault fault{};fault.limit=in.limit;fault.work=in.work;
 NativeResources resources{};float de=0;int counter=0;bool written=false;
 FormulaParams params{data,&slots[tid],&resources,&fault,&counter,&written,in.local_limit};
 FormulaVec3 point(in.point[0],in.point[1],in.point[2]);
 FormulaVec3 result=formula_10089::execute_formula<10089>(point,params,de);
 publish(outputs[tid],result,de,data,counter,written,fault);
}
