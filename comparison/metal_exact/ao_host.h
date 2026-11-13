#pragma once
#include "ao_abi.h"
#include "scene_host.h"
#include "shadow_abi.h"

namespace mb3d::metal_exact {
struct PreparedExactAO {
    ExactAOScene device{};
    std::vector<ExactAOLevel> levels;
    std::vector<ExactU64> atan_terms;
};
PreparedExactAO PrepareExactAO(const PreparedExactScene& scene, unsigned threads = 1);
// Independent production image-space AO. No shared device AO math is used.
std::vector<ExactSurface> ReferenceAO(const PreparedExactScene& scene,
                                    const std::vector<ExactSurface>& surface,
                                    unsigned threads = 1);
// Native differential test of the device algorithm against ReferenceAO.
std::vector<ExactSurface> SoftwareAO(const PreparedExactAO& ao,
                                   const std::vector<ExactSurface>& surface);
} // namespace mb3d::metal_exact
