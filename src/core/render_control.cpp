#include "core/render_control.h"
#include <limits>
#include <stdexcept>

namespace mb3d {

thread_local RenderControl* current_render_control = nullptr;

const char* RenderStopped::what() const noexcept {
    if (status_ == RenderStatus::Cancelled) return "Render cancelled";
    if (status_ == RenderStatus::WorkExhausted) return "Render work budget exhausted";
    return "Render failed";
}

void RenderControl::start() {
    bool expected = false;
    if (!started_.compare_exchange_strong(expected, true))
        throw std::logic_error("Render control has already been used; create a new control for each job");
    auto prepared = RenderStatus::Prepared;
    status_.compare_exchange_strong(prepared, RenderStatus::Running,
                                   std::memory_order_acq_rel);
}

void RenderControl::cancel() noexcept {
    auto value = status();
    while (value == RenderStatus::Prepared || value == RenderStatus::Running) {
        if (status_.compare_exchange_weak(value, RenderStatus::Cancelled,
                                         std::memory_order_acq_rel)) return;
    }
}

void RenderControl::poll() const {
    const auto value = status();
    if (value == RenderStatus::Running) return;
    if (value == RenderStatus::Cancelled || value == RenderStatus::WorkExhausted)
        throw RenderStopped(value);
    if (value == RenderStatus::Failed) {
        std::exception_ptr error;
        { std::lock_guard lock(failure_mutex_); error = failure_; }
        if (error) std::rethrow_exception(error);
        throw RenderStopped(value);
    }
    throw std::logic_error("Render control is not running");
}

[[noreturn]] void RenderControl::exhaust() {
    auto running = RenderStatus::Running;
    status_.compare_exchange_strong(running, RenderStatus::WorkExhausted,
                                   std::memory_order_acq_rel);
    poll();
    throw RenderStopped(RenderStatus::WorkExhausted); // poll always throws here
}

void RenderControl::consume(uint64_t units) {
    poll();
    if (units == 0) return;
    const auto ceiling = limit_ ? limit_ : std::numeric_limits<uint64_t>::max();
    auto used = used_.load(std::memory_order_relaxed);
    for (;;) {
        if (units > ceiling - used) exhaust();
        if (used_.compare_exchange_weak(used, used + units,
                                       std::memory_order_relaxed)) break;
        poll();
    }
    // A concurrent cancellation may win after admission but before execution.
    // Such an event remains admitted; the counter never claims completed work.
    poll();
}

void RenderControl::complete() {
    poll();
    auto running = RenderStatus::Running;
    if (!status_.compare_exchange_strong(running, RenderStatus::Completed,
                                        std::memory_order_acq_rel)) poll();
}

void RenderControl::fail(std::exception_ptr error) noexcept {
    try {
        std::lock_guard lock(failure_mutex_);
        if (status() != RenderStatus::Running) return;
        failure_ = error;
        auto running = RenderStatus::Running;
        status_.compare_exchange_strong(running, RenderStatus::Failed,
                                       std::memory_order_acq_rel);
    } catch (...) {
        auto running = RenderStatus::Running;
        status_.compare_exchange_strong(running, RenderStatus::Failed,
                                       std::memory_order_acq_rel);
    }
}

ScopedRenderControl::ScopedRenderControl(RenderControl* control) noexcept
    : previous_(current_render_control) { current_render_control = control; }
ScopedRenderControl::~ScopedRenderControl() { current_render_control = previous_; }

} // namespace mb3d
