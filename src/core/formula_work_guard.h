#pragma once

#include "core/render_control.h"
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <type_traits>

namespace mb3d {

// Explicit ordinary-formula work policy, independent of Pascal's iteration
// counters. Exhaustion rejects a call; it never returns a truncated orbit.
struct FormulaWorkPolicy {
    static constexpr uint32_t maxInnerStepsPerCall = 1u << 20;
};

// Private C++/Win64-helper ABI. Only the owning callback writes this counter;
// there is no global state and no change to the Pascal parameter/record ABI.
struct FormulaWorkCounter {
    uint32_t steps;
    uint32_t limit;
};
static_assert(std::is_standard_layout_v<FormulaWorkCounter>);
static_assert(sizeof(FormulaWorkCounter) == 8);
static_assert(offsetof(FormulaWorkCounter, steps) == 0);
static_assert(offsetof(FormulaWorkCounter, limit) == 4);

class FormulaWorkError : public std::runtime_error {
public:
    FormulaWorkError(const char* formula, uint32_t steps, uint32_t limit)
        : std::runtime_error(std::string("Formula work budget exhausted (formula=") +
              formula + ", steps=" + std::to_string(steps) +
              ", limit=" + std::to_string(limit) + ")"),
          steps_(steps), limit_(limit) {}

    uint32_t steps() const noexcept { return steps_; }
    uint32_t limit() const noexcept { return limit_; }

private:
    uint32_t steps_;
    uint32_t limit_;
};

class FormulaWorkBudget {
public:
    explicit FormulaWorkBudget(const char* formula,
                               uint32_t limit = FormulaWorkPolicy::maxInnerStepsPerCall)
        : formula_(formula), counter_{0, limit} {}

    // Call immediately before an actual loop body, including its first step.
    // A geometric return before entering the loop consumes no work. A return
    // from the final permitted body succeeds; only the next body is rejected.
    void beforeStep() {
        RenderPoll();
        if (counter_.steps >= counter_.limit) fail();
        ++counter_.steps;
    }

    FormulaWorkCounter* nativeCounter() {
        RenderPoll();
        return &counter_;
    }
    uint32_t steps() const noexcept { return counter_.steps; }

    // Native helpers restore their x87 control word and return 0 (complete)
    // or 1 (exhausted). Exceptions must originate in C++, never cross ASM.
    void checkNativeStatus(uint32_t status) const {
        RenderPoll();
        if (status != 0) fail();
    }

private:
    [[noreturn]] void fail() const {
        throw FormulaWorkError(formula_, counter_.steps, counter_.limit);
    }

    const char* formula_;
    FormulaWorkCounter counter_;
};

} // namespace mb3d
