#pragma once
#include "postprocess_plan.h"
#include "image_execution_policy.h"
#include <functional>
#include <cstdint>
namespace mb3d::native_metal {
struct ZNormalsActivity {uint64_t records=0,foreground=0,changed=0,neighbors=0,dispatches=0;uint32_t faults=0;};
}
#ifdef __OBJC__
#include "comparison/metal_exact/metal_device.h"
namespace mb3d::native_metal {
struct ZNormalsOutput {id<MTLBuffer> surface=nil;ZNormalsActivity activity;};
ZNormalsOutput RunNativeZNormals(metal_exact::gpu::Metal&,const mb3d_zn::Params&,id<MTLBuffer> source,
                                const ZNormalsExecutionPolicy&,const std::function<void()>& poll={});
}
#endif
