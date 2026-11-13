#pragma once
// Private P2 allocation contract. Not a serialized/stable numerical ABI.
// Callers charge allocations that coexist at the peak, including superseded
// jobs until their submitted command completes. Reused storage is charged once.
#include <algorithm>
#include <cstdint>
#include <limits>
namespace mb3d::native_metal::continuation {
inline constexpr uint64_t MaxRayFrameBytes=16u*1024u;
inline constexpr uint64_t SlotOverheadBytes=128;
inline constexpr uint32_t MaxActiveFrames=4096;
inline constexpr uint32_t MaxActivationDepth=14;
struct PeakCharges {
    uint64_t retained;        // old CPU/GPU leases, including an in-flight job
    uint64_t prepared_cpu;    // new decoded maps, program/camera preparation
    uint64_t incoming_gpu;    // new immutable/mutable GPU storage except pool
    uint64_t published_cpu;   // RGB retained by consumers (not a GPU alias)
    uint64_t staging;         // ready records, private RGB and host bookkeeping
};
struct PoolPlan {bool admitted=false;uint32_t frames=0;uint64_t stride=0,peak=0;};
inline PoolPlan plan_pool(PeakCharges charges,uint64_t frame_bytes,uint32_t requested,
                          uint32_t workers,bool independent,uint64_t budget) {
    if(!frame_bytes||frame_bytes>MaxRayFrameBytes||!requested||requested>MaxActiveFrames||
       !workers||workers>64||(!independent&&requested!=workers))return {};
    uint64_t peak=0;
    for(auto value:{charges.retained,charges.prepared_cpu,charges.incoming_gpu,
                    charges.published_cpu,charges.staging}){
        if(peak>budget||value>budget-peak)return {};
        peak+=value;
    }
    const uint64_t stride=((frame_bytes+15u)&~uint64_t(15u))+SlotOverheadBytes;
    const uint64_t available=(budget-peak)/stride;
    const auto count=uint32_t(std::min(uint64_t(requested),available));
    // Dependent owners keep one active ray each. Reducing that set needs a
    // separately proved scheduler; it is not an implicit allocation fallback.
    if(!count||(!independent&&count!=workers))return {};
    return {true,count,stride,peak+uint64_t(count)*stride};
}
} // namespace mb3d::native_metal::continuation
