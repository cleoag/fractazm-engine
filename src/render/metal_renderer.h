#pragma once
#include "render/backend.h"
#include <string>

namespace mb3d {
enum class NativeMetalStatus { Completed, Unsupported, Unavailable, Failed, Fp32Range, NeedsGeneralInitialInside };

struct NativeMetalAttempt {
    NativeMetalStatus status = NativeMetalStatus::Failed;
    RenderResult result;
    std::string reason;
    std::string device;
    std::string library;
    ReflectionActivity reflections;
    MetalExecutionActivity execution;
};

// Unsupported/Unavailable are pre-dispatch outcomes eligible for caller policy
// fallback. Fp32Range is the one post-dispatch outcome eligible for it: a valid
// frame whose FP32 range exits exceed the general engine's pixel policy (user
// rule change of 2026-10-27). Failed includes wrong library profile, GPU errors
// and invalid output; callers must not hide those failures by retrying CPU. Parameters are copied
// during preparation; no PNG, file load, CPU render/oracle or subprocess occurs.
NativeMetalAttempt RenderNativeMetal(TMandHeader10& header, THeaderCustomAddon& addon,
                                    const RenderOptions& opts,
                                    const std::string& libraryPath);
} // namespace mb3d
