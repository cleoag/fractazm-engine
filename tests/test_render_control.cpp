#include "core/render_control.h"
#include "core/render_context.h"
#include "core/thread_pool.h"
#include "core/formula_iteration_guard.h"
#include "core/formula_work_guard.h"
#include <atomic>
#include <cfenv>
#include <chrono>
#include <cstdio>
#include <future>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

using namespace mb3d;
namespace {
int checks = 0;
void check(bool value, const char* message) {
    ++checks;
    if (!value) throw std::runtime_error(message);
}
template<class F> void stopped(F&& fn, RenderStatus expected) {
    bool caught = false;
    try { fn(); }
    catch (const RenderStopped& e) { caught = e.status() == expected; }
    check(caught, "wrong or absent terminal reason");
}
void boundaries() {
    check(current_render_control == nullptr, "default binding must be empty");
    RenderWork(std::numeric_limits<uint64_t>::max());
    RenderPoll();
    RenderControl pre;
    pre.cancel();
    check(pre.status() == RenderStatus::Cancelled, "pre-cancellation not visible");
    pre.start();
    stopped([&] { pre.poll(); }, RenderStatus::Cancelled);
    check(pre.work_used() == 0, "cancelled job admitted work");
    pre.fail(std::make_exception_ptr(std::runtime_error("later error")));
    check(pre.status() == RenderStatus::Cancelled, "later failure replaced cancellation");

    RenderControl exact(3);
    exact.start();
    exact.consume(2); exact.consume(); exact.complete(); exact.cancel();
    check(exact.work_used() == 3 && exact.status() == RenderStatus::Completed,
          "exact limit must complete and retain completion");
    bool reused = false;
    try { exact.start(); } catch (const std::logic_error&) { reused = true; }
    check(reused && exact.status() == RenderStatus::Completed, "control reuse changed prior job");

    RenderControl exhausted(3);
    exhausted.start(); exhausted.consume(3);
    int mutation = 0;
    stopped([&] { exhausted.consume(); ++mutation; }, RenderStatus::WorkExhausted);
    exhausted.cancel();
    check(exhausted.work_used() == 3 && mutation == 0 &&
              exhausted.status() == RenderStatus::WorkExhausted,
          "exhaustion overshot or was overwritten");
    RenderControl huge;
    huge.start(); huge.consume(std::numeric_limits<uint64_t>::max());
    stopped([&] { huge.consume(); }, RenderStatus::WorkExhausted);
    check(huge.work_used() == std::numeric_limits<uint64_t>::max(), "counter wrapped");

    RenderControl outer, inner;
    outer.start(); inner.start();
    {
        ScopedRenderControl binding(&outer);
        RenderWork(5);
        try {
            ScopedRenderControl nested(&inner);
            RenderWork(7);
            throw std::runtime_error("nested");
        } catch (const std::runtime_error&) {}
        RenderWork();
    }
    check(outer.work_used() == 6 && inner.work_used() == 7 && !current_render_control,
          "nested binding or exception lost caller control");
    outer.complete(); inner.complete();
}
void sharing(int workers) {
    auto context = std::make_shared<RenderContext>();
    context->control = std::make_shared<RenderControl>(10003);
    context->control->start();
    ScopedRenderContext binding(context);
    ThreadPool pool(workers);
    std::vector<std::future<void>> tasks;
    std::atomic<uint64_t> executed{0};
    for (int i = 0; i < workers; ++i)
        tasks.push_back(pool.submit([&] {
            for (;;) { RenderWork(); executed.fetch_add(1, std::memory_order_relaxed); }
        }));
    pool.wait_all();
    for (auto& task : tasks) stopped([&] { task.get(); }, RenderStatus::WorkExhausted);
    check(context->control->work_used() == 10003 && executed <= 10003,
          "parallel job did not share one exact admission limit");
    check(context->control->status() == RenderStatus::WorkExhausted,
          "task exception overwrote exhaustion");
}
void cancellationAndRecovery() {
    auto context = std::make_shared<RenderContext>();
    context->control = std::make_shared<RenderControl>();
    context->control->start();
    ScopedRenderContext binding(context);
    ThreadPool pool(2);
    std::promise<void> entered;
    auto ready = entered.get_future();
    auto task = pool.submit([&] {
        for (int i = 0; i < 100; ++i) RenderWork();
        entered.set_value();
        for (;;) RenderWork();
    });
    check(ready.wait_for(std::chrono::seconds(5)) == std::future_status::ready,
          "worker did not start");
    context->control->cancel();
    check(task.wait_for(std::chrono::seconds(5)) == std::future_status::ready,
          "worker ignored cancellation");
    stopped([&] { task.get(); }, RenderStatus::Cancelled);
    pool.wait_all();

    auto fresh = std::make_shared<RenderContext>();
    fresh->control = std::make_shared<RenderControl>(17);
    fresh->control->start();
    {
        ScopedRenderContext recovery(fresh);
        auto next = pool.submit([] { RenderWork(17); });
        next.get(); pool.wait_all(); fresh->control->complete();
    }
    check(fresh->control->status() == RenderStatus::Completed && fresh->control->work_used() == 17,
          "reused physical worker retained cancelled job state");
    check(context->control->status() == RenderStatus::Cancelled,
          "recovery mutated the cancelled job");
}
void failure() {
    auto context = std::make_shared<RenderContext>();
    context->control = std::make_shared<RenderControl>();
    context->control->start();
    ScopedRenderContext binding(context);
    ThreadPool pool(2);
    auto bad = pool.submit([] { throw std::runtime_error("original worker failure"); });
    bool original = false;
    try { bad.get(); }
    catch (const std::runtime_error& e) { original = std::string(e.what()) == "original worker failure"; }
    check(original, "lost original worker failure");
    auto sibling = pool.submit([] { throw std::logic_error("must not execute"); });
    original = false;
    try { sibling.get(); }
    catch (const std::runtime_error& e) { original = std::string(e.what()) == "original worker failure"; }
    pool.wait_all();
    check(original && context->control->status() == RenderStatus::Failed,
          "queued task ran after failure or replaced first diagnostic");
    context->control->cancel();
    check(context->control->status() == RenderStatus::Failed, "late cancellation replaced failure");
}
void directWorkers() {
    for (int workers : {1, 4}) {
        auto context = std::make_shared<RenderContext>();
        context->control = std::make_shared<RenderControl>(101);
        context->control->start();
        ScopedRenderContext binding(context);
        stopped([&] {
            RunParallelWorkers(workers, [](int) { for (;;) RenderWork(); });
        }, RenderStatus::WorkExhausted);
        check(context->control->work_used() == 101,
              "direct workers did not share the exact limit");

        auto cancelled = std::make_shared<RenderContext>();
        cancelled->control = std::make_shared<RenderControl>();
        cancelled->control->start(); cancelled->control->cancel();
        {
            ScopedRenderContext nested(cancelled);
            std::atomic<int> calls{0};
            stopped([&] { RunParallelWorkers(workers, [&](int) { ++calls; }); },
                    RenderStatus::Cancelled);
            check(calls == 0, "pre-cancelled direct worker executed");
        }
        check(CaptureRenderContext() == context && current_render_control == context->control.get(),
              "direct worker failure lost caller scope");
    }
    auto context = std::make_shared<RenderContext>();
    context->control = std::make_shared<RenderControl>();
    context->control->start();
    ScopedRenderContext binding(context);
    bool original = false;
    try {
        RunParallelWorkers(4, [](int i) {
            if (i == 3) throw std::runtime_error("direct worker failure");
            for (;;) RenderWork();
        });
    } catch (const std::runtime_error& e) {
        original = std::string(e.what()) == "direct worker failure";
    }
    check(original && context->control->status() == RenderStatus::Failed,
          "direct worker did not drain peers with the first failure");
}
void independentJobs() {
    auto cancelled = std::make_shared<RenderContext>();
    auto healthy = std::make_shared<RenderContext>();
    cancelled->control = std::make_shared<RenderControl>();
    healthy->control = std::make_shared<RenderControl>(19);
    cancelled->control->start(); healthy->control->start();
    ThreadPool pool(2);
    std::promise<void> entered;
    auto ready = entered.get_future();
    std::future<void> interrupted, completed;
    {
        ScopedRenderContext scope(cancelled);
        interrupted = pool.submit([&] {
            entered.set_value();
            for (;;) RenderWork();
        });
    }
    check(ready.wait_for(std::chrono::seconds(5)) == std::future_status::ready,
          "independent job did not start");
    {
        ScopedRenderContext scope(healthy);
        completed = pool.submit([&] {
            cancelled->control->cancel();
            RenderWork(19);
        });
    }
    completed.get();
    stopped([&] { interrupted.get(); }, RenderStatus::Cancelled);
    pool.wait_all(); healthy->control->complete();
    check(healthy->control->work_used() == 19 && healthy->control->status() == RenderStatus::Completed,
          "cancellation leaked between concurrent jobs on one pool");
    check(!current_render_control, "worker control leaked into caller");
}
void formulaGuards() {
    auto iterationError = [](auto&& fn, FormulaIterationFailure expected) {
        bool caught = false;
        try { fn(); }
        catch (const FormulaIterationError& error) { caught = error.failure() == expected; }
        check(caught, "local iteration guard changed its failure category");
    };
    // The new shared limit must not weaken the pre-existing local protection.
    FormulaIterationGuard local("native local policy");
    iterationError([&] { local.validateSlots(0, -1, 5); }, FormulaIterationFailure::InvalidSlotRange);
    int32_t empty[6]{};
    iterationError([&] { local.requireReachableFormula(empty, 0, 0, 5, 0); },
                   FormulaIterationFailure::EmptyCycle);
    for (uint32_t i = 0; i < FormulaIterationPolicy::maxCallsWithoutProgress; ++i)
        local.beforeFormula(0, 0);
    iterationError([&] { local.beforeFormula(0, 0); }, FormulaIterationFailure::NoProgressBudget);
    local.beforeFormula(1, 0); // Actual progress admits a fresh local allowance.
    for (uint32_t i = 1; i < FormulaIterationPolicy::maxCallsWithoutProgress; ++i)
        local.beforeFormula(0, 0); // Rolling back the count must not reset it.
    iterationError([&] { local.beforeFormula(0, 0); }, FormulaIterationFailure::NoProgressBudget);

    RenderControl control(4);
    control.start();
    {
        ScopedRenderControl scope(&control);
        FormulaIterationGuard guard("native test");
        guard.validateSlots(0, 0, 5);
        // Counted iterations intentionally stay fixed; shared work still moves.
        for (int i = 0; i < 4; ++i) guard.beforeFormula(0, 0);
        stopped([&] { guard.beforeFormula(0, 0); }, RenderStatus::WorkExhausted);
    }
    check(control.work_used() == 4, "job budget confused formula work with Pascal iterations");
    RenderControl cancel;
    cancel.start();
    {
        ScopedRenderControl scope(&cancel);
        FormulaWorkBudget body("native body", 3);
        body.beforeStep();
        check(body.steps() == 1 && cancel.work_used() == 0,
              "portable body changed cross-profile event accounting");
        cancel.cancel();
        stopped([&] { body.beforeStep(); }, RenderStatus::Cancelled);
        check(body.steps() == 1, "cancelled body changed local counter");
    }
    const int rounding = std::fegetround();
    std::feclearexcept(FE_ALL_EXCEPT);
    RenderControl integerOnly(10);
    integerOnly.start(); integerOnly.consume(10); integerOnly.complete();
    check(std::fegetround() == rounding && std::fetestexcept(FE_ALL_EXCEPT) == 0,
          "control modified floating-point environment");
}
} // namespace
int main() {
    try {
        boundaries();
        for (int workers : {1, 2, 4, 16}) sharing(workers);
        cancellationAndRecovery(); failure(); directWorkers(); independentJobs(); formulaGuards();
        std::printf("Render control: %d checks passed; native runtime/guard contracts, not image parity.\n", checks);
        return 0;
    } catch (const std::exception& error) {
        std::fprintf(stderr, "Render control FAILED: %s\n", error.what()); return 1;
    }
}
