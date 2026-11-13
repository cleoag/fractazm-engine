#include "render/backend.h"
#ifdef MB3D_ENABLE_METAL_RENDERER
#include "render/metal_renderer.h"
#include "render/native_metal/general_renderer.h"
#endif
#include <chrono>

namespace mb3d {

const char* RenderBackendName(RenderBackend backend) {
    switch (backend) {
    case RenderBackend::CPU: return "cpu";
    case RenderBackend::Metal: return "metal";
    case RenderBackend::Auto: return "auto";
    }
    return "invalid";
}

bool ParseRenderBackend(const std::string& text, RenderBackend& backend) {
    if (text == "cpu") backend = RenderBackend::CPU;
    else if (text == "metal") backend = RenderBackend::Metal;
    else if (text == "auto") backend = RenderBackend::Auto;
    else return false;
    return true;
}

static std::string UnsupportedOptions(const RenderOptions& o) {
    if (o.control) return "Metal does not implement render cancellation/work accounting";
    if (o.no_shadows)
        return "Metal does not implement the primary-shadow disable override";
    if (o.diag_only || !o.diag_points.empty() || !o.diag_dump_path.empty() ||
        !o.ray_setup_dump_path.empty() || !o.primary_trace_dump_path.empty() ||
        !o.parity_trace_dump_path.empty() || !o.parity_trace_start_stage.empty() ||
        !o.parity_trace_end_stage.empty() || o.parity_trace_schema_mode ||
        !o.parity_trace_detail.empty() || o.parity_trace_stage_state)
        return "Metal does not implement CPU diagnostic/trace output";
    return {};
}

BackendRenderResult RenderWithBackend(TMandHeader10& header, THeaderCustomAddon& addon,
                                     const RenderOptions& opts, const BackendOptions& backend) {
    BackendRenderResult out;
    out.backend.requested = backend.requested;
    auto cpu = [&]() {
        out.backend.effective = RenderBackend::CPU;
        out.render = Render(header, addon, opts);
    };
    if (backend.requested == RenderBackend::CPU) {
        cpu();
        return out;
    }
    // Reject invalid requests before admitting the CPU or the GPU job.
    if ((backend.requested != RenderBackend::Auto && backend.requested != RenderBackend::Metal) ||
        opts.precision != PrecisionMode::Native || opts.mc_samples != 0 || opts.native_metal_smoothing_policy>1) {
        out.render.status = RenderStatus::Unsupported;
        out.render.error = "Metal/auto requires native precision and an ordinary full render";
        out.render.profile.precision = opts.precision;
        return out;
    }
    const auto begin = std::chrono::steady_clock::now();
    auto finishSelection = [&]() {
        out.backend.selection_seconds = std::chrono::duration<double>(
            std::chrono::steady_clock::now() - begin).count();
    };
    auto fallback = [&](const char* code, const std::string& reason) {
        // The CPU renders whole frames only.
        if (!backend.allow_cpu_fallback || opts.lattice_stride != 1 || opts.lattice_x || opts.lattice_y || opts.admission_only) {
            out.backend.effective=RenderBackend::Metal;
            out.backend.fallback_code=code;out.backend.fallback_reason=reason;
            out.render.success=false;out.render.status=RenderStatus::Unsupported;
            out.render.error="Metal-only request: " + reason;
            finishSelection();return;
        }
        out.backend.fallback = true;
        out.backend.fallback_code = code;
        out.backend.fallback_reason = reason;
        finishSelection();
        // opts is Native and header/addon were not mutated by Metal admission.
        if (backend.cpu_fallback_control && !opts.control) {
            RenderOptions controlled = opts;
            controlled.control = backend.cpu_fallback_control;
            out.backend.effective = RenderBackend::CPU;
            out.render = Render(header, addon, controlled);
        } else cpu();
    };
    const auto unsupported = UnsupportedOptions(opts);
    if (!unsupported.empty()) {
        fallback("unsupported_options", unsupported);
        return out;
    }
#ifdef MB3D_ENABLE_METAL_RENDERER
    auto attempt = backend.prefer_general_metal
        ? native_metal::RenderGeneralNativeMetal(header,addon,opts,backend.metal_library,backend.metal_session)
        : RenderNativeMetal(header, addon, opts, backend.metal_library);
    const bool initialInside=attempt.status==NativeMetalStatus::NeedsGeneralInitialInside;
    const double primarySelectionSeconds=initialInside?std::chrono::duration<double>(std::chrono::steady_clock::now()-begin).count():0;
    // The original admitted capabilities retain their qualified images. Only
    // capability rejection (including validated initial-inside) tries the general engine;
    // device/library failures never become a retry or a CPU image. A completed
    // general frame beyond its FP32 range policy is rendered on CPU instead.
    if(!backend.prefer_general_metal && (attempt.status==NativeMetalStatus::Unsupported||initialInside))
        attempt=native_metal::RenderGeneralNativeMetal(header,addon,opts,backend.metal_library,backend.metal_session);
    // A post-primary capability handoff is GPU-to-GPU only. Unsupported
    // general shading cannot turn a dispatched frame into CPU fallback.
    if(initialInside && (attempt.status==NativeMetalStatus::Unsupported||attempt.status==NativeMetalStatus::Unavailable)) {
        attempt.status=NativeMetalStatus::Failed;
        attempt.result.error="Initial-inside frame requires general Metal: " + attempt.reason;
        attempt.result.success=false;
    }
    out.backend.device = std::move(attempt.device);
    out.backend.library = std::move(attempt.library);
    out.backend.reflections = attempt.reflections;
    out.backend.execution = attempt.execution;
    if(initialInside) {
        out.backend.selection_seconds=primarySelectionSeconds;
        const uint64_t pixels=uint64_t(opts.width_override>0?opts.width_override:header.Width)*
            uint64_t(opts.height_override>0?opts.height_override:header.Height);
        out.backend.execution.primary_records+=pixels;
        out.backend.execution.normal_records+=pixels;
        out.backend.execution.execution_plan="initial-inside GPU handoff; validated preliminary primary/normal records="+
            std::to_string(pixels)+"; "+out.backend.execution.execution_plan;
    }
    switch (attempt.status) {
    case NativeMetalStatus::Unsupported:
        fallback("unsupported_scene", attempt.reason);
        break;
    case NativeMetalStatus::Unavailable:
        fallback("metal_unavailable", attempt.reason);
        break;
    case NativeMetalStatus::Fp32Range:
        fallback("fp32_range", attempt.reason);
        break;
    case NativeMetalStatus::Completed:
    case NativeMetalStatus::Failed:
    case NativeMetalStatus::NeedsGeneralInitialInside:
        out.backend.effective = RenderBackend::Metal;
        out.render = std::move(attempt.result);
        if (!out.render.success && out.render.error.empty()) out.render.error = attempt.reason;
        // Successful GPU admission is part of its own preparation stage. This
        // field also includes the explicitly counted initial-inside GPU preflight.
        break;
    }
#else
    fallback("metal_not_built", "This executable was built without the native Metal renderer");
#endif
    return out;
}

} // namespace mb3d
