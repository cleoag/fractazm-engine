// external_formula_table_dIFSa.cpp — Auto-generated bulk formula registrations
// Category: dIFSa (21 formulas)
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
// Include dIFSa formula .inc files
// ============================================================================

#include "dIFSa/Apollo2D-IFS.inc"
#include "dIFSa/Apollo3D-IFS.inc"
#include "dIFSa/FernTransform.inc"
#include "dIFSa/Menger1-IFS.inc"
#include "dIFSa/PolyFold-symIFS.inc"
#include "dIFSa/PolyFoldEx-symIFS.inc"
#include "dIFSa/PolyFoldExIFS.inc"
#include "dIFSa/PolyFoldIFS.inc"
#include "dIFSa/TilingIFS.inc"
#include "dIFSa/Trans-KochIFS.inc"
#include "dIFSa/amazingIFS.inc"
#include "dIFSa/cayley2IFS.inc"
#include "dIFSa/foldinghexIFS.inc"
#include "dIFSa/foldinginfyIFS.inc"
#include "dIFSa/foldingoctIFS.inc"
#include "dIFSa/tilingbrick2IFS.inc"
#include "dIFSa/tilingbrickIFS.inc"
#include "dIFSa/totoricalIFS.inc"
#include "dIFSa/trans-qIFS.inc"
#include "dIFSa/transLogSpIFS.inc"
#include "dIFSa/transform2IFS.inc"

// ============================================================================
// PVar offset maps (dIFSa)
// ============================================================================

static const FormulaParamMap map_APOLLO2DIFS = {
    17, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x24, 1 },
        { -0x28, 1 }, { -0x2C, 1 }, { -0x30, 1 }, { -0x34, 1 },
        { -0x38, 1 }, { -0x3C, 1 }, { -0x40, 1 }, { -0x44, 1 },
        { -0x48, 2 }, { -0x50, 0 }, { -0x58, 0 }, { 0x00, 0 },
        { 0x08, 0 }
    }
};
static const FormulaConstEntry consts_APOLLO2DIFS[] = {
    { 0, UINT64_C(0x3FC45F306D974364) },
    { 0, UINT64_C(0x401921FB54411744) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER_EX(FORMULA_APOLLO2DIFS, 21, map_APOLLO2DIFS, 1, 1)

static const FormulaParamMap map_APOLLO3DIFS = {
    20, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x24, 1 },
        { -0x28, 1 }, { -0x2C, 1 }, { -0x30, 1 }, { -0x34, 1 },
        { -0x38, 1 }, { -0x3C, 1 }, { -0x40, 1 }, { -0x44, 1 },
        { -0x48, 2 }, { -0x50, 0 }, { -0x58, 0 }, { -0x60, 0 },
        { 0x00, 0 }, { 0x08, 0 }, { 0x10, 0 }, { 0x18, 0 }
    }
};
static const FormulaConstEntry consts_APOLLO3DIFS[] = {
    { 0, UINT64_C(0x3FC45F306D974364) },
    { 0, UINT64_C(0x401921FB54411744) },
    { 0, UINT64_C(0x3FF921FB54411744) },
    { 0, UINT64_C(0x3F91DF46AA91ED06) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER_EX(FORMULA_APOLLO3DIFS, 21, map_APOLLO3DIFS, 1, 1)

static const FormulaParamMap map_FERNTRANSFORM = {
    15, {
        { -0x0C, 1 }, { -0x10, 1 }, { -0x14, 1 }, { -0x18, 1 },
        { -0x1C, 1 }, { -0x20, 1 }, { -0x24, 1 }, { -0x28, 1 },
        { -0x2C, 1 }, { -0x30, 1 }, { -0x34, 1 }, { -0x38, 1 },
        { -0x3C, 1 }, { -0x40, 1 }, { -0x44, 1 }
    }
};
DEFINE_EXTERNAL_FORMULA_WRAPPER_EX(FORMULA_FERNTRANSFORM, 21, map_FERNTRANSFORM, 1, 1)

static const FormulaParamMap map_MENGER1_IFS = {
    13, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 },
        { -0x2C, 1 }, { -0x30, 1 }, { -0x34, 1 }, { -0x38, 1 },
        { -0x3C, 1 }, { -0x40, 1 }, { -0x44, 1 }, { -0x48, 1 },
        { -0x4C, 1 }
    }
};
DEFINE_EXTERNAL_FORMULA_WRAPPER_EX(FORMULA_MENGER1_IFS, 21, map_MENGER1_IFS, 1, 1)

static const FormulaParamMap map_POLYFOLD_SYMIFS = {
    18, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x24, 1 },
        { -0x28, 1 }, { -0x2C, 1 }, { -0x30, 1 }, { -0x34, 1 },
        { -0x38, 1 }, { -0x3C, 1 }, { -0x40, 1 }, { -0x44, 1 },
        { -0x48, 2 }, { -0x50, 0 }, { 0x00, 0 }, { 0x08, 0 },
        { 0x10, 0 }, { 0x18, 0 }
    }
};
static const FormulaConstEntry consts_POLYFOLD_SYMIFS[] = {
    { 0, UINT64_C(0x3FC45F306D974364) },
    { 0, UINT64_C(0x401921FB54411744) },
    { 0, UINT64_C(0x3FF921FB54411744) },
    { 0, UINT64_C(0x3F91DF46AA91ED06) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER_EX(FORMULA_POLYFOLD_SYMIFS, 21, map_POLYFOLD_SYMIFS, 1, 1)

static const FormulaParamMap map_POLYFOLDEX_SYMIFS = {
    20, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x24, 1 },
        { -0x28, 1 }, { -0x2C, 1 }, { -0x30, 1 }, { -0x34, 1 },
        { -0x38, 1 }, { -0x3C, 1 }, { -0x40, 1 }, { -0x44, 1 },
        { -0x48, 2 }, { -0x50, 0 }, { -0x54, 2 }, { -0x5C, 0 },
        { 0x00, 0 }, { 0x08, 0 }, { 0x10, 0 }, { 0x18, 0 }
    }
};
static const FormulaConstEntry consts_POLYFOLDEX_SYMIFS[] = {
    { 0, UINT64_C(0x3FC45F306D974364) },
    { 0, UINT64_C(0x401921FB54411744) },
    { 0, UINT64_C(0x3FF921FB54411744) },
    { 0, UINT64_C(0x3F91DF46AA91ED06) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER_EX(FORMULA_POLYFOLDEX_SYMIFS, 21, map_POLYFOLDEX_SYMIFS, 1, 1)

static const FormulaParamMap map_POLYFOLDEXIFS = {
    20, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x24, 1 },
        { -0x28, 1 }, { -0x2C, 1 }, { -0x30, 1 }, { -0x34, 1 },
        { -0x38, 1 }, { -0x3C, 1 }, { -0x40, 1 }, { -0x44, 1 },
        { -0x48, 2 }, { -0x50, 0 }, { -0x54, 2 }, { -0x5C, 0 },
        { 0x00, 0 }, { 0x08, 0 }, { 0x10, 0 }, { 0x18, 0 }
    }
};
static const FormulaConstEntry consts_POLYFOLDEXIFS[] = {
    { 0, UINT64_C(0x3FC45F306D974364) },
    { 0, UINT64_C(0x401921FB54411744) },
    { 0, UINT64_C(0x3FF921FB54411744) },
    { 0, UINT64_C(0x3F91DF46AA91ED06) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER_EX(FORMULA_POLYFOLDEXIFS, 21, map_POLYFOLDEXIFS, 1, 1)

static const FormulaParamMap map_POLYFOLDIFS = {
    18, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x24, 1 },
        { -0x28, 1 }, { -0x2C, 1 }, { -0x30, 1 }, { -0x34, 1 },
        { -0x38, 1 }, { -0x3C, 1 }, { -0x40, 1 }, { -0x44, 1 },
        { -0x48, 2 }, { -0x50, 0 }, { 0x00, 0 }, { 0x08, 0 },
        { 0x10, 0 }, { 0x18, 0 }
    }
};
static const FormulaConstEntry consts_POLYFOLDIFS[] = {
    { 0, UINT64_C(0x3FC45F306D974364) },
    { 0, UINT64_C(0x401921FB54411744) },
    { 0, UINT64_C(0x3FF921FB54411744) },
    { 0, UINT64_C(0x3F91DF46AA91ED06) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER_EX(FORMULA_POLYFOLDIFS, 21, map_POLYFOLDIFS, 1, 1)

static const FormulaParamMap map_TILINGIFS = {
    8, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 },
        { -0x30, 0 }, { -0x38, 0 }, { -0x40, 0 }, { -0x44, 2 }
    }
};
DEFINE_EXTERNAL_FORMULA_WRAPPER_EX(FORMULA_TILINGIFS, 21, map_TILINGIFS, 1, 1)

static const FormulaParamMap map_TRANS_KOCHIFS = {
    22, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x24, 1 },
        { -0x28, 1 }, { -0x2C, 1 }, { -0x30, 1 }, { -0x34, 1 },
        { -0x38, 1 }, { -0x3C, 1 }, { -0x40, 1 }, { -0x44, 1 },
        { -0x48, 2 }, { -0x4C, 1 }, { -0x50, 1 }, { -0x58, 0 },
        { -0x60, 0 }, { -0x68, 0 }, { -0x70, 0 }, { 0x00, 0 },
        { 0x08, 0 }, { 0x10, 0 }
    }
};
static const FormulaConstEntry consts_TRANS_KOCHIFS[] = {
    { 0, UINT64_C(0x3FC45F306D974364) },
    { 0, UINT64_C(0x401921FB54411744) },
    { 0, UINT64_C(0x400921FB54442D18) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER_EX(FORMULA_TRANS_KOCHIFS, 21, map_TRANS_KOCHIFS, 1, 1)

static const FormulaParamMap map_AMAZINGIFS = {
    16, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x24, 1 },
        { -0x28, 1 }, { -0x2C, 1 }, { -0x30, 1 }, { -0x34, 1 },
        { -0x38, 1 }, { -0x3C, 1 }, { -0x40, 1 }, { -0x44, 1 },
        { -0x4C, 0 }, { -0x54, 0 }, { -0x5C, 0 }, { -0x64, 0 }
    }
};
DEFINE_EXTERNAL_FORMULA_WRAPPER_EX(FORMULA_AMAZINGIFS, 21, map_AMAZINGIFS, 1, 1)

static const FormulaParamMap map_CAYLEY2IFS = {
    13, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x24, 1 },
        { -0x28, 1 }, { -0x2C, 1 }, { -0x30, 1 }, { -0x34, 1 },
        { -0x38, 1 }, { -0x3C, 1 }, { -0x40, 1 }, { -0x44, 1 },
        { -0x4C, 0 }
    }
};
DEFINE_EXTERNAL_FORMULA_WRAPPER_EX(FORMULA_CAYLEY2IFS, 21, map_CAYLEY2IFS, 1, 1)

static const FormulaParamMap map_FOLDINGHEXIFS = {
    6, {
        { -0x10, 0 }, { 0x00, 0 }, { 0x08, 0 }, { 0x10, 0 },
        { 0x18, 0 }, { 0x20, 0 }
    }
};
static const FormulaConstEntry consts_FOLDINGHEXIFS[] = {
    { 0, UINT64_C(0x3FE0000000000000) },
    { 0, UINT64_C(0x3FE279A6B50B0F28) },
    { 0, UINT64_C(0xBFEBB67A0F9096BC) },
    { 0, UINT64_C(0xBFE0000000000000) },
    { 0, UINT64_C(0x3FDBB67A0F9096BC) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER_EX(FORMULA_FOLDINGHEXIFS, 21, map_FOLDINGHEXIFS, 1, 1)

static const FormulaParamMap map_FOLDINGINFYIFS = {
    15, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x24, 1 },
        { -0x28, 1 }, { -0x2C, 1 }, { -0x30, 1 }, { -0x34, 1 },
        { -0x38, 1 }, { -0x3C, 1 }, { -0x40, 1 }, { -0x44, 1 },
        { -0x4C, 0 }, { -0x54, 0 }, { -0x5C, 0 }
    }
};
DEFINE_EXTERNAL_FORMULA_WRAPPER_EX(FORMULA_FOLDINGINFYIFS, 21, map_FOLDINGINFYIFS, 1, 1)

static const FormulaParamMap map_FOLDINGOCTIFS = {
    15, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x24, 1 },
        { -0x28, 1 }, { -0x2C, 1 }, { -0x30, 1 }, { -0x34, 1 },
        { -0x38, 1 }, { -0x3C, 1 }, { -0x40, 1 }, { -0x44, 1 },
        { -0x48, 2 }, { -0x4C, 2 }, { -0x50, 2 }
    }
};
DEFINE_EXTERNAL_FORMULA_WRAPPER_EX(FORMULA_FOLDINGOCTIFS, 21, map_FOLDINGOCTIFS, 1, 1)

static const FormulaParamMap map_TILINGBRICK2IFS = {
    15, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x24, 1 },
        { -0x28, 1 }, { -0x2C, 1 }, { -0x30, 1 }, { -0x34, 1 },
        { -0x38, 1 }, { -0x3C, 1 }, { -0x40, 1 }, { -0x44, 1 },
        { -0x4C, 0 }, { -0x54, 0 }, { -0x5C, 0 }
    }
};
DEFINE_EXTERNAL_FORMULA_WRAPPER_EX(FORMULA_TILINGBRICK2IFS, 21, map_TILINGBRICK2IFS, 1, 1)

static const FormulaParamMap map_TILINGBRICKIFS = {
    15, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x24, 1 },
        { -0x28, 1 }, { -0x2C, 1 }, { -0x30, 1 }, { -0x34, 1 },
        { -0x38, 1 }, { -0x3C, 1 }, { -0x40, 1 }, { -0x44, 1 },
        { -0x4C, 0 }, { -0x54, 0 }, { -0x5C, 0 }
    }
};
DEFINE_EXTERNAL_FORMULA_WRAPPER_EX(FORMULA_TILINGBRICKIFS, 21, map_TILINGBRICKIFS, 1, 1)

static const FormulaParamMap map_TOTORICALIFS = {
    11, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 },
        { -0x30, 0 }, { -0x38, 0 }, { -0x40, 0 }, { -0x44, 2 },
        { -0x4C, 0 }, { 0x00, 0 }, { 0x08, 0 }
    }
};
static const FormulaConstEntry consts_TOTORICALIFS[] = {
    { 0, UINT64_C(0x3FF921FB54442D18) },
    { 0, UINT64_C(0x3FD45F306DC9C883) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER_EX(FORMULA_TOTORICALIFS, 21, map_TOTORICALIFS, 1, 1)

static const FormulaParamMap map_TRANS_QIFS = {
    14, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x24, 1 },
        { -0x28, 1 }, { -0x2C, 1 }, { -0x30, 1 }, { -0x34, 1 },
        { -0x38, 1 }, { -0x3C, 1 }, { -0x40, 1 }, { -0x44, 1 },
        { -0x4C, 0 }, { -0x54, 0 }
    }
};
DEFINE_EXTERNAL_FORMULA_WRAPPER_EX(FORMULA_TRANS_QIFS, 21, map_TRANS_QIFS, 1, 1)

static const FormulaParamMap map_TRANSLOGSPIFS = {
    20, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x24, 1 },
        { -0x28, 1 }, { -0x2C, 1 }, { -0x30, 1 }, { -0x34, 1 },
        { -0x38, 1 }, { -0x3C, 1 }, { -0x40, 1 }, { -0x44, 1 },
        { -0x48, 2 }, { -0x4C, 2 }, { -0x54, 0 }, { -0x5C, 0 },
        { 0x00, 0 }, { 0x08, 0 }, { 0x10, 0 }, { 0x18, 0 }
    }
};
static const FormulaConstEntry consts_TRANSLOGSPIFS[] = {
    { 0, UINT64_C(0x3FC45F306D974364) },
    { 0, UINT64_C(0x401921FB54411744) },
    { 0, UINT64_C(0x3FA47AE147AE147B) },
    { 0, UINT64_C(0x3FD555475A31A4BE) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER_EX(FORMULA_TRANSLOGSPIFS, 21, map_TRANSLOGSPIFS, 1, 1)

static const FormulaParamMap map_TRANSFORM2IFS = {
    15, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x24, 1 },
        { -0x28, 1 }, { -0x2C, 1 }, { -0x30, 1 }, { -0x34, 1 },
        { -0x38, 1 }, { -0x3C, 1 }, { -0x40, 1 }, { -0x44, 1 },
        { -0x48, 2 }, { -0x4C, 2 }, { -0x50, 2 }
    }
};
DEFINE_EXTERNAL_FORMULA_WRAPPER_EX(FORMULA_TRANSFORM2IFS, 21, map_TRANSFORM2IFS, 1, 1)

// ============================================================================
// Registration: RegisterExternalFormulas_dIFSa()
// ============================================================================

void RegisterExternalFormulas_dIFSa() {
    RegisterExternalFormula({
        FORMULA_APOLLO2DIFS, "Apollo2D-IFS", 21, 17,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA_APOLLO2DIFS, &map_APOLLO2DIFS,
        7, {0, 0, 0, 6, 2, 0, 0},
        0x0000,
        2, consts_APOLLO2DIFS
    });

    RegisterExternalFormula({
        FORMULA_APOLLO3DIFS, "Apollo3D-IFS", 21, 20,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA_APOLLO3DIFS, &map_APOLLO3DIFS,
        8, {0, 0, 0, 6, 2, 0, 0, 0},
        0x0000,
        4, consts_APOLLO3DIFS
    });

    RegisterExternalFormula({
        FORMULA_FERNTRANSFORM, "FernTransform", 21, 15,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA_FERNTRANSFORM, &map_FERNTRANSFORM,
        11, {1, 1, 4, 4, 1, 4, 1, 4, 1, 1, 1},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA_MENGER1_IFS, "Menger1-IFS", 21, 13,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA_MENGER1_IFS, &map_MENGER1_IFS,
        5, {0, 0, 0, 0, 6},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA_POLYFOLD_SYMIFS, "PolyFold-symIFS", 21, 18,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA_POLYFOLD_SYMIFS, &map_POLYFOLD_SYMIFS,
        6, {0, 0, 0, 6, 2, 0},
        0x0000,
        4, consts_POLYFOLD_SYMIFS
    });

    RegisterExternalFormula({
        FORMULA_POLYFOLDEX_SYMIFS, "PolyFoldEx-symIFS", 21, 20,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA_POLYFOLDEX_SYMIFS, &map_POLYFOLDEX_SYMIFS,
        8, {0, 0, 0, 6, 2, 0, 2, 0},
        0x0000,
        4, consts_POLYFOLDEX_SYMIFS
    });

    RegisterExternalFormula({
        FORMULA_POLYFOLDEXIFS, "PolyFoldExIFS", 21, 20,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA_POLYFOLDEXIFS, &map_POLYFOLDEXIFS,
        8, {0, 0, 0, 6, 2, 0, 2, 0},
        0x0000,
        4, consts_POLYFOLDEXIFS
    });

    RegisterExternalFormula({
        FORMULA_POLYFOLDIFS, "PolyFoldIFS", 21, 18,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA_POLYFOLDIFS, &map_POLYFOLDIFS,
        6, {0, 0, 0, 6, 2, 0},
        0x0000,
        4, consts_POLYFOLDIFS
    });

    RegisterExternalFormula({
        FORMULA_TILINGIFS, "TilingIFS", 21, 8,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA_TILINGIFS, &map_TILINGIFS,
        8, {0, 0, 0, 0, 0, 0, 0, 2},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA_TRANS_KOCHIFS, "Trans-KochIFS", 21, 22,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA_TRANS_KOCHIFS, &map_TRANS_KOCHIFS,
        10, {0, 0, 0, 6, 2, 4, 0, 0, 0, 0},
        0x0000,
        3, consts_TRANS_KOCHIFS
    });

    RegisterExternalFormula({
        FORMULA_AMAZINGIFS, "amazingIFS", 21, 16,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA_AMAZINGIFS, &map_AMAZINGIFS,
        8, {0, 0, 0, 6, 0, 0, 0, 0},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA_CAYLEY2IFS, "cayley2IFS", 21, 13,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA_CAYLEY2IFS, &map_CAYLEY2IFS,
        5, {0, 0, 0, 6, 0},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA_FOLDINGHEXIFS, "foldinghexIFS", 21, 6,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA_FOLDINGHEXIFS, &map_FOLDINGHEXIFS,
        1, {0},
        0x0000,
        5, consts_FOLDINGHEXIFS
    });

    RegisterExternalFormula({
        FORMULA_FOLDINGINFYIFS, "foldinginfyIFS", 21, 15,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA_FOLDINGINFYIFS, &map_FOLDINGINFYIFS,
        7, {0, 0, 0, 6, 0, 0, 0},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA_FOLDINGOCTIFS, "foldingoctIFS", 21, 15,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA_FOLDINGOCTIFS, &map_FOLDINGOCTIFS,
        7, {0, 0, 0, 6, 2, 2, 2},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA_TILINGBRICK2IFS, "tilingbrick2IFS", 21, 15,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA_TILINGBRICK2IFS, &map_TILINGBRICK2IFS,
        7, {0, 0, 0, 6, 0, 0, 0},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA_TILINGBRICKIFS, "tilingbrickIFS", 21, 15,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA_TILINGBRICKIFS, &map_TILINGBRICKIFS,
        7, {0, 0, 0, 6, 0, 0, 0},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA_TOTORICALIFS, "totoricalIFS", 21, 11,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA_TOTORICALIFS, &map_TOTORICALIFS,
        9, {0, 0, 0, 0, 0, 0, 0, 2, 0},
        0x0000,
        2, consts_TOTORICALIFS
    });

    RegisterExternalFormula({
        FORMULA_TRANS_QIFS, "trans-qIFS", 21, 14,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA_TRANS_QIFS, &map_TRANS_QIFS,
        6, {0, 0, 0, 6, 0, 0},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA_TRANSLOGSPIFS, "transLogSpIFS", 21, 20,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA_TRANSLOGSPIFS, &map_TRANSLOGSPIFS,
        8, {0, 0, 0, 6, 2, 2, 0, 0},
        0x0000,
        4, consts_TRANSLOGSPIFS
    });

    RegisterExternalFormula({
        FORMULA_TRANSFORM2IFS, "transform2IFS", 21, 15,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA_TRANSFORM2IFS, &map_TRANSFORM2IFS,
        7, {0, 0, 0, 6, 2, 2, 2},
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
