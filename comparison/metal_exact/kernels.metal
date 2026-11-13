#include <metal_stdlib>
using namespace metal;
#include "soft_double.h"
#ifndef MB3D_METAL_NATIVE_FLOAT
#include "formula_fixture.h"
#endif
#include "scene_math.h"
#ifndef MB3D_METAL_NATIVE_FLOAT
#include "extended_kernel.metal.h"
#endif
using namespace mb3d_metal_exact;
#ifndef MB3D_METAL_NATIVE_FLOAT
#include "log_fixture.h"
kernel void exact_native_log(device const LogInput* input [[buffer(0)]],
                            device ulong* output [[buffer(1)]],
                            constant uint& count [[buffer(2)]],uint i [[thread_position_in_grid]]) {
    if(i<count) output[i]=evaluate_log_input(input[i]).bits;
}
struct ArithmeticInput { ulong a,b; uint op,reserved; };
kernel void exact_arithmetic(device const ArithmeticInput* input [[buffer(0)]],device ulong* output [[buffer(1)]],constant uint& count [[buffer(2)]],uint i [[thread_position_in_grid]]) {
    if(i>=count)return;ArithmeticInput p=input[i];F64 a=D(p.a),b=D(p.b);ulong result=0;
    switch(p.op) {
      case 0:result=(a+b).bits;break;case 1:result=(a-b).bits;break;
      case 2:result=(a*b).bits;break;case 3:result=(a/b).bits;break;
      case 4:result=sqrt(a).bits;break;case 5:result=S(uint(p.a)).bits;break;
      case 6:result=a.to_f32_bits();break;case 7:result=a==b;break;case 8:result=a<b;break;case 9:result=a<=b;break;
    } output[i]=result;
}
struct SqrtSeedInput {ulong a,hint,high;};
struct SqrtSeedOutput {ulong bits,root,remainder;uint valid,reserved;};
kernel void exact_sqrt_seed_controls(device const SqrtSeedInput* input [[buffer(0)]],
                                    device SqrtSeedOutput* output [[buffer(1)]],
                                    constant uint& count [[buffer(2)]],uint i [[thread_position_in_grid]]) {
    if(i>=count)return;
    const SqrtSeedInput p=input[i];
    const SqrtPrefix prefix=sqrt_high_prefix(p.high,p.hint);
    output[i]={sqrt_with_hint(D(p.a),p.hint).bits,prefix.root,prefix.remainder,uint(prefix.valid),0};
}
kernel void exact_sqrt_wide_seed_controls(device const SqrtSeedInput* input [[buffer(0)]],
                                         device SqrtSeedOutput* output [[buffer(1)]],
                                         constant uint& count [[buffer(2)]],uint i [[thread_position_in_grid]]) {
    if(i>=count)return;
    const SqrtSeedInput p=input[i];
    const SqrtPrefix prefix=sqrt_wide_prefix(p.high,p.hint);
    output[i]={sqrt_with_wide_hint(D(p.a),p.hint).bits,prefix.root,prefix.remainder,uint(prefix.valid),0};
}
kernel void exact_box_fixture(device const mb3d_exact::ExactBoxInput* input [[buffer(0)]],device mb3d_exact::ExactBoxOutput* output [[buffer(1)]],constant uint& count [[buffer(2)]],uint i [[thread_position_in_grid]]) {
    if(i>=count)return;mb3d_exact::ExactBoxInput p=input[i];output[i]=mb3d_exact::exact_box<F64>(p);
}
#endif // exact-only arithmetic controls

kernel void exact_arithmetic_profile(device uint* output [[buffer(0)]]) {
#ifdef MB3D_METAL_NATIVE_FLOAT
    output[0]=2;output[1]=4;output[2]=32;
#else
    output[0]=1;output[1]=4;output[2]=80;
#endif
}

kernel void exact_primary(device ExactPixel* output [[buffer(0)]],device const ExactTrig* columns [[buffer(1)]],device const ExactTrig* rows [[buffer(2)]],constant ExactScene& scene [[buffer(3)]],constant uint* launch [[buffer(4)]],uint2 xy [[thread_position_in_grid]]) {
    // Allocation extents are separate from the untrusted scene record.
    uint y=xy.y+launch[0];if(xy.x>=launch[1]||y>=launch[2])return;
    ExactScene local=scene;
    if(local.width!=launch[1]||local.height!=launch[2]||!valid_scene(local)) {
        ExactPixel error={};error.fault=1;output[y*launch[1]+xy.x]=error;return;
    }
    ExactTrig tx=columns[xy.x],ty=rows[y];
    output[y*launch[1]+xy.x]=render_primary(local,tx,ty,xy.x,y);
}
kernel void exact_primary_geometry(device ExactPixel* output [[buffer(0)]],
                                   device const ExactTrig* columns [[buffer(1)]],
                                   device const ExactTrig* rows [[buffer(2)]],
                                   constant ExactScene& scene [[buffer(3)]],
                                   constant uint* launch [[buffer(4)]],
                                   uint2 xy [[thread_position_in_grid]]) {
    uint y=xy.y+launch[0];if(xy.x>=launch[1]||y>=launch[2])return;
    ExactScene local=scene;
    if(local.width!=launch[1]||local.height!=launch[2]||!valid_scene(local)) {
        ExactPixel error={};error.fault=1;output[y*launch[1]+xy.x]=error;return;
    }
    output[y*launch[1]+xy.x]=render_primary_geometry(local,columns[xy.x],rows[y],xy.x,y);
}
kernel void exact_normal_probes(device const ExactPixel* input [[buffer(0)]],
                                device ExactNormalSamples* output [[buffer(1)]],
                                constant ExactScene& scene [[buffer(2)]],
                                constant uint& count [[buffer(3)]],
                                uint i [[thread_position_in_grid]]) {
    // count comes from allocation dimensions, not the untrusted descriptor.
    // Planar probes keep each SIMD group on one axis/sign and avoid executing
    // both the plain-DE and Y-minus trap paths with mostly masked lanes.
    if(count==0||i>=count*6)return;
    const uint probe=i/count,pixel=i%count;
    ExactScene local=scene;
    ExactNormalProbeResult value={F64(0),F64(0)};
    if(valid_scene(local)&&local.width*local.height==count)
        value=normal_probe(input[pixel],local,probe);
    output[pixel].distance[probe]=value.distance.to_bits();
    if(probe==5) {
        output[pixel].orbit_trap=value.trap.to_bits();
        output[pixel].reserved=0;
    }
}
kernel void exact_normal_finish(device ExactPixel* output [[buffer(0)]],
                                device const ExactNormalSamples* samples [[buffer(1)]],
                                constant uint& count [[buffer(2)]],
                                constant ExactScene& scene [[buffer(3)]],
                                uint i [[thread_position_in_grid]]) {
    if(i>=count)return;
    ExactScene local=scene;
    if(!valid_scene(local)||local.width*local.height!=count) {
        ExactPixel error={};error.fault=1;output[i]=error;return;
    }
    output[i]=finish_normal(output[i],samples[i]);
}
kernel void exact_normal_abi(device uint* output [[buffer(0)]]) {
    output[0]=sizeof(ExactNormalSamples);
    output[1]=__builtin_offsetof(ExactNormalSamples,distance);
    output[2]=__builtin_offsetof(ExactNormalSamples,orbit_trap);
    output[3]=__builtin_offsetof(ExactNormalSamples,reserved);
    output[4]=6;output[5]=sizeof(ExactPixel);
}
kernel void exact_preview(device const ExactPixel* input [[buffer(0)]],device uint* output [[buffer(1)]],constant uint& count [[buffer(2)]],uint i [[thread_position_in_grid]]) {
    if(i>=count)return;ExactPixel p=input[i];uint r=12,g=18,b=30;
    if(p.fault) {r=255;g=0;b=255;}
    else if(p.rm_result) {r=uint(p.normal[0]+32767)*255/65534;g=uint(p.normal[1]+32767)*255/65534;b=uint(p.normal[2]+32767)*255/65534;}
    output[i]=r|(g<<8)|(b<<16)|0xff000000U;
}
#ifndef MB3D_METAL_NATIVE_FLOAT
kernel void exact_abi(device uint* output [[buffer(0)]],constant ExactScene& scene [[buffer(1)]]) {
    output[0]=sizeof(ExactScene);output[1]=sizeof(ExactTrig);output[2]=sizeof(ExactPixel);output[3]=sizeof(ArithmeticInput);
    output[4]=sizeof(mb3d_exact::ExactBoxInput);output[5]=sizeof(mb3d_exact::ExactBoxOutput);
    output[6]=scene.version;output[7]=scene.width;output[8]=scene.height;output[9]=scene.de_stop;
    output[10]=uint(scene.params[0]);output[11]=uint(scene.params[0]>>32);
    output[12]=scene.formula_kind;output[13]=scene.capability_id;
}

#endif

#include "shadow_kernels.metal.h"
#include "shadow_trace_kernels.metal.h"
#include "ao_kernels.metal.h"
#include "color_kernels.metal.h"

#ifndef MB3D_METAL_NATIVE_FLOAT
#include "single_kernel.metal.h"
kernel void exact_shading_abi(device uint* output [[buffer(0)]],constant ExactScene& scene [[buffer(1)]]) {
    output[0]=sizeof(ExactSurface);output[1]=sizeof(ExactSurfaceParams);output[2]=sizeof(ExactShadowParams);
    output[3]=sizeof(ExactAOScene);output[4]=sizeof(ExactAOLevel);output[5]=sizeof(ExactColorScene);
    output[6]=sizeof(ExactColorPixel);output[7]=sizeof(SingleInput);output[8]=sizeof(ExactShadowRay);output[9]=sizeof(ExactShadowTrace);
}

#include "division_kernel.metal.h"

#endif // exact-only primitive controls
