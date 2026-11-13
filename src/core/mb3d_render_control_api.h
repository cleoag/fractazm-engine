#pragma once

#include "mb3d/mb3d.h"
#include "core/render_control.h"

// Private definition: callers of the public C ABI only see an opaque owner.
struct MB3DRenderControl {
    mb3d::RenderControlHandle value;
};

MB3DRenderStatus mb3d_api_render_status(mb3d::RenderStatus status) noexcept;
