#ifndef MB3D_CORE_FORMULAS_H
#define MB3D_CORE_FORMULAS_H

#include "core/types.h"
#include <string>
#include <vector>

namespace mb3d {

// ============================================================================
// Formula functions (ported from formulas.pas)
// ============================================================================

// Main hybrid iteration drivers
void doHybridPas(TPIteration3D PIteration3D);
double doHybridPasDE(TPIteration3D PIteration3D);
void doHybrid4DPas(TPIteration3D PIteration3D);
double doHybrid4DDEPas(TPIteration3D PIteration3D);
void doInterpolHybridPas(TPIteration3D PIteration3D);
double doInterpolHybridPasDE(TPIteration3D PIteration3D);
void doInterpolHybridPas4D(TPIteration3D PIteration3D);
double doInterpolHybridPas4DDE(TPIteration3D PIteration3D);
double doHybridIFS3D(TPIteration3D PIteration3D);
double doHybridIFS3DnoVecIni(TPIteration3D PIteration3D);

// Individual formula implementations -- integer power Mandelbulbs
void HybridItIntPow2(double& x, double& y, double& z, double& w, TPIteration3D PIteration3D);
void HybridItIntPow3(double& x, double& y, double& z, double& w, TPIteration3D PIteration3D);
void HybridItIntPow4(double& x, double& y, double& z, double& w, TPIteration3D PIteration3D);
void HybridIntP5(double& x, double& y, double& z, double& w, TPIteration3D PIteration3D);
void HybridIntP6(double& x, double& y, double& z, double& w, TPIteration3D PIteration3D);
void HybridIntP7(double& x, double& y, double& z, double& w, TPIteration3D PIteration3D);
void HybridIntP8(double& x, double& y, double& z, double& w, TPIteration3D PIteration3D);

// Arbitrary real exponent Mandelbulb
void HybridFloatPow(double& x, double& y, double& z, double& w, TPIteration3D PIteration3D);

// Quaternion iteration
void HybridQuat(double& x, double& y, double& z, double& w, TPIteration3D PIteration3D);

// Tricorn / Mandelbar
void HybridItTricorn(double& x, double& y, double& z, double& w, TPIteration3D PIteration3D);

// Amazing Box / Mandelbox variants
void HybridCube(double& x, double& y, double& z, double& w, TPIteration3D PIteration3D);
void HybridCubeDE(double& x, double& y, double& z, double& w, TPIteration3D PIteration3D);
void HybridCubeSSE2(double& x, double& y, double& z, double& w, TPIteration3D PIteration3D);
void HybridCubeSSE2DE(double& x, double& y, double& z, double& w, TPIteration3D PIteration3D);

// Sine bulb with scaling (used by Bulbox)
void HybridItIntPow2scale(double& x, double& y, double& z, double& w, TPIteration3D PIteration3D);

// Bulbox = hybrid Mandelbulb + Mandelbox
void HybridSuperCube2(double& x, double& y, double& z, double& w, TPIteration3D PIteration3D);

// Box folding then sub-formula
void HybridFolding(double& x, double& y, double& z, double& w, TPIteration3D PIteration3D);

// Internal test formulas
void TestHybrid(double& x, double& y, double& z, double& w, TPIteration3D PIteration3D);
void HybridCustomIFStest(double& x, double& y, double& z, double& w, TPIteration3D PIteration3D);

// Aexion with C rotation
void AexionC(double& x, double& y, double& z, double& w, TPIteration3D PIteration3D);

// Empty placeholder
void EmptyFormula(double& x, double& y, double& z, double& w, TPIteration3D PIteration3D);

// Smooth iteration calculation
void CalcSmoothIterations(TPIteration3D PIt3D, int n);

// Diagnostic tracing for formula-level iteration on a selected pixel.
void SetFormulaTraceContext(int pixelX, int pixelY, std::vector<std::string>* lines);
bool FormulaTraceActive();
void AppendFormulaTraceEvent(const char* stage, const std::string& payload);
const char* HybridIterationName(ThybridIteration fn);

// Formula diagnostics also exist when schema parity tracing is compiled out.
// A task must not retain a completed task's borrowed output vector in TLS.
class ScopedFormulaTraceContext {
public:
    ScopedFormulaTraceContext() noexcept;
    ~ScopedFormulaTraceContext();
    ScopedFormulaTraceContext(const ScopedFormulaTraceContext&) = delete;
    ScopedFormulaTraceContext& operator=(const ScopedFormulaTraceContext&) = delete;
private:
    int previous_x_;
    int previous_y_;
    std::vector<std::string>* previous_lines_;
};

// ============================================================================
// Orbit trap DE functions (from OTrapDEcalc.pas)
// ============================================================================

double CalcOTrapDE(TPIteration3Dext It3Dex, void* mctp);
double CalcOTrapDEfull(TPIteration3Dext It3Dex, void* mctp);
double CalcOTrapDEnoADE(TPIteration3Dext It3Dex, void* mctp);
void   do3DalternateOTrap(TPIteration3D PIteration3D);

// ============================================================================
// Global formula function pointers (initialized with defaults)
// ============================================================================

extern TMandFunction   fIsMemberAlternating;
extern TMandFunctionDE fIsMemberAlternatingDE;
extern TMandFunction   fIsMemberAlternating4D;
extern TMandFunction   fIsMemberIpol;
extern TMandFunctionDE fIsMemberIpolDE;
extern ThybridIteration fHybridCubeDE;
extern ThybridIteration fHybridCube;
extern ThybridIteration fHybridQuat;
extern ThybridIteration fHybridItIntPow2;
extern ThybridIteration fHybridTricorn;
extern ThybridIteration fHIntFunctions[7]; // [2..8] mapped to [0..6]

} // namespace mb3d

#endif // MB3D_CORE_FORMULAS_H
