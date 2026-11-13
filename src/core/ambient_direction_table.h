#pragma once
#include "core/math3d.h"
#include "core/numeric_precision.h"
#include <array>
#include <mutex>

namespace mb3d::detail {
using AmbientDirections = std::array<std::array<float, 2>, 32>;

// The selected trigonometric policy affects even Single stores near quadrant
// boundaries. Each immutable table must be initialized under its own policy.
class AmbientDirectionTables {
    struct Entry { std::once_flag once; AmbientDirections values{}; } entries_[2];
public:
    const AmbientDirections& get(PrecisionMode mode) {
        auto& entry=entries_[mode==PrecisionMode::Native ? 1 : 0];
        std::call_once(entry.once,[&] {
            const ScopedNumericPrecision precision(mode);
            for(int i=0;i<32;++i) {
                double sine,cosine;
                SinCosD(double(i)*Pi/16.0,sine,cosine);
                entry.values[i]={static_cast<float>(sine),static_cast<float>(cosine)};
            }
        });
        return entry.values;
    }
};
} // namespace mb3d::detail
