#pragma once
// Hooks the application API sets around a render on the rendering thread.
//
// Work meter: a live count of the GPU thread invocations a render has
// dispatched, for progress display. It is observation only: nothing reads it
// to decide how a frame is rendered, so it never changes an image or the
// engine selection.
#include <atomic>
#include <cstdint>
#include <exception>

namespace mb3d {
// Set by the caller around a render on the rendering thread; null otherwise.
inline thread_local std::atomic<std::uint64_t>* render_work_meter = nullptr;

// Cancellation at GPU command boundaries. The flag is read before each
// command is encoded; a set flag ends the render with RenderCancelRequested
// and no image. A command already running finishes first.
inline thread_local const std::atomic<bool>* render_cancel = nullptr;

struct RenderCancelRequested : std::exception {
    const char* what() const noexcept override { return "Render cancelled"; }
};

inline bool renderCancelRequested() {
    return render_cancel && render_cancel->load(std::memory_order_relaxed);
}

struct RenderCancelScope {
    const std::atomic<bool>* previous;
    explicit RenderCancelScope(const std::atomic<bool>* flag) : previous(render_cancel) { render_cancel = flag; }
    ~RenderCancelScope() { render_cancel = previous; }
    RenderCancelScope(const RenderCancelScope&) = delete;
    RenderCancelScope& operator=(const RenderCancelScope&) = delete;
};

struct RenderWorkMeterScope {
    std::atomic<std::uint64_t>* previous;
    explicit RenderWorkMeterScope(std::atomic<std::uint64_t>* meter) : previous(render_work_meter) { render_work_meter = meter; }
    ~RenderWorkMeterScope() { render_work_meter = previous; }
    RenderWorkMeterScope(const RenderWorkMeterScope&) = delete;
    RenderWorkMeterScope& operator=(const RenderWorkMeterScope&) = delete;
};
}
