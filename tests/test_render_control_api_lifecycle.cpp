#include "mb3d/mb3d.h"

#include <atomic>
#include <cstdio>
#include <limits>
#include <memory>
#include <stdexcept>
#include <thread>
#include <vector>

namespace {
int checks = 0;
void check(bool condition, const char* reason) {
    ++checks;
    if (!condition) throw std::runtime_error(reason);
}
}

int main() {
    try {
        mb3d_render_control_cancel(nullptr);
        mb3d_render_control_destroy(nullptr);
        check(mb3d_render_control_work_used(nullptr) == 0, "NULL work query");
        check(mb3d_render_control_status(nullptr) == MB3D_RENDER_FAILED, "NULL status query");
        for (uint64_t budget : {uint64_t{0}, uint64_t{1}, (std::numeric_limits<uint64_t>::max)()}) {
            std::unique_ptr<MB3DRenderControl, decltype(&mb3d_render_control_destroy)> control(
                mb3d_render_control_create(budget), mb3d_render_control_destroy);
            check(control != nullptr, "Control allocation");
            check(mb3d_render_control_status(control.get()) == MB3D_RENDER_PREPARED,
                  "Fresh control must be prepared");
            check(mb3d_render_control_work_used(control.get()) == 0, "Fresh control work must be zero");
            std::atomic<bool> invalid{false};
            std::vector<std::thread> callers;
            for (int i = 0; i < 8; ++i) callers.emplace_back([&] {
                for (int n = 0; n < 500; ++n) {
                    mb3d_render_control_cancel(control.get());
                    if (mb3d_render_control_status(control.get()) != MB3D_RENDER_CANCELLED ||
                        mb3d_render_control_work_used(control.get()) != 0)
                        invalid.store(true, std::memory_order_relaxed);
                }
            });
            for (auto& caller : callers) caller.join();
            check(!invalid.load(), "Concurrent C API cancellation/status requests are inconsistent");
            check(mb3d_render_control_status(control.get()) == MB3D_RENDER_CANCELLED,
                  "Cancellation must remain terminal");
            auto fresh = std::unique_ptr<MB3DRenderControl, decltype(&mb3d_render_control_destroy)>(
                mb3d_render_control_create(budget), mb3d_render_control_destroy);
            check(fresh && mb3d_render_control_status(fresh.get()) == MB3D_RENDER_PREPARED,
                  "One control's cancellation leaked to another");
        }
        std::printf("C API control lifecycle: %d checks, 12000 concurrent requests passed; no renderer or numerical qualification.\n", checks);
        return 0;
    } catch (const std::exception& error) {
        std::fprintf(stderr, "C API control lifecycle failed: %s\n", error.what());
        return 1;
    }
}
