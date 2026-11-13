#define MB3D_METAL_NATIVE_FLOAT 1
#define MB3D_NATIVE_GENERAL_HELPERS_ONLY 1
// The strict AO translation unit needs its own generated formula linkage.
// Other Native kernels compile these same explicit specializations with a
// different FP contraction policy. Buffer layouts and public kernel names
// are unchanged; only private formula/type namespace symbols are isolated.
#define mb3d_native mb3d_native_ao_device
#include "native_ao_de.metal.h"
#undef mb3d_native
