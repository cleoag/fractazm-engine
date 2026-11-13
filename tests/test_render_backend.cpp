#include "render/backend.h"
#include "render/native_metal/pixel_independence.h"
#include "core/render_context.h"
#include "core/ray_marcher.h"
#include "render/native_metal/native_inside_reflection_math.h"
#include <limits>
#include "io/m3p_parser.h"

#include <array>
#include <atomic>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <future>
#include <latch>
#include <stdexcept>
#include <string>

namespace {
std::atomic<unsigned> checks{0};
void check(bool condition, const char* message) {
    checks.fetch_add(1, std::memory_order_relaxed);
    if (!condition) throw std::runtime_error(message);
}

void reflectionDepthControls() {
    using mb3d_inside::reflection_color_depth;
    TMCTparameter m{};m.Zcorr=.2;m.ZcMul=100;m.iSmNormals=0;
    for(auto pair:std::array<std::array<float,2>,4>{{{.008722f,.855348f},{.01f,.5f},{1.f,2.f},{0.f,0.f}}}) {
        const auto d=reflection_color_depth(pair[0],pair[1],1);
        const float sum=pair[0]+pair[1],rad=sum*sum-pair[1]+1.0e-30f;
        volatile double source=rad;
        double legacy=std::sqrt(source);
#if defined(__APPLE__) && defined(__aarch64__)
        if(rad<0.f)check(std::isnan(legacy)&&!std::signbit(legacy), "ARM Native sqrt domain encoding changed");
#else
        // This helper specifies the Apple GPU profile, not another CPU's NaN sign.
        if(rad<0.f)legacy=std::numeric_limits<double>::quiet_NaN();
#endif
        TsiLight5 old{},now{};mb3d::CalcZposAndRough(&old,&m,legacy);mb3d::CalcZposAndRough(&now,&m,d.value);
        if(d.zero_packed){const uint32_t zero=0;std::memcpy(&now.RoughZposFine,&zero,4);}
        check(d.zero_packed==uint32_t(rad<0.f), "reflection domain classification changed");
        check(!d.fault && std::isfinite(d.value), "finite reflection color depth rejected");
        check(std::memcmp(&old.RoughZposFine,&now.RoughZposFine,4)==0,"reflection depth packing changed CPU boundary");
    }
    for(float bad:{std::numeric_limits<float>::infinity(),-std::numeric_limits<float>::infinity(),std::numeric_limits<float>::quiet_NaN()}) {
        check(reflection_color_depth(bad,1,1).fault!=0,"nonfinite segment admitted");
        check(reflection_color_depth(1,bad,1).fault!=0,"nonfinite depth admitted");
    }
    check(reflection_color_depth(std::numeric_limits<float>::max(),1,1).fault!=0,"overflow admitted");
    check(reflection_color_depth(1,-1,1).fault!=0,"negative prior depth admitted");
    check(reflection_color_depth(1,1,2).fault!=0,"invalid color mode admitted");
    check(reflection_color_depth(-.25f,1,0).value==-.25f,"plain color depth changed");
}

void pixelIndependenceControls() {
    using namespace mb3d_native;
    {
        NativeFormulaProgram boxes{};boxes.driver=Hybrid3DAnalytic;boxes.de_option=2;
        auto& a=boxes.slots[0];a.version=1;a.enabled=1;a.namespace_id=ExternalFormula;
        a.id=8024;a.de_option=2;a.param_count=8;a.wrapper_flags=1;
        auto& b=boxes.slots[1];b=a;b.id=8900;b.param_count=10;b.wrapper_flags=0;
        check(!IndependentPrimaryPixels(boxes), "Fractal Fan carried Vary admitted without first-write proof");
        b=a;b.id=10014;b.de_option=-1;b.param_count=13;b.traits=576;
        check(!IndependentPrimaryPixels(boxes), "Surreal shell carried Vary admitted without first-write proof");
        b.enabled=0;
        check(!IndependentPrimaryPixels(boxes), "ABoxMod1 conditional Vary initialization treated as entry reset");
        b.enabled=1;
        // Preserve positive descriptor/driver controls using the existing
        // stateless ABoxModKali subset, which never mutates Vary.
        a.id=8900;a.param_count=10;a.wrapper_flags=0;
        check(IndependentPrimaryPixels(boxes), "stateless box and orbit-local Julia writes rejected");
        for(unsigned driver=0;driver<=9;++driver) {
            boxes.driver=driver;
            check(IndependentPrimaryPixels(boxes)==(driver==Hybrid3DAnalytic), "box chain wrong driver admitted");
        }
        boxes.driver=Hybrid3DAnalytic;
        for(int de:{0,4,6,16,32}) {boxes.de_option=de;
            check(!IndependentPrimaryPixels(boxes), "box chain wrong derivative initialization admitted");}
        boxes.de_option=2;
        for(unsigned bit:{1u,2u,8u,1024u}) {b.traits=576|bit;
            check(!IndependentPrimaryPixels(boxes), "box chain retained state mutation admitted");}
        b.traits=576;b.wrapper_flags=0;check(!IndependentPrimaryPixels(boxes), "changed Julia wrapper admitted");
        b.wrapper_flags=1;b.param_count=12;check(!IndependentPrimaryPixels(boxes), "changed Julia parameter ABI admitted");
        b.param_count=13;b.namespace_id=InternalFormula;check(!IndependentPrimaryPixels(boxes), "box namespace collision admitted");
        b.namespace_id=ExternalFormula;b.difs_style=1;check(!IndependentPrimaryPixels(boxes), "box dIFS wrapper admitted");
        b.difs_style=0;b.id=9526;b.traits=0;check(!IndependentPrimaryPixels(boxes), "unaudited mixed box chain admitted");
        b.enabled=0;check(IndependentPrimaryPixels(boxes), "disabled box neighbour changes dependence");
        a.enabled=0;check(!IndependentPrimaryPixels(boxes), "empty box chain admitted");
    }
    {
        NativeFormulaProgram depth{};depth.driver=Hybrid3DAnalytic;depth.de_option=11;
        auto& a=depth.slots[0];a.version=kFormulaAbiVersion;a.enabled=1;a.namespace_id=ExternalFormula;
        a.id=10005;a.de_option=-1;a.param_count=5;a.traits=4160;a.wrapper_flags=1;
        auto& b=depth.slots[1];b.version=kFormulaAbiVersion;b.enabled=1;b.namespace_id=InternalFormula;
        b.id=4;b.de_option=11;b.wrapper_flags=2;
        check(IndependentPrimaryPixels(depth), "PartlyJulia orbit-local state rejected");
        for(unsigned bad:{2u,8u,1024u}){a.traits=4160|bad;check(!IndependentPrimaryPixels(depth), "PartlyJulia retained state admitted");}
        a.traits=4160;b.wrapper_flags=0;check(!IndependentPrimaryPixels(depth), "Wrong internal box wrapper admitted");b.wrapper_flags=2;
        for(unsigned driver=0;driver<=9;++driver){depth.driver=driver;check(IndependentPrimaryPixels(depth)==(driver==Hybrid3DAnalytic), "Depth chain wrong driver admitted");}
        depth.driver=Hybrid3DAnalytic;depth.de_option=2;
        a.id=7227;a.de_option=2;a.param_count=22;a.traits=0;
        b=a;b.id=7060;b.de_option=-1;b.param_count=9;
        check(IndependentPrimaryPixels(depth), "Koch/Rotate independent state rejected");
        for(unsigned bad:{1u,2u,8u,64u,512u,1024u}){b.traits=bad;check(!IndependentPrimaryPixels(depth), "Koch retained-state neighbour admitted");}
        b.traits=0;b.namespace_id=InternalFormula;check(!IndependentPrimaryPixels(depth), "Koch namespace collision admitted");b.namespace_id=ExternalFormula;
        b.param_count=8;check(!IndependentPrimaryPixels(depth), "Changed rotation layout admitted");b.param_count=9;
        b.version=0;check(!IndependentPrimaryPixels(depth), "Stale depth formula ABI admitted");b.version=kFormulaAbiVersion;
        b.id=9526;check(!IndependentPrimaryPixels(depth), "Unaudited depth mixed chain admitted");
    }
    NativeFormulaProgram p{};p.driver=Hybrid3DAnalytic;p.de_option=2;
    auto& s=p.slots[0];s.enabled=1;s.namespace_id=ExternalFormula;s.id=9526;
    check(IndependentPrimaryPixels(p), "audited Menger3 not independent");
    for(unsigned driver=2;driver<=9;++driver){p.driver=driver;
        check(!IndependentPrimaryPixels(p), "stateful/interpolated driver admitted");}
    p.driver=Hybrid3DAnalytic;p.de_option=32;
    check(!IndependentPrimaryPixels(p), "retained XYZ derivative admitted");p.de_option=2;
    for(unsigned id:{10014u,10343u,10350u,10353u,9401u,7004u,999999u}){
        s.id=id;check(!IndependentPrimaryPixels(p), "unaudited formula admitted");}
    s.id=9526;s.traits=2;check(!IndependentPrimaryPixels(p), "retained derivative trait admitted");
    s.traits=0;s.difs_style=1;check(!IndependentPrimaryPixels(p), "dIFS wrapper admitted");s.difs_style=0;
    auto& second=p.slots[1];second.enabled=1;second.namespace_id=ExternalFormula;second.id=10014;
    check(!IndependentPrimaryPixels(p), "mixed stateful chain admitted");
    second.enabled=0;check(IndependentPrimaryPixels(p), "disabled slot changes dependence");
    second.enabled=1;second.namespace_id=InternalFormula;second.id=9;
    check(!IndependentPrimaryPixels(p), "unaudited internal neighbour admitted");
    p={};p.driver=Hybrid4D;p.de_option=4;
    auto& f=p.slots[0];f.enabled=1;f.namespace_id=ExternalFormula;f.id=9401;
    check(IndependentPrimaryPixels(p), "Makin4D independent reset rejected");
    f.wrapper_flags=1;check(!IndependentPrimaryPixels(p), "changed Makin wrapper admitted");f.wrapper_flags=0;
    f.traits=2;check(!IndependentPrimaryPixels(p), "changed Makin traits admitted");f.traits=0;
    p.de_option=6;check(!IndependentPrimaryPixels(p), "wrong 4D DE admitted");
    p.driver=Hybrid4DAnalytic;
    for(unsigned id:{10350u,5823u,10343u}) {
        f.id=id;f.wrapper_flags=1;f.traits=id==10343?2:0;
        check(IndependentPrimaryPixels(p), "audited analytic4D rejected");
        f.traits|=8;check(!IndependentPrimaryPixels(p), "SSE analytic4D admitted");
    }
    f.traits=2;p.slots[1]=f;p.slots[1].id=999999;
    check(!IndependentPrimaryPixels(p), "unknown 4D neighbour admitted");
    p.slots[1].enabled=0;p.driver=Interpolated4DAnalytic;
    check(!IndependentPrimaryPixels(p), "interpolated 4D admitted");

}

struct InputSnapshot {
    std::array<std::byte, sizeof(TMandHeader10)> header;
    std::array<std::byte, sizeof(THeaderCustomAddon)> addon;
    explicit InputSnapshot(const mb3d::M3PLoadResult& input) {
        std::memcpy(header.data(), &input.header, header.size());
        std::memcpy(addon.data(), &input.addon, addon.size());
    }
    void unchanged(const mb3d::M3PLoadResult& input) const {
        check(std::memcmp(header.data(), &input.header, header.size()) == 0,
              "GPU/rejected call mutated caller header bytes");
        check(std::memcmp(addon.data(), &input.addon, addon.size()) == 0,
              "GPU/rejected call mutated caller addon bytes");
    }
    void committed(const mb3d::M3PLoadResult& input, const mb3d::RenderOptions& options) const {
        TMandHeader10 expected;
        std::memcpy(&expected, header.data(), header.size());
        expected.Width = options.width_override;
        expected.Height = options.height_override;
        if (options.deterministic_mode) {
            expected.iOptions &= ~1;
            expected.bCalcAmbShadowAutomatic &= static_cast<uint8_t>(~128u);
        }
        check(std::memcmp(&expected, &input.header, header.size()) == 0,
              "successful GPU call failed to commit dimensions/flags or changed unrelated header bytes");
        check(std::memcmp(addon.data(), &input.addon, addon.size()) == 0,
              "successful GPU call mutated caller addon bytes");
    }
};

mb3d::M3PLoadResult load(const std::filesystem::path& path) {
    auto input = mb3d::LoadM3PFile(path.string());
    if (!input.success) throw std::runtime_error(input.error);
    input.header.bCalc3D = 1;
    return input;
}

void sameImage(const mb3d::RenderResult& actual, const mb3d::RenderResult& expected) {
    check(actual.success && expected.success, "repeated render failed");
    check(actual.status == mb3d::RenderStatus::Completed, "successful result has wrong lifecycle status");
    check(actual.width == expected.width && actual.height == expected.height,
          "repeat changed dimensions");
    check(actual.image == expected.image, "repeat changed RGB");
}

void sameSurface(const mb3d::RenderResult& actual, const mb3d::RenderResult& expected) {
    check(actual.silight.size() == expected.silight.size(), "repeat changed surface count");
    check(!actual.silight.empty() &&
          std::memcmp(actual.silight.data(), expected.silight.data(),
                      actual.silight.size() * sizeof(TsiLight5)) == 0,
          "Metal/auto repeat changed exported surface bytes");
}
} // namespace

int main(int argc, char** argv) {
    try {
        pixelIndependenceControls();
    reflectionDepthControls();
        if (argc != 2 && !(argc == 4 && std::string(argv[2]) == "--metal-library"))
            throw std::runtime_error("usage: test_render_backend SOURCE_ROOT [--metal-library NATIVE.metallib]");
        using namespace mb3d;
        const std::filesystem::path source = std::filesystem::absolute(argv[1]);
        const bool hasMetal = argc == 4;
        const std::string library = hasMetal ? std::filesystem::absolute(argv[3]).string() : std::string{};
        if (hasMetal) check(std::filesystem::is_regular_file(library), "Native library is missing");
        const auto originalPrecision = CurrentPrecisionMode();
        const auto originalContext = CaptureRenderContext();
        auto* const originalControl = current_render_control;
        {
            auto caller = std::make_shared<RenderContext>();
            caller->precision = PrecisionMode::Compatible;
            caller->control = std::make_shared<RenderControl>();
            ScopedRenderContext callerBinding(caller);
            auto restored = [&] {
                check(CurrentPrecisionMode() == PrecisionMode::Compatible, "backend leaked Native arithmetic into caller TLS");
                check(CaptureRenderContext() == caller, "backend replaced caller render context");
                check(current_render_control == caller->control.get(), "backend leaked its control binding");
                check(caller->control->status() == RenderStatus::Prepared && caller->control->work_used() == 0,
                      "backend charged or changed caller's unrelated control");
            };
            RenderOptions options;
            options.precision = PrecisionMode::Native;
            options.width_override = 64;
            options.height_override = 48;
            options.thread_count = 6;
            options.deterministic_mode = true;
            options.profile_timing = true;
            options.resource_root = (source / "comparison/fixtures").string();
            BackendOptions backend;
            backend.metal_library = library;
            const std::filesystem::path scenes = source / "comparison/benchmark_scenes/effects-v1";
            const char* names[] = {"sphereifs-ao-shadows", "boxifs-ao-shadows", "integerpower8-ao-shadows"};
            RenderResult firstSceneCPU, firstSceneMetal;
            for (const char* name : names) {
                const auto path = scenes / (std::string(name) + ".m3p");
                auto cpuInput = load(path);
                backend.requested = RenderBackend::CPU;
                auto cpu = RenderWithBackend(cpuInput.header, cpuInput.addon, options, backend);
                check(cpu.render.success && cpu.backend.effective == RenderBackend::CPU && !cpu.backend.fallback,
                      "explicit CPU did not complete without fallback");
                check(cpu.render.profile.precision == PrecisionMode::Native, "CPU did not apply requested Native policy");
                restored();
                RenderResult firstMetal;
                bool haveFirst = false;
                for (const auto requested : {RenderBackend::Metal, RenderBackend::Auto}) {
                    backend.requested = requested;
                    for (int repeat = 0; repeat < 2; ++repeat) {
                        auto input = load(path);
                        input.header.iOptions |= 1;
                        input.header.bCalcAmbShadowAutomatic |= 128u;
                        const InputSnapshot before(input);
                        auto actual = RenderWithBackend(input.header, input.addon, options, backend);
                        check(actual.backend.requested == requested, "backend lost requested selection");
                        check(actual.render.success && actual.render.status == RenderStatus::Completed,
                              "Native backend repeat did not complete");
                        check(actual.render.profile.precision == PrecisionMode::Native,
                              "backend result lost Native arithmetic policy");
                        check(actual.render.width == 64 && actual.render.height == 48 && actual.render.image.size() == 64u * 48u * 3u,
                              "backend returned incomplete image dimensions");
                        if (hasMetal) {
                            check(actual.backend.effective == RenderBackend::Metal && !actual.backend.fallback,
                                  "supported Native scene unexpectedly fell back");
                            check(actual.render.profile.workers == 0, "GPU result invents a CPU render worker pool");
                            check(!actual.backend.device.empty() && !actual.backend.library.empty(),
                                  "GPU result omitted device/library identity");
                            before.committed(input, options);
                            if (!haveFirst) {
                                firstMetal = actual.render;
                                haveFirst = true;
                            }
                            sameImage(actual.render, firstMetal);
                            sameSurface(actual.render, firstMetal);
                        } else {
                            check(actual.backend.effective == RenderBackend::CPU && actual.backend.fallback &&
                                  actual.backend.fallback_code == "metal_not_built" && !actual.backend.fallback_reason.empty(),
                                  "no-Metal build hid fallback provenance");
                            sameImage(actual.render, cpu.render);
                        }
                        restored();
                    }
                }
                if (std::string(name) == names[0]) {
                    firstSceneCPU = cpu.render;
                    if (hasMetal) firstSceneMetal = firstMetal;
                }
            }

            // Opaque reflections are admitted only by the main Native API.
            // Each call starts with the same loaded input; the primary surface
            // remains exportable and must be unchanged by the color-only pass.
            const auto reflectionPath = scenes / "integerpower8-reflections.m3p";
            const auto reflectionOffPath = scenes / "integerpower8-reflections-off.m3p";
            auto reflectionCPUInput = load(reflectionPath);
            backend.requested = RenderBackend::CPU;
            const auto reflectionCPU = RenderWithBackend(reflectionCPUInput.header, reflectionCPUInput.addon, options, backend);
            auto offCPUInput = load(reflectionOffPath);
            const auto offCPU = RenderWithBackend(offCPUInput.header, offCPUInput.addon, options, backend);
            check(reflectionCPU.render.success && offCPU.render.success && reflectionCPU.render.image != offCPU.render.image,
                  "CPU reflection control did not change RGB");
            restored();
            RenderResult reflectionMetal;
            ReflectionActivity firstActivity;
            bool haveReflection = false;
            for (const auto requested : {RenderBackend::Metal, RenderBackend::Auto}) {
                backend.requested = requested;
                for (int repeat = 0; repeat < 3; ++repeat) {
                    auto input = load(reflectionPath);
                    const InputSnapshot before(input);
                    const auto result = RenderWithBackend(input.header, input.addon, options, backend);
                    if (!result.render.success)
                        std::fprintf(stderr, "reflection render error: %s\n", result.render.error.c_str());
                    check(result.render.success && result.render.profile.precision == PrecisionMode::Native,
                          "Native reflection render did not complete");
                    const auto& stage = result.render.profile.stages[static_cast<size_t>(RenderProfileStage::Reflections)];
                    check(std::strcmp(stage.state, "completed") == 0 && stage.seconds > 0, "Reflection stage did not run");
                    if (hasMetal) {
                        check(result.backend.effective == RenderBackend::Metal && !result.backend.fallback,
                              "Opaque reflection silently fell back");
                        before.committed(input, options);
                        const auto& a = result.backend.reflections;
                        check(a.enabled && a.rays > 0 && a.active_pixels > 0 && a.changed_pixels > 0 && a.faults == 0 &&
                              a.max_bounces >= 1 && a.max_bounces <= 2 && a.hits + a.misses == a.rays &&
                              a.ray_steps > 0 && a.ao_rays > 0 && a.ao_steps > 0 &&
                              a.shadow_rays > 0 && a.shadow_steps > 0 && a.max_command_seconds > 0,
                              "GPU reflection traversal/secondary effects did not execute cleanly");
                        if (!haveReflection) {
                            reflectionMetal = result.render;
                            firstActivity = a;
                            haveReflection = true;
                        }
                        sameImage(result.render, reflectionMetal);
                        sameSurface(result.render, reflectionMetal);
                        check(a.active_pixels == firstActivity.active_pixels && a.rays == firstActivity.rays &&
                              a.hits == firstActivity.hits && a.misses == firstActivity.misses &&
                              a.changed_pixels == firstActivity.changed_pixels && a.max_bounces == firstActivity.max_bounces &&
                              a.ray_steps == firstActivity.ray_steps && a.primary_skipped == firstActivity.primary_skipped &&
                              a.ao_rays == firstActivity.ao_rays && a.ao_steps == firstActivity.ao_steps &&
                              a.shadow_rays == firstActivity.shadow_rays && a.shadow_steps == firstActivity.shadow_steps,
                              "GPU reflection activity changed across repeated jobs");
                    } else {
                        check(result.backend.effective == RenderBackend::CPU && result.backend.fallback &&
                              result.backend.fallback_code == "metal_not_built", "No-Metal reflection fallback lost provenance");
                        sameImage(result.render, reflectionCPU.render);
                    }
                    restored();
                }
            }
            if (hasMetal) {
                backend.requested = RenderBackend::Metal;
                auto input = load(reflectionOffPath);
                const auto off = RenderWithBackend(input.header, input.addon, options, backend);
                check(off.render.success && off.backend.effective == RenderBackend::Metal && !off.backend.fallback &&
                      !off.backend.reflections.enabled && off.backend.reflections.rays == 0 &&
                      std::strcmp(off.render.profile.stages[static_cast<size_t>(RenderProfileStage::Reflections)].state, "not_run") == 0,
                      "Disabled reflection control still executed a reflection pass");
                check(off.render.image != reflectionMetal.image, "GPU reflection effect-off control did not change RGB");
                sameSurface(off.render, reflectionMetal);
                restored();
            }
            // Transmission restricted to dIFS surfaces remains unsupported:
            // rejection is pre-dispatch and the ordinary Native CPU path
            // receives the complete original job.
            auto transparency = load(reflectionPath);
            transparency.header.bCalcSRautomatic = 7;
            auto transparencyCPUInput = transparency;
            backend.requested = RenderBackend::CPU;
            const auto transparencyCPU = RenderWithBackend(transparencyCPUInput.header, transparencyCPUInput.addon, options, backend);
            for (const auto requested : {RenderBackend::Metal, RenderBackend::Auto}) {
                backend.requested = requested;
                auto input = transparency;
                const auto result = RenderWithBackend(input.header, input.addon, options, backend);
                check(result.backend.effective == RenderBackend::CPU && result.backend.fallback &&
                      result.backend.fallback_code == (hasMetal ? "unsupported_scene" : "metal_not_built"),
                      "dIFS-only transmission was accidentally admitted by a GPU pass");
                sameImage(result.render, transparencyCPU.render);
                restored();
            }

            const auto scene = scenes / "sphereifs-ao-shadows.m3p";
            // Invalid API arithmetic requests must not render or mutate input.
            for (const auto requested : {RenderBackend::Metal, RenderBackend::Auto}) {
                auto input = load(scene);
                const InputSnapshot before(input);
                auto incompatible = options;
                incompatible.precision = PrecisionMode::Compatible;
                backend.requested = requested;
                auto rejected = RenderWithBackend(input.header, input.addon, incompatible, backend);
                check(!rejected.render.success && rejected.render.status == RenderStatus::Unsupported &&
                      rejected.render.image.empty() && !rejected.backend.fallback,
                      "API silently converted explicit Compatible request to Native");
                before.unchanged(input);
                restored();
            }

            if (hasMetal) {
                // An existing invalid library is a hard failure. In particular,
                // neither successful-header commits nor CPU fallback may occur.
                backend.metal_library = (source / "README.md").string();
                for (const auto requested : {RenderBackend::Metal, RenderBackend::Auto}) {
                    backend.requested = requested;
                    auto input = load(scene);
                    input.header.iOptions |= 1;
                    input.header.bCalcAmbShadowAutomatic |= 128u;
                    const InputSnapshot before(input);
                    auto failed = RenderWithBackend(input.header, input.addon, options, backend);
                    check(!failed.render.success && failed.render.status == RenderStatus::Failed &&
                          failed.backend.effective == RenderBackend::Metal && !failed.backend.fallback,
                          "invalid existing library was hidden by CPU fallback");
                    check(failed.render.image.empty() && failed.render.silight.empty(),
                          "failed GPU library published partial output");
                    check(failed.render.profile.precision == PrecisionMode::Native,
                          "GPU failure profile inherited caller/build-default precision");
                    before.unchanged(input);
                    restored();
                }
                backend.metal_library = library;
            }

            // The presence of a control forces CPU accounting before GPU work.
            // Cancellation and exhaustion must publish nothing, then a new job
            // must recover without inheriting failed resources or policy state.
            for (const auto requested : {RenderBackend::Metal, RenderBackend::Auto}) {
                backend.requested = requested;
                for (bool cancelled : {true, false}) {
                    auto input = load(scene);
                    const InputSnapshot before(input);
                    auto controlled = options;
                    controlled.control = std::make_shared<RenderControl>(cancelled ? 0 : 1);
                    if (cancelled) controlled.control->cancel();
                    auto stopped = RenderWithBackend(input.header, input.addon, controlled, backend);
                    const auto expected = cancelled ? RenderStatus::Cancelled : RenderStatus::WorkExhausted;
                    check(stopped.backend.effective == RenderBackend::CPU && stopped.backend.fallback &&
                          stopped.backend.fallback_code == "unsupported_options",
                          "controlled render was sent to an uncounted GPU path");
                    check(!stopped.render.success && stopped.render.status == expected &&
                          stopped.render.image.empty() && stopped.render.silight.empty(),
                          "stopped fallback published output or changed lifecycle status");
                    check(controlled.control->status() == expected, "fallback lost control terminal status");
                    check(stopped.render.profile.precision == PrecisionMode::Native, "failed fallback lost Native policy");
                    if (cancelled) before.unchanged(input);
                    restored();
                }
                auto input = load(scene);
                auto unlimited = options;
                unlimited.control = std::make_shared<RenderControl>(0);
                auto completed = RenderWithBackend(input.header, input.addon, unlimited, backend);
                check(completed.backend.fallback && completed.backend.fallback_code == "unsupported_options",
                      "unlimited control bypassed CPU accounting");
                check(unlimited.control->status() == RenderStatus::Completed && unlimited.control->work_used() > 0,
                      "unlimited fallback did not account work");
                sameImage(completed.render, firstSceneCPU);
                restored();
                input = load(scene);
                auto recovered = RenderWithBackend(input.header, input.addon, options, backend);
                sameImage(recovered.render, hasMetal ? firstSceneMetal : firstSceneCPU);
                restored();
            }

            // Profiling is not what activates the arithmetic policy.
            auto input = load(scene);
            auto unprofiled = options;
            unprofiled.profile_timing = false;
            backend.requested = RenderBackend::Metal;
            auto plain = RenderWithBackend(input.header, input.addon, unprofiled, backend);
            sameImage(plain.render, hasMetal ? firstSceneMetal : firstSceneCPU);
            check(plain.render.profile.precision == PrecisionMode::Native, "unprofiled backend used build-default precision");
            restored();

            // Separate jobs may choose opposite arithmetic simultaneously. Use
            // the same scene so geometry changes cannot disguise policy leaks.
            auto compatibleOptions = options;
            compatibleOptions.precision = PrecisionMode::Compatible;
            compatibleOptions.control = std::make_shared<RenderControl>(0);
            BackendOptions compatibleBackend;
            compatibleBackend.requested = RenderBackend::CPU;
            auto compatibleInput = load(scene);
            auto compatibleBaseline = RenderWithBackend(compatibleInput.header, compatibleInput.addon,
                                                         compatibleOptions, compatibleBackend);
            check(compatibleBaseline.render.success &&
                  compatibleBaseline.render.profile.precision == PrecisionMode::Compatible,
                  "sequential Compatible CPU concurrency reference failed");
            check(compatibleOptions.control->status() == RenderStatus::Completed,
                  "sequential Compatible CPU reference lost its own control");
            restored();
            const auto sourceInput = load(scene);
            const InputSnapshot sourceBefore(sourceInput);
            struct IsolatedJob {
                BackendRenderResult result;
                RenderContextHandle caller;
                RenderControlHandle job_control;
            };
            for (const auto requested : {RenderBackend::Metal, RenderBackend::Auto}) {
                std::latch ready(2), start(1);
                auto isolated = [&](RenderOptions jobOptions, BackendOptions jobBackend) {
                    ready.count_down();
                    start.wait();
                    const auto initialPrecision = CurrentPrecisionMode();
                    const auto initialContext = CaptureRenderContext();
                    auto* const initialControl = current_render_control;
                    IsolatedJob completed;
                    completed.caller = std::make_shared<RenderContext>();
                    completed.caller->precision = jobOptions.precision == PrecisionMode::Native
                        ? PrecisionMode::Compatible : PrecisionMode::Native;
                    completed.caller->control = std::make_shared<RenderControl>();
                    // Native GPU must remain eligible; the simultaneous CPU job
                    // gets a separate accounting owner, never the caller's one.
                    if (jobBackend.requested == RenderBackend::CPU) {
                        jobOptions.control = std::make_shared<RenderControl>(0);
                        completed.job_control = jobOptions.control;
                    } else {
                        jobOptions.control.reset();
                    }
                    {
                        ScopedRenderContext binding(completed.caller);
                        auto localInput = sourceInput;
                        const InputSnapshot localBefore(localInput);
                        completed.result = RenderWithBackend(localInput.header, localInput.addon,
                                                             jobOptions, jobBackend);
                        check(CurrentPrecisionMode() == completed.caller->precision,
                              "concurrent job leaked its arithmetic into worker caller TLS");
                        check(CaptureRenderContext() == completed.caller &&
                              current_render_control == completed.caller->control.get(),
                              "concurrent job failed to restore worker caller context/control");
                        check(completed.caller->control->status() == RenderStatus::Prepared &&
                              completed.caller->control->work_used() == 0,
                              "concurrent job charged unrelated caller control");
                        if (hasMetal && jobBackend.requested != RenderBackend::CPU)
                            localBefore.committed(localInput, jobOptions);
                        if (completed.job_control)
                            check(completed.job_control->status() == RenderStatus::Completed &&
                                  completed.job_control->work_used() > 0,
                                  "concurrent CPU job lost its independent accounting");
                    }
                    check(CurrentPrecisionMode() == initialPrecision && CaptureRenderContext() == initialContext &&
                          current_render_control == initialControl,
                          "concurrent job leaked bindings after its caller scope");
                    return completed;
                };
                BackendOptions nativeBackend;
                nativeBackend.requested = requested;
                nativeBackend.metal_library = library;
                auto nativeJob = std::async(std::launch::async, isolated, options, nativeBackend);
                auto compatibleJob = std::async(std::launch::async, isolated, compatibleOptions, compatibleBackend);
                ready.wait();
                start.count_down();
                restored();
                auto nativeResult = nativeJob.get();
                auto compatibleResult = compatibleJob.get();
                check(nativeResult.caller != compatibleResult.caller &&
                      nativeResult.caller->control != compatibleResult.caller->control &&
                      compatibleResult.job_control != compatibleResult.caller->control &&
                      compatibleResult.job_control != compatibleOptions.control,
                      "concurrent jobs shared mutable context/control owners");
                sameImage(nativeResult.result.render, hasMetal ? firstSceneMetal : firstSceneCPU);
                sameImage(compatibleResult.result.render, compatibleBaseline.render);
                check(nativeResult.result.render.profile.precision == PrecisionMode::Native &&
                      compatibleResult.result.render.profile.precision == PrecisionMode::Compatible,
                      "concurrent results lost their independent precision profiles");
                check(compatibleResult.result.backend.effective == RenderBackend::CPU &&
                      !compatibleResult.result.backend.fallback,
                      "concurrent Compatible request did not remain explicit CPU");
                if (hasMetal) {
                    check(nativeResult.result.backend.effective == RenderBackend::Metal &&
                          !nativeResult.result.backend.fallback, "concurrent Native request unexpectedly fell back");
                    sameSurface(nativeResult.result.render, firstSceneMetal);
                } else {
                    check(nativeResult.result.backend.effective == RenderBackend::CPU &&
                          nativeResult.result.backend.fallback_code == "metal_not_built",
                          "concurrent no-Metal job lost fallback provenance");
                }
                sourceBefore.unchanged(sourceInput);
                restored();
            }
        }
        check(CurrentPrecisionMode() == originalPrecision, "API test leaked precision after caller scope");
        check(CaptureRenderContext() == originalContext, "API test leaked caller context after scope");
        check(current_render_control == originalControl, "API test leaked caller control after scope");
        std::printf("Backend API: %u checks passed; Metal build=%s; in-process repeats, concurrent isolated jobs, controls and TLS isolation.\n",
                    checks.load(std::memory_order_relaxed), hasMetal ? "yes" : "no");
        return 0;
    } catch (const std::exception& error) {
        std::fprintf(stderr, "Backend API failed after %u checks: %s\n", checks.load(std::memory_order_relaxed), error.what());
        return 1;
    }
}
