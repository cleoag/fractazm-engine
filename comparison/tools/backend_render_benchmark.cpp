// Full ordinary main-application API renders. Both backends recreate their
// complete job resources on every RenderWithBackend call; only process/OS/driver
// caches can be warm. No standalone Metal prepared-context reuse occurs here.
#include "core/formula_registry.h"
#include "core/image_process.h"
#include "render/backend.h"
#include "io/image_writer.h"
#include "io/m3p_parser.h"
#include "frame_deadline.h"

#include <algorithm>
#include <charconv>
#include <chrono>
#include <cctype>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <iomanip>
#include <locale>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace {
namespace fs = std::filesystem;
using Clock = std::chrono::steady_clock;

double milliseconds(Clock::time_point begin, Clock::time_point end) {
    return std::chrono::duration<double, std::milli>(end - begin).count();
}

std::string quoted(const std::string& value) {
    std::ostringstream out;
    out << '"';
    static constexpr char digits[] = "0123456789abcdef";
    for (unsigned char c : value) {
        if (c == '"' || c == '\\') out << '\\' << c;
        else if (c < 0x20) out << "\\u00" << digits[c >> 4] << digits[c & 15];
        else out << c;
    }
    out << '"';
    return out.str();
}

struct Options {
    fs::path scene, output, resource_root;
    mb3d::PrecisionMode precision = mb3d::PrecisionMode::Native;
    mb3d::BackendOptions backend;
    bool backend_set = false;
    // Keep legacy benchmark invocations stable; full-scene qualification
    // explicitly selects scene RNG, including authored first-step jitter.
    bool scene_rng = false;
    int width = 0, height = 0, threads = 0, warmups = 1, repeats = 3;
    int frame_timeout_seconds = 300;
};

int integer(const char* value, const std::string& key) {
    int result = 0;
    const char* end = value + std::strlen(value);
    const auto parsed = std::from_chars(value, end, result);
    if (parsed.ec != std::errc{} || parsed.ptr != end)
        throw std::invalid_argument("Invalid integer for " + key);
    return result;
}

Options parse(int argc, char** argv) {
    Options result;
    std::vector<std::string> seen;
    for (int i = 1; i < argc; ++i) {
        const std::string key = argv[i];
        if (std::find(seen.begin(), seen.end(), key) != seen.end())
            throw std::invalid_argument("Repeated option: " + key);
        seen.push_back(key);
        if (i + 1 == argc) throw std::invalid_argument("Missing value for " + key);
        const char* value = argv[++i];
        if (key == "--scene") result.scene = value;
        else if (key == "--output") result.output = value;
        else if (key == "--resource-root") result.resource_root = value;
        else if (key == "--backend") {
            if (!mb3d::ParseRenderBackend(value, result.backend.requested) ||
                result.backend.requested == mb3d::RenderBackend::Auto)
                throw std::invalid_argument("Benchmark backend must be cpu or metal");
            result.backend_set = true;
        }
        else if (key == "--metal-library") result.backend.metal_library = fs::absolute(value).string();
        else if (key == "--precision") {
            if (!mb3d::ParsePrecisionMode(value, result.precision))
                throw std::invalid_argument("Precision must be compatible or native");
        }
        else if (key == "--width") result.width = integer(value, key);
        else if (key == "--height") result.height = integer(value, key);
        else if (key == "--threads") result.threads = integer(value, key);
        else if (key == "--warmups") result.warmups = integer(value, key);
        else if (key == "--repeats") result.repeats = integer(value, key);
        else if (key == "--frame-timeout-seconds") result.frame_timeout_seconds = integer(value,key);
        else if (key == "--rng-mode") {
            if (std::strcmp(value,"scene") && std::strcmp(value,"parity"))
                throw std::invalid_argument("RNG mode must be scene or parity");
            result.scene_rng = std::strcmp(value,"scene")==0;
        }
        else throw std::invalid_argument("Unknown option: " + key);
    }
    if (result.scene.empty() || result.output.empty() ||
        result.width < 1 || result.height < 1 || result.width > 16384 ||
        result.height > 16384 || result.threads < 1 || result.warmups < 0 ||
        result.repeats < 3 || !result.backend_set || result.frame_timeout_seconds<1 || result.frame_timeout_seconds>300 ||
        result.threads > std::min(64, result.height) || result.precision != mb3d::PrecisionMode::Native ||
        (result.backend.requested == mb3d::RenderBackend::Metal && result.backend.metal_library.empty()) ||
        (result.backend.requested == mb3d::RenderBackend::CPU && !result.backend.metal_library.empty()))
        throw std::invalid_argument(
            "Required: --scene FILE --output NEW_DIR --width 1..16384 "
            "--height 1..16384 --threads N [--warmups N>=0] [--repeats N>=3] "
            "[--resource-root DIR] [--rng-mode scene|parity] --backend cpu|metal --precision native [--metal-library FILE]");
    result.scene = fs::absolute(result.scene);
    result.output = fs::absolute(result.output);
    result.resource_root = result.resource_root.empty() ? fs::current_path()
                                                       : fs::absolute(result.resource_root);
    if (!fs::is_regular_file(result.scene))
        throw std::invalid_argument("Scene is not a regular file");
    std::string extension = result.scene.extension().string();
    std::transform(extension.begin(), extension.end(), extension.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    if (extension != ".m3p") throw std::invalid_argument("Only ordinary .m3p input is supported");
    if (!fs::is_directory(result.resource_root))
        throw std::invalid_argument("Resource root is not a directory");
    return result;
}

std::string formula_name(const THAformula& formula) {
    const char* data = reinterpret_cast<const char*>(formula.CustomFname);
    size_t size = 0;
    while (size < sizeof(formula.CustomFname) && data[size]) ++size;
    while (size && data[size - 1] == ' ') --size;
    return std::string(data, size);
}

void reject_deferred(const mb3d::M3PLoadResult& scene) {
    if (scene.header.MCoptions & 1u)
        throw std::invalid_argument("Deferred Monte Carlo scene is excluded");
    for (const auto& formula : scene.addon.Formulas) {
        if (mb3d::IsInternalFormula(formula.iFnr)) continue;
        std::string name = formula_name(formula);
        std::transform(name.begin(), name.end(), name.begin(),
                       [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        // Reject even dormant saved JIT slots rather than admit a deferred input.
        if (name.find("jit") != std::string::npos)
            throw std::invalid_argument("Deferred JIT scene is excluded");
    }
    // Render performs the existing prepared-parameter and reachable-formula
    // validation before it starts workers. Do not initialize its registry here:
    // cold timing must include the real first Render's preparation.
}

struct Sample {
    std::string phase, image, state = "not_run", error;
    int index = 0, width = 0, height = 0, threads = 0;
    bool compared = false, exact_rgb = false;
    double load_ms = 0, render_ms = 0, readback_ms = 0;
    double render_and_readback_ms = 0, png_encode_write_ms = 0;
    double end_to_end_ms = 0, cold_total_ms = 0, cold_end_to_end_ms = 0;
    double deadline_seconds = 0;
    int deadline_limit = 0;
    mb3d::RenderProfile profile;
    mb3d::BackendSelection backend;
    mb3d::RenderStageTiming final_surface, save;
};

struct Report {
    Options options;
    std::string status = "FAILED", error;
    mb3d::M3PLoadResult loaded{};
    double load_ms = 0, setup_ms = 0;
    Sample cold;
    std::vector<Sample> warmups, samples;
};

void write_backend(std::ostream& out, const mb3d::BackendSelection& b) {
    const auto& r = b.reflections;
    const auto& e = b.execution;
    out << "{\"requested\":" << quoted(mb3d::RenderBackendName(b.requested))
        << ",\"effective\":" << quoted(mb3d::RenderBackendName(b.effective))
        << ",\"fallback\":" << (b.fallback ? "true" : "false")
        << ",\"fallback_code\":" << quoted(b.fallback_code)
        << ",\"fallback_reason\":" << quoted(b.fallback_reason)
        << ",\"device\":" << quoted(b.device) << ",\"library\":" << quoted(b.library)
        << ",\"selection_seconds\":" << b.selection_seconds
        << ",\"reflections\":{\"enabled\":" << (r.enabled ? "true" : "false")
        << ",\"active_pixels\":" << r.active_pixels << ",\"rays\":" << r.rays
        << ",\"hits\":" << r.hits << ",\"misses\":" << r.misses
        << ",\"changed_pixels\":" << r.changed_pixels << ",\"faults\":" << r.faults
        << ",\"max_bounces\":" << r.max_bounces
        << ",\"ray_steps\":" << r.ray_steps << ",\"primary_skipped\":" << r.primary_skipped
        << ",\"ao_rays\":" << r.ao_rays << ",\"ao_steps\":" << r.ao_steps
        << ",\"shadow_rays\":" << r.shadow_rays << ",\"shadow_steps\":" << r.shadow_steps
        << ",\"max_command_seconds\":" << r.max_command_seconds << '}'
        << ",\"execution\":{\"validated\":" << (e.validated ? "true" : "false")
        << ",\"engine\":" << quoted(e.engine)
        << ",\"execution_plan_version\":" << e.execution_plan_version
        << ",\"execution_plan\":" << quoted(e.execution_plan)
        << ",\"library_profile\":[" << e.profile_kind << ',' << e.storage_bytes << ',' << e.arithmetic_bits << ']'
        << ",\"pipeline_archive\":[" << e.pipeline_archive_hits << ',' << e.pipeline_archive_misses << ']'
        << ",\"fp32_range_exits\":" << e.fp32_range_exits
        << ",\"fp32_range_exit_pixels\":" << e.fp32_range_exit_pixels
        << ",\"ao_fp32_range_exit_pixels\":" << e.ao_fp32_range_exit_pixels
        << ",\"fp32_range_gate_stage\":\"" << e.fp32_range_gate_stage << "\""
        << ",\"fp32_range_gate_units\":" << e.fp32_range_gate_units
        << ",\"fp32_range_gate_exceeded\":" << e.fp32_range_gate_exceeded
        << ",\"fp32_range_gate_percent\":" << e.fp32_range_gate_percent
        << ",\"difs_carried_reads\":" << e.difs_carried_reads
        << ",\"difs_carried_pixels\":" << e.difs_carried_pixels
        << ",\"difs_conditional_pixels\":" << (e.difs_conditional_pixels ? "true" : "false")
        << ",\"difs_rerun_carried_reads\":" << e.difs_rerun_carried_reads
        << ",\"difs_rerun_carried_pixels\":" << e.difs_rerun_carried_pixels
        << ",\"ao_difs_carried_reads\":" << e.ao_difs_carried_reads
        << ",\"ao_difs_carried_pixels\":" << e.ao_difs_carried_pixels
        << ",\"ao_difs_conditional_pixels\":" << (e.ao_difs_conditional_pixels ? "true" : "false")
        << ",\"ao_difs_rerun_carried_reads\":" << e.ao_difs_rerun_carried_reads
        << ",\"ao_difs_rerun_carried_pixels\":" << e.ao_difs_rerun_carried_pixels
        << ",\"primary_random_mode\":" << e.primary_random_mode
        << ",\"primary_random_probe_dispatches\":" << e.primary_random_probe_dispatches
        << ",\"primary_seed_scan_commands\":" << e.primary_seed_scan_commands
        << ",\"ao_seed_generation_commands\":" << e.ao_seed_generation_commands
        << ",\"primary_random_advances\":" << e.primary_random_advances
        << ",\"primary_contained\":" << e.primary_contained
        << ",\"primary_entry_equivalent\":" << e.primary_entry_equivalent
        << ",\"shadow_speculative_pixels\":" << e.shadow_speculative_pixels
        << ",\"shadow_recomputed_pixels\":" << e.shadow_recomputed_pixels
        << ",\"shadow_speculation_fallbacks\":" << e.shadow_speculation_fallbacks
        << ",\"primary_records\":" << e.primary_records
        << ",\"normal_records\":" << e.normal_records
        << ",\"surface_records\":" << e.surface_records
        << ",\"shadow_records\":" << e.shadow_records
        << ",\"shadow_rays\":" << e.shadow_rays
        << ",\"shadow_hits\":" << e.shadow_hits
        << ",\"shadow_misses\":" << e.shadow_misses
        << ",\"shadow_steps\":" << e.shadow_steps
        << ",\"shadow_changed\":" << e.shadow_changed;
    if(e.soft_shadow_records)
        out << ",\"soft_shadow_record_version\":" << e.soft_shadow_record_version << ",\"soft_shadow_records\":" << e.soft_shadow_records
            << ",\"soft_shadow_partial_pixels\":" << e.soft_shadow_partial_pixels
            << ",\"soft_shadow_dark_pixels\":" << e.soft_shadow_dark_pixels
            << ",\"soft_shadow_light_pixels\":" << e.soft_shadow_light_pixels;
    out << ",\"color_records\":" << e.color_records
        << ",\"reflection_records\":" << e.reflection_records
        << ",\"distance_calls\":" << e.distance_calls
        << ",\"formula_work\":" << e.formula_work
        << ",\"smoothing_record_version\":" << e.smoothing_record_version
        << ",\"smoothing_policy\":" << e.smoothing_policy
        << ",\"smoothing_calls\":" << e.smoothing_calls
        << ",\"smoothing_recovered\":" << e.smoothing_recovered
        << ",\"smoothing_stagnant\":" << e.smoothing_stagnant
        << ",\"smoothing_invalid\":" << e.smoothing_invalid
        << ",\"normal_samples\":" << e.normal_samples
        << ",\"normal_de_calls\":" << e.normal_de_calls
        << ",\"normal_orbit_calls\":" << e.normal_orbit_calls
        << ",\"normal_evaluations\":" << e.normal_evaluations
        << ",\"primary_rough_pixels\":" << e.primary_rough_pixels
        << ",\"dof_records\":" << e.dof_records
        << ",\"dof_passes\":" << e.dof_passes
        << ",\"dof_positive_radius_sources\":" << e.dof_positive_radius_sources
        << ",\"dof_sort_work\":" << e.dof_sort_work
        << ",\"dof_candidate_scan_checks\":" << e.dof_candidate_scan_checks
        << ",\"dof_candidate_checks\":" << e.dof_candidate_checks
        << ",\"dof_contributing_pairs\":" << e.dof_contributing_pairs
        << ",\"dof_changed_pixels\":" << e.dof_changed_pixels
        << ",\"dof_maximum_radius\":" << e.dof_maximum_radius
        << ",\"dof_dispatches\":" << e.dof_dispatches
        << ",\"znormal_records\":" << e.znormal_records
        << ",\"znormal_foreground\":" << e.znormal_foreground
        << ",\"znormal_changed\":" << e.znormal_changed
        << ",\"znormal_neighbors\":" << e.znormal_neighbors
        << ",\"znormal_dispatches\":" << e.znormal_dispatches
        << ",\"camera_records\":" << e.camera_records
        << ",\"camera_cut_starts\":" << e.camera_cut_starts
        << ",\"camera_cut_misses\":" << e.camera_cut_misses
        << ",\"camera_planar_rays\":" << e.camera_planar_rays
        << ",\"camera_spherical_rays\":" << e.camera_spherical_rays
        << ",\"camera_full_width\":" << e.camera_full_width
        << ",\"camera_full_height\":" << e.camera_full_height
        << ",\"inside_records\":" << e.inside_records
        << ",\"inside_attempts\":" << e.inside_attempts
        << ",\"inside_retries\":" << e.inside_retries
        << ",\"inside_initial_hits\":" << e.inside_initial_hits
        << ",\"inside_interior_surfaces\":" << e.inside_interior_surfaces
        << ",\"inside_smooth_calls\":" << e.inside_smooth_calls
        << ",\"reflection_inside_records\":" << e.reflection_inside_records
        << ",\"reflection_inside_selected_pixels\":" << e.reflection_inside_selected_pixels
        << ",\"reflection_inside_ao_rays\":" << e.reflection_inside_ao_rays
        << ",\"reflection_inside_absorption_bounces\":" << e.reflection_inside_absorption_bounces
        << ",\"reflection_inside_initial_primary\":" << e.reflection_inside_initial_primary
        << ",\"reflection_inside_cut_ao_skips\":" << e.reflection_inside_cut_ao_skips
        << ",\"reflection_inside_cut_shadow_limits\":" << e.reflection_inside_cut_shadow_limits
        << ",\"camera_left\":" << e.camera_left
        << ",\"camera_top\":" << e.camera_top
        << ",\"inside_initial_policy\":" << e.inside_initial_policy
        << ",\"inside_in_and_out\":" << e.inside_in_and_out
        << ",\"color_abi_version\":" << e.color_abi_version
        << ",\"surface_color_version\":" << e.surface_color_version
        << ",\"surface_color_option\":" << e.surface_color_option
        << ",\"orbit_color_records\":" << e.orbit_color_records
        << ",\"numerical_gradient_policy\":" << e.numerical_gradient_policy
        << ",\"primary_gradient_range_clamps\":" << e.primary_gradient_range_clamps
        << ",\"reflection_gradient_range_clamps\":" << e.reflection_gradient_range_clamps
        << ",\"primary_state_contract_version\":" << e.primary_state_contract_version
        << ",\"primary_state_chain\":" << e.primary_state_chain
        << ",\"primary_state_reason\":" << e.primary_state_reason
        << ",\"primary_logical_tasks\":" << e.primary_logical_tasks
        << ",\"primary_dispatches\":" << e.primary_dispatches
        << ",\"primary_formula_reads\":" << e.primary_formula_reads
        << ",\"primary_orbit_initialized\":" << e.primary_orbit_initialized
        << ",\"primary_scene_inputs\":" << e.primary_scene_inputs
        << ",\"primary_caller_controls\":" << e.primary_caller_controls
        << ",\"primary_driver_may_write\":" << e.primary_driver_may_write
        << ",\"soft_shadow_state_contract_version\":" << e.soft_shadow_state_contract_version
        << ",\"soft_shadow_state_chain\":" << e.soft_shadow_state_chain
        << ",\"soft_shadow_state_reason\":" << e.soft_shadow_state_reason
        << ",\"shadow_logical_tasks\":" << e.shadow_logical_tasks
        << ",\"shadow_dispatches\":" << e.shadow_dispatches
        << ",\"soft_shadow_formula_reads\":" << e.soft_shadow_formula_reads
        << ",\"soft_shadow_orbit_initialized\":" << e.soft_shadow_orbit_initialized
        << ",\"soft_shadow_scene_inputs\":" << e.soft_shadow_scene_inputs
        << ",\"soft_shadow_caller_controls\":" << e.soft_shadow_caller_controls
        << ",\"soft_shadow_driver_may_write\":" << e.soft_shadow_driver_may_write
        << ",\"volume_map_cells\":" << e.volume_map_cells
        << ",\"volume_map_dispatches\":" << e.volume_map_dispatches
        << ",\"volume_map_range_exit_cells\":" << e.volume_map_range_exit_cells
        << ",\"color_flags\":" << e.color_flags
        << ",\"ao_record_version\":" << e.ao_record_version
        << ",\"ao_enabled\":" << (e.ao_enabled?"true":"false")
        << ",\"ao_algorithm\":" << e.ao_algorithm
        << ",\"ao_quality\":" << e.ao_quality
        << ",\"ao_requested_repeats\":" << e.ao_requested_repeats
        << ",\"ao_records\":" << e.ao_records
        << ",\"ao_active\":" << e.ao_active
        << ",\"ao_changed\":" << e.ao_changed
        << ",\"ao_samples\":" << e.ao_samples
        << ",\"ao_depth_reads\":" << e.ao_depth_reads
        << ",\"ao_border_rejects\":" << e.ao_border_rejects
        << ",\"ao_rng_advances\":" << e.ao_rng_advances
        << ",\"ao_seed_planning_draws\":" << e.ao_seed_planning_draws
        << ",\"ao_seed_shortcuts\":" << e.ao_seed_shortcuts
        << ",\"ao_levels\":" << e.ao_levels
        << ",\"ao_repeats\":" << e.ao_repeats
        << ",\"ao_de_queries\":" << e.ao_de_queries
        << ",\"ao_de_steps\":" << e.ao_de_steps
        << ",\"ao_cut_skips\":" << e.ao_cut_skips
        << ",\"ao_rays\":" << e.ao_rays
        << ",\"ao_refinements\":" << e.ao_refinements
        << ",\"ao_formula_work\":" << e.ao_formula_work
        << ",\"ao_dispatches\":" << e.ao_dispatches
        << ",\"ao_command_counter_version\":" << e.ao_command_counter_version
        << ",\"ao_commands\":" << e.ao_commands
        << ",\"ao_certificate_version\":" << e.ao_certificate_version
        << ",\"ao_certificate_levels\":" << e.ao_certificate_levels
        << ",\"ao_certificate_pixels\":" << e.ao_certificate_pixels
        << ",\"ao_certificate_known\":" << e.ao_certificate_known
        << ",\"ao_certificate_used\":" << e.ao_certificate_used
        << ",\"ao_gpu_seconds\":" << e.ao_gpu_seconds
        << ",\"ao_max_command_seconds\":" << e.ao_max_command_seconds
        << ",\"ao_preparation_seconds\":" << e.ao_preparation_seconds
        << ",\"faults\":" << e.faults
        << ",\"nonfinite\":" << e.nonfinite
        << ",\"work_exhausted\":" << e.work_exhausted << "}}";
}

void write_sample(std::ostream& out, const Sample& s) {
    out << "{\"phase\":" << quoted(s.phase) << ",\"index\":" << s.index
        << ",\"image\":" << quoted(s.image) << ",\"state\":" << quoted(s.state)
        << ",\"error\":" << quoted(s.error) << ",\"width\":" << s.width
        << ",\"height\":" << s.height << ",\"threads\":" << s.threads
        << ",\"precision\":" << quoted(mb3d::PrecisionModeName(s.profile.precision))
        << ",\"backend\":";
    write_backend(out, s.backend);
    out << ",\"exact_rgb_matches_cold\":";
    if (s.compared) out << (s.exact_rgb ? "true" : "false");
    else out << "null";
    out << ",\"load_ms\":" << s.load_ms
        << ",\"preparation_ms\":" << s.profile.stages[static_cast<size_t>(mb3d::RenderProfileStage::Preparation)].seconds * 1000.0
        << ",\"render_ms\":" << s.render_ms
        << ",\"readback_ms\":" << s.readback_ms
        << ",\"render_and_readback_ms\":" << s.render_and_readback_ms
        << ",\"png_encode_write_ms\":" << s.png_encode_write_ms
        << ",\"end_to_end_ms\":" << s.end_to_end_ms
        << ",\"deadline\":{\"limit_seconds\":" << s.deadline_limit
        << ",\"seconds\":" << s.deadline_seconds << ",\"completed\":"
        << (s.deadline_seconds>0?"true":"false") << '}';
    if (s.phase == "cold")
        out << ",\"cold_total_ms\":" << s.cold_total_ms
            << ",\"cold_end_to_end_ms\":" << s.cold_end_to_end_ms;
    out << ",\"render_profile_total_ms\":" << s.profile.total_seconds * 1000.0
        << ",\"stages\": {";
    for (size_t i = 0; i < s.profile.stages.size(); ++i) {
        if (i) out << ',';
        const auto& stage = s.profile.stages[i];
        out << quoted(mb3d::RenderProfileStageName(static_cast<mb3d::RenderProfileStage>(i)))
            << ":{\"state\":" << quoted(stage.state) << ",\"seconds\":" << stage.seconds << '}';
    }
    out << ",\"load\":{\"state\":" << quoted(s.phase == "cold" ? "completed" : "not_run")
        << ",\"seconds\":" << s.load_ms / 1000.0 << '}'
        << ",\"final_surface\":{\"state\":" << quoted(s.final_surface.state)
        << ",\"seconds\":" << s.final_surface.seconds << '}'
        << ",\"save\":{\"state\":" << quoted(s.save.state)
        << ",\"seconds\":" << s.save.seconds << "}}}";
}

void write_report(const Report& report) {
    const auto& opts = report.options;
    const auto& h = report.loaded.header;
    std::ostringstream out;
    out.imbue(std::locale::classic());
    out << std::setprecision(17)
        << "{\n\"schema\":\"mb3d.backend-render-benchmark.v1\",\"backend\":"
        << quoted(mb3d::RenderBackendName(opts.backend.requested)) << ",\"status\":"
        << quoted(report.status) << ",\"error\":" << quoted(report.error)
        << ",\"scene\":" << quoted(opts.scene.string())
        << ",\"resource_root\":" << quoted(opts.resource_root.string())
#if defined(__aarch64__) || defined(_M_ARM64)
        << ",\"architecture\":\"arm64\""
#elif defined(__x86_64__) || defined(_M_X64)
        << ",\"architecture\":\"x86_64\""
#else
        << ",\"architecture\":\"other\""
#endif
        << ",\n\"settings\":{\"width\":" << opts.width << ",\"height\":" << opts.height
        << ",\"threads\":" << opts.threads << ",\"warmups\":" << opts.warmups
        << ",\"repeats\":" << opts.repeats << '}'
        << ",\"execution\":{\"deterministic\":" << (opts.scene_rng ? "false" : "true")
        << ",\"rng_mode\":" << quoted(opts.scene_rng ? "scene" : "parity")
        << ",\"profile_timing\":true,\"work_control\":\"none\",\"precision\":"
        << quoted(mb3d::PrecisionModeName(opts.precision)) << '}'
        << ",\"effective_rng\":{\"primary_first_step_jitter\":"
        << (opts.scene_rng && (h.iOptions & 1) ? "true" : "false")
        << ",\"ao_first_step_jitter\":"
        << (opts.scene_rng && (h.bCalcAmbShadowAutomatic & 128) ? "true" : "false") << '}'
        << ",\n\"timing_contract\":{"
           "\"cold\":\"first RenderWithBackend in a fresh process; OS/driver caches are not flushed\","
           "\"warm\":\"same process and loaded scene; every complete RenderWithBackend creates/releases fresh job resources, GPU contexts/pipelines/buffers or CPU workers\","
           "\"render_ms\":\"main application RenderWithBackend including backend selection, preparation, rendering, GPU readback and resource teardown\","
           "\"readback_ms\":\"BuildFinalRGBSurface CPU copy at scale 1 and crop 0; Metal device readback is already inside render_ms\","
           "\"cold_total_ms\":\"scene load through first final surface; excludes PNG encode/write\","
           "\"cold_end_to_end_ms\":\"scene load through first PNG encode/write\","
           "\"end_to_end_ms\":\"header/addon copy through PNG encode/write; cold includes load\","
           "\"validation\":\"RGB byte comparisons and report serialization happen after timing\"}"
        << ",\n\"setup_ms\":{\"load\":" << report.load_ms
        << ",\"ordinary_input_check\":" << report.setup_ms << '}';
    out << ",\n\"loaded_features\":{\"load_success\":" << (report.loaded.success ? "true" : "false")
        << ",\"source_mand_id\":" << report.loaded.source_mand_id
        << ",\"width\":" << h.Width << ",\"height\":" << h.Height
        << ",\"iterations\":" << h.Iterations
        << ",\"bCalc3D\":" << unsigned(h.bCalc3D) << ",\"effective_bCalc3D\":1"
        << ",\"iOptions\":" << h.iOptions
        << ",\"bCalculateHardShadow\":" << unsigned(h.bCalculateHardShadow)
        << ",\"bCalc1HSsoft\":" << unsigned(h.bCalc1HSsoft)
        << ",\"bNormalsOnDE\":" << unsigned(h.bNormalsOnDE)
        << ",\"bCalcAmbShadowAutomatic\":" << unsigned(h.bCalcAmbShadowAutomatic)
        << ",\"bCalcSRautomatic\":" << unsigned(h.bCalcSRautomatic)
        << ",\"SRamount\":" << h.SRamount << ",\"SRreflectioncount\":" << unsigned(h.SRreflectioncount)
        << ",\"bCalcDOFtype\":" << unsigned(h.bCalcDOFtype)
        << ",\"byCalcNsOnZBufAuto\":" << unsigned(h.byCalcNsOnZBufAuto)
        << ",\"bVolLightNr\":" << unsigned(h.bVolLightNr)
        << ",\"bStereoMode\":" << unsigned(h.bStereoMode)
        << ",\"bPlanarOptic\":" << unsigned(h.bPlanarOptic)
        << ",\"bDFogIt\":" << unsigned(h.bDFogIt)
        << ",\"TilingOptions\":" << h.TilingOptions
        << ",\"bIsJulia\":" << unsigned(h.bIsJulia)
        << ",\"MCoptions\":" << unsigned(h.MCoptions)
        << ",\"hybrid_mode\":" << unsigned(report.loaded.addon.bOptions1 & 3)
        << ",\"formulas\":[";
    for (int i = 0; i < 6; ++i) {
        if (i) out << ',';
        const auto& f = report.loaded.addon.Formulas[i];
        out << "{\"slot\":" << i << ",\"id\":" << f.iFnr
            << ",\"name\":" << quoted(formula_name(f)) << ",\"iterations\":" << f.iItCount << '}';
    }
    out << "]},\n\"cold\":";
    write_sample(out, report.cold);
    out << ",\n\"warmups\":[";
    for (size_t i = 0; i < report.warmups.size(); ++i) {
        if (i) out << ',';
        write_sample(out, report.warmups[i]);
    }
    out << "],\n\"samples\":[";
    for (size_t i = 0; i < report.samples.size(); ++i) {
        if (i) out << ',';
        write_sample(out, report.samples[i]);
    }
    out << "]\n}\n";
    const std::string json = out.str();
    const auto path = opts.output / "report.json";
    std::FILE* file = std::fopen(path.string().c_str(), "wbx");
    if (!file) throw std::runtime_error("Could not exclusively create report.json");
    const bool written = std::fwrite(json.data(), 1, json.size(), file) == json.size();
    const int closed = std::fclose(file);
    if (!written || closed != 0) throw std::runtime_error("Could not write report.json");
}

std::string image_name(const std::string& phase, int index) {
    if (phase == "cold") return "cold.png";
    std::ostringstream out;
    out << phase << '-' << std::setw(3) << std::setfill('0') << index << ".png";
    return out.str();
}

void run_sample(Report& report, Sample& sample, const mb3d::RenderOptions& options,
                const std::string& phase, int index, Clock::time_point cold_start,
                std::vector<uint8_t>& reference, int& reference_width, int& reference_height,
                FrameDeadline& deadline) {
    sample.phase = phase;
    sample.index = index;
    sample.image = image_name(phase, index);
    sample.state = "interrupted";
    sample.load_ms = phase == "cold" ? report.load_ms : 0;
    const fs::path path = report.options.output / sample.image;
    const auto begin = deadline.started;
    sample.deadline_limit=report.options.frame_timeout_seconds;
    // Render may mutate both records. Never carry those writes into another job.
    auto header = report.loaded.header;
    auto addon = report.loaded.addon;
    header.bCalc3D = 1; // Same full-3D headless convention as mb3d_render.
    const auto render_begin = Clock::now();
    auto result = mb3d::RenderWithBackend(header, addon, options, report.options.backend);
    auto& rendered = result.render;
    const auto render_end = Clock::now();
    sample.render_ms = milliseconds(render_begin, render_end);
    sample.profile = rendered.profile;
    sample.backend = result.backend;
    sample.width = rendered.width;
    sample.height = rendered.height;
    sample.threads = rendered.profile.workers;
    if (!rendered.success) {
        sample.error = rendered.error;
        throw std::runtime_error("Render failed: " + rendered.error);
    }
    if (sample.backend.requested != report.options.backend.requested ||
        sample.backend.effective != report.options.backend.requested || sample.backend.fallback)
        throw std::runtime_error("Benchmark refuses a backend fallback or selection mismatch");
    if (sample.profile.precision != mb3d::PrecisionMode::Native)
        throw std::runtime_error("Benchmark refuses non-Native arithmetic");
    sample.final_surface.state = "interrupted";
    mb3d::FinalRGBSurface surface;
    const auto surface_begin = Clock::now();
    const bool surface_ok = mb3d::BuildFinalRGBSurface(rendered.image.data(), rendered.width,
                                                      rendered.height, 1, 0, 0, surface);
    const auto surface_end = Clock::now();
    sample.readback_ms = milliseconds(surface_begin, surface_end);
    sample.final_surface.seconds = sample.readback_ms / 1000.0;
    sample.render_and_readback_ms = milliseconds(render_begin, surface_end);
    if (!surface_ok) throw std::runtime_error("BuildFinalRGBSurface failed");
    sample.final_surface.state = "completed";
    sample.save.state = "interrupted";
    const auto save_begin = Clock::now();
    const bool saved = mb3d::WritePNG(path.string(), surface.pixels.data(), surface.width, surface.height, 3);
    const auto save_end = Clock::now();
    sample.deadline_seconds=deadline.complete(save_end);
    sample.png_encode_write_ms = milliseconds(save_begin, save_end);
    sample.save.seconds = sample.png_encode_write_ms / 1000.0;
    sample.end_to_end_ms = milliseconds(begin, save_end);
    if (phase == "cold") {
        sample.cold_total_ms = milliseconds(cold_start, surface_end);
        sample.cold_end_to_end_ms = milliseconds(cold_start, save_end);
        sample.end_to_end_ms = sample.cold_end_to_end_ms;
    }
    if (!saved) throw std::runtime_error("PNG encode/write failed");
    sample.save.state = "completed";
    // No byte comparisons, hashing, test dispatch or independent oracle enters
    // any timed region. Retain only the cold RGB, not all repeated frame buffers.
    if (phase == "cold") {
        reference = std::move(surface.pixels);
        reference_width = surface.width;
        reference_height = surface.height;
        sample.exact_rgb = true;
    } else {
        sample.exact_rgb = surface.width == reference_width && surface.height == reference_height &&
                           surface.pixels == reference;
    }
    sample.compared = true;
    sample.state = "completed";
    if (!sample.exact_rgb) throw std::runtime_error("Repeated RGB bytes differ from cold frame");
}
} // namespace

int main(int argc, char** argv) {
    if (argc == 2 && std::strcmp(argv[1], "--help") == 0) {
        std::puts("mb3d_backend_render_benchmark --scene FILE --output NEW_DIR --width N --height N "
                  "--threads N --backend cpu|metal --precision native [--metal-library FILE] "
                  "[--warmups N>=0] [--repeats N>=3] [--resource-root DIR]");
        return 0;
    }
    Report report;
    bool created = false;
    try {
        report.options = parse(argc, argv);
        if (!fs::create_directory(report.options.output))
            throw std::invalid_argument("Output directory must be new");
        created = true;
        mb3d::RenderOptions options;
        options.precision = report.options.precision;
        options.width_override = report.options.width;
        options.height_override = report.options.height;
        options.thread_count = report.options.threads;
        options.resource_root = report.options.resource_root.string();
        options.deterministic_mode = !report.options.scene_rng;
        options.profile_timing = true;
        report.warmups.reserve(static_cast<size_t>(report.options.warmups));
        report.samples.reserve(static_cast<size_t>(report.options.repeats));
        FrameDeadline cold_deadline(report.options.frame_timeout_seconds);
        const auto cold_start = cold_deadline.started;
        report.loaded = mb3d::LoadM3PFile(report.options.scene.string());
        const auto loaded_at = Clock::now();
        report.load_ms = milliseconds(cold_start, loaded_at);
        if (!report.loaded.success) throw std::runtime_error("Scene load failed: " + report.loaded.error);
        reject_deferred(report.loaded);
        report.setup_ms = milliseconds(loaded_at, Clock::now());
        std::vector<uint8_t> reference;
        int reference_width = 0, reference_height = 0;
        run_sample(report, report.cold, options, "cold", 0, cold_start,
                   reference, reference_width, reference_height,cold_deadline);
        for (int i = 0; i < report.options.warmups; ++i) {
            report.warmups.emplace_back();
            FrameDeadline deadline(report.options.frame_timeout_seconds);
            run_sample(report, report.warmups.back(), options, "warmup", i, cold_start,
                       reference, reference_width, reference_height,deadline);
        }
        for (int i = 0; i < report.options.repeats; ++i) {
            report.samples.emplace_back();
            FrameDeadline deadline(report.options.frame_timeout_seconds);
            run_sample(report, report.samples.back(), options, "warm", i, cold_start,
                       reference, reference_width, reference_height,deadline);
        }
        report.status = "COMPLETE";
        write_report(report);
        std::printf("Backend benchmark complete: %s\n", (report.options.output / "report.json").string().c_str());
        return 0;
    } catch (const std::exception& error) {
        report.status = "FAILED";
        report.error = error.what();
        std::fprintf(stderr, "Backend benchmark failed: %s\n", error.what());
        if (created && !fs::exists(report.options.output / "report.json")) {
            try { write_report(report); }
            catch (const std::exception& write_error) {
                std::fprintf(stderr, "Failure report could not be saved: %s\n", write_error.what());
            }
        }
        return 1;
    }
}
