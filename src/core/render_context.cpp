#include "core/render_context.h"
#include <filesystem>
#include <stdexcept>
#include <utility>

namespace mb3d {
namespace {
thread_local RenderContextHandle current_context;
}

LightingMapResources::~LightingMapResources() {
    for (auto& light : lights) FreeLightMap(&light);
    FreeLightMap(&background);
    FreeLightMap(&diffuse);
    // small_background.LMa aliases background.LMa; it owns no allocation.
}

RenderContext::RenderContext(const std::string& resourceRoot) {
    // An empty root is reserved for standalone low-level callers, which keep
    // their existing cwd-relative behavior. Public renders pass a captured cwd.
    if (!resourceRoot.empty())
        maps.resource_root = std::filesystem::absolute(resourceRoot).lexically_normal().string();
}

FormulaConstBuffers& GetFormulaConstBuffers() {
    return CurrentRenderContext().formula_constants;
}

RenderContextHandle CaptureRenderContext() {
    if (!current_context) current_context = std::make_shared<RenderContext>();
    return current_context;
}

RenderContext& CurrentRenderContext() {
    if (!current_context) current_context = std::make_shared<RenderContext>();
    return *current_context;
}

ScopedRenderContext::ScopedRenderContext(RenderContextHandle context)
    : previous_(current_context), control_binding_(context ? context->control.get() : nullptr),
      precision_binding_(context ? context->precision : PrecisionMode::Compatible) {
    if (!context) throw std::invalid_argument("Cannot bind a null render context");
    current_context = std::move(context);
}

ScopedRenderContext::~ScopedRenderContext() { current_context = std::move(previous_); }

} // namespace mb3d
