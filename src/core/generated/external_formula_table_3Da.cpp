// external_formula_table_3Da.cpp — Auto-generated bulk formula registrations
// Category: 3Da (44 formulas)
//
// DO NOT EDIT — regenerate with: python3 scripts/gen_bulk_formula_table.py

#include "core/external_formulas.h"
#include "core/lightmap.h"
#include <cmath>
#include <cstring>
#include <cstdint>
#include <cfloat>

#define constant

using std::abs;
using std::sqrt;
using std::sin;
using std::cos;
using std::atan2;
using std::asin;
using std::acos;
using std::log;
using std::exp;
using std::pow;
using std::sinh;
using std::cosh;
using std::tanh;
using std::max;
using std::min;
using std::rint;

namespace mb3d {

// FormulaInfo struct (used by .inc metadata blocks)
struct FormulaInfo {
    unsigned int id;
    const char* name;
    int param_count;
    int complexity;
    bool uses_transcendental;
};

// ============================================================================
// Compatibility types and operators for .inc formula files
// ============================================================================

// cpu_double2 — packed double pair (GenIntPower, KaliLinCombSSE2sm, etc.)
struct cpu_double2 {
    double x, y;
    cpu_double2() : x(0.0), y(0.0) {}
    cpu_double2(double v) : x(v), y(v) {}
    cpu_double2(double x_, double y_) : x(x_), y(y_) {}
    cpu_double2 operator+(const cpu_double2& b) const { return {x + b.x, y + b.y}; }
    cpu_double2 operator-(const cpu_double2& b) const { return {x - b.x, y - b.y}; }
    cpu_double2 operator*(const cpu_double2& b) const { return {x * b.x, y * b.y}; }
    cpu_double2 operator*(double s) const { return {x * s, y * s}; }
    cpu_double2 operator/(double s) const { return {x / s, y / s}; }
    cpu_double2& operator+=(const cpu_double2& b) { x += b.x; y += b.y; return *this; }
    cpu_double2& operator-=(const cpu_double2& b) { x -= b.x; y -= b.y; return *this; }
    cpu_double2& operator*=(double s) { x *= s; y *= s; return *this; }
    cpu_double2& operator/=(double s) { x /= s; y /= s; return *this; }
};
inline cpu_double2 operator*(double s, const cpu_double2& v) { return {s * v.x, s * v.y}; }

// cpu_double3 operators (the struct is in external_formulas.h but lacks operators)
inline cpu_double3 operator+(const cpu_double3& a, const cpu_double3& b) { return {a.x + b.x, a.y + b.y, a.z + b.z}; }
inline cpu_double3 operator-(const cpu_double3& a, const cpu_double3& b) { return {a.x - b.x, a.y - b.y, a.z - b.z}; }
inline cpu_double3 operator*(const cpu_double3& a, double s) { return {a.x * s, a.y * s, a.z * s}; }
inline cpu_double3 operator*(double s, const cpu_double3& a) { return {s * a.x, s * a.y, s * a.z}; }
inline cpu_double3 operator/(const cpu_double3& a, double s) { return {a.x / s, a.y / s, a.z / s}; }
inline cpu_double3 operator-(const cpu_double3& a) { return {-a.x, -a.y, -a.z}; }

// length functions
inline double length(cpu_double2 v) { return std::sqrt(v.x * v.x + v.y * v.y); }
inline double length(cpu_double3 v) { return std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z); }

// rsqrt — reciprocal square root
inline double rsqrt(double v) { return 1.0 / std::sqrt(v); }

// M_PI_2_F — pi/2 as double constant (used by General.inc, _SinApprox.inc)
#ifndef M_PI_2_F
#define M_PI_2_F 1.5707963267948966
#endif

// sincos_of_power_theta — compute sin/cos of (power * atan2(num, den))
// Used by General.inc. Full angle-doubling approach omitted for bulk gen.
inline void sincos_of_power_theta(double num, double den, double power,
                                   double& sin_out, double& cos_out) {
    double angle = std::atan2(num, den) * power;
    sin_out = std::sin(angle);
    cos_out = std::cos(angle);
}

// cpu_double4 — packed 4 doubles (used by 4Da Sierpinski4ex, MixPinski4ex, etc.)
struct cpu_double4 {
    double x, y, z, w;
    cpu_double4() : x(0), y(0), z(0), w(0) {}
    cpu_double4(double v) : x(v), y(v), z(v), w(v) {}
    cpu_double4(double x_, double y_, double z_, double w_) : x(x_), y(y_), z(z_), w(w_) {}
    cpu_double4 operator+(const cpu_double4& b) const { return {x+b.x, y+b.y, z+b.z, w+b.w}; }
    cpu_double4 operator-(const cpu_double4& b) const { return {x-b.x, y-b.y, z-b.z, w-b.w}; }
    cpu_double4 operator*(double s) const { return {x*s, y*s, z*s, w*s}; }
    cpu_double4 operator/(double s) const { return {x/s, y/s, z/s, w/s}; }
    cpu_double4 operator-() const { return {-x, -y, -z, -w}; }
};
inline cpu_double4 operator*(double s, const cpu_double4& v) { return {s*v.x, s*v.y, s*v.z, s*v.w}; }
inline double dot(cpu_double4 a, cpu_double4 b) { return a.x*b.x + a.y*b.y + a.z*b.z + a.w*b.w; }
inline double length(cpu_double4 v) { return std::sqrt(dot(v, v)); }

// RotMatrix4 * cpu_double4 — 4x4 matrix-vector multiply
inline cpu_double4 operator*(const RotMatrix4& M, const cpu_double4& v) {
    return cpu_double4(
        M.m[0][0]*v.x + M.m[0][1]*v.y + M.m[0][2]*v.z + M.m[0][3]*v.w,
        M.m[1][0]*v.x + M.m[1][1]*v.y + M.m[1][2]*v.z + M.m[1][3]*v.w,
        M.m[2][0]*v.x + M.m[2][1]*v.y + M.m[2][2]*v.z + M.m[2][3]*v.w,
        M.m[3][0]*v.x + M.m[3][1]*v.y + M.m[3][2]*v.z + M.m[3][3]*v.w);
}

// cpu_bitcast_to_int — reinterpret float bits as int
inline int cpu_bitcast_to_int(double v) {
    float f = static_cast<float>(v);
    int32_t i; std::memcpy(&i, &f, 4); return i;
}

// CPUVec3 — alias for height map sampling results
using CPUVec3 = cpu_double3;

// Height map sampling helpers backed by the shared Pascal-style lightmap runtime.
inline CPUVec3 cpu_map_direct_sample(double u, double v, int map_nr) {
    TVec3D in = {u, v, 0.0};
    TVec3D out = {};
    GetMapPixelDirectXYspline(out, &in, map_nr);
    return CPUVec3(out[0], out[1], out[2]);
}
inline CPUVec3 cpu_map_sphere_sample(double x, double y, double z, int map_nr) {
    TVec3D in = {x, y, z};
    TVec3D out = {};
    GetMapPixelSphereSpline(out, &in, map_nr);
    return CPUVec3(out[0], out[1], out[2]);
}

// RotMatrix3Ext — extends RotMatrix3 with .col0/.col1/.col2 column access
// Many .inc files use RM.col0.x, RM.col1.y, etc. (column-major access).
// RotMatrix3.m is row-major: m[row][col]. Column j = {m[0][j], m[1][j], m[2][j]}.
struct RMCol { double x, y, z; };
struct RotMatrix3Ext : RotMatrix3 {
    RMCol col0, col1, col2;
    RotMatrix3Ext() : RotMatrix3{}, col0{}, col1{}, col2{} {}
    RotMatrix3Ext(const RotMatrix3& r) : RotMatrix3(r),
        col0{r.m[0][0], r.m[1][0], r.m[2][0]},
        col1{r.m[0][1], r.m[1][1], r.m[2][1]},
        col2{r.m[0][2], r.m[1][2], r.m[2][2]} {}
};
// Shadow buildRotMatrix3/rotMatrix3FromFloats to return RotMatrix3Ext
inline RotMatrix3Ext buildRotMatrix3Ext(double ax, double ay, double az) {
    return RotMatrix3Ext(buildRotMatrix3(ax, ay, az));
}
#define buildRotMatrix3 buildRotMatrix3Ext
inline RotMatrix3Ext rotMatrix3FromFloatsExt(const double* p, int i) {
    RotMatrix3 r = {}; r.m[0][0]=p[i]; r.m[0][1]=p[i+1]; r.m[0][2]=p[i+2];
    r.m[1][0]=p[i+3]; r.m[1][1]=p[i+4]; r.m[1][2]=p[i+5];
    r.m[2][0]=p[i+6]; r.m[2][1]=p[i+7]; r.m[2][2]=p[i+8];
    return RotMatrix3Ext(r);
}
#define rotMatrix3FromFloats rotMatrix3FromFloatsExt
// Redefine RotMatrix3 so .inc files that declare "RotMatrix3 Rot" get the Ext version
#define RotMatrix3 RotMatrix3Ext

// Undefine system macros that conflict with .inc variable names
#ifdef OVERFLOW
#undef OVERFLOW
#endif

// sign() function (returns -1, 0, or 1)
inline double sign(double v) { return (v > 0.0) ? 1.0 : ((v < 0.0) ? -1.0 : 0.0); }

// Integer clamp to avoid ambiguity with std::clamp in C++17
inline int clamp(int v, int lo, int hi) { return std::max(lo, std::min(v, hi)); }

// uint typedef (used by _RotateRnd.inc)
typedef unsigned int uint;

// dot product for RMCol (column of RotMatrix3Ext)
inline double dot(RMCol a, cpu_double3 b) { return a.x*b.x + a.y*b.y + a.z*b.z; }
inline double dot(cpu_double3 a, cpu_double3 b) { return a.x*b.x + a.y*b.y + a.z*b.z; }

// sincos shim — returns sin(angle) and writes cos(angle) to second arg
// Matches .inc usage: sin_val = sincos(angle, cos_val)
inline double sincos(double angle, double& cos_out) {
    cos_out = std::cos(angle);
    return std::sin(angle);
}

// Alias for formulas that call buildRotMatrix4d instead of buildRotMatrix4
inline RotMatrix4 buildRotMatrix4d(double a1, double a2, double a3,
                                    double a4, double a5, double a6) {
    return buildRotMatrix4(a1, a2, a3, a4, a5, a6);
}

// ============================================================================
// Include 3Da formula .inc files
// ============================================================================

#include "3Da/ABoxMapSFold.inc"
#include "3Da/ABoxMod2.inc"
#include "3Da/ABoxModKali.inc"
#include "3Da/ABoxPlatinum.inc"
#include "3Da/ABoxPlatinumB.inc"
#include "3Da/ABoxSmoothFold.inc"
#include "3Da/ABoxVSShapes.inc"
#include "3Da/ABoxVSWavy.inc"
#include "3Da/ABoxVS_icen1.inc"
#include "3Da/ABoxVS_icen2.inc"
#include "3Da/ABoxVaryScale2.inc"
#include "3Da/ASurfMod1.inc"
#include "3Da/ASurfSmoothFold.inc"
#include "3Da/ATetraVS.inc"
#include "3Da/ATetraVS2.inc"
#include "3Da/AmazingBox2.inc"
#include "3Da/Amazing_Surf.inc"
#include "3Da/Amazing_Surf_2.inc"
#include "3Da/CantorIFS.inc"
#include "3Da/CrossBiFold.inc"
#include "3Da/DodecahedronIFS.inc"
#include "3Da/DodecahedronIFSex.inc"
#include "3Da/HalfOctIFS.inc"
#include "3Da/IcosahedronIFS.inc"
#include "3Da/JCube3.inc"
#include "3Da/Mandalay-KIFS.inc"
#include "3Da/Mandalex.inc"
#include "3Da/Menger3.inc"
#include "3Da/MengerHyper.inc"
#include "3Da/MengerIFSplus.inc"
#include "3Da/MengerKoch.inc"
#include "3Da/MengerKochV2.inc"
#include "3Da/MengerSmt.inc"
#include "3Da/OctKoch.inc"
#include "3Da/OctaFullIFS.inc"
#include "3Da/OctahedronIFS.inc"
#include "3Da/SierpHilbert.inc"
#include "3Da/Sierpinski3alt.inc"
#include "3Da/SierpinskiFull.inc"
#include "3Da/_Scaling.inc"
#include "3Da/genIFS.inc"
#include "3Da/koch_cube.inc"
#include "3Da/koch_oct.inc"
#include "3Da/koch_surf.inc"

// ============================================================================
// PVar offset maps (3Da)
// ============================================================================

static const FormulaParamMap map_ABOXMAPSFOLD = {
    7, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 },
        { -0x2C, 2 }, { -0x30, 1 }, { -0x34, 1 }
    }
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA_ABOXMAPSFOLD, 11, map_ABOXMAPSFOLD)

static const FormulaParamMap map_ABOXMOD2 = {
    6, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 },
        { -0x30, 0 }, { -0x38, 0 }
    }
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA_ABOXMOD2, 2, map_ABOXMOD2)

static const FormulaParamMap map_ABOXMODKALI = {
    10, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x30, 0 },
        { -0x38, 0 }, { -0x40, 0 }, { 0x00, 3 }, { 0x08, 3 },
        { 0x10, 0 }, { 0x18, 0 }
    }
};
static const FormulaConstEntry consts_ABOXMODKALI[] = {
    { 3, UINT64_C(0x7FFFFFFFFFFFFFFF) },
    { 3, UINT64_C(0x7FFFFFFFFFFFFFFF) },
    { 0, UINT64_C(0x3FF0000000000000) },
    { 0, UINT64_C(0x3FF0000000000000) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER_EX(FORMULA_ABOXMODKALI, 2, map_ABOXMODKALI, 0, 0)

static const FormulaParamMap map_ABOXPLATINUM = {
    26, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 },
        { -0x2C, 1 }, { -0x30, 1 }, { -0x34, 1 }, { -0x38, 1 },
        { -0x3C, 1 }, { -0x40, 1 }, { -0x44, 1 }, { -0x48, 1 },
        { -0x4C, 1 }, { -0x54, 0 }, { -0x5C, 0 }, { -0x64, 0 },
        { -0x6C, 0 }, { -0x74, 0 }, { -0x7C, 0 }, { -0x84, 0 },
        { -0x8C, 0 }, { -0x94, 0 }, { -0x9C, 0 }, { -0xA4, 0 },
        { -0xAC, 0 }, { -0xB0, 2 }
    }
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA_ABOXPLATINUM, 2, map_ABOXPLATINUM)

static const FormulaParamMap map_ABOXPLATINUMB = {
    26, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 },
        { -0x2C, 1 }, { -0x30, 1 }, { -0x34, 1 }, { -0x38, 1 },
        { -0x3C, 1 }, { -0x40, 1 }, { -0x44, 1 }, { -0x48, 1 },
        { -0x4C, 1 }, { -0x54, 0 }, { -0x5C, 0 }, { -0x64, 0 },
        { -0x6C, 0 }, { -0x74, 0 }, { -0x7C, 0 }, { -0x84, 0 },
        { -0x8C, 0 }, { -0x94, 0 }, { -0x9C, 0 }, { -0xA4, 0 },
        { -0xAC, 0 }, { -0xB0, 2 }
    }
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA_ABOXPLATINUMB, 2, map_ABOXPLATINUMB)

static const FormulaParamMap map_ABOXSMOOTHFOLD = {
    10, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 },
        { -0x30, 0 }, { -0x34, 2 }, { -0x3C, 0 }, { -0x40, 2 },
        { -0x48, 0 }, { 0x00, 0 }
    }
};
static const FormulaConstEntry consts_ABOXSMOOTHFOLD[] = {
    { 0, UINT64_C(0x3FEFAE147AE147AE) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA_ABOXSMOOTHFOLD, 11, map_ABOXSMOOTHFOLD)

static const FormulaParamMap map_ABOXVSSHAPES = {
    11, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 },
        { -0x30, 0 }, { -0x34, 2 }, { -0x3C, 0 }, { -0x44, 0 },
        { 0x00, 0 }, { 0x08, 0 }, { 0x10, 0 }
    }
};
static const FormulaConstEntry consts_ABOXVSSHAPES[] = {
    { 0, UINT64_C(0x4008000000000000) },
    { 0, UINT64_C(0x3FB999999999999A) },
    { 0, UINT64_C(0x3FC3333333333333) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA_ABOXVSSHAPES, 11, map_ABOXVSSHAPES)

static const FormulaParamMap map_ABOXVSWAVY = {
    10, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 },
        { -0x30, 0 }, { -0x38, 0 }, { -0x40, 0 }, { -0x48, 0 },
        { -0x50, 0 }, { -0x54, 2 }
    }
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA_ABOXVSWAVY, 11, map_ABOXVSWAVY)

static const FormulaParamMap map_ABOXVS_ICEN1 = {
    9, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 },
        { -0x30, 0 }, { -0x38, 0 }, { -0x40, 0 }, { -0x48, 0 },
        { -0x4C, 2 }
    }
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA_ABOXVS_ICEN1, 11, map_ABOXVS_ICEN1)

static const FormulaParamMap map_ABOXVS_ICEN2 = {
    11, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 },
        { -0x30, 0 }, { -0x38, 0 }, { -0x40, 0 }, { -0x48, 0 },
        { -0x50, 0 }, { -0x54, 2 }, { -0x58, 0 }
    }
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA_ABOXVS_ICEN2, 11, map_ABOXVS_ICEN2)

static const FormulaParamMap map_ABOXVARYSCALE2 = {
    7, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 },
        { -0x30, 0 }, { -0x38, 0 }, { -0x40, 0 }
    }
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA_ABOXVARYSCALE2, 11, map_ABOXVARYSCALE2)

static const FormulaParamMap map_ASURFMOD1 = {
    17, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 },
        { -0x30, 0 }, { -0x38, 0 }, { -0x40, 0 }, { -0x48, 0 },
        { -0x4C, 1 }, { -0x50, 1 }, { -0x54, 1 }, { -0x58, 1 },
        { -0x5C, 1 }, { -0x60, 1 }, { -0x64, 1 }, { -0x68, 1 },
        { -0x6C, 1 }
    }
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA_ASURFMOD1, 2, map_ASURFMOD1)

static const FormulaParamMap map_ASURFSMOOTHFOLD = {
    19, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 },
        { -0x30, 0 }, { -0x34, 2 }, { -0x3C, 0 }, { -0x40, 2 },
        { -0x48, 0 }, { -0x4C, 1 }, { -0x50, 1 }, { -0x54, 1 },
        { -0x58, 1 }, { -0x5C, 1 }, { -0x60, 1 }, { -0x64, 1 },
        { -0x68, 1 }, { -0x6C, 1 }, { 0x00, 0 }
    }
};
static const FormulaConstEntry consts_ASURFSMOOTHFOLD[] = {
    { 0, UINT64_C(0x3FEFAE147AE147AE) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER_EX(FORMULA_ASURFSMOOTHFOLD, 11, map_ASURFSMOOTHFOLD, 0, 0)

static const FormulaParamMap map_ATETRAVS = {
    16, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 },
        { -0x30, 0 }, { -0x34, 1 }, { -0x38, 1 }, { -0x3C, 1 },
        { -0x40, 1 }, { -0x44, 1 }, { -0x48, 1 }, { -0x4C, 1 },
        { -0x50, 1 }, { -0x54, 1 }, { -0x58, 2 }, { -0x5C, 2 }
    }
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA_ATETRAVS, 11, map_ATETRAVS)

static const FormulaParamMap map_ATETRAVS2 = {
    7, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 },
        { -0x30, 0 }, { -0x38, 0 }, { 0x00, 0 }
    }
};
static const FormulaConstEntry consts_ATETRAVS2[] = {
    { 0, UINT64_C(0xBFE6A09E667F3BCD) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER_EX(FORMULA_ATETRAVS2, 11, map_ATETRAVS2, 0, 0)

static const FormulaParamMap map_AMAZINGBOX2 = {
    21, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 },
        { -0x2C, 1 }, { -0x30, 1 }, { -0x34, 1 }, { -0x38, 1 },
        { -0x3C, 1 }, { -0x40, 1 }, { -0x44, 1 }, { -0x48, 1 },
        { -0x4C, 1 }, { -0x54, 0 }, { -0x5C, 0 }, { -0x64, 0 },
        { -0x6C, 0 }, { -0x74, 0 }, { -0x7C, 0 }, { -0x84, 0 },
        { -0x8C, 0 }
    }
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA_AMAZINGBOX2, 2, map_AMAZINGBOX2)

static const FormulaParamMap map_AMAZING_SURF = {
    15, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 },
        { -0x2C, 1 }, { -0x30, 1 }, { -0x34, 1 }, { -0x38, 1 },
        { -0x3C, 1 }, { -0x40, 1 }, { -0x44, 1 }, { -0x48, 1 },
        { -0x4C, 1 }, { -0x54, 0 }, { -0x58, 2 }
    }
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA_AMAZING_SURF, 11, map_AMAZING_SURF)

static const FormulaParamMap map_AMAZING_SURF_2 = {
    19, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 },
        { -0x2C, 1 }, { -0x30, 1 }, { -0x34, 1 }, { -0x38, 1 },
        { -0x3C, 1 }, { -0x40, 1 }, { -0x44, 1 }, { -0x48, 1 },
        { -0x4C, 1 }, { -0x50, 2 }, { 0x00, 0 }, { 0x08, 0 },
        { 0x10, 0 }, { 0x18, 0 }, { 0x20, 0 }
    }
};
static const FormulaConstEntry consts_AMAZING_SURF_2[] = {
    { 0, UINT64_C(0xBFF8000000000000) },
    { 0, UINT64_C(0x3FE0000000000000) },
    { 0, UINT64_C(0x4000000000000000) },
    { 0, UINT64_C(0xC000000000000000) },
    { 0, UINT64_C(0xBFF8000000000000) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA_AMAZING_SURF_2, 2, map_AMAZING_SURF_2)

static const FormulaParamMap map_CANTORIFS = {
    22, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 },
        { -0x2C, 1 }, { -0x30, 1 }, { -0x34, 1 }, { -0x38, 1 },
        { -0x3C, 1 }, { -0x40, 1 }, { -0x44, 1 }, { -0x48, 1 },
        { -0x4C, 1 }, { -0x50, 1 }, { -0x54, 1 }, { -0x58, 1 },
        { -0x5C, 1 }, { -0x60, 1 }, { -0x64, 1 }, { -0x68, 1 },
        { -0x6C, 1 }, { -0x70, 1 }
    }
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA_CANTORIFS, 2, map_CANTORIFS)

static const FormulaParamMap map_CROSSBIFOLD = {
    24, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 },
        { -0x2C, 1 }, { -0x30, 1 }, { -0x34, 1 }, { -0x38, 1 },
        { -0x3C, 1 }, { -0x40, 1 }, { -0x44, 1 }, { -0x48, 1 },
        { -0x4C, 1 }, { -0x50, 1 }, { -0x54, 1 }, { -0x58, 1 },
        { -0x5C, 1 }, { -0x60, 1 }, { -0x64, 1 }, { -0x68, 1 },
        { -0x6C, 1 }, { -0x70, 1 }, { -0x78, 0 }, { -0x80, 0 }
    }
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA_CROSSBIFOLD, 2, map_CROSSBIFOLD)

static const FormulaParamMap map_DODECAHEDRONIFS = {
    25, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 },
        { -0x2C, 1 }, { -0x30, 1 }, { -0x34, 1 }, { -0x38, 1 },
        { -0x3C, 1 }, { -0x40, 1 }, { -0x44, 1 }, { -0x48, 1 },
        { -0x4C, 1 }, { -0x50, 1 }, { -0x54, 1 }, { -0x58, 1 },
        { -0x5C, 1 }, { -0x60, 1 }, { -0x64, 1 }, { -0x68, 1 },
        { -0x6C, 1 }, { -0x70, 1 }, { 0x00, 0 }, { 0x08, 0 },
        { 0x10, 0 }
    }
};
static const FormulaConstEntry consts_DODECAHEDRONIFS[] = {
    { 0, UINT64_C(0x3FD3C6EF372FE948) },
    { 0, UINT64_C(0xBFE0000000000000) },
    { 0, UINT64_C(0xBFE9E3779B97F4A4) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA_DODECAHEDRONIFS, 2, map_DODECAHEDRONIFS)

static const FormulaParamMap map_DODECAHEDRONIFS_EX = {
    24, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 },
        { -0x2C, 1 }, { -0x30, 1 }, { -0x34, 1 }, { -0x38, 1 },
        { -0x3C, 1 }, { -0x40, 1 }, { -0x44, 1 }, { -0x48, 1 },
        { -0x4C, 1 }, { -0x50, 1 }, { -0x54, 1 }, { -0x58, 1 },
        { -0x5C, 1 }, { -0x60, 1 }, { -0x64, 1 }, { -0x68, 1 },
        { -0x6C, 1 }, { -0x70, 1 }, { -0x78, 0 }, { -0x7C, 2 }
    }
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA_DODECAHEDRONIFS_EX, 2, map_DODECAHEDRONIFS_EX)

static const FormulaParamMap map_HALFOCTIFS = {
    22, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 },
        { -0x2C, 1 }, { -0x30, 1 }, { -0x34, 1 }, { -0x38, 1 },
        { -0x3C, 1 }, { -0x40, 1 }, { -0x44, 1 }, { -0x48, 1 },
        { -0x4C, 1 }, { -0x50, 1 }, { -0x54, 1 }, { -0x58, 1 },
        { -0x5C, 1 }, { -0x60, 1 }, { -0x64, 1 }, { -0x68, 1 },
        { -0x6C, 1 }, { -0x70, 1 }
    }
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA_HALFOCTIFS, 2, map_HALFOCTIFS)

static const FormulaParamMap map_ICOSAHEDRONIFS = {
    25, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 },
        { -0x2C, 1 }, { -0x30, 1 }, { -0x34, 1 }, { -0x38, 1 },
        { -0x3C, 1 }, { -0x40, 1 }, { -0x44, 1 }, { -0x48, 1 },
        { -0x4C, 1 }, { -0x50, 1 }, { -0x54, 1 }, { -0x58, 1 },
        { -0x5C, 1 }, { -0x60, 1 }, { -0x64, 1 }, { -0x68, 1 },
        { -0x6C, 1 }, { -0x70, 1 }, { 0x00, 0 }, { 0x08, 0 },
        { 0x10, 0 }
    }
};
static const FormulaConstEntry consts_ICOSAHEDRONIFS[] = {
    { 0, UINT64_C(0x3FD3C6EF372FE948) },
    { 0, UINT64_C(0xBFE0000000000000) },
    { 0, UINT64_C(0xBFE9E3779B97F4A4) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA_ICOSAHEDRONIFS, 2, map_ICOSAHEDRONIFS)

static const FormulaParamMap map_JCUBE3 = {
    9, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 },
        { -0x30, 0 }, { -0x38, 0 }, { -0x40, 0 }, { -0x48, 0 },
        { -0x50, 0 }
    }
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA_JCUBE3, 2, map_JCUBE3)

static const FormulaParamMap map_MANDALAY_KIFS = {
    16, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 },
        { -0x2C, 1 }, { -0x30, 1 }, { -0x34, 1 }, { -0x38, 1 },
        { -0x3C, 1 }, { -0x40, 1 }, { -0x44, 1 }, { -0x48, 1 },
        { -0x4C, 1 }, { -0x54, 0 }, { -0x5C, 0 }, { -0x64, 0 }
    }
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA_MANDALAY_KIFS, 2, map_MANDALAY_KIFS)

static const FormulaParamMap map_MANDALEX = {
    17, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 },
        { -0x2C, 1 }, { -0x30, 1 }, { -0x34, 1 }, { -0x38, 1 },
        { -0x3C, 1 }, { -0x40, 1 }, { -0x44, 1 }, { -0x48, 1 },
        { -0x4C, 1 }, { -0x54, 0 }, { -0x5C, 0 }, { -0x64, 0 },
        { -0x68, 2 }
    }
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA_MANDALEX, 2, map_MANDALEX)

static const FormulaParamMap map_MENGER3 = {
    22, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 },
        { -0x2C, 1 }, { -0x30, 1 }, { -0x34, 1 }, { -0x38, 1 },
        { -0x3C, 1 }, { -0x40, 1 }, { -0x44, 1 }, { -0x48, 1 },
        { -0x4C, 1 }, { -0x50, 1 }, { -0x54, 1 }, { -0x58, 1 },
        { -0x5C, 1 }, { -0x60, 1 }, { -0x64, 1 }, { -0x68, 1 },
        { -0x6C, 1 }, { -0x70, 1 }
    }
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA_MENGER3, 2, map_MENGER3)

static const FormulaParamMap map_MENGERHYPER = {
    11, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 },
        { -0x30, 0 }, { -0x38, 0 }, { -0x40, 0 }, { -0x48, 0 },
        { -0x50, 0 }, { -0x58, 0 }, { -0x5C, 2 }
    }
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA_MENGERHYPER, 2, map_MENGERHYPER)

static const FormulaParamMap map_MENGERIFS_PLUS = {
    16, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 },
        { -0x2C, 1 }, { -0x30, 1 }, { -0x34, 1 }, { -0x38, 1 },
        { -0x3C, 1 }, { -0x40, 1 }, { -0x44, 1 }, { -0x48, 1 },
        { -0x4C, 1 }, { -0x54, 0 }, { -0x5C, 0 }, { -0x64, 0 }
    }
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA_MENGERIFS_PLUS, 2, map_MENGERIFS_PLUS)

static const FormulaParamMap map_MENGERKOCH = {
    24, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 },
        { -0x2C, 1 }, { -0x30, 1 }, { -0x34, 1 }, { -0x38, 1 },
        { -0x3C, 1 }, { -0x40, 1 }, { -0x44, 1 }, { -0x48, 1 },
        { -0x4C, 1 }, { -0x50, 1 }, { -0x54, 1 }, { -0x58, 1 },
        { -0x5C, 1 }, { -0x60, 1 }, { -0x64, 1 }, { -0x68, 1 },
        { -0x6C, 1 }, { -0x70, 1 }, { -0x78, 0 }, { -0x80, 0 }
    }
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA_MENGERKOCH, 2, map_MENGERKOCH)

static const FormulaParamMap map_MENGERKOCHV2 = {
    25, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 },
        { -0x2C, 1 }, { -0x30, 1 }, { -0x34, 1 }, { -0x38, 1 },
        { -0x3C, 1 }, { -0x40, 1 }, { -0x44, 1 }, { -0x48, 1 },
        { -0x4C, 1 }, { -0x50, 1 }, { -0x54, 1 }, { -0x58, 1 },
        { -0x5C, 1 }, { -0x60, 1 }, { -0x64, 1 }, { -0x68, 1 },
        { -0x6C, 1 }, { -0x70, 1 }, { -0x78, 0 }, { -0x80, 0 },
        { -0x08, 0 }
    }
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA_MENGERKOCHV2, 2, map_MENGERKOCHV2)

static const FormulaParamMap map_MENGERSMT = {
    16, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 },
        { -0x2C, 1 }, { -0x30, 1 }, { -0x34, 1 }, { -0x38, 1 },
        { -0x3C, 1 }, { -0x40, 1 }, { -0x44, 1 }, { -0x48, 1 },
        { -0x4C, 1 }, { -0x54, 0 }, { -0x5C, 0 }, { 0x00, 0 }
    }
};
static const FormulaConstEntry consts_MENGERSMT[] = {
    { 0, UINT64_C(0x3FE0000000000000) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA_MENGERSMT, 2, map_MENGERSMT)

static const FormulaParamMap map_OCTKOCH = {
    25, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 },
        { -0x2C, 1 }, { -0x30, 1 }, { -0x34, 1 }, { -0x38, 1 },
        { -0x3C, 1 }, { -0x40, 1 }, { -0x44, 1 }, { -0x48, 1 },
        { -0x4C, 1 }, { -0x50, 1 }, { -0x54, 1 }, { -0x58, 1 },
        { -0x5C, 1 }, { -0x60, 1 }, { -0x64, 1 }, { -0x68, 1 },
        { -0x6C, 1 }, { -0x70, 1 }, { -0x78, 0 }, { -0x80, 0 },
        { -0x84, 2 }
    }
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA_OCTKOCH, 2, map_OCTKOCH)

static const FormulaParamMap map_OCTAFULLIFS = {
    22, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 },
        { -0x2C, 1 }, { -0x30, 1 }, { -0x34, 1 }, { -0x38, 1 },
        { -0x3C, 1 }, { -0x40, 1 }, { -0x44, 1 }, { -0x48, 1 },
        { -0x4C, 1 }, { -0x50, 1 }, { -0x54, 1 }, { -0x58, 1 },
        { -0x5C, 1 }, { -0x60, 1 }, { -0x64, 1 }, { -0x68, 1 },
        { -0x6C, 1 }, { -0x70, 1 }
    }
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA_OCTAFULLIFS, 2, map_OCTAFULLIFS)

static const FormulaParamMap map_OCTAHEDRONIFS = {
    22, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 },
        { -0x2C, 1 }, { -0x30, 1 }, { -0x34, 1 }, { -0x38, 1 },
        { -0x3C, 1 }, { -0x40, 1 }, { -0x44, 1 }, { -0x48, 1 },
        { -0x4C, 1 }, { -0x50, 1 }, { -0x54, 1 }, { -0x58, 1 },
        { -0x5C, 1 }, { -0x60, 1 }, { -0x64, 1 }, { -0x68, 1 },
        { -0x6C, 1 }, { -0x70, 1 }
    }
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA_OCTAHEDRONIFS, 2, map_OCTAHEDRONIFS)

static const FormulaParamMap map_SIERPHILBERT = {
    24, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 },
        { -0x2C, 1 }, { -0x30, 1 }, { -0x34, 1 }, { -0x38, 1 },
        { -0x3C, 1 }, { -0x40, 1 }, { -0x44, 1 }, { -0x48, 1 },
        { -0x4C, 1 }, { -0x50, 1 }, { -0x54, 1 }, { -0x58, 1 },
        { -0x5C, 1 }, { -0x60, 1 }, { -0x64, 1 }, { -0x68, 1 },
        { -0x6C, 1 }, { -0x70, 1 }, { -0x78, 0 }, { -0x80, 0 }
    }
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA_SIERPHILBERT, 2, map_SIERPHILBERT)

static const FormulaParamMap map_SIERPINSKI3ALT = {
    16, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 },
        { -0x30, 0 }, { -0x38, 0 }, { -0x40, 0 }, { -0x44, 1 },
        { -0x48, 1 }, { -0x4C, 1 }, { -0x50, 1 }, { -0x54, 1 },
        { -0x58, 1 }, { -0x5C, 1 }, { -0x60, 1 }, { -0x64, 1 }
    }
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA_SIERPINSKI3ALT, 2, map_SIERPINSKI3ALT)

static const FormulaParamMap map_SIERPINSKIFULL = {
    25, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 },
        { -0x2C, 1 }, { -0x30, 1 }, { -0x34, 1 }, { -0x38, 1 },
        { -0x3C, 1 }, { -0x40, 1 }, { -0x44, 1 }, { -0x48, 1 },
        { -0x4C, 1 }, { -0x50, 1 }, { -0x54, 1 }, { -0x58, 1 },
        { -0x5C, 1 }, { -0x60, 1 }, { -0x64, 1 }, { -0x68, 1 },
        { -0x6C, 1 }, { -0x70, 1 }, { -0x78, 0 }, { -0x80, 0 },
        { -0x84, 2 }
    }
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA_SIERPINSKIFULL, 2, map_SIERPINSKIFULL)

static const FormulaParamMap map__SCALING = { 1, { { -0x10, 0 } } };
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__SCALING, 2, map__SCALING)

static const FormulaParamMap map_GENIFS = {
    16, {
        { -0x10, 0 }, { -0x14, 2 }, { -0x18, 2 }, { -0x1C, 2 },
        { -0x20, 1 }, { -0x24, 1 }, { -0x28, 1 }, { -0x2C, 1 },
        { -0x30, 1 }, { -0x34, 1 }, { -0x38, 1 }, { -0x3C, 1 },
        { -0x40, 1 }, { -0x44, 1 }, { -0x48, 1 }, { -0x4C, 1 }
    }
};
DEFINE_EXTERNAL_FORMULA_WRAPPER_EX(FORMULA_GENIFS, 2, map_GENIFS, 0, 0)

static const FormulaParamMap map_KOCH_CUBE = {
    18, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 },
        { -0x30, 0 }, { -0x38, 0 }, { -0x3C, 1 }, { -0x40, 1 },
        { -0x44, 1 }, { -0x48, 1 }, { -0x4C, 1 }, { -0x50, 1 },
        { -0x54, 1 }, { -0x58, 1 }, { -0x5C, 1 }, { 0x00, 0 },
        { 0x08, 0 }, { 0x10, 0 }
    }
};
static const FormulaConstEntry consts_KOCH_CUBE[] = {
    { 0, UINT64_C(0x4000000000000000) },
    { 0, UINT64_C(0x4008000000000000) },
    { 0, UINT64_C(0x4010000000000000) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA_KOCH_CUBE, 2, map_KOCH_CUBE)

static const FormulaParamMap map_KOCH_OCT = {
    23, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 },
        { -0x2C, 1 }, { -0x30, 1 }, { -0x34, 1 }, { -0x38, 1 },
        { -0x3C, 1 }, { -0x40, 1 }, { -0x44, 1 }, { -0x48, 1 },
        { -0x4C, 1 }, { -0x50, 1 }, { -0x54, 1 }, { -0x58, 1 },
        { -0x5C, 1 }, { -0x60, 1 }, { -0x64, 1 }, { -0x68, 1 },
        { -0x6C, 1 }, { -0x70, 1 }, { -0x78, 0 }
    }
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA_KOCH_OCT, 2, map_KOCH_OCT)

static const FormulaParamMap map_KOCH_SURF = {
    22, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 },
        { -0x30, 0 }, { -0x38, 0 }, { -0x40, 0 }, { -0x48, 0 },
        { -0x50, 0 }, { -0x58, 0 }, { -0x5C, 1 }, { -0x60, 1 },
        { -0x64, 1 }, { -0x68, 1 }, { -0x6C, 1 }, { -0x70, 1 },
        { -0x74, 1 }, { -0x78, 1 }, { -0x7C, 1 }, { -0x80, 2 },
        { 0x00, 0 }, { 0x08, 0 }
    }
};
static const FormulaConstEntry consts_KOCH_SURF[] = {
    { 0, UINT64_C(0x401921FB54411744) },
    { 0, UINT64_C(0x3F91DF46AA91ED06) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA_KOCH_SURF, 2, map_KOCH_SURF)

// ============================================================================
// Registration: RegisterExternalFormulas_3Da()
// ============================================================================

void RegisterExternalFormulas_3Da() {
    RegisterExternalFormula({
        FORMULA_ABOXMAPSFOLD, "ABoxMapSFold", 11, 7,
        0.2, 1.0, 2.0, 1024.0,
        ExternWrap_FORMULA_ABOXMAPSFOLD, &map_ABOXMAPSFOLD,
        6, {0, 7, 0, 2, 1, 1},
        0x0008,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA_ABOXMOD2, "ABoxMod2", 2, 6,
        0.2, 1.0, 2.0, 1024.0,
        ExternWrap_FORMULA_ABOXMOD2, &map_ABOXMOD2,
        5, {0, 7, 0, 0, 0},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA_ABOXMODKALI, "ABoxModKali", 2, 10,
        0.15, 0.5, 2.0, 1024.0,
        ExternWrap_FORMULA_ABOXMODKALI, &map_ABOXMODKALI,
        5, {0, 7, 14, 0, 0},
        0x0000,
        4, consts_ABOXMODKALI
    });

    RegisterExternalFormula({
        FORMULA_ABOXPLATINUM, "ABoxPlatinum", 2, 26,
        0.2, 1.0, 2.0, 1024.0,
        ExternWrap_FORMULA_ABOXPLATINUM, &map_ABOXPLATINUM,
        13, {0, 7, 0, 6, 0, 0, 0, 13, 3, 3, 3, 3, 2},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA_ABOXPLATINUMB, "ABoxPlatinumB", 2, 26,
        0.2, 1.0, 2.0, 1024.0,
        ExternWrap_FORMULA_ABOXPLATINUMB, &map_ABOXPLATINUMB,
        13, {0, 7, 0, 6, 0, 0, 0, 13, 3, 3, 3, 3, 2},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA_ABOXSMOOTHFOLD, "ABoxSmoothFold", 11, 10,
        0.2, 1.0, 2.0, 1024.0,
        ExternWrap_FORMULA_ABOXSMOOTHFOLD, &map_ABOXSMOOTHFOLD,
        8, {0, 7, 0, 0, 2, 0, 2, 0},
        0x0000,
        1, consts_ABOXSMOOTHFOLD
    });

    RegisterExternalFormula({
        FORMULA_ABOXVSSHAPES, "ABoxVSShapes", 11, 11,
        0.2, 1.0, 2.0, 1024.0,
        ExternWrap_FORMULA_ABOXVSSHAPES, &map_ABOXVSSHAPES,
        7, {0, 7, 0, 0, 2, 0, 0},
        0x0000,
        3, consts_ABOXVSSHAPES
    });

    RegisterExternalFormula({
        FORMULA_ABOXVSWAVY, "ABoxVSWavy", 11, 10,
        0.2, 1.0, 2.0, 1024.0,
        ExternWrap_FORMULA_ABOXVSWAVY, &map_ABOXVSWAVY,
        9, {0, 7, 0, 0, 0, 0, 0, 0, 2},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA_ABOXVS_ICEN1, "ABoxVS_icen1", 11, 9,
        0.2, 1.0, 2.0, 1024.0,
        ExternWrap_FORMULA_ABOXVS_ICEN1, &map_ABOXVS_ICEN1,
        9, {0, 0, 0, 0, 0, 0, 0, 0, 2},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA_ABOXVS_ICEN2, "ABoxVS_icen2", 11, 11,
        0.2, 1.0, 2.0, 1024.0,
        ExternWrap_FORMULA_ABOXVS_ICEN2, &map_ABOXVS_ICEN2,
        9, {0, 7, 0, 0, 0, 0, 0, 0, 2},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA_ABOXVARYSCALE2, "ABoxVaryScale2", 11, 7,
        0.2, 1.0, 2.0, 1024.0,
        ExternWrap_FORMULA_ABOXVARYSCALE2, &map_ABOXVARYSCALE2,
        6, {0, 7, 0, 0, 0, 0},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA_ASURFMOD1, "ASurfMod1", 2, 17,
        0.2, 1.0, 2.0, 1024.0,
        ExternWrap_FORMULA_ASURFMOD1, &map_ASURFMOD1,
        8, {0, 7, 0, 0, 0, 0, 0, 6},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA_ASURFSMOOTHFOLD, "ASurfSmoothFold", 11, 19,
        0.2, 1.0, 2.0, 1024.0,
        ExternWrap_FORMULA_ASURFSMOOTHFOLD, &map_ASURFSMOOTHFOLD,
        9, {0, 7, 0, 0, 2, 0, 2, 0, 6},
        0x0000,
        1, consts_ASURFSMOOTHFOLD
    });

    RegisterExternalFormula({
        FORMULA_ATETRAVS, "ATetraVS", 11, 16,
        0.2, 1.0, 2.0, 1024.0,
        ExternWrap_FORMULA_ATETRAVS, &map_ATETRAVS,
        7, {0, 7, 0, 0, 6, 2, 2},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA_ATETRAVS2, "ATetraVS2", 11, 7,
        0.2, 1.0, 2.0, 1024.0,
        ExternWrap_FORMULA_ATETRAVS2, &map_ATETRAVS2,
        5, {0, 7, 0, 0, 13},
        0x0000,
        1, consts_ATETRAVS2
    });

    RegisterExternalFormula({
        FORMULA_AMAZINGBOX2, "AmazingBox2", 2, 21,
        0.2, 1.0, 2.0, 1024.0,
        ExternWrap_FORMULA_AMAZINGBOX2, &map_AMAZINGBOX2,
        10, {0, 7, 0, 6, 0, 0, 0, 13, 3, 3},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA_AMAZING_SURF, "Amazing Surf", 11, 15,
        0.2, 1.0, 2.0, 20.0,
        ExternWrap_FORMULA_AMAZING_SURF, &map_AMAZING_SURF,
        6, {0, 7, 0, 6, 0, 2},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA_AMAZING_SURF_2, "Amazing Surf 2", 2, 19,
        0.2, 1.0, 2.0, 1024.0,
        ExternWrap_FORMULA_AMAZING_SURF_2, &map_AMAZING_SURF_2,
        5, {0, 7, 0, 6, 2},
        0x0000,
        5, consts_AMAZING_SURF_2
    });

    RegisterExternalFormula({
        FORMULA_CANTORIFS, "CantorIFS", 2, 22,
        0.2, 1.0, 0.0, 1024.0,
        ExternWrap_FORMULA_CANTORIFS, &map_CANTORIFS,
        6, {0, 0, 0, 0, 6, 6},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA_CROSSBIFOLD, "CrossBiFold", 2, 24,
        0.2, 1.0, 0.0, 1024.0,
        ExternWrap_FORMULA_CROSSBIFOLD, &map_CROSSBIFOLD,
        8, {0, 0, 0, 0, 6, 6, 0, 0},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA_DODECAHEDRONIFS, "DodecahedronIFS", 2, 25,
        0.2, 1.0, 0.0, 1024.0,
        ExternWrap_FORMULA_DODECAHEDRONIFS, &map_DODECAHEDRONIFS,
        6, {0, 0, 0, 0, 6, 6},
        0x0000,
        3, consts_DODECAHEDRONIFS
    });

    RegisterExternalFormula({
        FORMULA_DODECAHEDRONIFS_EX, "DodecahedronIFSex", 2, 24,
        0.2, 1.0, 0.0, 1024.0,
        ExternWrap_FORMULA_DODECAHEDRONIFS_EX, &map_DODECAHEDRONIFS_EX,
        8, {0, 0, 0, 0, 6, 6, 0, 2},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA_HALFOCTIFS, "HalfOctIFS", 2, 22,
        0.2, 1.0, 0.0, 1024.0,
        ExternWrap_FORMULA_HALFOCTIFS, &map_HALFOCTIFS,
        6, {0, 0, 0, 0, 6, 6},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA_ICOSAHEDRONIFS, "IcosahedronIFS", 2, 25,
        0.2, 1.0, 0.0, 1024.0,
        ExternWrap_FORMULA_ICOSAHEDRONIFS, &map_ICOSAHEDRONIFS,
        6, {0, 0, 0, 0, 6, 6},
        0x0000,
        3, consts_ICOSAHEDRONIFS
    });

    RegisterExternalFormula({
        FORMULA_JCUBE3, "JCube3", 2, 9,
        0.2, 1.0, 0.0, 1024.0,
        ExternWrap_FORMULA_JCUBE3, &map_JCUBE3,
        8, {22, 0, 0, 0, 0, 0, 0, 0},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA_MANDALAY_KIFS, "Mandalay-KIFS", 2, 16,
        0.2, 1.0, 2.0, 1024.0,
        ExternWrap_FORMULA_MANDALAY_KIFS, &map_MANDALAY_KIFS,
        7, {0, 7, 0, 6, 0, 0, 0},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA_MANDALEX, "Mandalex", 2, 17,
        0.2, 1.0, 2.0, 1024.0,
        ExternWrap_FORMULA_MANDALEX, &map_MANDALEX,
        8, {0, 7, 0, 6, 0, 0, 0, 2},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA_MENGER3, "Menger3", 2, 22,
        0.2, 1.0, 0.0, 1024.0,
        ExternWrap_FORMULA_MENGER3, &map_MENGER3,
        6, {0, 0, 0, 0, 6, 6},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA_MENGERHYPER, "MengerHyper", 2, 11,
        0.2, 0.4, 0.0, 1024.0,
        ExternWrap_FORMULA_MENGERHYPER, &map_MENGERHYPER,
        11, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA_MENGERIFS_PLUS, "MengerIFSplus", 2, 16,
        0.2, 1.0, 0.0, 1024.0,
        ExternWrap_FORMULA_MENGERIFS_PLUS, &map_MENGERIFS_PLUS,
        8, {0, 0, 0, 0, 6, 0, 0, 0},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA_MENGERKOCH, "MengerKoch", 2, 24,
        0.2, 1.0, 0.0, 1024.0,
        ExternWrap_FORMULA_MENGERKOCH, &map_MENGERKOCH,
        8, {0, 0, 0, 0, 6, 6, 0, 0},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA_MENGERKOCHV2, "MengerKochV2", 2, 25,
        0.2, 1.0, 0.0, 1024.0,
        ExternWrap_FORMULA_MENGERKOCHV2, &map_MENGERKOCHV2,
        8, {0, 0, 0, 0, 6, 6, 0, 0},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA_MENGERSMT, "MengerSmt", 2, 16,
        0.2, 1.0, 0.0, 1024.0,
        ExternWrap_FORMULA_MENGERSMT, &map_MENGERSMT,
        7, {0, 0, 0, 0, 6, 0, 0},
        0x0000,
        1, consts_MENGERSMT
    });

    RegisterExternalFormula({
        FORMULA_OCTKOCH, "OctKoch", 2, 25,
        0.2, 1.0, 0.0, 1024.0,
        ExternWrap_FORMULA_OCTKOCH, &map_OCTKOCH,
        9, {0, 0, 0, 0, 6, 6, 0, 0, 2},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA_OCTAFULLIFS, "OctaFullIFS", 2, 22,
        0.2, 1.0, 0.0, 1024.0,
        ExternWrap_FORMULA_OCTAFULLIFS, &map_OCTAFULLIFS,
        6, {0, 0, 0, 0, 6, 6},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA_OCTAHEDRONIFS, "OctahedronIFS", 2, 22,
        0.2, 1.0, 0.0, 1024.0,
        ExternWrap_FORMULA_OCTAHEDRONIFS, &map_OCTAHEDRONIFS,
        6, {0, 0, 0, 0, 6, 6},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA_SIERPHILBERT, "SierpHilbert", 2, 24,
        0.2, 1.0, 2.0, 1024.0,
        ExternWrap_FORMULA_SIERPHILBERT, &map_SIERPHILBERT,
        8, {0, 0, 0, 0, 6, 6, 0, 0},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA_SIERPINSKI3ALT, "Sierpinski3alt", 2, 16,
        0.2, 1.0, 2.0, 1024.0,
        ExternWrap_FORMULA_SIERPINSKI3ALT, &map_SIERPINSKI3ALT,
        8, {0, 0, 0, 0, 0, 0, 0, 6},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA_SIERPINSKIFULL, "SierpinskiFull", 2, 25,
        0.2, 1.0, 2.0, 1024.0,
        ExternWrap_FORMULA_SIERPINSKIFULL, &map_SIERPINSKIFULL,
        9, {0, 0, 0, 0, 6, 6, 0, 0, 2},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA__SCALING, "_Scaling", 2, 1,
        0.2, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA__SCALING, &map__SCALING,
        1, {0},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA_GENIFS, "genIFS", 2, 16,
        0.2, 0.5, 0.0, 16.0,
        ExternWrap_FORMULA_GENIFS, &map_GENIFS,
        8, {0, 2, 2, 2, 1, 1, 1, 6},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA_KOCH_CUBE, "koch_cube", 2, 18,
        1.0, 1.0, 1.0, 5.0,
        ExternWrap_FORMULA_KOCH_CUBE, &map_KOCH_CUBE,
        7, {0, 0, 0, 0, 0, 0, 6},
        0x0000,
        3, consts_KOCH_CUBE
    });

    RegisterExternalFormula({
        FORMULA_KOCH_OCT, "koch_oct", 2, 23,
        0.2, 1.0, 0.0, 1024.0,
        ExternWrap_FORMULA_KOCH_OCT, &map_KOCH_OCT,
        7, {0, 0, 0, 0, 6, 6, 0},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA_KOCH_SURF, "koch_surf", 2, 22,
        1.0, 1.0, 1.0, 5.0,
        ExternWrap_FORMULA_KOCH_SURF, &map_KOCH_SURF,
        12, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 2},
        0x0000,
        2, consts_KOCH_SURF
    });

}

} // namespace mb3d

#undef constant
#undef buildRotMatrix3
#undef rotMatrix3FromFloats
#undef RotMatrix3
#undef M_PI_2_F
