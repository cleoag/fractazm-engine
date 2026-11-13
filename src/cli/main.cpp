#include "core/types.h"
#include "core/math3d.h"
#include "core/header_trafos.h"
#include "core/renderer.h"
#include "render/backend.h"
#include "core/render_context.h"
#include "core/render_control.h"
#include "core/colorizer.h"
#include "core/shadow.h"
#include "core/ambient_occlusion.h"
#include "core/formula_registry.h"
#include "core/parity_trace.h"
#include "core/image_process.h"
#include "io/m3p_parser.h"
#include "io/image_writer.h"
#include "io/buffer_export.h"
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <cctype>
#include <chrono>
#include <thread>
#include <filesystem>
#include <exception>
#include <charconv>
#include <iomanip>
#include <locale>
#ifdef __APPLE__
#include <mach-o/dyld.h>
#endif

#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#include <crtdbg.h>
#endif

static const char* VERSION = "1.0.0";

#ifdef _WIN32
static LONG WINAPI console_exception_filter(EXCEPTION_POINTERS* info) {
    unsigned long code = info && info->ExceptionRecord ? info->ExceptionRecord->ExceptionCode : 0;
    std::fprintf(stderr, "Fatal Windows exception: 0x%08lX\n", code);
    std::fflush(stderr);
    return EXCEPTION_EXECUTE_HANDLER;
}

static void terminate_to_stderr() {
    std::fprintf(stderr, "Fatal error: unhandled C++ exception or terminate()\n");
    std::fflush(stderr);
    std::_Exit(3);
}

static void install_console_crash_handlers() {
    SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX | SEM_NOOPENFILEERRORBOX);
    _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDERR);
    _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_ERROR, _CRTDBG_FILE_STDERR);
    _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDERR);
    SetUnhandledExceptionFilter(console_exception_filter);
    _set_abort_behavior(0, _WRITE_ABORT_MSG | _CALL_REPORTFAULT);
    std::set_terminate(terminate_to_stderr);
}
#else
static void install_console_crash_handlers() {}
#endif

struct CLIProfile {
    std::string path;
    std::string input;
    std::string output;
    std::string error;
    bool publish = true;
    bool render_entered = false;
    mb3d::RenderStatus render_status = mb3d::RenderStatus::Failed;
    mb3d::RenderProfile render;
    mb3d::BackendSelection backend;
    int threads_requested = 0;
    mb3d::RenderStageTiming load, final_surface, save;
};

class CLIStageTimer {
public:
    CLIStageTimer(CLIProfile& profile, mb3d::RenderStageTiming& stage)
        : timing_(profile.path.empty() ? nullptr : &stage) {
        if (timing_) {
            timing_->state = "interrupted";
            start_ = std::chrono::steady_clock::now();
        }
    }
    ~CLIStageTimer() { stop(false); }
    void complete() { stop(true); }
private:
    void stop(bool completed) {
        if (!timing_) return;
        timing_->seconds = std::chrono::duration<double>(std::chrono::steady_clock::now() - start_).count();
        timing_->state = completed ? "completed" : "interrupted";
        timing_ = nullptr;
    }
    mb3d::RenderStageTiming* timing_;
    std::chrono::steady_clock::time_point start_;
};

static std::string json_string(const std::string& value) {
    std::ostringstream out;
    out << '"';
    static const char* digits = "0123456789abcdef";
    for (unsigned char c : value) {
        if (c == '"' || c == '\\') out << '\\' << c;
        else if (c < 0x20) out << "\\u00" << digits[c >> 4] << digits[c & 15];
        else out << c;
    }
    out << '"';
    return out.str();
}

static bool write_profile(const CLIProfile& profile, int exitCode, double totalSeconds) {
    std::ostringstream out;
    out.imbue(std::locale::classic());
    out << std::setprecision(17);
    const char* statuses[] = {"complete", "argument_error", "load_error", "render_error",
                              "write_error", "cancelled", "work_exhausted"};
    const char* status = exitCode >= 0 && exitCode <= 6 ? statuses[exitCode] : "render_error";
    std::vector<std::pair<const char*, mb3d::RenderStageTiming>> stages;
    stages.emplace_back("load", profile.load);
    for (size_t i = 0; i < profile.render.stages.size(); ++i)
        stages.emplace_back(mb3d::RenderProfileStageName(static_cast<mb3d::RenderProfileStage>(i)),
                            profile.render.stages[i]);
    stages.emplace_back("final_surface", profile.final_surface);
    stages.emplace_back("save", profile.save);
    const char* renderStatus = "not_run";
    if (profile.render_entered) {
        switch (profile.render_status) {
        case mb3d::RenderStatus::Completed: renderStatus = "completed"; break;
        case mb3d::RenderStatus::Cancelled: renderStatus = "cancelled"; break;
        case mb3d::RenderStatus::WorkExhausted: renderStatus = "work_exhausted"; break;
        case mb3d::RenderStatus::Unsupported: renderStatus = "unsupported"; break;
        default: renderStatus = "failed"; break;
        }
    }
    const bool metal = profile.backend.effective == mb3d::RenderBackend::Metal;
    out << "{\n  \"schema\": " << json_string(metal ? "mb3d.render-stage-profile.v1" : "mb3d.cpu-stage-profile.v1")
        << ",\n  \"schema_version\": 1,\n"
        << "  \"status\": " << json_string(status) << ",\n  \"exit_code\": " << exitCode
        << ",\n  \"render_status\": " << json_string(renderStatus)
        << ",\n  \"input\": " << json_string(profile.input)
        << ",\n  \"output\": " << json_string(profile.output)
        << ",\n  \"error\": " << json_string(profile.error)
        << ",\n  \"width\": " << profile.render.width
        << ",\n  \"height\": " << profile.render.height
        << ",\n  \"workers\": " << profile.render.workers
        << ",\n  \"threads_requested\": " << profile.threads_requested
        << ",\n  \"precision\": " << json_string(mb3d::PrecisionModeName(profile.render.precision))
        << ",\n  \"backend\": " << json_string(mb3d::RenderBackendName(profile.backend.effective))
        << ",\n  \"backend_requested\": " << json_string(mb3d::RenderBackendName(profile.backend.requested))
        << ",\n  \"backend_fallback\": " << (profile.backend.fallback ? "true" : "false")
        << ",\n  \"backend_fallback_code\": " << json_string(profile.backend.fallback_code)
        << ",\n  \"backend_fallback_reason\": " << json_string(profile.backend.fallback_reason)
        << ",\n  \"metal_device\": " << json_string(profile.backend.device)
        << ",\n  \"metal_library\": " << json_string(profile.backend.library)
        << ",\n  \"metal_reflections\": {\"enabled\":" << (profile.backend.reflections.enabled ? "true" : "false")
        << ",\"active_pixels\":" << profile.backend.reflections.active_pixels
        << ",\"rays\":" << profile.backend.reflections.rays
        << ",\"hits\":" << profile.backend.reflections.hits
        << ",\"misses\":" << profile.backend.reflections.misses
        << ",\"changed_pixels\":" << profile.backend.reflections.changed_pixels
        << ",\"faults\":" << profile.backend.reflections.faults
        << ",\"max_bounces\":" << profile.backend.reflections.max_bounces
        << ",\"ray_steps\":" << profile.backend.reflections.ray_steps
        << ",\"primary_skipped\":" << profile.backend.reflections.primary_skipped
        << ",\"ao_rays\":" << profile.backend.reflections.ao_rays
        << ",\"ao_steps\":" << profile.backend.reflections.ao_steps
        << ",\"shadow_rays\":" << profile.backend.reflections.shadow_rays
        << ",\"shadow_steps\":" << profile.backend.reflections.shadow_steps
        << ",\"max_command_seconds\":" << profile.backend.reflections.max_command_seconds << '}'
        << ",\n  \"metal_execution\": {\"validated\":" << (profile.backend.execution.validated ? "true" : "false")
        << ",\"engine\":" << json_string(profile.backend.execution.engine)
        << ",\"execution_plan_version\":" << profile.backend.execution.execution_plan_version
        << ",\"execution_plan\":" << json_string(profile.backend.execution.execution_plan)
        << ",\"library_profile\":[" << profile.backend.execution.profile_kind << ','
        << profile.backend.execution.storage_bytes << ',' << profile.backend.execution.arithmetic_bits << ']'
        << ",\"pipeline_archive\":[" << profile.backend.execution.pipeline_archive_hits << ','
        << profile.backend.execution.pipeline_archive_misses << ']'
        << ",\"primary_records\":" << profile.backend.execution.primary_records
        << ",\"normal_records\":" << profile.backend.execution.normal_records
        << ",\"normal_samples\":" << profile.backend.execution.normal_samples
        << ",\"normal_de_calls\":" << profile.backend.execution.normal_de_calls
        << ",\"normal_orbit_calls\":" << profile.backend.execution.normal_orbit_calls
        << ",\"normal_evaluations\":" << profile.backend.execution.normal_evaluations
        << ",\"primary_rough_pixels\":" << profile.backend.execution.primary_rough_pixels
        << ",\"surface_records\":" << profile.backend.execution.surface_records
        << ",\"shadow_records\":" << profile.backend.execution.shadow_records
        << ",\"distance_calls\":" << profile.backend.execution.distance_calls
        << ",\"formula_work\":" << profile.backend.execution.formula_work
        << ",\"smoothing_record_version\":" << profile.backend.execution.smoothing_record_version
        << ",\"smoothing_policy\":" << profile.backend.execution.smoothing_policy
        << ",\"smoothing_calls\":" << profile.backend.execution.smoothing_calls
        << ",\"smoothing_recovered\":" << profile.backend.execution.smoothing_recovered
        << ",\"smoothing_stagnant\":" << profile.backend.execution.smoothing_stagnant
        << ",\"smoothing_invalid\":" << profile.backend.execution.smoothing_invalid
        << ",\"shadow_rays\":" << profile.backend.execution.shadow_rays
        << ",\"shadow_hits\":" << profile.backend.execution.shadow_hits
        << ",\"shadow_misses\":" << profile.backend.execution.shadow_misses
        << ",\"shadow_steps\":" << profile.backend.execution.shadow_steps
        << ",\"shadow_changed\":" << profile.backend.execution.shadow_changed;
    if(profile.backend.execution.soft_shadow_records)
        out << ",\"soft_shadow_record_version\":" << profile.backend.execution.soft_shadow_record_version
            << ",\"soft_shadow_records\":" << profile.backend.execution.soft_shadow_records
            << ",\"soft_shadow_partial_pixels\":" << profile.backend.execution.soft_shadow_partial_pixels
            << ",\"soft_shadow_dark_pixels\":" << profile.backend.execution.soft_shadow_dark_pixels
            << ",\"soft_shadow_light_pixels\":" << profile.backend.execution.soft_shadow_light_pixels;
    out << ",\"color_records\":" << profile.backend.execution.color_records
        << ",\"reflection_records\":" << profile.backend.execution.reflection_records
        << ",\"dof_records\":" << profile.backend.execution.dof_records
        << ",\"dof_passes\":" << profile.backend.execution.dof_passes
        << ",\"dof_positive_radius_sources\":" << profile.backend.execution.dof_positive_radius_sources
        << ",\"dof_sort_work\":" << profile.backend.execution.dof_sort_work
        << ",\"dof_candidate_scan_checks\":" << profile.backend.execution.dof_candidate_scan_checks
        << ",\"dof_candidate_checks\":" << profile.backend.execution.dof_candidate_checks
        << ",\"dof_contributing_pairs\":" << profile.backend.execution.dof_contributing_pairs
        << ",\"dof_changed_pixels\":" << profile.backend.execution.dof_changed_pixels
        << ",\"dof_maximum_radius\":" << profile.backend.execution.dof_maximum_radius
        << ",\"dof_dispatches\":" << profile.backend.execution.dof_dispatches
        << ",\"znormal_records\":" << profile.backend.execution.znormal_records
        << ",\"znormal_foreground\":" << profile.backend.execution.znormal_foreground
        << ",\"znormal_changed\":" << profile.backend.execution.znormal_changed
        << ",\"znormal_neighbors\":" << profile.backend.execution.znormal_neighbors
        << ",\"znormal_dispatches\":" << profile.backend.execution.znormal_dispatches
        << ",\"camera_records\":" << profile.backend.execution.camera_records
        << ",\"camera_cut_starts\":" << profile.backend.execution.camera_cut_starts
        << ",\"camera_cut_misses\":" << profile.backend.execution.camera_cut_misses
        << ",\"camera_planar_rays\":" << profile.backend.execution.camera_planar_rays
        << ",\"camera_spherical_rays\":" << profile.backend.execution.camera_spherical_rays
        << ",\"camera_full_width\":" << profile.backend.execution.camera_full_width
        << ",\"camera_full_height\":" << profile.backend.execution.camera_full_height
        << ",\"inside_records\":" << profile.backend.execution.inside_records
        << ",\"inside_attempts\":" << profile.backend.execution.inside_attempts
        << ",\"inside_retries\":" << profile.backend.execution.inside_retries
        << ",\"inside_initial_hits\":" << profile.backend.execution.inside_initial_hits
        << ",\"inside_interior_surfaces\":" << profile.backend.execution.inside_interior_surfaces
        << ",\"inside_smooth_calls\":" << profile.backend.execution.inside_smooth_calls
        << ",\"reflection_inside_records\":" << profile.backend.execution.reflection_inside_records
        << ",\"reflection_inside_selected_pixels\":" << profile.backend.execution.reflection_inside_selected_pixels
        << ",\"reflection_inside_ao_rays\":" << profile.backend.execution.reflection_inside_ao_rays
        << ",\"reflection_inside_absorption_bounces\":" << profile.backend.execution.reflection_inside_absorption_bounces
        << ",\"reflection_inside_initial_primary\":" << profile.backend.execution.reflection_inside_initial_primary
        << ",\"reflection_inside_cut_ao_skips\":" << profile.backend.execution.reflection_inside_cut_ao_skips
        << ",\"reflection_inside_cut_shadow_limits\":" << profile.backend.execution.reflection_inside_cut_shadow_limits
        << ",\"camera_left\":" << profile.backend.execution.camera_left
        << ",\"camera_top\":" << profile.backend.execution.camera_top
        << ",\"inside_initial_policy\":" << profile.backend.execution.inside_initial_policy
        << ",\"inside_in_and_out\":" << profile.backend.execution.inside_in_and_out
        << ",\"color_abi_version\":" << profile.backend.execution.color_abi_version
        << ",\"surface_color_version\":" << profile.backend.execution.surface_color_version
        << ",\"surface_color_option\":" << profile.backend.execution.surface_color_option
        << ",\"orbit_color_records\":" << profile.backend.execution.orbit_color_records
        << ",\"numerical_gradient_policy\":" << profile.backend.execution.numerical_gradient_policy
        << ",\"primary_gradient_range_clamps\":" << profile.backend.execution.primary_gradient_range_clamps
        << ",\"reflection_gradient_range_clamps\":" << profile.backend.execution.reflection_gradient_range_clamps
        << ",\"fp32_range_exits\":" << profile.backend.execution.fp32_range_exits
        << ",\"fp32_range_exit_pixels\":" << profile.backend.execution.fp32_range_exit_pixels
        << ",\"ao_fp32_range_exit_pixels\":" << profile.backend.execution.ao_fp32_range_exit_pixels
        << ",\"fp32_range_gate_stage\":\"" << profile.backend.execution.fp32_range_gate_stage << "\""
        << ",\"fp32_range_gate_units\":" << profile.backend.execution.fp32_range_gate_units
        << ",\"fp32_range_gate_exceeded\":" << profile.backend.execution.fp32_range_gate_exceeded
        << ",\"fp32_range_gate_percent\":" << profile.backend.execution.fp32_range_gate_percent
        << ",\"difs_carried_reads\":" << profile.backend.execution.difs_carried_reads
        << ",\"difs_carried_pixels\":" << profile.backend.execution.difs_carried_pixels
        << ",\"difs_conditional_pixels\":" << (profile.backend.execution.difs_conditional_pixels ? "true" : "false")
        << ",\"difs_rerun_carried_reads\":" << profile.backend.execution.difs_rerun_carried_reads
        << ",\"difs_rerun_carried_pixels\":" << profile.backend.execution.difs_rerun_carried_pixels
        << ",\"ao_difs_carried_reads\":" << profile.backend.execution.ao_difs_carried_reads
        << ",\"ao_difs_carried_pixels\":" << profile.backend.execution.ao_difs_carried_pixels
        << ",\"ao_difs_conditional_pixels\":" << (profile.backend.execution.ao_difs_conditional_pixels ? "true" : "false")
        << ",\"ao_difs_rerun_carried_reads\":" << profile.backend.execution.ao_difs_rerun_carried_reads
        << ",\"ao_difs_rerun_carried_pixels\":" << profile.backend.execution.ao_difs_rerun_carried_pixels
        << ",\"primary_state_contract_version\":" << profile.backend.execution.primary_state_contract_version
        << ",\"primary_state_chain\":" << profile.backend.execution.primary_state_chain
        << ",\"primary_state_reason\":" << profile.backend.execution.primary_state_reason
        << ",\"primary_logical_tasks\":" << profile.backend.execution.primary_logical_tasks
        << ",\"primary_dispatches\":" << profile.backend.execution.primary_dispatches
        << ",\"primary_random_mode\":" << profile.backend.execution.primary_random_mode
        << ",\"primary_random_probe_dispatches\":" << profile.backend.execution.primary_random_probe_dispatches
        << ",\"primary_seed_scan_commands\":" << profile.backend.execution.primary_seed_scan_commands
        << ",\"ao_seed_generation_commands\":" << profile.backend.execution.ao_seed_generation_commands
        << ",\"primary_random_advances\":" << profile.backend.execution.primary_random_advances
        << ",\"primary_formula_reads\":" << profile.backend.execution.primary_formula_reads
        << ",\"primary_orbit_initialized\":" << profile.backend.execution.primary_orbit_initialized
        << ",\"primary_scene_inputs\":" << profile.backend.execution.primary_scene_inputs
        << ",\"primary_caller_controls\":" << profile.backend.execution.primary_caller_controls
        << ",\"primary_driver_may_write\":" << profile.backend.execution.primary_driver_may_write
        << ",\"primary_contained\":" << profile.backend.execution.primary_contained
        << ",\"primary_entry_equivalent\":" << profile.backend.execution.primary_entry_equivalent
        << ",\"soft_shadow_state_contract_version\":" << profile.backend.execution.soft_shadow_state_contract_version
        << ",\"soft_shadow_state_chain\":" << profile.backend.execution.soft_shadow_state_chain
        << ",\"soft_shadow_state_reason\":" << profile.backend.execution.soft_shadow_state_reason
        << ",\"shadow_logical_tasks\":" << profile.backend.execution.shadow_logical_tasks
        << ",\"shadow_dispatches\":" << profile.backend.execution.shadow_dispatches
        << ",\"shadow_speculative_pixels\":" << profile.backend.execution.shadow_speculative_pixels
        << ",\"shadow_recomputed_pixels\":" << profile.backend.execution.shadow_recomputed_pixels
        << ",\"shadow_speculation_fallbacks\":" << profile.backend.execution.shadow_speculation_fallbacks
        << ",\"soft_shadow_formula_reads\":" << profile.backend.execution.soft_shadow_formula_reads
        << ",\"soft_shadow_orbit_initialized\":" << profile.backend.execution.soft_shadow_orbit_initialized
        << ",\"soft_shadow_scene_inputs\":" << profile.backend.execution.soft_shadow_scene_inputs
        << ",\"soft_shadow_caller_controls\":" << profile.backend.execution.soft_shadow_caller_controls
        << ",\"soft_shadow_driver_may_write\":" << profile.backend.execution.soft_shadow_driver_may_write
        << ",\"volume_map_cells\":" << profile.backend.execution.volume_map_cells
        << ",\"volume_map_dispatches\":" << profile.backend.execution.volume_map_dispatches
        << ",\"volume_map_range_exit_cells\":" << profile.backend.execution.volume_map_range_exit_cells
        << ",\"color_flags\":" << profile.backend.execution.color_flags
        << ",\"ao_record_version\":" << profile.backend.execution.ao_record_version
        << ",\"ao_enabled\":" << (profile.backend.execution.ao_enabled?"true":"false")
        << ",\"ao_algorithm\":" << profile.backend.execution.ao_algorithm
        << ",\"ao_quality\":" << profile.backend.execution.ao_quality
        << ",\"ao_requested_repeats\":" << profile.backend.execution.ao_requested_repeats
        << ",\"ao_records\":" << profile.backend.execution.ao_records
        << ",\"ao_active\":" << profile.backend.execution.ao_active
        << ",\"ao_changed\":" << profile.backend.execution.ao_changed
        << ",\"ao_samples\":" << profile.backend.execution.ao_samples
        << ",\"ao_depth_reads\":" << profile.backend.execution.ao_depth_reads
        << ",\"ao_border_rejects\":" << profile.backend.execution.ao_border_rejects
        << ",\"ao_rng_advances\":" << profile.backend.execution.ao_rng_advances
        << ",\"ao_seed_planning_draws\":" << profile.backend.execution.ao_seed_planning_draws
        << ",\"ao_seed_shortcuts\":" << profile.backend.execution.ao_seed_shortcuts
        << ",\"ao_levels\":" << profile.backend.execution.ao_levels
        << ",\"ao_repeats\":" << profile.backend.execution.ao_repeats
        << ",\"ao_de_queries\":" << profile.backend.execution.ao_de_queries
        << ",\"ao_de_steps\":" << profile.backend.execution.ao_de_steps
        << ",\"ao_cut_skips\":" << profile.backend.execution.ao_cut_skips
        << ",\"ao_rays\":" << profile.backend.execution.ao_rays
        << ",\"ao_refinements\":" << profile.backend.execution.ao_refinements
        << ",\"ao_formula_work\":" << profile.backend.execution.ao_formula_work
        << ",\"ao_dispatches\":" << profile.backend.execution.ao_dispatches
        << ",\"ao_command_counter_version\":" << profile.backend.execution.ao_command_counter_version
        << ",\"ao_commands\":" << profile.backend.execution.ao_commands
        << ",\"ao_certificate_version\":" << profile.backend.execution.ao_certificate_version
        << ",\"ao_certificate_levels\":" << profile.backend.execution.ao_certificate_levels
        << ",\"ao_certificate_pixels\":" << profile.backend.execution.ao_certificate_pixels
        << ",\"ao_certificate_known\":" << profile.backend.execution.ao_certificate_known
        << ",\"ao_certificate_used\":" << profile.backend.execution.ao_certificate_used
        << ",\"ao_gpu_seconds\":" << profile.backend.execution.ao_gpu_seconds
        << ",\"ao_max_command_seconds\":" << profile.backend.execution.ao_max_command_seconds
        << ",\"ao_preparation_seconds\":" << profile.backend.execution.ao_preparation_seconds
        << ",\"faults\":" << profile.backend.execution.faults
        << ",\"nonfinite\":" << profile.backend.execution.nonfinite
        << ",\"work_exhausted\":" << profile.backend.execution.work_exhausted << '}'
        << ",\n  \"numeric_profile\": " << json_string(metal ? "native_metal_fp32" :
            profile.render.precision == mb3d::PrecisionMode::Native ? "native_cpu_fp64" : "compatible_cpu")
        << ",\n  \"backend_selection_seconds\": " << profile.backend.selection_seconds
        << ",\n  \"render_total_seconds\": " << profile.render.total_seconds
        << ",\n  \"command_total_seconds\": " << totalSeconds
        << ",\n  \"clock\": \"steady_clock wall seconds; coordinated stages, not summed worker CPU time\",\n"
        << "  \"boundaries\": {"
           "\"primary\":\"CPU: ray marching or 2D slice, formula evaluation, inline normals and dynamic fog; Metal: GPU geometry, normals and surface packing\","
           "\"normals\":\"optional Z-buffer normal recalculation only\","
           "\"reflections\":\"reflection/transparency including its volume-map preparation\","
           "\"save\":\"main image encode/write only; optional diagnostic exports excluded\","
           "\"render_total_seconds\":\"selected renderer lifecycle including preparation, execution, validation and readback; fallback selection reported separately\","
           "\"command_total_seconds\":\"CLI entry through return, excluding profile serialization\"},\n"
        << "  \"stages\": {";
    bool first = true;
    for (const auto& stage : stages) {
        if (!first) out << ',';
        first = false;
        out << "\n    " << json_string(stage.first) << ": {\"state\":" << json_string(stage.second.state)
            << ",\"seconds\":" << stage.second.seconds << '}';
    }
    out << "\n  },\n  \"stages_seconds\": {";
    first = true;
    for (const auto& stage : stages) {
        if (std::strcmp(stage.second.state, "not_run") == 0) continue;
        if (!first) out << ',';
        first = false;
        out << "\n    " << json_string(stage.first) << ':' << stage.second.seconds;
    }
    out << "\n  }\n}\n";
    const auto json = out.str();
    // Exclusive creation also protects evidence from simultaneous CLI jobs.
    std::FILE* file = std::fopen(profile.path.c_str(), "wbx");
    if (!file) return false;
    const bool written = std::fwrite(json.data(), 1, json.size(), file) == json.size();
    const int closeResult = std::fclose(file);
    return written && closeResult == 0;
}

static int main_impl(int argc, char* argv[], CLIProfile& profile);

static void print_usage() {
    std::fprintf(stderr,
        "Usage: mb3d_render <input.m3p> [options]\n"
        "\n"
        "Required:\n"
        "  <input.m3p>              Path to parameter file\n"
        "\n"
        "Output:\n"
        "  -o, --output <path>      Output image path (default: input.png)\n"
        "  -f, --format <png|bmp>   Format (default: png)\n"
        "  --zbuffer <path>         Save z-buffer\n"
        "  --zbuffer-format <bmp|png8|png16>\n"
        "                          Z-buffer format (default: bmp for .bmp, png8 otherwise)\n"
        "  --normalmap <path>       Save normal map\n"
        "  --silight5-dump <path>   Save raw siLight5 buffer dump\n"
        "  --ray-setup-dump <path>  Save full-frame pre-first-CalcDE setup dump\n"
        "  --shadow-replay-from-silight <path>\n"
        "                          Replay only the hard-shadow pass from an existing siLight5 dump\n"
        "  --ao-replay-from-silight <path>\n"
        "                          Replay only the ambient-occlusion pass from an existing siLight5 dump\n"
        "  --paint-replay-from-silight <path>\n"
        "                          Replay only the paint/color pass from an existing siLight5 dump\n"
        "  --diag-dump <path>       Save per-pixel diagnostic dump\n"
        "  --trace-primary-dump <path>\n"
        "                          Save per-pixel primary ray step trace\n"
        "  --parity-trace <path>    Save unified parity trace for one diagnostic pixel\n"
        "  --parity-trace-schema    Emit schema-compliant Level-0 (checkpoint) format\n"
        "                          for cross-language Pascal<->C++ comparison\n"
        "                          (see docs/PARITY_TRACE.md)\n"
        "  --parity-trace-start-stage <name>\n"
        "                          Log parity trace only from this stage onward\n"
        "  --parity-trace-end-stage <name>\n"
        "                          Stop parity trace after this stage\n"
        "  --diag-points <x,y;...>  Diagnostic sample pixels (default: 9-point grid)\n"
        "  --diag-only             Process only diagnostic sample pixels\n"
        "  --mc-samples N          Native Monte Carlo, 1..65535 samples/pixel\n"
        "  --mc-linear-dump <path> Save Monte Carlo linear RGB means as text\n"
        "  --deterministic          Test-mode deterministic render path for parity work\n"
        "  --precision <mode>       compatible (software x87) or native (CPU FP64 / Metal FP32)\n"
        "  --profile-json <path>    Write ordinary full-render stage timings to a new JSON file\n"
        "  --backend <cpu|metal|auto> Renderer (default: cpu); metal/auto use native\n"
        "                          Metal when supported, otherwise native CPU with a reason\n"
        "  --metal-library <path>  Offline native library (default: beside executable)\n"
        "\n"
        "Parameter overrides:\n"
        "  -w, --width <int>        Image width\n"
        "  -h, --height <int>       Image height\n"
        "  -t, --threads <int>      Thread count (default: hardware_concurrency)\n"
        "  --max-iter <int>         Max iterations\n"
        "  --work-budget <uint64>   Shared ordinary render work limit (0 = unlimited)\n"
        "\n"
        "Post-processing overrides:\n"
        "  --lattice <s,x,y>        Native general Metal: render every s-th pixel (2..4) of the\n"
        "                          frame, starting at (x,y); the image has that many pixels\n"
        "  --no-shadows             Disable hard shadows\n"
        "  --no-hard-shadow-pass   Clear scene hard-shadow request (same-backend control)\n"
        "  --no-zbuffer-normals    Disable automatic Z-buffer normals\n"
        "  --no-volume             Disable scene volume lighting\n"
        "  --no-ao                  Disable ambient occlusion\n"
        "  --no-dof                 Disable depth of field\n"
        "  --no-reflections         Disable reflections\n"
        "  --no-postprocess         Disable all post-processing\n"
        "\n"
        "Info:\n"
        "  -v, --verbose            Verbose progress output\n"
        "  --info                   Show .m3p parameters without rendering\n"
        "  --version                Program version\n"
        "  --help                   Help\n"
        "\n"
        "Exit codes:\n"
        "  0 - success\n"
        "  1 - argument error\n"
        "  2 - .m3p read error\n"
        "  3 - render error\n"
        "  4 - file write error\n"
        "  5 - render cancelled\n"
        "  6 - render work limit exhausted\n"
    );
}

static int render_exit_code(mb3d::RenderStatus status) {
    if (status == mb3d::RenderStatus::Cancelled) return 5;
    if (status == mb3d::RenderStatus::WorkExhausted) return 6;
    return 3;
}

static bool write_silight5_dump(const std::string& path,
                                const std::vector<TsiLight5>& silight,
                                int width, int height) {
    std::ofstream out(path, std::ios::binary);
    if (!out) return false;

    const char magic[8] = {'S','I','L','5','D','M','P','1'};
    const uint32_t version = 1;
    const uint32_t w = static_cast<uint32_t>(width);
    const uint32_t h = static_cast<uint32_t>(height);
    const uint32_t recordSize = static_cast<uint32_t>(sizeof(TsiLight5));
    const uint32_t count = static_cast<uint32_t>(silight.size());

    out.write(magic, sizeof(magic));
    out.write(reinterpret_cast<const char*>(&version), sizeof(version));
    out.write(reinterpret_cast<const char*>(&w), sizeof(w));
    out.write(reinterpret_cast<const char*>(&h), sizeof(h));
    out.write(reinterpret_cast<const char*>(&recordSize), sizeof(recordSize));
    out.write(reinterpret_cast<const char*>(&count), sizeof(count));
    if (!silight.empty()) {
        out.write(reinterpret_cast<const char*>(silight.data()),
                  static_cast<std::streamsize>(silight.size() * sizeof(TsiLight5)));
    }
    return out.good();
}

static bool write_ray_setup_dump(const std::string& path,
                                 const std::vector<mb3d::RaySetupDumpRecord>& records,
                                 int width, int height) {
    std::ofstream out(path, std::ios::binary);
    if (!out) return false;

    const char magic[8] = {'R','A','Y','S','E','T','P','1'};
    const uint32_t version = 1;
    const uint32_t w = static_cast<uint32_t>(width);
    const uint32_t h = static_cast<uint32_t>(height);
    const uint32_t recordSize = static_cast<uint32_t>(sizeof(mb3d::RaySetupDumpRecord));
    const uint32_t count = static_cast<uint32_t>(records.size());

    out.write(magic, sizeof(magic));
    out.write(reinterpret_cast<const char*>(&version), sizeof(version));
    out.write(reinterpret_cast<const char*>(&w), sizeof(w));
    out.write(reinterpret_cast<const char*>(&h), sizeof(h));
    out.write(reinterpret_cast<const char*>(&recordSize), sizeof(recordSize));
    out.write(reinterpret_cast<const char*>(&count), sizeof(count));
    if (!records.empty()) {
        out.write(reinterpret_cast<const char*>(records.data()),
                  static_cast<std::streamsize>(records.size() * sizeof(mb3d::RaySetupDumpRecord)));
    }
    return out.good();
}

static bool read_silight5_dump(const std::string& path,
                               std::vector<TsiLight5>& silight,
                               int& width,
                               int& height,
                               std::string& error) {
    std::ifstream in(path, std::ios::binary);
    if (!in) {
        error = "failed to open siLight5 dump";
        return false;
    }

    char magic[8];
    uint32_t version = 0, w = 0, h = 0, recordSize = 0, count = 0;
    in.read(magic, sizeof(magic));
    in.read(reinterpret_cast<char*>(&version), sizeof(version));
    in.read(reinterpret_cast<char*>(&w), sizeof(w));
    in.read(reinterpret_cast<char*>(&h), sizeof(h));
    in.read(reinterpret_cast<char*>(&recordSize), sizeof(recordSize));
    in.read(reinterpret_cast<char*>(&count), sizeof(count));
    if (!in) {
        error = "failed to read siLight5 dump header";
        return false;
    }
    if (std::memcmp(magic, "SIL5DMP1", 8) != 0) {
        error = "bad siLight5 dump magic";
        return false;
    }
    if (version != 1) {
        error = "unsupported siLight5 dump version";
        return false;
    }
    if (recordSize != sizeof(TsiLight5)) {
        error = "unexpected siLight5 record size";
        return false;
    }
    if (count != w * h) {
        error = "siLight5 dump count mismatch";
        return false;
    }
    silight.resize(count);
    if (count > 0) {
        in.read(reinterpret_cast<char*>(silight.data()),
                static_cast<std::streamsize>(count * sizeof(TsiLight5)));
        if (!in) {
            error = "failed to read siLight5 records";
            return false;
        }
    }
    width = static_cast<int>(w);
    height = static_cast<int>(h);
    return true;
}

static bool write_optional_buffers_from_silight(const mb3d::RenderOptions& opts,
                                                const TMandHeader10& header,
                                                const std::vector<TsiLight5>& silight,
                                                int width,
                                                int height,
                                                int tilingCrop,
                                                std::string& error) {
    if (!opts.zbuffer_path.empty()) {
        bool zbufOk = false;
        std::string zbufFormat = opts.zbuffer_format;
        if (zbufFormat.empty()) {
            std::string zbExt;
            auto zbDot = opts.zbuffer_path.rfind('.');
            if (zbDot != std::string::npos)
                zbExt = opts.zbuffer_path.substr(zbDot);
            for (auto& c : zbExt) c = static_cast<char>(std::tolower(c));
            zbufFormat = (zbExt == ".bmp") ? "bmp" : "png8";
        }

        if (zbufFormat == "bmp") {
            zbufOk = mb3d::WriteZBufferBMP_8bit_Final(opts.zbuffer_path, silight.data(),
                                                      width, height,
                                                      header.bImageScale,
                                                      tilingCrop);
        } else if (zbufFormat == "png8") {
            zbufOk = mb3d::WriteZBufferPNG_8bit_Final(opts.zbuffer_path, silight.data(),
                                                      width, height,
                                                      header.bImageScale,
                                                      tilingCrop);
        } else if (zbufFormat == "png16") {
            double zcorr = 0.0;
            double zcmul = 0.0;
            double zzst = 0.0;
            mb3d::CalcPPZvals(const_cast<TMandHeader10&>(header), zcorr, zcmul, zzst);
            zbufOk = mb3d::WriteZBufferPNG_16bit_Final(opts.zbuffer_path, silight.data(),
                                                       width, height,
                                                       header.bImageScale,
                                                       tilingCrop,
                                                       zcmul, zcorr);
        } else {
            error = "unsupported z-buffer format '" + zbufFormat + "'";
            return false;
        }

        if (!zbufOk) {
            error = "failed to write z-buffer: " + opts.zbuffer_path;
            return false;
        }
    }

    if (!opts.normalmap_path.empty()) {
        std::vector<int16_t> normals(static_cast<size_t>(width) * static_cast<size_t>(height) * 3);
        for (int j = 0; j < width * height; ++j) {
            normals[static_cast<size_t>(j) * 3 + 0] = silight[static_cast<size_t>(j)].NormalX;
            normals[static_cast<size_t>(j) * 3 + 1] = silight[static_cast<size_t>(j)].NormalY;
            normals[static_cast<size_t>(j) * 3 + 2] = silight[static_cast<size_t>(j)].NormalZ;
        }
        if (!mb3d::WriteNormalMapFinal(opts.normalmap_path, normals.data(),
                                       width, height,
                                       header.bImageScale,
                                       tilingCrop)) {
            error = "failed to write normal map: " + opts.normalmap_path;
            return false;
        }
    }

    return true;
}

static void print_version() {
    std::printf("mb3d_render %s\n", VERSION);
}

// Derive default output path from input path
static std::string default_output_path(const std::string& inputPath, const std::string& format) {
    std::string base = inputPath;
    // Strip extension if present
    auto dotPos = base.rfind('.');
    auto slashPos = base.rfind('/');
    if (dotPos != std::string::npos && (slashPos == std::string::npos || dotPos > slashPos)) {
        base = base.substr(0, dotPos);
    }
    return base + "." + format;
}

static void print_info(const std::string& filepath, const TMandHeader10& header,
                       const THeaderCustomAddon& addon) {
    std::printf("File: %s\n", filepath.c_str());
    std::printf("  MandId: %d\n", header.MandId);
    std::printf("  Size: %d x %d\n", header.Width, header.Height);
    std::printf("  Iterations: %d\n", header.Iterations);
    std::printf("  Zoom: %g\n", header.dZoom);
    std::printf("  Bailout: %g\n", header.RStop);

    // Camera type
    const char* camType = "perspective";
    if (header.bPlanarOptic == 1) camType = "planar";
    else if (header.bPlanarOptic == 2) camType = "spherical";
    std::printf("  Camera: %s\n", camType);

    // Formula info: first active formula
    if (addon.Formulas[0].iItCount > 0) {
        int fnr = addon.Formulas[0].iFnr;
        const char* fname = "Unknown";
        if (fnr >= 0 && fnr <= 9) {
            static const char* names[] = {
                "Integer Power", "Real Power", "Quaternion", "Tricorn",
                "Amazing Box", "Bulbox", "Folding Int Pow",
                "Test 7", "Test 8", "Aexion C"
            };
            fname = names[fnr];
        } else if (fnr >= 20) {
            fname = "External";
        }
        if (fnr == 0) {
            int power = 2;
            if (addon.Formulas[0].dOptionValue[0] >= 2 && addon.Formulas[0].dOptionValue[0] <= 8)
                power = static_cast<int>(addon.Formulas[0].dOptionValue[0]);
            std::printf("  Formula: %s (power %d)\n", fname, power);
        } else {
            std::printf("  Formula: %s\n", fname);
        }
    }

    // Julia
    std::printf("  Julia: %s\n", header.bIsJulia ? "yes" : "no");

    // Hard shadows
    if (header.bCalculateHardShadow & 0xFC) {
        std::string lights;
        for (int i = 0; i < 6; i++) {
            if (header.bCalculateHardShadow & (4 << i)) {
                if (!lights.empty()) lights += ",";
                lights += std::to_string(i + 1);
            }
        }
        std::printf("  Hard shadows: yes (lights %s)\n", lights.c_str());
    } else {
        std::printf("  Hard shadows: no\n");
    }

    // Ambient occlusion
    if (header.bCalcAmbShadowAutomatic & 1) {
        int aoType = (header.bCalcAmbShadowAutomatic >> 2) & 3;
        int aoQuality = (header.bCalcAmbShadowAutomatic >> 4) & 3;
        const char* aoDesc = "standard";
        int rays = 0;
        if (aoType == 3) {
            aoDesc = "DE-based";
            int rayCounts[] = {3, 6, 16, 32};
            rays = rayCounts[aoQuality];
        } else if (aoType == 1) {
            aoDesc = "HiQ threshold";
        } else if (aoType == 2) {
            aoDesc = "HiQ random";
        }
        if (rays > 0)
            std::printf("  Ambient occlusion: %s (%d rays)\n", aoDesc, rays);
        else
            std::printf("  Ambient occlusion: %s\n", aoDesc);
    } else {
        std::printf("  Ambient occlusion: no\n");
    }

    // DOF
    if (header.bCalcDOFtype) {
        std::printf("  DOF: yes (aperture %g)\n", static_cast<double>(header.sDOFaperture));
    } else {
        std::printf("  DOF: no\n");
    }

    // Reflections
    if (header.bCalcSRautomatic & 1) {
        std::printf("  Reflections: yes (count %d, amount %g)\n",
                    static_cast<int>(header.SRreflectioncount),
                    static_cast<double>(header.SRamount));
    } else {
        std::printf("  Reflections: no\n");
    }
}

static double elapsed_sec(std::chrono::steady_clock::time_point start) {
    auto now = std::chrono::steady_clock::now();
    return std::chrono::duration<double>(now - start).count();
}

static std::vector<mb3d::DiagnosticPoint> default_diag_points(int width, int height) {
    std::vector<mb3d::DiagnosticPoint> points;
    if (width <= 0 || height <= 0) return points;
    points.push_back({0, 0});
    points.push_back({width / 2, 0});
    points.push_back({width - 1, 0});
    points.push_back({0, height / 2});
    points.push_back({width / 2, height / 2});
    points.push_back({width - 1, height / 2});
    points.push_back({0, height - 1});
    points.push_back({width / 2, height - 1});
    points.push_back({width - 1, height - 1});
    return points;
}

static bool parse_diag_points(const std::string& text, std::vector<mb3d::DiagnosticPoint>& out) {
    out.clear();
    size_t start = 0;
    while (start < text.size()) {
        size_t end = text.find(';', start);
        std::string token = text.substr(start, end == std::string::npos ? std::string::npos : end - start);
        if (!token.empty()) {
            size_t comma = token.find(',');
            if (comma == std::string::npos) return false;
            std::string xs = token.substr(0, comma);
            std::string ys = token.substr(comma + 1);
            char* xend = nullptr;
            char* yend = nullptr;
            long x = std::strtol(xs.c_str(), &xend, 10);
            long y = std::strtol(ys.c_str(), &yend, 10);
            if (!xend || *xend != '\0' || !yend || *yend != '\0') return false;
            out.push_back({static_cast<int>(x), static_cast<int>(y)});
        }
        if (end == std::string::npos) break;
        start = end + 1;
    }
    return !out.empty();
}

static bool write_diag_dump(const std::string& path, const std::string& scenePath,
                            const TMandHeader10& header,
                            const std::vector<mb3d::PixelDiagnosticRecord>& diagnostics) {
    std::ofstream out(path);
    if (!out) return false;

    out << "scene=" << scenePath << "\n";
    out << "width=" << header.Width << "\n";
    out << "height=" << header.Height << "\n";
    out << "diag_points=" << diagnostics.size() << "\n";
    for (size_t i = 0; i < diagnostics.size(); i++) {
        const auto& record = diagnostics[i];
        out << "\n[pixel " << i << "]\n";
        out << "x=" << record.x << "\n";
        out << "y=" << record.y << "\n";
        out << "in_bounds=" << (record.in_bounds ? 1 : 0) << "\n";
        if (!record.in_bounds) continue;
        out << "hit=" << (record.hit ? 1 : 0) << "\n";
        out << "rm_result=" << record.rm_result << "\n";
        out << "start_inside=" << record.start_inside << "\n";
        out << "cut_plane_hit=" << record.cut_plane_hit << "\n";
        out << "cafx=" << record.cafx << "\n";
        out << "cafy=" << record.cafy << "\n";
        out << "view_vec=" << record.view_vec[0] << "," << record.view_vec[1] << "," << record.view_vec[2] << "\n";
        out << "march_vec=" << record.march_vec[0] << "," << record.march_vec[1] << "," << record.march_vec[2] << "\n";
        out << "start_pos=" << record.start_pos[0] << "," << record.start_pos[1] << "," << record.start_pos[2] << "\n";
        out << "hit_pos=" << record.hit_pos[0] << "," << record.hit_pos[1] << "," << record.hit_pos[2] << "\n";
        out << "act_zpos=" << record.act_zpos << "\n";
        out << "z_stepped=" << record.z_stepped << "\n";
        out << "final_de=" << record.final_de << "\n";
        out << "ms_destop=" << record.ms_destop << "\n";
        out << "de_stop=" << record.de_stop << "\n";
        out << "step_count=" << record.step_count << "\n";
        out << "inside_rendering=" << (record.inside_rendering ? 1 : 0) << "\n";
        out << "normal=" << record.normal_x << "," << record.normal_y << "," << record.normal_z << "\n";
        out << "zpos=" << record.zpos << "\n";
        out << "rough_zpos_fine=" << record.rough_zpos_fine << "\n";
        out << "shadow=" << record.shadow << "\n";
        out << "amb_shadow=" << record.amb_shadow << "\n";
        out << "sigradient=" << record.sigradient << "\n";
        out << "otrap=" << record.otrap << "\n";
        out << "rgb=" << static_cast<int>(record.rgb[0]) << ","
            << static_cast<int>(record.rgb[1]) << ","
            << static_cast<int>(record.rgb[2]) << "\n";
    }
    return true;
}

static bool write_primary_trace_dump(const std::string& path,
                                     const std::vector<mb3d::PixelDiagnosticRecord>& diagnostics) {
    std::ofstream out(path);
    if (!out) return false;
    bool wroteAny = false;
    for (size_t i = 0; i < diagnostics.size(); i++) {
        const auto& record = diagnostics[i];
        if (record.primary_trace_lines.empty()) continue;
        if (wroteAny) out << "\n";
        out << "[pixel " << i << "]\n";
        for (const auto& line : record.primary_trace_lines) {
            out << line << "\n";
        }
        wroteAny = true;
    }
    return wroteAny;
}

static bool write_parity_trace_dump(const std::string& path,
                                    const std::vector<std::string>& pre_lines,
                                    const std::vector<std::string>& render_lines) {
    std::ofstream out(path);
    if (!out) return false;
    for (const auto& line : pre_lines) {
        out << line << "\n";
    }
    for (const auto& line : render_lines) {
        out << line << "\n";
    }
    return out.good();
}

static void append_trace_line(std::vector<std::string>& lines, const char* stage,
                              const std::string& payload) {
    if (!mb3d::ShouldLogParityStage(stage)) {
        return;
    }
    std::string line = std::string("stage=") + stage + " x=-1 y=-1";
    if (!payload.empty()) {
        line += " ";
        line += payload;
    }
    lines.push_back(std::move(line));
}

static void append_formula_schema_lines(std::vector<std::string>& lines,
                                        const THeaderCustomAddon& addon) {
    for (int slot = 0; slot < 6; ++slot) {
        const auto& f = addon.Formulas[slot];
        std::ostringstream payload;
        payload.setf(std::ios::fixed, std::ios::floatfield);
        payload << std::setprecision(12)
                << "slot=" << slot
                << " iFnr=" << f.iFnr
                << " iItCount=" << f.iItCount
                << " iOptionCount=" << f.iOptionCount
                << " bOptions1=" << static_cast<int>(addon.bOptions1)
                << " bOptions2=" << static_cast<int>(addon.bOptions2)
                << " bOptions3=" << static_cast<int>(addon.bOptions3)
                << " bHybOpt1=" << static_cast<int>(addon.bHybOpt1)
                << " bHybOpt2=" << static_cast<int>(addon.bHybOpt2);
        for (int i = 0; i < 16; ++i) {
            payload << " type" << i << "=" << static_cast<int>(f.byOptionType[i])
                    << " opt" << i << "=" << f.dOptionValue[i];
        }
        append_trace_line(lines, "formula_schema", payload.str());
    }
}

static void append_load_trace_lines(std::vector<std::string>& lines,
                                    const std::string& scenePath,
                                    const mb3d::M3PLoadResult& loadResult,
                                    const std::vector<mb3d::DiagnosticPoint>& diag_points) {
    (void)scenePath;
    (void)diag_points;
    int diffMapNr = static_cast<int>(loadResult.header.Light.bColorMap |
                    (static_cast<int>(loadResult.header.Light.Lights[1].AdditionalByteEx) << 8));
    int hasAnyLightMap = 0;
    for (const auto& light : loadResult.header.Light.Lights) {
        if (light.LightMapNr != 0) {
            hasAnyLightMap = 1;
            break;
        }
    }
    {
        std::ostringstream payload;
        payload.setf(std::ios::fixed, std::ios::floatfield);
        payload << std::setprecision(12)
                << "scene=" << std::filesystem::path(scenePath).filename().string()
                << " raw_file_size=" << loadResult.raw_file_size
                << " is_compressed=" << static_cast<int>(loadResult.is_compressed)
                << " decompressed_size=" << loadResult.loaded_data_size
                << " MandId=" << loadResult.source_mand_id
                << " raw_header_size=" << loadResult.raw_header_size
                << " pixel_data_size=" << loadResult.pixel_data_size
                << " addon_offset=" << loadResult.addon_offset
                << " width=" << loadResult.header.Width
                << " height=" << loadResult.header.Height
                << " iterations=" << loadResult.header.Iterations
                << " rstop=" << loadResult.header.RStop
                << " bCalc3D=" << static_cast<int>(loadResult.header.bCalc3D)
                << " bPlanarOptic=" << static_cast<int>(loadResult.header.bPlanarOptic)
                << " dFOVy=" << loadResult.header.dFOVy
                << " dZstart=" << loadResult.header.dZstart
                << " dZend=" << loadResult.header.dZend
                << " dZmid=" << loadResult.header.dZmid;
        append_trace_line(lines, "load_header", payload.str());
    }

    append_formula_schema_lines(lines, loadResult.addon);

    {
        std::ostringstream payload;
        payload.setf(std::ios::fixed, std::ios::floatfield);
        payload << std::setprecision(12)
                << "has_addon=1"
                << " formula_count=" << static_cast<int>(loadResult.addon.iFCount)
                << " hybrid_count=" << static_cast<int>((loadResult.addon.bHybOpt1 & 7) + 1)
                << " de_option=" << static_cast<int>(loadResult.addon.bOptions1 & 3)
                << " de_option2=" << static_cast<int>(loadResult.addon.bOptions2 & 7)
                << " color_option=" << static_cast<int>(loadResult.header.byColor2Option)
                << " color_on_it=" << static_cast<int>(loadResult.header.bColorOnIt)
                << " bInAndOutside=" << static_cast<int>((loadResult.addon.bOptions2 & 4) != 0)
                << " bMCTFirstStepRandom=" << static_cast<int>((loadResult.header.iOptions & 1) != 0)
                << " bColorMap=" << static_cast<int>(loadResult.header.Light.bColorMap != 0)
                << " bLightMap=" << hasAnyLightMap
                << " bDiffMap=" << static_cast<int>(diffMapNr != 0)
                << " bBackgroundImage=" << static_cast<int>(loadResult.header.Light.BGbmp[0] != 0);
        append_trace_line(lines, "addon_ready", payload.str());
    }
}

static int RunCLI(int argc, char* argv[], CLIProfile& profile) {
    install_console_crash_handlers();
    try {
        return main_impl(argc, argv, profile);
    } catch (const mb3d::RenderStopped& ex) {
        profile.error = ex.what();
        std::fprintf(stderr, "Render stopped: %s\n", ex.what());
        return render_exit_code(ex.status());
    } catch (const std::exception& ex) {
        profile.error = ex.what();
        std::fprintf(stderr, "Fatal error: %s\n", ex.what());
        return 3;
    } catch (...) {
        profile.error = "unknown unhandled exception";
        std::fprintf(stderr, "Fatal error: unknown unhandled exception\n");
        return 3;
    }
}

static int main_impl(int argc, char* argv[], CLIProfile& profile) {
    // Handle --help, --version with no other args
    if (argc < 2) {
        print_usage();
        return 1;
    }

    // Check for --help and --version before anything else
    for (int i = 1; i < argc; i++) {
        if (std::strcmp(argv[i], "--help") == 0) {
            print_usage();
            return 0;
        }
        if (std::strcmp(argv[i], "--version") == 0) {
            print_version();
            return 0;
        }
    }

    // First non-option argument is the input file
    std::string inputPath;
    bool infoMode = false;
    mb3d::RenderOptions opts;
    mb3d::BackendOptions backendOptions;
    bool backendSet = false, precisionSet = false, metalLibrarySet = false;
    std::string mcLinearDumpPath;
    std::string silight5DumpPath;
    std::string shadowReplayDumpPath;
    std::string aoReplayDumpPath;
    std::string paintReplayDumpPath;
    std::vector<std::string> preRenderParityTrace;
    bool outputSet = false;
    bool noPostprocess = false;
    bool noZbufferNormals = false, noVolume = false, noHardShadowPass = false;
    int maxIterOverride = 0;
    uint64_t workBudget = 0;
    bool workBudgetSet = false;

    // Parse all arguments
    int i = 1;
    while (i < argc) {
        const char* arg = argv[i];

        if (arg[0] == '-') {
            // Option
            if (std::strcmp(arg, "-o") == 0 || std::strcmp(arg, "--output") == 0) {
                if (i + 1 >= argc) {
                    std::fprintf(stderr, "Error: %s requires an argument\n", arg);
                    return 1;
                }
                opts.output_path = argv[++i];
                outputSet = true;
            } else if (std::strcmp(arg, "-f") == 0 || std::strcmp(arg, "--format") == 0) {
                if (i + 1 >= argc) {
                    std::fprintf(stderr, "Error: %s requires an argument\n", arg);
                    return 1;
                }
                opts.format = argv[++i];
                if (opts.format != "png" && opts.format != "bmp") {
                    std::fprintf(stderr, "Error: unsupported format '%s' (use png or bmp)\n", opts.format.c_str());
                    return 1;
                }
            } else if (std::strcmp(arg, "-w") == 0 || std::strcmp(arg, "--width") == 0) {
                if (i + 1 >= argc) {
                    std::fprintf(stderr, "Error: %s requires an argument\n", arg);
                    return 1;
                }
                opts.width_override = std::atoi(argv[++i]);
                if (opts.width_override <= 0) {
                    std::fprintf(stderr, "Error: invalid width\n");
                    return 1;
                }
            } else if (std::strcmp(arg, "-h") == 0 || std::strcmp(arg, "--height") == 0) {
                if (i + 1 >= argc) {
                    std::fprintf(stderr, "Error: %s requires an argument\n", arg);
                    return 1;
                }
                opts.height_override = std::atoi(argv[++i]);
                if (opts.height_override <= 0) {
                    std::fprintf(stderr, "Error: invalid height\n");
                    return 1;
                }
            } else if (std::strcmp(arg, "-t") == 0 || std::strcmp(arg, "--threads") == 0) {
                if (i + 1 >= argc) {
                    std::fprintf(stderr, "Error: %s requires an argument\n", arg);
                    return 1;
                }
                opts.thread_count = std::atoi(argv[++i]);
                if (opts.thread_count <= 0) {
                    std::fprintf(stderr, "Error: invalid thread count\n");
                    return 1;
                }
            } else if (std::strcmp(arg, "--max-iter") == 0) {
                if (i + 1 >= argc) {
                    std::fprintf(stderr, "Error: %s requires an argument\n", arg);
                    return 1;
                }
                maxIterOverride = std::atoi(argv[++i]);
                if (maxIterOverride <= 0) {
                    std::fprintf(stderr, "Error: invalid max iterations\n");
                    return 1;
                }
            } else if (std::strcmp(arg, "--profile-json") == 0) {
                if (i + 1 >= argc || !profile.path.empty()) {
                    std::fprintf(stderr, "Error: --profile-json requires one path and may appear only once\n");
                    return 1;
                }
                profile.path = argv[++i];
                if (profile.path.empty()) return 1;
                opts.profile_timing = true;
                // A profile is evidence. Never overwrite an existing file (including
                // scene inputs or previously rendered output) with that evidence.
                if (std::filesystem::exists(profile.path)) {
                    profile.publish = false;
                    std::fprintf(stderr, "Error: profile path already exists: %s\n", profile.path.c_str());
                    return 4;
                }
            } else if (std::strcmp(arg, "--work-budget") == 0) {
                if (i + 1 >= argc) {
                    std::fprintf(stderr, "Error: %s requires an argument\n", arg);
                    return 1;
                }
                const char* value = argv[++i];
                const char* end = value + std::strlen(value);
                const auto parsed = std::from_chars(value, end, workBudget);
                if (parsed.ec != std::errc{} || parsed.ptr != end || value == end) {
                    std::fprintf(stderr, "Error: invalid work budget (expected unsigned 64-bit integer)\n");
                    return 1;
                }
                workBudgetSet = true;
            } else if (std::strcmp(arg, "--zbuffer") == 0) {
                if (i + 1 >= argc) {
                    std::fprintf(stderr, "Error: %s requires an argument\n", arg);
                    return 1;
                }
                opts.zbuffer_path = argv[++i];
            } else if (std::strcmp(arg, "--zbuffer-format") == 0) {
                if (i + 1 >= argc) {
                    std::fprintf(stderr, "Error: %s requires an argument\n", arg);
                    return 1;
                }
                opts.zbuffer_format = argv[++i];
                if (opts.zbuffer_format != "bmp" &&
                    opts.zbuffer_format != "png8" &&
                    opts.zbuffer_format != "png16") {
                    std::fprintf(stderr, "Error: unsupported z-buffer format '%s' (use bmp, png8 or png16)\n",
                                 opts.zbuffer_format.c_str());
                    return 1;
                }
            } else if (std::strcmp(arg, "--normalmap") == 0) {
                if (i + 1 >= argc) {
                    std::fprintf(stderr, "Error: %s requires an argument\n", arg);
                    return 1;
                }
                opts.normalmap_path = argv[++i];
            } else if (std::strcmp(arg, "--mc-linear-dump") == 0) {
                if (i + 1 >= argc) return 1;
                mcLinearDumpPath = argv[++i];
            } else if (std::strcmp(arg, "--silight5-dump") == 0) {
                if (i + 1 >= argc) {
                    std::fprintf(stderr, "Error: %s requires an argument\n", arg);
                    return 1;
                }
                silight5DumpPath = argv[++i];
            } else if (std::strcmp(arg, "--ray-setup-dump") == 0) {
                if (i + 1 >= argc) {
                    std::fprintf(stderr, "Error: %s requires an argument\n", arg);
                    return 1;
                }
                opts.ray_setup_dump_path = argv[++i];
            } else if (std::strcmp(arg, "--shadow-replay-from-silight") == 0) {
                if (i + 1 >= argc) {
                    std::fprintf(stderr, "Error: %s requires an argument\n", arg);
                    return 1;
                }
                shadowReplayDumpPath = argv[++i];
            } else if (std::strcmp(arg, "--ao-replay-from-silight") == 0) {
                if (i + 1 >= argc) {
                    std::fprintf(stderr, "Error: %s requires an argument\n", arg);
                    return 1;
                }
                aoReplayDumpPath = argv[++i];
            } else if (std::strcmp(arg, "--paint-replay-from-silight") == 0) {
                if (i + 1 >= argc) {
                    std::fprintf(stderr, "Error: %s requires an argument\n", arg);
                    return 1;
                }
                paintReplayDumpPath = argv[++i];
            } else if (std::strcmp(arg, "--diag-dump") == 0) {
                if (i + 1 >= argc) {
                    std::fprintf(stderr, "Error: %s requires an argument\n", arg);
                    return 1;
                }
                opts.diag_dump_path = argv[++i];
            } else if (std::strcmp(arg, "--trace-primary-dump") == 0) {
                if (i + 1 >= argc) {
                    std::fprintf(stderr, "Error: %s requires an argument\n", arg);
                    return 1;
                }
                opts.primary_trace_dump_path = argv[++i];
            } else if (std::strcmp(arg, "--parity-trace") == 0) {
                if (i + 1 >= argc) {
                    std::fprintf(stderr, "Error: %s requires an argument\n", arg);
                    return 1;
                }
                opts.parity_trace_dump_path = argv[++i];
            } else if (std::strcmp(arg, "--parity-trace-start-stage") == 0) {
                if (i + 1 >= argc) {
                    std::fprintf(stderr, "Error: %s requires an argument\n", arg);
                    return 1;
                }
                opts.parity_trace_start_stage = argv[++i];
            } else if (std::strcmp(arg, "--parity-trace-end-stage") == 0) {
                if (i + 1 >= argc) {
                    std::fprintf(stderr, "Error: %s requires an argument\n", arg);
                    return 1;
                }
                opts.parity_trace_end_stage = argv[++i];
            } else if (std::strcmp(arg, "--parity-trace-schema") == 0) {
                opts.parity_trace_schema_mode = true;
            } else if (std::strcmp(arg, "--parity-trace-level") == 0) {
                if (i + 1 >= argc) {
                    std::fprintf(stderr, "Error: %s requires an argument\n", arg);
                    return 1;
                }
                const char* lv = argv[++i];
                if (std::strcmp(lv, "checkpoint") == 0) {
                    opts.parity_trace_level = "checkpoint";
                } else if (std::strcmp(lv, "detail") == 0) {
                    opts.parity_trace_level = "detail";
                } else if (std::strcmp(lv, "all") == 0) {
                    opts.parity_trace_level = "all";
                } else {
                    std::fprintf(stderr, "Error: invalid --parity-trace-level '%s'\n", lv);
                    return 1;
                }
            } else if (std::strcmp(arg, "--parity-trace-detail") == 0) {
                if (i + 1 >= argc) {
                    std::fprintf(stderr, "Error: %s requires an argument\n", arg);
                    return 1;
                }
                opts.parity_trace_detail = argv[++i];
            } else if (std::strcmp(arg, "--diag-points") == 0) {
                if (i + 1 >= argc) {
                    std::fprintf(stderr, "Error: %s requires an argument\n", arg);
                    return 1;
                }
                if (!parse_diag_points(argv[++i], opts.diag_points)) {
                    std::fprintf(stderr, "Error: invalid diagnostic point list '%s'\n", argv[i]);
                    return 1;
                }
            } else if (std::strcmp(arg, "--diag-only") == 0) {
                opts.diag_only = true;
            } else if (std::strcmp(arg, "--lattice") == 0) {
                if (i + 1 >= argc || std::sscanf(argv[++i], "%d,%d,%d", &opts.lattice_stride, &opts.lattice_x, &opts.lattice_y) != 3 ||
                    opts.lattice_stride < 2 || opts.lattice_stride > 4 || opts.lattice_x < 0 || opts.lattice_y < 0 ||
                    opts.lattice_x >= opts.lattice_stride || opts.lattice_y >= opts.lattice_stride) {
                    std::fprintf(stderr, "Error: --lattice needs stride,x,y with stride 2..4 and x,y below it\n");
                    return 1;
                }
            } else if (std::strcmp(arg, "--mc-samples") == 0) {
                if (i + 1 >= argc) { std::fprintf(stderr, "Error: --mc-samples needs a count\n"); return 1; }
                char* end = nullptr;
                long count = std::strtol(argv[++i], &end, 10);
                if (!end || *end || count < 1 || count > 65535) {
                    std::fprintf(stderr, "Error: --mc-samples must be 1..65535\n"); return 1;
                }
                opts.mc_samples = static_cast<int>(count);
            } else if (std::strcmp(arg, "--deterministic") == 0) {
                opts.deterministic_mode = true;
            } else if (std::strcmp(arg, "--backend") == 0) {
                if (backendSet || i + 1 >= argc || !mb3d::ParseRenderBackend(argv[++i], backendOptions.requested)) {
                    std::fprintf(stderr, "Error: --backend requires cpu, metal or auto, once\n");
                    return 1;
                }
                backendSet = true;
                profile.backend.requested = backendOptions.requested;
            } else if (std::strcmp(arg, "--metal-library") == 0) {
                if (metalLibrarySet || i + 1 >= argc || !*argv[i + 1]) {
                    std::fprintf(stderr, "Error: --metal-library requires one path, once\n");
                    return 1;
                }
                metalLibrarySet = true;
                backendOptions.metal_library = argv[++i];
            } else if (std::strcmp(arg, "--precision") == 0) {
                if (i + 1 >= argc || !mb3d::ParsePrecisionMode(argv[++i], opts.precision)) {
                    std::fprintf(stderr, "Error: --precision requires compatible or native\n");
                    return 1;
                }
                precisionSet = true;
                profile.render.precision = opts.precision;
            } else if (std::strcmp(arg, "-v") == 0 || std::strcmp(arg, "--verbose") == 0) {
                opts.verbose = true;
            } else if (std::strcmp(arg, "--info") == 0) {
                infoMode = true;
            } else if (std::strcmp(arg, "--no-shadows") == 0) {
                opts.no_shadows = true;
            } else if (std::strcmp(arg, "--no-ao") == 0) {
                opts.no_ao = true;
            } else if (std::strcmp(arg, "--no-dof") == 0) {
                opts.no_dof = true;
            } else if (std::strcmp(arg, "--no-reflections") == 0) {
                opts.no_reflections = true;
            } else if (std::strcmp(arg, "--no-postprocess") == 0) {
                noPostprocess = true;
            } else if (std::strcmp(arg, "--no-zbuffer-normals") == 0) {
                noZbufferNormals = true;
            } else if (std::strcmp(arg, "--no-volume") == 0) {
                noVolume = true;
            } else if (std::strcmp(arg, "--no-hard-shadow-pass") == 0) {
                noHardShadowPass = true;
            } else {
                std::fprintf(stderr, "Error: unknown option '%s'\n", arg);
                std::fprintf(stderr, "Use --help for usage information.\n");
                return 1;
            }
        } else {
            // Positional argument = input file
            if (inputPath.empty()) {
                inputPath = arg;
            } else {
                std::fprintf(stderr, "Error: unexpected argument '%s'\n", arg);
                return 1;
            }
        }
        i++;
    }

    if (inputPath.empty()) {
        std::fprintf(stderr, "Error: no input file specified\n");
        print_usage();
        return 1;
    }

    if (backendOptions.requested != mb3d::RenderBackend::CPU) {
        if (precisionSet && opts.precision != mb3d::PrecisionMode::Native) {
            std::fprintf(stderr, "Error: --backend metal/auto requires --precision native\n");
            return 1;
        }
        if (infoMode || opts.mc_samples != 0 || !shadowReplayDumpPath.empty() ||
            !aoReplayDumpPath.empty() || !paintReplayDumpPath.empty()) {
            std::fprintf(stderr, "Error: --backend metal/auto supports ordinary full rendering only\n");
            return 1;
        }
        opts.precision = mb3d::PrecisionMode::Native;
        if (!metalLibrarySet) {
            // Resolve the running binary, not the current scene/resource directory.
            std::filesystem::path executable = std::filesystem::absolute(argv[0]);
#ifdef __APPLE__
            uint32_t size = 0;
            _NSGetExecutablePath(nullptr, &size);
            std::vector<char> path(size);
            if (_NSGetExecutablePath(path.data(), &size) == 0)
                executable = std::filesystem::canonical(path.data());
#endif
            backendOptions.metal_library = (executable.parent_path() / "metal_native_scene.metallib").string();
        }
    } else if (metalLibrarySet) {
        std::fprintf(stderr, "Error: --metal-library requires --backend metal or auto\n");
        return 1;
    }
    profile.render.precision = opts.precision;
    profile.threads_requested = opts.thread_count;

    // --no-postprocess sets all post-processing flags
    if (noPostprocess) {
        opts.no_shadows = true;
        opts.no_ao = true;
        opts.no_dof = true;
        opts.no_reflections = true;
    }

    // Default output path
    if (!outputSet) {
        opts.output_path = default_output_path(inputPath, opts.format);
    }

    if (opts.profile_timing) {
        profile.input = inputPath;
        profile.output = opts.output_path;
        const auto profilePath = std::filesystem::weakly_canonical(profile.path);
        const std::vector<std::string> outputs = {inputPath, opts.output_path, opts.zbuffer_path,
            opts.normalmap_path, opts.diag_dump_path, opts.primary_trace_dump_path,
            opts.parity_trace_dump_path, silight5DumpPath, mcLinearDumpPath};
        for (const auto& output : outputs) {
            if (!output.empty() && profilePath == std::filesystem::weakly_canonical(output)) {
                profile.publish = false;
                std::fprintf(stderr, "Error: profile path must differ from input and render output paths\n");
                return 1;
            }
        }
        if (infoMode || opts.mc_samples != 0 || !shadowReplayDumpPath.empty() ||
            !aoReplayDumpPath.empty() || !paintReplayDumpPath.empty() || opts.diag_only ||
            !opts.ray_setup_dump_path.empty()) {
            profile.error = "--profile-json supports ordinary full rendering only";
            std::fprintf(stderr, "Error: %s\n", profile.error.c_str());
            return 1;
        }
    }

    // Load .m3p or .m3i file
    auto totalStart = std::chrono::steady_clock::now();

    // Detect extension
    if (workBudgetSet) {
        if (opts.mc_samples != 0 || !shadowReplayDumpPath.empty() || !aoReplayDumpPath.empty()) {
            std::fprintf(stderr, "Error: --work-budget supports ordinary rendering and paint replay only\n");
            return 1;
        }
        opts.control = std::make_shared<mb3d::RenderControl>(workBudget);
    }

    bool isM3I = false;
    {
        auto dotPos = inputPath.rfind('.');
        if (dotPos != std::string::npos) {
            std::string ext = inputPath.substr(dotPos);
            // Case-insensitive compare
            for (auto& c : ext) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
            if (ext == ".m3i") isM3I = true;
        }
    }

    CLIStageTimer loadTimer(profile, profile.load);
    mb3d::M3PLoadResult loadResult;
    mb3d::M3ILoadResult m3iResult;

    if (isM3I) {
        m3iResult = mb3d::LoadM3IFile(inputPath);
        if (!m3iResult.success) {
            profile.error = m3iResult.error;
            std::fprintf(stderr, "Error loading %s: %s\n", inputPath.c_str(), m3iResult.error.c_str());
            return 2;
        }
        loadResult = m3iResult.params;
        if (opts.verbose) {
            std::fprintf(stderr, "Loaded .m3i: siLight=%s, image=%s, %d pixels\n",
                        m3iResult.has_silight ? "yes" : "no",
                        m3iResult.has_image ? "yes" : "no",
                        static_cast<int>(m3iResult.siLight.size()));
        }
    } else {
        loadResult = mb3d::LoadM3PFile(inputPath);
        if (!loadResult.success) {
            profile.error = loadResult.error;
            std::fprintf(stderr, "Error loading %s: %s\n", inputPath.c_str(), loadResult.error.c_str());
            return 2;
        }
    }

    loadTimer.complete();

    // --info mode: print parameters and exit
    if (infoMode) {
        print_info(inputPath, loadResult.header, loadResult.addon);
        return 0;
    }

    // Mirror Pascal's headless render init (mb3d/Mand.pas:3228 in
    // TMand3DForm.LoadStartupParas): when running headless, the scene is
    // always rendered as a full 3D calculation, regardless of the bCalc3D
    // flag stored in the .m3p file. Pascal's "Calculate 3D" GUI button does
    // the same thing (Mand.pas:1832). Without this override the scene file's
    // bCalc3D=0 value (left over from a 2D preview snapshot) would make C++
    // take the Calc2DSliceRows path while Pascal runs full 3D -- the two
    // outputs are then completely different images.
    loadResult.header.bCalc3D = 1;
    // Pascal's headless --no-postprocess also disables the automatic
    // normals-on-Z stage before entering the postprocessing queue.
    if (noPostprocess || noZbufferNormals) loadResult.header.byCalcNsOnZBufAuto = 0;
    // Same-engine effect controls modify only the loaded header copy.
    if (noVolume) loadResult.header.bVolLightNr = 0;
    if (noHardShadowPass) loadResult.header.bCalculateHardShadow = 0;

    // Apply max iteration override
    if (maxIterOverride > 0) {
        loadResult.header.Iterations = maxIterOverride;
    }

    // Direct diagnostic/replay paths also prepare formula constants and may
    // start workers. Bind their policy just as an ordinary Render does.
    auto cliContext = std::make_shared<mb3d::RenderContext>(opts.resource_root);
    cliContext->precision = opts.precision;
    mb3d::ScopedRenderContext cliContextBinding(cliContext);

    int dispWidth = (opts.width_override > 0) ? opts.width_override : loadResult.header.Width;
    int dispHeight = (opts.height_override > 0) ? opts.height_override : loadResult.header.Height;
    // CLI diagnostics/replay address the tile buffer, while scene dimensions
    // continue to describe the full camera frame.
    auto displayHeader = loadResult.header;
    displayHeader.Width = dispWidth;
    displayHeader.Height = dispHeight;
    int tileX = 0, tileY = 0;
    mb3d::GetPaintTileSizes(&displayHeader, dispWidth, dispHeight, tileX, tileY);

    if (opts.verbose) {
        std::fprintf(stderr, "Loading: %s (%dx%d, %d iterations)\n",
                    inputPath.c_str(), dispWidth, dispHeight,
                    loadResult.header.Iterations);
    }

    if ((!opts.diag_dump_path.empty() || !opts.primary_trace_dump_path.empty()) &&
        opts.diag_points.empty()) {
        opts.diag_points = default_diag_points(dispWidth, dispHeight);
    }
    if (!opts.parity_trace_dump_path.empty()) {
        if (opts.diag_points.size() != 1) {
            std::fprintf(stderr, "Error: --parity-trace requires exactly one diagnostic point via --diag-points x,y\n");
            return 1;
        }
    }
    if (!shadowReplayDumpPath.empty() && opts.diag_points.size() != 1) {
        std::fprintf(stderr, "Error: --shadow-replay-from-silight requires exactly one diagnostic point via --diag-points x,y\n");
        return 1;
    }
    if (!aoReplayDumpPath.empty() && opts.diag_points.size() != 1) {
        std::fprintf(stderr, "Error: --ao-replay-from-silight requires exactly one diagnostic point via --diag-points x,y\n");
        return 1;
    }
    if (!paintReplayDumpPath.empty() && opts.diag_points.size() != 1) {
        std::fprintf(stderr, "Error: --paint-replay-from-silight requires exactly one diagnostic point via --diag-points x,y\n");
        return 1;
    }

#ifdef MB3D_ENABLE_PARITY_TRACE
    opts.parity_trace_stage_state.reset();
    mb3d::ResetParityTraceStageRange();
    mb3d::SetParityTraceSchemaMode(opts.parity_trace_schema_mode);
    if (opts.parity_trace_level == "detail") {
        mb3d::SetParityTraceLevel(mb3d::ParityTraceLevel::DetailSelected);
    } else if (opts.parity_trace_level == "all") {
        mb3d::SetParityTraceLevel(mb3d::ParityTraceLevel::DetailAll);
    } else {
        mb3d::SetParityTraceLevel(mb3d::ParityTraceLevel::Checkpoint);
    }
    mb3d::SetParityTraceDetailStages(opts.parity_trace_detail);
    if (!opts.parity_trace_dump_path.empty()) {
        // Canary must be first line of the trace file â€” format self-test matching
        // Pascal ParityTrace.EmitCanary. See docs/PARITY_TRACE.md.
        preRenderParityTrace.push_back(mb3d::ParityCanaryLine());
        mb3d::ConfigureParityTraceStageRange(opts.parity_trace_start_stage,
                                             opts.parity_trace_end_stage);
        if (!opts.parity_trace_schema_mode) {
            // Schema mode suppresses legacy load_header / formula_schema emission
            // to keep the trace file schema-conformant.
            append_load_trace_lines(preRenderParityTrace, inputPath, loadResult, opts.diag_points);
        }
        opts.parity_trace_stage_state = mb3d::CaptureParityTraceStageRangeState();
    }
#endif

    if (!shadowReplayDumpPath.empty()) {
        std::vector<TsiLight5> silight;
        int dumpWidth = 0;
        int dumpHeight = 0;
        std::string dumpError;
        if (!read_silight5_dump(shadowReplayDumpPath, silight, dumpWidth, dumpHeight, dumpError)) {
            std::fprintf(stderr, "Replay error: %s\n", dumpError.c_str());
            return 3;
        }
        if (dumpWidth != dispWidth || dumpHeight != dispHeight) {
            std::fprintf(stderr, "Replay error: siLight5 dump size %dx%d does not match scene %dx%d\n",
                         dumpWidth, dumpHeight, dispWidth, dispHeight);
            return 3;
        }

        TMandHeader10 hsHeader = loadResult.header;
        int hsBaseState = loadResult.header.bHScalculated;
        if ((hsHeader.bCalc1HSsoft & 1) != 0) {
            hsHeader.bHScalculated = static_cast<uint8_t>((hsBaseState & 1) | hsHeader.bCalculateHardShadow);
        } else {
            hsHeader.bHScalculated = static_cast<uint8_t>((hsBaseState & 0xFD) | hsHeader.bCalculateHardShadow);
        }
        if ((hsHeader.bHScalculated & 2) != 0) {
            for (int lx = 0; lx < 6; ++lx) {
                if (((hsHeader.bCalculateHardShadow >> (lx + 2)) & 1) != 0) {
                    hsHeader.Light.Lights[lx].LFunction &= 0xCF;
                }
            }
        }

        TMCTparameter hsMct = mb3d::GetMCTparasFromHeader(hsHeader, &loadResult.addon, 1);
        if (!hsMct.bMCTisValid) {
            std::fprintf(stderr, "Replay error: failed to setup hard shadow MCT parameters\n");
            return 3;
        }

        TLightVals baseLightVals;
        std::memset(&baseLightVals, 0, sizeof(TLightVals));
        TLValigned baseAlignedStorage;
        std::memset(&baseAlignedStorage, 0, sizeof(TLValigned));
        baseLightVals.PLValigned = &baseAlignedStorage;
#ifdef MB3D_ENABLE_PARITY_TRACE
        if (!opts.parity_trace_dump_path.empty()) {
            mb3d::ConfigureParityTraceStageRange(opts.parity_trace_start_stage,
                                                 opts.parity_trace_end_stage);
            auto p = opts.diag_points.front();
            mb3d::SetParityTraceContext(p.x, p.y, &preRenderParityTrace);
        }
#endif
        mb3d::MakeLightValsFromHeaderLight(&loadResult.header, &baseLightVals, 1.0, 0);
        mb3d::CompleteLightValsSetup(&loadResult.header, &baseLightVals);

        TLightVals hsLightVals = baseLightVals;
        TLValigned hsAlignedStorage = *baseLightVals.PLValigned;
        hsLightVals.PLValigned = &hsAlignedStorage;
        mb3d::CalcHSVecsFromLights(&hsLightVals, &hsMct);

        hsMct.calcHardShadow = hsHeader.bCalculateHardShadow;
        if (hsHeader.bCalc1HSsoft & 1) {
            hsMct.calcHardShadow |= 256;
        }
        hsMct.sHSmaxLengthMultiplier = (hsHeader.HSmaxLengthMultiplier > 0.0f)
            ? hsHeader.HSmaxLengthMultiplier : 1.0f;
        {
            float ssr = mb3d::ShortFloatToSingle(hsHeader.MCSoftShadowRadius);
            hsMct.SoftShadowRadius = (ssr > 0.0f) ? ssr : 1.0f;
        }

        auto p = opts.diag_points.front();
        mb3d::ReplayHardShadowsPrefix(silight.data(), dumpWidth, dumpHeight, dumpWidth,
                                      &hsMct, &hsLightVals, p.x, p.y);
#ifdef MB3D_ENABLE_PARITY_TRACE
        if (!opts.parity_trace_dump_path.empty()) {
            const auto& psi = silight[static_cast<size_t>(p.y) * dumpWidth + p.x];
            std::ostringstream payload;
            payload << "Shadow=" << psi.Shadow << " AmbShadow=" << psi.AmbShadow;
            mb3d::AppendParityTrace("shadow_post", payload.str());
            mb3d::ClearParityTraceContext();
        }
#endif

        if (!opts.parity_trace_dump_path.empty()) {
            if (!write_parity_trace_dump(opts.parity_trace_dump_path, {}, preRenderParityTrace)) {
                std::fprintf(stderr, "Failed to write parity trace dump: %s\n",
                             opts.parity_trace_dump_path.c_str());
                return 4;
            }
        }
        if (!silight5DumpPath.empty()) {
            if (!write_silight5_dump(silight5DumpPath, silight, dumpWidth, dumpHeight)) {
                std::fprintf(stderr, "Failed to write siLight5 dump: %s\n", silight5DumpPath.c_str());
                return 4;
            }
        }
        std::printf("Done: shadow replay %s @ %d,%d\n", inputPath.c_str(), p.x, p.y);
        return 0;
    }

    if (!aoReplayDumpPath.empty()) {
        std::vector<TsiLight5> silight;
        int dumpWidth = 0;
        int dumpHeight = 0;
        std::string dumpError;
        if (!read_silight5_dump(aoReplayDumpPath, silight, dumpWidth, dumpHeight, dumpError)) {
            std::fprintf(stderr, "AO replay error: %s\n", dumpError.c_str());
            return 3;
        }
        if (dumpWidth != dispWidth || dumpHeight != dispHeight) {
            std::fprintf(stderr, "AO replay error: siLight5 dump size %dx%d does not match scene %dx%d\n",
                         dumpWidth, dumpHeight, dispWidth, dispHeight);
            return 3;
        }

        TMCTparameter aoMct = mb3d::GetMCTparasFromHeader(loadResult.header, &loadResult.addon, 1);
        if (!aoMct.bMCTisValid) {
            std::fprintf(stderr, "AO replay error: failed to setup MCT parameters\n");
            return 3;
        }

        TLightVals baseLightVals;
        std::memset(&baseLightVals, 0, sizeof(TLightVals));
        TLValigned baseAlignedStorage;
        std::memset(&baseAlignedStorage, 0, sizeof(TLValigned));
        baseLightVals.PLValigned = &baseAlignedStorage;
#ifdef MB3D_ENABLE_PARITY_TRACE
        if (!opts.parity_trace_dump_path.empty()) {
            mb3d::ConfigureParityTraceStageRange(opts.parity_trace_start_stage,
                                                 opts.parity_trace_end_stage);
            auto p = opts.diag_points.front();
            mb3d::SetParityTraceContext(p.x, p.y, &preRenderParityTrace);
        }
#endif
        mb3d::MakeLightValsFromHeaderLight(&loadResult.header, &baseLightVals, 1.0, 0);
        mb3d::CompleteLightValsSetup(&loadResult.header, &baseLightVals);

        int aoType = (loadResult.header.bCalcAmbShadowAutomatic >> 2) & 3;
        int aoQuality = (loadResult.header.bCalcAmbShadowAutomatic >> 4) & 3;
        if (!opts.parity_trace_dump_path.empty()) {
            const auto& p = opts.diag_points.front();
            mb3d::ReplayAmbientOcclusionPixel(silight.data(), dumpWidth, dumpHeight, dumpWidth,
                                              &aoMct, &baseLightVals, aoType, aoQuality, &loadResult.header,
                                              p.x, p.y);
        } else {
            mb3d::ReplayAmbientOcclusion(silight.data(), dumpWidth, dumpHeight, dumpWidth,
                                         &aoMct, &baseLightVals, aoType, aoQuality, &loadResult.header,
                                         std::max(1, opts.thread_count));
        }

#ifdef MB3D_ENABLE_PARITY_TRACE
        if (!opts.parity_trace_dump_path.empty()) {
            const auto& p = opts.diag_points.front();
            const auto& psi = silight[static_cast<size_t>(p.y) * dumpWidth + p.x];
            std::ostringstream payload;
            payload << "Shadow=" << psi.Shadow << " AmbShadow=" << psi.AmbShadow;
            mb3d::AppendParityTrace("ao_post", payload.str());
            mb3d::ClearParityTraceContext();
        }
#endif

        if (!opts.parity_trace_dump_path.empty()) {
            if (!write_parity_trace_dump(opts.parity_trace_dump_path, {}, preRenderParityTrace)) {
                std::fprintf(stderr, "Failed to write parity trace dump: %s\n",
                             opts.parity_trace_dump_path.c_str());
                return 4;
            }
        }
        if (!silight5DumpPath.empty()) {
            if (!write_silight5_dump(silight5DumpPath, silight, dumpWidth, dumpHeight)) {
                std::fprintf(stderr, "Failed to write siLight5 dump: %s\n", silight5DumpPath.c_str());
                return 4;
            }
        }
        const auto& p = opts.diag_points.front();
        std::printf("Done: ao replay %s @ %d,%d\n", inputPath.c_str(), p.x, p.y);
        return 0;
    }

    if (!paintReplayDumpPath.empty()) {
        std::vector<TsiLight5> silight;
        int dumpWidth = 0;
        int dumpHeight = 0;
        std::string dumpError;
        if (!read_silight5_dump(paintReplayDumpPath, silight, dumpWidth, dumpHeight, dumpError)) {
            std::fprintf(stderr, "Paint replay error: %s\n", dumpError.c_str());
            return 3;
        }
        if (dumpWidth != dispWidth || dumpHeight != dispHeight) {
            std::fprintf(stderr, "Paint replay error: siLight5 dump size %dx%d does not match scene %dx%d\n",
                         dumpWidth, dumpHeight, dispWidth, dispHeight);
            return 3;
        }
#ifdef MB3D_ENABLE_PARITY_TRACE
        if (!opts.parity_trace_dump_path.empty()) {
            mb3d::ConfigureParityTraceStageRange(opts.parity_trace_start_stage,
                                                 opts.parity_trace_end_stage);
            append_load_trace_lines(preRenderParityTrace, inputPath, loadResult, opts.diag_points);
            opts.parity_trace_stage_state = mb3d::CaptureParityTraceStageRangeState();
        }
#endif
        auto replay = mb3d::PaintFromSiLight(loadResult.header, loadResult.addon, silight, opts);
        if (!replay.success) {
            std::fprintf(stderr, "Paint replay error: %s\n", replay.error.c_str());
            return render_exit_code(replay.status);
        }
        if (!opts.diag_only) {
            mb3d::FinalRGBSurface replaySurface;
            if (!mb3d::BuildFinalRGBSurface(replay.image.data(),
                                            replay.width,
                                            replay.height,
                                            loadResult.header.bImageScale,
                                            0,
                                            replay.tiling_crop,
                                            replaySurface)) {
                std::fprintf(stderr, "Paint replay error: failed to build final RGB surface\n");
                return 3;
            }
            bool writeOk = false;
            if (opts.format == "bmp") {
                writeOk = mb3d::WriteBMP(opts.output_path,
                                         replaySurface.pixels.data(),
                                         replaySurface.width,
                                         replaySurface.height);
            } else {
                writeOk = mb3d::WritePNG(opts.output_path,
                                         replaySurface.pixels.data(),
                                         replaySurface.width,
                                         replaySurface.height,
                                         3);
            }
            if (!writeOk) {
                std::fprintf(stderr, "Failed to write output: %s\n", opts.output_path.c_str());
                return 4;
            }
        }
        std::string bufferError;
        if (!write_optional_buffers_from_silight(opts, loadResult.header, silight,
                                                 dumpWidth, dumpHeight, replay.tiling_crop,
                                                 bufferError)) {
            std::fprintf(stderr, "Paint replay error: %s\n", bufferError.c_str());
            return 4;
        }
        if (!opts.parity_trace_dump_path.empty()) {
            preRenderParityTrace.insert(preRenderParityTrace.end(),
                                        replay.parity_trace_lines.begin(),
                                        replay.parity_trace_lines.end());
            if (!write_parity_trace_dump(opts.parity_trace_dump_path, {}, preRenderParityTrace)) {
                std::fprintf(stderr, "Failed to write parity trace dump: %s\n",
                             opts.parity_trace_dump_path.c_str());
                return 4;
            }
        }
        if (!silight5DumpPath.empty()) {
            if (!write_silight5_dump(silight5DumpPath, silight, dumpWidth, dumpHeight)) {
                std::fprintf(stderr, "Failed to write siLight5 dump: %s\n", silight5DumpPath.c_str());
                return 4;
            }
        }
        const auto& p = opts.diag_points.front();
        std::printf("Done: paint replay %s @ %d,%d\n", inputPath.c_str(), p.x, p.y);
        return 0;
    }

    // Render
    auto renderStart = std::chrono::steady_clock::now();
    profile.render_entered = opts.profile_timing;
    auto selected = mb3d::RenderWithBackend(loadResult.header, loadResult.addon, opts, backendOptions);
    auto render = std::move(selected.render);
    profile.backend = std::move(selected.backend);
    std::fprintf(stderr, "Backend: requested=%s effective=%s precision=%s%s%s\n",
        mb3d::RenderBackendName(profile.backend.requested), mb3d::RenderBackendName(profile.backend.effective),
        mb3d::PrecisionModeName(opts.precision), profile.backend.fallback ? "; fallback: " : "",
        profile.backend.fallback_reason.c_str());
    if (profile.backend.fallback_code == "fp32_range")
        std::fprintf(stderr, "Metal: the scene does not fit FP32 (%s; limit 1%%); the frame was rendered on CPU\n",
            profile.backend.fallback_reason.c_str());
    if (profile.backend.execution.difs_rerun_carried_reads)
        std::fprintf(stderr, "Metal: dIFS read carried state in %llu pixels (%llu orbits); the frame was re-rendered in CPU order\n",
            static_cast<unsigned long long>(profile.backend.execution.difs_rerun_carried_pixels),
            static_cast<unsigned long long>(profile.backend.execution.difs_rerun_carried_reads));
    if (profile.backend.execution.ao_difs_rerun_carried_reads)
        std::fprintf(stderr, "Metal: DE AO dIFS read carried state in %llu pixels (%llu orbits); AO was re-run in CPU task order\n",
            static_cast<unsigned long long>(profile.backend.execution.ao_difs_rerun_carried_pixels),
            static_cast<unsigned long long>(profile.backend.execution.ao_difs_rerun_carried_reads));
    profile.render = render.profile;
    profile.render_status = render.status;
    profile.error = render.error;
    double renderTime = elapsed_sec(renderStart);

    if (!render.success) {
        std::fprintf(stderr, "Render error: %s\n", render.error.c_str());
        return render_exit_code(render.status);
    }

    if (opts.verbose) {
        std::fprintf(stderr, "Render complete (%.1fs)\n", renderTime);
    }

    if (!opts.ray_setup_dump_path.empty()) {
        if (!write_ray_setup_dump(opts.ray_setup_dump_path, render.ray_setup_records,
                                  render.width, render.height)) {
            std::fprintf(stderr, "Failed to write ray setup dump: %s\n",
                         opts.ray_setup_dump_path.c_str());
            return 4;
        }
        if (opts.verbose) {
            std::fprintf(stderr, "Wrote ray setup dump: %s\n", opts.ray_setup_dump_path.c_str());
        } else {
            std::printf("Done: %s (%dx%d)\n", opts.ray_setup_dump_path.c_str(), render.width, render.height);
        }
        return 0;
    }

    // Write output image
    if (opts.verbose) {
        std::fprintf(stderr, "Writing: %s\n", opts.output_path.c_str());
    }

    CLIStageTimer finalSurfaceTimer(profile, profile.final_surface);
    mb3d::FinalRGBSurface finalSurface;
    // For headless CLI: save at full render resolution (no AA downscale).
    // bImageScale is a GUI-only feature for 2x/3x supersampled preview.
    // Pascal headless also saves at full render size.
    if (!mb3d::BuildFinalRGBSurface(render.image.data(),
                                    render.width,
                                    render.height,
                                    1, // no downscale for CLI
                                    0,
                                    0, // Pascal headless saves the tile overlap border.
                                    finalSurface)) {
        profile.error = "Failed to build final RGB surface";
        std::fprintf(stderr, "Failed to build final RGB surface\n");
        return 4;
    }
    finalSurfaceTimer.complete();
    if (opts.verbose && (finalSurface.width != render.width || finalSurface.height != render.height)) {
        std::fprintf(stderr, "Final save surface: %dx%d -> %dx%d (scale=%d crop=%d)\n",
                     render.width,
                     render.height,
                     finalSurface.width,
                     finalSurface.height,
                     loadResult.header.bImageScale,
                     render.tiling_crop);
    }

    CLIStageTimer saveTimer(profile, profile.save);
    bool writeOk = false;
    if (opts.format == "bmp") {
        writeOk = mb3d::WriteBMP(opts.output_path,
                                 finalSurface.pixels.data(),
                                 finalSurface.width,
                                 finalSurface.height);
    } else {
        writeOk = mb3d::WritePNG(opts.output_path,
                                 finalSurface.pixels.data(),
                                 finalSurface.width,
                                 finalSurface.height,
                                 3);
    }

    if (!writeOk) {
        profile.error = "Failed to write output image";
        std::fprintf(stderr, "Failed to write output: %s\n", opts.output_path.c_str());
        return 4;
    }

    saveTimer.complete();

    // Write optional buffers
    if (!mcLinearDumpPath.empty()) {
        if (render.mc_linear.empty()) { std::fprintf(stderr, "MC dump requires --mc-samples\n"); return 4; }
        std::ofstream out(mcLinearDumpPath);
        out.precision(17);
        out << render.width << " " << render.height << "\n";
        for (size_t i = 0; i < render.mc_linear.size(); i += 3)
            out << render.mc_linear[i] << " " << render.mc_linear[i+1] << " " << render.mc_linear[i+2] << "\n";
        if (!out) return 4;
    }
    std::string bufferError;
    if (!write_optional_buffers_from_silight(opts, loadResult.header, render.silight,
                                             render.width, render.height, render.tiling_crop,
                                             bufferError)) {
        std::fprintf(stderr, "Failed to write optional buffer: %s\n", bufferError.c_str());
        return 4;
    }

    if (!opts.diag_dump_path.empty()) {
        if (!write_diag_dump(opts.diag_dump_path, inputPath, loadResult.header, render.diagnostics)) {
            std::fprintf(stderr, "Failed to write diagnostic dump: %s\n", opts.diag_dump_path.c_str());
            return 4;
        }
    }

    if (!opts.primary_trace_dump_path.empty()) {
        if (!write_primary_trace_dump(opts.primary_trace_dump_path, render.diagnostics)) {
            std::fprintf(stderr, "Failed to write primary trace dump: %s\n",
                         opts.primary_trace_dump_path.c_str());
            return 4;
        }
    }

    if (!silight5DumpPath.empty()) {
#ifdef MB3D_ENABLE_PARITY_TRACE
        if (!opts.parity_trace_dump_path.empty() && opts.diag_points.size() == 1) {
            const auto& p = opts.diag_points.front();
            if (p.x >= 0 && p.x < render.width && p.y >= 0 && p.y < render.height) {
                const auto& psi = render.silight[static_cast<size_t>(p.y) * render.width + p.x];
                mb3d::SetParityTraceContext(p.x, p.y, &render.parity_trace_lines);
                std::ostringstream payload;
                payload << "Shadow=" << psi.Shadow
                        << " AmbShadow=" << psi.AmbShadow
                        << " Zpos=" << psi.Zpos
                        << " RoughZposFine=" << psi.RoughZposFine
                        << " SIgradient=" << psi.SIgradient
                        << " OTrap=" << psi.OTrap
                        << " NormalX=" << psi.NormalX
                        << " NormalY=" << psi.NormalY
                        << " NormalZ=" << psi.NormalZ;
                mb3d::AppendParityTrace("silight_dump", payload.str());
                mb3d::ClearParityTraceContext();
            }
        }
#endif
        if (!write_silight5_dump(silight5DumpPath, render.silight, render.width, render.height)) {
            std::fprintf(stderr, "Failed to write siLight5 dump: %s\n", silight5DumpPath.c_str());
            return 4;
        }
    }

    if (!opts.parity_trace_dump_path.empty()) {
        if (!write_parity_trace_dump(opts.parity_trace_dump_path,
                                     preRenderParityTrace,
                                     render.parity_trace_lines)) {
            std::fprintf(stderr, "Failed to write parity trace dump: %s\n",
                         opts.parity_trace_dump_path.c_str());
            return 4;
        }
    }

    double totalTime = elapsed_sec(totalStart);
    if (opts.verbose) {
        std::fprintf(stderr, "Done (%.1fs total)\n", totalTime);
    } else {
        std::printf("Done: %s (%dx%d)\n", opts.output_path.c_str(), render.width, render.height);
    }

    return 0;
}

int main(int argc, char** argv) {
    try {
        CLIProfile profile;
        const auto start = std::chrono::steady_clock::now();
        int code = RunCLI(argc, argv, profile);
        if (!profile.path.empty() && profile.publish) {
            const double elapsed = std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count();
            if (!write_profile(profile, code, elapsed)) {
                std::fprintf(stderr, "Failed to write profile: %s\n", profile.path.c_str());
                if (code == 0) code = 4;
            }
        }
        return code;
    } catch (const std::exception& error) {
        std::fprintf(stderr, "Render failed: %s\n", error.what());
    } catch (...) {
        std::fprintf(stderr, "Render failed: unknown exception\n");
    }
    return 3;
}
