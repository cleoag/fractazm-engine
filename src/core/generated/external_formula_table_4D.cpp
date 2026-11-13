// external_formula_table_4D.cpp — Auto-generated bulk formula registrations
// Category: 4D (41 formulas)
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
// Include 4D formula .inc files
// ============================================================================

#include "4D/Aexion1.inc"
#include "4D/Barnsley2_4Dc.inc"
#include "4D/Barnsley2_4Dnc.inc"
#include "4D/Barnsley4Dc.inc"
#include "4D/Barnsley4Dc2.inc"
#include "4D/Barnsley4Dnc.inc"
#include "4D/Beth1241.inc"
#include "4D/Beth1661dual.inc"
#include "4D/Beth2422.inc"
#include "4D/Beth641.inc"
#include "4D/CubicQuat.inc"
#include "4D/Hopf4D.inc"
#include "4D/Lambda4Dc.inc"
#include "4D/Lambda4Dnc.inc"
#include "4D/Magnet4D.inc"
#include "4D/Makin4D_p2a.inc"
#include "4D/Makin4D_p2b.inc"
#include "4D/Makin4D_p4a.inc"
#include "4D/Makin4D_p4b.inc"
#include "4D/Makin4D_p8a.inc"
#include "4D/Makin4D_p8b.inc"
#include "4D/Mandel4DBiC.inc"
#include "4D/MandyCousin.inc"
#include "4D/MandyCousin2.inc"
#include "4D/MandyCousin3.inc"
#include "4D/PG-bulb.inc"
#include "4D/QuadrayBrot.inc"
#include "4D/QuatPow.inc"
#include "4D/_Atan4D.inc"
#include "4D/_Exp4D.inc"
#include "4D/_Log4D.inc"
#include "4D/_Rpow3_4d.inc"
#include "4D/_Scalingplus.inc"
#include "4D/_Sin4Dplus.inc"
#include "4D/_SphereInvC4d.inc"
#include "4D/_Tanh4D.inc"
#include "4D/_conj4D.inc"
#include "4D/_coordvspow.inc"
#include "4D/_julibrot.inc"
#include "4D/_setw.inc"
#include "4D/hyperd_1.inc"

// ============================================================================
// PVar offset maps (4D)
// ============================================================================

static const FormulaParamMap map_AEXION1 = { 1, { { -0x10, 0 } } };
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA_AEXION1, 4, map_AEXION1)

static const FormulaParamMap map_BARNSLEY2_4DC = {
    8, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 },
        { -0x30, 0 }, { -0x38, 0 }, { -0x40, 0 }, { -0x48, 0 }
    }
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA_BARNSLEY2_4DC, 4, map_BARNSLEY2_4DC)

static const FormulaParamMap map_BARNSLEY2_4DNC = {
    8, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 },
        { -0x30, 0 }, { -0x38, 0 }, { -0x40, 0 }, { -0x48, 0 }
    }
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA_BARNSLEY2_4DNC, 4, map_BARNSLEY2_4DNC)

static const FormulaParamMap map_BARNSLEY4DC = {
    11, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 },
        { -0x30, 0 }, { -0x38, 0 }, { -0x3C, 2 }, { -0x40, 2 },
        { -0x44, 2 }, { 0x00, 0 }, { 0x08, 0 }
    }
};
static const FormulaConstEntry consts_BARNSLEY4DC[] = {
    { 0, UINT64_C(0x3FE0000000000000) },
    { 0, UINT64_C(0x4000000000000000) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA_BARNSLEY4DC, 4, map_BARNSLEY4DC)

static const FormulaParamMap map_BARNSLEY4DC2 = {
    11, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 },
        { -0x30, 0 }, { -0x38, 0 }, { -0x40, 0 }, { -0x44, 2 },
        { -0x48, 2 }, { 0x00, 0 }, { 0x08, 0 }
    }
};
static const FormulaConstEntry consts_BARNSLEY4DC2[] = {
    { 0, UINT64_C(0x3FE0000000000000) },
    { 0, UINT64_C(0x4000000000000000) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA_BARNSLEY4DC2, 4, map_BARNSLEY4DC2)

static const FormulaParamMap map_BARNSLEY4DNC = {
    11, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 },
        { -0x30, 0 }, { -0x38, 0 }, { -0x3C, 2 }, { -0x40, 2 },
        { -0x44, 2 }, { 0x00, 0 }, { 0x08, 0 }
    }
};
static const FormulaConstEntry consts_BARNSLEY4DNC[] = {
    { 0, UINT64_C(0x3FE0000000000000) },
    { 0, UINT64_C(0x4000000000000000) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA_BARNSLEY4DNC, 4, map_BARNSLEY4DNC)

static const FormulaParamMap map_BETH1241 = {
    6, {
        { -0x10, 0 }, { 0x00, 0 }, { 0x08, 0 }, { 0x10, 0 },
        { 0x18, 0 }, { 0x20, 0 }
    }
};
static const FormulaConstEntry consts_BETH1241[] = {
    { 0, UINT64_C(0xBFE0000000000000) },
    { 0, UINT64_C(0x4008000000000000) },
    { 0, UINT64_C(0x4024000000000000) },
    { 0, UINT64_C(0x402E000000000000) },
    { 0, UINT64_C(0x403E000000000000) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER_EX(FORMULA_BETH1241, 4, map_BETH1241, 0, 0)

static const FormulaParamMap map_BETH1661DUAL = {
    7, {
        { -0x10, 0 }, { 0x00, 0 }, { 0x08, 0 }, { 0x10, 0 },
        { 0x18, 0 }, { 0x20, 0 }, { 0x28, 0 }
    }
};
static const FormulaConstEntry consts_BETH1661DUAL[] = {
    { 0, UINT64_C(0xBFD948B0FCD6E9E0) },
    { 0, UINT64_C(0xC045000000000000) },
    { 0, UINT64_C(0xC051800000000000) },
    { 0, UINT64_C(0x403C000000000000) },
    { 0, UINT64_C(0xC05A400000000000) },
    { 0, UINT64_C(0x4018000000000000) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER_EX(FORMULA_BETH1661DUAL, 4, map_BETH1661DUAL, 0, 0)

static const FormulaParamMap map_BETH2422 = {
    6, {
        { -0x10, 0 }, { 0x00, 0 }, { 0x08, 0 }, { 0x10, 0 },
        { 0x18, 0 }, { 0x20, 0 }
    }
};
static const FormulaConstEntry consts_BETH2422[] = {
    { 0, UINT64_C(0x4004000000000000) },
    { 0, UINT64_C(0x4008000000000000) },
    { 0, UINT64_C(0xC024000000000000) },
    { 0, UINT64_C(0x401C000000000000) },
    { 0, UINT64_C(0xC018000000000000) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER_EX(FORMULA_BETH2422, 4, map_BETH2422, 0, 0)

static const FormulaParamMap map_BETH641 = { 4, { { -0x10, 0 }, { 0x00, 0 }, { 0x08, 0 }, { 0x10, 0 } } };
static const FormulaConstEntry consts_BETH641[] = {
    { 0, UINT64_C(0x4008000000000000) },
    { 0, UINT64_C(0x4014000000000000) },
    { 0, UINT64_C(0xC024000000000000) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER_EX(FORMULA_BETH641, 4, map_BETH641, 0, 0)

static const FormulaParamMap map_CUBICQUAT = {
    7, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 },
        { -0x30, 0 }, { -0x38, 0 }, { -0x40, 0 }
    }
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA_CUBICQUAT, 4, map_CUBICQUAT)

static const FormulaParamMap map_HOPF4D = { 2, { { -0x10, 0 }, { -0x18, 0 } } };
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA_HOPF4D, 4, map_HOPF4D)

static const FormulaParamMap map_LAMBDA4DC = { 4, { { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 } } };
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA_LAMBDA4DC, 4, map_LAMBDA4DC)

static const FormulaParamMap map_LAMBDA4DNC = { 4, { { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 } } };
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA_LAMBDA4DNC, 4, map_LAMBDA4DNC)

static const FormulaParamMap map_MAGNET4D = {
    6, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 },
        { -0x2C, 2 }, { 0x00, 0 }
    }
};
static const FormulaConstEntry consts_MAGNET4D[] = {
    { 0, UINT64_C(0x4000000000000000) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA_MAGNET4D, 4, map_MAGNET4D)

static const FormulaParamMap map_MAKIN4D_P2A = { 1, { { -0x10, 0 } } };
DEFINE_EXTERNAL_FORMULA_WRAPPER_EX(FORMULA_MAKIN4D_P2A, 4, map_MAKIN4D_P2A, 0, 0)

static const FormulaParamMap map_MAKIN4D_P2B = { 1, { { -0x10, 0 } } };
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA_MAKIN4D_P2B, 4, map_MAKIN4D_P2B)

static const FormulaParamMap map_MAKIN4D_P4A = { 1, { { -0x10, 0 } } };
DEFINE_EXTERNAL_FORMULA_WRAPPER_EX(FORMULA_MAKIN4D_P4A, 4, map_MAKIN4D_P4A, 0, 0)

static const FormulaParamMap map_MAKIN4D_P4B = { 1, { { -0x10, 0 } } };
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA_MAKIN4D_P4B, 4, map_MAKIN4D_P4B)

static const FormulaParamMap map_MAKIN4D_P8A = { 1, { { -0x10, 0 } } };
DEFINE_EXTERNAL_FORMULA_WRAPPER_EX(FORMULA_MAKIN4D_P8A, 4, map_MAKIN4D_P8A, 0, 0)

static const FormulaParamMap map_MAKIN4D_P8B = { 2, { { -0x10, 0 }, { -0x18, 0 } } };
DEFINE_EXTERNAL_FORMULA_WRAPPER_EX(FORMULA_MAKIN4D_P8B, 4, map_MAKIN4D_P8B, 0, 0)

static const FormulaParamMap map_MANDEL4DBIC = { 1, { { -0x10, 0 } } };
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA_MANDEL4DBIC, 4, map_MANDEL4DBIC)

static const FormulaParamMap map_MANDYCOUSIN = {
    6, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 },
        { -0x30, 0 }, { -0x38, 0 }
    }
};
DEFINE_EXTERNAL_FORMULA_WRAPPER_EX(FORMULA_MANDYCOUSIN, 4, map_MANDYCOUSIN, 0, 0)

static const FormulaParamMap map_MANDYCOUSIN2 = {
    6, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 },
        { -0x30, 0 }, { -0x38, 0 }
    }
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA_MANDYCOUSIN2, 4, map_MANDYCOUSIN2)

static const FormulaParamMap map_MANDYCOUSIN3 = {
    6, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 },
        { -0x30, 0 }, { -0x38, 0 }
    }
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA_MANDYCOUSIN3, 4, map_MANDYCOUSIN3)

static const FormulaParamMap map_PG_BULB = { 3, { { -0x10, 0 }, { -0x14, 2 }, { -0x18, 2 } } };
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA_PG_BULB, 4, map_PG_BULB)

static const FormulaParamMap map_QUADRAYBROT = { 0, {} };
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA_QUADRAYBROT, 4, map_QUADRAYBROT)

static const FormulaParamMap map_QUATPOW = {
    12, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 },
        { -0x30, 0 }, { -0x38, 0 }, { -0x40, 0 }, { -0x48, 0 },
        { -0x50, 0 }, { 0x00, 0 }, { 0x08, 0 }, { 0x10, 0 }
    }
};
static const FormulaConstEntry consts_QUATPOW[] = {
    { 0, UINT64_C(0x16687E92154EF7AC) },
    { 0, UINT64_C(0x3FE0000000000000) },
    { 0, UINT64_C(0x407CC00000000000) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA_QUATPOW, 4, map_QUATPOW)

static const FormulaParamMap map__ATAN4D = {
    8, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 },
        { -0x30, 0 }, { 0x00, 0 }, { 0x08, 0 }, { 0x10, 0 }
    }
};
static const FormulaConstEntry consts__ATAN4D[] = {
    { 0, UINT64_C(0x3FE0000000000000) },
    { 0, UINT64_C(0x3EE4F8B588E368F1) },
    { 0, UINT64_C(0x3FD0000000000000) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__ATAN4D, 4, map__ATAN4D)

static const FormulaParamMap map__EXP4D = { 1, { { 0x00, 0 } } };
static const FormulaConstEntry consts__EXP4D[] = {
    { 0, UINT64_C(0x16687E92154EF7AC) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__EXP4D, 4, map__EXP4D)

static const FormulaParamMap map__LOG4D = { 3, { { 0x00, 0 }, { 0x08, 0 }, { 0x10, 0 } } };
static const FormulaConstEntry consts__LOG4D[] = {
    { 0, UINT64_C(0x16687E92154EF7AC) },
    { 0, UINT64_C(0x3FE0000000000000) },
    { 0, UINT64_C(0x407CC00000000000) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__LOG4D, 4, map__LOG4D)

static const FormulaParamMap map__RPOW3_4D = { 0, {} };
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__RPOW3_4D, 4, map__RPOW3_4D)

static const FormulaParamMap map__SCALINGPLUS = { 4, { { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 } } };
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__SCALINGPLUS, 4, map__SCALINGPLUS)

static const FormulaParamMap map__SIN4DPLUS = {
    7, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 },
        { -0x30, 0 }, { 0x00, 0 }, { 0x08, 0 }
    }
};
static const FormulaConstEntry consts__SIN4DPLUS[] = {
    { 0, UINT64_C(0x3FE0000000000000) },
    { 0, UINT64_C(0x3EE4F8B588E368F1) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__SIN4DPLUS, 4, map__SIN4DPLUS)

static const FormulaParamMap map__SPHEREINVC4D = { 0, {} };
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__SPHEREINVC4D, 4, map__SPHEREINVC4D)

static const FormulaParamMap map_TANH4D = {
    6, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 },
        { -0x30, 0 }, { 0x00, 0 }
    }
};
static const FormulaConstEntry consts_TANH4D[] = {
    { 0, UINT64_C(0x16687E92154EF7AC) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA_TANH4D, 4, map_TANH4D)

static const FormulaParamMap map__CONJ4D = { 0, {} };
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__CONJ4D, 4, map__CONJ4D)

static const FormulaParamMap map_COORDVSPOW = {
    9, {
        { -0x0C, 2 }, { -0x14, 0 }, { -0x18, 2 }, { -0x1C, 2 },
        { -0x20, 2 }, { -0x24, 2 }, { -0x28, 2 }, { 0x00, 0 },
        { 0x08, 0 }
    }
};
static const FormulaConstEntry consts_COORDVSPOW[] = {
    { 0, UINT64_C(0x3F847AE147AE147B) },
    { 0, UINT64_C(0x3FE0000000000000) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA_COORDVSPOW, 4, map_COORDVSPOW)

static const FormulaParamMap map__JULIBROT = { 4, { { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 } } };
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__JULIBROT, 4, map__JULIBROT)

static const FormulaParamMap map_SETW = {
    5, {
        { -0x0C, 2 }, { -0x14, 0 }, { -0x1C, 0 }, { -0x24, 0 },
        { -0x2C, 0 }
    }
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA_SETW, 4, map_SETW)

static const FormulaParamMap map_HYPERD_1 = {
    5, {
        { -0x10, 0 }, { -0x14, 2 }, { -0x1C, 0 }, { -0x24, 0 },
        { -0x2C, 0 }
    }
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA_HYPERD_1, 4, map_HYPERD_1)

// ============================================================================
// Registration: RegisterExternalFormulas_4D()
// ============================================================================

void RegisterExternalFormulas_4D() {
    RegisterExternalFormula({
        FORMULA_AEXION1, "Aexion1", 4, 1,
        1.0, 1.0, 2.0, 100.0,
        ExternWrap_FORMULA_AEXION1, &map_AEXION1,
        1, {0},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA_BARNSLEY2_4DC, "Barnsley2_4Dc", 4, 8,
        1.0, 1.0, 2.0, 100.0,
        ExternWrap_FORMULA_BARNSLEY2_4DC, &map_BARNSLEY2_4DC,
        8, {0, 0, 0, 0, 0, 0, 0, 0},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA_BARNSLEY2_4DNC, "Barnsley2_4Dnc", 4, 8,
        1.0, 1.0, 2.0, 100.0,
        ExternWrap_FORMULA_BARNSLEY2_4DNC, &map_BARNSLEY2_4DNC,
        8, {0, 0, 0, 0, 0, 0, 0, 0},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA_BARNSLEY4DC, "Barnsley4Dc", 4, 11,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA_BARNSLEY4DC, &map_BARNSLEY4DC,
        9, {0, 0, 0, 0, 0, 0, 2, 2, 2},
        0x0000,
        2, consts_BARNSLEY4DC
    });

    RegisterExternalFormula({
        FORMULA_BARNSLEY4DC2, "Barnsley4Dc2", 4, 11,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA_BARNSLEY4DC2, &map_BARNSLEY4DC2,
        9, {0, 0, 0, 0, 0, 0, 0, 2, 2},
        0x0000,
        2, consts_BARNSLEY4DC2
    });

    RegisterExternalFormula({
        FORMULA_BARNSLEY4DNC, "Barnsley4Dnc", 4, 11,
        1.0, 1.0, 2.0, 100.0,
        ExternWrap_FORMULA_BARNSLEY4DNC, &map_BARNSLEY4DNC,
        9, {0, 0, 0, 0, 0, 0, 2, 2, 2},
        0x0000,
        2, consts_BARNSLEY4DNC
    });

    RegisterExternalFormula({
        FORMULA_BETH1241, "Beth1241", 4, 6,
        1.0, 1.0, 4.0, 100.0,
        ExternWrap_FORMULA_BETH1241, &map_BETH1241,
        1, {0},
        0x0000,
        5, consts_BETH1241
    });

    RegisterExternalFormula({
        FORMULA_BETH1661DUAL, "Beth1661dual", 4, 7,
        1.0, 1.0, 6.0, 100.0,
        ExternWrap_FORMULA_BETH1661DUAL, &map_BETH1661DUAL,
        1, {0},
        0x0000,
        6, consts_BETH1661DUAL
    });

    RegisterExternalFormula({
        FORMULA_BETH2422, "Beth2422", 4, 6,
        1.0, 1.0, 4.0, 100.0,
        ExternWrap_FORMULA_BETH2422, &map_BETH2422,
        1, {0},
        0x0000,
        5, consts_BETH2422
    });

    RegisterExternalFormula({
        FORMULA_BETH641, "Beth641", 4, 4,
        1.0, 1.0, 4.0, 100.0,
        ExternWrap_FORMULA_BETH641, &map_BETH641,
        1, {0},
        0x0000,
        3, consts_BETH641
    });

    RegisterExternalFormula({
        FORMULA_CUBICQUAT, "CubicQuat", 4, 7,
        1.0, 1.0, 2.0, 100.0,
        ExternWrap_FORMULA_CUBICQUAT, &map_CUBICQUAT,
        7, {0, 0, 0, 0, 0, 0, 0},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA_HOPF4D, "Hopf4D", 4, 2,
        1.0, 1.0, 2.0, 100.0,
        ExternWrap_FORMULA_HOPF4D, &map_HOPF4D,
        2, {0, 0},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA_LAMBDA4DC, "Lambda4Dc", 4, 4,
        1.0, 1.0, 2.0, 100.0,
        ExternWrap_FORMULA_LAMBDA4DC, &map_LAMBDA4DC,
        4, {0, 0, 0, 0},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA_LAMBDA4DNC, "Lambda4Dnc", 4, 4,
        1.0, 1.0, 2.0, 100.0,
        ExternWrap_FORMULA_LAMBDA4DNC, &map_LAMBDA4DNC,
        4, {0, 0, 0, 0},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA_MAGNET4D, "Magnet4D", 4, 6,
        1.0, 1.0, 2.0, 100.0,
        ExternWrap_FORMULA_MAGNET4D, &map_MAGNET4D,
        5, {0, 0, 0, 0, 2},
        0x0000,
        1, consts_MAGNET4D
    });

    RegisterExternalFormula({
        FORMULA_MAKIN4D_P2A, "Makin4D p2a", 4, 1,
        1.0, 1.0, 2.0, 100.0,
        ExternWrap_FORMULA_MAKIN4D_P2A, &map_MAKIN4D_P2A,
        1, {0},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA_MAKIN4D_P2B, "Makin4D p2b", 4, 1,
        1.0, 1.0, 2.0, 100.0,
        ExternWrap_FORMULA_MAKIN4D_P2B, &map_MAKIN4D_P2B,
        1, {0},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA_MAKIN4D_P4A, "Makin4D p4a", 4, 1,
        1.0, 1.0, 2.0, 100.0,
        ExternWrap_FORMULA_MAKIN4D_P4A, &map_MAKIN4D_P4A,
        1, {0},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA_MAKIN4D_P4B, "Makin4D p4b", 4, 1,
        1.0, 1.0, 2.0, 100.0,
        ExternWrap_FORMULA_MAKIN4D_P4B, &map_MAKIN4D_P4B,
        1, {0},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA_MAKIN4D_P8A, "Makin4D p8a", 4, 1,
        1.0, 1.0, 2.0, 100.0,
        ExternWrap_FORMULA_MAKIN4D_P8A, &map_MAKIN4D_P8A,
        1, {0},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA_MAKIN4D_P8B, "Makin4D p8b", 4, 2,
        1.0, 1.0, 2.0, 100.0,
        ExternWrap_FORMULA_MAKIN4D_P8B, &map_MAKIN4D_P8B,
        2, {0, 0},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA_MANDEL4DBIC, "Mandel4DBiC", 4, 1,
        1.0, 1.0, 2.0, 100.0,
        ExternWrap_FORMULA_MANDEL4DBIC, &map_MANDEL4DBIC,
        1, {0},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA_MANDYCOUSIN, "MandyCousin", 4, 6,
        1.0, 1.0, 2.0, 100.0,
        ExternWrap_FORMULA_MANDYCOUSIN, &map_MANDYCOUSIN,
        6, {0, 0, 0, 0, 0, 0},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA_MANDYCOUSIN2, "MandyCousin2", 4, 6,
        1.0, 1.0, 2.0, 100.0,
        ExternWrap_FORMULA_MANDYCOUSIN2, &map_MANDYCOUSIN2,
        6, {0, 0, 0, 0, 0, 0},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA_MANDYCOUSIN3, "MandyCousin3", 4, 6,
        1.0, 1.0, 2.0, 100.0,
        ExternWrap_FORMULA_MANDYCOUSIN3, &map_MANDYCOUSIN3,
        6, {0, 0, 0, 0, 0, 0},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA_PG_BULB, "PG-bulb", 4, 3,
        1.0, 1.0, 2.0, 100.0,
        ExternWrap_FORMULA_PG_BULB, &map_PG_BULB,
        3, {0, 2, 2},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA_QUADRAYBROT, "QuadrayBrot", 4, 0,
        1.0, 1.0, 2.0, 100.0,
        ExternWrap_FORMULA_QUADRAYBROT, &map_QUADRAYBROT,
        0, {},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA_QUATPOW, "QuatPow", 4, 12,
        1.0, 1.0, 2.0, 100.0,
        ExternWrap_FORMULA_QUATPOW, &map_QUATPOW,
        9, {0, 0, 0, 0, 0, 0, 0, 0, 0},
        0x0000,
        3, consts_QUATPOW
    });

    RegisterExternalFormula({
        FORMULA__ATAN4D, "_Atan4D", 4, 8,
        1.0, 1.0, 4.0, 100.0,
        ExternWrap_FORMULA__ATAN4D, &map__ATAN4D,
        5, {0, 0, 0, 0, 0},
        0x0000,
        3, consts__ATAN4D
    });

    RegisterExternalFormula({
        FORMULA__EXP4D, "_Exp4D", 4, 1,
        1.0, 1.0, 4.0, 100.0,
        ExternWrap_FORMULA__EXP4D, &map__EXP4D,
        0, {},
        0x0000,
        1, consts__EXP4D
    });

    RegisterExternalFormula({
        FORMULA__LOG4D, "_Log4D", 4, 3,
        1.0, 1.0, 4.0, 100.0,
        ExternWrap_FORMULA__LOG4D, &map__LOG4D,
        0, {},
        0x0000,
        3, consts__LOG4D
    });

    RegisterExternalFormula({
        FORMULA__RPOW3_4D, "_Rpow3_4d", 4, 0,
        1.0, 1.0, 3.0, 16.0,
        ExternWrap_FORMULA__RPOW3_4D, &map__RPOW3_4D,
        0, {},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA__SCALINGPLUS, "_Scalingplus", 4, 4,
        1.0, 1.0, 1.0, 100.0,
        ExternWrap_FORMULA__SCALINGPLUS, &map__SCALINGPLUS,
        4, {0, 0, 0, 0},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA__SIN4DPLUS, "_Sin4Dplus", 4, 7,
        1.0, 1.0, 4.0, 100.0,
        ExternWrap_FORMULA__SIN4DPLUS, &map__SIN4DPLUS,
        5, {0, 0, 0, 0, 0},
        0x0000,
        2, consts__SIN4DPLUS
    });

    RegisterExternalFormula({
        FORMULA__SPHEREINVC4D, "_SphereInvC4d", 4, 0,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA__SPHEREINVC4D, &map__SPHEREINVC4D,
        0, {},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA_TANH4D, "_Tanh4D", 4, 6,
        1.0, 1.0, 4.0, 100.0,
        ExternWrap_FORMULA_TANH4D, &map_TANH4D,
        5, {0, 0, 0, 0, 0},
        0x0000,
        1, consts_TANH4D
    });

    RegisterExternalFormula({
        FORMULA__CONJ4D, "_conj4D", 4, 0,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA__CONJ4D, &map__CONJ4D,
        0, {},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA_COORDVSPOW, "_coordvspow", 4, 9,
        1.0, 1.0, 2.0, 100.0,
        ExternWrap_FORMULA_COORDVSPOW, &map_COORDVSPOW,
        7, {2, 0, 2, 2, 2, 2, 2},
        0x0000,
        2, consts_COORDVSPOW
    });

    RegisterExternalFormula({
        FORMULA__JULIBROT, "_julibrot", 4, 4,
        1.0, 1.0, 2.0, 100.0,
        ExternWrap_FORMULA__JULIBROT, &map__JULIBROT,
        4, {0, 0, 0, 0},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA_SETW, "_setw", 4, 5,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA_SETW, &map_SETW,
        5, {2, 0, 0, 0, 0},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA_HYPERD_1, "hyperd_1", 4, 5,
        1.0, 1.0, 2.0, 100.0,
        ExternWrap_FORMULA_HYPERD_1, &map_HYPERD_1,
        5, {0, 2, 0, 0, 0},
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
