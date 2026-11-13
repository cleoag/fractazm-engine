#include "core/thread_pool.h"
#include "core/render_context.h"
#include "core/formulas.h"

namespace mb3d {

void RunParallelWorkers(int count, const std::function<void(int)>& work) {
    if (count <= 1) {
        try { RenderPoll(); work(0); }
        catch (...) { FailRender(std::current_exception()); throw; }
        return;
    }
    const auto context = CaptureRenderContext();
    std::vector<std::exception_ptr> errors(static_cast<size_t>(count));
    {
        // jthread also joins the already-created workers if thread creation
        // fails. errors and work remain alive until all joins finish.
        std::vector<std::jthread> workers;
        workers.reserve(static_cast<size_t>(count));
        try {
        for (int i = 0; i < count; ++i) {
            workers.emplace_back([&, i, context] {
                try {
                    ScopedRenderContext binding(context);
                    ScopedParityTraceSession traceSession;
                    ScopedFormulaTraceContext formulaTraceSession;
                    RenderPoll();
                    work(i);
                }
                catch (...) {
                    const auto error = std::current_exception();
                    if (context && context->control) context->control->fail(error);
                    errors[i] = error;
                }
            });
        }
        } catch (...) {
            // Publish before jthread destruction waits for already-started work.
            FailRender(std::current_exception());
            throw;
        }
    }
    for (const auto& error : errors) {
        if (error) std::rethrow_exception(error);
    }
}

ThreadPool::ThreadPool(int num_threads) {
    if (num_threads < 1) num_threads = 1;

    workers_.reserve(static_cast<size_t>(num_threads));
    try {
        for (int i = 0; i < num_threads; i++) {
            workers_.emplace_back([this]() {
                for (;;) {
                    std::packaged_task<void()> task;
                    {
                        std::unique_lock<std::mutex> lock(mutex_);
                        cv_.wait(lock, [this]() {
                            return stop_.load() || !tasks_.empty();
                        });
                        if (stop_.load() && tasks_.empty())
                            return;
                        task = std::move(tasks_.front());
                        tasks_.pop();
                        active_tasks_.fetch_add(1);
                    }
                    task();
                    active_tasks_.fetch_sub(1);
                    {
                        std::lock_guard<std::mutex> lock(done_mutex_);
                        done_cv_.notify_all();
                    }
                }
            });
        }
    } catch (...) {
        // The destructor is not run when construction throws. Explicitly stop
        // and join the workers already created before rethrowing the failure.
        {
            std::lock_guard<std::mutex> lock(mutex_);
            stop_.store(true);
        }
        cv_.notify_all();
        for (auto& worker : workers_) worker.join();
        throw;
    }
}

ThreadPool::~ThreadPool() {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        stop_.store(true);
    }
    cv_.notify_all();
    for (auto& w : workers_)
        w.join();
}

std::future<void> ThreadPool::submit(std::function<void()> task) {
    auto context = CaptureRenderContext();
    std::packaged_task<void()> ptask([context = std::move(context), task = std::move(task)] {
        try {
            ScopedRenderContext binding(context);
            ScopedParityTraceSession traceSession;
            ScopedFormulaTraceContext formulaTraceSession;
            RenderPoll();
            task();
        } catch (...) {
            // Scope construction can fail too; TLS may already be restored.
            if (context && context->control) context->control->fail(std::current_exception());
            throw;
        }
    });
    auto future = ptask.get_future();
    {
        std::lock_guard<std::mutex> lock(mutex_);
        tasks_.push(std::move(ptask));
    }
    cv_.notify_one();
    return future;
}

void ThreadPool::wait_all() {
    std::unique_lock<std::mutex> lock(done_mutex_);
    done_cv_.wait(lock, [this]() {
        std::lock_guard<std::mutex> qlock(mutex_);
        return tasks_.empty() && active_tasks_.load() == 0;
    });
}

int ThreadPool::thread_count() const {
    return static_cast<int>(workers_.size());
}

} // namespace mb3d
