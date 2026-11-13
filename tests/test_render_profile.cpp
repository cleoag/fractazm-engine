#include "core/renderer.h"
#include "io/m3p_parser.h"
#include <cmath>
#include <atomic>
#include <chrono>
#include <thread>
#include <cstring>
#include <filesystem>
#include <future>
#include <cstdio>
#include <stdexcept>

using namespace mb3d;
namespace {
std::atomic<int> checks{0};
void check(bool value, const char* message) {
    ++checks;
    if (!value) throw std::runtime_error(message);
}
RenderResult run(const std::string& scene, RenderOptions opts) {
    auto loaded = LoadM3PFile(scene);
    check(loaded.success, "scene load failed");
    loaded.header.bCalc3D = 1;
    return Render(loaded.header, loaded.addon, opts);
}
void valid(const RenderProfile& profile) {
    check(profile.enabled, "profile missing");
    check(std::isfinite(profile.total_seconds) && profile.total_seconds >= 0, "invalid total");
    double sum = 0;
    for (const auto& stage : profile.stages) {
        check(std::isfinite(stage.seconds) && stage.seconds >= 0, "invalid stage time");
        check(std::strcmp(stage.state, "not_run") == 0 ||
              std::strcmp(stage.state, "completed") == 0 ||
              std::strcmp(stage.state, "interrupted") == 0, "invalid stage state");
        if (std::strcmp(stage.state, "not_run") == 0)
            check(stage.seconds == 0, "unexecuted stage has time");
        sum += stage.seconds;
    }
    check(sum <= profile.total_seconds, "stage times exceed inclusive core total");
}
void equal(const RenderResult& left, const RenderResult& right) {
    check(left.success && right.success, "finite render failed");
    check(left.width == right.width && left.height == right.height, "dimensions changed");
    check(left.image == right.image, "profiling changed RGB");
    check(left.silight.size() == right.silight.size(), "siLight count changed");
    check(std::memcmp(left.silight.data(), right.silight.data(),
                      left.silight.size() * sizeof(TsiLight5)) == 0, "profiling changed live siLight");
}
}
int main(int argc, char** argv) {
    try {
        if (argc != 2) throw std::runtime_error("usage: test_render_profile <source-root>");
        const auto root = std::filesystem::path(argv[1]);
        const auto builtin = (root / "comparison/scenes/test_01_intpow2.m3p").string();
        const auto external = (root / "comparison/scenes/ext_01_bristorbrot.m3p").string();
        RenderOptions opts;
        opts.width_override = 32;
        opts.height_override = 24;
        opts.deterministic_mode = true;
        for (const int workers : {1, 3}) {
            opts.thread_count = workers;
            for (const auto& scene : {builtin, external}) {
                opts.profile_timing = false;
                const auto plain = run(scene, opts);
                check(!plain.profile.enabled, "default path unexpectedly profiled");
                opts.profile_timing = true;
                const auto measured = run(scene, opts);
                equal(plain, measured);
                valid(measured.profile);
                check(measured.profile.width == 32 && measured.profile.height == 24 &&
                      measured.profile.workers == workers, "effective render geometry missing");
                check(std::strcmp(measured.profile.stages[static_cast<size_t>(RenderProfileStage::Primary)].state,
                                  "completed") == 0, "primary stage not complete");
            }
        }
        // Retain the ordinary jitter/RNG path as well as deterministic mode.
        opts.thread_count = 2;
        opts.deterministic_mode = false;
        opts.profile_timing = false;
        const auto jitterPlain = run(builtin, opts);
        opts.profile_timing = true;
        const auto jitterMeasured = run(builtin, opts);
        equal(jitterPlain, jitterMeasured);
        valid(jitterMeasured.profile);
        opts.deterministic_mode = true;
        opts.control = std::make_shared<RenderControl>();
        opts.control->cancel();
        auto cancelled = run(builtin, opts);
        check(!cancelled.success && cancelled.status == RenderStatus::Cancelled && cancelled.image.empty(),
              "pre-cancellation contract changed");
        valid(cancelled.profile);
        for (const auto& stage : cancelled.profile.stages)
            check(std::strcmp(stage.state, "not_run") == 0, "pre-cancelled stage executed");
        opts.control = std::make_shared<RenderControl>(1);
        auto exhausted = run(builtin, opts);
        check(!exhausted.success && exhausted.status == RenderStatus::WorkExhausted && exhausted.image.empty(),
              "budget contract changed");
        valid(exhausted.profile);
        bool partial = false;
        for (const auto& stage : exhausted.profile.stages)
            partial |= std::strcmp(stage.state, "interrupted") == 0;
        check(partial, "exhausted profile lost partial timing");
        opts.control = std::make_shared<RenderControl>();
        opts.width_override = 1024;
        opts.height_override = 768;
        auto active = std::async(std::launch::async, [&] { return run(builtin, opts); });
        const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(10);
        while (opts.control->work_used() == 0 &&
               active.wait_for(std::chrono::milliseconds(0)) != std::future_status::ready &&
               std::chrono::steady_clock::now() < deadline) std::this_thread::yield();
        const bool didWork = opts.control->work_used() > 0;
        opts.control->cancel();
        const auto interrupted = active.get();
        check(didWork && !interrupted.success && interrupted.status == RenderStatus::Cancelled &&
              interrupted.image.empty(), "in-flight cancellation failed");
        valid(interrupted.profile);
        partial = false;
        for (const auto& stage : interrupted.profile.stages)
            partial |= std::strcmp(stage.state, "interrupted") == 0;
        check(partial, "cancelled profile lost partial timing");
        opts.control.reset();
        opts.height_override = 24;
        opts.width_override = 16385;
        auto failed = run(builtin, opts);
        check(!failed.success && failed.status == RenderStatus::Failed && failed.image.empty(),
              "failed render published image");
        valid(failed.profile);
        check(std::strcmp(failed.profile.stages[static_cast<size_t>(RenderProfileStage::Preparation)].state,
                          "interrupted") == 0, "failed preparation falsely completed");
        opts.width_override = 32;
        // The same immutable options can be used concurrently; timings belong to
        // results, not to shared options or thread-local process state.
        auto neighbor = std::async(std::launch::async, [&] { return run(external, opts); });
        const auto simultaneous = run(builtin, opts);
        const auto other = neighbor.get();
        valid(simultaneous.profile);
        valid(other.profile);
        equal(simultaneous, run(builtin, opts));
        equal(other, run(external, opts));
        std::printf("Render profile: %d checks passed.\n", checks.load());
        return 0;
    } catch (const std::exception& error) {
        std::fprintf(stderr, "Render profile failed after %d checks: %s\n", checks.load(), error.what());
        return 1;
    }
}
