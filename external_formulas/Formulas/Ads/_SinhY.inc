// _SinhY.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: _SinhY
// Hand-ported from ASM: 399 bytes, DEoption=-1

constexpr unsigned int FORMULA__SINHY = 7028;

// ASM trace:
//   DD02        fld [edx]           → y
//   DD46E0      fld [esi-0x20]      → Mul
//   DEC9        fmulp               → y*Mul
//   DD46F0      fld [esi-0x10]      → fixY
//   DEC1        faddp               → A = y*Mul + fixY
//   D9C0        fld st0             → dup A
//   D8C8        fmul st0,st0        → A²
//   D9E8        fld1                → 1.0
//   DEC1        faddp               → A² + 1
//   D9FA        fsqrt               → √(A²+1)
//   DEC1        faddp               → A + √(A²+1) = arg for ln
//   DD46D8      fld [esi-0x28]      → Base
//   D9C9        fxch st1
//   D9F1        fyl2x               → Base * log2(A + √(A²+1))
//   DD46E8      fld [esi-0x18]      → fixLg
//   DEE9        fsubp st1,st0       → result - fixLg
//   DD1A        fstp [edx]          → y'
//
// Actually computes asinh: ln(A + √(A²+1)) scaled by Base.
// y' = Base * log2(Mul*y + fixY + √((Mul*y+fixY)² + 1)) - fixLg

template<>
cpu_double3 execute_formula<FORMULA__SINHY>(cpu_double3 p, constant double* params, double &de) {
    const double fixY  = params[0];
    const double fixLg = params[1];
    const double Mul   = params[2];
    const double Base  = params[3];

    double A = Mul * p.y + fixY;
    double arg = A + sqrt(A * A + 1.0);
    double result = Base * log2(arg) - fixLg;

    return cpu_double3(p.x, result, p.z);
}

// Formula metadata
constant FormulaInfo formula_info__SinhY = {
    .id = FORMULA__SINHY,
    .name = "_SinhY",
    .param_count = 4,
    .complexity = 90,
    .uses_transcendental = true
};

/*
Original M3 Description:
Hyperbolic sine (asinh) for y (formula by Luca GN 2013):
A = Mul*y + fixY
y' = Base * log2(A + sqrt(A*A + 1)) - fixLg

Parameters:
  [0] fixY: double (default: 0.0)
  [1] fixLg: double (default: 0.0)
  [2] Mul: double (default: -2.0)
  [3] Base: double (default: 0.367879441 = 1/e)
*/
