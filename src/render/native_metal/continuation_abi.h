#pragma once
// Private, replaceable P2 layout proposal. No renderer entry point uses it yet.
// Tags govern initialized fields; never persist a NativeResources/FormulaParams
// pointer. P5 may replace these scalar types and invalidate the entire layout.
#include "formula_abi.h"
#include "continuation_frames.h"
#include "general_geometry_abi.h"
#include "general_normals_abi.h"
#include "native_camera_abi.h"
#include "native_inside_abi.h"
#include "comparison/metal_exact/scene_abi.h"
namespace mb3d_native { namespace continuation {
enum : U32 { LayoutVersion=5, MaxDepth=14, MaxFrameBytes=16384 };
enum class Tag : U32 { Empty, Ray, Owner, Inside, Primary, Refinement, Normal,
    NormalSample, DistanceEntry, Combined, Distance, OrbitReplay, OrbitPass,
    FormulaStep, Wrapper, Body, Complete, Failed };
struct Header { U32 version,tag,pc,parent_tag,parent_pc; };
enum class FaultOwner : U32 { None, Distance, DirectNormal };
// Observer destinations are owning-frame indices/tags, not saved addresses.
struct ObserverBindings { U32 smoothing,range_exits,carried_reads;float radius_stop; };
struct OwnerFrame { Header header;U32 owner,ordinal,phase,cell,x,y,width,height,record,committed; };
struct InsideFrame { Header header;mb3d_inside::RetryState retry;U32 attempt; };
struct PrimaryFrame {
    Header header;
    float ray[3],start[3],point[3],active[3];
    float axial_multiplier,base_stop,stop_factor,z_div,limiter,depth,stop;
    float count,slope,final_de,axial,de,last_step,last_de,refine_step;
    U32 iteration,program_maximum,combined_maximum,step_index,remaining;
    U32 ended,de_limited;
    I32 inside_word;
    NativeOrbitResult child;
};
struct RefinementFrame {
    Header header;
    float point[3],ray[3],depth,stop,target,step,multiplier,last,last_difference,smooth;
    I32 temporary_max;
    U32 pass,refinement_steps,iteration,first;
    NativeOrbitResult child;
};
struct NormalFrame {
    Header header;
    float center[3],depth,stop,offset,middle,normal[3],plus[3],minus[3],a;
    U32 initial_samples,direct,sample,axis;
    NativeNormalResult result;
};
struct NormalSampleFrame {
    Header header;float point[3],stop,value,smooth;
    U32 direct,want_smooth;
    ObserverBindings previous;
    NativeFormulaFault fault;
    NativeOrbitResult child;
};
struct DistanceEntryFrame {
    Header header;float point[3],stop;
    I32 max_iterations;U32 smooth,temporary_maximum,range_clamps;
    ObserverBindings previous;
    NativeOrbitResult child;
};
struct CombinedFrame {
    Header header;float point[3],stop;
    U32 type,calc_smooth,temporary_maximum;
    I32 max_iterations,second_maximum;
    NativeFormulaState state;
    NativeOrbitResult first,result;
};
struct DistanceFrame {
    Header header;NativeOrbitResult result;
    float point[3],stop,value,radius,center,delta[3],original,sample_coordinate,sample_scale;
    I32 max_iterations,iterations,steps;
    U32 part,calc_smooth,inside_mode,saved_smooth,limit,axis;
    bool numerical,bounded_minimum,retried,representable,radius_certified;
};
struct OrbitReplayFrame {
    Header header;NativeFormulaState entry;
    U32 part,pass_limit,exit_pass,inert,radius_certified;
    float result;
};
struct OrbitPassFrame {
    Header header;
    U32 part,driver,difs,interpolated,four,analytic,slot;
    U32 pass,index,pass_limit,exit_pass,radius_certified;
    bool improved,last_improved,overflow,disabled_exit,inert;
    I32 remaining,minimum_iteration;
    float saved[4],first[4],old_derivative,first_derivative,result;
};
struct FormulaStepFrame { Header header;U32 slot,overflow,fault_owner,child_success; };
struct WrapperFrame {
    Header header;float a[64];
    U32 count,traits,flags,n,c,first_counter_written,local_steps,fault_owner;
    I32 option,first_counter;
    float p[3],result[3],de,vs,saved;
};
// Keep alternatives separately allocated until their lifetime/alias proof is
// complete. This is conservative and remains inside the audited ray envelope.
struct BodyLocals {
    continuation_10274::Frame tess;continuation_10265::Frame pine;
    continuation_10220::Frame columns;continuation_10321::Frame quat;
    continuation_10089::Frame gnarly;
};
struct BodyFrame {Header header;U32 identity;float result[3];BodyLocals locals;};
// Conservative simultaneous allocation, even for mutually exclusive refinement
// and normal branches. Typed child return records intentionally do not alias.
struct alignas(16) RayFrame {
    Header header;
    NativeFormulaState lane;
    NativeGeometryRecord geometry;
    NativeNormalParams normal_params;
    NativeNormalRecord normal_record;
    NativeNormalResult normal_result;
    mb3d_inside::Record inside_record;
    mb3d_camera::Ray camera;
    ExactPixel pixel;
    ObserverBindings bindings;
    OwnerFrame owner;InsideFrame inside;PrimaryFrame primary;
    RefinementFrame refinement;NormalFrame normal;NormalSampleFrame sample;
    DistanceEntryFrame de;CombinedFrame combined;DistanceFrame distance;
    OrbitReplayFrame replay;OrbitPassFrame orbit;FormulaStepFrame step;
    WrapperFrame wrapper;BodyFrame body;
};
static_assert(sizeof(RayFrame)<=MaxFrameBytes,"P2 ray exceeds audited pool envelope");
#ifndef __METAL_VERSION__
static_assert(std::is_standard_layout_v<RayFrame>);
static_assert(std::is_trivially_copyable_v<RayFrame>);
#endif
} }
