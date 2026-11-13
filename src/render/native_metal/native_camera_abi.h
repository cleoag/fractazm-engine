#pragma once
// Task-local cycle6 candidate. Not integrated, compiled or device-qualified.
#ifdef __METAL_VERSION__
#include <metal_stdlib>
namespace mb3d_camera { using U32=uint; using I32=int; }
#else
#include <cstdint>
namespace mb3d_camera { using U32=uint32_t; using I32=int32_t; }
#endif
namespace mb3d_camera {
enum : U32 { Version=1, MaxExtent=4096 };
// Full-frame dimensions control projection; width/height describe allocated
// tile pixels including overlap. Stereo is already folded into origin/tables.
struct Params {
    U32 version,optic,full_width,full_height;
    U32 width,height; I32 left,top;
    // stride_extra: 0 for a dense frame; s-1 when the allocated pixels are every
    // s-th pixel of the full frame, starting at (left, top).
    float primary_planar_z,paint_planar_z; U32 cut_options,stride_extra;
    float origin[4];
    float matrix[3][4]; // original column-major Vgrads, scaled by StepWidth
    float cut[4];
};
// Distinct primary/paint tables: the CPU rounds paint angle arithmetic to
// Single before sin/cos, but primary angles use Double after Single FOV fields.
struct Axis { float angle,sine,cosine,reserved; };
struct Ray {
    U32 version,fault,cut_axis,cut_miss;
    float direction[4],origin[4];
    float cut_depth; U32 reserved[3];
};
struct Paint { float direction[4],y_position; U32 fault,reserved[2]; };
struct Launch { U32 base,count,allocation,reserved; };
// Initial-inside cut surface is a separate explicit test/result contract.
// Production integration may inline this helper into surface packing, without
// allocating another per-pixel buffer or borrowing ExactPixel reserved words.
struct CutSurface { U32 fault,depth; I32 normal[3]; U32 reserved[3]; };
struct DepthPacking { float z_corr,z_mul; U32 reserved[2]; };
}
#ifndef __METAL_VERSION__
static_assert(sizeof(mb3d_camera::Params)==128);
static_assert(sizeof(mb3d_camera::Axis)==16);
static_assert(sizeof(mb3d_camera::Ray)==64);
static_assert(sizeof(mb3d_camera::Paint)==32);
static_assert(sizeof(mb3d_camera::Launch)==16);
static_assert(sizeof(mb3d_camera::CutSurface)==32);
static_assert(sizeof(mb3d_camera::DepthPacking)==16);
#endif
