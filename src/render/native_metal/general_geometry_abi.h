#pragma once
#include "formula_abi.h"
#include "surface_color_abi.h"
namespace mb3d_native {
enum : U32 { kGeneralGeometryVersion=6, kGeneralFormulaKind=4,
    kGeneralCapability=0x4e415431, kGeneralMaxDistanceCalls=1u<<24,
    kGeneralMaxFormulaWork=1u<<28,
    // Reflection descriptor step budget for the point shadow and AO marches of
    // reflected points, which CPU does not cap. Primary and shadow passes keep
    // their own 65536-step descriptors.
    kGeneralMaxSecondarySteps=1u<<22,
    // CalcSR.pas: a secondary ray becomes open air on its 100001st iteration.
    kSecondaryRayStepLimit=100000,
    // Header.bStepsafterDEStop is uint8_t. GetMCT may raise a dIFS value
    // to its2..4 minimum; the effective value remains in this complete domain.
    kGeneralMaxRefinementSteps=255, kGeneralMaxIterations=2000,
    // CPU CalcHardShadowsRows overrides the header count for this pass only.
    kGeneralShadowRefinementSteps=8 };
// RayMarch first-step jitter (header iOptions bit 0, Calc.pas T06). A CPU
// primary task carries one LCG seed through its pixels; a pixel advances it
// once iff its final attempt marches. The probe records that decision, the
// GPU prefix scan turns it into each pixel's entering seed in task order, and the
// seeded pass checks its own advance against the next seed of its task.
// Carried lanes keep a task's running seed after the pixel words.
enum NativePrimaryRandomMode : U32 {
    PrimaryRandomOff=0, PrimaryRandomProbe=1, PrimaryRandomSeeded=2, PrimaryRandomCarried=3 };
struct NativePrimaryRandom { U32 version,mode,workers,reserved; };
enum : U32 { kNativePrimaryRandomVersion=1 };
// PasLcgInitFromThread for logical task t, whose CPU iThreadId is t+1.
inline U32 primary_random_task_seed(U32 task) { return (task+2u)*0x324594A1u+0x24563487u; }
inline U32 primary_random_advance(U32 seed) { return seed*214013u+2531011u; }
inline bool valid_primary_random(NativePrimaryRandom r,U32 height) {
    return r.version==kNativePrimaryRandomVersion&&r.mode<=PrimaryRandomCarried&&!r.reserved&&
        (r.mode==PrimaryRandomOff||(r.workers>=1&&r.workers<=height));
}
inline bool valid_general_primary_limits(U32 refinement_steps,U32 max_iterations) {
    return refinement_steps<=kGeneralMaxRefinementSteps &&
        max_iterations>=1 && max_iterations<=kGeneralMaxIterations;
}
struct NativeGeometryRecord {
    U32 version,fault,ray_steps,de_calls;
    U32 formula_work,normal_calls,refine_calls,hit;
    NativeFormulaFault formula_fault;
    // Initial-inside hits use the initial orbit radius for their interior
    // gradient. ExactPixel's smooth/normal fields retain their original ABI.
    float initial_rout;
    U32 gradient_range_clamps; // successful bounded numerical-DE queries <= de_calls
    // Version 6: FP32 range exits (orbit_driver, and secondary marches that
    // left binary32 after at least one DE query), at most
    // kNativeRangeExitsPerDistance per DE query and one per direct normal orbit.
    U32 fp32_range_exits;
    // dIFS orbits that returned lanes carried from an earlier call (no
    // iteration improved the 65535 entry minimum): one per completed orbit,
    // so two per query of a DE combination.
    U32 difs_carried_reads;
    NativeSmoothingCounters smoothing;
    NativeHitColorPayload hit_color;
};
inline bool valid_fp32_range_exits(U32 exits,U32 de_calls,U32 normal_calls) {
    return de_calls<=kGeneralMaxDistanceCalls && exits<=kNativeRangeExitsPerDistance*de_calls+normal_calls;
}
inline bool valid_difs_carried_reads(U32 reads,U32 de_calls,U32 normal_calls) {
    return de_calls<=kGeneralMaxDistanceCalls && reads<=2*(de_calls+normal_calls);
}
#ifndef __METAL_VERSION__
static_assert(sizeof(NativeGeometryRecord)==112);
#endif
}
