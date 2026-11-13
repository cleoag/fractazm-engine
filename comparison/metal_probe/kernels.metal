#include <metal_stdlib>
using namespace metal;
#include "probe_math.h"

kernel void abi_probe(device uint* out [[buffer(0)]],constant ProbeScene& scene [[buffer(1)]]) {
    out[0]=sizeof(ProbeScene);out[1]=sizeof(ProbePixel);out[2]=sizeof(ArithmeticInput);
    out[3]=sizeof(FormulaInput);out[4]=sizeof(FormulaOutput);
    out[5]=scene.version;out[6]=scene.width;out[7]=as_type<uint>(scene.epsilon);
}
kernel void arithmetic_probe(device const ArithmeticInput* input [[buffer(0)]],device float* out [[buffer(1)]],
                             constant uint& count [[buffer(2)]],uint i [[thread_position_in_grid]]) {
    if(i>=count) return;
    ArithmeticInput p=input[i];out[i]=arithmetic(p.operation,p.a,p.b,p.c);
}
kernel void box_fixture_probe(device const FormulaInput* input [[buffer(0)]],device FormulaOutput* out [[buffer(1)]],
                              constant uint& count [[buffer(2)]],uint i [[thread_position_in_grid]]) {
    if(i>=count) return;
    FormulaInput p=input[i];
    auto r=box_formula(V3<float>{p.x,p.y,p.z},p.params);
    out[i]={r.position.x,r.position.y,r.position.z,p.w,r.distance,r.vary,r.trap,0};
}
kernel void shape_points(device const float4* input [[buffer(0)]],device float4* out [[buffer(1)]],
                         constant uint& count [[buffer(2)]],uint i [[thread_position_in_grid]]) {
    if(i>=count) return;float4 p=input[i];V3<float> v={p.x,p.y,p.z};
    out[i]={shape_de<float,0>(v,6),shape_de<float,1>(v,6),shape_de<float,2>(v,6),0};
}
#define RENDER_KERNEL(Name, Formula) \
kernel void Name(device ProbePixel* out [[buffer(0)]],constant ProbeScene& input [[buffer(1)]],uint2 xy [[thread_position_in_grid]]) { \
    ProbeScene scene=input; \
    if(xy.x>=scene.width || xy.y>=scene.height) return; \
    if(scene.version!=1 || scene.max_steps>512 || scene.shadow_steps>4096 || scene.ao_samples>8 || scene.iterations>8) { \
        ProbePixel error={};error.fault=3;out[xy.y*scene.width+xy.x]=error;return; \
    } \
    out[xy.y*scene.width+xy.x]=render_pixel<float,Formula>(xy.x,xy.y,scene); \
}
RENDER_KERNEL(render_sphere,0)
RENDER_KERNEL(render_box,1)
RENDER_KERNEL(render_menger,2)
