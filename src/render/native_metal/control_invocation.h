#pragma once
#include "core/render_control.h"
#include <utility>

namespace mb3d::native_metal {
// Owns only a successfully started invocation. Unsupported preparation returns
// before construction/start so CPU fallback can start the same control once.
class ControlInvocation {
    RenderControlHandle control_;
    bool owns_=false;
    bool* outer_ownership_=nullptr;
public:
    explicit ControlInvocation(RenderControlHandle control,bool* outer_ownership=nullptr)
        :control_(std::move(control)),outer_ownership_(outer_ownership) {}
    ControlInvocation(const ControlInvocation&)=delete;
    ControlInvocation& operator=(const ControlInvocation&)=delete;
    ~ControlInvocation() { if(owns_)control_->fail(); }
    void start() {
        if(!control_)return;
        control_->start(); // A reused control throws before ownership is acquired.
        owns_=true;
        if(outer_ownership_)*outer_ownership_=true;
        control_->poll();
    }
    // Metal work units count admitted GPU thread invocations, including ABI
    // markers. They are not CPU formula events or a claim of completed work.
    // Admission precedes encoding/commit; existing per-kernel limits still apply.
    void work(uint64_t units) { if(owns_)control_->consume(units); }
    void poll() const { if(owns_)control_->poll(); }
    void complete() { if(owns_)control_->complete(); }
    // An ordered re-render continues the started invocation and its budget.
    bool started() const { return owns_; }
    void fail() { if(owns_)control_->fail(); }
};
}
