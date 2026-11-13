#include "division_abi.h"
// Full-value controls use native hardware division as their host oracle;
// digit controls additionally use independent host unsigned-128 arithmetic.
kernel void exact_division_seed_controls(device const DivisionSeedInput* input [[buffer(0)]],
                                        device DivisionSeedOutput* output [[buffer(1)]],
                                        constant uint& count [[buffer(2)]],
                                        uint i [[thread_position_in_grid]]) {
    if (i >= count) return;
    const DivisionSeedInput p = input[i];
    const F64 a = F64::from_bits(p.a), b = F64::from_bits(p.b);
    const DivisionDigit digit = division_digit(p.numerator, p.denominator, p.shift, p.hint);
    output[i] = {(a / b).bits, divide_restoring(a, b).bits,
        divide_with_reciprocal_hint(a, b, as_type<float>(p.reciprocal_bits)).bits,
        digit.quotient, digit.remainder, uint(digit.valid), 0};
}
