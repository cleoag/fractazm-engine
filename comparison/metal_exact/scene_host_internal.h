#pragma once
// Host-only ownership shared by independent stage adapters. Never uploaded.
#include "scene_host.h"
#include "core/render_context.h"
#include "core/ray_marcher.h"
#include "io/m3p_parser.h"
namespace mb3d::metal_exact {
struct SceneHostState {
    RenderContextHandle context;
    M3PLoadResult file;
    TMCTparameter mct{};
};
}
