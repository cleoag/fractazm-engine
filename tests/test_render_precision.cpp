#include "core/renderer.h"
#include "core/render_context.h"
#include "core/thread_pool.h"
#include "core/ambient_direction_table.h"
#include "core/external_formulas.h"
#include "io/m3p_parser.h"
#include "mb3d/mb3d.h"
#include <atomic>
#include <cstring>
#include <filesystem>
#include <future>
#include <stdexcept>
#include <cstdio>
#include <bit>
#include <cmath>

#ifdef MB3D_SOFTWARE_EXT80
namespace mb3d {
#include "core/formula_iteration_rotation.h"
}
#endif

namespace {
unsigned checks = 0;
void check(bool value, const char* message) {
    ++checks;
    if (!value) throw std::runtime_error(message);
}
mb3d::RenderResult render(const std::filesystem::path& scene, mb3d::PrecisionMode mode, int workers) {
    auto input = mb3d::LoadM3PFile(scene.string());
    if (!input.success) throw std::runtime_error(input.error);
    mb3d::RenderOptions options;
    options.precision = mode;
    options.width_override = 64;
    options.height_override = 48;
    options.thread_count = workers;
    options.deterministic_mode = true;
    options.profile_timing = true;
    options.control = std::make_shared<mb3d::RenderControl>(20000000);
    return mb3d::Render(input.header, input.addon, options);
}
void equal(const mb3d::RenderResult& actual, const mb3d::RenderResult& reference,
           mb3d::PrecisionMode mode) {
    check(actual.success && reference.success, "render did not complete");
    check(actual.width == reference.width && actual.height == reference.height,
          "render dimensions changed");
    check(actual.image == reference.image, "profile/worker isolation changed RGB");
    check(actual.profile.precision == mode, "effective precision missing from result");
}
void preparationPolicies() {
    using namespace mb3d;
    detail::AmbientDirections expected[2];
    for(unsigned index=0;index<2;++index) {
        const auto mode=index?PrecisionMode::Native:PrecisionMode::Compatible;
        const ScopedNumericPrecision precision(mode);
        for(int direction=0;direction<32;++direction) {
            const double angle=double(direction)*Pi/16.0;
            double sine,cosine;SinCosD(angle,sine,cosine);
            expected[index][direction]={float(sine),float(cosine)};
            if(index) {
                check(std::bit_cast<uint64_t>(sine)==std::bit_cast<uint64_t>(std::sin(angle)),"native shared sine policy");
                check(std::bit_cast<uint64_t>(cosine)==std::bit_cast<uint64_t>(std::cos(angle)),"native shared cosine policy");
            }
        }
        THeaderCustomAddon addon{};
        auto& formula=addon.Formulas[0];formula.iFnr=0;formula.iOptionCount=1;
        formula.dOptionValue[0]=0x1p70;
        const double radians=formula.dOptionValue[0]*Pid180;
        for(unsigned type:{3u,4u}) {
            formula.byOptionType[0]=uint8_t(type);BuildFormulaConstBuffer(0,&addon,0);
            auto* constants=static_cast<const uint8_t*>(GetFormulaConstBuffers().pConstPointer[0]);
            if(type==3) {
                double sine,cosine;std::memcpy(&sine,constants-16,8);std::memcpy(&cosine,constants-24,8);
                check(sine==(index?std::sin(radians):radians),"double angle follows selected out-of-range policy");
                check(cosine==(index?std::cos(radians):radians),"double cosine follows selected out-of-range policy");
            } else {
                float sine,cosine;std::memcpy(&sine,constants-12,4);std::memcpy(&cosine,constants-16,4);
                check(sine==float(index?std::sin(radians):radians),"single angle follows selected out-of-range policy");
                check(cosine==float(index?std::cos(radians):radians),"single cosine follows selected out-of-range policy");
            }
        }
#ifdef MB3D_SOFTWARE_EXT80
        double parameters[MAX_FORMULA_PARAMS]{};
        parameters[0]=parameters[4]=parameters[8]=parameters[10]=1;
        parameters[9]=0x1p64;
        const auto rotated=formulaIterationRotation(cpu_double3(1,0,0),parameters,false);
        check(parameters[BFIRSTIT_SLOT]==1,"rotation preserves persistent counter in both profiles");
        check(index ? rotated.x==std::cos(0x1p64) && rotated.y==-std::sin(0x1p64)
                    : std::isnan(rotated.x) && std::isnan(rotated.y),
              "native rotation bypasses compatible missing-push behavior");
#endif
    }
    for(unsigned first:{0u,1u}) {
        detail::AmbientDirectionTables tables;
        for(unsigned index:{first,1u-first,first,1u-first}) {
            const auto& actual=tables.get(index?PrecisionMode::Native:PrecisionMode::Compatible);
            check(std::memcmp(&actual,&expected[index],sizeof(actual))==0,
                  "AO direction cache depends on requested profile, not initialization order");
        }
    }
    detail::AmbientDirectionTables concurrent;
    auto compatible=std::async(std::launch::async,[&]{return concurrent.get(PrecisionMode::Compatible);});
    auto native=std::async(std::launch::async,[&]{return concurrent.get(PrecisionMode::Native);});
    const auto c=compatible.get(),n=native.get();
    check(std::memcmp(&c,&expected[0],sizeof(c))==0 && std::memcmp(&n,&expected[1],sizeof(n))==0,
          "concurrent AO profile table initialization remains isolated");
}
}

int main(int argc, char** argv) {
    try {
        if (argc != 2) throw std::runtime_error("source root required");
        using mb3d::PrecisionMode;
        const auto initial = mb3d::CurrentPrecisionMode();
        check(mb3d::RenderOptions{}.precision==mb3d::DefaultPrecisionMode(),"render options follow configured build default");
        preparationPolicies();
        for (auto mode : {PrecisionMode::Compatible, PrecisionMode::Native}) {
            auto context = std::make_shared<mb3d::RenderContext>();
            context->precision = mode;
            mb3d::ScopedRenderContext binding(context);
            std::atomic<unsigned> errors{0};
            mb3d::RunParallelWorkers(4, [&](int) {
                if (mb3d::CurrentPrecisionMode() != mode) ++errors;
            });
            mb3d::ThreadPool pool(2);
            for (int i = 0; i < 12; ++i) pool.submit([&] {
                if (mb3d::CurrentPrecisionMode() != mode) ++errors;
                mb3d::ScopedNumericPrecision nested(mode == PrecisionMode::Native
                    ? PrecisionMode::Compatible : PrecisionMode::Native);
            });
            pool.wait_all();
            check(errors == 0 && mb3d::CurrentPrecisionMode() == mode,
                  "worker pool did not propagate/restore precision");
        }
        check(mb3d::CurrentPrecisionMode() == initial, "context did not restore caller precision");
        const std::filesystem::path root(argv[1]);
        for (const auto* name : {
            "comparison/benchmark_scenes/effects-v1/integerpower8-reflections.m3p",
            "comparison/scenes/extall_3Da_ABoxVSWavy.m3p"}) {
            const auto scene = root / name;
            const auto compatible = render(scene, PrecisionMode::Compatible, 1);
            const auto native = render(scene, PrecisionMode::Native, 1);
            auto a = std::async(std::launch::async, [&] { return render(scene, PrecisionMode::Compatible, 3); });
            auto b = std::async(std::launch::async, [&] { return render(scene, PrecisionMode::Native, 2); });
            equal(a.get(), compatible, PrecisionMode::Compatible);
            equal(b.get(), native, PrecisionMode::Native);
            equal(render(scene, PrecisionMode::Compatible, 2), compatible, PrecisionMode::Compatible);
            check(mb3d::CurrentPrecisionMode() == initial, "render leaked its precision into caller");
        }
        const auto scene = (root / "comparison/scenes/test_01_intpow2.m3p").string();
        auto loaded=mb3d::LoadM3PFile(scene);
        check(loaded.success,"default-profile scene load");
        mb3d::RenderOptions defaultOptions;
        defaultOptions.width_override=32;defaultOptions.height_override=24;
        defaultOptions.thread_count=1;
        defaultOptions.no_shadows=defaultOptions.no_ao=defaultOptions.no_dof=defaultOptions.no_reflections=true;
        const auto defaultRender=mb3d::Render(loaded.header,loaded.addon,defaultOptions);
        check(defaultRender.success && !defaultRender.profile.enabled &&
              defaultRender.profile.precision==mb3d::DefaultPrecisionMode(),
              "effective default precision is returned without timing enabled");
        MB3DRenderOptions options{};
        options.width = 32; options.height = 24; options.thread_count = 2;
        options.no_shadows = options.no_ao = options.no_dof = options.no_reflections = 1;
        for (auto mode : {MB3D_PRECISION_COMPATIBLE, MB3D_PRECISION_NATIVE, MB3D_PRECISION_DEFAULT}) {
            MB3DRenderStatus status;
            auto result = mb3d_render_file_with_precision(scene.c_str(), &options, mode, nullptr, &status);
            const bool success = result.success && status == MB3D_RENDER_SUCCESS && result.pixels;
            mb3d_free(result.pixels); mb3d_free(result.zbuffer);
            check(success, "C precision entrypoint failed");
        }
        MB3DRenderStatus status;
        auto invalid = mb3d_render_file_with_precision(scene.c_str(), &options,
            static_cast<MB3DPrecisionMode>(999), nullptr, &status);
        check(!invalid.success && status == MB3D_RENDER_FAILED && !invalid.pixels && !invalid.zbuffer,
              "C API accepted an unknown precision mode");
        std::printf("Render precision: %u checks, 0 failures\n", checks);
        return 0;
    } catch (const std::exception& error) {
        std::fprintf(stderr, "Render precision: %s\n", error.what());
        return 1;
    }
}
