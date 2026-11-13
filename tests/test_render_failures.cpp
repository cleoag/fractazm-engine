#include "core/renderer.h"
#include "io/m3p_parser.h"
#include "mb3d/mb3d.h"

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

namespace fs = std::filesystem;
int checks = 0;

void check(bool value, const std::string& message) {
    ++checks;
    if (!value) throw std::runtime_error(message);
}

struct Scene {
    TMandHeader10 header;
    THeaderCustomAddon addon;
};

Scene load(const fs::path& path) {
    const auto loaded = mb3d::LoadM3PFile(path.string());
    check(loaded.success, "Cannot load fixture " + path.string() + ": " + loaded.error);
    Scene scene{loaded.header, loaded.addon};
    scene.header.Width = 16;
    scene.header.Height = 12;
    scene.header.Iterations = 8;
    return scene;
}

void name(THAformula& formula, const char* value) {
    std::memset(formula.CustomFname, 0, sizeof(formula.CustomFname));
    std::memcpy(formula.CustomFname, value,
        std::min(std::strlen(value), sizeof(formula.CustomFname)));
}

mb3d::RenderOptions options(int threads) {
    mb3d::RenderOptions opts;
    opts.width_override = 16;
    opts.height_override = 12;
    opts.thread_count = threads;
    opts.no_shadows = opts.no_ao = opts.no_dof = opts.no_reflections = true;
    return opts;
}

mb3d::RenderResult render(Scene scene, int threads) {
    return mb3d::Render(scene.header, scene.addon, options(threads));
}

mb3d::RenderResult repaint(Scene scene, const std::vector<TsiLight5>& silight,
                          int threads) {
    return mb3d::PaintFromSiLight(scene.header, scene.addon, silight, options(threads));
}

void successful(const mb3d::RenderResult& result, const std::string& where) {
    check(result.success && result.error.empty(), where + ": " + result.error);
    check(result.width == 16 && result.height == 12 && result.image.size() == 16 * 12 * 3 &&
          result.silight.size() == 16 * 12, where + ": incomplete successful surface");
}

void emptyFailure(const mb3d::RenderResult& result, const std::string& expected,
                  const std::string& where) {
    check(!result.success, where + ": unsupported input succeeded");
    check(result.error.find(expected) != std::string::npos,
          where + ": wrong diagnostic: " + result.error);
    check(result.width == 0 && result.height == 0 && result.image.empty() &&
          result.silight.empty() && result.diagnostics.empty() &&
          result.ray_setup_records.empty() && result.parity_trace_lines.empty(),
          where + ": failed render leaked a partial surface or diagnostic output");
}

struct ApiResult {
    MB3DRenderResult value;
    explicit ApiResult(const fs::path& path, int threads) {
        MB3DRenderOptions opts{};
        opts.width = 16;
        opts.height = 12;
        opts.thread_count = threads;
        opts.max_iterations = 8;
        opts.no_shadows = opts.no_ao = opts.no_dof = opts.no_reflections = 1;
        value = mb3d_render_file(path.string().c_str(), &opts);
    }
    ~ApiResult() { mb3d_free(value.pixels); mb3d_free(value.zbuffer); }
    ApiResult(const ApiResult&) = delete;
    ApiResult& operator=(const ApiResult&) = delete;
};

void apiSuccessful(const ApiResult& result, const std::vector<uint8_t>& pixels,
                   const std::string& where) {
    const auto& value = result.value;
    check(value.success && value.error[0] == '\0', where + ": " + value.error);
    check(value.width == 16 && value.height == 12 && value.pixels && value.zbuffer,
          where + ": incomplete successful C surface");
    check(std::memcmp(value.pixels, pixels.data(), pixels.size()) == 0,
          where + ": C API differs from public Render control");
}

void recovery(const Scene& control, const fs::path& controlPath, int threads,
              const mb3d::RenderResult& baseline, const std::string& where) {
    const auto recovered = render(control, threads);
    successful(recovered, where + " public recovery");
    check(recovered.image == baseline.image &&
          std::memcmp(recovered.silight.data(), baseline.silight.data(),
                      baseline.silight.size() * sizeof(TsiLight5)) == 0,
          where + ": public recovery differs from its same-thread control");
    ApiResult api(controlPath, threads);
    apiSuccessful(api, baseline.image, where + " C API recovery");
    for (size_t i = 0; i < baseline.silight.size(); ++i)
        check(api.value.zbuffer[i] == baseline.silight[i].Zpos,
              where + ": recovered C depth differs from public control");
}

struct FailureCase {
    std::string id;
    Scene scene;
    std::string diagnostic;
};

} // namespace

int main(int argc, char** argv) {
    try {
        if (argc != 3) throw std::runtime_error("usage: test_render_failures <source-root> <scratch-dir>");
        const fs::path fixtures = fs::path(argv[1]) / "tests/fixtures/render_failures";
        const fs::path scratch = fs::absolute(argv[2]);
        fs::create_directories(scratch);
        const auto control = load(fixtures / "challenge_TrifoxComplexAngles_early.m3p");
        const auto controlPath = scratch / "control.m3p";
        check(mb3d::SaveM3PFile(controlPath.string(), control.header, control.addon), "Cannot save control");

        std::vector<FailureCase> failures;
        for (const char* formula : {"gnarlyIFS", "gnarly2IFS", "gnarly3IFS", "ducksIFS"}) {
            const auto id = std::string("challenge_") + formula + "_p8_alternate";
            failures.push_back({id, load(fixtures / (id + ".m3p")),
                "FORMULA_INPUT_REJECTED[caller_dependent_flip]"});
        }
        for (int option : {0, 3}) {
            const auto id = "challenge_TrifoxComplexAngles_p" + std::to_string(option) + "_zero";
            failures.push_back({id, load(fixtures / (id + ".m3p")),
                "FORMULA_INPUT_REJECTED[nonterminating_zero_power]"});
        }
        const auto warp = load(fixtures / "challenge__GenCWarp_early.m3p");
        for (int option : {5, 6}) {
            for (int value : {(std::numeric_limits<int>::min)(), -1, 24,
                              (std::numeric_limits<int>::max)()}) {
                auto invalid = warp;
                // Matrix expansion uses nine runtime entries for three stored
                // angles; keep all sixteen expanded option entries present.
                invalid.addon.Formulas[0].iOptionCount = 16;
                invalid.addon.Formulas[0].dOptionValue[option] = value;
                failures.push_back({"gencwarp_" + std::to_string(option) + "_" + std::to_string(value),
                    invalid, "FORMULA_INPUT_REJECTED[uninitialized_angle_stack]"});
            }
        }
        auto unknown = control;
        // A full 32-byte field deliberately has no NUL terminator.
        name(unknown.addon.Formulas[0], "not_a_registered_external_formula");
        failures.push_back({"unknown_active", unknown,
            "FORMULA_INPUT_REJECTED[unknown_external_formula]"});

        auto invalidMix = load(fs::path(argv[1]) /
            "comparison/scenes_phase3_decomb/phase3_decomb_intpow-bulbox_v0_medium.m3p");
        invalidMix.addon.bOptions3 = 5;
        failures.push_back({"invalid_de_mix_non_difs_second", invalidMix,
            "FORMULA_INPUT_REJECTED[invalid_de_mix]"});

        // A real conditional callback turns off the initially counted partner.
        // Slot 0 still runs, so this tests a worker's no-progress exception rather
        // than a preparation-time validation error or a static empty-slot check.
        auto guarded = control;
        guarded.addon.bOptions1 = 0;
        guarded.addon.Formulas[1] = guarded.addon.Formulas[0];
        auto& conditional = guarded.addon.Formulas[0];
        conditional = THAformula{};
        conditional.iFnr = 20;
        conditional.iItCount = 1;
        conditional.iOptionCount = 5;
        name(conditional, "_CondItcountSwitch");
        conditional.dOptionValue[0] = 1;
        conditional.dOptionValue[1] = 2;
        failures.push_back({"conditional_disables_counted_partner", guarded,
            "Formula iteration failed: no-progress work budget exhausted"});

        // A single callback can exhaust its internal budget even though the
        // surrounding hybrid dispatcher keeps normal counted-iteration limits.
        // Exercise the real native helper, public renderer and C ABI recovery.
        auto longFormula = load(fixtures / "challenge_Knots2IFS_early.m3p");
        longFormula.addon.Formulas[0].dOptionValue[11] =
            (std::numeric_limits<int32_t>::max)();
        failures.push_back({"knots_internal_work_exhausted", longFormula,
            "Formula work budget exhausted"});

        auto dormant = control;
        dormant.addon.Formulas[5] = unknown.addon.Formulas[0];
        dormant.addon.Formulas[5].iItCount = 0;
        const auto dormantPath = scratch / "unknown_dormant.m3p";
        check(mb3d::SaveM3PFile(dormantPath.string(), dormant.header, dormant.addon), "Cannot save dormant control");

        for (int threads : {1, 4}) {
            const auto baseline = render(control, threads);
            successful(baseline, "public baseline");
            const auto paintBaseline = repaint(control, baseline.silight, threads);
            successful(paintBaseline, "paint replay baseline");
            recovery(control, controlPath, threads, baseline, "initial baseline");
            const auto dormantResult = render(dormant, threads);
            successful(dormantResult, "dormant unknown");
            check(dormantResult.image == baseline.image, "dormant unknown changed the image");
            ApiResult dormantApi(dormantPath, threads);
            apiSuccessful(dormantApi, baseline.image, "dormant unknown C API");

            for (const auto& failure : failures) {
                const auto tag = failure.id + ", threads=" + std::to_string(threads);
                const auto result = render(failure.scene, threads);
                emptyFailure(result, failure.diagnostic, tag + " public");
                recovery(control, controlPath, threads, baseline, tag + " after public failure");
                // Paint replay is a separate public entry point. Reuse a valid
                // surface so an invalid formula cannot be hidden by shape
                // validation. These cases fail during preparation; conditional
                // and inner-loop failures require orbit work, which is disabled
                // by this paint-only control's options.
                if (failure.diagnostic.rfind("FORMULA_INPUT_REJECTED[", 0) == 0) {
                    const auto painted = repaint(failure.scene, baseline.silight, threads);
                    emptyFailure(painted, failure.diagnostic, tag + " paint replay");
                    const auto paintRecovered = repaint(control, baseline.silight, threads);
                    successful(paintRecovered, tag + " paint replay recovery");
                    check(paintRecovered.image == paintBaseline.image &&
                          std::memcmp(paintRecovered.silight.data(), paintBaseline.silight.data(),
                                      paintBaseline.silight.size() * sizeof(TsiLight5)) == 0,
                          tag + ": paint replay recovery differs from its same-thread control");
                }
                const auto path = scratch / (failure.id + ".m3p");
                check(mb3d::SaveM3PFile(path.string(), failure.scene.header, failure.scene.addon),
                      tag + ": cannot save fixture");
                ApiResult api(path, threads);
                check(!api.value.success && !api.value.pixels && !api.value.zbuffer &&
                      api.value.width == 0 && api.value.height == 0,
                      tag + ": C API returned a partial failed surface");
                check(std::string(api.value.error).find(failure.diagnostic) != std::string::npos,
                      tag + ": wrong C API diagnostic: " + api.value.error);
                recovery(control, controlPath, threads, baseline, tag + " after C API failure");
                std::printf("Error/recovery contract OK: %s\n", tag.c_str());
            }
        }
        std::printf("Public Render, PaintFromSiLight and C API failure/recovery: %d checks passed; not numerical parity evidence.\n", checks);
        return 0;
    } catch (const std::exception& error) {
        std::fprintf(stderr, "Render failure/recovery test failed after %d checks: %s\n", checks, error.what());
        return 1;
    }
}
