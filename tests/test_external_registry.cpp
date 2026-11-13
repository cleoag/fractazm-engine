#include "core/external_formulas.h"

#include <array>
#include <condition_variable>
#include <cstdio>
#include <cstring>
#include <exception>
#include <mutex>
#include <stdexcept>
#include <thread>
#include <vector>

namespace {

void check(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}

class StartGate {
public:
    explicit StartGate(int participants) : remaining_(participants) {}

    void wait() {
        std::unique_lock<std::mutex> lock(mutex_);
        if (--remaining_ == 0) {
            open_ = true;
            ready_.notify_all();
        } else {
            ready_.wait(lock, [this] { return open_; });
        }
    }

private:
    std::mutex mutex_;
    std::condition_variable ready_;
    int remaining_;
    bool open_ = false;
};

} // namespace

int main() {
    try {
        // Ordinary formulas at the beginning and end of the registration table.
        // Do not initialize or query the registry before releasing the workers.
        constexpr unsigned firstId = 6001;
        constexpr unsigned lastId = 10243;
        constexpr int workerCount = 24;
        StartGate gate(workerCount);
        std::array<const mb3d::ExternalFormulaInfo*, workerCount> firstSeen{};
        std::array<const mb3d::ExternalFormulaInfo*, workerCount> lastSeen{};
        std::array<std::exception_ptr, workerCount> failures{};
        std::vector<std::thread> workers;
        workers.reserve(workerCount);
        for (int worker = 0; worker < workerCount; ++worker) {
            workers.emplace_back([&, worker] {
                gate.wait();
                try {
                    switch (worker % 3) {
                    case 0:
                        mb3d::InitExternalFormulaRegistry();
                        break;
                    case 1:
                        check(mb3d::FindExternalFormula("transform2IFS") != nullptr,
                              "cold name lookup observed an incomplete registry");
                        break;
                    default:
                        check(mb3d::FindExternalFormulaById(lastId) != nullptr,
                              "cold ID lookup observed an incomplete registry");
                        break;
                    }

                    auto* first = mb3d::FindExternalFormula("_Folding1");
                    auto* last = mb3d::FindExternalFormula("transform2IFS");
                    check(first && first->id == firstId && first->wrapperFunc,
                          "first registration is missing or invalid");
                    check(last && last->id == lastId && last->wrapperFunc,
                          "last registration is missing or invalid");
                    firstSeen[worker] = first;
                    lastSeen[worker] = last;
                    for (int repeat = 0; repeat < 64; ++repeat) {
                        mb3d::InitExternalFormulaRegistry();
                        check(mb3d::FindExternalFormulaById(firstId) == first &&
                              mb3d::FindExternalFormulaById(lastId) == last &&
                              mb3d::FindExternalFormula("_folding1") == first &&
                              mb3d::FindExternalFormula("TRANSFORM2IFS") == last,
                              "registry pointer or lookup changed after initialization");
                    }
                } catch (...) {
                    failures[worker] = std::current_exception();
                }
            });
        }
        for (auto& worker : workers) worker.join();
        for (const auto& failure : failures) {
            if (failure) std::rethrow_exception(failure);
        }
        for (int worker = 1; worker < workerCount; ++worker) {
            check(firstSeen[worker] == firstSeen[0] && lastSeen[worker] == lastSeen[0],
                  "workers observed different registry storage");
        }

        bool lateRegistrationRejected = false;
        try {
            mb3d::RegisterExternalFormula(*firstSeen[0]);
        } catch (const std::logic_error&) {
            lateRegistrationRejected = true;
        }
        check(lateRegistrationRejected, "registration remained mutable after publication");
        check(mb3d::FindExternalFormulaById(firstId) == firstSeen[0] &&
              mb3d::FindExternalFormulaById(lastId) == lastSeen[0],
              "rejected registration changed published pointers");
        check(std::strcmp(lastSeen[0]->name, "transform2IFS") == 0,
              "published metadata was corrupted");
        std::puts("External registry: concurrent cold start, complete lookup and stable pointers OK");
        return 0;
    } catch (const std::exception& error) {
        std::fprintf(stderr, "%s\n", error.what());
        return 1;
    }
}
