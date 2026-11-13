// Build-time compiled hardware FP32 kernels. Runtime selects only offline
// function constants; no source compiler, bytecode engine or CPU pixel callback.
#define MB3D_METAL_NATIVE_FLOAT 1
#define MB3D_NATIVE_FORMULA_PROBE_KERNELS 1
#include "general_geometry.metal.h"
#include "generic_shadow.metal.h"
#include "generic_reflection_kernel.metal.h"
#include "generic_color.metal.h"

#include "camera_kernels.metal.h"
#include "camera_inside_shadow.metal.h"
#include "camera_multi_shadow.metal.h"
#include "camera_soft_shadow.metal.h"
#include "camera_inside_reflection_kernel.metal.h"
