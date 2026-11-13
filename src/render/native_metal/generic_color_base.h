#pragma once
#include "generic_color_abi.h"
#include "native_camera_host.h"
#include "comparison/metal_exact/color_host.h"
namespace mb3d::native_metal {
metal_exact::PreparedExactColor PrepareGeneralColorBase(const metal_exact::PreparedExactScene& scene,
    bool native_soft_completed=false, float* object_y=nullptr,
    mb3d_native::NativeLightGeometry* light_geometry=nullptr, Lattice lattice={});
}
