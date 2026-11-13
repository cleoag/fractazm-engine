#ifndef MB3D_CORE_RENDERER_H
#define MB3D_CORE_RENDERER_H

#include "core/parity_trace.h"
#include "core/ray_setup_dump.h"
#include "core/types.h"
#include "core/render_control.h"
#include "core/numeric_precision.h"
#include <string>
#include <vector>
#include <cstdint>
#include <thread>
#include <array>
#include <functional>
#include <string>

namespace mb3d {

struct DiagnosticPoint {
    int x = 0;
    int y = 0;
};

struct PixelDiagnosticRecord {
    int x = 0;
    int y = 0;
    bool in_bounds = false;
    bool hit = false;
    int rm_result = 0;
    int start_inside = 0;
    int cut_plane_hit = 0;
    double cafx = 0.0;
    double cafy = 0.0;
    std::array<double, 3> view_vec{{0.0, 0.0, 0.0}};
    std::array<double, 3> march_vec{{0.0, 0.0, 0.0}};
    std::array<double, 3> start_pos{{0.0, 0.0, 0.0}};
    std::array<double, 3> hit_pos{{0.0, 0.0, 0.0}};
    double act_zpos = 0.0;
    double z_stepped = 0.0;
    double final_de = 0.0;
    double ms_destop = 0.0;
    double de_stop = 0.0;
    float step_count = 0.0f;
    bool inside_rendering = false;
    uint16_t zpos = 0;
    uint16_t rough_zpos_fine = 0;
    uint16_t shadow = 0;
    uint16_t amb_shadow = 0;
    uint16_t sigradient = 0;
    uint16_t otrap = 0;
    int16_t normal_x = 0;
    int16_t normal_y = 0;
    int16_t normal_z = 0;
    std::array<uint8_t, 3> rgb{{0, 0, 0}};
    std::vector<std::string> primary_trace_lines;
};

// Wall-clock stage measurements owned by one render invocation. Primary includes
// formula evaluation, inline surface normals and dynamic fog; Normals measures
// only the optional image-space Z-buffer normal pass. No per-pixel clocks.
enum class RenderProfileStage {
    Preparation, Volume, Primary, Normals, Shadows, AO, Color, Reflections, DOF, Count
};
// Stage observer: the application API sets it around a render on the rendering
// thread to tell its caller which stage has begun. Every renderer calls
// NoteRenderStage where it starts timing a stage, with or without a profile.
// Observation only: nothing reads it to decide how a frame is rendered.
using RenderStageObserver = std::function<void(RenderProfileStage)>;
inline thread_local const RenderStageObserver* render_stage_observer = nullptr;
inline void NoteRenderStage(RenderProfileStage stage) {
    if (render_stage_observer && *render_stage_observer) (*render_stage_observer)(stage);
}
struct RenderStageObserverScope {
    const RenderStageObserver* previous;
    explicit RenderStageObserverScope(const RenderStageObserver* observer) : previous(render_stage_observer) { render_stage_observer = observer; }
    ~RenderStageObserverScope() { render_stage_observer = previous; }
    RenderStageObserverScope(const RenderStageObserverScope&) = delete;
    RenderStageObserverScope& operator=(const RenderStageObserverScope&) = delete;
};
struct RenderStageTiming {
    // not_run, interrupted (entered but not completed), or completed.
    const char* state = "not_run";
    double seconds = 0.0;
};
struct RenderProfile {
    bool enabled = false;
    int width = 0;
    int height = 0;
    int workers = 0;
    PrecisionMode precision = DefaultPrecisionMode();
    double total_seconds = 0.0;
    std::array<RenderStageTiming, static_cast<size_t>(RenderProfileStage::Count)> stages{};
};
const char* RenderProfileStageName(RenderProfileStage stage);

struct RenderOptions {
    PrecisionMode precision = DefaultPrecisionMode();
    int width_override = 0;   // 0 = use from .m3p
    int height_override = 0;
    int thread_count = 0;     // 0 = use hardware_concurrency
    int mc_samples = 0; // 0: regular; 1..65535: native path tracing
    int mc_max_steps = 100000; // fail on exhausted march budget
    bool no_shadows = false;
    bool no_ao = false;
    bool no_dof = false;
    bool no_reflections = false;
    // Base directory for this job's maps/backgrounds; empty captures the cwd.
    std::string resource_root;
    std::string output_path;
    std::string format = "png";
    std::string zbuffer_path;
    std::string zbuffer_format;
    std::string normalmap_path;
    std::vector<DiagnosticPoint> diag_points;
    std::string diag_dump_path;
    std::string ray_setup_dump_path;
    std::string primary_trace_dump_path;
    std::string parity_trace_dump_path;
    std::string parity_trace_start_stage;
    std::string parity_trace_end_stage;
    bool parity_trace_schema_mode = false;
    std::string parity_trace_level = "checkpoint";
    std::string parity_trace_detail;
    ParityTraceStageRangeStateHandle parity_trace_stage_state;
    bool diag_only = false;
    bool deterministic_mode = false;
    uint32_t deterministic_seed = 0x24563487u;
    bool verbose = false;
    bool profile_timing = false; // optional ordinary-render wall-clock timings
    // Optional single-job control; nullptr preserves uncounted legacy execution.
    RenderControlHandle control;
    // Native general Metal only:1 stable interpolation,0 diagnostic legacy.
    // No CLI flag and no effect on CPU/Compatible arithmetic.
    uint32_t native_metal_smoothing_policy = 1;
    // Native general Metal only: every pipeline calls the formula program
    // instead of inlining it at each call site, as the reflection and shadow
    // kernels always do. A cold pipeline then compiles several times faster.
    bool native_metal_outline_formulas = false;
    // Native general Metal only: render every lattice_stride-th pixel of the
    // width x height frame, starting at (lattice_x, lattice_y); the result has
    // that many pixels. 1: the whole frame. Other renderers refuse a lattice.
    int lattice_stride = 1, lattice_x = 0, lattice_y = 0;
    // Metal only: stop once an engine admits the frame (capabilities, options,
    // buffer sizes against the device's budget). Nothing is dispatched and the
    // result holds no image.
    bool admission_only = false;
};

struct RenderResult {
    std::vector<uint8_t> image;  // RGB
    std::vector<TsiLight5> silight;
    std::vector<float> mc_linear; // mean radiance before display
    std::vector<float> mc_variance; // unbiased luminance sample variance
    std::vector<uint32_t> mc_sample_counts;
    std::vector<PixelDiagnosticRecord> diagnostics;
    std::vector<RaySetupDumpRecord> ray_setup_records;
    std::vector<std::string> parity_trace_lines;
    int width = 0;
    int height = 0;
    int tiling_crop = 0;  // crop border pixels to remove (0 = no tiling)
    bool success = false;
    std::string error;
    RenderStatus status = RenderStatus::Failed;
    uint64_t work_units = 0;
    RenderProfile profile;
};

// Main render function: load parameters -> ray march -> color -> produce image
RenderResult Render(TMandHeader10& header, THeaderCustomAddon& addon, const RenderOptions& opts);
RenderResult PaintFromSiLight(TMandHeader10& header, THeaderCustomAddon& addon,
                              const std::vector<TsiLight5>& silight, const RenderOptions& opts);

} // namespace mb3d

#endif // MB3D_CORE_RENDERER_H
