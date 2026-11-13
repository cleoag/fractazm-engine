#include "render_plan.h"
#include "seed_scan_plan.h"
#include "generic_shadow_abi.h"
#include "generic_reflection_abi.h"
#include "logical_lane_abi.h"
#include "dof_host.h"
#include "zbuffer_host.h"
#include "native_ao_readback.h"
#include "comparison/metal_exact/scene_host_internal.h"
#include "comparison/metal_exact/shadow_host.h"
#include "core/numeric_precision.h"
#include <algorithm>
#include <chrono>
#include <thread>

namespace mb3d::native_metal {
using namespace metal_exact;
bool PreparedRenderPlan::ready() const {
    return stages.size()==10&&std::none_of(stages.begin(),stages.end(),
        [](const auto& s){return s.status=="REJECTED";});
}
std::string PreparedRenderPlan::firstFailure() const {
    for(const auto& s:stages)if(s.status=="REJECTED")return s.reason;
    return {};
}
PreparedRenderPlan PrepareNativeRenderPlan(const TMandHeader10& header,const THeaderCustomAddon& addon,
    const RenderPlanRequest& request,PlanDiagnostics diagnostics) {
    PreparedRenderPlan p;
    p.requested_header=header;p.requested_addon=addon;p.effective_header=header;p.request=request;
    p.lighting_requirements=NativeLightingRequirements(header);
    auto& effective=p.effective_header;
    if(request.force_calc3d)effective.bCalc3D=1;
    if(request.deterministic){effective.iOptions&=~1;effective.bCalcAmbShadowAutomatic&=uint8_t(~128u);}
    if(request.no_dof)effective.bCalcDOFtype&=uint8_t(~1u);
    if(request.no_reflections)effective.bCalcSRautomatic&=uint8_t(~1u);
    const ScopedNumericPrecision precision(PrecisionMode::Native);
    auto stage=[&](const char* name,bool required,const auto& action){
        if(!required){p.stages.push_back({name,"NOT_REQUIRED",{}});return true;}
        try{action();p.stages.push_back({name,"ACCEPTED",{}});return true;}
        catch(const std::exception& e){p.stages.push_back({name,"REJECTED",e.what()});return false;}
    };
    const bool collect=diagnostics==PlanDiagnostics::CollectIndependentStages;
    if(!stage("general_scene",true,[&]{
        if(!request.width||!request.height||request.width>4096||request.height>4096)
            throw std::invalid_argument("Native general image dimensions outside 1..4096");
        // Depth of field and Z-buffer normals read neighbouring pixels of the full frame.
        if(!request.lattice.dense()&&((effective.bCalcDOFtype&1)||(effective.bCalc3D&&(effective.byCalcNsOnZBufAuto&1))))
            throw std::invalid_argument("Native lattice frame with depth of field or Z-buffer normals");
        p.scene=PrepareGeneralScene(effective,addon,request.width,request.height,request.resource_root,65536,request.lattice);
    }))return p; // Every subsequent stage depends on the scene and its owning context.
    auto& prepared=p.scene;
    int workers=request.logical_workers;
    if(workers<=0){workers=int(std::thread::hardware_concurrency());if(workers<=0)workers=4;}
    p.logical_workers=uint32_t(std::clamp(workers,1,int(prepared.base.device.height)));
    p.shadows=!request.no_shadows&&(prepared.base.original_shadow_bits&1)&&(prepared.base.original_shadow_bits&0xfc);
    p.soft_shadows=p.shadows&&(effective.bCalc1HSsoft&1);
    p.reflections=(effective.bCalcSRautomatic&1)!=0;
    p.primary_random=(effective.iOptions&1)!=0;
    if(!stage("inside_policy",true,[&]{p.inside=PrepareNativeInside(prepared.base.host->mct);})&&!collect)return p;
    if(!stage("normal_descriptor",true,[&]{p.normals=PrepareGeneralNormals(prepared.base.host->mct,prepared.formulas.device);})&&!collect)return p;
    if(!stage("dof_descriptor",true,[&]{
        auto h=effective;h.Width=request.width;h.Height=request.height;
        const auto descriptor=PrepareNativeDOF(h);const auto bytes=NativeDOFAllocationBytes(descriptor);
        p.dof=descriptor;p.dof_bytes=bytes;
    })&&!collect)return p;
    if(!stage("z_normal_descriptor",true,[&]{
        const auto descriptor=PrepareNativeZNormals(prepared.base.host->file.header);
        const auto bytes=NativeZNormalsAllocationBytes(descriptor);p.z_normals=descriptor;p.z_normal_bytes=bytes;
    })&&!collect)return p;
    if(!stage("surface",true,[&]{p.packing=PrepareGeneralSurface(prepared);})&&!collect)return p;
    if(!stage("hard_shadow",p.shadows,[&]{
        auto descriptor=PrepareShadow(prepared.base,false,65536,true,p.soft_shadows);
        if(!descriptor.enabled||!descriptor.light_mask||descriptor.light_mask>63)
            throw std::invalid_argument("Invalid Native general hard-shadow mask");
        mb3d_native::NativeSoftShadowConfig soft{};
        if(p.soft_shadows)soft=PrepareGeneralSoftShadow(prepared.base,descriptor);
        p.shadow=descriptor;p.soft_shadow=soft;
    })&&!collect)return p;
    p.ambient=!request.no_ao&&prepared.base.host->mct.bCalcAmbShadow;
    if(!stage("ambient_occlusion",p.ambient,[&]{
        const auto begin=std::chrono::steady_clock::now();
        // Full camera header and actual tile allocation are deliberately distinct.
        const auto& h=prepared.base.host->file.header;
        const auto algorithm=mb3d_native_ao::dispatch(h.bCalcAmbShadowAutomatic,h.bPlanarOptic);
        uint64_t bytes;
        if(algorithm==mb3d_native_ao::DistanceEstimate){
            auto descriptor=PrepareNativeDistanceAO(h,prepared.base.host->mct,prepared.base.device.width,
                prepared.base.device.height,p.logical_workers);
            bytes=mb3d_native_ao::de_scratch_bytes(descriptor);p.distance_ao=descriptor;
        }else{
            // On a lattice the neighbours of a pixel are `stride` pixels away:
            // screen AO is prepared as for a frame of the lattice's own size,
            // whose packed depths are the same.
            auto aoHeader=h;
            if(!request.lattice.dense()){aoHeader.Width=int(prepared.base.device.width);aoHeader.Height=int(prepared.base.device.height);}
            auto descriptor=PrepareNativeScreenAO(aoHeader,prepared.base.device.width,prepared.base.device.height,p.logical_workers);
            const auto execution=ProductionScreenAOExecutionPolicy(descriptor);
            bytes=mb3d_native_ao::screen_scratch_bytes(descriptor.params,execution.uses_certificates(),execution.uses_chunk_prefix());p.screen_ao=std::move(descriptor);
        }
        p.ao_algorithm=algorithm;p.ao_bytes=bytes;
        p.ao_preparation_seconds=std::chrono::duration<double>(std::chrono::steady_clock::now()-begin).count();
    })&&!collect)return p;
    if(!stage("color",true,[&]{
        ValidateNativeLightingRequirements(p.lighting_requirements,true);
        p.color=PrepareGeneralColor(prepared.base,p.shadows,request.lattice);
    })&&!collect)return p;
    stage("reflection",p.reflections,[&]{
        // A DE combination's secondary rays compare each DE with its winning
        // part's maximum; DEoptionResult only selects transparency, closed here.
        const auto inside=PrepareNativeInsideReflection(prepared.base.host->mct,prepared.base.host->file.header,
            prepared.base.host->file.addon);
        const auto descriptor=PrepareGeneralNativeReflection(prepared.base,65536,true);
        // Secondary rays require this descriptor even with primary shadows off.
        // Diagnostic continuation after a failed shadow stage cannot reuse its output.
        auto shadow=p.shadow;
        const bool shadowReady=std::any_of(p.stages.begin(),p.stages.end(),[](const auto& s){
            return s.name=="hard_shadow"&&s.status=="ACCEPTED";});
        // CalcHSsoftPoint reuses the primary soft descriptor, so soft
        // reflection shadows require the accepted primary soft pass.
        if((effective.bCalc1HSsoft&1)&&descriptor.shadows_enabled&&(!p.soft_shadows||!shadowReady))
            throw std::invalid_argument("Unsupported Native reflection soft shadows without the primary soft pass");
        if(!p.shadows||!shadowReady)shadow=PrepareShadow(prepared.base,false,65536,true,effective.bCalc1HSsoft&1);
        // Reflected points march one shadow ray per enabled light (CalcHShardPoint).
        p.inside_reflection=inside;p.reflection=descriptor;p.shadow=shadow;
    });
    return p;
}
GeneralExecutionRequest NativeGeneralExecutionRequest(const PreparedRenderPlan& plan) {
    const auto& p=plan.scene;
    return {p.base.device.width,p.base.device.height,plan.logical_workers,
        plan.inside.initial_inside,plan.inside.in_and_out,plan.shadow.light_mask,
        p.formulas.requires_persistent_lane,plan.shadows,plan.soft_shadows,plan.reflections,
        p.formulas.maps.empty()&&p.formulas.map_pixels.empty(),!plan.request.ordered_carried_state,
        plan.shadow.position_mask};
}
std::string DescribeNativeExecutionPolicy(const PreparedRenderResources& r) {
    if(!r.execution||!r.dof_execution||!r.znormal_execution)
        throw std::invalid_argument("Incomplete Native execution policy");
    std::string s="general-v1|configured-policy-not-activity|"+DescribeEntry(r.execution->primary())+"|"+
        DescribeEntry(r.execution->shadow())+"|"+DescribeEntry(r.execution->reflection());
    if(r.distance_execution){s+="|"+DescribeEntry(r.distance_execution->entry());
        if(r.distance_execution->seeded())s+="|ao-de-seeded";}
    if(r.screen_execution){const auto& a=*r.screen_execution;
        s+="|ao-sampling:"+DescribeImageEntry(a.sampling())+"|ao-raster-slice="+std::to_string(a.raster_slice_batch())+
            "|wavelet:"+DescribeImageEntry(a.wavelet())+"|ao-command-policy:"+a.commands().describe();
        if(a.uses_rng()){s+="|rng:"+DescribeImageEntry(a.rng());
            if(a.parallel_random())s+="|ao-seeded-pixel-batch="+std::to_string(a.seeded_pixel_batch());
            s+="|seed-geometry:version=1,certified="+std::to_string(a.uses_certificates())+
                ",chunk-prefix="+std::to_string(a.uses_chunk_prefix())+",base-table-reuse="+std::to_string(a.uses_chunk_prefix())+",forced-unknown="+std::to_string(a.force_unknown())+",batch="+std::to_string(a.certificate_pixel_batch());}
        if(a.uses_median())s+="|median:"+DescribeImageEntry(a.median());}
    if(r.dof_execution->allocation().pixels)s+="|dof-sort:"+DescribeImageEntry(r.dof_execution->sort())+
       "|dof-spatial-option:"+DescribeImageEntry(r.dof_execution->spatial())+
       "|dof-gather:"+DescribeImageEntry(r.dof_execution->gather())+
       "|dof-finish:"+DescribeImageEntry(r.dof_execution->finish());
    else s+="|dof=disabled";
    if(r.znormal_execution->allocation().surface)s+="|zn:"+DescribeImageEntry(r.znormal_execution->reconstruction_and_normals());
    else s+="|zn=disabled";
    return s;
}
PreparedRenderResources PlanNativeRenderResources(const PreparedRenderPlan& plan,const RenderDeviceLimits& limits) {
    if(!plan.ready())throw std::invalid_argument("Incomplete Native render plan cannot allocate resources");
    if(!limits.max_buffer_bytes)throw std::invalid_argument("Invalid Native device buffer limit");
    const auto& prepared=plan.scene;const auto& scene=prepared.base.device;
    const uint32_t count=scene.width*scene.height,logicalWorkers=plan.logical_workers;
    const auto& color=plan.color.base;const auto& interior=plan.color.interior;const auto& normalParams=plan.normals;
    const auto& insidePolicy=plan.inside;const auto& insideConfig=plan.inside_reflection;
    const auto& shadow=plan.shadow;const auto& dof=plan.dof;
    auto dofBytes=plan.dof_bytes;const auto znormalBytes=plan.z_normal_bytes,aoBytes=plan.ao_bytes;
    const bool shadows=plan.shadows,softShadow=plan.soft_shadows,reflections=plan.reflections;
    const bool persistent=prepared.formulas.requires_persistent_lane;
    const GeneralExecutionPolicy execution(prepared.formulas.device,NativeGeneralExecutionRequest(plan),
        prepared.formulas.program_count()>1?&prepared.formulas.second:nullptr);
    const auto primaryState=execution.primary_state(),softShadowState=execution.soft_state();
    const bool primaryPersistent=execution.primary_persistent(),parallelSoftShadows=execution.parallel_soft_shadows();
    const bool shadowPersistent=execution.shadow_persistent(),multiShadow=execution.multi_shadow();
    const bool parallelReflections=execution.parallel_reflections();
    const uint32_t shadowRows=execution.shadow_rows(),shadowTasks=execution.shadow_tasks();
    std::optional<DistanceAOExecutionPolicy> distanceExecution;
    std::optional<ScreenAOExecutionPolicy> screenExecution;
    if(plan.ambient&&plan.ao_algorithm==mb3d_native_ao::DistanceEstimate)
        distanceExecution.emplace(prepared.formulas.device,plan.distance_ao,
            prepared.formulas.program_count()>1?&prepared.formulas.second:nullptr);
    else if(plan.ambient)screenExecution=ProductionScreenAOExecutionPolicy(plan.screen_ao);
    const bool speculativeShadows=shadows&&execution.speculative_shadows();
    const std::array<uint64_t,49> sizes={count*sizeof(ExactPixel),count*sizeof(mb3d_native::NativeGeometryRecord),
        count*sizeof(ExactSurface),count*sizeof(ExactSurface),count*sizeof(ExactColorPixel),
        prepared.base.columns.size()*sizeof(ExactTrig),prepared.base.rows.size()*sizeof(ExactTrig),
        prepared.formulas.program_count()*sizeof(mb3d_native::NativeFormulaProgram),
        std::max(size_t(1),prepared.formulas.maps.size())*sizeof(mb3d_native::NativeMapDescriptor),
        std::max(size_t(1),prepared.formulas.map_pixels.size())*sizeof(float),
        // Keep stable buffer indices; Native AO owns its bounded workspace.
        16,16,16,16,sizeof(ExactColorScene),
        color.columns.size()*sizeof(ExactTrig),color.rows.size()*sizeof(ExactTrig),
        shadows?count*(softShadow?sizeof(mb3d_native::NativeSoftShadowRecord):(multiShadow?sizeof(mb3d_native::NativeMultiShadowRecord):sizeof(mb3d_native::NativeShadowRecord))):16,
        primaryPersistent?logicalWorkers*sizeof(mb3d_native::NativeLogicalLane):16,
        primaryPersistent?logicalWorkers*sizeof(mb3d_native::NativeFormulaState):16,
        primaryPersistent?logicalWorkers*sizeof(mb3d_native::NativeLogicalLaneRecord):16,
        shadows&&shadowPersistent?shadowTasks*sizeof(mb3d_native::NativeLogicalLane):16,
        shadows&&shadowPersistent?shadowTasks*sizeof(mb3d_native::NativeFormulaState):16,
        shadows&&shadowPersistent?shadowTasks*(multiShadow?sizeof(mb3d_native::NativeMultiShadowRowState):sizeof(mb3d_native::NativeShadowRowState)):16,
        shadows&&shadowPersistent?shadowTasks*sizeof(mb3d_native::NativeLogicalLaneRecord):16,
        reflections?count*sizeof(mb3d_native::NativeGenericReflectionRecord):16,
        reflections?shadowTasks*sizeof(mb3d_native::NativeLogicalLane):16,
        reflections?shadowTasks*sizeof(mb3d_native::NativeFormulaState):16,
        reflections?shadowTasks*sizeof(mb3d_native::NativeLogicalLaneRecord):16,
        sizeof(interior),sizeof(normalParams),count*sizeof(mb3d_native::NativeNormalRecord),
        count*sizeof(mb3d_native::NativeNormalResult),
        reflections?count*sizeof(mb3d_native::NativeNormalRecord):16,
        reflections?count*sizeof(mb3d_native::NativeNormalResult):16,
sizeof(mb3d_camera::Params),
        prepared.camera.columns.size()*sizeof(mb3d_camera::Axis),
        prepared.camera.rows.size()*sizeof(mb3d_camera::Axis),
        prepared.camera.paint_columns.size()*sizeof(mb3d_camera::Axis),
        prepared.camera.paint_rows.size()*sizeof(mb3d_camera::Axis),
        count*sizeof(mb3d_camera::Ray),sizeof(insidePolicy),count*sizeof(mb3d_inside::Record),
        sizeof(insideConfig),reflections?count*sizeof(mb3d_inside::ReflectionRecord):16,
        speculativeShadows?count*(multiShadow?sizeof(mb3d_native::NativeMultiShadowSpeculation):sizeof(mb3d_native::NativeShadowSpeculation)):16,
        speculativeShadows?count*(multiShadow?sizeof(mb3d_native::NativeMultiShadowRowState):sizeof(mb3d_native::NativeShadowRowState)):16,
        speculativeShadows?uint64_t(scene.height)*2*sizeof(uint32_t):16,
        // Primary jitter: a seed per pixel, then each task's running seed.
        plan.primary_random?(uint64_t(count)+logicalWorkers)*sizeof(uint32_t):16};
    uint64_t bytes=0;const uint64_t budget=NativeDeviceBudget(limits.recommended_working_set_bytes);
    for(auto size:sizes){if(size>limits.max_buffer_bytes||size>budget-bytes)
        throw std::invalid_argument("Native general buffers exceed device budget");bytes+=size;}
    if(plan.primary_random&&!primaryPersistent){
        const auto scan=PlanSeedScan(scene.width,scene.height,logicalWorkers,
            mb3d_seed_scan::Interleaved,(scene.height+logicalWorkers-1)/logicalWorkers,
            mb3d_seed_scan::PrimaryFlags);
        if(scan.scratch_bytes>limits.max_buffer_bytes||scan.scratch_bytes>budget-bytes)
            throw std::invalid_argument("Native primary seed scan exceeds device budget");
        bytes+=scan.scratch_bytes;
    }
    DOFLimits dofLimits{};
    if(dof.version && count>=16384) {
        // The wide tile batch keeps its 1 GiB frame threshold, so a frame
        // that fitted the former fixed budget keeps its DOF schedule.
        const uint64_t wideBudget=std::min(budget,kNativeStageWorkspaceCap);
        auto wide=dofLimits;wide.tile_batch=64;
        const uint64_t wideBytes=NativeDOFAllocationBytes(dof,wide);
        if(wideBytes<=limits.max_buffer_bytes && wideBytes<=wideBudget && bytes<=wideBudget-wideBytes) {
            dofLimits=wide;dofBytes=wideBytes;
        }
    }
    if(dofBytes>limits.max_buffer_bytes||dofBytes>kNativeStageWorkspaceCap||dofBytes>budget||bytes>budget-dofBytes)
        throw std::invalid_argument("Native general and DOF buffers exceed device budget");
    bytes+=dofBytes;
    if(znormalBytes>limits.max_buffer_bytes||znormalBytes>kNativeStageWorkspaceCap||znormalBytes>budget||bytes>budget-znormalBytes)
        throw std::invalid_argument("Native general and Z normal buffers exceed device budget");
    bytes+=znormalBytes;
    const uint64_t aoAllocationBudget=budget-bytes;
    if(aoBytes>limits.max_buffer_bytes||aoBytes>aoAllocationBudget)
        throw std::invalid_argument("Native general and AO buffers exceed device budget");
    return {sizes,dofLimits,budget,bytes+aoBytes,aoAllocationBudget,shadowRows,shadowTasks,
            persistent,multiShadow,parallelReflections,primaryPersistent,primaryState,
            shadowPersistent,parallelSoftShadows,softShadowState,execution,distanceExecution,screenExecution,
            DOFExecutionPolicy(dof,dofLimits),ZNormalsExecutionPolicy(plan.z_normals)};
}

}
