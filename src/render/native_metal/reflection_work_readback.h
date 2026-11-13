#pragma once
// Validation-only exception for CPU SR_CalcRay's initial-interior OpenAir
// branch. It does not create, repair or replace any surface/color result.
#include "inside_readback.h"
#include "generic_reflection_abi.h"
#include <bit>

namespace mb3d_inside {
inline bool initial_open_air_without_de(bool inside_record_valid, U32 gradient,
    const ReflectionRecord& inside, const NativeReflectionRecord& ray,
    const mb3d_native::NativeGeometryRecord& eval) {
    // The shader binds initial_primary to the original high SIgradient. That
    // branch skips refinement, emits one miss, then terminates after shading
    // background. Ordinary empty rays must not receive this exemption.
    const auto& fault=eval.formula_fault;
    return inside_record_valid && gradient>=32768u && gradient<=65535u &&
        inside.version==1 && !inside.fault && inside.initial_primary==1 &&
        !inside.inside_ao_rays && !inside.absorption_bounces &&
        !inside.cut_ao_skips && !inside.cut_shadow_limits &&
        !ray.fault && !ray.nonfinite && !ray.reserved0 && !ray.reserved1 &&
        ray.active==1 && ray.rays==1 && ray.bounces==1 && ray.misses==1 &&
        !ray.hits && !ray.primary_skipped && !ray.ray_steps &&
        !ray.ao_rays && !ray.ao_steps && !ray.shadow_rays && !ray.shadow_steps &&
        eval.version==mb3d_native::kGeneralGeometryVersion && !eval.fault &&
        !eval.ray_steps && !eval.de_calls && !eval.formula_work &&
        !eval.smoothing.calls && !eval.smoothing.recovered && !eval.smoothing.stagnant && !eval.smoothing.invalid &&
        !eval.normal_calls && !eval.refine_calls && !eval.hit && eval.initial_rout==0 &&
        !eval.hit_color.version && !std::bit_cast<mb3d_native::U32>(eval.hit_color.rout) &&
        !std::bit_cast<mb3d_native::U32>(eval.hit_color.rold) && !eval.hit_color.reserved &&
        !eval.gradient_range_clamps && !eval.fp32_range_exits && !eval.difs_carried_reads &&
        !fault.code && !fault.formula_id && !fault.slot && !fault.iteration &&
        !fault.work && !fault.limit && !fault.reserved[0] && !fault.reserved[1];
}
}
