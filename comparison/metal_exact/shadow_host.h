#pragma once
#include "scene_host.h"
#include "shadow_abi.h"
#include "core/types.h"
#include <vector>

namespace mb3d::metal_exact {
ExactSurfaceParams PrepareSurface(const PreparedExactScene& scene);
ExactShadowParams PrepareShadow(const PreparedExactScene& scene,
                               bool enable_diagnostic_shadow = false,
                               unsigned max_steps = 65536, bool camera_inside = false,
                               bool native_soft = false);
// calcHSsoft's ZRSmul numerator for the soft light: 80 directional, 70 or
// 40 for a position light by iLightPos&6.
unsigned NativeSoftShadowNumerator(const PreparedExactScene& scene);
ExactSurface ExportSurface(const TsiLight5& surface);
TsiLight5 ImportSurface(const ExactSurface& surface);
// Production rendering supplies the independent packed-surface reference.
std::vector<ExactSurface> ReferenceSurface(const PreparedExactScene& scene,
                                         unsigned threads = 1);
// Production hard-shadow code, with each output compared before AO or color.
std::vector<ExactSurface> ReferenceShadow(const PreparedExactScene& scene,
                                        const std::vector<ExactSurface>& surface,
                                        bool enable_diagnostic_shadow = false);
} // namespace mb3d::metal_exact
