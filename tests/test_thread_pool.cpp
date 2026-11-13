#include "core/thread_pool.h"
#include <atomic>
#include <cstdio>
#include <stdexcept>

static void check(bool value, const char* message) {
    if (!value) throw std::runtime_error(message);
}

int main() {
    try {
        for (int workers : {0, 1, 4, 16}) {
            for (int round = 0; round < 1000; ++round) {
                std::atomic<int> completed{0};
                mb3d::ThreadPool pool(workers);
                pool.wait_all();
                std::vector<std::future<void>> tasks;
                for (int i = 0; i < 32; ++i)
                    tasks.push_back(pool.submit([&] { completed.fetch_add(1); }));
                pool.wait_all();
                const int observed = completed.load();
                for (auto& task : tasks) task.get();
                check(observed == 32, "wait_all returned before every task completed");
                auto failure = pool.submit([] { throw std::runtime_error("worker failure"); });
                pool.wait_all();
                bool propagated = false;
                try { failure.get(); } catch (const std::runtime_error&) { propagated = true; }
                check(propagated, "worker exception was lost");
                auto next = pool.submit([&] { completed.fetch_add(1); });
                pool.wait_all();
                next.get();
                check(completed == 33, "pool failed after a task exception");
            }
        }
        std::atomic<int> drained{0};
        {
            mb3d::ThreadPool pool(4);
            for (int i = 0; i < 1024; ++i)
                pool.submit([&] { drained.fetch_add(1); });
        }
        check(drained == 1024, "destructor did not drain queued tasks");
        for (int count : {1, 4, 16}) {
            std::vector<int> completed(static_cast<size_t>(count));
            bool propagated = false;
            try {
                mb3d::RunParallelWorkers(count, [&](int worker) {
                    completed[worker] = 1;
                    if (worker == 0) throw std::runtime_error("parallel worker failure");
                });
            } catch (const std::runtime_error& error) {
                propagated = std::string(error.what()) == "parallel worker failure";
            }
            check(propagated, "parallel worker exception was lost");
            for (int done : completed) check(done == 1, "exception escaped before joining all workers");
            mb3d::RunParallelWorkers(count, [&](int worker) { ++completed[worker]; });
            for (int done : completed) check(done == 2, "parallel worker schedule changed");
        }
        std::puts("Thread pool: 4000 cycles, completion, exceptions, shutdown OK");
        return 0;
    } catch (const std::exception& error) {
        std::fprintf(stderr, "%s\n", error.what());
        return 1;
    }
}
