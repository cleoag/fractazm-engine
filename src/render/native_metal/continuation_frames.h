#pragma once
// Generated from the audited raw recipes. Private, replaceable layout.
#include "formula_abi.h"
#define MB3D_P2_SHARED_FRAME_TYPES 1
namespace mb3d_native {
#if defined(MB3D_METAL_NATIVE_FLOAT)
// Compute callers include formula_math.metal.h before this header.
using ContinuationFrameVector=FormulaVec3;
#else
struct ContinuationFrameVector { float x,y,z; };
#endif
static_assert(sizeof(ContinuationFrameVector)==12,"continuation vector layout");
namespace continuation_10274 { struct Frame { U32 pc; float v[3];
I32 maxDepth;
float axiom;
float threshold;
I32 count; U32 work_steps; ContinuationFrameVector result; }; }
namespace continuation_10265 { struct Frame { U32 pc; float Scale;
float xOff;
float pixel_x;
float pixel_y;
float pixel_z;
float yz_angle;
float Sides;
float s23;
float s13;
float s12;
float pi_const;
float Cx;
float Cy;
float Cz;
float x;
float y;
float z;
U32 T1_Count;
bool rotate;
bool polygon; U32 i; float half_angle,double_half,theta; ContinuationFrameVector result; }; }
namespace continuation_10220 { struct Frame { U32 pc; I32 apply;
ContinuationFrameVector local;
float d1;
float d2;
U32 count; U32 i; ContinuationFrameVector result; }; }
namespace continuation_10321 { struct Frame { U32 pc; I32 pvals[8];
float terms[8];
float results[4]; U32 i; ContinuationFrameVector result; }; }
namespace continuation_10089 { struct Frame { U32 pc; float Beta;
float Scale;
float Amplitude;
float Alpha;
float Step;
float Gamma;
float OTrapClrMul;
float x;
float y;
float z;
float yy;
float xx;
float ref_axis;
I32 Apply_scale_add;
I32 Flip_XZ;
I32 iter_count; U32 i; ContinuationFrameVector result; }; }
} // namespace mb3d_native
