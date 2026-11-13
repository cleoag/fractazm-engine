#pragma once
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <functional>
#include <vector>
#include <atomic>
#include <future>

namespace mb3d {

// Run one task per logical worker. Join every worker before returning or
// rethrowing; worker exceptions must never terminate the process.
void RunParallelWorkers(int count, const std::function<void(int)>& work);

class ThreadPool {
public:
    explicit ThreadPool(int num_threads);
    ~ThreadPool();

    // Submit a task and get a future to wait on it
    std::future<void> submit(std::function<void()> task);

    // Block until all submitted tasks have completed
    void wait_all();

    // Number of worker threads
    int thread_count() const;

private:
    std::vector<std::thread> workers_;
    std::queue<std::packaged_task<void()>> tasks_;
    std::mutex mutex_;
    std::condition_variable cv_;
    std::atomic<bool> stop_{false};
    std::atomic<int> active_tasks_{0};
    std::condition_variable done_cv_;
    std::mutex done_mutex_;
};

} // namespace mb3d
