#include <metal_stdlib>
#include "numerical_de_abi.h"
#include "soft_extended_log.h"
using namespace metal;
using namespace mb3d_metal_exact;

kernel void exact_numerical_de(device const NumericalDEInput* input [[buffer(0)]],
                              device NumericalDEOutput* output [[buffer(1)]],
                              constant uint& count [[buffer(2)]],
                              uint index [[thread_position_in_grid]]) {
    if (index >= count) return;
    const NumericalDEInput p = input[index];
    const auto result = extended_numerical_de(F64::from_bits(p.values[0]),
        F64::from_bits(p.values[1]), F64::from_bits(p.values[2]),
        F64::from_bits(p.values[3]), F64::from_bits(p.values[4]),
        F64::from_bits(p.values[5]));
    output[index] = {result.raw.bits, result.denominator.bits};
}
