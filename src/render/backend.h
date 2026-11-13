#pragma once
#include "core/renderer.h"

namespace mb3d {
namespace native_metal { struct RenderSession; }

enum class RenderBackend { CPU, Metal, Auto };
const char* RenderBackendName(RenderBackend backend);
bool ParseRenderBackend(const std::string& text, RenderBackend& backend);

struct BackendOptions {
    RenderBackend requested = RenderBackend::CPU;
    // An explicit offline library path; the CLI resolves its adjacent default.
    std::string metal_library;
    // Interactive clients may forbid whole-frame CPU fallback and prefer the
    // general GPU engine, which supports rays beginning inside the surface.
    bool allow_cpu_fallback = true;
    bool prefer_general_metal = false;
    // Optional device state kept between renders (general Metal engine only).
    // Null renders statelessly, exactly as before.
    native_metal::RenderSession* metal_session = nullptr;
    // Control for a CPU frame rendered as the fallback of a Metal request, so
    // that frame can be cancelled too. The Metal attempt does not use it.
    RenderControlHandle cpu_fallback_control;
};

// Counts read from validated GPU records, not inferred from scene flags.
// They describe the admitted opaque Native reflection pass only.
struct ReflectionActivity {
    bool enabled = false;
    uint64_t active_pixels = 0, rays = 0, hits = 0, misses = 0;
    uint64_t changed_pixels = 0, faults = 0, max_bounces = 0;
    uint64_t ray_steps = 0, primary_skipped = 0;
    uint64_t ao_rays = 0, ao_steps = 0, shadow_rays = 0, shadow_steps = 0;
    double max_command_seconds = 0;
};

// Host validation counts actual returned records. A completed render alone is
// not evidence of finite GPU work; validation and the shader profile are explicit.
struct MetalExecutionActivity {
    bool validated = false;
    std::string engine = "none";
    uint32_t execution_plan_version=0;
    std::string execution_plan;
    uint32_t profile_kind = 0, storage_bytes = 0, arithmetic_bits = 0;
    uint64_t primary_records = 0, normal_records = 0, surface_records = 0;
    uint64_t shadow_records = 0, color_records = 0, reflection_records = 0;
    uint64_t faults = 0, nonfinite = 0, work_exhausted = 0;
    uint64_t distance_calls = 0, formula_work = 0;
    // Completed GPU volume-map dispatches and individually validated cell
    // records. These are measured work, not a configured volume-light flag.
    uint64_t volume_map_cells = 0, volume_map_dispatches = 0, volume_map_range_exit_cells = 0;
    // Persistent pipeline archive: specialized pipelines loaded or compiled.
    uint64_t pipeline_archive_hits = 0, pipeline_archive_misses = 0;
    // GPU execution time and count of the commands the render waited for.
    double gpu_seconds = 0;
    uint64_t gpu_commands = 0;
    uint32_t smoothing_record_version = 0, smoothing_policy = 0;
    uint64_t smoothing_calls = 0, smoothing_recovered = 0, smoothing_stagnant = 0, smoothing_invalid = 0;
    // Primary and secondary normal samples come from distinct GPU records.
    uint64_t normal_samples = 0, normal_de_calls = 0, normal_orbit_calls = 0;
    uint64_t normal_evaluations = 0, primary_rough_pixels = 0;
    uint64_t shadow_rays = 0, shadow_hits = 0, shadow_misses = 0, shadow_steps = 0, shadow_changed = 0;
    uint32_t soft_shadow_record_version = 0;
    uint64_t soft_shadow_records = 0, soft_shadow_partial_pixels = 0;
    uint64_t soft_shadow_dark_pixels = 0, soft_shadow_light_pixels = 0;
    uint64_t dof_records = 0, dof_passes = 0, dof_positive_radius_sources = 0;
    uint64_t dof_sort_work = 0, dof_candidate_scan_checks = 0, dof_candidate_checks = 0;
    uint64_t dof_contributing_pairs = 0, dof_changed_pixels = 0, dof_maximum_radius = 0, dof_dispatches = 0;
    uint64_t znormal_records = 0, znormal_foreground = 0, znormal_changed = 0;
    uint64_t znormal_neighbors = 0, znormal_dispatches = 0;
    uint64_t camera_records = 0;
    uint64_t camera_cut_starts = 0;
    uint64_t camera_cut_misses = 0;
    uint64_t camera_planar_rays = 0;
    uint64_t camera_spherical_rays = 0;
    uint64_t camera_full_width = 0;
    uint64_t camera_full_height = 0;
    uint64_t inside_records = 0;
    uint64_t inside_attempts = 0;
    uint64_t inside_retries = 0;
    uint64_t inside_initial_hits = 0;
    uint64_t inside_interior_surfaces = 0;
    uint64_t inside_smooth_calls = 0;
    uint64_t reflection_inside_records = 0;
    uint64_t reflection_inside_selected_pixels = 0;
    uint64_t reflection_inside_ao_rays = 0;
    uint64_t reflection_inside_absorption_bounces = 0;
    uint64_t reflection_inside_initial_primary = 0;
    uint64_t reflection_inside_cut_ao_skips = 0;
    uint64_t reflection_inside_cut_shadow_limits = 0;
    int32_t camera_left = 0;
    int32_t camera_top = 0;
    int32_t inside_initial_policy = 0;
    uint32_t inside_in_and_out = 0;
    uint32_t color_abi_version = 0;
    uint32_t surface_color_version = 0, surface_color_option = 0;
    uint64_t orbit_color_records = 0;
    uint32_t numerical_gradient_policy = 0;
    uint64_t primary_gradient_range_clamps = 0, reflection_gradient_range_clamps = 0;
    // FP32 range exits: orbits completed as escaped (all stages), pixels
    // with one in primary/normal/shadow/reflection work, and DE AO pixels.
    uint64_t fp32_range_exits = 0, fp32_range_exit_pixels = 0, ao_fp32_range_exit_pixels = 0;
    // Policy observation survives an early whole-frame fallback. These are
    // discarded-stage counts, not completed GPU frame/qualification counts.
    std::string fp32_range_gate_stage = "none";
    uint64_t fp32_range_gate_units = 0, fp32_range_gate_exceeded = 0;
    uint32_t fp32_range_gate_percent = 1;
    // dIFS orbits that returned lanes carried from an earlier call, and pixels
    // with one in primary/normal/shadow/reflection work.
    uint64_t difs_carried_reads = 0, difs_carried_pixels = 0;
    // Pixel ownership by the conditional dIFS contract (no carried read), and
    // the carried reads of a discarded pixel frame re-rendered in CPU order.
    bool difs_conditional_pixels = false;
    uint64_t difs_rerun_carried_reads = 0, difs_rerun_carried_pixels = 0;
    // DE AO alone: carried reads of the returned pass (ordered tasks may count
    // them), pixel ownership by the conditional contract, and the carried
    // reads of a discarded pixel pass that the ordered tasks replaced.
    uint64_t ao_difs_carried_reads = 0, ao_difs_carried_pixels = 0;
    bool ao_difs_conditional_pixels = false;
    uint64_t ao_difs_rerun_carried_reads = 0, ao_difs_rerun_carried_pixels = 0;
    uint32_t soft_shadow_state_contract_version=0,soft_shadow_state_chain=0,soft_shadow_state_reason=0;
    // WholeRow hard-shadow pixels traced from a fresh cache, traced again from the
    // ordered row cache, and frames that fell back to ordered rows.
    uint64_t shadow_speculative_pixels=0,shadow_recomputed_pixels=0,shadow_speculation_fallbacks=0;
    uint64_t shadow_logical_tasks=0,shadow_dispatches=0,soft_shadow_formula_reads=0,
        soft_shadow_orbit_initialized=0,soft_shadow_scene_inputs=0,soft_shadow_caller_controls=0,soft_shadow_driver_may_write=0;
    uint32_t primary_state_contract_version=0,primary_state_chain=0,primary_state_reason=0;
    uint32_t primary_logical_tasks=0,primary_dispatches=0;
    // Primary first-step jitter: NativePrimaryRandomMode (0 off, 2 seeded
    // pixels, 3 carried tasks), probe commands, and seeded pixel advances.
    uint32_t primary_random_mode=0,primary_random_probe_dispatches=0;
    uint64_t primary_seed_scan_commands=0,ao_seed_generation_commands=0;
    uint64_t primary_random_advances=0;
    uint64_t primary_formula_reads=0,primary_orbit_initialized=0,primary_scene_inputs=0;
    uint64_t primary_caller_controls=0,primary_driver_may_write=0,primary_contained=0;
    uint64_t primary_entry_equivalent=0;
    uint32_t color_flags = 0;
    // Version0 means this engine does not expose the general AO record ABI;
    // only version1 makes enabled/algorithm and the following counts meaningful.
    uint32_t ao_record_version = 0;
    bool ao_enabled = false;
    uint32_t ao_algorithm = 0, ao_quality = 0, ao_requested_repeats = 0;
    uint64_t ao_records = 0;
    uint64_t ao_active = 0;
    uint64_t ao_changed = 0;
    uint64_t ao_samples = 0;
    uint64_t ao_depth_reads = 0;
    uint64_t ao_border_rejects = 0;
    uint64_t ao_rng_advances = 0;
    uint64_t ao_seed_planning_draws = 0;
    uint64_t ao_seed_shortcuts = 0;
    uint64_t ao_levels = 0;
    uint64_t ao_repeats = 0;
    uint64_t ao_de_queries = 0;
    uint64_t ao_de_steps = 0;
    uint64_t ao_cut_skips = 0;
    uint64_t ao_rays = 0;
    uint64_t ao_refinements = 0;
    uint64_t ao_formula_work = 0;
    uint64_t ao_dispatches = 0;
    uint32_t ao_command_counter_version = 0;
    uint64_t ao_commands = 0;
    uint32_t ao_certificate_version = 0;
    uint64_t ao_certificate_levels = 0, ao_certificate_pixels = 0;
    uint64_t ao_certificate_known = 0, ao_certificate_used = 0;
    double ao_gpu_seconds = 0, ao_max_command_seconds = 0, ao_preparation_seconds = 0;

};

struct BackendSelection {
    RenderBackend requested = RenderBackend::CPU;
    RenderBackend effective = RenderBackend::CPU;
    bool fallback = false;
    std::string fallback_code, fallback_reason;
    std::string device, library;
    double selection_seconds = 0;
    ReflectionActivity reflections;
    MetalExecutionActivity execution;
};

struct BackendRenderResult {
    RenderResult render;
    BackendSelection backend;
};

// Ordinary full-render entry point for applications that opt into native GPU
// rendering. CPU preserves the requested precision. Metal and Auto require
// Native; unsupported jobs/devices fall back before any render dispatch.
// Device faults and invalid libraries fail instead of publishing a CPU image.
// Render()/PaintFromSiLight() and the existing C API remain explicit CPU APIs.
BackendRenderResult RenderWithBackend(TMandHeader10& header, THeaderCustomAddon& addon,
                                     const RenderOptions& options,
                                     const BackendOptions& backend);

} // namespace mb3d
