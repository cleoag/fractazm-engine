#pragma once

namespace mb3d {

// A render's arithmetic policy is independent of CPU architecture and backend.
enum class PrecisionMode { Compatible, Native };

constexpr PrecisionMode DefaultPrecisionMode() noexcept {
#ifdef MB3D_DEFAULT_PRECISION_NATIVE
    return PrecisionMode::Native;
#else
    return PrecisionMode::Compatible;
#endif
}

const char* PrecisionModeName(PrecisionMode mode) noexcept;
bool ParsePrecisionMode(const char* value, PrecisionMode& mode) noexcept;
PrecisionMode CurrentPrecisionMode() noexcept;
bool NativePrecisionEnabled() noexcept;

// Worker/context bindings propagate the resolved per-render choice. Nested
// calls restore their caller; unrelated renders never mutate a global policy.
class ScopedNumericPrecision {
public:
    explicit ScopedNumericPrecision(PrecisionMode mode) noexcept;
    ~ScopedNumericPrecision();
    ScopedNumericPrecision(const ScopedNumericPrecision&) = delete;
    ScopedNumericPrecision& operator=(const ScopedNumericPrecision&) = delete;
private:
    PrecisionMode previous_;
};

} // namespace mb3d
