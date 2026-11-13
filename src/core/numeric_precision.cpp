#include "core/numeric_precision.h"
#include <cstring>

namespace mb3d {
namespace {
// Low-level numeric fixtures have no render owner and retain the reference
// contract. Public RenderOptions use the separately selected build default.
thread_local PrecisionMode precision = PrecisionMode::Compatible;
}
const char* PrecisionModeName(PrecisionMode mode) noexcept {
    return mode == PrecisionMode::Native ? "native" : "compatible";
}
bool ParsePrecisionMode(const char* value, PrecisionMode& mode) noexcept {
    if (value && std::strcmp(value, "compatible") == 0) {
        mode = PrecisionMode::Compatible;
        return true;
    }
    if (value && std::strcmp(value, "native") == 0) {
        mode = PrecisionMode::Native;
        return true;
    }
    return false;
}
PrecisionMode CurrentPrecisionMode() noexcept { return precision; }
bool NativePrecisionEnabled() noexcept { return precision == PrecisionMode::Native; }
ScopedNumericPrecision::ScopedNumericPrecision(PrecisionMode mode) noexcept
    : previous_(precision) { precision = mode; }
ScopedNumericPrecision::~ScopedNumericPrecision() { precision = previous_; }
} // namespace mb3d
