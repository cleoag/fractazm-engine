#pragma once
#include "pixel_state_contract.h"
#include "execution_policy.h"
#include "image_execution_policy.h"
#include "general_scene_host.h"
#include "generic_color_host.h"
#include "general_normals_host.h"
#include "native_inside_host.h"
#include "native_ao_host.h"
#include "postprocess_plan.h"
#include "resource_requirements.h"
#include "soft_shadow_host.h"
#include "comparison/metal_exact/native_reflection_host.h"
#include <string>
#include <array>
#include <vector>
#include <optional>

namespace mb3d::native_metal {
// Host-only preparation policy. Diagnostic continuation does not make a failed
// plan dispatchable. Preparation is ordered: legacy helpers mutate owned context.
enum class PlanDiagnostics { StopAtFirstFailure, CollectIndependentStages };
struct RenderPlanRequest {
    unsigned width=0,height=0;
    int logical_workers=0;
    std::string resource_root;
    bool deterministic=false,no_dof=false,no_reflections=false,no_shadows=false,no_ao=false;
    bool force_calc3d=false; // Explicit CLI/admission normalization, not an implicit renderer default.
    // Re-render of a conditional (dIFS) contract frame in CPU order.
    bool ordered_carried_state=false;
    // Every stride-th pixel of the width x height frame; see Lattice.
    Lattice lattice;
};
struct RenderPlanStage {std::string name,status,reason;};
struct PreparedRenderPlan {
    // Preserve source intent before normalization/map loading can change flags.
    TMandHeader10 requested_header{},effective_header{};
    THeaderCustomAddon requested_addon{};
    RenderPlanRequest request;
    std::vector<LightingResourceRequirement> lighting_requirements;
    PreparedGeneralScene scene;
    mb3d_native::NativeSurfaceParams packing{};
    PreparedGeneralColor color;
    NativeAOPreparation screen_ao;
    mb3d_native_ao::DEParams distance_ao{};
    mb3d_dof::Params dof{};
    mb3d_zn::Params z_normals{};
    mb3d_native::NativeNormalParams normals{};
    mb3d_inside::Params inside{};
    mb3d_inside::ReflectionConfig inside_reflection{};
    ExactShadowParams shadow{};
    mb3d_native::NativeSoftShadowConfig soft_shadow{};
    NativeReflectionParams reflection{};
    uint32_t ao_algorithm=0,logical_workers=1;
    uint64_t ao_bytes=0,dof_bytes=0,z_normal_bytes=0;
    double ao_preparation_seconds=0;
    bool shadows=false,soft_shadows=false,ambient=false,reflections=false;
    // RayMarch first-step jitter of the effective header (iOptions bit 0).
    bool primary_random=false;
    std::vector<RenderPlanStage> stages;
    bool ready() const;
    std::string firstFailure() const;
};
// Caller owns the returned context for as long as descriptors/pointers are used.
// No Metal API, pixel/orbit evaluation, or GPU allocation occurs here.
PreparedRenderPlan PrepareNativeRenderPlan(const TMandHeader10&,const THeaderCustomAddon&,
    const RenderPlanRequest&,PlanDiagnostics=PlanDiagnostics::StopAtFirstFailure);
GeneralExecutionRequest NativeGeneralExecutionRequest(const PreparedRenderPlan&);
struct RenderDeviceLimits {
    uint64_t max_buffer_bytes=0,recommended_working_set_bytes=0;
};
struct PreparedRenderResources {
    std::array<uint64_t,49> buffers{};
    DOFLimits dof_limits{};
    uint64_t budget=0,allocated_bytes=0,ao_budget=0;
    uint32_t shadow_rows=0,shadow_tasks=0;
    bool persistent=false,multi_shadow=false,parallel_reflections=false;
    bool primary_persistent=false;
    mb3d_native::PixelStateContract primary_state{};
    bool shadow_persistent=false,parallel_soft_shadows=false;
    mb3d_native::PixelStateContract soft_shadow_state{};
    std::optional<GeneralExecutionPolicy> execution;
    std::optional<DistanceAOExecutionPolicy> distance_execution;
    std::optional<ScreenAOExecutionPolicy> screen_execution;
    std::optional<DOFExecutionPolicy> dof_execution;
    std::optional<ZNormalsExecutionPolicy> znormal_execution;
};
// Device limits are supplied as data; admission can label a hypothetical budget
// explicitly without creating a Metal device. Runtime passes actual device limits.
PreparedRenderResources PlanNativeRenderResources(const PreparedRenderPlan&,const RenderDeviceLimits&);
std::string DescribeNativeExecutionPolicy(const PreparedRenderResources&);
}
