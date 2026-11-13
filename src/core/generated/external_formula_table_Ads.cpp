// external_formula_table_Ads.cpp — Auto-generated bulk formula registrations
// Category: Ads (160 formulas)
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
// Include Ads formula .inc files
// ============================================================================

#include "Ads/_Abs4d.inc"
#include "Ads/_AbsY.inc"
#include "Ads/_AbsZ.inc"
#include "Ads/_BPolygonFromCircle.inc"
#include "Ads/_BPolygonToCircle.inc"
#include "Ads/_BPolyhedronFromSphere.inc"
#include "Ads/_BPolyhedronToSphere.inc"
#include "Ads/_BRotateFromMag.inc"
#include "Ads/_BRotateToMag.inc"
#include "Ads/_BSkewXmaxV1.inc"
#include "Ads/_BStellahedron_F_Sphere.inc"
#include "Ads/_BT1_4D_Transform1.inc"
#include "Ads/_BT1_4D_clampXYZ.inc"
#include "Ads/_BT1_Transform1.inc"
#include "Ads/_BT2_Transform2.inc"
#include "Ads/_BT3_Transform3.inc"
#include "Ads/_BT4_Transform4.inc"
#include "Ads/_BT5b_Transform5b.inc"
#include "Ads/_BenesiSpheric.inc"
#include "Ads/_BenesiUnSpheric.inc"
#include "Ads/_CelticMode.inc"
#include "Ads/_CondItcountR.inc"
#include "Ads/_CondItcountSwX2.inc"
#include "Ads/_CondItcountSwitch.inc"
#include "Ads/_CondLyapunov.inc"
#include "Ads/_CondLyapunovEx.inc"
#include "Ads/_DEComb_ISph_Plane.inc"
#include "Ads/_DonutTransform.inc"
#include "Ads/_FlipXY.inc"
#include "Ads/_FlipXZ.inc"
#include "Ads/_FlipXZc.inc"
#include "Ads/_FlipYZ.inc"
#include "Ads/_FlipYZc.inc"
#include "Ads/_Fold45double.inc"
#include "Ads/_Fold45single.inc"
#include "Ads/_Folding2.inc"
#include "Ads/_FoldingCTetra3d.inc"
#include "Ads/_FoldingIcosa.inc"
#include "Ads/_FoldingInfy.inc"
#include "Ads/_FoldingOct.inc"
#include "Ads/_FoldingTetra3d.inc"
#include "Ads/_FoldingTetraKaleid.inc"
#include "Ads/_FoldingVar.inc"
#include "Ads/_GenCWarp.inc"
#include "Ads/_HexaTransform.inc"
#include "Ads/_HopSqrtX.inc"
#include "Ads/_HopSqrtY.inc"
#include "Ads/_HopSqrtZ.inc"
#include "Ads/_IntPowRotateC.inc"
#include "Ads/_JuliaSets.inc"
#include "Ads/_LinCombineCxyz.inc"
#include "Ads/_LinCombineXYZ.inc"
#include "Ads/_LogX.inc"
#include "Ads/_LogY.inc"
#include "Ads/_LogZ.inc"
#include "Ads/_MapTranslate.inc"
#include "Ads/_MaxClipping.inc"
#include "Ads/_NeoSqrX.inc"
#include "Ads/_NeoSqrY.inc"
#include "Ads/_NeoSqrZ.inc"
#include "Ads/_PartlyJulia.inc"
#include "Ads/_PartlyJuliaR.inc"
#include "Ads/_PartlyJuliaRoff.inc"
#include "Ads/_PlusSinApp.inc"
#include "Ads/_PolyFold-sym.inc"
#include "Ads/_PolyFolding.inc"
#include "Ads/_QuadrayTransform.inc"
#include "Ads/_QuadrayTransform2.inc"
#include "Ads/_Rotate4d.inc"
#include "Ads/_RotateBack.inc"
#include "Ads/_RotateC.inc"
#include "Ads/_RotatePlaneOnIts.inc"
#include "Ads/_RotateRnd.inc"
#include "Ads/_RotateXYonIts.inc"
#include "Ads/_RotateXYonItsP.inc"
#include "Ads/_RotatedAbs.inc"
#include "Ads/_RotatedFolding.inc"
#include "Ads/_Rpow3.inc"
#include "Ads/_SinApprox.inc"
#include "Ads/_SinY.inc"
#include "Ads/_SinePow2.inc"
#include "Ads/_SinhX.inc"
#include "Ads/_SinhY.inc"
#include "Ads/_SinhZ.inc"
#include "Ads/_SphereFolding1.inc"
#include "Ads/_SphereFolding2.inc"
#include "Ads/_SphereInvC.inc"
#include "Ads/_SquareR.inc"
#include "Ads/_Transform.inc"
#include "Ads/_Translate.inc"
#include "Ads/_TranslateC4d.inc"
#include "Ads/_TranslateRnd.inc"
#include "Ads/_YplusSinZ.inc"
#include "Ads/_YplusSinZ2.inc"
#include "Ads/_addC.inc"
#include "Ads/_asdamTrans.inc"
#include "Ads/_boxtiling.inc"
#include "Ads/_conj.inc"
#include "Ads/_conj2D.inc"
#include "Ads/_cosmartin.inc"
#include "Ads/_cylripple.inc"
#include "Ads/_dynamic.inc"
#include "Ads/_dynamicfast.inc"
#include "Ads/_flipC-xyz.inc"
#include "Ads/_gnarl2D.inc"
#include "Ads/_gnarl2D_2.inc"
#include "Ads/_gnarl2D_2fast.inc"
#include "Ads/_gnarl2Dfast.inc"
#include "Ads/_gnarl3D.inc"
#include "Ads/_gnarl3Dfast.inc"
#include "Ads/_helispiral.inc"
#include "Ads/_hopalm1.inc"
#include "Ads/_hopalm2.inc"
#include "Ads/_hopalong.inc"
#include "Ads/_hyc3Dinv.inc"
#include "Ads/_invbisp.inc"
#include "Ads/_invcylindrical.inc"
#include "Ads/_invspherical.inc"
#include "Ads/_invtorical.inc"
#include "Ads/_invtorical2.inc"
#include "Ads/_juliax2.inc"
#include "Ads/_kamtor.inc"
#include "Ads/_kamtorcut.inc"
#include "Ads/_lorenz.inc"
#include "Ads/_ngon.inc"
#include "Ads/_ngon2.inc"
#include "Ads/_planeFold.inc"
#include "Ads/_poincare.inc"
#include "Ads/_rabinovich-f.inc"
#include "Ads/_recFold.inc"
#include "Ads/_reciprocalX.inc"
#include "Ads/_reciprocalX2.inc"
#include "Ads/_reciprocalX3.inc"
#include "Ads/_reciprocalX3b.inc"
#include "Ads/_reciprocalY2.inc"
#include "Ads/_reciprocalY3.inc"
#include "Ads/_reciprocalY3b.inc"
#include "Ads/_reciprocalZ2.inc"
#include "Ads/_reciprocalZ3.inc"
#include "Ads/_reciprocalZ3b.inc"
#include "Ads/_rossler.inc"
#include "Ads/_scaleC.inc"
#include "Ads/_skew.inc"
#include "Ads/_sphereXinv.inc"
#include "Ads/_sphereYinv.inc"
#include "Ads/_sphereZinv.inc"
#include "Ads/_sqr_3D.inc"
#include "Ads/_sqr_4D.inc"
#include "Ads/_tocylindrical.inc"
#include "Ads/_toruspinv1.inc"
#include "Ads/_toruspinv2.inc"
#include "Ads/_toruspinv3.inc"
#include "Ads/_tospherical.inc"
#include "Ads/_totorical.inc"
#include "Ads/_totorical2.inc"
#include "Ads/_updateC.inc"
#include "Ads/_updateC2.inc"
#include "Ads/_updateC_4d.inc"
#include "Ads/_vanderpol.inc"
#include "Ads/_vanderpol2.inc"

// ============================================================================
// PVar offset maps (Ads)
// ============================================================================

static const FormulaParamMap map__ABS4D = { 0, {} };
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__ABS4D, -1, map__ABS4D)

static const FormulaParamMap map__ABSY = { 1, { { -0x10, 0 } } };
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__ABSY, -1, map__ABSY)

static const FormulaParamMap map__ABSZ = { 1, { { -0x10, 0 } } };
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__ABSZ, -1, map__ABSZ)

static const FormulaParamMap map__BPOLYGONFROMCIRCLE = { 2, { { -0x10, 0 }, { 0x00, 0 } } };
static const FormulaConstEntry consts__BPOLYGONFROMCIRCLE[] = {
    { 0, UINT64_C(0x400921FB54442D28) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__BPOLYGONFROMCIRCLE, -1, map__BPOLYGONFROMCIRCLE)

static const FormulaParamMap map__BPOLYGONTOCIRCLE = { 2, { { -0x10, 0 }, { 0x00, 0 } } };
static const FormulaConstEntry consts__BPOLYGONTOCIRCLE[] = {
    { 0, UINT64_C(0x400921FB54442D28) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__BPOLYGONTOCIRCLE, -1, map__BPOLYGONTOCIRCLE)

static const FormulaParamMap map__BPOLYHEDRONFROMSPHERE = { 3, { { -0x10, 0 }, { -0x18, 0 }, { 0x00, 0 } } };
static const FormulaConstEntry consts__BPOLYHEDRONFROMSPHERE[] = {
    { 0, UINT64_C(0x400921FB54442D28) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__BPOLYHEDRONFROMSPHERE, -1, map__BPOLYHEDRONFROMSPHERE)

static const FormulaParamMap map__BPOLYHEDRONTOSPHERE = { 3, { { -0x10, 0 }, { -0x18, 0 }, { 0x00, 0 } } };
static const FormulaConstEntry consts__BPOLYHEDRONTOSPHERE[] = {
    { 0, UINT64_C(0x400921FB54442D28) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__BPOLYHEDRONTOSPHERE, -1, map__BPOLYHEDRONTOSPHERE)

static const FormulaParamMap map__BROTATEFFROMMAG = { 3, { { 0x00, 0 }, { 0x08, 0 }, { 0x10, 0 } } };
static const FormulaConstEntry consts__BROTATEFFROMMAG[] = {
    { 0, UINT64_C(0x3FEA20BD700C2C3E) },
    { 0, UINT64_C(0x3FE279A74590331D) },
    { 0, UINT64_C(0x3FE6A09E667F3BCC) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__BROTATEFFROMMAG, -1, map__BROTATEFFROMMAG)

static const FormulaParamMap map__BROTATETOMAG = { 3, { { 0x00, 0 }, { 0x08, 0 }, { 0x10, 0 } } };
static const FormulaConstEntry consts__BROTATETOMAG[] = {
    { 0, UINT64_C(0x3FEA20BD700C2C3E) },
    { 0, UINT64_C(0x3FE279A74590331D) },
    { 0, UINT64_C(0x3FE6A09E667F3BCC) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__BROTATETOMAG, -1, map__BROTATETOMAG)

static const FormulaParamMap map__BSKEWXMAXV1 = {
    9, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 },
        { -0x30, 0 }, { -0x38, 0 }, { -0x40, 0 }, { -0x48, 0 },
        { -0x50, 0 }
    }
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__BSKEWXMAXV1, -1, map__BSKEWXMAXV1)

static const FormulaParamMap map__BSTELLAHEDRON_F_SPHERE = {
    6, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 },
        { 0x00, 0 }, { 0x08, 0 }
    }
};
static const FormulaConstEntry consts__BSTELLAHEDRON_F_SPHERE[] = {
    { 0, UINT64_C(0x400921FB54442D28) },
    { 0, UINT64_C(0x3FE0000000000000) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__BSTELLAHEDRON_F_SPHERE, -1, map__BSTELLAHEDRON_F_SPHERE)

static const FormulaParamMap map__BT1_4D_TRANSFORM1 = {
    7, {
        { -0x10, 0 }, { -0x18, 0 }, { 0x00, 0 }, { 0x08, 0 },
        { 0x10, 0 }, { 0x18, 0 }, { 0x20, 0 }
    }
};
static const FormulaConstEntry consts__BT1_4D_TRANSFORM1[] = {
    { 0, UINT64_C(0x3FEA20BD700C2C3E) },
    { 0, UINT64_C(0x3FE279A74590331D) },
    { 0, UINT64_C(0x3FE6A09E667F3BCC) },
    { 0, UINT64_C(0x3FEBB67AE8584CAB) },
    { 0, UINT64_C(0x3FE0000000000000) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__BT1_4D_TRANSFORM1, -1, map__BT1_4D_TRANSFORM1)

static const FormulaParamMap map__BT1_4D_CLAMPXYZ = {
    7, {
        { -0x10, 0 }, { -0x18, 0 }, { 0x00, 0 }, { 0x08, 0 },
        { 0x10, 0 }, { 0x18, 0 }, { 0x20, 0 }
    }
};
static const FormulaConstEntry consts__BT1_4D_CLAMPXYZ[] = {
    { 0, UINT64_C(0x3FEA20BD700C2C3E) },
    { 0, UINT64_C(0x3FE279A74590331D) },
    { 0, UINT64_C(0x3FE6A09E667F3BCC) },
    { 0, UINT64_C(0x3FEBB67AE8584CAB) },
    { 0, UINT64_C(0x3FE0000000000000) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__BT1_4D_CLAMPXYZ, -1, map__BT1_4D_CLAMPXYZ)

static const FormulaParamMap map__BT1_TRANSFORM1 = {
    5, {
        { -0x10, 0 }, { -0x18, 0 }, { 0x00, 0 }, { 0x08, 0 },
        { 0x10, 0 }
    }
};
static const FormulaConstEntry consts__BT1_TRANSFORM1[] = {
    { 0, UINT64_C(0x3FEA20BD700C2C3E) },
    { 0, UINT64_C(0x3FE279A74590331D) },
    { 0, UINT64_C(0x3FE6A09E667F3BCC) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__BT1_TRANSFORM1, -1, map__BT1_TRANSFORM1)

static const FormulaParamMap map__BT2_TRANSFORM2 = {
    5, {
        { -0x10, 0 }, { -0x18, 0 }, { 0x00, 0 }, { 0x08, 0 },
        { 0x10, 0 }
    }
};
static const FormulaConstEntry consts__BT2_TRANSFORM2[] = {
    { 0, UINT64_C(0x3FEA20BD700C2C3E) },
    { 0, UINT64_C(0x3FE279A74590331D) },
    { 0, UINT64_C(0x3FE6A09E667F3BCC) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__BT2_TRANSFORM2, -1, map__BT2_TRANSFORM2)

static const FormulaParamMap map__BT3_TRANSFORM3 = {
    5, {
        { -0x10, 0 }, { -0x18, 0 }, { 0x00, 0 }, { 0x08, 0 },
        { 0x10, 0 }
    }
};
static const FormulaConstEntry consts__BT3_TRANSFORM3[] = {
    { 0, UINT64_C(0x3FEA20BD700C2C3E) },
    { 0, UINT64_C(0x3FE279A74590331D) },
    { 0, UINT64_C(0x3FE6A09E667F3BCC) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__BT3_TRANSFORM3, -1, map__BT3_TRANSFORM3)

static const FormulaParamMap map__BT4_TRANSFORM4 = {
    5, {
        { -0x10, 0 }, { -0x18, 0 }, { 0x00, 0 }, { 0x08, 0 },
        { 0x10, 0 }
    }
};
static const FormulaConstEntry consts__BT4_TRANSFORM4[] = {
    { 0, UINT64_C(0x3FEA20BD700C2C3E) },
    { 0, UINT64_C(0x3FE279A74590331D) },
    { 0, UINT64_C(0x3FE6A09E667F3BCC) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__BT4_TRANSFORM4, -1, map__BT4_TRANSFORM4)

static const FormulaParamMap map__BT5B_TRANSFORM5B = {
    5, {
        { -0x10, 0 }, { -0x18, 0 }, { 0x00, 0 }, { 0x08, 0 },
        { 0x10, 0 }
    }
};
static const FormulaConstEntry consts__BT5B_TRANSFORM5B[] = {
    { 0, UINT64_C(0x3FEA20BD700C2C3E) },
    { 0, UINT64_C(0x3FE279A74590331D) },
    { 0, UINT64_C(0x3FE6A09E667F3BCC) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__BT5B_TRANSFORM5B, -1, map__BT5B_TRANSFORM5B)

static const FormulaParamMap map__BENESISPHERIC = { 1, { { -0x10, 0 } } };
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__BENESISPHERIC, -1, map__BENESISPHERIC)

static const FormulaParamMap map__BENESIUNSPHERIC = { 1, { { -0x10, 0 } } };
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__BENESIUNSPHERIC, -1, map__BENESIUNSPHERIC)

static const FormulaParamMap map__CELTICMODE = { 4, { { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 } } };
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__CELTICMODE, -1, map__CELTICMODE)

static const FormulaParamMap map__CONDITCOUNTR = {
    6, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x1C, 2 }, { -0x20, 2 },
        { -0x24, 2 }, { -0x28, 2 }
    }
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__CONDITCOUNTR, -1, map__CONDITCOUNTR)

static const FormulaParamMap map__CONDITCOUNTSWX2 = {
    10, {
        { -0x0C, 2 }, { -0x10, 2 }, { -0x14, 2 }, { -0x18, 2 },
        { -0x1C, 2 }, { -0x20, 2 }, { -0x24, 2 }, { -0x28, 2 },
        { -0x2C, 2 }, { -0x30, 2 }
    }
};
DEFINE_EXTERNAL_FORMULA_WRAPPER_EX(FORMULA__CONDITCOUNTSWX2, -1, map__CONDITCOUNTSWX2, 0, 0)

static const FormulaParamMap map__CONDITCOUNTSWITCH = {
    5, {
        { -0x0C, 2 }, { -0x10, 2 }, { -0x14, 2 }, { -0x18, 2 },
        { -0x1C, 2 }
    }
};
DEFINE_EXTERNAL_FORMULA_WRAPPER_EX(FORMULA__CONDITCOUNTSWITCH, -1, map__CONDITCOUNTSWITCH, 0, 0)

static const FormulaParamMap map__CONDLYAPUNOV = {
    5, {
        { -0x0C, 2 }, { -0x10, 2 }, { -0x14, 2 }, { -0x18, 2 },
        { -0x1C, 2 }
    }
};
DEFINE_EXTERNAL_FORMULA_WRAPPER_EX(FORMULA__CONDLYAPUNOV, -1, map__CONDLYAPUNOV, 0, 0)

static const FormulaParamMap map__CONDLYAPUNOVEX = {
    5, {
        { -0x0C, 2 }, { -0x10, 2 }, { -0x14, 2 }, { -0x18, 2 },
        { -0x1C, 2 }
    }
};
DEFINE_EXTERNAL_FORMULA_WRAPPER_EX(FORMULA__CONDLYAPUNOVEX, -1, map__CONDLYAPUNOVEX, 0, 0)

static const FormulaParamMap map_DECOMB_ISPH_PLANE = {
    7, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 },
        { -0x30, 0 }, { -0x38, 0 }, { -0x40, 0 }
    }
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA_DECOMB_ISPH_PLANE, 0, map_DECOMB_ISPH_PLANE)

static const FormulaParamMap map__DONUTTRANSFORM = { 3, { { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 } } };
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__DONUTTRANSFORM, -1, map__DONUTTRANSFORM)

static const FormulaParamMap map__FLIPXY = { 0, {} };
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__FLIPXY, -1, map__FLIPXY)

static const FormulaParamMap map__FLIPXZ = { 0, {} };
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__FLIPXZ, -1, map__FLIPXZ)

static const FormulaParamMap map__FLIPXZC = { 1, { { -0x0C, 2 } } };
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__FLIPXZC, -1, map__FLIPXZC)

static const FormulaParamMap map__FLIPYZ = { 0, {} };
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__FLIPYZ, -1, map__FLIPYZ)

static const FormulaParamMap map__FLIPYZC = { 1, { { -0x0C, 2 } } };
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__FLIPYZC, -1, map__FLIPYZC)

static const FormulaParamMap map__FOLD45DOUBLE = {
    7, {
        { -0x10, 0 }, { -0x14, 2 }, { -0x18, 2 }, { -0x1C, 2 },
        { -0x20, 2 }, { -0x28, 0 }, { 0x00, 0 }
    }
};
static const FormulaConstEntry consts__FOLD45DOUBLE[] = {
    { 0, UINT64_C(0x3FE5555555555555) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__FOLD45DOUBLE, -1, map__FOLD45DOUBLE)

static const FormulaParamMap map__FOLD45SINGLE = {
    7, {
        { -0x10, 0 }, { -0x14, 2 }, { -0x18, 2 }, { -0x1C, 2 },
        { -0x20, 2 }, { -0x24, 2 }, { 0x00, 0 }
    }
};
static const FormulaConstEntry consts__FOLD45SINGLE[] = {
    { 0, UINT64_C(0x3FE5555555555555) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__FOLD45SINGLE, -1, map__FOLD45SINGLE)

static const FormulaParamMap map__FOLDING2 = { 4, { { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 } } };
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__FOLDING2, -1, map__FOLDING2)

static const FormulaParamMap map__FOLDINGCTETRA3D = { 3, { { -0x0C, 2 }, { -0x10, 2 }, { -0x14, 2 } } };
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__FOLDINGCTETRA3D, -1, map__FOLDINGCTETRA3D)

static const FormulaParamMap map__FOLDINGICOSA = {
    12, {
        { -0x0C, 1 }, { -0x10, 1 }, { -0x14, 1 }, { -0x18, 1 },
        { -0x1C, 1 }, { -0x20, 1 }, { -0x24, 1 }, { -0x28, 1 },
        { -0x2C, 1 }, { 0x00, 0 }, { 0x08, 0 }, { 0x10, 0 }
    }
};
static const FormulaConstEntry consts__FOLDINGICOSA[] = {
    { 0, UINT64_C(0x3FD3C6EF372FE948) },
    { 0, UINT64_C(0xBFE0000000000000) },
    { 0, UINT64_C(0xBFE9E3779B97F4A4) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__FOLDINGICOSA, -1, map__FOLDINGICOSA)

static const FormulaParamMap map__FOLDINGINFY = { 4, { { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 } } };
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__FOLDINGINFY, -1, map__FOLDINGINFY)

static const FormulaParamMap map__FOLDINGOCT = { 0, {} };
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__FOLDINGOCT, -1, map__FOLDINGOCT)

static const FormulaParamMap map__FOLDINGTETRA3D = { 2, { { -0x0C, 2 }, { -0x10, 2 } } };
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__FOLDINGTETRA3D, -1, map__FOLDINGTETRA3D)

static const FormulaParamMap map__FOLDINGTETRAKALEID = { 4, { { -0x10, 0 }, { -0x14, 2 }, { -0x18, 2 }, { 0x00, 0 } } };
static const FormulaConstEntry consts__FOLDINGTETRAKALEID[] = {
    { 0, UINT64_C(0x3FE5555555555555) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__FOLDINGTETRAKALEID, -1, map__FOLDINGTETRAKALEID)

static const FormulaParamMap map__FOLDINGVAR = {
    12, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 },
        { -0x30, 0 }, { -0x38, 0 }, { -0x40, 0 }, { -0x48, 0 },
        { -0x50, 0 }, { -0x58, 0 }, { -0x60, 0 }, { -0x68, 0 }
    }
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__FOLDINGVAR, -1, map__FOLDINGVAR)

static const FormulaParamMap map__GENCWARP = {
    17, {
        { -0x0C, 1 }, { -0x10, 1 }, { -0x14, 1 }, { -0x18, 1 },
        { -0x1C, 1 }, { -0x20, 1 }, { -0x24, 1 }, { -0x28, 1 },
        { -0x2C, 1 }, { -0x34, 0 }, { -0x3C, 0 }, { -0x40, 2 },
        { -0x44, 2 }, { -0x48, 2 }, { -0x50, 0 }, { -0x58, 0 },
        { 0x00, 0 }
    }
};
static const FormulaConstEntry consts__GENCWARP[] = {
    { 0, UINT64_C(0x3FF921FAFC8B007A) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__GENCWARP, -1, map__GENCWARP)

static const FormulaParamMap map__HEXATRANSFORM = { 3, { { -0x10, 0 }, { 0x00, 0 }, { 0x08, 0 } } };
static const FormulaConstEntry consts__HEXATRANSFORM[] = {
    { 0, UINT64_C(0x3FEBB67AE8584CAA) },
    { 0, UINT64_C(0xBFE0000000000000) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__HEXATRANSFORM, -1, map__HEXATRANSFORM)

static const FormulaParamMap map__HOPSQRTX = { 4, { { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 } } };
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__HOPSQRTX, -1, map__HOPSQRTX)

static const FormulaParamMap map__HOPSQRTY = { 4, { { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 } } };
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__HOPSQRTY, -1, map__HOPSQRTY)

static const FormulaParamMap map__HOPSQRTZ = { 4, { { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 } } };
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__HOPSQRTZ, -1, map__HOPSQRTZ)

static const FormulaParamMap map__INTPOWROTATEC = {
    13, {
        { -0x0C, 2 }, { -0x10, 2 }, { -0x14, 2 }, { -0x18, 2 },
        { -0x1C, 2 }, { -0x20, 2 }, { -0x24, 2 }, { -0x28, 2 },
        { -0x2C, 2 }, { -0x34, 0 }, { 0x00, 3 }, { 0x08, 3 },
        { 0x10, 0 }
    }
};
static const FormulaConstEntry consts__INTPOWROTATEC[] = {
    { 3, UINT64_C(0x0000000000000000) },
    { 3, UINT64_C(0x8000000000000000) },
    { 0, UINT64_C(0x3FF0000000000000) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__INTPOWROTATEC, -1, map__INTPOWROTATEC)

static const FormulaParamMap map__JULIASETS = { 2, { { -0x10, 0 }, { -0x18, 0 } } };
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__JULIASETS, -1, map__JULIASETS)

static const FormulaParamMap map__LINCOMBINECXYZ = {
    9, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 },
        { -0x30, 0 }, { -0x38, 0 }, { -0x40, 0 }, { -0x48, 0 },
        { -0x50, 0 }
    }
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__LINCOMBINECXYZ, -1, map__LINCOMBINECXYZ)

static const FormulaParamMap map__LINCOMBINEXYZ = {
    9, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 },
        { -0x30, 0 }, { -0x38, 0 }, { -0x40, 0 }, { -0x48, 0 },
        { -0x50, 0 }
    }
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__LINCOMBINEXYZ, -1, map__LINCOMBINEXYZ)

static const FormulaParamMap map__LOGX = { 4, { { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 } } };
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__LOGX, -1, map__LOGX)

static const FormulaParamMap map__LOGY = { 4, { { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 } } };
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__LOGY, -1, map__LOGY)

static const FormulaParamMap map__LOGZ = { 4, { { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 } } };
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__LOGZ, -1, map__LOGZ)

static const FormulaParamMap map__MAPTRANSLATE = {
    7, {
        { -0x0C, 2 }, { -0x10, 1 }, { -0x14, 1 }, { -0x18, 1 },
        { -0x1C, 1 }, { -0x20, 1 }, { -0x24, 1 }
    }
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__MAPTRANSLATE, -1, map__MAPTRANSLATE)

static const FormulaParamMap map__MAXCLIPPING = { 3, { { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 } } };
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__MAXCLIPPING, -1, map__MAXCLIPPING)

static const FormulaParamMap map__NEOSQRX = { 4, { { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 } } };
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__NEOSQRX, -1, map__NEOSQRX)

static const FormulaParamMap map__NEOSQRY = { 4, { { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 } } };
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__NEOSQRY, -1, map__NEOSQRY)

static const FormulaParamMap map__NEOSQRZ = { 4, { { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 } } };
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__NEOSQRZ, -1, map__NEOSQRZ)

static const FormulaParamMap map__PARTLYJULIA = {
    9, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 },
        { -0x30, 0 }, { -0x38, 0 }, { -0x40, 0 }, { -0x48, 0 },
        { -0x50, 0 }
    }
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__PARTLYJULIA, -1, map__PARTLYJULIA)

static const FormulaParamMap map__PARTLYJULIAR = {
    11, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 },
        { -0x30, 0 }, { -0x38, 0 }, { -0x40, 0 }, { -0x48, 0 },
        { -0x4C, 2 }, { -0x54, 0 }, { -0x58, 2 }
    }
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__PARTLYJULIAR, -1, map__PARTLYJULIAR)

static const FormulaParamMap map__PARTLYJULIAROFF = {
    5, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 },
        { 0x00, 0 }
    }
};
static const FormulaConstEntry consts__PARTLYJULIAROFF[] = {
    { 0, UINT64_C(0x3DDB7CDFD9D7BDBB) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__PARTLYJULIAROFF, -1, map__PARTLYJULIAROFF)

static const FormulaParamMap map__PLUSSINAPP = {
    9, {
        { -0x0C, 2 }, { -0x10, 2 }, { -0x14, 1 }, { -0x18, 1 },
        { -0x1C, 1 }, { -0x20, 1 }, { 0x00, 0 }, { 0x08, 0 },
        { 0x10, 0 }
    }
};
static const FormulaConstEntry consts__PLUSSINAPP[] = {
    { 0, UINT64_C(0x3FC45F306DC9C881) },
    { 0, UINT64_C(0x3FF921FB54442D18) },
    { 0, UINT64_C(0x3FC2F684BDA12F67) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__PLUSSINAPP, -1, map__PLUSSINAPP)

static const FormulaParamMap map__POLYFOLDSYM = {
    7, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 },
        { -0x30, 0 }, { 0x00, 0 }, { 0x08, 0 }
    }
};
static const FormulaConstEntry consts__POLYFOLDSYM[] = {
    { 0, UINT64_C(0x401921FB54411744) },
    { 0, UINT64_C(0x3F91DF46AA91ED06) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__POLYFOLDSYM, -1, map__POLYFOLDSYM)

static const FormulaParamMap map__POLYFOLDING = {
    7, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 },
        { -0x30, 0 }, { 0x00, 0 }, { 0x08, 0 }
    }
};
static const FormulaConstEntry consts__POLYFOLDING[] = {
    { 0, UINT64_C(0x401921FB54411744) },
    { 0, UINT64_C(0x3F91DF46AA91ED06) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__POLYFOLDING, -1, map__POLYFOLDING)

static const FormulaParamMap map__QUADRAYTRANSFORM = { 0, {} };
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__QUADRAYTRANSFORM, -1, map__QUADRAYTRANSFORM)

static const FormulaParamMap map__QUADRAYTRANSFORM2 = { 2, { { -0x10, 0 }, { -0x18, 0 } } };
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__QUADRAYTRANSFORM2, -1, map__QUADRAYTRANSFORM2)

static const FormulaParamMap map__ROTATE4D = {
    16, {
        { -0x0C, 1 }, { -0x10, 1 }, { -0x14, 1 }, { -0x18, 1 },
        { -0x1C, 1 }, { -0x20, 1 }, { -0x24, 1 }, { -0x28, 1 },
        { -0x2C, 1 }, { -0x30, 1 }, { -0x34, 1 }, { -0x38, 1 },
        { -0x3C, 1 }, { -0x40, 1 }, { -0x44, 1 }, { -0x48, 1 }
    }
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__ROTATE4D, -1, map__ROTATE4D)

static const FormulaParamMap map__ROTATEBACK = {
    9, {
        { -0x0C, 1 }, { -0x10, 1 }, { -0x14, 1 }, { -0x18, 1 },
        { -0x1C, 1 }, { -0x20, 1 }, { -0x24, 1 }, { -0x28, 1 },
        { -0x2C, 1 }
    }
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__ROTATEBACK, -1, map__ROTATEBACK)

static const FormulaParamMap map__ROTATEC = {
    9, {
        { -0x0C, 1 }, { -0x10, 1 }, { -0x14, 1 }, { -0x18, 1 },
        { -0x1C, 1 }, { -0x20, 1 }, { -0x24, 1 }, { -0x28, 1 },
        { -0x2C, 1 }
    }
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__ROTATEC, -1, map__ROTATEC)

static const FormulaParamMap map__ROTATEPLANEONITS = {
    11, {
        { -0x0C, 1 }, { -0x10, 1 }, { -0x14, 1 }, { -0x18, 1 },
        { -0x1C, 1 }, { -0x20, 1 }, { -0x24, 1 }, { -0x28, 1 },
        { -0x2C, 1 }, { -0x34, 0 }, { 0x00, 0 }
    }
};
static const FormulaConstEntry consts__ROTATEPLANEONITS[] = {
    { 0, UINT64_C(0x3F91DF46A2529D39) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__ROTATEPLANEONITS, -1, map__ROTATEPLANEONITS)

static const FormulaParamMap map__ROTATERND = {
    9, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x24, 2 },
        { -0x28, 2 }, { -0x2C, 2 }, { 0x00, 0 }, { 0x08, 0 },
        { 0x10, 0 }
    }
};
static const FormulaConstEntry consts__ROTATERND[] = {
    { 0, UINT64_C(0x4000000000000000) },
    { 0, UINT64_C(0x3F20004208FDC455) },
    { 0, UINT64_C(0x3FF0000000000000) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__ROTATERND, -1, map__ROTATERND)

static const FormulaParamMap map__ROTATEXYONITS = {
    11, {
        { -0x0C, 1 }, { -0x10, 1 }, { -0x14, 1 }, { -0x18, 1 },
        { -0x1C, 1 }, { -0x20, 1 }, { -0x24, 1 }, { -0x28, 1 },
        { -0x2C, 1 }, { -0x34, 0 }, { 0x00, 0 }
    }
};
static const FormulaConstEntry consts__ROTATEXYONITS[] = {
    { 0, UINT64_C(0x3F91DF46A2529D39) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__ROTATEXYONITS, -1, map__ROTATEXYONITS)

static const FormulaParamMap map__ROTATEXYONITSP = {
    13, {
        { -0x0C, 1 }, { -0x10, 1 }, { -0x14, 1 }, { -0x18, 1 },
        { -0x1C, 1 }, { -0x20, 1 }, { -0x24, 1 }, { -0x28, 1 },
        { -0x2C, 1 }, { -0x34, 0 }, { -0x3C, 0 }, { -0x44, 0 },
        { 0x00, 0 }
    }
};
static const FormulaConstEntry consts__ROTATEXYONITSP[] = {
    { 0, UINT64_C(0x3F91DF46A2529D39) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__ROTATEXYONITSP, -1, map__ROTATEXYONITSP)

static const FormulaParamMap map__ROTATEDABS = {
    11, {
        { -0x0C, 1 }, { -0x10, 1 }, { -0x14, 1 }, { -0x18, 1 },
        { -0x1C, 1 }, { -0x20, 1 }, { -0x24, 1 }, { -0x28, 1 },
        { -0x2C, 1 }, { -0x34, 0 }, { -0x3C, 0 }
    }
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__ROTATEDABS, -1, map__ROTATEDABS)

static const FormulaParamMap map__ROTATEDFOLDING = {
    10, {
        { -0x0C, 1 }, { -0x10, 1 }, { -0x14, 1 }, { -0x18, 1 },
        { -0x1C, 1 }, { -0x20, 1 }, { -0x24, 1 }, { -0x28, 1 },
        { -0x2C, 1 }, { -0x34, 0 }
    }
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__ROTATEDFOLDING, -1, map__ROTATEDFOLDING)

static const FormulaParamMap map__RPOW3 = { 0, {} };
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__RPOW3, 0, map__RPOW3)

static const FormulaParamMap map__SINAPPROX = {
    8, {
        { -0x0C, 2 }, { -0x10, 1 }, { -0x14, 1 }, { -0x18, 1 },
        { -0x1C, 1 }, { 0x00, 0 }, { 0x08, 0 }, { 0x10, 0 }
    }
};
static const FormulaConstEntry consts__SINAPPROX[] = {
    { 0, UINT64_C(0x3FC45F306DC9C881) },
    { 0, UINT64_C(0x3FF921FB54442D18) },
    { 0, UINT64_C(0x3FC2F684BDA12F67) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__SINAPPROX, -1, map__SINAPPROX)

static const FormulaParamMap map__SINY = {
    5, {
        { -0x0C, 2 }, { -0x10, 1 }, { -0x14, 1 }, { -0x18, 1 },
        { -0x1C, 1 }
    }
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__SINY, -1, map__SINY)

static const FormulaParamMap map__SINEPOW2 = { 2, { { -0x10, 0 }, { -0x18, 0 } } };
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__SINEPOW2, 0, map__SINEPOW2)

static const FormulaParamMap map__SINHX = { 4, { { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 } } };
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__SINHX, -1, map__SINHX)

static const FormulaParamMap map__SINHY = { 4, { { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 } } };
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__SINHY, -1, map__SINHY)

static const FormulaParamMap map__SINHZ = { 4, { { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 } } };
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__SINHZ, -1, map__SINHZ)

static const FormulaParamMap map__SPHEREFOLDING1 = { 4, { { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 } } };
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__SPHEREFOLDING1, -1, map__SPHEREFOLDING1)

static const FormulaParamMap map__SPHEREFOLDING2 = { 4, { { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 } } };
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__SPHEREFOLDING2, -1, map__SPHEREFOLDING2)

static const FormulaParamMap map__SPHEREINVC = { 0, {} };
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__SPHEREINVC, 0, map__SPHEREINVC)

static const FormulaParamMap map__SQUARER = { 0, {} };
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__SQUARER, 0, map__SQUARER)

static const FormulaParamMap map__TRANSFORM = {
    14, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 },
        { -0x30, 0 }, { -0x38, 0 }, { -0x40, 0 }, { -0x48, 0 },
        { -0x50, 0 }, { -0x58, 0 }, { -0x60, 0 }, { -0x68, 0 },
        { -0x70, 0 }, { -0x74, 2 }
    }
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__TRANSFORM, -1, map__TRANSFORM)

static const FormulaParamMap map__TRANSLATE = { 3, { { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 } } };
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__TRANSLATE, -1, map__TRANSLATE)

static const FormulaParamMap map__TRANSLATEC4D = {
    8, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 },
        { -0x30, 0 }, { -0x38, 0 }, { -0x40, 0 }, { -0x48, 0 }
    }
};
DEFINE_EXTERNAL_FORMULA_WRAPPER_EX(FORMULA__TRANSLATEC4D, -1, map__TRANSLATEC4D, 0, 0)

static const FormulaParamMap map__TRANSLATERND = {
    8, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x24, 2 },
        { -0x28, 2 }, { -0x2C, 2 }, { 0x00, 0 }, { 0x08, 0 }
    }
};
static const FormulaConstEntry consts__TRANSLATERND[] = {
    { 0, UINT64_C(0x3FE0000000000000) },
    { 0, UINT64_C(0x3F0FFB480A5ACCD5) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__TRANSLATERND, -1, map__TRANSLATERND)

static const FormulaParamMap map__YPLUSSINZ = { 0, {} };
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__YPLUSSINZ, -1, map__YPLUSSINZ)

static const FormulaParamMap map__YPLUSSINZ2 = { 4, { { -0x10, 0 }, { -0x18, 0 }, { 0x00, 0 }, { 0x08, 0 } } };
static const FormulaConstEntry consts__YPLUSSINZ2[] = {
    { 0, UINT64_C(0x4000000000000000) },
    { 0, UINT64_C(0x3FE0000000000000) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__YPLUSSINZ2, -1, map__YPLUSSINZ2)

static const FormulaParamMap map__ADDC = { 3, { { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 } } };
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__ADDC, -1, map__ADDC)

static const FormulaParamMap map__ASDAMTRANS = {
    10, {
        { -0x0C, 1 }, { -0x10, 1 }, { -0x14, 1 }, { -0x18, 1 },
        { -0x1C, 1 }, { -0x20, 1 }, { -0x24, 1 }, { -0x28, 1 },
        { -0x2C, 1 }, { -0x34, 0 }
    }
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__ASDAMTRANS, -1, map__ASDAMTRANS)

static const FormulaParamMap map__BOXTILING = { 4, { { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 } } };
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__BOXTILING, -1, map__BOXTILING)

static const FormulaParamMap map__CONJ = { 0, {} };
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__CONJ, -1, map__CONJ)

static const FormulaParamMap map__CONJ2D = { 0, {} };
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__CONJ2D, -1, map__CONJ2D)

static const FormulaParamMap map__COSMARTIN = {
    5, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 },
        { -0x30, 0 }
    }
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__COSMARTIN, -1, map__COSMARTIN)

static const FormulaParamMap map_CYLRIPPLE = {
    15, {
        { -0x0C, 1 }, { -0x10, 1 }, { -0x14, 1 }, { -0x18, 1 },
        { -0x1C, 1 }, { -0x20, 1 }, { -0x24, 1 }, { -0x28, 1 },
        { -0x2C, 1 }, { -0x34, 0 }, { -0x3C, 0 }, { -0x44, 0 },
        { -0x4C, 0 }, { -0x50, 2 }, { -0x54, 2 }
    }
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA_CYLRIPPLE, 0, map_CYLRIPPLE)

static const FormulaParamMap map__DYNAMIC = {
    5, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 },
        { -0x30, 0 }
    }
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__DYNAMIC, -1, map__DYNAMIC)

static const FormulaParamMap map__DYNAMICFAST = {
    11, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 },
        { -0x30, 0 }, { 0x00, 0 }, { 0x08, 0 }, { 0x10, 0 },
        { 0x18, 0 }, { 0x20, 0 }, { 0x28, 0 }
    }
};
static const FormulaConstEntry consts__DYNAMICFAST[] = {
    { 0, UINT64_C(0x3FD45F306DC9C883) },
    { 0, UINT64_C(0x400921FB54442D18) },
    { 0, UINT64_C(0xC014AB646AFA6041) },
    { 0, UINT64_C(0x400458B339271BE9) },
    { 0, UINT64_C(0xBFE1D452596EB687) },
    { 0, UINT64_C(0x3FF921FB54442D18) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__DYNAMICFAST, -1, map__DYNAMICFAST)

static const FormulaParamMap map__FLIPCXYZ = { 0, {} };
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__FLIPCXYZ, -1, map__FLIPCXYZ)

static const FormulaParamMap map__GNARL2D = {
    5, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 },
        { -0x30, 0 }
    }
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__GNARL2D, -1, map__GNARL2D)

static const FormulaParamMap map__GNARL2D_2 = {
    6, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 },
        { -0x30, 0 }, { -0x38, 0 }
    }
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__GNARL2D_2, -1, map__GNARL2D_2)

static const FormulaParamMap map__GNARL2D_2FAST = {
    12, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 },
        { -0x30, 0 }, { -0x38, 0 }, { 0x00, 0 }, { 0x08, 0 },
        { 0x10, 0 }, { 0x18, 0 }, { 0x20, 0 }, { 0x28, 0 }
    }
};
static const FormulaConstEntry consts__GNARL2D_2FAST[] = {
    { 0, UINT64_C(0x3FD45F306DC9C883) },
    { 0, UINT64_C(0x400921FB54442D18) },
    { 0, UINT64_C(0xC014AB646AFA6041) },
    { 0, UINT64_C(0x400458B339271BE9) },
    { 0, UINT64_C(0xBFE1D452596EB687) },
    { 0, UINT64_C(0x3FF921FB54442D18) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__GNARL2D_2FAST, -1, map__GNARL2D_2FAST)

static const FormulaParamMap map__GNARL2DFAST = {
    10, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 },
        { -0x30, 0 }, { 0x00, 0 }, { 0x08, 0 }, { 0x10, 0 },
        { 0x18, 0 }, { 0x20, 0 }
    }
};
static const FormulaConstEntry consts__GNARL2DFAST[] = {
    { 0, UINT64_C(0x3FD45F306DC9C883) },
    { 0, UINT64_C(0x400921FB54442D18) },
    { 0, UINT64_C(0xC014AB646AFA6041) },
    { 0, UINT64_C(0x400458B339271BE9) },
    { 0, UINT64_C(0xBFE1D452596EB687) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__GNARL2DFAST, -1, map__GNARL2DFAST)

static const FormulaParamMap map__GNARL3D = {
    6, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 },
        { -0x30, 0 }, { -0x38, 0 }
    }
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__GNARL3D, -1, map__GNARL3D)

static const FormulaParamMap map__GNARL3DFAST = {
    11, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 },
        { -0x30, 0 }, { -0x38, 0 }, { 0x00, 0 }, { 0x08, 0 },
        { 0x10, 0 }, { 0x18, 0 }, { 0x20, 0 }
    }
};
static const FormulaConstEntry consts__GNARL3DFAST[] = {
    { 0, UINT64_C(0x3FD45F306DC9C883) },
    { 0, UINT64_C(0x400921FB54442D18) },
    { 0, UINT64_C(0xC014AB646AFA6041) },
    { 0, UINT64_C(0x400458B339271BE9) },
    { 0, UINT64_C(0xBFE1D452596EB687) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__GNARL3DFAST, -1, map__GNARL3DFAST)

static const FormulaParamMap map__HELISPIRAL = {
    15, {
        { -0x0C, 1 }, { -0x10, 1 }, { -0x14, 1 }, { -0x18, 1 },
        { -0x1C, 1 }, { -0x20, 1 }, { -0x24, 1 }, { -0x28, 1 },
        { -0x2C, 1 }, { -0x34, 0 }, { -0x3C, 0 }, { -0x40, 2 },
        { -0x44, 2 }, { -0x4C, 0 }, { 0x00, 0 }
    }
};
static const FormulaConstEntry consts__HELISPIRAL[] = {
    { 0, UINT64_C(0x3F91DF46AA91ED06) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__HELISPIRAL, -1, map__HELISPIRAL)

static const FormulaParamMap map__HOPALM1 = {
    6, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 },
        { -0x30, 0 }, { -0x38, 0 }
    }
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__HOPALM1, -1, map__HOPALM1)

static const FormulaParamMap map__HOPALM2 = {
    6, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 },
        { -0x30, 0 }, { -0x38, 0 }
    }
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__HOPALM2, -1, map__HOPALM2)

static const FormulaParamMap map__HOPALONG = {
    6, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 },
        { -0x30, 0 }, { -0x38, 0 }
    }
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__HOPALONG, -1, map__HOPALONG)

static const FormulaParamMap map__HYC3DINV = { 4, { { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 } } };
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__HYC3DINV, -1, map__HYC3DINV)

static const FormulaParamMap map__INVBISP = { 2, { { -0x10, 0 }, { 0x00, 0 } } };
static const FormulaConstEntry consts__INVBISP[] = {
    { 0, UINT64_C(0x3FF921FB54442D18) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__INVBISP, -1, map__INVBISP)

static const FormulaParamMap map__INVCYLINDRICAL = { 0, {} };
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__INVCYLINDRICAL, -1, map__INVCYLINDRICAL)

static const FormulaParamMap map__INVSPHERICAL = { 0, {} };
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__INVSPHERICAL, -1, map__INVSPHERICAL)

static const FormulaParamMap map__INVTORICAL = { 1, { { -0x10, 0 } } };
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__INVTORICAL, -1, map__INVTORICAL)

static const FormulaParamMap map__INVTORICAL2 = { 2, { { -0x10, 0 }, { -0x18, 0 } } };
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__INVTORICAL2, -1, map__INVTORICAL2)

static const FormulaParamMap map__JULIAX2 = {
    7, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 },
        { -0x30, 0 }, { -0x38, 0 }, { -0x40, 0 }
    }
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__JULIAX2, -1, map__JULIAX2)

static const FormulaParamMap map__KAMTOR = {
    14, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 },
        { -0x30, 0 }, { -0x38, 0 }, { -0x40, 0 }, { -0x48, 0 },
        { -0x50, 0 }, { -0x58, 0 }, { -0x60, 0 }, { -0x68, 0 },
        { -0x70, 0 }, { -0x78, 0 }
    }
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__KAMTOR, -1, map__KAMTOR)

static const FormulaParamMap map__KAMTORCUT = {
    16, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 },
        { -0x30, 0 }, { -0x38, 0 }, { -0x40, 0 }, { -0x48, 0 },
        { -0x50, 0 }, { -0x58, 0 }, { -0x60, 0 }, { -0x68, 0 },
        { -0x70, 0 }, { -0x78, 0 }, { 0x00, 0 }, { 0x08, 0 }
    }
};
static const FormulaConstEntry consts__KAMTORCUT[] = {
    { 0, UINT64_C(0x4059000000000000) },
    { 0, UINT64_C(0x3FEB333333333333) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__KAMTORCUT, -1, map__KAMTORCUT)

static const FormulaParamMap map__LORENZ = {
    5, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 },
        { -0x30, 0 }
    }
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__LORENZ, -1, map__LORENZ)

static const FormulaParamMap map__NGON = {
    8, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 },
        { -0x30, 0 }, { -0x34, 2 }, { 0x00, 0 }, { 0x08, 0 }
    }
};
static const FormulaConstEntry consts__NGON[] = {
    { 0, UINT64_C(0x401921FB54411744) },
    { 0, UINT64_C(0x3F91DF46AA91ED06) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__NGON, -1, map__NGON)

static const FormulaParamMap map__NGON2 = {
    7, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 },
        { -0x30, 0 }, { 0x00, 0 }, { 0x08, 0 }
    }
};
static const FormulaConstEntry consts__NGON2[] = {
    { 0, UINT64_C(0x401921FB54411744) },
    { 0, UINT64_C(0x3F91DF46AA91ED06) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__NGON2, -1, map__NGON2)

static const FormulaParamMap map__PLANEFOLD = { 3, { { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 } } };
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__PLANEFOLD, -1, map__PLANEFOLD)

static const FormulaParamMap map__POINCARE = {
    5, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 },
        { -0x30, 0 }
    }
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__POINCARE, -1, map__POINCARE)

static const FormulaParamMap map__RABINOVICH_F = {
    5, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 },
        { -0x30, 0 }
    }
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__RABINOVICH_F, -1, map__RABINOVICH_F)

static const FormulaParamMap map__RECFOLD = {
    7, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 },
        { -0x2C, 2 }, { 0x00, 0 }, { 0x08, 0 }
    }
};
static const FormulaConstEntry consts__RECFOLD[] = {
    { 0, UINT64_C(0x3FE0000000000000) },
    { 0, UINT64_C(0x3FE6A09E667F3BCD) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__RECFOLD, -1, map__RECFOLD)

static const FormulaParamMap map__RECIPROCALX = { 0, {} };
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__RECIPROCALX, -1, map__RECIPROCALX)

static const FormulaParamMap map__RECIPROCALX2 = { 1, { { -0x10, 0 } } };
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__RECIPROCALX2, -1, map__RECIPROCALX2)

static const FormulaParamMap map__RECIPROCALX3 = { 2, { { -0x10, 0 }, { -0x18, 0 } } };
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__RECIPROCALX3, -1, map__RECIPROCALX3)

static const FormulaParamMap map__RECIPROCALX3B = {
    6, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 },
        { -0x30, 0 }, { -0x38, 0 }
    }
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__RECIPROCALX3B, -1, map__RECIPROCALX3B)

static const FormulaParamMap map__RECIPROCALY2 = { 1, { { -0x10, 0 } } };
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__RECIPROCALY2, -1, map__RECIPROCALY2)

static const FormulaParamMap map__RECIPROCALY3 = { 2, { { -0x10, 0 }, { -0x18, 0 } } };
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__RECIPROCALY3, -1, map__RECIPROCALY3)

static const FormulaParamMap map__RECIPROCALY3B = {
    6, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 },
        { -0x30, 0 }, { -0x38, 0 }
    }
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__RECIPROCALY3B, -1, map__RECIPROCALY3B)

static const FormulaParamMap map__RECIPROCALZ2 = { 1, { { -0x10, 0 } } };
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__RECIPROCALZ2, -1, map__RECIPROCALZ2)

static const FormulaParamMap map__RECIPROCALZ3 = { 2, { { -0x10, 0 }, { -0x18, 0 } } };
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__RECIPROCALZ3, -1, map__RECIPROCALZ3)

static const FormulaParamMap map__RECIPROCALZ3B = {
    6, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 },
        { -0x30, 0 }, { -0x38, 0 }
    }
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__RECIPROCALZ3B, -1, map__RECIPROCALZ3B)

static const FormulaParamMap map__ROSSLER = {
    5, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 },
        { -0x30, 0 }
    }
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__ROSSLER, -1, map__ROSSLER)

static const FormulaParamMap map__SCALEC = { 3, { { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 } } };
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__SCALEC, -1, map__SCALEC)

static const FormulaParamMap map__SKEW = { 2, { { -0x10, 0 }, { -0x18, 0 } } };
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__SKEW, -1, map__SKEW)

static const FormulaParamMap map__SPHEREXINV = { 4, { { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 } } };
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__SPHEREXINV, -1, map__SPHEREXINV)

static const FormulaParamMap map__SPHEREYINV = { 4, { { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 } } };
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__SPHEREYINV, -1, map__SPHEREYINV)

static const FormulaParamMap map__SPHEREZINV = { 4, { { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 } } };
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__SPHEREZINV, -1, map__SPHEREZINV)

static const FormulaParamMap map__SQR_3D = { 4, { { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 } } };
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__SQR_3D, -1, map__SQR_3D)

static const FormulaParamMap map__SQR_4D = {
    5, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 },
        { -0x30, 0 }
    }
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__SQR_4D, -1, map__SQR_4D)

static const FormulaParamMap map__TOCYLINDRICAL = { 0, {} };
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__TOCYLINDRICAL, -1, map__TOCYLINDRICAL)

static const FormulaParamMap map__TORUSPINV1 = {
    5, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 },
        { -0x30, 0 }
    }
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__TORUSPINV1, -1, map__TORUSPINV1)

static const FormulaParamMap map__TORUSPINV2 = {
    5, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 },
        { -0x30, 0 }
    }
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__TORUSPINV2, -1, map__TORUSPINV2)

static const FormulaParamMap map__TORUSPINV3 = {
    5, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 },
        { -0x30, 0 }
    }
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__TORUSPINV3, -1, map__TORUSPINV3)

static const FormulaParamMap map__TOSPHERICAL = { 0, {} };
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__TOSPHERICAL, -1, map__TOSPHERICAL)

static const FormulaParamMap map__TOTORICAL = { 1, { { -0x10, 0 } } };
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__TOTORICAL, -1, map__TOTORICAL)

static const FormulaParamMap map__TOTORICAL2 = { 3, { { -0x10, 0 }, { -0x18, 0 }, { 0x00, 0 } } };
static const FormulaConstEntry consts__TOTORICAL2[] = {
    { 0, UINT64_C(0x3FF921FB54442D18) },
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__TOTORICAL2, -1, map__TOTORICAL2)

static const FormulaParamMap map__UPDATEC = { 2, { { -0x10, 0 }, { -0x18, 0 } } };
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__UPDATEC, -1, map__UPDATEC)

static const FormulaParamMap map__UPDATEC2 = {
    13, {
        { -0x0C, 1 }, { -0x10, 1 }, { -0x14, 1 }, { -0x18, 1 },
        { -0x1C, 1 }, { -0x20, 1 }, { -0x24, 1 }, { -0x28, 1 },
        { -0x2C, 1 }, { -0x34, 0 }, { -0x3C, 0 }, { -0x44, 0 },
        { -0x4C, 0 }
    }
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__UPDATEC2, -1, map__UPDATEC2)

static const FormulaParamMap map__UPDATEC_4D = { 0, {} };
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__UPDATEC_4D, -1, map__UPDATEC_4D)

static const FormulaParamMap map__VANDERPOL = {
    8, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 },
        { -0x30, 0 }, { -0x38, 0 }, { -0x40, 0 }, { -0x48, 0 }
    }
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__VANDERPOL, -1, map__VANDERPOL)

static const FormulaParamMap map__VANDERPOL2 = {
    9, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 },
        { -0x30, 0 }, { -0x38, 0 }, { -0x40, 0 }, { -0x48, 0 },
        { -0x50, 0 }
    }
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__VANDERPOL2, -1, map__VANDERPOL2)

// ============================================================================
// Registration: RegisterExternalFormulas_Ads()
// ============================================================================

void RegisterExternalFormulas_Ads() {
    RegisterExternalFormula({
        FORMULA__ABS4D, "_Abs4d", -1, 0,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA__ABS4D, &map__ABS4D,
        0, {},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA__ABSY, "_AbsY", -1, 1,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA__ABSY, &map__ABSY,
        1, {0},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA__ABSZ, "_AbsZ", -1, 1,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA__ABSZ, &map__ABSZ,
        1, {0},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA__BPOLYGONFROMCIRCLE, "_BPolygonFromCircle", -1, 2,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA__BPOLYGONFROMCIRCLE, &map__BPOLYGONFROMCIRCLE,
        1, {0},
        0x0000,
        1, consts__BPOLYGONFROMCIRCLE
    });

    RegisterExternalFormula({
        FORMULA__BPOLYGONTOCIRCLE, "_BPolygonToCircle", -1, 2,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA__BPOLYGONTOCIRCLE, &map__BPOLYGONTOCIRCLE,
        1, {0},
        0x0000,
        1, consts__BPOLYGONTOCIRCLE
    });

    RegisterExternalFormula({
        FORMULA__BPOLYHEDRONFROMSPHERE, "_BPolyhedronFromSphere", -1, 3,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA__BPOLYHEDRONFROMSPHERE, &map__BPOLYHEDRONFROMSPHERE,
        2, {0, 0},
        0x0000,
        1, consts__BPOLYHEDRONFROMSPHERE
    });

    RegisterExternalFormula({
        FORMULA__BPOLYHEDRONTOSPHERE, "_BPolyhedronToSphere", -1, 3,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA__BPOLYHEDRONTOSPHERE, &map__BPOLYHEDRONTOSPHERE,
        2, {0, 0},
        0x0000,
        1, consts__BPOLYHEDRONTOSPHERE
    });

    RegisterExternalFormula({
        FORMULA__BROTATEFFROMMAG, "_BRotateFromMag", -1, 3,
        1.0, 1.0, 2.0, 100.0,
        ExternWrap_FORMULA__BROTATEFFROMMAG, &map__BROTATEFFROMMAG,
        0, {},
        0x0000,
        3, consts__BROTATEFFROMMAG
    });

    RegisterExternalFormula({
        FORMULA__BROTATETOMAG, "_BRotateToMag", -1, 3,
        1.0, 1.0, 2.0, 100.0,
        ExternWrap_FORMULA__BROTATETOMAG, &map__BROTATETOMAG,
        0, {},
        0x0000,
        3, consts__BROTATETOMAG
    });

    RegisterExternalFormula({
        FORMULA__BSKEWXMAXV1, "_BSkewXmaxV1", -1, 9,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA__BSKEWXMAXV1, &map__BSKEWXMAXV1,
        9, {0, 0, 0, 0, 0, 0, 0, 0, 0},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA__BSTELLAHEDRON_F_SPHERE, "_BStellahedron_F_Sphere", -1, 6,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA__BSTELLAHEDRON_F_SPHERE, &map__BSTELLAHEDRON_F_SPHERE,
        4, {0, 0, 0, 0},
        0x0000,
        2, consts__BSTELLAHEDRON_F_SPHERE
    });

    RegisterExternalFormula({
        FORMULA__BT1_4D_TRANSFORM1, "_BT1_4D_Transform1", -1, 7,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA__BT1_4D_TRANSFORM1, &map__BT1_4D_TRANSFORM1,
        2, {0, 0},
        0x0000,
        5, consts__BT1_4D_TRANSFORM1
    });

    RegisterExternalFormula({
        FORMULA__BT1_4D_CLAMPXYZ, "_BT1_4D_clampXYZ", -1, 7,
        1.0, 1.0, 2.0, 100.0,
        ExternWrap_FORMULA__BT1_4D_CLAMPXYZ, &map__BT1_4D_CLAMPXYZ,
        2, {0, 0},
        0x0000,
        5, consts__BT1_4D_CLAMPXYZ
    });

    RegisterExternalFormula({
        FORMULA__BT1_TRANSFORM1, "_BT1_Transform1", -1, 5,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA__BT1_TRANSFORM1, &map__BT1_TRANSFORM1,
        2, {0, 0},
        0x0000,
        3, consts__BT1_TRANSFORM1
    });

    RegisterExternalFormula({
        FORMULA__BT2_TRANSFORM2, "_BT2_Transform2", -1, 5,
        1.0, 1.0, 2.0, 100.0,
        ExternWrap_FORMULA__BT2_TRANSFORM2, &map__BT2_TRANSFORM2,
        2, {0, 0},
        0x0000,
        3, consts__BT2_TRANSFORM2
    });

    RegisterExternalFormula({
        FORMULA__BT3_TRANSFORM3, "_BT3_Transform3", -1, 5,
        1.0, 1.0, 2.0, 100.0,
        ExternWrap_FORMULA__BT3_TRANSFORM3, &map__BT3_TRANSFORM3,
        2, {0, 0},
        0x0000,
        3, consts__BT3_TRANSFORM3
    });

    RegisterExternalFormula({
        FORMULA__BT4_TRANSFORM4, "_BT4_Transform4", -1, 5,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA__BT4_TRANSFORM4, &map__BT4_TRANSFORM4,
        2, {0, 0},
        0x0000,
        3, consts__BT4_TRANSFORM4
    });

    RegisterExternalFormula({
        FORMULA__BT5B_TRANSFORM5B, "_BT5b_Transform5b", -1, 5,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA__BT5B_TRANSFORM5B, &map__BT5B_TRANSFORM5B,
        2, {0, 0},
        0x0000,
        3, consts__BT5B_TRANSFORM5B
    });

    RegisterExternalFormula({
        FORMULA__BENESISPHERIC, "_BenesiSpheric", -1, 1,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA__BENESISPHERIC, &map__BENESISPHERIC,
        1, {0},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA__BENESIUNSPHERIC, "_BenesiUnSpheric", -1, 1,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA__BENESIUNSPHERIC, &map__BENESIUNSPHERIC,
        1, {0},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA__CELTICMODE, "_CelticMode", -1, 4,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA__CELTICMODE, &map__CELTICMODE,
        4, {0, 0, 0, 0},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA__CONDITCOUNTR, "_CondItcountR", -1, 6,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA__CONDITCOUNTR, &map__CONDITCOUNTR,
        6, {9, 9, 2, 2, 2, 2},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA__CONDITCOUNTSWX2, "_CondItcountSwX2", -1, 10,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA__CONDITCOUNTSWX2, &map__CONDITCOUNTSWX2,
        10, {2, 2, 2, 2, 2, 2, 2, 2, 2, 2},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA__CONDITCOUNTSWITCH, "_CondItcountSwitch", -1, 5,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA__CONDITCOUNTSWITCH, &map__CONDITCOUNTSWITCH,
        5, {2, 2, 2, 2, 2},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA__CONDLYAPUNOV, "_CondLyapunov", -1, 5,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA__CONDLYAPUNOV, &map__CONDLYAPUNOV,
        5, {2, 2, 2, 2, 2},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA__CONDLYAPUNOVEX, "_CondLyapunovEx", -1, 5,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA__CONDLYAPUNOVEX, &map__CONDLYAPUNOVEX,
        5, {2, 2, 2, 2, 2},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA_DECOMB_ISPH_PLANE, "_DEComb_ISph_Plane", 0, 7,
        1.0, 1.0, 2.0, 16.0,
        ExternWrap_FORMULA_DECOMB_ISPH_PLANE, &map_DECOMB_ISPH_PLANE,
        7, {0, 0, 0, 0, 0, 0, 0},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA__DONUTTRANSFORM, "_DonutTransform", -1, 3,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA__DONUTTRANSFORM, &map__DONUTTRANSFORM,
        3, {0, 0, 0},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA__FLIPXY, "_FlipXY", -1, 0,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA__FLIPXY, &map__FLIPXY,
        0, {},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA__FLIPXZ, "_FlipXZ", -1, 0,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA__FLIPXZ, &map__FLIPXZ,
        0, {},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA__FLIPXZC, "_FlipXZc", -1, 1,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA__FLIPXZC, &map__FLIPXZC,
        1, {2},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA__FLIPYZ, "_FlipYZ", -1, 0,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA__FLIPYZ, &map__FLIPYZ,
        0, {},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA__FLIPYZC, "_FlipYZc", -1, 1,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA__FLIPYZC, &map__FLIPYZC,
        1, {2},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA__FOLD45DOUBLE, "_Fold45double", -1, 7,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA__FOLD45DOUBLE, &map__FOLD45DOUBLE,
        6, {0, 2, 2, 2, 2, 0},
        0x0000,
        1, consts__FOLD45DOUBLE
    });

    RegisterExternalFormula({
        FORMULA__FOLD45SINGLE, "_Fold45single", -1, 7,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA__FOLD45SINGLE, &map__FOLD45SINGLE,
        6, {0, 2, 2, 2, 2, 2},
        0x0000,
        1, consts__FOLD45SINGLE
    });

    RegisterExternalFormula({
        FORMULA__FOLDING2, "_Folding2", -1, 4,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA__FOLDING2, &map__FOLDING2,
        1, {8},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA__FOLDINGCTETRA3D, "_FoldingCTetra3d", -1, 3,
        1.0, 1.0, 2.0, 100.0,
        ExternWrap_FORMULA__FOLDINGCTETRA3D, &map__FOLDINGCTETRA3D,
        3, {2, 2, 2},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA__FOLDINGICOSA, "_FoldingIcosa", -1, 12,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA__FOLDINGICOSA, &map__FOLDINGICOSA,
        1, {6},
        0x0000,
        3, consts__FOLDINGICOSA
    });

    RegisterExternalFormula({
        FORMULA__FOLDINGINFY, "_FoldingInfy", -1, 4,
        10.0, 1.0, 1.0, 100.0,
        ExternWrap_FORMULA__FOLDINGINFY, &map__FOLDINGINFY,
        4, {0, 0, 0, 0},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA__FOLDINGOCT, "_FoldingOct", -1, 0,
        1.0, 1.0, 2.0, 100.0,
        ExternWrap_FORMULA__FOLDINGOCT, &map__FOLDINGOCT,
        0, {},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA__FOLDINGTETRA3D, "_FoldingTetra3d", -1, 2,
        1.0, 1.0, 2.0, 100.0,
        ExternWrap_FORMULA__FOLDINGTETRA3D, &map__FOLDINGTETRA3D,
        2, {2, 2},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA__FOLDINGTETRAKALEID, "_FoldingTetraKaleid", -1, 4,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA__FOLDINGTETRAKALEID, &map__FOLDINGTETRAKALEID,
        3, {0, 2, 2},
        0x0000,
        1, consts__FOLDINGTETRAKALEID
    });

    RegisterExternalFormula({
        FORMULA__FOLDINGVAR, "_FoldingVar", -1, 12,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA__FOLDINGVAR, &map__FOLDINGVAR,
        12, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA__GENCWARP, "_GenCWarp", -1, 17,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA__GENCWARP, &map__GENCWARP,
        8, {6, 0, 0, 2, 2, 2, 0, 0},
        0x0000,
        1, consts__GENCWARP
    });

    RegisterExternalFormula({
        FORMULA__HEXATRANSFORM, "_HexaTransform", -1, 3,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA__HEXATRANSFORM, &map__HEXATRANSFORM,
        1, {0},
        0x0000,
        2, consts__HEXATRANSFORM
    });

    RegisterExternalFormula({
        FORMULA__HOPSQRTX, "_HopSqrtX", -1, 4,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA__HOPSQRTX, &map__HOPSQRTX,
        4, {0, 0, 0, 0},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA__HOPSQRTY, "_HopSqrtY", -1, 4,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA__HOPSQRTY, &map__HOPSQRTY,
        4, {0, 0, 0, 0},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA__HOPSQRTZ, "_HopSqrtZ", -1, 4,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA__HOPSQRTZ, &map__HOPSQRTZ,
        4, {0, 0, 0, 0},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA__INTPOWROTATEC, "_IntPowRotateC", -1, 13,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA__INTPOWROTATEC, &map__INTPOWROTATEC,
        10, {2, 2, 2, 2, 2, 2, 2, 2, 2, 0},
        0x0000,
        3, consts__INTPOWROTATEC
    });

    RegisterExternalFormula({
        FORMULA__JULIASETS, "_JuliaSets", -1, 2,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA__JULIASETS, &map__JULIASETS,
        2, {0, 0},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA__LINCOMBINECXYZ, "_LinCombineCxyz", -1, 9,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA__LINCOMBINECXYZ, &map__LINCOMBINECXYZ,
        9, {0, 0, 0, 0, 0, 0, 0, 0, 0},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA__LINCOMBINEXYZ, "_LinCombineXYZ", -1, 9,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA__LINCOMBINEXYZ, &map__LINCOMBINEXYZ,
        9, {0, 0, 0, 0, 0, 0, 0, 0, 0},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA__LOGX, "_LogX", -1, 4,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA__LOGX, &map__LOGX,
        4, {0, 0, 0, 0},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA__LOGY, "_LogY", -1, 4,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA__LOGY, &map__LOGY,
        4, {0, 0, 0, 0},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA__LOGZ, "_LogZ", -1, 4,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA__LOGZ, &map__LOGZ,
        4, {0, 0, 0, 0},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA__MAPTRANSLATE, "_MapTranslate", -1, 7,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA__MAPTRANSLATE, &map__MAPTRANSLATE,
        7, {2, 1, 1, 1, 1, 1, 1},
        0x0001,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA__MAXCLIPPING, "_MaxClipping", -1, 3,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA__MAXCLIPPING, &map__MAXCLIPPING,
        3, {0, 0, 0},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA__NEOSQRX, "_NeoSqrX", -1, 4,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA__NEOSQRX, &map__NEOSQRX,
        4, {0, 0, 0, 0},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA__NEOSQRY, "_NeoSqrY", -1, 4,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA__NEOSQRY, &map__NEOSQRY,
        4, {0, 0, 0, 0},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA__NEOSQRZ, "_NeoSqrZ", -1, 4,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA__NEOSQRZ, &map__NEOSQRZ,
        4, {0, 0, 0, 0},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA__PARTLYJULIA, "_PartlyJulia", -1, 9,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA__PARTLYJULIA, &map__PARTLYJULIA,
        9, {0, 0, 0, 0, 0, 0, 0, 0, 0},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA__PARTLYJULIAR, "_PartlyJuliaR", -1, 11,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA__PARTLYJULIAR, &map__PARTLYJULIAR,
        11, {0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 2},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA__PARTLYJULIAROFF, "_PartlyJuliaRoff", -1, 5,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA__PARTLYJULIAROFF, &map__PARTLYJULIAROFF,
        4, {0, 0, 0, 0},
        0x0000,
        1, consts__PARTLYJULIAROFF
    });

    RegisterExternalFormula({
        FORMULA__PLUSSINAPP, "_PlusSinApp", -1, 9,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA__PLUSSINAPP, &map__PLUSSINAPP,
        6, {2, 2, 1, 1, 1, 1},
        0x0000,
        3, consts__PLUSSINAPP
    });

    RegisterExternalFormula({
        FORMULA__POLYFOLDSYM, "_PolyFold-sym", -1, 7,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA__POLYFOLDSYM, &map__POLYFOLDSYM,
        5, {0, 0, 0, 0, 0},
        0x0000,
        2, consts__POLYFOLDSYM
    });

    RegisterExternalFormula({
        FORMULA__POLYFOLDING, "_PolyFolding", -1, 7,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA__POLYFOLDING, &map__POLYFOLDING,
        5, {0, 0, 0, 0, 0},
        0x0000,
        2, consts__POLYFOLDING
    });

    RegisterExternalFormula({
        FORMULA__QUADRAYTRANSFORM, "_QuadrayTransform", -1, 0,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA__QUADRAYTRANSFORM, &map__QUADRAYTRANSFORM,
        0, {},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA__QUADRAYTRANSFORM2, "_QuadrayTransform2", -1, 2,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA__QUADRAYTRANSFORM2, &map__QUADRAYTRANSFORM2,
        2, {0, 0},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA__ROTATE4D, "_Rotate4d", -1, 16,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA__ROTATE4D, &map__ROTATE4D,
        1, {12},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA__ROTATEBACK, "_RotateBack", -1, 9,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA__ROTATEBACK, &map__ROTATEBACK,
        1, {6},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA__ROTATEC, "_RotateC", -1, 9,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA__ROTATEC, &map__ROTATEC,
        1, {6},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA__ROTATEPLANEONITS, "_RotatePlaneOnIts", -1, 11,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA__ROTATEPLANEONITS, &map__ROTATEPLANEONITS,
        2, {6, 0},
        0x0000,
        1, consts__ROTATEPLANEONITS
    });

    RegisterExternalFormula({
        FORMULA__ROTATERND, "_RotateRnd", -1, 9,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA__ROTATERND, &map__ROTATERND,
        6, {0, 0, 0, 2, 2, 2},
        0x0000,
        3, consts__ROTATERND
    });

    RegisterExternalFormula({
        FORMULA__ROTATEXYONITS, "_RotateXYonIts", -1, 11,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA__ROTATEXYONITS, &map__ROTATEXYONITS,
        2, {6, 0},
        0x0000,
        1, consts__ROTATEXYONITS
    });

    RegisterExternalFormula({
        FORMULA__ROTATEXYONITSP, "_RotateXYonItsP", -1, 13,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA__ROTATEXYONITSP, &map__ROTATEXYONITSP,
        4, {6, 0, 0, 0},
        0x0000,
        1, consts__ROTATEXYONITSP
    });

    RegisterExternalFormula({
        FORMULA__ROTATEDABS, "_RotatedAbs", -1, 11,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA__ROTATEDABS, &map__ROTATEDABS,
        3, {6, 0, 0},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA__ROTATEDFOLDING, "_RotatedFolding", -1, 10,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA__ROTATEDFOLDING, &map__ROTATEDFOLDING,
        2, {6, 0},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA__RPOW3, "_Rpow3", 0, 0,
        1.0, 1.0, 3.0, 16.0,
        ExternWrap_FORMULA__RPOW3, &map__RPOW3,
        0, {},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA__SINAPPROX, "_SinApprox", -1, 8,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA__SINAPPROX, &map__SINAPPROX,
        5, {2, 1, 1, 1, 1},
        0x0000,
        3, consts__SINAPPROX
    });

    RegisterExternalFormula({
        FORMULA__SINY, "_SinY", -1, 5,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA__SINY, &map__SINY,
        5, {2, 1, 1, 1, 1},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA__SINEPOW2, "_SinePow2", 0, 2,
        1.0, 1.0, 2.0, 16.0,
        ExternWrap_FORMULA__SINEPOW2, &map__SINEPOW2,
        2, {0, 0},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA__SINHX, "_SinhX", -1, 4,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA__SINHX, &map__SINHX,
        4, {0, 0, 0, 0},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA__SINHY, "_SinhY", -1, 4,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA__SINHY, &map__SINHY,
        4, {0, 0, 0, 0},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA__SINHZ, "_SinhZ", -1, 4,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA__SINHZ, &map__SINHZ,
        4, {0, 0, 0, 0},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA__SPHEREFOLDING1, "_SphereFolding1", -1, 4,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA__SPHEREFOLDING1, &map__SPHEREFOLDING1,
        1, {8},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA__SPHEREFOLDING2, "_SphereFolding2", -1, 4,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA__SPHEREFOLDING2, &map__SPHEREFOLDING2,
        1, {8},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA__SPHEREINVC, "_SphereInvC", 0, 0,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA__SPHEREINVC, &map__SPHEREINVC,
        0, {},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA__SQUARER, "_SquareR", 0, 0,
        1.0, 1.0, 2.0, 16.0,
        ExternWrap_FORMULA__SQUARER, &map__SQUARER,
        0, {},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA__TRANSFORM, "_Transform", -1, 14,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA__TRANSFORM, &map__TRANSFORM,
        6, {0, 0, 0, 0, 5, 2},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA__TRANSLATE, "_Translate", -1, 3,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA__TRANSLATE, &map__TRANSLATE,
        3, {0, 0, 0},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA__TRANSLATEC4D, "_TranslateC4d", -1, 8,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA__TRANSLATEC4D, &map__TRANSLATEC4D,
        8, {0, 0, 0, 0, 0, 0, 0, 0},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA__TRANSLATERND, "_TranslateRnd", -1, 8,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA__TRANSLATERND, &map__TRANSLATERND,
        6, {0, 0, 0, 2, 2, 2},
        0x0000,
        2, consts__TRANSLATERND
    });

    RegisterExternalFormula({
        FORMULA__YPLUSSINZ, "_YplusSinZ", -1, 0,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA__YPLUSSINZ, &map__YPLUSSINZ,
        0, {},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA__YPLUSSINZ2, "_YplusSinZ2", -1, 4,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA__YPLUSSINZ2, &map__YPLUSSINZ2,
        2, {0, 0},
        0x0000,
        2, consts__YPLUSSINZ2
    });

    RegisterExternalFormula({
        FORMULA__ADDC, "_addC", -1, 3,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA__ADDC, &map__ADDC,
        3, {0, 0, 0},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA__ASDAMTRANS, "_asdamTrans", -1, 10,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA__ASDAMTRANS, &map__ASDAMTRANS,
        2, {6, 0},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA__BOXTILING, "_boxtiling", -1, 4,
        10.0, 1.0, 1.0, 100.0,
        ExternWrap_FORMULA__BOXTILING, &map__BOXTILING,
        4, {0, 0, 0, 0},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA__CONJ, "_conj", -1, 0,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA__CONJ, &map__CONJ,
        0, {},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA__CONJ2D, "_conj2D", -1, 0,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA__CONJ2D, &map__CONJ2D,
        0, {},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA__COSMARTIN, "_cosmartin", -1, 5,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA__COSMARTIN, &map__COSMARTIN,
        5, {0, 0, 0, 0, 0},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA_CYLRIPPLE, "_cylripple", 0, 15,
        0.1, 1.0, 2.0, 100.0,
        ExternWrap_FORMULA_CYLRIPPLE, &map_CYLRIPPLE,
        7, {6, 0, 0, 0, 0, 2, 2},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA__DYNAMIC, "_dynamic", -1, 5,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA__DYNAMIC, &map__DYNAMIC,
        5, {0, 0, 0, 0, 0},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA__DYNAMICFAST, "_dynamicfast", -1, 11,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA__DYNAMICFAST, &map__DYNAMICFAST,
        5, {0, 0, 0, 0, 0},
        0x0000,
        6, consts__DYNAMICFAST
    });

    RegisterExternalFormula({
        FORMULA__FLIPCXYZ, "_flipC-xyz", -1, 0,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA__FLIPCXYZ, &map__FLIPCXYZ,
        0, {},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA__GNARL2D, "_gnarl2D", -1, 5,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA__GNARL2D, &map__GNARL2D,
        5, {0, 0, 0, 0, 0},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA__GNARL2D_2, "_gnarl2D_2", -1, 6,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA__GNARL2D_2, &map__GNARL2D_2,
        6, {0, 0, 0, 0, 0, 0},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA__GNARL2D_2FAST, "_gnarl2D_2fast", -1, 12,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA__GNARL2D_2FAST, &map__GNARL2D_2FAST,
        6, {0, 0, 0, 0, 0, 0},
        0x0000,
        6, consts__GNARL2D_2FAST
    });

    RegisterExternalFormula({
        FORMULA__GNARL2DFAST, "_gnarl2Dfast", -1, 10,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA__GNARL2DFAST, &map__GNARL2DFAST,
        5, {0, 0, 0, 0, 0},
        0x0000,
        5, consts__GNARL2DFAST
    });

    RegisterExternalFormula({
        FORMULA__GNARL3D, "_gnarl3D", -1, 6,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA__GNARL3D, &map__GNARL3D,
        6, {0, 0, 0, 0, 0, 0},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA__GNARL3DFAST, "_gnarl3Dfast", -1, 11,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA__GNARL3DFAST, &map__GNARL3DFAST,
        6, {0, 0, 0, 0, 0, 0},
        0x0000,
        5, consts__GNARL3DFAST
    });

    RegisterExternalFormula({
        FORMULA__HELISPIRAL, "_helispiral", -1, 15,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA__HELISPIRAL, &map__HELISPIRAL,
        6, {6, 0, 0, 2, 2, 0},
        0x0000,
        1, consts__HELISPIRAL
    });

    RegisterExternalFormula({
        FORMULA__HOPALM1, "_hopalm1", -1, 6,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA__HOPALM1, &map__HOPALM1,
        6, {0, 0, 0, 0, 0, 0},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA__HOPALM2, "_hopalm2", -1, 6,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA__HOPALM2, &map__HOPALM2,
        6, {0, 0, 0, 0, 0, 0},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA__HOPALONG, "_hopalong", -1, 6,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA__HOPALONG, &map__HOPALONG,
        6, {0, 0, 0, 0, 0, 0},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA__HYC3DINV, "_hyc3Dinv", -1, 4,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA__HYC3DINV, &map__HYC3DINV,
        4, {0, 0, 0, 0},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA__INVBISP, "_invbisp", -1, 2,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA__INVBISP, &map__INVBISP,
        1, {0},
        0x0000,
        1, consts__INVBISP
    });

    RegisterExternalFormula({
        FORMULA__INVCYLINDRICAL, "_invcylindrical", -1, 0,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA__INVCYLINDRICAL, &map__INVCYLINDRICAL,
        0, {},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA__INVSPHERICAL, "_invspherical", -1, 0,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA__INVSPHERICAL, &map__INVSPHERICAL,
        0, {},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA__INVTORICAL, "_invtorical", -1, 1,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA__INVTORICAL, &map__INVTORICAL,
        1, {0},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA__INVTORICAL2, "_invtorical2", -1, 2,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA__INVTORICAL2, &map__INVTORICAL2,
        2, {0, 0},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA__JULIAX2, "_juliax2", -1, 7,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA__JULIAX2, &map__JULIAX2,
        7, {0, 0, 0, 0, 0, 0, 0},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA__KAMTOR, "_kamtor", -1, 14,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA__KAMTOR, &map__KAMTOR,
        14, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA__KAMTORCUT, "_kamtorcut", -1, 16,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA__KAMTORCUT, &map__KAMTORCUT,
        14, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        0x0000,
        2, consts__KAMTORCUT
    });

    RegisterExternalFormula({
        FORMULA__LORENZ, "_lorenz", -1, 5,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA__LORENZ, &map__LORENZ,
        5, {0, 0, 0, 0, 0},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA__NGON, "_ngon", -1, 8,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA__NGON, &map__NGON,
        6, {0, 0, 0, 0, 0, 2},
        0x0000,
        2, consts__NGON
    });

    RegisterExternalFormula({
        FORMULA__NGON2, "_ngon2", -1, 7,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA__NGON2, &map__NGON2,
        5, {0, 0, 0, 0, 0},
        0x0000,
        2, consts__NGON2
    });

    RegisterExternalFormula({
        FORMULA__PLANEFOLD, "_planeFold", -1, 3,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA__PLANEFOLD, &map__PLANEFOLD,
        3, {0, 0, 0},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA__POINCARE, "_poincare", -1, 5,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA__POINCARE, &map__POINCARE,
        5, {0, 0, 0, 0, 0},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA__RABINOVICH_F, "_rabinovich-f", -1, 5,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA__RABINOVICH_F, &map__RABINOVICH_F,
        5, {0, 0, 0, 0, 0},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA__RECFOLD, "_recFold", -1, 7,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA__RECFOLD, &map__RECFOLD,
        5, {0, 0, 0, 0, 2},
        0x0000,
        2, consts__RECFOLD
    });

    RegisterExternalFormula({
        FORMULA__RECIPROCALX, "_reciprocalX", -1, 0,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA__RECIPROCALX, &map__RECIPROCALX,
        0, {},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA__RECIPROCALX2, "_reciprocalX2", -1, 1,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA__RECIPROCALX2, &map__RECIPROCALX2,
        1, {0},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA__RECIPROCALX3, "_reciprocalX3", -1, 2,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA__RECIPROCALX3, &map__RECIPROCALX3,
        1, {22},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA__RECIPROCALX3B, "_reciprocalX3b", -1, 6,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA__RECIPROCALX3B, &map__RECIPROCALX3B,
        4, {22, 22, 0, 0},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA__RECIPROCALY2, "_reciprocalY2", -1, 1,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA__RECIPROCALY2, &map__RECIPROCALY2,
        1, {0},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA__RECIPROCALY3, "_reciprocalY3", -1, 2,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA__RECIPROCALY3, &map__RECIPROCALY3,
        1, {22},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA__RECIPROCALY3B, "_reciprocalY3b", -1, 6,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA__RECIPROCALY3B, &map__RECIPROCALY3B,
        4, {22, 22, 0, 0},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA__RECIPROCALZ2, "_reciprocalZ2", -1, 1,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA__RECIPROCALZ2, &map__RECIPROCALZ2,
        1, {0},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA__RECIPROCALZ3, "_reciprocalZ3", -1, 2,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA__RECIPROCALZ3, &map__RECIPROCALZ3,
        1, {22},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA__RECIPROCALZ3B, "_reciprocalZ3b", -1, 6,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA__RECIPROCALZ3B, &map__RECIPROCALZ3B,
        4, {22, 22, 0, 0},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA__ROSSLER, "_rossler", -1, 5,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA__ROSSLER, &map__ROSSLER,
        5, {0, 0, 0, 0, 0},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA__SCALEC, "_scaleC", -1, 3,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA__SCALEC, &map__SCALEC,
        3, {0, 0, 0},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA__SKEW, "_skew", -1, 2,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA__SKEW, &map__SKEW,
        2, {0, 0},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA__SPHEREXINV, "_sphereXinv", -1, 4,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA__SPHEREXINV, &map__SPHEREXINV,
        4, {0, 0, 0, 0},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA__SPHEREYINV, "_sphereYinv", -1, 4,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA__SPHEREYINV, &map__SPHEREYINV,
        4, {0, 0, 0, 0},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA__SPHEREZINV, "_sphereZinv", -1, 4,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA__SPHEREZINV, &map__SPHEREZINV,
        4, {0, 0, 0, 0},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA__SQR_3D, "_sqr_3D", -1, 4,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA__SQR_3D, &map__SQR_3D,
        4, {0, 0, 0, 0},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA__SQR_4D, "_sqr_4D", -1, 5,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA__SQR_4D, &map__SQR_4D,
        5, {0, 0, 0, 0, 0},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA__TOCYLINDRICAL, "_tocylindrical", -1, 0,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA__TOCYLINDRICAL, &map__TOCYLINDRICAL,
        0, {},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA__TORUSPINV1, "_toruspinv1", -1, 5,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA__TORUSPINV1, &map__TORUSPINV1,
        5, {0, 0, 0, 0, 0},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA__TORUSPINV2, "_toruspinv2", -1, 5,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA__TORUSPINV2, &map__TORUSPINV2,
        5, {0, 0, 0, 0, 0},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA__TORUSPINV3, "_toruspinv3", -1, 5,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA__TORUSPINV3, &map__TORUSPINV3,
        5, {0, 0, 0, 0, 0},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA__TOSPHERICAL, "_tospherical", -1, 0,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA__TOSPHERICAL, &map__TOSPHERICAL,
        0, {},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA__TOTORICAL, "_totorical", -1, 1,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA__TOTORICAL, &map__TOTORICAL,
        1, {0},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA__TOTORICAL2, "_totorical2", -1, 3,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA__TOTORICAL2, &map__TOTORICAL2,
        2, {0, 0},
        0x0000,
        1, consts__TOTORICAL2
    });

    RegisterExternalFormula({
        FORMULA__UPDATEC, "_updateC", -1, 2,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA__UPDATEC, &map__UPDATEC,
        2, {0, 0},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA__UPDATEC2, "_updateC2", -1, 13,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA__UPDATEC2, &map__UPDATEC2,
        5, {6, 0, 0, 0, 0},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA__UPDATEC_4D, "_updateC_4d", -1, 0,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA__UPDATEC_4D, &map__UPDATEC_4D,
        0, {},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA__VANDERPOL, "_vanderpol", -1, 8,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA__VANDERPOL, &map__VANDERPOL,
        8, {0, 0, 0, 0, 0, 0, 0, 0},
        0x0000,
        0, nullptr
    });

    RegisterExternalFormula({
        FORMULA__VANDERPOL2, "_vanderpol2", -1, 9,
        1.0, 1.0, 0.0, 100.0,
        ExternWrap_FORMULA__VANDERPOL2, &map__VANDERPOL2,
        9, {0, 0, 0, 0, 0, 0, 0, 0, 0},
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
