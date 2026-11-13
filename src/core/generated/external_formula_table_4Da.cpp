// external_formula_table_4Da.cpp — Auto-generated bulk formula registrations
// Category: 4Da (13 formulas)
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
// Include 4Da formula .inc files
// ============================================================================

#include "4Da/ABoxMod4d.inc"
#include "4Da/ABoxSphereOffset4d.inc"
#include "4Da/ABoxVaryScale4d.inc"
#include "4Da/HalfOct4a.inc"
#include "4Da/HalfOct4b.inc"
#include "4Da/Menger4.inc"
#include "4Da/Menger4ex.inc"
#include "4Da/MixPinski4.inc"
#include "4Da/MixPinski4ex.inc"
#include "4Da/Octahedron4.inc"
#include "4Da/Sierpinski4.inc"
#include "4Da/Sierpinski4ex.inc"
#include "4Da/_ScaleC4d.inc"

// ============================================================================
// PVar offset maps (4Da)
// ============================================================================

static const FormulaParamMap map_ABOXMOD4D = { 4, { { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { 0x00, 0 } } };
static const FormulaConstEntry consts_ABOXMOD4D[] = {
    { 0, UINT64_C(0x3379B604AAACA626) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA_ABOXMOD4D, 5, map_ABOXMOD4D)

static const FormulaParamMap map_ABOXSPHEREOFFSET4D = {
    9, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 },
        { -0x30, 0 }, { -0x38, 0 }, { -0x40, 0 }, { -0x48, 0 },
        { -0x50, 0 }
    }
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA_ABOXSPHEREOFFSET4D, 5, map_ABOXSPHEREOFFSET4D)

static const FormulaParamMap map_ABOXVARYSCALE4D = {
    7, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 },
        { -0x30, 0 }, { -0x34, 1 }, { -0x3C, 0 }
    }
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA_ABOXVARYSCALE4D, 5, map_ABOXVARYSCALE4D)

static const FormulaParamMap map_HALFOCT4A = {
    21, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 },
        { -0x30, 0 }, { -0x34, 1 }, { -0x38, 1 }, { -0x3C, 1 },
        { -0x40, 1 }, { -0x44, 1 }, { -0x48, 1 }, { -0x4C, 1 },
        { -0x50, 1 }, { -0x54, 1 }, { -0x58, 1 }, { -0x5C, 1 },
        { -0x60, 1 }, { -0x64, 1 }, { -0x68, 1 }, { -0x6C, 1 },
        { -0x70, 1 }
    }
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA_HALFOCT4A, 6, map_HALFOCT4A)

static const FormulaParamMap map_HALFOCT4B = {
    21, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 },
        { -0x30, 0 }, { -0x34, 1 }, { -0x38, 1 }, { -0x3C, 1 },
        { -0x40, 1 }, { -0x44, 1 }, { -0x48, 1 }, { -0x4C, 1 },
        { -0x50, 1 }, { -0x54, 1 }, { -0x58, 1 }, { -0x5C, 1 },
        { -0x60, 1 }, { -0x64, 1 }, { -0x68, 1 }, { -0x6C, 1 },
        { -0x70, 1 }
    }
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA_HALFOCT4B, 6, map_HALFOCT4B)

static const FormulaParamMap map_MENGER4 = {
    21, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 },
        { -0x30, 0 }, { -0x34, 1 }, { -0x38, 1 }, { -0x3C, 1 },
        { -0x40, 1 }, { -0x44, 1 }, { -0x48, 1 }, { -0x4C, 1 },
        { -0x50, 1 }, { -0x54, 1 }, { -0x58, 1 }, { -0x5C, 1 },
        { -0x60, 1 }, { -0x64, 1 }, { -0x68, 1 }, { -0x6C, 1 },
        { -0x70, 1 }
    }
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA_MENGER4, 6, map_MENGER4)

static const FormulaParamMap map_MENGER4EX = {
    25, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 },
        { -0x30, 0 }, { -0x34, 1 }, { -0x38, 1 }, { -0x3C, 1 },
        { -0x40, 1 }, { -0x44, 1 }, { -0x48, 1 }, { -0x4C, 1 },
        { -0x50, 1 }, { -0x54, 1 }, { -0x58, 1 }, { -0x5C, 1 },
        { -0x60, 1 }, { -0x64, 1 }, { -0x68, 1 }, { -0x6C, 1 },
        { -0x70, 1 }, { -0x78, 0 }, { -0x80, 0 }, { -0x88, 0 },
        { -0x90, 0 }
    }
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA_MENGER4EX, 6, map_MENGER4EX)

static const FormulaParamMap map_MIXPINSKI4 = {
    21, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 },
        { -0x30, 0 }, { -0x34, 1 }, { -0x38, 1 }, { -0x3C, 1 },
        { -0x40, 1 }, { -0x44, 1 }, { -0x48, 1 }, { -0x4C, 1 },
        { -0x50, 1 }, { -0x54, 1 }, { -0x58, 1 }, { -0x5C, 1 },
        { -0x60, 1 }, { -0x64, 1 }, { -0x68, 1 }, { -0x6C, 1 },
        { -0x70, 1 }
    }
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA_MIXPINSKI4, 6, map_MIXPINSKI4)

static const FormulaParamMap map_MIXPINSKI4EX = {
    25, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 },
        { -0x30, 0 }, { -0x34, 1 }, { -0x38, 1 }, { -0x3C, 1 },
        { -0x40, 1 }, { -0x44, 1 }, { -0x48, 1 }, { -0x4C, 1 },
        { -0x50, 1 }, { -0x54, 1 }, { -0x58, 1 }, { -0x5C, 1 },
        { -0x60, 1 }, { -0x64, 1 }, { -0x68, 1 }, { -0x6C, 1 },
        { -0x70, 1 }, { -0x78, 0 }, { -0x80, 0 }, { -0x88, 0 },
        { -0x90, 0 }
    }
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA_MIXPINSKI4EX, 6, map_MIXPINSKI4EX)

static const FormulaParamMap map_OCTAHEDRON4 = {
    21, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 },
        { -0x30, 0 }, { -0x34, 1 }, { -0x38, 1 }, { -0x3C, 1 },
        { -0x40, 1 }, { -0x44, 1 }, { -0x48, 1 }, { -0x4C, 1 },
        { -0x50, 1 }, { -0x54, 1 }, { -0x58, 1 }, { -0x5C, 1 },
        { -0x60, 1 }, { -0x64, 1 }, { -0x68, 1 }, { -0x6C, 1 },
        { -0x70, 1 }
    }
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA_OCTAHEDRON4, 6, map_OCTAHEDRON4)

static const FormulaParamMap map_SIERPINSKI4 = {
    21, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 },
        { -0x30, 0 }, { -0x34, 1 }, { -0x38, 1 }, { -0x3C, 1 },
        { -0x40, 1 }, { -0x44, 1 }, { -0x48, 1 }, { -0x4C, 1 },
        { -0x50, 1 }, { -0x54, 1 }, { -0x58, 1 }, { -0x5C, 1 },
        { -0x60, 1 }, { -0x64, 1 }, { -0x68, 1 }, { -0x6C, 1 },
        { -0x70, 1 }
    }
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA_SIERPINSKI4, 6, map_SIERPINSKI4)

static const FormulaParamMap map_SIERPINSKI4EX = {
    25, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 },
        { -0x30, 0 }, { -0x34, 1 }, { -0x38, 1 }, { -0x3C, 1 },
        { -0x40, 1 }, { -0x44, 1 }, { -0x48, 1 }, { -0x4C, 1 },
        { -0x50, 1 }, { -0x54, 1 }, { -0x58, 1 }, { -0x5C, 1 },
        { -0x60, 1 }, { -0x64, 1 }, { -0x68, 1 }, { -0x6C, 1 },
        { -0x70, 1 }, { -0x78, 0 }, { -0x80, 0 }, { -0x88, 0 },
        { -0x90, 0 }
    }
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA_SIERPINSKI4EX, 6, map_SIERPINSKI4EX)

static const FormulaParamMap map__SCALEC4D = { 2, { { -0x10, 0 }, { -0x18, 0 } } };
DEFINE_EXTERNAL_FORMULA_WRAPPER_EX(FORMULA__SCALEC4D, 5, map__SCALEC4D, 0, 0)

// ============================================================================
// Registration: RegisterExternalFormulas_4Da()
// ============================================================================

void RegisterExternalFormulas_4Da() {
    RegisterExternalFormula({
        FORMULA_ABOXMOD4D, "ABoxMod4d", 5, 4,
        0.2, 1.0, 2.0, 1024.0,
        ExternWrap_FORMULA_ABOXMOD4D, &map_ABOXMOD4D,
        3, {0, 0, 0},
        0x0000,
        1, consts_ABOXMOD4D
    });

    RegisterExternalFormula({
        FORMULA_ABOXSPHEREOFFSET4D, "ABoxSphereOffset4d", 5, 9,
        0.2, 1.0, 2.0, 1024.0,
        ExternWrap_FORMULA_ABOXSPHEREOFFSET4D, &map_ABOXSPHEREOFFSET4D,
        8, {0, 7, 0, 0, 0, 0, 0, 0},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA_ABOXVARYSCALE4D, "ABoxVaryScale4d", 5, 7,
        0.2, 1.0, 2.0, 1024.0,
        ExternWrap_FORMULA_ABOXVARYSCALE4D, &map_ABOXVARYSCALE4D,
        6, {0, 7, 0, 0, 1, 0},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA_HALFOCT4A, "HalfOct4a", 6, 21,
        0.2, 1.1, 2.0, 1024.0,
        ExternWrap_FORMULA_HALFOCT4A, &map_HALFOCT4A,
        6, {0, 0, 0, 0, 0, 12},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA_HALFOCT4B, "HalfOct4b", 6, 21,
        0.2, 1.1, 2.0, 1024.0,
        ExternWrap_FORMULA_HALFOCT4B, &map_HALFOCT4B,
        6, {0, 0, 0, 0, 0, 12},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA_MENGER4, "Menger4", 6, 21,
        0.2, 0.5, 2.0, 1024.0,
        ExternWrap_FORMULA_MENGER4, &map_MENGER4,
        6, {0, 0, 0, 0, 0, 12},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA_MENGER4EX, "Menger4ex", 6, 25,
        0.2, 0.5, 2.0, 1024.0,
        ExternWrap_FORMULA_MENGER4EX, &map_MENGER4EX,
        10, {0, 0, 0, 0, 0, 12, 0, 0, 0, 0},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA_MIXPINSKI4, "MixPinski4", 6, 21,
        0.2, 2.0, 2.0, 1024.0,
        ExternWrap_FORMULA_MIXPINSKI4, &map_MIXPINSKI4,
        6, {0, 0, 0, 0, 0, 12},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA_MIXPINSKI4EX, "MixPinski4ex", 6, 25,
        0.2, 2.0, 2.0, 1024.0,
        ExternWrap_FORMULA_MIXPINSKI4EX, &map_MIXPINSKI4EX,
        10, {0, 0, 0, 0, 0, 12, 0, 0, 0, 0},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA_OCTAHEDRON4, "Octahedron4", 6, 21,
        0.2, 1.0, 2.0, 1024.0,
        ExternWrap_FORMULA_OCTAHEDRON4, &map_OCTAHEDRON4,
        6, {0, 0, 0, 0, 0, 12},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA_SIERPINSKI4, "Sierpinski4", 6, 21,
        0.2, 1.1, 2.0, 1024.0,
        ExternWrap_FORMULA_SIERPINSKI4, &map_SIERPINSKI4,
        6, {0, 0, 0, 0, 0, 12},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA_SIERPINSKI4EX, "Sierpinski4ex", 6, 25,
        0.2, 1.1, 2.0, 1024.0,
        ExternWrap_FORMULA_SIERPINSKI4EX, &map_SIERPINSKI4EX,
        10, {0, 0, 0, 0, 0, 12, 0, 0, 0, 0},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA__SCALEC4D, "_ScaleC4d", 5, 2,
        0.2, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA__SCALEC4D, &map__SCALEC4D,
        2, {0, 0},
        0x0000,
        0, nullptr
    });

}

} // namespace mb3d

#undef constant
#undef buildRotMatrix3
#undef rotMatrix3FromFloats
#undef RotMatrix3
#undef M_PI_2_F
