#pragma once

#include <atomic>
#include <cstdint>
#include <exception>
#include <memory>
#include <mutex>

namespace mb3d {

enum class RenderStatus {
    Prepared, Running, Completed, Cancelled, WorkExhausted, Failed, Unsupported
};

class RenderStopped : public std::exception {
public:
    explicit RenderStopped(RenderStatus status) noexcept : status_(status) {}
    RenderStatus status() const noexcept { return status_; }
    const char* what() const noexcept override;
private:
    RenderStatus status_;
};

// One control per invocation. The limit counts admitted ordinary work events,
// not Pascal iterations, elapsed time or machine instructions. A zero limit is
// unlimited. Even unlimited controls count events and support cancellation.
// Create a fresh control for another job; completed controls cannot be reset.
class RenderControl {
public:
    explicit RenderControl(uint64_t maxWork = 0) noexcept : limit_(maxWork) {}
    RenderControl(const RenderControl&) = delete;
    RenderControl& operator=(const RenderControl&) = delete;

    void cancel() noexcept;
    uint64_t work_used() const noexcept { return used_.load(std::memory_order_relaxed); }
    uint64_t max_work() const noexcept { return limit_; }
    RenderStatus status() const noexcept { return status_.load(std::memory_order_acquire); }

    // Runtime lifecycle: start once, join every task, then complete or fail.
    // start accepts a pre-cancelled control; the first poll reports cancellation.
    void start();
    void poll() const;
    void consume(uint64_t units = 1);
    void complete();
    void fail(std::exception_ptr error = {}) noexcept;

private:
    [[noreturn]] void exhaust();
    const uint64_t limit_;
#if defined(__APPLE__) && defined(__aarch64__)
    // Apple Silicon uses 128-byte cache lines. Admission writes must not
    // invalidate the read-mostly cancellation/lifecycle state on every worker.
    alignas(128)
#endif
    std::atomic<uint64_t> used_{0};
#if defined(__APPLE__) && defined(__aarch64__)
    alignas(128)
#endif
    std::atomic<RenderStatus> status_{RenderStatus::Prepared};
    std::atomic<bool> started_{false};
    mutable std::mutex failure_mutex_;
    std::exception_ptr failure_;
};

using RenderControlHandle = std::shared_ptr<RenderControl>;

// A non-owning hot-path binding. RenderContext and worker captures retain the
// owner; this binding is restored when nested work or an exception unwinds.
extern thread_local RenderControl* current_render_control;

class ScopedRenderControl {
public:
    explicit ScopedRenderControl(RenderControl* control) noexcept;
    ~ScopedRenderControl();
    ScopedRenderControl(const ScopedRenderControl&) = delete;
    ScopedRenderControl& operator=(const ScopedRenderControl&) = delete;
private:
    RenderControl* previous_;
};

// Null control is the legacy path: no counters, allocation or FP operations.
inline void RenderPoll() {
    if (current_render_control) current_render_control->poll();
}
inline void RenderWork(uint64_t units = 1) {
    if (current_render_control) current_render_control->consume(units);
}
inline void FailRender(std::exception_ptr error) noexcept {
    if (current_render_control) current_render_control->fail(error);
}

} // namespace mb3d
