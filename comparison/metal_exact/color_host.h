#pragma once
#include "color_abi.h"
#include "scene_host.h"
#include "shadow_abi.h"
namespace mb3d::metal_exact {
struct PreparedExactColor {
    ExactColorScene device{};
    std::vector<ExactTrig> columns, rows;
};
PreparedExactColor PrepareExactColor(const PreparedExactScene& scene);
std::vector<ExactColorPixel> ReferenceColor(const PreparedExactScene& scene,
                                          const std::vector<ExactSurface>& surface,
                                          unsigned threads=1);
}
