#pragma once
#include "native_reflection_abi.h"
#include "scene_host.h"

namespace mb3d::metal_exact {
// Scene-only preparation for the main renderer's Native opaque reflection pass.
// Disabled scenes return an all-zero descriptor (max_bounces == 0). Unsupported
// active scenes throw before any GPU work. No CPU pixel/orbit work is performed.
NativeReflectionParams PrepareNativeReflection(const PreparedExactScene& scene,
                                              unsigned max_steps = 65536);
// Separate formula admission for the generic Native engine. Its GPU ABI and
// device work validation are independent from the original bulb extension.
NativeReflectionParams PrepareGeneralNativeReflection(const PreparedExactScene& scene,
                                              unsigned max_steps = 65536, bool camera_inside = false);
}
