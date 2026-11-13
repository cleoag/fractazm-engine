#pragma once
#include "postprocess_plan.h"
#include "image_execution_policy.h"
#include <cstdint>
#include <functional>
namespace mb3d::native_metal {
struct DOFActivity {
    uint64_t records=0,passes=0,positive_radius_sources=0,sort_work=0,candidate_scan_checks=0,candidate_checks=0;
    uint64_t contributing_pairs=0,changed_pixels=0,tile_batches=0,dispatches=0;
    uint32_t maximum_radius=0,faults=0;
};
}
#ifdef __OBJC__
#include "comparison/metal_exact/metal_device.h"
namespace mb3d::native_metal {
struct DOFOutput { id<MTLBuffer> rgb=nil; DOFActivity activity; };
DOFOutput RunNativeDOF(metal_exact::gpu::Metal&,const mb3d_dof::Params&,
                      id<MTLBuffer> surface,id<MTLBuffer> input,
                      const DOFExecutionPolicy&,const std::function<void()>& poll = {});
}
#endif
