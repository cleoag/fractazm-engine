#pragma once
#import <Metal/Metal.h>
#include "native_ao_host.h"
#include "native_ao_readback.h"
#include "execution_policy.h"
#include "image_execution_policy.h"
#include "comparison/metal_exact/metal_device.h"
#include <functional>
namespace mb3d {
struct NativeAOResult {
    id<MTLBuffer> surface=nil;
    mb3d_native_ao::Totals totals;
    std::uint64_t dispatches=0,commands=0,seed_planning_draws=0,seed_shortcuts=0;
    std::uint64_t seed_generation_commands=0;
    std::uint32_t certificate_version=0;
    std::uint64_t certificate_levels=0,certificate_pixels=0,certificate_known=0,certificate_used=0;
    double gpu_ms=0,max_command_ms=0;
    // DE AO conditional contract: the discarded pixel pass's carried reads.
    // Its commands and GPU time remain in the counters above.
    std::uint64_t difs_rerun_carried_reads=0,difs_rerun_carried_pixels=0;
    bool difs_conditional_pixels=false;
};
// Caller must pass RenderPoll for cancellation and include returned dispatch
// time plus setup/readback in its complete AO stage. Inputs remain unchanged.
NativeAOResult RunNativeScreenAO(metal_exact::gpu::Metal& metal,
    const NativeAOPreparation& preparation,id<MTLBuffer> input,
    std::uint64_t allocation_budget,const native_metal::ScreenAOExecutionPolicy&,
    const std::function<void()>& poll);
NativeAOResult RunNativeDistanceAO(metal_exact::gpu::Metal& metal,
    const mb3d_native_ao::DEParams& preparation,const ExactScene& scene,
    const mb3d_native::NativeFormulaProgram& program,const mb3d_native::NativeFormulaProgram* second_part,
    id<MTLBuffer> camera_rays,
    id<MTLBuffer> input,id<MTLBuffer> map_descriptors,id<MTLBuffer> map_pixels,
    std::uint32_t map_count,std::uint32_t map_elements,
    std::uint64_t allocation_budget,const native_metal::DistanceAOExecutionPolicy&,
    const std::function<void()>& poll);
}
