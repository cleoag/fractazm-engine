#pragma once
#include "formula_host.h"
#include "native_camera_host.h"
#include "general_geometry_abi.h"
#include "volume_light_abi.h"
#include "comparison/metal_exact/scene_host.h"
#include "comparison/metal_exact/shadow_abi.h"

namespace mb3d::native_metal {
// This descriptor reuses the data-only camera/packed-surface ABI. Formula kind
// 4 must only be dispatched to generic Native kernels, never exact distance().
constexpr unsigned kGeneralFormulaKind = mb3d_native::kGeneralFormulaKind;
constexpr unsigned kGeneralCapability = mb3d_native::kGeneralCapability;
struct PreparedGeneralScene {
    metal_exact::PreparedExactScene base;
    PreparedFormulaProgram formulas;
    PreparedNativeCamera camera;
    // Volumetric light map geometry; version 0 when the scene has none.
    mb3d_native::NativeVolumeLight volume{};
};
PreparedGeneralScene PrepareGeneralScene(const TMandHeader10&, const THeaderCustomAddon&,
    unsigned width, unsigned height, const std::string& resource_root,
    unsigned max_steps = 65536, Lattice lattice = {});
// Internal preview preparation: scalar/camera/host state is always fresh.
PreparedGeneralScene PrepareGeneralSceneWithMaps(const TMandHeader10&, const THeaderCustomAddon&,
    unsigned width, unsigned height, const std::string& resource_root,
    const PreparedFormulaProgram* snapshot, unsigned max_steps = 65536, Lattice lattice = {});
mb3d_native::NativeSurfaceParams PrepareGeneralSurface(const PreparedGeneralScene&);
} // namespace mb3d::native_metal
