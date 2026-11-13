#include "mb3d/mb3d.h"

#include <chrono>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <future>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <thread>

namespace {
int checks = 0;
void check(bool condition, const std::string& reason) {
    ++checks;
    if (!condition) throw std::runtime_error(reason);
}
using Control = std::unique_ptr<MB3DRenderControl, decltype(&mb3d_render_control_destroy)>;
Control control(uint64_t limit = 0) {
    Control value(mb3d_render_control_create(limit), mb3d_render_control_destroy);
    check(value != nullptr, "Cannot allocate C render control");
    return value;
}
struct Result {
    MB3DRenderResult value{};
    MB3DRenderStatus status = MB3D_RENDER_FAILED;
    Result() = default;
    Result(const Result&) = delete;
    Result& operator=(const Result&) = delete;
    Result(Result&& other) noexcept : value(other.value), status(other.status) { other.value = {}; }
    Result& operator=(Result&& other) noexcept {
        if (this != &other) {
            mb3d_free(value.pixels);
            mb3d_free(value.zbuffer);
            value = other.value;
            status = other.status;
            other.value = {};
        }
        return *this;
    }
    ~Result() { mb3d_free(value.pixels); mb3d_free(value.zbuffer); }
};
MB3DRenderOptions options(int workers, int width = 32, int height = 24) {
    MB3DRenderOptions opts{};
    opts.width = width;
    opts.height = height;
    opts.thread_count = workers;
    opts.max_iterations = 20;
    opts.no_shadows = opts.no_ao = opts.no_dof = opts.no_reflections = 1;
    return opts;
}
Result render(const std::string& path, const MB3DRenderOptions& opts,
              MB3DRenderControl* value) {
    Result result;
    result.value = mb3d_render_file_with_control(path.c_str(), &opts, value, &result.status);
    return result;
}
void successful(const Result& result, const std::string& where) {
    check(result.value.success && result.status == MB3D_RENDER_SUCCESS,
          where + ": " + result.value.error);
    check(result.value.pixels && result.value.zbuffer && result.value.width > 0 &&
          result.value.height > 0 && result.value.error[0] == '\0', where + ": invalid successful surface");
}
void empty(const Result& result, MB3DRenderStatus expected, const std::string& where) {
    check(!result.value.success && result.status == expected, where + ": wrong terminal status");
    check(!result.value.pixels && !result.value.zbuffer && result.value.width == 0 &&
          result.value.height == 0 && result.value.error[0] != '\0', where + ": partial failed output");
}
void identical(const Result& left, const Result& right, const std::string& where) {
    successful(left, where + " actual");
    successful(right, where + " control");
    check(left.value.width == right.value.width && left.value.height == right.value.height,
          where + ": dimensions differ");
    const size_t pixels = static_cast<size_t>(left.value.width) * left.value.height;
    check(std::memcmp(left.value.pixels, right.value.pixels, pixels * 3) == 0,
          where + ": RGB differs");
    check(std::memcmp(left.value.zbuffer, right.value.zbuffer, pixels * sizeof(unsigned short)) == 0,
          where + ": depth differs");
}
}

int main(int argc, char** argv) {
    try {
        if (argc != 2) throw std::runtime_error("usage: test_render_control_api <source-root>");
        const std::string scene = (std::filesystem::path(argv[1]) /
            "comparison/scenes/test_01_intpow2.m3p").string();
        for (const int workers : {1, 4}) {
            const auto opts = options(workers);
            Result legacy;
            legacy.value = mb3d_render_file(scene.c_str(), &opts);
            legacy.status = legacy.value.success ? MB3D_RENDER_SUCCESS : MB3D_RENDER_FAILED;
            auto ordinary = render(scene, opts, nullptr);
            identical(ordinary, legacy, "Null control preserves original ABI behavior");

            auto unlimited = control();
            const auto finite = render(scene, opts, unlimited.get());
            identical(finite, legacy, "Explicit unlimited control");
            check(mb3d_render_control_work_used(unlimited.get()) > 0, "No shared work accounted");
            check(mb3d_render_control_status(unlimited.get()) == MB3D_RENDER_SUCCESS,
                  "Completed control status differs from returned status");
            mb3d_render_control_cancel(unlimited.get());
            check(mb3d_render_control_status(unlimited.get()) == MB3D_RENDER_SUCCESS,
                  "Late cancellation changed an already completed job");
            const auto reused = render(scene, opts, unlimited.get());
            empty(reused, MB3D_RENDER_FAILED, "A completed control cannot start another job");

            auto cancelled = control();
            mb3d_render_control_cancel(cancelled.get());
            const auto stopped = render(scene, opts, cancelled.get());
            empty(stopped, MB3D_RENDER_CANCELLED, "Pre-cancelled render");
            check(mb3d_render_control_work_used(cancelled.get()) == 0, "Pre-cancel performed work");

            auto limited = control(1);
            const auto exhausted = render(scene, opts, limited.get());
            empty(exhausted, MB3D_RENDER_WORK_LIMIT_EXCEEDED, "Shared budget exhausted");
            check(mb3d_render_control_work_used(limited.get()) <= 1, "Shared work exceeded its cap");
            check(mb3d_render_control_status(limited.get()) == MB3D_RENDER_WORK_LIMIT_EXCEEDED,
                  "Exhausted control status differs from returned status");

            auto active = control();
            auto pending = std::async(std::launch::async, [&] {
                return render(scene, options(workers, 1024, 768), active.get());
            });
            const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(10);
            while (mb3d_render_control_work_used(active.get()) == 0 &&
                   pending.wait_for(std::chrono::milliseconds(0)) != std::future_status::ready &&
                   std::chrono::steady_clock::now() < deadline) std::this_thread::yield();
            const bool observedWork = mb3d_render_control_work_used(active.get()) > 0;
            mb3d_render_control_cancel(active.get());
            auto independent = control();
            const auto neighbor = render(scene, opts, independent.get());
            const auto interrupted = pending.get();
            check(observedWork, "In-flight cancellation never observed active work");
            empty(interrupted, MB3D_RENDER_CANCELLED, "In-flight cancellation");
            identical(neighbor, legacy, "Cancellation must not affect an independent job");

            auto recovery = control((std::numeric_limits<uint64_t>::max)());
            const auto recovered = render(scene, opts, recovery.get());
            identical(recovered, legacy, "Fresh control recovery");
        }
        MB3DRenderStatus status = MB3D_RENDER_SUCCESS;
        Result invalid;
        invalid.value = mb3d_render_file_with_control(nullptr, nullptr, nullptr, &status);
        invalid.status = status;
        empty(invalid, MB3D_RENDER_FAILED, "NULL path error remains a failure");
        std::printf("C API render control: %d checks passed; behavioral evidence only.\n", checks);
        return 0;
    } catch (const std::exception& error) {
        std::fprintf(stderr, "C API render control failed: %s\n", error.what());
        return 1;
    }
}
