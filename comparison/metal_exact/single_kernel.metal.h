#pragma once
#include "single_abi.h"
#include "soft_single.h"
kernel void exact_single(device const SingleInput* input [[buffer(0)]],
                          device SingleOutput* output [[buffer(1)]],
                          constant uint& count [[buffer(2)]],
                          uint index [[thread_position_in_grid]]) {
    if(index>=count)return;
    using namespace mb3d_metal_exact;
    const SingleInput p=input[index];F32 a=F32::from_bits(p.a),b=F32::from_bits(p.b),value;
    uint fault=p.reserved!=0;
    switch(p.op) {
        case 0:value=a+b;break;case 1:value=a-b;break;case 2:value=a*b;break;case 3:value=a/b;break;
        case 4:value=sqrt(a);break;case 5:value=-a;break;case 6:value=abs(a);break;
        case 7:value=min(a,b);break;case 8:value=max(a,b);break;
        case 9:value=F32(int(a<b));break;case 10:value=F32(int(a==b));break;case 11:value=F32(int(a<=b));break;
        default:value=F32::from_bits(0x7fc00000u);fault=1;break;
    }
    output[index]={value.bits,value.to_i32_rtz(),value.to_i32_rne(),fault,value.to_f64().bits,0};
}
