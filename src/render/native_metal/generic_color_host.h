#pragma once
#include "generic_color_abi.h"
#include "native_camera_host.h"
#include "comparison/metal_exact/color_host.h"
#include <vector>
namespace mb3d::native_metal {
struct PreparedGeneralColor {
    metal_exact::PreparedExactColor base;
    mb3d_native::NativeInteriorColorParams interior{};
    mb3d_native::NativeLightGeometry lights{}; // consumed only with kNativeLightGeometryFlag
    mb3d_native::NativeMaterialMaps maps{};    // consumed only with kNativeMaterialMapFlag
    std::vector<float> map_samples;            // raw loader samples of every bound map
    mb3d_native::NativeReflectionMaterial reflection{}; // camera-aware reflections only
};
PreparedGeneralColor PrepareGeneralColor(const metal_exact::PreparedExactScene& scene,
                                        bool hard_shadow_completed=true, Lattice lattice={});
} // namespace mb3d::native_metal
