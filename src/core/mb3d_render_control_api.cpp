#include "core/mb3d_render_control_api.h"

#include <memory>

MB3DRenderStatus mb3d_api_render_status(mb3d::RenderStatus status) noexcept {
    switch (status) {
    case mb3d::RenderStatus::Prepared: return MB3D_RENDER_PREPARED;
    case mb3d::RenderStatus::Running: return MB3D_RENDER_RUNNING;
    case mb3d::RenderStatus::Completed: return MB3D_RENDER_SUCCESS;
    case mb3d::RenderStatus::Cancelled: return MB3D_RENDER_CANCELLED;
    case mb3d::RenderStatus::WorkExhausted: return MB3D_RENDER_WORK_LIMIT_EXCEEDED;
    case mb3d::RenderStatus::Unsupported: return MB3D_RENDER_UNSUPPORTED;
    case mb3d::RenderStatus::Failed: return MB3D_RENDER_FAILED;
    }
    return MB3D_RENDER_FAILED;
}

extern "C" {

MB3DRenderControl* mb3d_render_control_create(uint64_t max_work) {
    try {
        auto owner = std::make_unique<MB3DRenderControl>();
        owner->value = std::make_shared<mb3d::RenderControl>(max_work);
        return owner.release();
    } catch (...) {
        return nullptr;
    }
}

void mb3d_render_control_cancel(MB3DRenderControl* control) {
    if (control) control->value->cancel();
}

uint64_t mb3d_render_control_work_used(const MB3DRenderControl* control) {
    return control ? control->value->work_used() : 0;
}

MB3DRenderStatus mb3d_render_control_status(const MB3DRenderControl* control) {
    return control ? mb3d_api_render_status(control->value->status()) : MB3D_RENDER_FAILED;
}

void mb3d_render_control_destroy(MB3DRenderControl* control) {
    delete control;
}

} // extern "C"
