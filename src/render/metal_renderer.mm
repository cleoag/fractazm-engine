#include "src/render/native_metal/restricted_primary_admission.h"
#include "render/metal_renderer.h"
#include "comparison/metal_exact/render_dispatch.h"
#include "comparison/metal_exact/native_reflection_host.h"
#include "comparison/metal_exact/scene_host_internal.h"
#include "render/native_metal/resource_requirements.h"
#include "render/native_metal/restricted_execution_policy.h"
#include <algorithm>
#include <bit>
#include <chrono>
#include <filesystem>
#include <memory>
#include <thread>

namespace mb3d {
namespace {
using namespace metal_exact;
namespace gpu = metal_exact::gpu;

// Remains attached to the outer result while exceptions unwind. A failed
// command therefore preserves completed stages and marks the active one as
// interrupted instead of publishing a fictitious all-not-run profile.
struct ProfileRecorder {
    RenderProfile& profile;
    gpu::Clock::time_point start, entered;
    int active = -1;
    explicit ProfileRecorder(RenderProfile& p, gpu::Clock::time_point t) : profile(p), start(t) {}
    void begin(RenderProfileStage stage) {
        NoteRenderStage(stage);
        if (!profile.enabled) return;
        active = int(stage); entered = gpu::Clock::now();
        profile.stages[size_t(active)].state = "interrupted";
    }
    void end(bool completed) {
        if (!profile.enabled || active < 0) return;
        auto& timing = profile.stages[size_t(active)];
        timing.seconds += gpu::elapsed(entered) / 1000;
        timing.state = completed ? "completed" : "interrupted";
        active = -1;
    }
    ~ProfileRecorder() {
        end(false);
        if (profile.enabled) profile.total_seconds = gpu::elapsed(start) / 1000;
    }
};

bool UnsupportedOptions(const RenderOptions& o) {
    return o.precision != PrecisionMode::Native || o.mc_samples != 0 || o.control ||
        o.lattice_stride != 1 || o.lattice_x || o.lattice_y ||
        o.native_metal_smoothing_policy!=1 ||
        o.no_shadows || o.diag_only ||
        !o.diag_points.empty() || !o.diag_dump_path.empty() || !o.ray_setup_dump_path.empty() ||
        !o.primary_trace_dump_path.empty() || !o.parity_trace_dump_path.empty() ||
        !o.parity_trace_start_stage.empty() || !o.parity_trace_end_stage.empty() ||
        !o.parity_trace_detail.empty() || o.parity_trace_schema_mode || o.parity_trace_stage_state;
}

struct NeedsGeneralInitialInside {};
void CheckInitialInside(const gpu::RenderStageBuffers& b, size_t count) {
    if(native_metal::RequiresGeneralInitialInside(static_cast<const ExactPixel*>(b.pixels.contents),
        static_cast<const ExactNormalSamples*>(b.normalSamples.contents),count))throw NeedsGeneralInitialInside{};
}

void ValidateFrame(const gpu::RenderStageBuffers& b, size_t count, bool shadows,
                   MetalExecutionActivity& activity) {
    const auto* primary = static_cast<const ExactPixel*>(b.pixels.contents);
    const auto* surface = static_cast<const ExactSurface*>(b.surface.contents);
    const auto* shaded = static_cast<const ExactSurface*>(b.shaded.contents);
    const auto* colors = static_cast<const ExactColorPixel*>(b.rgb.contents);
    const auto* normals = static_cast<const ExactNormalSamples*>(b.normalSamples.contents);
    const auto* rays = shadows ? static_cast<const ExactShadowRay*>(b.rays.contents) : nullptr;
    const auto* traces = shadows ? static_cast<const ExactShadowTrace*>(b.traces.contents) : nullptr;
    std::string firstFault;
    for (size_t i = 0; i < count; ++i) {
        const auto& p = primary[i];
        bool invalid = p.fault || p.reserved_tail || normals[i].reserved;
        bool nonfinite = false;
        auto finite = [&](uint64_t value) { nonfinite |= !std::isfinite(std::bit_cast<double>(value)); };
        for (unsigned axis = 0; axis < 3; ++axis) {
            finite(p.ray[axis]); finite(p.start[axis]); finite(p.position[axis]); finite(p.raw_normal[axis]);
        }
        for (auto value : {p.depth, p.axial_depth, p.final_de, p.orbit_trap}) finite(value);
        for (auto value : {p.step_count, p.smooth_iterations, p.final_stop})
            nonfinite |= !std::isfinite(std::bit_cast<float>(value));
        for (const auto* s : {&surface[i], &shaded[i]})
            invalid |= s->fault || s->reserved[0] || s->reserved[1] || s->reserved[2];
        invalid |= colors[i].fault || colors[i].reserved0 || colors[i].reserved1;
        if (shadows) {
            invalid |= rays[i].fault || rays[i].reserved[0] || rays[i].reserved[1];
            invalid |= traces[i].fault || traces[i].reserved[0] || traces[i].reserved[1] || traces[i].reserved[2];
            // terminal_fault may be suppressed by the ordered shadow shortcut.
            // Any unsuppressed fault is propagated into final shaded.fault.
        }
        invalid |= nonfinite;
        activity.primary_records++;
        activity.normal_records++;
        activity.surface_records += 2;
        activity.color_records++;
        if (shadows) activity.shadow_records += 2;
        activity.nonfinite += nonfinite || ((p.fault | shaded[i].fault | colors[i].fault) & 4u);
        activity.work_exhausted += ((p.fault | shaded[i].fault) & 2u) != 0;
        activity.faults += invalid;
        if (invalid && firstFault.empty()) firstFault = std::to_string(i) +
            " primary=" + std::to_string(p.fault) + " surface=" + std::to_string(surface[i].fault) +
            " shaded=" + std::to_string(shaded[i].fault) + " color=" + std::to_string(colors[i].fault) +
            " start_inside=" + std::to_string(p.start_inside);
    }
    if (!firstFault.empty())
        throw std::runtime_error("Native Metal produced a faulted/nonfinite stage record at pixel " + firstFault);
}

struct ReflectionPipeline {
    id<MTLComputePipelineState> kernel = nil;
    id<MTLBuffer> records = nil;
    ReflectionPipeline(gpu::Metal& m, size_t count) {
        auto marker = m.pipeline(@"native_reflection_abi");
        auto identity = m.buffer(sizeof(uint32_t) * 3);
        std::memset(identity.contents, 0, identity.length);
        gpu::dispatchStage(m, marker, 1, [&](id<MTLComputeCommandEncoder> encoder) {
            [encoder setBuffer:identity offset:0 atIndex:0];
        });
        const auto* fields = static_cast<const uint32_t*>(identity.contents);
        gpu::require(fields[0] == 1 && fields[1] == sizeof(NativeReflectionParams) &&
                     fields[2] == sizeof(NativeReflectionRecord),
                     "Native Metal reflection ABI mismatch");
        kernel = m.pipeline(@"native_reflection");
        records = m.buffer(count * sizeof(NativeReflectionRecord));
    }
};

void RunReflections(gpu::Metal& m, const ReflectionPipeline& pipeline,
                    const ExactScene& scene, const ExactSurfaceParams& packing,
                    const ExactShadowParams& shadow, const NativeReflectionParams& params,
                    const gpu::RenderStageBuffers& b, ReflectionActivity& activity,
                    const native_metal::RestrictedExecutionPolicy& execution) {
    const uint32_t count = scene.width * scene.height;
    // Limit each command independently of the scene dimensions or ray budget.
    const uint32_t batch = execution.reflection_batch();
    for (uint32_t offset = 0; offset < count; offset += batch) {
        const uint32_t launch[] = {offset, std::min(batch, count - offset), count};
        const double milliseconds = gpu::dispatchStage(m, pipeline.kernel, launch[1],
            [&](id<MTLComputeCommandEncoder> encoder) {
                [encoder setBytes:&scene length:sizeof(scene) atIndex:0];
                [encoder setBytes:&packing length:sizeof(packing) atIndex:1];
                [encoder setBytes:&shadow length:sizeof(shadow) atIndex:2];
                [encoder setBuffer:b.colorScene offset:0 atIndex:3];
                [encoder setBytes:&params length:sizeof(params) atIndex:4];
                [encoder setBuffer:b.pixels offset:0 atIndex:5];
                [encoder setBuffer:b.shaded offset:0 atIndex:6];
                [encoder setBuffer:b.rgb offset:0 atIndex:7];
                [encoder setBuffer:pipeline.records offset:0 atIndex:8];
                [encoder setBytes:launch length:sizeof(launch) atIndex:9];
            }, 1, execution.reflection_group_width());
        activity.max_command_seconds = std::max(activity.max_command_seconds, milliseconds / 1000);
    }
    const auto* records = static_cast<const NativeReflectionRecord*>(pipeline.records.contents);
    const auto* colors = static_cast<const ExactColorPixel*>(b.rgb.contents);
    std::string firstFault;
    for (uint32_t i = 0; i < count; ++i) {
        const auto& r = records[i];
        activity.active_pixels += r.active;
        activity.rays += r.rays; activity.hits += r.hits; activity.misses += r.misses;
        activity.changed_pixels += r.changed_pixels;
        activity.ray_steps += r.ray_steps; activity.primary_skipped += r.primary_skipped;
        activity.ao_rays += r.ao_rays; activity.ao_steps += r.ao_steps;
        activity.shadow_rays += r.shadow_rays; activity.shadow_steps += r.shadow_steps;
        activity.max_bounces = std::max(activity.max_bounces, uint64_t(r.bounces));
        const bool invalid = r.fault || r.nonfinite || r.reserved0 || r.reserved1 ||
            r.active > 1 || r.changed_pixels > 1 || r.primary_skipped > 1 ||
            r.bounces > params.max_bounces || r.rays > params.max_bounces ||
            r.hits + r.misses != r.rays || colors[i].fault ||
            colors[i].reserved0 || colors[i].reserved1;
        activity.faults += invalid;
        if (invalid && firstFault.empty())
            firstFault = "; first pixel=" + std::to_string(i) + " code=" + std::to_string(r.fault) +
                " nonfinite=" + std::to_string(r.nonfinite) + " bounce=" + std::to_string(r.bounces) +
                " rays/hits/misses=" + std::to_string(r.rays) + "/" + std::to_string(r.hits) +
                "/" + std::to_string(r.misses) + " steps=" + std::to_string(r.ray_steps) +
                " ao_steps=" + std::to_string(r.ao_steps) + " shadow_steps=" + std::to_string(r.shadow_steps);
    }
    gpu::require(activity.faults == 0,
                 "Native Metal reflection pass produced invalid or faulted records: " +
                 std::to_string(activity.faults) + firstFault);
}

void Attempt(TMandHeader10& header, THeaderCustomAddon& addon,
             const RenderOptions& opts, const std::string& libraryPath,
             NativeMetalAttempt& attempt) {
    attempt.result.profile.precision = PrecisionMode::Native;
    attempt.library = libraryPath;
    auto reject = [&](NativeMetalStatus status, const std::string& reason) {
        attempt.status = status;
        attempt.reason = reason;
        attempt.result.error = reason;
        attempt.result.status = RenderStatus::Failed;
    };
    if (UnsupportedOptions(opts) || (header.MCoptions & 1))
        return reject(NativeMetalStatus::Unsupported, "Native Metal does not support these precision, pass, diagnostic or control options");
    TMandHeader10 effective = header;
    if (opts.deterministic_mode) {
        effective.iOptions &= ~1;
        effective.bCalcAmbShadowAutomatic &= static_cast<uint8_t>(~128u);
    }
    // These overrides disable their final passes. AO is selected separately:
    // its override skips the image pass but must not disable point AO inside
    // an enabled reflection, matching the ordinary CPU render contract.
    if (opts.no_dof) effective.bCalcDOFtype &= static_cast<uint8_t>(~1u);
    if (opts.no_reflections) effective.bCalcSRautomatic &= static_cast<uint8_t>(~1u);
    const int width = opts.width_override > 0 ? opts.width_override : effective.Width;
    const int height = opts.height_override > 0 ? opts.height_override : effective.Height;
    if (width < 1 || height < 1 || width > 4096 || height > 4096)
        return reject(NativeMetalStatus::Unsupported, "Native Metal dimensions must be within 1..4096");
    const size_t count = size_t(width) * size_t(height);
    int threads = opts.thread_count > 0 ? opts.thread_count : int(std::thread::hardware_concurrency());
    threads = std::clamp(threads > 0 ? threads : 4, 1, height);
    const auto start = gpu::Clock::now();
    const ScopedNumericPrecision precision(PrecisionMode::Native);
    auto& result = attempt.result;
    result.profile.enabled = opts.profile_timing;
    result.profile.width = width; result.profile.height = height;
    result.profile.workers = 0;
    ProfileRecorder recorder(result.profile, start);
    recorder.begin(RenderProfileStage::Preparation);
    PreparedExactScene prepared;
    ExactSurfaceParams packing{};
    ExactShadowParams shadow{};
    PreparedExactAO ao;
    PreparedExactColor color;
    NativeReflectionParams reflection{};
    bool ambient = false;
    // Every capability check runs before the library marker or any render
    // dispatch. Preparation owns copies; rejection never mutates caller state.
    try {
        native_metal::ValidateNativeLightingRequirements(native_metal::NativeLightingRequirements(header));
        const std::string root = opts.resource_root.empty()
            ? std::filesystem::current_path().string() : opts.resource_root;
        prepared = PrepareLoadedExactScene(effective, addon, unsigned(width), unsigned(height), root);
        packing = PrepareSurface(prepared);
        shadow = PrepareShadow(prepared);
        ambient = !opts.no_ao && prepared.host->mct.bCalcAmbShadow;
        if (ambient) ao = PrepareExactAO(prepared, unsigned(threads));
        color = PrepareExactColor(prepared);
        reflection = PrepareNativeReflection(prepared);
    } catch (const std::invalid_argument& error) {
        return reject(NativeMetalStatus::Unsupported, error.what());
    } catch (const std::runtime_error& error) {
        return reject(NativeMetalStatus::Unsupported, error.what());
    }
    const auto scene = prepared.device;
    const bool shadows = shadow.enabled != 0;
    const bool reflections = reflection.max_bounces != 0;
    const native_metal::RestrictedExecutionPolicy execution({scene.width,scene.height,shadows,ambient,reflections});
    const std::filesystem::path library(libraryPath);
    std::error_code fileError;
    const bool exists = !libraryPath.empty() && std::filesystem::exists(library, fileError);
    if (fileError)
        return reject(NativeMetalStatus::Failed, "Cannot inspect Native Metal library: " + fileError.message());
    if (!exists)
        return reject(NativeMetalStatus::Unavailable, "Native Metal library is unavailable: " + libraryPath);
    if (!std::filesystem::is_regular_file(library, fileError) || fileError)
        return reject(NativeMetalStatus::Failed, "Native Metal library is not a readable regular file");
    id<MTLDevice> device = MTLCreateSystemDefaultDevice();
    if (!device)
        return reject(NativeMetalStatus::Unavailable, "Metal device unavailable");
    attempt.device = device.name.UTF8String ?: "";
    attempt.library = std::filesystem::absolute(library).lexically_normal().string();
    const uint64_t sizes[] = {
        prepared.columns.size() * sizeof(ExactTrig), prepared.rows.size() * sizeof(ExactTrig),
        count * sizeof(ExactPixel), count * sizeof(ExactSurface), count * sizeof(ExactSurface),
        count * sizeof(ExactNormalSamples), shadows ? count * sizeof(ExactShadowRay) : 0,
        shadows ? count * sizeof(ExactShadowTrace) : 0,
        ambient ? count * 4 : 16, ambient ? count * 4 : 16, ambient ? count * 64 : 16,
        ao.atan_terms.size() * sizeof(ExactU64), sizeof(ExactColorScene),
        color.columns.size() * sizeof(ExactTrig), color.rows.size() * sizeof(ExactTrig), count * sizeof(ExactColorPixel),
        reflections ? count * sizeof(NativeReflectionRecord) : 0};
    uint64_t total = 0;
    const uint64_t recommendation = device.recommendedMaxWorkingSetSize;
    const uint64_t budget = native_metal::NativeDeviceBudget(recommendation);
    for (uint64_t bytes : sizes) {
        if (bytes > device.maxBufferLength || bytes > budget - total)
            return reject(NativeMetalStatus::Unsupported, "Native Metal render buffers exceed the bounded device budget");
        total += bytes;
    }
    if (opts.admission_only) {
        attempt.result.success = true; attempt.result.status = RenderStatus::Completed;
        attempt.status = NativeMetalStatus::Completed;
        return;
    }
    // From this point onward every failure is an execution/library failure.
    // The constructor dispatches and verifies the real [2,4,32] shader marker.
    gpu::Metal m(library, scene.formula_kind, scene.bulb.power, scene.capability_id, false, true);
    attempt.execution.profile_kind = m.arithmeticProfile[0];
    attempt.execution.engine = "exact_capability";
    attempt.execution.execution_plan_version=1;
    attempt.execution.execution_plan=execution.describe();
    attempt.execution.storage_bytes = m.arithmeticProfile[1];
    attempt.execution.arithmetic_bits = m.arithmeticProfile[2];
    const auto& dispatchOptions=execution.dispatch();
    gpu::RenderBenchmarkPipelines pipelines(m, dispatchOptions, shadows, ambient);
    std::unique_ptr<ReflectionPipeline> reflectionPipeline;
    if (reflections) {
        attempt.reflections.enabled = true;
        reflectionPipeline = std::make_unique<ReflectionPipeline>(m, count);
    }
    gpu::RenderStageBuffers b;
    b.cols = m.buffer(sizes[0], prepared.columns.data());
    b.rows = m.buffer(sizes[1], prepared.rows.data());
    b.pixels = m.buffer(sizes[2]);
    b.surface = m.buffer(sizes[3]);
    b.shaded = m.buffer(sizes[4]);
    b.normalSamples = m.buffer(sizes[5]);
    if (shadows) { b.rays = m.buffer(sizes[6]); b.traces = m.buffer(sizes[7]); }
    b.depth = m.buffer(sizes[8]); b.temporary = m.buffer(sizes[9]); b.angles = m.buffer(sizes[10]);
    if (ambient) b.atan = m.buffer(sizes[11], ao.atan_terms.data());
    b.colorScene = m.buffer(sizes[12], &color.device);
    b.colorCols = m.buffer(sizes[13], color.columns.data());
    b.colorRows = m.buffer(sizes[14], color.rows.data());
    b.rgb = m.buffer(sizes[15]);
    result.width = width; result.height = height;
    result.image.resize(count * 3);
    result.silight.resize(count);
    const double preparationSeconds = gpu::elapsed(start) / 1000;
    recorder.end(true);
    double maxPrimary = 0, maxShadow = 0;
    auto observe = [&](unsigned index, bool entered) {
        if (index == 7 && !ambient) return;
        const RenderProfileStage stages[] = {
            RenderProfileStage::Primary, RenderProfileStage::Primary,
            RenderProfileStage::Shadows, RenderProfileStage::Shadows, RenderProfileStage::Shadows,
            RenderProfileStage::AO, RenderProfileStage::Color, RenderProfileStage::AO};
        if (entered) recorder.begin(stages[index]);
        else recorder.end(index == 1 || index == 4 || index == 5 || index == 6);
    };
    const auto metrics = gpu::RunRenderStages(m, dispatchOptions, scene, packing, shadow, ao, pipelines, b, maxPrimary, maxShadow, observe, ambient, [&]{CheckInitialInside(b,count);});
    ValidateFrame(b, count, shadows, attempt.execution);
    double reflectionSeconds = 0;
    if (reflections) {
        const auto reflectionStart = gpu::Clock::now();
        recorder.begin(RenderProfileStage::Reflections);
        RunReflections(m, *reflectionPipeline, scene, packing, shadow, reflection, b, attempt.reflections,execution);
        attempt.execution.reflection_records = count;
        reflectionSeconds = gpu::elapsed(reflectionStart) / 1000;
        recorder.end(true);
    }
    const auto readStart = gpu::Clock::now();
    const auto* colors = static_cast<const ExactColorPixel*>(b.rgb.contents);
    const auto* surface = static_cast<const ExactSurface*>(b.shaded.contents);
    for (size_t i = 0; i < count; ++i) {
        for (unsigned channel = 0; channel < 3; ++channel)
            result.image[i * 3 + channel] = uint8_t(colors[i].rgb >> (channel * 8));
        result.silight[i] = ImportSurface(surface[i]);
    }
    if (opts.profile_timing) {
        auto& profile = result.profile;
        profile.enabled = true; profile.width = width; profile.height = height;
        // No CPU render worker pool is created by this adapter. The requested
        // thread count is only a host-preparation option, not executed workers.
        profile.workers = 0; profile.precision = PrecisionMode::Native;
        profile.total_seconds = gpu::elapsed(start) / 1000;
        auto stage = [&](RenderProfileStage key, double seconds) {
            profile.stages[size_t(key)] = {"completed", seconds};
        };
        stage(RenderProfileStage::Preparation, preparationSeconds);
        stage(RenderProfileStage::Primary, (metrics.wall[0] + metrics.wall[1] + (ambient ? 0 : metrics.wall[7])) / 1000);
        if (shadows) stage(RenderProfileStage::Shadows, (metrics.wall[2] + metrics.wall[3] + metrics.wall[4]) / 1000);
        if (ambient) stage(RenderProfileStage::AO, (metrics.wall[5] + metrics.wall[7]) / 1000);
        stage(RenderProfileStage::Color, (metrics.wall[6] + gpu::elapsed(readStart)) / 1000);
        if (reflections) stage(RenderProfileStage::Reflections, reflectionSeconds);
    }
    result.success = true; result.status = RenderStatus::Completed;
    attempt.execution.validated = true;
    attempt.status = NativeMetalStatus::Completed;
    // Commit the same observable size/deterministic changes as ordinary CPU
    // Render only after the complete GPU result has passed validation. Failed
    // or unsupported attempts leave fallback's input parameters untouched.
    header.Width = width;
    header.Height = height;
    header.iOptions = effective.iOptions;
    header.bCalcAmbShadowAutomatic = effective.bCalcAmbShadowAutomatic;
}
} // namespace

NativeMetalAttempt RenderNativeMetal(TMandHeader10& header, THeaderCustomAddon& addon,
                                    const RenderOptions& opts, const std::string& libraryPath) {
    @autoreleasepool {
        NativeMetalAttempt attempt;
        attempt.library = libraryPath;
        attempt.result.profile.precision = PrecisionMode::Native;
        try {
            Attempt(header, addon, opts, libraryPath, attempt);
        } catch (const NeedsGeneralInitialInside&) {
            attempt.status=NativeMetalStatus::NeedsGeneralInitialInside;
            attempt.reason="Restricted Metal surface packing is outside-only; general Metal required";
            attempt.result.success=false;
            attempt.result.image.clear();attempt.result.silight.clear();
        } catch (const RenderCancelRequested& error) {
            attempt.status = NativeMetalStatus::Failed;
            attempt.reason = error.what();
            attempt.result.error = attempt.reason;
            attempt.result.success = false;
            attempt.result.status = RenderStatus::Cancelled;
            attempt.result.image.clear();
            attempt.result.silight.clear();
        } catch (const std::exception& error) {
            attempt.status = NativeMetalStatus::Failed;
            attempt.reason = error.what();
            attempt.result.error = attempt.reason;
            attempt.result.success = false;
            attempt.result.status = RenderStatus::Failed;
            attempt.result.image.clear();
            attempt.result.silight.clear();
        }
        return attempt;
    }
}
} // namespace mb3d
