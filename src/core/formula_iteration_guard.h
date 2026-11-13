#pragma once

#include "core/render_control.h"
#include <cstdint>
#include <stdexcept>
#include <string>

namespace mb3d {

// Ordinary interpreter policy, independent of the scene's counted maxIt.
// A finite chain requiring more consecutive calls without counted progress is
// explicitly unsupported by this budget; exhaustion is not proof of a cycle.
// This bounds dispatches, not time spent inside an individual formula callback.
struct FormulaIterationPolicy {
    static constexpr uint32_t maxCallsWithoutProgress = 1u << 20;
};

enum class FormulaIterationFailure {
    InvalidSlotRange,
    EmptyCycle,
    NoProgressBudget
};

class FormulaIterationError : public std::runtime_error {
public:
    FormulaIterationError(FormulaIterationFailure failure, const char* driver,
                          int slot, int countedIteration, uint32_t calls)
        : std::runtime_error(message(failure, driver, slot, countedIteration, calls)),
          failure_(failure) {}

    FormulaIterationFailure failure() const noexcept { return failure_; }

private:
    static std::string message(FormulaIterationFailure failure, const char* driver,
                               int slot, int countedIteration, uint32_t calls) {
        const char* reason = failure == FormulaIterationFailure::EmptyCycle
            ? "empty hybrid cycle (no active formula is reachable)"
            : failure == FormulaIterationFailure::InvalidSlotRange
                ? "hybrid slot index outside [0,5]"
                : "no-progress work budget exhausted";
        return std::string("Formula iteration failed: ") + reason +
            " (driver=" + driver + ", slot=" + std::to_string(slot) +
            ", ItResultI=" + std::to_string(countedIteration) +
            ", calls_without_progress=" + std::to_string(calls) +
            ", limit=" + std::to_string(FormulaIterationPolicy::maxCallsWithoutProgress) + ")";
    }

    FormulaIterationFailure failure_;
};

// Per invocation: no global/TLS setting or changes to the Pascal-compatible ABI.
// All checks on the success path use integers and leave orbit state untouched.
class FormulaIterationGuard {
public:
    explicit FormulaIterationGuard(const char* driver) : driver_(driver) {}

    void validateSlots(int start, int repeat, int end) const {
        const int slots[] = {start, repeat, end};
        for (int slot : slots) {
            if (slot < 0 || slot >= 6) {
                throw FormulaIterationError(FormulaIterationFailure::InvalidSlotRange,
                                            driver_, slot, 0, 0);
            }
        }
    }

    void visitInactiveSlot(int slot, uint32_t& visited, int countedIteration) const {
        const uint32_t bit = 1u << slot;
        if (visited & bit) {
            // No formula ran during this search, so no conditional can change
            // the counts before this exact selection state repeats.
            throw FormulaIterationError(FormulaIterationFailure::EmptyCycle,
                                        driver_, slot, countedIteration, calls_);
        }
        visited |= bit;
    }

    void beforeFormula(int countedIteration, int slot) {
        if (countedIteration > highWaterIteration_) {
            highWaterIteration_ = countedIteration;
            calls_ = 0;
        }
        if (calls_ == FormulaIterationPolicy::maxCallsWithoutProgress) {
            throw FormulaIterationError(FormulaIterationFailure::NoProgressBudget,
                                        driver_, slot, countedIteration, calls_);
        }
        RenderWork();
        ++calls_;
    }

    void requireReachableFormula(const int32_t* counts, int slot, int repeat,
                                 int end, int countedIteration) const {
        uint32_t visited = 0;
        while ((counts[slot] & 0x7fffffff) == 0) {
            visitInactiveSlot(slot, visited, countedIteration);
            if (++slot > end) slot = repeat;
        }
    }

private:
    const char* driver_;
    int highWaterIteration_ = 0;
    uint32_t calls_ = 0;
};

} // namespace mb3d
