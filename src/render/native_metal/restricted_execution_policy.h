#pragma once
// The existing restricted Native adapter is a separate production engine.
// Its private formula state is not the general engine's 47-word lane. Keep
// Compatible/standalone RunRenderStages callers outside this host policy.
#include "execution_policy.h"
#include "image_execution_policy.h"
#include <string>
namespace mb3d::native_metal {
struct RestrictedExecutionRequest {
    uint32_t width=0,height=0;
    bool shadows=false,ambient=false,reflections=false;
    bool operator==(const RestrictedExecutionRequest&) const=default;
};
struct RestrictedDispatchOptions {
    std::string normalMode="parallel",shadowMode="parallel";
    unsigned normalThreads=32,shadowPrepThreads=32,shadowTraceThreads=32;
    unsigned shadowRows=256,primaryRows=256,primaryThreads=32,primaryTileWidth=8;
};
class RestrictedExecutionPolicy {
    RestrictedExecutionRequest request_;
    RestrictedDispatchOptions dispatch_;
public:
    explicit RestrictedExecutionPolicy(const RestrictedExecutionRequest& r):request_(r) {
        if(!r.width||r.width>4096||!r.height||r.height>4096)
            throw std::invalid_argument("Invalid restricted Native execution dimensions");
    }
    void validate(const RestrictedExecutionRequest& r) const {
        if(!(r==request_))throw std::invalid_argument("Restricted Native execution policy input mismatch");
    }
    const RestrictedDispatchOptions& dispatch() const{return dispatch_;}
    StateDomain formula_domain() const{return StateDomain::RestrictedPrivateOrbit;}
    uint32_t normal_probes_per_pixel() const{return 6;}
    LogicalOwner shadow_owner() const{return LogicalOwner::WholeRow;}
    uint32_t reflection_batch() const{return 4096;}
    uint32_t reflection_group_width() const{return 32;}
    std::string describe() const {
        std::ostringstream s;s<<"restricted-v1|configured-policy-not-activity|domain="<<uint32_t(formula_domain())
            <<"|size="<<request_.width<<'x'<<request_.height
            <<"|shadow="<<request_.shadows<<",ao="<<request_.ambient<<",reflection="<<request_.reflections
            <<"|primary-rows="<<dispatch_.primaryRows<<",group="<<dispatch_.primaryThreads<<",tile="<<dispatch_.primaryTileWidth
            <<"|normal-probes-per-pixel="<<normal_probes_per_pixel()<<",group="<<dispatch_.normalThreads
            <<"|shadow-resolve-owner="<<uint32_t(shadow_owner())<<",row-batch="<<dispatch_.shadowRows
            <<"|shadow-prepare="<<dispatch_.shadowPrepThreads<<",trace="<<dispatch_.shadowTraceThreads
            <<"|reflection-batch="<<reflection_batch()<<",group="<<reflection_group_width();return s.str();
    }
    // Primary, six normal probes, shadow prepare/trace are independent. The
    // shadow resolve keeps original light order and row shortcuts. Restricted
    // exact AO is deterministic and keeps ordered levels/wavelet axes; it does
    // not inherit general random-AO semantics. Reflections keep dependent
    // bounces within the pixel and preserve incoming RGB on a failed attempt.
    // Some exact image kernels signal malformed inputs only by returning:
    // preparation and host validation remain required, not generic fault bits.
};
}
