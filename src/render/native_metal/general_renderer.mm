#include <mutex>
#include "native_inside_host.h"
#include "inside_readback.h"
#include "reflection_work_readback.h"
#include "general_renderer.h"
#include "control_invocation.h"
#include "general_scene_host.h"
#include "render_plan.h"
#include "general_geometry_abi.h"
#include "smoothing_readback.h"
#include "primary_range_readback.h"
#include "general_normals_host.h"
#include "generic_shadow_abi.h"
#include "soft_shadow_host.h"
#include "soft_shadow_readback.h"
#include "multi_shadow_readback.h"
#include "logical_lane_abi.h"
#include "pixel_independence.h"
#include "wrapper_policy.h"
#include "generic_reflection_abi.h"
#include "generic_color_host.h"
#include "dof_host.h"
#include "dof_dispatch.h"
#include "zbuffer_host.h"
#include "zbuffer_dispatch.h"
#include "native_ao_dispatch.h"
#include "seed_scan_dispatch.h"
#include "comparison/metal_exact/native_reflection_host.h"
#include "comparison/metal_exact/render_dispatch.h"
#include "comparison/metal_exact/scene_host_internal.h"
#include <bit>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <limits>
#include <string_view>
#include <thread>

namespace mb3d::native_metal {
namespace {
namespace gpu=metal_exact::gpu;
using namespace metal_exact;
using mb3d_native::NativeGeometryRecord;
struct Timings {
    RenderProfile& profile;
    gpu::Clock::time_point start=gpu::Clock::now(),entered;
    int active=-1;
    void begin(RenderProfileStage stage) {
        NoteRenderStage(stage);
        active=int(stage);entered=gpu::Clock::now();
        profile.stages[active].state="interrupted";
    }
    void end(bool completed=true) {
        if(active<0)return;
        profile.stages[active].seconds+=gpu::elapsed(entered)/1000;
        profile.stages[active].state=completed?"completed":"interrupted";active=-1;
    }
    ~Timings(){end(false);profile.total_seconds=gpu::elapsed(start)/1000;}
};
struct CameraBuffers {
    id<MTLBuffer> params=nil,columns=nil,rows=nil,paintColumns=nil,paintRows=nil,rays=nil;
    void bind(id<MTLComputeCommandEncoder> e,bool paint=false) const {
        [e setBuffer:params offset:0 atIndex:23];
        [e setBuffer:paint?paintColumns:columns offset:0 atIndex:24];
        [e setBuffer:paint?paintRows:rows offset:0 atIndex:25];
        [e setBuffer:rays offset:0 atIndex:26];
    }
};
struct ImagePipelines {
    id<MTLComputePipelineState> color=nil;
    explicit ImagePipelines(gpu::Metal& m) {color=m.pipeline(@"native_camera_general_color");}
};
void colorStage(gpu::Metal& m,const ImagePipelines& p,const ExactScene& scene,
                const gpu::RenderStageBuffers& b,id<MTLBuffer> interior,id<MTLBuffer> lights,
                id<MTLBuffer> maps,id<MTLBuffer> mapSamples,const CameraBuffers& camera,Timings& timing) {
    timing.begin(RenderProfileStage::Color);
    const uint32_t launch[]={0,scene.width,scene.height};
    gpu::dispatchStage(m,p.color,scene.width,[&](id<MTLComputeCommandEncoder> e){
        [e setBuffer:b.colorScene offset:0 atIndex:0];[e setBuffer:b.colorCols offset:0 atIndex:1];
        [e setBuffer:b.colorRows offset:0 atIndex:2];[e setBuffer:b.shaded offset:0 atIndex:3];
        [e setBuffer:b.rgb offset:0 atIndex:4];[e setBytes:launch length:sizeof(launch) atIndex:5];
        [e setBuffer:interior offset:0 atIndex:6];[e setBuffer:lights offset:0 atIndex:7];
        [e setBuffer:maps offset:0 atIndex:8];[e setBuffer:mapSamples offset:0 atIndex:9];camera.bind(e,true);
    },scene.height);
    timing.end();
}
// Only the frame's FP32 range policy rejected it: after every record validated,
// or earlier when the primary geometry alone already exceeds the policy.
struct Fp32RangeExceeded:std::runtime_error{using std::runtime_error::runtime_error;};
// FP32 range policy: range exits complete as escapes while they stay a small
// part of the frame's geometry (DE AO exits only shade). Beyond this percent
// of pixels the scene does not fit binary32 on this engine.
constexpr uint64_t kFp32RangePixelPercent=1;
// Diagnostic MB3D_NATIVE_FP32_RANGE_PERCENT=0..100 replaces the policy percent,
// so a frame beyond the policy can be compared with CPU. Production keeps 1.
uint64_t fp32RangePixelPercent() {
    if(const char* value=std::getenv("MB3D_NATIVE_FP32_RANGE_PERCENT")){
        char* end=nullptr;const long percent=std::strtol(value,&end,10);
        gpu::require(end&&!*end&&percent>=0&&percent<=100,"Invalid MB3D_NATIVE_FP32_RANGE_PERCENT");
        return uint64_t(percent);
    }
    return kFp32RangePixelPercent;
}
// Pixel ownership came from the conditional dIFS contract and a record counted
// a carried-state read: the frame is not order independent.
struct CarriedStateObserved:std::runtime_error{
    uint64_t reads,pixels;
    CarriedStateObserved(uint64_t r,uint64_t p):std::runtime_error("Native dIFS carried state"),reads(r),pixels(p){}
};
void attempt(TMandHeader10& header,THeaderCustomAddon& addon,const RenderOptions& opts,
             const std::string& path,NativeMetalAttempt& out,ControlInvocation& invocation,bool ordered,
             gpu::SharedMetal* shared) {
    auto reject=[&](NativeMetalStatus status,const std::string& message){
        out.status=status;out.reason=message;out.result.error=message;out.result.status=RenderStatus::Failed;
    };
    if(opts.native_metal_smoothing_policy>1)
        return reject(NativeMetalStatus::Failed,"Invalid Native smoothing policy");
    if(opts.precision!=PrecisionMode::Native || opts.mc_samples ||
       opts.diag_only || !opts.diag_points.empty() || !opts.diag_dump_path.empty() ||
       !opts.ray_setup_dump_path.empty() || !opts.primary_trace_dump_path.empty() ||
       !opts.parity_trace_dump_path.empty() || !opts.parity_trace_start_stage.empty() ||
       !opts.parity_trace_end_stage.empty() || !opts.parity_trace_detail.empty() ||
       opts.parity_trace_schema_mode || opts.parity_trace_stage_state)
        return reject(NativeMetalStatus::Unsupported,"Native general options are not implemented");
    const int width=opts.width_override>0?opts.width_override:header.Width;
    const int height=opts.height_override>0?opts.height_override:header.Height;
    if(width<1||height<1||width>4096||height>4096)
        return reject(NativeMetalStatus::Unsupported,"Native general image dimensions outside 1..4096");
    const auto precision=ScopedNumericPrecision(PrecisionMode::Native);
    auto& result=out.result;auto& profile=result.profile;
    profile.enabled=opts.profile_timing;profile.precision=PrecisionMode::Native;
    profile.width=width;profile.height=height;profile.workers=0;
    Timings timing{profile};timing.begin(RenderProfileStage::Preparation);
    PreparedRenderPlan plan;
    try {
        RenderPlanRequest request;
        request.width=unsigned(width);request.height=unsigned(height);request.logical_workers=opts.thread_count;
        request.resource_root=opts.resource_root.empty()?std::filesystem::current_path().string():opts.resource_root;
        request.deterministic=opts.deterministic_mode;request.no_dof=opts.no_dof;
        request.no_reflections=opts.no_reflections;request.no_shadows=opts.no_shadows;request.no_ao=opts.no_ao;
        request.ordered_carried_state=ordered;
        request.lattice={opts.lattice_stride,opts.lattice_x,opts.lattice_y};
        plan=PrepareNativeRenderPlan(header,addon,request);
        if(!plan.ready())return reject(NativeMetalStatus::Unsupported,plan.firstFailure());
    }catch(const std::exception& e){return reject(NativeMetalStatus::Unsupported,e.what());}
    // The plan owns all prepared context and resources for the complete render.
    const auto& prepared=plan.scene;
    const auto& effective=plan.effective_header;
    const auto& packing=plan.packing;const auto& color=plan.color.base;const auto& interior=plan.color.interior;
    const auto& screenAO=plan.screen_ao;const auto& distanceAO=plan.distance_ao;
    const auto aoAlgorithm=plan.ao_algorithm,logicalWorkers=plan.logical_workers;
    const auto aoPreparationSeconds=plan.ao_preparation_seconds;
    const auto& dof=plan.dof;const auto& znormals=plan.z_normals;const auto& normalParams=plan.normals;
    const auto& insidePolicy=plan.inside;const auto& insideConfig=plan.inside_reflection;
    const auto& shadow=plan.shadow;const auto& softConfig=plan.soft_shadow;const auto& reflection=plan.reflection;
    // Secondary rays of one pixel: one per bounce, or every edge of a transmission tree.
    const uint32_t shadeCapacityRays=mb3d_native::native_reflection_shade_capacity(reflection.max_bounces,insideConfig.transmission!=0)-1;
    const bool shadows=plan.shadows,softShadow=plan.soft_shadows,ambient=plan.ambient,reflections=plan.reflections;
    std::error_code ec;
    if(!std::filesystem::exists(path,ec)){
        if(ec)return reject(NativeMetalStatus::Failed,"Cannot inspect Native library: "+ec.message());
        return reject(NativeMetalStatus::Unavailable,"Native library is unavailable: "+path);
    }
    if(!std::filesystem::is_regular_file(path,ec)||ec)
        return reject(NativeMetalStatus::Failed,"Native library is not a regular file");
    id<MTLDevice> device=shared?shared->device:MTLCreateSystemDefaultDevice();
    if(!device)return reject(NativeMetalStatus::Unavailable,"Metal device unavailable");
    out.device=device.name.UTF8String?:"";out.library=std::filesystem::absolute(path).lexically_normal().string();
    const auto& scene=prepared.base.device;
    const uint32_t count=scene.width*scene.height;
    profile.width=scene.width;profile.height=scene.height;
    PreparedRenderResources resources;
    try{resources=PlanNativeRenderResources(plan,{uint64_t(device.maxBufferLength),uint64_t(device.recommendedMaxWorkingSetSize)});}
    catch(const std::exception& e){return reject(NativeMetalStatus::Unsupported,e.what());}
    const auto& sizes=resources.buffers;const auto& dofLimits=resources.dof_limits;
    const auto aoAllocationBudget=resources.ao_budget;
    gpu::require(resources.execution.has_value(),"Missing Native execution policy");
    const auto& execution=*resources.execution;
    execution.validate(prepared.formulas.device,NativeGeneralExecutionRequest(plan),
        prepared.formulas.program_count()>1?&prepared.formulas.second:nullptr);
    if(ambient&&aoAlgorithm==mb3d_native_ao::DistanceEstimate){
        gpu::require(resources.distance_execution.has_value(),"Missing Native DE AO execution policy");
        resources.distance_execution->validate(prepared.formulas.device,distanceAO,
            prepared.formulas.program_count()>1?&prepared.formulas.second:nullptr);
    }
    if(ambient&&aoAlgorithm!=mb3d_native_ao::DistanceEstimate){
        gpu::require(resources.screen_execution.has_value(),"Missing Native screen AO execution policy");
        resources.screen_execution->validate(screenAO);
    }
    gpu::require(resources.dof_execution.has_value()&&resources.znormal_execution.has_value(),"Missing Native postprocess execution policy");
    resources.dof_execution->validate(dof,resources.dof_limits);
    resources.znormal_execution->validate(znormals);
    const auto shadowRows=execution.shadow_rows(),shadowTasks=execution.shadow_tasks();
    const bool multiShadow=execution.multi_shadow(),parallelReflections=execution.parallel_reflections();
    const bool primaryPersistent=execution.primary_persistent(),primaryRandom=plan.primary_random;
    const auto& volume=prepared.volume;
    const bool volumeLight=scene.fog_iteration==mb3d_native::kNativeVolumeFog;
    const uint64_t volumeCells=volumeLight?uint64_t(volume.cube_size)*volume.cube_size*volume.side_count:0;
    // A map ray starts a fresh formula lane; carried-state programs would need
    // CalcVolLightMap's per-thread row order.
    if(volumeCells&&primaryPersistent)
        return reject(NativeMetalStatus::Unsupported,"Native volumetric light with carried formula state is not implemented");
    if(volumeCells*sizeof(float)>uint64_t(device.maxBufferLength)||volumeCells>std::numeric_limits<uint32_t>::max())
        return reject(NativeMetalStatus::Unsupported,"Native volumetric light map exceeds the Metal buffer limit");
    const bool shadowPersistent=execution.shadow_persistent(),parallelSoftShadows=execution.parallel_soft_shadows();
    const bool speculativeShadows=shadows&&execution.speculative_shadows();
    // No Unsupported outcome is allowed after the first GPU dispatch.
    std::array<uint32_t,6> formulaIds{};
    const uint32_t wrapperPolicy=mb3d_native::ProductionWrapperPolicy(prepared.formulas.device);
    for(unsigned i=0;i<6;++i){const auto& slot=prepared.formulas.device.slots[i];
        gpu::require(mb3d_native::ValidFormulaSelectorIdentity(slot.namespace_id,slot.id,slot.enabled),"Invalid Native formula specialization identity");
        formulaIds[i]=mb3d_native::FormulaPipelineSelector(slot.namespace_id,slot.id,slot.enabled);}
    if(opts.admission_only){
        out.result.success=true;out.result.status=RenderStatus::Completed;out.status=NativeMetalStatus::Completed;return;
    }
    if(!invocation.started())invocation.start();
    gpu::DispatchObserver observer;
    if(opts.control)observer={[&](uint64_t units){invocation.work(units);},[&]{invocation.poll();}};
    gpu::Metal m(path,scene.formula_kind,0,scene.capability_id,false,true,&formulaIds,prepared.formulas.device.driver,std::move(observer),opts.native_metal_smoothing_policy,true,wrapperPolicy,shared);
    // Before the first formula pipeline: specialization includes the combination.
    m.nativeFormulaType=prepared.formulas.device.formula_type;
    // Diagnostic MB3D_NATIVE_OUTLINE_DE=all outlines the formula program in
    // every pipeline. FMA contraction then differs at the call boundary, so
    // FP32 results may shift; production outlines reflection and shadow
    // kernels, and every kernel of the formulas below.
    if(const char* outline=std::getenv("MB3D_NATIVE_OUTLINE_DE");outline&&std::string_view(outline)=="all")m.nativeOutlineDE=1;
    // The same policy at a caller's request (MB3DRenderRequest.quick_pipelines).
    if(opts.native_metal_outline_formulas)m.nativeOutlineDE=1;
    // Inlined, these map formulas took the backend compiler minutes for the
    // primary and AO pipelines (cold single-formula scenes: _MapTranslate
    // 112 s, HeightMapIFS 165 s, HeightMap2IFS over 180 s). Outlined, each
    // scene compiled in under 4 s and the measured images were unchanged.
    for(const uint32_t id:formulaIds)if(id==10006u||id==10189u||id==10198u)m.nativeOutlineDE=1;
    if(m.nativeFormulaType){
        gpu::require(prepared.formulas.second.formula_type==m.nativeFormulaType,"Native DE combination program mismatch");
        m.nativeOrbitDriver2=prepared.formulas.second.driver;
    }
    out.execution.profile_kind=m.arithmeticProfile[0];out.execution.storage_bytes=m.arithmeticProfile[1];
    out.execution.engine="general_formula";
    out.execution.execution_plan_version=1;
    out.execution.execution_plan=DescribeNativeExecutionPolicy(resources);
    out.execution.primary_state_contract_version=resources.primary_state.version;
    out.execution.soft_shadow_state_contract_version=softShadow?resources.soft_shadow_state.version:0;
    // Reasons: 1 chain pixels, 2 whole rows, 3 persistent tasks, 4 host-proof pixels.
    const uint32_t softPixelProof=execution.soft_shadow_pixel_proof();
    out.execution.soft_shadow_state_chain=softPixelProof==1?resources.soft_shadow_state.production_chain:0;
    out.execution.soft_shadow_state_reason=!softShadow?0u:softPixelProof==1?1u:parallelSoftShadows?4u:shadowPersistent?3u:2u;
    out.execution.shadow_logical_tasks=execution.shadow().tasks;
    out.execution.soft_shadow_formula_reads=softShadow&&resources.soft_shadow_state.known?resources.soft_shadow_state.formula_reads_before_write:0;
    out.execution.soft_shadow_orbit_initialized=softShadow?resources.soft_shadow_state.initialized:0;
    out.execution.soft_shadow_scene_inputs=softShadow?resources.soft_shadow_state.live_inputs:0;
    out.execution.soft_shadow_caller_controls=softShadow?resources.soft_shadow_state.caller_owned_controls:0;
    out.execution.soft_shadow_driver_may_write=softShadow?resources.soft_shadow_state.driver_may_write:0;
    out.execution.primary_state_chain=resources.primary_state.production_chain;
    // 1: closed descriptor proof, 2: legacy qualification, 3: unknown/dependent,
    // 5: independent state contract. Earlier binaries recorded 4 for an
    // independent contract that production did not yet admit.
    out.execution.primary_state_reason=resources.primary_state.production_chain?1u:
        primaryPersistent?3u:execution.independent_primary_state()?5u:2u;
    out.execution.primary_formula_reads=resources.primary_state.known?resources.primary_state.formula_reads_before_write:0;
    out.execution.primary_orbit_initialized=resources.primary_state.initialized;
    out.execution.primary_scene_inputs=resources.primary_state.live_inputs;
    out.execution.primary_caller_controls=resources.primary_state.caller_owned_controls;
    out.execution.primary_driver_may_write=resources.primary_state.driver_may_write;
    out.execution.primary_contained=resources.primary_state.contained;
    out.execution.primary_entry_equivalent=resources.primary_state.entry_equivalent;
    out.execution.primary_logical_tasks=execution.primary().tasks;
    out.execution.smoothing_record_version=mb3d_native::kNativeSmoothingVersion;
    out.execution.smoothing_policy=opts.native_metal_smoothing_policy;
    out.execution.color_abi_version=interior.version;out.execution.color_flags=interior.flags;
    out.execution.surface_color_version=packing.version;out.execution.surface_color_option=packing.color_option;
    out.execution.soft_shadow_record_version=softShadow?mb3d_native::kNativeSoftShadowVersion:0u;
    out.execution.ao_record_version=mb3d_native_ao::Version;out.execution.ao_enabled=ambient;
    if(ambient){out.execution.ao_algorithm=aoAlgorithm;
        out.execution.ao_quality=aoAlgorithm==mb3d_native_ao::DistanceEstimate?distanceAO.quality:screenAO.params.quality;
        out.execution.ao_requested_repeats=aoAlgorithm==mb3d_native_ao::DistanceEstimate?1:screenAO.params.repeats;
        out.execution.ao_preparation_seconds=aoPreparationSeconds;}
    out.execution.arithmetic_bits=m.arithmeticProfile[2];
    const uint32_t expected[]={mb3d_native::kGeneralGeometryVersion,sizeof(ExactScene),sizeof(mb3d_native::NativeFormulaProgram),
        sizeof(ExactPixel),sizeof(ExactSurface),sizeof(NativeGeometryRecord),32};
    gpu::verifyMarker(m,@"native_general_abi",expected,sizeof(expected),"Native general shader ABI mismatch");
    const uint32_t expectedGradient[]={4,32};
    gpu::verifyMarker(m,@"native_general_gradient_abi",expectedGradient,sizeof(expectedGradient),"Native numerical gradient policy mismatch");
    out.execution.numerical_gradient_policy=4;
    const uint32_t expectedSmoothing[]={mb3d_native::kNativeSmoothingVersion,opts.native_metal_smoothing_policy,
        sizeof(mb3d_native::NativeSmoothingCounters),mb3d_native::kGeneralGeometryVersion};
    gpu::verifyMarker(m,@"native_general_smoothing_abi",expectedSmoothing,sizeof(expectedSmoothing),"Native smoothing policy/ABI mismatch");
    const uint32_t expectedColor[]={interior.version,sizeof(interior),sizeof(ExactColorScene),sizeof(ExactSurface),sizeof(ExactColorPixel)};
    gpu::verifyMarker(m,softShadow?@"native_general_soft_color_abi":@"native_general_color_abi",expectedColor,sizeof(expectedColor),"Native general color shader ABI mismatch");
    const uint32_t expectedLights[]={mb3d_native::kNativeLightGeometryVersion,sizeof(mb3d_native::NativeLightGeometry),
        sizeof(mb3d_native::NativePointLight)};
    gpu::verifyMarker(m,@"native_light_geometry_abi",expectedLights,sizeof(expectedLights),"Native light geometry ABI mismatch");
    const uint32_t expectedNormal[]={mb3d_native::kGeneralNormalVersion,sizeof(normalParams),
        sizeof(mb3d_native::NativeNormalRecord),sizeof(mb3d_native::NativeNormalResult),mb3d_native::kGeneralNormalMaxSamples};
    gpu::verifyMarker(m,@"native_general_normal_abi",expectedNormal,sizeof(expectedNormal),"Native general normal shader ABI mismatch");
    const uint32_t expectedCamera[]={mb3d_camera::Version,sizeof(mb3d_camera::Params),sizeof(mb3d_camera::Axis),
        sizeof(mb3d_camera::Ray),sizeof(mb3d_camera::Paint),sizeof(mb3d_camera::CutSurface),sizeof(mb3d_camera::DepthPacking)};
    gpu::verifyMarker(m,@"native_camera_abi",expectedCamera,sizeof(expectedCamera),"Native camera ABI mismatch");
    const uint32_t expectedInside[]={mb3d_inside::Version,sizeof(insidePolicy),sizeof(mb3d_inside::Record),
        sizeof(mb3d_camera::Params),sizeof(mb3d_camera::Ray),sizeof(normalParams)};
    gpu::verifyMarker(m,@"native_camera_inside_abi",expectedInside,sizeof(expectedInside),"Native camera/inside primary ABI mismatch");
    const uint32_t expectedVolume[]={mb3d_native::kNativeVolumeLightVersion,sizeof(volume),
        sizeof(mb3d_native::NativeVolumeCell),mb3d_native::kNativeVolumeMapSteps};
    gpu::verifyMarker(m,@"native_volume_light_abi",expectedVolume,sizeof(expectedVolume),"Native volumetric light ABI mismatch");
    gpu::require(!volumeLight||volume.version==mb3d_native::kNativeVolumeLightVersion,"Missing Native volumetric light descriptor");
    id<MTLComputePipelineState> volumePipeline=volumeCells?m.pipeline(@"native_volume_light_map"):nil;
    // These are alternative entry points. Preparing the unused independent
    // entry still asks the driver to optimize the entire specialized orbit.
    // Stateful chains dispatch only persistentPrimary below.
    // Only the primary entries see function constant 22 (the first-step jitter).
    m.nativePrimaryRandom=primaryRandom;
    id<MTLComputePipelineState> primary=primaryPersistent?nil:m.pipeline(@"native_camera_general_primary");
    m.nativePrimaryRandom=0;
    id<MTLComputePipelineState> primaryProbe=primaryRandom&&!primaryPersistent?m.pipeline(@"native_camera_general_primary_probe"):nil;
    auto surface=m.pipeline(@"native_camera_general_surface");
    if(primaryPersistent){
        const uint32_t expectedLane[]={1,sizeof(mb3d_native::NativeLogicalLane),sizeof(mb3d_native::NativeFormulaState),
            sizeof(mb3d_native::NativeLogicalLaneRecord),32};
        gpu::verifyMarker(m,@"native_general_lane_abi",expectedLane,sizeof(expectedLane),"Native general logical lane shader ABI mismatch");
    }
    m.nativePrimaryRandom=primaryRandom;
    id<MTLComputePipelineState> persistentPrimary=primaryPersistent?m.pipeline(@"native_camera_general_primary_lanes"):nil;
    m.nativePrimaryRandom=0;
    // Shadow and reflection kernels are specialized after the primary pass:
    // a frame that already exceeds the FP32 range policy there skips them.
    id<MTLComputePipelineState> shadowPipeline=nil,reflectionPipeline=nil,reflectionShadePipeline=nil;
    id<MTLComputePipelineState> shadowPixelsPipeline=nil,shadowScanPipeline=nil;
    auto prepareSecondaryPipelines=[&]{
    if(shadows){
        const uint32_t expectedShadow[]={mb3d_native::kGenericShadowVersion,sizeof(mb3d_native::NativeShadowRecord),32,1};
        gpu::verifyMarker(m,@"native_general_shadow_abi",expectedShadow,sizeof(expectedShadow),"Native general shadow shader ABI mismatch");
        if(shadowPersistent){
            const uint32_t expectedLane[]={1,sizeof(mb3d_native::NativeLogicalLane),sizeof(mb3d_native::NativeFormulaState),
                sizeof(mb3d_native::NativeShadowRowState),sizeof(mb3d_native::NativeLogicalLaneRecord),32};
            gpu::verifyMarker(m,@"native_general_shadow_lane_abi",expectedLane,sizeof(expectedLane),"Native general shadow logical lane ABI mismatch");
        }
        const uint32_t expected[]={softShadow?mb3d_native::kNativeSoftShadowVersion:1u,sizeof(mb3d_camera::Params),sizeof(insidePolicy),
            uint32_t(softShadow?sizeof(mb3d_native::NativeSoftShadowRecord):(multiShadow?sizeof(mb3d_native::NativeMultiShadowRecord):sizeof(mb3d_native::NativeShadowRecord))),
            uint32_t(softShadow?sizeof(mb3d_native::NativeSoftShadowConfig):(multiShadow?sizeof(mb3d_native::NativeMultiShadowRowState):sizeof(mb3d_native::NativeShadowRowState))),mb3d_native::kLogicalLaneBatchPixels};
        gpu::verifyMarker(m,softShadow?@"native_camera_soft_shadow_abi":(multiShadow?@"native_camera_multi_shadow_abi":@"native_camera_inside_shadow_abi"),expected,sizeof(expected),"Native camera/inside shadow ABI mismatch");
        if(parallelSoftShadows){
            const uint32_t expectedPixels[]={2,sizeof(mb3d_native::NativeSoftShadowRecord),sizeof(ExactSurface),mb3d_native::kGeneralGeometryVersion,32};
            gpu::verifyMarker(m,@"native_camera_soft_shadow_pixel_abi",expectedPixels,sizeof(expectedPixels),"Native soft shadow pixel entry ABI mismatch");
        }
        if(speculativeShadows){
            auto pixelMarker=m.pipeline(multiShadow?@"native_camera_multi_shadow_pixels_abi":@"native_camera_inside_shadow_pixels_abi");
            auto pixelIdentity=m.buffer(4*sizeof(uint32_t));
            std::memset(pixelIdentity.contents,0,pixelIdentity.length);
            gpu::dispatchStage(m,pixelMarker,1,[&](id<MTLComputeCommandEncoder> e){[e setBuffer:pixelIdentity offset:0 atIndex:0];});
            const uint32_t expectedPixels[]={1,
                uint32_t(multiShadow?sizeof(mb3d_native::NativeMultiShadowSpeculation):sizeof(mb3d_native::NativeShadowSpeculation)),
                uint32_t(multiShadow?sizeof(mb3d_native::NativeMultiShadowRowState):sizeof(mb3d_native::NativeShadowRowState)),
                uint32_t(multiShadow?sizeof(mb3d_native::NativeMultiShadowRecord):sizeof(mb3d_native::NativeShadowRecord))};
            gpu::require(std::memcmp(pixelIdentity.contents,expectedPixels,sizeof(expectedPixels))==0,"Native shadow speculation ABI mismatch");
        }
        // A shadow kernel inlines the formula program at the inside start,
        // both refinements and each light's march. Large hybrids then exhaust
        // the backend compiler: rank 6's multi-light kernel ran
        // MTLCompilerService out of address space on every attempt. One
        // outlined DE per pipeline, as for reflections.
        const uint32_t outlinePrevious=m.nativeOutlineDE;m.nativeOutlineDE=1;
        if(speculativeShadows){
            shadowPixelsPipeline=m.pipeline(multiShadow?@"native_camera_multi_hard_shadow_pixels":@"native_camera_inside_hard_shadow_pixels");
            shadowScanPipeline=m.pipeline(multiShadow?@"native_camera_multi_shadow_scan":@"native_camera_inside_shadow_scan");
        }
        shadowPipeline=m.pipeline(softShadow?(parallelSoftShadows?@"native_camera_soft_shadow_pixels":shadowPersistent?@"native_camera_soft_shadow_lanes":@"native_camera_soft_shadow"):(multiShadow?
            (shadowPersistent?@"native_camera_multi_hard_shadow_lanes":@"native_camera_multi_hard_shadow"):
            (shadowPersistent?@"native_camera_inside_hard_shadow_lanes":@"native_camera_inside_hard_shadow")));
        m.nativeOutlineDE=outlinePrevious;
    }
    if(reflections){
        const uint32_t expected[]={mb3d_native::kGenericReflectionVersion,sizeof(NativeReflectionParams),sizeof(mb3d_native::NativeGenericReflectionRecord),
            sizeof(mb3d_native::NativeLogicalLane),sizeof(mb3d_native::NativeFormulaState),sizeof(mb3d_native::NativeLogicalLaneRecord),32};
        gpu::verifyMarker(m,@"native_general_reflection_abi",expected,sizeof(expected),"Native general reflection ABI mismatch");
        const uint32_t insideExpected[]={6,sizeof(insidePolicy),sizeof(insideConfig),sizeof(mb3d_inside::ReflectionRecord),
            sizeof(mb3d_camera::Params),sizeof(interior),32,sizeof(mb3d_native::NativeReflectionMaterial),
            sizeof(mb3d_native::NativeReflectionShadeRecord),sizeof(mb3d_native::NativeReflectionShadeEvent)};
        gpu::verifyMarker(m,@"native_general_inside_reflection_abi",insideExpected,sizeof(insideExpected),"Native camera/inside reflection ABI mismatch");
        // A reflection kernel inlines the formula program at every trace,
        // normal, AO and shadow call site; large hybrids exceed the backend
        // compiler (XPC interruption after minutes). One outlined DE keeps
        // it compilable and left the measured images bit-identical.
        const uint32_t outlinePrevious=m.nativeOutlineDE;m.nativeOutlineDE=1;
        reflectionPipeline=m.pipeline(parallelReflections?@"native_camera_inside_reflection_pixels":@"native_camera_inside_reflection_lanes");
        m.nativeOutlineDE=outlinePrevious;
        reflectionShadePipeline=m.pipeline(@"native_camera_inside_reflection_shade");
        out.reflections.enabled=true;
    }
    };
    ImagePipelines image(m);gpu::RenderStageBuffers b;
    b.pixels=m.buffer(sizes[0]);auto records=m.buffer(sizes[1]);
    b.surface=m.buffer(sizes[2]);b.shaded=m.buffer(sizes[3]);b.rgb=m.buffer(sizes[4]);
    b.cols=m.buffer(sizes[5],prepared.base.columns.data());b.rows=m.buffer(sizes[6],prepared.base.rows.data());
    const std::array<mb3d_native::NativeFormulaProgram,2> programs{prepared.formulas.device,prepared.formulas.second};
    gpu::require(sizes[7]==prepared.formulas.program_count()*sizeof(mb3d_native::NativeFormulaProgram),"Native formula program buffer size");
    auto program=m.buffer(sizes[7],programs.data());
    auto maps=m.buffer(sizes[8],prepared.formulas.maps.empty()?nullptr:prepared.formulas.maps.data());
    auto mapPixels=m.buffer(sizes[9],prepared.formulas.map_pixels.empty()?nullptr:prepared.formulas.map_pixels.data());
    b.depth=m.buffer(sizes[10]);b.temporary=m.buffer(sizes[11]);b.angles=m.buffer(sizes[12]);
    b.atan=m.buffer(sizes[13]);
    b.colorScene=m.buffer(sizes[14],&color.device);b.colorCols=m.buffer(sizes[15],color.columns.data());
    b.colorRows=m.buffer(sizes[16],color.rows.data());
    auto shadowRecords=m.buffer(sizes[17]);
    auto shadowSpeculation=m.buffer(sizes[45]),shadowEntry=m.buffer(sizes[46]),shadowScanStatus=m.buffer(sizes[47]);
    auto primaryLanes=m.buffer(sizes[18]),primaryStates=m.buffer(sizes[19]),primaryLaneRecords=m.buffer(sizes[20]);
    auto primaryRandomWords=m.buffer(sizes[48]);
    auto shadowLanes=m.buffer(sizes[21]),shadowStates=m.buffer(sizes[22]),shadowRowsState=m.buffer(sizes[23]),shadowLaneRecords=m.buffer(sizes[24]);
    auto reflectionRecords=m.buffer(sizes[25]),reflectionLanes=m.buffer(sizes[26]),reflectionStates=m.buffer(sizes[27]),reflectionLaneRecords=m.buffer(sizes[28]);
    auto interiorBuffer=m.buffer(sizes[29],&interior);
    auto lightBuffer=m.buffer(sizeof(plan.color.lights),&plan.color.lights);
    const auto& mapSamples=plan.color.map_samples;
    gpu::require(plan.color.maps.sample_count==mapSamples.size(),"Native material map sample count");
    auto materialMaps=m.buffer(sizeof(plan.color.maps),&plan.color.maps);
    auto materialSamples=m.buffer(std::max<std::size_t>(sizeof(float),mapSamples.size()*sizeof(float)),
        mapSamples.empty()?nullptr:mapSamples.data());
    auto reflectionMaterial=m.buffer(sizeof(plan.color.reflection),&plan.color.reflection);
    auto normalParamsBuffer=m.buffer(sizes[30],&normalParams),normalRecords=m.buffer(sizes[31]),normalResults=m.buffer(sizes[32]);
    auto reflectionNormalRecords=m.buffer(sizes[33]),reflectionNormalResults=m.buffer(sizes[34]);
    CameraBuffers camera;
    camera.params=m.buffer(sizes[35],&prepared.camera.device);
    camera.columns=m.buffer(sizes[36],prepared.camera.columns.data());
    camera.rows=m.buffer(sizes[37],prepared.camera.rows.data());
    camera.paintColumns=m.buffer(sizes[38],prepared.camera.paint_columns.data());
    camera.paintRows=m.buffer(sizes[39],prepared.camera.paint_rows.data());camera.rays=m.buffer(sizes[40]);
    auto insidePolicyBuffer=m.buffer(sizes[41],&insidePolicy),insideRecords=m.buffer(sizes[42]);
    auto insideConfigBuffer=m.buffer(sizes[43],&insideConfig),insideReflectionRecords=m.buffer(sizes[44]);
    auto volumeBuffer=m.buffer(sizeof(volume),&volume);
    auto volumeMap=m.buffer(std::max<uint64_t>(sizeof(float),volumeCells*sizeof(float)));
    auto volumeRecords=m.buffer(std::max<uint64_t>(sizeof(mb3d_native::NativeVolumeCell),volumeCells*sizeof(mb3d_native::NativeVolumeCell)));
    const uint32_t resourceCounts[]={uint32_t(prepared.formulas.maps.size()),uint32_t(prepared.formulas.map_pixels.size())};
    auto addSmoothing=[&](const mb3d_native::NativeSmoothingCounters& s){
        out.execution.smoothing_calls+=s.calls;out.execution.smoothing_recovered+=s.recovered;
        out.execution.smoothing_stagnant+=s.stagnant;out.execution.smoothing_invalid+=s.invalid;};
    // Observe only bounds-checked, actually processed primary records before
    // lane failure can abort the stage. Do not lose failed smoothing evidence
    // or inspect uninitialized records beyond a partial lane's processed count.
    auto observePrimary=[&](uint32_t index){
        const auto& g=static_cast<const NativeGeometryRecord*>(records.contents)[index];
        gpu::require(mb3d_native::valid_smoothing_observation(g,opts.native_metal_smoothing_policy),
            "Native primary smoothing record ABI/work mismatch");
        addSmoothing(g.smoothing);out.execution.distance_calls+=g.de_calls;out.execution.formula_work+=g.formula_work;
        if(g.smoothing.invalid){++out.execution.faults;++out.execution.nonfinite;
            throw std::runtime_error("Native smoothing interpolation failed at pixel "+std::to_string(index)+
                " policy="+std::to_string(opts.native_metal_smoothing_policy)+
                " invalid="+std::to_string(g.smoothing.invalid));}
        if(g.formula_fault.code){++out.execution.faults;
            out.execution.nonfinite+=g.formula_fault.code==mb3d_native::NonFiniteFormula;
            throw std::runtime_error("Native primary formula failed at pixel "+std::to_string(index)+
                " code="+std::to_string(g.formula_fault.code)+
                " formula="+std::to_string(g.formula_fault.formula_id)+
                " slot="+std::to_string(g.formula_fault.slot)+
                " iteration="+std::to_string(g.formula_fault.iteration));}
    };
    timing.end();
    if(volumeCells){
        // CalcVolLightMap before the primary march: one independent ray per cell.
        timing.begin(RenderProfileStage::Volume);
        const uint32_t cells=uint32_t(volumeCells);
        for(uint32_t first=0;first<cells;first+=execution.primary().dispatch_batch){
            const uint32_t launch[]={first,std::min(execution.primary().dispatch_batch,cells-first),cells,0};
            gpu::dispatchStage(m,volumePipeline,launch[1],[&](id<MTLComputeCommandEncoder> e){
                [e setBuffer:volumeMap offset:0 atIndex:0];[e setBuffer:volumeRecords offset:0 atIndex:1];
                [e setBytes:&scene length:sizeof(scene) atIndex:2];[e setBuffer:program offset:0 atIndex:3];
                [e setBuffer:volumeBuffer offset:0 atIndex:4];
                [e setBuffer:maps offset:0 atIndex:6];[e setBuffer:mapPixels offset:0 atIndex:7];
                [e setBytes:launch length:sizeof(launch) atIndex:8];[e setBytes:resourceCounts length:sizeof(resourceCounts) atIndex:9];
            },1,execution.primary().group_width);
            ++out.execution.volume_map_dispatches;
        }
        const auto* cell=static_cast<const mb3d_native::NativeVolumeCell*>(volumeRecords.contents);
        uint64_t rangeCells=0;
        for(uint32_t i=0;i<cells;++i){
            if(cell[i].fault){++out.execution.faults;
                throw std::runtime_error("Native volumetric light map failed at cell "+std::to_string(i)+
                    " fault="+std::to_string(cell[i].fault));}
            ++out.execution.volume_map_cells;
            rangeCells+=cell[i].range_exits!=0;
        }
        out.execution.volume_map_range_exit_cells=rangeCells;
        if(const char* path=std::getenv("MB3D_VLM_DUMP")){
            // CalcVolLightMap's diagnostic layout: int32 size/sides, float
            // stretch/minDistance, double lightPos[3], float matrix[3][4], cells.
            std::ofstream dump(path,std::ios::binary);
            const int32_t header[]={int32_t(volume.cube_size),int32_t(volume.side_count)};
            const double light[]={volume.light[0],volume.light[1],volume.light[2]};
            dump.write(reinterpret_cast<const char*>(header),sizeof(header));
            dump.write(reinterpret_cast<const char*>(&volume.stretch),4);
            dump.write(reinterpret_cast<const char*>(&volume.min_distance),4);
            dump.write(reinterpret_cast<const char*>(light),sizeof(light));
            dump.write(reinterpret_cast<const char*>(volume.rotation),sizeof(volume.rotation));
            dump.write(static_cast<const char*>(volumeMap.contents),std::streamsize(volumeCells*sizeof(float)));
            gpu::require(bool(dump),"Cannot write volumetric map dump");
        }
        // The frame's FP32 policy applies to the map rays as to the pixels.
        out.execution.fp32_range_gate_stage="volume";
        out.execution.fp32_range_gate_units=volumeCells;out.execution.fp32_range_gate_exceeded=rangeCells;
        out.execution.fp32_range_gate_percent=fp32RangePixelPercent();
        if(rangeCells*100>volumeCells*fp32RangePixelPercent())
            throw Fp32RangeExceeded("Native FP32 range: "+std::to_string(rangeCells)+" of "+std::to_string(cells)+
                " volumetric map cells left the binary32 range");
        timing.end();
    }
    timing.begin(RenderProfileStage::Primary);
    // Independent pixels/surface packing need enough groups to fill the GPU.
    // Stateful logical lanes retain their separate bounded 32-pixel batches.
    // Each lane owns disjoint rows, state and records; the lane kernels use
    // only thread_position_in_grid and no threadgroup shared state/barriers.
    // Put these few, divergent tasks in separate groups so they can execute
    // on different GPU cores. Never split or reorder a lane's carried state.
    constexpr uint32_t batch=16384;
    if(primaryPersistent){
        std::vector<mb3d_native::NativeLogicalLane> lanes(logicalWorkers);
        uint32_t remaining=count;
        for(uint32_t worker=0;worker<logicalWorkers;++worker){auto& lane=lanes[worker];
            lane.row_start=worker;lane.row_stride=logicalWorkers;lane.row_count=(scene.height-1-worker)/logicalWorkers+1;
            lane.allocation_count=count;}
        const mb3d_native::NativePrimaryRandom carriedRandom{mb3d_native::kNativePrimaryRandomVersion,
            primaryRandom?mb3d_native::PrimaryRandomCarried:mb3d_native::PrimaryRandomOff,logicalWorkers,0};
        out.execution.primary_random_mode=carriedRandom.mode;
        while(remaining){
            for(auto& lane:lanes)lane.pixel_count=std::min(execution.primary().continuation_pixels,lane.row_count*scene.width-lane.cursor);
            std::memcpy(primaryLanes.contents,lanes.data(),sizes[18]);
            const uint32_t launch[]={0,logicalWorkers,logicalWorkers,count};
            gpu::dispatchStage(m,persistentPrimary,logicalWorkers,[&](id<MTLComputeCommandEncoder> e){
                [e setBuffer:b.pixels offset:0 atIndex:0];[e setBuffer:records offset:0 atIndex:1];
                [e setBytes:&scene length:sizeof(scene) atIndex:2];[e setBuffer:program offset:0 atIndex:3];
                [e setBuffer:b.cols offset:0 atIndex:4];[e setBuffer:b.rows offset:0 atIndex:5];
                [e setBuffer:maps offset:0 atIndex:6];[e setBuffer:mapPixels offset:0 atIndex:7];
                [e setBytes:launch length:sizeof(launch) atIndex:8];[e setBytes:resourceCounts length:sizeof(resourceCounts) atIndex:9];
                [e setBuffer:primaryLanes offset:0 atIndex:10];[e setBuffer:primaryStates offset:0 atIndex:11];
                [e setBuffer:primaryLaneRecords offset:0 atIndex:12];
                [e setBuffer:normalParamsBuffer offset:0 atIndex:20];[e setBuffer:normalRecords offset:0 atIndex:21];
                [e setBuffer:normalResults offset:0 atIndex:22];
                camera.bind(e);
                [e setBuffer:insidePolicyBuffer offset:0 atIndex:27];[e setBuffer:insideRecords offset:0 atIndex:28];
                [e setBuffer:volumeBuffer offset:0 atIndex:29];[e setBuffer:volumeMap offset:0 atIndex:30];
                [e setBytes:&carriedRandom length:sizeof(carriedRandom) atIndex:13];
                [e setBuffer:primaryRandomWords offset:0 atIndex:14];
            },1,execution.primary().group_width);
            ++out.execution.primary_dispatches;
            const auto* actual=static_cast<const mb3d_native::NativeLogicalLaneRecord*>(primaryLaneRecords.contents);
            for(uint32_t worker=0;worker<logicalWorkers;++worker){auto& lane=lanes[worker];const auto& record=actual[worker];
                gpu::require(record.version==1&&record.processed<=lane.pixel_count&&
                    record.next_cursor==lane.cursor+record.processed,"Native primary partial lane bounds");
                for(uint32_t k=0;k<record.processed;++k){const uint32_t ordinal=lane.cursor+k;
                    const uint32_t x=ordinal%scene.width,y=lane.row_start+(ordinal/scene.width)*lane.row_stride;
                    gpu::require(x<scene.width&&y<scene.height,"Native primary partial record bounds");observePrimary(y*scene.width+x);}
                gpu::require(record.version==1&&!record.fault&&record.processed==lane.pixel_count&&
                    record.next_cursor==lane.cursor+lane.pixel_count,"Native general primary logical lane failed");
                lane.cursor=record.next_cursor;remaining-=record.processed;}
        }
    }else{
        auto dispatchPixels=[&](uint32_t mode){
        const mb3d_native::NativePrimaryRandom random{mb3d_native::kNativePrimaryRandomVersion,mode,logicalWorkers,0};
        const bool probe=mode==mb3d_native::PrimaryRandomProbe;
        for(uint32_t first=0;first<count;first+=execution.primary().dispatch_batch){
        const uint32_t launch[]={first,std::min(execution.primary().dispatch_batch,count-first),count,0};
        gpu::dispatchStage(m,probe?primaryProbe:primary,launch[1],[&](id<MTLComputeCommandEncoder> e){
            if(!probe){[e setBuffer:b.pixels offset:0 atIndex:0];[e setBuffer:records offset:0 atIndex:1];}
            [e setBytes:&scene length:sizeof(scene) atIndex:2];[e setBuffer:program offset:0 atIndex:3];
            [e setBuffer:b.cols offset:0 atIndex:4];[e setBuffer:b.rows offset:0 atIndex:5];
            [e setBuffer:maps offset:0 atIndex:6];[e setBuffer:mapPixels offset:0 atIndex:7];
            [e setBytes:launch length:sizeof(launch) atIndex:8];[e setBytes:resourceCounts length:sizeof(resourceCounts) atIndex:9];
            [e setBytes:&random length:sizeof(random) atIndex:13];[e setBuffer:primaryRandomWords offset:0 atIndex:14];
            [e setBuffer:normalParamsBuffer offset:0 atIndex:20];[e setBuffer:normalRecords offset:0 atIndex:21];
                [e setBuffer:normalResults offset:0 atIndex:22];
                camera.bind(e);
                [e setBuffer:insidePolicyBuffer offset:0 atIndex:27];[e setBuffer:insideRecords offset:0 atIndex:28];
                [e setBuffer:volumeBuffer offset:0 atIndex:29];[e setBuffer:volumeMap offset:0 atIndex:30];
        },1,execution.primary().group_width);
        if(probe){++out.execution.primary_random_probe_dispatches;continue;}
        ++out.execution.primary_dispatches;
        for(uint32_t i=first;i<first+launch[1];++i)observePrimary(i);
        }};
        if(primaryRandom){
            // CPU RayMarch carries one seed through each task's pixels. The
            // probe finds which pixels march; a bounded GPU prefix scan derives
            // entering seeds in task order and then each task's exit.
            dispatchPixels(mb3d_native::PrimaryRandomProbe);
            const auto seedPlan=PlanSeedScan(scene.width,scene.height,logicalWorkers,
                mb3d_seed_scan::Interleaved,(scene.height+logicalWorkers-1)/logicalWorkers,
                mb3d_seed_scan::PrimaryFlags);
            const auto seeded=RunSeedScan(m,seedPlan,primaryRandomWords,seedPlan.scratch_bytes,[&]{invocation.poll();});
            out.execution.primary_random_advances=seeded.draws;
            out.execution.primary_seed_scan_commands=seeded.commands;
            out.execution.primary_random_mode=mb3d_native::PrimaryRandomSeeded;
        }
        dispatchPixels(primaryRandom?mb3d_native::PrimaryRandomSeeded:mb3d_native::PrimaryRandomOff);
    }
    for(uint32_t first=0;first<count;first+=batch){
        const uint32_t launch[]={first,std::min(batch,count-first),count,0};
        gpu::dispatchStage(m,surface,launch[1],[&](id<MTLComputeCommandEncoder> e){
            [e setBuffer:b.pixels offset:0 atIndex:0];[e setBuffer:b.surface offset:0 atIndex:1];
            [e setBytes:&scene length:sizeof(scene) atIndex:2];[e setBytes:&packing length:sizeof(packing) atIndex:3];
            [e setBuffer:program offset:0 atIndex:4];[e setBytes:launch length:sizeof(launch) atIndex:5];
            [e setBuffer:records offset:0 atIndex:6];
            [e setBuffer:normalParamsBuffer offset:0 atIndex:20];[e setBuffer:normalRecords offset:0 atIndex:21];
                [e setBuffer:normalResults offset:0 atIndex:22];
                camera.bind(e);
                [e setBuffer:insidePolicyBuffer offset:0 atIndex:27];[e setBuffer:insideRecords offset:0 atIndex:28];
        },1,32);
    }
    timing.end();
    // Later stages only add range exits, so a frame whose primary geometry
    // already exceeds the policy cannot pass it; large secondary kernels take
    // minutes to specialize cold.
    uint64_t primaryRangePixels=0;
    for(uint32_t i=0;i<count;++i)primaryRangePixels+=static_cast<const NativeGeometryRecord*>(records.contents)[i].fp32_range_exits!=0;
    out.execution.fp32_range_gate_stage="primary";
    out.execution.fp32_range_gate_units=count;out.execution.fp32_range_gate_exceeded=primaryRangePixels;
    out.execution.fp32_range_gate_percent=fp32RangePixelPercent();
    if(primaryRangePixels*100>uint64_t(count)*fp32RangePixelPercent()){
        // Later effects will be skipped, so validate all available primary
        // outputs here. A bad normal/camera/packed surface must still fail.
        for(uint32_t i=0;i<count;++i){
            if((i&255u)==0)invocation.poll();
            const char* error=mb3d_native::primary_range_error(
                static_cast<const NativeGeometryRecord*>(records.contents)[i],
                static_cast<const ExactPixel*>(b.pixels.contents)[i],
                static_cast<const ExactSurface*>(b.surface.contents)[i],
                static_cast<const mb3d_camera::Ray*>(camera.rays.contents)[i],
                prepared.camera.device.cut_options,std::bit_cast<double>(scene.max_ray_length),normalParams,
                static_cast<const mb3d_native::NativeNormalRecord*>(normalRecords.contents)[i],
                static_cast<const mb3d_native::NativeNormalResult*>(normalResults.contents)[i],insidePolicy,
                static_cast<const mb3d_inside::Record*>(insideRecords.contents)[i],
                opts.native_metal_smoothing_policy,packing.color_option);
            if(error){++out.execution.faults;
                throw std::runtime_error("Native range decision rejected primary record at pixel "+
                    std::to_string(i)+" ("+error+")");}
        }
        throw Fp32RangeExceeded("Native FP32 range: "+std::to_string(primaryRangePixels)+" of "+
            std::to_string(count)+" primary pixels left the binary32 range");
    }
    // Likewise a conditional frame whose primary geometry already counts a
    // carried dIFS read is discarded; the CPU-order frame replaces it, so its
    // secondary stages are skipped. The counts cover the primary stage only.
    if(execution.conditional_state()){
        uint64_t carriedReads=0,carriedPixels=0;
        for(uint32_t i=0;i<count;++i){
            const uint32_t reads=static_cast<const NativeGeometryRecord*>(records.contents)[i].difs_carried_reads;
            carriedReads+=reads;carriedPixels+=reads!=0;
        }
        if(carriedReads)throw CarriedStateObserved(carriedReads,carriedPixels);
    }
    timing.begin(RenderProfileStage::Preparation);prepareSecondaryPipelines();timing.end();
    // Reconstruct normals from the packed primary depth before shadows/AO.
    // Retain the returned buffer; its values are consumed by every later stage.
    ZNormalsOutput znormalOutput;
    if(znormals.version){
        timing.begin(RenderProfileStage::Normals);
        znormalOutput=RunNativeZNormals(m,znormals,b.surface,*resources.znormal_execution,[&]{invocation.poll();});
        b.surface=znormalOutput.surface;
        const auto& a=znormalOutput.activity;
        gpu::require(a.records==count&&!a.faults,"Native Z normals incomplete records");
        out.execution.znormal_records=a.records;out.execution.znormal_foreground=a.foreground;
        out.execution.znormal_changed=a.changed;out.execution.znormal_neighbors=a.neighbors;
        out.execution.znormal_dispatches=a.dispatches;
        timing.end();
    }
    if(shadows){
        timing.begin(RenderProfileStage::Shadows);
        if(shadowPersistent){
            std::vector<mb3d_native::NativeLogicalLane> lanes(shadowTasks);
            uint32_t remaining=count;
            for(uint32_t task=0;task<shadowTasks;++task){auto& lane=lanes[task];
                lane.row_start=task*shadowRows;lane.row_stride=1;lane.row_count=std::min(shadowRows,scene.height-lane.row_start);
                lane.allocation_count=count;}
            while(remaining){
                for(auto& lane:lanes)lane.pixel_count=std::min(execution.shadow().continuation_pixels,lane.row_count*scene.width-lane.cursor);
                std::memcpy(shadowLanes.contents,lanes.data(),sizes[21]);
                const uint32_t launch[]={0,shadowTasks,shadowTasks,count};
                gpu::dispatchStage(m,shadowPipeline,shadowTasks,[&](id<MTLComputeCommandEncoder> e){
                    [e setBuffer:b.pixels offset:0 atIndex:0];[e setBuffer:b.surface offset:0 atIndex:1];
                    [e setBuffer:b.shaded offset:0 atIndex:2];[e setBuffer:shadowRecords offset:0 atIndex:3];
                    [e setBytes:&scene length:sizeof(scene) atIndex:4];[e setBuffer:program offset:0 atIndex:5];
                    [e setBytes:&shadow length:sizeof(shadow) atIndex:6];[e setBuffer:maps offset:0 atIndex:7];
                    [e setBuffer:mapPixels offset:0 atIndex:8];[e setBytes:launch length:sizeof(launch) atIndex:9];
                    [e setBytes:resourceCounts length:sizeof(resourceCounts) atIndex:10];
                [e setBuffer:camera.params offset:0 atIndex:23];[e setBuffer:insidePolicyBuffer offset:0 atIndex:27];
                    [e setBuffer:shadowLanes offset:0 atIndex:11];[e setBuffer:shadowStates offset:0 atIndex:12];
                    [e setBuffer:shadowRowsState offset:0 atIndex:13];[e setBuffer:shadowLaneRecords offset:0 atIndex:14];
                    [e setBytes:&shadowRows length:sizeof(shadowRows) atIndex:15];
                    if(softShadow)[e setBytes:&softConfig length:sizeof(softConfig) atIndex:28];
                },1,execution.shadow().group_width);
                ++out.execution.shadow_dispatches;
                const auto* actual=static_cast<const mb3d_native::NativeLogicalLaneRecord*>(shadowLaneRecords.contents);
                for(uint32_t task=0;task<shadowTasks;++task){auto& lane=lanes[task];const auto& record=actual[task];
                    gpu::require(record.version==1&&!record.fault&&record.processed==lane.pixel_count&&
                        record.next_cursor==lane.cursor+lane.pixel_count,"Native general shadow logical lane failed");
                    lane.cursor=record.next_cursor;remaining-=record.processed;}
            }
        }else{
        // The separately audited soft-shadow entry owns one whole pixel per
        // thread. Its core has no row shortcut; all probes inside the pixel
        // remain ordered. The other entries below continue to own whole rows.
        // Stateless formula programs retain only the row's ordered shortcut.
        // Rows are independent; only the shortcut within each row is ordered.
        // The audited depth chains have no cross-row formula state. Fill
        // more SIMD lanes with independent rows, preserving each row's ordered
        // shadow shortcut and every per-pixel DE evaluation. Keep command work
        // bounded; other chains retain their qualified scheduling.
        // The nonpersistent multi-light kernel owns one whole row per grid
        // thread: input is read-only, output/records use disjoint row indices,
        // and lane + six-light shortcut caches are thread-local. It has no
        // group memory, barriers or SIMD collectives. Regroup physical threads
        // only; preserve all pixel/light order and cache arithmetic in the row.
        const uint32_t rowBatch=execution.shadow().dispatch_batch;
        const uint32_t rowGroup=execution.shadow().group_width;
        const uint32_t tasks=execution.shadow().tasks;
        auto orderedRows=[&]{
        for(uint32_t row=0;row<tasks;row+=rowBatch){
            const uint32_t launch[]={row,std::min(rowBatch,tasks-row),count,0};
            gpu::dispatchStage(m,shadowPipeline,launch[1],[&](id<MTLComputeCommandEncoder> e){
                [e setBuffer:b.pixels offset:0 atIndex:0];[e setBuffer:b.surface offset:0 atIndex:1];
                [e setBuffer:b.shaded offset:0 atIndex:2];[e setBuffer:shadowRecords offset:0 atIndex:3];
                [e setBytes:&scene length:sizeof(scene) atIndex:4];[e setBuffer:program offset:0 atIndex:5];
                [e setBytes:&shadow length:sizeof(shadow) atIndex:6];[e setBuffer:maps offset:0 atIndex:7];
                [e setBuffer:mapPixels offset:0 atIndex:8];[e setBytes:launch length:sizeof(launch) atIndex:9];
                [e setBytes:resourceCounts length:sizeof(resourceCounts) atIndex:10];
                [e setBuffer:camera.params offset:0 atIndex:23];[e setBuffer:insidePolicyBuffer offset:0 atIndex:27];
                if(softShadow)[e setBytes:&softConfig length:sizeof(softConfig) atIndex:28];
                if(parallelSoftShadows)[e setBytes:&softPixelProof length:sizeof(softPixelProof) atIndex:29];
            },1,rowGroup);
            ++out.execution.shadow_dispatches;
        }
        };
        if(!speculativeShadows)orderedRows();
        else{
            // Rebuild each row's ordered shortcut cache instead of tracing the
            // row in order: trace every pixel from a fresh cache, replay the
            // cache per row, then trace the pixels whose ordered entry state can
            // skip again from that state. A fault in either pass, which the
            // ordered row could have avoided by skipping, traces the rows in order.
            auto pixelPass=[&](uint32_t mode){
                for(uint32_t first=0;first<count;first+=execution.primary().dispatch_batch){
                    const uint32_t launch[]={first,std::min(execution.primary().dispatch_batch,count-first),count,mode};
                    gpu::dispatchStage(m,shadowPixelsPipeline,launch[1],[&](id<MTLComputeCommandEncoder> e){
                        [e setBuffer:b.pixels offset:0 atIndex:0];[e setBuffer:b.surface offset:0 atIndex:1];
                        [e setBuffer:b.shaded offset:0 atIndex:2];[e setBuffer:shadowRecords offset:0 atIndex:3];
                        [e setBytes:&scene length:sizeof(scene) atIndex:4];[e setBuffer:program offset:0 atIndex:5];
                        [e setBytes:&shadow length:sizeof(shadow) atIndex:6];[e setBuffer:maps offset:0 atIndex:7];
                        [e setBuffer:mapPixels offset:0 atIndex:8];[e setBytes:launch length:sizeof(launch) atIndex:9];
                        [e setBytes:resourceCounts length:sizeof(resourceCounts) atIndex:10];
                        [e setBuffer:shadowSpeculation offset:0 atIndex:11];[e setBuffer:shadowEntry offset:0 atIndex:12];
                        [e setBuffer:camera.params offset:0 atIndex:23];[e setBuffer:insidePolicyBuffer offset:0 atIndex:27];
                    },1,32);
                    ++out.execution.shadow_dispatches;
                }
            };
            pixelPass(0);
            std::memset(shadowScanStatus.contents,0xff,shadowScanStatus.length);
            const uint32_t scanLaunch[]={0,scene.height,count,0};
            gpu::dispatchStage(m,shadowScanPipeline,scene.height,[&](id<MTLComputeCommandEncoder> e){
                [e setBuffer:shadowSpeculation offset:0 atIndex:0];[e setBuffer:shadowEntry offset:0 atIndex:1];
                [e setBuffer:shadowScanStatus offset:0 atIndex:2];[e setBytes:&scene length:sizeof(scene) atIndex:4];
                [e setBytes:&shadow length:sizeof(shadow) atIndex:6];[e setBytes:scanLaunch length:sizeof(scanLaunch) atIndex:9];
            },1,32);
            ++out.execution.shadow_dispatches;
            const auto* status=static_cast<const uint32_t*>(shadowScanStatus.contents);
            bool faulted=false;uint64_t marked=0;
            for(uint32_t y=0;y<scene.height;++y){faulted|=status[2*y]!=0;marked+=status[2*y+1];}
            out.execution.shadow_speculative_pixels=count;
            if(faulted){++out.execution.shadow_speculation_fallbacks;orderedRows();}
            else{out.execution.shadow_recomputed_pixels=marked;if(marked)pixelPass(1);}
        }
        }
        timing.end();
    }else{
    timing.begin(RenderProfileStage::Primary);
    invocation.poll();
    auto copyCommand=[m.queue commandBuffer];auto copy=[copyCommand blitCommandEncoder];
    gpu::require(copyCommand&&copy,"Native general surface copy unavailable");
    [copy copyFromBuffer:b.surface sourceOffset:0 toBuffer:b.shaded destinationOffset:0 size:b.surface.length];
    [copy endEncoding];[copyCommand commit];[copyCommand waitUntilCompleted];
    gpu::require(copyCommand.status==MTLCommandBufferStatusCompleted,"Native general surface copy failed");
    invocation.poll();
    timing.end();
    }
    if(ambient){
        timing.begin(RenderProfileStage::AO);
        // no_ao controls this pass only. Reflection point-AO retains the
        // original header policy and its separate secondary-ray counters.
        const auto ao=aoAlgorithm==mb3d_native_ao::DistanceEstimate?
            RunNativeDistanceAO(m,distanceAO,scene,prepared.formulas.device,prepared.formulas.program_count()>1?&prepared.formulas.second:nullptr,camera.rays,b.shaded,
                maps,mapPixels,resourceCounts[0],resourceCounts[1],aoAllocationBudget,*resources.distance_execution,[&]{invocation.poll();}):
            RunNativeScreenAO(m,screenAO,b.shaded,aoAllocationBudget,*resources.screen_execution,[&]{invocation.poll();});
        gpu::require(ao.surface&&ao.totals.records==count,"Native AO incomplete returned surface/records");
        // Runtime validation has checked all records and every untouched
        // surface field before the replacement is visible to later stages.
        b.shaded=ao.surface;
        auto& e=out.execution;const auto& a=ao.totals;
        e.ao_enabled=true;e.ao_algorithm=aoAlgorithm;
        e.ao_quality=aoAlgorithm==mb3d_native_ao::DistanceEstimate?distanceAO.quality:screenAO.params.quality;
        e.ao_requested_repeats=aoAlgorithm==mb3d_native_ao::DistanceEstimate?1:screenAO.params.repeats;
        e.ao_records=a.records;e.ao_active=a.active;e.ao_changed=a.changed;e.ao_samples=a.samples;
        e.ao_depth_reads=a.depth_reads;e.ao_border_rejects=a.border_rejects;e.ao_rng_advances=a.rng_advances;
        e.ao_levels=a.levels;e.ao_repeats=a.repeats;e.ao_de_queries=a.de_queries;e.ao_de_steps=a.de_steps;
        e.ao_cut_skips=a.cut_skips;e.ao_rays=a.rays;e.ao_refinements=a.refinements;e.ao_formula_work=a.formula_work;
        e.ao_seed_generation_commands=ao.seed_generation_commands;
        e.ao_seed_shortcuts=ao.seed_shortcuts;e.ao_seed_planning_draws=ao.seed_planning_draws;e.ao_dispatches=ao.dispatches;e.ao_gpu_seconds=ao.gpu_ms/1000;e.ao_max_command_seconds=ao.max_command_ms/1000;
        e.ao_command_counter_version=1;e.ao_commands=ao.commands;
        e.ao_certificate_version=ao.certificate_version;e.ao_certificate_levels=ao.certificate_levels;
        e.ao_certificate_pixels=ao.certificate_pixels;e.ao_certificate_known=ao.certificate_known;e.ao_certificate_used=ao.certificate_used;
        // Descriptor preparation is measured separately within Preparation;
        // AO stage wall includes pipeline setup, dispatch and strict readback.
        e.ao_preparation_seconds=aoPreparationSeconds;
        e.smoothing_calls+=a.smoothing_calls;e.smoothing_recovered+=a.smoothing_recovered;
        e.smoothing_stagnant+=a.smoothing_stagnant;e.smoothing_invalid+=a.smoothing_invalid;
        e.distance_calls+=a.de_queries;e.formula_work+=a.formula_work;
        e.fp32_range_exits+=a.fp32_range_exits;e.ao_fp32_range_exit_pixels=a.fp32_range_exit_pixels;
        e.ao_difs_carried_reads=a.difs_carried_reads;e.ao_difs_carried_pixels=a.difs_carried_pixels;
        e.ao_difs_conditional_pixels=ao.difs_conditional_pixels;
        e.ao_difs_rerun_carried_reads=ao.difs_rerun_carried_reads;e.ao_difs_rerun_carried_pixels=ao.difs_rerun_carried_pixels;
        timing.end();
    }
    colorStage(m,image,scene,b,interiorBuffer,lightBuffer,materialMaps,materialSamples,camera,timing);
    if(reflections){
        timing.begin(RenderProfileStage::Reflections);
        // Each trace dispatch records its radiance events into these slots; the
        // formula-free shade pass consumes them before the next trace dispatch.
        // A transmission tree records up to 2^(bounces+1)-1 events per pixel,
        // so fewer pixels share one dispatch's bounded slot memory.
        const uint32_t shadeCapacity=mb3d_native::native_reflection_shade_capacity(reflection.max_bounces,insideConfig.transmission!=0);
        const std::size_t shadeBytes=mb3d_native::native_reflection_shade_bytes(shadeCapacity);
        constexpr std::size_t kShadeBudget=std::size_t(256)<<20;
        const uint32_t shadeFit=uint32_t(std::max<std::size_t>(1,kShadeBudget/shadeBytes));
        const uint32_t reflectionBatch=std::min(execution.reflection().dispatch_batch,shadeFit);
        const uint32_t shadeStride=parallelReflections?1u:std::min(execution.reflection().continuation_pixels,
            uint32_t(std::max<std::size_t>(1,shadeFit/std::max<uint32_t>(1,shadowTasks))));
        const uint32_t shadeSlots=parallelReflections?reflectionBatch:shadowTasks*shadeStride;
        gpu::require(shadeStride&&shadeSlots,"Native reflection shade slots");
        const uint32_t shadeLayout[]={shadeStride,shadeCapacity};
        auto shadeRecords=m.buffer(std::size_t(shadeSlots)*shadeBytes);
        std::memset(shadeRecords.contents,0,shadeRecords.length);
        auto dispatchShade=[&](uint32_t slots){
            const uint32_t launch[]={0,slots,count,0};
            const double ms=gpu::dispatchStage(m,reflectionShadePipeline,slots,[&](id<MTLComputeCommandEncoder> e){
                [e setBuffer:b.colorScene offset:0 atIndex:3];[e setBuffer:b.rgb offset:0 atIndex:7];
                [e setBuffer:reflectionRecords offset:0 atIndex:8];[e setBytes:launch length:4*sizeof(uint32_t) atIndex:9];
                [e setBuffer:interiorBuffer offset:0 atIndex:18];[e setBuffer:reflectionMaterial offset:0 atIndex:19];
                [e setBuffer:materialSamples offset:0 atIndex:24];[e setBuffer:shadeRecords offset:0 atIndex:25];
                [e setBytes:shadeLayout length:sizeof(shadeLayout) atIndex:26];
                [e setBuffer:insideReflectionRecords offset:0 atIndex:30];
            });
            out.reflections.max_command_seconds=std::max(out.reflections.max_command_seconds,ms/1000);
        };
        // CalcHSsoftPoint at reflected hits reuses the primary soft descriptor;
        // version 0 selects CalcHShardPoint.
        const mb3d_native::NativeSoftShadowConfig reflectionSoft=softShadow?softConfig:mb3d_native::NativeSoftShadowConfig{};
        auto dispatchReflection=[&](const uint32_t* launch){
            const double ms=gpu::dispatchStage(m,reflectionPipeline,launch[1],[&](id<MTLComputeCommandEncoder> e){
                [e setBytes:&scene length:sizeof(scene) atIndex:0];[e setBytes:&packing length:sizeof(packing) atIndex:1];
                [e setBytes:&shadow length:sizeof(shadow) atIndex:2];[e setBuffer:b.colorScene offset:0 atIndex:3];
                [e setBytes:&reflection length:sizeof(reflection) atIndex:4];[e setBuffer:b.pixels offset:0 atIndex:5];
                [e setBuffer:b.shaded offset:0 atIndex:6];[e setBuffer:b.rgb offset:0 atIndex:7];
                [e setBuffer:reflectionRecords offset:0 atIndex:8];[e setBytes:launch length:4*sizeof(uint32_t) atIndex:9];
                [e setBuffer:program offset:0 atIndex:10];[e setBuffer:maps offset:0 atIndex:11];[e setBuffer:mapPixels offset:0 atIndex:12];
                [e setBytes:resourceCounts length:sizeof(resourceCounts) atIndex:13];[e setBuffer:reflectionLanes offset:0 atIndex:14];
                [e setBuffer:reflectionStates offset:0 atIndex:15];[e setBuffer:reflectionLaneRecords offset:0 atIndex:16];
                [e setBytes:&shadowRows length:sizeof(shadowRows) atIndex:17];
                [e setBuffer:normalParamsBuffer offset:0 atIndex:20];[e setBuffer:reflectionNormalRecords offset:0 atIndex:21];
                [e setBuffer:reflectionNormalResults offset:0 atIndex:22];
                [e setBuffer:interiorBuffer offset:0 atIndex:18];[e setBuffer:reflectionMaterial offset:0 atIndex:19];
                [e setBuffer:camera.params offset:0 atIndex:23];[e setBuffer:materialSamples offset:0 atIndex:24];
                [e setBuffer:insidePolicyBuffer offset:0 atIndex:27];[e setBuffer:insideConfigBuffer offset:0 atIndex:29];
                [e setBytes:&reflectionSoft length:sizeof(reflectionSoft) atIndex:28];
                [e setBuffer:shadeRecords offset:0 atIndex:25];[e setBytes:shadeLayout length:sizeof(shadeLayout) atIndex:26];
                [e setBuffer:insideReflectionRecords offset:0 atIndex:30];
            },1,execution.reflection().group_width);
            out.reflections.max_command_seconds=std::max(out.reflections.max_command_seconds,ms/1000);
            dispatchShade(parallelReflections?launch[1]:shadeSlots);
        };
        if(parallelReflections) {
            for(uint32_t first=0;first<count;first+=reflectionBatch) {
                const uint32_t launch[]={first,std::min(reflectionBatch,count-first),count,0};
                dispatchReflection(launch);
            }
        } else {
        std::vector<mb3d_native::NativeLogicalLane> lanes(shadowTasks);uint32_t remaining=count;
        for(uint32_t task=0;task<shadowTasks;++task){auto& lane=lanes[task];lane.row_start=task*shadowRows;
            lane.row_stride=1;lane.row_count=std::min(shadowRows,scene.height-lane.row_start);lane.allocation_count=count;}
        while(remaining){
            for(auto& lane:lanes)lane.pixel_count=std::min(shadeStride,lane.row_count*scene.width-lane.cursor);
            std::memcpy(reflectionLanes.contents,lanes.data(),sizes[26]);
            const uint32_t launch[]={0,shadowTasks,shadowTasks,count};
            dispatchReflection(launch);
            const auto* actual=static_cast<const mb3d_native::NativeLogicalLaneRecord*>(reflectionLaneRecords.contents);
            for(uint32_t task=0;task<shadowTasks;++task){auto& lane=lanes[task];const auto& record=actual[task];
                if(!(record.version==1&&!record.fault&&record.processed==lane.pixel_count&&
                     record.next_cursor==lane.cursor+lane.pixel_count)){
                    std::string detail="Native general reflection logical lane failed: lane fault="+std::to_string(record.fault)+
                        " processed="+std::to_string(record.processed)+"/"+std::to_string(lane.pixel_count);
                    if(record.processed&&record.processed<=lane.pixel_count){
                        const uint32_t pixel=lane.row_start*scene.width+lane.cursor+record.processed-1;
                        const auto& r=static_cast<const mb3d_native::NativeGenericReflectionRecord*>(reflectionRecords.contents)[pixel];
                        detail+=" pixel="+std::to_string(pixel)+" activity="+std::to_string(r.activity.fault)+
                            " eval="+std::to_string(r.eval.fault)+" formula="+std::to_string(r.eval.formula_fault.code)+
                            " id="+std::to_string(r.eval.formula_fault.formula_id)+" rays="+std::to_string(r.activity.rays)+
                            " steps="+std::to_string(r.activity.ray_steps)+" de_calls="+std::to_string(r.eval.de_calls);
                    }
                    throw std::runtime_error(detail);
                }
                lane.cursor=record.next_cursor;remaining-=record.processed;}
        }
        }
        timing.end();
    }
    const auto* cameraRays=static_cast<const mb3d_camera::Ray*>(camera.rays.contents);
    const auto* geometry=static_cast<const NativeGeometryRecord*>(records.contents);
    const auto* pixels=static_cast<const ExactPixel*>(b.pixels.contents);
    const auto* packed=static_cast<const ExactSurface*>(b.surface.contents);
    const auto* shaded=static_cast<const ExactSurface*>(b.shaded.contents);
    const auto* colors=static_cast<const ExactColorPixel*>(b.rgb.contents);
    const uint32_t samplesPerNormal=mb3d_native::GeneralNormalSamples(normalParams.normals_on_de,normalParams.smoothing);
    auto validNormal=[&](const mb3d_native::NativeNormalRecord& n,const mb3d_native::NativeNormalResult& v,
                        uint32_t evaluations,const NativeGeometryRecord& geometry){
        if(!mb3d_native::valid_primary_normal(normalParams,n,v,evaluations,geometry))return false;
        out.execution.normal_samples+=n.sample_calls;out.execution.normal_de_calls+=n.de_calls;
        out.execution.normal_orbit_calls+=n.direct_orbit_calls;out.execution.normal_evaluations+=n.completed_evaluations;
        return true;
    };
    out.execution.camera_full_width=prepared.camera.device.full_width;out.execution.camera_full_height=prepared.camera.device.full_height;
    out.execution.camera_left=prepared.camera.device.left;out.execution.camera_top=prepared.camera.device.top;
    out.execution.inside_initial_policy=insidePolicy.initial_inside;out.execution.inside_in_and_out=insidePolicy.in_and_out;
    std::string firstFault;std::vector<uint32_t> faultPixels;
    for(uint32_t i=0;i<count;++i){
        if((i&255u)==0)invocation.poll();
        const auto& g=geometry[i];const auto& p=pixels[i];const auto& ray=cameraRays[i];
        bool cameraFault=ray.version!=mb3d_camera::Version||ray.fault||ray.cut_axis>3||ray.cut_miss>1||
            ray.direction[3]!=0||ray.origin[3]!=0||ray.reserved[0]||ray.reserved[1]||ray.reserved[2]||
            !std::isfinite(ray.cut_depth)||ray.cut_depth<0;
        for(unsigned axis=0;axis<3;++axis)cameraFault|=!std::isfinite(ray.direction[axis])||!std::isfinite(ray.origin[axis]);
        cameraFault|=ray.cut_miss!=uint32_t(ray.cut_depth>std::bit_cast<double>(scene.max_ray_length));
        if(ray.cut_axis)cameraFault|=!(prepared.camera.device.cut_options&(1u<<(ray.cut_axis-1)));
        out.execution.camera_records++;
        out.execution.camera_cut_starts+=ray.cut_axis!=0;out.execution.camera_cut_misses+=ray.cut_miss;
        out.execution.camera_planar_rays+=prepared.camera.device.optic==1;
        out.execution.camera_spherical_rays+=prepared.camera.device.optic==2;
        bool nonfinite=false;
        for(unsigned axis=0;axis<3;++axis)for(auto v:{p.ray[axis],p.start[axis],p.position[axis],p.raw_normal[axis]})
            nonfinite|=!std::isfinite(std::bit_cast<double>(v));
        for(auto v:{p.depth,p.axial_depth,p.final_de,p.orbit_trap})nonfinite|=!std::isfinite(std::bit_cast<double>(v));
        for(auto v:{p.step_count,p.smooth_iterations,p.final_stop})nonfinite|=!std::isfinite(std::bit_cast<float>(v));
        auto invalidGeometry=[&](const NativeGeometryRecord& value){
            return !mb3d_native::valid_smoothing_counters(value.smoothing,value.formula_work,opts.native_metal_smoothing_policy)||value.version!=mb3d_native::kGeneralGeometryVersion||value.fault||value.formula_fault.code||
                value.formula_fault.reserved[0]||value.formula_fault.reserved[1]||
                !mb3d_native::valid_difs_carried_reads(value.difs_carried_reads,value.de_calls,value.normal_calls)||value.gradient_range_clamps>value.de_calls||
                !mb3d_native::valid_fp32_range_exits(value.fp32_range_exits,value.de_calls,value.normal_calls)||!std::isfinite(value.initial_rout)||
                value.de_calls>mb3d_native::kGeneralMaxDistanceCalls||value.formula_work>mb3d_native::kGeneralMaxFormulaWork;
        };
        nonfinite|=!std::isfinite(g.initial_rout);
        bool orbitColorFault=false;
        if(p.rm_result&&packing.color_option){
            const auto& h=g.hit_color;
            orbitColorFault=h.version!=mb3d_native::kNativeHitColorVersion||h.reserved;
            nonfinite|=!std::isfinite(h.rout)||!std::isfinite(h.rold);
            out.execution.orbit_color_records++;
        }
        const auto& in=static_cast<const mb3d_inside::Record*>(insideRecords.contents)[i];
        const bool insideFault=mb3d_inside::validate_primary_record(insidePolicy,in,
            {p.fault,uint32_t(p.rm_result!=0),p.start_inside})!=nullptr;
        out.execution.inside_records++;out.execution.inside_attempts+=in.attempts;out.execution.inside_retries+=in.retries;
        out.execution.inside_initial_hits+=p.rm_result&&in.start_inside;out.execution.inside_interior_surfaces+=p.rm_result&&in.final_inside;
        out.execution.inside_smooth_calls+=in.smooth_calls;
        bool fault=orbitColorFault||insideFault||cameraFault||invalidGeometry(g)||
            p.fault||p.reserved_tail||p.rm_result>2||p.start_inside>1||
            g.hit!=uint32_t(p.rm_result!=0)||(ray.cut_miss?(g.de_calls||g.formula_work):(!g.de_calls||!g.formula_work))||
            !validNormal(static_cast<const mb3d_native::NativeNormalRecord*>(normalRecords.contents)[i],
                static_cast<const mb3d_native::NativeNormalResult*>(normalResults.contents)[i],
                uint32_t(p.rm_result&&!p.start_inside),g)||
            g.de_calls>mb3d_native::kGeneralMaxDistanceCalls||g.formula_work>mb3d_native::kGeneralMaxFormulaWork||
            colors[i].fault||colors[i].reserved0||colors[i].reserved1||nonfinite;
        for(const auto* s:{&packed[i],&shaded[i]})fault|=s->fault||s->reserved[0]||s->reserved[1]||s->reserved[2];
        const auto& primaryNormal=static_cast<const mb3d_native::NativeNormalResult*>(normalResults.contents)[i];
        fault|=(packed[i].depth_packed&255u)!=primaryNormal.roughness_byte;
        out.execution.primary_rough_pixels+=primaryNormal.roughness_byte!=0;
        uint64_t rangeExits=g.fp32_range_exits,carriedReads=g.difs_carried_reads;
        if(shadows){
            const auto* multi=multiShadow?&static_cast<const mb3d_native::NativeMultiShadowRecord*>(shadowRecords.contents)[i]:nullptr;
            const auto* soft=softShadow?&static_cast<const mb3d_native::NativeSoftShadowRecord*>(shadowRecords.contents)[i]:nullptr;
            const auto& s=soft?soft->total:(multi?multi->total:static_cast<const mb3d_native::NativeShadowRecord*>(shadowRecords.contents)[i]);
            if(soft){
                fault|=!ValidSoftShadowRecord(*soft,packed[i],shaded[i],shadow.max_steps,ambient);
                out.execution.soft_shadow_records++;out.execution.soft_shadow_partial_pixels+=soft->partial;
                out.execution.soft_shadow_dark_pixels+=soft->intensity==0;
                out.execution.soft_shadow_light_pixels+=soft->intensity==63;
            }
            if(multi)fault|=!ValidMultiShadowRecord(*multi,packed[i],shaded[i],shadow.light_mask,shadow.max_steps,ambient,shadow.position_mask);
            const unsigned lights=multiShadow?std::popcount(shadow.light_mask):1;
            fault|=s.version!=mb3d_native::kGenericShadowVersion||s.fault||s.reserved||s.active>1||s.backface>lights||s.rays>lights||s.hits>lights||s.misses>lights||
                s.skipped>lights||s.changed>1||s.hits+s.misses!=s.rays||s.steps>shadow.max_steps*lights||
                invalidGeometry(s.eval)||
                s.shadow_de_calls>s.eval.de_calls||s.changed!=uint32_t(packed[i].shadow!=shaded[i].shadow)||
                (s.active&&(!s.eval.de_calls||!s.eval.formula_work));
            addSmoothing(s.eval.smoothing);
            out.execution.shadow_records++;rangeExits+=s.eval.fp32_range_exits;carriedReads+=s.eval.difs_carried_reads;
            out.execution.distance_calls+=s.eval.de_calls;out.execution.formula_work+=s.eval.formula_work;
            out.execution.shadow_rays+=s.rays;out.execution.shadow_hits+=s.hits;out.execution.shadow_misses+=s.misses;
            out.execution.shadow_steps+=s.steps;out.execution.shadow_changed+=s.changed;
            out.execution.nonfinite+=s.eval.formula_fault.code==mb3d_native::NonFiniteFormula||!std::isfinite(s.eval.initial_rout);
            out.execution.work_exhausted+=s.eval.formula_fault.code==mb3d_native::FormulaWorkExhausted||((s.fault|s.eval.fault)&2u)!=0;
        }
        // Primary work/status was observed exactly once after each completed batch.
        if(reflections){
            const auto& record=static_cast<const mb3d_native::NativeGenericReflectionRecord*>(reflectionRecords.contents)[i];
            const auto& r=record.activity;const auto& eval=record.eval;auto& activity=out.reflections;
            const auto& in=static_cast<const mb3d_inside::ReflectionRecord*>(insideReflectionRecords.contents)[i];
            const bool insideFault=mb3d_inside::validate_reflection_record(insidePolicy,insideConfig,prepared.camera.device.cut_options,in,
                {r.fault,r.active,uint32_t(r.active&&shaded[i].si_gradient>=32768),shaded[i].o_trap,r.ao_rays,r.hits,r.shadow_rays})!=nullptr;
            out.execution.reflection_inside_records++;out.execution.reflection_inside_selected_pixels+=r.active&&in.selected_inside;
            out.execution.reflection_inside_ao_rays+=in.inside_ao_rays;out.execution.reflection_inside_absorption_bounces+=in.absorption_bounces;
            out.execution.reflection_inside_initial_primary+=in.initial_primary;out.execution.reflection_inside_cut_ao_skips+=in.cut_ao_skips;
            out.execution.reflection_inside_cut_shadow_limits+=in.cut_shadow_limits;
            const bool initialOpenAir=mb3d_inside::initial_open_air_without_de(
                !insideFault,shaded[i].si_gradient,in,r,eval);
            const bool invalid=insideFault||r.fault||r.nonfinite||r.reserved0||r.reserved1||r.active>1||r.changed_pixels>1||
                r.primary_skipped>1||r.bounces>reflection.max_bounces||r.rays>shadeCapacityRays||r.hits+r.misses!=r.rays||
                invalidGeometry(eval)||
                !validNormal(static_cast<const mb3d_native::NativeNormalRecord*>(reflectionNormalRecords.contents)[i],
                    static_cast<const mb3d_native::NativeNormalResult*>(reflectionNormalResults.contents)[i],r.hits,eval)||
                (r.rays&&(!eval.de_calls||!eval.formula_work)&&!initialOpenAir);
            if(invalid && firstFault.empty()) {
                const auto& nr=static_cast<const mb3d_native::NativeNormalRecord*>(reflectionNormalRecords.contents)[i];
                const auto& nv=static_cast<const mb3d_native::NativeNormalResult*>(reflectionNormalResults.contents)[i];
                firstFault=std::to_string(i)+" reflection fault="+std::to_string(r.fault)+" nonfinite="+std::to_string(r.nonfinite)+
                    " eval="+std::to_string(eval.fault)+" formula="+std::to_string(eval.formula_fault.code)+
                    " id="+std::to_string(eval.formula_fault.formula_id)+" iteration="+std::to_string(eval.formula_fault.iteration)+
                    " hits="+std::to_string(r.hits)+" rays="+std::to_string(r.rays)+" steps="+std::to_string(r.ray_steps)+
                    " normal_fault="+std::to_string(nr.fault)+" normal_result="+std::to_string(nv.fault)+
                    " inside="+std::to_string(insideFault)+" smooth_invalid="+std::to_string(eval.smoothing.invalid)+
                    " geometry="+std::to_string(invalidGeometry(eval))+" normal="+std::to_string(!validNormal(nr,nv,r.hits,eval))+
                    " bounces="+std::to_string(r.bounces)+" misses="+std::to_string(r.misses)+" de_calls="+std::to_string(eval.de_calls)+
                    " work="+std::to_string(eval.formula_work)+" range_exits="+std::to_string(eval.fp32_range_exits)+
                    " carried="+std::to_string(eval.difs_carried_reads);
            }
            fault|=invalid;activity.faults+=invalid;activity.active_pixels+=r.active;activity.rays+=r.rays;
            activity.hits+=r.hits;activity.misses+=r.misses;activity.changed_pixels+=r.changed_pixels;
            activity.primary_skipped+=r.primary_skipped;activity.ray_steps+=r.ray_steps;
            activity.ao_rays+=r.ao_rays;activity.ao_steps+=r.ao_steps;activity.shadow_rays+=r.shadow_rays;activity.shadow_steps+=r.shadow_steps;
            activity.max_bounces=std::max(activity.max_bounces,uint64_t(r.bounces));
            addSmoothing(eval.smoothing);
            out.execution.reflection_records++;out.execution.distance_calls+=eval.de_calls;out.execution.formula_work+=eval.formula_work;
            out.execution.reflection_gradient_range_clamps+=eval.gradient_range_clamps;rangeExits+=eval.fp32_range_exits;
            carriedReads+=eval.difs_carried_reads;
            out.execution.nonfinite+=r.nonfinite||eval.formula_fault.code==mb3d_native::NonFiniteFormula||!std::isfinite(eval.initial_rout);
            out.execution.work_exhausted+=(r.fault|eval.fault)&2u?1u:0u;
        }
        out.execution.primary_records++;out.execution.normal_records++;out.execution.surface_records+=2;out.execution.color_records++;
        out.execution.primary_gradient_range_clamps+=g.gradient_range_clamps;
        out.execution.fp32_range_exits+=rangeExits;out.execution.fp32_range_exit_pixels+=rangeExits!=0;
        out.execution.difs_carried_reads+=carriedReads;out.execution.difs_carried_pixels+=carriedReads!=0;
        out.execution.faults+=fault;out.execution.nonfinite+=nonfinite||g.formula_fault.code==mb3d_native::NonFiniteFormula;
        if(fault&&faultPixels.size()<12)faultPixels.push_back(i);
        out.execution.work_exhausted+=g.formula_fault.code==mb3d_native::FormulaWorkExhausted||((g.fault|p.fault)&2u)!=0;
        if(fault&&firstFault.empty())firstFault=std::to_string(i)+" geometry="+std::to_string(g.fault)+" formula="+std::to_string(g.formula_fault.code)+
            " id="+std::to_string(g.formula_fault.formula_id)+" iteration="+std::to_string(g.formula_fault.iteration)+
            " calls="+std::to_string(g.de_calls)+" work="+std::to_string(g.formula_work)+
            " start_inside="+std::to_string(p.start_inside)+" primary="+std::to_string(p.fault)+
            " normal_calls="+std::to_string(g.normal_calls)+" expected_samples="+std::to_string(samplesPerNormal)+
            " surface="+std::to_string(packed[i].fault)+" shaded="+std::to_string(shaded[i].fault)+
            " color="+std::to_string(colors[i].fault)+" nonfinite="+std::to_string(nonfinite);
    }
    if(!firstFault.empty()){
        // The count and the first pixels tell a numeric edge from a dispatch fault.
        firstFault+=" faults="+std::to_string(out.execution.faults)+"/"+std::to_string(count)+
            " width="+std::to_string(scene.width)+" pixels=";
        for(size_t k=0;k<faultPixels.size();++k)firstFault+=(k?",":"")+std::to_string(faultPixels[k]);
    }
    gpu::require(firstFault.empty(),"Native general invalid GPU record at pixel "+firstFault);
    out.execution.difs_conditional_pixels=execution.conditional_state();
    if(execution.conditional_state()&&out.execution.difs_carried_reads)
        throw CarriedStateObserved(out.execution.difs_carried_reads,out.execution.difs_carried_pixels);
    out.execution.fp32_range_gate_stage="geometry";
    out.execution.fp32_range_gate_units=count;out.execution.fp32_range_gate_exceeded=out.execution.fp32_range_exit_pixels;
    out.execution.fp32_range_gate_percent=fp32RangePixelPercent();
    if(out.execution.fp32_range_exit_pixels*100>uint64_t(count)*fp32RangePixelPercent())
        throw Fp32RangeExceeded("Native FP32 range: "+std::to_string(out.execution.fp32_range_exit_pixels)+" of "+
            std::to_string(count)+" pixels left the binary32 range");
    // Validate every upstream color before postprocessing, including pixels
    // that might contribute no blur samples. A later pass cannot mask a fault.
    id<MTLBuffer> finalRgb=b.rgb;
    if(dof.version){
        timing.begin(RenderProfileStage::DOF);
        auto blurred=RunNativeDOF(m,dof,b.shaded,b.rgb,*resources.dof_execution,[&]{invocation.poll();});
        finalRgb=blurred.rgb;
        const auto& a=blurred.activity;
        gpu::require(a.passes==dof.passes&&a.records==uint64_t(count)*dof.passes&&!a.faults,
                     "Native DOF incomplete GPU records");
        out.execution.dof_records=a.records;out.execution.dof_passes=a.passes;
        out.execution.dof_positive_radius_sources=a.positive_radius_sources;
        out.execution.dof_sort_work=a.sort_work;out.execution.dof_candidate_scan_checks=a.candidate_scan_checks;
        out.execution.dof_candidate_checks=a.candidate_checks;out.execution.dof_contributing_pairs=a.contributing_pairs;
        out.execution.dof_changed_pixels=a.changed_pixels;out.execution.dof_maximum_radius=a.maximum_radius;
        out.execution.dof_dispatches=a.dispatches;
        timing.end();
    }
    const auto* finalColors=static_cast<const ExactColorPixel*>(finalRgb.contents);
    for(uint32_t i=0;i<count;++i){if((i&255u)==0)invocation.poll();
        gpu::require(!finalColors[i].fault&&!finalColors[i].reserved0&&
        !finalColors[i].reserved1&&finalColors[i].rgb<=0xffffffu,"Native final color record fault");}
    result.width=scene.width;result.height=scene.height;result.image.resize(size_t(count)*3);result.silight.resize(count);
    for(uint32_t i=0;i<count;++i){if((i&255u)==0)invocation.poll();for(unsigned channel=0;channel<3;++channel)result.image[i*3+channel]=uint8_t(finalColors[i].rgb>>(8*channel));
        result.silight[i]=ImportSurface(shaded[i]);}
    invocation.complete();
    out.execution.pipeline_archive_hits=m.pipelineArchiveHits;out.execution.pipeline_archive_misses=m.pipelineArchiveMisses;
    out.execution.gpu_seconds=m.gpuMs/1000;out.execution.gpu_commands=m.gpuCommands;
    out.execution.validated=true;result.success=true;result.status=RenderStatus::Completed;out.status=NativeMetalStatus::Completed;
    header.Width=width;header.Height=height;header.iOptions=effective.iOptions;
    header.bCalcAmbShadowAutomatic=effective.bCalcAmbShadowAutomatic;
}
}
// The library bytes are read and hashed once, when the session is created.
struct RenderSession {
    gpu::SharedMetal metal;
    std::string library_path;
    std::mutex mutex;
};
RenderSession* CreateRenderSession(const std::string& library_path,std::string& error) {
    @autoreleasepool {
        try{
            auto session=std::make_unique<RenderSession>();
            session->metal.device=MTLCreateSystemDefaultDevice();
            gpu::require(session->metal.device!=nil,"Metal device unavailable");
            session->metal.library=gpu::nativeLibrary(session->metal.device,library_path);
            session->metal.queue=[session->metal.device newCommandQueue];
            gpu::require(session->metal.queue!=nil,"Metal queue unavailable");
            session->library_path=library_path;
            return session.release();
        }catch(const std::exception& e){error=e.what();return nullptr;}
    }
}
void DestroyRenderSession(RenderSession* session) { @autoreleasepool { delete session; } }
NativeMetalAttempt RenderGeneralNativeMetal(TMandHeader10& header,THeaderCustomAddon& addon,
    const RenderOptions& options,const std::string& requested_library,RenderSession* session) {
    // One render at a time per session; the session's library snapshot wins.
    std::unique_lock<std::mutex> exclusive;
    if(session)exclusive=std::unique_lock<std::mutex>(session->mutex);
    const std::string& library_path=session?session->library_path:requested_library;
    gpu::SharedMetal* shared=session?&session->metal:nullptr;
    @autoreleasepool {
        NativeMetalAttempt out;out.library=library_path;out.result.profile.precision=PrecisionMode::Native;
        bool controlStarted=false;
        ControlInvocation invocation(options.control,&controlStarted);
        try{
            try{attempt(header,addon,options,library_path,out,invocation,false,shared);}
            catch(const CarriedStateObserved& e){
                // The conditional frame is discarded; the CPU-order frame is
                // the result, under the same started control and budget.
                out=NativeMetalAttempt{};out.library=library_path;out.result.profile.precision=PrecisionMode::Native;
                attempt(header,addon,options,library_path,out,invocation,true,shared);
                out.execution.difs_rerun_carried_reads=e.reads;out.execution.difs_rerun_carried_pixels=e.pixels;
            }
        }
        catch(const RenderStopped& e){out.status=NativeMetalStatus::Failed;out.reason=e.what();out.result.error=e.what();
            out.result.success=false;out.result.status=e.status();out.result.image.clear();out.result.silight.clear();}
        catch(const RenderCancelRequested& e){out.status=NativeMetalStatus::Failed;out.reason=e.what();out.result.error=e.what();
            out.result.success=false;out.result.status=RenderStatus::Cancelled;out.result.image.clear();out.result.silight.clear();}
        catch(const Fp32RangeExceeded& e){out.status=NativeMetalStatus::Fp32Range;out.reason=e.what();out.result.error=e.what();
            out.result.success=false;out.result.status=RenderStatus::Failed;out.result.image.clear();out.result.silight.clear();}
        catch(const std::exception& e){out.status=NativeMetalStatus::Failed;out.reason=e.what();out.result.error=e.what();
            out.result.success=false;out.result.status=RenderStatus::Failed;out.result.image.clear();out.result.silight.clear();}
        if(!out.result.success)invocation.fail();
        if(controlStarted&&options.control){
            out.result.work_units=options.control->work_used();
            if(!out.result.success){
                const auto terminal=options.control->status();
                if(terminal==RenderStatus::Cancelled||terminal==RenderStatus::WorkExhausted||terminal==RenderStatus::Failed)
                    out.result.status=terminal;
                if(terminal==RenderStatus::Cancelled||terminal==RenderStatus::WorkExhausted){
                    out.reason=RenderStopped(terminal).what();out.result.error=out.reason;
                }
            }
        }
        return out;
    }
}
} // namespace mb3d::native_metal
