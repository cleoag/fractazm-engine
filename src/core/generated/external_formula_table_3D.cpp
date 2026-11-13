// external_formula_table_3D.cpp — Auto-generated bulk formula registrations
// Category: 3D (75 formulas)
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
// Include 3D formula .inc files
// ============================================================================

#include "3D/Aex-Octo.inc"
#include "3D/Aex-TorusPow.inc"
#include "3D/BPine_only.inc"
#include "3D/BT1Pine.inc"
#include "3D/BT1Pinegon.inc"
#include "3D/BT1Pinehedron.inc"
#include "3D/BT2Pine.inc"
#include "3D/Benesi2Pow2.inc"
#include "3D/Benesi2Pow6.inc"
#include "3D/Benesi3Pow2.inc"
#include "3D/BenesiPine1.inc"
#include "3D/BenesiPine2.inc"
#include "3D/Beth1032.inc"
#include "3D/Beth1522.inc"
#include "3D/Beth322.inc"
#include "3D/Beth323.inc"
#include "3D/Beth432.inc"
#include "3D/Beth651.inc"
#include "3D/BulboxP-2.inc"
#include "3D/CosinePow2.inc"
#include "3D/CosinePow8.inc"
#include "3D/Faehrten.inc"
#include "3D/Fuzzy.inc"
#include "3D/GenIntPower.inc"
#include "3D/General.inc"
#include "3D/IQ-bulb.inc"
#include "3D/IQ-bulb_old.inc"
#include "3D/IQ_NormBulb.inc"
#include "3D/IQ_NormBulb_NoYZ.inc"
#include "3D/IQ_PineTree.inc"
#include "3D/IQ_PineTree_NoYZ.inc"
#include "3D/IdesFormula.inc"
#include "3D/Ikenagabulb.inc"
#include "3D/KaliDucks.inc"
#include "3D/KaliLinCombSSE2sm.inc"
#include "3D/Kalisets1.inc"
#include "3D/MagVsXYZ.inc"
#include "3D/MagVsXYZabs.inc"
#include "3D/MagVsXYZabs2.inc"
#include "3D/MagVsXYZabs3.inc"
#include "3D/MagVsXYZmsw.inc"
#include "3D/Makin-NSR-tri.inc"
#include "3D/Makin3D-1.inc"
#include "3D/Makin3D-2.inc"
#include "3D/Makin3D-3-4.inc"
#include "3D/MandelView.inc"
#include "3D/ManowarBulb.inc"
#include "3D/MsltoeFoldQuat.inc"
#include "3D/MsltoeSym2.inc"
#include "3D/MsltoeSym3.inc"
#include "3D/MsltoeSym4.inc"
#include "3D/Msltoe_Sym.inc"
#include "3D/PseudoXDB.inc"
#include "3D/Quad3Db.inc"
#include "3D/Quadrat3D.inc"
#include "3D/QuickDudley.inc"
#include "3D/QuickDudleyKM.inc"
#include "3D/Quintic.inc"
#include "3D/Riemann.inc"
#include "3D/Riemann2.inc"
#include "3D/RuckerBrot1.inc"
#include "3D/Ruckerbulb.inc"
#include "3D/TalisTpx.inc"
#include "3D/TetraFolding_Pow2.inc"
#include "3D/TgladDihed.inc"
#include "3D/TgladDihed2.inc"
#include "3D/TgladTetra.inc"
#include "3D/TgladTetra2.inc"
#include "3D/TrifoxComplexAngles.inc"
#include "3D/_IFS_tess.inc"
#include "3D/_MobiusTpx.inc"
#include "3D/_e_3Dtess.inc"
#include "3D/_mulC3D.inc"
#include "3D/_quadist.inc"
#include "3D/_sphereinv.inc"

// ============================================================================
// PVar offset maps (3D)
// ============================================================================

static const FormulaParamMap map_AEX_OCTO = { 2, { { -0x10, 0 }, { -0x18, 0 } } };
DEFINE_EXTERNAL_FORMULA_WRAPPER_EX(FORMULA_AEX_OCTO, 0, map_AEX_OCTO, 0, 0)

static const FormulaParamMap map_AEX_TORUSPOW = {
    7, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 },
        { -0x2C, 2 }, { 0x00, 0 }, { 0x08, 0 }
    }
};
static const FormulaConstEntry consts_AEX_TORUSPOW[] = {
    { 0, UINT64_C(0x3FE0000000000000) },
    { 0, UINT64_C(0xBFF8000000000000) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA_AEX_TORUSPOW, 0, map_AEX_TORUSPOW)

static const FormulaParamMap map_BPINE_ONLY = { 3, { { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 } } };
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA_BPINE_ONLY, 0, map_BPINE_ONLY)

static const FormulaParamMap map_BT1PINE = {
    9, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 },
        { -0x30, 0 }, { -0x34, 2 }, { 0x00, 0 }, { 0x08, 0 },
        { 0x10, 0 }
    }
};
static const FormulaConstEntry consts_BT1PINE[] = {
    { 0, UINT64_C(0x3FEA20BD700C2C3E) },
    { 0, UINT64_C(0x3FE279A74590331D) },
    { 0, UINT64_C(0x3FE6A09E667F3BCC) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA_BT1PINE, 0, map_BT1PINE)

static const FormulaParamMap map_BT1PINEGON = {
    16, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 },
        { -0x30, 0 }, { -0x38, 0 }, { -0x3C, 2 }, { -0x40, 2 },
        { -0x44, 2 }, { -0x48, 2 }, { -0x4C, 2 }, { -0x54, 0 },
        { 0x00, 0 }, { 0x08, 0 }, { 0x10, 0 }, { 0x18, 0 }
    }
};
static const FormulaConstEntry consts_BT1PINEGON[] = {
    { 0, UINT64_C(0x3FEA20BD700C2C3E) },
    { 0, UINT64_C(0x3FE279A74590331D) },
    { 0, UINT64_C(0x3FE6A09E667F3BCC) },
    { 0, UINT64_C(0x400921FB54442D28) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA_BT1PINEGON, 0, map_BT1PINEGON)

static const FormulaParamMap map_BT1PINEHEDRON = {
    17, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 },
        { -0x30, 0 }, { -0x38, 0 }, { -0x3C, 2 }, { -0x40, 2 },
        { -0x44, 2 }, { -0x48, 2 }, { -0x4C, 2 }, { -0x54, 0 },
        { -0x5C, 0 }, { 0x00, 0 }, { 0x08, 0 }, { 0x10, 0 },
        { 0x18, 0 }
    }
};
static const FormulaConstEntry consts_BT1PINEHEDRON[] = {
    { 0, UINT64_C(0x3FEA20BD700C2C3E) },
    { 0, UINT64_C(0x3FE279A74590331D) },
    { 0, UINT64_C(0x3FE6A09E667F3BCC) },
    { 0, UINT64_C(0x400921FB54442D28) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA_BT1PINEHEDRON, 0, map_BT1PINEHEDRON)

static const FormulaParamMap map_BT2PINE = {
    8, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 },
        { -0x30, 0 }, { 0x00, 0 }, { 0x08, 0 }, { 0x10, 0 }
    }
};
static const FormulaConstEntry consts_BT2PINE[] = {
    { 0, UINT64_C(0x3FEA20BD700C2C3E) },
    { 0, UINT64_C(0x3FE279A74590331D) },
    { 0, UINT64_C(0x3FE6A09E667F3BCC) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA_BT2PINE, 0, map_BT2PINE)

static const FormulaParamMap map_BENESI2POW2 = { 0, {} };
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA_BENESI2POW2, 0, map_BENESI2POW2)

static const FormulaParamMap map_BENESI2POW6 = { 1, { { 0x00, 0 } } };
static const FormulaConstEntry consts_BENESI2POW6[] = {
    { 0, UINT64_C(0x4028000000000000) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA_BENESI2POW6, 0, map_BENESI2POW6)

static const FormulaParamMap map_BENESI3POW2 = { 0, {} };
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA_BENESI3POW2, 0, map_BENESI3POW2)

static const FormulaParamMap map_BENESIPINE1 = {
    5, {
        { -0x10, 0 }, { -0x18, 0 }, { 0x00, 0 }, { 0x08, 0 },
        { 0x10, 0 }
    }
};
static const FormulaConstEntry consts_BENESIPINE1[] = {
    { 0, UINT64_C(0x3FEA20BD700C2C3E) },
    { 0, UINT64_C(0x3FE279A74590331D) },
    { 0, UINT64_C(0x3FE6A09E667F3BCC) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER_EX(FORMULA_BENESIPINE1, 0, map_BENESIPINE1, 0, 0)

static const FormulaParamMap map_BENESIPINE2 = {
    5, {
        { -0x10, 0 }, { -0x18, 0 }, { 0x00, 0 }, { 0x08, 0 },
        { 0x10, 0 }
    }
};
static const FormulaConstEntry consts_BENESIPINE2[] = {
    { 0, UINT64_C(0x3FEA20BD700C2C3E) },
    { 0, UINT64_C(0x3FE279A74590331D) },
    { 0, UINT64_C(0x3FE6A09E667F3BCC) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER_EX(FORMULA_BENESIPINE2, 0, map_BENESIPINE2, 0, 0)

static const FormulaParamMap map_BETH1032 = {
    13, {
        { 0x00, 0 }, { 0x08, 0 }, { 0x10, 0 }, { 0x18, 0 },
        { 0x20, 0 }, { 0x28, 0 }, { 0x30, 0 }, { 0x38, 0 },
        { 0x40, 0 }, { 0x48, 0 }, { 0x50, 0 }, { 0x58, 0 },
        { 0x60, 0 }
    }
};
static const FormulaConstEntry consts_BETH1032[] = {
    { 0, UINT64_C(0x3FE61F9ADD3C0CA4) },
    { 0, UINT64_C(0x40356A99B4B1F77E) },
    { 0, UINT64_C(0xC015AA66D2C7DDF7) },
    { 0, UINT64_C(0xC0399F80CB36C030) },
    { 0, UINT64_C(0x4065F3F01966D806) },
    { 0, UINT64_C(0x403B1FE68F0AF33D) },
    { 0, UINT64_C(0xC02A3FCD1E15E679) },
    { 0, UINT64_C(0x401ED5336963EEFC) },
    { 0, UINT64_C(0xC016D5336963EEFC) },
    { 0, UINT64_C(0x4000000000000000) },
    { 0, UINT64_C(0x4008000000000000) },
    { 0, UINT64_C(0x402E000000000000) },
    { 0, UINT64_C(0x4034000000000000) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA_BETH1032, 0, map_BETH1032)

static const FormulaParamMap map_BETH1522 = {
    5, {
        { 0x00, 0 }, { 0x08, 0 }, { 0x10, 0 }, { 0x18, 0 },
        { 0x20, 0 }
    }
};
static const FormulaConstEntry consts_BETH1522[] = {
    { 0, UINT64_C(0x3FC4000000000000) },
    { 0, UINT64_C(0x4034A708A824F613) },
    { 0, UINT64_C(0xC0254E115049EC26) },
    { 0, UINT64_C(0x4000000000000000) },
    { 0, UINT64_C(0x403E000000000000) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER_EX(FORMULA_BETH1522, 0, map_BETH1522, 0, 0)

static const FormulaParamMap map_BETH322 = { 1, { { 0x00, 0 } } };
static const FormulaConstEntry consts_BETH322[] = {
    { 0, UINT64_C(0x4014000000000000) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER_EX(FORMULA_BETH322, 0, map_BETH322, 0, 0)

static const FormulaParamMap map_BETH323 = { 3, { { 0x00, 0 }, { 0x08, 0 }, { 0x10, 0 } } };
static const FormulaConstEntry consts_BETH323[] = {
    { 0, UINT64_C(0x4035000000000000) },
    { 0, UINT64_C(0xC041800000000000) },
    { 0, UINT64_C(0x401C000000000000) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER_EX(FORMULA_BETH323, 0, map_BETH323, 0, 0)

static const FormulaParamMap map_BETH432 = {
    6, {
        { 0x00, 0 }, { 0x08, 0 }, { 0x10, 0 }, { 0x18, 0 },
        { 0x20, 0 }, { 0x28, 0 }
    }
};
static const FormulaConstEntry consts_BETH432[] = {
    { 0, UINT64_C(0xBFD948B0FCD6E9E0) },
    { 0, UINT64_C(0xC028000000000000) },
    { 0, UINT64_C(0x402C000000000000) },
    { 0, UINT64_C(0xC035000000000000) },
    { 0, UINT64_C(0xC041800000000000) },
    { 0, UINT64_C(0xC05A400000000000) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER_EX(FORMULA_BETH432, 0, map_BETH432, 0, 0)

static const FormulaParamMap map_BETH651 = {
    8, {
        { 0x00, 0 }, { 0x08, 0 }, { 0x10, 0 }, { 0x18, 0 },
        { 0x20, 0 }, { 0x28, 0 }, { 0x30, 0 }, { 0x38, 0 }
    }
};
static const FormulaConstEntry consts_BETH651[] = {
    { 0, UINT64_C(0xC0061C8864680B58) },
    { 0, UINT64_C(0x3FF3C6EF372FE950) },
    { 0, UINT64_C(0x4000000000000000) },
    { 0, UINT64_C(0x4001E3779B97F4A8) },
    { 0, UINT64_C(0x4003333333333333) },
    { 0, UINT64_C(0x4009E3779B97F4A8) },
    { 0, UINT64_C(0x4011E3779B97F4A8) },
    { 0, UINT64_C(0x401CF1BBCDCBFA54) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER_EX(FORMULA_BETH651, 0, map_BETH651, 0, 0)

static const FormulaParamMap map_BULBOXP2 = {
    10, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 },
        { -0x30, 0 }, { -0x38, 0 }, { -0x40, 0 }, { -0x48, 0 },
        { -0x50, 0 }, { -0x54, 2 }
    }
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA_BULBOXP2, 0, map_BULBOXP2)

static const FormulaParamMap map_COSINEPOW2 = { 1, { { -0x10, 0 } } };
DEFINE_EXTERNAL_FORMULA_WRAPPER_EX(FORMULA_COSINEPOW2, 0, map_COSINEPOW2, 0, 0)

static const FormulaParamMap map_COSINEPOW8 = {
    6, {
        { -0x10, 0 }, { 0x00, 0 }, { 0x08, 0 }, { 0x10, 0 },
        { 0x18, 0 }, { 0x20, 0 }
    }
};
static const FormulaConstEntry consts_COSINEPOW8[] = {
    { 0, UINT64_C(0x37A16C262777579C) },
    { 0, UINT64_C(0x401C000000000000) },
    { 0, UINT64_C(0x4020000000000000) },
    { 0, UINT64_C(0x403C000000000000) },
    { 0, UINT64_C(0x4051800000000000) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA_COSINEPOW8, 0, map_COSINEPOW8)

static const FormulaParamMap map_FAEHRTEN = { 4, { { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 } } };
DEFINE_EXTERNAL_FORMULA_WRAPPER_EX(FORMULA_FAEHRTEN, 0, map_FAEHRTEN, 0, 0)

static const FormulaParamMap map_FUZZY = { 2, { { -0x10, 0 }, { -0x18, 0 } } };
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA_FUZZY, 0, map_FUZZY)

static const FormulaParamMap map_GENINTPOWER = {
    13, {
        { -0x0C, 2 }, { -0x10, 2 }, { -0x14, 2 }, { -0x18, 2 },
        { -0x1C, 2 }, { -0x20, 2 }, { -0x24, 2 }, { -0x28, 2 },
        { -0x2C, 2 }, { -0x34, 0 }, { 0x00, 3 }, { 0x08, 3 },
        { 0x10, 0 }
    }
};
static const FormulaConstEntry consts_GENINTPOWER[] = {
    { 3, UINT64_C(0x0000000000000000) },
    { 3, UINT64_C(0x8000000000000000) },
    { 0, UINT64_C(0x3FF0000000000000) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER_EX(FORMULA_GENINTPOWER, 0, map_GENINTPOWER, 0, 0)

static const FormulaParamMap map_GENERAL = {
    12, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x24, 2 },
        { -0x28, 2 }, { -0x2C, 2 }, { -0x30, 2 }, { -0x34, 2 },
        { -0x38, 2 }, { -0x3C, 2 }, { -0x40, 2 }, { 0x00, 0 }
    }
};
static const FormulaConstEntry consts_GENERAL[] = {
    { 0, UINT64_C(0x3FF921FAFC8B007A) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA_GENERAL, 0, map_GENERAL)

static const FormulaParamMap map_IQBULB = {
    6, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x24, 2 },
        { -0x28, 2 }, { 0x00, 0 }
    }
};
static const FormulaConstEntry consts_IQBULB[] = {
    { 0, UINT64_C(0x3FF0000000000000) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA_IQBULB, 0, map_IQBULB)

static const FormulaParamMap map_IQBULB_OLD = {
    6, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x24, 2 },
        { -0x28, 2 }, { 0x00, 0 }
    }
};
static const FormulaConstEntry consts_IQBULB_OLD[] = {
    { 0, UINT64_C(0x3FF0000000000000) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA_IQBULB_OLD, 0, map_IQBULB_OLD)

static const FormulaParamMap map_IQ_NORMBULB = {
    6, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x24, 2 },
        { -0x28, 2 }, { 0x00, 0 }
    }
};
static const FormulaConstEntry consts_IQ_NORMBULB[] = {
    { 0, UINT64_C(0x3FF0000000000000) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA_IQ_NORMBULB, 0, map_IQ_NORMBULB)

static const FormulaParamMap map_IQ_NORMBULB_NOYZ = {
    6, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x24, 2 },
        { -0x28, 2 }, { 0x00, 0 }
    }
};
static const FormulaConstEntry consts_IQ_NORMBULB_NOYZ[] = {
    { 0, UINT64_C(0x3FF0000000000000) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA_IQ_NORMBULB_NOYZ, 0, map_IQ_NORMBULB_NOYZ)

static const FormulaParamMap map_IQ_PINETREE = {
    6, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x24, 2 },
        { -0x28, 2 }, { 0x00, 0 }
    }
};
static const FormulaConstEntry consts_IQ_PINETREE[] = {
    { 0, UINT64_C(0x3FF0000000000000) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA_IQ_PINETREE, 0, map_IQ_PINETREE)

static const FormulaParamMap map_IQ_PINETREE_NOYZ = {
    6, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x24, 2 },
        { -0x28, 2 }, { 0x00, 0 }
    }
};
static const FormulaConstEntry consts_IQ_PINETREE_NOYZ[] = {
    { 0, UINT64_C(0x3FF0000000000000) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA_IQ_PINETREE_NOYZ, 0, map_IQ_PINETREE_NOYZ)

static const FormulaParamMap map_IDESFORMULA = {
    5, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 },
        { -0x30, 0 }
    }
};
DEFINE_EXTERNAL_FORMULA_WRAPPER_EX(FORMULA_IDESFORMULA, 0, map_IDESFORMULA, 0, 0)

static const FormulaParamMap map_IKENAGABULB = {
    7, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 },
        { -0x30, 0 }, { -0x38, 0 }, { 0x00, 0 }
    }
};
static const FormulaConstEntry consts_IKENAGABULB[] = {
    { 0, UINT64_C(0x4008000000000000) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA_IKENAGABULB, 0, map_IKENAGABULB)

static const FormulaParamMap map_KALIDUCKS = { 2, { { -0x10, 0 }, { -0x18, 0 } } };
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA_KALIDUCKS, 0, map_KALIDUCKS)

static const FormulaParamMap map_KALILINCOMBSSE2SM = { 3, { { -0x0C, 2 }, { 0x00, 0 }, { -0x08, 0 } } };
static const FormulaConstEntry consts_KALILINCOMBSSE2SM[] = {
    { 0, UINT64_C(0x0000000000000000) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA_KALILINCOMBSSE2SM, 0, map_KALILINCOMBSSE2SM)

static const FormulaParamMap map_KALISETS1 = { 2, { { -0x10, 0 }, { -0x18, 0 } } };
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA_KALISETS1, 0, map_KALISETS1)

static const FormulaParamMap map_MAGVSXYZ = {
    7, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 },
        { -0x30, 0 }, { -0x38, 0 }, { -0x40, 0 }
    }
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA_MAGVSXYZ, 0, map_MAGVSXYZ)

static const FormulaParamMap map_MAGVSXYZABS = {
    8, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 },
        { -0x30, 0 }, { -0x38, 0 }, { -0x40, 0 }, { 0x00, 0 }
    }
};
static const FormulaConstEntry consts_MAGVSXYZABS[] = {
    { 0, UINT64_C(0x3FE0000000000000) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA_MAGVSXYZABS, 0, map_MAGVSXYZABS)

static const FormulaParamMap map_MAGVSXYZABS2 = {
    8, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 },
        { -0x30, 0 }, { -0x38, 0 }, { -0x40, 0 }, { 0x00, 0 }
    }
};
static const FormulaConstEntry consts_MAGVSXYZABS2[] = {
    { 0, UINT64_C(0x3FE0000000000000) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA_MAGVSXYZABS2, 0, map_MAGVSXYZABS2)

static const FormulaParamMap map_MAGVSXYZABS3 = {
    7, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 },
        { -0x30, 0 }, { -0x38, 0 }, { -0x40, 0 }
    }
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA_MAGVSXYZABS3, 0, map_MAGVSXYZABS3)

static const FormulaParamMap map_MAGVSXYZMSW = {
    7, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 },
        { -0x30, 0 }, { -0x38, 0 }, { -0x40, 0 }
    }
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA_MAGVSXYZMSW, 0, map_MAGVSXYZMSW)

static const FormulaParamMap map_MAKIN_NSR_TRI = { 2, { { -0x10, 0 }, { -0x18, 0 } } };
DEFINE_EXTERNAL_FORMULA_WRAPPER_EX(FORMULA_MAKIN_NSR_TRI, 0, map_MAKIN_NSR_TRI, 0, 0)

static const FormulaParamMap map_MAKIN3D_1 = { 0, {} };
DEFINE_EXTERNAL_FORMULA_WRAPPER_EX(FORMULA_MAKIN3D_1, 0, map_MAKIN3D_1, 0, 0)

static const FormulaParamMap map_MAKIN3D_2 = { 0, {} };
DEFINE_EXTERNAL_FORMULA_WRAPPER_EX(FORMULA_MAKIN3D_2, 0, map_MAKIN3D_2, 0, 0)

static const FormulaParamMap map_MAKIN3D_3_4 = { 3, { { -0x0C, 2 }, { -0x10, 2 }, { -0x18, 0 } } };
DEFINE_EXTERNAL_FORMULA_WRAPPER_EX(FORMULA_MAKIN3D_3_4, 0, map_MAKIN3D_3_4, 0, 0)

static const FormulaParamMap map_MANDELVIEW = { 2, { { -0x10, 0 }, { -0x18, 0 } } };
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA_MANDELVIEW, 0, map_MANDELVIEW)

static const FormulaParamMap map_MANOWARBULB = {
    13, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 },
        { -0x2C, 1 }, { -0x30, 1 }, { -0x34, 1 }, { -0x38, 1 },
        { -0x3C, 1 }, { -0x40, 1 }, { -0x44, 1 }, { -0x48, 1 },
        { -0x4C, 1 }
    }
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA_MANOWARBULB, 0, map_MANOWARBULB)

static const FormulaParamMap map_MSLTOEFOLDQUAT = { 0, {} };
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA_MSLTOEFOLDQUAT, 0, map_MSLTOEFOLDQUAT)

static const FormulaParamMap map_MSLTOESYM2 = { 1, { { -0x10, 0 } } };
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA_MSLTOESYM2, 0, map_MSLTOESYM2)

static const FormulaParamMap map_MSLTOESYM3 = { 1, { { -0x10, 0 } } };
DEFINE_EXTERNAL_FORMULA_WRAPPER_EX(FORMULA_MSLTOESYM3, 0, map_MSLTOESYM3, 0, 0)

static const FormulaParamMap map_MSLTOESYM4 = { 3, { { -0x0C, 1 }, { -0x10, 1 }, { -0x14, 1 } } };
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA_MSLTOESYM4, 0, map_MSLTOESYM4)

static const FormulaParamMap map_MSLTOE_SYM = {
    5, {
        { 0x00, 0 }, { 0x08, 0 }, { 0x10, 0 }, { 0x18, 0 },
        { 0x20, 0 }
    }
};
static const FormulaConstEntry consts_MSLTOE_SYM[] = {
    { 0, UINT64_C(0x3FD921FB54442D18) },
    { 0, UINT64_C(0x3FE921FB54442D18) },
    { 0, UINT64_C(0x3FF0000000000000) },
    { 0, UINT64_C(0x4000000000000000) },
    { 0, UINT64_C(0x401921FB54442D18) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER_EX(FORMULA_MSLTOE_SYM, 0, map_MSLTOE_SYM, 0, 0)

static const FormulaParamMap map_PSEUDOXDB = { 1, { { -0x10, 0 } } };
DEFINE_EXTERNAL_FORMULA_WRAPPER_EX(FORMULA_PSEUDOXDB, 0, map_PSEUDOXDB, 0, 0)

static const FormulaParamMap map_QUAD3DB = {
    6, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 },
        { -0x30, 0 }, { -0x34, 2 }
    }
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA_QUAD3DB, 0, map_QUAD3DB)

static const FormulaParamMap map_QUADRAT3D = {
    5, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 },
        { -0x30, 0 }
    }
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA_QUADRAT3D, 0, map_QUADRAT3D)

static const FormulaParamMap map_QUICKDUDLEY = { 0, {} };
DEFINE_EXTERNAL_FORMULA_WRAPPER_EX(FORMULA_QUICKDUDLEY, 0, map_QUICKDUDLEY, 0, 0)

static const FormulaParamMap map_QUICKDUDLEYKM = { 0, {} };
DEFINE_EXTERNAL_FORMULA_WRAPPER_EX(FORMULA_QUICKDUDLEYKM, 0, map_QUICKDUDLEYKM, 0, 0)

static const FormulaParamMap map_QUINTIC = {
    7, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 },
        { -0x2C, 2 }, { 0x00, 0 }, { 0x08, 0 }
    }
};
static const FormulaConstEntry consts_QUINTIC[] = {
    { 0, UINT64_C(0xC024000000000000) },
    { 0, UINT64_C(0x4014000000000000) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA_QUINTIC, 0, map_QUINTIC)

static const FormulaParamMap map_RIEMANN = { 2, { { -0x10, 0 }, { -0x08, 0 } } };
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA_RIEMANN, 0, map_RIEMANN)

static const FormulaParamMap map_RIEMANN2 = {
    19, {
        { -0x0C, 1 }, { -0x10, 1 }, { -0x14, 1 }, { -0x18, 1 },
        { -0x1C, 1 }, { -0x20, 1 }, { -0x24, 1 }, { -0x28, 1 },
        { -0x2C, 1 }, { -0x34, 0 }, { -0x3C, 0 }, { -0x44, 0 },
        { -0x4C, 0 }, { -0x54, 0 }, { -0x5C, 0 }, { -0x64, 0 },
        { -0x6C, 0 }, { 0x00, 0 }, { 0x08, 0 }
    }
};
static const FormulaConstEntry consts_RIEMANN2[] = {
    { 0, UINT64_C(0x400921FB54442D28) },
    { 0, UINT64_C(0x4000000000000000) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA_RIEMANN2, 0, map_RIEMANN2)

static const FormulaParamMap map_RUCKERBROT1 = {
    5, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 },
        { 0x00, 0 }
    }
};
static const FormulaConstEntry consts_RUCKERBROT1[] = {
    { 0, UINT64_C(0x3DDB7CDFD9D7BDBB) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA_RUCKERBROT1, 0, map_RUCKERBROT1)

static const FormulaParamMap map_RUCKERBULB = {
    6, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x24, 2 },
        { -0x28, 2 }, { 0x00, 0 }
    }
};
static const FormulaConstEntry consts_RUCKERBULB[] = {
    { 0, UINT64_C(0x3FF0000000000000) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA_RUCKERBULB, 0, map_RUCKERBULB)

static const FormulaParamMap map_TALITTPX = {
    8, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 },
        { -0x30, 0 }, { -0x38, 0 }, { -0x40, 0 }, { -0x44, 2 }
    }
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA_TALITTPX, 0, map_TALITTPX)

static const FormulaParamMap map_TETRAFOLDINGPOW2 = {
    18, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 },
        { -0x30, 0 }, { -0x38, 0 }, { -0x3C, 1 }, { -0x40, 1 },
        { -0x44, 1 }, { -0x48, 1 }, { -0x4C, 1 }, { -0x50, 1 },
        { -0x54, 1 }, { -0x58, 1 }, { -0x5C, 1 }, { -0x60, 2 },
        { -0x64, 2 }, { -0x6C, 0 }
    }
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA_TETRAFOLDINGPOW2, 0, map_TETRAFOLDINGPOW2)

static const FormulaParamMap map_TGLADDIHED = {
    9, {
        { -0x0C, 2 }, { -0x10, 2 }, { 0x00, 0 }, { 0x08, 0 },
        { 0x10, 0 }, { 0x18, 0 }, { 0x20, 0 }, { 0x28, 0 },
        { 0x30, 0 }
    }
};
static const FormulaConstEntry consts_TGLADDIHED[] = {
    { 0, UINT64_C(0x3FE0000000000000) },
    { 0, UINT64_C(0xC008000000000000) },
    { 0, UINT64_C(0x4006A09E667F3BCD) },
    { 0, UINT64_C(0x3FD6A09E667F3BCD) },
    { 0, UINT64_C(0x400974B2334F2346) },
    { 0, UINT64_C(0x4000000000000000) },
    { 0, UINT64_C(0x4202A05F20000000) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA_TGLADDIHED, 0, map_TGLADDIHED)

static const FormulaParamMap map_TGLADDIHED2 = {
    9, {
        { -0x0C, 2 }, { -0x10, 2 }, { 0x00, 0 }, { 0x08, 0 },
        { 0x10, 0 }, { 0x18, 0 }, { 0x20, 0 }, { 0x28, 0 },
        { 0x30, 0 }
    }
};
static const FormulaConstEntry consts_TGLADDIHED2[] = {
    { 0, UINT64_C(0x3FE0000000000000) },
    { 0, UINT64_C(0xC008000000000000) },
    { 0, UINT64_C(0x4006A09E667F3BCD) },
    { 0, UINT64_C(0x3FD6A09E667F3BCD) },
    { 0, UINT64_C(0x400974B2334F2346) },
    { 0, UINT64_C(0x4000000000000000) },
    { 0, UINT64_C(0x4202A05F20000000) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA_TGLADDIHED2, 0, map_TGLADDIHED2)

static const FormulaParamMap map_TGLADTETRA = {
    9, {
        { -0x0C, 2 }, { -0x10, 2 }, { 0x00, 0 }, { 0x08, 0 },
        { 0x10, 0 }, { 0x18, 0 }, { 0x20, 0 }, { 0x28, 0 },
        { 0x30, 0 }
    }
};
static const FormulaConstEntry consts_TGLADTETRA[] = {
    { 0, UINT64_C(0x3FE0000000000000) },
    { 0, UINT64_C(0xC008000000000000) },
    { 0, UINT64_C(0x4006A09E667F3BCD) },
    { 0, UINT64_C(0x3FD6A09E667F3BCD) },
    { 0, UINT64_C(0x400974B2334F2346) },
    { 0, UINT64_C(0x4000000000000000) },
    { 0, UINT64_C(0x4202A05F20000000) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA_TGLADTETRA, 0, map_TGLADTETRA)

static const FormulaParamMap map_TGLADTETRA2 = {
    9, {
        { -0x0C, 2 }, { -0x10, 2 }, { 0x00, 0 }, { 0x08, 0 },
        { 0x10, 0 }, { 0x18, 0 }, { 0x20, 0 }, { 0x28, 0 },
        { 0x30, 0 }
    }
};
static const FormulaConstEntry consts_TGLADTETRA2[] = {
    { 0, UINT64_C(0x3FE0000000000000) },
    { 0, UINT64_C(0xC008000000000000) },
    { 0, UINT64_C(0x4006A09E667F3BCD) },
    { 0, UINT64_C(0x3FD6A09E667F3BCD) },
    { 0, UINT64_C(0x400974B2334F2346) },
    { 0, UINT64_C(0x4000000000000000) },
    { 0, UINT64_C(0x4202A05F20000000) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA_TGLADTETRA2, 0, map_TGLADTETRA2)

static const FormulaParamMap map_TRIFOXCOMPLEXANGLES = {
    8, {
        { -0x0C, 2 }, { -0x10, 1 }, { -0x14, 1 }, { -0x18, 2 },
        { -0x1C, 2 }, { -0x20, 2 }, { -0x24, 1 }, { -0x28, 1 }
    }
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA_TRIFOXCOMPLEXANGLES, 0, map_TRIFOXCOMPLEXANGLES)

static const FormulaParamMap map_IFSTESS = {
    13, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 },
        { -0x30, 0 }, { -0x38, 0 }, { -0x40, 0 }, { -0x44, 2 },
        { -0x48, 2 }, { 0x00, 0 }, { 0x08, 0 }, { 0x10, 0 },
        { 0x18, 0 }
    }
};
static const FormulaConstEntry consts_IFSTESS[] = {
    { 0, UINT64_C(0x4000000000000000) },
    { 0, UINT64_C(0x3FE6A09E6665983E) },
    { 0, UINT64_C(0x3F91DF46AA91ED06) },
    { 0, UINT64_C(0x407F400000000000) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA_IFSTESS, 0, map_IFSTESS)

static const FormulaParamMap map_MOBIUSTPX = {
    14, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 },
        { -0x30, 0 }, { -0x38, 0 }, { -0x40, 0 }, { -0x48, 0 },
        { -0x50, 0 }, { -0x58, 0 }, { -0x60, 0 }, { -0x68, 0 },
        { 0x00, 0 }, { 0x08, 0 }
    }
};
static const FormulaConstEntry consts_MOBIUSTPX[] = {
    { 0, UINT64_C(0x2D404BD984990E6F) },
    { 0, UINT64_C(0x529F6B0F092959C7) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA_MOBIUSTPX, 0, map_MOBIUSTPX)

static const FormulaParamMap map_E3DTESS = {
    13, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 },
        { -0x30, 0 }, { -0x38, 0 }, { -0x3C, 2 }, { -0x40, 2 },
        { 0x00, 0 }, { 0x08, 0 }, { 0x10, 0 }, { 0x18, 0 },
        { 0x20, 0 }
    }
};
static const FormulaConstEntry consts_E3DTESS[] = {
    { 0, UINT64_C(0x3FE0000000000000) },
    { 0, UINT64_C(0x4000000000000000) },
    { 0, UINT64_C(0x3FD0000000000000) },
    { 0, UINT64_C(0x3FC0000000000000) },
    { 0, UINT64_C(0x3CD203AF9EE75616) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA_E3DTESS, 0, map_E3DTESS)

static const FormulaParamMap map__MULC3D = {
    15, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 },
        { -0x30, 0 }, { -0x34, 1 }, { -0x38, 1 }, { -0x3C, 1 },
        { -0x40, 1 }, { -0x44, 1 }, { -0x48, 1 }, { -0x4C, 1 },
        { -0x50, 1 }, { -0x54, 1 }, { -0x5C, 0 }
    }
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__MULC3D, 0, map__MULC3D)

static const FormulaParamMap map__QUADIST = {
    7, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 },
        { -0x30, 0 }, { -0x38, 0 }, { -0x40, 0 }
    }
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__QUADIST, 0, map__QUADIST)

static const FormulaParamMap map__SPHEREINV = {
    7, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 },
        { -0x30, 0 }, { -0x38, 0 }, { -0x3C, 2 }
    }
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__SPHEREINV, 1, map__SPHEREINV)

// ============================================================================
// Registration: RegisterExternalFormulas_3D()
// ============================================================================

void RegisterExternalFormulas_3D() {
    RegisterExternalFormula({
        FORMULA_AEX_OCTO, "Aex-Octo", 0, 2,
        1.0, 1.0, 2.0, 100.0,
        ExternWrap_FORMULA_AEX_OCTO, &map_AEX_OCTO,
        2, {0, 0},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA_AEX_TORUSPOW, "Aex-TorusPow", 0, 7,
        1.0, 1.0, 5.0, 100.0,
        ExternWrap_FORMULA_AEX_TORUSPOW, &map_AEX_TORUSPOW,
        5, {0, 0, 0, 0, 2},
        0x0000,
        2, consts_AEX_TORUSPOW
    });

    RegisterExternalFormula({
        FORMULA_BPINE_ONLY, "BPine_only", 0, 3,
        1.0, 1.0, 2.0, 100.0,
        ExternWrap_FORMULA_BPINE_ONLY, &map_BPINE_ONLY,
        3, {0, 0, 0},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA_BT1PINE, "BT1Pine", 0, 9,
        1.0, 1.0, 2.0, 100.0,
        ExternWrap_FORMULA_BT1PINE, &map_BT1PINE,
        6, {0, 0, 0, 0, 0, 2},
        0x0000,
        3, consts_BT1PINE
    });

    RegisterExternalFormula({
        FORMULA_BT1PINEGON, "BT1Pinegon", 0, 16,
        1.0, 1.0, 2.0, 100.0,
        ExternWrap_FORMULA_BT1PINEGON, &map_BT1PINEGON,
        12, {0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 0},
        0x0000,
        4, consts_BT1PINEGON
    });

    RegisterExternalFormula({
        FORMULA_BT1PINEHEDRON, "BT1Pinehedron", 0, 17,
        1.0, 1.0, 2.0, 100.0,
        ExternWrap_FORMULA_BT1PINEHEDRON, &map_BT1PINEHEDRON,
        13, {0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 0, 0},
        0x0000,
        4, consts_BT1PINEHEDRON
    });

    RegisterExternalFormula({
        FORMULA_BT2PINE, "BT2Pine", 0, 8,
        1.0, 1.0, 2.0, 100.0,
        ExternWrap_FORMULA_BT2PINE, &map_BT2PINE,
        5, {0, 0, 0, 0, 0},
        0x0000,
        3, consts_BT2PINE
    });

    RegisterExternalFormula({
        FORMULA_BENESI2POW2, "Benesi2Pow2", 0, 0,
        1.0, 1.0, 2.0, 100.0,
        ExternWrap_FORMULA_BENESI2POW2, &map_BENESI2POW2,
        0, {},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA_BENESI2POW6, "Benesi2Pow6", 0, 1,
        1.0, 1.0, 6.0, 100.0,
        ExternWrap_FORMULA_BENESI2POW6, &map_BENESI2POW6,
        0, {},
        0x0000,
        1, consts_BENESI2POW6
    });

    RegisterExternalFormula({
        FORMULA_BENESI3POW2, "Benesi3Pow2", 0, 0,
        1.0, 1.0, 2.0, 100.0,
        ExternWrap_FORMULA_BENESI3POW2, &map_BENESI3POW2,
        0, {},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA_BENESIPINE1, "BenesiPine1", 0, 5,
        1.0, 1.0, 2.0, 100.0,
        ExternWrap_FORMULA_BENESIPINE1, &map_BENESIPINE1,
        2, {0, 0},
        0x0000,
        3, consts_BENESIPINE1
    });

    RegisterExternalFormula({
        FORMULA_BENESIPINE2, "BenesiPine2", 0, 5,
        1.0, 1.0, 2.0, 100.0,
        ExternWrap_FORMULA_BENESIPINE2, &map_BENESIPINE2,
        2, {0, 0},
        0x0000,
        3, consts_BENESIPINE2
    });

    RegisterExternalFormula({
        FORMULA_BETH1032, "Beth1032", 0, 13,
        1.0, 1.0, 6.0, 100.0,
        ExternWrap_FORMULA_BETH1032, &map_BETH1032,
        0, {},
        0x0000,
        13, consts_BETH1032
    });

    RegisterExternalFormula({
        FORMULA_BETH1522, "Beth1522", 0, 5,
        1.0, 1.0, 4.0, 100.0,
        ExternWrap_FORMULA_BETH1522, &map_BETH1522,
        0, {},
        0x0000,
        5, consts_BETH1522
    });

    RegisterExternalFormula({
        FORMULA_BETH322, "Beth322", 0, 1,
        1.0, 1.0, 3.0, 100.0,
        ExternWrap_FORMULA_BETH322, &map_BETH322,
        0, {},
        0x0000,
        1, consts_BETH322
    });

    RegisterExternalFormula({
        FORMULA_BETH323, "Beth323", 0, 3,
        1.0, 1.0, 7.0, 100.0,
        ExternWrap_FORMULA_BETH323, &map_BETH323,
        0, {},
        0x0000,
        3, consts_BETH323
    });

    RegisterExternalFormula({
        FORMULA_BETH432, "Beth432", 0, 6,
        1.0, 1.0, 4.0, 100.0,
        ExternWrap_FORMULA_BETH432, &map_BETH432,
        0, {},
        0x0000,
        6, consts_BETH432
    });

    RegisterExternalFormula({
        FORMULA_BETH651, "Beth651", 0, 8,
        1.0, 1.0, 4.0, 100.0,
        ExternWrap_FORMULA_BETH651, &map_BETH651,
        0, {},
        0x0000,
        8, consts_BETH651
    });

    RegisterExternalFormula({
        FORMULA_BULBOXP2, "BulboxP-2", 0, 10,
        1.0, 1.0, 2.0, 1024.0,
        ExternWrap_FORMULA_BULBOXP2, &map_BULBOXP2,
        10, {0, 0, 0, 0, 0, 0, 0, 0, 0, 2},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA_COSINEPOW2, "CosinePow2", 0, 1,
        1.0, 1.0, 2.0, 100.0,
        ExternWrap_FORMULA_COSINEPOW2, &map_COSINEPOW2,
        1, {0},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA_COSINEPOW8, "CosinePow8", 0, 6,
        1.0, 1.0, 8.0, 100.0,
        ExternWrap_FORMULA_COSINEPOW8, &map_COSINEPOW8,
        1, {0},
        0x0000,
        5, consts_COSINEPOW8
    });

    RegisterExternalFormula({
        FORMULA_FAEHRTEN, "Faehrten", 0, 4,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA_FAEHRTEN, &map_FAEHRTEN,
        4, {0, 0, 0, 0},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA_FUZZY, "Fuzzy", 0, 2,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA_FUZZY, &map_FUZZY,
        2, {0, 0},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA_GENINTPOWER, "GenIntPower", 0, 13,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA_GENINTPOWER, &map_GENINTPOWER,
        10, {2, 2, 2, 2, 2, 2, 2, 2, 2, 0},
        0x0000,
        3, consts_GENINTPOWER
    });

    RegisterExternalFormula({
        FORMULA_GENERAL, "General", 0, 12,
        1.0, 1.0, 2.0, 100.0,
        ExternWrap_FORMULA_GENERAL, &map_GENERAL,
        11, {0, 0, 0, 2, 2, 2, 2, 2, 2, 2, 2},
        0x0000,
        1, consts_GENERAL
    });

    RegisterExternalFormula({
        FORMULA_IQBULB, "IQ-bulb", 0, 6,
        1.5, 1.0, 2.0, 100.0,
        ExternWrap_FORMULA_IQBULB, &map_IQBULB,
        5, {0, 0, 0, 2, 2},
        0x0000,
        1, consts_IQBULB
    });

    RegisterExternalFormula({
        FORMULA_IQBULB_OLD, "IQ-bulb old", 0, 6,
        1.5, 1.0, 2.0, 100.0,
        ExternWrap_FORMULA_IQBULB_OLD, &map_IQBULB_OLD,
        5, {0, 0, 0, 2, 2},
        0x0000,
        1, consts_IQBULB_OLD
    });

    RegisterExternalFormula({
        FORMULA_IQ_NORMBULB, "IQ_NormBulb", 0, 6,
        1.5, 1.0, 2.0, 100.0,
        ExternWrap_FORMULA_IQ_NORMBULB, &map_IQ_NORMBULB,
        5, {0, 0, 0, 2, 2},
        0x0000,
        1, consts_IQ_NORMBULB
    });

    RegisterExternalFormula({
        FORMULA_IQ_NORMBULB_NOYZ, "IQ_NormBulb_NoYZ", 0, 6,
        1.5, 1.0, 2.0, 100.0,
        ExternWrap_FORMULA_IQ_NORMBULB_NOYZ, &map_IQ_NORMBULB_NOYZ,
        5, {0, 0, 0, 2, 2},
        0x0000,
        1, consts_IQ_NORMBULB_NOYZ
    });

    RegisterExternalFormula({
        FORMULA_IQ_PINETREE, "IQ_PineTree", 0, 6,
        1.5, 1.0, 2.0, 100.0,
        ExternWrap_FORMULA_IQ_PINETREE, &map_IQ_PINETREE,
        5, {0, 0, 0, 2, 2},
        0x0000,
        1, consts_IQ_PINETREE
    });

    RegisterExternalFormula({
        FORMULA_IQ_PINETREE_NOYZ, "IQ_PineTree_NoYZ", 0, 6,
        1.5, 1.0, 2.0, 100.0,
        ExternWrap_FORMULA_IQ_PINETREE_NOYZ, &map_IQ_PINETREE_NOYZ,
        5, {0, 0, 0, 2, 2},
        0x0000,
        1, consts_IQ_PINETREE_NOYZ
    });

    RegisterExternalFormula({
        FORMULA_IDESFORMULA, "IdesFormula", 0, 5,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA_IDESFORMULA, &map_IDESFORMULA,
        5, {0, 0, 0, 0, 0},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA_IKENAGABULB, "Ikenagabulb", 0, 7,
        1.0, 1.0, 2.0, 100.0,
        ExternWrap_FORMULA_IKENAGABULB, &map_IKENAGABULB,
        6, {0, 0, 0, 0, 0, 0},
        0x0000,
        1, consts_IKENAGABULB
    });

    RegisterExternalFormula({
        FORMULA_KALIDUCKS, "KaliDucks", 0, 2,
        0.5, 1.0, 0.0, 2.0,
        ExternWrap_FORMULA_KALIDUCKS, &map_KALIDUCKS,
        2, {0, 0},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA_KALILINCOMBSSE2SM, "KaliLinCombSSE2sm", 0, 3,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA_KALILINCOMBSSE2SM, &map_KALILINCOMBSSE2SM,
        1, {2},
        0x0000,
        1, consts_KALILINCOMBSSE2SM
    });

    RegisterExternalFormula({
        FORMULA_KALISETS1, "Kalisets1", 0, 2,
        1.0, 1.0, 0.0, 2.0,
        ExternWrap_FORMULA_KALISETS1, &map_KALISETS1,
        2, {0, 0},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA_MAGVSXYZ, "MagVsXYZ", 0, 7,
        0.5, 1.0, 2.0, 100.0,
        ExternWrap_FORMULA_MAGVSXYZ, &map_MAGVSXYZ,
        7, {0, 0, 0, 0, 0, 0, 0},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA_MAGVSXYZABS, "MagVsXYZabs", 0, 8,
        0.5, 1.0, 2.0, 100.0,
        ExternWrap_FORMULA_MAGVSXYZABS, &map_MAGVSXYZABS,
        7, {0, 0, 0, 0, 0, 0, 0},
        0x0000,
        1, consts_MAGVSXYZABS
    });

    RegisterExternalFormula({
        FORMULA_MAGVSXYZABS2, "MagVsXYZabs2", 0, 8,
        0.5, 1.0, 2.0, 100.0,
        ExternWrap_FORMULA_MAGVSXYZABS2, &map_MAGVSXYZABS2,
        7, {0, 0, 0, 0, 0, 0, 0},
        0x0000,
        1, consts_MAGVSXYZABS2
    });

    RegisterExternalFormula({
        FORMULA_MAGVSXYZABS3, "MagVsXYZabs3", 0, 7,
        0.5, 1.0, 2.0, 100.0,
        ExternWrap_FORMULA_MAGVSXYZABS3, &map_MAGVSXYZABS3,
        7, {0, 0, 0, 0, 0, 0, 0},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA_MAGVSXYZMSW, "MagVsXYZmsw", 0, 7,
        1.0, 1.0, 2.0, 100.0,
        ExternWrap_FORMULA_MAGVSXYZMSW, &map_MAGVSXYZMSW,
        7, {0, 0, 0, 0, 0, 0, 0},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA_MAKIN_NSR_TRI, "Makin-NSR-tri", 0, 2,
        1.0, 1.0, 2.0, 100.0,
        ExternWrap_FORMULA_MAKIN_NSR_TRI, &map_MAKIN_NSR_TRI,
        2, {0, 0},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA_MAKIN3D_1, "Makin3D-1", 0, 0,
        1.0, 1.0, 2.0, 100.0,
        ExternWrap_FORMULA_MAKIN3D_1, &map_MAKIN3D_1,
        0, {},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA_MAKIN3D_2, "Makin3D-2", 0, 0,
        2.0, 1.0, 2.0, 100.0,
        ExternWrap_FORMULA_MAKIN3D_2, &map_MAKIN3D_2,
        0, {},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA_MAKIN3D_3_4, "Makin3D-3-4", 0, 3,
        1.0, 1.0, 2.0, 100.0,
        ExternWrap_FORMULA_MAKIN3D_3_4, &map_MAKIN3D_3_4,
        3, {2, 2, 0},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA_MANDELVIEW, "MandelView", 0, 2,
        1.0, 1.0, 2.0, 100.0,
        ExternWrap_FORMULA_MANDELVIEW, &map_MANDELVIEW,
        2, {0, 0},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA_MANOWARBULB, "ManowarBulb", 0, 13,
        1.0, 1.0, 2.0, 100.0,
        ExternWrap_FORMULA_MANOWARBULB, &map_MANOWARBULB,
        5, {0, 0, 0, 0, 6},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA_MSLTOEFOLDQUAT, "MsltoeFoldQuat", 0, 0,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA_MSLTOEFOLDQUAT, &map_MSLTOEFOLDQUAT,
        0, {},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA_MSLTOESYM2, "MsltoeSym2", 0, 1,
        1.0, 1.0, 2.0, 100.0,
        ExternWrap_FORMULA_MSLTOESYM2, &map_MSLTOESYM2,
        1, {0},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA_MSLTOESYM3, "MsltoeSym3", 0, 1,
        1.0, 1.0, 2.0, 100.0,
        ExternWrap_FORMULA_MSLTOESYM3, &map_MSLTOESYM3,
        1, {0},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA_MSLTOESYM4, "MsltoeSym4", 0, 3,
        1.0, 1.0, 2.0, 100.0,
        ExternWrap_FORMULA_MSLTOESYM4, &map_MSLTOESYM4,
        3, {1, 1, 1},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA_MSLTOE_SYM, "Msltoe_Sym", 0, 5,
        1.0, 1.0, 2.0, 100.0,
        ExternWrap_FORMULA_MSLTOE_SYM, &map_MSLTOE_SYM,
        0, {},
        0x0000,
        5, consts_MSLTOE_SYM
    });

    RegisterExternalFormula({
        FORMULA_PSEUDOXDB, "PseudoXDB", 0, 1,
        1.0, 1.0, 2.0, 100.0,
        ExternWrap_FORMULA_PSEUDOXDB, &map_PSEUDOXDB,
        1, {0},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA_QUAD3DB, "Quad3Db", 0, 6,
        1.0, 1.0, 2.0, 100.0,
        ExternWrap_FORMULA_QUAD3DB, &map_QUAD3DB,
        6, {0, 0, 0, 0, 0, 2},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA_QUADRAT3D, "Quadrat3D", 0, 5,
        1.0, 1.0, 2.0, 100.0,
        ExternWrap_FORMULA_QUADRAT3D, &map_QUADRAT3D,
        5, {0, 0, 0, 0, 0},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA_QUICKDUDLEY, "QuickDudley", 0, 0,
        2.0, 1.0, 2.0, 100.0,
        ExternWrap_FORMULA_QUICKDUDLEY, &map_QUICKDUDLEY,
        0, {},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA_QUICKDUDLEYKM, "QuickDudleyKM", 0, 0,
        2.0, 1.0, 2.0, 100.0,
        ExternWrap_FORMULA_QUICKDUDLEYKM, &map_QUICKDUDLEYKM,
        0, {},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA_QUINTIC, "Quintic", 0, 7,
        1.0, 1.0, 5.0, 100.0,
        ExternWrap_FORMULA_QUINTIC, &map_QUINTIC,
        5, {0, 0, 0, 0, 2},
        0x0000,
        2, consts_QUINTIC
    });

    RegisterExternalFormula({
        FORMULA_RIEMANN, "Riemann", 0, 2,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA_RIEMANN, &map_RIEMANN,
        1, {0},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA_RIEMANN2, "Riemann2", 0, 19,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA_RIEMANN2, &map_RIEMANN2,
        9, {6, 0, 0, 0, 0, 0, 0, 0, 0},
        0x0000,
        2, consts_RIEMANN2
    });

    RegisterExternalFormula({
        FORMULA_RUCKERBROT1, "RuckerBrot1", 0, 5,
        1.0, 1.0, 2.0, 100.0,
        ExternWrap_FORMULA_RUCKERBROT1, &map_RUCKERBROT1,
        4, {0, 0, 0, 0},
        0x0000,
        1, consts_RUCKERBROT1
    });

    RegisterExternalFormula({
        FORMULA_RUCKERBULB, "Ruckerbulb", 0, 6,
        1.5, 1.0, 2.0, 100.0,
        ExternWrap_FORMULA_RUCKERBULB, &map_RUCKERBULB,
        5, {0, 0, 0, 2, 2},
        0x0000,
        1, consts_RUCKERBULB
    });

    RegisterExternalFormula({
        FORMULA_TALITTPX, "TalisTpx", 0, 8,
        1.5, 1.0, 2.0, 100.0,
        ExternWrap_FORMULA_TALITTPX, &map_TALITTPX,
        8, {0, 0, 0, 0, 0, 0, 0, 2},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA_TETRAFOLDINGPOW2, "TetraFolding Pow2", 0, 18,
        1.0, 1.0, 2.0, 100.0,
        ExternWrap_FORMULA_TETRAFOLDINGPOW2, &map_TETRAFOLDINGPOW2,
        10, {0, 0, 0, 0, 0, 0, 6, 2, 2, 0},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA_TGLADDIHED, "TgladDihed", 0, 9,
        1.0, 1.0, 2.0, 100.0,
        ExternWrap_FORMULA_TGLADDIHED, &map_TGLADDIHED,
        2, {2, 2},
        0x0002,
        7, consts_TGLADDIHED
    });

    RegisterExternalFormula({
        FORMULA_TGLADDIHED2, "TgladDihed2", 0, 9,
        1.0, 1.0, 2.0, 100.0,
        ExternWrap_FORMULA_TGLADDIHED2, &map_TGLADDIHED2,
        2, {2, 2},
        0x0002,
        7, consts_TGLADDIHED2
    });

    RegisterExternalFormula({
        FORMULA_TGLADTETRA, "TgladTetra", 0, 9,
        1.0, 1.0, 2.0, 100.0,
        ExternWrap_FORMULA_TGLADTETRA, &map_TGLADTETRA,
        2, {2, 2},
        0x0002,
        7, consts_TGLADTETRA
    });

    RegisterExternalFormula({
        FORMULA_TGLADTETRA2, "TgladTetra2", 0, 9,
        1.0, 1.0, 2.0, 100.0,
        ExternWrap_FORMULA_TGLADTETRA2, &map_TGLADTETRA2,
        2, {2, 2},
        0x0002,
        7, consts_TGLADTETRA2
    });

    RegisterExternalFormula({
        FORMULA_TRIFOXCOMPLEXANGLES, "TrifoxComplexAngles", 0, 8,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA_TRIFOXCOMPLEXANGLES, &map_TRIFOXCOMPLEXANGLES,
        7, {2, 21, 2, 2, 2, 1, 1},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA_IFSTESS, "_IFS_tess", 0, 13,
        1.0, 1.0, 1.0, 100.0,
        ExternWrap_FORMULA_IFSTESS, &map_IFSTESS,
        9, {0, 0, 0, 0, 0, 0, 0, 2, 2},
        0x0000,
        4, consts_IFSTESS
    });

    RegisterExternalFormula({
        FORMULA_MOBIUSTPX, "_MobiusTpx", 0, 14,
        1.0, 1.0, 1.0, 100.0,
        ExternWrap_FORMULA_MOBIUSTPX, &map_MOBIUSTPX,
        12, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        0x0000,
        2, consts_MOBIUSTPX
    });

    RegisterExternalFormula({
        FORMULA_E3DTESS, "_e_3Dtess", 0, 13,
        1.0, 1.0, 1.0, 100.0,
        ExternWrap_FORMULA_E3DTESS, &map_E3DTESS,
        8, {0, 0, 0, 0, 0, 0, 2, 2},
        0x0000,
        5, consts_E3DTESS
    });

    RegisterExternalFormula({
        FORMULA__MULC3D, "_mulC3D", 0, 15,
        0.1, 1.0, 2.0, 100.0,
        ExternWrap_FORMULA__MULC3D, &map__MULC3D,
        7, {0, 0, 0, 0, 0, 6, 0},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA__QUADIST, "_quadist", 0, 7,
        0.05, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA__QUADIST, &map__QUADIST,
        7, {0, 0, 0, 0, 0, 0, 0},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA__SPHEREINV, "_sphereinv", 1, 7,
        0.2, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA__SPHEREINV, &map__SPHEREINV,
        6, {0, 0, 0, 13, 22, 2},
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
