#include "core/render_control.h"
#include "core/formulas.h"
#include "core/formula_iteration_guard.h"
#include "core/math3d.h"
#include "core/parity_trace.h"
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <limits>
#include <stdexcept>

namespace mb3d {

// ============================================================================
// Helper: get TIteration3Dext* from TPIteration3D
// In Pascal, TIteration3D sits at offset +56 (7 doubles) inside TIteration3Dext
// TPIteration3Dext(Integer(PIteration3D) - 56)
// ============================================================================

static inline TIteration3Dext* ExtFromIt3D(TPIteration3D pit) {
    return reinterpret_cast<TIteration3Dext*>(
        reinterpret_cast<char*>(pit) - 7 * sizeof(double));
}

#if defined(_MSC_VER) && defined(_M_X64)
extern "C" void HybridFloatPow_x87(void* params);
extern "C" void CalcSmoothIterationsOriginal_x87(void* params);
extern "C" void HybridIntP8Original_x87(double* params);
extern "C" void HybridItIntPow2_x87(void* params);
extern "C" void HybridItIntPow2_x87_extcw(void* params);
extern "C" unsigned short GetX87ControlWord();
extern "C" void Rout3_x87(double* out, const double* xyz);
extern "C" void Rout3_x87_extcw(double* out, const double* xyz);
extern "C" void Rout3_x87_pascw(double* out, const double* xyz);
extern "C" void Rout3_pas_sse2(double* out, const double* xyz);
extern "C" void Rout3_compiled_pas_sse2(double* out, const double* xyz);
extern "C" void Atan2_x87_pascw(double* out, const double* y, const double* x);
extern "C" void Pow_x87_pascw(double* out, const double* base, const double* expo);

static inline double Atan2_Pas(double y, double x) {
    double out;
    Atan2_x87_pascw(&out, &y, &x);
    return out;
}
static inline double Pow_Pas(double base, double expo) {
    double out;
    Pow_x87_pascw(&out, &base, &expo);
    return out;
}
#else
static inline double Atan2_Pas(double y, double x) { return std::atan2(y, x); }
static inline double Pow_Pas(double base, double expo) { return std::pow(base, expo); }
#endif

struct HybridItIntPow2X87Params {
    double x;
    double y;
    double z;
    double j1;
    double j2;
    double j3;
    double dZmul;
};

// Env lookups cached — see src/core/ray_marcher.cpp for rationale.
static inline bool DebugEnvOnce(const char* name) {
    const char* spec = std::getenv(name);
    return spec && *spec;
}

#if defined(_MSC_VER)
__declspec(noinline)
#elif defined(__GNUC__) || defined(__clang__)
__attribute__((noinline))
#endif
static void DebugOrbitBarrier(double& x, double& y, double& z, double& w,
                                                   double& rout, double& otrap) {
    volatile double sx = x;
    volatile double sy = y;
    volatile double sz = z;
    volatile double sw = w;
    volatile double sr = rout;
    volatile double so = otrap;
    x = sx;
    y = sy;
    z = sz;
    w = sw;
    rout = sr;
    otrap = so;
}

static inline bool DebugOrbitBarrierEnabled() {
    static const bool value = DebugEnvOnce("MB3D_DEBUG_ORBIT_BARRIER");
    return value;
}

static inline bool DebugHybridP2PascalOrderEnabled() {
    static const bool value = DebugEnvOnce("MB3D_DEBUG_HYBRID_P2_PASCAL_ORDER");
    return value;
}

static inline double ComputeHybridRoutPasLike(double x, double y, double z) {
    // FPC -CfSSE2 uses y*y + x*x, then z*z + that sum. Preserve operand
    // order as well as rounding: the destination determines NaN propagation,
    // and CalcSmoothIterations inspects the signed high word of this value.
#if defined(_MSC_VER) && defined(_M_X64)
    const double xyz[] = {x, y, z};
    double result;
    Rout3_compiled_pas_sse2(&result, xyz);
    return result;
#else
    double xx = x * x;
    double yy = y * y;
    double zz = z * z;
    return (xx + yy) + zz;
#endif
}

static inline void MaterializeFormulaDouble(double& value) {
    volatile double spill = value;
    value = spill;
}

static inline std::string DoubleBitsHex(double value) {
    std::uint64_t bits = 0;
    std::memcpy(&bits, &value, sizeof(bits));
    std::ostringstream ss;
    ss << "0x" << std::uppercase << std::hex << std::setw(16) << std::setfill('0') << bits;
    return ss.str();
}

static inline void TraceFormulaState(const char* stage, int slot, int itResultI, int bTmp,
                                     double x, double y, double z, double w,
                                     double rout, double otrap, bool includeBTmp) {
    if (!FormulaTraceActive()) return;
    std::ostringstream payload;
    payload.setf(std::ios::fixed, std::ios::floatfield);
    payload << std::setprecision(17)
            << "slot=" << slot
            << " ItResultI=" << itResultI;
    if (includeBTmp) {
        payload << " bTmp=" << bTmp;
    }
    payload << " pvar=slot" << slot
            << " x=" << x
            << " x_bits=" << DoubleBitsHex(x)
            << " y=" << y
            << " y_bits=" << DoubleBitsHex(y)
            << " z=" << z
            << " z_bits=" << DoubleBitsHex(z)
            << " w=" << w
            << " w_bits=" << DoubleBitsHex(w)
            << " Rout=" << rout
            << " Rout_bits=" << DoubleBitsHex(rout)
            << " OTrap=" << otrap;
    AppendFormulaTraceEvent(stage, payload.str());
}

static inline void TraceFormulaContractState(const char* stage, int slot,
                                             TPIteration3D it3d,
                                             double x, double y, double z, double w) {
    if (!FormulaTraceActive()) return;
    static constexpr int kRawOffsets[] = {
        -64, -56, -48, -40, -32, -24, -16, -8, 0, 8, 16, 24, 32, 40, 48, 56, 64
    };
    TIteration3Dext* ext = ExtFromIt3D(it3d);
    auto appendRawPointerWord = [](std::ostringstream& payload, const char* name, std::uint64_t raw) {
        std::ostringstream rawHex;
        rawHex << std::uppercase << std::hex << std::setw(16) << std::setfill('0') << raw;
        payload << " " << name << "=0x" << rawHex.str();
    };

    std::ostringstream payload;
    payload.setf(std::ios::fixed, std::ios::floatfield);
    payload << std::setprecision(17)
            << "slot=" << slot
            << " DEoption=" << it3d->DEoption
            << " ItResultI=" << it3d->ItResultI
            << " x=" << x
            << " y=" << y
            << " z=" << z
            << " w=" << w
            << " C1=" << it3d->C1
            << " C2=" << it3d->C2
            << " C3=" << it3d->C3
            << " J1=" << it3d->J1
            << " J2=" << it3d->J2
            << " J3=" << it3d->J3
            << " J4=" << ext->J4
            << " Rout=" << it3d->Rout
            << " OTrap=" << it3d->OTrap
            << " Rold=" << ext->Rold
            << " VaryScale=" << it3d->VaryScale
            << " bFirstIt=" << static_cast<int>(it3d->bFirstIt)
            << " Dfree1=" << it3d->Dfree1
            << " Dfree2=" << it3d->Dfree2;

    const char* pvarBytes = static_cast<const char*>(it3d->PVar);
    for (int offset : kRawOffsets) {
        std::uint64_t raw = 0;
        std::memcpy(&raw, pvarBytes + offset, sizeof(raw));
        char name[8];
        std::snprintf(name, sizeof(name), "%sv%x", offset < 0 ? "p" : "p", std::abs(offset));
        if (offset < 0) {
            std::snprintf(name, sizeof(name), "pvm%x", -offset);
        } else {
            std::snprintf(name, sizeof(name), "pvp%x", offset);
        }
        appendRawPointerWord(payload, name, raw);
    }
    AppendFormulaTraceEvent(stage, payload.str());
}

static inline void TraceIFSPostFormulaState(const char* stage, int slot, int itResultI,
                                            double deout, const TIteration3Dext* ext) {
    if (!FormulaTraceActive()) return;
    std::ostringstream payload;
    payload.setf(std::ios::fixed, std::ios::floatfield);
    payload << std::setprecision(17)
            << "slot=" << slot
            << " ItResultI=" << itResultI
            << " DEout=" << deout
            << " Rout=" << ext->Rout
            << " VaryScale=" << ext->VaryScale
            << " OTrap=" << ext->OTrap
            << " Dfree1=" << ext->Dfree1
            << " Dfree2=" << ext->Dfree2
            << " RStopD=" << ext->RStopD
            << " bInsideTmp=" << ext->bIsInsideRender;
    AppendFormulaTraceEvent(stage, payload.str());
}

static inline void TraceIFSReturnState(const char* stage, int slot,
                                       double minDe, const TIteration3Dext* ext) {
    if (!FormulaTraceActive()) return;
    std::ostringstream payload;
    payload.setf(std::ios::fixed, std::ios::floatfield);
    payload << std::setprecision(17)
            << "slot=" << slot
            << " ItResultI=" << ext->ItResultI
            << " minDE=" << minDe
            << " OTrap=" << ext->OTrap
            << " Dfree1=" << ext->Dfree1
            << " Dfree2=" << ext->Dfree2
            << " Rout=" << ext->Rout
            << " VaryScale=" << ext->VaryScale
            << " SmoothItD=" << ext->SmoothItD
            << " bTmp=" << ext->bTmp
            << " bInsideTmp=" << ext->bIsInsideRender;
    AppendFormulaTraceEvent(stage, payload.str());
}

const char* HybridIterationName(ThybridIteration fn) {
    if (fn == HybridItIntPow2) return "HybridItIntPow2";
    if (fn == HybridItIntPow3) return "HybridItIntPow3";
    if (fn == HybridItIntPow4) return "HybridItIntPow4";
    if (fn == HybridIntP5) return "HybridIntP5";
    if (fn == HybridIntP6) return "HybridIntP6";
    if (fn == HybridIntP7) return "HybridIntP7";
    if (fn == HybridIntP8) return "HybridIntP8";
    if (fn == HybridFloatPow) return "HybridFloatPow";
    if (fn == HybridQuat) return "HybridQuat";
    if (fn == HybridItTricorn) return "HybridItTricorn";
    if (fn == HybridCube) return "HybridCube";
    if (fn == HybridCubeDE) return "HybridCubeDE";
    if (fn == HybridCubeSSE2) return "HybridCubeSSE2";
    if (fn == HybridCubeSSE2DE) return "HybridCubeSSE2DE";
    if (fn == HybridItIntPow2scale) return "HybridItIntPow2scale";
    if (fn == HybridSuperCube2) return "HybridSuperCube2";
    if (fn == HybridFolding) return "HybridFolding";
    if (fn == TestHybrid) return "TestHybrid";
    if (fn == HybridCustomIFStest) return "HybridCustomIFStest";
    if (fn == AexionC) return "AexionC";
    if (fn == EmptyFormula) return "EmptyFormula";
    return "Hybrid_unknown";
}

// ============================================================================
// PVar access helpers
//
// PVar points into the option value buffer. For internal formulas, PVar is set
// to &dOptionValue[2], so:
//   PVar[-2] = dOptionValue[0] = power / dZmul
//   PVar[-1] = dOptionValue[1] = secondary option (e.g., Z multiplier)
//   PVar[0]  = dOptionValue[2] = first user option after power/zmul
//
// In Pascal, PVar is typed as generic Pointer and values are accessed as:
//   PDouble(Integer(PVar) - 16)^ means PVar[-2] in double units
//   PDouble(Integer(PVar) - 24)^ means PVar[-3] in double units
//
// The asm code also accesses precomputed constants at large positive offsets
// from PAligned16 (e.g., +120=3.0, +128=4.0, ...). In our C++ port, we use
// literal constants instead since they are known at compile time.
// ============================================================================

static inline double* PVarDoubles(void* pvar) {
    return static_cast<double*>(pvar);
}

// ============================================================================
// EmptyFormula -- placeholder for unused formula slots
// ============================================================================

void EmptyFormula(double& /*x*/, double& /*y*/, double& /*z*/, double& /*w*/, TPIteration3D /*PIteration3D*/) {
    // Intentionally empty -- used for formula slots with no assigned formula
}

// ============================================================================
// HybridItIntPow2 -- Mandelbulb power-2 formula (sine bulb)
// Ported from the SSE2 Pascal version in formulas.pas
//
// Algorithm:
//   S1 = x*x, S2 = y*y, S3 = z*z
//   XT = S1 + S2
//   z' = 2 * z * dZmul * sqrt(XT) + J3
//   a  = (XT - S3) / XT
//   x' = (S1 - S2) * a + J1
//   y' = 2 * x * y * a + J2
//
// dZmul is at PVar[-2] (offset -16 from PVar pointer)
// ============================================================================

#if defined(_MSC_VER)
#pragma float_control(precise, on, push)
#pragma fp_contract(off)
#endif
void HybridItIntPow2(double& x, double& y, double& z, double& w, TPIteration3D PIteration3D) {
    (void)w;
    double* PVar = PVarDoubles(PIteration3D->PVar);
    double dZmul = *(PVar - 2); // offset -16 bytes = -2 doubles

    double xIn = x;
    double yIn = y;
    double zIn = z;
    double S2 = y * y;
    double S1 = x * x;
    double XT = S1 + S2;
    double sqrtXT = std::sqrt(XT);
    double S3 = z * z;
    double a = XT - S3;
    a /= XT;

    double newX;
    double newY;
    double newZ;
#if defined(_MSC_VER) && defined(_M_X64)
    {
        HybridItIntPow2X87Params params{
            x, y, z,
            PIteration3D->J1,
            PIteration3D->J2,
            PIteration3D->J3,
            dZmul,
        };
        HybridItIntPow2_x87_extcw(&params);
        newX = params.x;
        newY = params.y;
        newZ = params.z;
    }
#else
    if (DebugHybridP2PascalOrderEnabled()) {
        // Emulate the Pascal x87 stack order more literally for diagnosis:
        // compute/store z first, then materialize and continue with z*z and a.
        newZ = sqrtXT;
        MaterializeFormulaDouble(newZ);
        newZ *= dZmul;
        MaterializeFormulaDouble(newZ);
        newZ *= z;
        MaterializeFormulaDouble(newZ);
        newZ += newZ;
        MaterializeFormulaDouble(newZ);
        newZ += PIteration3D->J3;
        MaterializeFormulaDouble(newZ);

        S3 = z * z;
        MaterializeFormulaDouble(S3);
        a = XT - S3;
        MaterializeFormulaDouble(a);
        a /= XT;
        MaterializeFormulaDouble(a);

        newX = S1 - S2;
        MaterializeFormulaDouble(newX);
        newX *= a;
        MaterializeFormulaDouble(newX);
        newX += PIteration3D->J1;
        MaterializeFormulaDouble(newX);

        newY = x * y;
        MaterializeFormulaDouble(newY);
        newY *= a;
        MaterializeFormulaDouble(newY);
        newY += newY;
        MaterializeFormulaDouble(newY);
        newY += PIteration3D->J2;
        MaterializeFormulaDouble(newY);
    } else {
        newZ = sqrtXT;
        newZ *= dZmul;
        newZ *= z;
        newZ += newZ;
        newZ += PIteration3D->J3;

        newX = S1 - S2;
        newX *= a;
        newX += PIteration3D->J1;

        newY = x;
        newY *= y;
        newY *= a;
        newY += newY;
        newY += PIteration3D->J2;
    }
#endif

    if (FormulaTraceActive()) {
        std::ostringstream payload;
        payload.setf(std::ios::fixed, std::ios::floatfield);
        payload << std::setprecision(17)
                << "formula=HybridItIntPow2"
                << " x_in=" << xIn
                << " y_in=" << yIn
                << " z_in=" << zIn
                << " w_in=" << w
                << " J1=" << PIteration3D->J1
                << " J2=" << PIteration3D->J2
                << " J3=" << PIteration3D->J3
                << " pvar=" << ParityPointerState(PIteration3D->PVar)
                << " dZmul=" << dZmul
                << " S1=" << S1
                << " S2=" << S2
                << " S3=" << S3
                << " XT=" << XT
                << " sqrt_XT=" << sqrtXT
                << " a=" << a
                << " x_out=" << newX
                << " y_out=" << newY
                << " z_out=" << newZ;
        AppendFormulaTraceEvent("formula_step_detail", payload.str());
    }

    x = newX;
    y = newY;
    z = newZ;
}
#if defined(_MSC_VER)
#pragma float_control(pop)
#endif

// ============================================================================
// HybridItIntPow3 -- Mandelbulb power-3 (White's formula, power 3)
// Ported from formulas.pas asm at line 4307
//
// Algorithm (from asm comments):
//   sx = x*x, sy = y*y, sz = z*z
//   R  = sx + sy
//   A  = 1 - 3*sz / (R + d1em40)    [equivalent identity: A = (R - 3*sz) / R]
//   Actually from asm: A = 1 - 3*sz/(R + dOption3)  where dOption3 comes from [edi+24]
//   But examining carefully: fld qword [edi + 24] is PAligned16[24] = 1e-100
//   So: A = 1 - 3*sz / (R + 1e-100)
//
//   x' = A * (sx - 3*sy) * x + J1
//   y' = A * (3*sx - sy) * y + J2
//   z' = dZmul * z * (sz - 3*R) + J3
//      Note: in the asm this is: J3 - dZmul * z * (sz - 3*R) which equals
//      J3 + dZmul * z * (3*R - sz)
// ============================================================================

void HybridItIntPow3(double& x, double& y, double& z, double& w, TPIteration3D PIteration3D) {
    (void)w;
    double* PVar = PVarDoubles(PIteration3D->PVar);
    double dZmul = *(PVar - 2);

    double sx = x * x;
    double sy = y * y;
    double sz = z * z;
    double R  = sx + sy;

    // A = 1 - 3*sz / (R + 1e-100)
    double A = 1.0 - 3.0 * sz / (R + 1e-100);

    // x' = A * (sx - 3*sy) * x + J1
    double newX = A * (sx - 3.0 * sy) * x + PIteration3D->J1;

    // y' = A * (3*sx - sy) * y + J2
    double newY = A * (3.0 * sx - sy) * y + PIteration3D->J2;

    // z' = J3 - dZmul * z * (sz - 3*R)
    // which is: dZmul * z * (3*R - sz) + J3
    double newZ = PIteration3D->J3 - dZmul * z * (sz - 3.0 * R);

    x = newX;
    y = newY;
    z = newZ;
}

// ============================================================================
// HybridItIntPow4 -- Mandelbulb power-4 (White's formula)
// Ported from formulas.pas line 4354 (Pascal comment block)
//
// Pascal code:
//   R  := x*x + y*y;
//   A  := 1 + (sz * (sz - 6*R)) / (R*R + d1em40);
//   y  := 4*x*y*A*(sx - sy) + J2;
//   z  := dZmul * 4*sqrt(R)*z*(R - sz) + J3;
//   x  := A*(sx*(sx - 6*sy) + sy*sy) + J1;
// ============================================================================

void HybridItIntPow4(double& x, double& y, double& z, double& w, TPIteration3D PIteration3D) {
    (void)w;
    double* PVar = PVarDoubles(PIteration3D->PVar);
    double dZmul = *(PVar - 2);

    double sx = x * x;
    double sy = y * y;
    double sz = z * z;
    double R  = sx + sy;

    double A = 1.0 + (sz * (sz - 6.0 * R)) / (R * R + 1e-40);

    double newY = 4.0 * x * y * A * (sx - sy) + PIteration3D->J2;
    double newZ = dZmul * 4.0 * std::sqrt(R) * z * (R - sz) + PIteration3D->J3;
    double newX = A * (sx * (sx - 6.0 * sy) + sy * sy) + PIteration3D->J1;

    x = newX;
    y = newY;
    z = newZ;
}

// ============================================================================
// HybridIntP5 -- Mandelbulb power-5 (White's formula)
// Ported from formulas.pas line 4421 (Pascal comment block)
//
// Pascal code:
//   sx := x*x; sy := y*y; R := sx + sy; sz := z*z;
//   A  := 1 + 5*(sz*sz - 2*R*sz) / (R*R + d1em40);
//   y  := A*y*(5*sx*sx - sy*(10*sx - sy)) + J2;
//   z  := dZmul * z * (sz*(sz - 10*R) + 5*R*R) + J3;
//   x  := A*x*(sx*(sx - 10*sy) + 5*sy*sy) + J1;
// ============================================================================

void HybridIntP5(double& x, double& y, double& z, double& w, TPIteration3D PIteration3D) {
    (void)w;
    double* PVar = PVarDoubles(PIteration3D->PVar);
    double dZmul = *(PVar - 2);

    double sx = x * x;
    double sy = y * y;
    double sz = z * z;
    double R  = sx + sy;

    double A = 1.0 + 5.0 * (sz * sz - 2.0 * R * sz) / (R * R + 1e-40);

    double newY = A * y * (5.0 * sx * sx - sy * (10.0 * sx - sy)) + PIteration3D->J2;
    double newZ = dZmul * z * (sz * (sz - 10.0 * R) + 5.0 * R * R) + PIteration3D->J3;
    double newX = A * x * (sx * (sx - 10.0 * sy) + 5.0 * sy * sy) + PIteration3D->J1;

    x = newX;
    y = newY;
    z = newZ;
}

// ============================================================================
// HybridIntP6 -- Mandelbulb power-6 (White's formula)
// Ported from formulas.pas line 4506 (Pascal comment block)
//
// Pascal code:
//   S1 := x*x; S2 := y*y; R := S1 + S2; sz := z*z;
//   A  := 1 - (sz*(sz*(sz - 15*R) + 15*R*R)) / (R*R*R + d1em40);
//   y  := 2*A*x*y*(S1*(3*S1 - 10*S2) + 3*S2*S2) + J2;
//   z  := dZmul * 2*z*sqrt(R)*(sz*(3*sz - 10*R) + 3*R*R) + J3;
//   x  := A*(S1*S1*(S1 - 15*S2) + S2*S2*(15*S1 - S2)) + J1;
// ============================================================================

void HybridIntP6(double& x, double& y, double& z, double& w, TPIteration3D PIteration3D) {
    (void)w;
    double* PVar = PVarDoubles(PIteration3D->PVar);
    double dZmul = *(PVar - 2);

    double S1 = x * x;
    double S2 = y * y;
    double sz = z * z;
    double R  = S1 + S2;

    double A = 1.0 - (sz * (sz * (sz - 15.0 * R) + 15.0 * R * R)) / (R * R * R + 1e-40);

    double newY = 2.0 * A * x * y * (S1 * (3.0 * S1 - 10.0 * S2) + 3.0 * S2 * S2) + PIteration3D->J2;
    double newZ = dZmul * 2.0 * z * std::sqrt(R) * (sz * (3.0 * sz - 10.0 * R) + 3.0 * R * R) + PIteration3D->J3;
    double newX = A * (S1 * S1 * (S1 - 15.0 * S2) + S2 * S2 * (15.0 * S1 - S2)) + PIteration3D->J1;

    x = newX;
    y = newY;
    z = newZ;
}

// ============================================================================
// HybridIntP7 -- Mandelbulb power-7 (White's formula)
// Ported from formulas.pas line 4607 (Pascal comment block)
//
// Pascal code:
//   S1 := x*x; S2 := y*y; R := S1 + S2; S3 := z*z;
//   A  := 1 - 7*(S3*(S3*(S3 - 5*R) + 3*R*R)) / (R*R*R + d1em40);
//   y  := A*y*(S1*(S1*(7*S1 - 35*S2) + 21*S2*S2) - S2*S2*S2) + J2;
//   z  := J3 - dZmul * (z*S3*S3*S3 - 7*z*R*(S3*(3*S3 - 5*R) + R*R));
//        Note: the sign is inverted compared to even powers
//   x  := A*x*(S1*(S1*(S1 - 21*S2) + 35*S2*S2) - 7*S2*S2*S2) + J1;
// ============================================================================

void HybridIntP7(double& x, double& y, double& z, double& w, TPIteration3D PIteration3D) {
    (void)w;
    double* PVar = PVarDoubles(PIteration3D->PVar);
    double dZmul = *(PVar - 2);

    double S1 = x * x;
    double S2 = y * y;
    double S3 = z * z;
    double R  = S1 + S2;

    double A = 1.0 - 7.0 * (S3 * (S3 * (S3 - 5.0 * R) + 3.0 * R * R)) / (R * R * R + 1e-40);

    double newY = A * y * (S1 * (S1 * (7.0 * S1 - 35.0 * S2) + 21.0 * S2 * S2) - S2 * S2 * S2) + PIteration3D->J2;
    // z formula: J3 - dZmul * z * (S3*S3*S3 - 7*R*(S3*(3*S3 - 5*R) + R*R))
    // Simplify: S3^3 = sz*sz*sz
    double zPart = z * (S3 * S3 * S3 - 7.0 * R * (S3 * (3.0 * S3 - 5.0 * R) + R * R));
    double newZ = PIteration3D->J3 - dZmul * zPart;
    double newX = A * x * (S1 * (S1 * (S1 - 21.0 * S2) + 35.0 * S2 * S2) - 7.0 * S2 * S2 * S2) + PIteration3D->J1;

    x = newX;
    y = newY;
    z = newZ;
}

// ============================================================================
// HybridIntP8 -- Mandelbulb power-8 (White's formula)
// Ported from formulas.pas asm at line 4716
//
// From the asm with comments:
//   sx = x*x, sy = y*y, sz = z*z
//   R  = sx + sy
//   RR = R*R
//   zzzz = sz*sz
//
//   z' = -dZmul * 8 * z * sqrt(R) * (sz - R) * (zzzz - 6*R*sz + RR) + J3
//
//   A  = 1 + (zzzz*(70*RR + zzzz) - 28*sz*R*(zzzz + RR)) / (RR*RR + d1em40)
//
//   xxxx = sx*sx, yyyy = sy*sy
//   y' = 8 * x * y * A * (yyyy*(7*sx - sy) + xxxx*(sx - 7*sy)) + J2
//   x' = A * (xxxx*xxxx + yyyy*(70*xxxx + yyyy) - 28*sx*sy*(yyyy + xxxx)) + J1
// ============================================================================

void HybridIntP8(double& x, double& y, double& z, double& w, TPIteration3D PIteration3D) {
    (void)w;
    auto* pvarBytes = static_cast<uint8_t*>(PIteration3D->PVar);
    double* PVar = PVarDoubles(PIteration3D->PVar);
    const long double dZmul = static_cast<long double>(*(PVar - 2));

    double c6d = 0.0, c7d = 0.0, c8d = 0.0, c28d = 0.0, c70d = 0.0, cOneD = 0.0, cTinyD = 0.0;
    std::memcpy(&c6d, pvarBytes + 144, sizeof(c6d));
    std::memcpy(&c7d, pvarBytes + 152, sizeof(c7d));
    std::memcpy(&c8d, pvarBytes + 160, sizeof(c8d));
    std::memcpy(&c28d, pvarBytes + 192, sizeof(c28d));
    std::memcpy(&c70d, pvarBytes + 208, sizeof(c70d));
    std::memcpy(&cOneD, pvarBytes + 32, sizeof(cOneD));
    std::memcpy(&cTinyD, pvarBytes + 24, sizeof(cTinyD));

#if defined(_MSC_VER) && defined(_M_X64)
    double state[] = {x, y, z, PIteration3D->J1, PIteration3D->J2,
                      PIteration3D->J3, double(dZmul), c6d, c7d, c8d,
                      c28d, c70d, cOneD, cTinyD};
    HybridIntP8Original_x87(state);
    x = state[0]; y = state[1]; z = state[2];
#else

    const long double c6 = static_cast<long double>(c6d);
    const long double c7 = static_cast<long double>(c7d);
    const long double c8 = static_cast<long double>(c8d);
    const long double c28 = static_cast<long double>(c28d);
    const long double c70 = static_cast<long double>(c70d);
    const long double cOne = static_cast<long double>(cOneD);
    const long double cTiny = static_cast<long double>(cTinyD);

    const long double lx = static_cast<long double>(x);
    const long double ly = static_cast<long double>(y);
    const long double lz = static_cast<long double>(z);

    const long double sx = lx * lx;
    const long double sy = ly * ly;
    const long double sz = lz * lz;
    const long double R = sx + sy;
    const long double RR = R * R;
    const long double zzzz = sz * sz;

    long double zTerm = R;
    zTerm *= sz;
    zTerm *= c6;
    zTerm = zzzz - zTerm;
    zTerm += RR;
    long double zScale = sz - R;
    zTerm *= zScale;
    zTerm *= std::sqrt(R);
    zTerm *= lz;
    zTerm *= c8;
    zTerm *= dZmul;
    zTerm = -zTerm;
    const long double newZ = zTerm + static_cast<long double>(PIteration3D->J3);

    long double ALeft = zzzz + RR;
    ALeft *= R;
    ALeft *= sz;
    ALeft *= c28;
    long double ARight = RR * c70;
    ARight += zzzz;
    ARight *= zzzz;
    long double denom = RR * RR;
    denom += cTiny;
    long double A = ARight - ALeft;
    A /= denom;
    A += cOne;

    const long double xxxx = sx * sx;
    const long double yyyy = sy * sy;

    long double yPoly1 = c7 * sx - sy;
    long double yPoly2 = sx - c7 * sy;
    long double yTerm = yyyy * yPoly1;
    yTerm += xxxx * yPoly2;
    yTerm *= c8;
    yTerm *= lx;
    yTerm *= ly;
    yTerm *= A;
    const long double newY = yTerm + static_cast<long double>(PIteration3D->J2);

    long double xTerm = c70 * xxxx;
    xTerm += yyyy;
    xTerm *= yyyy;
    long double xyTerm = yyyy + xxxx;
    xyTerm *= sy;
    xyTerm *= sx;
    xyTerm *= c28;
    long double xCore = xxxx * xxxx;
    xCore += xTerm;
    xCore -= xyTerm;
    xCore *= A;
    const long double newX = xCore + static_cast<long double>(PIteration3D->J1);

    x = static_cast<double>(newX);
    y = static_cast<double>(newY);
    z = static_cast<double>(newZ);
#endif
}

// ============================================================================
// HybridFloatPow -- Mandelbulb with arbitrary real exponent
// Ported from formulas.pas line 4227 (Pascal comment block)
//
// Algorithm (spherical coordinates):
//   th := ArcTan2(y, x)
//   ph := ArcTan2(z, sqrt(x*x + y*y))
//   pp := Rout^(0.5 * power)    where Rout = x*x+y*y+z*z (stored in PIteration3D->Rout)
//   x  := pp * cos(power*ph) * cos(power*th) + J1
//   y  := pp * cos(power*ph) * sin(power*th) + J2
//   z  := dZmul * pp * sin(power*ph) + J3
//
//   power = PVar[-2], dZmul = PVar[-3]
// ============================================================================

void HybridFloatPow(double& x, double& y, double& z, double& w, TPIteration3D PIteration3D) {
    (void)w;
    TIteration3Dext* ext = ExtFromIt3D(PIteration3D);
    double* PVar = PVarDoubles(PIteration3D->PVar);
    double power = *(PVar - 2);
    double dZmul = *(PVar - 3);

#if defined(_MSC_VER) && defined(_M_X64)
    // Keep the complete Pascal x87 expression on its stack: spilling each
    // angle/radius to double changes the finite-difference orbit used by DE.
    double params[] = {x, y, z, PIteration3D->J1, PIteration3D->J2,
                       PIteration3D->J3, power, dZmul, *(PVar - 1), ext->Rout};
    HybridFloatPow_x87(params);
    x = params[0]; y = params[1]; z = params[2];
#else
    double th = Atan2_Pas(y, x);
    double ph = Atan2_Pas(z, std::sqrt(x * x + y * y));
    double pp = Pow_Pas(ext->Rout, 0.5 * power);

    double sinPh, cosPh, sinTh, cosTh;
    SinCosD(power * ph, sinPh, cosPh);
    SinCosD(power * th, sinTh, cosTh);

    x = pp * cosPh * cosTh + PIteration3D->J1;
    y = pp * cosPh * sinTh + PIteration3D->J2;
    z = dZmul * pp * sinPh + PIteration3D->J3;
#endif
}

// ============================================================================
// HybridQuat -- 4D Quaternion iteration
// Ported from formulas.pas line 4088 (Pascal source)
//
// Algorithm:
//   x' = x*x - y*y - z*z - w*w + J1
//   y' = 2*(y*xt + z*w) + J2
//   z' = 2*(z*xt + dZmul*yt*w) + J3
//   w' = 2*(w*xt + yt*zt) + PVar[-3] + J4
//
//   dZmul = PVar[-2]
//   PVar[-3] is the J4 constant from options
//   J4 is at ext offset -56 (the J4 field)
// ============================================================================

void HybridQuat(double& x, double& y, double& z, double& w, TPIteration3D PIteration3D) {
    TIteration3Dext* ext = ExtFromIt3D(PIteration3D);
    double* PVar = PVarDoubles(PIteration3D->PVar);
    double dZmul = *(PVar - 2);
    double wConst = *(PVar - 3);  // PDouble(Integer(PVar) - 24)^

    double xt = x;
    double yt = y;
    double zt = z;

    x = x * x - y * y - z * z - w * w + PIteration3D->J1;
    y = 2.0 * (yt * xt + zt * w) + PIteration3D->J2;
    z = 2.0 * (zt * xt + dZmul * yt * w) + PIteration3D->J3;
    w = 2.0 * (w * xt + yt * zt) + wConst + ext->J4;
}

// ============================================================================
// HybridItTricorn -- Tricorn / Mandelbar formula
// Ported from formulas.pas line 4076 (Pascal comment block)
//
// Pascal code:
//   xt := x;
//   x  := x*x - y*y - z*z + J1;
//   y  := 2*xt*y + J2;
//   z  := dOption1 * xt * z + J3;
//
// From the asm: dOption1 is at PVar[-2] * some factor.
// Actually from asm line 4060: fmul qword [edi - 16] means PVar[-2] = dZmul
// And line 4062: fmul qword [edi - 24] means PVar[-3]
// But looking at the asm more carefully:
//   z' = z * x * PVar[-2] + J3 * PVar[-3]
// Wait, let me re-read the asm:
//   z*x, then fmul [edi-16]=PVar[-2] -> z*x*PVar[-2]
//   Then fld [esi+40]=J3, fmul [edi-24]=PVar[-3] -> J3*PVar[-3]
//   Then faddp -> z*x*PVar[-2] + J3*PVar[-3]
//   fstp [ecx] -> z' = z*x*PVar[-2] + J3*PVar[-3]
// But the Pascal says: z := dOption1 * xt * z + J3
// This means PVar[-3] should be 1.0 for normal operation, making it J3.
// The dZmul (PVar[-2]) serves as dOption1.
// ============================================================================

void HybridItTricorn(double& x, double& y, double& z, double& w, TPIteration3D PIteration3D) {
    (void)w;
    double* PVar = PVarDoubles(PIteration3D->PVar);
    double dZmul = *(PVar - 2);
    double J3mul = *(PVar - 3);

    double xt = x;
    x = x * x - y * y - z * z + PIteration3D->J1;
    y = 2.0 * xt * y + PIteration3D->J2;
    z = dZmul * xt * z + J3mul * PIteration3D->J3;
}

// ============================================================================
// Box fold helper -- used by HybridCube, HybridCubeDE, HybridCubeSSE2, etc.
//
// The box fold operation:
//   result = abs(val + fold) - abs(val - fold) - val
//
// This is equivalent to clamping val to [-fold, fold]:
//   if val > fold:  val = 2*fold - val
//   if val < -fold: val = -2*fold - val
// ============================================================================

static inline double BoxFold(double val, double fold) {
    return std::fabs(val + fold) - std::fabs(val - fold) - val;
}

// ============================================================================
// HybridCube -- Amazing Box / Mandelbox without DE tracking
// Ported from formulas.pas asm at line 4859
//
// PVar layout for Mandelbox:
//   PVar[-5] = fold value
//   PVar[-4] = minR^2 (sphere fold inner radius squared)
//   PVar[-3] = scale / minR^2 (for sphere fold when r^2 < minR^2)
//   PVar[-2] = scale (for sphere fold when minR^2 <= r^2 < 1.0)
//   PVar[+4] = 1.0 (fixedR^2, the outer sphere fold radius squared)
//
// Algorithm:
//   1. Box fold each axis with fold value
//   2. Compute r^2 = x^2 + y^2 + z^2
//   3. Sphere fold:
//      if r^2 < minR^2:       mul = scale/minR^2
//      else if r^2 < fixedR^2: mul = scale/r^2
//      else:                   mul = scale
//   4. x = mul*x + J1, y = mul*y + J2, z = mul*z + J3
// ============================================================================

void HybridCube(double& x, double& y, double& z, double& w, TPIteration3D PIteration3D) {
    (void)w;
    double* PVar = PVarDoubles(PIteration3D->PVar);

    double fold    = *(PVar - 5);  // PVar[-40 bytes] = PVar[-5 doubles]
    double minRsqr = *(PVar - 4);  // PVar[-32 bytes] = PVar[-4 doubles]
    double scaleDivMinR = *(PVar - 3); // PVar[-24 bytes]
    double scale   = *(PVar - 2);  // PVar[-16 bytes]
    double fixedRsqr = *(PVar + 4); // PVar[+32 bytes] = 1.0

    double xIn = x;
    double yIn = y;
    double zIn = z;

    // Box fold
    double foldedX = BoxFold(x, fold);
    double foldedY = BoxFold(y, fold);
    double foldedZ = BoxFold(z, fold);
    x = foldedX;
    y = foldedY;
    z = foldedZ;

    // Sphere fold
    double r2 = x * x + y * y + z * z;
    double mul;
    const char* sphereCase;
    if (r2 < minRsqr) {
        mul = scaleDivMinR;
        sphereCase = "inner";
    } else if (r2 < fixedRsqr) {
        mul = scale / r2;
        sphereCase = "middle";
    } else {
        mul = scale;
        sphereCase = "outer";
    }

    // Scale and translate
    double newX = mul * x + PIteration3D->J1;
    double newY = mul * y + PIteration3D->J2;
    double newZ = mul * z + PIteration3D->J3;

    if (FormulaTraceActive()) {
        std::ostringstream payload;
        payload.setf(std::ios::fixed, std::ios::floatfield);
        payload << std::setprecision(12)
                << "formula=HybridCube"
                << " x_in=" << xIn
                << " y_in=" << yIn
                << " z_in=" << zIn
                << " w_in=" << w
                << " J1=" << PIteration3D->J1
                << " J2=" << PIteration3D->J2
                << " J3=" << PIteration3D->J3
                << " pvar=" << ParityPointerState(PIteration3D->PVar)
                << " fold=" << fold
                << " minRsqr=" << minRsqr
                << " scaleDivMinR=" << scaleDivMinR
                << " scale=" << scale
                << " fixedRsqr=" << fixedRsqr
                << " folded_x=" << foldedX
                << " folded_y=" << foldedY
                << " folded_z=" << foldedZ
                << " r2=" << r2
                << " mul=" << mul
                << " sphere_case=" << sphereCase
                << " x_out=" << newX
                << " y_out=" << newY
                << " z_out=" << newZ;
        AppendFormulaTraceEvent("formula_step_detail", payload.str());
    }

    x = newX;
    y = newY;
    z = newZ;
}

// ============================================================================
// HybridCubeDE -- Amazing Box / Mandelbox with DE tracking on w
// Ported from formulas.pas asm at line 4939
//
// Same as HybridCube but also multiplies w by the sphere fold factor.
// w tracks the derivative for distance estimation.
// ============================================================================

void HybridCubeDE(double& x, double& y, double& z, double& w, TPIteration3D PIteration3D) {
    double* PVar = PVarDoubles(PIteration3D->PVar);

    double fold    = *(PVar - 5);
    double minRsqr = *(PVar - 4);
    double scaleDivMinR = *(PVar - 3);
    double scale   = *(PVar - 2);
    double fixedRsqr = *(PVar + 4);

    // Box fold
    x = BoxFold(x, fold);
    y = BoxFold(y, fold);
    z = BoxFold(z, fold);

    // Sphere fold
    double r2 = x * x + y * y + z * z;
    double mul;
    if (r2 < minRsqr) {
        mul = scaleDivMinR;
    } else if (r2 < fixedRsqr) {
        mul = scale / r2;
    } else {
        mul = scale;
    }

    // DE tracking: w *= mul
    w = w * mul;

    // Scale and translate
    x = mul * x + PIteration3D->J1;
    y = mul * y + PIteration3D->J2;
    z = mul * z + PIteration3D->J3;
}

// ============================================================================
// HybridCubeSSE2 -- Amazing Box (SSE2 variant ported as scalar)
// Ported from formulas.pas asm at line 4812
//
// Same algorithm as HybridCube. The SSE2 version uses clamp operations
// (maxpd/minpd) for the box fold instead of the abs-based formula.
// The scalar port produces the same result.
//
// SSE2 box fold: clamp(val, -fold, fold) * 2 - val
// ============================================================================

void HybridCubeSSE2(double& x, double& y, double& z, double& w, TPIteration3D PIteration3D) {
    (void)w;
    double* PVar = PVarDoubles(PIteration3D->PVar);

    // SSE2 version uses negative fold at PVar[-8] = [ebx-64] and positive fold at PVar[-6] = [ebx-48]
    // In the SSE2 layout:
    //   [ebx-64]: -fold (negative, used as min clamp for max operation)
    //   [ebx-48]: +fold (positive, used as max clamp for min operation)
    //   [ebx-32]: minR^2
    //   [ebx-24]: scale/minR^2
    //   [ebx-16]: scale
    //   [ebx+32]: fixedR^2 = 1.0
    double negFold   = *(PVar - 8);  // -fold
    double posFold   = *(PVar - 6);  // +fold
    double minRsqr   = *(PVar - 4);
    double scaleDivMinR = *(PVar - 3);
    double scale     = *(PVar - 2);
    double fixedRsqr = *(PVar + 4);

    // Box fold via clamp: clamp(val, -fold, fold) * 2 - val
    auto clampFold = [&](double val) -> double {
        double clamped = std::max(val, negFold);
        clamped = std::min(clamped, posFold);
        return 2.0 * clamped - val;
    };

    x = clampFold(x);
    y = clampFold(y);
    z = clampFold(z);

    // Sphere fold
    double r2 = x * x + y * y + z * z;
    double mul;
    if (r2 < minRsqr) {
        mul = scaleDivMinR;
    } else if (r2 < fixedRsqr) {
        mul = scale / r2;
    } else {
        mul = scale;
    }

    // Scale and translate
    x = mul * x + PIteration3D->J1;
    y = mul * y + PIteration3D->J2;
    z = mul * z + PIteration3D->J3;
}

// ============================================================================
// HybridCubeSSE2DE -- Amazing Box SSE2 with DE tracking
// Ported from formulas.pas asm at line 5024
//
// Same as HybridCubeSSE2 but also multiplies w by the sphere fold factor.
// In the asm, w is at [ecx+8] and gets multiplied by the sphere fold mul.
// ============================================================================

void HybridCubeSSE2DE(double& x, double& y, double& z, double& w, TPIteration3D PIteration3D) {
    double* PVar = PVarDoubles(PIteration3D->PVar);

    double negFold   = *(PVar - 8);
    double posFold   = *(PVar - 6);
    double minRsqr   = *(PVar - 4);
    double scaleDivMinR = *(PVar - 3);
    double scale     = *(PVar - 2);
    double fixedRsqr = *(PVar + 4);

    // Box fold via clamp
    auto clampFold = [&](double val) -> double {
        double clamped = std::max(val, negFold);
        clamped = std::min(clamped, posFold);
        return 2.0 * clamped - val;
    };

    x = clampFold(x);
    y = clampFold(y);
    z = clampFold(z);

    // Sphere fold
    double r2 = x * x + y * y + z * z;
    double mul;
    if (r2 < minRsqr) {
        mul = scaleDivMinR;
    } else if (r2 < fixedRsqr) {
        mul = scale / r2;
    } else {
        mul = scale;
    }

    // DE tracking: w *= mul
    w = w * mul;

    // Scale and translate
    x = mul * x + PIteration3D->J1;
    y = mul * y + PIteration3D->J2;
    z = mul * z + PIteration3D->J3;
}

// ============================================================================
// HybridItIntPow2scale -- Sine bulb (power 2) with scaling
// Ported from formulas.pas asm at line 5072
//
// Same as IntPow2 but with an additional 1/scale factor applied before
// the iteration, and scale factor applied to the result.
//
// Algorithm:
//   invScale = 1.0 / scale    (scale is at PVar[-9] = offset -72 bytes)
//   x *= invScale, y *= invScale, z *= invScale
//   ... then standard IntPow2 ...
//   z' = -2 * scale * z * sqrt(XT) + J3
//   x' = scale * (S1-S2) * a + J1
//   y' = 2 * scale * x * y * a + J2
// ============================================================================

void HybridItIntPow2scale(double& x, double& y, double& z, double& w, TPIteration3D PIteration3D) {
    (void)w;
    double* PVar = PVarDoubles(PIteration3D->PVar);
    double scale = *(PVar - 9);  // offset -72 bytes from PVar

    double invScale = 1.0 / scale;
    x *= invScale;
    y *= invScale;
    z *= invScale;

    double S1 = x * x;
    double S2 = y * y;
    double S3 = z * z;
    double XT = S1 + S2;

    // z' = -2 * scale * z * sqrt(XT) + J3
    // In the asm: fchs then fmul [esi - 72] which is *scale
    double newZ = -2.0 * scale * z * std::sqrt(XT) + PIteration3D->J3;

    // a = (XT - S3) / XT
    double a = (XT - S3) / XT;

    // x' = scale * (S1 - S2) * a + J1
    double newX = scale * (S1 - S2) * a + PIteration3D->J1;

    // y' = 2 * scale * x * y * a + J2
    double newY = 2.0 * scale * x * y * a + PIteration3D->J2;

    x = newX;
    y = newY;
    z = newZ;
}

// ============================================================================
// HybridSuperCube2 -- Bulbox = hybrid Mandelbulb + Mandelbox
// Ported from formulas.pas line 5121 (Pascal source)
//
// If Rout < threshold2 (PVar[-10]):
//   If Rout < threshold1 (PVar[-11]):
//     Use IntPow2scale (pure bulb)
//   Else:
//     Blend between IntPow2scale and Cube based on distance from threshold1
// Else:
//   Use Cube (pure box)
// ============================================================================

void HybridSuperCube2(double& x, double& y, double& z, double& w, TPIteration3D PIteration3D) {
    TIteration3Dext* ext = ExtFromIt3D(PIteration3D);
    double* PVar = PVarDoubles(PIteration3D->PVar);

    double threshold2 = *(PVar - 10); // PVar[-80 bytes]  outer threshold
    double threshold1 = *(PVar - 11); // PVar[-88 bytes]  inner threshold

    if (ext->Rout < threshold2) {
        if (ext->Rout < threshold1) {
            // Pure bulb
            HybridItIntPow2scale(x, y, z, w, PIteration3D);
        } else {
            // Blend zone
            double R1 = threshold1;
            double m = (ext->Rout - R1) / (threshold2 - R1);

            // Save input position
            double xIn = x, yIn = y, zIn = z;

            // Compute bulb result
            HybridItIntPow2scale(x, y, z, w, PIteration3D);
            double xOut = x, yOut = y, zOut = z;

            // Restore input and compute box result
            x = xIn; y = yIn; z = zIn;
            fHybridCube(x, y, z, w, PIteration3D);

            // Blend
            double cubeR = std::sqrt(x * x + y * y + z * z);
            double m2 = 1.0 - m;
            double bulbR = std::sqrt(xOut * xOut + yOut * yOut + zOut * zOut);
            double targetR = cubeR * m + m2 * bulbR;

            x = x * m + m2 * xOut;
            y = y * m + m2 * yOut;
            z = z * m + m2 * zOut;

            // Normalize to target radius
            double currentR = std::sqrt(x * x + y * y + z * z + 1e-40);
            double scale = targetR / currentR;
            x *= scale;
            y *= scale;
            z *= scale;
        }
    } else {
        // Pure box
        fHybridCube(x, y, z, w, PIteration3D);
    }
}

// ============================================================================
// HybridFolding -- Box folding then sub-formula call
// Ported from formulas.pas asm at line 5160
//
// Algorithm:
//   1. Apply box fold to x, y, z using fold value from PVar[-3]
//   2. Call the sub-formula stored at PVar[-52/8] (function pointer)
//
// PVar layout for Folding:
//   PVar[-3]  = fold value (offset -24 bytes from PVar)
//   The function pointer is at a specific offset in the PVar buffer
//
// From asm: fold is at [edi-24], function pointer called at [edi-52]
// In doubles: PVar[-3] = fold, and at byte offset -52 there's a function pointer.
// Since doubles are 8 bytes: byte offset -52 is between double[-7] and double[-6].
// The function pointer is stored as a raw pointer at PVar byte offset -52.
//
// However, in the C++ port we need to handle this differently. The sub-formula
// is stored as a ThybridIteration function pointer. We read it from the
// PVar buffer at byte offset -52.
// ============================================================================

void HybridFolding(double& x, double& y, double& z, double& w, TPIteration3D PIteration3D) {
    double* PVar = PVarDoubles(PIteration3D->PVar);
    char* pvarBytes = reinterpret_cast<char*>(PIteration3D->PVar);
    double fold = *(PVar - 3);  // [edi - 24]

    // Box fold all three axes
    x = BoxFold(x, fold);
    y = BoxFold(y, fold);
    z = BoxFold(z, fold);

    ThybridIteration subFormula = nullptr;
    // Pascal stores the 32-bit callback at [edi-52] after 44 bytes of preceding
    // constants. In the 64-bit port the callback itself occupies 8 bytes, so the
    // equivalent slot starts at -56 and the surrounding constant offsets stay
    // aligned with the formula code.
    std::memcpy(&subFormula, pvarBytes - 56, sizeof(subFormula));
    if (!subFormula) subFormula = fHybridCube;
    subFormula(x, y, z, w, PIteration3D);
}

// ============================================================================
// TestHybrid -- internal test formula 7
// Active Pascal path is the PlusSinApp branch from formulas.pas.
// ============================================================================

void TestHybrid(double& x, double& y, double& z, double& w, TPIteration3D PIteration3D) {
    char* pvarBytes = reinterpret_cast<char*>(PIteration3D->PVar);

    int32_t dstIndex;
    int32_t srcIndex;
    float offset1;
    float scale1;
    float scale2;
    float offset2;
    std::memcpy(&dstIndex, pvarBytes - 12, sizeof(dstIndex));
    std::memcpy(&srcIndex, pvarBytes - 16, sizeof(srcIndex));
    std::memcpy(&offset1, pvarBytes - 20, sizeof(offset1));
    std::memcpy(&scale1, pvarBytes - 24, sizeof(scale1));
    std::memcpy(&scale2, pvarBytes - 28, sizeof(scale2));
    std::memcpy(&offset2, pvarBytes - 32, sizeof(offset2));

    double* vec[4] = {&x, &y, &z, &w};
    double t = (*vec[srcIndex & 3] - offset1) * scale1 * (Pi * 0.5);
    double r = t - std::nearbyint(t - 0.5);
    r += r;

    bool negate = false;
    if (r > 1.0) {
        r = 2.0 - r;
        negate = !negate;
    }

    double a = r * Pi;
    if (a >= Pi * 0.5) {
        a = Pi - a;
        negate = !negate;
    }

    double s = a - 0.148148148148148 * a * a * a;
    if (negate) s = -s;
    *vec[dstIndex & 3] += s * scale2 + offset2;
}

// ============================================================================
// HybridCustomIFStest -- internal test formula 8
// Active Pascal path is PlaneIFS with optional map-based orbit-trap coloring.
// ============================================================================

void HybridCustomIFStest(double& x, double& y, double& z, double& w, TPIteration3D PIteration3D) {
    (void)w;
    TIteration3Dext* ext = ExtFromIt3D(PIteration3D);
    char* pvarBytes = reinterpret_cast<char*>(PIteration3D->PVar);

    double normalZ;
    double normalY;
    double normalX;
    double offset;
    double scale;
    int32_t mapNr;
    int32_t mapChannel;
    float otrapOffset;
    float otrapScale;
    int32_t solid;

    std::memcpy(&normalZ, pvarBytes - 16, sizeof(normalZ));
    std::memcpy(&normalY, pvarBytes - 24, sizeof(normalY));
    std::memcpy(&normalX, pvarBytes - 32, sizeof(normalX));
    std::memcpy(&offset, pvarBytes - 40, sizeof(offset));
    std::memcpy(&scale, pvarBytes - 48, sizeof(scale));
    std::memcpy(&mapNr, pvarBytes - 52, sizeof(mapNr));
    std::memcpy(&mapChannel, pvarBytes - 56, sizeof(mapChannel));
    std::memcpy(&otrapOffset, pvarBytes - 60, sizeof(otrapOffset));
    std::memcpy(&otrapScale, pvarBytes - 64, sizeof(otrapScale));
    std::memcpy(&solid, pvarBytes - 68, sizeof(solid));

    double dist = normalX * x + normalY * y + normalZ * z - offset;
    if (solid == 0) {
        dist = std::fabs(dist);
    }
    ext->Rout = dist;

    if (mapNr == 0 || ext->PMapFunc2 == nullptr) {
        return;
    }

    TVec3D normal{normalX, normalY, normalZ};
    NormalizeVec3D_inplace(normal);

    TSVec ortho{};
    if (std::fabs(normal[0]) > 0.1) {
        ortho[0] = static_cast<float>(normal[2]);
        ortho[2] = static_cast<float>(-normal[0]);
    } else {
        ortho[1] = static_cast<float>(-normal[2]);
        ortho[2] = static_cast<float>(normal[1]);
    }
    NormalizeSVec_inplace(ortho);

    TSVec normalS{};
    DVecToSVec(normalS, normal);
    TSVec ortho2{};
    CrossSVec(ortho2, ortho, normalS);

    TVec3D mapInput{
        (ortho[0] * x + ortho[1] * y + ortho[2] * z) * scale,
        (ortho2[0] * x + ortho2[1] * y + ortho2[2] * z) * scale,
        0.0
    };
    TVec3D mapSample{};
    ext->PMapFunc2(&mapSample, &mapInput, mapNr);
    ext->Dfree1 = (mapSample[mapChannel & 3] + otrapOffset) * otrapScale;
}

// ============================================================================
// AexionC -- Aexion formula with C rotation
// Ported from formulas.pas line 5403 (Pascal comment block + asm)
//
// This formula uses spherical coordinates with a configurable power and
// optional rotation of the Julia constant (C).
//
// PVar layout (byte offsets from PVar):
//   PVar[-2] = power (double)        [edi - 16]
//   PVar[-3] = dZmul (double)        [edi - 24]
//   PVar[-4] (int32 at byte -28) = enable RotC (0 or 1)  [edi - 28]
//   PVar[-4] (double at byte -32) = conditional phi flag  [edi - 32]
//   PVar[-5] (double at byte -40) = power for C rotation  [edi - 40]
//   PVar[-6] (double at byte -48) = Cz mul for C rotation [edi - 48]
//   PVar[-7] (int32 at byte -52) = use distance instead of powerC [edi - 52]
//   PVar[-7] (int32 at byte -56) = modus bits              [edi - 56]
//   PVar[-1] = 0.5 (double)          [edi - 8]
//
// First part: standard spherical power formula
//   th = atan2(sqrt(xx+zz), y) * power
//   ph = atan2(z, x) * power
//   r1 = (xx+yy+zz)^(power*0.5)
//   x' = cos(th)*cos(ph)*r1 + J1
//   y' = cos(th)*sin(ph)*r1 + J2
//   z' = sin(th)*r1*dZmul + J3
//
// Second part (if RotC enabled): rotate J1,J2,J3 using similar spherical mapping
// ============================================================================

void AexionC(double& x, double& y, double& z, double& w, TPIteration3D PIteration3D) {
    (void)w;
    TIteration3Dext* ext = ExtFromIt3D(PIteration3D);
    double* PVar = PVarDoubles(PIteration3D->PVar);
    char* pvarBytes = reinterpret_cast<char*>(PIteration3D->PVar);

    double power = *(PVar - 2);
    double dZmul = *(PVar - 3);

    // Part 1: spherical power formula.
    // Pascal inline asm (formulas.pas, AexionC) computes xx+zz first, then
    // adds yy: r1 = yy + (xx+zz). Match the operation order exactly so the
    // ~2-ULP drift that shows up in Rout on border pixels disappears.
    double xx = x * x;
    double yy = y * y;
    double zz = z * z;
    double xxzz = xx + zz;
    double r1 = yy + xxzz;

    double th = Atan2_Pas(std::sqrt(xxzz), y) * power;
    double ph = Atan2_Pas(z, x) * power;

    r1 = Pow_Pas(r1, power * 0.5);

    double sinTh, cosTh, sinPh, cosPh;
    SinCosD(th, sinTh, cosTh);
    SinCosD(ph, sinPh, cosPh);

    x = cosTh * cosPh * r1 + PIteration3D->J1;
    y = cosTh * sinPh * r1 + PIteration3D->J2;
    z = sinTh * r1 * dZmul + PIteration3D->J3;

    // Part 2: rotate C (Julia constant) if enabled
    int32_t enableRotC;
    std::memcpy(&enableRotC, pvarBytes - 28, sizeof(int32_t));

    if (enableRotC == 0) {
        return;
    }

    // Get powerC
    double powerC;
    int32_t useDistance;
    std::memcpy(&useDistance, pvarBytes - 52, sizeof(int32_t));

    if (useDistance != 0) {
        // Use distance from current point to Julia constant as powerC
        double dx = x - PIteration3D->J1;
        double dy = y - PIteration3D->J2;
        double dz = z - PIteration3D->J3;
        powerC = std::sqrt(dx * dx + dy * dy + dz * dz);
    } else {
        std::memcpy(&powerC, pvarBytes - 40, sizeof(double));
    }

    // Compute r1 for C rotation
    r1 = std::sqrt(PIteration3D->J1 * PIteration3D->J1 +
                   PIteration3D->J2 * PIteration3D->J2 +
                   PIteration3D->J3 * PIteration3D->J3);

    // Get modus bits
    int32_t modus;
    std::memcpy(&modus, pvarBytes - 56, sizeof(int32_t));

    // Determine which values to use for atan2 calculations
    double Cx, Cy, Cz;
    bool useDiffs = (modus & 16) != 0;

    if (useDiffs) {
        Cx = x - PIteration3D->J1;
        Cz = z - PIteration3D->J3;
        Cy = y - PIteration3D->J2;
    } else {
        Cx = PIteration3D->J1;
        Cz = PIteration3D->J3;
        Cy = PIteration3D->J2;
    }

    double origCx = Cx;
    double origCy = Cy;
    double origCz = Cz;

    // Modus bit4: flip Cy<>Cx
    if (modus & 8) {
        std::swap(Cy, Cx);
    }

    // Compute th and ph for C rotation
    double sqrtXZ = std::sqrt(Cz * Cz + Cx * Cx);

    // Modus bit1: flip atan theta args
    if (modus & 1) {
        th = Atan2_Pas(Cy, sqrtXZ);
    } else {
        th = Atan2_Pas(sqrtXZ, Cy);
    }

    // Modus bit2: flip atan phi args
    if (modus & 2) {
        ph = Atan2_Pas(Cx, Cz);
    } else {
        ph = Atan2_Pas(Cz, Cx);
    }

    // Modus bit3: swap th and ph
    if (modus & 4) {
        std::swap(th, ph);
    }

    // Conditional phi uses a coordinate selected by the same modus-dependent
    // register permutation as the Pascal asm path.
    int32_t condPhi;
    std::memcpy(&condPhi, pvarBytes - 32, sizeof(int32_t));
    double condCoord = origCx;
    if ((modus & 4) != 0) {
        condCoord = ((modus & 8) != 0) ? origCy : origCz;
    } else if ((modus & 2) != 0) {
        condCoord = ((modus & 8) != 0) ? origCz : origCy;
    }
    if (condPhi != 0 && !std::signbit(condCoord)) {
        ph = -ph;
    }

    ph *= powerC;
    th *= powerC;

    SinCosD(th, sinTh, cosTh);
    SinCosD(ph, sinPh, cosPh);

    double CzMul;
    std::memcpy(&CzMul, pvarBytes - 48, sizeof(double));

    PIteration3D->J1 = cosTh * cosPh * r1;
    PIteration3D->J2 = cosTh * sinPh * r1;
    PIteration3D->J3 = sinTh * r1 * CzMul;
}

// ============================================================================
// CalcSmoothIterations -- smooth iteration count for coloring
// Executable Pascal assembly, including high-word branches and x87 cancellation.
// ============================================================================

void CalcSmoothIterations(TPIteration3D PIt3D, int n) {
    TIteration3Dext* pExt = ExtFromIt3D(PIt3D);
#if defined(_MSC_VER) && defined(_M_X64)
    struct SmoothState {
        double radius, oldRadius, half, epsilon;
        float logStop, inverseLogPower;
        int32_t iterations;
        float result;
    } state{pExt->Rout, pExt->Rold, .5, 1e-100,
            PIt3D->LNRStop, PIt3D->fHln[n], PIt3D->ItResultI, 0};
    static_assert(offsetof(SmoothState, result) == 44);
    CalcSmoothIterationsOriginal_x87(&state);
    PIt3D->SmoothItD = state.result;
#else
    if (pExt->Rout <= 1.0) {
        PIt3D->SmoothItD = static_cast<float>(PIt3D->ItResultI);
    } else if (pExt->Rold < 1.0) {
        double lnRout = std::log(0.5 * std::log(pExt->Rout));
        PIt3D->SmoothItD = static_cast<float>(
            PIt3D->ItResultI + PIt3D->LNRStop - lnRout * PIt3D->fHln[n]);
    } else {
        double d = std::log(0.5 * std::log(pExt->Rout));
        double dOld = std::log(0.5 * std::log(pExt->Rold));
        double denom = d - dOld + 1e-100;
        PIt3D->SmoothItD = static_cast<float>(
            PIt3D->ItResultI + (PIt3D->LNRStop - d) / denom);
    }
#endif
}

// ============================================================================
// Orbit termination follows Pascal repeat-until: a NaN radius is not greater
// than RStop, so it must keep iterating until maxIt. Do not rewrite !(Rout >
// RStop) as Rout <= RStop: they differ on unordered values.
// doHybridPas -- main hybrid iteration driver (non-interpolating)
// Ported from formulas.pas lines 3442-3481
// ============================================================================

void doHybridPas(TPIteration3D PIteration3D) {
    TIteration3Dext* ext = ExtFromIt3D(PIteration3D);
    FormulaIterationGuard guard("doHybridPas");
    guard.validateSlots(ext->iStartFrom, ext->iRepeatFrom, ext->EndTo);

    if (ext->DoJulia) {
        ext->J1 = ext->Ju1; ext->J2 = ext->Ju2; ext->J3 = ext->Ju3;
        ext->J4 = ext->Ju4;
    } else {
        ext->J1 = ext->C1; ext->J2 = ext->C2; ext->J3 = ext->C3;
    }

    ext->x = ext->C1;
    ext->y = ext->C2;
    ext->z = ext->C3;
    ext->w = 0;
    ext->VaryScale = 1.0;
    ext->Rout = ComputeHybridRoutPasLike(ext->x, ext->y, ext->z);
    ext->OTrap = ext->Rout;
    // FPC SSE2 doHybridPas: XMM0 holds x*x+y*y, XMM1 holds Rout.
    ext->FormulaSse01[0] = ext->x*ext->x + ext->y*ext->y;
    ext->FormulaSse01[1] = 0;
    ext->FormulaSse01[2] = ext->Rout;
    ext->FormulaSse01[3] = 0;

    int n = ext->iStartFrom;
    ext->bTmp = PIteration3D->nHybrid[n] & 0x7FFFFFFF;
    PIteration3D->PVar = PIteration3D->fHPVar[n];
    ext->bFirstIt = 0;
    PIteration3D->ItResultI = 0;

    int endTo = ext->EndTo;
    int repeatFrom = ext->iRepeatFrom;

    do {
        ext->Rold = ext->Rout;

        uint32_t visitedInactive = 0;
        while (ext->bTmp <= 0) {
            guard.visitInactiveSlot(n, visitedInactive, ext->ItResultI);
            n++;
            if (n > endTo) n = repeatFrom;
            ext->bTmp = PIteration3D->nHybrid[n] & 0x7FFFFFFF;
            if (ext->bTmp > 0) PIteration3D->PVar = PIteration3D->fHPVar[n];
            if (ext->bTmp <= 0 && n == ext->iStartFrom) {
                // formulas.pas:3465-3469 explicitly terminates as inside on
                // returning to a disabled start slot, including all-disabled
                // counts. This is a finite exit, not an empty selection cycle.
                // A repeated suffix that excludes iStartFrom still reaches
                // visitInactiveSlot's cycle error above.
                PIteration3D->ItResultI = PIteration3D->maxIt;
                return; // Pascal also skips CalcSmoothIterations here.
            }
        }
        guard.beforeFormula(ext->ItResultI, n);

        if (FormulaTraceActive()) {
            TraceFormulaState("formula_pre", n, PIteration3D->ItResultI, ext->bTmp,
                              ext->x, ext->y, ext->z, ext->w,
                              ext->Rout, ext->OTrap, true);
        }
        TraceFormulaContractState("formula_contract_in", n, PIteration3D,
                                  ext->x, ext->y, ext->z, ext->w);
        double x = ext->x;
        double y = ext->y;
        double z = ext->z;
        double w = ext->w;
        PIteration3D->fHybrid[n](x, y, z, w, PIteration3D);
        ext->x = x;
        ext->y = y;
        ext->z = z;
        ext->w = w;
        TraceFormulaContractState("formula_contract_out", n, PIteration3D,
                                  ext->x, ext->y, ext->z, ext->w);
        if (FormulaTraceActive()) {
            TraceFormulaState("formula_post", n, PIteration3D->ItResultI, 0,
                              ext->x, ext->y, ext->z, ext->w,
                              ext->Rout, ext->OTrap, false);
        }
        if (DebugOrbitBarrierEnabled()) {
            DebugOrbitBarrier(ext->x, ext->y, ext->z, ext->w, ext->Rout, ext->OTrap);
        }
        ext->bTmp--;

        if (PIteration3D->nHybrid[n] < 0) {
            // CVTSS2SD changes only the low lane; no radius calculation here.
            ext->FormulaSse01[0] = PIteration3D->RStop;
            continue;
        } else {
            PIteration3D->ItResultI++;
            ext->Rout = ComputeHybridRoutPasLike(ext->x, ext->y, ext->z);
            if (ext->Rout < ext->OTrap) ext->OTrap = ext->Rout;
            // Radius MOVSD loads clear both upper lanes; the loop test then
            // overwrites XMM0.low with RStop. Manowar consumes these lanes.
            ext->FormulaSse01[0] = PIteration3D->RStop;
            ext->FormulaSse01[1] = 0;
            ext->FormulaSse01[2] = ext->Rout;
            ext->FormulaSse01[3] = 0;
        }
    } while (PIteration3D->ItResultI < PIteration3D->maxIt &&
             !(ext->Rout > PIteration3D->RStop));

    if (ext->CalcSIT) {
        CalcSmoothIterations(PIteration3D, n);
    }
}

// ============================================================================
// doHybridPasDE -- hybrid iteration with DE return
// Ported from formulas.pas lines 3697-3747
//
// Handles the full alternating hybrid chain with DE calculation.
// Initializes w based on DEoption bits:
//   (DEoption & 0x38) == 16: w = Rout (for some DE modes)
//   (DEoption & 0x38) == 32: Deriv1=1, Deriv2=0, Deriv3=0 (analytic gradient)
//   else: w = 1 (standard DE tracking)
//
// DE calculation after iteration:
//   (DEoption & 0x38) == 32: sqrt(Rout)*0.5*ln(Rout)/Deriv1
//   (DEoption & 7) == 4: abs(y)*ln(abs(y))/w  (Julia mode)
//   (DEoption & 7) == 7: sqrt(Rout/RStop)*power(PVar[-2], -ItResultI)
//   else: sqrt(Rout)/abs(w)  (AmazingBox + IFS)
// ============================================================================

double doHybridPasDE(TPIteration3D PIteration3D) {
    TIteration3Dext* ext = ExtFromIt3D(PIteration3D);
    FormulaIterationGuard guard("doHybridPasDE");
    guard.validateSlots(ext->iStartFrom, ext->iRepeatFrom, ext->EndTo);

    if (ext->DoJulia) {
        ext->J1 = ext->Ju1; ext->J2 = ext->Ju2; ext->J3 = ext->Ju3;
        ext->J4 = ext->Ju4;
    } else {
        ext->J1 = ext->C1; ext->J2 = ext->C2; ext->J3 = ext->C3;
    }

    ext->x = ext->C1;
    ext->y = ext->C2;
    ext->z = ext->C3;
    ext->VaryScale = 1.0;
    ext->Rout = ComputeHybridRoutPasLike(ext->x, ext->y, ext->z);
    ext->OTrap = ext->Rout;

    int n = ext->iStartFrom;
    ext->bTmp = ext->nHybrid[n] & 0x7FFFFFFF;
    PIteration3D->PVar = PIteration3D->fHPVar[n];

    // Initialize w and Deriv based on DEoption
    int deOpt38 = ext->DEoption & 0x38;
    if (deOpt38 == 16) {
        ext->w = ext->Rout;
    } else if (deOpt38 == 32) {
        ext->Deriv1 = 1;
        ext->Deriv2 = 0;
        ext->Deriv3 = 0;
    } else {
        ext->w = 1;
    }

    ext->bFirstIt = 0;
    ext->ItResultI = 0;

    int endTo = ext->EndTo;
    int repeatFrom = ext->iRepeatFrom;

    do {
        ext->Rold = ext->Rout;

        uint32_t visitedInactive = 0;
        while (ext->bTmp <= 0) {
            guard.visitInactiveSlot(n, visitedInactive, ext->ItResultI);
            n++;
            if (n > endTo) n = repeatFrom;
            ext->bTmp = ext->nHybrid[n] & 0x7FFFFFFF;
            PIteration3D->PVar = PIteration3D->fHPVar[n];
        }
        guard.beforeFormula(ext->ItResultI, n);

        if (FormulaTraceActive()) {
            TraceFormulaState("formula_pre", n, ext->ItResultI, ext->bTmp,
                              ext->x, ext->y, ext->z, ext->w,
                              ext->Rout, ext->OTrap, true);
        }
        TraceFormulaContractState("formula_contract_in", n, PIteration3D,
                                  ext->x, ext->y, ext->z, ext->w);
        double x = ext->x;
        double y = ext->y;
        double z = ext->z;
        double w = ext->w;
        PIteration3D->fHybrid[n](x, y, z, w, PIteration3D);
        ext->x = x;
        ext->y = y;
        ext->z = z;
        ext->w = w;
        TraceFormulaContractState("formula_contract_out", n, PIteration3D,
                                  ext->x, ext->y, ext->z, ext->w);
        if (FormulaTraceActive()) {
            TraceFormulaState("formula_post", n, ext->ItResultI, 0,
                              ext->x, ext->y, ext->z, ext->w,
                              ext->Rout, ext->OTrap, false);
        }
        if (DebugOrbitBarrierEnabled()) {
            DebugOrbitBarrier(ext->x, ext->y, ext->z, ext->w, ext->Rout, ext->OTrap);
        }
        ext->bTmp--;

        if (ext->nHybrid[n] >= 0) {
            ext->ItResultI++;
            ext->Rout = ComputeHybridRoutPasLike(ext->x, ext->y, ext->z);
            if (ext->Rout < ext->OTrap) ext->OTrap = ext->Rout;
        }
    } while (ext->ItResultI < ext->maxIt && !(ext->Rout > PIteration3D->RStop));

    // Calculate DE result
    double result;
    if ((ext->DEoption & 0x38) == 32) {
        // Analytic gradient DE: sqrt(Rout) * 0.5 * ln(Rout) / Deriv1
        result = std::sqrt(ext->Rout) * 0.5 * std::log(ext->Rout) / ext->Deriv1;
    } else {
        switch (ext->DEoption & 7) {
            case 4:
                // Julia DE mode: abs(y) * ln(abs(y)) / w
                result = std::fabs(ext->y) * std::log(std::fabs(ext->y)) / ext->w;
                break;
            case 7: {
                // Power-based DE: sqrt(Rout/RStop) * pow(PVar[-2], -ItResultI)
                double* PVar = PVarDoubles(PIteration3D->PVar);
                double base = *(PVar - 2);
                result = std::sqrt(ext->Rout / PIteration3D->RStop) *
                         std::pow(std::fabs(base), -static_cast<double>(ext->ItResultI));
                break;
            }
            default:
                // Standard AmazingBox + IFS DE: sqrt(Rout) / abs(w)
                result = std::sqrt(ext->Rout) / std::fabs(ext->w);
                break;
        }
    }

    if (ext->CalcSIT) {
        CalcSmoothIterations(PIteration3D, n);
    }

    return result;
}

// ============================================================================
// doHybrid4DPas -- 4D hybrid iteration (no DE return)
// Ported from formulas.pas lines 3540-3585
//
// Same as doHybridPas but includes w (4th dimension) in Rout calculation:
//   Rout = x*x + y*y + z*z + w*w
// Applies 4D rotation matrix (SMatrix4) before iteration.
// ============================================================================

void doHybrid4DPas(TPIteration3D PIteration3D) {
    TIteration3Dext* ext = ExtFromIt3D(PIteration3D);
    FormulaIterationGuard guard("doHybrid4DPas");
    guard.validateSlots(ext->iStartFrom, ext->iRepeatFrom, ext->EndTo);

    // 4D rotation: Rotate4Dex(@C1, @x, @SMatrix4)
    // Apply SMatrix4 to C1..C3 -> x,y,z,w
    // Apply 4D rotation: Rotate4Dex(@C1, @x, @SMatrix4)
    // Full 4D matrix multiply on C1..C3 -> x,y,z,w
    ext->x = ext->C1;
    ext->y = ext->C2;
    ext->z = ext->C3;
    // Apply 4D rotation using SMatrix4
    {
        double ix = ext->C1, iy = ext->C2, iz = ext->C3;
        double iw = 0; // initial w = 0 before rotation
        TSMatrix4& m = ext->SMatrix4;
        // Pascal Rotate4Dex: Vout[i] = Vin[0]*SM4[i,0] + Vin[1]*SM4[i,1] + Vin[2]*SM4[i,2] + Vin[3]*SM4[i,3]
        ext->x = ix * m[0][0] + iy * m[0][1] + iz * m[0][2] + iw * m[0][3];
        ext->y = ix * m[1][0] + iy * m[1][1] + iz * m[1][2] + iw * m[1][3];
        ext->z = ix * m[2][0] + iy * m[2][1] + iz * m[2][2] + iw * m[2][3];
        ext->w = ix * m[3][0] + iy * m[3][1] + iz * m[3][2] + iw * m[3][3];
    }

    if (ext->DoJulia) {
        ext->J1 = ext->Ju1; ext->J2 = ext->Ju2; ext->J3 = ext->Ju3;
        ext->J4 = ext->Ju4;
    } else {
        ext->J1 = ext->x; ext->J2 = ext->y; ext->J3 = ext->z;
        ext->J4 = ext->w;
    }

    ext->Rout = ext->x * ext->x + ext->y * ext->y + ext->z * ext->z + ext->w * ext->w;
    ext->OTrap = ext->Rout;

    int n = ext->iStartFrom;
    ext->bTmp = ext->nHybrid[n] & 0x7FFFFFFF;
    PIteration3D->PVar = PIteration3D->fHPVar[n];
    ext->bFirstIt = 0;
    ext->ItResultI = 0;

    int endTo = ext->EndTo;
    int repeatFrom = ext->iRepeatFrom;

    do {
        ext->Rold = ext->Rout;

        uint32_t visitedInactive = 0;
        while (ext->bTmp <= 0) {
            guard.visitInactiveSlot(n, visitedInactive, ext->ItResultI);
            n++;
            if (n > endTo) n = repeatFrom;
            ext->bTmp = ext->nHybrid[n] & 0x7FFFFFFF;
            if (ext->bTmp > 0) PIteration3D->PVar = PIteration3D->fHPVar[n];
        }
        guard.beforeFormula(ext->ItResultI, n);

        TraceFormulaState("formula_pre", n, ext->ItResultI, ext->bTmp,
                          ext->x, ext->y, ext->z, ext->w, ext->Rout, ext->OTrap, true);
        PIteration3D->fHybrid[n](ext->x, ext->y, ext->z, ext->w, PIteration3D);
        TraceFormulaState("formula_post", n, ext->ItResultI, 0,
                          ext->x, ext->y, ext->z, ext->w, ext->Rout, ext->OTrap, false);
        ext->bTmp--;

        if (ext->nHybrid[n] >= 0) {
            ext->ItResultI++;
            ext->Rout = ext->x * ext->x + ext->y * ext->y + ext->z * ext->z + ext->w * ext->w;
            if (ext->Rout < ext->OTrap) ext->OTrap = ext->Rout;
        }
    } while (ext->ItResultI < ext->maxIt && !(ext->Rout > PIteration3D->RStop));

    if (ext->CalcSIT) {
        CalcSmoothIterations(PIteration3D, n);
    }
}

// ============================================================================
// doHybrid4DDEPas -- 4D hybrid iteration with DE return
// Ported from formulas.pas lines 3483-3537
//
// Same structure as doHybrid4DPas but with DE calculation.
// DE result uses Deriv1 instead of w for analytic mode:
//   case 4: abs(z)*ln(abs(z))/Deriv1
//   case 7: sqrt(Rout/RStop)*pow(PVar[-2], -ItResultI)
//   else:   sqrt(Rout)/abs(Deriv1)
// ============================================================================

double doHybrid4DDEPas(TPIteration3D PIteration3D) {
    TIteration3Dext* ext = ExtFromIt3D(PIteration3D);
    FormulaIterationGuard guard("doHybrid4DDEPas");
    guard.validateSlots(ext->iStartFrom, ext->iRepeatFrom, ext->EndTo);

    // 4D rotation
    ext->x = ext->C1;
    ext->y = ext->C2;
    ext->z = ext->C3;
    {
        double ix = ext->C1, iy = ext->C2, iz = ext->C3;
        double iw = 0;
        TSMatrix4& m = ext->SMatrix4;
        // Pascal Rotate4Dex: Vout[i] = Vin[0]*SM4[i,0] + Vin[1]*SM4[i,1] + Vin[2]*SM4[i,2] + Vin[3]*SM4[i,3]
        ext->x = ix * m[0][0] + iy * m[0][1] + iz * m[0][2] + iw * m[0][3];
        ext->y = ix * m[1][0] + iy * m[1][1] + iz * m[1][2] + iw * m[1][3];
        ext->z = ix * m[2][0] + iy * m[2][1] + iz * m[2][2] + iw * m[2][3];
        ext->w = ix * m[3][0] + iy * m[3][1] + iz * m[3][2] + iw * m[3][3];
    }

    if (ext->DoJulia) {
        ext->J1 = ext->Ju1; ext->J2 = ext->Ju2; ext->J3 = ext->Ju3;
        ext->J4 = ext->Ju4;
    } else {
        ext->J1 = ext->x; ext->J2 = ext->y; ext->J3 = ext->z;
        ext->J4 = ext->w;
    }

    ext->Rout = ext->x * ext->x + ext->y * ext->y + ext->z * ext->z + ext->w * ext->w;

    // Initialize DE tracking based on DEoption
    switch (ext->DEoption & 0x38) {
        case 16: ext->Deriv1 = ext->Rout; break;
        case 32: ext->Deriv1 = 1; ext->Deriv2 = 0; ext->Deriv3 = 0; break;
        default: ext->Deriv1 = 1; break;
    }

    ext->OTrap = ext->Rout;
    int n = ext->iStartFrom;
    ext->bTmp = ext->nHybrid[n] & 0x7FFFFFFF;
    PIteration3D->PVar = PIteration3D->fHPVar[n];
    ext->bFirstIt = 0;
    ext->ItResultI = 0;

    int endTo = ext->EndTo;
    int repeatFrom = ext->iRepeatFrom;

    do {
        ext->Rold = ext->Rout;

        uint32_t visitedInactive = 0;
        while (ext->bTmp <= 0) {
            guard.visitInactiveSlot(n, visitedInactive, ext->ItResultI);
            n++;
            if (n > endTo) n = repeatFrom;
            ext->bTmp = ext->nHybrid[n] & 0x7FFFFFFF;
            if (ext->bTmp > 0) PIteration3D->PVar = PIteration3D->fHPVar[n];
        }
        guard.beforeFormula(ext->ItResultI, n);

        TraceFormulaState("formula_pre", n, ext->ItResultI, ext->bTmp,
                          ext->x, ext->y, ext->z, ext->w, ext->Rout, ext->OTrap, true);
        PIteration3D->fHybrid[n](ext->x, ext->y, ext->z, ext->w, PIteration3D);
        TraceFormulaState("formula_post", n, ext->ItResultI, 0,
                          ext->x, ext->y, ext->z, ext->w, ext->Rout, ext->OTrap, false);
        ext->bTmp--;

        if (ext->nHybrid[n] >= 0) {
            ext->ItResultI++;
            ext->Rout = ext->x * ext->x + ext->y * ext->y + ext->z * ext->z + ext->w * ext->w;
            if (ext->Rout < ext->OTrap) ext->OTrap = ext->Rout;
        }
    } while (ext->ItResultI < ext->maxIt && !(ext->Rout > PIteration3D->RStop));

    // Calculate DE result
    double result;
    switch (ext->DEoption & 7) {
        case 4:
            result = std::fabs(ext->z) * std::log(std::fabs(ext->z)) / ext->Deriv1;
            break;
        case 7: {
            double* PVar = PVarDoubles(PIteration3D->PVar);
            double base = *(PVar - 2);
            result = std::sqrt(ext->Rout / PIteration3D->RStop) *
                     std::pow(std::fabs(base), -static_cast<double>(ext->ItResultI));
            break;
        }
        default:
            // AmBox4D + IFS4D
            result = std::sqrt(ext->Rout) / std::fabs(ext->Deriv1);
            break;
    }

    if (ext->CalcSIT) {
        CalcSmoothIterations(PIteration3D, n);
    }

    return result;
}

// ============================================================================
// doInterpolHybridPas -- interpolating hybrid iteration (non-DE)
// Ported from formulas.pas lines 2606-2648
//
// Interpolates between formulas [0] and [1] each iteration:
//   1. Save current position as Y1
//   2. Run formula[0] on position, save result as X1
//   3. Restore position to Y1, run formula[1]
//   4. Weighted blend: pos = X1*S1 + pos*S2  (S1,S2 are normalized weights)
//   5. Scale to weighted radius: XX = |X1|*S1 + |pos|*S2
//   6. Normalize blended position to radius XX
//
// Weights S1,S2 are stored as floats reinterpreted from nHybrid[0], nHybrid[1]
// ============================================================================

// Original mCopyVec4=CopyVec4SSE2 leaves the copied position in XMM0/1.
// ManowarBulb consumes those caller lanes, including in zero-weight slots.
// Keep this state explicit in every interpolating driver; it is independent
// of the host compiler's register allocation and of diagnostic callbacks.
static inline void SetInterpolatedFormulaCallerLanes(TIteration3Dext* ext) {
    ext->FormulaSse01[0] = ext->x;
    ext->FormulaSse01[1] = ext->y;
    ext->FormulaSse01[2] = ext->z;
    ext->FormulaSse01[3] = ext->w;
}

void doInterpolHybridPas(TPIteration3D PIteration3D) {
    TIteration3Dext* ext = ExtFromIt3D(PIteration3D);
    FormulaIterationGuard guard("doInterpolHybridPas");

    if (ext->DoJulia) {
        ext->J1 = ext->Ju1; ext->J2 = ext->Ju2; ext->J3 = ext->Ju3;
        ext->J4 = ext->Ju4;
    } else {
        ext->J1 = ext->C1; ext->J2 = ext->C2; ext->J3 = ext->C3;
    }

    ext->x = ext->C1;
    ext->y = ext->C2;
    ext->z = ext->C3;
    ext->w = 0;
    ext->VaryScale = 1.0;

    ext->Rout = ext->x * ext->x + ext->y * ext->y + ext->z * ext->z;
    ext->OTrap = ext->Rout;
    ext->bFirstIt = 0;
    ext->ItResultI = 0;

    // Weights are stored as Single (float) reinterpreted from nHybrid int
    float S1, S2;
    std::memcpy(&S1, &ext->nHybrid[0], sizeof(float));
    std::memcpy(&S2, &ext->nHybrid[1], sizeof(float));

    do {
        ext->Rold = ext->Rout;

        // Save position as Y1
        double Y1x = ext->x, Y1y = ext->y, Y1z = ext->z, Y1w = ext->w;

        // Run formula[0]
        PIteration3D->PVar = PIteration3D->fHPVar[0];
        guard.beforeFormula(ext->ItResultI, 0);
        TraceFormulaState("formula_pre", 0, ext->ItResultI, 1,
                          ext->x, ext->y, ext->z, ext->w, ext->Rout, ext->OTrap, true);
        SetInterpolatedFormulaCallerLanes(ext);
        PIteration3D->fHybrid[0](ext->x, ext->y, ext->z, ext->w, PIteration3D);
        TraceFormulaState("formula_post", 0, ext->ItResultI, 0,
                          ext->x, ext->y, ext->z, ext->w, ext->Rout, ext->OTrap, false);

        // Save formula[0] result as X1
        double X1x = ext->x, X1y = ext->y, X1z = ext->z, X1w = ext->w;

        // Restore position to Y1
        ext->x = Y1x; ext->y = Y1y; ext->z = Y1z; ext->w = Y1w;

        // Run formula[1]
        PIteration3D->PVar = PIteration3D->fHPVar[1];
        guard.beforeFormula(ext->ItResultI, 1);
        TraceFormulaState("formula_pre", 1, ext->ItResultI, 1,
                          ext->x, ext->y, ext->z, ext->w, ext->Rout, ext->OTrap, true);
        SetInterpolatedFormulaCallerLanes(ext);
        PIteration3D->fHybrid[1](ext->x, ext->y, ext->z, ext->w, PIteration3D);
        TraceFormulaState("formula_post", 1, ext->ItResultI, 0,
                          ext->x, ext->y, ext->z, ext->w, ext->Rout, ext->OTrap, false);

        // Compute radii
        double XX = std::sqrt(X1x * X1x + X1y * X1y + X1z * X1z);
        double YY = std::sqrt(ext->x * ext->x + ext->y * ext->y + ext->z * ext->z);

        // Weighted target radius
        XX = XX * S1 + YY * S2;

        // Weighted blend of positions
        ext->x = X1x * S1 + ext->x * S2;
        ext->y = X1y * S1 + ext->y * S2;
        ext->z = X1z * S1 + ext->z * S2;
        ext->w = X1w * S1 + ext->w * S2;

        // Normalize blended position to target radius
        YY = XX / std::sqrt(ext->x * ext->x + ext->y * ext->y + ext->z * ext->z + 1e-40);
        ext->x *= YY;
        ext->y *= YY;
        ext->z *= YY;

        ext->ItResultI++;
        ext->Rout = XX * XX;
        if (ext->Rout < ext->OTrap) ext->OTrap = ext->Rout;
    } while (ext->ItResultI < ext->maxIt && !(ext->Rout > PIteration3D->RStop));

    if (ext->CalcSIT) {
        CalcSmoothIterations(PIteration3D, 0);
    }
}

// ============================================================================
// doInterpolHybridPasDE -- interpolating hybrid with DE return
// Ported from formulas.pas lines 2815-2865
//
// Same as doInterpolHybridPas but:
//   - Initializes w based on DEoption (for DE tracking)
//   - Uses abs(w) for DE blending: w = abs(X1w)*S1 + abs(w)*S2
//   - Returns DE based on DEoption mode
// ============================================================================

double doInterpolHybridPasDE(TPIteration3D PIteration3D) {
    TIteration3Dext* ext = ExtFromIt3D(PIteration3D);
    FormulaIterationGuard guard("doInterpolHybridPasDE");

    if (ext->DoJulia) {
        ext->J1 = ext->Ju1; ext->J2 = ext->Ju2; ext->J3 = ext->Ju3;
        ext->J4 = ext->Ju4;
    } else {
        ext->J1 = ext->C1; ext->J2 = ext->C2; ext->J3 = ext->C3;
    }

    ext->x = ext->C1;
    ext->y = ext->C2;
    ext->z = ext->C3;
    ext->VaryScale = 1.0;

    ext->Rout = ext->x * ext->x + ext->y * ext->y + ext->z * ext->z;
    ext->OTrap = ext->Rout;

    // Initialize w based on DEoption
    if ((ext->DEoption & 0x18) == 16) {
        ext->w = ext->Rout;
    } else {
        ext->w = 1;
    }

    float S1, S2;
    std::memcpy(&S1, &ext->nHybrid[0], sizeof(float));
    std::memcpy(&S2, &ext->nHybrid[1], sizeof(float));

    ext->bFirstIt = 0;
    ext->ItResultI = 0;

    do {
        ext->Rold = ext->Rout;

        double Y1x = ext->x, Y1y = ext->y, Y1z = ext->z, Y1w = ext->w;

        PIteration3D->PVar = PIteration3D->fHPVar[0];
        guard.beforeFormula(ext->ItResultI, 0);
        TraceFormulaState("formula_pre", 0, ext->ItResultI, 1,
                          ext->x, ext->y, ext->z, ext->w, ext->Rout, ext->OTrap, true);
        SetInterpolatedFormulaCallerLanes(ext);
        PIteration3D->fHybrid[0](ext->x, ext->y, ext->z, ext->w, PIteration3D);
        TraceFormulaState("formula_post", 0, ext->ItResultI, 0,
                          ext->x, ext->y, ext->z, ext->w, ext->Rout, ext->OTrap, false);

        double X1x = ext->x, X1y = ext->y, X1z = ext->z, X1w = ext->w;

        ext->x = Y1x; ext->y = Y1y; ext->z = Y1z; ext->w = Y1w;

        PIteration3D->PVar = PIteration3D->fHPVar[1];
        guard.beforeFormula(ext->ItResultI, 1);
        TraceFormulaState("formula_pre", 1, ext->ItResultI, 1,
                          ext->x, ext->y, ext->z, ext->w, ext->Rout, ext->OTrap, true);
        SetInterpolatedFormulaCallerLanes(ext);
        PIteration3D->fHybrid[1](ext->x, ext->y, ext->z, ext->w, PIteration3D);
        TraceFormulaState("formula_post", 1, ext->ItResultI, 0,
                          ext->x, ext->y, ext->z, ext->w, ext->Rout, ext->OTrap, false);

        double XX = std::sqrt(X1x * X1x + X1y * X1y + X1z * X1z);
        double YY = std::sqrt(ext->x * ext->x + ext->y * ext->y + ext->z * ext->z);
        XX = XX * S1 + YY * S2;

        ext->x = X1x * S1 + ext->x * S2;
        ext->y = X1y * S1 + ext->y * S2;
        ext->z = X1z * S1 + ext->z * S2;
        // DE tracking: use abs values for w blending
        ext->w = std::fabs(X1w) * S1 + std::fabs(ext->w) * S2;

        YY = XX / std::sqrt(ext->x * ext->x + ext->y * ext->y + ext->z * ext->z + 1e-40);
        ext->x *= YY;
        ext->y *= YY;
        ext->z *= YY;

        ext->ItResultI++;
        ext->Rout = XX * XX;
        if (ext->Rout < ext->OTrap) ext->OTrap = ext->Rout;
    } while (ext->ItResultI < ext->maxIt && !(ext->Rout > PIteration3D->RStop));

    // Calculate DE result
    double result;
    switch (ext->DEoption & 7) {
        case 4:
            // abs(z)*ln(abs(z))/w
            result = std::fabs(ext->z) * std::log(std::fabs(ext->z)) / ext->w;
            break;
        case 7: {
            // sqrt(Rout/RStop) * pow(PVar[-2], -ItResultI)
            double* PVar = PVarDoubles(PIteration3D->PVar);
            double base = *(PVar - 2);
            result = std::sqrt(ext->Rout / PIteration3D->RStop) *
                     std::pow(std::fabs(base), -static_cast<double>(ext->ItResultI));
            break;
        }
        default:
            // sqrt(Rout)/abs(w) -- AmBox + IFS
            result = std::sqrt(ext->Rout) / std::fabs(ext->w);
            break;
    }

    if (ext->CalcSIT) {
        CalcSmoothIterations(PIteration3D, 0);
    }

    return result;
}

// ============================================================================
// doInterpolHybridPas4D -- 4D interpolating hybrid (no DE)
// Ported from formulas.pas lines 3091-3143
//
// Same as doInterpolHybridPas but includes w in all calculations:
//   XX = sqrt(X1x^2+X1y^2+X1z^2+X1w^2)
//   Rout = x^2+y^2+z^2+w^2
// Also applies 4D rotation via SMatrix4.
// ============================================================================

void doInterpolHybridPas4D(TPIteration3D PIteration3D) {
    TIteration3Dext* ext = ExtFromIt3D(PIteration3D);
    FormulaIterationGuard guard("doInterpolHybridPas4D");

    // 4D rotation
    {
        double ix = ext->C1, iy = ext->C2, iz = ext->C3;
        double iw = 0;
        TSMatrix4& m = ext->SMatrix4;
        // Pascal Rotate4Dex: Vout[i] = Vin[0]*SM4[i,0] + Vin[1]*SM4[i,1] + Vin[2]*SM4[i,2] + Vin[3]*SM4[i,3]
        ext->x = ix * m[0][0] + iy * m[0][1] + iz * m[0][2] + iw * m[0][3];
        ext->y = ix * m[1][0] + iy * m[1][1] + iz * m[1][2] + iw * m[1][3];
        ext->z = ix * m[2][0] + iy * m[2][1] + iz * m[2][2] + iw * m[2][3];
        ext->w = ix * m[3][0] + iy * m[3][1] + iz * m[3][2] + iw * m[3][3];
    }

    if (ext->DoJulia) {
        ext->J1 = ext->Ju1; ext->J2 = ext->Ju2; ext->J3 = ext->Ju3;
        ext->J4 = ext->Ju4;
    } else {
        ext->J1 = ext->x; ext->J2 = ext->y; ext->J3 = ext->z;
        ext->J4 = ext->w;
    }

    ext->Rout = ext->x * ext->x + ext->y * ext->y + ext->z * ext->z + ext->w * ext->w;
    ext->OTrap = ext->Rout;
    ext->bFirstIt = 0;
    ext->ItResultI = 0;

    float S1, S2;
    std::memcpy(&S1, &ext->nHybrid[0], sizeof(float));
    std::memcpy(&S2, &ext->nHybrid[1], sizeof(float));

    do {
        ext->Rold = ext->Rout;

        double Y1x = ext->x, Y1y = ext->y, Y1z = ext->z, Y1w = ext->w;

        PIteration3D->PVar = PIteration3D->fHPVar[0];
        guard.beforeFormula(ext->ItResultI, 0);
        TraceFormulaState("formula_pre", 0, ext->ItResultI, 1,
                          ext->x, ext->y, ext->z, ext->w, ext->Rout, ext->OTrap, true);
        SetInterpolatedFormulaCallerLanes(ext);
        PIteration3D->fHybrid[0](ext->x, ext->y, ext->z, ext->w, PIteration3D);
        TraceFormulaState("formula_post", 0, ext->ItResultI, 0,
                          ext->x, ext->y, ext->z, ext->w, ext->Rout, ext->OTrap, false);

        double X1x = ext->x, X1y = ext->y, X1z = ext->z, X1w = ext->w;

        ext->x = Y1x; ext->y = Y1y; ext->z = Y1z; ext->w = Y1w;

        PIteration3D->PVar = PIteration3D->fHPVar[1];
        guard.beforeFormula(ext->ItResultI, 1);
        TraceFormulaState("formula_pre", 1, ext->ItResultI, 1,
                          ext->x, ext->y, ext->z, ext->w, ext->Rout, ext->OTrap, true);
        SetInterpolatedFormulaCallerLanes(ext);
        PIteration3D->fHybrid[1](ext->x, ext->y, ext->z, ext->w, PIteration3D);
        TraceFormulaState("formula_post", 1, ext->ItResultI, 0,
                          ext->x, ext->y, ext->z, ext->w, ext->Rout, ext->OTrap, false);

        // 4D radius calculation
        double XX = std::sqrt(X1x * X1x + X1y * X1y + X1z * X1z + X1w * X1w);
        double YY = std::sqrt(ext->x * ext->x + ext->y * ext->y + ext->z * ext->z + ext->w * ext->w);
        XX = XX * S1 + YY * S2;

        ext->x = X1x * S1 + ext->x * S2;
        ext->y = X1y * S1 + ext->y * S2;
        ext->z = X1z * S1 + ext->z * S2;
        ext->w = X1w * S1 + ext->w * S2;

        // 4D normalization
        YY = XX / std::sqrt(ext->x * ext->x + ext->y * ext->y + ext->z * ext->z + ext->w * ext->w + 1e-40);
        ext->x *= YY;
        ext->y *= YY;
        ext->z *= YY;
        ext->w *= YY;

        ext->ItResultI++;
        ext->Rout = XX * XX;
        if (ext->Rout < ext->OTrap) ext->OTrap = ext->Rout;
    } while (ext->ItResultI < ext->maxIt && !(ext->Rout > PIteration3D->RStop));

    if (ext->CalcSIT) {
        CalcSmoothIterations(PIteration3D, 0);
    }
}

// ============================================================================
// doInterpolHybridPas4DDE -- 4D interpolating hybrid with DE return
// Ported from formulas.pas lines 3145-3211
//
// Combines doInterpolHybridPas4D + DE calculation using Deriv1.
// Saves/restores Deriv1 across formula calls for separate blending.
// ============================================================================

double doInterpolHybridPas4DDE(TPIteration3D PIteration3D) {
    TIteration3Dext* ext = ExtFromIt3D(PIteration3D);
    FormulaIterationGuard guard("doInterpolHybridPas4DDE");

    // 4D rotation
    {
        double ix = ext->C1, iy = ext->C2, iz = ext->C3;
        double iw = 0;
        TSMatrix4& m = ext->SMatrix4;
        // Pascal Rotate4Dex: Vout[i] = Vin[0]*SM4[i,0] + Vin[1]*SM4[i,1] + Vin[2]*SM4[i,2] + Vin[3]*SM4[i,3]
        ext->x = ix * m[0][0] + iy * m[0][1] + iz * m[0][2] + iw * m[0][3];
        ext->y = ix * m[1][0] + iy * m[1][1] + iz * m[1][2] + iw * m[1][3];
        ext->z = ix * m[2][0] + iy * m[2][1] + iz * m[2][2] + iw * m[2][3];
        ext->w = ix * m[3][0] + iy * m[3][1] + iz * m[3][2] + iw * m[3][3];
    }

    if (ext->DoJulia) {
        ext->J1 = ext->Ju1; ext->J2 = ext->Ju2; ext->J3 = ext->Ju3;
        ext->J4 = ext->Ju4;
    } else {
        ext->J1 = ext->x; ext->J2 = ext->y; ext->J3 = ext->z;
        ext->J4 = ext->w;
    }

    ext->Rout = ext->x * ext->x + ext->y * ext->y + ext->z * ext->z + ext->w * ext->w;
    ext->OTrap = ext->Rout;
    ext->bFirstIt = 0;
    ext->ItResultI = 0;

    // Initialize Deriv based on DEoption
    switch (ext->DEoption & 0x38) {
        case 16: ext->Deriv1 = ext->Rout; break;
        case 32: ext->Deriv1 = 1; ext->Deriv2 = 0; ext->Deriv3 = 0; break;
        default: ext->Deriv1 = 1; break;
    }

    float S1, S2;
    std::memcpy(&S1, &ext->nHybrid[0], sizeof(float));
    std::memcpy(&S2, &ext->nHybrid[1], sizeof(float));

    do {
        ext->Rold = ext->Rout;

        double Y1x = ext->x, Y1y = ext->y, Y1z = ext->z, Y1w = ext->w;
        double DT = ext->Deriv1;  // Save Deriv1 before formula[0]

        PIteration3D->PVar = PIteration3D->fHPVar[0];
        guard.beforeFormula(ext->ItResultI, 0);
        TraceFormulaState("formula_pre", 0, ext->ItResultI, 1,
                          ext->x, ext->y, ext->z, ext->w, ext->Rout, ext->OTrap, true);
        SetInterpolatedFormulaCallerLanes(ext);
        // FPC SSE2 loads saved Deriv1 after CopyVec4; MOVSD clears the high lane.
        ext->FormulaSse01[0] = DT;
        ext->FormulaSse01[1] = 0.0;
        PIteration3D->fHybrid[0](ext->x, ext->y, ext->z, ext->w, PIteration3D);
        TraceFormulaState("formula_post", 0, ext->ItResultI, 0,
                          ext->x, ext->y, ext->z, ext->w, ext->Rout, ext->OTrap, false);

        double X1x = ext->x, X1y = ext->y, X1z = ext->z, X1w = ext->w;
        double DD = ext->Deriv1;  // Deriv1 after formula[0]

        ext->x = Y1x; ext->y = Y1y; ext->z = Y1z; ext->w = Y1w;
        ext->Deriv1 = DT;  // Restore for formula[1]

        PIteration3D->PVar = PIteration3D->fHPVar[1];
        guard.beforeFormula(ext->ItResultI, 1);
        TraceFormulaState("formula_pre", 1, ext->ItResultI, 1,
                          ext->x, ext->y, ext->z, ext->w, ext->Rout, ext->OTrap, true);
        SetInterpolatedFormulaCallerLanes(ext);
        // FPC SSE2 loads saved Deriv1 after CopyVec4; MOVSD clears the high lane.
        ext->FormulaSse01[0] = DT;
        ext->FormulaSse01[1] = 0.0;
        PIteration3D->fHybrid[1](ext->x, ext->y, ext->z, ext->w, PIteration3D);
        TraceFormulaState("formula_post", 1, ext->ItResultI, 0,
                          ext->x, ext->y, ext->z, ext->w, ext->Rout, ext->OTrap, false);

        double XX = std::sqrt(X1x * X1x + X1y * X1y + X1z * X1z + X1w * X1w);
        double YY = std::sqrt(ext->x * ext->x + ext->y * ext->y + ext->z * ext->z + ext->w * ext->w);
        XX = XX * S1 + YY * S2;

        ext->x = X1x * S1 + ext->x * S2;
        ext->y = X1y * S1 + ext->y * S2;
        ext->z = X1z * S1 + ext->z * S2;
        ext->w = X1w * S1 + ext->w * S2;

        // Blend Deriv1
        ext->Deriv1 = std::fabs(DD) * S1 + std::fabs(ext->Deriv1) * S2;

        YY = XX / std::sqrt(ext->x * ext->x + ext->y * ext->y + ext->z * ext->z + ext->w * ext->w + 1e-40);
        ext->x *= YY;
        ext->y *= YY;
        ext->z *= YY;
        ext->w *= YY;

        ext->ItResultI++;
        ext->Rout = XX * XX;
        if (ext->Rout < ext->OTrap) ext->OTrap = ext->Rout;
    } while (ext->ItResultI < ext->maxIt && !(ext->Rout > PIteration3D->RStop));

    // Calculate DE result
    double result;
    switch (ext->DEoption & 7) {
        case 4:
            result = std::fabs(ext->z) * std::log(std::fabs(ext->z)) / ext->Deriv1;
            break;
        case 7: {
            double* PVar = PVarDoubles(PIteration3D->PVar);
            double base = *(PVar - 2);
            result = std::sqrt(ext->Rout / PIteration3D->RStop) *
                     std::pow(std::fabs(base), -static_cast<double>(ext->ItResultI));
            break;
        }
        default:
            result = std::sqrt(ext->Rout) / std::fabs(ext->Deriv1);
            break;
    }

    if (ext->CalcSIT) {
        CalcSmoothIterations(PIteration3D, 0);
    }

    return result;
}

// ============================================================================
// doHybridIFS3D -- IFS (Iterated Function System) mode
// Ported from formulas.pas lines 3213-3301 (asm)
//
// IFS mode tracks the minimum DE/VaryScale ratio across all iterations,
// rather than using bailout radius. The fractal surface is defined as
// the minimum distance estimate across the iteration orbit.
//
// Key differences from standard hybrid:
//   - VaryScale starts at 1.0 (formulas modify it as accumulated scale)
//   - OTrap holds minimum DE (Rout/VaryScale ratio)
//   - Dfree1/Dfree2 track color info at minDE iteration
//   - Iteration stops when minDE < RStopD (DEstop condition)
//   - ItResultI is set to the iteration that had minimum DE
//   - SmoothItD is set to the iteration count at minimum DE
// ============================================================================

double doHybridIFS3D(TPIteration3D PIteration3D) {
    TIteration3Dext* ext = ExtFromIt3D(PIteration3D);
    FormulaIterationGuard guard("doHybridIFS3D");
    guard.validateSlots(ext->iStartFrom, ext->iRepeatFrom, ext->EndTo);

    // Initialize position from C
    ext->x = ext->C1;
    ext->y = ext->C2;
    ext->z = ext->C3;
    ext->w = 0;  // Pascal zeroes w (stack-allocated struct), match that contract

    // Set J from Julia or C
    if (ext->DoJulia) {
        ext->J1 = ext->Ju1; ext->J2 = ext->Ju2; ext->J3 = ext->Ju3;
    } else {
        ext->J1 = ext->C1; ext->J2 = ext->C2; ext->J3 = ext->C3;
    }

    ext->bFirstIt = 0;
    ext->ItResultI = 0;
    ext->Rout = 0.0;

    // Save bIsInsideRender in SmoothItD slot temporarily
    int32_t savedInsideRender = ext->bIsInsideRender;

    int n = ext->iStartFrom;
    ext->Dfree1 = 0;       // color tracking
    // Pascal retains Dfree2 when no new minimum is found.
    ext->VaryScale = 1.0;  // accumulated absolute scale
    ext->OTrap = 65535.0;  // minimum DE (starts large)

    int bTmpLocal = ext->nHybrid[n] & 0x7FFFFFFF;
    PIteration3D->PVar = PIteration3D->fHPVar[n];

    int endTo = ext->EndTo;
    int repeatFrom = ext->iRepeatFrom;
    int itOnMinDE = ext->bTmp;

    do {
        // Advance formula chain
        uint32_t visitedInactive = 0;
        while (bTmpLocal <= 0) {
            guard.visitInactiveSlot(n, visitedInactive, ext->ItResultI);
            n++;
            if (n > endTo) n = repeatFrom;
            bTmpLocal = ext->nHybrid[n] & 0x7FFFFFFF;
            if (bTmpLocal > 0) PIteration3D->PVar = PIteration3D->fHPVar[n];
        }
        guard.beforeFormula(ext->ItResultI, n);

        // Call formula
        TraceFormulaState("formula_pre", n, ext->ItResultI, bTmpLocal,
                          ext->x, ext->y, ext->z, ext->w, ext->Rout, ext->OTrap, true);
        PIteration3D->fHybrid[n](ext->x, ext->y, ext->z, ext->w, PIteration3D);
        TraceFormulaState("formula_post", n, ext->ItResultI, 0,
                          ext->x, ext->y, ext->z, ext->w, ext->Rout, ext->OTrap, false);
        bTmpLocal--;

        // Skip iteration counting for negative nHybrid
        if (ext->nHybrid[n] < 0) continue;

        // Rout contains DE output from formula (set by formula)
        double DEout = ext->Rout / ext->VaryScale;
        ext->ItResultI++;
        TraceIFSPostFormulaState("ifs_post_formula", n, ext->ItResultI, DEout, ext);

        // Track minimum DE
        // UCOMISD/JNC skips ordered >= only; unordered also updates the minimum.
        if (!(DEout >= ext->OTrap)) {
            itOnMinDE = ext->ItResultI;
            ext->bTmp = itOnMinDE;
            ext->Dfree2 = ext->Dfree1;  // save color at minDE
            ext->OTrap = DEout;          // new minimum DE

            // NC9: Check stop condition for outside rendering
            if (savedInsideRender >= 0) { // Pascal JS skips negative inside flags.
                if (!(DEout >= ext->RStopD)) break;
            }
        }

        // Check max iterations
    } while (ext->ItResultI < ext->maxIt);

    // Set results
    // SmoothItD = iteration count at minimum DE
    ext->SmoothItD = static_cast<float>(itOnMinDE);
    ext->ItResultI = itOnMinDE;
    ext->bTmp = itOnMinDE;

    // OTrap gets the minDE value, Dfree2 stored color info swapped in
    // The result is the minimum DE value in OTrap
    double minDE = ext->OTrap;
    TraceIFSReturnState("ifs_return", n, minDE, ext);
    ext->OTrap = ext->Dfree2;  // swap: OTrap now holds color, minDE is returned

    return minDE;
}

// ============================================================================
// doHybridIFS3DnoVecIni -- IFS mode without vector initialization
// Ported from formulas.pas lines 3303-3380 (asm)
//
// Same as doHybridIFS3D but does NOT initialize x,y,z from C or set J.
// Used as the second formula group in DEcombination mode (FormulaType >= 6)
// where the first formula group has already run and set x,y,z.
// ============================================================================

double doHybridIFS3DnoVecIni(TPIteration3D PIteration3D) {
    TIteration3Dext* ext = ExtFromIt3D(PIteration3D);
    FormulaIterationGuard guard("doHybridIFS3DnoVecIni");
    guard.validateSlots(ext->iStartFrom, ext->iRepeatFrom, ext->EndTo);

    // Do NOT initialize position -- use existing x,y,z from prior iteration

    ext->bFirstIt = 0;
    ext->bTmp = 0;
    ext->ItResultI = 0;
    ext->Rout = 0.0;

    int32_t savedInsideRender = ext->bIsInsideRender;

    int n = ext->iStartFrom;
    ext->Dfree1 = 0;
    // Pascal retains the previous minimum color, even in noVecIni.
    ext->VaryScale = 1.0;
    ext->OTrap = 65535.0;

    int bTmpLocal = ext->nHybrid[n] & 0x7FFFFFFF;
    PIteration3D->PVar = PIteration3D->fHPVar[n];

    int endTo = ext->EndTo;
    int repeatFrom = ext->iRepeatFrom;
    int itOnMinDE = 0;

    do {
        uint32_t visitedInactive = 0;
        while (bTmpLocal <= 0) {
            guard.visitInactiveSlot(n, visitedInactive, ext->ItResultI);
            n++;
            if (n > endTo) n = repeatFrom;
            bTmpLocal = ext->nHybrid[n] & 0x7FFFFFFF;
            if (bTmpLocal > 0) PIteration3D->PVar = PIteration3D->fHPVar[n];
        }
        guard.beforeFormula(ext->ItResultI, n);

        TraceFormulaState("formula_pre", n, ext->ItResultI, bTmpLocal,
                          ext->x, ext->y, ext->z, ext->w, ext->Rout, ext->OTrap, true);
        PIteration3D->fHybrid[n](ext->x, ext->y, ext->z, ext->w, PIteration3D);
        TraceFormulaState("formula_post", n, ext->ItResultI, 0,
                          ext->x, ext->y, ext->z, ext->w, ext->Rout, ext->OTrap, false);
        bTmpLocal--;

        if (ext->nHybrid[n] < 0) continue;

        double DEout = ext->Rout / ext->VaryScale;
        ext->ItResultI++;
        TraceIFSPostFormulaState("ifs_post_formula", n, ext->ItResultI, DEout, ext);

        if (!(DEout >= ext->OTrap)) {
            itOnMinDE = ext->ItResultI;
            ext->bTmp = itOnMinDE;
            ext->Dfree2 = ext->Dfree1;
            ext->OTrap = DEout;

            if (savedInsideRender == 0) { // noVecIni uses JNE, unlike the JS above.
                if (!(DEout >= ext->RStopD)) break;
            }
        }
    } while (ext->ItResultI < ext->maxIt);

    ext->SmoothItD = static_cast<float>(itOnMinDE);
    ext->ItResultI = itOnMinDE;
    ext->bTmp = itOnMinDE;

    double minDE = ext->OTrap;
    TraceIFSReturnState("ifs_return", n, minDE, ext);
    ext->OTrap = ext->Dfree2;

    return minDE;
}

// ============================================================================
// Orbit trap functions (from OTrapDEcalc.pas)
// ============================================================================

// CalcOTrapMode3D -- compute orbit trap value based on mode
static double CalcOTrapMode3D(TPIteration3D PIteration3D) {
    TIteration3Dext* ext = reinterpret_cast<TIteration3Dext*>(
        reinterpret_cast<char*>(PIteration3D) - 7 * sizeof(double));
    switch (ext->OTrapMode) {
        case 1: return std::min(std::min(ext->x * ext->x + ext->y * ext->y,
                                          ext->x * ext->x + ext->z * ext->z),
                                ext->y * ext->y + ext->z * ext->z);
        case 2: return ext->x * ext->x;
        case 3: return ext->y * ext->y;
        case 4: return ext->z * ext->z;
        default: return ext->Rout;
    }
}

// do3DalternateOTrap -- alternating hybrid iteration with orbit trap tracking
void do3DalternateOTrap(TPIteration3D PIteration3D) {
    TIteration3Dext* ext = reinterpret_cast<TIteration3Dext*>(
        reinterpret_cast<char*>(PIteration3D) - 7 * sizeof(double));
    FormulaIterationGuard guard("do3DalternateOTrap");
    guard.validateSlots(ext->iStartFrom, ext->iRepeatFrom, ext->EndTo);

    ext->OTrapMode = 1;

    if (ext->DoJulia) {
        ext->J1 = ext->Ju1; ext->J2 = ext->Ju2; ext->J3 = ext->Ju3;
        ext->J4 = ext->Ju4;
    } else {
        ext->J1 = ext->C1; ext->J2 = ext->C2; ext->J3 = ext->C3;
    }

    ext->x = ext->C1;
    ext->y = ext->C2;
    ext->z = ext->C3;
    ext->w = 0;
    ext->VaryScale = 1.0;

    ext->Rout = ext->x * ext->x + ext->y * ext->y + ext->z * ext->z;
    ext->OTrap = ext->Rout;
    ext->OTrapDE = CalcOTrapMode3D(PIteration3D);

    int n = ext->iStartFrom;
    ext->bTmp = ext->nHybrid[n] & 0x7FFFFFFF;
    PIteration3D->PVar = PIteration3D->fHPVar[n];
    ext->bFirstIt = 0;
    ext->ItResultI = 0;

    int endTo = ext->EndTo;
    int repeatFrom = ext->iRepeatFrom;

    do {
        ext->Rold = ext->Rout;

        uint32_t visitedInactive = 0;
        while (ext->bTmp <= 0) {
            guard.visitInactiveSlot(n, visitedInactive, ext->ItResultI);
            n++;
            if (n > endTo) n = repeatFrom;
            ext->bTmp = ext->nHybrid[n] & 0x7FFFFFFF;
            if (ext->bTmp > 0) PIteration3D->PVar = PIteration3D->fHPVar[n];
        }
        guard.beforeFormula(ext->ItResultI, n);

        TraceFormulaState("formula_pre", n, ext->ItResultI, ext->bTmp,
                          ext->x, ext->y, ext->z, ext->w, ext->Rout, ext->OTrap, true);
        PIteration3D->fHybrid[n](ext->x, ext->y, ext->z, ext->w, PIteration3D);
        TraceFormulaState("formula_post", n, ext->ItResultI, 0,
                          ext->x, ext->y, ext->z, ext->w, ext->Rout, ext->OTrap, false);
        ext->bTmp--;

        if (ext->nHybrid[n] < 0) continue;

        ext->ItResultI++;
        ext->Rout = ext->x * ext->x + ext->y * ext->y + ext->z * ext->z;
        if (ext->Rout < ext->OTrap) ext->OTrap = ext->Rout;

        double otVal = CalcOTrapMode3D(PIteration3D);
        if (otVal < ext->OTrapDE) ext->OTrapDE = otVal;
    } while (ext->ItResultI < ext->maxIt && !(ext->Rout > PIteration3D->RStop));

    if (ext->CalcSIT) {
        CalcSmoothIterations(PIteration3D, n);
    }
}

// A boundary-search step may temporarily increment a supported header limit.
// Keep the orbit-trap outside sentinel explicit when that reaches INT_MAX.
static int32_t OTrapOutsideIterationSentinel(int32_t maxIt) {
    if (maxIt == std::numeric_limits<int32_t>::max()) {
        throw std::invalid_argument("Unsupported orbit-trap maximum iteration limit: "
                                    "INT_MAX leaves no room for the outside-orbit sentinel");
    }
    return maxIt + 1;
}

// CalcOTrapDE -- orbit trap DE (analytic-like)
double CalcOTrapDE(TPIteration3Dext It3Dex, void* mctp) {
    RenderWork();
    PMCTparameter mct = static_cast<PMCTparameter>(mctp);

    do3DalternateOTrap(reinterpret_cast<TPIteration3D>(&It3Dex->C1));

    double result = std::sqrt(It3Dex->OTrapDE) * mct->dDEscale;
    mct->MaxItsResult = OTrapOutsideIterationSentinel(It3Dex->maxIt);
    mct->DEoptionResult = It3Dex->DEoption;

    if (mct->bCalcInside) {
        result = mct->msDEstop * 4.0 - result * 3.0;
    }

    return result;
}

static void OTrapPrepareF1DEcomb(TBufDEcomb* DEcombRec, PMCTparameter mct) {
    *reinterpret_cast<uint32_t*>(&mct->pIt3Dext->iRepeatFrom) =
        *reinterpret_cast<uint32_t*>(&mct->RepeatFrom2);
    mct->pIt3Dext->EndTo = mct->iEnd2;

    DEcombRec->bufMCTMandFunctionDE = mct->mMandFunctionDE;
    mct->mMandFunctionDE = mct->mMandFunctionDE2;

    DEcombRec->bufIt3DItResultI = mct->pIt3Dext->ItResultI;
    DEcombRec->bufIt3DOtrap = mct->pIt3Dext->OTrap;
    DEcombRec->bufIt3DSmoothItD = mct->pIt3Dext->SmoothItD;
    DEcombRec->bufMCTdDEscale = mct->dDEscale;

    mct->dDEscale = mct->dDEscale2;
    mct->bCalcInside = 0;
    mct->pIt3Dext->maxIt = mct->iMaxitF2;
}

static void OTrapRestoreF1DEcomb(TBufDEcomb* DEcombRec, PMCTparameter mct) {
    mct->pIt3Dext->maxIt = mct->iMaxIt;
    *reinterpret_cast<uint32_t*>(&mct->pIt3Dext->iRepeatFrom) =
        *reinterpret_cast<uint32_t*>(&mct->RepeatFrom1);
    mct->pIt3Dext->EndTo = mct->wEndTo;

    mct->dDEscale = DEcombRec->bufMCTdDEscale;
    mct->bCalcInside = mct->bInsideRendering;
    mct->mMandFunctionDE = DEcombRec->bufMCTMandFunctionDE;
}

// CalcOTrapDEfull -- orbit trap DE full (with DEcomb support)
double CalcOTrapDEfull(TPIteration3Dext It3Dex, void* mctp) {
    RenderWork();
    PMCTparameter mct = static_cast<PMCTparameter>(mctp);
    TBufDEcomb DEcombRec{};
    double wt = 0.0;
    double dt = 0.0;
    double RD1 = 0.0;
    double Rst = It3Dex->C1 * It3Dex->C1 + It3Dex->C2 * It3Dex->C2 + It3Dex->C3 * It3Dex->C3;
    do3DalternateOTrap(reinterpret_cast<TPIteration3D>(&It3Dex->C1));

    double result = std::sqrt(It3Dex->OTrapDE * Rst / (It3Dex->Rout + 1e-100)) * mct->dDEscale;
    mct->MaxItsResult = OTrapOutsideIterationSentinel(mct->iMaxIt);
    mct->DEoptionResult = It3Dex->DEoption;

    if (mct->FormulaType > 0) {
        OTrapPrepareF1DEcomb(&DEcombRec, mct);
        if (mct->FormulaType == 3) {
            mct->bCalcInside = !mct->bCalcInside;
        }
        Rst = CalcOTrapDE(It3Dex, mctp);
        switch (mct->FormulaType) {
            case 1:
                if (Rst < result) {
                    result = Rst;
                } else {
                    It3Dex->ItResultI = DEcombRec.bufIt3DItResultI;
                    It3Dex->SmoothItD = DEcombRec.bufIt3DSmoothItD;
                    It3Dex->OTrap = DEcombRec.bufIt3DOtrap;
                    mct->DEoptionResult = mct->DEoption;
                    mct->MaxItsResult = OTrapOutsideIterationSentinel(mct->iMaxIt);
                }
                break;
            case 2:
            case 3:
                if (Rst > result) {
                    result = Rst;
                } else {
                    It3Dex->ItResultI = DEcombRec.bufIt3DItResultI;
                    It3Dex->SmoothItD = DEcombRec.bufIt3DSmoothItD;
                    It3Dex->OTrap = DEcombRec.bufIt3DOtrap;
                    mct->DEoptionResult = mct->DEoption;
                    mct->MaxItsResult = OTrapOutsideIterationSentinel(mct->iMaxIt);
                }
                break;
            case 4:
            case 5:
                if (Rst >= result) {
                    It3Dex->ItResultI = DEcombRec.bufIt3DItResultI;
                    mct->DEoptionResult = mct->DEoption;
                    mct->MaxItsResult = OTrapOutsideIterationSentinel(mct->iMaxIt);
                }
                wt = std::abs(result);
                if (mct->FormulaType == 4) {
                    result = MinCD(Rst - Clamp0D(mct->sDEcombSmooth - result),
                                   result - Clamp0D(mct->sDEcombSmooth - Rst));
                } else {
                    RD1 = Clamp0D(mct->sDEcombSmooth - result - mct->msDEstop);
                    dt = Clamp0D(mct->sDEcombSmooth - Rst - mct->msDEstop);
                    result = MinCD(Rst - RD1 * (mct->sDEcombSmooth - dt) / mct->sDEcombSmooth,
                                   result - dt * (mct->sDEcombSmooth - RD1) / mct->sDEcombSmooth);
                }
                Rst = std::abs(Rst);
                dt = 1.0 / (wt + Rst + 1e-10);
                It3Dex->SmoothItD = static_cast<float>(
                    (It3Dex->SmoothItD * wt + DEcombRec.bufIt3DSmoothItD * Rst) * dt);
                It3Dex->OTrap = (It3Dex->OTrap * wt + DEcombRec.bufIt3DOtrap * Rst) * dt;
                break;
            default:
                break;
        }
        OTrapRestoreF1DEcomb(&DEcombRec, mct);
    }

    if (mct->bCalcInside) {
        result = mct->msDEstop * 4.0 - result * 3.0;
    }

    return result;
}

// CalcOTrapDEnoADE -- orbit trap DE with numerical gradient
double CalcOTrapDEnoADE(TPIteration3Dext It3Dex, void* mctp) {
    RenderWork();
    PMCTparameter mct = static_cast<PMCTparameter>(mctp);

    mct->mMandFunction(reinterpret_cast<TPIteration3D>(&It3Dex->C1));

    if (mct->bInsideRendering && It3Dex->ItResultI == It3Dex->maxIt) {
        mct->MaxItsResult = It3Dex->maxIt;
        mct->DEoptionResult = It3Dex->DEoption;
        return mct->msDEstop * 0.25;
    }

    double result;
    if (It3Dex->Rout < 1e-200) {
        result = 0;
    } else {
        // Prepare for 3D gradient
        LongBool bufCalcSIT = *reinterpret_cast<LongBool*>(&It3Dex->CalcSIT);
        int bufMaxIt = It3Dex->maxIt;
        double bufRout = It3Dex->Rout;
        float bufSmoothItD = It3Dex->SmoothItD;
        It3Dex->CalcSIT = 0;
        It3Dex->maxIt = It3Dex->ItResultI;
        It3Dex->RStop = static_cast<float>(It3Dex->RStop * It3Dex->RStop * 64);

        // Save C1
        TVec3D bufVec;
        bufVec[0] = It3Dex->C1; bufVec[1] = It3Dex->C2; bufVec[2] = It3Dex->C3;

        // Gradient along Vgrads[2]
        It3Dex->C1 += mct->Vgrads[2][0] * mct->mctDEoffset;
        It3Dex->C2 += mct->Vgrads[2][1] * mct->mctDEoffset;
        It3Dex->C3 += mct->Vgrads[2][2] * mct->mctDEoffset;
        mct->mMandFunction(reinterpret_cast<TPIteration3D>(&It3Dex->C1));
        double dt = (bufRout - It3Dex->Rout);
        dt = dt * dt;

        // Gradient along Vgrads[1]
        It3Dex->C1 = bufVec[0] + mct->Vgrads[1][0] * mct->mctDEoffset;
        It3Dex->C2 = bufVec[1] + mct->Vgrads[1][1] * mct->mctDEoffset;
        It3Dex->C3 = bufVec[2] + mct->Vgrads[1][2] * mct->mctDEoffset;
        mct->mMandFunction(reinterpret_cast<TPIteration3D>(&It3Dex->C1));
        double wt = (bufRout - It3Dex->Rout);
        wt = wt * wt;

        // Gradient along Vgrads[0]
        It3Dex->C1 = bufVec[0] + mct->Vgrads[0][0] * mct->mctDEoffset;
        It3Dex->C2 = bufVec[1] + mct->Vgrads[0][1] * mct->mctDEoffset;
        It3Dex->C3 = bufVec[2] + mct->Vgrads[0][2] * mct->mctDEoffset;
        mct->mMandFunction(reinterpret_cast<TPIteration3D>(&It3Dex->C1));
        double Rst = (bufRout - It3Dex->Rout);
        Rst = Rst * Rst;

        result = bufRout * std::log(bufRout) * mct->dDEscale /
                 (std::sqrt(Rst + wt + dt) + mct->mctDEoffset006);

        It3Dex->Rout = bufRout;
        It3Dex->ItResultI = It3Dex->maxIt;
        It3Dex->maxIt = bufMaxIt;
        It3Dex->SmoothItD = bufSmoothItD;
        *reinterpret_cast<LongBool*>(&It3Dex->CalcSIT) = bufCalcSIT;
        It3Dex->RStop = mct->dRStop;
        It3Dex->C1 = bufVec[0]; It3Dex->C2 = bufVec[1]; It3Dex->C3 = bufVec[2];
    }

    mct->MaxItsResult = It3Dex->maxIt;
    double minDE = mct->msDEstop * 0.25;
    if (result < minDE) result = minDE;
    mct->DEoptionResult = It3Dex->DEoption;

    if (mct->bCalcInside) {
        result = mct->msDEstop * 4.0 - result * 3.0;
    }

    return result;
}

// ============================================================================
// Global formula function pointers (initialized with defaults)
// ============================================================================

TMandFunction   fIsMemberAlternating   = doHybridPas;
TMandFunctionDE fIsMemberAlternatingDE = doHybridPasDE;
TMandFunction   fIsMemberAlternating4D = doHybrid4DPas;
TMandFunction   fIsMemberIpol          = doInterpolHybridPas;
TMandFunctionDE fIsMemberIpolDE        = doInterpolHybridPasDE;

ThybridIteration fHybridCubeDE    = HybridCubeDE;
ThybridIteration fHybridCube      = HybridCube;
ThybridIteration fHybridQuat      = HybridQuat;
ThybridIteration fHybridItIntPow2 = HybridItIntPow2;
ThybridIteration fHybridTricorn   = HybridItTricorn;

ThybridIteration fHIntFunctions[7] = {
    HybridItIntPow2,   // power 2
    HybridItIntPow3,   // power 3
    HybridItIntPow4,   // power 4
    HybridIntP5,       // power 5
    HybridIntP6,       // power 6
    HybridIntP7,       // power 7
    HybridIntP8,       // power 8
};

} // namespace mb3d
