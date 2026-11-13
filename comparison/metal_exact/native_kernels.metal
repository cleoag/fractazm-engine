// Separate FP32 render library. Shared storage ABI and dispatch sequence;
// software binary64/extended/single arithmetic is excluded at preprocessing.
#define MB3D_METAL_NATIVE_FLOAT 1
#include "kernels.metal"
#include "native_reflection_kernel.metal.h"
