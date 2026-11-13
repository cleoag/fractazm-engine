#pragma once
#include "scene_catalog.h"
#include "core/numeric_precision.h"
#include "core/types.h"
#include <memory>
#include <string>
#include <vector>

namespace mb3d::metal_exact {
struct SceneHostState;
struct PreparedExactScene {
    ExactScene device{};
    std::vector<ExactTrig> columns, rows;
    std::string source_path, source_sha256, scene_id, formula_name;
    unsigned original_shadow_bits{};
    PrecisionMode precision = PrecisionMode::Compatible;
    std::shared_ptr<SceneHostState> host;
};

// Known catalog inputs retain their pinned contracts. Other inputs are admitted
// only after production preparation matches an implemented numerical profile;
// unsupported formula branches and scene features fail closed.
PreparedExactScene PrepareExactScene(const std::string& path, unsigned width,
                                    unsigned height, unsigned max_steps = 65536,
                                    bool hard_shadows = false,
                                    PrecisionMode precision = PrecisionMode::Compatible);

// Main-renderer entry point. Copies caller-owned loaded parameters; all
// preparation and borrowed formula/map resources live in the returned owner.
// Runtime capabilities are checked from prepared values, without a file hash.
PreparedExactScene PrepareLoadedExactScene(const TMandHeader10& header,
                                         const THeaderCustomAddon& addon,
                                         unsigned width, unsigned height,
                                         const std::string& resource_root,
                                         unsigned max_steps = 65536,
                                         PrecisionMode precision = PrecisionMode::Native);

// Independent reference calls the production CPU camera, hybrid driver,
// RayMarch and RMCalculateNormals. It does not call the Metal/shared algorithm.
std::vector<ExactPixel> ReferencePrimary(const PreparedExactScene& scene,
                                       unsigned threads = 1);
} // namespace mb3d::metal_exact
