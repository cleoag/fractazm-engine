#pragma once

#include "core/header_trafos.h"
#include "core/lightmap.h"
#include "core/parity_trace.h"
#include "core/reflection.h"
#include "core/render_control.h"
#include "core/numeric_precision.h"
#include <memory>
#include <string>

namespace mb3d {

// These records own their pixel allocations. small_background is a borrowed
// view of background, as in the original lighting setup; it must not free it.
struct LightingMapResources {
    TLightMap lights[6]{};
    TLightMap background{};
    TLightMap small_background{};
    TLightMap diffuse{};
    LightingMapResources() = default;
    ~LightingMapResources();
    LightingMapResources(const LightingMapResources&) = delete;
    LightingMapResources& operator=(const LightingMapResources&) = delete;
};

// One owning context per ordinary Render/PaintFromSiLight invocation. Packed
// Pascal records retain their ABI and borrow pointers from this stable owner.
// Output/surface buffers remain owned by RenderResult; lane scratch remains in
// the existing workers, preserving the numerical order and reset boundaries.
struct RenderContext {
    explicit RenderContext(const std::string& resourceRoot = {});
    RenderContext(const RenderContext&) = delete;
    RenderContext& operator=(const RenderContext&) = delete;

    FormulaConstBuffers formula_constants{};
    MapResources maps;
    LightingMapResources lighting_maps;
    TLValigned fallback_light_values{};
    TVolumeLightMap volume_light_map{};
    bool volume_light_map_calculated = false;
    ParityTraceConfig parity_trace_config;
    RenderControlHandle control;
    PrecisionMode precision = PrecisionMode::Compatible;
};

using RenderContextHandle = std::shared_ptr<RenderContext>;

// Low-level probes can use a caller-thread default context or bind an explicit
// one. A render always creates a fresh owner. TLS is only the callback binding:
// pool tasks and direct workers capture the owning handle from their submitter.
RenderContextHandle CaptureRenderContext();
RenderContext& CurrentRenderContext();

class ScopedRenderContext {
public:
    explicit ScopedRenderContext(RenderContextHandle context);
    ~ScopedRenderContext();
    ScopedRenderContext(const ScopedRenderContext&) = delete;
    ScopedRenderContext& operator=(const ScopedRenderContext&) = delete;
private:
    RenderContextHandle previous_;
    ScopedRenderControl control_binding_;
    ScopedNumericPrecision precision_binding_;
};

} // namespace mb3d
