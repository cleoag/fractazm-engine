#include "native_reflection_host.h"
#include "scene_host_internal.h"
#include "io/m3p_parser.h"
#include <bit>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <limits>
#include <stdexcept>

using namespace mb3d;
using namespace mb3d::metal_exact;

int main(int argc, char** argv) {
    if (argc != 2) {
        std::fprintf(stderr, "usage: test_native_reflection_host integerpower8-reflections.m3p\n");
        return 2;
    }
    unsigned checks = 0, failures = 0;
    auto check = [&](bool okay, const char* description) {
        ++checks;
        if (!okay && ++failures <= 20) std::fprintf(stderr, "FAIL: %s\n", description);
    };
    auto rejected = [&](auto&& operation, const char* description) {
        bool failed = false;
        try { operation(); }
        catch (const std::invalid_argument&) { failed = true; }
        check(failed, description);
    };
    try {
        const ScopedNumericPrecision caller(PrecisionMode::Compatible);
        const auto caller_context = CaptureRenderContext();
        auto file = LoadM3PFile(argv[1]);
        if (!file.success) throw std::runtime_error(file.error);
        const auto original_header = file.header;
        const auto original_addon = file.addon;
        check(file.header.bCalcSRautomatic == 1 && file.header.SRreflectioncount == 2 &&
              file.header.SRamount == 0.8f, "pinned opaque two-bounce fixture");
        const auto scene = PrepareLoadedExactScene(file.header, file.addon, 64, 48, "",
                                                  65536, PrecisionMode::Native);
        const auto params = PrepareNativeReflection(scene);
        check(params.version == 1 && params.width == 64 && params.height == 48,
              "native loaded scene descriptor and dimensions");
        check(params.max_bounces == 2 && params.max_steps == 65536 &&
              params.ao_enabled == 1 && params.shadows_enabled == 1 && params.vary_de_stop == 1,
              "reflection, secondary AO/shadow and varying threshold enabled");
        check(params.amount == std::bit_cast<unsigned>(0.8f), "reflection amount retained");
        check(params.ao[0].ray_count == 7 && params.ao[1].ray_count == 3,
              "bounce-dependent point AO: seven then three directions");
        check(std::bit_cast<float>(params.ao[0].correction_weight) == 0.1666f &&
              std::bit_cast<float>(params.ao[1].correction_weight) == 0.2f,
              "point AO overlap correction depends on quality");
        // The fixture DEAOmaxL=1 and the 64x48 diagonal is80: maxdistance=40.
        check(std::bit_cast<float>(params.ao[0].max_distance_base) == 40.0f &&
              params.ao[0].max_distance_base == params.ao[1].max_distance_base,
              "point AO distance prepared from overridden scene dimensions");
        check(params.ao[2].ray_count == 3 && std::bit_cast<float>(params.ao[2].step_multiplier) == 1.8f &&
              std::bit_cast<float>(params.ao[2].correction_weight) == 0.3f &&
              std::bit_cast<float>(params.ao[2].minimum_overlap_dot) == -1.0f &&
              params.ao[2].max_distance_base == params.ao[0].max_distance_base &&
              std::bit_cast<float>(params.ao[2].directions[0][2]) == -std::cos(0.5f * 60.0f * float(mb3d::Pid180)),
              "later bounces use CPU's fixed three-ray quality-0 cone");
        check(params.first_step_random == 0, "exact reflection descriptor has no first-step jitter");
        for (const auto word : {params.step_width, params.fov_y, params.z_corr,
                                params.z_c_mul, params.back_distance})
            check(std::isfinite(std::bit_cast<float>(word)) && std::bit_cast<float>(word) > 0,
                  "finite positive geometry scalar");
        check(std::isfinite(std::bit_cast<float>(params.z_mid_difference)), "finite signed depth offset");
        for (const auto& ao : params.ao) {
            check(ao.reserved == 0, "reserved AO word zero");
            check(std::bit_cast<float>(ao.step_multiplier) > 1 &&
                  std::bit_cast<float>(ao.de_multiplier) > 1 &&
                  std::isfinite(std::bit_cast<float>(ao.overlap_angle_scale)),
                  "finite positive point AO marching coefficients");
            for (unsigned ray = 0; ray < 8; ++ray) {
                double square_length = 0;
                for (unsigned axis = 0; axis < 3; ++axis) {
                    const float value = std::bit_cast<float>(ao.directions[ray][axis]);
                    check(std::isfinite(value), "finite point AO direction");
                    square_length += double(value) * value;
                    if (ray >= ao.ray_count)
                        check(ao.directions[ray][axis] == 0, "unused point AO direction zero");
                }
                check(ao.directions[ray][3] == 0, "point AO padding zero");
                if (ray < ao.ray_count) {
                    check(std::fabs(square_length - 1) < 2e-6, "unit point AO direction");
                    check(std::bit_cast<float>(ao.directions[ray][2]) < 0,
                          "point AO samples the negative-z hemisphere");
                }
            }
        }
        for (unsigned row = 0; row < 3; ++row) {
            double view_length = 0, ao_length = 0;
            for (unsigned column = 0; column < 3; ++column) {
                const float view = std::bit_cast<float>(params.view_matrix[row][column]);
                const float ao = std::bit_cast<float>(params.ao_matrix[row][column]);
                check(std::isfinite(view) && std::isfinite(ao), "finite camera/AO matrix");
                view_length += double(view) * view;
                ao_length += double(ao) * ao;
            }
            const double step = std::bit_cast<float>(params.step_width);
            check(std::fabs(view_length - 1) < 2e-6, "normalized view matrix");
            check(std::fabs(ao_length / (step * step) - 1) < 2e-6, "AO matrix retains scene step width");
            check(params.view_matrix[row][3] == 0 && params.ao_matrix[row][3] == 0,
                  "matrix padding zero");
        }
        check(CurrentPrecisionMode() == PrecisionMode::Compatible &&
              CaptureRenderContext() == caller_context, "preparation restores caller precision/context");
        check(std::memcmp(&file.header, &original_header, sizeof(file.header)) == 0 &&
              std::memcmp(&file.addon, &original_addon, sizeof(file.addon)) == 0,
              "loaded scene preparation preserves caller records");

        // Clone only scene-owned metadata; no geometry/render invocation or
        // shared mutable header is needed for fail-closed host admission tests.
        auto changed = [&]() {
            auto copy = scene;
            copy.host = std::make_shared<SceneHostState>(*scene.host);
            return copy;
        };
        auto disabled_header = file.header;
        disabled_header.bCalcSRautomatic = 0;
        const auto disabled = PrepareLoadedExactScene(disabled_header, file.addon, 64, 48, "",
                                                     65536, PrecisionMode::Compatible);
        const auto disabled_params = PrepareNativeReflection(disabled);
        const NativeReflectionParams empty{};
        check(std::memcmp(&disabled_params, &empty, sizeof(empty)) == 0,
              "disabled reflections have a canonical all-zero descriptor");
        for (unsigned count : {1u, 2u}) {
            auto copy = changed(); copy.host->file.header.SRreflectioncount = count;
            check(PrepareNativeReflection(copy).max_bounces == count, "one/two-bounce opaque scenes admitted");
        }
        {
            auto copy = changed(); copy.host->file.header.SRamount = 0;
            check(PrepareNativeReflection(copy).amount == 0, "zero-amount activity control admitted");
        }
        {
            auto copy = changed(); copy.precision = PrecisionMode::Compatible;
            rejected([&] { (void)PrepareNativeReflection(copy); }, "active compatible reflections rejected");
        }
        for (unsigned count : {0u, 3u, 255u}) {
            auto copy = changed(); copy.host->file.header.SRreflectioncount = count;
            rejected([&] { (void)PrepareNativeReflection(copy); }, "unsupported bounce count rejected");
        }
        for (unsigned flags : {3u, 7u, 9u, 129u}) {
            auto copy = changed(); copy.host->file.header.bCalcSRautomatic = flags;
            rejected([&] { (void)PrepareNativeReflection(copy); }, "transmission/unknown reflection flags rejected");
        }
        {
            // Only-dIFS selects transmitting surfaces; it is inert without transmission.
            auto copy = changed(); copy.host->file.header.bCalcSRautomatic = 5;
            const auto onlyDIFS = PrepareNativeReflection(copy), plain = PrepareNativeReflection(changed());
            check(std::memcmp(&onlyDIFS, &plain, sizeof(plain)) == 0, "only-dIFS flag changed opaque reflections");
        }
        for (float amount : {-0.1f, 1.1f, std::numeric_limits<float>::infinity(),
                             std::numeric_limits<float>::quiet_NaN()}) {
            auto copy = changed(); copy.host->file.header.SRamount = amount;
            rejected([&] { (void)PrepareNativeReflection(copy); }, "invalid reflection amount rejected");
        }
        for (unsigned kind : {0u, 1u, 2u, 4u}) {
            auto copy = changed(); copy.device.formula_kind = kind;
            rejected([&] { (void)PrepareNativeReflection(copy); }, "unsupported reflection formula rejected");
        }
        for (unsigned power : {1u, 2u, 3u, 4u, 5u, 6u, 7u, 9u}) {
            auto copy = changed(); copy.device.bulb.power = power;
            rejected([&] { (void)PrepareNativeReflection(copy); }, "unsupported internal power rejected");
        }
        for (unsigned bound : {0u, 65537u})
            rejected([&] { (void)PrepareNativeReflection(scene, bound); }, "invalid bounded work rejected");
        {
            auto copy = changed(); copy.host->mct.bInsideRendering = 1;
            rejected([&] { (void)PrepareNativeReflection(copy); }, "inside reflection rejected");
        }
        {
            auto copy = changed(); copy.host->mct.iCutOptions = 1;
            rejected([&] { (void)PrepareNativeReflection(copy); }, "cut-plane reflection rejected");
        }
        {
            auto copy = changed(); copy.host->mct.DEAOmaxL = 0;
            rejected([&] { (void)PrepareNativeReflection(copy); }, "invalid point AO range rejected");
        }
        rejected([&] { (void)PrepareExactScene(argv[1], 64, 48, 65536, false, PrecisionMode::Native); },
                 "standalone Native preparation still rejects unimplemented reflection dispatch");
        rejected([&] { (void)PrepareExactScene(argv[1], 64, 48); },
                 "standalone exact qualification does not inherit Native reflections");
        check(CurrentPrecisionMode() == PrecisionMode::Compatible && CaptureRenderContext() == caller_context,
              "all rejection paths restore caller precision/context");
    } catch (const std::exception& error) {
        check(false, error.what());
    }
    std::printf("native reflection host: %u checks, %u failures\n", checks, failures);
    return failures ? 1 : 0;
}
