#include "core/render_context.h"
#include "core/thread_pool.h"
#include "core/formulas.h"
#include <atomic>
#include <cstdio>
#include <future>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

void check(bool value, const char* message) {
    if (!value) throw std::runtime_error(message);
}

void TestScopedContext() {
    const auto caller = mb3d::CaptureRenderContext();
    const auto first = std::make_shared<mb3d::RenderContext>();
    const auto second = std::make_shared<mb3d::RenderContext>();
    first->formula_constants.buf[0][0] = 17;
    second->formula_constants.buf[0][0] = 29;
    {
        mb3d::ScopedRenderContext binding(first);
        check(&mb3d::GetFormulaConstBuffers() == &first->formula_constants,
              "formula constants did not use the bound owner");
        try {
            mb3d::ScopedRenderContext nested(second);
            check(mb3d::GetFormulaConstBuffers().buf[0][0] == 29,
                  "nested context inherited the outer constants");
            throw std::runtime_error("nested failure");
        } catch (const std::runtime_error& error) {
            check(std::string(error.what()) == "nested failure", error.what());
        }
        check(mb3d::CaptureRenderContext() == first,
              "exception did not restore the previous context");
        check(mb3d::GetFormulaConstBuffers().buf[0][0] == 17,
              "nested context changed the outer constants");
        bool rejected = false;
        try { mb3d::ScopedRenderContext invalid(nullptr); }
        catch (const std::invalid_argument&) { rejected = true; }
        check(rejected && mb3d::CaptureRenderContext() == first,
              "null context did not preserve the existing binding");
    }
    check(mb3d::CaptureRenderContext() == caller,
          "outer context scope did not restore its caller");
}

void TestTraceSession() {
#ifdef MB3D_ENABLE_PARITY_TRACE
    mb3d::ScopedRenderContext context(std::make_shared<mb3d::RenderContext>());
    mb3d::ScopedParityTraceSession outer;
    mb3d::SetParityTraceSchemaMode(true);
    mb3d::SetParityTraceLevel(mb3d::ParityTraceLevel::DetailAll);
    std::vector<std::string> parentLines;
    std::vector<std::string> childLines;
    mb3d::SetParityTraceContext(3, 4, &parentLines);
    mb3d::ConfigureParityTraceStageRange("parent_start", "parent_end");
    const auto parentRange = mb3d::CaptureParityTraceStageRangeState();
    mb3d::ParityEnterPixel(3, 4);
    mb3d::ParityBeginCurrentStage("parent_stage");
    mb3d::ParityAddInt("before", 17);
    bool propagated = false;
    try {
        mb3d::ScopedParityTraceSession nested;
        check(mb3d::GetParityTracePixelX() == -1 && !mb3d::ParityActive(),
              "nested trace session inherited its caller's pixel/output");
        check(mb3d::CaptureParityTraceStageRangeState() != parentRange,
              "nested trace session shared its caller's mutable stage cursor");
        mb3d::ConfigureParityTraceStageRange("child_start", "child_end");
        mb3d::SetParityTraceContext(8, 9, &childLines);
        mb3d::ParityEnterPixel(8, 9);
        mb3d::ParityBeginCurrentStage("abandoned_child");
        mb3d::ParityAddInt("child", 99);
        throw std::runtime_error("trace failure");
    } catch (const std::runtime_error& error) {
        propagated = std::string(error.what()) == "trace failure";
        if (!propagated) throw;
    }
    check(propagated && mb3d::ParityActive(), "exception lost the outer active pixel");
    check(mb3d::CaptureParityTraceStageRangeState() == parentRange &&
          parentRange->start_stage == "parent_start",
          "nested trace session changed the outer stage cursor");
    mb3d::ParityAddInt("after", 23);
    mb3d::ParityEndStage();
    check(childLines.empty(), "abandoned child stage was published");
    check(parentLines.size() == 1 &&
          parentLines[0].find("stage=parent_stage x=3 y=4 before=17 after=23 checksum=") == 0,
          "nested trace scope did not restore the partially built parent stage");

    // Suppression is independent of the builder's pixel_active flag. A nested
    // session must start clean and restore the outer suppression on return.
    mb3d::SetParityTraceSchemaMode(false);
    {
        mb3d::ScopedParityTracePixel suppressed(30, 40);
        check(!mb3d::HasParityTraceContext(), "outer pixel was not suppressed");
        {
            mb3d::ScopedParityTraceSession nested;
            mb3d::SetParityTraceContext(8, 9, &childLines);
            check(mb3d::HasParityTraceContext(), "fresh session inherited suppression");
        }
        check(!mb3d::HasParityTraceContext(), "nested session lost outer suppression");
    }
    check(mb3d::HasParityTraceContext(), "pixel scope failed to restore outer tracing");
#endif
}

void TestFormulaTraceSession() {
    mb3d::ScopedRenderContext context(std::make_shared<mb3d::RenderContext>());
    mb3d::ScopedParityTraceSession paritySession;
    mb3d::ScopedFormulaTraceContext outer;
    std::vector<std::string> parentLines;
    std::vector<std::string> childLines;
    mb3d::SetFormulaTraceContext(3, 4, &parentLines);
    mb3d::AppendFormulaTraceEvent("parent_before", "value=17");
    try {
        mb3d::ScopedFormulaTraceContext nested;
        mb3d::AppendFormulaTraceEvent("unbound_child", "value=0");
        check(parentLines.size() == 1, "fresh formula scope retained parent output");
        mb3d::SetFormulaTraceContext(8, 9, &childLines);
        mb3d::AppendFormulaTraceEvent("child", "value=29");
        throw std::runtime_error("formula trace failure");
    } catch (const std::runtime_error& error) {
        if (std::string(error.what()) != "formula trace failure") throw;
    }
    mb3d::AppendFormulaTraceEvent("parent_after", "value=23");
    check(parentLines == std::vector<std::string>{
        "stage=parent_before x=3 y=4 value=17", "stage=parent_after x=3 y=4 value=23"},
        "formula trace exception did not restore caller output and coordinates");
    check(childLines == std::vector<std::string>{"stage=child x=8 y=9 value=29"},
          "formula trace events escaped their scope");
}

void TestSharedPool() {
    const auto caller = mb3d::CaptureRenderContext();
    std::weak_ptr<mb3d::RenderContext> retired[2];
    for (int workers : {1, 4}) {
        {
            mb3d::ThreadPool pool(workers);
            std::promise<void> release;
            const auto gate = release.get_future().share();
            std::vector<std::future<void>> blockers;
            for (int i = 0; i < workers; ++i)
                blockers.push_back(pool.submit([gate] { gate.wait(); }));

            std::vector<std::future<void>> tasks;
            for (int job = 0; job < 2; ++job) {
                auto owner = std::make_shared<mb3d::RenderContext>();
                retired[job] = owner;
                const auto expectedOwner = owner.get();
                owner->formula_constants.buf[0][0] = static_cast<uint8_t>(job + 41);
                owner->parity_trace_config.schema_mode = true;
                owner->parity_trace_config.level = job == 0
                    ? mb3d::ParityTraceLevel::DetailAll : mb3d::ParityTraceLevel::Checkpoint;
                mb3d::ScopedRenderContext binding(owner);
                for (int task = 0; task < 8; ++task) {
                    tasks.push_back(pool.submit([expectedOwner, job, task] {
                        check(&mb3d::CurrentRenderContext() == expectedOwner,
                              "shared pool used its creator's or another task's owner");
                        check(mb3d::GetFormulaConstBuffers().buf[0][0] == job + 41,
                              "queued task constants were overwritten by another job");
                        check(!mb3d::FormulaTraceActive(),
                              "pool task inherited a completed task's formula output pointer");
                        std::vector<std::string> formulaLines;
                        mb3d::SetFormulaTraceContext(job, task, &formulaLines);
                        mb3d::AppendFormulaTraceEvent("task_formula", "value=1");
                        check(formulaLines.size() == 1, "task formula trace output was lost");
#ifdef MB3D_ENABLE_PARITY_TRACE
                        check(mb3d::GetParityTracePixelX() == -1 && !mb3d::ParityActive(),
                              "pool task inherited a completed task's trace pointer");
                        std::vector<std::string> lines;
                        mb3d::SetParityTraceContext(job, task, &lines);
                        mb3d::ParityEnterPixel(job, task);
                        mb3d::ParityBeginCurrentStage("pool_task");
                        mb3d::ParityAddInt("job", job);
                        mb3d::ParityEndStage();
                        check(lines.size() == 1, "worker did not inherit schema tracing");
                        check((lines[0].find(" job=") != std::string::npos) == (job == 0),
                              "concurrent task trace verbosity came from another job");
                        // Deliberately leave the TLS pointer set. Task scope must
                        // remove it on both normal completion and unwinding.
#endif
                        if (task == 3) throw std::runtime_error("expected pool failure");
                    }));
                }
            }
            const bool ownersRetained = !retired[0].expired() && !retired[1].expired();
            release.set_value();
            for (auto& blocker : blockers) blocker.get();
            int expectedFailures = 0;
            for (auto& task : tasks) {
                try { task.get(); }
                catch (const std::runtime_error& error) {
                    if (std::string(error.what()) != "expected pool failure") throw;
                    ++expectedFailures;
                }
            }
            pool.wait_all();
            check(ownersRetained, "queued tasks did not retain their owners");
            check(expectedFailures == 2, "worker failures were lost");
        }
        check(retired[0].expired() && retired[1].expired(),
              "finished pool retained a job owner");
        check(mb3d::CaptureRenderContext() == caller,
              "task submission changed the caller's binding");
    }
}

void TestDirectWorkers() {
    const auto owner = std::make_shared<mb3d::RenderContext>();
    mb3d::ScopedRenderContext binding(owner);
    mb3d::ScopedParityTraceSession traceSession;
    mb3d::ScopedFormulaTraceContext formulaSession;
    owner->formula_constants.buf[0][0] = 63;
    owner->parity_trace_config.schema_mode = true;
    owner->parity_trace_config.level = mb3d::ParityTraceLevel::DetailSelected;
    owner->parity_trace_config.detail_stages = {"selected"};
    std::vector<std::string> parentFormulaLines;
    mb3d::SetFormulaTraceContext(3, 4, &parentFormulaLines);
#ifdef MB3D_ENABLE_PARITY_TRACE
    std::vector<std::string> parentLines;
    mb3d::SetParityTraceContext(3, 4, &parentLines);
    mb3d::ParityEnterPixel(3, 4);
#endif
    for (int workers : {1, 2, 4, 16}) {
        std::atomic<int> completed{0};
        bool propagated = false;
        try {
            mb3d::RunParallelWorkers(workers, [&](int worker) {
                check(mb3d::CaptureRenderContext() == owner,
                      "direct worker did not inherit its submitting owner");
                check(mb3d::GetFormulaConstBuffers().buf[0][0] == 63,
                      "direct worker used default constants");
                check(mb3d::FormulaTraceActive() == (workers == 1),
                      "direct worker changed inline/fresh-thread formula trace semantics");
                check(mb3d::CurrentRenderContext().parity_trace_config.detail_stages ==
                          std::vector<std::string>{"selected"},
                      "direct worker lost job trace config");
#ifdef MB3D_ENABLE_PARITY_TRACE
                check(mb3d::GetParityTracePixelX() == (workers == 1 ? 3 : -1),
                      "direct worker changed inline/fresh-thread trace semantics");
#endif
                ++completed;
                if (worker == 0) throw std::runtime_error("expected direct failure");
            });
        } catch (const std::runtime_error& error) {
            propagated = std::string(error.what()) == "expected direct failure";
            if (!propagated) throw;
        }
        check(propagated && completed == workers,
              "direct worker failure escaped before all workers joined");
        check(mb3d::CaptureRenderContext() == owner,
              "direct workers changed their caller's owner");
        check(mb3d::FormulaTraceActive(),
              "direct worker failure disturbed the caller's formula trace");
#ifdef MB3D_ENABLE_PARITY_TRACE
        check(mb3d::ParityActive() && mb3d::GetParityTracePixelX() == 3,
              "direct worker failure disturbed its caller's trace session");
#endif
    }
}

} // namespace

int main() {
    try {
        TestScopedContext();
        TestTraceSession();
        TestFormulaTraceSession();
        TestSharedPool();
        TestDirectWorkers();
        std::puts("Render context: nested scopes, task ownership, traces, exceptions and release OK");
        return 0;
    } catch (const std::exception& error) {
        std::fprintf(stderr, "%s\n", error.what());
        return 1;
    }
}
