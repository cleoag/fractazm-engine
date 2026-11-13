#pragma once

#include "core/formulas.h"
#include "core/formula_complex_power.h"
#include "core/formula_work_guard.h"
#include "core/types.h"
#include "core/parity_trace.h"
#include "core/extended80.h"
#include "core/extended_transcendentals.h"
#include "core/extended_sine.h"
#include <cstdint>
#include <algorithm>
#include <cmath>
#include <cstring>
#include <limits>
#include <sstream>
#include <iomanip>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#ifndef M_LOG2E
#define M_LOG2E 1.44269504088896340736
#endif

namespace mb3d {

// ============================================================================
// Compatibility types for external formula .inc files
// ============================================================================

struct cpu_double3 {
    double x, y, z;
    cpu_double3() : x(0.0), y(0.0), z(0.0) {}
    cpu_double3(double v) : x(v), y(v), z(v) {}  // broadcast constructor
    cpu_double3(double x_, double y_, double z_) : x(x_), y(y_), z(z_) {}
    double& operator[](int i) { return (&x)[i]; }
    const double& operator[](int i) const { return (&x)[i]; }
};

// clamp helpers (used by .inc formula code)
inline double clamp(double v, double lo, double hi) {
    return std::max(lo, std::min(v, hi));
}

inline float clamp(float v, float lo, float hi) {
    return std::max(lo, std::min(v, hi));
}

// x87-compatible nearest-even helpers used by dIFS formulas that were ported
// from frndint/fistp code paths. The default host rounding mode is expected
// to remain FE_TONEAREST; we intentionally model Pascal/x87 semantics here
// instead of std::round()'s half-away-from-zero behavior.
inline double x87_frndint(double v) {
    return std::nearbyint(v);
}

// Shared original polynomial, range reduction and affine operations from
// _SinApprox/_PlusSinApp. Keep extended intermediates through the final store:
// at input 1e12 premature double reduction changes the result by about 2e-5.
#if defined(_MSC_VER) && defined(_M_X64)
extern "C" void SineApproxTransform_x87(double* state, int add);
extern "C" void SetWTransform_x87(double* state, int powerMode);
extern "C" uint32_t E3DtessOriginal_x87(double*, const double*, int axiom,
                                        int maxDepth, FormulaWorkCounter*);
extern "C" uint32_t IFSTessOriginal_x87(double*, const double*, FormulaWorkCounter*);
extern "C" uint32_t Knots2IFSOriginal_x87(double*, const void*, FormulaWorkCounter*);
#endif
inline double external_sine_transform(double coordinate, const double* options,
                                      const double* constants, double accumulator, bool add) {
#if defined(_MSC_VER) && defined(_M_X64)
    double state[] = {coordinate, options[0], options[1], options[2], options[3],
                      constants[0], constants[1], constants[2], accumulator};
    SineApproxTransform_x87(state, add ? 1 : 0);
    return state[0];
#else
    const numeric::X87Scope extendedScope;
    numeric::X87Real t = (static_cast<numeric::X87Real>(coordinate) - options[0]) * options[1];
    t *= constants[0];
    numeric::X87Real v = 2 * (t - numeric::x87Round(t - numeric::X87Real(0.5)));
    bool negative = false;
    if (v > 1) { v = 2 - v; negative = true; }
    const numeric::X87Real pi = numeric::x87Pi();
    v *= pi;
    if (v >= constants[1]) { v -= pi; negative = !negative; }
    numeric::X87Real result = v - ((v * v) * v) * constants[2];
    if (negative) result = -result;
    result = result * options[2] + options[3];
    if (add) result += accumulator;
    return static_cast<double>(result);
#endif
}

#ifdef MB3D_SOFTWARE_EXT80
inline void external_pine_t1(double& x, double& y, double& z,
                             double s23, double s13, double s12, double scale, double offset) {
    const numeric::ExtendedScope scope;
    using E = numeric::Extended80;
    const double tx = double((E(x)*E(s23)-E(z)*E(s13))*E(s12));
    const double nz = double(abs(E(x)*E(s13)+E(z)*E(s23)));
    const double nx = double(abs(E(tx)-E(y)*E(s12)));
    const double ny = double(abs(E(tx)+E(y)*E(s12)));
    const double tx2 = double((E(nx)+E(ny))*E(s12));
    y = double((E(ny)-E(nx))*E(s12));
    x = double(E(tx2)*E(s23)+E(nz)*E(s13));
    z = double(-E(tx2)*E(s13)+E(nz)*E(s23));
    x = double(E(x)*E(scale)-E(offset));
    y = double(E(y)*E(scale)); z = double(E(z)*E(scale));
}
#endif

inline int x87_fistp_int32(double v) {
    const double rounded = std::nearbyint(v);
    // Masked x87 FIST writes the integer-indefinite bit pattern for NaN,
    // infinity and rounded values outside the signed destination range.
    // CRT lrint overflow and the width of long differ across platforms.
    if (!(rounded >= -2147483648.0 && rounded < 2147483648.0))
        return std::numeric_limits<int32_t>::min();
    return static_cast<int32_t>(rounded);
}

// x87 FPREM-style remainder using extended-precision quotient truncation.
// This models the formulas that were originally written against the x87
// remainder path more closely than raw double-precision fmod().
inline double x87_fprem(double numer, double denom) {
    if (!std::isfinite(numer) || !std::isfinite(denom) || denom == 0.0) {
        return std::numeric_limits<double>::quiet_NaN();
    }
    long double xn = static_cast<long double>(numer);
    long double xd = static_cast<long double>(denom);
    long double q = std::truncl(xn / xd);
    long double r = xn - q * xd;
    return static_cast<double>(r);
}

// Power sequence used by ABoxVS: FYL2X, multiply exponent, then
// FRNDINT/F2XM1/FSCALE. Unlike libm pow, zero/negative bases do not
// acquire special integer-exponent or 0^0 results. Infinite logarithms
// become NaN when the sequence subtracts the rounded exponent.
inline double x87_log_power(double base, double exponent) {
    if (!(base > 0.0) || !std::isfinite(base) || !std::isfinite(exponent))
        return std::numeric_limits<double>::quiet_NaN();
    return std::pow(base, exponent);
}

// x87 exp() implemented via fldl2e/fprem/f2xm1/fscale-style decomposition.
// This intentionally mirrors the old formula blobs more closely than std::exp()
// when tiny differences compound across many orbit iterations.
inline double x87_exp(double v) {
    if (std::isnan(v)) return std::numeric_limits<double>::quiet_NaN();
    if (v == std::numeric_limits<double>::infinity()) return v;
    if (v == -std::numeric_limits<double>::infinity()) return 0.0;

    long double t = static_cast<long double>(v) * static_cast<long double>(M_LOG2E);
    long double int_part = std::truncl(t);
    // FSCALE overflows/underflows here; converting this exponent to a C++ int
    // first would be undefined. Test before the cast, including t == infinity
    // on hosts where long double has binary64 range.
    if (int_part > std::numeric_limits<int>::max())
        return std::numeric_limits<double>::infinity();
    if (int_part < std::numeric_limits<int>::min()) return 0.0;
    long double frac_part = t - int_part;
    long double mantissa = std::exp2l(frac_part);
    long double result = std::ldexpl(mantissa, static_cast<int>(int_part));
    return static_cast<double>(result);
}

// ============================================================================
// Rotation matrix types
// ============================================================================

struct RotMatrix3 {
    double m[3][3];

    cpu_double3 operator*(const cpu_double3& v) const {
        return cpu_double3(
            m[0][0] * v.x + m[0][1] * v.y + m[0][2] * v.z,
            m[1][0] * v.x + m[1][1] * v.y + m[1][2] * v.z,
            m[2][0] * v.x + m[2][1] * v.y + m[2][2] * v.z
        );
    }
};

RotMatrix3 buildRotMatrix3(double ax_deg, double ay_deg, double az_deg);

// Note: Pascal TSMatrix4 uses Single (float), so this uses float to match.
// See also header_trafos.cpp BuildSMatrix4 which operates on TSMatrix4 (float).
struct RotMatrix4 {
    float m[4][4];
};

// Build RotMatrix3 from 9 pre-computed float matrix elements (row-major).
// Used when PVar stores a pre-computed rotation matrix from .3SingleAngles.
// Pascal FillCustomVBufWithVars calls BuildRotMatrix and stores 9 Singles in PVar.
inline RotMatrix3 rotMatrix3FromFloats(const double* params, int startIdx) {
    RotMatrix3 r{};
    r.m[0][0] = params[startIdx + 0];
    r.m[0][1] = params[startIdx + 1];
    r.m[0][2] = params[startIdx + 2];
    r.m[1][0] = params[startIdx + 3];
    r.m[1][1] = params[startIdx + 4];
    r.m[1][2] = params[startIdx + 5];
    r.m[2][0] = params[startIdx + 6];
    r.m[2][1] = params[startIdx + 7];
    r.m[2][2] = params[startIdx + 8];
    return r;
}

// HeightMap-family dIFS formulas consume the packed 3x3 values in an ASM-specific
// triple order: last triple, first triple, middle triple.
inline RotMatrix3 heightMapRotMatrix3FromFloats(const double* params, int startIdx) {
    RotMatrix3 r{};
    r.m[0][0] = params[startIdx + 6];
    r.m[0][1] = params[startIdx + 7];
    r.m[0][2] = params[startIdx + 8];
    r.m[1][0] = params[startIdx + 0];
    r.m[1][1] = params[startIdx + 1];
    r.m[1][2] = params[startIdx + 2];
    r.m[2][0] = params[startIdx + 3];
    r.m[2][1] = params[startIdx + 4];
    r.m[2][2] = params[startIdx + 5];
    return r;
}

// Build RotMatrix4 from 16 pre-computed float matrix elements (row-major).
// Used when PVar stores a pre-computed 4D rotation matrix from .6SingleAngles.
inline RotMatrix4 rotMatrix4FromFloats(const double* params, int startIdx) {
    RotMatrix4 r{};
    for (int i = 0; i < 4; ++i)
        for (int j = 0; j < 4; ++j)
            r.m[i][j] = static_cast<float>(params[startIdx + i * 4 + j]);
    return r;
}

RotMatrix4 buildRotMatrix4(double ax_deg, double ay_deg, double az_deg,
                           double aw_deg, double au_deg, double av_deg);

// ============================================================================
// External formula parameter mapping
// ============================================================================

constexpr int MAX_FORMULA_PARAMS = 64;
constexpr int MAX_FORMULA_CONSTS = 32;
constexpr int FORMULA_ORBIT_W_SLOT = MAX_FORMULA_PARAMS - 17; // trait-gated input
constexpr int FORMULA_STATE_X_SLOT = MAX_FORMULA_PARAMS - 16;  // slot 48
constexpr int FORMULA_STATE_Y_SLOT = MAX_FORMULA_PARAMS - 15;  // slot 49
constexpr int FORMULA_STATE_Z_SLOT = MAX_FORMULA_PARAMS - 14;  // slot 50
constexpr int FORMULA_STATE_W_SLOT = MAX_FORMULA_PARAMS - 13;  // slot 51
constexpr int WRITE_J1_SLOT = MAX_FORMULA_PARAMS - 12;  // slot 52
constexpr int WRITE_J2_SLOT = MAX_FORMULA_PARAMS - 11;  // slot 53
constexpr int WRITE_J3_SLOT = MAX_FORMULA_PARAMS - 10;  // slot 54
constexpr int WRITE_J4_SLOT = MAX_FORMULA_PARAMS - 9;   // slot 55
constexpr int FORMULA_ROUT_SLOT = MAX_FORMULA_PARAMS - 8; // slot 56; trait-gated input

// Well-known params slot for ItResultI (current iteration count).
// Used by formulas like BT1Pinegon/BT1Pinehedron that need per-iteration branching.
constexpr int ITER_COUNT_SLOT = MAX_FORMULA_PARAMS - 5;  // slot 59

// Well-known params slot for dIFS VaryScale tracking.
// dIFS formulas read/write VaryScale via params[DIFS_VARYSCALE_SLOT].
// The wrapper copies ext->VaryScale here before the call and reads it back after.
constexpr int DIFS_VARYSCALE_SLOT = MAX_FORMULA_PARAMS - 4;  // slot 60

// Well-known params slot for dIFS Rold (absolute DEstop = msDEstop * sStepWm103).
// In Pascal, dIFS formulas access this via [esi-0x88] which maps to TIteration3Dext.Rold.
// The wrapper copies ext->Rold here before the call so formulas can read it.
constexpr int DIFS_ROLD_SLOT = MAX_FORMULA_PARAMS - 6;  // slot 58

// Well-known params slot for bFirstIt tracking.
// Pascal ASM formulas check bFirstIt to initialize VaryScale on first call.
// The wrapper copies ext->bFirstIt here before the call and reads it back after.
constexpr int BFIRSTIT_SLOT = MAX_FORMULA_PARAMS - 7;  // slot 57
// Well-known params slot for dIFS STYLE=1 xyz-state commit.
// Pascal dIFS formulas decide independently whether transformed xyz are written back
// to live state; this is not equivalent to "returned scratch coords" in C++.
// Formulas set this to 1 when transformed xyz must become the new live position.
constexpr int DIFS_COMMIT_XYZ_SLOT = MAX_FORMULA_PARAMS - 3;  // slot 61

struct FormulaParamMap {
    int count;
    struct ParamEntry {
        int16_t byteOffset;   // negative offset from PVar (e.g., -0x18)
        uint8_t valueType;    // 0=double(8B), 1=float(4B), 2=int32(4B), 3=int64(8B)
    } entries[MAX_FORMULA_PARAMS];
};

struct FormulaConstEntry {
    uint8_t valueType;        // 0=double, 1=float, 2=int32, 3=int64
    uint64_t rawBits;         // raw storage copied into the +N*8 PVar slot
};

// ============================================================================
// External formula info and slot cache
// ============================================================================

struct ExternalFormulaInfo {
    unsigned int id;
    const char* name;
    int DEoption;
    int paramCount;
    double DEscale;
    double ADEscale;
    double SIpow;
    double RStop;
    ThybridIteration wrapperFunc;
    const FormulaParamMap* paramMap;
    // Correct option types from .m3f [OPTIONS] section.
    // Pascal uses TCustomFormula.byOptionTypes from the .m3f, NOT THAformula.byOptionType
    // from the .m3p file. The .m3p byOptionType may be wrong for multi-value options
    // (e.g., .3SingleAngles stores only one type entry but consumes 3 value slots).
    int optionTypeCount;                // number of entries (not values)
    uint8_t optionTypes[16];            // correct byOptionType array
    // Legacy Pascal-compatible prefetch hint: integer option labels containing
    // MAP, including channel/Log2MapPower labels. Not mandatory callback assets.
    uint16_t mapValueMask;
    int constantCount;                  // number of +N*8 constant slots from .m3f [CONSTANTS]
    const FormulaConstEntry* constants; // raw slot payloads copied over PAligned16
};

struct FormulaSlotCache {
    double params[MAX_FORMULA_PARAMS];
    int paramCount;
    const FormulaParamMap* paramMap;
    bool valid;
};

template<unsigned int ID>
struct ExternalFormulaMultipliesAnalyticDE {
    static constexpr bool value = false;
};

template<unsigned int ID>
struct ExternalFormulaUsesXYZState {
    static constexpr bool value = false;
};

template<unsigned int ID>
struct ExternalFormulaUsesKaliState { static constexpr bool value = false; };

template<unsigned int ID>
struct ExternalFormulaUsesSse01 { static constexpr bool value = false; };

template<unsigned int ID>
struct ExternalFormulaUsesXYZWState {
    static constexpr bool value = false;
};

template<unsigned int ID>
struct ExternalFormulaUsesWrittenCForSameIterationAdd {
    static constexpr bool value = false;
};

// Explicitly declared writes must propagate NaNs too; NaN is otherwise the legacy no-write sentinel.
template<unsigned int ID>
struct ExternalFormulaAlwaysWritesJuliaXYZ { static constexpr bool value = false; };
template<unsigned int ID>
struct ExternalFormulaAlwaysWritesJuliaW { static constexpr bool value = false; };

template<unsigned int ID>
struct ExternalFormulaReadsRout { static constexpr bool value = false; };

template<unsigned int ID>
struct ExternalFormulaWritesWState {
    static constexpr bool value = false;
};

template<unsigned int ID>
struct ExternalFormulaWritesPersistentCLanes {
    static constexpr bool value = false;
};

template<unsigned int ID>
struct ExternalFormulaConsumesPersistentCLanes {
    static constexpr bool value = false;
};

// ============================================================================
// Registry API
// ============================================================================

// Thread-safe initialization; both Find functions also initialize on first use.
// Returned metadata pointers remain valid for the lifetime of the process.
void InitExternalFormulaRegistry();
// Setup-only hooks used by the generated registration table. Registration is
// permitted only inside InitExternalFormulaRegistry's initializer, must not
// perform registry lookups, and cannot be used for runtime plugin registration.
void RegisterExternalFormulaTable();
void RegisterExternalFormula(const ExternalFormulaInfo& info);
const ExternalFormulaInfo* FindExternalFormula(const char* name);
const ExternalFormulaInfo* FindExternalFormulaById(unsigned int id);
// Catalog enumeration in registration order.
size_t ExternalFormulaCount();
const ExternalFormulaInfo* ExternalFormulaAt(size_t index);
ThybridIteration GetExternalFormula(const char* customFname, int* outDEoption = nullptr);
int ReadFormulaParams(const void* pVar, const FormulaParamMap& map, double* outParams);
void AppendCValues(TPIteration3D pit, double* params, int& paramCount);
void AppendPersistentCValues(TPIteration3D pit, double* params, int& paramCount);

// Some transforms need both the mutable Julia constants and original point.
// Append original C1..C3/J4 after the normal Julia and extended-state lanes.
template<unsigned int FormulaId>
struct ExternalFormulaAppendsPersistentCLanes { static constexpr bool value = false; };
template<unsigned int FormulaId>
struct ExternalFormulaAppendsJuliaMode { static constexpr bool value = false; };
void AppendExtState(TIteration3Dext* ext, double* params, int& paramCount);
void WriteBackVaryScale(TIteration3Dext* ext, const double* params, int varyScaleIdx);
TIteration3Dext* It3DextFromPIt3D(TPIteration3D pit);
// Apply correct option types from external formula registry to THAformula.
// Must be called before BuildFormulaConstBuffer for external formulas.
void ApplyExternalFormulaOptionTypes(struct THAformula& formula);

inline int FindFormulaSlot(TPIteration3D pit) {
    if (!pit || !pit->PVar) return -1;
    for (int i = 0; i < 6; ++i) {
        if (pit->fHPVar[i] == pit->PVar) return i;
    }
    return -1;
}

inline void AppendExternalFormulaContractTrace(const char* stage,
                                              const char* formulaName,
                                              int deOption,
                                              TPIteration3D pit,
                                              TIteration3Dext* ext,
                                              double x,
                                              double y,
                                              double z,
                                              double w) {
    if (!FormulaTraceActive() || !pit || !ext || !pit->PVar) return;

    static const int kOffsets[] = {
        -64, -56, -48, -40, -32, -24, -16, -8,
          0,   8,  16,  24,  32,  40,  48,  56,  64
    };

    std::ostringstream payload;
    payload.setf(std::ios::fixed, std::ios::floatfield);
    payload << std::setprecision(12)
            << "slot=" << FindFormulaSlot(pit)
            << " formula=" << formulaName
            << " DEoption=" << deOption
            << " ItResultI=" << pit->ItResultI
            << " x=" << x
            << " y=" << y
            << " z=" << z
            << " w=" << w
            << " C1=" << pit->C1
            << " C2=" << pit->C2
            << " C3=" << pit->C3
            << " J1=" << pit->J1
            << " J2=" << pit->J2
            << " J3=" << pit->J3
            << " J4=" << ext->J4
            << " Rout=" << pit->Rout
            << " OTrap=" << pit->OTrap
            << " Rold=" << ext->Rold
            << " VaryScale=" << ext->VaryScale
            << " bFirstIt=" << ext->bFirstIt
            << " Dfree1=" << pit->Dfree1
            << " Dfree2=" << pit->Dfree2;

    const uint8_t* base = static_cast<const uint8_t*>(pit->PVar);
    payload << std::uppercase << std::hex << std::setfill('0');
    for (int offset : kOffsets) {
        uint64_t raw = 0;
        std::memcpy(&raw, base + offset, sizeof(raw));
        payload << " pv";
        if (offset < 0) payload << 'm' << (-offset);
        else payload << 'p' << offset;
        payload << "=0x" << std::setw(16) << raw;
    }
    AppendFormulaTraceEvent(stage, payload.str());
}

// ============================================================================
// Wrapper macro for external formula .inc files
// ============================================================================

// Forward declare the template that each .inc file specializes.
// params is mutable so formulas can write back VaryScale (at a known index after C values).
template<unsigned int ID>
cpu_double3 execute_formula(cpu_double3 p, double* params, double& de);

// Map-aware specializations receive the iteration's actual callback. Keeping it
// explicit avoids global callback state and permits independent UV/ABI probes.
template<unsigned int ID>
inline cpu_double3 execute_formula_with_direct_map(cpu_double3 p, double* params,
                                                   double& de, TLMSfunction map) {
    return execute_formula<ID>(p, params, de);
}

template<unsigned int ID>
inline cpu_double3 execute_formula_with_sphere_map(cpu_double3 p, double* params,
                                                   double& de, TLMSfunction map) {
    return execute_formula<ID>(p, params, de);
}

template<unsigned int ID>
struct ExternalDifsUsesSphereMap { static constexpr bool value = false; };

// dIFS formula template: receives VaryScale and Rout as separate outputs
template<unsigned int ID>
cpu_double3 execute_formula_dIFS(cpu_double3 p, double* params,
                                  double& varyScale, double& rout);

// dIFS transform template (DEoption=21): only updates VaryScale, no shape DE
template<unsigned int ID>
cpu_double3 execute_formula_dIFS_transform(cpu_double3 p, double* params,
                                            double& varyScale);

template<unsigned int ID>
inline cpu_double3 execute_formula_dIFS(cpu_double3 p, double* params,
                                        double& varyScale, double& rout) {
    params[DIFS_VARYSCALE_SLOT] = varyScale;
    double de = rout;
    cpu_double3 result = execute_formula<ID>(p, params, de);
    varyScale = params[DIFS_VARYSCALE_SLOT];
    rout = de;
    return result;
}

template<unsigned int ID>
inline cpu_double3 execute_formula_dIFS_transform(cpu_double3 p, double* params,
                                                  double& varyScale) {
    params[DIFS_VARYSCALE_SLOT] = varyScale;
    double de = varyScale;
    cpu_double3 result = execute_formula<ID>(p, params, de);
    varyScale = params[DIFS_VARYSCALE_SLOT];
    return result;
}

// The five conditional assets operate on the iteration engine, not on xyz.
// Offsets below are the original PVar ABI (including expanded DSQUARE options).
// Keep sequential writes: two targets may designate the same hybrid slot.
template<unsigned ID>
inline void ExecuteConditionalFormula(TPIteration3D it) {
    auto integer = [&](int offset) { int32_t v; std::memcpy(&v, static_cast<const char*>(it->PVar)+offset,4); return v; };
    auto real = [&](int offset) { double v; std::memcpy(&v, static_cast<const char*>(it->PVar)+offset,8); return v; };
    auto write = [&](int slot,int count) {
        int32_t& dest=it->nHybrid[std::clamp(slot,1,6)-1];
        uint32_t bits=(static_cast<uint32_t>(dest)&0x80000000u)|static_cast<uint32_t>(count);
        std::memcpy(&dest,&bits,4);
    };
    if constexpr(ID==10353) {
        // x87 unordered comparisons follow the low branch, as do equal radii.
        int offset=!(it->Rout>real(-16)) ? -32 : !(it->Rout>real(-24)) ? -36 : -40;
        write(integer(-28),std::clamp(integer(offset),0,99999));
    } else {
        uint32_t counter=static_cast<uint32_t>(it->bFirstIt)+1u;
        std::memcpy(&it->bFirstIt,&counter,4);
        if constexpr(ID==10354 || ID==10355) {
            for(int base=0;base<(ID==10355?40:20);base+=20) {
                int threshold=std::clamp(integer(-12-base),1,100);
                int offset=it->bFirstIt<threshold ? -20 : it->bFirstIt==threshold ? -24 : -28;
                write(integer(-16-base),std::clamp(integer(offset-base),0,100));
            }
        } else {
            unsigned shift=std::max(0,static_cast<int>(counter&15u)-1);
            bool bit=(static_cast<uint32_t>(integer(-12))>>shift)&1u;
            int a=integer(-16),b=integer(-20);
            int low=ID==10357 ? std::clamp(integer(-24),0,100) : 0;
            int high=ID==10357 ? std::clamp(integer(-28),1,100) : 1;
            write(bit?b:a,low); write(bit?a:b,high);
        }
    }
}

// ADDS_C: 1 = formula adds C internally from params, 0 = wrapper must add C after call
// DIFS_STYLE: 0 = uses execute_formula_dIFS<> template, 1 = uses execute_formula<> with de output
#define DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA_ID, DE_OPTION, PARAM_MAP_VAR) \
    DEFINE_EXTERNAL_FORMULA_WRAPPER_EX(FORMULA_ID, DE_OPTION, PARAM_MAP_VAR, 1, 0)

#define DEFINE_EXTERNAL_FORMULA_WRAPPER_EX(FORMULA_ID, DE_OPTION, PARAM_MAP_VAR, ADDS_C, DIFS_STYLE) \
    static void ExternWrap_##FORMULA_ID(double& x, double& y, double& z, double& w, \
                                         TPIteration3D PIt3D) { \
        TIteration3Dext* ext = It3DextFromPIt3D(PIt3D); \
        if constexpr((FORMULA_ID)>=10353 && (FORMULA_ID)<=10357) { \
            AppendExternalFormulaContractTrace("formula_contract_in", #FORMULA_ID, (DE_OPTION), PIt3D, ext, x,y,z,w); \
            ExecuteConditionalFormula<FORMULA_ID>(PIt3D); \
            AppendExternalFormulaContractTrace("formula_contract_out", #FORMULA_ID, (DE_OPTION), PIt3D, ext, x,y,z,w); \
            return; \
        } \
        double params[MAX_FORMULA_PARAMS] = {}; \
        int nParams = ReadFormulaParams(PIt3D->PVar, PARAM_MAP_VAR, params); \
        if (HasParityTraceContext() && ShouldLogParityStage("formula_param_snapshot")) { \
            std::ostringstream payload; \
            payload.setf(std::ios::fixed, std::ios::floatfield); \
            payload.precision(12); \
            payload << "formula=" << #FORMULA_ID << " nParams=" << nParams; \
            for (int i = 0; i < nParams; ++i) { \
                payload << " p" << i << "=" << params[i]; \
            } \
            AppendParityTrace("formula_param_snapshot", payload.str()); \
        } \
        int cIdx = nParams; /* index where C values start */ \
        if (ExternalFormulaConsumesPersistentCLanes<FORMULA_ID>::value) AppendPersistentCValues(PIt3D, params, nParams); \
        else AppendCValues(PIt3D, params, nParams); \
        AppendExtState(ext, params, nParams); \
        if (ExternalFormulaAppendsPersistentCLanes<FORMULA_ID>::value) AppendPersistentCValues(PIt3D, params, nParams); \
        if (ExternalFormulaAppendsJuliaMode<FORMULA_ID>::value) params[nParams++] = PIt3D->DoJulia; \
        if (ExternalFormulaReadsRout<FORMULA_ID>::value) params[FORMULA_ROUT_SLOT] = PIt3D->Rout; \
        params[WRITE_J1_SLOT] = std::numeric_limits<double>::quiet_NaN(); \
        params[WRITE_J2_SLOT] = std::numeric_limits<double>::quiet_NaN(); \
        params[WRITE_J3_SLOT] = std::numeric_limits<double>::quiet_NaN(); \
        params[WRITE_J4_SLOT] = std::numeric_limits<double>::quiet_NaN(); \
        if (ExternalFormulaUsesSse01<FORMULA_ID>::value) { \
            params[FORMULA_ORBIT_W_SLOT] = w; \
            params[FORMULA_STATE_X_SLOT] = ext->FormulaSse01[0]; \
            params[FORMULA_STATE_Y_SLOT] = ext->FormulaSse01[1]; \
            params[FORMULA_STATE_Z_SLOT] = ext->FormulaSse01[2]; \
            params[FORMULA_STATE_W_SLOT] = ext->FormulaSse01[3]; \
        } else if (ExternalFormulaUsesKaliState<FORMULA_ID>::value) { \
            params[FORMULA_STATE_X_SLOT] = ext->Dfree1; \
            params[FORMULA_STATE_Y_SLOT] = ext->Dfree2; \
            params[FORMULA_STATE_Z_SLOT] = ext->Deriv2; \
            params[FORMULA_STATE_W_SLOT] = ext->Deriv3; \
        } else if (ExternalFormulaUsesXYZWState<FORMULA_ID>::value) { \
            params[FORMULA_STATE_X_SLOT] = ext->Deriv1; \
            params[FORMULA_STATE_Y_SLOT] = ext->Deriv2; \
            params[FORMULA_STATE_Z_SLOT] = ext->Deriv3; \
            params[FORMULA_STATE_W_SLOT] = ext->Dfree1; \
        } else if (ExternalFormulaUsesXYZState<FORMULA_ID>::value) { \
            params[FORMULA_STATE_X_SLOT] = ext->Deriv1; \
            params[FORMULA_STATE_Y_SLOT] = ext->Deriv2; \
            params[FORMULA_STATE_Z_SLOT] = ext->Deriv3; \
        } \
        if (ExternalFormulaWritesWState<FORMULA_ID>::value) { \
            params[FORMULA_STATE_W_SLOT] = w; \
        } \
        params[ITER_COUNT_SLOT] = static_cast<double>(PIt3D->ItResultI); \
        params[BFIRSTIT_SLOT] = static_cast<double>(ext->bFirstIt); \
        cpu_double3 p_in(x, y, z); \
        cpu_double3 result; \
        AppendExternalFormulaContractTrace("formula_contract_in", #FORMULA_ID, (DE_OPTION), PIt3D, ext, x, y, z, w); \
        if ((DE_OPTION) == 20 && (DIFS_STYLE) == 0) { \
            double vs = ext->VaryScale; \
            double rout = 0.0; \
            params[MAX_FORMULA_PARAMS - 1] = ext->Dfree1; \
            params[MAX_FORMULA_PARAMS - 2] = ext->RStopD; \
            params[DIFS_ROLD_SLOT] = ext->Rold; \
            params[DIFS_COMMIT_XYZ_SLOT] = 1.0; \
            result = execute_formula_dIFS<FORMULA_ID>(p_in, params, vs, rout); \
            ext->VaryScale = vs; \
            ext->Rout = rout; \
            /* Do NOT write w = rout for dIFS: doHybridIFS3D reads Rout, not w. */ \
            ext->Dfree1 = params[MAX_FORMULA_PARAMS - 1]; \
            if (params[DIFS_COMMIT_XYZ_SLOT] == 0.0) { \
                result = p_in; \
            } \
        } else if ((DE_OPTION) == 20 && (DIFS_STYLE) == 1) { \
            /* dIFS formula using standard execute_formula<> template */ \
            /* Pascal passes x/y/z by reference and accepts the formula's post-call */ \
            /* coordinates directly. STYLE=1 formulas that need "no commit" already */ \
            /* return their preserved state explicitly (for example via state_x/y/z */ \
            /* or by returning p unchanged). A wrapper-side fallback to p_in discards */ \
            /* valid transformed coordinates for many formulas and creates a shared */ \
            /* dIFS regression cluster. */ \
            params[MAX_FORMULA_PARAMS - 1] = ext->Dfree1; \
            params[MAX_FORMULA_PARAMS - 2] = ext->RStopD; \
            params[DIFS_VARYSCALE_SLOT] = ext->VaryScale; \
            params[DIFS_ROLD_SLOT] = ext->Rold; \
            params[DIFS_COMMIT_XYZ_SLOT] = 0.0; \
            /* STYLE=1 dIFS traces still show zero incoming Rout on formula entry */ \
            /* for representative Pascal cases such as helixIFS. Keep the wrapper */ \
            /* aligned with that contract until a counterexample is proven by trace. */ \
            double de = 0.0; \
            if constexpr (ExternalDifsUsesSphereMap<FORMULA_ID>::value) \
                result = execute_formula_with_sphere_map<FORMULA_ID>(p_in, params, de, ext->PMapFunc); \
            else \
                result = execute_formula_with_direct_map<FORMULA_ID>(p_in, params, de, ext->PMapFunc2); \
            ext->VaryScale = params[DIFS_VARYSCALE_SLOT]; \
            ext->bFirstIt = static_cast<int32_t>(params[BFIRSTIT_SLOT]); \
            ext->Rout = de; \
            /* Do NOT write w = de for dIFS: doHybridIFS3D reads Rout, not w. */ \
            /* Writing de into w contaminates the next IFS iteration's w input. */ \
            /* Pascal's dIFS formula ASM does not write DE to w. */ \
            ext->Dfree1 = params[MAX_FORMULA_PARAMS - 1]; \
        } else if ((DE_OPTION) == 21 && (DIFS_STYLE) == 0) { \
            double vs = ext->VaryScale; \
            result = execute_formula_dIFS_transform<FORMULA_ID>(p_in, params, vs); \
            ext->VaryScale = vs; \
        } else if ((DE_OPTION) == 21 && (DIFS_STYLE) == 1) { \
            /* dIFS transform using standard execute_formula<> template */ \
            double savedVS = ext->VaryScale; \
            params[DIFS_VARYSCALE_SLOT] = ext->VaryScale; \
            double de = ext->VaryScale; \
            result = execute_formula<FORMULA_ID>(p_in, params, de); \
            ext->VaryScale = (params[DIFS_VARYSCALE_SLOT] != savedVS) ? params[DIFS_VARYSCALE_SLOT] : de; \
            ext->bFirstIt = static_cast<int32_t>(params[BFIRSTIT_SLOT]); \
        } else { \
            double de = 0.0; \
            if ((DE_OPTION) == 0) de = 0.0; \
            else if ((DE_OPTION) == 2 || (DE_OPTION) == 11) de = w; \
            else if ((DE_OPTION) == 4 || (DE_OPTION) == 5 || (DE_OPTION) == 6) de = w; \
            /* Also set VaryScale at well-known slot for formulas that use it */ \
            params[DIFS_VARYSCALE_SLOT] = ext->VaryScale; \
            result = execute_formula_with_sphere_map<FORMULA_ID>(p_in, params, de, ext->PMapFunc); \
            /* DEoption=6 (4D IFS analytic DE): Pascal ASM formulas update */ \
            /* Deriv1 *= Scale on each iteration via [esi+0xE8]. Apply */ \
            /* unconditionally — all DEoption=6 formulas follow this contract. */ \
            if ((DE_OPTION) == 6) { \
                ext->Deriv1 *= std::abs(params[0]); \
            } \
            /* Write back VaryScale from well-known slot (preferred) or AppendExtState */ \
            ext->VaryScale = params[DIFS_VARYSCALE_SLOT]; \
            ext->bFirstIt = static_cast<int32_t>(params[BFIRSTIT_SLOT]); \
            if ((DE_OPTION) == 2 || (DE_OPTION) == 11) w = de; \
            else if ((DE_OPTION) == 4 || (DE_OPTION) == 5 || (DE_OPTION) == 6) w = de; \
        } \
        if (HasParityTraceContext() && ShouldLogParityStage("formula_raw_result")) { \
            AppendExternalFormulaContractTrace("formula_raw_result", #FORMULA_ID, (DE_OPTION), PIt3D, ext, result.x, result.y, result.z, w); \
        } \
        /* Add C (Julia constants) for formulas that don't do it internally */ \
        /* Pascal ASM formulas add C via [edi-0x68/0x60/0x58] (J1/J2/J3). */ \
        /* ADDS_C=0 means wrapper must add C; ADDS_C=1 means formula handles it */ \
        /* (or formula doesn't need C, e.g., IFS formulas). */ \
        if (!(ADDS_C)) { \
            double cx_add = params[cIdx + 0]; \
            double cy_add = params[cIdx + 1]; \
            double cz_add = params[cIdx + 2]; \
            double cw_add = params[cIdx + 3]; \
            /* Some formulas (for example _FlipXYc) update Julia state that must feed */ \
            /* the same-iteration wrapper-side +C, while the generic 3D variant was */ \
            /* previously shown to break formulas like _SphereInvC. Keep this behavior */ \
            /* trait-gated rather than turning it on for every 3D ADDS_C=0 formula. */ \
            if ((DE_OPTION) == 4 || (DE_OPTION) == 5 || (DE_OPTION) == 6 || \
                ExternalFormulaUsesWrittenCForSameIterationAdd<FORMULA_ID>::value) { \
                if (!std::isnan(params[WRITE_J1_SLOT])) cx_add = params[WRITE_J1_SLOT]; \
                if (!std::isnan(params[WRITE_J2_SLOT])) cy_add = params[WRITE_J2_SLOT]; \
                if (!std::isnan(params[WRITE_J3_SLOT])) cz_add = params[WRITE_J3_SLOT]; \
                if (!std::isnan(params[WRITE_J4_SLOT])) cw_add = params[WRITE_J4_SLOT]; \
            } \
            result.x += cx_add; \
            result.y += cy_add; \
            result.z += cz_add; \
            if ((DE_OPTION) == 4 || (DE_OPTION) == 5 || (DE_OPTION) == 6) { \
                w += cw_add; \
            } \
        } \
        if (ExternalFormulaUsesSse01<FORMULA_ID>::value) { \
            ext->FormulaSse01[0] = params[FORMULA_STATE_X_SLOT]; \
            ext->FormulaSse01[1] = params[FORMULA_STATE_Y_SLOT]; \
            ext->FormulaSse01[2] = params[FORMULA_STATE_Z_SLOT]; \
            ext->FormulaSse01[3] = params[FORMULA_STATE_W_SLOT]; \
        } else if (ExternalFormulaUsesKaliState<FORMULA_ID>::value) { \
            ext->Dfree1 = params[FORMULA_STATE_X_SLOT]; \
            ext->Dfree2 = params[FORMULA_STATE_Y_SLOT]; \
            ext->Deriv2 = params[FORMULA_STATE_Z_SLOT]; \
            ext->Deriv3 = params[FORMULA_STATE_W_SLOT]; \
        } else if (ExternalFormulaUsesXYZWState<FORMULA_ID>::value) { \
            ext->Deriv1 = params[FORMULA_STATE_X_SLOT]; \
            ext->Deriv2 = params[FORMULA_STATE_Y_SLOT]; \
            ext->Deriv3 = params[FORMULA_STATE_Z_SLOT]; \
            ext->Dfree1 = params[FORMULA_STATE_W_SLOT]; \
        } else if (ExternalFormulaUsesXYZState<FORMULA_ID>::value) { \
            ext->Deriv1 = params[FORMULA_STATE_X_SLOT]; \
            ext->Deriv2 = params[FORMULA_STATE_Y_SLOT]; \
            ext->Deriv3 = params[FORMULA_STATE_Z_SLOT]; \
        } \
        if (ExternalFormulaWritesWState<FORMULA_ID>::value) { \
            w = params[FORMULA_STATE_W_SLOT]; \
        } \
        if (ExternalFormulaAlwaysWritesJuliaXYZ<FORMULA_ID>::value || !std::isnan(params[WRITE_J1_SLOT])) { PIt3D->J1 = params[WRITE_J1_SLOT]; if (ExternalFormulaWritesPersistentCLanes<FORMULA_ID>::value) PIt3D->C1 = params[WRITE_J1_SLOT]; } \
        if (ExternalFormulaAlwaysWritesJuliaXYZ<FORMULA_ID>::value || !std::isnan(params[WRITE_J2_SLOT])) { PIt3D->J2 = params[WRITE_J2_SLOT]; if (ExternalFormulaWritesPersistentCLanes<FORMULA_ID>::value) PIt3D->C2 = params[WRITE_J2_SLOT]; } \
        if (ExternalFormulaAlwaysWritesJuliaXYZ<FORMULA_ID>::value || !std::isnan(params[WRITE_J3_SLOT])) { PIt3D->J3 = params[WRITE_J3_SLOT]; if (ExternalFormulaWritesPersistentCLanes<FORMULA_ID>::value) PIt3D->C3 = params[WRITE_J3_SLOT]; } \
        if (ExternalFormulaAlwaysWritesJuliaW<FORMULA_ID>::value || !std::isnan(params[WRITE_J4_SLOT])) { ext->J4 = params[WRITE_J4_SLOT]; } \
        AppendExternalFormulaContractTrace("formula_contract_out", #FORMULA_ID, (DE_OPTION), PIt3D, ext, result.x, result.y, result.z, w); \
        x = result.x; y = result.y; z = result.z; \
    }

} // namespace mb3d
