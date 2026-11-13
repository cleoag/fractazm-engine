#pragma once
#include "extended_abi.h"
using namespace mb3d_metal_exact;
kernel void exact_extended(device const ExtendedInput* input [[buffer(0)]],device ExtendedOutput* output [[buffer(1)]],constant uint& count [[buffer(2)]],uint i [[thread_position_in_grid]]) {
    if(i>=count)return;
    ExtendedInput p=input[i]; E80 a=E80::from_bits(p.a_exp,p.a_sig), b=E80::from_bits(p.b_exp,p.b_sig), value;
    switch(p.op) {
        case 0:value=a+b;break;
        case 1:value=a-b;break;
        case 2:value=a*b;break;
        case 3:value=a/b;break;
        case 4:value=sqrt(a);break;
        // Ray-position refinement: extended multiply/add then original Double store.
        case 5:value=E80((a*b+E80(F64::from_bits(p.c_f64))).to_f64());break;
        // DE-stop update: extended multiply/add/multiply then original Single store.
        case 6:value=E80(F64::from_f32_bits(((a*b+E80(1))*E80(F64::from_bits(p.c_f64))).to_f32_bits()));break;
        default:value=ext_nan();break;
    }
    output[i]={value.significand,value.sign_exponent,value.to_f32_bits(),value.to_f64().bits,value.to_i32_rne(),value.to_i32_rtz()};
}
