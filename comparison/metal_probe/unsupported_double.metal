// Deliberately negative offline compiler diagnostic. Never loaded or executed.
#include <metal_stdlib>
using namespace metal;
kernel void unsupported_double(device double* values [[buffer(0)]],uint i [[thread_position_in_grid]]) {
    values[i]=values[i]*2.0;
}
