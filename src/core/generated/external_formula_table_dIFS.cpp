// external_formula_table_dIFS.cpp — Auto-generated bulk formula registrations
// Category: dIFS (82 formulas)
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
// Include dIFS formula .inc files
// ============================================================================

#include "dIFS/CitrusIFS.inc"
#include "dIFS/CommaIFS.inc"
#include "dIFS/CylinderHMVoid.inc"
#include "dIFS/CylinderHeightMap.inc"
#include "dIFS/DecoCubeIFS.inc"
#include "dIFS/DevilIFS.inc"
#include "dIFS/HeightMap2IFS.inc"
#include "dIFS/HeightMapIFS.inc"
#include "dIFS/KleinBagelIFS.inc"
#include "dIFS/KleinBotTIFS.inc"
#include "dIFS/Knots2IFS.inc"
#include "dIFS/KnotsIFS.inc"
#include "dIFS/LimpetIFS.inc"
#include "dIFS/MitreIFS.inc"
#include "dIFS/NewFoldCutIFS.inc"
#include "dIFS/NewMenger2IFS.inc"
#include "dIFS/NewMengerIFS.inc"
#include "dIFS/PSphereIFS.inc"
#include "dIFS/PenN-gonIFS.inc"
#include "dIFS/PiriformIFS.inc"
#include "dIFS/PolyPyraIFS.inc"
#include "dIFS/QCylIFS.inc"
#include "dIFS/QuadricsIFS.inc"
#include "dIFS/RandCubesIFS.inc"
#include "dIFS/RiemSqrtIFS.inc"
#include "dIFS/RoomDoorsIFS.inc"
#include "dIFS/RoundedBox.inc"
#include "dIFS/ScarabIFS.inc"
#include "dIFS/ScherkFS.inc"
#include "dIFS/Seashell.inc"
#include "dIFS/SphCageIFS.inc"
#include "dIFS/SphereHMVoid.inc"
#include "dIFS/SphereHeightMap.inc"
#include "dIFS/TorusEllipIFS.inc"
#include "dIFS/TorusHMVoid.inc"
#include "dIFS/TorusIFS.inc"
#include "dIFS/UmbrellaIFS.inc"
#include "dIFS/WeirdCubeIFS.inc"
#include "dIFS/YinYangIFS.inc"
#include "dIFS/boardIFS.inc"
#include "dIFS/borgIFS.inc"
#include "dIFS/cayleySurfIFS.inc"
#include "dIFS/columnsIFS.inc"
#include "dIFS/cubetubeIFS.inc"
#include "dIFS/cylinderIFS.inc"
#include "dIFS/diniIFS.inc"
#include "dIFS/discoballIFS.inc"
#include "dIFS/dodecatubeIFS.inc"
#include "dIFS/ducksIFS.inc"
#include "dIFS/foldcut12-IFS.inc"
#include "dIFS/foldcut8-IFS.inc"
#include "dIFS/gear2IFS.inc"
#include "dIFS/gnarly2IFS.inc"
#include "dIFS/gnarly3IFS.inc"
#include "dIFS/gnarlyIFS.inc"
#include "dIFS/greekIFS.inc"
#include "dIFS/gumdropIFS.inc"
#include "dIFS/gyroidIFS.inc"
#include "dIFS/heartKluchIFS.inc"
#include "dIFS/helistairsIFS.inc"
#include "dIFS/helixIFS.inc"
#include "dIFS/hexgridIFS.inc"
#include "dIFS/hextgrid2IFS.inc"
#include "dIFS/hextgrid3IFS.inc"
#include "dIFS/hextgrid4IFS.inc"
#include "dIFS/hextgridIFS.inc"
#include "dIFS/lidinoidIFS.inc"
#include "dIFS/logspIFS.inc"
#include "dIFS/loxodrome2IFS.inc"
#include "dIFS/loxodromeIFS.inc"
#include "dIFS/pyramIFS.inc"
#include "dIFS/roomclrIFS.inc"
#include "dIFS/schwartzIFS.inc"
#include "dIFS/sliceExIFS.inc"
#include "dIFS/sliceIFS.inc"
#include "dIFS/tetratubeIFS.inc"
#include "dIFS/toupieIFS.inc"
#include "dIFS/trifoliumIFS.inc"
#include "dIFS/tritgrid2IFS.inc"
#include "dIFS/tritgridIFS.inc"
#include "dIFS/waveIFS.inc"
#include "dIFS/wavespIFS.inc"

// ============================================================================
// PVar offset maps (dIFS)
// ============================================================================

static const FormulaParamMap map_CITRUSIFS = {
    17, {
        { -0x10, 0 }, { -0x20, 0 }, { -0x30, 0 }, { -0x38, 0 },
        { -0x40, 0 }, { -0x48, 0 }, { -0x4C, 2 }, { -0x50, 1 },
        { -0x54, 1 }, { -0x58, 1 }, { -0x5C, 1 }, { -0x60, 1 },
        { -0x64, 1 }, { -0x68, 1 }, { -0x6C, 1 }, { -0x70, 1 },
        { 0x00, 0 }
    }
};
static const FormulaConstEntry consts_CITRUSIFS[] = {
    { 0, UINT64_C(0x3FF0000000000000) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER_EX(FORMULA_CITRUSIFS, 20, map_CITRUSIFS, 1, 1)

static const FormulaParamMap map_COMMAIFS = {
    15, {
        { -0x10, 0 }, { -0x20, 0 }, { -0x30, 0 }, { -0x38, 0 },
        { -0x40, 0 }, { -0x48, 0 }, { -0x4C, 2 }, { -0x54, 0 },
        { -0x5C, 0 }, { -0x60, 2 }, { -0x64, 2 }, { -0x68, 2 },
        { 0x00, 0 }, { 0x08, 0 }, { 0x10, 0 }
    }
};
static const FormulaConstEntry consts_COMMAIFS[] = {
    { 0, UINT64_C(0x3FC45F30E7FF583A) },
    { 0, UINT64_C(0x3FE0000000000000) },
    { 0, UINT64_C(0x0000000000000000) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER_EX(FORMULA_COMMAIFS, 20, map_COMMAIFS, 1, 1)

static const FormulaParamMap map_CYLINDERHMVOID = {
    23, {
        { -0x0C, 2 }, { -0x10, 2 }, { -0x14, 1 }, { -0x18, 1 },
        { -0x1C, 1 }, { -0x20, 1 }, { -0x24, 1 }, { -0x28, 1 },
        { -0x2C, 1 }, { -0x30, 1 }, { -0x34, 1 }, { -0x38, 1 },
        { -0x3C, 1 }, { -0x40, 1 }, { -0x44, 1 }, { -0x48, 1 },
        { -0x4C, 1 }, { -0x50, 2 }, { -0x54, 1 }, { -0x58, 1 },
        { -0x5C, 1 }, { 0x00, 0 }, { 0x08, 0 }
    }
};
static const FormulaConstEntry consts_CYLINDERHMVOID[] = {
    { 0, UINT64_C(0x3FC45F306DC9C883) },
    { 0, UINT64_C(0x3F847AE147AE147B) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER_EX(FORMULA_CYLINDERHMVOID, 20, map_CYLINDERHMVOID, 1, 1)

static const FormulaParamMap map_CYLINDERHEIGHTMAP = {
    21, {
        { -0x0C, 2 }, { -0x10, 2 }, { -0x14, 1 }, { -0x18, 1 },
        { -0x1C, 1 }, { -0x20, 1 }, { -0x24, 1 }, { -0x28, 1 },
        { -0x2C, 1 }, { -0x30, 1 }, { -0x34, 1 }, { -0x38, 1 },
        { -0x3C, 1 }, { -0x40, 1 }, { -0x44, 1 }, { -0x48, 1 },
        { -0x4C, 1 }, { -0x50, 2 }, { -0x54, 1 }, { -0x58, 1 },
        { 0x00, 0 }
    }
};
static const FormulaConstEntry consts_CYLINDERHEIGHTMAP[] = {
    { 0, UINT64_C(0x3FC45F306DC9C883) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER_EX(FORMULA_CYLINDERHEIGHTMAP, 20, map_CYLINDERHEIGHTMAP, 1, 1)

static const FormulaParamMap map_DECOCUBEIFS = {
    9, {
        { -0x10, 0 }, { -0x20, 0 }, { -0x30, 0 }, { -0x38, 0 },
        { -0x40, 0 }, { -0x48, 0 }, { -0x4C, 2 }, { -0x54, 0 },
        { 0x00, 0 }
    }
};
static const FormulaConstEntry consts_DECOCUBEIFS[] = {
    { 0, UINT64_C(0x3FF0000000000000) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER_EX(FORMULA_DECOCUBEIFS, 20, map_DECOCUBEIFS, 1, 1)

static const FormulaParamMap map_DEVILIFS = {
    19, {
        { -0x10, 0 }, { -0x20, 0 }, { -0x30, 0 }, { -0x38, 0 },
        { -0x40, 0 }, { -0x48, 0 }, { -0x4C, 2 }, { -0x50, 1 },
        { -0x54, 1 }, { -0x58, 1 }, { -0x5C, 1 }, { -0x60, 1 },
        { -0x64, 1 }, { -0x68, 1 }, { -0x6C, 1 }, { -0x70, 1 },
        { 0x00, 0 }, { 0x08, 0 }, { 0x10, 0 }
    }
};
static const FormulaConstEntry consts_DEVILIFS[] = {
    { 0, UINT64_C(0x3FD70A3D70A3D70A) },
    { 0, UINT64_C(0x3FD0000000000000) },
    { 0, UINT64_C(0x4000000000000000) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER_EX(FORMULA_DEVILIFS, 20, map_DEVILIFS, 1, 1)

static const FormulaParamMap map_HEIGHTMAP2IFS = {
    23, {
        { -0x0C, 2 }, { -0x10, 1 }, { -0x14, 1 }, { -0x18, 1 },
        { -0x1C, 1 }, { -0x20, 1 }, { -0x24, 1 }, { -0x28, 1 },
        { -0x2C, 1 }, { -0x30, 1 }, { -0x34, 1 }, { -0x38, 1 },
        { -0x3C, 1 }, { -0x40, 1 }, { -0x44, 1 }, { -0x48, 1 },
        { -0x4C, 2 }, { -0x50, 2 }, { -0x54, 1 }, { -0x58, 1 },
        { -0x5C, 2 }, { 0x00, 0 }, { 0x08, 0 }
    }
};
static const FormulaConstEntry consts_HEIGHTMAP2IFS[] = {
    { 0, UINT64_C(0x3FF0000000000000) },
    { 0, UINT64_C(0x3BC79CA10C924223) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER_EX(FORMULA_HEIGHTMAP2IFS, 20, map_HEIGHTMAP2IFS, 1, 1)

static const FormulaParamMap map_HEIGHTMAPIFS = {
    20, {
        { -0x0C, 2 }, { -0x10, 1 }, { -0x14, 1 }, { -0x18, 1 },
        { -0x1C, 1 }, { -0x20, 1 }, { -0x24, 1 }, { -0x28, 1 },
        { -0x2C, 1 }, { -0x30, 1 }, { -0x34, 1 }, { -0x38, 1 },
        { -0x3C, 1 }, { -0x40, 1 }, { -0x44, 1 }, { -0x48, 1 },
        { -0x4C, 2 }, { -0x50, 2 }, { -0x54, 1 }, { -0x58, 1 }
    }
};
DEFINE_EXTERNAL_FORMULA_WRAPPER_EX(FORMULA_HEIGHTMAPIFS, 20, map_HEIGHTMAPIFS, 1, 1)

static const FormulaParamMap map_KLEINBAGELIFS = {
    11, {
        { -0x10, 0 }, { -0x20, 0 }, { -0x30, 0 }, { -0x38, 0 },
        { -0x40, 0 }, { -0x48, 0 }, { -0x4C, 2 }, { -0x50, 2 },
        { -0x54, 2 }, { -0x5C, 0 }, { 0x00, 0 }
    }
};
static const FormulaConstEntry consts_KLEINBAGELIFS[] = {
    { 0, UINT64_C(0x3FE0000000000000) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER_EX(FORMULA_KLEINBAGELIFS, 20, map_KLEINBAGELIFS, 1, 1)

static const FormulaParamMap map_KLEINBOTTIFS = {
    13, {
        { -0x10, 0 }, { -0x20, 0 }, { -0x30, 0 }, { -0x38, 0 },
        { -0x40, 0 }, { -0x48, 0 }, { -0x4C, 2 }, { -0x54, 0 },
        { 0x00, 0 }, { 0x08, 0 }, { 0x10, 0 }, { 0x18, 0 },
        { 0x20, 0 }
    }
};
static const FormulaConstEntry consts_KLEINBOTTIFS[] = {
    { 0, UINT64_C(0x3FF0000000000000) },
    { 0, UINT64_C(0x4000000000000000) },
    { 0, UINT64_C(0x3FA999999999999A) },
    { 0, UINT64_C(0x3FE0000000000000) },
    { 0, UINT64_C(0x3FD45F06F6944674) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA_KLEINBOTTIFS, 20, map_KLEINBOTTIFS)

static const FormulaParamMap map_KNOTS2IFS = {
    16, {
        { -0x10, 0 }, { -0x20, 0 }, { -0x30, 0 }, { -0x38, 0 },
        { -0x40, 0 }, { -0x48, 0 }, { -0x4C, 2 }, { -0x50, 2 },
        { -0x54, 2 }, { -0x58, 2 }, { -0x60, 0 }, { -0x64, 2 },
        { 0x00, 0 }, { 0x08, 0 }, { 0x10, 0 }, { 0x18, 0 }
    }
};
static const FormulaConstEntry consts_KNOTS2IFS[] = {
    { 0, UINT64_C(0x3FC45F306D974364) },
    { 0, UINT64_C(0x401921FB54411744) },
    { 0, UINT64_C(0x4202A05F20000000) },
    { 0, UINT64_C(0x3FF921FB54411744) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER_EX(FORMULA_KNOTS2IFS, 20, map_KNOTS2IFS, 1, 1)

static const FormulaParamMap map_KNOTSIFS = {
    14, {
        { -0x10, 0 }, { -0x20, 0 }, { -0x30, 0 }, { -0x38, 0 },
        { -0x40, 0 }, { -0x48, 0 }, { -0x4C, 2 }, { -0x50, 2 },
        { -0x54, 2 }, { -0x58, 2 }, { -0x60, 0 }, { 0x00, 0 },
        { 0x08, 0 }, { 0x10, 0 }
    }
};
static const FormulaConstEntry consts_KNOTSIFS[] = {
    { 0, UINT64_C(0x3FC45F306D974364) },
    { 0, UINT64_C(0xC01921FB54411744) },
    { 0, UINT64_C(0x3FF921FB54411744) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA_KNOTSIFS, 20, map_KNOTSIFS)

static const FormulaParamMap map_LIMPETIFS = {
    17, {
        { -0x10, 0 }, { -0x20, 0 }, { -0x30, 0 }, { -0x38, 0 },
        { -0x40, 0 }, { -0x48, 0 }, { -0x4C, 2 }, { -0x54, 0 },
        { -0x58, 1 }, { -0x5C, 1 }, { -0x60, 1 }, { -0x64, 1 },
        { -0x68, 1 }, { -0x6C, 1 }, { -0x70, 1 }, { -0x74, 1 },
        { -0x78, 1 }
    }
};
DEFINE_EXTERNAL_FORMULA_WRAPPER_EX(FORMULA_LIMPETIFS, 20, map_LIMPETIFS, 1, 1)

static const FormulaParamMap map_MITREIFS = {
    17, {
        { -0x10, 0 }, { -0x20, 0 }, { -0x30, 0 }, { -0x38, 0 },
        { -0x40, 0 }, { -0x48, 0 }, { -0x4C, 2 }, { -0x50, 1 },
        { -0x54, 1 }, { -0x58, 1 }, { -0x5C, 1 }, { -0x60, 1 },
        { -0x64, 1 }, { -0x68, 1 }, { -0x6C, 1 }, { -0x70, 1 },
        { 0x00, 0 }
    }
};
static const FormulaConstEntry consts_MITREIFS[] = {
    { 0, UINT64_C(0x3FD3333333333333) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER_EX(FORMULA_MITREIFS, 20, map_MITREIFS, 1, 1)

static const FormulaParamMap map_NEWFOLDCUTIFS = {
    22, {
        { -0x10, 0 }, { -0x20, 0 }, { -0x30, 0 }, { -0x38, 0 },
        { -0x40, 0 }, { -0x44, 1 }, { -0x48, 1 }, { -0x4C, 2 },
        { -0x54, 0 }, { -0x5C, 0 }, { -0x64, 0 }, { -0x68, 2 },
        { -0x6C, 1 }, { -0x70, 1 }, { -0x74, 1 }, { -0x78, 1 },
        { -0x7C, 1 }, { -0x80, 1 }, { -0x84, 1 }, { -0x88, 1 },
        { -0x8C, 1 }, { 0x00, 0 }
    }
};
static const FormulaConstEntry consts_NEWFOLDCUTIFS[] = {
    { 0, UINT64_C(0x409F400000000000) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER_EX(FORMULA_NEWFOLDCUTIFS, 20, map_NEWFOLDCUTIFS, 1, 1)

static const FormulaParamMap map_NEWMENGER2IFS = {
    24, {
        { -0x10, 0 }, { -0x20, 0 }, { -0x30, 0 }, { -0x38, 0 },
        { -0x40, 0 }, { -0x48, 0 }, { -0x4C, 2 }, { -0x54, 0 },
        { -0x5C, 0 }, { -0x64, 0 }, { -0x68, 2 }, { -0x6C, 1 },
        { -0x70, 1 }, { -0x74, 1 }, { -0x78, 1 }, { -0x7C, 1 },
        { -0x80, 1 }, { -0x84, 1 }, { -0x88, 1 }, { -0x8C, 1 },
        { -0x94, 0 }, { 0x00, 0 }, { 0x08, 0 }, { 0x10, 0 }
    }
};
static const FormulaConstEntry consts_NEWMENGER2IFS[] = {
    { 0, UINT64_C(0x3FE0000000000000) },
    { 0, UINT64_C(0x4008000000000000) },
    { 0, UINT64_C(0x3FF8000000000000) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER_EX(FORMULA_NEWMENGER2IFS, 20, map_NEWMENGER2IFS, 1, 1)

static const FormulaParamMap map_NEWMENGERIFS = {
    24, {
        { -0x10, 0 }, { -0x20, 0 }, { -0x30, 0 }, { -0x38, 0 },
        { -0x40, 0 }, { -0x48, 0 }, { -0x4C, 2 }, { -0x54, 0 },
        { -0x5C, 0 }, { -0x64, 0 }, { -0x68, 2 }, { -0x6C, 1 },
        { -0x70, 1 }, { -0x74, 1 }, { -0x78, 1 }, { -0x7C, 1 },
        { -0x80, 1 }, { -0x84, 1 }, { -0x88, 1 }, { -0x8C, 1 },
        { -0x94, 0 }, { 0x00, 0 }, { 0x08, 0 }, { 0x10, 0 }
    }
};
static const FormulaConstEntry consts_NEWMENGERIFS[] = {
    { 0, UINT64_C(0x3FE0000000000000) },
    { 0, UINT64_C(0x4008000000000000) },
    { 0, UINT64_C(0x3FF8000000000000) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER_EX(FORMULA_NEWMENGERIFS, 20, map_NEWMENGERIFS, 1, 1)

static const FormulaParamMap map_PSPHEREIFS = {
    13, {
        { -0x10, 0 }, { -0x20, 0 }, { -0x30, 0 }, { -0x38, 0 },
        { -0x40, 0 }, { -0x48, 0 }, { -0x4C, 2 }, { -0x54, 0 },
        { -0x5C, 0 }, { -0x60, 2 }, { -0x64, 2 }, { 0x00, 0 },
        { 0x08, 0 }
    }
};
static const FormulaConstEntry consts_PSPHEREIFS[] = {
    { 0, UINT64_C(0x3FD3333333333333) },
    { 0, UINT64_C(0x3F947AE147AE147B) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER_EX(FORMULA_PSPHEREIFS, 20, map_PSPHEREIFS, 1, 1)

static const FormulaParamMap map_PENN_GONIFS = {
    15, {
        { -0x10, 0 }, { -0x20, 0 }, { -0x30, 0 }, { -0x38, 0 },
        { -0x40, 0 }, { -0x48, 0 }, { -0x4C, 2 }, { -0x50, 2 },
        { -0x54, 2 }, { -0x5C, 0 }, { 0x00, 0 }, { 0x08, 0 },
        { 0x10, 0 }, { 0x18, 0 }, { 0x20, 0 }
    }
};
static const FormulaConstEntry consts_PENN_GONIFS[] = {
    { 0, UINT64_C(0x3FC45F306D974364) },
    { 0, UINT64_C(0x401921FB54411744) },
    { 0, UINT64_C(0x3FF921FB54411744) },
    { 0, UINT64_C(0x3FD0000000000000) },
    { 0, UINT64_C(0x4014000000000000) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER_EX(FORMULA_PENN_GONIFS, 20, map_PENN_GONIFS, 1, 1)

static const FormulaParamMap map_PIRIFORMIFS = {
    12, {
        { -0x10, 0 }, { -0x20, 0 }, { -0x30, 0 }, { -0x38, 0 },
        { -0x40, 0 }, { -0x48, 0 }, { -0x4C, 2 }, { -0x50, 2 },
        { -0x58, 0 }, { -0x5C, 2 }, { -0x60, 2 }, { 0x00, 0 }
    }
};
static const FormulaConstEntry consts_PIRIFORMIFS[] = {
    { 0, UINT64_C(0x3FE0000000000000) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER_EX(FORMULA_PIRIFORMIFS, 20, map_PIRIFORMIFS, 1, 1)

static const FormulaParamMap map_POLYPYRAIFS = {
    18, {
        { -0x10, 0 }, { -0x20, 0 }, { -0x30, 0 }, { -0x38, 0 },
        { -0x40, 0 }, { -0x48, 0 }, { -0x4C, 2 }, { -0x50, 2 },
        { -0x58, 0 }, { -0x60, 0 }, { -0x68, 0 }, { -0x70, 0 },
        { -0x78, 0 }, { -0x80, 0 }, { 0x00, 0 }, { 0x08, 0 },
        { 0x10, 0 }, { 0x18, 0 }
    }
};
static const FormulaConstEntry consts_POLYPYRAIFS[] = {
    { 0, UINT64_C(0x3FC45F306D974364) },
    { 0, UINT64_C(0xC01921FB54411744) },
    { 0, UINT64_C(0x3FF921FB54411744) },
    { 0, UINT64_C(0x3F91DF46AA91ED06) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER_EX(FORMULA_POLYPYRAIFS, 20, map_POLYPYRAIFS, 1, 1)

static const FormulaParamMap map_QCYLIFS = {
    8, {
        { -0x10, 0 }, { -0x20, 0 }, { -0x30, 0 }, { -0x38, 0 },
        { -0x40, 0 }, { -0x48, 0 }, { -0x4C, 2 }, { -0x54, 0 }
    }
};
DEFINE_EXTERNAL_FORMULA_WRAPPER_EX(FORMULA_QCYLIFS, 20, map_QCYLIFS, 1, 1)

static const FormulaParamMap map_QUADRICSIFS = {
    11, {
        { -0x10, 0 }, { -0x20, 0 }, { -0x30, 0 }, { -0x38, 0 },
        { -0x40, 0 }, { -0x48, 0 }, { -0x4C, 2 }, { -0x54, 0 },
        { -0x5C, 0 }, { -0x60, 2 }, { -0x64, 2 }
    }
};
DEFINE_EXTERNAL_FORMULA_WRAPPER_EX(FORMULA_QUADRICSIFS, 20, map_QUADRICSIFS, 1, 1)

static const FormulaParamMap map_RANDCUBESIFS = {
    19, {
        { -0x10, 0 }, { -0x20, 0 }, { -0x30, 0 }, { -0x38, 0 },
        { -0x40, 0 }, { -0x48, 0 }, { -0x4C, 2 }, { -0x54, 0 },
        { -0x58, 2 }, { -0x5C, 2 }, { -0x60, 2 }, { -0x64, 2 },
        { -0x6C, 0 }, { 0x00, 0 }, { 0x08, 0 }, { 0x10, 0 },
        { 0x18, 0 }, { 0x20, 0 }, { 0x28, 0 }
    }
};
static const FormulaConstEntry consts_RANDCUBESIFS[] = {
    { 0, UINT64_C(0xBEDCCCE9A40544C4) },
    { 0, UINT64_C(0xBFDCCCCCCCCCCCCD) },
    { 0, UINT64_C(0x3EEF75104D551D69) },
    { 0, UINT64_C(0xBEECCCE6E4554895) },
    { 0, UINT64_C(0xBEE599C53883D50C) },
    { 0, UINT64_C(0xBECA013305E6C9CE) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER_EX(FORMULA_RANDCUBESIFS, 20, map_RANDCUBESIFS, 1, 1)

static const FormulaParamMap map_RIEMSQRTIFS = {
    12, {
        { -0x10, 0 }, { -0x20, 0 }, { -0x30, 0 }, { -0x38, 0 },
        { -0x40, 0 }, { -0x48, 0 }, { -0x4C, 2 }, { -0x50, 2 },
        { -0x58, 0 }, { -0x60, 0 }, { 0x00, 0 }, { 0x08, 0 }
    }
};
static const FormulaConstEntry consts_RIEMSQRTIFS[] = {
    { 0, UINT64_C(0x3FE0000000000000) },
    { 0, UINT64_C(0x3F91DF46AA91ED06) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER_EX(FORMULA_RIEMSQRTIFS, 20, map_RIEMSQRTIFS, 1, 1)

static const FormulaParamMap map_ROOMDOORSIFS = {
    23, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x30, 0 },
        { -0x38, 0 }, { -0x40, 0 }, { -0x48, 0 }, { -0x4C, 2 },
        { -0x54, 0 }, { -0x5C, 0 }, { -0x64, 0 }, { -0x6C, 0 },
        { -0x74, 0 }, { -0x78, 2 }, { -0x80, 0 }, { -0x88, 0 },
        { -0x90, 0 }, { -0x98, 0 }, { 0x00, 0 }, { 0x08, 0 },
        { 0x10, 0 }, { 0x18, 0 }, { 0x20, 0 }
    }
};
static const FormulaConstEntry consts_ROOMDOORSIFS[] = {
    { 0, UINT64_C(0x3FE0000000000000) },
    { 0, UINT64_C(0x3FC45F306D974364) },
    { 0, UINT64_C(0xC01921FB54411744) },
    { 0, UINT64_C(0x3FF921FB54411744) },
    { 0, UINT64_C(0xC03E000000000000) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER_EX(FORMULA_ROOMDOORSIFS, 20, map_ROOMDOORSIFS, 1, 1)

static const FormulaParamMap map_ROUNDEDBOX = {
    14, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x30, 0 },
        { -0x40, 0 }, { -0x48, 0 }, { -0x50, 0 }, { -0x60, 0 },
        { -0x64, 2 }, { -0x68, 2 }, { -0x6C, 1 }, { -0x70, 1 },
        { 0x00, 3 }, { 0x08, 3 }
    }
};
static const FormulaConstEntry consts_ROUNDEDBOX[] = {
    { 3, UINT64_C(0x7FFFFFFFFFFFFFFF) },
    { 3, UINT64_C(0x7FFFFFFFFFFFFFFF) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER_EX(FORMULA_ROUNDEDBOX, 20, map_ROUNDEDBOX, 1, 1)

static const FormulaParamMap map_SCARABIFS = {
    7, {
        { -0x10, 0 }, { -0x20, 0 }, { -0x30, 0 }, { -0x38, 0 },
        { -0x40, 0 }, { -0x48, 0 }, { -0x4C, 2 }
    }
};
DEFINE_EXTERNAL_FORMULA_WRAPPER_EX(FORMULA_SCARABIFS, 20, map_SCARABIFS, 1, 1)

static const FormulaParamMap map_SCHERKFS = {
    17, {
        { -0x10, 0 }, { -0x20, 0 }, { -0x30, 0 }, { -0x38, 0 },
        { -0x40, 0 }, { -0x48, 0 }, { -0x4C, 2 }, { -0x50, 1 },
        { -0x54, 1 }, { -0x58, 1 }, { -0x5C, 1 }, { -0x60, 1 },
        { -0x64, 1 }, { -0x68, 1 }, { -0x6C, 1 }, { -0x70, 1 },
        { 0x00, 0 }
    }
};
static const FormulaConstEntry consts_SCHERKFS[] = {
    { 0, UINT64_C(0x3FF8000000000000) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER_EX(FORMULA_SCHERKFS, 20, map_SCHERKFS, 1, 1)

static const FormulaParamMap map_SEASHELL = {
    22, {
        { -0x10, 0 }, { -0x20, 0 }, { -0x30, 0 }, { -0x38, 0 },
        { -0x48, 0 }, { -0x4C, 2 }, { -0x54, 0 }, { -0x58, 1 },
        { -0x5C, 1 }, { -0x60, 1 }, { -0x64, 1 }, { -0x68, 1 },
        { -0x6C, 1 }, { -0x70, 1 }, { -0x74, 1 }, { -0x78, 1 },
        { -0x80, 0 }, { -0x88, 0 }, { -0x90, 0 }, { -0x98, 0 },
        { -0xA0, 0 }, { 0x00, 0 }
    }
};
static const FormulaConstEntry consts_SEASHELL[] = {
    { 0, UINT64_C(0x3FE0000000000000) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER_EX(FORMULA_SEASHELL, 20, map_SEASHELL, 1, 1)

static const FormulaParamMap map_SPHCAGEIFS = {
    13, {
        { -0x10, 0 }, { -0x20, 0 }, { -0x30, 0 }, { -0x38, 0 },
        { -0x40, 0 }, { -0x48, 0 }, { -0x4C, 2 }, { -0x50, 2 },
        { -0x54, 2 }, { 0x00, 0 }, { 0x08, 0 }, { 0x10, 0 },
        { 0x18, 0 }
    }
};
static const FormulaConstEntry consts_SPHCAGEIFS[] = {
    { 0, UINT64_C(0x3FC45F306D974364) },
    { 0, UINT64_C(0x401921FB54411744) },
    { 0, UINT64_C(0x3FF921FB54411744) },
    { 0, UINT64_C(0x4000000000000000) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA_SPHCAGEIFS, 20, map_SPHCAGEIFS)

static const FormulaParamMap map_SPHEREHMVOID = {
    23, {
        { -0x0C, 2 }, { -0x10, 2 }, { -0x14, 1 }, { -0x18, 1 },
        { -0x1C, 1 }, { -0x20, 1 }, { -0x24, 1 }, { -0x28, 1 },
        { -0x2C, 1 }, { -0x30, 1 }, { -0x34, 1 }, { -0x38, 1 },
        { -0x3C, 1 }, { -0x40, 1 }, { -0x44, 1 }, { -0x48, 1 },
        { -0x4C, 1 }, { -0x50, 2 }, { -0x54, 1 }, { -0x58, 1 },
        { -0x60, 0 }, { 0x00, 0 }, { 0x08, 0 }
    }
};
static const FormulaConstEntry consts_SPHEREHMVOID[] = {
    { 0, UINT64_C(0x3F847AE147AE147B) },
    { 0, UINT64_C(0x3FF0000000000000) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER_EX(FORMULA_SPHEREHMVOID, 20, map_SPHEREHMVOID, 1, 1)

static const FormulaParamMap map_SPHEREHEIGHTMAP = {
    20, {
        { -0x0C, 2 }, { -0x10, 2 }, { -0x14, 1 }, { -0x18, 1 },
        { -0x1C, 1 }, { -0x20, 1 }, { -0x24, 1 }, { -0x28, 1 },
        { -0x2C, 1 }, { -0x30, 1 }, { -0x34, 1 }, { -0x38, 1 },
        { -0x3C, 1 }, { -0x40, 1 }, { -0x44, 1 }, { -0x48, 1 },
        { -0x4C, 1 }, { -0x50, 2 }, { -0x54, 1 }, { -0x58, 1 }
    }
};
DEFINE_EXTERNAL_FORMULA_WRAPPER_EX(FORMULA_SPHEREHEIGHTMAP, 20, map_SPHEREHEIGHTMAP, 1, 1)

static const FormulaParamMap map_TORUSELLIPIFS = {
    10, {
        { -0x10, 0 }, { -0x20, 0 }, { -0x30, 0 }, { -0x38, 0 },
        { -0x40, 0 }, { -0x48, 0 }, { -0x4C, 2 }, { -0x50, 2 },
        { -0x54, 2 }, { -0x5C, 0 }
    }
};
DEFINE_EXTERNAL_FORMULA_WRAPPER_EX(FORMULA_TORUSELLIPIFS, 20, map_TORUSELLIPIFS, 1, 1)

static const FormulaParamMap map_TORUSHMVOID = {
    22, {
        { -0x0C, 2 }, { -0x10, 2 }, { -0x14, 1 }, { -0x18, 1 },
        { -0x1C, 1 }, { -0x20, 1 }, { -0x24, 1 }, { -0x28, 1 },
        { -0x2C, 1 }, { -0x30, 1 }, { -0x34, 1 }, { -0x38, 1 },
        { -0x3C, 1 }, { -0x40, 1 }, { -0x44, 1 }, { -0x48, 1 },
        { -0x4C, 1 }, { -0x50, 2 }, { -0x54, 1 }, { -0x58, 1 },
        { 0x00, 0 }, { 0x08, 0 }
    }
};
static const FormulaConstEntry consts_TORUSHMVOID[] = {
    { 0, UINT64_C(0x3FC45F306DC9C883) },
    { 0, UINT64_C(0x3FA999999999999A) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER_EX(FORMULA_TORUSHMVOID, 20, map_TORUSHMVOID, 1, 1)

static const FormulaParamMap map_TORUSIFS = {
    12, {
        { -0x10, 0 }, { -0x20, 0 }, { -0x30, 0 }, { -0x38, 0 },
        { -0x40, 0 }, { -0x48, 0 }, { -0x4C, 2 }, { -0x50, 2 },
        { -0x54, 2 }, { -0x5C, 0 }, { -0x64, 0 }, { -0x6C, 0 }
    }
};
DEFINE_EXTERNAL_FORMULA_WRAPPER_EX(FORMULA_TORUSIFS, 20, map_TORUSIFS, 1, 1)

static const FormulaParamMap map_UMBRELLAIFS = {
    12, {
        { -0x10, 0 }, { -0x20, 0 }, { -0x30, 0 }, { -0x38, 0 },
        { -0x40, 0 }, { -0x48, 0 }, { -0x4C, 2 }, { -0x50, 2 },
        { -0x58, 0 }, { -0x60, 0 }, { -0x68, 0 }, { 0x00, 0 }
    }
};
static const FormulaConstEntry consts_UMBRELLAIFS[] = {
    { 0, UINT64_C(0x3FB999999999999A) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER_EX(FORMULA_UMBRELLAIFS, 20, map_UMBRELLAIFS, 1, 1)

static const FormulaParamMap map_WEIRDCUBEIFS = {
    22, {
        { -0x10, 0 }, { -0x20, 0 }, { -0x30, 0 }, { -0x38, 0 },
        { -0x40, 0 }, { -0x48, 0 }, { -0x4C, 2 }, { -0x50, 1 },
        { -0x54, 1 }, { -0x58, 1 }, { -0x5C, 1 }, { -0x60, 1 },
        { -0x64, 1 }, { -0x68, 1 }, { -0x6C, 1 }, { -0x70, 1 },
        { 0x00, 0 }, { 0x08, 0 }, { 0x10, 0 }, { 0x18, 0 },
        { 0x20, 0 }, { 0x28, 0 }
    }
};
static const FormulaConstEntry consts_WEIRDCUBEIFS[] = {
    { 0, UINT64_C(0x4039000000000000) },
    { 0, UINT64_C(0x4049000000000000) },
    { 0, UINT64_C(0x408F380000000000) },
    { 0, UINT64_C(0x3FE0AAAAAA7CDA8B) },
    { 0, UINT64_C(0xC010000000000000) },
    { 0, UINT64_C(0x3FE570A3D70A3D71) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER_EX(FORMULA_WEIRDCUBEIFS, 20, map_WEIRDCUBEIFS, 1, 1)

static const FormulaParamMap map_YINYANGIFS = {
    18, {
        { -0x10, 0 }, { -0x20, 0 }, { -0x30, 0 }, { -0x38, 0 },
        { -0x40, 0 }, { -0x48, 0 }, { -0x4C, 2 }, { -0x54, 0 },
        { -0x58, 1 }, { -0x5C, 1 }, { -0x60, 1 }, { -0x64, 1 },
        { -0x68, 1 }, { -0x6C, 1 }, { -0x70, 1 }, { -0x74, 1 },
        { -0x78, 1 }, { 0x00, 0 }
    }
};
static const FormulaConstEntry consts_YINYANGIFS[] = {
    { 0, UINT64_C(0xBFF0000000000000) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER_EX(FORMULA_YINYANGIFS, 20, map_YINYANGIFS, 1, 1)

static const FormulaParamMap map_BOARDIFS = {
    21, {
        { -0x10, 0 }, { -0x20, 0 }, { -0x30, 0 }, { -0x38, 0 },
        { -0x40, 0 }, { -0x48, 0 }, { -0x4C, 2 }, { -0x50, 1 },
        { -0x54, 1 }, { -0x58, 1 }, { -0x5C, 1 }, { -0x60, 1 },
        { -0x64, 1 }, { -0x68, 1 }, { -0x6C, 1 }, { -0x70, 1 },
        { -0x74, 2 }, { -0x7C, 0 }, { -0x84, 0 }, { -0x8C, 0 },
        { 0x00, 0 }
    }
};
static const FormulaConstEntry consts_BOARDIFS[] = {
    { 0, UINT64_C(0x4072C00000000000) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER_EX(FORMULA_BOARDIFS, 20, map_BOARDIFS, 1, 1)

static const FormulaParamMap map_BORGIFS = {
    17, {
        { -0x10, 0 }, { -0x20, 0 }, { -0x30, 0 }, { -0x38, 0 },
        { -0x40, 0 }, { -0x48, 0 }, { -0x4C, 2 }, { -0x50, 1 },
        { -0x54, 1 }, { -0x58, 1 }, { -0x5C, 1 }, { -0x60, 1 },
        { -0x64, 1 }, { -0x68, 1 }, { -0x6C, 1 }, { -0x70, 1 },
        { -0x78, 0 }
    }
};
DEFINE_EXTERNAL_FORMULA_WRAPPER_EX(FORMULA_BORGIFS, 20, map_BORGIFS, 1, 1)

static const FormulaParamMap map_CAYLEYSURFIFS = {
    17, {
        { -0x10, 0 }, { -0x20, 0 }, { -0x30, 0 }, { -0x38, 0 },
        { -0x40, 0 }, { -0x48, 0 }, { -0x4C, 2 }, { -0x50, 1 },
        { -0x54, 1 }, { -0x58, 1 }, { -0x5C, 1 }, { -0x60, 1 },
        { -0x64, 1 }, { -0x68, 1 }, { -0x6C, 1 }, { -0x70, 1 },
        { 0x00, 0 }
    }
};
static const FormulaConstEntry consts_CAYLEYSURFIFS[] = {
    { 0, UINT64_C(0x4008000000000000) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER_EX(FORMULA_CAYLEYSURFIFS, 20, map_CAYLEYSURFIFS, 1, 1)

static const FormulaParamMap map_COLUMNSIFS = {
    15, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x30, 0 },
        { -0x38, 0 }, { -0x40, 0 }, { -0x48, 0 }, { -0x50, 0 },
        { -0x54, 2 }, { -0x5C, 0 }, { -0x64, 0 }, { -0x6C, 0 },
        { -0x70, 2 }, { 0x00, 0 }, { 0x08, 0 }
    }
};
static const FormulaConstEntry consts_COLUMNSIFS[] = {
    { 0, UINT64_C(0x3FE0000000000000) },
    { 0, UINT64_C(0x3FE6A09E667F3BCD) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER_EX(FORMULA_COLUMNSIFS, 20, map_COLUMNSIFS, 1, 1)

static const FormulaParamMap map_CUBETUBEIFS = {
    16, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x30, 0 },
        { -0x38, 0 }, { -0x40, 0 }, { -0x48, 0 }, { -0x50, 0 },
        { -0x58, 0 }, { -0x60, 0 }, { -0x68, 0 }, { -0x6C, 2 },
        { -0x74, 0 }, { -0x7C, 0 }, { -0x84, 0 }, { -0x8C, 0 }
    }
};
DEFINE_EXTERNAL_FORMULA_WRAPPER_EX(FORMULA_CUBETUBEIFS, 20, map_CUBETUBEIFS, 1, 1)

static const FormulaParamMap map_CYLINDERIFS = {
    15, {
        { -0x10, 0 }, { -0x20, 0 }, { -0x30, 0 }, { -0x38, 0 },
        { -0x40, 0 }, { -0x48, 0 }, { -0x50, 0 }, { -0x58, 0 },
        { -0x5C, 2 }, { -0x60, 2 }, { -0x64, 1 }, { -0x68, 1 },
        { -0x70, 0 }, { 0x00, 3 }, { 0x08, 3 }
    }
};
static const FormulaConstEntry consts_CYLINDERIFS[] = {
    { 3, UINT64_C(0x7FFFFFFFFFFFFFFF) },
    { 3, UINT64_C(0x7FFFFFFFFFFFFFFF) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER_EX(FORMULA_CYLINDERIFS, 20, map_CYLINDERIFS, 1, 1)

static const FormulaParamMap map_DINIIFS = {
    15, {
        { -0x10, 0 }, { -0x20, 0 }, { -0x30, 0 }, { -0x38, 0 },
        { -0x40, 0 }, { -0x48, 0 }, { -0x4C, 2 }, { -0x54, 0 },
        { -0x5C, 0 }, { -0x60, 2 }, { -0x64, 2 }, { -0x6C, 0 },
        { 0x00, 0 }, { 0x08, 0 }, { 0x10, 0 }
    }
};
static const FormulaConstEntry consts_DINIIFS[] = {
    { 0, UINT64_C(0x3FD3333333333333) },
    { 0, UINT64_C(0x3F947AE147AE147B) },
    { 0, UINT64_C(0x6974E718D7D7625A) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER_EX(FORMULA_DINIIFS, 20, map_DINIIFS, 1, 1)

static const FormulaParamMap map_DISCOBALLIFS = {
    13, {
        { -0x0C, 2 }, { -0x10, 2 }, { -0x20, 0 }, { -0x30, 0 },
        { -0x38, 0 }, { -0x40, 0 }, { -0x44, 2 }, { -0x4C, 0 },
        { -0x54, 0 }, { 0x00, 0 }, { 0x08, 0 }, { 0x10, 0 },
        { 0x18, 0 }
    }
};
static const FormulaConstEntry consts_DISCOBALLIFS[] = {
    { 0, UINT64_C(0x3FC45F306D974364) },
    { 0, UINT64_C(0x401921FB54411744) },
    { 0, UINT64_C(0x3FF921FB54411744) },
    { 0, UINT64_C(0x3F91DF46AA91ED06) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA_DISCOBALLIFS, 20, map_DISCOBALLIFS)

static const FormulaParamMap map_DODECATUBEIFS = {
    19, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x30, 0 },
        { -0x38, 0 }, { -0x40, 0 }, { -0x48, 0 }, { -0x50, 0 },
        { -0x58, 0 }, { -0x60, 0 }, { -0x68, 0 }, { -0x6C, 2 },
        { -0x74, 0 }, { -0x7C, 0 }, { -0x84, 0 }, { -0x8C, 0 },
        { 0x00, 0 }, { 0x08, 0 }, { 0x10, 0 }
    }
};
static const FormulaConstEntry consts_DODECATUBEIFS[] = {
    { 0, UINT64_C(0xBFE9E3779E9D0E99) },
    { 0, UINT64_C(0x3FD3C6EF3D3A1D32) },
    { 0, UINT64_C(0x3FE0000000000000) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER_EX(FORMULA_DODECATUBEIFS, 20, map_DODECATUBEIFS, 1, 1)

static const FormulaParamMap map_DUCKSIFS = {
    17, {
        { -0x10, 0 }, { -0x20, 0 }, { -0x30, 0 }, { -0x38, 0 },
        { -0x40, 0 }, { -0x48, 0 }, { -0x4C, 2 }, { -0x50, 2 },
        { -0x54, 2 }, { -0x5C, 0 }, { -0x64, 0 }, { -0x68, 1 },
        { -0x6C, 1 }, { -0x74, 0 }, { 0x00, 0 }, { 0x08, 0 },
        { 0x10, 0 }
    }
};
static const FormulaConstEntry consts_DUCKSIFS[] = {
    { 0, UINT64_C(0x3FE0000000000000) },
    { 0, UINT64_C(0x3F9EB851EB851EB8) },
    { 0, UINT64_C(0x3FB999999999999A) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER_EX(FORMULA_DUCKSIFS, 20, map_DUCKSIFS, 1, 1)

static const FormulaParamMap map_FOLDCUT12IFS = {
    13, {
        { -0x10, 0 }, { -0x20, 0 }, { -0x30, 0 }, { -0x38, 0 },
        { -0x40, 0 }, { -0x44, 2 }, { -0x4C, 0 }, { -0x54, 0 },
        { -0x5C, 0 }, { -0x64, 0 }, { 0x00, 0 }, { 0x08, 0 },
        { 0x10, 0 }
    }
};
static const FormulaConstEntry consts_FOLDCUT12IFS[] = {
    { 0, UINT64_C(0xBFE9E3779E9D0E99) },
    { 0, UINT64_C(0x3FD3C6EF3D3A1D32) },
    { 0, UINT64_C(0x3FE0000000000000) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER_EX(FORMULA_FOLDCUT12IFS, 20, map_FOLDCUT12IFS, 1, 1)

static const FormulaParamMap map_FOLDCUT8IFS = {
    10, {
        { -0x10, 0 }, { -0x20, 0 }, { -0x30, 0 }, { -0x38, 0 },
        { -0x40, 0 }, { -0x44, 2 }, { -0x4C, 0 }, { -0x54, 0 },
        { -0x5C, 0 }, { -0x64, 0 }
    }
};
DEFINE_EXTERNAL_FORMULA_WRAPPER_EX(FORMULA_FOLDCUT8IFS, 20, map_FOLDCUT8IFS, 1, 1)

static const FormulaParamMap map_GEAR2IFS = {
    15, {
        { -0x10, 0 }, { -0x20, 0 }, { -0x30, 0 }, { -0x38, 0 },
        { -0x40, 0 }, { -0x48, 0 }, { -0x4C, 2 }, { -0x54, 0 },
        { -0x5C, 0 }, { -0x64, 0 }, { -0x6C, 0 }, { -0x74, 0 },
        { 0x00, 0 }, { 0x08, 0 }, { 0x10, 0 }
    }
};
static const FormulaConstEntry consts_GEAR2IFS[] = {
    { 0, UINT64_C(0x405CA5DC1A47A9E3) },
    { 0, UINT64_C(0x3F81DF469963E11D) },
    { 0, UINT64_C(0x4066800000000000) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA_GEAR2IFS, 20, map_GEAR2IFS)

static const FormulaParamMap map_GNARLY2IFS = {
    15, {
        { -0x10, 0 }, { -0x20, 0 }, { -0x30, 0 }, { -0x38, 0 },
        { -0x40, 0 }, { -0x48, 0 }, { -0x4C, 2 }, { -0x50, 2 },
        { -0x54, 2 }, { -0x5C, 0 }, { -0x64, 0 }, { -0x6C, 0 },
        { -0x74, 0 }, { 0x00, 0 }, { 0x08, 0 }
    }
};
static const FormulaConstEntry consts_GNARLY2IFS[] = {
    { 0, UINT64_C(0x3FF8000000000000) },
    { 0, UINT64_C(0x3FB999999999999A) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER_EX(FORMULA_GNARLY2IFS, 20, map_GNARLY2IFS, 1, 1)

static const FormulaParamMap map_GNARLY3IFS = {
    15, {
        { -0x10, 0 }, { -0x20, 0 }, { -0x30, 0 }, { -0x38, 0 },
        { -0x40, 0 }, { -0x48, 0 }, { -0x4C, 2 }, { -0x50, 2 },
        { -0x54, 2 }, { -0x5C, 0 }, { -0x64, 0 }, { -0x6C, 0 },
        { -0x74, 0 }, { 0x00, 0 }, { 0x08, 0 }
    }
};
static const FormulaConstEntry consts_GNARLY3IFS[] = {
    { 0, UINT64_C(0x3FF8000000000000) },
    { 0, UINT64_C(0x3FB999999999999A) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER_EX(FORMULA_GNARLY3IFS, 20, map_GNARLY3IFS, 1, 1)

static const FormulaParamMap map_GNARLYIFS = {
    15, {
        { -0x10, 0 }, { -0x20, 0 }, { -0x30, 0 }, { -0x38, 0 },
        { -0x40, 0 }, { -0x48, 0 }, { -0x4C, 2 }, { -0x50, 2 },
        { -0x54, 2 }, { -0x5C, 0 }, { -0x64, 0 }, { -0x6C, 0 },
        { -0x74, 0 }, { 0x00, 0 }, { 0x08, 0 }
    }
};
static const FormulaConstEntry consts_GNARLYIFS[] = {
    { 0, UINT64_C(0x3FF8000000000000) },
    { 0, UINT64_C(0x3FB999999999999A) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER_EX(FORMULA_GNARLYIFS, 20, map_GNARLYIFS, 1, 1)

static const FormulaParamMap map_GREEKIFS = {
    18, {
        { -0x10, 0 }, { -0x20, 0 }, { -0x30, 0 }, { -0x38, 0 },
        { -0x40, 0 }, { -0x44, 2 }, { -0x48, 2 }, { -0x4C, 2 },
        { -0x50, 1 }, { -0x54, 1 }, { -0x58, 1 }, { -0x5C, 1 },
        { -0x60, 1 }, { -0x64, 1 }, { -0x68, 1 }, { -0x6C, 1 },
        { -0x70, 1 }, { 0x00, 0 }
    }
};
static const FormulaConstEntry consts_GREEKIFS[] = {
    { 0, UINT64_C(0x3FE0000000000000) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER_EX(FORMULA_GREEKIFS, 20, map_GREEKIFS, 1, 1)

static const FormulaParamMap map_GUMDROPIFS = {
    17, {
        { -0x10, 0 }, { -0x20, 0 }, { -0x30, 0 }, { -0x38, 0 },
        { -0x40, 0 }, { -0x48, 0 }, { -0x4C, 2 }, { -0x50, 1 },
        { -0x54, 1 }, { -0x58, 1 }, { -0x5C, 1 }, { -0x60, 1 },
        { -0x64, 1 }, { -0x68, 1 }, { -0x6C, 1 }, { -0x70, 1 },
        { 0x00, 0 }
    }
};
static const FormulaConstEntry consts_GUMDROPIFS[] = {
    { 0, UINT64_C(0x4014000000000000) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER_EX(FORMULA_GUMDROPIFS, 20, map_GUMDROPIFS, 1, 1)

static const FormulaParamMap map_GYROIDIFS = {
    17, {
        { -0x10, 0 }, { -0x20, 0 }, { -0x30, 0 }, { -0x38, 0 },
        { -0x40, 0 }, { -0x48, 0 }, { -0x4C, 2 }, { -0x50, 1 },
        { -0x54, 1 }, { -0x58, 1 }, { -0x5C, 1 }, { -0x60, 1 },
        { -0x64, 1 }, { -0x68, 1 }, { -0x6C, 1 }, { -0x70, 1 },
        { -0x78, 0 }
    }
};
DEFINE_EXTERNAL_FORMULA_WRAPPER_EX(FORMULA_GYROIDIFS, 20, map_GYROIDIFS, 1, 1)

static const FormulaParamMap map_HEARTKLUCHIFS = {
    23, {
        { -0x10, 0 }, { -0x20, 0 }, { -0x30, 0 }, { -0x38, 0 },
        { -0x40, 0 }, { -0x44, 2 }, { -0x4C, 0 }, { -0x50, 1 },
        { -0x54, 1 }, { -0x58, 1 }, { -0x5C, 1 }, { -0x60, 1 },
        { -0x64, 1 }, { -0x68, 1 }, { -0x6C, 1 }, { -0x70, 1 },
        { 0x00, 0 }, { 0x08, 0 }, { 0x10, 0 }, { 0x18, 0 },
        { 0x20, 0 }, { 0x28, 0 }, { 0x30, 0 }
    }
};
static const FormulaConstEntry consts_HEARTKLUCHIFS[] = {
    { 0, UINT64_C(0x3FF6666666666666) },
    { 0, UINT64_C(0x3F847AE147AE147B) },
    { 0, UINT64_C(0x3FD0000000000000) },
    { 0, UINT64_C(0xBFF199999999999A) },
    { 0, UINT64_C(0xBFE999999999999A) },
    { 0, UINT64_C(0x4019000000000000) },
    { 0, UINT64_C(0x3FF71EB851EB851F) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER_EX(FORMULA_HEARTKLUCHIFS, 20, map_HEARTKLUCHIFS, 1, 1)

static const FormulaParamMap map_HELISTAIRSIFS = {
    26, {
        { -0x10, 0 }, { -0x20, 0 }, { -0x30, 0 }, { -0x38, 0 },
        { -0x40, 0 }, { -0x44, 2 }, { -0x4C, 0 }, { -0x50, 1 },
        { -0x54, 1 }, { -0x58, 1 }, { -0x5C, 1 }, { -0x60, 1 },
        { -0x64, 1 }, { -0x68, 1 }, { -0x6C, 1 }, { -0x70, 1 },
        { -0x74, 2 }, { -0x7C, 0 }, { -0x84, 0 }, { -0x8C, 0 },
        { -0x94, 0 }, { 0x00, 0 }, { 0x08, 0 }, { 0x10, 0 },
        { 0x18, 0 }, { 0x20, 0 }
    }
};
static const FormulaConstEntry consts_HELISTAIRSIFS[] = {
    { 0, UINT64_C(0x3FF921FB54442D18) },
    { 0, UINT64_C(0x3FC45F306DC9C883) },
    { 0, UINT64_C(0x401921FB54442D18) },
    { 0, UINT64_C(0x3FE0000000000000) },
    { 0, UINT64_C(0x3FE0000000000000) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER_EX(FORMULA_HELISTAIRSIFS, 20, map_HELISTAIRSIFS, 1, 1)

static const FormulaParamMap map_HELIXIFS = {
    12, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x30, 0 },
        { -0x38, 0 }, { -0x40, 0 }, { -0x48, 0 }, { -0x50, 0 },
        { -0x54, 2 }, { -0x58, 2 }, { -0x5C, 2 }, { 0x00, 0 }
    }
};
static const FormulaConstEntry consts_HELIXIFS[] = {
    { 0, UINT64_C(0x40C3880000000000) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER_EX(FORMULA_HELIXIFS, 20, map_HELIXIFS, 1, 1)

static const FormulaParamMap map_HEXGRIDIFS = {
    25, {
        { -0x10, 0 }, { -0x20, 0 }, { -0x30, 0 }, { -0x38, 0 },
        { -0x40, 0 }, { -0x48, 0 }, { -0x4C, 2 }, { -0x50, 1 },
        { -0x54, 1 }, { -0x58, 1 }, { -0x5C, 1 }, { -0x60, 1 },
        { -0x64, 1 }, { -0x68, 1 }, { -0x6C, 1 }, { -0x70, 1 },
        { -0x74, 1 }, { -0x78, 1 }, { -0x80, 0 }, { -0x88, 0 },
        { 0x00, 0 }, { 0x08, 0 }, { 0x10, 0 }, { 0x18, 0 },
        { 0x20, 0 }
    }
};
static const FormulaConstEntry consts_HEXGRIDIFS[] = {
    { 0, UINT64_C(0x3FE0000000000000) },
    { 0, UINT64_C(0x3FE279A6B50B0F28) },
    { 0, UINT64_C(0xBFEBB67A0F9096BC) },
    { 0, UINT64_C(0xBFE0000000000000) },
    { 0, UINT64_C(0x3FDBB67A0F9096BC) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER_EX(FORMULA_HEXGRIDIFS, 20, map_HEXGRIDIFS, 1, 1)

static const FormulaParamMap map_HEXTGRID2IFS = {
    21, {
        { -0x10, 0 }, { -0x20, 0 }, { -0x30, 0 }, { -0x38, 0 },
        { -0x40, 0 }, { -0x48, 0 }, { -0x4C, 2 }, { -0x50, 1 },
        { -0x54, 1 }, { -0x58, 1 }, { -0x5C, 1 }, { -0x60, 1 },
        { -0x64, 1 }, { -0x68, 1 }, { -0x6C, 1 }, { -0x70, 1 },
        { 0x00, 0 }, { 0x08, 0 }, { 0x10, 0 }, { 0x18, 0 },
        { 0x20, 0 }
    }
};
static const FormulaConstEntry consts_HEXTGRID2IFS[] = {
    { 0, UINT64_C(0xC000000000000000) },
    { 0, UINT64_C(0x3FE0000000000000) },
    { 0, UINT64_C(0x3FEBB67A0F9096BC) },
    { 0, UINT64_C(0xC008000000000000) },
    { 0, UINT64_C(0x3FD279A6B50B0F28) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER_EX(FORMULA_HEXTGRID2IFS, 20, map_HEXTGRID2IFS, 1, 1)

static const FormulaParamMap map_HEXTGRID3IFS = {
    24, {
        { -0x10, 0 }, { -0x20, 0 }, { -0x30, 0 }, { -0x38, 0 },
        { -0x40, 0 }, { -0x48, 0 }, { -0x4C, 2 }, { -0x50, 1 },
        { -0x54, 1 }, { -0x58, 1 }, { -0x5C, 1 }, { -0x60, 1 },
        { -0x64, 1 }, { -0x68, 1 }, { -0x6C, 1 }, { -0x70, 1 },
        { -0x74, 1 }, { -0x78, 1 }, { -0x80, 0 }, { 0x00, 0 },
        { 0x08, 0 }, { 0x10, 0 }, { 0x18, 0 }, { 0x20, 0 }
    }
};
static const FormulaConstEntry consts_HEXTGRID3IFS[] = {
    { 0, UINT64_C(0x3FE0000000000000) },
    { 0, UINT64_C(0x3FE279A6B50B0F28) },
    { 0, UINT64_C(0xBFEBB67A0F9096BC) },
    { 0, UINT64_C(0xBFE0000000000000) },
    { 0, UINT64_C(0x3FDBB67A0F9096BC) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER_EX(FORMULA_HEXTGRID3IFS, 20, map_HEXTGRID3IFS, 1, 1)

static const FormulaParamMap map_HEXTGRID4IFS = {
    21, {
        { -0x10, 0 }, { -0x20, 0 }, { -0x30, 0 }, { -0x38, 0 },
        { -0x40, 0 }, { -0x48, 0 }, { -0x4C, 2 }, { -0x50, 1 },
        { -0x54, 1 }, { -0x58, 1 }, { -0x5C, 1 }, { -0x60, 1 },
        { -0x64, 1 }, { -0x68, 1 }, { -0x6C, 1 }, { -0x70, 1 },
        { -0x78, 0 }, { -0x80, 0 }, { 0x00, 0 }, { 0x08, 0 },
        { 0x10, 0 }
    }
};
static const FormulaConstEntry consts_HEXTGRID4IFS[] = {
    { 0, UINT64_C(0x3FE0000000000000) },
    { 0, UINT64_C(0x400BB67AE6502B91) },
    { 0, UINT64_C(0x3FEBB67A0F9096BC) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER_EX(FORMULA_HEXTGRID4IFS, 20, map_HEXTGRID4IFS, 1, 1)

static const FormulaParamMap map_HEXTGRIDIFS = {
    22, {
        { -0x10, 0 }, { -0x20, 0 }, { -0x30, 0 }, { -0x38, 0 },
        { -0x40, 0 }, { -0x48, 0 }, { -0x4C, 2 }, { -0x50, 1 },
        { -0x54, 1 }, { -0x58, 1 }, { -0x5C, 1 }, { -0x60, 1 },
        { -0x64, 1 }, { -0x68, 1 }, { -0x6C, 1 }, { -0x70, 1 },
        { 0x00, 0 }, { 0x08, 0 }, { 0x10, 0 }, { 0x18, 0 },
        { 0x20, 0 }, { 0x28, 0 }
    }
};
static const FormulaConstEntry consts_HEXTGRIDIFS[] = {
    { 0, UINT64_C(0x4020000000000000) },
    { 0, UINT64_C(0x4018000000000000) },
    { 0, UINT64_C(0x4010000000000000) },
    { 0, UINT64_C(0x4000000000000000) },
    { 0, UINT64_C(0x400BB67AE8FF5D6E) },
    { 0, UINT64_C(0x401BB67AE85390F7) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA_HEXTGRIDIFS, 20, map_HEXTGRIDIFS)

static const FormulaParamMap map_LIDINOIDIFS = {
    19, {
        { -0x10, 0 }, { -0x20, 0 }, { -0x30, 0 }, { -0x38, 0 },
        { -0x40, 0 }, { -0x48, 0 }, { -0x4C, 2 }, { -0x50, 1 },
        { -0x54, 1 }, { -0x58, 1 }, { -0x5C, 1 }, { -0x60, 1 },
        { -0x64, 1 }, { -0x68, 1 }, { -0x6C, 1 }, { -0x70, 1 },
        { -0x78, 0 }, { 0x00, 0 }, { 0x08, 0 }
    }
};
static const FormulaConstEntry consts_LIDINOIDIFS[] = {
    { 0, UINT64_C(0x3FE0000000000000) },
    { 0, UINT64_C(0x3FC3333333333333) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER_EX(FORMULA_LIDINOIDIFS, 20, map_LIDINOIDIFS, 1, 1)

static const FormulaParamMap map_LOGSPIFS = {
    23, {
        { -0x10, 0 }, { -0x20, 0 }, { -0x30, 0 }, { -0x38, 0 },
        { -0x40, 0 }, { -0x48, 0 }, { -0x4C, 2 }, { -0x50, 2 },
        { -0x58, 0 }, { -0x5C, 2 }, { -0x60, 1 }, { -0x64, 1 },
        { -0x68, 1 }, { -0x6C, 1 }, { -0x70, 1 }, { -0x74, 1 },
        { -0x78, 1 }, { -0x7C, 1 }, { -0x80, 1 }, { -0x84, 1 },
        { -0x88, 1 }, { 0x00, 0 }, { 0x08, 0 }
    }
};
static const FormulaConstEntry consts_LOGSPIFS[] = {
    { 0, UINT64_C(0xBFC45F306D974364) },
    { 0, UINT64_C(0x3FA999999999999A) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER_EX(FORMULA_LOGSPIFS, 20, map_LOGSPIFS, 1, 1)

static const FormulaParamMap map_LOXODROME2IFS = {
    20, {
        { -0x10, 0 }, { -0x20, 0 }, { -0x30, 0 }, { -0x38, 0 },
        { -0x40, 0 }, { -0x48, 0 }, { -0x4C, 2 }, { -0x54, 0 },
        { -0x5C, 0 }, { -0x64, 0 }, { -0x6C, 0 }, { -0x74, 0 },
        { -0x78, 2 }, { 0x00, 0 }, { 0x08, 0 }, { 0x10, 0 },
        { 0x18, 0 }, { 0x20, 0 }, { 0x28, 0 }, { 0x30, 0 }
    }
};
static const FormulaConstEntry consts_LOXODROME2IFS[] = {
    { 0, UINT64_C(0x405CA5DC1A47A9E3) },
    { 0, UINT64_C(0x3F81DF469963E11D) },
    { 0, UINT64_C(0x3FA999999999999A) },
    { 0, UINT64_C(0xBFB999999999999A) },
    { 0, UINT64_C(0x3FEFFFFFFF768FA1) },
    { 0, UINT64_C(0x3FF051EB851EB852) },
    { 0, UINT64_C(0x400922D0E5604189) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER_EX(FORMULA_LOXODROME2IFS, 20, map_LOXODROME2IFS, 1, 1)

static const FormulaParamMap map_LOXODROMEIFS = {
    19, {
        { -0x10, 0 }, { -0x20, 0 }, { -0x30, 0 }, { -0x38, 0 },
        { -0x40, 0 }, { -0x48, 0 }, { -0x4C, 2 }, { -0x54, 0 },
        { -0x5C, 0 }, { -0x64, 0 }, { -0x6C, 0 }, { -0x74, 0 },
        { -0x7C, 0 }, { 0x00, 0 }, { 0x08, 0 }, { 0x10, 0 },
        { 0x18, 0 }, { 0x20, 0 }, { 0x28, 0 }
    }
};
static const FormulaConstEntry consts_LOXODROMEIFS[] = {
    { 0, UINT64_C(0x405CA5DC1A47A9E3) },
    { 0, UINT64_C(0x3F81DF469963E11D) },
    { 0, UINT64_C(0x4066800000000000) },
    { 0, UINT64_C(0xBFB999999999999A) },
    { 0, UINT64_C(0x3FEFFFFFFF768FA1) },
    { 0, UINT64_C(0x3FF0000000000000) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER_EX(FORMULA_LOXODROMEIFS, 20, map_LOXODROMEIFS, 1, 1)

static const FormulaParamMap map_PYRAMIFS = {
    8, {
        { -0x10, 0 }, { -0x20, 0 }, { -0x30, 0 }, { -0x38, 0 },
        { -0x40, 0 }, { -0x48, 0 }, { -0x4C, 2 }, { -0x54, 0 }
    }
};
DEFINE_EXTERNAL_FORMULA_WRAPPER_EX(FORMULA_PYRAMIFS, 20, map_PYRAMIFS, 1, 1)

static const FormulaParamMap map_ROOMCLRIFS = {
    13, {
        { -0x10, 0 }, { -0x20, 0 }, { -0x30, 0 }, { -0x38, 0 },
        { -0x40, 0 }, { -0x48, 0 }, { -0x4C, 2 }, { -0x54, 0 },
        { -0x5C, 0 }, { -0x64, 0 }, { -0x6C, 0 }, { -0x74, 0 },
        { -0x7C, 0 }
    }
};
DEFINE_EXTERNAL_FORMULA_WRAPPER_EX(FORMULA_ROOMCLRIFS, 20, map_ROOMCLRIFS, 1, 1)

static const FormulaParamMap map_SCHWARTZIFS = {
    17, {
        { -0x10, 0 }, { -0x20, 0 }, { -0x30, 0 }, { -0x38, 0 },
        { -0x40, 0 }, { -0x48, 0 }, { -0x4C, 2 }, { -0x50, 1 },
        { -0x54, 1 }, { -0x58, 1 }, { -0x5C, 1 }, { -0x60, 1 },
        { -0x64, 1 }, { -0x68, 1 }, { -0x6C, 1 }, { -0x70, 1 },
        { 0x00, 0 }
    }
};
static const FormulaConstEntry consts_SCHWARTZIFS[] = {
    { 0, UINT64_C(0x3FE0000000000000) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER_EX(FORMULA_SCHWARTZIFS, 20, map_SCHWARTZIFS, 1, 1)

static const FormulaParamMap map_SLICEEXIFS = {
    16, {
        { -0x10, 0 }, { -0x20, 0 }, { -0x30, 0 }, { -0x38, 0 },
        { -0x40, 0 }, { -0x48, 0 }, { -0x4C, 2 }, { -0x54, 0 },
        { -0x5C, 0 }, { -0x64, 0 }, { -0x6C, 0 }, { -0x74, 0 },
        { -0x78, 2 }, { 0x00, 0 }, { 0x08, 0 }, { 0x10, 0 }
    }
};
static const FormulaConstEntry consts_SLICEEXIFS[] = {
    { 0, UINT64_C(0x405CA5DC1A47A9E3) },
    { 0, UINT64_C(0x3F81DF469963E11D) },
    { 0, UINT64_C(0x4066800000000000) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER_EX(FORMULA_SLICEEXIFS, 20, map_SLICEEXIFS, 1, 1)

static const FormulaParamMap map_SLICEIFS = {
    12, {
        { -0x10, 0 }, { -0x20, 0 }, { -0x30, 0 }, { -0x38, 0 },
        { -0x40, 0 }, { -0x48, 0 }, { -0x4C, 2 }, { -0x54, 0 },
        { -0x5C, 0 }, { 0x00, 0 }, { 0x08, 0 }, { 0x10, 0 }
    }
};
static const FormulaConstEntry consts_SLICEIFS[] = {
    { 0, UINT64_C(0x405CA5DC1A47A9E3) },
    { 0, UINT64_C(0x3F81DF469963E11D) },
    { 0, UINT64_C(0x4066800000000000) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER_EX(FORMULA_SLICEIFS, 20, map_SLICEIFS, 1, 1)

static const FormulaParamMap map_TETRATUBEIFS = {
    16, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x30, 0 },
        { -0x38, 0 }, { -0x40, 0 }, { -0x48, 0 }, { -0x50, 0 },
        { -0x58, 0 }, { -0x60, 0 }, { -0x68, 0 }, { -0x6C, 2 },
        { -0x74, 0 }, { -0x7C, 0 }, { -0x84, 0 }, { -0x8C, 0 }
    }
};
DEFINE_EXTERNAL_FORMULA_WRAPPER_EX(FORMULA_TETRATUBEIFS, 20, map_TETRATUBEIFS, 1, 1)

static const FormulaParamMap map_TOUPIEIFS = {
    17, {
        { -0x10, 0 }, { -0x20, 0 }, { -0x30, 0 }, { -0x38, 0 },
        { -0x40, 0 }, { -0x48, 0 }, { -0x4C, 2 }, { -0x50, 1 },
        { -0x54, 1 }, { -0x58, 1 }, { -0x5C, 1 }, { -0x60, 1 },
        { -0x64, 1 }, { -0x68, 1 }, { -0x6C, 1 }, { -0x70, 1 },
        { 0x00, 0 }
    }
};
static const FormulaConstEntry consts_TOUPIEIFS[] = {
    { 0, UINT64_C(0x3FE0000000000000) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER_EX(FORMULA_TOUPIEIFS, 20, map_TOUPIEIFS, 1, 1)

static const FormulaParamMap map_TRIFOLIUMIFS = {
    22, {
        { -0x10, 0 }, { -0x20, 0 }, { -0x30, 0 }, { -0x38, 0 },
        { -0x40, 0 }, { -0x48, 0 }, { -0x4C, 2 }, { -0x50, 1 },
        { -0x54, 1 }, { -0x58, 1 }, { -0x5C, 1 }, { -0x60, 1 },
        { -0x64, 1 }, { -0x68, 1 }, { -0x6C, 1 }, { -0x70, 1 },
        { -0x78, 0 }, { -0x80, 0 }, { -0x88, 0 }, { 0x00, 0 },
        { 0x08, 0 }, { 0x10, 0 }
    }
};
static const FormulaConstEntry consts_TRIFOLIUMIFS[] = {
    { 0, UINT64_C(0xC008000000000000) },
    { 0, UINT64_C(0x3FE0000000000000) },
    { 0, UINT64_C(0x4014000000000000) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER_EX(FORMULA_TRIFOLIUMIFS, 20, map_TRIFOLIUMIFS, 1, 1)

static const FormulaParamMap map_TRITGRID2IFS = {
    19, {
        { -0x10, 0 }, { -0x20, 0 }, { -0x30, 0 }, { -0x38, 0 },
        { -0x40, 0 }, { -0x48, 0 }, { -0x4C, 2 }, { -0x50, 1 },
        { -0x54, 1 }, { -0x58, 1 }, { -0x5C, 1 }, { -0x60, 1 },
        { -0x64, 1 }, { -0x68, 1 }, { -0x6C, 1 }, { -0x70, 1 },
        { 0x00, 0 }, { 0x08, 0 }, { 0x10, 0 }
    }
};
static const FormulaConstEntry consts_TRITGRID2IFS[] = {
    { 0, UINT64_C(0x3FE0000000000000) },
    { 0, UINT64_C(0x3FEBB67A0F9096BC) },
    { 0, UINT64_C(0x3FF0000000000000) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER_EX(FORMULA_TRITGRID2IFS, 20, map_TRITGRID2IFS, 1, 1)

static const FormulaParamMap map_TRITGRIDIFS = {
    18, {
        { -0x10, 0 }, { -0x20, 0 }, { -0x30, 0 }, { -0x38, 0 },
        { -0x40, 0 }, { -0x48, 0 }, { -0x4C, 2 }, { -0x50, 1 },
        { -0x54, 1 }, { -0x58, 1 }, { -0x5C, 1 }, { -0x60, 1 },
        { -0x64, 1 }, { -0x68, 1 }, { -0x6C, 1 }, { -0x70, 1 },
        { 0x00, 0 }, { 0x08, 0 }
    }
};
static const FormulaConstEntry consts_TRITGRIDIFS[] = {
    { 0, UINT64_C(0x3FE0000000000000) },
    { 0, UINT64_C(0x3FEBB67A0F9096BC) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER_EX(FORMULA_TRITGRIDIFS, 20, map_TRITGRIDIFS, 1, 1)

static const FormulaParamMap map_WAVEIFS = {
    10, {
        { -0x10, 0 }, { -0x20, 0 }, { -0x30, 0 }, { -0x38, 0 },
        { -0x40, 0 }, { -0x48, 0 }, { -0x4C, 2 }, { -0x50, 2 },
        { -0x54, 2 }, { -0x58, 2 }
    }
};
DEFINE_EXTERNAL_FORMULA_WRAPPER_EX(FORMULA_WAVEIFS, 20, map_WAVEIFS, 1, 1)

static const FormulaParamMap map_WAVESPIFS = {
    10, {
        { -0x10, 0 }, { -0x20, 0 }, { -0x30, 0 }, { -0x38, 0 },
        { -0x40, 0 }, { -0x48, 0 }, { -0x4C, 2 }, { -0x54, 0 },
        { -0x58, 2 }, { 0x00, 0 }
    }
};
static const FormulaConstEntry consts_WAVESPIFS[] = {
    { 0, UINT64_C(0x3FF0000000000000) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER_EX(FORMULA_WAVESPIFS, 20, map_WAVESPIFS, 1, 1)

// ============================================================================
// Registration: RegisterExternalFormulas_dIFS()
// ============================================================================

void RegisterExternalFormulas_dIFS() {
    RegisterExternalFormula({
        FORMULA_CITRUSIFS, "CitrusIFS", 20, 17,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA_CITRUSIFS, &map_CITRUSIFS,
        8, {0, 14, 14, 0, 0, 0, 2, 6},
        0x0000,
        1, consts_CITRUSIFS
    });

    RegisterExternalFormula({
        FORMULA_COMMAIFS, "CommaIFS", 20, 15,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA_COMMAIFS, &map_COMMAIFS,
        12, {0, 14, 14, 0, 0, 0, 2, 0, 0, 2, 2, 2},
        0x0000,
        3, consts_COMMAIFS
    });

    RegisterExternalFormula({
        FORMULA_CYLINDERHMVOID, "CylinderHMVoid", 20, 23,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA_CYLINDERHMVOID, &map_CYLINDERHMVOID,
        13, {2, 2, 1, 1, 1, 1, 1, 1, 18, 2, 1, 1, 1},
        0x0003,
        2, consts_CYLINDERHMVOID
    });

    RegisterExternalFormula({
        FORMULA_CYLINDERHEIGHTMAP, "CylinderHeightMap", 20, 21,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA_CYLINDERHEIGHTMAP, &map_CYLINDERHEIGHTMAP,
        12, {2, 2, 1, 1, 1, 1, 1, 1, 18, 2, 1, 1},
        0x0003,
        1, consts_CYLINDERHEIGHTMAP
    });

    RegisterExternalFormula({
        FORMULA_DECOCUBEIFS, "DecoCubeIFS", 20, 9,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA_DECOCUBEIFS, &map_DECOCUBEIFS,
        8, {0, 14, 14, 0, 0, 0, 2, 0},
        0x0000,
        1, consts_DECOCUBEIFS
    });

    RegisterExternalFormula({
        FORMULA_DEVILIFS, "DevilIFS", 20, 19,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA_DEVILIFS, &map_DEVILIFS,
        8, {0, 14, 14, 0, 0, 0, 2, 6},
        0x0000,
        3, consts_DEVILIFS
    });

    RegisterExternalFormula({
        FORMULA_HEIGHTMAP2IFS, "HeightMap2IFS", 20, 23,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA_HEIGHTMAP2IFS, &map_HEIGHTMAP2IFS,
        13, {2, 1, 1, 1, 1, 1, 1, 6, 2, 2, 1, 1, 2},
        0x0401,
        2, consts_HEIGHTMAP2IFS
    });

    RegisterExternalFormula({
        FORMULA_HEIGHTMAPIFS, "HeightMapIFS", 20, 20,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA_HEIGHTMAPIFS, &map_HEIGHTMAPIFS,
        12, {2, 1, 1, 1, 1, 1, 1, 6, 2, 2, 1, 1},
        0x0401,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA_KLEINBAGELIFS, "KleinBagelIFS", 20, 11,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA_KLEINBAGELIFS, &map_KLEINBAGELIFS,
        10, {0, 14, 14, 0, 0, 0, 2, 2, 2, 0},
        0x0000,
        1, consts_KLEINBAGELIFS
    });

    RegisterExternalFormula({
        FORMULA_KLEINBOTTIFS, "KleinBotTIFS", 20, 13,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA_KLEINBOTTIFS, &map_KLEINBOTTIFS,
        8, {0, 14, 14, 0, 0, 0, 2, 0},
        0x0000,
        5, consts_KLEINBOTTIFS
    });

    RegisterExternalFormula({
        FORMULA_KNOTS2IFS, "Knots2IFS", 20, 16,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA_KNOTS2IFS, &map_KNOTS2IFS,
        12, {0, 14, 14, 0, 0, 0, 2, 2, 2, 2, 0, 2},
        0x0000,
        4, consts_KNOTS2IFS
    });

    RegisterExternalFormula({
        FORMULA_KNOTSIFS, "KnotsIFS", 20, 14,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA_KNOTSIFS, &map_KNOTSIFS,
        11, {0, 14, 14, 0, 0, 0, 2, 2, 2, 2, 0},
        0x0000,
        3, consts_KNOTSIFS
    });

    RegisterExternalFormula({
        FORMULA_LIMPETIFS, "LimpetIFS", 20, 17,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA_LIMPETIFS, &map_LIMPETIFS,
        9, {0, 14, 14, 0, 0, 0, 2, 0, 6},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA_MITREIFS, "MitreIFS", 20, 17,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA_MITREIFS, &map_MITREIFS,
        8, {0, 14, 14, 0, 0, 0, 2, 6},
        0x0000,
        1, consts_MITREIFS
    });

    RegisterExternalFormula({
        FORMULA_NEWFOLDCUTIFS, "NewFoldCutIFS", 20, 22,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA_NEWFOLDCUTIFS, &map_NEWFOLDCUTIFS,
        12, {0, 14, 14, 0, 0, 4, 2, 0, 0, 0, 2, 6},
        0x0000,
        1, consts_NEWFOLDCUTIFS
    });

    RegisterExternalFormula({
        FORMULA_NEWMENGER2IFS, "NewMenger2IFS", 20, 24,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA_NEWMENGER2IFS, &map_NEWMENGER2IFS,
        13, {0, 14, 14, 0, 0, 0, 2, 0, 0, 0, 2, 6, 0},
        0x0000,
        3, consts_NEWMENGER2IFS
    });

    RegisterExternalFormula({
        FORMULA_NEWMENGERIFS, "NewMengerIFS", 20, 24,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA_NEWMENGERIFS, &map_NEWMENGERIFS,
        13, {0, 14, 14, 0, 0, 0, 2, 0, 0, 0, 2, 6, 0},
        0x0000,
        3, consts_NEWMENGERIFS
    });

    RegisterExternalFormula({
        FORMULA_PSPHEREIFS, "PSphereIFS", 20, 13,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA_PSPHEREIFS, &map_PSPHEREIFS,
        11, {0, 14, 14, 0, 0, 0, 2, 0, 0, 2, 2},
        0x0000,
        2, consts_PSPHEREIFS
    });

    RegisterExternalFormula({
        FORMULA_PENN_GONIFS, "PenN-gonIFS", 20, 15,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA_PENN_GONIFS, &map_PENN_GONIFS,
        10, {0, 14, 14, 0, 0, 0, 2, 2, 2, 0},
        0x0000,
        5, consts_PENN_GONIFS
    });

    RegisterExternalFormula({
        FORMULA_PIRIFORMIFS, "PiriformIFS", 20, 12,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA_PIRIFORMIFS, &map_PIRIFORMIFS,
        11, {0, 14, 14, 0, 0, 0, 2, 2, 0, 2, 2},
        0x0000,
        1, consts_PIRIFORMIFS
    });

    RegisterExternalFormula({
        FORMULA_POLYPYRAIFS, "PolyPyraIFS", 20, 18,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA_POLYPYRAIFS, &map_POLYPYRAIFS,
        14, {0, 14, 14, 0, 0, 0, 2, 2, 0, 0, 0, 0, 0, 0},
        0x0000,
        4, consts_POLYPYRAIFS
    });

    RegisterExternalFormula({
        FORMULA_QCYLIFS, "QCylIFS", 20, 8,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA_QCYLIFS, &map_QCYLIFS,
        8, {0, 14, 14, 0, 0, 0, 2, 0},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA_QUADRICSIFS, "QuadricsIFS", 20, 11,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA_QUADRICSIFS, &map_QUADRICSIFS,
        11, {0, 14, 14, 0, 0, 0, 2, 0, 0, 2, 2},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA_RANDCUBESIFS, "RandCubesIFS", 20, 19,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA_RANDCUBESIFS, &map_RANDCUBESIFS,
        13, {0, 14, 14, 0, 0, 0, 2, 0, 2, 2, 2, 2, 0},
        0x0000,
        6, consts_RANDCUBESIFS
    });

    RegisterExternalFormula({
        FORMULA_RIEMSQRTIFS, "RiemSqrtIFS", 20, 12,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA_RIEMSQRTIFS, &map_RIEMSQRTIFS,
        10, {0, 14, 14, 0, 0, 0, 2, 2, 0, 0},
        0x0000,
        2, consts_RIEMSQRTIFS
    });

    RegisterExternalFormula({
        FORMULA_ROOMDOORSIFS, "RoomDoorsIFS", 20, 23,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA_ROOMDOORSIFS, &map_ROOMDOORSIFS,
        16, {0, 0, 0, 14, 0, 0, 0, 2, 0, 3, 3, 2, 0, 0, 0, 0},
        0x0000,
        5, consts_ROOMDOORSIFS
    });

    RegisterExternalFormula({
        FORMULA_ROUNDEDBOX, "RoundedBox", 20, 14,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA_ROUNDEDBOX, &map_ROUNDEDBOX,
        12, {0, 0, 0, 14, 14, 0, 0, 14, 2, 2, 1, 1},
        0x0000,
        2, consts_ROUNDEDBOX
    });

    RegisterExternalFormula({
        FORMULA_SCARABIFS, "ScarabIFS", 20, 7,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA_SCARABIFS, &map_SCARABIFS,
        7, {0, 14, 14, 0, 0, 0, 2},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA_SCHERKFS, "ScherkFS", 20, 17,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA_SCHERKFS, &map_SCHERKFS,
        8, {0, 14, 14, 0, 0, 0, 2, 6},
        0x0000,
        1, consts_SCHERKFS
    });

    RegisterExternalFormula({
        FORMULA_SEASHELL, "Seashell", 20, 22,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA_SEASHELL, &map_SEASHELL,
        13, {0, 14, 14, 0, 14, 2, 0, 6, 0, 0, 0, 0, 0},
        0x0000,
        1, consts_SEASHELL
    });

    RegisterExternalFormula({
        FORMULA_SPHCAGEIFS, "SphCageIFS", 20, 13,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA_SPHCAGEIFS, &map_SPHCAGEIFS,
        9, {0, 14, 14, 0, 0, 0, 2, 2, 2},
        0x0000,
        4, consts_SPHCAGEIFS
    });

    RegisterExternalFormula({
        FORMULA_SPHEREHMVOID, "SphereHMVoid", 20, 23,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA_SPHEREHMVOID, &map_SPHEREHMVOID,
        13, {2, 2, 1, 1, 1, 1, 1, 1, 18, 2, 1, 1, 0},
        0x0003,
        2, consts_SPHEREHMVOID
    });

    RegisterExternalFormula({
        FORMULA_SPHEREHEIGHTMAP, "SphereHeightMap", 20, 20,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA_SPHEREHEIGHTMAP, &map_SPHEREHEIGHTMAP,
        12, {2, 2, 1, 1, 1, 1, 1, 1, 18, 2, 1, 1},
        0x0003,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA_TORUSELLIPIFS, "TorusEllipIFS", 20, 10,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA_TORUSELLIPIFS, &map_TORUSELLIPIFS,
        10, {0, 14, 14, 0, 0, 0, 2, 2, 2, 0},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA_TORUSHMVOID, "TorusHMVoid", 20, 22,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA_TORUSHMVOID, &map_TORUSHMVOID,
        12, {2, 2, 1, 1, 1, 1, 1, 1, 18, 2, 1, 1},
        0x0003,
        2, consts_TORUSHMVOID
    });

    RegisterExternalFormula({
        FORMULA_TORUSIFS, "TorusIFS", 20, 12,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA_TORUSIFS, &map_TORUSIFS,
        12, {0, 14, 14, 0, 0, 0, 2, 2, 2, 0, 0, 0},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA_UMBRELLAIFS, "UmbrellaIFS", 20, 12,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA_UMBRELLAIFS, &map_UMBRELLAIFS,
        11, {0, 14, 14, 0, 0, 0, 2, 2, 0, 0, 0},
        0x0000,
        1, consts_UMBRELLAIFS
    });

    RegisterExternalFormula({
        FORMULA_WEIRDCUBEIFS, "WeirdCubeIFS", 20, 22,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA_WEIRDCUBEIFS, &map_WEIRDCUBEIFS,
        8, {0, 14, 14, 0, 0, 0, 2, 6},
        0x0000,
        6, consts_WEIRDCUBEIFS
    });

    RegisterExternalFormula({
        FORMULA_YINYANGIFS, "YinYangIFS", 20, 18,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA_YINYANGIFS, &map_YINYANGIFS,
        9, {0, 14, 14, 0, 0, 0, 2, 0, 6},
        0x0000,
        1, consts_YINYANGIFS
    });

    RegisterExternalFormula({
        FORMULA_BOARDIFS, "boardIFS", 20, 21,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA_BOARDIFS, &map_BOARDIFS,
        12, {0, 14, 14, 0, 0, 0, 2, 6, 2, 0, 0, 0},
        0x0000,
        1, consts_BOARDIFS
    });

    RegisterExternalFormula({
        FORMULA_BORGIFS, "borgIFS", 20, 17,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA_BORGIFS, &map_BORGIFS,
        9, {0, 14, 14, 0, 0, 0, 2, 6, 0},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA_CAYLEYSURFIFS, "cayleySurfIFS", 20, 17,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA_CAYLEYSURFIFS, &map_CAYLEYSURFIFS,
        8, {0, 14, 14, 0, 0, 0, 2, 6},
        0x0000,
        1, consts_CAYLEYSURFIFS
    });

    RegisterExternalFormula({
        FORMULA_COLUMNSIFS, "columnsIFS", 20, 15,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA_COLUMNSIFS, &map_COLUMNSIFS,
        13, {0, 0, 0, 14, 0, 0, 0, 0, 2, 0, 0, 0, 2},
        0x0000,
        2, consts_COLUMNSIFS
    });

    RegisterExternalFormula({
        FORMULA_CUBETUBEIFS, "cubetubeIFS", 20, 16,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA_CUBETUBEIFS, &map_CUBETUBEIFS,
        14, {0, 0, 0, 14, 0, 0, 0, 3, 3, 2, 0, 0, 0, 0},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA_CYLINDERIFS, "cylinderIFS", 20, 15,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA_CYLINDERIFS, &map_CYLINDERIFS,
        13, {0, 14, 14, 0, 0, 0, 0, 0, 2, 2, 1, 1, 0},
        0x0000,
        2, consts_CYLINDERIFS
    });

    RegisterExternalFormula({
        FORMULA_DINIIFS, "diniIFS", 20, 15,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA_DINIIFS, &map_DINIIFS,
        12, {0, 14, 14, 0, 0, 0, 2, 0, 0, 2, 2, 0},
        0x0000,
        3, consts_DINIIFS
    });

    RegisterExternalFormula({
        FORMULA_DISCOBALLIFS, "discoballIFS", 20, 13,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA_DISCOBALLIFS, &map_DISCOBALLIFS,
        9, {2, 2, 14, 14, 0, 0, 2, 0, 0},
        0x0000,
        4, consts_DISCOBALLIFS
    });

    RegisterExternalFormula({
        FORMULA_DODECATUBEIFS, "dodecatubeIFS", 20, 19,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA_DODECATUBEIFS, &map_DODECATUBEIFS,
        14, {0, 0, 0, 14, 0, 0, 0, 3, 3, 2, 0, 0, 0, 0},
        0x0000,
        3, consts_DODECATUBEIFS
    });

    RegisterExternalFormula({
        FORMULA_DUCKSIFS, "ducksIFS", 20, 17,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA_DUCKSIFS, &map_DUCKSIFS,
        13, {0, 14, 14, 0, 0, 0, 2, 2, 2, 0, 0, 4, 0},
        0x0000,
        3, consts_DUCKSIFS
    });

    RegisterExternalFormula({
        FORMULA_FOLDCUT12IFS, "foldcut12-IFS", 20, 13,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA_FOLDCUT12IFS, &map_FOLDCUT12IFS,
        10, {0, 14, 14, 0, 0, 2, 0, 0, 0, 0},
        0x0000,
        3, consts_FOLDCUT12IFS
    });

    RegisterExternalFormula({
        FORMULA_FOLDCUT8IFS, "foldcut8-IFS", 20, 10,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA_FOLDCUT8IFS, &map_FOLDCUT8IFS,
        10, {0, 14, 14, 0, 0, 2, 0, 0, 0, 0},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA_GEAR2IFS, "gear2IFS", 20, 15,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA_GEAR2IFS, &map_GEAR2IFS,
        12, {0, 14, 14, 0, 0, 0, 2, 0, 0, 0, 0, 0},
        0x0000,
        3, consts_GEAR2IFS
    });

    RegisterExternalFormula({
        FORMULA_GNARLY2IFS, "gnarly2IFS", 20, 15,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA_GNARLY2IFS, &map_GNARLY2IFS,
        13, {0, 14, 14, 0, 0, 0, 2, 2, 2, 0, 0, 0, 0},
        0x0000,
        2, consts_GNARLY2IFS
    });

    RegisterExternalFormula({
        FORMULA_GNARLY3IFS, "gnarly3IFS", 20, 15,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA_GNARLY3IFS, &map_GNARLY3IFS,
        13, {0, 14, 14, 0, 0, 0, 2, 2, 2, 0, 0, 0, 0},
        0x0000,
        2, consts_GNARLY3IFS
    });

    RegisterExternalFormula({
        FORMULA_GNARLYIFS, "gnarlyIFS", 20, 15,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA_GNARLYIFS, &map_GNARLYIFS,
        13, {0, 14, 14, 0, 0, 0, 2, 2, 2, 0, 0, 0, 0},
        0x0000,
        2, consts_GNARLYIFS
    });

    RegisterExternalFormula({
        FORMULA_GREEKIFS, "greekIFS", 20, 18,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA_GREEKIFS, &map_GREEKIFS,
        9, {0, 14, 14, 0, 0, 2, 2, 2, 6},
        0x0000,
        1, consts_GREEKIFS
    });

    RegisterExternalFormula({
        FORMULA_GUMDROPIFS, "gumdropIFS", 20, 17,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA_GUMDROPIFS, &map_GUMDROPIFS,
        8, {0, 14, 14, 0, 0, 0, 2, 6},
        0x0000,
        1, consts_GUMDROPIFS
    });

    RegisterExternalFormula({
        FORMULA_GYROIDIFS, "gyroidIFS", 20, 17,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA_GYROIDIFS, &map_GYROIDIFS,
        9, {0, 14, 14, 0, 0, 0, 2, 6, 0},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA_HEARTKLUCHIFS, "heartKluchIFS", 20, 23,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA_HEARTKLUCHIFS, &map_HEARTKLUCHIFS,
        8, {0, 14, 14, 0, 0, 2, 0, 6},
        0x0000,
        7, consts_HEARTKLUCHIFS
    });

    RegisterExternalFormula({
        FORMULA_HELISTAIRSIFS, "helistairsIFS", 20, 26,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA_HELISTAIRSIFS, &map_HELISTAIRSIFS,
        13, {0, 14, 14, 0, 0, 2, 0, 6, 2, 0, 0, 0, 0},
        0x0000,
        5, consts_HELISTAIRSIFS
    });

    RegisterExternalFormula({
        FORMULA_HELIXIFS, "helixIFS", 20, 12,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA_HELIXIFS, &map_HELIXIFS,
        11, {0, 0, 0, 14, 0, 0, 0, 0, 2, 2, 2},
        0x0000,
        1, consts_HELIXIFS
    });

    RegisterExternalFormula({
        FORMULA_HEXGRIDIFS, "hexgridIFS", 20, 25,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA_HEXGRIDIFS, &map_HEXGRIDIFS,
        11, {0, 14, 14, 0, 0, 0, 2, 6, 4, 0, 0},
        0x0000,
        5, consts_HEXGRIDIFS
    });

    RegisterExternalFormula({
        FORMULA_HEXTGRID2IFS, "hextgrid2IFS", 20, 21,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA_HEXTGRID2IFS, &map_HEXTGRID2IFS,
        8, {0, 14, 14, 0, 0, 0, 2, 6},
        0x0000,
        5, consts_HEXTGRID2IFS
    });

    RegisterExternalFormula({
        FORMULA_HEXTGRID3IFS, "hextgrid3IFS", 20, 24,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA_HEXTGRID3IFS, &map_HEXTGRID3IFS,
        10, {0, 14, 14, 0, 0, 0, 2, 6, 4, 0},
        0x0000,
        5, consts_HEXTGRID3IFS
    });

    RegisterExternalFormula({
        FORMULA_HEXTGRID4IFS, "hextgrid4IFS", 20, 21,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA_HEXTGRID4IFS, &map_HEXTGRID4IFS,
        10, {0, 14, 14, 0, 0, 0, 2, 6, 0, 0},
        0x0000,
        3, consts_HEXTGRID4IFS
    });

    RegisterExternalFormula({
        FORMULA_HEXTGRIDIFS, "hextgridIFS", 20, 22,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA_HEXTGRIDIFS, &map_HEXTGRIDIFS,
        8, {0, 14, 14, 0, 0, 0, 2, 6},
        0x0000,
        6, consts_HEXTGRIDIFS
    });

    RegisterExternalFormula({
        FORMULA_LIDINOIDIFS, "lidinoidIFS", 20, 19,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA_LIDINOIDIFS, &map_LIDINOIDIFS,
        9, {0, 14, 14, 0, 0, 0, 2, 6, 0},
        0x0000,
        2, consts_LIDINOIDIFS
    });

    RegisterExternalFormula({
        FORMULA_LOGSPIFS, "logspIFS", 20, 23,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA_LOGSPIFS, &map_LOGSPIFS,
        12, {0, 14, 14, 0, 0, 0, 2, 2, 0, 2, 6, 4},
        0x0000,
        2, consts_LOGSPIFS
    });

    RegisterExternalFormula({
        FORMULA_LOXODROME2IFS, "loxodrome2IFS", 20, 20,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA_LOXODROME2IFS, &map_LOXODROME2IFS,
        13, {0, 14, 14, 0, 0, 0, 2, 0, 0, 0, 0, 0, 2},
        0x0000,
        7, consts_LOXODROME2IFS
    });

    RegisterExternalFormula({
        FORMULA_LOXODROMEIFS, "loxodromeIFS", 20, 19,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA_LOXODROMEIFS, &map_LOXODROMEIFS,
        13, {0, 14, 14, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0},
        0x0000,
        6, consts_LOXODROMEIFS
    });

    RegisterExternalFormula({
        FORMULA_PYRAMIFS, "pyramIFS", 20, 8,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA_PYRAMIFS, &map_PYRAMIFS,
        8, {0, 14, 14, 0, 0, 0, 2, 0},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA_ROOMCLRIFS, "roomclrIFS", 20, 13,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA_ROOMCLRIFS, &map_ROOMCLRIFS,
        13, {0, 14, 14, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA_SCHWARTZIFS, "schwartzIFS", 20, 17,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA_SCHWARTZIFS, &map_SCHWARTZIFS,
        8, {0, 14, 14, 0, 0, 0, 2, 6},
        0x0000,
        1, consts_SCHWARTZIFS
    });

    RegisterExternalFormula({
        FORMULA_SLICEEXIFS, "sliceExIFS", 20, 16,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA_SLICEEXIFS, &map_SLICEEXIFS,
        13, {0, 14, 14, 0, 0, 0, 2, 0, 0, 0, 0, 0, 2},
        0x0000,
        3, consts_SLICEEXIFS
    });

    RegisterExternalFormula({
        FORMULA_SLICEIFS, "sliceIFS", 20, 12,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA_SLICEIFS, &map_SLICEIFS,
        9, {0, 14, 14, 0, 0, 0, 2, 0, 0},
        0x0000,
        3, consts_SLICEIFS
    });

    RegisterExternalFormula({
        FORMULA_TETRATUBEIFS, "tetratubeIFS", 20, 16,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA_TETRATUBEIFS, &map_TETRATUBEIFS,
        14, {0, 0, 0, 14, 0, 0, 0, 3, 3, 2, 0, 0, 0, 0},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA_TOUPIEIFS, "toupieIFS", 20, 17,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA_TOUPIEIFS, &map_TOUPIEIFS,
        8, {0, 14, 14, 0, 0, 0, 2, 6},
        0x0000,
        1, consts_TOUPIEIFS
    });

    RegisterExternalFormula({
        FORMULA_TRIFOLIUMIFS, "trifoliumIFS", 20, 22,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA_TRIFOLIUMIFS, &map_TRIFOLIUMIFS,
        11, {0, 14, 14, 0, 0, 0, 2, 6, 0, 0, 0},
        0x0000,
        3, consts_TRIFOLIUMIFS
    });

    RegisterExternalFormula({
        FORMULA_TRITGRID2IFS, "tritgrid2IFS", 20, 19,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA_TRITGRID2IFS, &map_TRITGRID2IFS,
        8, {0, 14, 14, 0, 0, 0, 2, 6},
        0x0000,
        3, consts_TRITGRID2IFS
    });

    RegisterExternalFormula({
        FORMULA_TRITGRIDIFS, "tritgridIFS", 20, 18,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA_TRITGRIDIFS, &map_TRITGRIDIFS,
        8, {0, 14, 14, 0, 0, 0, 2, 6},
        0x0000,
        2, consts_TRITGRIDIFS
    });

    RegisterExternalFormula({
        FORMULA_WAVEIFS, "waveIFS", 20, 10,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA_WAVEIFS, &map_WAVEIFS,
        10, {0, 14, 14, 0, 0, 0, 2, 2, 2, 2},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA_WAVESPIFS, "wavespIFS", 20, 10,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA_WAVESPIFS, &map_WAVESPIFS,
        9, {0, 14, 14, 0, 0, 0, 2, 0, 2},
        0x0000,
        1, consts_WAVESPIFS
    });

}

} // namespace mb3d

#undef constant
#undef buildRotMatrix3
#undef rotMatrix3FromFloats
#undef RotMatrix3
#undef M_PI_2_F
